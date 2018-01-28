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


#include "MarketDB.h"
#include "../common/dbcore.h"
#include "../common/logsys.h"
#include "../common/EVEDBUtils.h"
#include "../common/PyRep.h"
#include "../common/EVEUtils.h"
#include <set>

#include "../packets/Market.h"

MarketDB::MarketDB(DBcore *db)
: ServiceDB(db)
{
}

MarketDB::~MarketDB() {
}

PyRep *MarketDB::GetStationAsks(uint32 stationID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		"	typeID, MAX(price) AS price, volRemaining, stationID "
		" FROM market_orders "
		" WHERE stationID=%lu AND bid=0"
		" GROUP BY typeID", stationID))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	//NOTE: this SHOULD return a crazy dbutil.RowDict object which is
	//made up of packed blue.DBRow objects, but we do not understand
	//the marshalling of those well enough right now, and this object
	//provides the same interface. It is significantly bigger on the wire though.
	return(DBResultToIndexRowset("typeID", res));
}

PyRep *MarketDB::GetSystemAsks(uint32 solarSystemID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		"	typeID, MAX(price) AS price, volRemaining, stationID "
		" FROM market_orders "
		" WHERE solarSystemID=%lu AND bid=0"
		" GROUP BY typeID", solarSystemID))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	//NOTE: this SHOULD return a crazy dbutil.RowDict object which is
	//made up of packed blue.DBRow objects, but we do not understand
	//the marshalling of those well enough right now, and this object
	//provides the same interface. It is significantly bigger on the wire though.
	return(DBResultToIndexRowset("typeID", res));
}

PyRep *MarketDB::GetRegionBest(uint32 regionID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		"	typeID, MAX(price) AS price, volRemaining, stationID "
		" FROM market_orders "
		" WHERE regionID=%lu AND bid=0"
		" GROUP BY typeID", regionID))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	//NOTE: this SHOULD return a crazy dbutil.RowDict object which is
	//made up of packed blue.DBRow objects, but we do not understand
	//the marshalling of those well enough right now, and this object
	//provides the same interface. It is significantly bigger on the wire though.
	return(DBResultToIndexRowset("typeID", res));
}

PyRep *MarketDB::GetOrders(uint32 regionID, uint32 typeID) {
	DBQueryResult res;

	DBColumnTypeMap colmap;
	colmap["volRemaining"] = DBTYPE_R8;
	colmap["price"] = DBTYPE_CY;
	colmap["issued"] = DBTYPE_FILETIME;
	
	colmap["orderID"] = DBTYPE_I4;
	colmap["volEntered"] = DBTYPE_I4;
	colmap["minVolume"] = DBTYPE_I4;
	colmap["stationID"] = DBTYPE_I4;
	colmap["regionID"] = DBTYPE_I4;
	colmap["solarSystemID"] = DBTYPE_I4;
	colmap["jumps"] = DBTYPE_I4;
	
	colmap["duration"] = DBTYPE_I2;
	colmap["typeID"] = DBTYPE_I2;
	colmap["range"] = DBTYPE_I2;
	
	colmap["bid"] = DBTYPE_BOOL;

	//ordering: (painstakingly determined from packets)
	DBColumnOrdering ordering;
	ordering.push_back("price");
	ordering.push_back("volRemaining");
	ordering.push_back("issued");
	ordering.push_back("orderID");
	ordering.push_back("volEntered");
	ordering.push_back("minVolume");
	ordering.push_back("stationID");
	ordering.push_back("regionID");
	ordering.push_back("solarSystemID");
		ordering.push_back("jumps");	//not working right...
	ordering.push_back("typeID");
	ordering.push_back("range");
	ordering.push_back("duration");
	ordering.push_back("bid");
	
	
	
	//query sell orders
	//TODO: consider the `jumps` field... is it actually used? might be a pain in the ass if we need to actually populate it based on each queryier's location
	if(!m_db->RunQuery(res,
		"SELECT"
		"	price, volRemaining, typeID, range, orderID,"
		"   volEntered, minVolume, bid, issued, duration,"
		"   stationID, regionID, solarSystemID, jumps "
		" FROM market_orders "
		" WHERE regionID=%lu AND typeID=%lu AND bid=0", regionID, typeID))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	PyRepTuple *tup = new PyRepTuple(2);
	//this is wrong.
	tup->items[0] = DBResultToPackedRowList(res, colmap, ordering);
	
	//query buy orders
	if(!m_db->RunQuery(res,
		"SELECT"
		"	price, volRemaining, typeID, range, orderID,"
		"   volEntered, minVolume, bid, issued, duration,"
		"   stationID, regionID, solarSystemID, jumps "
		" FROM market_orders "
		" WHERE regionID=%lu AND typeID=%lu AND bid=1", regionID, typeID))
	{
		delete tup;
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	//this is wrong.
	tup->items[1] = DBResultToPackedRowList(res, colmap, ordering);
	
	return(tup);
}

PyRep *MarketDB::GetCharOrders(uint32 characterID) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT"
		"   orderID, typeID, charID, regionID, stationID,"
		"   range, bid, price, volEntered, volRemaining,"
		"   issued, orderState, minVolume, contraband,"
		"   accountID, duration, isCorp, solarSystemID,"
		"   escrow "
		" FROM market_orders "
		" WHERE charID=%lu", characterID))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

