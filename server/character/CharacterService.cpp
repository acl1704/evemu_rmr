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


#include "CharacterService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../common/EVEUtils.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../cache/ObjCacheService.h"
#include "../EntityList.h"
#include "../common/EVEUtils.h"
#include "../inventory/InventoryItem.h"
#include "../inventory/ItemFactory.h"

#include "../packets/General.h"
#include "../packets/Character.h"

PyCallable_Make_InnerDispatcher(CharacterService)


CharacterService::CharacterService(PyServiceMgr *mgr, DBcore *dbc)
: PyService(mgr, "character"),
  m_dispatch(new Dispatcher(this)),
  m_db(dbc)
{
	_SetCallDispatcher(m_dispatch);
	
	PyCallable_REG_CALL(CharacterService, GetCharactersToSelect)
	PyCallable_REG_CALL(CharacterService, GetCharacterToSelect)
	PyCallable_REG_CALL(CharacterService, SelectCharacterID)
	PyCallable_REG_CALL(CharacterService, GetOwnerNoteLabels)
	PyCallable_REG_CALL(CharacterService, GetCharCreationInfo)
	PyCallable_REG_CALL(CharacterService, GetAppearanceInfo)
	PyCallable_REG_CALL(CharacterService, ValidateName)
	PyCallable_REG_CALL(CharacterService, CreateCharacter)
	PyCallable_REG_CALL(CharacterService, PrepareCharacterForDelete)
	PyCallable_REG_CALL(CharacterService, CancelCharacterDeletePrepare)
	PyCallable_REG_CALL(CharacterService, AddOwnerNote)
	PyCallable_REG_CALL(CharacterService, EditOwnerNote)
	PyCallable_REG_CALL(CharacterService, GetOwnerNote)
	PyCallable_REG_CALL(CharacterService, GetHomeStation)
	PyCallable_REG_CALL(CharacterService, GetCloneTypeID)
	PyCallable_REG_CALL(CharacterService, GetCharacterAppearanceList)

	PyCallable_REG_CALL(CharacterService, GetCharacterDescription)//mandela
	PyCallable_REG_CALL(CharacterService, SetCharacterDescription)//mandela

	PyCallable_REG_CALL(CharacterService, GetNote)//LSMoura
	PyCallable_REG_CALL(CharacterService, SetNote)//LSMoura
}

CharacterService::~CharacterService() {
	delete m_dispatch;
}

PyCallResult CharacterService::Handle_GetCharactersToSelect(PyCallArgs &call) {
	
	PyRep *result = NULL;

	result = m_db.GetCharacterList(call.client->GetAccountID());
	if(result == NULL) {
		//TODO: throw exception
		result = new PyRepNone();
	}
	
	return(result);
}

PyCallResult CharacterService::Handle_GetCharacterToSelect(PyCallArgs &call) {
	PyRep *result = NULL;
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}
	
	result = m_db.GetCharSelectInfo(args.arg);
	if(result == NULL) {
		_log(CLIENT__ERROR, "Failed to load character %d", args.arg);
		return(NULL);
	}
	
	return(result);
}

PyCallResult CharacterService::Handle_SelectCharacterID(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Failed to parse args from account %lu", call.client->GetAccountID());
		//TODO: throw exception
		return(NULL);
	}
	
	uint32 ship_id = m_db.GetCurrentShipID(args.arg);
	_log(CLIENT__MESSAGE, "Determined that %lu is the current ship for char %lu", ship_id, args.arg);
	
	if(!m_db.LoadCharacter(args.arg, call.client->GetChar())) {
		_log(CLIENT__ERROR, "Failed to load char %lu for selection", args.arg);
		//TODO: throw exception
	} else {
		if(!call.client->LoadInventory(ship_id)) {
			_log(CLIENT__ERROR, "Failed to load character inventory");
		} else if(!call.client->EnterSystem(call.client->Ship()->position())) {
			//error prints handled in EnterSystem.
			//TODO: throw exception
		} else {
			call.client->InitialEnterGame();
			//send the session changes to kick off the client state changes.
			call.client->SessionSyncChar();
		}
	}
	
	return(NULL);
}

