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
#include "ModuleManager.h"

#include "../common/EVEUtils.h"
#include "../Client.h"
#include "../inventory/InventoryItem.h"
#include "../system/Damage.h"
#include "../system/SystemEntity.h"
#include "../ship/DestinyManager.h"

#include "../packets/DogmaIM.h"
#include "../packets/Destiny.h"

ModuleManager::ModuleManager(Client *pilot)
: m_pilot(pilot)
{
	//DO NOT access m_pilot here!
	memset(m_modules, 0, sizeof(m_modules));
}

ModuleManager::~ModuleManager() {
	uint8 r;
	for(r = 0; r < MAX_MODULE_COUNT; r++) {
		delete m_modules[r];
	}
}

void ModuleManager::Process() {
	uint8 r;
	ShipModule **cur = m_modules;
	for(r = 0; r < MAX_MODULE_COUNT; r++, cur++) {
		if(*cur == NULL)
			continue;
		(*cur)->Process();
	}
}

void ModuleManager::UpdateModules() {
	//I am not sure if I like this model of pulling the ship directly
	//from the client..
	InventoryItem *ship = m_pilot->Ship();
	
	_log(SHIP__MODULE_TRACE, "%s: Refreshing modules for ship %s (%lu).", m_pilot->GetName(), ship->itemName().c_str(), ship->itemID());
	
	//TODO: iterate through all slots and fill in m_modules
	//checking for inconsistencies as to not re-create 
	// module entries which already exist.
	m_moduleByID.clear();
	
	uint8 slot;
	for(slot = 0; slot < MAX_MODULE_COUNT; slot++) {
		EVEItemFlags flag = SlotToFlag(slot);
		
		//apparently they thought it was a good idea to put the module AND its 
		// charges in the module slot of the main ship, instead of storing the
		// charges inside the module itself. This makes our life much more
		// difficult because we need to figure out what is what.
		std::vector<InventoryItem *> items;
		if(ship->FindByFlag(flag, items, false) == 0) {
			delete m_modules[slot];
			m_modules[slot] = NULL;
			continue;	//nothing in that slot..
		}

		//ok, we have the list of crap in this slot, figure out what is the actual module.
		InventoryItem *module = NULL;
		InventoryItem *charge = NULL;
		if(items.size() == 1) {
			module = items[0]->Ref();
		} else if(items.size() == 2) {
			//could be done better
			bool valid = true;
			if(items[0]->categoryID() == EVEDB::invCategories::Module) {
				if(items[1]->categoryID() == EVEDB::invCategories::Charge) {
					module = items[0]->Ref();
					charge = items[1]->Ref();
				} else {
					_log(SHIP__ERROR, "%s: Unexpected item category %d in slot %d (wanted charge)", m_pilot->GetName(), items[1]->categoryID(), flag);
					valid = false;
				}
			} else if(items[0]->categoryID() == EVEDB::invCategories::Charge) {
				if(items[1]->categoryID() == EVEDB::invCategories::Module) {
					module = items[1]->Ref();
					charge = items[0]->Ref();
				} else {
					_log(SHIP__ERROR, "%s: Unexpected item category %d in slot %d (wanted module)", m_pilot->GetName(), items[1]->categoryID(), flag);
					valid = false;
				}
			} else {
				_log(SHIP__ERROR, "%s: Unexpected item category %d in slot %d (wanted module or charge)", m_pilot->GetName(), items[0]->categoryID(), flag);
				valid = false;
			}
			if(!valid) {
				delete m_modules[slot];
				m_modules[slot] = NULL;
				continue;
			}
		} else {
			//no idea why this would happen.. but it may be possible..
			_log(SHIP__ERROR, "%s: More than two items in a module slot (%d). We do not understand this yet!", m_pilot->GetName(), flag);
			//ignore for now...
			delete m_modules[slot];
			m_modules[slot] = NULL;
			continue;
		}
		
		if(m_modules[slot] != NULL) {
			if(m_modules[slot]->item() == module) {
				if(m_modules[slot]->charge() == charge) {
					_log(SHIP__MODULE_TRACE, "%s: Item %s (%lu) is still in slot %d.", m_pilot->GetName(), module->itemName().c_str(), module->itemID(), slot);
				} else {
					_log(SHIP__MODULE_TRACE, "%s: Item %s (%lu) is still in slot %d, but has a new charge item %s (%lu).", m_pilot->GetName(), module->itemName().c_str(), module->itemID(), slot, (charge==NULL)?"None":charge->itemName().c_str(), (charge==NULL)?0:charge->itemID());
					m_modules[slot]->ChangeCharge(charge);
				}
				//no change...
				m_moduleByID[module->itemID()] = slot;
				continue;
			}
			delete m_modules[slot];
			m_modules[slot] = NULL;
		}
		_log(SHIP__MODULE_TRACE, "%s: Item %s (%lu) is now in slot %d. Using charge %s (%lu)", m_pilot->GetName(), module->itemName().c_str(), module->itemID(), slot, (charge==NULL)?"None":charge->itemName().c_str(), (charge==NULL)?0:charge->itemID());
		m_modules[slot] = ShipModule::CreateModule(m_pilot, module, charge);
		m_moduleByID[module->itemID()] = slot;
	}
}

