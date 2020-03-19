#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Jan 17 09:53:54 2020

@author: psl
"""

from psl_package import paris_saclay_league as psl
import time
import math as m
import numpy as np

rayon_robot=90.
field_xmin, field_xmax = -2720./2, 2720./2
field_ymin, field_ymax = -1990./2, 1990./2
field_length = field_xmax - field_xmin
pf=2./field_length
pobst=5.*rayon_robot
vit_lin=1.
vit_ang=1.
dmin=1.5
vit_appr=0.1
xtir,ytir=200,0
xpasse,ypasse=500,-200

def stop(i,team):
    tcontrol = psl.SSLgrSimClient('127.0.0.1', 20011)
    tcontrol.connect()
    packet_b0 = psl.grSim_pb2.grSim_Packet()
    commands_b0 = packet_b0.commands
    commands_b0.timestamp = time.time()
    if team=='y':
        commands_b0.isteamyellow = True
    else :
        commands_b0.isteamyellow = False
    robot_commands_b0 = commands_b0.robot_commands.add()
    robot_commands_b0.id = i
    robot_commands_b0.kickspeedx = 0.
    robot_commands_b0.kickspeedz = 0.
    robot_commands_b0.veltangent = 0.
    robot_commands_b0.velnormal  = 0.
    robot_commands_b0.velangular = 0. # Ask for rotation
    robot_commands_b0.spinner = False
    robot_commands_b0.wheelsspeed = False
    tcontrol.send(packet_b0)
    tcontrol.close()
    
def cadran(x,y):
    if x<0 and y<0:
        return '1'
    elif x<0 and y>0:
        return '2'
    elif x>0 and y>0:
        return '3'
    else : 
        return '4'
    
def positionall():
    t1=time.time()
    vision = psl.SSLVisionClient('224.5.23.2', 10020)
    vision.connect()
    tcontrol = psl.SSLgrSimClient('127.0.0.1', 20011)
    tcontrol.connect()
    L=[]
    while len(L)!=5:
        data=vision.receive()
        if len(data.detection.robots_yellow) != 0 :
            for r in data.detection.robots_yellow:
                if r.robot_id==0:
                    dy0=data
                    if 'y0' not in L:
                        L.append('y0')
                else :
                    dy1=data
                    if 'y1' not in L:
                        L.append('y1')
        elif len(data.detection.robots_blue)!=0:
            for r in data.detection.robots_blue:
                if r.robot_id==1:
                    db1=data
                    if 'b1' not in L:
                        L.append('b1')
                else :
                    db0=data
                    if 'b0' not in L:
                        L.append('b0')
        if len(data.detection.balls)!=0:
            dball=data
            if 'ball' not in L:
                L.append('ball')
    t2=time.time()
    Pos=dict()
    Pos[('b',0)]=db0
    Pos[('b',1)]=db1
    Pos[('y',0)]=dy0
    Pos[('y',1)]=dy1    
    Pos['ball']=dball
    return Pos

tcontrol = psl.SSLgrSimClient('127.0.0.1', 20011)
tcontrol.connect()

class ball():
    def __init__(self):
        pos=positionall()
        for r in pos['ball'].detection.balls:
            ball.x=r.x
            ball.y=r.y
            
class robot():
    def __init__(self,i,team):
        self.id=i
        self.team=team
        self.packet=psl.grSim_pb2.grSim_Packet()
        self.commands_b0 = self.packet.commands
        self.commands_b0.timestamp = time.time()
        self.command=self.commands_b0.robot_commands.add()
        self.command.id = i
        self.command.kickspeedx = 0.
        self.command.kickspeedz = 0.
        self.command.veltangent = 0.
        self.command.velnormal  = 0.
        self.command.velangular = 0. # Ask for rotation
        self.command.spinner = False
        self.command.wheelsspeed = False
        self.commands_b0.isteamyellow = (self.team=='y')
    def state(self,etat):
        self.etat=etat
    def pos(self):
        Pos=positionall()
        if self.team=='b':
            for r in Pos[(self.team,self.id)].detection.robots_blue:
                self.x=r.x
                self.y=r.y
                self.o=r.orientation
        else :
            for r in Pos[(self.team,self.id)].detection.robots_yellow:
                self.x=r.x
                self.y=r.y
                self.o=r.orientation
    def is_(self,etat):
        return self.etat==etat
#    def ang_to(self,xf,yf):
#        cad=cadran(self.x,self.y)
#        if cad == '3':
#            thetal=m.atan((yf-self.y)/(xf-self.x))-m.pi
#        elif cad == '4':
#            thetal=m.atan((yf-self.y)/(xf-self.x))+m.pi
#        elif cad == '2' : 
#            thetal=m.atan((yf-self.y)/(xf-self.x))
#        else :
#            thetal=-m.atan((yf-self.y)/(xf-self.x))+m.pi/2
#        self.angle=self.o-m.atan((yf-self.y)/(xf-self.x))
#        return self.o-m.atan((yf-self.y)/(xf-self.x))
    def dist_to(self,xf,yf):
        return (((self.x-xf)**2+(self.y-yf)**2)**1/2)/10000
    def hasball(self,xball,yball):
        return self.dist_to(xball,yball)<1
    def velang(self,v):
        self.command.velangular = -m.copysign(v,self.angle)
    def veltoutdroit(self,v):
        self.command.veltangent = v
    def catch(self):
        self.command.spinner = True
    def let(self):
        self.command.spinner = False
    def tir(self,v):
        self.command.kickspeedx = v
    def send(self):
        tcontrol.send(self.packet)
        
def stopall(l):
    for r in l:
        stop(r.i,r.team)
        
def strat():
    rb0=robot(0,'b')
    rb1=robot(1,'b')
    ry0=robot(0,'y')
    ry1=robot(1,'y')
    ball()
    robots=[rb0,rb1,ry0,ry1]
    for r in robots:
        r.pos()
    if cadran(ry0.x,ry0.y)<3 and cadran(ry1.x,ry1.y)<3 and cadran(ball.x,ball.y)<3:
        stratb="defense"
        if ry0.hasball() or ry1.hasball():
            stratb="defurgent"
    if cadran(ball.x,ball.y)>2 and cadran(rb0.x,rb0.y)>2 and cadran(rb1.x,rb1.y)>2:
        stratb="attaque"
        if rb0.hasball() or rb1.hasball():
            stratb="atturgent"
    return stratb
            
def mouvement(r,xf,yf,obstacles):  
    Eatt = np.array([-pf*(r.x-xf)/m.hypot(xf-r.x,yf-r.y),-pf*(r.y-yf)/m.hypot(xf-r.x,yf-r.y)])
    Erep = np.array([0,0])
    for obst in obstacles :
        xo=obst[0]
        yo=obst[1]
        ro=obst[2]
        do=m.hypot(r.x-xo,r.y-yo)
        if do>ro:
            Erep[0]+=pobst*(r.x-xo)
            Erep[1]+=pobst*(r.y-yo)
    E=Eatt+Erep
    angle=np.angle(E)
    if abs(r.o-angle)>0.1:
        r.velang(vit_ang)
    else:
        r.velang(0)
    r.send()
    if r.dist_to(xf,yf)>1.5*dmin:
        r.veltoutdroit(vit_lin/5)
    elif r.dist_to(xf,yf)>dmin:
        r.veltoutdroit(vit_appr)
        r.catch()
    else:
        r.veltoutdroit(0.)
        r.state('poss')
    r.send()
    
def gotocatch(r,x,y):
    if abs(r.ang_to(x,y))>0.1:
        r.velang(vit_ang)
    else:
        r.velang(0.)
    r.send()
    if r.dist_to(x,y)>1.5*dmin:
        r.veltoutdroit(vit_lin/5)
    elif r.dist_to(x,y)>dmin:
        r.veltoutdroit(vit_appr)
        r.catch()
    else:
        r.veltoutdroit(0.)
        r.state('poss')
    r.send()
    
def goto(r,x,y,newstate):
    if abs(r.ang_to(x,y))>0.1:
        r.velang(vit_ang)
        print('a')
    else:
        r.velang(0.)
    r.send()
    if r.dist_to(x,y)>300:
        r.veltoutdroit(vit_lin/5)
        print('b')
    else :
        r.veltoutdroit(0.)
        r.state(newstate)
    r.send()
        
def recept(r,x,y):
    if abs(r.ang_to(x,y))>0.1:
        r.velang(vit_ang)
    else:
        r.velang(0.)
        r.catch()
    r.send()
    
    
        
        
def jeu():
    """
    """
    robotb0=robot(0,'b')
    robotb1=robot(1,'b')
    roboty0=robot(0,'y')
    roboty1=robot(1,'y')
    robots=[robotb0,robotb1,roboty0,roboty1]
    while True :
        ball()
        robots[1].pos()
        robots[0].pos()
        if 1==1:
            if robots[0].dist_to(ball.x,ball.y)<robots[1].dist_to(ball.x,ball.y):
                robots[0].state('chball')
                robotball=robots[0]
                robots[1].state("placement")
                robotpasse=robots[1]
            else:
                robots[1].state("chball")
                robotball=robots[1]
                robots[0].state("placement")
                robotpasse=robots[0]
            if robotball.is_('chball'):
                robotball.pos()
                gotocatch(robotball,ball.x,ball.y)
            if robotball.is_('poss'):
                robotball.pos()
                goto(robotball,xtir,ytir,"tir")
            if robotball.is_('tir'):
                robotball.pos()
                robotball.let()
                robotball.tir(5)
                robotball.send()
                stop(0,'b')
            if robotpasse.is_("placement"):
                robotpasse.pos()
                goto(robotpasse,xpasse,ypasse,"placé")
            if robotpasse.is_("placé") and robotball.is_("poss+passe"):
                robotpasse.pos()
                robotball.pos()
                recept(robotpasse,robotball.x,robotball.y)
            
            
                
            
            