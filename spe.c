/*
The MIT License (MIT)

Copyright (c) 2017 badlyby

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "spe.h"

#ifndef SPE_RAMLessMode
volatile static uint16_t SPE_Scount;
volatile static uint8_t SPE_Sbuf[(SPE_MAX_PacketLength+3)/7*8+1];
#endif

volatile static uint8_t SPE_Rbuf[SPE_MAX_PacketLength];
volatile static uint8_t SPE_TmpBits = 0;
volatile static uint8_t SPE_LBits = 0;
volatile static uint8_t SPE_Begin = 0;
static const uint8_t SPE_BitMasks[]={0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};


void SPE_Start_Send(void)//数据包开始
{
	SPE_Begin = 1;
#ifndef SPE_RAMLessMode
	SPE_Scount = 0;
#endif
}

void SPE_Byte_Send(uint8_t bits)//发送一个字节
{
	if(SPE_Begin)
	{
		SPE_Begin = 0;
	#ifdef SPE_RAMLessMode
		SendByte((bits >> 2) | 0x80);
	#else
		if(SPE_Scount >= sizeof(SPE_Sbuf)) return;
		SPE_Sbuf[SPE_Scount++] = (bits >> 2) | 0x80;
	#endif
		SPE_TmpBits = bits & 0x03;
		SPE_LBits = 2;
	}
	else
	{
	#ifdef SPE_RAMLessMode
		SendByte((SPE_TmpBits << (7 - SPE_LBits)) | (bits >> (SPE_LBits + 1)));
	#else
		if(SPE_Scount >= sizeof(SPE_Sbuf)) return;
		SPE_Sbuf[SPE_Scount++] = (SPE_TmpBits << (7 - SPE_LBits)) | (bits >> (SPE_LBits + 1));
	#endif
		SPE_LBits ++;
		SPE_TmpBits = bits & SPE_BitMasks[SPE_LBits];
		if(SPE_LBits == 7)
		{
		#ifdef SPE_RAMLessMode
			SendByte(SPE_TmpBits);
		#else
			if(SPE_Scount >= sizeof(SPE_Sbuf)) return;
			SPE_Sbuf[SPE_Scount++] = SPE_TmpBits;
		#endif
			SPE_LBits = 0;
			SPE_TmpBits = 0;
		}
	}
}

void SPE_End_Send(void)//数据包结束
{
	if(SPE_Begin)
	{
		SPE_Begin = 0;
	#ifdef SPE_RAMLessMode
		SendByte(0xC0);
	#else
		SPE_Sbuf[0] = 0xC0;
		SendBytes(SPE_Sbuf, 1);
	#endif
	}
	else
	{
	#ifdef SPE_RAMLessMode
		SendByte((SPE_TmpBits << (6 - SPE_LBits)) | 0xC0);
	#else
		if(SPE_Scount >= sizeof(SPE_Sbuf)) return;
		SPE_Sbuf[SPE_Scount++] = (SPE_TmpBits << (6 - SPE_LBits)) | 0xC0;
		SendBytes(SPE_Sbuf, SPE_Scount);
	#endif
	}
}

void SPE_Bytes_Send(uint8_t *bytes, uint16_t length)//发送多个字节,SPE_Byte_Send的多字节版
{
	uint16_t i;
	for(i=0;i<length;i++)
		SPE_Byte_Send(bytes[i]);
}

void SPE_Send_Packet(uint8_t *data, uint16_t length)
{
	uint16_t idx = 0;
#ifndef SPE_RAMLessMode
	SPE_Scount = 0;
#endif
	if(length)
	{
	#ifdef SPE_RAMLessMode
		SendByte((data[idx] >> 2) | 0x80);
	#else
		if(SPE_Scount >= sizeof(SPE_Sbuf)) return;
		SPE_Sbuf[SPE_Scount++] = (data[idx] >> 2) | 0x80;
	#endif
		SPE_TmpBits = data[idx] & 0x03;
		SPE_LBits = 2;
		idx++;
		while(idx < length)
		{
		#ifdef SPE_RAMLessMode
			SendByte((SPE_TmpBits << (7 - SPE_LBits)) | (data[idx] >> (SPE_LBits + 1)));
		#else
			if(SPE_Scount >= sizeof(SPE_Sbuf)) return;
			SPE_Sbuf[SPE_Scount++] = (SPE_TmpBits << (7 - SPE_LBits)) | (data[idx] >> (SPE_LBits + 1));
		#endif
			SPE_LBits ++;
			SPE_TmpBits = data[idx] & SPE_BitMasks[SPE_LBits];
			idx++;
			if(SPE_LBits == 7)
			{
			#ifdef SPE_RAMLessMode
				SendByte(SPE_TmpBits);
			#else
				if(SPE_Scount >= sizeof(SPE_Sbuf)) return;
				SPE_Sbuf[SPE_Scount++] = SPE_TmpBits;
			#endif
				SPE_LBits = 0;
				SPE_TmpBits = 0;
			}
		}
	#ifdef SPE_RAMLessMode
		SendByte((SPE_TmpBits << (6 - SPE_LBits)) | 0xC0);
	#else
		if(SPE_Scount >= sizeof(SPE_Sbuf)) return;
		SPE_Sbuf[SPE_Scount++] = (SPE_TmpBits << (6 - SPE_LBits)) | 0xC0;
		SendBytes(SPE_Sbuf, SPE_Scount);
	#endif
	}
}

void SPE_Receive_Byte(uint8_t byte)//串口接收一个字节调用一次本函数
{
	static volatile uint8_t ridx = 4;
	static volatile uint16_t rcount = 0;
	static volatile uint8_t rbits = 0;
	if((byte & 0xC0) == 0x80)
	{
		rcount = 0;
		SPE_Rbuf[rcount] = (byte & 0x3F) << 2;
		rbits = 6;
	}
	else if((byte & 0xC0) == 0xC0)
	{
		if(rbits != 8)
		{
			if(rcount >= SPE_MAX_PacketLength) return;
			SPE_Rbuf[rcount] |= ((byte & 0x3F) >> (rbits - 2));
			rcount ++;
		}
		SPE_ProcessData(SPE_Rbuf, rcount);
		rcount = 0;
		rbits = 8;
	}
	else
	{
		if(rbits == 1)
		{
			SPE_Rbuf[rcount] |= byte & 0x7F;
			rcount ++;
			rbits = 8;
		}
		else
		{
			if(rbits != 8)
			{
				SPE_Rbuf[rcount] |= ((byte & 0x7F) >> (rbits - 1));
				rcount ++;
			}
			if(rcount >= SPE_MAX_PacketLength) return;
			SPE_Rbuf[rcount] = (byte << (9 - rbits));
			rbits --;
		}
	}
}

void SPE_Receive_Bytes(uint8_t *bytes, uint16_t length)//串口接收数据调用本函数,SPE_Receive_Byte的多字节版
{
	uint16_t i;
	for(i = 0; i < length; i ++) SPE_Receive_Byte(bytes[i]);
}
