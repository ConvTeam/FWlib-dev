#!/usr/bin/python

import time
from socket import *

DelayTime = 1
dstAddr = "\x00\x08\xDC\x11\x22\x33"
s = socket(AF_PACKET, SOCK_RAW, 0x0000)
s.bind("eth0",0x0000)

txFrame = "\xFF\xFF\xFF\xFF\xFF\xFF" + (dstAddr * 16)

while 1:
	time.sleep(DelayTime) 
	ret = s.send(txFrame)
	print "Sent Magic Packet!"
