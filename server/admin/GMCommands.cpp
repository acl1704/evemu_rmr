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

#include "AllCommands.h"
#include "../PyServiceMgr.h"
#include "../inventory/InventoryItem.h"
#include "../inventory/ItemFactory.h"
#include "../NPC.h"
#include "../ship/DestinyManager.h"
#include "../system/SystemManager.h"

#include "../packets/Inventory.h"
#include "../packets/General.h"


void Command_summon(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	int arg2 = 1;
	
	_log(COMMAND__MESSAGE, "Summon %s %s times", args.arg[1], args.arg[2]);
	if(!args.IsNumber(1)) {
		who->SendErrorMsg("argument 1 should be an item ID");
		return;
	}

	if(args.IsNumber(2)) {
		arg2 = atoi(args.arg[2]);
	}
		
	

	//summon into their cargo hold unless they are docked in a station,
	//then stick it in their hangar instead.
	EVEItemFlags flag = flagCargoHold;
	uint32 locationID = who->GetShipID();
	if(IsStation(who->GetLocationID())) {
		flag = flagHangar;
		locationID = who->GetStationID();
	}
	
	InventoryItem *i;
	i = services->item_factory->Spawn(
		atoi(args.arg[1]),
		arg2,
		who->GetCharacterID(),
		0,	//temp location.
		flag
		);
	if(i == NULL) {
		who->SendErrorMsg("Unable to spawn item of type %s.", args.arg[1]);
		return;
	}

	//Move to location
	i->Move( locationID, flag, true );
	//we dont need our reference anymore...
	i->Release();
	
}

void Command_search(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	//an empty query is a bad idea.
	if(args.argplus[1][0] == '\0') {
		who->SelfChatMessage("Usage: /search [text]");
		return;
	}
	
	
	std::map<uint32, std::string> matches;
	if(!db->ItemSearch(args.argplus[1], matches)) {
		who->SelfChatMessage("Failed to query DB.");
		return;
	}
	if(matches.empty()) {
		who->SelfChatMessage("Nothing found.");
		return;
	}

	std::string result;
	char buf[16];

	snprintf(buf, sizeof(buf), "%d", matches.size());
	result = buf;
	result += " matches found.<br>";
	
	std::map<uint32, std::string>::iterator cur, end;
	cur = matches.begin();
	end = matches.end();
	for(; cur != end; cur++) {
		snprintf(buf, sizeof(buf), "%lu: ", cur->first);
		result += buf;
		result += cur->second;
		result += "<br>";
	}
	
	if(matches.size() > 10) {
		//send the results in an evemail.
		who->SelfChatMessage("%d results found, sent in an evemail.", matches.size());
		std::string subject("Search results for ");
		subject += args.argplus[1];
		who->SelfEveMail(subject.c_str(), "%s", result.c_str());
		return;
	}
	
	who->SelfChatMessage("%s", result.c_str());
}

void Command_translocate(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	_log(COMMAND__MESSAGE, "Translocate to %s", args.arg[1]);
	if(!args.IsNumber(1)) {
		who->SendErrorMsg("argument 1 should be an entity ID");
		return;
	}
	uint32 loc = atoi(args.arg[1]);
	
	GPoint p(0.0f, 1000000.0f, 0.0f);	//when going to a system, who knows where to stick them... could find a stargate and stick them near it I guess...
	
	if(!IsStation(loc) && !IsSolarSystem(loc)) {
		Client *tgt;
		tgt = services->entity_list->FindCharacter(loc);
		if(tgt == NULL)
			tgt = services->entity_list->FindByShip(loc);
		if(tgt == NULL) {
			who->SendErrorMsg("Unable to find location %d", loc);
			return;
		}
		loc = tgt->GetLocationID();
		p = tgt->GetPosition();
	}

	who->MoveToLocation(loc, p);
}


void Command_tr(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(std::string("me") != args.arg[1]) {
		_log(COMMAND__ERROR, "Translocate (/TR) to non-me who '%s' is not supported yet.", args.arg[1]);
		return;
	}
	
	_log(COMMAND__MESSAGE, "Translocate to %s", args.arg[2]);
	if(!args.IsNumber(2)) {
		who->SendErrorMsg("argument 1 should be an entity ID");
		return;
	}
	uint32 loc = atoi(args.arg[2]);
	
	GPoint p(0.0f, 1000000.0f, 0.0f);	//when going to a system, who knows where to stick them... could find a stargate and stick them near it I guess...
	
	if(!IsStation(loc) && !IsSolarSystem(loc)) {
		Client *tgt;
		tgt = services->entity_list->FindCharacter(loc);
		if(tgt == NULL)
			tgt = services->entity_list->FindByShip(loc);
		if(tgt == NULL) {
			who->SendErrorMsg("Unable to find location %d", loc);
			return;
		}
		loc = tgt->GetLocationID();
		p = tgt->GetPosition();
	}

	who->MoveToLocation(loc, p);
}

void Command_giveisk(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(!args.IsNumber(1)) {
		who->SendErrorMsg("argument 1 should be an entity ID (0=self)");
		return;
	}
	if(!args.IsNumber(2)) {
		who->SendErrorMsg("argument 2 should be an amount of ISK");
		return;
	}
	uint32 entity = atoi(args.arg[1]);
	double amount = strtod(args.arg[2], NULL);
	
	Client *tgt;
	if(entity == 0) {
		tgt = who;
	} else {
		tgt = services->entity_list->FindCharacter(entity);
		if(tgt == NULL) {
			who->SendErrorMsg("Unable to find character %lu", entity);
			return;
		}
	}
	
	tgt->AddBalance(amount);
}

void Command_pop(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(args.argnum != 3) {
		who->SendErrorMsg("Bad arguments.");
		return;
	}

	//CustomNotify: notify
	//ServerMessage: msg
	//CustomError: error
	
	Notify_OnRemoteMessage n;
	n.msgType = args.arg[1];
	n.args[ args.arg[2] ] = new PyRepString(args.argplus[3]);
	PyRepTuple *tmp = n.FastEncode();
	
	who->SendNotification("OnRemoteMessage", "charid", &tmp);
}


void Command_goto(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(args.argnum != 3 || !args.IsNumber(1) || !args.IsNumber(2) || !args.IsNumber(3)) {
		who->SendErrorMsg("arguments must be 3 numbers");
		return;
	}
	GPoint p(	atof(args.arg[1]),
				atof(args.arg[2]),
				atof(args.arg[3]) );
	
	_log(COMMAND__MESSAGE, "%s: Goto (%.13f, %.13f, %.13f)", who->GetName(), p.x, p.y, p.z);
	
	who->MoveToPosition(p);
}

void Command_spawn(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	_log(COMMAND__MESSAGE, "Spawn %s", args.arg[1]);
	if(!args.IsNumber(1)) {
		who->SendErrorMsg("argument 1 should be an item type ID");
		return;
	}

	if(who->IsInSpace() == false) {
		who->SendErrorMsg("You must be in space to spawn things.");
		return;
	}
	
	InventoryItem *i;
	i = services->item_factory->SpawnSingleton(
		atoi(args.arg[1]),
		who->GetCorporationID(),	//owner
		who->GetLocationID(),
		flagAutoFit
		);
	if(i == NULL) {
		who->SendErrorMsg("Unable to spawn item of type %s.", args.arg[1]);
		return;
	}

	//hackin it...
	GPoint loc(who->GetPosition());
	loc.x += 1500;

	SystemManager *sys = who->GetSystem();
	NPC *it = new NPC(sys, services, i, who->GetCorporationID(), loc);
	sys->AddNPC(it);
}















