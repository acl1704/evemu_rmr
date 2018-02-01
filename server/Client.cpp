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




#include "../common/common.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "Client.h"
#include "../common/PyPacket.h"
#include "../common/PyRep.h"
#include "../common/EVEPresentation.h"
#include "../common/logsys.h"
#include "../common/EVEMarshal.h"
#include "../common/packet_functions.h"
#include "../common/Base64.h"
#include "../common/misc.h"
#include "../common/EVEUnmarshal.h"
#include "../common/EVEVersion.h"
#include "../common/PyDumpVisitor.h"
#include "../common/DestinyStructs.h"
#include "../common/EVEUtils.h"
#include "../common/MiscFunctions.h"
#include "../common/sigexcept/sigexcept.h"

#include "../packets/AccountPkts.h"
#include "../packets/General.h"
#include "../packets/Inventory.h"
#include "../packets/Wallet.h"
#include "../packets/Character.h"
#include "../packets/DogmaIM.h"
#include "../packets/Destiny.h"

#include "PyService.h"
#include "PyServiceMgr.h"
#include "ServiceDB.h"
#include "EntityList.h"
#include "chat/LSCChannel.h"
#include "inventory/InventoryItem.h"
#include "inventory/ItemFactory.h"
#include "cache/ObjCacheService.h"
#include "ship/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/Damage.h"
#include "NPC.h"

static const uint32 PING_INTERVAL_US = 60000;
static const uint64 HackFixedClientID = 130293001608LL;	//should prolly generate these for each client some day...

Client::Client(PyServiceMgr *services, EVEPresentation **n)
: DynamicSystemEntity(NULL),
  modules(this),
  m_ship(NULL),
  m_services(services),
  m_net(*n),
  m_pingTimer(PING_INTERVAL_US),
  m_accountID(0),
  m_role(1),
  m_gangRole(1),
  m_system(NULL),
  char_valid(false),
//  m_destinyTimer(1000, true),	//accurate timing is essential
//  m_lastDestinyTime(Timer::GetTimeSeconds()),
  m_moveState(msIdle),
  m_moveTimer(500),
  m_movePoint(0, 0, 0),
  m_nextNotifySequence(1)
//  m_nextDestinyUpdate(46751)
{
	*n = NULL;

	m_moveTimer.Disable();
	m_pingTimer.Start();
	//m_destinyTimer.Start();

	char_valid = true;
	m_char.name = "monkey";
	m_char.charid = 444666;
	m_char.bloodlineID = 1;
	m_char.genderID = 0;
	m_char.ancestryID = 2;
	m_char.schoolID = 3;
	m_char.departmentID = 4;
	m_char.fieldID = 5;
	m_char.specialityID = 6;
	m_char.Intelligence = 7;
	m_char.Charisma = 8;
	m_char.Perception = 9;
	m_char.Memory = 10;
	m_char.Willpower = 11;
}

Client::~Client() {
	//make sure we leave any channels we were in...
	{
		std::set<LSCChannel *>::iterator cur, end;
		cur = m_channels.begin();
		end = m_channels.end();
		for(; cur != end; cur++) {
			(*cur)->Handle_LeaveChannel(this);
		}
	}
	
	
	m_services->ClearBoundObjects(this);
	
	//before we remove ourself from the system, store our last location.
	SavePosition();
	
	if(m_system != NULL)
		m_system->RemoveClient(this);	//handles removing us from bubbles and sending RemoveBall events.
	
	delete m_net;
	
	m_ship->Release();
	targets.DoDestruction();
}

void Client::QueuePacket(PyPacket *p) {
	p->userid = m_accountID;
	m_net->QueuePacket(p);
}

void Client::FastQueuePacket(PyPacket **p) {
	m_net->FastQueuePacket(p);
}

bool Client::ProcessNet() {
	TRY_SIGEXCEPT {
		
		if(!m_net->Connected())
			return(false);
		
		if(m_pingTimer.Check()) {
			_SendPingRequest();
		}
		
		PyPacket *p;
		while((p = m_net->PopPacket())) {
			{
				PyLogsysDump dumper(CLIENT__IN_ALL);
				_log(CLIENT__IN_ALL, "Received packet:");
				p->Dump(CLIENT__IN_ALL, &dumper);
			}
	
			switch(p->type) {
			case AUTHENTICATION_REQ:
				_ProcessLogin(p);
				break;
			case CALL_REQ:
				_ProcessCallRequest(p);
				break;
			case NOTIFICATION:
				_ProcessNotification(p);
				break;
			case PING_RSP:
				_log(CLIENT__TRACE, "%s: Received ping response.", GetName());
				break;
			default:
				_log(CLIENT__ERROR, "%s: Unhandled message type %d", GetName(), p->type);
			}
			
			delete p;
			p = NULL;
		}
	
		return(true);
	} CATCH_SIGEXCEPT(e) {
		  _log(CLIENT__ERROR, "%s: Exception caught processing network packets\n%s", GetName(), e.stack_string().c_str());
		  return(false);
	}
}

void Client::Process() {
	if(m_moveTimer.Check(false)) {
		m_moveTimer.Disable();
		_MoveState s = m_moveState;
		m_moveState = msIdle;
		switch(s) {
		case msIdle:
			_log(CLIENT__ERROR, "%s: Move timer expired when no move is pending.", GetName());
			break;
		case msWarp:
			_ExecuteWarp();
			break;
		case msJump:
			_ExecuteJump();
			break;
		case msJump2:
			_ExecuteJump_Phase2();
			break;
		case msJump3:
			_ExecuteJump_Phase3();
			break;
		case msLogIntoSpace:
			_ExecuteLogIntoSpace();
			break;
		case msLogIntoStation:
			_ExecuteLogIntoStation();
			break;
		case msUndockIntoSpace:
			_ExecuteUndockIntoSpace();
			break;
		}
	}

	modules.Process();
	
	SystemEntity::Process();
}

//this displays a modal error dialog on the client side.
void Client::SendErrorMsg(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vaMakeAnyLenString(&str, fmt, args);
	
	_log(CLIENT__ERROR, "Sending Error Message to %s:", GetName());
	log_messageVA(CLIENT__ERROR, fmt, args);
	va_end(args);

	//want to send some sort of notify with a "ServerMessage" message ID maybe?
	//else maybe a "ChatTxt"??
	Notify_OnRemoteMessage n;
	n.msgType = "CustomError";
	n.args[ "error" ] = new PyRepString(str);
	PyRepTuple *tmp = n.FastEncode();
	
	SendNotification("OnRemoteMessage", "charid", &tmp);
	
	delete[] str;
}

//this displays a modal info dialog on the client side.
void Client::SendInfoModalMsg(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vaMakeAnyLenString(&str, fmt, args);
	
	_log(CLIENT__MESSAGE, "Info Modal to %s:", GetName());
	log_messageVA(CLIENT__MESSAGE, fmt, args);
	va_end(args);

	//want to send some sort of notify with a "ServerMessage" message ID maybe?
	//else maybe a "ChatTxt"??
	Notify_OnRemoteMessage n;
	n.msgType = "ServerMessage";
	n.args[ "msg" ] = new PyRepString(str);
	PyRepTuple *tmp = n.FastEncode();
	
	SendNotification("OnRemoteMessage", "charid", &tmp);
	
	delete[] str;
}

