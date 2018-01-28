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



#include "LSCDB.h"
#include "../common/dbcore.h"
#include "../common/logsys.h"
#include "../common/EVEDBUtils.h"
#include "../common/EVEUtils.h"
#include "LSCService.h"
#include "LSCChannel.h"

#include "../packets/General.h"
#include "../packets/LSCPkts.h"

LSCDB::LSCDB(DBcore *db)
: ServiceDB(db)
{
}

LSCDB::~LSCDB() {
}

void LSCDB::LoadChannels(LSCService *svc, std::vector<LSCChannel *> &channels) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	channelID, ownerID, displayName, motd,"
		"	comparisonKey, memberless, password,"
		"	mailingList, cspa, mode"
		" FROM channels"
	))
	{
		_log(SERVICE__ERROR, "Error in LoadChannels query: %s", res.error.c_str());
		return;
	}

	DBResultRow row;
	while(res.GetRow(row)) {
		LSCChannelDesc desc;
		desc.type = LSCChannelDesc::normal;
		desc.number = row.GetInt(0);
		channels.push_back(new LSCChannel(
			svc, desc,
			row.GetInt(1),
			row.GetText(2),
			row.GetText(3),
			row.GetText(4),
			(row.GetInt(5)!=0),
			row.GetText(6),
			(row.GetInt(7)!=0),
			(row.GetInt(8)!=0),
			false,
			row.GetInt(9) ) );
	}
	
}

/*PyRepObject *LSCDB::ListChannels(uint32 charId, uint32 corpID) {

	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	channelID, ownerID, displayName, motd,"
		"	comparisonKey, memberless, password,"
		"	mailingList, cspa, temporary, mode,"
		"	subscribed, estimatedMemberCount"
		" FROM channels"
	))
	{
		_log(SERVICE__ERROR, "Error in ListChannels query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

PyRepObject *LSCDB::ListChannels(const std::vector<uint32> &channels) {

	std::string ids;
	ListToINString(channels, ids, "-1");
	
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	channelID, ownerID, displayName, motd,"
		"	comparisonKey, memberless, password,"
		"	mailingList, cspa, temporary,"
		"	subscribed"
		" FROM channels WHERE channelID IN (%s)", ids.c_str()
	))
	{
		_log(SERVICE__ERROR, "Error in ListChannels query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
	
}*/

PyRepObject *LSCDB::QueryChannelInfo(uint32 channel_id) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	channelID, ownerID, displayName, motd,"
		"	comparisonKey, memberless, password,"
		"	mailingList, cspa, temporary,"
		"	subscribed"
		" FROM channels WHERE channelID=%lu", channel_id
	))
	{
		_log(SERVICE__ERROR, "Error in QueryChannelInfo query: %s", res.error.c_str());
		return(NULL);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in QueryChannelInfo query: no data for channel %d", channel_id);
		return(false);
	}
	
	return(DBRowToRow(row));
}


