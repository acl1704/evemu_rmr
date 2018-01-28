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



#include "ServiceDB.h"
#include "../common/dbcore.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/EVEDBUtils.h"
#include "../common/gpoint.h"
#include "inventory/InventoryItem.h"
#include "inventory/ItemFactory.h"
#include "Client.h"	//for CorpMemberInfo

ServiceDB::ServiceDB(DBcore *db)
: m_db(db)
{
}

//this is a specialized constructor to help with information hiding:
// if somebody needs to make a new specialized ServiceDB subclass,
// and already has some other ServiceDB subclass which points to the
// correct DB, then it can simply use that ServiceDB as a template
// instead of having to go find the raw dbcore object.
ServiceDB::ServiceDB(ServiceDB *existing_db)
: m_db(existing_db->m_db)
{
}

ServiceDB::~ServiceDB() {
}

bool ServiceDB::DoLogin(const char *login, const char *pass, uint32 &out_accountID, uint32 &out_role) {
	DBQueryResult res;

	if(pass[0] == '\0') {
		_log(SERVICE__ERROR, "Empty password not allowed (%s)", login);
		return(false);
	}

	if(!m_db->IsSafeString(login) || !m_db->IsSafeString(pass)) {
		_log(SERVICE__ERROR, "Invalid characters in login or password!");
		return(false);
	}
	
	if(!m_db->RunQuery(res,
		"SELECT accountID,role,password,PASSWORD('%s'),MD5('%s')"
		" FROM account"
		" WHERE accountName='%s'", pass, pass, login))
	{
		_log(SERVICE__ERROR, "Error in login query: %s", res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Unknown account '%s'", login);
		return(false);
	}

	if(   strcmp(row.GetText(2), row.GetText(3)) != 0
	   && strcmp(row.GetText(2), row.GetText(4)) != 0
	   && strcmp(row.GetText(2), pass) != 0) {
		_log(SERVICE__ERROR, "Login failed for account '%s'", login);
		return(false);
	}
	
	out_accountID = row.GetUInt(0);
	out_role = row.GetUInt(1);
	
	return(true);
}

void ServiceDB::SetCharacterLocation(uint32 characterID, uint32 stationID, 
	uint32 systemID, uint32 constellationID, uint32 regionID) {
	DBerror err;
	if(!m_db->RunQuery(err,
	"UPDATE character_"
	" SET"
	"	stationID=%lu,"
	"	solarSystemID=%lu,"
	"	constellationID=%lu,"
	"	regionID=%lu"
	" WHERE characterID=%lu",
		stationID, systemID, constellationID, regionID,
		characterID)
	) {
		codelog(SERVICE__ERROR, "Failed to set character location %li: %s", characterID, err.c_str());
	}
}

bool ServiceDB::ListEntitiesByCategory(uint32 ownerID, uint32 categoryID, std::vector<uint32> &into) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
	"SELECT "
	"	entity.itemID"
	" FROM entity"
	"	LEFT JOIN invTypes ON entity.typeID=invTypes.typeID"
	"	LEFT JOIN invGroups ON invTypes.groupID=invGroups.groupID"
	" WHERE invGroups.categoryID=%lu AND entity.ownerID=%lu", categoryID, ownerID))
	{
		_log(SERVICE__ERROR, "Error in ListEntitiesByCategory query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	while(res.GetRow(row)) {
		into.push_back(row.GetInt(0));
	}
	return(true);
}

uint32 ServiceDB::GetCurrentShipID(uint32 characterID) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		//not sure if this is gunna be valid all the time...
	"SELECT"
	"	locationID"
	" FROM entity"
	" WHERE itemID=%lu", characterID
	/*"SELECT "
	"	itemID"
	" FROM entity AS chare LEFT JOIN entity AS shipe"
	"	LEFT JOIN invTypes ON shipe.typeID=invTypes.typeID"
	"	LEFT JOIN invGroups ON invTypes.groupID=invGroups.groupID"
	" WHERE invGroups.categoryID=6 AND 
	" WHERE typeID=%lu", typeID*/
	))
	{
		_log(SERVICE__ERROR, "Error in GetCurrentShipID query: %s", res.error.c_str());
		return(0);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in GetCurrentShipID query: no ship for char id %d", characterID);
		return(0);
	}

	return(row.GetUInt(0));
}

