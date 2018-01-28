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



#include "CorpRegistryService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../EntityList.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../PyBoundObject.h"
#include "../common/EVEUtils.h"
#include "../cache/ObjCacheService.h"

#include "../packets/CorporationPkts.h"
#include "../packets/General.h"

PyCallable_Make_InnerDispatcher(CorpRegistryService)



class CorpRegistryBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(CorpRegistryBound)
	
	CorpRegistryBound(PyServiceMgr *mgr, CorporationDB *db)
	: PyBoundObject(mgr, "CorpRegistryBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(CorpRegistryBound, GetEveOwners)
		PyCallable_REG_CALL(CorpRegistryBound, GetCorporation)
		PyCallable_REG_CALL(CorpRegistryBound, GetCorporations)
		PyCallable_REG_CALL(CorpRegistryBound, GetInfoWindowDataForChar)
		PyCallable_REG_CALL(CorpRegistryBound, GetLockedItemLocations)
		PyCallable_REG_CALL(CorpRegistryBound, AddCorporation)
		PyCallable_REG_CALL(CorpRegistryBound, GetSuggestedTickerNames)
		PyCallable_REG_CALL(CorpRegistryBound, GetOffices)
		PyCallable_REG_CALL(CorpRegistryBound, GetStations)
		PyCallable_REG_CALL(CorpRegistryBound, GetMyApplications)
		PyCallable_REG_CALL(CorpRegistryBound, InsertApplication)
		PyCallable_REG_CALL(CorpRegistryBound, GetApplications)
		PyCallable_REG_CALL(CorpRegistryBound, UpdateApplicationOffer)
		PyCallable_REG_CALL(CorpRegistryBound, DeleteApplication)
		PyCallable_REG_CALL(CorpRegistryBound, UpdateApplication)
	}
	virtual ~CorpRegistryBound() {}
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL(GetEveOwners)
	PyCallable_DECL_CALL(GetCorporation)
	PyCallable_DECL_CALL(GetCorporations)
	PyCallable_DECL_CALL(GetInfoWindowDataForChar)
	PyCallable_DECL_CALL(GetLockedItemLocations)
	PyCallable_DECL_CALL(AddCorporation)
	PyCallable_DECL_CALL(GetSuggestedTickerNames)
	PyCallable_DECL_CALL(GetOffices)
	PyCallable_DECL_CALL(GetStations)
	PyCallable_DECL_CALL(GetMyApplications)
	PyCallable_DECL_CALL(InsertApplication)
	PyCallable_DECL_CALL(GetApplications)
	PyCallable_DECL_CALL(UpdateApplicationOffer)
	PyCallable_DECL_CALL(DeleteApplication)
	PyCallable_DECL_CALL(UpdateApplication)

protected:
	bool JoinCorporation(Client *who, uint32 newCorpID, const CorpMemberInfo &roles);
	static void FillOCApplicationChange(Notify_OnCorporationApplicationChanged & OCAC, const ApplicationInfo & Old, const ApplicationInfo & New);
	
	CorporationDB *const m_db;
	Dispatcher *const m_dispatch;
};

class SparseCorpOfficeListBound
: public PyBoundObject {
public:

	// or CorpRegistryBound?
	PyCallable_Make_Dispatcher(SparseCorpOfficeListBound)

	SparseCorpOfficeListBound(PyServiceMgr *mgr, CorporationDB *db)
	: PyBoundObject(mgr, "SparseCorpOfficeListBound"),
	  m_dispatch(new Dispatcher(this)),
	  m_db(db)
	{
		_SetCallDispatcher(m_dispatch);

		PyCallable_REG_CALL(SparseCorpOfficeListBound, Fetch)
		//PyCallable_REG_CALL(SparseCorpOfficeListBound, FetchByKey)
		//PyCallable_REG_CALL(SparseCorpOfficeListBound, GetByKey)
	}
	virtual ~SparseCorpOfficeListBound() {}
	virtual void Release() {
		delete this;
	}

	PyCallable_DECL_CALL(Fetch) //(startPos, fetchSize)
	//PyCallable_DECL_CALL(FetchByKey) //([keys])
	//PyCallable_DECL_CALL(GetByKey) //(key)


protected:
	Dispatcher *const m_dispatch;
	CorporationDB *const m_db;
};



