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
#ifndef __PYDUMPVISITOR_H_INCL__
#define __PYDUMPVISITOR_H_INCL__

#include "PyVisitor.h"
#include "logsys.h"
#include <stdio.h>
#include <string>
#include <stack>

class PyDumpVisitor
: public PyVisitor {
public:
	PyDumpVisitor(bool full_lists);
	virtual ~PyDumpVisitor();
	
protected:
	virtual void _print(const char *str, ...) = 0;
	virtual void _hexDump(const byte *bytes, uint32 len) = 0;
	
	const bool m_full_lists;
	
	std::stack<const char *> m_indentStack;
	
	inline const char *top() const { return(m_indentStack.top()); }
	inline void pop() { m_indentStack.pop(); }
	inline void push(const char *v) { m_indentStack.push(v); }
	
	virtual void VisitInteger(const PyRepInteger *rep);
	virtual void VisitReal(const PyRepReal *rep);
	virtual void VisitBoolean(const PyRepBoolean *rep);
	virtual void VisitNone(const PyRepNone *rep);
	virtual void VisitBuffer(const PyRepBuffer *rep);
	virtual void VisitString(const PyRepString *rep);
	
	virtual void VisitObject(const PyRepObject *rep);
	
	virtual void VisitPacked(const PyRepPackedRow *rep);
	virtual void VisitPackedRowHeader(const PyRepPackedRowHeader *rep);
	virtual void VisitPackedRowHeaderElement(const PyRepPackedRowHeader *rep, uint32 index, const PyRepPackedRow *element);
	virtual void VisitPackedResultSet(const PyRepPackedResultSet *rep);
	virtual void VisitPackedResultSetElement(const PyRepPackedResultSet *rep, uint32 index, const PyRepPackedRow *element);
	
	virtual void VisitSubStruct(const PyRepSubStruct *rep);
	virtual void VisitSubStream(const PyRepSubStream *rep);
	
	virtual void VisitDict(const PyRepDict *rep);
	virtual void VisitDictElement(const PyRepDict *rep, uint32 index, const PyRep *key, const PyRep *value);
	
	virtual void VisitList(const PyRepList *rep);
	virtual void VisitListElement(const PyRepList *rep, uint32 index, const PyRep *ele);
	
	virtual void VisitTuple(const PyRepTuple *rep);
	virtual void VisitTupleElement(const PyRepTuple *rep, uint32 index, const PyRep *ele);

};

class PyLogsysDump : public PyDumpVisitor {
public:
	PyLogsysDump(LogType type, bool full_hex = false, bool full_lists = false);
	PyLogsysDump(LogType type, LogType hex_type, bool full_hex, bool full_lists);
	virtual ~PyLogsysDump() { }

protected:
	//overloaded for speed enhancements when disabled
	virtual void VisitDict(const PyRepDict *rep);
	virtual void VisitList(const PyRepList *rep);
	virtual void VisitTuple(const PyRepTuple *rep);
	virtual void VisitSubStream(const PyRepSubStream *rep);
	
	const LogType m_type;
	const LogType m_hex_type;
	const bool m_full_hex;
	virtual void _print(const char *str, ...);
	virtual void _hexDump(const byte *bytes, uint32 len);
};

class PyFileDump : public PyDumpVisitor {
public:
	PyFileDump(FILE *into, bool full_hex = false);
	virtual ~PyFileDump() { }

protected:
	FILE *const m_into;
	const bool m_full_hex;
	virtual void _print(const char *str, ...);
	virtual void _hexDump(const byte *bytes, uint32 len);
	void _pfxHexDump(const byte *bytes, uint32 len);
};

#endif



