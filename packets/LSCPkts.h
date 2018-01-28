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

#ifndef _____packets_LSCPkts_h__
#define _____packets_LSCPkts_h__

#include <string>
#include <vector>
#include <map>
#include "../common/common.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"


#include "General.h"
#include "ObjectCaching.h"

class LSCChannelMultiDesc {
public:
	LSCChannelMultiDesc();
	~LSCChannelMultiDesc();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	LSCChannelMultiDesc *Clone() const;
	void CloneFrom(const LSCChannelMultiDesc *from);
	
	std::string	type;
	uint32		number;


};


class RspGetChannels {
public:
	RspGetChannels();
	~RspGetChannels();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	RspGetChannels *Clone() const;
	void CloneFrom(const RspGetChannels *from);
	
	/*  0  */
	objectCaching_CachedMethodCallResult	cachedcall;


};


class CallJoinChannels {
public:
	CallJoinChannels();
	~CallJoinChannels();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	CallJoinChannels *Clone() const;
	void CloneFrom(const CallJoinChannels *from);
	
	/*  0  */
	std::vector<uint32>	channels;


};


class RspJoinChannels {
public:
	RspJoinChannels();
	~RspJoinChannels();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	RspJoinChannels *Clone() const;
	void CloneFrom(const RspJoinChannels *from);
	
	/*  0  */
	/*  list of complex elements, one for each requested channel
	    with details about the channel  */
	PyRepList	channels;


};


class OnLSC_SenderInfo {
public:
	OnLSC_SenderInfo();
	~OnLSC_SenderInfo();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	OnLSC_SenderInfo *Clone() const;
	void CloneFrom(const OnLSC_SenderInfo *from);
	
	/*  0: whoAllianceID  */
	/*  1: whoCorpID  */
	uint32		corpID;
	/*  2: who (could be this list, or could be just an integer ID)  */
	uint32		senderID;
	std::string	senderName;
	uint32		senderType;
	/*  3: whoRole  */
	uint32		role;
	/*  4: whoCorpRole  */
	uint32		corp_role;


};


class NotifyOnLSC {
public:
	NotifyOnLSC();
	~NotifyOnLSC();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	NotifyOnLSC *Clone() const;
	void CloneFrom(const NotifyOnLSC *from);
	
	/*  0: channel ID, either an int, or an LSCChannelMultiDesc   */
	PyRep		*channelID;
	/*  1: estimated member count  */
	uint32		member_count;
	/*  2  */
	std::string	method;
	/*  3  */
	OnLSC_SenderInfo	*sender;
	/*  4  */
	PyRepTuple	*arguments;	//due to (somewhat false) sizing constructor, must be dynamic memory.


};


class OnLSC_SendMessage {
public:
	OnLSC_SendMessage();
	~OnLSC_SendMessage();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	OnLSC_SendMessage *Clone() const;
	void CloneFrom(const OnLSC_SendMessage *from);
	
	/*  0: channel ID, either an int, or an LSCChannelMultiDesc   */
	PyRep		*channelID;
	/*  1  */
	uint32		member_count;
	/*  2  */
	std::string	method;
	/*  3: sender info  */
	OnLSC_SenderInfo	*sender;
	/*  4  */
	/*  0  */
	std::string	message;


};


class OnLSC_JoinChannel {
public:
	OnLSC_JoinChannel();
	~OnLSC_JoinChannel();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	OnLSC_JoinChannel *Clone() const;
	void CloneFrom(const OnLSC_JoinChannel *from);
	
	/*  0: channel ID, either an int, or an LSCChannelMultiDesc   */
	PyRep		*channelID;
	/*  1  */
	uint32		member_count;
	/*  2  */
	std::string	method;
	/*  3: sender info  */
	OnLSC_SenderInfo	*sender;
	/*  4  */


};


class OnLSC_LeaveChannel {
public:
	OnLSC_LeaveChannel();
	~OnLSC_LeaveChannel();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	OnLSC_LeaveChannel *Clone() const;
	void CloneFrom(const OnLSC_LeaveChannel *from);
	