CorpRegistryService::CorpRegistryService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "corpRegistry"),
m_dispatch(new Dispatcher(this)),
m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	//PyCallable_REG_CALL(CorpRegistryService, GetEveOwners)
}

CorpRegistryService::~CorpRegistryService() {
	delete m_dispatch;
}



PyBoundObject *CorpRegistryService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "CorpRegistryService bind request for:");
	bind_args->Dump(stdout, "    ");

	return(new CorpRegistryBound(m_manager, &m_db));
}


PyCallResult CorpRegistryBound::Handle_GetEveOwners(PyCallArgs &call) {
	return (m_db->GetEveOwners());
}

PyCallResult CorpRegistryBound::Handle_GetInfoWindowDataForChar(PyCallArgs &call) {
	//takes characterID

	_log(CLIENT__MESSAGE, "%s: GetInfoWindowDataForChar not implemented!", GetName());
	
	return(new PyRepNone());
}

PyCallResult CorpRegistryBound::Handle_GetLockedItemLocations(PyCallArgs &call) {
	//takes characterID

	_log(CLIENT__MESSAGE, "%s: GetLockedItemLocations not implemented!", GetName());

	//this returns an empty list for me on live.
	
	return(new PyRepList());
}

PyCallResult CorpRegistryBound::Handle_GetCorporation(PyCallArgs &call) {
	return(m_db->GetCorporation(call.client->GetCorporationID()));
}
PyCallResult CorpRegistryBound::Handle_GetCorporations(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Bad incoming params.");
		return NULL;
	}

	return(m_db->GetCorporation(arg.arg));
}


PyCallResult CorpRegistryBound::Handle_AddCorporation(PyCallArgs &call) {
	Call_AddCorporation args;

	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(new PyRepInteger(0));
	}

	//first make sure the char can even afford it.
	uint32 corp_costu;
	if(!m_db->GetConstant("corporationStartupCost", corp_costu)) {
		codelog(SERVICE__ERROR, "%s: Failed to determine corporation costs.", call.client->GetName());
		return(new PyRepInteger(0));
	}
	sint32 corp_cost = corp_costu;
	
	if(call.client->GetBalance() < double(corp_cost)) {
		_log(SERVICE__ERROR, "%s: Cannot afford corporation startup costs!", call.client->GetName());
		return(new PyRepInteger(0));
	}

	
	
	// Register new corp
	uint32 corpID;
	if (!m_db->AddCorporation(args, call.client->GetCharacterID(), call.client->GetStationID(), corpID)) {
		codelog(SERVICE__ERROR, "New corporation creation failed...");
		return (new PyRepInteger(0));
	}
	//adding a corporation might affect eveStaticOwners, so we gotta invalidate the cache...
	PyRepString cache_name("config.StaticOwners");
	m_manager->GetCache()->InvalidateCache(&cache_name);
	
	
	
	//take the money out of their wallet (sends wallet blink event)
	// The ammount has to be double!!!
	if(!call.client->AddBalance(double(-corp_cost))) {
		codelog(SERVICE__ERROR, "%s: Failed to take money for corp startup!", call.client->GetName());
		//TODO: do something? I dont wanna deal with deleting the corp right now...
	}

	//record the transaction in their journal.
	std::string reason = "Creating new corporation: ";
	reason += args.corpName;
	if(!m_db->GiveCash(
		call.client->GetCharacterID(),
		RefType_corpRegFee,
		call.client->GetCharacterID(),	//eve system
		1,
		"1",
		call.client->GetAccountID(),
		accountCash,
		-corp_cost,
		call.client->GetBalance(),
		reason.c_str()
		)
	) {
		codelog(CLIENT__ERROR, "Failed to record corp creation transaction.");
		//no good reason to return... the money has actually been moved.
	}

	uint32 location = call.client->GetLocationID();

	// Here we send a notification about creating a new corporation...
	Notify_OnCorporaionChanged cc;
	cc.corpID = corpID;
	if (!m_db->CreateCorporationCreatePacket(cc, call.client->GetCorporationID(), corpID)) {
		codelog(SERVICE__ERROR, "Failed to create OnCorpChanged notification stream.");
		// This is a big problem, because this way we won't be able to see the difference...
		call.client->SendErrorMsg("Unable to notify about corp creation. Try logging in again.");
		return (new PyRepInteger(0));
	}
	PyRepTuple* a1 = cc.Encode();
	PyRepTuple* a2 = cc.Encode();
	m_manager->entity_list->Multicast("OnCorporationChanged", "clientID", &a1, NOTIF_DEST__LOCATION, location);
	m_manager->entity_list->Multicast("OnCorporationChanged", "stationid", &a2, NOTIF_DEST__LOCATION, location);

	// Set char's roles in corp
	CorpMemberInfo roles;
	roles.corprole = corpRoleAll;
	roles.rolesAtAll = corpRoleAll;
	roles.rolesAtBase = corpRoleAll;
	roles.rolesAtHQ = corpRoleAll;
	roles.rolesAtOther = corpRoleAll;

	//loads up roles and alters session.
	if(!JoinCorporation(call.client, corpID, roles)) {
		codelog(CLIENT__ERROR, "Failed to force char '%s' to join new corporation %lu. This will be interesting.", call.client->GetName(), corpID);
		return (new PyRepInteger(0));
	}
	
	return (new PyRepInteger(corpID));
}

