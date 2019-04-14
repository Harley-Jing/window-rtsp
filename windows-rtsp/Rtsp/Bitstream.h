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

#ifndef __MBS_INCLUDED__
#define __MBS_INCLUDED__ 

class Bitstream {
public:
	Bitstream() {
		m_pBuf = NULL;
		m_bitPos = 0;
		m_numBits = 0;
	}

	void AllocBytes(UINT32 numBytes);

	void SetBytes(UINT8* pBytes, UINT32 numBytes);

	void PutBytes(UINT8* pBytes, UINT32 numBytes);

	void PutBits(UINT32 bits, UINT32 numBits);

	UINT32 GetBits(UINT32 numBits);

	void SkipBytes(UINT32 numBytes) {
		SkipBits(numBytes << 3);
	}

	void SkipBits(UINT32 numBits) {
		SetBitPosition(GetBitPosition() + numBits);
	}

	UINT32 GetBitPosition() {
		return m_bitPos;
	}

	void SetBitPosition(UINT32 bitPos) {
		if (bitPos > m_numBits) {
			throw;
		}
		m_bitPos = bitPos;
	}

	UINT8* GetBuffer() {
		return m_pBuf;
	}

	UINT32 GetNumberOfBytes() {
		return (GetNumberOfBits() + 7) / 8;
	}

	UINT32 GetNumberOfBits() {
		return m_numBits;
	}

protected:
	UINT8*	m_pBuf;
	UINT32	m_bitPos;
	UINT32	m_numBits;
};

#endif /* __MBS_INCLUDED__ */ 

