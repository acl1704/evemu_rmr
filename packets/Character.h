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

#ifndef _____packets_Character_h__
#define _____packets_Character_h__

#include <string>
#include <vector>
#include <map>
#include "../common/common.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"


#include "General.h"

class RspGetCharactersToSelect {
public:
	RspGetCharactersToSelect();
	~RspGetCharactersToSelect();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	RspGetCharactersToSelect *Clone() const;
	void CloneFrom(const RspGetCharactersToSelect *from);
	
	/*  0  */
	util_Rowset	chars;


};


class CallGetCharacterToSelect {
public:
	CallGetCharacterToSelect();
	~CallGetCharacterToSelect();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	CallGetCharacterToSelect *Clone() const;
	void CloneFrom(const CallGetCharacterToSelect *from);
	
	/*  0  */
	uint32		charID;


};


class RspGetCharacterToSelect {
public:
	RspGetCharacterToSelect();
	~RspGetCharacterToSelect();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	RspGetCharacterToSelect *Clone() const;
	void CloneFrom(const RspGetCharacterToSelect *from);
	
	/*  0  */
	util_Rowset	character;


};


class RspGetCharCreationInfo {
public:
	RspGetCharCreationInfo();
	~RspGetCharCreationInfo();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	RspGetCharCreationInfo *Clone() const;
	void CloneFrom(const RspGetCharCreationInfo *from);
	
	/*  0  */
	std::map<std::string, PyRep *>	caches;


};


class CallSelectCharacterID {
public:
	CallSelectCharacterID();
	~CallSelectCharacterID();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	CallSelectCharacterID *Clone() const;
	void CloneFrom(const CallSelectCharacterID *from);
	
	/*  0  */
	uint32		charID;


};


class CallValidateName {
public:
	CallValidateName();
	~CallValidateName();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	CallValidateName *Clone() const;
	void CloneFrom(const CallValidateName *from);
	
	/*  0  */
	std::string	name;


};


class RspValidateName {
public:
	RspValidateName();
	~RspValidateName();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	RspValidateName *Clone() const;
	void CloneFrom(const RspValidateName *from);
	
	/*  0  */
	bool		result;


};


class CallCreateCharacter {
public:
	CallCreateCharacter();
	~CallCreateCharacter();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	CallCreateCharacter *Clone() const;
	void CloneFrom(const CallCreateCharacter *from);
	
	/*  0  */
	std::string	name;
	/*  1  */
	uint32		bloodlineID;
	/*  2  */
	uint32		genderID;
	/*  3  */
	uint32		ancestryID;
	/*  4  */
	uint32		schoolID;
	/*  5  */
	uint32		departmentID;
	/*  6  */
	uint32		fieldID;
	/*  7  */
	uint32		specialityID;
	/*  8  */
	uint32		IntelligenceAdd;
	/*  9  */
	uint32		CharismaAdd;
	/*  10  */
	uint32		PerceptionAdd;
	/*  11  */
	uint32		MemoryAdd;
	/*  12  */
	uint32		WillpowerAdd;
	/*  13  */
	std::map<std::string, PyRep *>	appearance;


};


class RspCreateCharacter {
public:
	RspCreateCharacter();
	~RspCreateCharacter();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	RspCreateCharacter *Clone() const;
	void CloneFrom(const RspCreateCharacter *from);
	
	/*  0  */
	uint32		charID;


};


class NotifyOnCharNowInStation {
public:
	NotifyOnCharNowInStation();
	~NotifyOnCharNowInStation();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	NotifyOnCharNowInStation *Clone() const;
	void CloneFrom(const NotifyOnCharNowInStation *from);
	
	/*  0  */
	uint32		charID;


};


class Call_SetNote {
public:
	Call_SetNote();
	~Call_SetNote();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	Call_SetNote *Clone() const;
	void CloneFrom(const Call_SetNote *from);
	
	uint32		itemID;
	std::string	note;


};


class Call_AddOwnerNote {
public:
	Call_AddOwnerNote();
	~Call_AddOwnerNote();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	Call_AddOwnerNote *Clone() const;
	void CloneFrom(const Call_AddOwnerNote *from);
	
	std::string	label;
	std::string	content;


};


class Call_EditOwnerNote {
public:
	Call_EditOwnerNote();
	~Call_EditOwnerNote();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	Call_EditOwnerNote *Clone() const;
	void CloneFrom(const Call_EditOwnerNote *from);
	
	uint32		noteID;
	std::string	label;
	std::string	content;


};



#endif


