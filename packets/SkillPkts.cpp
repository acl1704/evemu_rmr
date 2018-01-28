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
#include "..\packets\SkillPkts.h"
#include "../common/PyRep.h"



	/*  this belongs somewhere else  */

NotifyOnItemChange::NotifyOnItemChange() {
	/*  0  */
	/* object of type util.Row */
	string0 = "";
	string2 = "";

NotifyOnItemChange::~NotifyOnItemChange() {
	/*  0  */
	/*  1  */
	std::map<uint32, PyRep *>::iterator dict4_cur, dict4_end;
	//free any existing elements first
	dict4_cur = dict4.begin();
	dict4_end = dict4.end();
	for(; dict4_cur != dict4_end; dict4_cur++) {
		delete dict4_cur->second;
	}
	
}

void NotifyOnItemChange::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sNotifyOnItemChange", pfx);
	/*  0  */
	_log(l_type, "%sObject of type util.Row:", pfx);
	_log(l_type, "%sstring0='%s'", pfx, string0.c_str());
	_log(l_type, "%slist1: ", pfx);
	std::string list1_n(pfx);
	list1_n += "    ";
	list1.Dump(l_type, list1_n.c_str());
	_log(l_type, "%sstring2='%s'", pfx, string2.c_str());

PyRepTuple *NotifyOnItemChange::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(2);
	/*  0  */
	PyRep *args1;
	PyRepDict *dict2 = new PyRepDict();
	args1 = dict2;
	
	tuple0->items[0] = new PyRepObject(
			"util.Row",
			args1
		);
	
	/*  1  */
	
	PyRepDict *dict3 = new PyRepDict();
	std::map<uint32, PyRep *>::iterator dict4_cur, dict4_end;
	dict4_cur = dict4.begin();
	dict4_end = dict4.end();
	for(; dict4_cur != dict4_end; dict4_cur++) {
		dict3->items[
			new PyRepInteger(dict4_cur->first)
		] = dict4_cur->second->Clone();
	}
	tuple0->items[1] = dict3;
	
	res = tuple0;
	

	return(res);
}