PyCallResult CharacterService::Handle_GetOwnerNoteLabels(PyCallArgs &call) {
	return m_db.GetOwnerNoteLabels(call.client->GetCharacterID());
}

PyCallResult CharacterService::Handle_GetCharCreationInfo(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepDict *cachables = new PyRepDict();
	result = cachables;

	//send all the cache hints needed for char creation.
	call.client->GetServices()->GetCache()->InsertCacheHints(
		ObjCacheService::hCharCreateCachables,
		cachables );
		
	_log(CLIENT__MESSAGE, "Sending char creation info reply");
	
	return(result);
}

PyCallResult CharacterService::Handle_GetAppearanceInfo(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepDict *cachables = new PyRepDict();
	result = cachables;

	//send all the cache hints needed for char creation.
	call.client->GetServices()->GetCache()->InsertCacheHints(
		ObjCacheService::hAppearanceCachables,
		cachables );
	
	_log(CLIENT__MESSAGE, "Sending appearance info reply");
	
	return(result);
}

PyCallResult CharacterService::Handle_ValidateName(PyCallArgs &call) {
	PyRepTuple *args = call.tuple;

	bool res = false;
	//passes a single string in the arg tuple
	if(args->items.size() != 1) {
		_log(CLIENT__ERROR, "Invalid arg count to ValidateName: %d", args->items.size());
		res = false;
	} else if(!args->items[0]->CheckType(PyRep::String)) {
		_log(CLIENT__ERROR, "Invalid argument to ValidateName: string expected");
		res = false;
	} else {
		PyRepString *s = (PyRepString *) args->items[0];
		
		res = m_db.ValidateCharName(s->value.c_str());
	}
	
	PyRep *result = NULL;
	result = new PyRepBoolean(res);

	return(result);
}

