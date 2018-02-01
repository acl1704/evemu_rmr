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



#include "LSCService.h"
#include "LSCChannel.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../admin/CommandDispatcher.h"
#include "../common/EVEUtils.h"
#include "../common/MiscFunctions.h"

#include "../packets/LSCPkts.h"
#include "../packets/General.h"

PyCallable_Make_InnerDispatcher(LSCService)


LSCService::LSCService(PyServiceMgr *mgr, DBcore *db, CommandDispatcher* cd)
: PyService(mgr, "LSC"),
  m_dispatch(new Dispatcher(this)),
  m_commandDispatch(cd),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	//make sure you edit the header file too
	PyCallable_REG_CALL(LSCService, GetMyMessages)
	PyCallable_REG_CALL(LSCService, GetMessageDetails)
	PyCallable_REG_CALL(LSCService, GetChannels)
	PyCallable_REG_CALL(LSCService, JoinChannels)
	PyCallable_REG_CALL(LSCService, LeaveChannels)
	PyCallable_REG_CALL(LSCService, LeaveChannel)
	PyCallable_REG_CALL(LSCService, SendMessage)
	PyCallable_REG_CALL(LSCService, CreateChannel)
	PyCallable_REG_CALL(LSCService, DestroyChannel)
	PyCallable_REG_CALL(LSCService, Page)
	PyCallable_REG_CALL(LSCService, MarkMessagesRead);
	PyCallable_REG_CALL(LSCService, DeleteMessages);

	//load up all permanent channels from the DB.
	std::vector<LSCChannel *> channels;
	m_db.LoadChannels(this, channels);
	std::vector<LSCChannel *>::iterator cur, end;
	cur = channels.begin();
	end = channels.end();
	for(; cur != end; cur++) {
		m_channels[(*cur)->Describe()] = *cur;
	}
}

LSCService::~LSCService() {
	delete m_dispatch;
	std::map<LSCChannelDesc, LSCChannel *>::iterator cur, end;
	cur = m_channels.begin();
	end = m_channels.end();
	for(; cur != end; cur++) {
		delete cur->second;
	}
}


PyCallResult LSCService::Handle_GetMyMessages(PyCallArgs &call) {
	PyRep *result = m_db.GetEVEMailHeaders(call.client->GetCharacterID());
	if (!result) {
		result = new PyRepNone();
	}
	return(result);
}

PyCallResult LSCService::Handle_GetMessageDetails(PyCallArgs &call) {
	Call_TwoIntegerArgs args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}

	//TODO: verify ability to read this message...

	PyRep *result = m_db.GetEVEMailDetails(args.arg2, args.arg1);
	if(result == NULL) {
		codelog(SERVICE__ERROR, "%s: Failed to get details for message %lu", call.client->GetName(), args.arg2);
		return(NULL);
	}

	return (result);
}


PyCallResult LSCService::Handle_GetChannels(PyCallArgs &call) {
	util_Rowset res;
	
	res.header.push_back("channelID");
	res.header.push_back("ownerID");
	res.header.push_back("displayName");
	res.header.push_back("motd");
	res.header.push_back("comparisonKey");
	res.header.push_back("memberless");
	res.header.push_back("password");
	res.header.push_back("mailingList");
	res.header.push_back("cspa");
	res.header.push_back("temporary");
	res.header.push_back("mode");
	res.header.push_back("subscribed");
	res.header.push_back("estimatedMemberCount");

	std::map<LSCChannelDesc, LSCChannel *>::const_iterator cur, end;
	cur = m_channels.begin();
	end = m_channels.end();
	for(; cur != end; ++cur) {
		res.lines.add( 
			cur->second->EncodeListEntry(call.client->GetCharacterID()) 
			);
	}
	
	return(res.FastEncode());
}


