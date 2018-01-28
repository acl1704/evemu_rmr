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


#include "PyVisitor.h"
#include "PyRep.h"

void PyVisitor::VisitInteger(const PyRepInteger *rep) { }
void PyVisitor::VisitReal(const PyRepReal *rep) { }
void PyVisitor::VisitBoolean(const PyRepBoolean *rep) { }
void PyVisitor::VisitNone(const PyRepNone *rep) { }
void PyVisitor::VisitBuffer(const PyRepBuffer *rep) { }
void PyVisitor::VisitString(const PyRepString *rep) { }

void PyVisitor::VisitObject(const PyRepObject *rep) {
	rep->arguments->visit(this);
}

void PyVisitor::VisitPacked(const PyRepPackedRow *rep) { }

void PyVisitor::VisitPackedRowHeader(const PyRepPackedRowHeader *rep) {
	rep->header_type->visit(this);
	rep->arguments->visit(this);
	
	PyRepPackedResultSet::const_iterator cur, end;
	uint32 r;
	
	cur = rep->rows.begin();
	end = rep->rows.end();
	for(r = 0; cur != end; cur++, r++) {
		VisitPackedRowHeaderElement(rep, r, *cur);
	}
}

void PyVisitor::VisitPackedRowHeaderElement(const PyRepPackedRowHeader *rep, uint32 index, const PyRepPackedRow *element) {
	element->visit(this);
}


void PyVisitor::VisitPackedResultSet(const PyRepPackedResultSet *rep) {
	rep->header->visit(this);
	
	PyRepPackedResultSet::const_iterator cur, end;
	cur = rep->begin();
	end = rep->end();
	uint32 r;
	for(r = 0; cur != end; cur++, r++) {
		VisitPackedResultSetElement(rep, r, *cur);
	}
}

void PyVisitor::VisitPackedResultSetElement(const PyRepPackedResultSet *rep, uint32 index, const PyRepPackedRow *element) {
	element->visit(this);
}


void PyVisitor::VisitSubStruct(const PyRepSubStruct *rep) {
	rep->sub->visit(this);
}

void PyVisitor::VisitSubStream(const PyRepSubStream *rep) {
	if(rep->decoded != NULL)
		rep->decoded->visit(this);
}

void PyVisitor::VisitChecksumedStream(const PyRepChecksumedStream *rep) {
	rep->stream->visit(this);
}

void PyVisitor::VisitDict(const PyRepDict *rep) {
	
	PyRepDict::const_iterator cur, end;
	cur = rep->begin();
	end = rep->end();
	uint32 r;
	for(r = 0; cur != end; cur++, r++) {
		VisitDictElement(rep, r, cur->first, cur->second);
	}
}

void PyVisitor::VisitDictElement(const PyRepDict *rep, uint32 index, const PyRep *key, const PyRep *value) {
	key->visit(this);
	value->visit(this);
}

void PyVisitor::VisitList(const PyRepList *rep) {
	PyRepList::const_iterator cur, end;
	cur = rep->begin();
	end = rep->end();
	uint32 r;
	for(r = 0; cur != end; cur++, r++) {
		VisitListElement(rep, r, *cur);
	}
}

void PyVisitor::VisitListElement(const PyRepList *rep, uint32 index, const PyRep *ele) {
	ele->visit(this);
}

void PyVisitor::VisitTuple(const PyRepTuple *rep) {
	PyRepTuple::const_iterator cur, end;
	cur = rep->begin();
	end = rep->end();
	uint32 r;
	for(r = 0; cur != end; cur++, r++) {
		VisitTupleElement(rep, r, *cur);
	}
}

void PyVisitor::VisitTupleElement(const PyRepTuple *rep, uint32 index, const PyRep *ele) {
	ele->visit(this);
}

void SubStreamDecoder::VisitSubStream(const PyRepSubStream *rep) {
	bool was_null = (rep->decoded == NULL);
	rep->DecodeData();
	if(was_null && rep->decoded != NULL)
		rep->decoded->visit(this);	//recurse
}