	/*  0: channel ID, either an int, or an LSCChannelMultiDesc   */
	PyRep		*channelID;
	/*  1  */
	uint32		member_count;
	/*  2  */
	std::string	method;
	/*  3: sender info  */
	OnLSC_SenderInfo	*sender;
	/*  4  */


};


class CallLeaveChannels {
public:
	CallLeaveChannels();
	~CallLeaveChannels();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	CallLeaveChannels *Clone() const;
	void CloneFrom(const CallLeaveChannels *from);
	
	/*  0  */
	PyRepList	channels;
	/*  1  */
	uint32		unsubscribe;


};


class CallLeaveChannel {
public:
	CallLeaveChannel();
	~CallLeaveChannel();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	CallLeaveChannel *Clone() const;
	void CloneFrom(const CallLeaveChannel *from);
	
	/*  0  */
	PyRep		*channel;
	/*  1  */
	uint32		unsubscribe;


};


class Call_Page {
public:
	Call_Page();
	~Call_Page();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	Call_Page *Clone() const;
	void CloneFrom(const Call_Page *from);
	
	std::vector<uint32>	recipients;
	std::string	subject;
	std::string	body;


};


class NotifyOnMessage {
public:
	NotifyOnMessage();
	~NotifyOnMessage();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	NotifyOnMessage *Clone() const;
	void CloneFrom(const NotifyOnMessage *from);
	
	std::vector<uint32>	recipients;
	/*  1: messageID: this might be a more complex type in the future.   */
	uint32		messageID;
	uint32		senderID;
	std::string	subject;
	uint64		sentTime;
	/*  not sure _exactly_ what timestamp this should be  */


};


class Call_DeleteMessages {
public:
	Call_DeleteMessages();
	~Call_DeleteMessages();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	Call_DeleteMessages *Clone() const;
	void CloneFrom(const Call_DeleteMessages *from);
	
	/*  most likely this could be an LSCChannelMultiDesc too...  */
	uint32		channelID;
	std::vector<uint32>	messages;


};


class Rsp_GetEVEMailDetails {
public:
	Rsp_GetEVEMailDetails();
	~Rsp_GetEVEMailDetails();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	Rsp_GetEVEMailDetails *Clone() const;
	void CloneFrom(const Rsp_GetEVEMailDetails *from);
	
	/*  first the actul message  */
	/* object of type util.Row */
	std::string	head_messageID;
	std::string	head_senderID;
	std::string	head_subject;
	std::string	head_body;
	std::string	head_created;
	std::string	head_mimeType;
	std::string	head_channelID;
	std::string	head_deleted;
	uint32		messageID;
	uint32		senderID;
	std::string	subject;
	std::string	body;
	uint64		created;
	/*  mimeType is a nested row... silly CCP  */
	/* object of type util.Row */
	std::string	head_mimeTypeID;
	std::string	head_mimeType2;
	std::string	head_binary;
	uint32		mimeTypeID;
	std::string	mimeType;
	uint32		binary;
	uint32		channelID;
	uint32		deleted;
	/*  then any attachments. this is a util.Rowset  */
	/*  this might not work due to a type1 string issue in rowclass here,
      if that is the case, then change this over to a <list> temporarily  */
	util_Rowset	attachments;


};


class Call_LookupStringInt {
public:
	Call_LookupStringInt();
	~Call_LookupStringInt();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	Call_LookupStringInt *Clone() const;
	void CloneFrom(const Call_LookupStringInt *from);
	
	std::string	searchString;
	uint32		searchOption;


};


class Call_LookupIntString {
public:
	Call_LookupIntString();
	~Call_LookupIntString();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	Call_LookupIntString *Clone() const;
	void CloneFrom(const Call_LookupIntString *from);
	
	uint32		searchOption;
	std::string	searchString;


};



#endif