bool CorpRegistryBound::JoinCorporation(Client *who, uint32 newCorpID, const CorpMemberInfo &roles) {
	
	// Change current user's corporation in DB (incl. member counts)
	if(!m_db->JoinCorporation(
			who->GetCharacterID(), 
			newCorpID,
			who->GetCorporationID(),
			roles)
	) {
		codelog(CLIENT__ERROR, "Failed to update database for corporation join of '%s' to corp %lu", who->GetName(), newCorpID);
		who->SendErrorMsg("Unable to notify about corp creation. Try logging in again.");
		return(false);
	}

	who->JoinCorporationUpdate(newCorpID);
	return(true);
}

PyCallResult CorpRegistryBound::Handle_GetSuggestedTickerNames(PyCallArgs &call) {
	Call_SingleStringArg arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}

	PyRepList * result = new PyRepList;
	Item_GetSuggestedTickerNames sTN;
	sTN.tN = "";
	uint32 cnLen = arg.arg.length();
	// Easiest ticker-generation method: get the capital letters.
	for (uint32 i=0;i<cnLen;i++) {
		if (arg.arg[i] >= 'A' && arg.arg[i] <= 'Z') {
			sTN.tN += arg.arg[i];
		}
	}
	result->add(sTN.Encode());

	return result;
}

PyCallResult CorpRegistryBound::Handle_GetStations(PyCallArgs &call) {
	// No param

	// Need to fetch stations of current corporation...
	return m_db->GetStations(call.client->GetCorporationID());
}

PyCallResult CorpRegistryBound::Handle_GetOffices(PyCallArgs &call) {
	
	_log(CLIENT__MESSAGE, "CorpRegistryBound bind request for OfficeList of corp %lu", call.client->GetCorporationID());

	// First create the boundable object

	PyBoundObject *bObj;
	bObj = new SparseCorpOfficeListBound(m_manager, m_db);
	if(bObj == NULL) {
		_log(SERVICE__ERROR, "%s Service: %s: Unable to create bound object for:", GetName(), call.client->GetName());
		return(NULL);
	}

	CorpOfficeSparseRowset ret;

	//now we register 
	PyRepDict *dict = new PyRepDict();

	// First time we only need the number of rows, not the data itself
	// Data will be fetched from the SparseRowset
	uint32 officeN = m_db->GetOffices(call.client->GetCorporationID());

	// No idea what this is
	dict->add("realRowCount", new PyRepInteger(officeN));
	// But this one holds the real row number
	ret.officeNumber = officeN;
	
	ret.bindedObject = m_manager->BindObject(call.client, bObj, &dict);

	//call.client->temp_hack_officeLists[call.client->GetCorporationID()] = bindID; //m_manager->FindBoundObject(bObj);

	PyRepObject * res = ret.Encode();
	return res;
}