int ModuleManager::Activate(uint32 itemID, const std::string &effectName, uint32 target, uint32 repeat) {
	std::map<uint32, uint8>::const_iterator res;
	res = m_moduleByID.find(itemID);
	if(res == m_moduleByID.end()) {
		_log(SHIP__ERROR, "%s: failed to activate module %lu. Not found.", m_pilot->GetName(), itemID);
		return(0);
	}
	ShipModule *mod = m_modules[res->second];
	if(mod == NULL) {
		//should never happen.
		codelog(SHIP__ERROR, "%s: failed to activate module %lu. Internal data inconsistency.", m_pilot->GetName(), itemID);
		return(0);
	}
	return(mod->Activate(effectName, target, repeat));
}

void ModuleManager::Deactivate(uint32 itemID, const std::string &effectName) {
	std::map<uint32, uint8>::const_iterator res;
	res = m_moduleByID.find(itemID);
	if(res == m_moduleByID.end()) {
		_log(SHIP__ERROR, "%s: failed to deactivate module %lu. Not found.", m_pilot->GetName(), itemID);
		return;
	}
	ShipModule *mod = m_modules[res->second];
	if(mod == NULL) {
		//should never happen.
		codelog(SHIP__ERROR, "%s: failed to deactivate module %lu. Internal data inconsistency.", m_pilot->GetName(), itemID);
		return;
	}
	mod->Deactivate(effectName);
}

void ModuleManager::ReplaceCharges(EVEItemFlags flag, InventoryItem *new_charge) {
	uint8 slot = FlagToSlot(flag);
	InventoryItem *charge = m_modules[slot]->charge();
	if(charge != NULL) {
		//put the previous charge into the cargo hold.
		charge->ChangeFlag(flagCargoHold);
	}
	
	m_modules[slot]->ChangeCharge(new_charge);
}

uint8 ModuleManager::FlagToSlot(EVEItemFlags flag) {
	return(flag - flagSlotFirst);
}

EVEItemFlags ModuleManager::SlotToFlag(uint8 slot) {
	return((EVEItemFlags)(slot+flagSlotFirst));
}

ShipModule *ShipModule::CreateModule(Client *owner, InventoryItem *self, InventoryItem *charge_) {
	switch(self->groupID()) {
	//TODO: make an enum for this crap, or pull it from the DB.
	case 74:	//Hybrid Weapon
	case 55:	//Projectile Weapon
		return(new HybridWeaponModule(owner, self, charge_));
	case 53:	//Energy Weapon
		return(new LaserWeaponModule(owner, self, charge_));
	case 54:	//Mining Laser
		return(new MiningLaserModule(owner, self, charge_));
	default:
		return(new GenericShipModule(owner, self, charge_));
	}
}

void ShipModule::ChangeCharge(InventoryItem *new_charge) {
	if(m_charge != NULL) {
		m_charge->Release();
	}
	m_charge = new_charge;
	//TODO: handle state transitons? can they swap charges while active???
}

ShipModule::ShipModule(Client *pilot, InventoryItem *self, InventoryItem *charge_)
: m_state(Offline),
  m_pilot(pilot),
  m_item(self),
  m_charge(charge_),
  m_timer(0)
{
	m_timer.Disable();
	if(m_item->isOnline()) {
		m_state = Online;
	}
}

ShipModule::~ShipModule() {
	m_item->Release();
	if(m_charge != NULL)
		m_charge->Release();
}

void ShipModule::Process() {
	switch(m_state) {
	
	case Offline:
		//do nothing.
		break;
	
	case PuttingOnline:
		if(m_timer.Check(false)) {
			_log(SHIP__MODULE_TRACE, "Module %s (%lu): Putting module online.", m_item->itemName().c_str(), m_item->itemID());
			m_state = Online;
			m_item->PutOnline();
		}
		break;
		
	case Online:
		//do nothing.
		break;

	case Active:
	case Deactivating:
		//not supported in this module
		break;
		
	//no default on purpose.
	}
}

