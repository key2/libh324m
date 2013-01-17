/*
 * BERDecoder.cxx
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
 * $Log: BERDecoder.cxx,v $
 * Revision 1.17  2005/11/21 10:34:40  btrummer
 * Fixed the decoding of a CHOICE, containing an element using the indefinite
 * length encoding.
 *
 * Revision 1.16  2005/09/07 15:53:34  btrummer
 * Added indefinite length support for SEQUENCE and SEQUENCE_OF.
 *
 * Revision 1.15  2005/08/31 14:00:30  btrummer
 * Applied some changes to the parser and the asn1 library to make BER en-
 * and decoding work correctly (or let's say, more correct than before. ;-)
 * (Thanks to Harald Okorn)
 *
 * Revision 1.14  2002/12/08 07:25:36  mangelo
 * Fixed the problem for decoding types with CHOICE.SEQUENCE.CHOICE structure
 *
 * Revision 1.13  2002/11/06 08:02:48  btrummer
 * Minor optimization in do_visit(BIT_STRING&).
 *
 * Revision 1.12  2002/11/04 15:30:10  btrummer
 * dded length check in do_visit(GeneralizedTime&).
 * Further, a '\0' must be added, before GeneralizedTime::set() is called,
 * because the strlen() function is used inside.
 *
 * Revision 1.11  2002/11/04 12:33:26  btrummer
 * Some minor optimizations.
 *
 * Revision 1.10  2002/11/04 09:39:45  btrummer
 * The decoded length in do_visit(BOOLEAN&) must be 1 (see 16.3.2).
 *
 * Revision 1.9  2002/11/04 09:19:34  btrummer
 * Removed the assert() statement from decodeByte().
 * Added a missing atEnd() check for decodeHeader(unsigned&, bool&, unsigned&).
 *
 * Revision 1.8  2002/11/04 09:04:32  btrummer
 * Catch a decoded len_len of 0x80 (16.2.3.3)
 * in decodeHeader(unsigned&, bool&, unsigned&),
 * since this length-encoding is not implemented (yet).
 *
 * Revision 1.7  2002/11/04 08:44:06  btrummer
 * The decoded length in do_visit(Null&) must be 0 (see 16.3.1).
 *
 * Revision 1.6  2002/11/04 08:07:59  btrummer
 * Fixed the length check in do_visit(SEQUENCE_OF_Base&).
 *
 * Revision 1.5  2002/11/04 07:42:15  btrummer
 * Fixed the atEnd() check in BERDecoder::do_visit(OBJECT_IDENTIFIER&).
 * Fixed all calls to decodeBlock(), which now does a length-check
 * before calling resize() with the decoded length. The decoding itself
 * was moved to do_decodeBlock().
 *
 * Revision 1.4  2002/07/02 04:45:13  mangelo
 * Modify for VC.Net and GCC 3.1
 *
 * Revision 1.3  2001/10/05 19:09:27  mangelo
 * Added Log
 *
 * 2001/07/16 Huang-Ming Huang
 * Optional components of SEQUENCE is now created on demand.
 *
 * 2001/06/26 Huang-Ming Huang 
 * Version 2.1 Reimplemented to minimize the code size.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <boost/cast.hpp>
#include "asn1.h"

namespace ASN1 {

extern unsigned CountBits(unsigned range);

bool CHOICE::setID(unsigned tagVal, unsigned tagClass)
{
	bool result = true;
    unsigned tag = tagClass << 16 | tagVal;

	if (info()->tags == NULL)
		choiceID = tagVal;
	else
	{
		unsigned* tagInfo = std::lower_bound(info()->tags, info()->tags+info()->numChoices, tag);
		choiceID = tagInfo - info()->tags;
		result = (choiceID != info()->numChoices) && (*tagInfo == tag); 
	}

	if (result)
	{
		if (!createSelection())
            return false;
	}
	else if ( info()->tags[0] == 0)
    {
		choiceID = 0;
		createSelection();
		CHOICE* obj = boost::polymorphic_downcast<CHOICE*>(choice.get());
		if (obj->setID(tagVal, tagClass))
			result = true;
		
        if (!result)
			choiceID = unknownSelection_;
    }


	return result;
}

inline bool BERDecoder::atEnd() 
{ 
    return beginPosition >= endPosition; 
}

inline unsigned BERDecoder::getBytesLeft() const
{ 
    return endPosition - beginPosition; 
}

inline unsigned char BERDecoder::decodeByte() 
{ 
	return *beginPosition++; 
}


bool BERDecoder::do_visit(Null& value)
{
  unsigned len;
  bool indefinite;
  if (!decodeHeader(value, len, indefinite) || indefinite || len != 0)
    return false;

  return true;
}

bool BERDecoder::do_visit(BOOLEAN& value)
{
  unsigned len;
  bool indefinite;
  if (!decodeHeader(value, len, indefinite) || indefinite || len != 1 || atEnd())
    return false;

  value = (decodeByte() != 0);

  return true;
}

bool BERDecoder::do_visit(INTEGER& value)
{
  unsigned len;
  bool indefinite;
  if (!decodeHeader(value, len, indefinite) || indefinite || len == 0 || getBytesLeft() < len)
    return false;

  int accumulator = static_cast<signed char>(decodeByte()); // sign extended first byte
  while (--len > 0) {
    accumulator = (accumulator << 8) | decodeByte();
  }

  value = accumulator;
  return true;
}

bool BERDecoder::do_visit(ENUMERATED& value)
{
  unsigned len;
  bool indefinite;
  if (!decodeHeader(value, len, indefinite) || indefinite || len == 0 || getBytesLeft() < len)
    return false;

  unsigned val = 0;
  while (len-- > 0) {
    val = (val << 8) |  decodeByte();
  }

  value.setFromInt(val);
  return true;
}


bool BERDecoder::do_visit(OBJECT_IDENTIFIER& value)
{
  unsigned len;
  bool indefinite;
  if (!decodeHeader(value, len, indefinite) || indefinite || getBytesLeft() < len)
    return false;

  beginPosition += len;
  return value.decodeCommon(beginPosition-len, len);
}

bool BERDecoder::do_visit(BIT_STRING& value)
{
  unsigned len;
  bool indefinite;
  if (!decodeHeader(value, len, indefinite) ||
      indefinite ||  // We don't support 8.1.3.6 for BIT_STRINGs yet.
      len == 0 || atEnd())
    return false;
  value.totalBits = --len*8 - decodeByte();
  return decodeBlock(value.bitData, len) == len;
}

bool BERDecoder::do_visit(OCTET_STRING& value)
{
  unsigned len;
  bool indefinite;
  if (!decodeHeader(value, len, indefinite) ||
      indefinite)  // We don't support 8.1.3.6 for OCTET_STRINGs yet.
    return false;
  return decodeBlock(value, len) == len;
}

bool BERDecoder::do_visit(AbstractString& value)
{
  unsigned len;
  bool indefinite;
  if (!decodeHeader(value, len, indefinite) ||
      indefinite)  // We don't support 8.1.3.6 for AbstractStrings yet.
    return false;
  return decodeBlock(value, len) == len;
}

bool BERDecoder::do_visit(BMPString& value)
{
  unsigned len;
  bool indefinite;
  if (!decodeHeader(value, len, indefinite) ||
      indefinite ||  // We don't support 8.1.3.6 for BMPStrings yet.
      getBytesLeft() < len)
    return false;

  len /= 2;
  value.resize(len);
  for (unsigned i = 0; i < len; ++i)
      value[i] = (decodeByte() << 8) | decodeByte();
  return true;
}

bool BERDecoder::decodeChoicePreamle(CHOICE& value, memento_type& nextPosition)
{
  const char* savedPosition = beginPosition;

  unsigned tag;
  bool primitive;
  unsigned entryLen;
  bool indefinite;
  if (!decodeHeader(tag, primitive, entryLen, indefinite))
    return false;

  // With the following lines of code BER decoding doesn't work,
  // because the call of this method overwrites the correct values
  // of tag, primitive and entryLen.
  //if (dontCheckTag || value.getTag() != 0)
  //{
  //  savedPosition = beginPosition;
  //  if (!decodeHeader(tag, primitive, entryLen, indefinite))
  //    return false;
  //}
  nextPosition = indefinite ? NULL : beginPosition + entryLen;
  beginPosition = savedPosition;
  if (value.setID(tag & 0xffff, tag >> 16))
  {
    if (value.getSelectionTag() != 0) 
      dontCheckTag = 1;

    return true;
  }
  return false;
}


bool BERDecoder::do_visit(CHOICE& value)
{
    memento_type memento;
    if (decodeChoicePreamle(value,memento))
    {
        if (!value.isUnknownSelection() && 
            !value.getSelection()->accept(*this))
            return false;
        if (memento)
            rollback(memento);
        return true;
    }
    return false;
}

bool BERDecoder::do_visit(SEQUENCE_OF_Base& value)
{
  value.clear();

  unsigned len;
  bool indefinite;
  if (!decodeHeader(value, len, indefinite) || getBytesLeft() < len)
    return false;

  const char* endPos = beginPosition + len; //only valid if indefinite == true

  SEQUENCE_OF_Base::iterator it = value.begin(), last = value.end();
  while (((indefinite && !decodeEndOfContents()) ||
          beginPosition < endPos) && it != last)
  {
    if (!(*it)->accept(*this))
    {
      value.erase(it, last);
      return false;
    }
    ++it;
  }

  if (it != last)
    value.erase(it, last);

  while ((indefinite && !decodeEndOfContents()) || beginPosition < endPos)
  {
    std::auto_ptr<AbstractData> obj(value.createElement());
    if (!obj->accept(*this))
      return false;
    value.push_back(obj.release());
  }

  //if (!indefinite)
  //{
  //  beginPosition = endPos;
  //}

  return true;
}

bool BERDecoder::do_visit(OpenData& value)
{
  const char* savedPosition = beginPosition;

  unsigned tag;
  bool primitive;
  unsigned entryLen;
  bool indefinite;
  if (!decodeHeader(tag, primitive, entryLen, indefinite) ||
      indefinite)  // We don't support 8.1.3.6 for OpenDatas yet.
    return false;

  if (value.getTag() == 0)
	beginPosition = savedPosition;

  if (!value.has_buf())
	  value.grab(new OpenBuf);
  decodeBlock(value.get_buf(), entryLen);
  return true;
}

bool BERDecoder::do_revisit(OpenData& value)
{
    if (!value.has_buf() || !value.has_data())
        return false;
    BERDecoder decoder(&*value.get_buf().begin(), &*value.get_buf().end(), get_env());
    return value.get_data().accept(decoder);
}

bool BERDecoder::do_visit(TypeConstrainedOpenData& value)
{
	assert(value.has_data());
	const char* savedPosition = beginPosition;
	
	unsigned tag;
	bool primitive;
	unsigned entryLen;
	bool indefinite;
	if (!decodeHeader(tag, primitive, entryLen, indefinite) ||
	    indefinite)  // We don't support 8.1.3.6 for TCOpenDatas yet.
		return false;
	
	if (value.getTag() == 0)
		beginPosition = savedPosition;
	return value.get_data().accept(*this);
}


bool BERDecoder::do_visit(GeneralizedTime& value)
{
  unsigned len;
  bool indefinite;
  if (!decodeHeader(value, len, indefinite) ||
      indefinite ||  // We don't support 8.1.3.6 for GeneralizedTimes yet.
      getBytesLeft() < len)
    return false;

  std::vector<char> block(len+1);
  if (decodeBlock(block, len) == len)
  {
	  block.push_back('\0');
	  value.set(&*block.begin());
	  return true;
  }
  return false;
}

Visitor::VISIT_SEQ_RESULT BERDecoder::preVisitExtensionRoots(SEQUENCE& value)
{
  unsigned len;
  bool indefinite;
  if (!decodeHeader(value, len, indefinite))
    return FAIL;

  endSEQUENCEPositions.push_back(indefinite ? NULL : beginPosition + len);
  return !atEnd() ? CONTINUE : FAIL;
}

Visitor::VISIT_SEQ_RESULT BERDecoder::visitExtensionRoot(SEQUENCE& value, int index, int optional_id)
{
    if (atEnd())
        return optional_id == -1 ? FAIL : CONTINUE;

    const char* savedPosition = beginPosition;
    const char* nextEndPosition = endSEQUENCEPositions.back();
    
    if ((nextEndPosition == savedPosition && optional_id == -1) || 
        (nextEndPosition != NULL && nextEndPosition < savedPosition))
        return FAIL;
    
    unsigned tag;
    bool primitive;
    unsigned entryLen;
    bool indefinite;
    if (!decodeHeader(tag, primitive, entryLen, indefinite))
        return FAIL;
    beginPosition = savedPosition;
    unsigned fieldTag = value.getFieldTag(index);

    if ((fieldTag == tag) || (fieldTag == 0))
    {

        if (optional_id != -1)
            value.includeOptionalField(optional_id, index);
        
        AbstractData* field = value.getField(index);
        if (field)
        {
            if (value.tagMode() != SEQUENCE::IMPLICIT_TAG)
                dontCheckTag = 1;
            if (field->accept(*this))
                return CONTINUE;
            
            if (optional_id != -1)
            {
                value.removeOptionalField(optional_id);
                if (fieldTag == 0)
                    return CONTINUE;
            }
            return FAIL;
        }
        return optional_id != -1 ? CONTINUE : FAIL;
    }
    return CONTINUE;
}


Visitor::VISIT_SEQ_RESULT BERDecoder::visitKnownExtension(SEQUENCE& value, int index, int optional_id)
{
	return visitExtensionRoot(value, index, optional_id);
}

bool BERDecoder::visitUnknownExtensions(SEQUENCE& value)
{
  const char* nextEndPosition = endSEQUENCEPositions.back();
  endSEQUENCEPositions.pop_back();

  if (nextEndPosition == NULL)
  {
    // Unknown extensions may exist which makes the end-of-contents marker
    // lie some octets behind.
    while (getBytesLeft() >= 2)
    {
      bool rc = decodeEndOfContents();
      if (rc)  return true;

      (void)decodeByte();  // Skip one octet.
    }

    // No end-of-contents marker found.
    return false;  
  }

  beginPosition = nextEndPosition;
  return true;
}

bool BERDecoder::decodeEndOfContents()
{
  const char* savedPosition = beginPosition;
  if (getBytesLeft() >= 2 && decodeByte() == '\0' && decodeByte() == '\0')
  {
    return true;
  }

  beginPosition = savedPosition;
  return false;
}

bool BERDecoder::decodeHeader(unsigned& tag,
                      bool & primitive,
                      unsigned & len,
                      bool & indefinite)
{
  if (atEnd())
    return false;

  unsigned tagVal, tagClass;
  unsigned char ident = decodeByte();
  tagClass = ident & 0xC0;
  primitive = (ident & 0x20) == 0;
  tagVal = ident&31;
  if (tagVal == 31) {
    unsigned char b;
    tagVal = 0;
    do {
      if (atEnd())
        return false;

      b = decodeByte();
      tagVal = (tagVal << 7) | (b&0x7f);
    } while ((b&0x80) != 0);
  }

  tag = tagVal | (tagClass << 16);

  if (atEnd())
    return false;

  unsigned char len_len = decodeByte();
  if (len_len == 0x80) {
    len = 0;
    indefinite = true;
    return true;
  }

  indefinite = false;

  if ((len_len & 0x80) == 0) {
    len = len_len;
    return true;
  }

  len_len &= 0x7f;

  if (getBytesLeft() < len_len)
    return false;

  len = 0;
  while (len_len-- > 0) {
    len = (len << 8) | decodeByte();
  }

  return true;
}

bool BERDecoder::decodeHeader(AbstractData & obj, unsigned & len, bool & indefinite)
{
  const char* pos = beginPosition;

  unsigned tag;
  bool primitive;
  if (decodeHeader(tag, primitive, len, indefinite) &&
      (tag == obj.getTag() || dontCheckTag--))
    return true;

  beginPosition = pos;
  return false;
}

unsigned BERDecoder::decodeBlock(std::vector<char> &buf, unsigned nBytes)
{
  if (getBytesLeft() < nBytes)
    nBytes = getBytesLeft();

  buf.resize(nBytes);

  return do_decodeBlock(&*buf.begin(), nBytes);
}

unsigned BERDecoder::decodeBlock(std::string &buf, unsigned nBytes)
{
  if (getBytesLeft() < nBytes)
    nBytes = getBytesLeft();

  buf.resize(nBytes);

  return do_decodeBlock(&*buf.begin(), nBytes);
}

unsigned BERDecoder::do_decodeBlock(char * bufptr, unsigned nBytes)
{
  if (nBytes == 0)
    return 0;

  memcpy(bufptr, beginPosition, nBytes);

  beginPosition += nBytes;
  return nBytes;
}

}