PyCallResult LSCService::Handle_JoinChannels(PyCallArgs &call) {
	PyRepTuple *iargs = call.tuple;
	PyRep *result = NULL;
	//([1, 1000044, 444666],)

	//parse the PyRep to get the list of channels.
	std::vector<LSCChannelDesc> channels;
	if(iargs->items.size() != 1) {
		_log(CLIENT__ERROR, "Invalid arg count to JoinChannels: %d", iargs->items.size());
		//TODO: throw exception
	} else if(!iargs->items[0]->CheckType(PyRep::List)) {
		_log(SERVICE__ERROR, "Invalid argument to JoinChannels: got %s, list expected", iargs->items[0]->TypeString());
	} else {
		PyRepList *chanlist = (PyRepList *) iargs->items[0];

		PyRepList::iterator cur, end;
		cur = chanlist->begin();
		end = chanlist->end();
		LSCChannelDesc j;
		for(; cur != end; cur++) {
			if(!j.Decode(*cur)) {
				_log(SERVICE__ERROR, "Invalid channel argument to JoinChannels: failed to decode.");
				continue;
			}
			channels.push_back(j);
		}
	}
	
	PyRepList *ml = new PyRepList();
	result = ml;

	std::vector<LSCChannelDesc>::iterator cur, end;
	cur = channels.begin();
	end = channels.end();
	for(; cur != end; cur++) {
		const LSCChannelDesc &curj = *cur;
		
		//try to join the channel.
		std::map<LSCChannelDesc, LSCChannel *>::const_iterator res;
		res = m_channels.find(curj);
		if(res == m_channels.end()) {
			_log(SERVICE__ERROR, "%s: Failed to find channel %s for join", call.client->GetName(), curj.ToString().c_str());
			continue;
		}
		if(!res->second->Handle_JoinChannel(call.client)) {
			_log(SERVICE__ERROR, "Channel %s rejected request from %s to join", curj.ToString().c_str(), call.client->GetName());
			continue;
		}
		
		
		PyRepTuple *topt = new PyRepTuple(3);

		topt->items[0] = curj.Encode();	//channelID
		topt->items[1] = new PyRepInteger(1);	//bool OK
		//load the channel spec, access lists, and member list
		topt->items[2] = _QueryChannelDetails(curj);
		if(topt->items[2] == NULL) {
			delete topt;
			continue;	//error printed in sub
		}

		
		ml->add(topt);
	}
	
	return(result);
}


PyCallResult LSCService::Handle_LeaveChannels(PyCallArgs &call) {
	PyRep *result = NULL;
	
	//args are: (channel, message)

	CallLeaveChannels args;
	if(!args.Decode(&call.tuple)) {
		_log(LSC__ERROR, "Unable to parse arguments to LeaveChannels");
		
	} else {
		LSCChannelDesc channel;

        PyRepList::iterator cur, end;
		cur = args.channels.begin();
		end = args.channels.end();
		for(; cur != end; cur++) {
			if(!channel.Decode(*cur)) {
				_log(LSC__ERROR, "Unable to parse LeaveChannels channel arg");
			} else {
				std::map<LSCChannelDesc, LSCChannel *>::const_iterator res;
				res = m_channels.find(channel);
				if(res == m_channels.end()) {
					_log(SERVICE__ERROR, "Unable to find channel object for channel %s", channel.ToString().c_str());
				} else {
					res->second->Handle_LeaveChannel(call.client);
				}
			}
			
		}

		_log(LSC__ERROR, "Unhandled LeaveChannels");
	}
	
	result = new PyRepNone();
	
	return(result);
}

PyCallResult LSCService::Handle_LeaveChannel(PyCallArgs &call) {
	PyRep *result = NULL;
	
	//args are: (channel, message)

	CallLeaveChannel args;
	if(!args.Decode(&call.tuple)) {
		_log(LSC__ERROR, "Unable to parse arguments to LeaveChannel");
		
	} else {
		LSCChannelDesc channel;
		if(!channel.Decode(args.channel)) {
			_log(LSC__ERROR, "Unable to parse LeaveChannel channel arg");
		} else {
			std::map<LSCChannelDesc, LSCChannel *>::const_iterator res;
			res = m_channels.find(channel);
			if(res == m_channels.end()) {
				_log(SERVICE__ERROR, "Unable to find channel object for channel %s", channel.ToString().c_str());
			} else {
				res->second->Handle_LeaveChannel(call.client);
			}
		}
	}
	
	result = new PyRepNone();
	
	return(result);
}