int ShipModule::Activate(const std::string &effectName, uint32 target, uint32 repeat) {
	_log(SHIP__MODULE_TRACE, "Module %s (%lu): Activation requested for effect '%s' in state %d.", m_item->itemName().c_str(), m_item->itemID(), effectName.c_str(), m_state);
	if(effectName == "online") {
		if(m_state == Offline) {
			_log(SHIP__MODULE_TRACE, "Module %s (%lu): Activate requested for effect.", m_item->itemName().c_str(), m_item->itemID());
			m_timer.Start(_ActivationInterval());
			m_state = PuttingOnline;
			return(1);
		} else {
			_log(SHIP__MODULE_TRACE, "Module %s (%lu): Activation requested in state %d, ignoring.", m_item->itemName().c_str(), m_item->itemID(), m_state);
			return(0);
		}
	} else {
		_log(SHIP__MODULE_TRACE, "Module %s (%lu): Activation requested in state %d with unknown effect '%s'", m_item->itemName().c_str(), m_item->itemID(), m_state, effectName.c_str());
		return(0);
	}
}

void ShipModule::Deactivate(const std::string &effectName) {
	_log(SHIP__MODULE_TRACE, "Module %s (%lu): Deactivate requested for effect '%s' in state %d.", m_item->itemName().c_str(), m_item->itemID(), effectName.c_str(), m_state);
	if(effectName == "online") {
		if(m_state == Active) {
			_log(SHIP__MODULE_TRACE, "Module %s (%lu): Deactivate requested for effect.", m_item->itemName().c_str(), m_item->itemID());
			m_state = Deactivating;
			//timer is already running.
			m_item->PutOffline();
		} else {
			_log(SHIP__MODULE_TRACE, "Module %s (%lu): Deactivation requested in state %d, ignoring.", m_item->itemName().c_str(), m_item->itemID(), m_state);
		}
	} else {
		_log(SHIP__MODULE_TRACE, "Module %s (%lu): Deactivation requested in state %d with unknown effect '%s'", m_item->itemName().c_str(), m_item->itemID(), m_state, effectName.c_str());
	}
}

uint32 ShipModule::_ActivationInterval() const {
	return(m_item->speed());
}






ActivatableModule::ActivatableModule(const char *effectName, Client *pilot, InventoryItem *self, InventoryItem *charge_)
: ShipModule(pilot, self, charge_),
  m_repeatCount(0),
  m_target(0),
  m_effectName(effectName)
{
}

void ActivatableModule::Process() {
	switch(m_state) {
	case Offline:
	case PuttingOnline:
	case Online:
		//pass up to parent.
		ShipModule::Process();
		break;
		
	case Active:
		if(m_timer.Check()) {
			_log(SHIP__MODULE_TRACE, "Module %s (%lu): Activation timer expired.", m_item->itemName().c_str(), m_item->itemID());
			
			DoEffect();
			
			if(m_repeatCount > 0) {
				m_repeatCount--;
				//let the timer go again.
			} else {
				_log(SHIP__MODULE_TRACE, "Module %s (%lu): No more repeates requested. Deactivating.", m_item->itemName().c_str(), m_item->itemID());
				m_state = Deactivating;
			}
		}
		break;
		
	case Deactivating:
		if(m_timer.Check(false)) {
			_log(SHIP__MODULE_TRACE, "Module %s (%lu): Deactivation complete, module online.", m_item->itemName().c_str(), m_item->itemID());
			m_state = Online;
			StopEffect();	//must send stop effect before clearing target info!
			m_target = 0;
			m_repeatCount = 0;
		}
		break;
		
	//no default on purpose.
	}
}

int ActivatableModule::Activate(const std::string &effectName, uint32 target, uint32 repeat) {
	if(effectName == m_effectName) {
		if(m_state == Online) {
			_log(SHIP__MODULE_TRACE, "Module %s (%lu): Activation requested with %s (tgt=%lu, r=%lu)", m_item->itemName().c_str(), m_item->itemID(), m_effectName, target, repeat);
			m_timer.Start(_ActivationInterval());
			if(repeat > 1) {
				m_state = Active;
				m_repeatCount = repeat;
			} else {
				m_state = Deactivating;
				m_repeatCount = 0;
			}
			m_target = target;
			StartEffect();
			DoEffect();
			return(1);
		} else {
			_log(SHIP__MODULE_TRACE, "Module %s (%lu): Activation requested with %s in state %d. Ignoring..", m_item->itemName().c_str(), m_item->itemID(), m_effectName, m_state);
			return(0);
		}
	} else {
		return(ShipModule::Activate(effectName, target, repeat));
	}
}

