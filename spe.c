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
	volatile static uint8_t SPE_Sbuf[(SPE_MAX_PacketLength+2)/3*4];
#endif
volatile static uint8_t SPE_Rbuf[SPE_MAX_PacketLength];
volatile static uint8_t SPE_SIndex = 4;
volatile static uint8_t SPE_tmpbit = 0;

__weak void SendByte(uint8_t byte)//测试代码 * 串口发送一个字节，如果其他代码中未实例此函数，会执行测试代码
{
	SPE_Receive_Byte(byte);
}

#ifndef SPE_RAMLessMode
__weak void SendBytes(uint8_t *bytes, uint16_t length)//测试代码 * 串口发送多个字节，如果其他代码中未实例此函数，会执行测试代码，低内存占用模式时不可用
{
	printf("SendBytes\n");
	SPE_Receive_Bytes(bytes, length);
}
#endif

__weak void SPE_ProcessData(uint8_t *data, uint16_t length)//测试代码 * 接收到数据处理，如果其他代码中未实例此函数，会执行测试代码
{
	uint16_t i;
	for(i=0;i<length;i++)
		printf("%c",data[i]);
	printf("\n");
}

void SPE_Start_Send(void)//数据包开始
{
	SPE_SIndex = 0;
	#ifndef SPE_RAMLessMode
		SPE_Scount = 0;
	#endif
}

void SPE_Byte_Send(uint8_t bits)//发送一个字节
{
	switch(SPE_SIndex)
	{
	case 0:
		#ifdef SPE_RAMLessMode
			SendByte(((bits >> 2) & 0x3F) | 0x80);
		#else
			if(SPE_Scount >= sizeof(SPE_Sbuf)) break;
			SPE_Sbuf[SPE_Scount++] = (((bits>>2)&0x3F)|0x80);
		#endif
		SPE_tmpbit = bits & 0x03;
		SPE_SIndex = 1;
		break;
	case 3:
		#ifdef SPE_RAMLessMode
			SendByte(SPE_tmpbit);
		#else
			if(SPE_Scount >= sizeof(SPE_Sbuf)) break;
			SPE_Sbuf[SPE_Scount++] = SPE_tmpbit;
		#endif
		#ifdef SPE_RAMLessMode
			SendByte((bits >> 2) & 0x3F);
		#else
			if(SPE_Scount >= sizeof(SPE_Sbuf)) break;
			SPE_Sbuf[SPE_Scount++] = ((bits>>2)&0x3F);
		#endif
		SPE_tmpbit = bits & 0x03;
		SPE_SIndex = 1;
		break;
	case 1:
		#ifdef SPE_RAMLessMode
			SendByte(((bits >> 4) & 0x0F) | (SPE_tmpbit << 4));
		#else
			if(SPE_Scount >= sizeof(SPE_Sbuf)) break;
			SPE_Sbuf[SPE_Scount++] = (((bits >> 4) & 0x0F) | (SPE_tmpbit<<4));
		#endif
		SPE_tmpbit = bits & 0x0F;
		SPE_SIndex = 2;
		break;
	case 2:
		#ifdef SPE_RAMLessMode
			SendByte(((bits >> 6) & 0x03) | (SPE_tmpbit << 2));
		#else
			if(SPE_Scount >= sizeof(SPE_Sbuf)) break;
			SPE_Sbuf[SPE_Scount++] = (((bits >> 6) & 0x03) | (SPE_tmpbit << 2));
		#endif
		SPE_tmpbit = bits & 0x3F;
		SPE_SIndex = 3;
		break;
	default:
		break;
	}
}

void SPE_End_Send(void)//数据包结束
{
	switch(SPE_SIndex)
	{
	case 0:
		#ifdef SPE_RAMLessMode
			SendByte(0xC0);
		#else
			SPE_Sbuf[SPE_Scount++] = 0xC0;
			SendBytes(SPE_Sbuf, SPE_Scount);
		#endif
		SPE_SIndex = 4;
		break;
	case 1:
		SPE_tmpbit <<= 4;
	case 2:
		SPE_tmpbit <<= 2;
	case 3:
		#ifdef SPE_RAMLessMode
			SendByte(SPE_tmpbit | 0xC0);
		#else
			if(SPE_Scount >= sizeof(SPE_Sbuf)) break;
			SPE_Sbuf[SPE_Scount++] = SPE_tmpbit | 0xC0;
			SendBytes(SPE_Sbuf, SPE_Scount);
		#endif
		SPE_SIndex = 4;
		break;
	default:
		break;
	}
}

void SPE_Bytes_Send(uint8_t *bytes, uint16_t length)//发送多个字节,SPE_Byte_Send的多字节版
{
	uint16_t i;
	for(i=0;i<length;i++)
		SPE_Byte_Send(bytes[i]);
}

void SPE_Receive_Byte(uint8_t byte)//串口接收一个字节调用一次本函数
{
	static volatile uint8_t ridx = 4;
	static volatile uint16_t rcount = 0;
	if((byte & 0xC0) == 0x80)
	{
		SPE_Rbuf[0] = (byte & 0x3F) << 2;
		rcount = 0;
		ridx = 1;
	}
	else
	{
		switch(ridx)
		{
		case 0:
			SPE_Rbuf[rcount] = (byte & 0x3F) << 2;
			ridx = 1;
			break;
		case 1:
			SPE_Rbuf[rcount] |= ((byte >> 4) & 0x03);
			rcount++;
			SPE_Rbuf[rcount] = (byte << 4) & 0xF0;
			ridx = 2;
			break;
		case 2:
			SPE_Rbuf[rcount] |= ((byte >> 2) & 0x0F);
			rcount++;
			SPE_Rbuf[rcount] = (byte << 6) & 0xC0;
			ridx = 3;
			break;
		case 3:
			SPE_Rbuf[rcount] |= (byte & 0x3F);
			rcount++;
			ridx = 0;
			break;
		}
		if((byte & 0xC0) == 0xC0)
		{
			ridx = 4;
			SPE_ProcessData(SPE_Rbuf, rcount);
		}
	}
}

void SPE_Receive_Bytes(uint8_t *bytes, uint16_t length)//串口接收数据调用本函数,SPE_Receive_Byte的多字节版
{
	uint16_t i;
	for(i = 0; i < length; i ++) SPE_Receive_Byte(bytes[i]);
}

__weak int main()//测试代码
{
	uint8_t hello[]="Hello world!";
	SPE_Start_Send();
	SPE_Bytes_Send(hello, strlen(hello));
	SPE_End_Send();
}