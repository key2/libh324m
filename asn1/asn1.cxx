/*
 * asn1.cxx
 *
 * Abstract Syntax Notation 1 Encoding Rules
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): Huang-Ming Huang 
 *
 * The code is adapted from asner.cxx of PWLib, but the dependancy on PWLib has
 * been removed.
 *
 * $Log: asn1.cxx,v $
 * Revision 1.16  2006/12/26 17:06:39  mangelo
 * Avoid dereferencing the end() iterator which causes problem in MSVC debug mode
 *
 * Revision 1.15  2005/12/02 03:12:16  mangelo
 * Disable VC2005 deprecated CRT functions warnings
 *
 * Revision 1.14  2005/09/07 17:00:24  btrummer
 * Split up all constructors taking a "const void* info = &theInfo"
 * and made all info-constructors protected.
 *
 * Revision 1.13  2005/08/31 14:00:30  btrummer
 * Applied some changes to the parser and the asn1 library to make BER en-
 * and decoding work correctly (or let's say, more correct than before. ;-)
 * (Thanks to Harald Okorn)
 *
 * Revision 1.12  2003/07/24 11:00:50  btrummer
 * Fixed Visitor::visit(SEQUENCE&): If there are no extensions to visit,
 * the extensionMap is cleared. This way, no errors will occur, when
 * the message is encoded again.
 *
 * Revision 1.11  2003/03/04 08:30:29  btrummer
 * Removed unneeded comma in the TagClass enum.
 *
 * Revision 1.10  2002/11/04 13:21:02  btrummer
 * Fixed the initialization of the members millisec, utc and mindiff
 * in GeneralizedTime::set(const char*).
 *
 * Revision 1.9  2002/09/30 06:10:38  btrummer
 * Removed the assert statement in the ENUMERATED constructor,
 * since 0 might be a valid maximum value.
 *
 * Revision 1.8  2002/09/20 08:12:02  btrummer
 * Replaced some assert() statements with "if (...) return false;".
 *
 * Revision 1.7  2002/07/03 06:01:59  btrummer
 * Fixed two buffer overflows in OBJECT_IDENTIFIER::decodeCommon(),
 * that may occur, when decoding invalid (random) data.
 *
 * Revision 1.6  2002/07/02 04:45:13  mangelo
 * Modify for VC.Net and GCC 3.1
 *
 * Revision 1.5  2002/01/11 05:43:12  mangelo
 * Fixed OBJECT_IDENTIFIER::decodeCommon (Thanks to George Biro)
 *
 * Revision 1.4  2001/10/09 16:39:11  mangelo
 * fixed the problem when vector<char*>::iterator is not char*
 *
 * Revision 1.3  2001/10/05 19:15:11  mangelo
 * Change Log Sequence
 *
 * Revision 1.2  2001/10/05 19:00:37  mangelo
 * Added Log
 *
 * 2001/07/16 Huang-Ming Huang
 * Optional components of SEQUENCE is now created on demand.
 *
 * 2001/06/26 Huang-Ming Huang 
 * Version 2.1 Reimplemented to minimize the code size.
 *
 * 2001/05/01 Huang-Ming Huang 
 * Fixed problem with unconstrained PASN_NumericString coding in 8 bits
 *   instead of 4 in accordance with PWLib asner.cxx Revision 1.39.
 */

#if _MSC_VER < 1310
#pragma warning(disable: 4786)
#elif _MSC_VER >=1400
#pragma warning(disable:4996)
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <boost/cast.hpp>
#include <stdio.h>
#include "asn1.h"


#ifdef ASN1_HAS_IOSTREAM
#include "ios_helper.h"
#endif

namespace ASN1 {
    
    enum TagClass {
        UniversalTagClass =0,
            ApplicationTagClass = 0x40,
            ContextSpecificTagClass = 0x80,
            PrivateTagClass = 0xC0
    };
    
    enum UniversalTags {
        InvalidUniversalTag,
            UniversalBoolean,
            UniversalInteger,
            UniversalBitString,
            UniversalOctetString,
            UniversalNull,
            UniversalObjectId,
            UniversalObjectDescriptor,
            UniversalExternalType,
            UniversalReal,
            UniversalEnumeration,
            UniversalEmbeddedPDV,
            UniversalSequence = 16,
            UniversalSet,
            UniversalNumericString,
            UniversalPrintableString,
            UniversalTeletexString,
            UniversalVideotexString,
            UniversalIA5String,
            UniversalUTCTime,
            UniversalGeneralisedTime,
            UniversalGraphicString,
            UniversalVisibleString,
            UniversalGeneralString,
            UniversalUniversalString,
            UniversalBMPString = 30
    };

unsigned CountBits(unsigned range)
{
  if (range == 0)
    return sizeof(unsigned)*8;

  unsigned nBits = 0;
  while (nBits < (sizeof(unsigned)*8) && range > (unsigned)(1 << nBits))
    nBits++;
  return nBits;
}

template <class Iterator>
int lexicographic_compare_bytes(
    Iterator first1, Iterator last1, 
		Iterator first2, Iterator last2)
{
	int r;

	for (; first1 != last1 && first2 != last2; ++first1, ++first2)
	{
		r = *first1 - *first2;
		if (r != 0) return r;
	}

	if (first1 == last1 && first2 == last2) return 0;
	return (first1 != last1 ? 1 : -1);
}

/////////////////////////////////////////////////////////////////////


AbstractData::AbstractData(const void* information)
: info_(information)
{
}

////////////////////////////////////////////////////////////

const Null::InfoType Null::theInfo = { 
    Null::create, 
    UniversalTagClass << 16 | UniversalNull
};


AbstractData* Null::create(const void* info)
{
    return new Null(info);
}


bool Null::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool Null::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}

