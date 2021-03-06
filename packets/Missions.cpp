/*  EVEmu: EVE Online Server Emulator
  
  **************************************************************
  This file is automatically generated, DO NOT EDIT IT DIRECTLY.
  **************************************************************
  
  (If you need to customize an object, you must copy that object
  into another source file, and give up the ability to generate it)
  
  
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


#include <string>
#include "..\packets\Missions.h"
#include "../common/PyRep.h"



	/*  this object is not really an object type on live, just convenience here  */

DoAction_Dialogue_Item::DoAction_Dialogue_Item() {
	actionID = 0;
	actionText = "";
}

DoAction_Dialogue_Item::~DoAction_Dialogue_Item() {
}

void DoAction_Dialogue_Item::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sDoAction_Dialogue_Item", pfx);
	_log(l_type, "%sactionID=%lu", pfx, actionID);
	_log(l_type, "%sactionText='%s'", pfx, actionText.c_str());
}

PyRepTuple *DoAction_Dialogue_Item::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(2);
	tuple0->items[0] = new PyRepInteger(actionID);
	tuple0->items[1] = new PyRepString(actionText);
	res = tuple0;
	

	return(res);
}

PyRepTuple *DoAction_Dialogue_Item::FastEncode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(2);
	tuple0->items[0] = new PyRepInteger(actionID);
	tuple0->items[1] = new PyRepString(actionText);
	res = tuple0;
	

	return(res);
}

bool DoAction_Dialogue_Item::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool DoAction_Dialogue_Item::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Dialogue_Item failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 2) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Dialogue_Item failed: tuple0 is the wrong size: expected 2, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	if(!tuple0->items[0]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Dialogue_Item failed: actionID is not an int: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_1 = (PyRepInteger *) tuple0->items[0];
	if(int_1->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode DoAction_Dialogue_Item: truncating 64 bit into into 32 bit int for field actionID");
	}
	actionID = int_1->value;
	if(!tuple0->items[1]->CheckType(PyRep::String)) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Dialogue_Item failed: actionText is not a string: %s", tuple0->items[1]->TypeString());
		delete packet;
		return(false);
	}
	PyRepString *string_2 = (PyRepString *) tuple0->items[1];
	actionText = string_2->value;

	delete packet;
	return(true);
}

DoAction_Dialogue_Item *DoAction_Dialogue_Item::Clone() const {
	DoAction_Dialogue_Item *res = new DoAction_Dialogue_Item;
	res->CloneFrom(this);
	return(res);
}

void DoAction_Dialogue_Item::CloneFrom(const DoAction_Dialogue_Item *from) {
	actionID = from->actionID;
	actionText = from->actionText;
	
}


DoAction_Result::DoAction_Result() {
	/*  this is the main text  */
	agentSays = "";
	/*  this is a list of DoAction_Dialogue_Item  */
	loyaltyPoints = 0;
}

DoAction_Result::~DoAction_Result() {
	/*  this is the main text  */
	/*  this is a list of DoAction_Dialogue_Item  */
}

void DoAction_Result::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sDoAction_Result", pfx);
	/*  this is the main text  */
	_log(l_type, "%sagentSays='%s'", pfx, agentSays.c_str());
	/*  this is a list of DoAction_Dialogue_Item  */
	_log(l_type, "%sdialogue: ", pfx);
	std::string dialogue_n(pfx);
	dialogue_n += "    ";
	dialogue.Dump(l_type, dialogue_n.c_str());
	_log(l_type, "%sloyaltyPoints=%lu", pfx, loyaltyPoints);
}

PyRepTuple *DoAction_Result::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(2);
	PyRepTuple *tuple1 = new PyRepTuple(2);
	/*  this is the main text  */
	tuple1->items[0] = new PyRepString(agentSays);
	/*  this is a list of DoAction_Dialogue_Item  */
	tuple1->items[1] = dialogue.Clone();
	tuple0->items[0] = tuple1;
	
	PyRepDict *dict2 = new PyRepDict();
	PyRep *dict2_0;
	dict2_0 = new PyRepInteger(loyaltyPoints);
	dict2->items[
		new PyRepString("loyaltyPoints")
	] = dict2_0;
	tuple0->items[1] = dict2;
	
	res = tuple0;
	

	return(res);
}