PyCallResult SparseCorpOfficeListBound::Handle_Fetch(PyCallArgs &call) {
	Call_TwoIntegerArgs args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}

	return m_db->Fetch(call.client->GetCorporationID(), args.arg1, args.arg2);
}

PyCallResult CorpRegistryBound::Handle_GetMyApplications(PyCallArgs &call) {
	/// We have a dict
	/// With an STI and an integer
	/// Ignore them for now
	return m_db->GetMyApplications(call.client->GetCharacterID());
}

PyCallResult CorpRegistryBound::Handle_InsertApplication(PyCallArgs &call) {
	/** Incoming:
	 *  Integer: 777777777 <- corp id
	 *  String: "Ignore me" <- text that was entered into the box
	 */

	Call_InsertApplication res;
	if (!res.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}	

	/// Insert query into the db
	ApplicationInfo aInfo;
	aInfo.charID = call.client->GetCharacterID();
	aInfo.corpID = res.corpID;
	aInfo.appText = res.message;
	if (!m_db->InsertApplication(aInfo)) {
		codelog(SERVICE__ERROR, "%s: Failed to insert application request", call.client->GetName());
		return NULL;
	}

	/// BroadcastStuff::Notify( OnCorporationApplicationChanged ,...)
	Notify_OnCorporationApplicationChanged OCAC;
	ApplicationInfo oldInfo(false);
	FillOCApplicationChange(OCAC, oldInfo, aInfo);
	OCAC.corpID = res.corpID;
	OCAC.charID = aInfo.charID;
	
	PyRepTuple* notif = OCAC.Encode();
	// Who needs to know this?
	// Everyone who's in that corporation, right?
	
	MulticastTarget mct;
	mct.characters.insert(OCAC.charID);
	mct.corporations.insert(OCAC.corpID);
	m_manager->entity_list->Multicast(
		"OnCorporationApplicationChanged", 
		"clientID", &notif, mct);
	

	/// need to find out what happens on the other side
	/// if there's anything at all on the other side

	/// Send an evemail to those who can decide
	/// Well, for the moment, send it to the ceo
	std::string subject = "New application from " + call.client->GetChar().name,
				body = res.message;
	std::vector<uint32> recipients;
	recipients.push_back(m_db->GetCorporationCEO(res.corpID));
	m_manager->SendNewEveMail(call.client->GetCharacterID(), recipients, subject, body);

	/// Reply: ~\x00\x00\x00\x00\x01
	return NULL;
}

