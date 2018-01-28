/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "ReprocessingService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../PyBoundObject.h"
#include "../packets/General.h"
#include "../packets/Inventory.h"
#include "../inventory/InventoryItem.h"

PyCallable_Make_InnerDispatcher(ReprocessingService)


class ReprocessingServiceBound
: public PyBoundObject {
public:
	ReprocessingServiceBound(PyServiceMgr *mgr, ReprocessingDB *db, const uint32 stationID);
	virtual ~ReprocessingServiceBound();

	PyCallable_DECL_CALL(GetOptionsForItemTypes)
	PyCallable_DECL_CALL(GetReprocessingInfo)
	PyCallable_DECL_CALL(GetQuote)
	PyCallable_DECL_CALL(Reprocess)

	virtual void Release();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;
	const uint32 m_stationID;
	ReprocessingDB *const m_db;
	double m_staEfficiency;
	double m_tax;
	
	double _CalcReprocessingEfficiency(InventoryItem *client, const uint32 groupID = 0) const;
	PyRep *_BuildGetQuotePacket(const std::vector<Recoverable> &recoverables, const uint32 portionSize, const uint32 itemQuantity, const double efficiency) const;
};

PyCallable_Make_InnerDispatcher(ReprocessingServiceBound)

ReprocessingService::ReprocessingService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "reprocessingSvc"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);
}

ReprocessingService::~ReprocessingService() {
	delete m_dispatch;
}

PyBoundObject *ReprocessingService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	if(!bind_args->CheckType(PyRep::Integer)) {
		codelog(CLIENT__ERROR, "%s: Non-integer bind argument '%s'", c->GetName(), bind_args->TypeString());
		return(NULL);
	}

	const PyRepInteger * stationID = (const PyRepInteger *) bind_args;

	if(!IsStation(stationID->value)) {
		codelog(CLIENT__ERROR, "%s: Expected stationID, but hasn't.", c->GetName());
		return(NULL);
	}

	return(new ReprocessingServiceBound(m_manager, &m_db, stationID->value));
}

//******************************************************************************

ReprocessingServiceBound::ReprocessingServiceBound(PyServiceMgr *mgr, ReprocessingDB *db, const uint32 stationID)
: PyBoundObject(mgr, "reprocessingSvcBound"),
  m_dispatch(new Dispatcher(this)),
  m_stationID(stationID),
  m_db(db),
  m_staEfficiency(0),
  m_tax(0)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(ReprocessingServiceBound, GetOptionsForItemTypes)
	PyCallable_REG_CALL(ReprocessingServiceBound, GetReprocessingInfo)
	PyCallable_REG_CALL(ReprocessingServiceBound, GetQuote)
	PyCallable_REG_CALL(ReprocessingServiceBound, Reprocess)
//TODO: move this call somewhere else so the return code can be checked.
	if(!m_db->LoadStatic(m_stationID, m_staEfficiency, m_tax))
		_log(SERVICE__ERROR, "Failed to load static info for station %lu.", m_stationID);
}

ReprocessingServiceBound::~ReprocessingServiceBound() {
}

void ReprocessingServiceBound::Release() {
	//I hate this statement
	delete this;
}

PyCallResult ReprocessingServiceBound::Handle_GetOptionsForItemTypes(PyCallArgs &call) {
	PyRep *result = NULL;

	Call_GetOptionsForItemTypes call_args;
	if(!call_args.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Unable to decode args.");
		return(NULL);
	}

	PyRepDict *dict = new PyRepDict;
	result = dict;

	PyRepDict::const_iterator	cur = call_args.typeIDs.begin();
	PyRepDict::const_iterator end = call_args.typeIDs.end();
	for(; cur != end; cur++) {
		if(!cur->first->CheckType(PyRep::Integer)) {
			_log(SERVICE__ERROR, "Unexpected argument, skipping.");
			continue;
		}

		Call_GetOptionsForItemTypes_Arg arg;
		arg.isRecyclable = m_db->IsRecyclable(	((PyRepInteger *)cur->first)->value	);
		arg.isRefinable = m_db->IsRefinable(	((PyRepInteger *)cur->first)->value	);

		dict->add(cur->first->Clone(), arg.Encode());
	}

	return(result);
}

