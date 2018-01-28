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



#include "InvBrokerService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../PyBoundObject.h"
#include "../inventory/ItemFactory.h"
#include "../inventory/InventoryItem.h"

#include "../packets/General.h"
#include "../packets/Inventory.h"

PyCallable_Make_InnerDispatcher(InvBrokerService)


class InvBrokerBound
: public PyBoundObject {
public:

	PyCallable_Make_Dispatcher(InvBrokerBound)
	
	InvBrokerBound(uint32 entityID, PyServiceMgr *mgr, InventoryDB *db)
	: PyBoundObject(mgr, "InvBrokerBound"),
	  m_dispatch(new Dispatcher(this)),
	  m_entityID(entityID),
	  m_db(db)
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(InvBrokerBound, GetInventoryFromId)
		PyCallable_REG_CALL(InvBrokerBound, GetInventory)
		PyCallable_REG_CALL(InvBrokerBound, SetLabel)
		PyCallable_REG_CALL(InvBrokerBound, TrashItems)
	}
	virtual ~InvBrokerBound() {}
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL(GetInventoryFromId)
	PyCallable_DECL_CALL(GetInventory)
	PyCallable_DECL_CALL(SetLabel)
	PyCallable_DECL_CALL(TrashItems)
	

protected:
	Dispatcher *const m_dispatch;

	uint32 m_entityID;
	
	InventoryDB *const m_db;
};


class InventoryBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(InventoryBound)
	
	InventoryBound(InventoryItem *item, EVEItemFlags flag, PyServiceMgr *mgr, InventoryDB *db)
	: PyBoundObject(mgr, "InventoryBound"),
	  m_dispatch(new Dispatcher(this)),
	  m_item(item),
	  m_flag(flag),
	  m_db(db)
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(InventoryBound, List)
		PyCallable_REG_CALL(InventoryBound, Add)
		PyCallable_REG_CALL(InventoryBound, MultiAdd)
		PyCallable_REG_CALL(InventoryBound, GetItem)
		PyCallable_REG_CALL(InventoryBound, ListStations)
		PyCallable_REG_CALL(InventoryBound, ReplaceCharges)
		PyCallable_REG_CALL(InventoryBound, MultiMerge)
		PyCallable_REG_CALL(InventoryBound, StackAll)

	}
	virtual ~InventoryBound() { m_item->Release(); }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL(List)
	PyCallable_DECL_CALL(Add)
	PyCallable_DECL_CALL(MultiAdd)
	PyCallable_DECL_CALL(GetItem)
	PyCallable_DECL_CALL(ListStations)
	PyCallable_DECL_CALL(ReplaceCharges)
	PyCallable_DECL_CALL(MultiMerge)
	PyCallable_DECL_CALL(StackAll)

	
	

protected:
	Dispatcher *const m_dispatch;

	InventoryItem *const m_item;	//we own a reference of this
	const EVEItemFlags m_flag;
	
	InventoryDB *const m_db;

	PyCallResult _ExecAdd(Client *c, const std::vector<uint32> &items, uint32 quantity, EVEItemFlags flag);
	PyCallResult _ValidateAdd( Client *c, const std::vector<uint32> &items, uint32 quantity, EVEItemFlags flag);
};


InvBrokerService::InvBrokerService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "invbroker"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);
	
	//PyCallable_REG_CALL(InvBrokerService, MachoBindObject)
}

InvBrokerService::~InvBrokerService() {
	delete m_dispatch;
}


PyBoundObject *InvBrokerService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	InvBroker_BindArgs args;
	//temp crap until I rework _CreateBoundObject's signature
	PyRep *t = bind_args->Clone();
	if(!args.Decode(&t)) {
		codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", c->GetName());
		return(NULL);
	}
	_log(CLIENT__MESSAGE, "InvBrokerService bind request for:");
	args.Dump(CLIENT__MESSAGE, "    ");

	return(new InvBrokerBound(args.entityID, m_manager, &m_db));
}

