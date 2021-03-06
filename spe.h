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
#ifndef __SPE_4638_H__
#define __SPE_4638_H__
	#include <stdint.h>

	#define SPE_MAX_PacketLength	16	//设置每个数据包最大包含字节数
	#define SPE_RAMLessMode				//低内存占用模式，为低端单片机设计,此模式时每次单片机串口发送一个字节

	void SendByte(uint8_t byte); //串口发送一个字节，需要在其他代码中实现
	void SPE_ProcessData(uint8_t *data, uint32_t length); //接收到数据处理，需要在其他代码中实现
#ifndef SPE_RAMLessMode
	void SendBytes(uint8_t *bytes, uint32_t length); //串口发送多个字节，需要在其他代码中实现，低内存占用模式时不可用
#endif
	void SPE_Start_Send(void);//数据包开始
	void SPE_Byte_Send(uint8_t bits);//发送一个字节
	void SPE_End_Send(void);//数据包结束
	void SPE_Bytes_Send(uint8_t *bytes, uint32_t length);//发送多个字节,SPE_Byte_Send的多字节版

	void SPE_Send_Packet(uint8_t *data, uint32_t length);//另一种发送方式，必须单独使用
	
	void SPE_Receive_Byte(uint8_t byte);//串口接收一个字节调用一次本函数
	void SPE_Receive_Bytes(uint8_t *bytes, uint32_t length);//串口接收数据调用本函数,SPE_Receive_Byte的多字节版
#endif //__SPE_4638_H__
