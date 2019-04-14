/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is MPEG4IP.
 * 
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2001-2002.  All Rights Reserved.
 * 
 * Contributor(s): 
 *		Dave Mackie		dmackie@cisco.com
 */

//#include "mp4av_common.h"
#include "StdAfx.h"
#include "Bitstream.h"

#include <malloc.h>
#include <ERRNO.H>

void Bitstream::AllocBytes(UINT32 numBytes) 
{
	m_pBuf = (UINT8*)calloc(numBytes, 1);
	if (!m_pBuf) {
		throw ENOMEM;
	}
	m_bitPos = 0;
	m_numBits = numBytes << 3;
}

void Bitstream::SetBytes(UINT8* pBytes, UINT32 numBytes) 
{
	m_pBuf = pBytes;
	m_bitPos = 0;
	m_numBits = numBytes << 3;
}

void Bitstream::PutBytes(UINT8* pBytes, UINT32 numBytes)
{
	UINT32 numBits = numBytes << 3;

	if (numBits + m_bitPos > m_numBits) {
		throw EIO;
	}

	if ((m_bitPos & 7) == 0) {
		memcpy(&m_pBuf[m_bitPos >> 3], pBytes, numBytes);
		m_bitPos += numBits;
	} else {
		for (UINT32 i = 0; i < numBytes; i++) {
			PutBits(pBytes[i], 8);
		}
	}
}

void Bitstream::PutBits(UINT32 bits, UINT32 numBits)
{
	if (numBits + m_bitPos > m_numBits) {
		throw EIO;
	}
	if (numBits > 32) {
		throw EIO;
	}

	for (INT8 i = numBits - 1; i >= 0; i--) {
		m_pBuf[m_bitPos >> 3] |= ((bits >> i) & 1) << (7 - (m_bitPos & 7));
		m_bitPos++;
	}
}

UINT32 Bitstream::GetBits(UINT32 numBits)
{
	if (numBits + m_bitPos > m_numBits) {
		throw EIO;
	}
	if (numBits > 32) {
		throw EIO;
	}

	UINT32 bits = 0;

	for (UINT8 i = 0; i < numBits; i++) {
		bits <<= 1;
		bits |= (m_pBuf[m_bitPos >> 3] >> (7 - (m_bitPos & 7))) & 1;
		m_bitPos++;
	}

	return bits;
}

