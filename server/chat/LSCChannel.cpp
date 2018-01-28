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


#include "LSCChannel.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../packets/LSCPkts.h"
#include "LSCService.h"
#include "../Client.h"

LSCChannel::LSCChannel(
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
)
: m_service(svc),
  m_chanNum(chanNum),
  m_ownerID(ownerID),
  m_displayName(displayName==NULL?"":displayName),
  m_motd(motd==NULL?"":motd),
  m_comparisonKey(comparisonKey==NULL?"":comparisonKey),
  m_memberless(memberless),
  m_password(password==NULL?"":password),
  m_mailingList(mailingList),
  m_cspa(cspa),
  m_temporary(temporary),
  m_mode(mode)
{
	_log(LSC__CHANNELS, "Creating channel %s", GetName().c_str());
}

LSCChannel::~LSCChannel() {
	_log(LSC__CHANNELS, "Destroying channel %s", GetName().c_str());
}

void LSCChannel::Handle_SendMessage(Client *from, const char *text) {
	
	_log(LSC__MESSAGE, "Channel %s: %s", GetName().c_str(), text);

	bool self_only = false;
	if(text[0] == '#') {
		self_only = m_service->ExecuteCommand(from, text);
	}
	
	//start building the notify
	OnLSC_SendMessage sm;
	sm.sender = m_service->_MakeSenderInfo(from);
	sm.message = text;
	sm.member_count = m_members.size();
	sm.channelID = m_chanNum.Encode();

	//build a little notification out of it.
	EVENotificationStream notify;
	notify.remoteObject = 1;
	notify.args = sm.Encode();
	
	//now sent it to the client
	PyAddress dest;
	dest.type = PyAddress::Broadcast;
	dest.service = "OnLSC";
	dest.bcast_idtype = "*multicastID";
	//TODO: need to add the narrowcast crap eventually.

	if(self_only) {
		from->SendNotification(dest, &notify);
	} else {
		//send to all members of this channel
		m_service->GetEntityList()->Multicast(m_members, dest, &notify);
	}
}

bool LSCChannel::Handle_JoinChannel(Client *from) {
	_log(LSC__CHANNELS, "Channel %s: Join from %s", GetName().c_str(), from->GetName());
	
	m_members.insert(from->GetCharacterID());
	from->ChannelJoined(this);

	if(m_chanNum.type != LSCChannelDesc::normal) {
		//non-normal channels? send off an OnLSC join notify
		OnLSC_JoinChannel join;
		join.sender = m_service->_MakeSenderInfo(from);
		join.member_count = m_members.size();
		join.channelID = m_chanNum.Encode();
		
		PyAddress dest;
		dest.type = PyAddress::Broadcast;
		dest.service = "OnLSC";
		dest.bcast_idtype = m_chanNum.GetTypeString();
	
		EVENotificationStream notify;
		notify.remoteObject = 1;
		notify.args = join.Encode();
		
		m_service->GetEntityList()->Multicast(m_members, dest, &notify);
	}
	
	return(true);
}

void LSCChannel::Handle_LeaveChannel(Client *from) {
	_log(LSC__CHANNELS, "Channel %s: Leave from %s", GetName().c_str(), from->GetName());
	m_members.erase(from->GetCharacterID());
}

void LSCChannel::SendUnicastMessage(Client *to, const char *msg) {
	//start building the notify
	OnLSC_SendMessage sm;
	
	OnLSC_SenderInfo *sender = new OnLSC_SenderInfo;

	//this is a mega-hack...
	//the client seems to ignore messages from itself...
	sender->senderID = 0;
	sender->senderName = "Command";
	sender->senderType = 1378;
	sender->corpID = 1000006;
	//hackin the role to make sure it shows up.
	sender->role = ROLE_CHTADMINISTRATOR|ROLE_PLAYER|ROLE_LOGIN;
	sender->corp_role = 0;
	
	sm.sender = sender;	//m_service->_MakeSenderInfo(to);
	sm.message = msg;
	sm.member_count = m_members.size();
	sm.channelID = m_chanNum.Encode();

	//build a little notification out of it.
	EVENotificationStream notify;
	notify.remoteObject = 1;
	notify.args = sm.Encode();
	
	//now sent it to the client
	PyAddress dest;
	dest.type = PyAddress::Broadcast;
	dest.service = "OnLSC";
	dest.bcast_idtype = "*multicastID";
	//TODO: need to add the narrowcast crap eventually.
	
	to->SendNotification(dest, &notify);
}

PyRepList *LSCChannel::EncodeListEntry(uint32 CharID) const {
	PyRepList *res = new PyRepList();
	res->add(new PyRepInteger(m_chanNum.number));
	res->add(new PyRepInteger(m_ownerID));
	if(m_displayName.empty())
		res->add(new PyRepNone());
	else
		res->add(new PyRepString(m_displayName));
	if(m_motd.empty())
		res->add(new PyRepNone());
	else
		res->add(new PyRepString(m_motd));
	if(m_comparisonKey.empty())
		res->add(new PyRepNone());
	else
		res->add(new PyRepString(m_comparisonKey));
	res->add(new PyRepInteger(m_memberless));
	if(m_password.empty())
		res->add(new PyRepNone());
	else
		res->add(new PyRepString(m_password));
	res->add(new PyRepInteger(m_mailingList));
	res->add(new PyRepInteger(m_cspa));
	res->add(new PyRepInteger(m_temporary));
	res->add(new PyRepInteger(m_mode));
	if(m_members.find(CharID) != m_members.end())
		res->add(new PyRepInteger(true));
	else
		res->add(new PyRepInteger(true));
	//TODO: implement membercount:
	res->add(new PyRepInteger(1));
	return(res);
}


















