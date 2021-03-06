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
#include "..\packets\Wallet.h"
#include "../common/PyRep.h"




OnAccountChange::OnAccountChange() {
	accountKey = "";
	ownerid = 0;
	balance = 0.0;
}

OnAccountChange::~OnAccountChange() {
}

void OnAccountChange::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sOnAccountChange", pfx);
	_log(l_type, "%saccountKey='%s'", pfx, accountKey.c_str());
	_log(l_type, "%sownerid=%lu", pfx, ownerid);
	_log(l_type, "%sbalance=%.13f", pfx, balance);
}

PyRepTuple *OnAccountChange::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(3);
	tuple0->items[0] = new PyRepString(accountKey);
	tuple0->items[1] = new PyRepInteger(ownerid);
	tuple0->items[2] = new PyRepReal(balance);
	res = tuple0;
	

	return(res);
}

PyRepTuple *OnAccountChange::FastEncode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(3);
	tuple0->items[0] = new PyRepString(accountKey);
	tuple0->items[1] = new PyRepInteger(ownerid);
	tuple0->items[2] = new PyRepReal(balance);
	res = tuple0;
	

	return(res);
}

bool OnAccountChange::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool OnAccountChange::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode OnAccountChange failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 3) {
		_log(NET__PACKET_ERROR, "Decode OnAccountChange failed: tuple0 is the wrong size: expected 3, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	if(!tuple0->items[0]->CheckType(PyRep::String)) {
		_log(NET__PACKET_ERROR, "Decode OnAccountChange failed: accountKey is not a string: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepString *string_1 = (PyRepString *) tuple0->items[0];
	accountKey = string_1->value;
	if(!tuple0->items[1]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode OnAccountChange failed: ownerid is not an int: %s", tuple0->items[1]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_2 = (PyRepInteger *) tuple0->items[1];
	if(int_2->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode OnAccountChange: truncating 64 bit into into 32 bit int for field ownerid");
	}
	ownerid = int_2->value;
	if(!tuple0->items[2]->CheckType(PyRep::Real)) {
		_log(NET__PACKET_ERROR, "Decode OnAccountChange failed: balance is not a real: %s", tuple0->items[2]->TypeString());
		delete packet;
		return(false);
	}
	PyRepReal *real_3 = (PyRepReal *) tuple0->items[2];
	balance = real_3->value;

	delete packet;
	return(true);
}

OnAccountChange *OnAccountChange::Clone() const {
	OnAccountChange *res = new OnAccountChange;
	res->CloneFrom(this);
	return(res);
}

void OnAccountChange::CloneFrom(const OnAccountChange *from) {
	accountKey = from->accountKey;
	ownerid = from->ownerid;
	balance = from->balance;
	
}


Call_GiveCash::Call_GiveCash() {
	destination = 0;
	amount = 0.0;
	reason = "";
}

Call_GiveCash::~Call_GiveCash() {
}

void Call_GiveCash::Dump(LogType l_type, const char *pfx) const {
	_log(l_type, "%sCall_GiveCash", pfx);
	_log(l_type, "%sdestination=%lu", pfx, destination);
	_log(l_type, "%samount=%.13f", pfx, amount);
	_log(l_type, "%sreason='%s'", pfx, reason.c_str());
}

PyRepTuple *Call_GiveCash::Encode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(3);
	tuple0->items[0] = new PyRepInteger(destination);
	tuple0->items[1] = new PyRepReal(amount);
	tuple0->items[2] = new PyRepString(reason);
	res = tuple0;
	

	return(res);
}

PyRepTuple *Call_GiveCash::FastEncode() {
	PyRepTuple *res = NULL;
	PyRepTuple *tuple0 = new PyRepTuple(3);
	tuple0->items[0] = new PyRepInteger(destination);
	tuple0->items[1] = new PyRepReal(amount);
	tuple0->items[2] = new PyRepString(reason);
	res = tuple0;
	

	return(res);
}

bool Call_GiveCash::Decode(PyRepTuple **in_packet) {
	//quick forwarder to avoid making the user cast it if they have a properly typed object
	PyRep *packet = *in_packet;
	*in_packet = NULL;
	return(Decode(&packet));
}

bool Call_GiveCash::Decode(PyRep **in_packet) {
	PyRep *packet = *in_packet;
	*in_packet = NULL;

	if(!packet->CheckType(PyRep::Tuple)) {
		_log(NET__PACKET_ERROR, "Decode Call_GiveCash failed: tuple0 is the wrong type: %s", packet->TypeString());
		delete packet;
		return(false);
	}
	PyRepTuple *tuple0 = (PyRepTuple *) packet;
	if(tuple0->items.size() != 3) {
		_log(NET__PACKET_ERROR, "Decode Call_GiveCash failed: tuple0 is the wrong size: expected 3, but got %d", tuple0->items.size());
		delete packet;
		return(false);
	}

	if(!tuple0->items[0]->CheckType(PyRep::Integer)) {
		_log(NET__PACKET_ERROR, "Decode Call_GiveCash failed: destination is not an int: %s", tuple0->items[0]->TypeString());
		delete packet;
		return(false);
	}
	PyRepInteger *int_1 = (PyRepInteger *) tuple0->items[0];
	if(int_1->value > 0xFFFFFFFF) {
		_log(NET__PACKET_WARNING, "Decode Call_GiveCash: truncating 64 bit into into 32 bit int for field destination");
	}
	destination = int_1->value;
	if(!tuple0->items[1]->CheckType(PyRep::Real)) {
		_log(NET__PACKET_ERROR, "Decode Call_GiveCash failed: amount is not a real: %s", tuple0->items[1]->TypeString());
		delete packet;
		return(false);
	}
	PyRepReal *real_2 = (PyRepReal *) tuple0->items[1];
	amount = real_2->value;
	if(!tuple0->items[2]->CheckType(PyRep::String)) {
		_log(NET__PACKET_ERROR, "Decode Call_GiveCash failed: reason is not a string: %s", tuple0->items[2]->TypeString());
		delete packet;
		return(false);
	}
	PyRepString *string_3 = (PyRepString *) tuple0->items[2];
	reason = string_3->value;

	delete packet;
	return(true);
}

Call_GiveCash *Call_GiveCash::Clone() const {
	Call_GiveCash *res = new Call_GiveCash;
	res->CloneFrom(this);
	return(res);
}

void Call_GiveCash::CloneFrom(const Call_GiveCash *from) {
	destination = from->destination;
	amount = from->amount;
	reason = from->reason;
	
}