//this displays a little notice (like combat messages)
void Client::SendNotifyMsg(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vaMakeAnyLenString(&str, fmt, args);

	_log(CLIENT__MESSAGE, "Notify to %s:", GetName());
	log_messageVA(CLIENT__MESSAGE, fmt, args);
	va_end(args);

	//want to send some sort of notify with a "ServerMessage" message ID maybe?
	//else maybe a "ChatTxt"??
	Notify_OnRemoteMessage n;
	n.msgType = "CustomNotify";
	n.args[ "notify" ] = new PyRepString(str);
	PyRepTuple *tmp = n.FastEncode();
	
	SendNotification("OnRemoteMessage", "charid", &tmp);
	
	delete[] str;
}

//there may be a less hackish way to do this.
void Client::SelfChatMessage(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vaMakeAnyLenString(&str, fmt, args);
	va_end(args);

	if(m_channels.empty()) {
		_log(CLIENT__ERROR, "%s: Tried to send self chat, but we are not joined to any channels: %s", GetName(), str);
		delete[] str;
		return;
	}

	
	_log(CLIENT__TEXT, "%s: Self message on all channels: %s", GetName(), str);

	//this is such a pile of crap, but im not sure whats better.
	//maybe a private message...
	std::set<LSCChannel *>::iterator cur, end;
	cur = m_channels.begin();
	end = m_channels.end();
	for(; cur != end; cur++) {
		(*cur)->SendUnicastMessage(this, str);
	}

	//just send it to the first channel we are in..
    /*LSCChannel *chan = *(m_channels.begin());
	char self_id[24];	//such crap..
	snprintf(self_id, sizeof(self_id), "%lu", GetCharacterID());
	if(chan->GetName() == self_id) {
		if(m_channels.size() > 1) {
			chan = *(++m_channels.begin());
		}
    }*/
	
	delete[] str;
}

void Client::ChannelJoined(LSCChannel *chan) {
	m_channels.insert(chan);
}

void Client::ChannelLeft(LSCChannel *chan) {
	//disabled for now.. need to deal with destructor->LSCChannel::Handle_LeaveChannel->ChannelLeft nested mod of m_channels.
//	m_channels.erase(chan);
}

void Client::SendHandshake() {
	uint32 user_count = 17973;
	m_net->SendHandshake(MachoNetVersion, user_count, EVEVersionNumber, EVEBuildVersion, EVEProjectVersion);
}

void Client::_ProcessLogin(PyPacket *packet) {
	PyRep *payload = packet->payload;	//stupid casting thing..
	packet->payload = NULL;
	
	AuthenticationReq req;
	if(!req.Decode(&payload)) {
		_log(CLIENT__ERROR, "Failed to decode AuthenticationReq, rejecting.\n");
		_SendLoginFailed(packet->source.callID);
		return;
	}
	/*
	if(packet->payload->items.size() != 3) {
		_log(CLIENT__ERROR, "macho.AuthenticationReq: invalid tuple length %d", packet->payload->items.size());
		_SendLoginFailed(packet->source.callID);
		return;
	}
	if(!packet->payload->items[1]->CheckType(PyRep::String)) {
		_log(CLIENT__ERROR, "macho.AuthenticationReq: non-string login name");
		_SendLoginFailed(packet->source.callID);
		return;
	}
	if(!packet->payload->items[2]->CheckType(PyRep::String)) {
		_log(CLIENT__ERROR, "macho.AuthenticationReq: non-string password");
		_SendLoginFailed(packet->source.callID);
		return;
	}
	PyRepString *login = (PyRepString *) packet->payload->items[1];
	PyRepString *password = (PyRepString *) packet->payload->items[2];
*/
	_log(CLIENT__MESSAGE, "Login with %s/%s", req.login.c_str(), req.password.c_str());
	req.Dump(CLIENT__MESSAGE, "");
	
	if(!m_services->GetServiceDB()->DoLogin(req.login.c_str(), req.password.c_str(), m_accountID, m_role)) {
		_log(CLIENT__MESSAGE, "Login rejected by DB");
		_SendLoginFailed(packet->source.callID);
		return;
	}
	
	session.Set_userType(23);	//TODO: what is this??
	session.Set_userid(m_accountID);
	session.Set_address(m_net->GetConnectedAddress().c_str());
	session.Set_role(m_role);
	session.Set_languageID(req.languageID.c_str());
	
	_CheckSessionChange();	// get session change out before success reply
	
	_SendLoginSuccess(packet->source.callID);
	//_SendLoginFailed(packet->source.callID);
}

void Client::_SendLoginSuccess(uint32 callid) {

	AuthenticationRsp rsp;
	rsp.project_version = EVEProjectVersion;
	rsp.version_number = EVEVersionNumber;
	rsp.build_version = EVEBuildVersion;

	rsp.accountID = m_accountID;
	rsp.role = m_role;
	rsp.proxyNodeID = 10012002;

	m_services->GetCache()->InsertCacheHints(
		ObjCacheService::hLoginCachables, 
		&rsp.cachables);

	_BuildServiceListDict(&rsp.services);
	

	//build the packet:
	PyPacket *p = new PyPacket();
	p->type_string = "macho.AuthenticationRsp";
	p->type = AUTHENTICATION_RSP;
	
	p->source.type = PyAddress::Any;

	p->dest.type = PyAddress::Client;
	p->dest.typeID = HackFixedClientID;
	p->dest.callID = callid;

	p->userid = m_accountID;
	
	p->payload = (PyRepTuple *) rsp.Encode();
	
	p->named_payload = new PyRepDict();
	p->named_payload->add("channel", new PyRepString("authentication"));
	
	FastQueuePacket(&p);
}


void Client::_SendPingRequest() {
	PyPacket *ping_req = new PyPacket();
	
	ping_req->type = PING_REQ;
	ping_req->type_string = "macho.PingReq";
	
	ping_req->source.type = PyAddress::Node;
	ping_req->source.typeID = m_services->GetNodeID();
	ping_req->source.service = "ping";
	ping_req->source.callID = 0;
	
	ping_req->dest.type = PyAddress::Client;
	ping_req->dest.typeID = HackFixedClientID;
	ping_req->dest.callID = 0;
	
	ping_req->userid = m_accountID;
	
	ping_req->payload = new PyRepTuple(1);
	ping_req->payload->items[0] = new PyRepList();	//times
	ping_req->named_payload = new PyRepDict();
	
	FastQueuePacket(&ping_req);
}