void CorpRegistryBound::FillOCApplicationChange(Notify_OnCorporationApplicationChanged & OCAC, const ApplicationInfo & Old, const ApplicationInfo & New) {
	if (Old.valid) {
		OCAC.applicationDateTimeOld = new PyRepInteger(Old.appTime);
		OCAC.applicationTextOld = new PyRepString(Old.appText);
		OCAC.characterIDOld = new PyRepInteger(Old.charID);
		OCAC.corporationIDOld = new PyRepInteger(Old.corpID);
		OCAC.deletedOld = new PyRepInteger(Old.deleted);
		OCAC.grantableRolesOld = new PyRepInteger(Old.grantRole);
		if (Old.lastCID) {
			OCAC.lastCorpUpdaterIDOld = new PyRepInteger(Old.lastCID);
		} else {
			OCAC.lastCorpUpdaterIDOld = new PyRepNone();
		}
		OCAC.rolesOld = new PyRepInteger(Old.role);
		OCAC.statusOld = new PyRepInteger(Old.status);
	} else {
		OCAC.applicationDateTimeOld = new PyRepNone();
		OCAC.applicationTextOld = new PyRepNone();
		OCAC.characterIDOld = new PyRepNone();
		OCAC.corporationIDOld = new PyRepNone();
		OCAC.deletedOld = new PyRepNone();
		OCAC.grantableRolesOld = new PyRepNone();
		OCAC.lastCorpUpdaterIDOld = new PyRepNone();
		OCAC.rolesOld = new PyRepNone();
		OCAC.statusOld = new PyRepNone();
	}

	if (New.valid) {
		OCAC.applicationDateTimeNew = new PyRepInteger(New.appTime);
		OCAC.applicationTextNew = new PyRepString(New.appText);
		OCAC.characterIDNew = new PyRepInteger(New.charID);
		OCAC.corporationIDNew = new PyRepInteger(New.corpID);
		OCAC.deletedNew = new PyRepInteger(New.deleted);
		OCAC.grantableRolesNew = new PyRepInteger(New.grantRole);
		if (New.lastCID) {
			OCAC.lastCorpUpdaterIDNew = new PyRepInteger(New.lastCID);
		} else {
			OCAC.lastCorpUpdaterIDNew = new PyRepNone();
		}
		OCAC.rolesNew = new PyRepInteger(New.role);
		OCAC.statusNew = new PyRepInteger(New.status);
	} else {
		OCAC.applicationDateTimeNew = new PyRepNone();
		OCAC.applicationTextNew = new PyRepNone();
		OCAC.characterIDNew = new PyRepNone();
		OCAC.corporationIDNew = new PyRepNone();
		OCAC.deletedNew = new PyRepNone();
		OCAC.grantableRolesNew = new PyRepNone();
		OCAC.lastCorpUpdaterIDNew = new PyRepNone();
		OCAC.rolesNew = new PyRepNone();
		OCAC.statusNew = new PyRepNone();
	}
}

PyCallResult CorpRegistryBound::Handle_GetApplications(PyCallArgs &call) {
	/** Incoming:
	 *  Nothing at all
	 */
	return m_db->GetApplications(call.client->GetCorporationID());
}

/** AppInfo:
 *  status / corp side / user side
 *    0        new         applied
 *    1        update      reneg
 *    2        accepted    accepted
 *    4        error       reject
 *    6        offer       offer
 */
