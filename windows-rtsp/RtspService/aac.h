#pragma once

static UINT aac_split_adts(Buffer* p_adts_buff, VBuffer* p_adts_buff_list)
{
	PBYTE& p_data = p_adts_buff->m_pData;
	UINT& i_data_size = p_adts_buff->m_nDataSize;

	UINT writeSize = 0;

	while( i_data_size > 7 && ( p_data[0] != 0xFF || (p_data[1] & 0xF6) != 0xF0) )
	{
		i_data_size--;
		p_data++;
	}

	/* Split adts units */
	while( i_data_size > 7 )
	{
		int i_offset;
		int i_size = i_data_size;
		int i_skip = i_data_size;

		/* search adts end */
		for( i_offset = 7; i_offset+2 < i_data_size ; i_offset++)
		{
			if( p_data[i_offset] == 0xFF && (p_data[i_offset+1] & 0xF6) == 0xF0)
			{
				/* we found another startcode */
				i_size = i_offset - ( p_data[i_offset-1] == 0 ? 1 : 0);
				i_skip = i_offset;
				break;
			} 
		}

		if (i_size)
		{
			Buffer* p_adts_buffer = p_adts_buff_list->GetEmptyBuffer();
			if (p_adts_buffer == NULL)
				p_adts_buffer = new Buffer();
			p_adts_buffer->FillData(p_data, i_size);
			p_adts_buff_list->AddFullBuffer(p_adts_buffer);
		}

		writeSize += i_size;

		i_data_size -= i_skip;
		p_data += i_skip;
	}
	return writeSize;
}

static unsigned aac_get_frame_size(unsigned char*& framePtr, unsigned dataSize) {
	// Look at the LATM data length byte(s), to determine the size
	// of the LATM payload.
	unsigned resultFrameSize = 0;
	unsigned i;
	for (i = 0; i < dataSize; ++i) {
		resultFrameSize += framePtr[i];
		if (framePtr[i] != 0xFF) break;
	}
	++i;
	if (fIncludeLATMDataLengthField) {
		resultFrameSize += i;
	} else {
		framePtr += i;
		dataSize -= i;
	}

	return (resultFrameSize <= dataSize) ? resultFrameSize : dataSize;
}