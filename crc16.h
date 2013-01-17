/*
 *                    3GPP H324M Library
 *
 * Copyright (c) 2006 Amin Ramtin
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
 */

#ifndef CRC16_H
#define CRC16_H


unsigned short crcsum(const unsigned char* message, unsigned int length,  unsigned short crc);

int crcverify(const unsigned char* message, unsigned int length);

void crcappend(unsigned char* message, unsigned int length);

#endif