//this is a view into the entire inventory item.
PyCallResult InvBrokerBound::Handle_GetInventoryFromId(PyCallArgs &call) {
	Call_TwoIntegerArgs args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return (NULL);
	}
    //bool passive = (args.arg2 != 0);	//no idea what this is for.
	
	InventoryItem *item = m_manager->item_factory->Load(args.arg1, true);
	if(item == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to load item %lu", call.client->GetName(), args.arg1);
		return (NULL);
	}
	
	//we just bind up a new inventory object and give it back to them.
	InventoryBound *ib = new InventoryBound(item, flagAutoFit, m_manager, m_db);
	PyRep *result = m_manager->BindObject(call.client, ib);

	return(result);
}

//this is a view into an inventory item using a specific flag.
PyCallResult InvBrokerBound::Handle_GetInventory(PyCallArgs &call) {
	Inventory_GetInventory args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments");
		return(NULL);
	}
	PyRep *result = NULL;
	
	EVEItemFlags flag;
	switch(args.container) {
	case containerWallet:
		flag = flagWallet;
		break;
	case containerCharacter:
		flag = flagSkill;
		break;
	case containerHangar:
		flag = flagHangar;
		break;
		
	case containerGlobal:
	case containerSolarSystem:
	case containerScrapHeap:
	case containerFactory:
	case containerBank:
	case containerRecycler:
	case containerOffices:
	case containerStationCharacters:
	case containerCorpMarket:
	default:
		codelog(SERVICE__ERROR, "Unhandled container type %d", args.container);
		return(NULL);
	}
	
	InventoryItem *item = m_manager->item_factory->Load(m_entityID, true);
	if(item == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to load item %lu", call.client->GetName(), m_entityID);
		return (NULL);
	}

	_log(SERVICE__MESSAGE, "%s: Binding inventory object for %s for container %lu with flag %lu", GetName(), call.client->GetName(), m_entityID, flag);
	
	//we just bind up a new inventory object and give it back to them.
	InventoryBound *ib = new InventoryBound(item, flag, m_manager, m_db);
	result = m_manager->BindObject(call.client, ib);
	
	return(result);
}

PyCallResult InvBrokerBound::Handle_SetLabel(PyCallArgs &call) {
	CallSetLabel args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments");
		return(NULL);
	}
	
	InventoryItem *item = m_manager->item_factory->Load(args.itemID, false);
	if(item == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to load item %lu", call.client->GetName(), args.itemID);
		return (NULL);
	}
	
	//TODO: check to make sure we are allowed to rename this item

	item->Rename(args.itemName.c_str());
	item->Release();
	
	//do we need to send some sort of update?
	
	return(NULL);
}

PyCallResult InvBrokerBound::Handle_TrashItems(PyCallArgs &call) {
	Call_SingleIntList args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments");
		return(new PyRepList());
	}
	
	std::vector<uint32>::const_iterator cur, end;

	cur = args.ints.begin();
	end = args.ints.end();

	for( ;cur != end; cur++)
	{
		InventoryItem *item = m_manager->item_factory->Load(*cur, false);
		
		if(item == NULL) 
		{
			codelog(SERVICE__ERROR, "%s: Unable to load item %lu", call.client->GetName(), *cur);
			return (new PyRepList());
		}
		
		if (call.client->GetCharacterID() == item->ownerID())
		{
			
			//Notify client that we trashed the item

			//TODO: Add proper defs for for these values
			//6 = Junk location
			//2 = Junk Owner
			//0 = Junk Flag?
			item->Move(6, flagAutoFit, true); //Move item to junk location
			item->ChangeOwner(2, true); //Set owner as junk owner

			//Release our loaded ref (just kidding, Delete does this)
			//item->Release();		
			
			//Finally Delete the item from the DB
			item->Delete();
		}
		else
		{
			codelog(SERVICE__ERROR, "Character %s tried to trash item %lu, this item isn't owned by them", call.client->GetName(), *cur);
			item->Release();
			return (new PyRepList());
		}
	
	}
	return(new PyRepList());
}

PyCallResult InventoryBound::Handle_List(PyCallArgs &call) {
	PyRep *result = NULL;

	//TODO: check to make sure we are allowed to list this container

	//this is such crap, need better logic.
	/*uint32 list_flag = flagCargoHold;
	if(m_entityID == call.client->GetLocationID())
		list_flag = flagInventory;*/
	
	
	
	result = m_item->GetInventoryRowset(m_flag);
	if(result == NULL)
		result = new PyRepNone();
	
	return(result);
}