AbstractData * Null::do_clone() const
{
  assert(typeid(*this) == typeid(Null));
  return new Null(*this);
}


int Null::do_compare(const AbstractData& other) const
{
  assert(typeid(other) == typeid(Null));
  return 0;
}


/////////////////////////////////////////////////////////

const BOOLEAN::InfoType BOOLEAN::theInfo = {
    BOOLEAN::create,
    UniversalTagClass << 16 | UniversalBoolean 
};

AbstractData* BOOLEAN::create(const void* info)
{
    return new BOOLEAN(info);
}


bool BOOLEAN::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool BOOLEAN::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}

AbstractData * BOOLEAN::do_clone() const
{
  assert(typeid(*this) == typeid(BOOLEAN));
  return new BOOLEAN(*this);
}


int BOOLEAN::do_compare(const AbstractData& data) const
{
	const BOOLEAN& that = *boost::polymorphic_downcast<const BOOLEAN*>(&data);
	return value - that.value;
}


///////////////////////////////////////////////////////////////////////


const INTEGER::InfoType INTEGER::theInfo = {
    &INTEGER::create,
    UniversalTagClass << 16 | UniversalInteger,
    Unconstrained,
    0,
    UINT_MAX
};


AbstractData* INTEGER::create(const void* info)
{
    return new INTEGER(info);
}


bool INTEGER::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool INTEGER::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}


AbstractData * INTEGER::do_clone() const
{
  return new INTEGER(*this);
}

int INTEGER::do_compare(const AbstractData& data) const
{
	const INTEGER& that = *boost::polymorphic_downcast<const INTEGER*>(&data);
	if (getLowerLimit() >= 0)
		return value - that.value;
	else
		return (int) value - (int) that.value;
}

///////////////////////////////////////////////////////

AbstractData* IntegerWithNamedNumber::create(const void* info)
{
    return new IntegerWithNamedNumber(info);
}

bool IntegerWithNamedNumber::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool IntegerWithNamedNumber::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}


//////////////////////////////////////////////////////

ENUMERATED& ENUMERATED::operator = (const ENUMERATED& that) 
{
	assert(info() == that.info()); // compatible type check
	value = that.value;
	return *this;
}

bool ENUMERATED::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool ENUMERATED::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}

AbstractData * ENUMERATED::do_clone() const
{
  return new ENUMERATED(*this);
}


int ENUMERATED::do_compare(const AbstractData& other) const
{
	const ENUMERATED& that = *static_cast<const ENUMERATED*>(&other);
	assert(info_ == that.info_); // compatible type check
	return value - that.value;
}

AbstractData* ENUMERATED::create(const void* info)
{
    return new ENUMERATED(info);
}

///////////////////////////////////////////////////////////////////////


const OBJECT_IDENTIFIER::InfoType OBJECT_IDENTIFIER::theInfo = {
    OBJECT_IDENTIFIER::create,
    UniversalTagClass << 16 | UniversalObjectId
};


AbstractData* OBJECT_IDENTIFIER::create(const void* info)
{
    return new OBJECT_IDENTIFIER(info);
}


bool OBJECT_IDENTIFIER::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool OBJECT_IDENTIFIER::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}

bool OBJECT_IDENTIFIER::decodeCommon(const char* strm, unsigned dataLen)
{
  unsigned byteOffset=0;

  value.clear();

  // handle zero length strings correctly
  if (dataLen == 0)
  {
      return true;
  }

  unsigned subId;

  // Avoid reallocations in the while-loop below.
  value.reserve(dataLen+1);

  // start at the second identifier in the buffer, because we will later
  // expand the first number into the first two IDs
  value.push_back(0);

  while (dataLen > 0) {
    unsigned byte;
    subId = 0;
    do {    /* shift and add in low order 7 bits */
      if (dataLen == 0)
        return false;
      byte = strm[byteOffset++];
      subId = (subId << 7) + (byte & 0x7f);
      dataLen--;
    } while ((byte & 0x80) != 0);
    value.push_back(subId);
  }

  /*
   * The first two subidentifiers are encoded into the first component
   * with the value (X * 40) + Y, where:
   *  X is the value of the first subidentifier.
   *  Y is the value of the second subidentifier.
   */
  subId = value[1];
  if (subId < 40) {
    value[0] = 0;
    value[1] = subId;
  }
  else if (subId < 80) {
    value[0] = 1;
    value[1] = subId-40;
  }
  else {
    value[0] = 2;
    value[1] = subId-80;
  }


  return true;
}