void Client::_BuildServiceListDict(PyRepDict *into) {
	into->items[ new PyRepString("objectCaching") ] = new PyRepNone();
	into->items[ new PyRepString("lookupSvc") ] = new PyRepNone();
	into->items[ new PyRepString("beyonce") ] = new PyRepNone();
	into->items[ new PyRepString("standing2") ] = new PyRepNone();
	into->items[ new PyRepString("ram") ] = new PyRepNone();
	into->items[ new PyRepString("lien") ] = new PyRepNone();
	into->items[ new PyRepString("voucher") ] = new PyRepNone();
	into->items[ new PyRepString("entity") ] = new PyRepNone();
	into->items[ new PyRepString("keeper") ] = new PyRepString("solarsystem");
	into->items[ new PyRepString("agentMgr") ] = new PyRepNone();
	into->items[ new PyRepString("dogmaIM") ] = new PyRepNone();
	into->items[ new PyRepString("machoNet") ] = new PyRepNone();
	into->items[ new PyRepString("watchdog") ] = new PyRepNone();
	into->items[ new PyRepString("ship") ] = new PyRepNone();
	into->items[ new PyRepString("npcSvc") ] = new PyRepNone();
	into->items[ new PyRepString("cacheSvc") ] = new PyRepNone();
	into->items[ new PyRepString("market") ] = new PyRepNone();
	into->items[ new PyRepString("dungeon") ] = new PyRepNone();
	into->items[ new PyRepString("aggressionMgr") ] = new PyRepNone();
	into->items[ new PyRepString("sessionMgr") ] = new PyRepNone();
	into->items[ new PyRepString("LSC") ] = new PyRepString("location");
	into->items[ new PyRepString("allianceRegistry") ] = new PyRepNone();
	into->items[ new PyRepString("bookmark") ] = new PyRepString("location");
	into->items[ new PyRepString("invbroker") ] = new PyRepNone();
	into->items[ new PyRepString("character") ] = new PyRepNone();
	into->items[ new PyRepString("factory") ] = new PyRepNone();
	into->items[ new PyRepString("corpStationMgr") ] = new PyRepNone();
	into->items[ new PyRepString("authentication") ] = new PyRepNone();
	into->items[ new PyRepString("station") ] = new PyRepString("station");
	into->items[ new PyRepString("slash") ] = new PyRepNone();
	into->items[ new PyRepString("charmgr") ] = new PyRepNone();
	into->items[ new PyRepString("stationSvc") ] = new PyRepNone();
	into->items[ new PyRepString("gangsta") ] = new PyRepNone();
	into->items[ new PyRepString("config") ] = new PyRepString("locationPreferred");
	into->items[ new PyRepString("deadlineMgr") ] = new PyRepNone();
	into->items[ new PyRepString("billingMgr") ] = new PyRepNone();
	into->items[ new PyRepString("billMgr") ] = new PyRepNone();
	into->items[ new PyRepString("map") ] = new PyRepNone();
	into->items[ new PyRepString("posMgr") ] = new PyRepNone();
	into->items[ new PyRepString("lootSvc") ] = new PyRepNone();
	into->items[ new PyRepString("http") ] = new PyRepNone();
	into->items[ new PyRepString("gagger") ] = new PyRepNone();
	into->items[ new PyRepString("dataconfig") ] = new PyRepNone();
	into->items[ new PyRepString("DB") ] = new PyRepNone();
	into->items[ new PyRepString("i2") ] = new PyRepNone();
	into->items[ new PyRepString("account") ] = new PyRepString("location");
	into->items[ new PyRepString("telnet") ] = new PyRepNone();
	into->items[ new PyRepString("alert") ] = new PyRepNone();
	into->items[ new PyRepString("director") ] = new PyRepNone();
	into->items[ new PyRepString("dogma") ] = new PyRepNone();
	into->items[ new PyRepString("pathfinder") ] = new PyRepNone();
	into->items[ new PyRepString("corporationSvc") ] = new PyRepNone();
	into->items[ new PyRepString("clones") ] = new PyRepNone();
	into->items[ new PyRepString("effectCompiler") ] = new PyRepNone();
	into->items[ new PyRepString("corpmgr") ] = new PyRepNone();
	into->items[ new PyRepString("warRegistry") ] = new PyRepNone();
	into->items[ new PyRepString("corpRegistry") ] = new PyRepNone();
	into->items[ new PyRepString("missionMgr") ] = new PyRepNone();
	into->items[ new PyRepString("userSvc") ] = new PyRepNone();
	into->items[ new PyRepString("counter") ] = new PyRepNone();
	into->items[ new PyRepString("petitioner") ] = new PyRepNone();
	into->items[ new PyRepString("debug") ] = new PyRepNone();
	into->items[ new PyRepString("shipSvc") ] = new PyRepNone();
	into->items[ new PyRepString("onlineStatus") ] = new PyRepNone();
	into->items[ new PyRepString("skillMgr") ] = new PyRepNone();
}


void Client::_CheckSessionChange() {
	if(!session.IsDirty())
		return;
	
	PyRepDict *d = session.EncodeChange();
	if(d->items.size() == 0) {
		delete d;
		return;
	}

	_log(CLIENT__SESSION, "Session updated, sending session change");
	d->Dump(CLIENT__SESSION, "  Changes: ");
	session.Dump(CLIENT__SESSION);
	
	PyRepTuple *t = new PyRepTuple(2);
		PyRepTuple *maint = new PyRepTuple(2);
		t->items[0] = maint;
			maint->items[0] = new PyRepInteger(0);
			maint->items[1] = d;
		PyRepList *nodesOfInterest = new PyRepList();
		t->items[1] = nodesOfInterest;
			//this is prolly not nescesary... and is a little kludgy
			if(GetCharacterID() != 0)
				nodesOfInterest->add(new PyRepInteger(m_services->GetNodeID()));


	//build the packet:
	PyPacket *p = new PyPacket();
	p->type_string = "macho.SessionChangeNotification";
	p->type = SESSIONCHANGENOTIFICATION;
	
	p->source.type = PyAddress::Node;
	p->source.typeID = m_services->GetNodeID();
	p->source.callID = 0;

	p->dest.type = PyAddress::Client;
	p->dest.typeID = HackFixedClientID;
	p->dest.callID = 0;

	p->userid = 0;
	
	p->payload = t;
	
	p->named_payload = new PyRepDict();
	p->named_payload->add("channel", new PyRepString("sessionchange"));

	FastQueuePacket(&p);
}

/* sync the session with m_char. */
void Client::SessionSyncChar() {
	session.Set_constellationid(m_char.constellationID);
	session.Set_corpid(m_char.corporationID);
	session.Set_regionid(m_char.regionID);
	if(m_char.stationID != 0) {
		session.Set_stationid(m_char.stationID);
		session.Clear_solarsystemid();
		session.Set_locationid(m_char.stationID);
	} else {
		session.Clear_stationid();
		session.Set_solarsystemid(m_char.solarSystemID);
		session.Set_locationid(m_char.solarSystemID);
	}
	session.Set_gangrole(m_gangRole);
#ifndef WIN32
#warning hqID hacked for station
#endif
	session.Set_hqID(60004420);
	session.Set_solarsystemid2(m_char.solarSystemID);
	session.Set_shipid(GetShipID());
	session.Set_charid(m_char.charid);

	//force a session send.
	_CheckSessionChange();
}