PyRepTuple *DoAction_Result::FastEncode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(2);
	PyRepTuple *tuple1 = new PyRepTuple(2);
	/*  this is the main text  */
	tuple1->items[0] = new PyRepString(agentSays);
	/*  this is a list of DoAction_Dialogue_Item  */
	
	PyRepList *list2 = new PyRepList();
	list2->items = dialogue.items;
	dialogue.items.clear();
	tuple1->items[1] = list2;
	tuple0->items[0] = tuple1;
	
	PyRepDict *dict3 = new PyRepDict();
	PyRep *dict3_0;
	dict3_0 = new PyRepInteger(loyaltyPoints);
	dict3->items[
		new PyRepString("loyaltyPoints")
	] = dict3_0;
	tuple0->items[1] = dict3;
	
	res = tuple0;
	

	return(res);
}

bool DoAction_Result::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool DoAction_Result::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Result failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 2) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Result failed: tuple0 is the wrong size: expected 2, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	if(!tuple0->items[0]->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Result failed: tuple1 is the wrong type: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple1 = (PyRepTuple *) tuple0->items[0];
	if(tuple1->items.size() != 2) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Result failed: tuple1 is the wrong size: expected 2, but got %d", tuple1->items.size());
		delete packet;
		return(false);
	}

	/*  this is the main text  */
	if(!tuple1->items[0]->CheckType(PyRep::String)) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Result failed: agentSays is not a string: %s", tuple1->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepString *string_2 = (PyRepString *) tuple1->items[0];
	agentSays = string_2->value;
	/*  this is a list of DoAction_Dialogue_Item  */
	if(!tuple1->items[1]->CheckType(PyRep::List)) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Result failed: dialogue is not a list: %s", tuple1->items[1]->TypeString());
		delete packet;
		return(false);
	}
	PyRepList *list_dialogue = (PyRepList *) tuple1->items[1];	dialogue.items = list_dialogue->items;
	list_dialogue->items.clear();
	
	if(!tuple0->items[1]->CheckType(PyRep::Dict)) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Result failed: dict3 is the wrong type: %s", tuple0->items[1]->TypeString());
		delete packet;
		return(false);
	}
	bool dict3_loyaltyPoints = false;
	PyRepDict *dict3 = (PyRepDict *) tuple0->items[1];
	
	PyRepDict::iterator dict3_cur, dict3_end;
	dict3_cur = dict3->items.begin();
	dict3_end = dict3->items.end();
	for(; dict3_cur != dict3_end; dict3_cur++) {
		PyRep *key__ = dict3_cur->first;
		if(!key__->CheckType(PyRep::String)) {
			_log(NET__PACKET_ERROR, "Decode DoAction_Result failed: a key in dict3 is the wrong type: %s", key__->TypeString());
			delete packet;
			return(false);
		}
		PyRepString *key_string__ = (PyRepString *) key__;
		
		if(key_string__->value == "loyaltyPoints") {
			dict3_loyaltyPoints = true;
	if(!dict3_cur->second->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Result failed: loyaltyPoints is not an int: %s", dict3_cur->second->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_4 = (PyRepInteger *) dict3_cur->second;
	if(int_4->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode DoAction_Result: truncating 64 bit into into 32 bit int for field loyaltyPoints");
	}
	loyaltyPoints = int_4->value;
		} else
		{
			_log(NET__PACKET_ERROR, "Decode DoAction_Result failed: Unknown key string '%s' in dict3", key_string__->value.c_str());
			delete packet;
			return(false);
		}
	}
	
	if(!dict3_loyaltyPoints) {
		_log(NET__PACKET_ERROR, "Decode DoAction_Result failed: Missing dict entry for 'loyaltyPoints' in dict3");
		delete packet;
		return(false);
	}
	

	delete packet;
	return(true);
}

DoAction_Result *DoAction_Result::Clone() const {
	DoAction_Result *res = new DoAction_Result;
	res->CloneFrom(this);
	return(res);
}

void DoAction_Result::CloneFrom(const DoAction_Result *from) {
	/*  this is the main text  */
	agentSays = from->agentSays;
	/*  this is a list of DoAction_Dialogue_Item  */
	dialogue.CloneFrom(&from->dialogue);
	loyaltyPoints = from->loyaltyPoints;
	
}