void OBJECT_IDENTIFIER::encodeCommon(std::vector<char> & encodecObjectId) const
{
  unsigned length = value.size();
  const unsigned* objId = &value[0];

  if (length < 2) {
    // Thise case is really illegal, but we have to do SOMETHING
    encodecObjectId.resize(0);
    return;
  }

  unsigned subId = (objId[0] * 40) + objId[1];
  objId += 2;

  unsigned outputPosition = 0;
  encodecObjectId.reserve(length);
  std::insert_iterator<std::vector<char> > insertItr(encodecObjectId, encodecObjectId.begin());
  while (--length > 0) {
    if (subId < 128)
      *insertItr++ = (char)subId;
    else {
      unsigned mask = 0x7F; /* handle subid == 0 case */
      int bits = 0;

      /* testmask *MUST* !!!! be of an unsigned type */
      unsigned testmask = 0x7F;
      int      testbits = 0;
      while (testmask != 0) {
        if (subId & testmask) {  /* if any bits set */
          mask = testmask;
	        bits = testbits;
	      }
        testmask <<= 7;
        testbits += 7;
      }

      /* mask can't be zero here */
      while (mask != 0x7F) {
        /* fix a mask that got truncated above */
      	if (mask == 0x1E00000)
	        mask = 0xFE00000;

        *insertItr++ = (char)(((subId & mask) >> bits) | 0x80);

        mask >>= 7;
        bits -= 7;
      }

      *insertItr++ = (char)(subId & mask);
    }
	if (length >1)
	  subId = *objId++;
  }
}


AbstractData * OBJECT_IDENTIFIER::do_clone() const
{
  return new OBJECT_IDENTIFIER(*this);
}


int OBJECT_IDENTIFIER::do_compare(const AbstractData& other) const
{
	const OBJECT_IDENTIFIER& that = *boost::polymorphic_downcast<const OBJECT_IDENTIFIER*>(&other);
	int min_level = std::min(levels(), that.levels());
	for (int i = 0; i < min_level; ++i)
		if (value[i] != that.value[i])
			return value[i] - that.value[i];
	return levels() - that.levels();
}

///////////////////////////////////////////////////////////////////////

const BIT_STRING::InfoType BIT_STRING::theInfo = {
    BIT_STRING::create,
    UniversalTagClass << 16 | UniversalBitString,
    Unconstrained,
    0,
    UINT_MAX
};


AbstractData* BIT_STRING::create(const void* info)
{
    return new BIT_STRING(info);
}

bool BIT_STRING::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool BIT_STRING::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}

AbstractData* BIT_STRING::do_clone() const
{
	return new BIT_STRING(*this);
}

int BIT_STRING::do_compare(const AbstractData& other) const
{
	const BIT_STRING& that = *boost::polymorphic_downcast<const BIT_STRING*>(&other);
	int nBytes = std::min(bitData.size(), that.bitData.size());
	for (int i = 0 ; i < nBytes; ++i)
	{
		char mask = bitData[i] ^ that.bitData[i]; // find the first byte which differs
		if (mask != 0)
			return (bitData[i] & mask) - (that.bitData[i] & mask);
	}
	return totalBits - that.totalBits;
}

///////////////////////////////////////////////////////////////////////
const OCTET_STRING::InfoType OCTET_STRING::theInfo = {
    OCTET_STRING::create,
    UniversalTagClass << 16 | UniversalOctetString,
    Unconstrained,
    0,
    UINT_MAX
};


AbstractData* OCTET_STRING::create(const void* info)
{
    return new OCTET_STRING(info);
}

bool OCTET_STRING::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool OCTET_STRING::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}


AbstractData * OCTET_STRING::do_clone() const
{
  return new OCTET_STRING(*this);
}

int OCTET_STRING::do_compare(const AbstractData& other) const
{
	const OCTET_STRING& that = *boost::polymorphic_downcast<const OCTET_STRING*>(&other);
	return lexicographic_compare_bytes(begin(), end(), that.begin(), that.end());
}

///////////////////////////////////////////////////////////////////////

bool AbstractString::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool AbstractString::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}


AbstractData* AbstractString::do_clone() const
{
	return new AbstractString(*this);
}

