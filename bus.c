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
#include <string.h>
#include "spe.h"


void SendByte(uint8_t byte)//测试代码 把发送出去的数据发送给接收函数
{
	SPE_Receive_Byte(byte);
}

#ifndef SPE_RAMLessMode
void SendBytes(uint8_t *bytes, uint32_t length)//测试代码 把发送出去的数据发送给接收函数
{
	SPE_Receive_Bytes(bytes, length);
}
#endif

void SPE_ProcessData(uint8_t *data, uint32_t length)//接收到的数据
{
	uint8_t idx = data[0];
	int32_t *val = data + 1;
	if(idx == 3) printf("字符串(%d):%s\n",idx,data+1);
	else printf("数值(%d):%d\n",idx,*val);
}

void Send8(uint8_t val)
{
	SPE_Byte_Send(val);
}

void Send16(uint16_t val)
{
	SPE_Bytes_Send(&val, 2);
}

void Send32(uint32_t val)
{
	SPE_Bytes_Send(&val, 4);
}

void SendValue(uint8_t index, uint32_t value)//发送数值
{
	SPE_Start_Send();
	Send8(index);
	Send32(value);
	SPE_End_Send();
}

void SendString(uint8_t index, const char *str)//发送字符串
{
	SPE_Start_Send();
	Send8(index);
	SPE_Bytes_Send(str, strlen(str));
	Send8(0);
	SPE_End_Send();
}

int main()//测试代码
{
	uint8_t hello[]="Hello world!";
	SendValue(1, 3);
	SendValue(2, -1);
	SendString(3, hello);
}