PyRep *MarketDB::GetOldPriceHistory(uint32 regionID, uint32 typeID) {
	DBQueryResult res;

	DBColumnTypeMap colmap;
	colmap["historyDate"] = DBTYPE_FILETIME;
	colmap["lowPrice"] = DBTYPE_CY;
	colmap["highPrice"] = DBTYPE_CY;
	colmap["avgPrice"] = DBTYPE_CY;
	colmap["volume"] = DBTYPE_I8;
	colmap["orders"] = DBTYPE_I4;

	//ordering: (painstakingly determined from packets)
	DBColumnOrdering ordering;
	ordering.push_back("historyDate");
	ordering.push_back("lowPrice");
	ordering.push_back("highPrice");
	ordering.push_back("avgPrice");
	ordering.push_back("volume");
	ordering.push_back("orders");
	
	if(!m_db->RunQuery(res,
		"SELECT"
		"	historyDate, lowPrice, highPrice, avgPrice,"
		"	volume, orders "
		" FROM market_history_old "
		" WHERE regionID=%lu AND typeID=%lu", regionID, typeID))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToPackedRowListTuple(res, colmap, ordering));
}

PyRep *MarketDB::GetNewPriceHistory(uint32 regionID, uint32 typeID) {
	DBQueryResult res;

	DBColumnTypeMap colmap;
	colmap["historyDate"] = DBTYPE_FILETIME;
	colmap["lowPrice"] = DBTYPE_CY;
	colmap["highPrice"] = DBTYPE_CY;
	colmap["avgPrice"] = DBTYPE_CY;
	colmap["volume"] = DBTYPE_I8;
	colmap["orders"] = DBTYPE_I4;

	//ordering: (painstakingly determined from packets)
	DBColumnOrdering ordering;
	ordering.push_back("historyDate");
	ordering.push_back("lowPrice");
	ordering.push_back("highPrice");
	ordering.push_back("avgPrice");
	ordering.push_back("volume");
	ordering.push_back("orders");

	//build the history record from the recent market transactions.
	//NOTE: it may be a good idea to cache the historyDate column in each
	//record when they are inserted instead of re-calculating it each query.
	// this would also allow us to put together an index as well...
	if(!m_db->RunQuery(res,
		"SELECT"
		"	transactionDateTime-(transactionDateTime%%" I64d ") AS historyDate,"
		"	MIN(price) AS lowPrice,"
		"	MAX(price) AS highPrice,"
		"	AVG(price) AS avgPrice,"
		"	SUM(quantity) AS volume,"
		"	COUNT(transactionID) AS orders"
		" FROM market_transactions "
		" WHERE regionID=%lu AND typeID=%lu"
		"	AND transactionType=%d "	//both buy and sell transactions get recorded, only compound one set of data... choice was arbitrary.
		" GROUP BY historyDate", 
		Win32Time_Day, regionID, typeID, TransactionTypeBuy))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToPackedRowListTuple(res, colmap, ordering));
}