int AbstractString::do_compare(const AbstractData& other) const
{
	const AbstractString& that = *boost::polymorphic_downcast<const AbstractString*>(&other);
	return base_string::compare(that);
}

AbstractData* AbstractString::create(const void* info)
{
    return new AbstractString(info);
}

/////////////////////////////////////////////////////////////////////////////


static const char NumericStringSet[]   = " 0123456789";
static const char PrintableStringSet[] = " '()+,-./0123456789:=?"
										  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
										  "abcdefghijklmnopqrstuvwxyz";
static const char VisibleStringSet[]   = " !\"#$%&'()*+,-./0123456789:;<=>?"
										  "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
										  "`abcdefghijklmnopqrstuvwxyz{|}~";
static const char IA5StringSet[]       = "\000\001\002\003\004\005\006\007"
										  "\010\011\012\013\014\015\016\017"
										  "\020\021\022\023\024\025\026\027"
										  "\030\031\032\033\034\035\036\037"
                                          " !\"#$%&'()*+,-./0123456789:;<=>?"
										  "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
										  "`abcdefghijklmnopqrstuvwxyz{|}~\177";
static const char GeneralStringSet[]   = "\000\001\002\003\004\005\006\007"
										  "\010\011\012\013\014\015\016\017"
                                          "\020\021\022\023\024\025\026\027"
										  "\030\031\032\033\034\035\036\037"
										  " !\"#$%&'()*+,-./0123456789:;<=>?"
										  "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
										  "`abcdefghijklmnopqrstuvwxyz{|}~\177"
										  "\200\201\202\203\204\205\206\207"
										  "\210\211\212\213\214\215\216\217"
										  "\220\221\222\223\224\225\226\227"
										  "\230\231\232\233\234\235\236\237"
										  "\240\241\242\243\244\245\246\247"
										  "\250\251\252\253\254\255\256\257"
										  "\260\261\262\263\264\265\266\267"
										  "\270\271\272\273\274\275\276\277"
										  "\300\301\302\303\304\305\306\307"
										  "\310\311\312\313\314\315\316\317"
										  "\320\321\322\323\324\325\326\327"
										  "\330\331\332\333\334\335\336\337"
										  "\340\341\342\343\344\345\346\347"
										  "\350\351\352\353\354\355\356\357"
										  "\360\361\362\363\364\365\366\367"
										  "\370\371\372\373\374\375\376\377";

const NumericString::InfoType NumericString::theInfo = {
    AbstractString::create,
    UniversalTagClass << 16 | UniversalNumericString,
    Unconstrained,
    0, 
    UINT_MAX,
    NumericStringSet,
    11,
    4,
    4,
    4
};

const PrintableString::InfoType PrintableString::theInfo = {
    AbstractString::create,
    UniversalTagClass << 16 | UniversalPrintableString,
    Unconstrained,
    0, 
    UINT_MAX,
    PrintableStringSet,
    74,
    7,
    7,
    8
};


const VisibleString::InfoType VisibleString::theInfo = {
    AbstractString::create,
    UniversalTagClass << 16 | UniversalVisibleString,
    Unconstrained,
    0, 
    UINT_MAX,
    VisibleStringSet,
    95,
    7,
    7,
    8
};

const IA5String::InfoType IA5String::theInfo = {
    AbstractString::create,
    UniversalTagClass << 16 | UniversalIA5String,
    Unconstrained,
    0, 
    UINT_MAX,
    IA5StringSet,
    128,
    7,
    7,
    8
};


const GeneralString::InfoType GeneralString::theInfo = {
    AbstractString::create,
    UniversalTagClass << 16 | UniversalGeneralString,
    Unconstrained,
    0, 
    UINT_MAX,
    GeneralStringSet,
    256,
    8,
    8,
    8
};



///////////////////////////////////////////////////////////////////////

const BMPString::InfoType BMPString::theInfo = {
    BMPString::create,
    UniversalTagClass << 16 | UniversalBMPString,
    Unconstrained,
    0, 
    UINT_MAX,
    0,
    0xffff,
    16,
    16
};


AbstractData* BMPString::create(const void* info)
{
    return new BMPString(info);
}



bool BMPString::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool BMPString::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}

AbstractData * BMPString::do_clone() const
{
  return new BMPString(*this);
}

int BMPString::do_compare(const AbstractData& other) const 
{
	const BMPString& that = *boost::polymorphic_downcast<const BMPString*>(&other);
	return base_string::compare(that);
}

///////////////////////////////////////////////////////////////////////
const GeneralizedTime::InfoType GeneralizedTime::theInfo = {
    GeneralizedTime::create,
    UniversalTagClass << 16 | UniversalGeneralisedTime
};

AbstractData* GeneralizedTime::create(const void* info)
{
	return new GeneralizedTime(info);
}


