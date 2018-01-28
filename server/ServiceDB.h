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


#ifndef __SERVICEDB_H_INCL__
#define __SERVICEDB_H_INCL__

#include "../common/common.h"
#include "../common/packet_types.h"

#include <string>
#include <vector>
#include <map>

/*                                                                              
 * This object is the home for common DB operations which may be needed by many
 * different service objects. It should be inherited by each service DB
 * implementation.
 *
 *
*/

class DBcore;
class DBResultRow;
class GPoint;
class ItemFactory;

class PyRepObject;
class PyRepDict;
class EntityList;
class Client;
class CorpMemberInfo;
//class InventoryItem;

class ServiceDB {
public:
	ServiceDB(DBcore *db);
	ServiceDB(ServiceDB *db);	//alternate constructor which simply steals the dbcore from a different serviceDB object to prevent having to expose it.
	virtual ~ServiceDB();

	bool DoLogin(const char *login, const char *pass, uint32 &out_accountID, uint32 &out_role);

	//entity/item stuff:
	bool ListEntitiesByCategory(uint32 ownerID, uint32 categoryID, std::vector<uint32> &into);
	uint32 GetCurrentShipID(uint32 characterID);
	PyRepObject *GetInventory(uint32 entityID, EVEItemFlags flag);
	bool GetStaticLocation(uint32 entityID, uint32 &regionID, uint32 &constellationID, uint32 &solarSystemID, GPoint &location);

	//this really wants to move back to LSCDB:
	uint32 StoreNewEVEMail(uint32 senderID, uint32 recipID, const char * subject, const char * message, uint64 sentTime);
	
	void SetCharacterLocation(uint32 characterID, uint32 stationID, uint32 systemID, uint32 constellationID, uint32 regionID);
	bool SetCharacterBalance(uint32 char_id, double newbalance);
	bool AddCharacterBalance(uint32 char_id, double delta);
	bool LoadCorporationMemberInfo(uint32 charID, CorpMemberInfo &info);
	
	PyRepObject *GetSolRow(uint32 systemID) const;
	PyRepObject *GetSolDroneState(uint32 systemID) const;

	void MoveCharacter(uint32 charID);
	
	bool GetSystemParents(uint32 systemID, uint32 &constellationID, uint32 &regionID);
	bool GetStationParents(uint32 stationID, uint32 &systemID, uint32 &constellationID, uint32 &regionID);
	bool GetStaticPosition(uint32 itemID, double &x, double &y, double &z);
	bool GetStaticPosition(uint32 itemID, uint32 &systemID, double &x, double &y, double &z);
	bool GetStationDockPosition(uint32 stationID, GPoint &pos);
	uint32 GetDestinationStargateID(uint32 fromSystem, uint32 toSystem);
	
	bool GetConstant(const char *name, uint32 &into);
	
	//these really want to move back into AccountDB
	bool GiveCash( uint32 characterID, JournalRefType refTypeID, uint32 ownerID1, uint32 ownerID2, const char *argID1, uint32 accountID, EVEAccountKeys accountKey, double amount, double balance, const char *reason);
	bool AddBalanceToCorp(uint32 corpID, double amount);
	double GetCorpBalance(uint32 corpID);

protected:
	
	DBcore *const m_db;	//we do not own this pointer
	
};





#endif