bool Client::EnterSystem(const GPoint &point) {
	if(m_system != NULL) {
		m_system->RemoveClient(this);
		m_system = NULL;
		//send removeball after we are removed from the system, so we dont get it.
		//if(m_destiny != NULL)	//if we were in a station, we have no destiny manager.
		//	m_destiny->SendRemoveBall();
	}
	delete m_destiny;
	m_destiny = NULL;
	
	//find our system manager and register ourself with it.
	m_system = m_services->entity_list->FindOrBootSystem(GetSystemID());
	if(m_system == NULL) {
		_log(CLIENT__ERROR, "Failed to boot system %lu for char %s (%lu)", GetSystemID(), GetName(), GetCharacterID());
		SendErrorMsg("Unable to boot system %lu", GetSystemID());
		return(false);
	}
	
	if(IsStation(GetLocationID())) {
		//leave destiny NULL, we do not process destiny while in station.
		NotifyOnCharNowInStation n;
		n.charID = GetCharacterID();
		PyRepTuple *tmp = n.Encode();
		m_services->entity_list->Broadcast("OnCharNowInStation", "stationid", &tmp);
	} else if(IsSolarSystem(GetLocationID())) {
		//if we are not in a station, we are in a system, so we need a destiny manager
		m_destiny = new DestinyManager(this, m_system);
		//ship should never be NULL.
		m_destiny->SetShipCapabilities(m_ship);
		//set position.
		//NOTE: this actually puts us in our bubble before we are added to the system below...
		m_destiny->SetPosition(point, false);

		//for now, we always enter a system stopped.
		m_destiny->Halt(false);
		
		//send add ball before we add ourself to the system, so we dont get it.
//		m_destiny->SendAddBall();	//bubble manager does this now.
		m_destiny->SendJumpIn();
	} else {
		_log(CLIENT__ERROR, "Char %s (%lu) is in a bad location %lu", GetName(), GetCharacterID(), GetLocationID());
		SendErrorMsg("In a bad location %lu", GetLocationID());
		return(false);
	}
	
	m_system->AddClient(this);
	return(true);
}

void Client::MoveToJumpGate(uint32 systemID, uint32 gateID) {
	if(!IsSolarSystem(systemID)) {
		codelog(CLIENT__ERROR, "%s: system %lu, gate %lu: NOT A SYSTEM!", GetName(), systemID, gateID);
		return;
	}
	//TODO: verify that this is actually a gate?
	// (no real harm I guess in using this on other static celestial objects)
	if(!IsStargate(gateID)) {
		codelog(CLIENT__ERROR, "%s: system %lu, gate %lu: NOT A GATE!", GetName(), systemID, gateID);
		return;
	}
	GPoint pos;
	//TODO: verify that this gate is actually in this system
	if(!m_services->GetServiceDB()->GetStaticPosition(gateID, pos.x, pos.y, pos.z)) {
		codelog(CLIENT__ERROR, "%s: system %lu, gate %lu: Unable to find gate.", GetName(), systemID, gateID);
		return;
	}
	pos.x += 100.0;
	pos.y += 100.0;
	pos.z += 100.0;
	_log(CLIENT__TRACE, "%s: move to jump gate %lu in system %lu,: (%.2f, %.2f, %.2f).", GetName(), gateID, systemID, pos.x, pos.y, pos.z);
	MoveToLocation(systemID, pos);
}

void Client::MoveToStationDock(uint32 systemID, uint32 stationID) {
	if(!IsSolarSystem(systemID)) {
		codelog(CLIENT__ERROR, "%s: system %lu, station %lu: NOT A SYSTEM!", GetName(), systemID, stationID);
		return;
	}
	if(!IsStation(stationID)) {
		codelog(CLIENT__ERROR, "%s: system %lu, station %lu: NOT A STATION!", GetName(), systemID, stationID);
		return;
	}
	GPoint pos;
	//TODO: verify that this station is actually in this system
	if(!m_services->GetServiceDB()->GetStationDockPosition(stationID, pos)) {
		codelog(CLIENT__ERROR, "%s: system %lu, station %lu: NOT A SYSTEM!", GetName(), systemID, stationID);
		return;
	}
	_log(CLIENT__TRACE, "%s: move to station %lu's dock in system %lu,: (%.2f, %.2f, %.2f).", GetName(), stationID, systemID, pos.x, pos.y, pos.z);
	MoveToLocation(systemID, pos);
}

void Client::MoveToLocation(uint32 location, const GPoint &pt) {
	EVEItemFlags flag;

	if(GetLocationID() == location) {
		// This is a warp or simple movement
		MoveToPosition(pt);
		return;
	}
	
	if(IsStation(GetLocationID())) {
		//TODO: send OnCharNoLongerInStation
	}
	
	if(IsStation(location)) {
		// Entering station
		m_char.stationID = location;
		
		m_services->GetServiceDB()->GetStationParents(
			m_char.stationID,
			m_char.solarSystemID, m_char.constellationID, m_char.regionID );

		flag = flagHangar;
	} else if(IsSolarSystem(location)) {
		// Entering a solarsystem
		// source is GetLocation()
		// destinaion is location

		m_char.stationID = 0;
		m_char.solarSystemID = location;
		
		m_services->GetServiceDB()->GetSystemParents(
			m_char.solarSystemID,
			m_char.constellationID, m_char.regionID );

		flag = flagShipOffline;
	} else {
		SendErrorMsg("Move requested to unsupported location %lu", location);
		return;
	}

	if(!EnterSystem(pt))
		return;
	
	//move the character_ record... we really should derive the char's location from the entity table...
	m_services->GetServiceDB()->SetCharacterLocation(
		GetCharacterID(),
		m_char.stationID, m_char.solarSystemID, 
		m_char.constellationID, m_char.regionID );
	
	
	SessionSyncChar();
	
	//move the ship into space
	MoveItem(GetShipID(), location, flag);
}

void Client::MoveToPosition(const GPoint &pt) {
	if(m_destiny == NULL)
		return;
	m_destiny->Halt(true);
	m_destiny->SetPosition(pt, true);
}

void Client::MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag) {
	
	InventoryItem *item = m_services->item_factory->Load(itemID, false);
	if(item == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to load item %lu", GetName(), itemID);
		return;
	}
	
	bool was_module = ( item->flag() >= flagSlotFirst && item->flag() <= flagSlotLast);

	//do the move. This will update the DB and send the notification.
	item->Move(location, flag);

	if(was_module || (item->flag() >= flagSlotFirst && item->flag() <= flagSlotLast)) {
		//it was equipped, or is now. so modules need to know.
		modules.UpdateModules();
	}
	
	//release the item ref
	item->Release();
}

void Client::BoardShip(uint32 ship_id, bool do_destiny_updates) {
	//get the real item...
	InventoryItem *new_ship;
	new_ship = m_services->item_factory->Load(ship_id, true);
	if(new_ship == NULL) {
		_log(CLIENT__ERROR, "%s: Unable to find ship %lu in order to board it!", GetName(), ship_id);
		return;
	}
	
	if(!new_ship->LoadContents(true)) {
		_log(CLIENT__ERROR, "%s: Unable to load contents of ship %lu in order to board it!", GetName(), ship_id);
		return;
	}
	
	BoardExistingShip(new_ship, do_destiny_updates);	//makes its own ref.
	
	new_ship->Release();
}

void Client::BoardExistingShip(InventoryItem *new_ship, bool do_destiny_updates) {
	//TODO: make sure we are really allowed to board this thing...
	
	if(new_ship->singleton() == false) {
		_log(CLIENT__ERROR, "%s: tried to board ship %lu, which is not assembled.", GetName(), new_ship->itemID());
		SendErrorMsg("You cannot board a ship which is not assembled!");
		return;
	}
	
	if(do_destiny_updates && m_destiny != NULL) {
		m_destiny->SendRemoveBall();
	}
	
	session.Set_shipid(new_ship->itemID());
	m_self->MoveInto(new_ship, flagPilot, false);
	InventoryItem *old_ship = m_ship;
	m_ship = new_ship->Ref();
	//we are not longer referencing m_ship directly (its still in our inventory though)
	old_ship->Release();

	//I am not sure where the right place to do this is, but until
	//we properly persist ship attributes into the DB, we are just
	//going to do it here. Could be exploited. oh well.
	// TODO: use the ship aggregate value.
	m_ship->Set_shieldCharge(m_ship->shieldCapacity());
	
	modules.UpdateModules();
	
	if(do_destiny_updates && m_destiny != NULL) {
		m_destiny->SetShipCapabilities(m_ship);
		m_destiny->SendAddBall();
	}
}