GeneralizedTime::GeneralizedTime(const void* info)
: AbstractData(info), year(1), month(1), day(1), hour(0), minute(0), second(0), millisec(0), mindiff(0)
, utc(false)
{}

GeneralizedTime::GeneralizedTime()
: AbstractData(&theInfo), year(1), month(1), day(1), hour(0), minute(0), second(0), millisec(0), mindiff(0)
, utc(false)
{}

GeneralizedTime::GeneralizedTime(const char* value) 
: AbstractData(&theInfo)
{ 
    set(value); 
}


GeneralizedTime::GeneralizedTime(int yr, int mon, int dy, 
		int hr , int mn, int sc,
		int ms , int md, bool u )
: AbstractData(&theInfo), year(yr), month(mon), day(dy), hour(hr), minute(mn), second(sc), millisec(ms), mindiff(md)
, utc(u)
{
}

GeneralizedTime& GeneralizedTime::operator = (const GeneralizedTime& other )
{
	year = other.year; month = other.month; day = other.day; 
	hour = other.hour; minute = other.minute; second = other.second;
	millisec = other.millisec; mindiff = other.mindiff; utc = other.utc;
	return *this;
}


GeneralizedTime::GeneralizedTime(const GeneralizedTime& other)
: AbstractData(other), year(other.year), month(other.month), day(other.day)
  , hour(other.hour), minute(other.minute), second(other.second), millisec(other.millisec)
  , mindiff(other.mindiff), utc(other.utc)
{}

void GeneralizedTime::set(const char* valueNotion) 
{
	sscanf(valueNotion, "%4d%2d%2d%2d%2d%2d",&year, &month, &day, &hour, &minute, &second);
	if (valueNotion[14] == '.')
	{
		float f;
		sscanf(&valueNotion[14], "%f",&f);
		millisec = (int) f*1000;
	}
	else
	{
		millisec = 0;
	}

	int len = strlen(valueNotion);
	int pos = len - 5;
	if (valueNotion[len-1] == 'Z')
		utc = true;
	else
	{
		utc = false;
		if (valueNotion[pos] == '+' ||
		    valueNotion[pos] == '-' )
			sscanf(&valueNotion[pos], "%d", &mindiff);
		else
			mindiff = 0;
        }
}

std::string GeneralizedTime::get() const 
{ 
	char buf[30];
	int len = 14;

	sprintf(buf, "%04d%02d%02d%02d%02d%02d", year, month, day, hour, minute, second);
	if (millisec)
	{
		sprintf(&buf[14], ".%03d", millisec);
		len = 18;
	}

	if (utc)
		sprintf(&buf[len], "Z");
	else if (mindiff)
		sprintf(&buf[len], "%+03d%02d", mindiff/60, mindiff%60); 
	return std::string(buf); 
}

bool GeneralizedTime::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool GeneralizedTime::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}

void GeneralizedTime::swap(GeneralizedTime& other)
{
	std::swap(year, other.year);
	std::swap(month, other.month);
	std::swap(day, other.day);
	std::swap(hour, other.hour);
	std::swap(minute, other.minute);
	std::swap(second, other.second);
	std::swap(millisec, other.millisec);
	std::swap(mindiff, other.mindiff);
	std::swap(utc, other.utc);
}

int GeneralizedTime::do_compare(const AbstractData& other) const 
{
	const GeneralizedTime& that = *boost::polymorphic_downcast<const GeneralizedTime*>(&other);
	const int* src = &year, *dst = &that.year;
	for (; src != &mindiff; ++src, ++dst)
		if (*src != *dst)
			return *src - *dst;
	return utc - that.utc;
}

time_t GeneralizedTime::get_time_t()
{
	struct tm t;
	t.tm_year = year-1900;
	t.tm_mon = month-1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;
	return mktime(&t);
}

void GeneralizedTime::set_time_t(time_t gmt)
{
	struct tm* t = gmtime(&gmt);
	year = t->tm_year+1900;
	month = t->tm_mon+1;
	day = t->tm_mday;
	hour = t->tm_hour;
	minute = t->tm_min;
	second = t->tm_sec;
}

AbstractData* GeneralizedTime::do_clone() const 
{
    return new GeneralizedTime(*this);
}

///////////////////////////////////////////////////////////////////////

AbstractData* CHOICE::create(const void* info)
{
	return new CHOICE(info);
}

CHOICE::CHOICE(const void* information, int id, AbstractData* value)
: AbstractData(information), choice(value) , choiceID(id)
{

}

CHOICE::CHOICE(const CHOICE & other)
  : AbstractData(other)
  , choice( other.choice.get() == NULL ? NULL : other.choice->clone())
  , choiceID( other.choiceID)
{}

CHOICE::~CHOICE()
{
}


CHOICE & CHOICE::operator=(const CHOICE & other)
{
  assert(info_ == other.info_);
  choice.reset(other.choice.get() == NULL ? NULL : other.choice->clone());
  choiceID = other.choiceID;
  return *this;
}