PyCallResult ReprocessingServiceBound::Handle_GetReprocessingInfo(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepDict *dict = new PyRepDict;
	result = dict;

	PyRepList *wetake = new PyRepList;
	dict->add("wetake", wetake);
	dict->add("yield", new PyRepReal(m_staEfficiency));
	dict->add("combinedyield", new PyRepReal(_CalcReprocessingEfficiency(call.client->Item())));
	wetake->addReal(m_tax);
	wetake->addInt(0);

	return(result);
}

PyCallResult ReprocessingServiceBound::Handle_GetQuote(PyCallArgs &call) {
	PyRep *result = NULL;

	Call_SingleIntegerArg call_args;	// itemID
	if(!call_args.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Unable to decode args.");
		return(NULL);
	}

	InventoryItem *item = call.client->Item()->factory->Load(call_args.arg, true);
	if(item == NULL) {
		_log(SERVICE__ERROR, "Failed to load item ID %lu.", call_args.arg);
		return(NULL);
	}

	if(item->ownerID() != call.client->GetCharacterID()) {
		_log(SERVICE__ERROR, "Character %lu tried to reprocess item %lu of character %lu.", call.client->GetCharacterID(), item->itemID(), item->ownerID());
		item->Release();
		return(NULL);
	}

	uint32 portionSize = m_db->GetPortionSize(item->typeID());
	if(portionSize == 0) {
		_log(SERVICE__MESSAGE, "Failed to get portion size of item ID %lu.", item->typeID());
		item->Release();
		return(NULL);
	}

	std::vector<Recoverable> recoverables;
	if(!m_db->GetRecoverables(item->typeID(), recoverables)) {
		codelog(SERVICE__ERROR, "Failed to query recoverables.");
		item->Release();
		return(NULL);
	}
	result = _BuildGetQuotePacket(recoverables,
								  portionSize,
								  item->quantity(),
								  _CalcReprocessingEfficiency(call.client->Item(), item->groupID()));
	if(result == NULL) {
		_log(SERVICE__ERROR, "Failed to build GetQuote packet.");
		item->Release();
		return(NULL);
	}

	item->Release();

	return(result);
}

PyCallResult ReprocessingServiceBound::Handle_Reprocess(PyCallArgs &call) {
	if(!IsStation(call.client->GetLocationID())) {
		_log(SERVICE__MESSAGE, "Character %s tried to reprocess, but isn't is station.", call.client->GetName());
		return(NULL);
	}

	Call_Reprocess call_args;

	if(!call_args.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Failed to parse args.");
		return(NULL);
	}

	std::vector<uint32>::iterator cur, end;
	cur = call_args.itemIDs.begin();
	end = call_args.itemIDs.end();
	for(; cur != end; cur++) {
		InventoryItem *item = m_manager->item_factory->Load(*cur, true);
		if(item == NULL) {
			_log(SERVICE__ERROR, "Couldn't load item ID %lu. Skipping.", *cur);
			continue;
		}
		if(item->ownerID() != call.client->GetCharacterID()) {
			_log(SERVICE__ERROR, "Character %lu tried to reprocess item %lu of character %lu. Skipping.", call.client->GetCharacterID(), item->itemID(), item->ownerID());
			continue;
		}

		uint32 portionSize = m_db->GetPortionSize(item->typeID());
		if(portionSize == 0) {
			_log(SERVICE__ERROR, "Unable to get portion size of item %lu. Skipping.", item->typeID());
			item->Release();
			continue;
		}
		if(portionSize > item->quantity()) {
			call.client->SendErrorMsg("You need at least %lu units to reprocess %s.", portionSize, item->itemName().c_str());
			item->Release();
			continue;
		}

		double efficiency = _CalcReprocessingEfficiency(call.client->Item(), item->groupID());
		
		std::vector<Recoverable> recoverables;
		if(!m_db->GetRecoverables(item->typeID(), recoverables)) {
			_log(SERVICE__ERROR, "Unable to get recoverables for typeID %lu. Skipping.", item->typeID());
			item->Release();
			continue;
		}

		std::vector<Recoverable>::iterator cur_rec, end_rec;
		cur_rec = recoverables.begin();
		end_rec = recoverables.end();
		for(; cur_rec != end_rec; cur_rec++) {
			uint32 quantity = uint32(cur_rec->amountPerBatch * efficiency * (1.0 - m_tax) * (double)item->quantity() / (double)portionSize);
			InventoryItem *i = m_manager->item_factory->Spawn(cur_rec->typeID,
				quantity,
				call.client->GetCharacterID(),
				0,
				flagHangar);
			if(i == NULL) {
				_log(SERVICE__ERROR, "Unable to spawn item type %lu. Skipping", cur_rec->typeID);
				continue;
			}
			i->Move(call.client->GetStationID(), flagHangar);
			i->Release();
		}
		//delete recoverables;

		item->Move(6, flagAutoFit);
		item->ChangeOwner(2);
		item->Delete();
	}

	return(NULL);
}