void ActivatableModule::Deactivate(const std::string &effectName) {
	if(effectName == m_effectName) {
		if(m_state == Active) {
			_log(SHIP__MODULE_TRACE, "Module %s (%lu): Deactivation requested with %s.", m_item->itemName().c_str(), m_item->itemID(), m_effectName);
			m_state = Deactivating;
			//timer is already running.
		} else {
			_log(SHIP__MODULE_TRACE, "Module %s (%lu): Deactivation requested with %s in state %d, ignoring.", m_item->itemName().c_str(), m_item->itemID(), m_effectName, m_state);
		}
	} else {
		ShipModule::Deactivate(effectName);
	}
}

void ActivatableModule::_SendGodmaShipEffect(EVEEffectID effect, bool active) {
	Notify_OnGodmaShipEffect gse;
	gse.itemID = m_item->itemID();
	gse.effectID = effect;
	gse.when = Win32TimeNow();
	gse.start = active?1:0;
	gse.active = active?1:0;
	gse.env_itemID = m_item->itemID();
	gse.env_charID = m_item->ownerID();	//a little questionable
	gse.env_shipID = m_item->locationID();
	gse.env_target = m_target;
	if(active) {
		PyRepTuple *env_other = new PyRepTuple(3);
		gse.env_other = env_other;
		env_other->items[0] = new PyRepInteger(m_item->locationID());	//ship ID.
		env_other->items[1] = new PyRepInteger(29);	//no idea
		env_other->items[2] = new PyRepInteger(215);	//no idea
	} else {
		gse.env_other = new PyRepNone();
	}
	gse.env_effectID = gse.effectID;
	gse.startTime = gse.when;
	gse.duration = _ActivationInterval();
	gse.repeat = m_repeatCount;
	gse.randomSeed = new PyRepNone();
	gse.error = new PyRepNone();

	//should this only go to ourself?
	PyRepTuple *up = gse.FastEncode();
	m_pilot->QueueDestinyEvent(&up);
	delete up;
}

void ActivatableModule::_SendWeaponEffect(const char *effect, SystemEntity *target) {
	DoDestiny_OnSpecialFX13 sfx;
	sfx.entityID = m_pilot->Ship()->itemID();
	sfx.moduleID = m_item->itemID();
	sfx.moduleTypeID = m_item->typeID();
	sfx.targetID = target->GetID();
	sfx.otherTypeID = 0;	//target->Item()->typeID();
	sfx.effect_type = effect;
	sfx.isOffensive = 1;
	sfx.start = 1;
	sfx.active = 1;
//omit these for now, setting up the repeat might be a network optimization, but we dont need it right now.
	sfx.duration_ms = 1960;	//no idea...
	sfx.repeat = 1;
	sfx.startTime = Win32TimeNow();
	
	PyRepTuple *up = sfx.FastEncode();
	m_pilot->Destiny()->SendSingleDestinyUpdate(&up);	//consumed
	delete up;
}

void ActivatableModule::DeactivateModule(bool update) {
	Deactivate(m_effectName);
	if(update) {
		StopEffect();
	}
}




void HybridWeaponModule::StartEffect() {
	_SendGodmaShipEffect(effectProjectileFired, true);
}

void HybridWeaponModule::DoEffect() {
	codelog(SHIP__MODULE_TRACE, "Module %s (%lu): Triggering.", m_item->itemName().c_str(), m_item->itemID());
	
	//lookup target by ID. Use the target manager to reduce the search set.
	//this also ensures that they are in fact targeted.
	SystemEntity *target = m_pilot->targets.GetTarget(m_target, true);
	if(target == NULL) {
		codelog(SHIP__MODULE_TRACE, "Module %s (%lu): Unable to find target %lu", m_item->itemName().c_str(), m_item->itemID(), m_target);
		m_pilot->targets.Dump();
		DeactivateModule(true);
		return;
	}
	//TODO: check range.
	//TODO: check ammo/charge
	if(m_charge == NULL) {
		_log(SHIP__MODULE_TRACE, "Module %s (%lu): No ammo fitted. Unable to activate.", m_item->itemName().c_str(), m_item->itemID(), m_target);
		DeactivateModule(true);
		return;
	}

	//TODO: consume a unit of ammo, but do not release our ref until after
	//we are done processing damage, if we are now out of ammo...
	
	//send destiny OnSpecialFX to display the attack
	//TODO: we should actually pull this string from dgmTypeEffects
	_SendWeaponEffect("effects.ProjectileFired", target);
	
	Damage d(
		m_item, m_charge,
		m_pilot, 
		effectProjectileFired);
	m_pilot->ApplyDamageModifiers(d, target);
	target->ApplyDamage(d);
	
}