JournalDetails_Mission_Item::JournalDetails_Mission_Item() {
	/*  states: 0,1: not accepted, >1: accepted  */
	missionState = 0;
	/*  really a soft boolean  */
	importantMission = 0;
	missionType = "";
	missionName = "";
	agentID = 0;
	expirationTime = 0;
}

JournalDetails_Mission_Item::~JournalDetails_Mission_Item() {
	/*  states: 0,1: not accepted, >1: accepted  */
	/*  really a soft boolean  */
}

void JournalDetails_Mission_Item::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sJournalDetails_Mission_Item", pfx);
	/*  states: 0,1: not accepted, >1: accepted  */
	_log(l_type, "%smissionState=%lu", pfx, missionState);
	/*  really a soft boolean  */
	_log(l_type, "%simportantMission=%lu", pfx, importantMission);
	_log(l_type, "%smissionType='%s'", pfx, missionType.c_str());
	_log(l_type, "%smissionName='%s'", pfx, missionName.c_str());
	_log(l_type, "%sagentID=%lu", pfx, agentID);
	_log(l_type, "%sexpirationTime=" I64u, pfx, expirationTime);
	_log(l_type, "%sbookmarks: ", pfx);
	std::string bookmarks_n(pfx);
	bookmarks_n += "    ";
	bookmarks.Dump(l_type, bookmarks_n.c_str());
}

PyRepTuple *JournalDetails_Mission_Item::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(7);
	/*  states: 0,1: not accepted, >1: accepted  */
	tuple0->items[0] = new PyRepInteger(missionState);
	/*  really a soft boolean  */
	tuple0->items[1] = new PyRepInteger(importantMission);
	tuple0->items[2] = new PyRepString(missionType);
	tuple0->items[3] = new PyRepString(missionName);
	tuple0->items[4] = new PyRepInteger(agentID);
	tuple0->items[5] = new PyRepInteger(expirationTime);
	tuple0->items[6] = bookmarks.Clone();
	res = tuple0;
	

	return(res);
}

PyRepTuple *JournalDetails_Mission_Item::FastEncode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(7);
	/*  states: 0,1: not accepted, >1: accepted  */
	tuple0->items[0] = new PyRepInteger(missionState);
	/*  really a soft boolean  */
	tuple0->items[1] = new PyRepInteger(importantMission);
	tuple0->items[2] = new PyRepString(missionType);
	tuple0->items[3] = new PyRepString(missionName);
	tuple0->items[4] = new PyRepInteger(agentID);
	tuple0->items[5] = new PyRepInteger(expirationTime);
	
	PyRepList *list1 = new PyRepList();
	list1->items = bookmarks.items;
	bookmarks.items.clear();
	tuple0->items[6] = list1;
	res = tuple0;
	

	return(res);
}