double ReprocessingServiceBound::_CalcReprocessingEfficiency(InventoryItem *client, const uint32 groupID) const {
/*
	 3385		Refining Skill
	 3389		Refinery Efficiency Skill
	12180		Arkonor Processing
	12181		Bistot Processing
	12182		Crokite Processing
	12183		Dark Ochre Processing
	12184		Gneiss Processing
	12185		Hedbergite Processing
	12186		Hemorphite Processing
	12187		Jaspet Processing
	12188		Kernite Processing
	12189		Mercoxit Processing
	12190		Omber Processing
	12191		Plagioclase Processing
	12192		Pyroxeres Processing
	12193		Scordite Processing
	12194		Spodumain Processing
	12195		Veldspar Processing
	12196		Scrapmetal Processing - Applies on everything except ores and ice
	18025		Ice Processing
*/
	// formula is: reprocessingEfficiency + 0.375*(1 + 0.02*RefiningSkill)*(1 + 0.04*RefineryEfficiencySkill)*(1 + 0.05*OreProcessingSkill)
	double efficiency =	0.375*(1 + 0.02*client->GetSkillLevel(3385))*
						(1 + 0.04*client->GetSkillLevel(3389));

	switch(groupID) {
		case 450:	efficiency *= (1 + 0.05*client->GetSkillLevel(12180));
					break;
		case 451:	efficiency *= (1 + 0.05*client->GetSkillLevel(12181));
					break;
		case 452:	efficiency *= (1 + 0.05*client->GetSkillLevel(12182));
					break;
		case 453:	efficiency *= (1 + 0.05*client->GetSkillLevel(12183));
					break;
		case 467:	efficiency *= (1 + 0.05*client->GetSkillLevel(12184));
					break;
		case 454:	efficiency *= (1 + 0.05*client->GetSkillLevel(12185));
					break;
		case 455:	efficiency *= (1 + 0.05*client->GetSkillLevel(12186));
					break;
		case 456:	efficiency *= (1 + 0.05*client->GetSkillLevel(12187));
					break;
		case 457:	efficiency *= (1 + 0.05*client->GetSkillLevel(12188));
					break;
		case 468:	efficiency *= (1 + 0.05*client->GetSkillLevel(12189));
					break;
		case 469:	efficiency *= (1 + 0.05*client->GetSkillLevel(12190));
					break;
		case 458:	efficiency *= (1 + 0.05*client->GetSkillLevel(12191));
					break;
		case 459:	efficiency *= (1 + 0.05*client->GetSkillLevel(12192));
					break;
		case 460:	efficiency *= (1 + 0.05*client->GetSkillLevel(12193));
					break;
		case 461:	efficiency *= (1 + 0.05*client->GetSkillLevel(12194));
					break;
		case 462:	efficiency *= (1 + 0.05*client->GetSkillLevel(12195));
					break;
		case 465:	efficiency *= (1 + 0.05*client->GetSkillLevel(18025));
					break;
		default:	efficiency *= (1 + 0.05*client->GetSkillLevel(12196));
		case 0:		break;
	}
	
	efficiency += m_staEfficiency;

	if(efficiency > 1)
		efficiency = 1.0;

	return(efficiency);
}

PyRep *ReprocessingServiceBound::_BuildGetQuotePacket(const std::vector<Recoverable> &recoverables, const uint32 portionSize, const uint32 itemQuantity, const double efficiency) const {
	Rsp_GetQuote res;
	res.quantityToProcess = itemQuantity;

	std::vector<Recoverable>::const_iterator cur, end;
	cur = recoverables.begin();
	end = recoverables.end();

	for(; cur != end; cur++) {
		Rsp_GetQuote_Line line;

		line.typeID =			cur->typeID;
		double ratio = cur->amountPerBatch * (double)itemQuantity / (double)portionSize;
		line.unrecoverable =	uint32((1.0 - efficiency)			* ratio);
		line.station =			uint32(efficiency * m_tax			* ratio);
		line.client =			uint32(efficiency * (1.0 - m_tax)	* ratio);

		res.lines.add(line.Encode());
	}

	
	

	return(res.Encode());
}