void HybridWeaponModule::StopEffect() {
	_SendGodmaShipEffect(effectProjectileFired, false);
}





void LaserWeaponModule::StartEffect() {
	_SendGodmaShipEffect(effectTargetAttack, true);
}

void LaserWeaponModule::DoEffect() {
	codelog(SHIP__MODULE_TRACE, "Module %s (%lu type %lu): Triggering.", m_item->itemName().c_str(), m_item->itemID(), m_item->typeID());
	
	//lookup target by ID. Use the target manager to reduce the search set.
	//this also ensures that they are in fact targeted.
	SystemEntity *target = m_pilot->targets.GetTarget(m_target, true);
	if(target == NULL) {
		codelog(SHIP__MODULE_TRACE, "Module %s (%lu): Unable to find target %lu", m_item->itemName().c_str(), m_item->itemID(), m_target);
		m_pilot->targets.Dump();
		DeactivateModule(true);
		return;
	}
	//TODO: check range.
	if(m_charge == NULL) {
		_log(SHIP__MODULE_TRACE, "Module %s (%lu): No crystal fitted. Unable to activate.", m_item->itemName().c_str(), m_item->itemID(), m_target);
		DeactivateModule(true);
		return;
	}
	
	//TODO: check capacitor load
	//m_item->capacitorNeed()

	//TODO: check for crystal failure?
	
	//send destiny OnSpecialFX to display the attack
	//TODO: we should actually pull this string from dgmTypeEffects
	_SendWeaponEffect("effects.Laser", target);
	
	Damage d(
		m_item, m_charge,
		m_pilot, 
		effectTargetAttack);
	m_pilot->ApplyDamageModifiers(d, target);
	target->ApplyDamage(d);
}

void LaserWeaponModule::StopEffect() {
	_SendGodmaShipEffect(effectTargetAttack, false);
}






void MiningLaserModule::StartEffect() {
	_SendGodmaShipEffect(effectMiningLaser, true);
}

void MiningLaserModule::DoEffect() {
	codelog(SHIP__MODULE_TRACE, "Module %s (%lu type %lu): Triggering.", m_item->itemName().c_str(), m_item->itemID(), m_item->typeID());
	
	//lookup target by ID. Use the target manager to reduce the search set.
	//this also ensures that they are in fact targeted.
	SystemEntity *target = m_pilot->targets.GetTarget(m_target, true);
	if(target == NULL) {
		codelog(SHIP__MODULE_TRACE, "Module %s (%lu): Unable to find target %lu", m_item->itemName().c_str(), m_item->itemID(), m_target);
		m_pilot->targets.Dump();
		DeactivateModule(true);
		return;
	}
	InventoryItem *target_item = target->Item();
	if(target_item == NULL) {
		codelog(SHIP__MODULE_TRACE, "Module %s (%lu): Use on non-asteroid. Target has no item.", m_item->itemName().c_str(), m_item->itemID(), (target_item==NULL)?0:target_item->itemID(), target_item);
		//send the client an error dialog?
		DeactivateModule(true);
		return;
	}
	if(target_item->categoryID() != EVEDB::invCategories::Asteroid) {
		codelog(SHIP__MODULE_TRACE, "Module %s (%lu): Use on non-asteroid item %lu of type %lu", m_item->itemName().c_str(), m_item->itemID(), target_item->itemID(), target_item->typeID());
		//send the client an error dialog?
		DeactivateModule(true);
		return;
	}
	//TODO: check range.
	//m_item->maxRange()
	if(m_charge == NULL) {
		//some use crystals, some do not???
		//_log(SHIP__MODULE_TRACE, "Module %s (%lu): No crystal fitted. Unable to activate.", m_item->itemName().c_str(), m_item->itemID(), m_target);
	}
	
	//TODO: check for crystal failure?
	//m_item->crystalsGetDamaged()
	
	//send destiny OnSpecialFX to display the attack
	//TODO: we should actually pull this string from dgmTypeEffects
	_SendWeaponEffect("effects.Laser", target);


	//TODO: something useful with this event...

}

void MiningLaserModule::StopEffect() {
	_SendGodmaShipEffect(effectMiningLaser, false);
}

uint32 MiningLaserModule::_ActivationInterval() const {
	//mining lasers use duration, not speed... no idea...
	return(m_item->duration());
}