PyCallResult LSCService::Handle_CreateChannel(PyCallArgs &call) {
	PyRepTuple *iargs = call.tuple;
	PyRep *result = NULL;
	
	//args are: (channel, message)
	//takes various optional dict args:
	// memberless, mailingList, joinExisting

	LSCChannelDesc channel;
	
	if(iargs->items.size() != 1) {
		_log(CLIENT__ERROR, "Invalid arg count to CreateChannel: %d", iargs->items.size());
		//TODO: throw exception
	} else if(!iargs->items[0]->CheckType(PyRep::String)) {
		_log(SERVICE__ERROR, "Invalid name argument to CreateChannel: got %s, string expected", iargs->items[1]->TypeString());
	} else {
		//PyRepString *msg = (PyRepString *) iargs->items[0];

		//TODO: create the channel, build a channelID for it, and then return
		//result = _QueryChannelDetails(channelID);
		_log(LSC__ERROR, "Unhandled LSC CreateChannel");
	}

	

	
	result = new PyRepNone();
	
	return(result);
}


PyCallResult LSCService::Handle_DestroyChannel(PyCallArgs &call) {
	PyRepTuple *iargs = call.tuple;
	PyRep *result = NULL;
	
	//args are: (channel, message)

	LSCChannelDesc channel;
	
	if(iargs->items.size() != 1) {
		_log(CLIENT__ERROR, "Invalid arg count to DestroyChannel: %d", iargs->items.size());
		//TODO: throw exception
	} else if(!channel.Decode(iargs->items[0])) {
		_log(SERVICE__ERROR, "Unable to decode channel argument to DestroyChannel");
	} else {
		//PyRepString *msg = (PyRepString *) iargs->items[0];

		//TODO: create the channel, build a channelID for it, and then return
		//result = _QueryChannelDetails(channelID);
		_log(LSC__ERROR, "Unhandled LSC DestroyChannel");
	}

	

	
	result = new PyRepNone();
	
	return(result);
}


PyCallResult LSCService::Handle_SendMessage(PyCallArgs &call) {
	PyRepTuple *iargs = call.tuple;
	PyRep *result = NULL;
	
	//args are: (channel, message)

	LSCChannelDesc channel;
	
	if(iargs->items.size() != 2) {
		_log(CLIENT__ERROR, "Invalid arg count to SendMessage: %d", iargs->items.size());
		//TODO: throw exception
	} else if(!channel.Decode( iargs->items[0] )) {
		_log(SERVICE__ERROR, "Invalid channel argument to SendMessage: unable to decode channel");
	} else if(!iargs->items[1]->CheckType(PyRep::String)) {
		_log(SERVICE__ERROR, "Invalid message argument to SendMessage: got %s, string expected", iargs->items[1]->TypeString());
	} else {
		PyRepString *msg = (PyRepString *) iargs->items[1];

		_log(SERVICE__MESSAGE, "Received chat msg for channel %s: %s", channel.ToString().c_str(), msg->value.c_str());

		std::map<LSCChannelDesc, LSCChannel *>::const_iterator res;
		res = m_channels.find(channel);
		if(res == m_channels.end()) {
			_log(SERVICE__ERROR, "Unable to find channel object for channel %s", channel.ToString().c_str());
		} else {
			res->second->Handle_SendMessage(call.client, msg->value.c_str());
		}
	}

	

	
	result = new PyRepNone();
	
	return(result);
}

PyCallResult LSCService::Handle_Page(PyCallArgs &call) {
	Call_Page args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}

	_log(SERVICE__MESSAGE, "%s: Received evemail msg with subject '%s': %s", call.client->GetName(), args.subject.c_str(), args.body.c_str());
	
	m_manager->SendNewEveMail(call.client->GetCharacterID(), args.recipients, args.subject, args.body);
	
	return(NULL);
}