bool MarketDB::BuildOldPriceHistory() {
	DBerror err;

	uint64 cutoff_time = Win32TimeNow();
	cutoff_time -= cutoff_time % Win32Time_Day;	//round down to an even day boundary.
	cutoff_time -= HISTORY_AGGREGATION_DAYS * Win32Time_Day;

	//build the history record from the recent market transactions.
	if(!m_db->RunQuery(err,
		"INSERT INTO"
		"	market_history_old"
		" 	(regionID, typeID, historyDate, lowPrice, highPrice, avgPrice, volume, orders)"
		" SELECT"
		"	regionID,"
		"	typeID,"
		"	transactionDateTime-(transactionDateTime%%" I64d ") AS historyDate,"
		"	MIN(price) AS lowPrice,"
		"	MAX(price) AS highPrice,"
		"	AVG(price) AS avgPrice,"
		"	SUM(quantity) AS volume,"
		"	COUNT(transactionID) AS orders"
		" FROM market_transactions "
		" WHERE"
		"	transactionType=%d AND "	//both buy and sell transactions get recorded, only compound one set of data... choice was arbitrary.
		"	(transactionDateTime-(transactionDateTime%%" I64d ")) < " I64d 
		" GROUP BY regionID, typeID, historyDate", 
			Win32Time_Day,
			TransactionTypeBuy,
			Win32Time_Day,
			cutoff_time
			))
	{
		codelog(MARKET__ERROR, "Error in query: %s", err.c_str());
		return(false);
	}
	
	//now remove the transactions which have been aged out?
	if(!m_db->RunQuery(err,
		"DELETE FROM"
		"	market_transactions"
		" WHERE"
		"	historyDate < " I64d,
		cutoff_time))

	{
		codelog(MARKET__ERROR, "Error in query: %s", err.c_str());
		return(false);
	}

	return(true);
}
PyRepObject *MarketDB::GetCorporationBills(uint32 corpID, bool payable) {
	DBQueryResult res;
	std::string table, field;
	if (payable) {
		table = "billsPayable";
		field = "debtorID";
	} else {
		table = "billsReceivable";
		field = "creditorID";
	}
	if (!m_db->RunQuery(res,
		" SELECT "
		" billID, billTypeID, debtorID, creditorID, amount, dueDateTime, interest, externalID, paid externalID2 "
		" FROM %s "
		" WHERE %s = %lu ",
		table.c_str(), field.c_str(), corpID))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}


//helper routine for GetMarketGroups
static void _PropigateItems(std::map< int, std::set<uint32> > &types, std::map<int, int> &parentChild, std::map<int, std::set<int> > &childParent, int group) {
	std::map<int, std::set<int> >::iterator children_res;
	children_res = childParent.find(group);
	if(children_res != childParent.end()) {
		//recurse to all children first.
		std::set<int>::iterator ccur, cend;
		ccur = children_res->second.begin();
		cend = children_res->second.end();
		for(; ccur != cend; ccur++) {
			_PropigateItems(types, parentChild, childParent, *ccur);
		}
	}

	//find our parent.
	std::map<int, int>::iterator parent_res;
	parent_res = parentChild.find(group);
	if(parent_res == parentChild.end()) {
		codelog(MARKET__ERROR, "Failed to find parent group in parentChild for %d", group);
		return;	//should never happen...
	}
	int parentID = parent_res->second;
	if(parentID == -1) {
		return;	//do not propigate up to NULL, we dont need it, and it would contain ALL items..
	}
	
	
	//now propigate all our items (which now includes all children items) up to our parent.
	std::map< int, std::set<uint32> >::iterator pres, self_res;
	self_res = types.find(group);
	if(self_res == types.end()) {
		return;	//we have nothing for this type??
	}
	pres = types.find(parentID);
	if(pres == types.end()) {
		types[parentID] = self_res->second;
	} else {
		//add all of self into parent.
		std::set<uint32>::iterator ccur, cend;
		ccur = self_res->second.begin();
		cend = self_res->second.end();
		for(; ccur != cend; ccur++) {
			pres->second.insert(*ccur);
		}
	}
	
}

