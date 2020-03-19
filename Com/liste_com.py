# -*- coding: utf-8 -*-
"""
Created on Fri Feb 28 09:36:21 2020

@author: Leopold
"""

import serial.tools.list_ports
X = serial.tools.list_ports.comports()
print("il y a {} ports".format(len(X)))
for elem in X:
    print("{} : {}".format(elem.device, elem.description))