PyCallResult InventoryBound::Handle_ReplaceCharges(PyCallArgs &call) {
	Inventory_CallReplaceCharges args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments");
		return(NULL);
	}
	
	//validate flag.
	if(args.flag < flagSlotFirst || args.flag > flagSlotLast) {
		codelog(SERVICE__ERROR, "%s: Invalid flag %d", call.client->GetName(), args.flag);
		return(NULL);
	}
	
	InventoryItem *new_charge = m_item->GetByID(args.itemID, false);
	if(new_charge == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to find charge %d", call.client->GetName(), args.itemID);
		return(NULL);
	}

	if(new_charge->quantity() < args.quantity) {
		codelog(SERVICE__ERROR, "%s: Item %lu: Requested quantity (%d) exceeds actual quantity (%d), using actual.", call.client->GetName(), args.itemID, args.quantity, new_charge->quantity());
	} else if(new_charge->quantity() > args.quantity) {
		new_charge = m_item->Split(new_charge, args.quantity);
		if(new_charge == NULL) {
			codelog(SERVICE__ERROR, "%s: Unable to split charge %d into %d", call.client->GetName(), args.itemID, args.quantity);
			return(NULL);
		}
	}
	
	call.client->modules.ReplaceCharges((EVEItemFlags) args.flag, new_charge);
	
	return(new PyRepInteger(1));
}


PyCallResult InventoryBound::Handle_ListStations(PyCallArgs &call) {
	PyRep *result = NULL;

	codelog(SERVICE__ERROR, "Unimplemented.");

	PyRepObject *rowset = new PyRepObject();
	result = rowset;
	rowset->type = "util.Rowset";
	PyRepDict *args = new PyRepDict();
	rowset->arguments = args;

	//RowClass:
	args->add("RowClass", new PyRepString("util.Row", true));

	//header:
	PyRepList *header = new PyRepList();
	args->add("header", header);
	header->add("stationID");
	header->add("itemCount");

	//lines:
	PyRepList *invlist = new PyRepList();
	args->add("lines", invlist);
	
	return(result);
}

PyCallResult InventoryBound::Handle_GetItem(PyCallArgs &call) {
	PyRep *result = NULL;

	result = m_item->GetEntityRow();
	if(result == NULL)
		result = new PyRepNone();
	
	return(result);
}

PyCallResult InventoryBound::Handle_Add(PyCallArgs &call) {

	
	if((call.tuple)->items.size() == 3) {
	Inventory_CallAdd args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
		return(NULL);
	}

	std::vector<uint32> items;
	items.push_back(args.itemID);

		return(_ExecAdd(call.client, items, args.quantity, (EVEItemFlags)args.flag));

	} else if((call.tuple)->items.size() == 2) {
		Inventory_CallAddCargoContainer args;
		//chances are its trying to transfer into a cargo container
		if(!args.Decode(&call.tuple)) {
			codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
			return(NULL);
		}

		std::vector<uint32> items;
		items.push_back(args.itemID);
		return(_ExecAdd(call.client, items, args.quantity, flagAutoFit));

	} else {
		codelog(SERVICE__ERROR, "[Add]Unknown number of args in tuple");
		return(NULL);
	}
		
	return(NULL);
}

PyCallResult InventoryBound::Handle_MultiAdd(PyCallArgs &call) {
	if((call.tuple)->items.size() == 1) {
		
		Call_SingleIntList args;
		if(!args.Decode(&call.tuple)) {
			codelog(SERVICE__ERROR, "Unable to decode arguments");
			return(NULL);
		}

		//not sure about what flag to use here...
		return(_ExecAdd(call.client, args.ints, 1, flagHangar));
	} else {
		Inventory_CallMultiAdd args;
		if(!args.Decode(&call.tuple)) {
			codelog(SERVICE__ERROR, "Unable to decode arguments");
			return(NULL);
		}

		//NOTE: They can specify "None" in the quantity field to indicate
		//their intention to move all... we turn this into a 0 for simplicity.
		
		//TODO: should verify args.flag before casting!
		return(_ExecAdd(call.client, args.itemIDs, args.quantity, (EVEItemFlags)args.flag));
	}

	return(NULL);
}