AbstractData* CHOICE::setSelection(int id, AbstractData* obj)
{
    choice.reset(obj);
    choiceID= id;   
    return obj;
}

bool CHOICE::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool CHOICE::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}


void CHOICE::swap(CHOICE& other)
{
  assert(info_ == other.info_);
    AbstractData* tmp = other.choice.release();
	other.choice.reset(choice.release());
	choice.reset(tmp);
	std::swap(choiceID, other.choiceID);
}

int CHOICE::do_compare(const AbstractData& other) const
{
	const CHOICE& that = *boost::polymorphic_downcast<const CHOICE*>(&other);
	if (choiceID >= 0 && choiceID == that.choiceID)
		return choice->compare(*that.choice);
	return choiceID - that.choiceID;
}

AbstractData* CHOICE::do_clone() const
{
    return new CHOICE(*this);
}
    
bool CHOICE::createSelection()
{
    if (choiceID >= 0 && static_cast<unsigned>(choiceID) < info()->totalChoices )
    {
        const AbstractData::InfoType* selectionInfo = 
			static_cast<const AbstractData::InfoType*>(info()->selectionInfos[choiceID]);
        
        if (selectionInfo)
        {
            choice.reset(selectionInfo->create(selectionInfo));
            return true;
        }
    }
    choiceID = unknownSelection_;
    choice.reset();
    return false;
}


///////////////////////////////////////////////////////////////////////
SEQUENCE::FieldVector::~FieldVector()
{
	iterator it = begin(), last = end();
	for (; it != last; ++it)
		delete *it;
	clear();
}

SEQUENCE::FieldVector::FieldVector(const SEQUENCE::FieldVector& other)
{
	const_iterator it = other.begin(), last = other.end();
	reserve(other.size());
	for (; it != last; ++it)
	{
		push_back( *it ? (*it)->clone() : NULL);
	}
}

void SEQUENCE::BitMap::resize(unsigned nBits)
{
    bitData.resize((nBits+7)/8);
    totalBits = nBits;
}

bool SEQUENCE::BitMap::operator [] (unsigned bit) const 	
{	
    if (bit < totalBits)
        return (bitData[bit>>3] & (1 << (7 - (bit&7)))) != 0;
    return false;
}

void SEQUENCE::BitMap::set(unsigned bit)
{
    if (bit < totalBits)
        bitData[(unsigned)(bit>>3)] |= 1 << (7 - (bit&7));
}

void SEQUENCE::BitMap::clear(unsigned bit)
{
    if (bit < totalBits)
        bitData[(unsigned)(bit>>3)] &= ~(1 << (7 - (bit&7)));
}      

inline void SEQUENCE::BitMap::swap(BitMap& other)
{
    bitData.swap(other.bitData);
    std::swap(totalBits, other.totalBits);
}


const unsigned SEQUENCE::defaultTag =0;

AbstractData* SEQUENCE::create(const void* info)
{
	return new SEQUENCE(info);
}

SEQUENCE::SEQUENCE(const void* information)
: AbstractData(information)
{
	unsigned nBaseFields = info()->numFields;
	int nExtensions = info()->knownExtensions;
    unsigned i;
    fields.resize(nBaseFields+nExtensions);
    optionMap.resize(info()->numOptional);
    for (i = 0; i < nBaseFields; ++i)
		if (info()->ids[i]  == -1 )
			fields[i] = AbstractData::create(info()->fieldInfos[i]);

	if (info()->nonOptionalExtensions)
	{
		extensionMap.bitData.assign(info()->nonOptionalExtensions,
			info()->nonOptionalExtensions + (nExtensions+7/8));
		extensionMap.totalBits = nExtensions;
	}

	for (i = 0; i < extensionMap.size(); ++i)
		if (extensionMap[i]) 
			fields[i+nBaseFields] =	AbstractData::create(info()->fieldInfos[i+nBaseFields]);

}


SEQUENCE::SEQUENCE(const SEQUENCE & other)
: AbstractData(other),
    fields(other.fields),
	optionMap(other.optionMap),
	extensionMap(other.extensionMap)
{	
}



SEQUENCE::~SEQUENCE()
{
}



SEQUENCE & SEQUENCE::operator=(const SEQUENCE & other)
{
  assert(info_ == other.info_ );

  FieldVector temp_fields(other.fields);
  BitMap temp_optionalMap(other.optionMap);
  BitMap temp_extensionMap(other.extensionMap);

  fields.swap(temp_fields);
  optionMap.swap(temp_optionalMap);
  extensionMap.swap(temp_extensionMap);

  return *this;
}


bool SEQUENCE::hasOptionalField(unsigned opt) const
{
  if (opt < (unsigned)optionMap.size())
    return optionMap[opt];
  else
    return extensionMap[opt - optionMap.size()];
}