void Client::_SendLoginFailed(uint32 callid) {
	
	//build the exception
	PyRepTuple *t = new PyRepTuple(3);
		t->items[0] = new PyRepInteger(0);
		t->items[1] = new PyRepInteger(2);
		PyRepTuple *et = new PyRepTuple(1);
		t->items[2] = et;
			PyRepSubStream *ss = new PyRepSubStream();
			et->items[0] = ss;
				PyRepObject *sso = new PyRepObject();
				ss->decoded = sso;
					sso->type = "ccp_exceptions.UserError";
					PyRepDict *ssa = new PyRepDict();   
					sso->arguments = ssa;
						PyRepTuple *ssaa = new PyRepTuple(1);
						ssa->items[ new PyRepString("args") ] = ssaa;
							ssaa->items[0] = new PyRepString("LoginAuthFailed");
						ssa->items[ new PyRepString("dict") ] =
							new PyRepNone();
						ssa->items[ new PyRepString("msg") ] =
							new PyRepString("LoginAuthFailed");
					//end arguments to obj
				//end obj
			//end substream
		//end tuple[2]
	//end tuple

	//build the packet:
	PyPacket *p = new PyPacket();
	p->type_string = "macho.ErrorResponse";
	p->type = ERRORRESPONSE;
	
	p->source.type = PyAddress::Any;

	p->dest.type = PyAddress::Client;
	p->dest.typeID = HackFixedClientID;
	p->dest.callID = callid;

	p->userid = 0;
	
	p->payload = t;
	
	p->named_payload = new PyRepDict();
	p->named_payload->add("channel", new PyRepString("authentication"));
	
	FastQueuePacket(&p);
}

void Client::_ProcessCallRequest(PyPacket *packet) {

	PyService *svc = m_services->LookupService(packet);
	if(svc == NULL) {
		_log(CLIENT__ERROR, "Unable to find service to handle call to:");
		packet->dest.Dump(stdout, "    ");
#ifndef WIN32
#warning TODO: throw proper exception to client.
#endif
		PyRep *except = new PyRepNone();
		_SendException(packet, WRAPPEDEXCEPTION, &except);
		return;
	}
	
	//turn this thing into a call:
	PyCallStream call;
	if(!call.Decode(packet->type_string, packet->payload)) {	//payload is consumed
		_log(CLIENT__ERROR, "Failed to convert rep into a call stream");
#ifndef WIN32
#warning TODO: throw proper exception to client.
#endif
		PyRep *except = new PyRepNone();
		_SendException(packet, WRAPPEDEXCEPTION, &except);
		return;
	}

	if(is_log_enabled(CLIENT__CALL_DUMP)) {
		PyLogsysDump dumper(CLIENT__CALL_DUMP);
		_log(CLIENT__CALL_DUMP, "Call Stream Decoded:");
		packet->source.Dump(CLIENT__CALL_DUMP, "  From: ");
		packet->dest.Dump(CLIENT__CALL_DUMP, "  To: ");
		call.Dump(CLIENT__CALL_DUMP, &dumper);
	}
	
	//build arguments
	PyCallArgs args(this, &call.arg_tuple, &call.arg_dict);
	
	//parts of call may be consumed here
	PyCallResult result = svc->Call(call, args);
	
	switch(result.type) {
	case PyCallResult::RegularResult: {
		//successful call.
		PyRepTuple *t = new PyRepTuple(1);
		t->items[0] = result.ssResult.hijack();
		
		_CheckSessionChange();	//send out the session change before the return.
		
		_SendCallReturn(packet, &t);
	} break;
	
	case PyCallResult::ThrowException: {
		PyRep *except = result.ssResult.hijack();
		_SendException(packet, WRAPPEDEXCEPTION, &except);
	} break;
	//no default on purpose
	}
}

void Client::_ProcessNotification(PyPacket *packet) {
	//turn this thing into a notify stream:
	ServerNotification *notify = new ServerNotification();
	if(!notify->Decode(&packet->payload)) {	//payload is consumed
		_log(CLIENT__ERROR, "Failed to convert rep into a notify stream");
		delete notify;
		return;
	}
	
	if(notify->method == "ClientHasReleasedTheseObjects") {
		m_services->ClientHasReleasedTheseObjects(notify->boundID.c_str());
	} else {
		_log(CLIENT__ERROR, "Unhandled notification from %s: %s", GetName(), notify->boundID.c_str());
	}
	
	_CheckSessionChange();	//just for good measure...
}

void Client::_SendCallReturn(PyPacket *req, PyRepTuple **return_value, const char *channel) {
	
	//build the packet:
	PyPacket *p = new PyPacket();
	p->type_string = "macho.CallRsp";
	p->type = CALL_RSP;
	
	p->source = req->dest;

	p->dest.type = PyAddress::Client;
	p->dest.typeID = HackFixedClientID;
	p->dest.callID = req->source.callID;

	p->userid = m_accountID;
	
	p->payload = *return_value;
	*return_value = NULL;	//consumed
	
	if(channel != NULL) {
		p->named_payload = new PyRepDict();
		p->named_payload->add("channel", new PyRepString(channel));
	}

	FastQueuePacket(&p);
}

void Client::_SendException(PyPacket *req, MACHONETERR_TYPE type, PyRep **payload) {
	//build the packet:
	PyPacket *p = new PyPacket();
	p->type_string = "macho.ErrorResponse";
	p->type = ERRORRESPONSE;
	
	p->source = req->dest;

	p->dest.type = PyAddress::Client;
	p->dest.typeID = HackFixedClientID;
	p->dest.callID = req->source.callID;

	p->userid = m_accountID;
	
	macho_MachoException e;
	e.in_response_to = req->type;
	e.exception_type = type;
	e.payload = *payload;
	*payload = NULL;	//consumed

	p->payload = e.Encode();
	FastQueuePacket(&p);
}

//these are specialized Queue functions when our caller can
//easily provide us with our own copy of the data.
void Client::QueueDestinyUpdate(PyRepTuple **du) {
	m_destinyUpdateQueue.push_back(*du);
	*du = NULL;
}

void Client::QueueDestinyEvent(PyRepTuple **multiEvent) {
	m_destinyEventQueue.push_back(*multiEvent);
	*multiEvent = NULL;
}

void Client::ProcessDestiny(uint32 stamp) {
	if(m_destiny != NULL) {
		//send updates from last tic before changing our stamp and processing our activity.
		_SendQueuedUpdates(m_destiny->GetLastDestinyStamp());
	}
	DynamicSystemEntity::ProcessDestiny(stamp);
}