/*
PyRepObject *ServiceDB::GetInventory(uint32 containerID, EVEItemFlags flag) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" entity.itemID,"
		" entity.typeID,"
		" entity.ownerID,"
		" entity.locationID,"
		" entity.flag,"
		" entity.contraband,"
		" entity.singleton,"
		" entity.quantity,"
		" invTypes.groupID,"
		" invGroups.categoryID,"
		" entity.customInfo"
		" FROM entity "
		"	LEFT JOIN invTypes ON entity.typeID=invTypes.typeID"
		"	LEFT JOIN invGroups ON invTypes.groupID=invGroups.groupID"
		" WHERE entity.locationID=%lu "
		"	AND ( %lu=0 OR entity.flag=%lu)",	//crazy =0 logic is to make 0 match any flag without copying the entire query
			containerID, flag, flag))
	{
		codelog(SERVICE__ERROR, "Error in query for %d,%d: %s", containerID, flag, res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}
*/


PyRepObject *ServiceDB::GetSolRow(uint32 systemID) const {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		//not sure if this is gunna be valid all the time...
	"SELECT "
	"	itemID,entity.typeID,ownerID,locationID,flag,contraband,singleton,quantity,"
	"	invGroups.groupID, invGroups.categoryID,"
	"	customInfo"
    " FROM entity "
	"	LEFT JOIN invTypes ON entity.typeID=invTypes.typeID"
	"	LEFT JOIN invGroups ON invTypes.groupID=invGroups.groupID"
	" WHERE entity.itemID=%lu",
		systemID
	))
	{
		_log(SERVICE__ERROR, "Error in GetSolRow query: %s", res.error.c_str());
		return(0);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in GetSolRow query: unable to find sol info for system %d", systemID);
		return(0);
	}

	return(DBRowToRow(row, "util.Row"));
}

//this function is temporary, I dont plan to keep this crap in the DB.
PyRepObject *ServiceDB::GetSolDroneState(uint32 systemID) const {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		//not sure if this is gunna be valid all the time...
	"SELECT "
	"	droneID, solarSystemID, ownerID, controllerID,"
	"	activityState, typeID, controllerOwnerID"
    " FROM droneState "
	" WHERE solarSystemID=%lu",
		systemID
	))
	{
		_log(SERVICE__ERROR, "Error in GetSolDroneState query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}


bool ServiceDB::GetSystemParents(uint32 systemID, uint32 &constellationID, uint32 &regionID) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
	"SELECT"
	"	constellationID,regionID"
	" FROM mapSolarSystems"
	" WHERE solarSystemID=%lu", systemID
	))
	{
		_log(SERVICE__ERROR, "Error in GetSystemParents query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in GetSystemParents query: no system data for %d", systemID);
		return(false);
	}
	
	constellationID = row.GetUInt(0);
	regionID = row.GetUInt(1);
	
	return(true);
}

bool ServiceDB::GetStationParents(uint32 stationID, uint32 &systemID, uint32 &constellationID, uint32 &regionID) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
	"SELECT"
	"	solarSystemID,constellationID,regionID"
	" FROM staStations"
	" WHERE stationID=%lu", stationID
	))
	{
		_log(SERVICE__ERROR, "Error in GetStationParents query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in GetStationParents query: no station data for %d", stationID);
		return(false);
	}

	systemID = row.GetUInt(0);
	constellationID = row.GetUInt(1);
	regionID = row.GetUInt(2);
	
	return(true);
}

bool ServiceDB::GetStaticPosition(uint32 itemID, double &x, double &y, double &z) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
	"SELECT"
	"	x, y, z"
	" FROM mapDenormalize"
	" WHERE itemID=%lu", itemID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Error in query: no data for %d", itemID);
		return(false);
	}

	x = row.GetDouble(0);
	y = row.GetDouble(1);
	z = row.GetDouble(2);
	
	return(true);
}

bool ServiceDB::GetStaticPosition(uint32 itemID, uint32 &systemID, double &x, double &y, double &z) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
	"SELECT"
	"	solarSystemID, x, y, z"
	" FROM mapDenormalize"
	" WHERE itemID=%lu", itemID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Error in query: no data for %d", itemID);
		return(false);
	}

	systemID = row.GetUInt(0);
	x = row.GetDouble(1);
	y = row.GetDouble(2);
	z = row.GetDouble(3);
	
	return(true);
}

bool ServiceDB::GetStaticLocation(uint32 entityID, uint32 &regionID, uint32 &constellationID, uint32 &solarSystemID, GPoint &location) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
	"SELECT"
	"	regionID, constellationID, solarSystemID, "
	"	x, y, z"
	" FROM mapDenormalize"
	" WHERE itemID=%lu", entityID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Error in query: no data for %d", entityID);
		return(false);
	}

	regionID = row.GetUInt(0);
	constellationID = row.GetUInt(1);
	solarSystemID = row.GetUInt(2);
	location.x = row.GetDouble(3);
	location.y = row.GetDouble(4);
	location.z = row.GetDouble(5);
	
	return(true);
}