PyCallResult InventoryBound::Handle_MultiMerge(PyCallArgs &call) {
	
	//Decode Args
	Inventory_CallMultiMerge elements;

	if(!elements.Decode(&call.tuple)) 
	{
		codelog(SERVICE__ERROR, "Unable to decode elements");
		return(new PyRepNone());
	}	

	Inventory_CallMultiMergeElement element;
	PyRep* codedElement;

	std::vector<PyRep *>::const_iterator current, endlist;
	current = elements.MMElements.items.begin();
	endlist = elements.MMElements.items.end();

	for (;current != endlist; current++) 
	{
		codedElement = *current;
		
		if(!element.Decode(&codedElement)) 
		{
			codelog(SERVICE__ERROR, "Unable to decode element");
			return(new PyRepNone());
		}
	

		InventoryItem *draggedItem = m_manager->item_factory->Load(element.draggedItemID,false);
		InventoryItem *stationaryItem = m_manager->item_factory->Load(element.stationaryItemID,false);

		//If the items have the same type and both have the singletons unset we can merge
		if( (draggedItem->typeID() == stationaryItem->typeID()) && (draggedItem->singleton() == false) && (stationaryItem->singleton() == false) )
		{

			//Merge the 2 items
			if( !stationaryItem->AlterQuantity(element.draggedQty, true) )
			{
				codelog(SERVICE__ERROR, "Unable to add %lu qty to item %lu",element.draggedQty, element.stationaryItemID);
				stationaryItem->Release();
				draggedItem->Release();
			}
			else
			{
				
				//Set the dragged item qty to 0
				draggedItem->SetQuantity(0, true);
				//move to blank location
				draggedItem->Move( 0, flagAutoFit, false);
				
				//release out Loaded Ref
				//draggedItem->Release();

				//Delete the dragged item					
				draggedItem->Delete();

				//Release Stationary item
				stationaryItem->Release();
			}
		}
		else
		{
			//Items cannot be merged
			//Log it
			codelog(SERVICE__ERROR,"Error Stacking items for %lu", m_item);
			
			
			//TODO: Send an proper error response like the live server instead of this
			//should be error response:UserException-CannotMergeSingletonItems
			call.client->SendErrorMsg("Cannot Merge items with singleton set.");

			//Release items
			draggedItem->Release();
			stationaryItem->Release();

		}

	}

	elements.MMElements.items.clear();
	return(new PyRepNone());
}

PyCallResult InventoryBound::Handle_StackAll(PyCallArgs &call) {

	Call_SingleIntegerArg arg;

	EVEItemFlags stackFlag = flagHangar;
	
	if( call.tuple->items.size() != 0)
	{
		if( !arg.Decode( &call.tuple ))
		{
			return(new PyRepNone());
		}

		stackFlag = (EVEItemFlags)arg.arg;
	}
	
	//Stack Items contained in this inventory
	if( !m_item->StackContainedItems(stackFlag))
	{
		codelog(SERVICE__ERROR,"Error Stacking items for %lu", m_item);
	}

	return(new PyRepNone());
}

