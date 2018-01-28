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

#ifndef __LSCCHANNEL_H_INCL__
#define __LSCCHANNEL_H_INCL__

#include "types.h"
#include "../EntityList.h"
#include "LSCService.h"

class PyRep;

class LSCChannel {
public:
	LSCChannel(
		LSCService *svc, 
		const LSCChannelDesc &chanNum,
		uint32 ownerID,
		const char *displayName,
		const char *motd,
		const char *comparisonKey,
		bool memberless,
		const char *password,
		bool mailingList,
		bool cspa,
		bool temporary,
		uint32 mode
		);
	virtual ~LSCChannel();

	const LSCChannelDesc &Describe() const { return(m_chanNum); }
	std::string GetName() const { return(m_chanNum.ToString()); }
	
	void Handle_SendMessage(Client *from, const char *msg);
	bool Handle_JoinChannel(Client *from);
	void Handle_LeaveChannel(Client *from);
	
	PyRepList *EncodeListEntry(uint32 charID) const;
	
	void SendUnicastMessage(Client *to, const char *msg);
protected:
	LSCService *const m_service;	//we do not own this

	const LSCChannelDesc m_chanNum;	//we own this

	EntityList::character_set m_members;
	
	uint32 m_ownerID;
	std::string m_displayName;
	std::string m_motd;
	std::string m_comparisonKey;
	bool m_memberless;
	std::string m_password;
	bool m_mailingList;
	bool m_cspa;
	bool m_temporary;
	uint32 m_mode;
};





#endif


