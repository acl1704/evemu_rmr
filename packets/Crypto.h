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

#ifndef _____packets_Crypto_h__
#define _____packets_Crypto_h__

#include <string>
#include <vector>
#include <map>
#include "../common/common.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"


	/*  this is not being used at the moment, more hrer for completeness  */

class VersionExchange {
public:
	VersionExchange();
	~VersionExchange();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	VersionExchange *Clone() const;
	void CloneFrom(const VersionExchange *from);
	
	uint32		birthday;
	uint32		macho_version;
	uint32		user_count;
	double		version_number;
	uint32		build_version;
	std::string	project_version;


};


class CryptoRequestPacket {
public:
	CryptoRequestPacket();
	~CryptoRequestPacket();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	CryptoRequestPacket *Clone() const;
	void CloneFrom(const CryptoRequestPacket *from);
	
	/*  0  */
	std::string	keyVersion;
	/*  1  */
	PyRepDict	keyParams;


};


class CryptoAPIRequestPacket {
public:
	CryptoAPIRequestPacket();
	~CryptoAPIRequestPacket();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepTuple **packet);
	PyRepTuple *Encode();
	PyRepTuple *FastEncode();
	CryptoAPIRequestPacket *Clone() const;
	void CloneFrom(const CryptoAPIRequestPacket *from);
	
	/*  0  */
	std::string	keyVersion;
	/*  1  */
	std::string	sessionkey;
	std::string	hashmethod;
	uint32		sessionkeylength;
	std::string	provider;
	std::string	sessionkeymethod;


};


class CryptoChallengePacket {
public:
	CryptoChallengePacket();
	~CryptoChallengePacket();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepSubStream **packet);
	PyRepSubStream *Encode();
	PyRepSubStream *FastEncode();
	CryptoChallengePacket *Clone() const;
	void CloneFrom(const CryptoChallengePacket *from);
	
	/*  0  */
	std::string	clientChallenge;
	/*  1  */
	uint32		macho_version;
	double		boot_version;
	uint32		boot_build;
	std::string	boot_codename;
	std::string	boot_region;
	std::string	user_name;
	PyRep		*user_password;
	std::string	user_languageid;
	uint32		user_affiliateid;


};


class CryptoServerHandshake {
public:
	CryptoServerHandshake();
	~CryptoServerHandshake();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepSubStream **packet);
	PyRepSubStream *Encode();
	PyRepSubStream *FastEncode();
	CryptoServerHandshake *Clone() const;
	void CloneFrom(const CryptoServerHandshake *from);
	
	/*  0: serverChallenge  */
	std::string	serverChallenge;
	/*  1: func  */
	std::string	func;
	/*  2: context  */
	std::map<std::string, PyRep *>	context;
	/*  3: response  */
	std::string	challenge_responsehash;
	uint32		macho_version;
	double		boot_version;
	uint32		boot_build;
	std::string	boot_codename;
	std::string	boot_region;
	uint32		cluster_usercount;
	uint32		proxy_nodeid;
	uint32		user_logonqueueposition;


};


class CryptoHandshakeResult {
public:
	CryptoHandshakeResult();
	~CryptoHandshakeResult();
	
	void Dump(LogType type, const char *pfx = "") const;
	bool Decode(PyRep **packet);	//consumes packet
	bool Decode(PyRepSubStream **packet);
	PyRepSubStream *Encode();
	PyRepSubStream *FastEncode();
	CryptoHandshakeResult *Clone() const;
	void CloneFrom(const CryptoHandshakeResult *from);
	
	/*  0  */
	std::string	challenge_responsehash;
	/*  1  */
	std::string	func_output;
	/*  2  */
	uint32		func_result;


};



#endif