void Client::_SendQueuedUpdates(uint32 stamp) {
	std::vector<PyRepTuple *>::const_iterator cur, end;
	
	if(m_destinyUpdateQueue.empty()) {
		//no destiny stuff to do...
		if(m_destinyEventQueue.empty()) {
			//no multi-events either...
			return;
		}
		//so, we have multi-event stuff, but no destiny stuff to send.
		//send it out as an OnMultiEvent instead.
		Notify_OnMultiEvent nom;
		nom.events.items = m_destinyEventQueue;
		m_destinyEventQueue.clear();
		PyRepTuple *tmp = nom.FastEncode();	//this is consumed below
		tmp->Dump(DESTINY__UPDATES, "");
		SendNotification("OnMultiEvent", "charid", &tmp);
		return;
	}
	
	DoDestinyUpdateMain dum;

	//first encode the destiny updates.
	cur = m_destinyUpdateQueue.begin();
	end = m_destinyUpdateQueue.end();
	for(; cur != end; cur++) {
		DoDestinyAction act;	//inside the loop just in case memory management techniques ever chance.
		act.update_id = stamp;
		act.update = *cur;
		dum.updates.add( act.FastEncode() );
	}
	m_destinyUpdateQueue.clear();
	
	//right now, we never wait. I am sure they do this for a reason, but
	//I havent found it yet
	dum.waitForBubble = false;

	//encode any multi-events which go along with it.
	dum.events.items = m_destinyEventQueue;
	m_destinyEventQueue.clear();

	PyRepTuple *tmp = dum.FastEncode();
	tmp->Dump(DESTINY__UPDATES, "");
	SendNotification("DoDestinyUpdate", "clientID", &tmp);
}

void Client::SendNotification(const char *notifyType, const char *idType, PyRepTuple **payload, bool seq) {
	
	//build a little notification out of it.
	EVENotificationStream notify;
	notify.remoteObject = 1;
	notify.args = *payload;
	*payload = NULL;	//consumed

	//now sent it to the client
	PyAddress dest;
	dest.type = PyAddress::Broadcast;
	dest.service = notifyType;
	dest.bcast_idtype = idType;
	SendNotification(dest, &notify, seq);
}


void Client::SendNotification(const PyAddress &dest, EVENotificationStream *noti, bool seq) {
	
	//build the packet:
	PyPacket *p = new PyPacket();
	p->type_string = "macho.Notification";
	p->type = NOTIFICATION;
	
	p->source.type = PyAddress::Node;
	p->source.typeID = 111555;
	
	p->dest = dest;
	
	p->userid = m_accountID;
	
	p->payload = noti->Encode();

	if(seq) {
		p->named_payload = new PyRepDict();
		p->named_payload->add("sn", new PyRepInteger(m_nextNotifySequence));
		m_nextNotifySequence++;
	}

	_log(CLIENT__NOTIFY_DUMP, "Sending notify of type %s with ID type %s", dest.service.c_str(), dest.bcast_idtype.c_str());
	if(is_log_enabled(CLIENT__NOTIFY_REP)) {
		PyLogsysDump dumper(CLIENT__NOTIFY_REP, CLIENT__NOTIFY_REP, true, true);
		p->Dump(CLIENT__NOTIFY_REP, &dumper);
	}
	
	FastQueuePacket(&p);
}

//dogmaIM
/*void Client::Handle_MachoBindObject(PyPacket *packet, PyCallStream *call) {
	//takes ((stationID, u1_2=15 from above), (command, tuple, dict))

	call->args->Dump(stdout, "    ");

	if(call->args->items.size() != 2) {
		_log(CLIENT__ERROR, "Invalid arg tuple size in MachoBindObject: %d", call->args->items.size());
		return;
	}
	
	if(call->args->items[0]->CheckType(PyRep::Integer)) {
		_log(CLIENT__MESSAGE, "Hacking integer-only bind argument");
	
		PyRepTuple *t = new PyRepTuple(1);
		PyRepSubStream *ss = new PyRepSubStream();
		t->items[0] = ss;
	
		PyRepTuple *robjs = new PyRepTuple(2);
		ss->decoded = robjs;
	
		PyRepTuple *objt;
		
		objt = new PyRepTuple(2);
		objt->items[0] = new PyRepString("N=126774:147");
		objt->items[1] = new PyRepInteger(127943660267991372LL);
		robjs->items[0] = new PyRepSubStruct(new PyRepSubStream(objt));

		robjs->items[1] = new PyRepNone();
	
		_SendCallReturn(packet, &t);
		
		
		return;
	}
	
	_log(CLIENT__ERROR, "Unknown MachoBindObject!");
}*/


void Client::Handle_GetInventoryFromId(PyPacket *packet, PyCallStream *call) {
	_log(CLIENT__MESSAGE, "got GetInventoryFromId:");
	call->arg_tuple->Dump(stdout, "    ");

	
	//returns a list of remote objects?

	PyRepTuple *t = new PyRepTuple(1);
	PyRepSubStream *ss = new PyRepSubStream();
	t->items[0] = ss;

	PyRepTuple *objt;
	objt = new PyRepTuple(2);
	objt->items[0] = new PyRepString("N=126774:80367");
	objt->items[1] = new PyRepInteger(127943660215796209LL);
	ss->decoded = new PyRepSubStruct(new PyRepSubStream(objt));

	_SendCallReturn(packet, &t);
}

void Client::SendInitialDestinySetstate() {
	if(m_destiny == NULL)
		return;
	
	//m_warpActive = as_warp;
	m_destiny->SendSetState(Bubble());
	//m_warpActive = false;
}

void Client::WarpTo(const GPoint &to) {
	if(m_moveState != msIdle || m_moveTimer.Enabled()) {
		_log(CLIENT__ERROR, "%s: WarpTo called when a move is already pending. Ignoring.", GetName());
		return;
	}
	
	m_movePoint = to;
	_postMove(msWarp);
}

void Client::_ExecuteWarp() {
	m_destiny->WarpTo(m_movePoint, 15000.0);
	//TODO: OnModuleAttributeChange with attribute 18 for capacitory charge
}


void Client::StargateJump(uint32 fromGate, uint32 toGate) {
	if(m_moveState != msIdle || m_moveTimer.Enabled()) {
		_log(CLIENT__ERROR, "%s: StargateJump called when a move is already pending. Ignoring.", GetName());
		return;
	}

	//TODO: verify that they are actually close to 'fromGate'
	//TODO: verify that 'fromGate' actually jumps to 'toGate'
	
	uint32 regionID;
	uint32 constellationID;
	uint32 solarSystemID;
	GPoint location;
	if(!m_services->GetServiceDB()->GetStaticLocation(
		toGate,
		regionID, constellationID, solarSystemID,
		location
		)
	) {
		codelog(CLIENT__ERROR, "%s: Failed to query information for stargate %lu", GetName(), toGate);
		return;
	}
	
	m_moveSystemID = solarSystemID;
	m_movePoint = location;
	m_movePoint.x -= 15000;
	_postMove(msJump, 200);
}

void Client::_ExecuteJump() {
	if(m_destiny == NULL)
		return;

	m_destiny->SendJumpOut();
	
	_postMove(msJump2, 800);
}

void Client::_ExecuteJump_Phase2() {
	
	MoveToLocation(m_moveSystemID, m_movePoint);
	
	_postMove(msJump3, 1500);
}

void Client::_ExecuteJump_Phase3() {
	if(m_destiny == NULL)
		return;

	//NOTE: this should actually be called on the gate we used, not
	//on ourself... but I think it still kinda works.
	m_destiny->SendGateActivity();
	
	m_destiny->SendSetState(Bubble());
}

void Client::_postMove(_MoveState type, uint32 wait_ms) {
	m_moveState = type;
	m_moveTimer.Start(wait_ms);
}