bool NotifyOnItemChange::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool NotifyOnItemChange::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode NotifyOnItemChange failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 2) {
		_log(NET__PACKET_ERROR, "Decode NotifyOnItemChange failed: tuple0 is the wrong size: expected 2, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	/*  0  */
	if(!tuple0->items[0]->CheckType(PyRep::Object)) {
		_log(NET__PACKET_ERROR, "Decode NotifyOnItemChange failed: obj_1 is the wrong type: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepObject *obj_1 = (PyRepObject *) tuple0->items[0];
	
	if(obj_1->type != "util.Row") {
		_log(NET__PACKET_ERROR, "Decode NotifyOnItemChange failed: obj_1 is the wrong object type. Expected 'util.Row', got '%s'", obj_1->type.c_str());
		delete packet;
		return(false);
	}
	
	if(!obj_1->arguments->CheckType(PyRep::Dict)) {
		_log(NET__PACKET_ERROR, "Decode NotifyOnItemChange failed: dict2 is the wrong type: %s", obj_1->arguments->TypeString());
		delete packet;
		return(false);
	}
	//dict2 is empty from our perspective, not enforcing though.
	/*  1  */
	if(!tuple0->items[1]->CheckType(PyRep::Dict)) {
		_log(NET__PACKET_ERROR, "Decode NotifyOnItemChange failed: dict4 is not a dict: %s", tuple0->items[1]->TypeString());
		delete packet;
		return(false);
	}
	dict4.clear();
	PyRepDict *dict_3 = (PyRepDict *) tuple0->items[1];
	PyRepDict::iterator dict4_cur, dict4_end;
	dict4_cur = dict_3->items.begin();
	dict4_end = dict_3->items.end();
	int dict4_index;
	for(dict4_index = 0; dict4_cur != dict4_end; dict4_cur++, dict4_index++) {
		if(!dict4_cur->first->CheckType(PyRep::Integer)) {
			_log(NET__PACKET_ERROR, "Decode NotifyOnItemChange failed: Key %d in dict dict4 is not an integer: %s", dict4_index, dict4_cur->first->TypeString());
			delete packet;
			return(false);
		}
		PyRepInteger *k = (PyRepInteger *) dict4_cur->first;
		if(k->value > 0xFFFFFFFFLL) {
			_log(NET__PACKET_WARNING, "Decode NotifyOnItemChange: truncating 64 bit into into 32 bit int in key of entry %d in field dict4", dict4_index);
		}
		dict4[k->value] = dict4_cur->second->Clone();
	}
	

	delete packet;
	return(true);
}

NotifyOnItemChange *NotifyOnItemChange::Clone() const {
	NotifyOnItemChange *res = new NotifyOnItemChange;
	/*  0  */
	/* object of type util.Row */
	res->string0 = string0;
	res->list1.CloneFrom(&list1);
	res->string2 = string2;

CallCharStartTrainingSkill::CallCharStartTrainingSkill() {
	/*  0  */
	skillItemID = 0;
}

CallCharStartTrainingSkill::~CallCharStartTrainingSkill() {
	/*  0  */
}

void CallCharStartTrainingSkill::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sCallCharStartTrainingSkill", pfx);
	/*  0  */
	_log(l_type, "%sskillItemID=%lu", pfx, skillItemID);
}

PyRepTuple *CallCharStartTrainingSkill::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(1);
	/*  0  */
	tuple0->items[0] = new PyRepInteger(skillItemID);
	res = tuple0;
	

	return(res);
}

bool CallCharStartTrainingSkill::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool CallCharStartTrainingSkill::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode CallCharStartTrainingSkill failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 1) {
		_log(NET__PACKET_ERROR, "Decode CallCharStartTrainingSkill failed: tuple0 is the wrong size: expected 1, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	/*  0  */
	if(!tuple0->items[0]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode CallCharStartTrainingSkill failed: skillItemID is not an int: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_1 = (PyRepInteger *) tuple0->items[0];
	if(int_1->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode CallCharStartTrainingSkill: truncating 64 bit into into 32 bit int for field skillItemID");
	}
	skillItemID = int_1->value;

	delete packet;
	return(true);
}

CallCharStartTrainingSkill *CallCharStartTrainingSkill::Clone() const {
	CallCharStartTrainingSkill *res = new CallCharStartTrainingSkill;
	/*  0  */
	res->skillItemID = skillItemID;

	return(res);
}

NotifyOnSkillStartTraining::NotifyOnSkillStartTraining() {
	/*  0  */
	skillItemID = 0;
	/*  1  */
	/*  not sure exactly  */
	complete_time = 0;
}

NotifyOnSkillStartTraining::~NotifyOnSkillStartTraining() {
	/*  0  */
	/*  1  */
	/*  not sure exactly  */
}

void NotifyOnSkillStartTraining::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sNotifyOnSkillStartTraining", pfx);
	/*  0  */
	_log(l_type, "%sskillItemID=%lu", pfx, skillItemID);
	/*  1  */
	/*  not sure exactly  */
	_log(l_type, "%scomplete_time=" I64u, pfx, complete_time);
}

PyRepTuple *NotifyOnSkillStartTraining::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(2);
	/*  0  */
	tuple0->items[0] = new PyRepInteger(skillItemID);
	/*  1  */
	/*  not sure exactly  */
	tuple0->items[1] = new PyRepInteger(complete_time);
	res = tuple0;
	

	return(res);
}

bool NotifyOnSkillStartTraining::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool NotifyOnSkillStartTraining::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode NotifyOnSkillStartTraining failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 2) {
		_log(NET__PACKET_ERROR, "Decode NotifyOnSkillStartTraining failed: tuple0 is the wrong size: expected 2, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	/*  0  */
	if(!tuple0->items[0]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode NotifyOnSkillStartTraining failed: skillItemID is not an int: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_1 = (PyRepInteger *) tuple0->items[0];
	if(int_1->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode NotifyOnSkillStartTraining: truncating 64 bit into into 32 bit int for field skillItemID");
	}
	skillItemID = int_1->value;
	/*  1  */
	/*  not sure exactly  */
	if(!tuple0->items[1]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode NotifyOnSkillStartTraining failed: complete_time is not an int: %s", tuple0->items[1]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int64_2 = (PyRepInteger *) tuple0->items[1];
	complete_time = int64_2->value;

	delete packet;
	return(true);
}

NotifyOnSkillStartTraining *NotifyOnSkillStartTraining::Clone() const {
	NotifyOnSkillStartTraining *res = new NotifyOnSkillStartTraining;
	/*  0  */
	res->skillItemID = skillItemID;
	/*  1  */
	/*  not sure exactly  */
	res->complete_time = complete_time;

	return(res);
}

RspMachoBindObject/CharStartTrainingSkill::RspMachoBindObject/CharStartTrainingSkill() {
	skillItemID = 0;
}

RspMachoBindObject/CharStartTrainingSkill::~RspMachoBindObject/CharStartTrainingSkill() {
}

void RspMachoBindObject/CharStartTrainingSkill::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sRspMachoBindObject/CharStartTrainingSkill", pfx);
	_log(l_type, "%sskillItemID=%lu", pfx, skillItemID);
}

PyRepInteger *RspMachoBindObject/CharStartTrainingSkill::Encode() {
	PyRepInteger *res = NULL;
	res = new PyRepInteger(skillItemID);

	return(res);
}

bool RspMachoBindObject/CharStartTrainingSkill::Decode(PyRepInteger **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool RspMachoBindObject/CharStartTrainingSkill::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode RspMachoBindObject/CharStartTrainingSkill failed: skillItemID is not an int: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_0 = (PyRepInteger *) packet;
	if(int_0->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode RspMachoBindObject/CharStartTrainingSkill: truncating 64 bit into into 32 bit int for field skillItemID");
	}
	skillItemID = int_0->value;

	delete packet;
	return(true);
}

RspMachoBindObject/CharStartTrainingSkill *RspMachoBindObject/CharStartTrainingSkill::Clone() const {
	RspMachoBindObject/CharStartTrainingSkill *res = new RspMachoBindObject/CharStartTrainingSkill;
	res->skillItemID = skillItemID;

	return(res);
}

CallItemGetInfo::CallItemGetInfo() {
	/*  0  */
	itemID = 0;
}

CallItemGetInfo::~CallItemGetInfo() {
	/*  0  */
}

void CallItemGetInfo::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sCallItemGetInfo", pfx);
	/*  0  */
	_log(l_type, "%sitemID=%lu", pfx, itemID);
}

PyRepTuple *CallItemGetInfo::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(1);
	/*  0  */
	tuple0->items[0] = new PyRepInteger(itemID);
	res = tuple0;
	

	return(res);
}