PyCallResult CharacterService::Handle_CreateCharacter(PyCallArgs &call) {
	PyRepTuple *call_args = call.tuple;
	
	PyRep *result = NULL;
	if(call_args->items.size() != 14) {
		_log(CLIENT__ERROR, "Invalid arg tuple size in CreateCharacter: %d", call_args->items.size());
		return(new PyRepNone());
	}

	CharacterData cdata;
	CharacterAppearance capp;
	
	if(!call_args->items[0]->CheckType(PyRep::String)) {
		_log(CLIENT__ERROR, "Invalid CreateCharacter name type");
		return(new PyRepNone());
	}
	PyRepString *char_name = (PyRepString *) call_args->items[0];
	cdata.name = char_name->value;

	uint32 args[12];
	uint8 r;
	for(r = 1; r < 13; r++) {
		if(!call_args->items[r]->CheckType(PyRep::Integer)) {
			_log(CLIENT__ERROR, "Invalid CreateCharacter argument %d, expected integer", r);
			return(new PyRepNone());
		}
		PyRepInteger *i = (PyRepInteger *) call_args->items[r];
		args[r-1] = i->value;
	}


	cdata.bloodlineID = args[0];
	cdata.genderID = args[1];
	cdata.ancestryID = args[2];
	cdata.schoolID = args[3];
	cdata.departmentID = args[4];
	cdata.fieldID = args[5];
	cdata.specialityID = args[6];
	cdata.Intelligence = args[7];
	cdata.Charisma = args[8];
	cdata.Perception = args[9];
	cdata.Memory = args[10];
	cdata.Willpower = args[11];

	_log(CLIENT__MESSAGE, "CreateCharacter called for '%s'", char_name->value.c_str());
	_log(CLIENT__MESSAGE, "  bloodlineID=%d genderID=%d ancestryID=%d schoolID=%d", 
			cdata.bloodlineID, cdata.genderID, cdata.ancestryID, cdata.schoolID);
	_log(CLIENT__MESSAGE, "  departmentID=%d fieldID=%d specialityID=%d", 
			cdata.departmentID, cdata.fieldID, cdata.specialityID);
	_log(CLIENT__MESSAGE, "  +INT=%d +CHA=%d +PER=%d +MEM=%d +WIL=%d", 
			cdata.Intelligence, cdata.Charisma, cdata.Perception, cdata.Memory, cdata.Willpower);
	_log(CLIENT__MESSAGE, "  Appearance Data:");


	//must default all the fields to 'NULL'
#define M(n) \
	capp.n = 999999;
			M(accessoryID)
			M(beardID)
			M(costumeID)
			M(decoID)
			M(eyebrowsID)
			M(eyesID)
			M(hairID)
			M(lipstickID)
			M(makeupID)
			M(skinID)
			M(backgroundID)
			M(lightID)
#undef M
#define M(n) \
	capp.n = 999;
			M(morph1e)
			M(morph1n)
			M(morph1s)
			M(morph1w)
			M(morph2e)
			M(morph2n)
			M(morph2s)
			M(morph2w)
			M(morph3e)
			M(morph3n)
			M(morph3s)
			M(morph3w)
			M(morph4e)
			M(morph4n)
			M(morph4s)
			M(morph4w)
#undef M
	


	if(!call_args->items[13]->CheckType(PyRep::Dict)) {
		_log(CLIENT__ERROR, "Invalid CreateCharacter appearance type");
		return(NULL);
	}
	PyRepDict *appearance = (PyRepDict *) call_args->items[13];
	std::map<PyRep *, PyRep *>::iterator cur, end;
	cur = appearance->items.begin();
	end = appearance->items.end();
	for(; cur != end; cur++) {
		if(!cur->first->CheckType(PyRep::String)) {
			_log(CLIENT__ERROR, "Invalid CreateCharacter appearance element key type");
			continue;
		}
		PyRepString *k = (PyRepString *) cur->first;
		if(cur->second->CheckType(PyRep::Integer)) {
			PyRepInteger *v = (PyRepInteger *) cur->second;
			//this is terrible...
#define M(n) \
	if(k->value == #n) { \
		capp.n = v->value; \
	} else 
			M(accessoryID)
			M(beardID)
			M(costumeID)
			M(decoID)
			M(eyebrowsID)
			M(eyesID)
			M(hairID)
			M(lipstickID)
			M(makeupID)
			M(skinID)
			M(backgroundID)
			M(lightID)
#undef M
			{	//end of if chain...
				_log(CLIENT__ERROR, "Unknown integer appearance element %s: %d", k->value.c_str(), v->value);
			}
			_log(CLIENT__MESSAGE, "     %s: %d", k->value.c_str(), v->value);
		} else if(cur->second->CheckType(PyRep::Real)) {
			PyRepReal *v = (PyRepReal *) cur->second;
#define M(n) \
	if(k->value == #n) { \
		capp.n = v->value; \
	} else 
			M(headRotation1)
			M(headRotation2)
			M(headRotation3)
			M(eyeRotation1)
			M(eyeRotation2)
			M(eyeRotation3)
			M(camPos1)
			M(camPos2)
			M(camPos3)
			M(morph1e)
			M(morph1n)
			M(morph1s)
			M(morph1w)
			M(morph2e)
			M(morph2n)
			M(morph2s)
			M(morph2w)
			M(morph3e)
			M(morph3n)
			M(morph3s)
			M(morph3w)
			M(morph4e)
			M(morph4n)
			M(morph4s)
			M(morph4w)
#undef M
			{	//end of if chain...
				_log(CLIENT__ERROR, "Unknown float appearance element %s: %.13f", k->value.c_str(), v->value);
			}
			_log(CLIENT__MESSAGE, "     %s: %f", k->value.c_str(), v->value);
		} else {
			_log(CLIENT__ERROR, "Invalid CreateCharacter appearance element value type for %s: %s", k->value.c_str(), cur->second->TypeString());
			continue;
		}
	}

	
	//TODO: choose these based on selected parameters...
	// Setting character's starting position, and getting it's location...
	double x=0,y=0,z=0;

	if(   !m_db.GetLocationCorporationFromSchool(cdata, x, y, z)
	   || !m_db.GetAttributesFromBloodline(cdata)
	   || !m_db.GetAttributesFromAncestry(cdata) ) {
		codelog(CLIENT__ERROR, "Failed to load char create details. School %lu, bloodline %lu, ancestry %lu.",
			cdata.schoolID, cdata.bloodlineID, cdata.ancestryID);
		return(NULL);
	}
	
	cdata.raceID = m_db.GetRaceByBloodline(cdata.bloodlineID);
	if(cdata.raceID == 0) {
		codelog(CLIENT__ERROR, "Failed to find raceID from bloodline %lu",
			cdata.bloodlineID);
		return(NULL);
	}

	//NOTE: these are currently hard coded to Todaki because other things are
	//also hard coded to only work in Todaki. Once these various things get fixed,
	//just take this code out and the above calls should have cdata populated correctly.
	cdata.stationID = 60004420;
	cdata.solarSystemID = 30001407;
	cdata.constellationID = 20000206;
	cdata.regionID = 10000016;
	cdata.corporationID = 1000044;


	
	
	cdata.bounty = 0;
	cdata.balance = 6666000000.0f;
	cdata.securityRating = 0;
	cdata.logonMinutes = 13;
	cdata.title = "CharTitle";
	
	cdata.startDateTime = Win32TimeNow();
	cdata.createDateTime = cdata.startDateTime;
	cdata.corporationDateTime = cdata.startDateTime;

	//now we have all the data in our happy struct, stick it in the DB
	InventoryItem *char_item = m_db.CreateCharacter(call.client->GetAccountID(), m_manager->item_factory, cdata, capp);
	if(char_item == NULL) {
		//a return to the client of 0 seems to be the only means of marking failure
		codelog(CLIENT__ERROR, "Failed to create character '%s'", cdata.name.c_str());
		return(NULL);
	}
	uint32 charid = char_item->itemID();
	cdata.charid = charid;
	
	//now set up some initial inventory:

	//create them a nice capsule
	{
		std::string capsule_name = cdata.name + "'s Capsule";
		InventoryItem *capsule = m_manager->item_factory->SpawnSingleton(
			itemTypeCapsule,
			charid,
			cdata.stationID,
			flagHangar,
			capsule_name.c_str());
		if(capsule == NULL) {
			codelog(CLIENT__ERROR, "Failed to create capsule for character '%s'", cdata.name.c_str());
			//what to do?
			return(NULL);
		}
		//hack in a relocation
		//capsule->Relocate(-1464808120320.0, 96202629120.0, -2573596385280.0);
		capsule->Relocate(GPoint(x,y,z));
	
		//put the player in their capsule
		char_item->MoveInto(capsule, flagPilot, false);
		capsule->Release();	//no longer needed.
	}
	
	//add some skills for good measure...
	uint32 shipTypeID;
	// TODO: when adding same skill several times, instead of adding n times, raise level
	//       btw. how is level and skill points are handled?
	std::vector<uint32> startingSkills;
	if(   !m_db.GetSkillsFromBloodline(cdata.bloodlineID, startingSkills, shipTypeID)
	   || !m_db.GetSkillsFromAncestry(cdata.ancestryID, startingSkills)
	   || !m_db.GetSkillsFromDepartment(cdata.departmentID, startingSkills)
	   || !m_db.GetSkillsFromField(cdata.fieldID, startingSkills)
	   || !m_db.GetSkillsFromSpeciality(cdata.specialityID, startingSkills) ) {
		codelog(CLIENT__ERROR, "Failed to load char create skills. bloodline %lu, ancestry %lu, department %lu, field %lu, speciality %lu.",
			cdata.bloodlineID, cdata.ancestryID, cdata.departmentID, cdata.fieldID, cdata.specialityID);
		m_db.DeleteCharacter(cdata.charid);
		char_item->Delete();
		return(NULL);
	}

	//all skills loaded, now stick them in the DB
	//TODO: handle reccurring skills.
	std::vector<uint32>::iterator curs, ends;
	curs = startingSkills.begin();
	ends = startingSkills.end();
	for(; curs != ends; curs++) {
		if(*curs == 0)
			continue;
		InventoryItem *i;
		i = char_item->SpawnSingleton(*curs, cdata.charid, flagSkill);
		if(i == NULL) {
			_log(CLIENT__ERROR, "Failed to add skill %lu to char %s (%lu) during char create.", *curs, cdata.name.c_str(), cdata.charid);
		} else {
			float points = SkillBasePoints * float(i->skillTimeConstant());
			i->Set_skillLevel(1);
			i->Set_skillPoints(int(points));
			_log(CLIENT__MESSAGE, "Training skill %lu to level %d (%d points)", i->typeID(), i->skillLevel(), i->skillPoints());
			//we dont actually need the item anymore...
			i->Release();
		}
	}

	
	//now some items:
	{	//item scope
		InventoryItem *junk;
		junk = m_manager->item_factory->Spawn(
			2046,	//Damage Control I
			1,
			charid, cdata.stationID, flagHangar);
		if(junk == NULL) {
			codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", cdata.name.c_str());
		} else {
			junk->Release();
		}
		
		junk = m_manager->item_factory->Spawn(
			34,		//Tritanium
			1,
			charid,
			cdata.stationID,
			flagHangar);
		if(junk == NULL) {
			codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", cdata.name.c_str());
		} else {
			junk->Release();
		}
		junk = NULL;
	}

	{	//item scope
		std::string ship_name = cdata.name + "'s Ship";
		InventoryItem *ship_item;
		ship_item = m_manager->item_factory->SpawnSingleton(
			shipTypeID,		// The race-specific start ship
			charid,
			cdata.stationID,
			flagHangar,
			ship_name.c_str());
		if(ship_item == NULL) {
			codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", cdata.name.c_str());
		} else {
			//hack in a relocation
			//ship_item->Relocate(-1464808120320.0, 96202629120.0, -2573596385280.0);
			ship_item->Relocate(GPoint(x,y,z));
			
			ship_item->Release();
			ship_item = NULL;
		}
	}
	
	//recursively save everything we just did.
	char_item->Save(true);
	char_item->Release();
	
	
	
	_log(CLIENT__MESSAGE, "Sending char create ID %d as reply", charid);
	
	result = new PyRepInteger(charid);

	return(result);
}

