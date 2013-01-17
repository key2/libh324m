/*
 * avnencoder.cxx
 *
 * Copyright (c) 2001 Institute for Information Industry, Taiwan, Republic of China 
 * (http://www.iii.org.tw/iiia/ewelcome.htm)
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is III ASN.1 Tool
 *
 * The Initial Developer of the Original Code is Institute for Information Industry.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): 
 *
 * $Log: AVNEncoder.cxx,v $
 * Revision 1.6  2005/12/02 03:12:16  mangelo
 * Disable VC2005 deprecated CRT functions warnings
 *
 * Revision 1.5  2004/12/10 19:53:10  btrummer
 * Removed unnecessary strm.good() calls inside the do_visit() methods.
 * Write appropriate string values, if a CHOICE has no valid selection,
 * instead of marking strm as "bad" (by returning false).
 *
 * Revision 1.4  2003/03/21 06:37:52  btrummer
 * IntegerWithNamedNumber::getName() and  ENUMERATED::getName()
 * must not be embedded inside an #ifdef ASN1_HAS_IOSTREAM.
 *
 * Revision 1.3  2002/07/02 04:45:13  mangelo
 * Modify for VC.Net and GCC 3.1
 *
 * Revision 1.2  2001/10/05 19:02:02  mangelo
 * Added Log
 *
 * 2001/06/26 Huang-Ming Huang 
 * Version 2.1 Reimplemented to minimize the code size.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if _MSC_VER >=1400
#pragma warning(disable:4996)
#endif

#include "asn1.h"
#ifdef ASN1_HAS_IOSTREAM
#include <boost/smart_ptr.hpp>
#include <iomanip>
#include "ios_helper.h"
#endif

namespace ASN1 {

#ifdef ASN1_HAS_IOSTREAM

std::string AbstractData::asValueNotation() const
{
    std::ostringstream strm;
    AVNEncoder encoder(strm);
    accept(encoder);
    return strm.str();
}

std::ios_base::iostate AbstractData::print_on(std::ostream & strm) const
{
    AVNEncoder encoder(strm);
    if (accept(encoder))
        return std::ios_base::goodbit;
    return std::ios_base::failbit;
}

std::ostream & operator<<(std::ostream & os, const AbstractData & arg)
{
    if (!os.good()) return os;
    return g_inserter(os, arg);
}

#endif //ASN1_HAS_IOSTREAM

bool IntegerWithNamedNumber::getName(std::string& str) const
{
	const NameEntry* begin = info()->nameEntries;
	const NameEntry* end   = begin+ info()->entryCount;
	const NameEntry* i = std::lower_bound(begin, end, value, NameEntryCmp());

	if (i != end && i->value == value)
	{
		str = i->name;
		return true;
	}
	return false;
}

const char* ENUMERATED::getName() const
{
	if (value >=0 && value <= getMaximum())
		return info()->names[value];
	return 0;
}

#ifdef ASN1_HAS_IOSTREAM

bool AVNEncoder::do_visit(const Null& value)
{
	strm << "NULL";
	return strm.good();
}

bool AVNEncoder::do_visit(const BOOLEAN& value)
{
	if (value) 
		strm << "TRUE";
	else
		strm << "FALSE";
	return strm.good();
}

bool AVNEncoder::do_visit(const INTEGER& value)
{
	if (!value.constrained() || value.getLowerLimit() < 0)
		strm << (int)value.getValue();
	else
		strm << (unsigned)value.getValue();
	return strm.good();
}

bool AVNEncoder::do_visit(const IntegerWithNamedNumber& value)
{
	std::string str;
	if (value.getName(str))
		strm << str;
	else 
		return visit(static_cast<const INTEGER&>(value));
	return strm.good();
}

bool AVNEncoder::do_visit(const ENUMERATED& value)
{
	const char* name = value.getName();
	if (name != 0)
		strm << name;
	else 
		strm << value.asInt();
	return strm.good();
}

bool AVNEncoder::do_visit(const OBJECT_IDENTIFIER& value)
{
	strm << "{ ";
	for (unsigned i = 0; i < value.levels(); ++i) 
		strm << value[i] << ' ';
	strm << "}";
	return strm.good();
}

bool AVNEncoder::do_visit(const BIT_STRING& value)
{
	strm << '\'';
	for (unsigned i = 0; i < value.size(); ++i)
		if (value[i])
			strm << '1';
		else
			strm << '0';
	strm << "\'B";
	return strm.good();
}

bool AVNEncoder::do_visit(const OCTET_STRING& value)
{
	std::ios_base::fmtflags flags = strm.flags();
	strm << '\'';
	
	for (unsigned i = 0; i < value.size(); ++i)
	{
		strm << std::hex << std::setw(2) << std::setfill('0') << (0xFF & value[i]);
		if (i != value.size()-1) 
			strm << ' ';
	}
	
	strm << "\'H";
	strm.setf(flags);
	strm << std::setfill(' ');
	return strm.good();
}

bool AVNEncoder::do_visit(const AbstractString& value)
{
	strm << '\"' << static_cast<const std::string&>(value) << '\"';
	return strm.good();
}

bool AVNEncoder::do_visit(const BMPString& value)
{
	boost::scoped_array<char> tmp(new char[value.size()*2+1]);
	int len = wcstombs(tmp.get(), &*value.begin(), value.size());
	if (len != -1)
	{
		tmp[len] = 0;
		strm << '\"' << tmp.get() << '\"';
	}
	else // output Quadruple form
	{
		strm << '{';
		for (unsigned i = 0; i < value.size(); ++i)
		{
			strm << "{ 0, 0, " << (int) (value[i] >> 8) << ", " << (int) value[i]  << '}';
			if (i != value.size()-1)
				strm << ", ";
		}
		strm << '}';
	}
	return strm.good();
}

bool AVNEncoder::do_visit(const CHOICE& value)
{
	if (value.currentSelection() == CHOICE::unknownSelection_)
	{
		strm << "<unknown selection>";
	}
	else if (value.currentSelection() == CHOICE::unselected_)
	{
		strm << "<unselected>";
	}
	else if (value.currentSelection() >= 0)
	{
		strm << value.getSelectionName() << " : ";
		return value.getSelection()->accept(*this);
	}
	return strm.good();
}

bool AVNEncoder::do_visit(const OpenData& value)
{
	if (value.has_data())
	{
		return value.get_data().accept(*this);
	}
	else if (value.has_buf())
	{
		OCTET_STRING ostr(value.get_buf());
		return ostr.accept(*this);
	}
	return false;
}

bool AVNEncoder::do_visit(const GeneralizedTime& value)
{
	strm << '\"' << value.get() << '\"';
	return strm.good();
}

bool AVNEncoder::do_visit(const SEQUENCE_OF_Base& value)
{
	strm << "{\n";
    SEQUENCE_OF_Base::const_iterator first = value.begin(), last = value.end();
    indent +=2;
	for (; first != last; ++first)
	{
		strm << std::setw(indent) << " ";
		if (!(*first)->accept(*this))
			return false;
		if (first != last-1)
			strm << ",\n";
	}
    indent -=2;
	if (value.size())
		strm << '\n';
	strm << std::setw(indent+1) << "}";
	return strm.good();
}

bool AVNEncoder::preVisitExtensionRoots(const SEQUENCE& value) 
{
	outputSeparators.push_back(false);
	strm << "{\n";
	return strm.good();
}

bool AVNEncoder::visitExtensionRoot(const SEQUENCE& value, int index)
{
	if (outputSeparators.back())
		strm << ",\n";
	strm << std::setw(strlen(value.getFieldName(index)) + indent +2) 
		<< value.getFieldName(index) << ' ';
    indent +=2;
	if (!value.getField(index)->accept(*this))
		return false;
    indent -=2;
	outputSeparators.back() = true;
	return strm.good();
}

bool AVNEncoder::visitKnownExtension(const SEQUENCE& value, int index)
{
	return visitExtensionRoot(value, index);
}

bool AVNEncoder::afterVisitSequence(const SEQUENCE& value)
{
	if (outputSeparators.back())
		strm  << '\n';
	outputSeparators.pop_back();
	strm << std::setw(indent + 1) << "}";
	return strm.good();
}

#endif //ASN1_HAS_IOSTREAM

}