bool CallItemGetInfo::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool CallItemGetInfo::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode CallItemGetInfo failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 1) {
		_log(NET__PACKET_ERROR, "Decode CallItemGetInfo failed: tuple0 is the wrong size: expected 1, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	/*  0  */
	if(!tuple0->items[0]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode CallItemGetInfo failed: itemID is not an int: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_1 = (PyRepInteger *) tuple0->items[0];
	if(int_1->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode CallItemGetInfo: truncating 64 bit into into 32 bit int for field itemID");
	}
	itemID = int_1->value;

	delete packet;
	return(true);
}

CallItemGetInfo *CallItemGetInfo::Clone() const {
	CallItemGetInfo *res = new CallItemGetInfo;
	/*  0  */
	res->itemID = itemID;

	return(res);
}

RspItemGetInfo::RspItemGetInfo() {
	/*  0  */
	row = NULL;
}

RspItemGetInfo::~RspItemGetInfo() {
	/*  0  */
	delete row;
}

void RspItemGetInfo::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sRspItemGetInfo", pfx);
	/*  0  */
	_log(l_type, "%srow:", pfx);
	std::string row_n(pfx);
	row_n += "    ";
	row->Dump(l_type, row_n.c_str());
}

PyRepTuple *RspItemGetInfo::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(1);
	/*  0  */
	PyRep *ss_1;
	ss_1 = row->Encode();
	tuple0->items[0] = new PyRepSubStream(ss_1);
	res = tuple0;
	

	return(res);
}

bool RspItemGetInfo::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool RspItemGetInfo::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode RspItemGetInfo failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 1) {
		_log(NET__PACKET_ERROR, "Decode RspItemGetInfo failed: tuple0 is the wrong size: expected 1, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	/*  0  */
	if(!tuple0->items[0]->CheckType(PyRep::SubStream)) {
		_log(NET__PACKET_ERROR, "Decode RspItemGetInfo failed: ss_1 is not a substream: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}	
	PyRepSubStream *ss_1 = (PyRepSubStream *) tuple0->items[0];
	//make sure its decoded
	ss_1->DecodeData();
	if(ss_1->decoded == NULL) {
		_log(NET__PACKET_ERROR, "Decode RspItemGetInfo failed: Unable to decode ss_1");
		delete packet;
		return(false);
	}
	
	PyRep *rep_2 = ss_1->decoded;
	ss_1->decoded = NULL;
	delete row;
	row = new util_Row;
	if(!row->Decode(&rep_2)) {
		_log(NET__PACKET_ERROR, "Decode RspItemGetInfo failed: unable to decode element row");
		delete packet;
		return(false);
	}
	

	delete packet;
	return(true);
}

RspItemGetInfo *RspItemGetInfo::Clone() const {
	RspItemGetInfo *res = new RspItemGetInfo;
	/*  0  */
	res->row = row->Clone();

	return(res);
}