bool JournalDetails_Mission_Item::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool JournalDetails_Mission_Item::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Mission_Item failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 7) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Mission_Item failed: tuple0 is the wrong size: expected 7, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	/*  states: 0,1: not accepted, >1: accepted  */
	if(!tuple0->items[0]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Mission_Item failed: missionState is not an int: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_1 = (PyRepInteger *) tuple0->items[0];
	if(int_1->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode JournalDetails_Mission_Item: truncating 64 bit into into 32 bit int for field missionState");
	}
	missionState = int_1->value;
	/*  really a soft boolean  */
	if(!tuple0->items[1]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Mission_Item failed: importantMission is not an int: %s", tuple0->items[1]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_2 = (PyRepInteger *) tuple0->items[1];
	if(int_2->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode JournalDetails_Mission_Item: truncating 64 bit into into 32 bit int for field importantMission");
	}
	importantMission = int_2->value;
	if(!tuple0->items[2]->CheckType(PyRep::String)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Mission_Item failed: missionType is not a string: %s", tuple0->items[2]->TypeString());
		delete packet;
		return(false);
	}
	PyRepString *string_3 = (PyRepString *) tuple0->items[2];
	missionType = string_3->value;
	if(!tuple0->items[3]->CheckType(PyRep::String)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Mission_Item failed: missionName is not a string: %s", tuple0->items[3]->TypeString());
		delete packet;
		return(false);
	}
	PyRepString *string_4 = (PyRepString *) tuple0->items[3];
	missionName = string_4->value;
	if(!tuple0->items[4]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Mission_Item failed: agentID is not an int: %s", tuple0->items[4]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_5 = (PyRepInteger *) tuple0->items[4];
	if(int_5->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode JournalDetails_Mission_Item: truncating 64 bit into into 32 bit int for field agentID");
	}
	agentID = int_5->value;
	if(!tuple0->items[5]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Mission_Item failed: expirationTime is not an int: %s", tuple0->items[5]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int64_6 = (PyRepInteger *) tuple0->items[5];
	expirationTime = int64_6->value;
	if(!tuple0->items[6]->CheckType(PyRep::List)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Mission_Item failed: bookmarks is not a list: %s", tuple0->items[6]->TypeString());
		delete packet;
		return(false);
	}
	PyRepList *list_bookmarks = (PyRepList *) tuple0->items[6];	bookmarks.items = list_bookmarks->items;
	list_bookmarks->items.clear();
	

	delete packet;
	return(true);
}

JournalDetails_Mission_Item *JournalDetails_Mission_Item::Clone() const {
	JournalDetails_Mission_Item *res = new JournalDetails_Mission_Item;
	res->CloneFrom(this);
	return(res);
}

void JournalDetails_Mission_Item::CloneFrom(const JournalDetails_Mission_Item *from) {
	/*  states: 0,1: not accepted, >1: accepted  */
	missionState = from->missionState;
	/*  really a soft boolean  */
	importantMission = from->importantMission;
	missionType = from->missionType;
	missionName = from->missionName;
	agentID = from->agentID;
	expirationTime = from->expirationTime;
	bookmarks.CloneFrom(&from->bookmarks);
	
}


JournalDetails_Offer_Item::JournalDetails_Offer_Item() {
	agentID = 0;
	offerName = "";
	/*  LP cost  */
	loyaltyPoints = 0;
	/*  this might need to be a string  */
	expirationTime = 0;
}

JournalDetails_Offer_Item::~JournalDetails_Offer_Item() {
	/*  LP cost  */
	/*  this might need to be a string  */
}

void JournalDetails_Offer_Item::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sJournalDetails_Offer_Item", pfx);
	_log(l_type, "%sagentID=%lu", pfx, agentID);
	_log(l_type, "%sofferName='%s'", pfx, offerName.c_str());
	/*  LP cost  */
	_log(l_type, "%sloyaltyPoints=%lu", pfx, loyaltyPoints);
	/*  this might need to be a string  */
	_log(l_type, "%sexpirationTime=" I64u, pfx, expirationTime);
}

PyRepTuple *JournalDetails_Offer_Item::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(4);
	tuple0->items[0] = new PyRepInteger(agentID);
	tuple0->items[1] = new PyRepString(offerName);
	/*  LP cost  */
	tuple0->items[2] = new PyRepInteger(loyaltyPoints);
	/*  this might need to be a string  */
	tuple0->items[3] = new PyRepInteger(expirationTime);
	res = tuple0;
	

	return(res);
}

PyRepTuple *JournalDetails_Offer_Item::FastEncode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(4);
	tuple0->items[0] = new PyRepInteger(agentID);
	tuple0->items[1] = new PyRepString(offerName);
	/*  LP cost  */
	tuple0->items[2] = new PyRepInteger(loyaltyPoints);
	/*  this might need to be a string  */
	tuple0->items[3] = new PyRepInteger(expirationTime);
	res = tuple0;
	

	return(res);
}