void SEQUENCE::includeOptionalField(unsigned opt, unsigned pos)
{
  if (opt < (unsigned)optionMap.size())
    optionMap.set(opt);
  else {
    assert(extendable());
    opt -= optionMap.size();
    if (opt >= (unsigned)extensionMap.size())
      extensionMap.resize(opt+1);
    extensionMap.set(opt);
  }
  if (fields[pos] == NULL)
	fields[pos] = AbstractData::create(info()->fieldInfos[pos]);
}


void SEQUENCE::removeOptionalField(unsigned opt)
{
  if (opt < (unsigned)optionMap.size())
    optionMap.clear(opt);
  else {
    assert(extendable());
    opt -= optionMap.size();
    extensionMap.clear(opt);
  }
}

bool SEQUENCE::do_accept(Visitor& visitor)
{
    return visitor.visit(*this);
}

bool SEQUENCE::do_accept(ConstVisitor& visitor) const
{
    return visitor.visit(*this);
}


AbstractData * SEQUENCE::do_clone() const
{
  return new SEQUENCE(*this);
}

void SEQUENCE::swap(SEQUENCE& other)
{
  fields.swap(other.fields);
  optionMap.swap(other.optionMap);
  extensionMap.swap(other.extensionMap);
}

int SEQUENCE::do_compare(const AbstractData& other) const
{
	const SEQUENCE& that = *boost::polymorphic_downcast<const SEQUENCE*>(&other);
	assert(info_ == that.info_);

    int lastOptionalId = -1, result = 0;
    unsigned i;
    for (i = 0; i < info()->numFields ; ++i)
	{
		int id = info()->ids[i];
		if (id == mandatory_  || (hasOptionalField(id) && that.hasOptionalField(id)) )
			result = fields[i]->compare(*that.fields[i]);
		else 
			result = hasOptionalField(id) - that.hasOptionalField(id);

		if (result != 0) return result;
		lastOptionalId = ( id != mandatory_ ? id : lastOptionalId);
	}
	
	for (; i < fields.size(); ++i)
	{
		if (hasOptionalField(++lastOptionalId) && that.hasOptionalField(lastOptionalId))
			result = fields[i]->compare(*that.fields[i]);
		else
			result = hasOptionalField(lastOptionalId) - that.hasOptionalField(lastOptionalId);
	}
	return result;

}

///////////////////////////////////////////////////////

SEQUENCE_OF_Base::SEQUENCE_OF_Base(const SEQUENCE_OF_Base & other)
  : ConstrainedObject(other)
{
	Container::const_iterator first = other.container.begin(), last = other.container.end();
    container.reserve(other.container.size());
	for (; first != last; ++ first)
		container.push_back( (*first)->clone());
}


SEQUENCE_OF_Base & SEQUENCE_OF_Base::operator=(const SEQUENCE_OF_Base & other)
{
    Container temp;
    Container::const_iterator first = other.container.begin(), last = other.container.end();
    temp.reserve(other.container.size());
    for (; first != last; ++ first)
        temp.push_back( (*first)->clone());
    temp.swap(container);
    return *this;
}

AbstractData* SEQUENCE_OF_Base::do_clone() const
{
    return new SEQUENCE_OF_Base(*this);
}

bool SEQUENCE_OF_Base::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool SEQUENCE_OF_Base::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}


void SEQUENCE_OF_Base::clear()
{
  Container::const_iterator first = container.begin(), last = container.end();
  for (;first != last; ++first)
	  delete *first;
  container.clear();
}

int SEQUENCE_OF_Base::do_compare(const AbstractData& other) const
{
  const SEQUENCE_OF_Base& that = *boost::polymorphic_downcast<const SEQUENCE_OF_Base*>(&other);
  Container::const_iterator first1 = container.begin(), last1 = container.end();
  Container::const_iterator first2 = that.container.begin(), last2 = that.container.end();
  for (; first1 != last1 && first2 != last2; ++first1, ++first2)
  {
	  int r = (*first1)->compare(*(*first2));
	  if (r != 0)
		  return r;
  }
  return container.size() - that.container.size();	  
}

void SEQUENCE_OF_Base::insert(iterator position, SEQUENCE_OF_Base::Container::size_type n, const AbstractData& x)	
{
        SEQUENCE_OF_Base::Container::difference_type dist = std::distance(container.begin(), position);
        reserve(size()+ n);
        std::generate_n(std::inserter(container, container.begin() + dist), n, create_from0(x));
}

void SEQUENCE_OF_Base::insert(iterator position, const_iterator first, const_iterator last)
{
        SEQUENCE_OF_Base::Container::difference_type dist = std::distance(container.begin(), position);
        reserve(size()+ std::distance(first, last));
		std::transform(first, last, std::inserter(container, container.begin() + dist), create_from_ptr());
}

