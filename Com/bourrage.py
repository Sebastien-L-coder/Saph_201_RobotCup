# -*- coding: utf-8 -*-
"""
Created on Fri Feb 28 11:02:24 2020

@author: Leopold
"""
import serial

class Robot():
    def __init__(self, nom, com):
        self.nom = nom
        self.BT = serial.Serial(com, timeout=0.1)
        self.pos = [0, 0, 0]
        self.commande_pos = [0, 0, 0]
        self.commande_tir = [0, 0]
    
    def __del__(self):
        self.BT.close()
        del self.BT
        
    def __repr__(self):
        return "Robot {}, en {}".format(self.nom, self.pos)
        
    def update(self, forced = False):
        if self.BT.out_waiting == 0 or forced:
            pos = self.commande_pos
            tir = self.commande_tir
            chaine = "{},{},{},{},{};".format(pos[0], pos[1], pos[2], tir[0], tir[1])
            print(chaine)
            self.BT.write(chaine.encode("ASCII"))
    
    def set_commande_pos(self, frontal, lateral, rotation):
        self.commande_pos = [frontal, lateral, rotation]
        
    def stop(self):
        self.set_commande_pos(0, 0, 0)
        self.update(True)
    