bool JournalDetails_Offer_Item::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool JournalDetails_Offer_Item::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Offer_Item failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 4) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Offer_Item failed: tuple0 is the wrong size: expected 4, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	if(!tuple0->items[0]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Offer_Item failed: agentID is not an int: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_1 = (PyRepInteger *) tuple0->items[0];
	if(int_1->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode JournalDetails_Offer_Item: truncating 64 bit into into 32 bit int for field agentID");
	}
	agentID = int_1->value;
	if(!tuple0->items[1]->CheckType(PyRep::String)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Offer_Item failed: offerName is not a string: %s", tuple0->items[1]->TypeString());
		delete packet;
		return(false);
	}
	PyRepString *string_2 = (PyRepString *) tuple0->items[1];
	offerName = string_2->value;
	/*  LP cost  */
	if(!tuple0->items[2]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Offer_Item failed: loyaltyPoints is not an int: %s", tuple0->items[2]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_3 = (PyRepInteger *) tuple0->items[2];
	if(int_3->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode JournalDetails_Offer_Item: truncating 64 bit into into 32 bit int for field loyaltyPoints");
	}
	loyaltyPoints = int_3->value;
	/*  this might need to be a string  */
	if(!tuple0->items[3]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Offer_Item failed: expirationTime is not an int: %s", tuple0->items[3]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int64_4 = (PyRepInteger *) tuple0->items[3];
	expirationTime = int64_4->value;

	delete packet;
	return(true);
}

JournalDetails_Offer_Item *JournalDetails_Offer_Item::Clone() const {
	JournalDetails_Offer_Item *res = new JournalDetails_Offer_Item;
	res->CloneFrom(this);
	return(res);
}

void JournalDetails_Offer_Item::CloneFrom(const JournalDetails_Offer_Item *from) {
	agentID = from->agentID;
	offerName = from->offerName;
	/*  LP cost  */
	loyaltyPoints = from->loyaltyPoints;
	/*  this might need to be a string  */
	expirationTime = from->expirationTime;
	
}


JournalDetails_Research_Item::JournalDetails_Research_Item() {
	agentID = 0;
	/*  field  */
	typeID = 0;
	/*  points per day  */
	ppd = 0.0;
	/*  current points  */
	points = 0.0;
}

JournalDetails_Research_Item::~JournalDetails_Research_Item() {
	/*  field  */
	/*  points per day  */
	/*  current points  */
}

void JournalDetails_Research_Item::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sJournalDetails_Research_Item", pfx);
	_log(l_type, "%sagentID=%lu", pfx, agentID);
	/*  field  */
	_log(l_type, "%stypeID=%lu", pfx, typeID);
	/*  points per day  */
	_log(l_type, "%sppd=%.13f", pfx, ppd);
	/*  current points  */
	_log(l_type, "%spoints=%.13f", pfx, points);
}

PyRepTuple *JournalDetails_Research_Item::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(4);
	tuple0->items[0] = new PyRepInteger(agentID);
	/*  field  */
	tuple0->items[1] = new PyRepInteger(typeID);
	/*  points per day  */
	tuple0->items[2] = new PyRepReal(ppd);
	/*  current points  */
	tuple0->items[3] = new PyRepReal(points);
	res = tuple0;
	

	return(res);
}

PyRepTuple *JournalDetails_Research_Item::FastEncode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(4);
	tuple0->items[0] = new PyRepInteger(agentID);
	/*  field  */
	tuple0->items[1] = new PyRepInteger(typeID);
	/*  points per day  */
	tuple0->items[2] = new PyRepReal(ppd);
	/*  current points  */
	tuple0->items[3] = new PyRepReal(points);
	res = tuple0;
	

	return(res);
}