//this is a crap load of work... there HAS to be a better way to do this..
PyRepObject *MarketDB::GetMarketGroups() {
	DBQueryResult res;


	//returns cached object marketProxy.GetMarketGroups
	//marketGroupID, parentGroupID, marketGroupName, description, graphicID, hasTypes, types
	//this is going to be a real pain... another "nested" query thing...
	// I really wanna know how they do this crap with their MS SQL server.. 
	// I hope its not as much of a nightmare as it is for us....

	
	//first we need to query out all the types because we need them to 
	// fill in the 'types' subquery for each row of the result
	std::map< int, std::set<uint32> > types;
	uint32 currentGroup = 0;
	std::set<uint32> currentTypes;
	if(!m_db->RunQuery(res,
		"SELECT"
		"	marketGroupID,typeID"
		" FROM invTypes"
		" ORDER BY marketGroupID"))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	DBResultRow row;
	while(res.GetRow(row)) {
		uint32 g = row.GetUInt(0);
		if(currentGroup != g) {
			//group has changed
			if(currentGroup != 0) {
				types[currentGroup] = currentTypes;
				currentTypes.clear();
			}
			currentGroup = g;
		}
		currentTypes.insert(row.GetUInt(1));
	}




	
	
	std::map<int, int> parentChild;	//maps client -> parent
	std::map<int, std::set<int> > childParent; //maps parent -> all children.
	if(!m_db->RunQuery(res,
		"SELECT"
		"	marketGroupID, parentGroupID"
		" FROM invMarketGroups"))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	while(res.GetRow(row)) {
		int marketGroupID = row.GetUInt(0);
		//figure out the parent ID, mapping NULL to -1 for our map.
		int parentID;
		if(row.IsNull(1)) {
			parentID = -1;
		} else {
			parentID = row.GetUInt(1);
		}
		parentChild[marketGroupID] = parentID;
		childParent[parentID].insert(marketGroupID);
	}

	//now we need to propigate all of the items up the tree (a parent group's items list contains ALL items of its children.)
	_PropigateItems(types, parentChild, childParent, -1);
	
	//now we get to do the other query.
	if(!m_db->RunQuery(res,
		"SELECT"
		"	marketGroupID, parentGroupID, marketGroupName, description, graphicID, hasTypes"
		" FROM invMarketGroups"))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	//doing this the long (non XML) way to avoid the extra copies due to the huge volume of data here.
	PyRepDict *args = new PyRepDict();
	PyRepObject *o = new PyRepObject("util.FilterRowset", args);

	PyRepDict *parentSets = new PyRepDict();
	PyRepList *header = new PyRepList();
	
	header->add("marketGroupID");
	header->add("parentGroupID");
	header->add("marketGroupName");
	header->add("description");
	header->add("graphicID");
	header->add("hasTypes");
	header->add("types");	//this column really contains an entire list.
	
	args->add("header", header);
	args->add("idName", new PyRepString("parentGroupID"));
	args->add("RowClass", new PyRepString("util.Row", true));
	args->add("idName2", new PyRepNone());
	args->add("items", parentSets);

	std::map<int, PyRepList *> parentLists;
	std::map<int, PyRepList *>::iterator cur, end;
	
	//now fill in items.
	// we have to satisfy this structure... which uses parentGroupID as the
	// main dict key, each dict entry is then a list of MarketGroup_Entrys
	// which have that parentGroupID
	//marketGroupID, parentGroupID, marketGroupName, description, graphicID, hasTypes, types
	std::map< int, std::set<uint32> >::const_iterator tt;
	MarketGroup_Entry entry;
	PyRepNone none_gid;
	PyRepInteger int_gid(0);
	PyRepInteger int_pgid(0);
	while(res.GetRow(row)) {
		entry.marketGroupID = row.GetUInt(0);

		//figure out the parent ID, mapping NULL to -1 for our map.
		int parentID;
		if(row.IsNull(1)) {
			entry.parentGroupID = &none_gid;
			parentID = -1;
		} else {
			int_pgid.value = row.GetUInt(1);
			parentID = int_pgid.value;
			entry.parentGroupID = &int_pgid;
		}
		//find the list for this parent ID
		PyRepList *parentList;
		cur = parentLists.find(parentID);
		if(cur != parentLists.end()) {
			parentList = cur->second;
		} else {
			parentList = new PyRepList();
			parentLists[parentID] = parentList;
		}
		
		entry.marketGroupName = row.GetText(2);
		entry.description = row.GetText(3);
		
		if(row.IsNull(4)) {
			entry.graphicID = &none_gid;
		} else {
			int_gid.value = row.GetUInt(4);
			entry.graphicID = &int_gid;
		}
		
		entry.hasTypes = row.GetUInt(5);

		tt = types.find(entry.marketGroupID);
		entry.types.clear();
		if(tt != types.end()) {
			std::set<uint32>::const_iterator curi, endi;
			curi = tt->second.begin();
			endi = tt->second.end();
			for(; curi != endi; curi++) {
				entry.types.push_back(*curi);
			}
		}

		//record ourself in the parent's list.
		parentList->add( entry.Encode() );
	}
	entry.graphicID = NULL;
	entry.parentGroupID = NULL;

	//now we actually stick the parent lists into the FilterRowset's items dict.
	cur = parentLists.begin();
	end = parentLists.end();
	for(; cur != end; cur++) {
		PyRep *key;
		if(cur->first == -1)
			key = new PyRepNone();
		else
			key = new PyRepInteger(cur->first);
		parentSets->add(key, cur->second);	//takes ownership of the list.
	}

	return(o);
}

