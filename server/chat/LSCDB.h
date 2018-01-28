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


#ifndef __LSCDB_H_INCL__
#define __LSCDB_H_INCL__

#include "../ServiceDB.h"
#include "../common/PyRep.h"
#include <vector>

class LSCService;
class LSCChannel;

class LSCDB
: public ServiceDB {
public:
	LSCDB(DBcore *db);
	virtual ~LSCDB();
	
	void LoadChannels(LSCService *svc, std::vector<LSCChannel *> &channels);
	//PyRepObject *ListChannels(uint32 charId, uint32 corpID);
	//PyRepObject *ListChannels(const std::vector<uint32> &channels);
	PyRepObject *QueryChannelInfo(uint32 channel_id);
	PyRepObject *QueryChannelMods(uint32 channel_id);
	PyRepObject *QueryChannelChars(uint32 channel_id);
	PyRepObject *LookupChars(const char *match, bool exact);
	PyRepObject *GetEVEMailHeaders(uint32 recID);
	PyRepObject *LookupCorporations(const std::string &);
	PyRepObject *LookupFactions(const std::string &);
	PyRepObject *LookupCorporationTickers(const std::string &);
	PyRepObject *LookupStations(const std::string &);
	PyRepObject *LookupLocationsByGroup(const std::string &, uint32);

	PyRep *GetEVEMailDetails(uint32 messageID, uint32 readerID);
	bool MarkMessageRead(uint32 messageID);
	bool DeleteMessage(uint32 messageID, uint32 readerID);

protected:
};





#endif