bool JournalDetails_Research_Item::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool JournalDetails_Research_Item::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Research_Item failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 4) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Research_Item failed: tuple0 is the wrong size: expected 4, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	if(!tuple0->items[0]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Research_Item failed: agentID is not an int: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_1 = (PyRepInteger *) tuple0->items[0];
	if(int_1->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode JournalDetails_Research_Item: truncating 64 bit into into 32 bit int for field agentID");
	}
	agentID = int_1->value;
	/*  field  */
	if(!tuple0->items[1]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Research_Item failed: typeID is not an int: %s", tuple0->items[1]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_2 = (PyRepInteger *) tuple0->items[1];
	if(int_2->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode JournalDetails_Research_Item: truncating 64 bit into into 32 bit int for field typeID");
	}
	typeID = int_2->value;
	/*  points per day  */
	if(!tuple0->items[2]->CheckType(PyRep::Real)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Research_Item failed: ppd is not a real: %s", tuple0->items[2]->TypeString());
		delete packet;
		return(false);
	}
	PyRepReal *real_3 = (PyRepReal *) tuple0->items[2];
	ppd = real_3->value;
	/*  current points  */
	if(!tuple0->items[3]->CheckType(PyRep::Real)) {
		_log(NET__PACKET_ERROR, "Decode JournalDetails_Research_Item failed: points is not a real: %s", tuple0->items[3]->TypeString());
		delete packet;
		return(false);
	}
	PyRepReal *real_4 = (PyRepReal *) tuple0->items[3];
	points = real_4->value;

	delete packet;
	return(true);
}

JournalDetails_Research_Item *JournalDetails_Research_Item::Clone() const {
	JournalDetails_Research_Item *res = new JournalDetails_Research_Item;
	res->CloneFrom(this);
	return(res);
}

void JournalDetails_Research_Item::CloneFrom(const JournalDetails_Research_Item *from) {
	agentID = from->agentID;
	/*  field  */
	typeID = from->typeID;
	/*  points per day  */
	ppd = from->ppd;
	/*  current points  */
	points = from->points;
	
}


GetMyJournalDetails_Result::GetMyJournalDetails_Result() {
	/*  this is a list of JournalDetails_Mission_Item  */
	/*  this is a list of JournalDetails_Offer_Item  */
	/*  this is a list of JournalDetails_Research_Item  */
}

GetMyJournalDetails_Result::~GetMyJournalDetails_Result() {
	/*  this is a list of JournalDetails_Mission_Item  */
	/*  this is a list of JournalDetails_Offer_Item  */
	/*  this is a list of JournalDetails_Research_Item  */
}

void GetMyJournalDetails_Result::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sGetMyJournalDetails_Result", pfx);
	/*  this is a list of JournalDetails_Mission_Item  */
	_log(l_type, "%smissions: ", pfx);
	std::string missions_n(pfx);
	missions_n += "    ";
	missions.Dump(l_type, missions_n.c_str());
	/*  this is a list of JournalDetails_Offer_Item  */
	_log(l_type, "%soffers: ", pfx);
	std::string offers_n(pfx);
	offers_n += "    ";
	offers.Dump(l_type, offers_n.c_str());
	/*  this is a list of JournalDetails_Research_Item  */
	_log(l_type, "%sresearch: ", pfx);
	std::string research_n(pfx);
	research_n += "    ";
	research.Dump(l_type, research_n.c_str());
}

PyRepTuple *GetMyJournalDetails_Result::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(3);
	/*  this is a list of JournalDetails_Mission_Item  */
	tuple0->items[0] = missions.Clone();
	/*  this is a list of JournalDetails_Offer_Item  */
	tuple0->items[1] = offers.Clone();
	/*  this is a list of JournalDetails_Research_Item  */
	tuple0->items[2] = research.Clone();
	res = tuple0;
	

	return(res);
}

PyRepTuple *GetMyJournalDetails_Result::FastEncode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(3);
	/*  this is a list of JournalDetails_Mission_Item  */
	
	PyRepList *list1 = new PyRepList();
	list1->items = missions.items;
	missions.items.clear();
	tuple0->items[0] = list1;
	/*  this is a list of JournalDetails_Offer_Item  */
	
	PyRepList *list2 = new PyRepList();
	list2->items = offers.items;
	offers.items.clear();
	tuple0->items[1] = list2;
	/*  this is a list of JournalDetails_Research_Item  */
	
	PyRepList *list3 = new PyRepList();
	list3->items = research.items;
	research.items.clear();
	tuple0->items[2] = list3;
	res = tuple0;
	

	return(res);
}