PyCallResult InventoryBound::_ValidateAdd( Client *c, const std::vector<uint32> &items, uint32 quantity, EVEItemFlags flag)
{

	double totalVolume = 0.0;
	std::vector<uint32>::const_iterator cur, end;
	cur = items.begin();
	end = items.end();
	for(; cur != end; cur++) {
		
		InventoryItem *sourceItem = m_manager->item_factory->Load((*cur), true);

		
		//If hold already contains this item then we can ignore remaining space
		if ((!m_item->Contains(sourceItem)) || (m_item->flag() != flag))
		{
			//Add volume to totalVolume
			if( items.size() > 1)
			{
				totalVolume += (sourceItem->quantity() * sourceItem->volume());
			}
			else
			{
				totalVolume += (quantity * sourceItem->volume());
			}
		}
		
		//TODO: check to see if they are allowed to move this item
		if( (flag == flagDroneBay) && (sourceItem->categoryID() != EVEDB::invCategories::Drone) )
		{
			//Can only put drones in drone bay
			//Return ErrorResponse
			Inventory_ItemCannotBeInDroneBayException except;			
			sourceItem->Release();
			return(PyCallException(except.FastEncode()));
	}


		//Release the sourceItem 
		sourceItem->Release();
	}	

	//Check total volume used size
	if( flag != flagHangar)
	{		
		if( totalVolume > m_item->GetRemainingCapacity(flag))
		{
			//The moving item is too big to fit into dest space
			//Log Error
			if( items.size() > 1)
			{
				_log(ITEM__ERROR, "Cannot Perform Add. Items are too large (%f m3) to fit into destination %lu (%f m3 capacity remaining)",totalVolume, m_item->itemID(), m_item->GetRemainingCapacity(flag)); 
			}
			else
			{
				_log(ITEM__ERROR, "Cannot Perform Add. Item %lu is too large (%f m3) to fit into destination %lu (%f m3 capacity remaining)",items[0],totalVolume, m_item->itemID(), m_item->GetRemainingCapacity(flag));
			}
			//release source item
			Inventory_NotEnoughCargoSpaceException except;
			except.spaceavail = m_item->GetRemainingCapacity( flag );
			except.spaceavaildict = except.spaceavail;
			except.volume = totalVolume;
			except.volumedict = except.volume;
			return(PyCallException(except.FastEncode()));
		}
	}
	return(NULL);
}
	

PyCallResult InventoryBound::_ExecAdd(Client *c, const std::vector<uint32> &items, uint32 quantity, EVEItemFlags flag) {
	
	bool fLoadoutRequest = false;
	
	//TODO: handle auto-fit flag
	if(flag == flagAutoFit)
		flag = flagHangar;

	
	//Is this a loadout request
	if( flag >= flagSlotFirst && flag <= flagSlotLast)
	{
		//Validate a loadout request
		fLoadoutRequest = true;
		
	}
	else
	{
		//Make sure all items can be moved successfully will be ok
		PyCallResult returnResult = _ValidateAdd(c, items, quantity, flag);

		if( returnResult.type != PyCallResult::RegularResult )
		{
			//Invalid add, return the ErrorResponse
			return(returnResult);
		}

	}
	
	


	//If were here, we can try move all the items (validated)
	std::vector<uint32>::const_iterator cur, end;
	cur = items.begin();
	end = items.end();
	for(; cur != end; cur++) {
		
		InventoryItem *sourceItem = m_manager->item_factory->Load((*cur), true);			

		//NOTE: a multiadd can come in with quantity 0 to indicate "all"
		
		/*Check if its a simple item move or an item split qty is diff if its a 
		  split also multiple items cannot be split so the size() should be 1*/
		if( (sourceItem->quantity() != quantity) && items.size() == 1 )
		{
			//Its a split request, create a new item
			//We don't release this ref, as the new container inherits it
			InventoryItem *newItem;
			newItem = m_manager->item_factory->Spawn(
				sourceItem->typeID(),
				quantity,
				c->GetCharacterID(),
				0,
				flag
				);

		
			//Move New item to its new location
			if( newItem != NULL )
			{
				newItem->MoveInto(m_item, flag);				
				if(fLoadoutRequest == true)
				{
					newItem->ChangeSingleton( true );
				}
				//Remove qty from origional item
				if( sourceItem->AlterQuantity(-quantity, true) == false)
				{
					codelog(SERVICE__ERROR,"Error Removing qty %lu from item %lu.", -quantity, newItem->itemID());
				}

				//Create new item id return result
				Call_SingleIntegerArg result;
				result.arg = newItem->itemID();
				
				//Release Source Item
				sourceItem->Release();

				//Return new item result
				return( result.FastEncode() );
			}
			else
			{
				codelog(SERVICE__ERROR,"Error spawning an item of type %lu", sourceItem->typeID());
			}
		
	}
		else
		{
			//Its a move request
			sourceItem->MoveInto( m_item, flag);
			if(fLoadoutRequest == true)
			{
				sourceItem->ChangeSingleton( true );
			}
			
		}
	
		//Release the source Item PTR, we dont need it anymore
		sourceItem->Release();

		
	}
	//Return Null if no item was created
	return(NULL);
}

