PyCallResult CharacterService::Handle_PrepareCharacterForDelete(PyCallArgs &call) {
	
	/*                                                                              
     * Deletion is apparently a timed process, and is related to the
     * deletePrepareDateTime column in the char select result.
     *
     * For now, we will just implement an immediate delete as its better than
     * nothing, since clearing out all the tables by hand is a nightmare.
    */
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return(NULL);
	}

	//TODO: make sure this person actually owns this char...
	
	_log(CLIENT__MESSAGE, "Timed delete of char %lu unimplemented. Deleting Immediately.", args.arg);
	m_db.DeleteCharacter(args.arg);
	
	//we return deletePrepareDateTime, in eve time format.
	PyRep *result = NULL;
	result = new PyRepInteger(Win32TimeNow() + Win32Time_Second*5);

	return(result);
}

PyCallResult CharacterService::Handle_CancelCharacterDeletePrepare(PyCallArgs &call) {
	
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return(NULL);
	}
	
	_log(CLIENT__ERROR, "Cancel delete (of char %lu) unimplemented.", args.arg);

	//returns nothing.
	PyRep *result = NULL;
	result = new PyRepNone();

	return(result);
}

PyCallResult CharacterService::Handle_AddOwnerNote(PyCallArgs &call) {
	
	Call_AddOwnerNote args;

	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(new PyRepInteger(0));
	}

	uint32 noteID = m_db.AddOwnerNote(call.client->GetCharacterID(), args.label, args.content);

	if (noteID == 0) {
		codelog(SERVICE__ERROR, "%s: Failed to set Owner note.", call.client->GetName());
		return (new PyRepNone());
	}

	return(new PyRepInteger(noteID));
}