bool GetMyJournalDetails_Result::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool GetMyJournalDetails_Result::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode GetMyJournalDetails_Result failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 3) {
		_log(NET__PACKET_ERROR, "Decode GetMyJournalDetails_Result failed: tuple0 is the wrong size: expected 3, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	/*  this is a list of JournalDetails_Mission_Item  */
	if(!tuple0->items[0]->CheckType(PyRep::List)) {
		_log(NET__PACKET_ERROR, "Decode GetMyJournalDetails_Result failed: missions is not a list: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepList *list_missions = (PyRepList *) tuple0->items[0];	missions.items = list_missions->items;
	list_missions->items.clear();
	
	/*  this is a list of JournalDetails_Offer_Item  */
	if(!tuple0->items[1]->CheckType(PyRep::List)) {
		_log(NET__PACKET_ERROR, "Decode GetMyJournalDetails_Result failed: offers is not a list: %s", tuple0->items[1]->TypeString());
		delete packet;
		return(false);
	}
	PyRepList *list_offers = (PyRepList *) tuple0->items[1];	offers.items = list_offers->items;
	list_offers->items.clear();
	
	/*  this is a list of JournalDetails_Research_Item  */
	if(!tuple0->items[2]->CheckType(PyRep::List)) {
		_log(NET__PACKET_ERROR, "Decode GetMyJournalDetails_Result failed: research is not a list: %s", tuple0->items[2]->TypeString());
		delete packet;
		return(false);
	}
	PyRepList *list_research = (PyRepList *) tuple0->items[2];	research.items = list_research->items;
	list_research->items.clear();
	

	delete packet;
	return(true);
}

GetMyJournalDetails_Result *GetMyJournalDetails_Result::Clone() const {
	GetMyJournalDetails_Result *res = new GetMyJournalDetails_Result;
	res->CloneFrom(this);
	return(res);
}

void GetMyJournalDetails_Result::CloneFrom(const GetMyJournalDetails_Result *from) {
	/*  this is a list of JournalDetails_Mission_Item  */
	missions.CloneFrom(&from->missions);
	/*  this is a list of JournalDetails_Offer_Item  */
	offers.CloneFrom(&from->offers);
	/*  this is a list of JournalDetails_Research_Item  */
	research.CloneFrom(&from->research);
	
}

	/*  may be preceeded by an
OnRemoteMessage: ('AgtMissionOfferWarning', {} )  */

OnAgentMissionChange_Args::OnAgentMissionChange_Args() {
	action = "";
	/*  seen 'offered'  */
	missionID = 0;
}

OnAgentMissionChange_Args::~OnAgentMissionChange_Args() {
	/*  seen 'offered'  */
}

void OnAgentMissionChange_Args::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sOnAgentMissionChange_Args", pfx);
	_log(l_type, "%saction='%s'", pfx, action.c_str());
	/*  seen 'offered'  */
	_log(l_type, "%smissionID=%lu", pfx, missionID);
}

PyRepTuple *OnAgentMissionChange_Args::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(2);
	tuple0->items[0] = new PyRepString(action);
	/*  seen 'offered'  */
	tuple0->items[1] = new PyRepInteger(missionID);
	res = tuple0;
	

	return(res);
}

PyRepTuple *OnAgentMissionChange_Args::FastEncode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(2);
	tuple0->items[0] = new PyRepString(action);
	/*  seen 'offered'  */
	tuple0->items[1] = new PyRepInteger(missionID);
	res = tuple0;
	

	return(res);
}

bool OnAgentMissionChange_Args::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool OnAgentMissionChange_Args::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode OnAgentMissionChange_Args failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 2) {
		_log(NET__PACKET_ERROR, "Decode OnAgentMissionChange_Args failed: tuple0 is the wrong size: expected 2, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	if(!tuple0->items[0]->CheckType(PyRep::String)) {
		_log(NET__PACKET_ERROR, "Decode OnAgentMissionChange_Args failed: action is not a string: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepString *string_1 = (PyRepString *) tuple0->items[0];
	action = string_1->value;
	/*  seen 'offered'  */
	if(!tuple0->items[1]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode OnAgentMissionChange_Args failed: missionID is not an int: %s", tuple0->items[1]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_2 = (PyRepInteger *) tuple0->items[1];
	if(int_2->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode OnAgentMissionChange_Args: truncating 64 bit into into 32 bit int for field missionID");
	}
	missionID = int_2->value;

	delete packet;
	return(true);
}

OnAgentMissionChange_Args *OnAgentMissionChange_Args::Clone() const {
	OnAgentMissionChange_Args *res = new OnAgentMissionChange_Args;
	res->CloneFrom(this);
	return(res);
}

void OnAgentMissionChange_Args::CloneFrom(const OnAgentMissionChange_Args *from) {
	action = from->action;
	/*  seen 'offered'  */
	missionID = from->missionID;
	
}