void Client::InitialEnterGame() {
	//create their channel:
	m_services->lsc_service->CreateChannel(LSCChannelDesc(GetCharacterID()));

	m_services->GetServiceDB()->
		LoadCorporationMemberInfo(GetCharacterID(), m_corpstate);

	session.Set_corprole(m_corpstate.corprole);
	session.Set_rolesAtAll(m_corpstate.rolesAtAll);
	session.Set_rolesAtBase(m_corpstate.rolesAtBase);
	session.Set_rolesAtHQ(m_corpstate.rolesAtHQ);
	session.Set_rolesAtOther(m_corpstate.rolesAtOther);

	//ensure that these changes takes effect immediately... because sometimes they doesn't... :(
	_CheckSessionChange();

	if(IsStation(GetLocationID())) {
		_postMove(msLogIntoStation, 10000);
	} else {
		_postMove(msLogIntoSpace, 4000);
	}
}

void Client::UndockingIntoSpace() {
	_postMove(msUndockIntoSpace, 1000);
}

void Client::_SendInitialSkillTraining() {
	InventoryItem *skill = Item()->FindFirstByFlag(flagSkill, false);
	if(skill != NULL) {
		NotifyOnSkillStartTraining osst;
		osst.itemID = skill->itemID();
		osst.endOfTraining = Win32TimeNow() + Win32Time_Month; //hack hack hack
		
		PyRepTuple *tmp = osst.FastEncode();	//this is consumed below
		SendNotification("OnSkillStartTraining", "charid", &tmp);
	}
}

void Client::CheckLogIntoSpace() {
	if(m_moveState == msLogIntoSpace) {
		m_moveTimer.Disable();
		m_moveState = msIdle;
		_ExecuteLogIntoSpace();
	}
}

void Client::_ExecuteLogIntoSpace() {
	SendInitialDestinySetstate();
	
	//this may be a little early...
	_SendInitialSkillTraining();
}

void Client::_ExecuteLogIntoStation() {
	_SendInitialSkillTraining();
}

void Client::_ExecuteUndockIntoSpace() {
	SendInitialDestinySetstate();
}

bool Client::AddBalance(double amount) {
	if(amount == 0)
		return(true);
	
	double result = m_char.balance + amount;
	
	//remember, this can take a negative amount...
	if(result < 0) {
		return(false);
	}
	
	m_char.balance = result;
	
	if(!m_services->GetServiceDB()->SetCharacterBalance(GetCharacterID(), m_char.balance))
		return(false);
	
	//send notification of change
	OnAccountChange ac;
	ac.accountKey = "cash";
	ac.ownerid = GetCharacterID();
	ac.balance = m_char.balance;
	PyRepTuple *answer = ac.Encode();
	SendNotification("OnAccountChange", "cash", &answer, false);

	return(true);
}



bool Client::LoadInventory(uint32 ship_id) {
	if(ship_id == 0) {
		_log(CLIENT__ERROR, "%s: Character %d is not in a ship (or capsule)... not supported", GetName(), GetCharacterID());
		return(false);
	}
	
	//first, recursively load our ship item, this will also load ourself.
	m_ship = m_services->item_factory->Load(ship_id, true);
	if(m_ship == NULL) {
		_log(CLIENT__ERROR, "Failed to load ship item (%d) for char %s", ship_id, GetName());
		return(false);
	}
	
	//then, find the pilot of our ship, that should be us...
	InventoryItem *self = m_ship->FindFirstByFlag(flagPilot, true);
	if(self == NULL) {
		_log(CLIENT__ERROR, "Failed to find pilot (self) for ship (%d) for char %s", GetShipID(), GetName());
		return(false);
	}
	_SetSelf(self);
	
	//make sure its us...
	if(m_self->itemID() != GetCharacterID()) {
		_log(CLIENT__ERROR, "%s: Pilot of ship %d is not us! (pilot is %d, we are %d)", GetName(), GetShipID(), m_self->itemID(), GetCharacterID());
		return(false);
	}
	
	modules.UpdateModules();
	
	return(true);
}

uint32 Client::GetShipID() const {
	if(m_ship == NULL)
		return(0);
	return(m_ship->itemID());
}

double Client::GetMass() const {
	if(m_ship == NULL)
		return(0);
	return(m_ship->mass());
}

double Client::GetMaxVelocity() const {
	if(m_ship == NULL)
		return(0);
	return(m_ship->maxVelocity());
}

double Client::GetAgility() const {
	if(m_ship == NULL)
		return(0);
	return(m_ship->agility());
}

double Client::GetPropulsionStrength() const {
	if(m_ship == NULL)
		return(3.0f);
	//just making shit up, I think skills modify this, as newbies
	//tend to end up with 3.038 instead of the base 3.0 on their ship..
	double res;
	res =  m_ship->propulsionFusionStrength();
	res += m_ship->propulsionIonStrength();
	res += m_ship->propulsionMagpulseStrength();
	res += m_ship->propulsionPlasmaStrength();
	res += m_ship->propulsionFusionStrengthBonus();
	res += m_ship->propulsionIonStrengthBonus();
	res += m_ship->propulsionMagpulseStrengthBonus();
	res += m_ship->propulsionPlasmaStrengthBonus();
	res += 0.038f;
	return(res);
}

void Client::TargetAdded(SystemEntity *who) {

	/*send a destiny update with:
		destiny: OnDamageStateChange
		OnMultiEvent: OnTarget add*/
	//then maybe OnMultiEvent: OnTarget try? for auto target?

	//this is pretty weak
	if(m_destiny != NULL) {
		m_destiny->DoTargetAdded(who);
	}
}