AbstractData* SEQUENCE_OF_Base::create(const void* info)
{
    return new SEQUENCE_OF_Base(info);
}

void SEQUENCE_OF_Base::resize(Container::size_type sz)
{
    if (sz < size())
    {
        iterator i = begin()+sz, last = end();
        for (; i != last; ++i)
            delete *i;
        container.resize(sz); 
    } 
    else  
    {
        container.reserve(sz);
        for (unsigned i = size(); i < sz; ++i)
            container.push_back(createElement());
    }
}

void SEQUENCE_OF_Base::erase(iterator first, iterator last)
{
    iterator f = first;
    for (;f != last; ++f)
        delete *f;
    container.erase(first, last);
}

//////////////////////////////////////////////////////

const OpenData::InfoType OpenData::theInfo = {
    OpenData::create,
    0
};



OpenData::OpenData(const OpenData& that)
: AbstractData(that)
, data(that.has_data() ? that.get_data().clone() : NULL)
, buf(that.has_buf() ? new OpenBuf(that.get_buf()) : NULL )
{}

AbstractData* OpenData::create(const void* info)
{
    return new OpenData(info);
}


void OpenData::swap(OpenData& other)	
{
	AbstractData* tmpData = data.release();
	data.reset(other.data.release());
	other.data.reset(tmpData);

	OpenBuf* tmpBuf = buf.release();
	buf.reset(other.buf.release());
	other.buf.reset(tmpBuf);
}

bool OpenData::do_accept(Visitor& visitor)
{
	return visitor.visit(*this);
}

bool OpenData::do_accept(ConstVisitor& visitor) const
{
	return visitor.visit(*this);
}


int OpenData::do_compare(const AbstractData& other) const
{
	const OpenData& that = *boost::polymorphic_downcast<const OpenData*>(&other);
	if (has_data() && that.has_data())
		return get_data().compare(that.get_data());
	if (has_buf() && that.has_buf())
		return lexicographic_compare_bytes(get_buf().begin(), get_buf().end(),
			that.get_buf().begin(), that.get_buf().end());
	if (isEmpty() && that.isEmpty())
		return 0;
	return has_data() ? 1 : -1;
}

AbstractData* OpenData::do_clone() const 
{
    return new OpenData(*this);
}

/////////////////////////////////////////////////////////

bool TypeConstrainedOpenData::do_accept(Visitor& v)
{
    return v.visit(*this);
}

AbstractData* TypeConstrainedOpenData::create(const void* info)
{
    return new TypeConstrainedOpenData( 
        AbstractData::create(static_cast<const InfoType*>(info)->typeInfo), 
        info);
}


/////////////////////////////////////////////////////////

bool Visitor::visit(SEQUENCE& value) 
{ 
  VISIT_SEQ_RESULT result = preVisitExtensionRoots(value);
  if (result <= STOP)
    return result != FAIL;

  bool visitExtension = (result == CONTINUE);

  int lastOptionalId = -1;
  unsigned i;
  for (i = 0 ; i < value.info()->numFields; ++i)
  {
	  int optionalId = value.info()->ids[i];
      result = visitExtensionRoot(value, i, optionalId);
      if ( result <= STOP)
		return result != FAIL;
	  lastOptionalId = ( optionalId != -1 ? optionalId : lastOptionalId);
  }

  if (!visitExtension)
  {
      value.extensionMap.resize(0);
      return true;
  }

  result = preVisitExtensions(value);

  if ( result !=  CONTINUE )
	return result != FAIL;

  for (; i < value.fields.size(); ++i)
  {
      result = visitKnownExtension(value, i, ++lastOptionalId);
      if ( result !=  CONTINUE )
          return result != FAIL;
  }

  return visitUnknownExtensions(value);
}

bool ConstVisitor::visit(const SEQUENCE& value)
{
  if (!preVisitExtensionRoots(value))
	  return false;

  unsigned i;
  int lastOptionalId = -1;	
  for (i = 0; i < value.info()->numFields ; ++i)
  {
	  int optionalId = value.info()->ids[i];
	  if (optionalId == -1 || value.hasOptionalField(optionalId))
	  {
		  if (value.fields[i] == NULL || !visitExtensionRoot(value, i))
			  return false;
	  }
	  lastOptionalId = ( optionalId != -1 ? optionalId : lastOptionalId);
  }

  if (value.extensionMap.size())
  {
	  assert(value.extendable());
	  
	  if (!preVisitExtensions(value))
		  return false;
	  
	  for (; i < value.fields.size(); ++i)
		  if (value.hasOptionalField(++lastOptionalId)) 
		  {
			  if (value.fields[i] == NULL || !visitKnownExtension(value, i))
				  return false;
		  }
  }
  return afterVisitSequence(value);
}

} // namespace ASN1