bool ServiceDB::GetStationDockPosition(uint32 stationID, GPoint &location) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		" SELECT "
		"   staStations.x + staStationTypes.dockEntryX, "
		"   staStations.y + staStationTypes.dockEntryY, "
		"   staStations.z + staStationTypes.dockEntryZ "
		" FROM staStations "
		" LEFT JOIN staStationTypes "
		" ON staStations.stationTypeID = staStationTypes.stationTypeID "
		" WHERE staStations.stationID = %lu ", stationID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Error in query: no data for %d", stationID);
		return(false);
	}

	location.x = row.GetDouble(0);
	location.y = row.GetDouble(1);
	location.z = row.GetDouble(2);
	
	return(true);
}

uint32 ServiceDB::GetDestinationStargateID(uint32 fromSystem, uint32 toSystem) {
DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		" SELECT "
		"    mapDenormalize.solarSystemID AS FromSystem, mapJumps.stargateID AS FromGate, "
		"    mapJumps.celestialID AS ToGate, mapDenormalize2.solarSystemID AS ToSystem "
		" FROM mapJumps "
		" LEFT JOIN mapDenormalize "
		" ON mapDenormalize.itemID = mapJumps.stargateID "
		" LEFT JOIN mapDenormalize AS mapDenormalize2 "
		" ON mapDenormalize2.itemID = mapJumps.celestialID "
		" WHERE mapDenormalize.solarSystemID = %lu "
		" AND mapDenormalize2.solarSystemID = %lu ",
		fromSystem, toSystem
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(0);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Error in query: no data for %d, %d", fromSystem, toSystem);
		return(0);
	}

	return row.GetUInt(2);
}

bool ServiceDB::SetCharacterBalance(uint32 char_id, double newbalance)
{
	DBerror err;

	if(!m_db->RunQuery(err,
		"UPDATE character_ SET balance=%.2f WHERE characterID=%lu",newbalance,char_id))
	{
		_log(SERVICE__ERROR, "Error in query : %s", err.c_str());
		return(false);
	}

	return (true);
}

bool ServiceDB::AddCharacterBalance(uint32 char_id, double delta)
{
	DBerror err;

	if(!m_db->RunQuery(err,
		"UPDATE character_ SET balance=balance+%.2f WHERE characterID=%lu",delta,char_id))
	{
		_log(SERVICE__ERROR, "Error in query : %s", err.c_str());
		return(false);
	}

	return (true);
}

bool ServiceDB::GetConstant(const char *name, uint32 &into) {
	DBQueryResult res;

	std::string escaped;
	m_db->DoEscapeString(escaped, name);
	
	if(!m_db->RunQuery(res,
	"SELECT"
	"	constantValue"
	" FROM eveConstants"
	" WHERE constantID='%s'",
		escaped.c_str()
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Unable to find constant %s", name);
		return(false);
	}
	
	into = row.GetUInt64(0);
	
	return(true);
}

bool ServiceDB::LoadCorporationMemberInfo(uint32 charID, CorpMemberInfo &info) {
	DBQueryResult res;
	DBResultRow row;

	if (!m_db->RunQuery(res, 
		" SELECT"
		"	corprole,rolesAtAll,rolesAtBase,rolesAtHQ,rolesAtOther "
		" FROM chrCorporationRoles"
		" WHERE characterID = %lu ", 
		charID))
	{
			codelog(CLIENT__ERROR, "Error on query: %s", res.error.c_str());
			return false;
	}

	if (!res.GetRow(row))
	{
		// No data for this character, fill in with dummy values
		info.corprole = 0;
		info.rolesAtAll = 0;
		info.rolesAtBase = 0;
		info.rolesAtHQ = 0;
		info.rolesAtOther = 0;
	} else {
		// Data found
		info.corprole = row.GetUInt64(0);
		info.rolesAtAll = row.GetUInt64(1);
		info.rolesAtBase = row.GetUInt64(2);
		info.rolesAtHQ = row.GetUInt64(3);
		info.rolesAtOther = row.GetUInt64(4);
	}

	//this is really hacky and belongs somewhere else eventually:
	if(!m_db->RunQuery(res,
	"SELECT"
	"	corporation.stationID"
	" FROM corporation"
	"	LEFT JOIN character_ ON corporation.corporationID=character_.corporationID"
	" WHERE character_.characterID='%lu'",
		charID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Unable to find corporatioin for %lu", charID);
		return(false);
	}
	
	info.corpHQ = row.GetUInt(0);
	
	return(true);
}



