void Client::TargetLost(SystemEntity *who) {
	//OnMultiEvent: OnTarget lost
    Notify_OnTarget te;
    te.mode = "lost";
	te.targetID = who->GetID();

	Notify_OnMultiEvent multi;
    multi.events.add(te.FastEncode());
	
	PyRepTuple *tmp = multi.FastEncode();	//this is consumed below
	SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::TargetedAdd(SystemEntity *who) {
	//OnMultiEvent: OnTarget otheradd
    Notify_OnTarget te;
    te.mode = "otheradd";
	te.targetID = who->GetID();

	Notify_OnMultiEvent multi;
    multi.events.add(te.FastEncode());
	
	PyRepTuple *tmp = multi.FastEncode();	//this is consumed below
	SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::TargetedLost(SystemEntity *who) {
	//OnMultiEvent: OnTarget otherlost
    Notify_OnTarget te;
    te.mode = "otherlost";
	te.targetID = who->GetID();

	Notify_OnMultiEvent multi;
    multi.events.add(te.FastEncode());
	
	PyRepTuple *tmp = multi.FastEncode();	//this is consumed below
	SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::TargetsCleared() {
	//OnMultiEvent: OnTarget clear
    Notify_OnTarget te;
    te.mode = "clear";
	te.targetID = 0;

	Notify_OnMultiEvent multi;
    multi.events.add(te.FastEncode());
	
	PyRepTuple *tmp = multi.FastEncode();	//this is consumed below
	SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::SavePosition() {
	if(m_ship == NULL || m_destiny == NULL) {
		_log(CLIENT__TRACE, "%s: Unable to save position. We are prolly not in space.", GetName());
		return;
	}
	m_ship->Relocate( m_destiny->GetPosition() );
}

bool Client::LaunchDrone(InventoryItem *drone) {
#if 0
drop 166328265

OnItemChange ,*args= (Row(itemID: 166328265,typeID: 15508,
	ownerID: 105651216,locationID: 166363674,flag: 87,
	contraband: 0,singleton: 1,quantity: 1,groupID: 100,
	categoryID: 18,customInfo: (166363674, None)), 
	{10: None}) ,**kw= {}

OnItemChange ,*args= (Row(itemID: 166328265,typeID: 15508,
	ownerID: 105651216,locationID: 30001369,flag: 0,
	contraband: 0,singleton: 1,quantity: 1,groupID: 100,
	categoryID: 18,customInfo: (166363674, None)), 
	{3: 166363674, 4: 87}) ,**kw= {}
	
returns list of deployed drones.

internaly scatters:
	OnItemLaunch ,*args= ([166328265], [166328265])

DoDestinyUpdate ,*args= ([(31759, 
	('OnDroneStateChange', 
		[166328265, 105651216, 166363674, 0, 15508, 105651216])
	), (31759, 
	('OnDamageStateChange', 
		(2100212375, [(1.0, 400000.0, 127997215757036940L), 1.0, 1.0]))
	), (31759, ('AddBalls',
		...
	True

DoDestinyUpdate ,*args= ([(31759, 
	('Orbit', (166328265, 166363674, 750.0))
	), (31759, 
	('SetSpeedFraction', (166328265, 0.265625)))], 
	False) ,**kw= {} )
#endif

	if(!IsSolarSystem(GetLocationID())) {
		_log(SERVICE__ERROR, "%s: Trying to launch drone when not in space!", GetName());
		return(false);
	}

	_log(CLIENT__MESSAGE, "%s: Launching drone %lu", GetName(), drone->itemID());
	
	//first, the item gets moved into space
	//TODO: set customInfo to a tuple: (shipID, None)
	drone->Move(GetSystemID(), flagAutoFit);
//temp for testing:
drone->Move(GetShipID(), flagDroneBay, false);

	//now we create an NPC to represent it.
	GPoint position(GetPosition());
	position.x += 750.0f;	//total crap.
	
	//this adds itself into the system.
	NPC *drone_npc = new NPC(
		m_system,
		m_services,
		drone, 
		GetCorporationID(),
		position);
	m_system->AddNPC(drone_npc);

	//now we tell the client that "its ALIIIIIVE"
	//DoDestinyUpdate:

	//drone_npc->Destiny()->SendAddBall();
	
    /*PyRepList *actions = new PyRepList();
	DoDestinyAction act;
	act.update_id = NextDestinyUpdateID();	//update ID?
    */
	// 	OnDroneStateChange
/*  {
		DoDestiny_OnDroneStateChange du;
		du.droneID = drone->itemID();
		du.ownerID = GetCharacterID();
		du.controllerID = GetShipID();
		du.activityState = 0;
		du.droneTypeID = drone->typeID();
		du.controllerOwnerID = Ship()->ownerID();
		act.update = du.FastEncode();
		actions->add( act.FastEncode() );
    }*/
	
	// 	AddBall
    /*{
		DoDestiny_AddBall addball;
		drone_npc->MakeAddBall(addball, act.update_id);
		act.update = addball.FastEncode();
		actions->add( act.FastEncode() );
    }*/
	
	// 	Orbit
/*	{
		DoDestiny_Orbit du;
		du.entityID = drone->itemID();
		du.orbitEntityID = GetCharacterID();
		du.distance = 750;
		act.update = du.FastEncode();
		actions->add( act.FastEncode() );
	}
	
	// 	SetSpeedFraction
	{
		DoDestiny_SetSpeedFraction du;
		du.entityID = drone->itemID();
		du.fraction = 0.265625f;
		act.update = du.FastEncode();
		actions->add( act.FastEncode() );
	}*/

//testing:
/*	{
		DoDestiny_SetBallInteractive du;
		du.entityID = drone->itemID();
		du.interactive = 1;
		act.update = du.FastEncode();
		actions->add( act.FastEncode() );
	}
	{
		DoDestiny_SetBallInteractive du;
		du.entityID = GetCharacterID();
		du.interactive = 1;
		act.update = du.FastEncode();
		actions->add( act.FastEncode() );
	}*/
	
	//NOTE: we really want to broadcast this...
	//TODO: delay this until after the return.
	//_SendDestinyUpdate(&actions, false);

	return(false);
}

//assumes that the backend DB stuff was already done.
void Client::JoinCorporationUpdate(uint32 corp_id) {
	m_char.corporationID = corp_id;
	
	//TODO: recursively change corp on all our items.
	
	m_services->GetServiceDB()->LoadCorporationMemberInfo(GetCharacterID(), m_corpstate);
	
	session.Set_corpid(corp_id);
	session.Set_corprole(m_corpstate.corprole);
	session.Set_rolesAtAll(m_corpstate.rolesAtAll);
	session.Set_rolesAtBase(m_corpstate.rolesAtBase);
	session.Set_rolesAtHQ(m_corpstate.rolesAtHQ);
	session.Set_rolesAtOther(m_corpstate.rolesAtOther);

	//logs indicate that we need to push this update out asap.
	_CheckSessionChange();
}

FunctorTimerQueue::TimerID Client::Delay( uint32 time_in_ms, void (Client::* clientCall)() ) {
	Functor *f = new SimpleClientFunctor(this, clientCall);
	return(m_delayQueue.Schedule( &f, time_in_ms ));
}

FunctorTimerQueue::TimerID Client::Delay( uint32 time_in_ms, ClientFunctor **functor ) {
	Functor *f = *functor;
	*functor = NULL;
	return(m_delayQueue.Schedule( &f, time_in_ms ));
}




FunctorTimerQueue::FunctorTimerQueue()
: m_nextID(0)
{
}

FunctorTimerQueue::~FunctorTimerQueue() {
	std::vector<Entry *>::iterator cur, end;
	cur = m_queue.begin();
	end = m_queue.end();
	for(; cur != end; cur++) {
		delete *cur;
	}
}

FunctorTimerQueue::TimerID FunctorTimerQueue::Schedule(Functor **what, uint32 in_how_many_ms) {
	Entry *e = new Entry(++m_nextID, *what, in_how_many_ms);
	*what = NULL;
	m_queue.push_back(e);
	return(e->id);
}

bool FunctorTimerQueue::Cancel(TimerID id) {
	std::vector<Entry *>::iterator cur, end;
	cur = m_queue.begin();
	end = m_queue.end();
	for(; cur != end; cur++) {
		if((*cur)->id == id) {
			m_queue.erase(cur);
			return(true);
		}
	}
	return(false);
}

void FunctorTimerQueue::Process() {
	std::vector<Entry *>::iterator cur, tmp;
	cur = m_queue.begin();
	while(cur != m_queue.end()) {
		Entry *e = *cur;
		if(e->when.Check(false)) {
			//call the functor.
			(*e->func)();
			//we are done with this crap.
			delete e;
			cur = m_queue.erase(cur);
		} else {
			cur++;
		}
	}
}

FunctorTimerQueue::Entry::Entry(TimerID _id, Functor *_func, uint32 time_ms)
: id(_id),
  func(_func),
  when(time_ms)
{
	when.Start();
}

FunctorTimerQueue::Entry::~Entry() {
	delete func;
}






