PyCallResult CharacterService::Handle_EditOwnerNote(PyCallArgs &call) {
	
	Call_EditOwnerNote args;

	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(new PyRepInteger(0));
	}
	
	if (!m_db.EditOwnerNote(call.client->GetCharacterID(), args.noteID, args.label, args.content)) {
		codelog(SERVICE__ERROR, "%s: Failed to set Owner note.", call.client->GetName());
		return (new PyRepNone());
	}

	return(new PyRepInteger(args.noteID));
}

PyCallResult CharacterService::Handle_GetOwnerNote(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(new PyRepInteger(0));
	}
	return m_db.GetOwnerNote(call.client->GetCharacterID(), arg.arg);
}


PyCallResult CharacterService::Handle_GetHomeStation(PyCallArgs &call) {
	//returns the station ID of the station where this player's Clone is
	PyRep *result = NULL;

	_log(CLIENT__ERROR, "GetHomeStation() is not really implemented.");
	
	result = new PyRepInteger(call.client->GetChar().stationID);

	return(result);
}

PyCallResult CharacterService::Handle_GetCloneTypeID(PyCallArgs &call) {
	PyRep *result = NULL;
	
	_log(CLIENT__ERROR, "GetCloneTypeID() is not really implemented.");
	
	result = new PyRepInteger(8);

	return(result);
}