//stuck here to be close to related functionality
void PyServiceMgr::SendNewEveMail(uint32 sender, std::vector<uint32> recipients, const std::string &subject, const std::string &content) {
	NotifyOnMessage notify;
	std::set<uint32> successful_recipients;
	
	notify.subject = subject;
	notify.sentTime = Win32TimeNow();
	notify.senderID = sender;

	// there's attachmentID and messageID... does this means a single message can contain multiple attachments?
	// eg. text/plain and text/html? we should be watching for this at reading mails...
	// created should be creation time. But Win32TimeNow returns uint64, and is stored as bigint(20),
	// so change in the db is needed
	std::vector<uint32>::iterator cur, end;
	cur = recipients.begin();
	end = recipients.end();

	for(; cur != end; cur++) {
		
		uint32 messageID = m_svcDB->StoreNewEVEMail(sender, *cur, subject.c_str(), content.c_str(), notify.sentTime);
		if(messageID == 0) {
			_log(SERVICE__ERROR, "Failed to store message from %lu for recipient %lu", sender, *cur);
			continue;
		}
		//TODO: supposed to have a different messageID in each notify I suspect..
		notify.messageID = messageID;
		
		_log(SERVICE__MESSAGE, "Delivered message from %lu to recipient %lu", sender, *cur);
		//record this person in the 'delivered to' list:
		notify.recipients.push_back(*cur);
		successful_recipients.insert(*cur);
	}
	
	//now, send a notification to each successful recipient
	PyRepTuple *answer = notify.Encode();
	entity_list->Multicast(successful_recipients, "OnMessage", "*multicastID", &answer, false);
}

//stuck here to be close to related functionality
//theres a lot of duplicated crap in here...
//this could be replaced by the SendNewEveMail if it weren't in the Client
void Client::SelfEveMail(const char *subject, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vaMakeAnyLenString(&str, fmt, args);
	va_end(args);
	
	NotifyOnMessage notify;
	notify.sentTime = Win32TimeNow();


	uint32 messageID = m_services->GetServiceDB()->StoreNewEVEMail(GetCharacterID(), GetCharacterID(), subject, str, notify.sentTime);
	if(messageID == 0) {
		_log(SERVICE__ERROR, "%s: Failed to store message '%s' for self %lu", GetName(), subject, GetCharacterID());
		delete[] str;
		return;
	}
	delete[] str;
	
	notify.messageID = messageID;
	notify.senderID = GetCharacterID();
	notify.recipients.push_back(GetCharacterID());
	notify.subject = subject;
	PyRepTuple *alert = notify.Encode();
	
	SendNotification("OnMessage", "*multicastID", &alert, false);
}



PyCallResult LSCService::Handle_MarkMessagesRead(PyCallArgs &call) {
	Call_SingleIntList args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}
	
	std::vector<uint32>::iterator cur, end;
	cur = args.ints.begin();
	end = args.ints.end();
	for(; cur != end; cur++) {
		m_db.MarkMessageRead(*cur);
	}
	return(NULL);
}

PyCallResult LSCService::Handle_DeleteMessages(PyCallArgs &call) {
	Call_DeleteMessages args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}

	if(args.channelID != call.client->GetCharacterID()) {
		_log(SERVICE__ERROR, "%s (%d) tried to delete messages in channel %lu. Denied.", call.client->GetName(), call.client->GetCharacterID(), args.channelID);
		return(NULL);
	}
	
	std::vector<uint32>::iterator cur, end;
	cur = args.messages.begin();
	end = args.messages.end();
	for(; cur != end; cur++) {
		m_db.DeleteMessage(*cur, args.channelID);
	}

	return(NULL);
}


bool LSCService::ExecuteCommand(Client *from, const char *msg) {
	return(m_commandDispatch->Execute(from, msg));
}

bool LSCService::CreateChannel(const LSCChannelDesc &desc) {
	if(m_channels.find(desc) != m_channels.end()) {
		return(true); //already exists.
	}
	LSCChannel *chan = new LSCChannel(this, desc, desc.number, NULL, NULL, NULL, true, NULL, true, true, false, 15);
	m_channels[desc] = chan;
	return(true);
}