typedef enum {	//from eveConstants
	crpApplicationAppliedByCharacter = 0,
	crpApplicationRenegotiatedByCharacter = 1,
	crpApplicationAcceptedByCharacter = 2,
	crpApplicationRejectedByCharacter = 3,
	crpApplicationRejectedByCorporation = 4,
	crpApplicationRenegotiatedByCorporation = 5,
	crpApplicationAcceptedByCorporation = 6
} CorpApplicationStatus;
PyCallResult CorpRegistryBound::Handle_UpdateApplicationOffer(PyCallArgs &call) {
	/** Incoming:
	 *  Tuple
	 *   - int 140000017	<- this is the charID, whose app should be handled
	 *   - string message
	 *   - int decision
	 *		4: rejection
	 *		6: acception
	 *   - (none), so far
	 */

	Call_UpdateApplicationOffer args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}

	// OnCorporationApplicationChanged event, prolly be good to make it two (or more) times, independently, depending on update type

	Notify_OnCorporationApplicationChanged OCAC;
	PyRepTuple * answer;


	switch (args.newStatus) {
	case crpApplicationRejectedByCorporation:
	{
		ApplicationInfo newInfo(true);
		ApplicationInfo oldInfo(true);
		ApplicationInfo invalidInfo(false);
		OCAC.charID = args.charID;
		OCAC.corpID = call.client->GetCorporationID();
		if(!m_db->GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
			codelog(SERVICE__ERROR, "%s: Failed to query application for char %lu corp %lu", call.client->GetName(), OCAC.charID, OCAC.corpID);
			return NULL;
		}
		newInfo = oldInfo;
		newInfo.status = crpApplicationRejectedByCorporation;
		newInfo.lastCID = call.client->GetCharacterID();
		
		if(!m_db->UpdateApplication(newInfo)) {
			codelog(SERVICE__ERROR, "%s: Failed to update application", call.client->GetName());
			return NULL;
		}
		
		FillOCApplicationChange(OCAC, oldInfo, newInfo);
		answer = OCAC.Encode();
		m_manager->entity_list->Unicast(OCAC.charID, 
			"OnCorporationApplicationChanged", 
			"*corpid&corprole", &answer);
		
		FillOCApplicationChange(OCAC, oldInfo, invalidInfo);
		answer = OCAC.Encode();
		// Maybe this will remove the app from the corp
		m_manager->entity_list->Multicast(
			"OnCorporationApplicationChanged", 
			"*corpid&corprole", &answer, 
			NOTIF_DEST__CORPORATION, OCAC.corpID);
		break;
	}
	case crpApplicationAcceptedByCorporation: /// accepted
	{
		// the acceptor corporation MUST have free space!!


		/// OnCorporationApplicationChanged
		ApplicationInfo newInfo(true);
		ApplicationInfo oldInfo(true);
		OCAC.charID = args.charID;
		OCAC.corpID = call.client->GetCorporationID();
		if(!m_db->GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
			codelog(SERVICE__ERROR, "%s: Failed to query application info for char %lu corp %lu", call.client->GetName(), OCAC.charID, OCAC.corpID);
			return NULL;
		}
		newInfo = oldInfo;
		newInfo.status = crpApplicationAcceptedByCharacter;
		newInfo.lastCID = call.client->GetCharacterID();
		
		if(!m_db->UpdateApplication(newInfo)) {
			codelog(SERVICE__ERROR, "%s: Failed to update application for char %lu corp %lu", call.client->GetName(), OCAC.charID, OCAC.corpID);
			return NULL;
		}
		
		FillOCApplicationChange(OCAC, oldInfo, newInfo);
		
		answer = OCAC.Encode();
		MulticastTarget mct;
		mct.characters.insert(OCAC.charID);
		mct.corporations.insert(OCAC.corpID);
		m_manager->entity_list->Multicast(
			"OnCorporationApplicationChanged", 
			"*corpid&corprole", &answer, mct);

//TODO: should prolly put this into a function, since there may be other
//places (gm commands at a minimum) where we want to change corp.
/** TODO: Update employment history object, if present
 */
		// OnObjectPublicAttributesUpdated event
		Notify_OnObjectPublicAttributesUpdated N_pau;
		MemberAttributeUpdate change;

		N_pau.realRowCount = 4;
		N_pau.bindID = GetBindID();
		N_pau.changePKIndexValue = args.charID;

		if (!m_db->CreateMemberAttributeUpdate(change, oldInfo.corpID, args.charID)) {
			codelog(SERVICE__ERROR, "Couldn't get data from the character. Sorry.");
			return NULL;
		}

		N_pau.changes = change.Encode();

		answer = N_pau.Encode();
		m_manager->entity_list->Multicast(
			"OnObjectPublicAttributesUpdated", 
			"objectID", &answer, 
			NOTIF_DEST__CORPORATION, OCAC.corpID);

		// OnCorporationMemberChanged event
		Notify_OnCorpMemberChange ocmc;

		ocmc.charID = args.charID;
		ocmc.newCorpID = ((PyRepInteger *)change.corporationIDNew)->value;
		ocmc.oldCorpID = ((PyRepInteger *)change.corporationIDOld)->value;
		ocmc.newDate = ((PyRepInteger *)OCAC.applicationDateTimeNew)->value;
		ocmc.oldDate = ((PyRepInteger *)OCAC.applicationDateTimeOld)->value;


		// both corporations' members will be notified about the change
		MulticastTarget both_corps;
		both_corps.corporations.insert(ocmc.newCorpID);
		both_corps.corporations.insert(ocmc.oldCorpID);
		answer = ocmc.Encode();
		m_manager->entity_list->Multicast(
			"OnCorporationMemberChanged", "corpid", 
			&answer, both_corps);

		
		// OnCorporationMemberChanged event again
		// this is the same as the first one
		// and goes twice
		// HAHA: just because they do it dosent mean we need to...
		Notify_IntRaw notif;
		notif.key = args.charID;
		notif.data = change.Encode();

		answer = notif.Encode();
		m_manager->entity_list->Multicast(
			"OnCorporationMemberChanged", "corpid", 
			&answer, both_corps);

		answer = notif.Encode();
		m_manager->entity_list->Multicast(
			"OnCorporationMemberChanged", "corpid", 
			&answer, both_corps);

		//NOTE: this really should happen sooner, in case it fails.
		if(!m_db->JoinCorporation(args.charID, ocmc.newCorpID, ocmc.oldCorpID, CorpMemberInfo())) {
			codelog(SERVICE__ERROR, "%s: Failed to record corp join for char %lu corp %lu", call.client->GetName(), OCAC.charID, OCAC.corpID);
			return NULL;
		}
		
		Client *recruit = m_manager->entity_list->FindCharacter(ocmc.charID);
		if(recruit != NULL) {
			recruit->JoinCorporationUpdate(ocmc.newCorpID);
		}
		
		break;
		}
	}
	return new PyRepInteger(1);
}