uint32 MarketDB::StoreBuyOrder(
	uint32 clientID, 
	uint32 accountID, 
	uint32 stationID, 
	uint32 typeID, 
	double price, 
	uint32 quantity, 
	uint8 orderRange, 
	uint32 minVolume, 
	uint8 duration,
	bool useCorp
) {
	return(StoreGenericOrder(clientID, accountID, stationID, typeID, price, quantity, orderRange, minVolume, duration, useCorp, true));
}

uint32 MarketDB::StoreSellOrder(
	uint32 clientID, 
	uint32 accountID, 
	uint32 stationID, 
	uint32 typeID, 
	double price, 
	uint32 quantity, 
	uint8 orderRange, 
	uint32 minVolume, 
	uint8 duration,
	bool useCorp
) {
	return(StoreGenericOrder(clientID, accountID, stationID, typeID, price, quantity, orderRange, minVolume, duration, useCorp, false));
}

uint32 MarketDB::StoreGenericOrder(
	uint32 clientID, 
	uint32 accountID, 
	uint32 stationID, 
	uint32 typeID, 
	double price, 
	uint32 quantity, 
	uint8 orderRange, 
	uint32 minVolume, 
	uint8 duration,
	bool useCorp,
	bool isBuy
) {
	DBerror err;

	uint32 solarSystemID;
	uint32 constellationID;
	uint32 regionID;
	if(!GetStationParents(stationID, solarSystemID, constellationID, regionID)) {
		codelog(MARKET__ERROR, "Char %lu: Failed to find parents for station %lu", clientID, stationID);
		return(0);
	}

	//TODO: figure out what the orderState field means...
	//TODO: implement the contraband flag properly.
	//TODO: implement the isCorp flag properly.
	uint32 orderID;
	if(!m_db->RunQueryLID(err, orderID,
		"INSERT INTO market_orders ("
		"	orderID, typeID, charID, regionID, stationID,"
		"	range, bid, price, volEntered, volRemaining, issued,"
		"	orderState, minVolume, contraband, accountID, duration,"
		"	isCorp, solarSystemID, escrow, jumps "
		" ) VALUES ("
		"	NULL, %lu, %lu, %lu, %lu, "
		"	%d, %d, %f, %lu, %lu, " I64u ", "
		"	1, %lu, 0, %lu, %d, "
		"	%d, %lu, 0, 1"
		" )",
			typeID, clientID, regionID, stationID,
			orderRange, isBuy?1:0, price, quantity, quantity, Win32TimeNow(),
			minVolume, accountID, duration,
			useCorp?1:0, solarSystemID
		))

	{
		codelog(MARKET__ERROR, "Error in query: %s", err.c_str());
		return(0);
	}

	return(orderID);
}