/*
void LSCService::_LoadChannel(const LSCChannelDesc &cid) {
	//eventually we might want to do something more useful here...
	std::map<LSCChannelDesc, LSCChannel *>::iterator res;
	res = m_channels.find(cid);
	if(res != m_channels.end()) {
		_log(SERVICE__ERROR, "Requested to load channel %s, but it is already loaded!", cid.ToString().c_str());
		return;
	}

	LSCChannel *chan = m_db.LoadChannel(this, cid);
	
	
	
	m_channels[cid] = new LSCChannel(this, cid);
}*/

PyRepTuple *LSCService::_QueryChannelDetails(const LSCChannelDesc &channel_id) {
	PyRepTuple *rows = new PyRepTuple(3);
	
	//these lists should really come from the channel object... in fact this whole section of code should be in the channel.
	
	//info
	rows->items[0] = m_db.QueryChannelInfo(channel_id.number);
	if(rows->items[0] == NULL) {
		_log(SERVICE__ERROR, "Unable to query channel info for channel %s", channel_id.ToString().c_str());
		delete rows;
		return(NULL);
	}
	
	//acl
	rows->items[1] = m_db.QueryChannelMods(channel_id.number);
	if(rows->items[1] == NULL) {
		_log(SERVICE__ERROR, "Unable to query channel mods for channel %s", channel_id.ToString().c_str());
		delete rows;
		return(NULL);
	}
	
	//memberList
	rows->items[2] = m_db.QueryChannelChars(channel_id.number);
	if(rows->items[2] == NULL) {
		_log(SERVICE__ERROR, "Unable to query channel chars for channel %s", channel_id.ToString().c_str());
		delete rows;
		return(NULL);
	}

	return(rows);
}

OnLSC_SenderInfo *LSCService::_MakeSenderInfo(Client *from) {
	OnLSC_SenderInfo *sender = new OnLSC_SenderInfo;
	
	sender->senderID = from->GetCharacterID();
	sender->senderName = from->GetName();
	sender->senderType = from->GetChar().typeID;
	sender->corpID = from->GetCorporationID();
	sender->role = from->GetRole();
	sender->corp_role = 0;

	return(sender);
}

LSCChannelDesc::LSCChannelDesc(uint32 num, Type t)
: type(t),
  number(num)
{
}

const char *LSCChannelDesc::GetTypeString() const {
	switch(type) {
	case normal:
		return("normal");
	case corp:
		return("corpid");
	case solarsystem:
		return("solarsystemid2");
	case region:
		return("regionid");
	case constellation:
		return("constellationid");
	//no default on purpose
	}
	return("unknown");
}
	
PyRep *LSCChannelDesc::Encode() const {
	if(type == normal)
		return(new PyRepInteger(number));

	LSCChannelMultiDesc desc;
	desc.number = number;
	desc.type = GetTypeString();
	
	return(desc.Encode());
}

bool LSCChannelDesc::Decode(const PyRep *from) {
	if(from->CheckType(PyRep::Integer)) {
		type = normal;
		const PyRepInteger *v = (const PyRepInteger *) from;
		number = v->value;
		return(true);
	}

	PyRep *from_dup = from->Clone();	//shouldent have to do this...
	LSCChannelMultiDesc md;
	if(!md.Decode(&from_dup)) {
		_log(SERVICE__WARNING, "Failed to decode LSC channel description");
		return(false);
	}
	
	if(md.type == "corpid")
		type = corp;
	else if(md.type == "solarsystemid2")
		type = solarsystem;
	else if(md.type == "regionid")
		type = region;
	else if(md.type == "constellationid")
		type = constellation;
	else {
		_log(SERVICE__WARNING, "Invalid channel type in LSC channel description: %s", md.type.c_str());
		return(false);
	}

	number = md.number;

	return(true);
}

std::string LSCChannelDesc::ToString() const {
	char buf[32];
	if(type == normal)
		snprintf(buf, 32, "%lu", number);
	else
		snprintf(buf, 32, "%s:%lu", GetTypeString(), number);
	return(std::string(buf));
}




















