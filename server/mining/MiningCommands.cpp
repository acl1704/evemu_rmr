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

#include "../admin/AllCommands.h"
#include "../PyServiceMgr.h"
#include "../inventory/InventoryItem.h"
#include "../inventory/ItemFactory.h"
#include "../system/SystemManager.h"
#include "Asteroid.h"
//#include "../NPC.h"
//#include "../ship/DestinyManager.h"

//#include "../packets/Inventory.h"
//#include "../packets/General.h"


void Command_roid(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	_log(COMMAND__MESSAGE, "Roid %s", args.arg[1]);
	if(!args.IsNumber(1)) {
		who->SendErrorMsg("argument 1 should be an item type ID");
		return;
	}
	if(!args.IsNumber(2)) {
		who->SendErrorMsg("argument 2 should be a radius");
		return;
	}
	double radius = atof(args.arg[2]);
	if(radius <= 0) {
		who->SendErrorMsg("Invalid radius.");
		return;
	}

	if(who->IsInSpace() == false) {
		who->SendErrorMsg("You must be in space to spawn things.");
		return;
	}

	GPoint position(who->GetPosition());
	position.x += radius + 1 + who->GetRadius();	//put it raw enough away to not push us around.
	
	InventoryItem *i;
	//TODO: make item in IsUniverseAsteroid() range...
	i = services->item_factory->SpawnSingleton(
		atoi(args.arg[1]),
		500021,	//who->GetCorporationID(),	//owner
		who->GetLocationID(),
		flagAutoFit,
		NULL,	//name
		position
		);
	if(i == NULL) {
		who->SendErrorMsg("Unable to spawn item of type %s.", args.arg[1]);
		return;
	}

	i->Set_radius(radius);
	i->Save();

	SystemManager *sys = who->GetSystem();
	Asteroid *new_roid = new Asteroid(sys, i);	//takes a ref.
	//TODO: check for a local asteroid belt object?
	//TODO: actually add this to the asteroid belt too...
	sys->AddEntity(new_roid);
}

void Command_growbelt(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
}