//NOTE: needs a lot of work to implement orderRange
uint32 MarketDB::FindBuyOrder(
	uint32 stationID, 
	uint32 typeID, 
	double price, 
	uint32 quantity, 
	uint32 orderRange
) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT orderID"
		"	FROM market_orders"
		"	WHERE bid=1"
		"		AND typeID=%lu"
		"		AND stationID=%lu"
		"		AND volRemaining >= %lu"
		"		AND price <= %f"
		"	LIMIT 1",	//right now, we just care about the first order which can satisfy our needs.
		typeID,
		stationID,
		quantity,
		price))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row))
		return(0);	//no order found.
	
	return(row.GetUInt(0));
}

uint32 MarketDB::FindSellOrder(
	uint32 stationID, 
	uint32 typeID, 
	double price, 
	uint32 quantity, 
	uint32 orderRange
) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT orderID"
		"	FROM market_orders"
		"	WHERE bid=0"
		"		AND typeID=%lu"
		"		AND stationID=%lu"
		"		AND volRemaining >= %lu"
		"		AND price <= %f"
		"	LIMIT 1",	//right now, we just care about the first order which can satisfy our needs.
		typeID,
		stationID,
		quantity,
		price))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row))
		return(0);	//no order found.
	
	return(row.GetUInt(0));
}


//NOTE: this logic needs some work if there are multiple concurrent market services running at once.
bool MarketDB::DBExecuteOrder(
	uint32 buy_order_id,
	uint32 quantity,
	bool &order_deleted,
	uint32 &orderOwnerID,
	uint32 &typeID,
	double &price
) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT volRemaining,price,typeID,charID"
		"	FROM market_orders"
		"	WHERE orderID=%lu",
		buy_order_id))
	{
		codelog(MARKET__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(MARKET__ERROR, "No row for order %lu", buy_order_id);
		return(false);
	}
	uint32 volRemaining = row.GetUInt(0);
	
	if(volRemaining < quantity) {
		codelog(MARKET__ERROR, "Order %lu only has %lu remaining, not the requested %lu!", buy_order_id, volRemaining, quantity);
		return(false);
	}
	
	price = row.GetDouble(1);
	typeID = row.GetUInt(2);
	orderOwnerID = row.GetUInt(3);
	
	if(volRemaining == quantity) {
		DBerror err;
		if(!m_db->RunQuery(err,
			"DELETE FROM market_orders WHERE orderID=%lu",
			buy_order_id)
		) {
			codelog(MARKET__ERROR, "Error in query: %s", err.c_str());
			return(false);
		}
		order_deleted = true;
	} else {
		DBerror err;
		if(!m_db->RunQuery(err,
			"UPDATE market_orders "
			"	SET volRemaining=volRemaining-%lu"
			"	WHERE orderID=%lu",
			quantity,
			buy_order_id)
		) {
			codelog(MARKET__ERROR, "Error in query: %s", err.c_str());
			return(false);
		}
		order_deleted = false;
	}
	return(true);
}

bool MarketDB::RecordTransaction(
	uint32 typeID, 
	uint32 quantity, 
	double price, 
	MktTransType transactionType, 
	uint32 charID, 
	uint32 regionID, 
	uint32 stationID
) {
	DBerror err;
	
	if(!m_db->RunQuery(err,
		"INSERT INTO"
		" market_transactions ("
		"	transactionID, transactionDateTime, typeID, quantity,"
		"	price, transactionType, clientID, regionID, stationID"
		" ) VALUES ("
		"	NULL, " I64u ", %lu, %lu,"
		"	%f, %d, %lu, %lu, %lu"
		" )", 
			Win32TimeNow(), typeID, quantity,
			price, transactionType, charID, regionID, stationID
			))
	{
		codelog(MARKET__ERROR, "Error in query: %s", err.c_str());
		return(false);
	}
	return(true);
}