/////////////////////////
PyCallResult CharacterService::Handle_GetCharacterDescription(PyCallArgs &call) {
	PyRep *result = NULL;
	Call_SingleIntegerArg args;
	
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}
		
	result = m_db.GetCharDesc(args.arg);
	if(result == NULL) {
		_log(CLIENT__ERROR, "Failed to get character description %d", args.arg);
		return(NULL);
	}
		
	return(result);
}



//////////////////////////////////
PyCallResult CharacterService::Handle_SetCharacterDescription(PyCallArgs &call) {
	Call_SingleStringArg args;
	
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}
	
	m_db.SetCharDesc(call.client->GetCharacterID(), args.arg.c_str());

	return(NULL);
}

PyCallResult CharacterService::Handle_GetCharacterAppearanceList(PyCallArgs &call) {
	Call_SingleIntList args;
	
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: Invalid arguments", call.client->GetName());
		return(NULL);
	}

	PyRepList *l = new PyRepList();
	
	std::vector<uint32>::iterator cur, end;
	cur = args.ints.begin();
	end = args.ints.end();
	for(; cur != end; cur++) {
		PyRep *r = m_db.GetCharacterAppearance(*cur);
		if(r == NULL)
			r = new PyRepNone();
		l->add(r);
	}

	return(l);
}

/** Retrieves a Character note.
 *
 * Checks the database for a given character note and returns it.
 *
 * @return PyRepSubStream pointer with the string with the note or with PyRepNone if no note is stored.
 *
 *  **LSMoura
 */
PyCallResult CharacterService::Handle_GetNote(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	PyRep *result;

	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}

	result = m_db.GetNote(call.client->GetCharacterID(), args.arg);

	if (!result)
		result = new PyRepNone();

	return(result);
}

/** Stores a Character note.
 *
 *  Stores the given character note in the database.
 *
 * @return PyRepSubStream pointer with a PyRepNone() value.
 *
 *  **LSMoura
 */
PyCallResult CharacterService::Handle_SetNote(PyCallArgs &call) {
	Call_SetNote args;
	
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}

	if (!m_db.SetNote(call.client->GetCharacterID(), args.itemID, args.note.c_str()))
		codelog(CLIENT__ERROR, "Error while storing the note on the database.");
	
	return(NULL);
}
