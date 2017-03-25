spe
============
This is a design of the data link layer protocol for serial port.

Principle of protocol：
----------------------

Split the original data by bits, Insert 1-2 control bits packaged into new data.

For example, The original data in binary representation: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

new data: 10AAAAAA 0AABBBBB 0BBBCCCC 0CCCCDDD 11DDDDD0

bus.c is a demo