PyRepObject *LSCDB::QueryChannelMods(uint32 channel_id) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	accessor, mode, untilWhen, originalMode, admin, reason"
		" FROM channelMods WHERE channelID=%lu", channel_id
	))
	{
		_log(SERVICE__ERROR, "Error in QueryChannelMods query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}


PyRepObject *LSCDB::QueryChannelChars(uint32 channel_id) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	charID, corpID, allianceID, role, extra"
		" FROM channelChars WHERE channelID=%lu", channel_id
	))
	{
		_log(SERVICE__ERROR, "Error in QueryChannelChars query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

PyRepObject *LSCDB::LookupChars(const char *match, bool exact) {
	DBQueryResult res;
	
	std::string matchEsc;
	m_db->DoEscapeString(matchEsc, match);
	if (matchEsc == "__ALL__") {
		if(!m_db->RunQuery(res,
			"SELECT "
			"	characterID, characterName, typeID"
			" FROM character_"
			" WHERE characterID >= 140000000"))
		{
			_log(SERVICE__ERROR, "Error in LookupChars query: %s", res.error.c_str());
			return(NULL);
		}
	} else {
		if(!m_db->RunQuery(res,
			"SELECT "
			"	characterID, characterName, typeID"
			" FROM character_"
			" WHERE characterName %s '%s'", 
			exact?"=":"RLIKE", matchEsc.c_str()
		))
		{
			_log(SERVICE__ERROR, "Error in LookupChars query: %s", res.error.c_str());
			return(NULL);
		}
	}
	
	
	return(DBResultToRowset(res));
}

//temporarily relocated into ServiceDB until some things get cleaned up...
uint32 ServiceDB::StoreNewEVEMail(uint32 senderID, uint32 recipID, const char * subject, const char * message, uint64 sentTime) {
	DBQueryResult res;
	DBerror err;
	DBResultRow row;

	std::string escaped;
	// Escape message header
	m_db->DoEscapeString(escaped, subject);

	// Store message header
	uint32 messageID;
	if (!m_db->RunQueryLID(err, messageID, 
		" INSERT INTO "
		" evemail "
		" (channelID, senderID, subject, created) "
		" VALUES (%lu, %lu, '%s', %llu) ",
		recipID, senderID, escaped.c_str(), sentTime ))
	{
		codelog(SERVICE__ERROR, "Error in query, message header couldn't be saved: %s", err.c_str());
		return (0);
	}

	_log(SERVICE__MESSAGE, "New messageID: %lu", messageID);

	// Escape message content
	m_db->DoEscapeString(escaped, message);

	// Store message content
	if (!m_db->RunQuery(err,
		" INSERT INTO evemailDetails "
		" (messageID, mimeTypeID, attachment) VALUES (%lu, 1, '%s') ",
		messageID, escaped.c_str()
		))
	{
		codelog(SERVICE__ERROR, "Error in query, message content couldn't be saved: %s", err.c_str());
		// Delete message header
		if (!m_db->RunQuery(err, "DELETE FROM `evemail` WHERE `messageID` = %lu;", messageID))
		{
			codelog(SERVICE__ERROR, "Failed to remove invalid header data for messgae id %lu: %s", messageID, err.c_str());
		}
		return (0);
	}


	return (messageID);
}

PyRepObject *LSCDB::GetEVEMailHeaders(uint32 recID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT channelID, messageID, senderID, subject, created, `read` "
		" FROM evemail "
		" WHERE channelID=%lu", recID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

PyRep *LSCDB::GetEVEMailDetails(uint32 messageID, uint32 readerID) {
	DBQueryResult result;
	DBResultRow row;

	//we need to query out the primary message here... not sure how to properly
	//grab the "main message" though... the text/plain clause is pretty hackish.
	if (!m_db->RunQuery(result,
		" SELECT evemail.messageID, evemail.senderID, evemail.subject, " // need messageID as char*
		" evemailDetails.attachment, evemailDetails.mimeTypeID, "
		" evemailMimeType.mimeType, evemailMimeType.`binary`, "
		" evemail.created, evemail.channelID "
		" FROM evemail "
		" LEFT JOIN evemailDetails"
		"	ON evemailDetails.messageID = evemail.messageID "
		" LEFT JOIN evemailMimeType"
		"	ON evemailMimeType.mimeTypeID = evemailDetails.mimeTypeID "
		" WHERE evemail.messageID=%lu"
		"	AND channelID=%lu",
			messageID, readerID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", result.error.c_str());
		return (NULL);
	}

	if (!result.GetRow(row)) {
		codelog(SERVICE__MESSAGE, "No message with messageID %lu", messageID);
		return (NULL);
	}

	Rsp_GetEVEMailDetails details;
	details.messageID = row.GetUInt(0);
	details.senderID = row.GetUInt(1);
	details.subject = row.GetText(2);
	details.body = row.GetText(3);
	details.created = row.GetUInt64(7);
	details.channelID = row.GetUInt(8);
	details.deleted = 0; // If a message's details are sent, then it isn't deleted. If it's deleted, details cannot be sent
	details.mimeTypeID = row.GetInt(4);
	details.mimeType = row.GetText(5);
	details.binary = row.GetInt(6);

	return(details.Encode());
}

bool LSCDB::MarkMessageRead(uint32 messageID) {
	DBerror err;

	if (!m_db->RunQuery(err,
		" UPDATE evemail "
		" SET `read` = 1 "
		" WHERE messageID=%lu", messageID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		return false;
	}

	return true;
}

bool LSCDB::DeleteMessage(uint32 messageID, uint32 readerID) {
	DBerror err;
	bool ret = true;

	if (!m_db->RunQuery(err,
		" DELETE FROM evemail "
		" WHERE messageID=%lu AND channelID=%lu", messageID, readerID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		ret = false;
	}
	if (!m_db->RunQuery(err,
		" DELETE FROM evemailDetails "
		" WHERE messageID=%lu", messageID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		ret = false;
	}

	return ret;

}
PyRepObject *LSCDB::LookupCorporations(const std::string & search) {
	DBQueryResult res;
	std::string secure;
	m_db->DoEscapeString(secure, search);

	if (!m_db->RunQuery(res, 
		" SELECT "
		" corporationID, corporationName, corporationType "
		" FROM corporation "
		" WHERE corporationName RLIKE '%s'", secure.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return 0;
	}

	return DBResultToRowset(res);
}
PyRepObject *LSCDB::LookupFactions(const std::string & search) {
	DBQueryResult res;
	std::string secure;
	m_db->DoEscapeString(secure, search);

	if (!m_db->RunQuery(res, 
		" SELECT "
		" factionID, factionName "
		" FROM chrFactions "
		" WHERE factionName RLIKE '%s'", secure.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return 0;
	}

	return DBResultToRowset(res);
}
PyRepObject *LSCDB::LookupCorporationTickers(const std::string & search) {
	DBQueryResult res;
	std::string secure;
	m_db->DoEscapeString(secure, search);

	if (!m_db->RunQuery(res, 
		" SELECT "
		" corporationID, corporationName, tickerName "
		" FROM corporation "
		" WHERE tickerName RLIKE '%s'", secure.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return 0;
	}

	return DBResultToRowset(res);
}
PyRepObject *LSCDB::LookupStations(const std::string & search) {
	DBQueryResult res;
	std::string secure;
	m_db->DoEscapeString(secure, search);

	if (!m_db->RunQuery(res, 
		" SELECT "
		" stationID, stationName, stationTypeID "
		" FROM staStations "
		" WHERE stationName RLIKE '%s'", secure.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return 0;
	}

	return DBResultToRowset(res);
}
PyRepObject *LSCDB::LookupLocationsByGroup(const std::string & search, uint32 typeID) {
	DBQueryResult res;
	std::string secure;
	m_db->DoEscapeString(secure, search);

	if (!m_db->RunQuery(res, 
		" SELECT "
		" itemID, itemName, typeID "
		" FROM entity "
		" WHERE itemName RLIKE '%s' AND typeID = %lu", secure.c_str(), typeID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return 0;
	}

	return DBResultToRowset(res);
}