PyCallResult CorpRegistryBound::Handle_DeleteApplication(PyCallArgs & call) {
	/** Incoming:
	 *  tuple of 2 elements, corpID and charID
	 */
	Call_TwoIntegerArgs args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(new PyRepInteger(0));
	}

	Notify_OnCorporationApplicationChanged OCAC;

	ApplicationInfo newInfo(false);
	ApplicationInfo oldInfo(true);
	OCAC.corpID = args.arg1;
	OCAC.charID = args.arg2;
	if(!m_db->GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
		codelog(SERVICE__ERROR, "%s: Failed to query application info for char %lu corp %lu", call.client->GetName(), OCAC.charID, OCAC.corpID);
		return(new PyRepInteger(0));
	}

	FillOCApplicationChange(OCAC, oldInfo, newInfo);
	
	if(!m_db->DeleteApplication(oldInfo)) {
		codelog(SERVICE__ERROR, "%s: Failed to delete application info for char %lu corp %lu", call.client->GetName(), OCAC.charID, OCAC.corpID);
		return(new PyRepInteger(0));
	}

	PyRepTuple * answer = OCAC.Encode();
	MulticastTarget mct;
	mct.characters.insert(OCAC.charID);
	mct.corporations.insert(OCAC.corpID);
	m_manager->entity_list->Multicast(
		"OnCorporationApplicationChanged", 
		"*corpid&corprole", &answer, mct);
	
	return new PyRepInteger(1);
}

PyCallResult CorpRegistryBound::Handle_UpdateApplication(PyCallArgs &call) {
	Call_UpdateApplication args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}

	ApplicationInfo oldInfo(true);
	ApplicationInfo newInfo;
	Notify_OnCorporationApplicationChanged OCAC;
	OCAC.charID = call.client->GetCharacterID();
	OCAC.corpID = args.corpID;
	if(!m_db->GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
		codelog(SERVICE__ERROR, "%s: Failed to query application info for char %lu corp %lu", call.client->GetName(), OCAC.charID, OCAC.corpID);
		return(new PyRepInteger(0));
	}
	newInfo = oldInfo;
	newInfo.appText = args.message;
	newInfo.status = args.status;

	if(!m_db->UpdateApplication(newInfo)) {
		codelog(SERVICE__ERROR, "%s: Failed to update application info for char %lu corp %lu", call.client->GetName(), OCAC.charID, OCAC.corpID);
		return(new PyRepInteger(0));
	}
	
	FillOCApplicationChange(OCAC, oldInfo, newInfo);

	PyRepTuple* notif = OCAC.Encode();
	m_manager->entity_list->Unicast(OCAC.charID,
		"OnCorporationApplicationChanged", 
		"clientID", &notif);

	FillOCApplicationChange(OCAC, ApplicationInfo(false), newInfo);
	notif = OCAC.Encode();
	m_manager->entity_list->Multicast(
		"OnCorporationApplicationChanged", 
		"clientID", &notif, 
		NOTIF_DEST__CORPORATION, OCAC.corpID);

	return NULL;
}








