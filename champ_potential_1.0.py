# -*- coding: utf-8 -*-
"""
Project: SAPHIRE - 201
Program name: champ_potentiel_1.0.py
Author: Bruno DENIS
Date created: 20191214
Purpose: Path planning using robot

Revision History :

Date      Author      Ref  Revision (Date in YYYYMMDD format)
"""

# importation des modules utiles
import numpy as np                     # calcul scientifique avec des matrice
import matplotlib.pyplot as plt        # visualisation graphique des données
import matplotlib.gridspec as gridspec # to specified a grid on figure
from matplotlib.ticker import MultipleLocator, FormatStrFormatter
import mpl_toolkits.mplot3d            # complément à matplotlib pour le 3D
import matplotlib.patches as patches
import math




def draw_U_and_E(Q, U, E):
    """
    # inputs :
    #  Q: list of two soccer field meshes with coordonates 
    #   - Q[0] values of abscissa (type numpy.ndarray)
    #   - Q[1] values of ordonate (type numpy.ndarray)
    #  U: soccer field mesh with potential value (type numpy.ndarray)
    #  E: couple (vector) of two soccer field meshes or electric field 
    #   - E[0] x-coordonate value of electric field vector (type numpy.ndarray)
    #   - E[1] y-coordonate value of electric field vector (type numpy.ndarray)
    # output :
    #   - display plots on screen
    #   - fig: matploblib object with 2 plots
    """
    plt.close('all')

    # create and setup figure with ratio 1/3 and scaling 1 
    fig = plt.figure(figsize=plt.figaspect(1/3)*1) 
    fig.suptitle(glob_title, fontsize=16, horizontalalignment = 'center')    
    gs = gridspec.GridSpec(nrows=1, ncols=5) # split figure onto 5 cells
    
    # create a 3D draw spreading over the first 3 columns 
    ax1 = fig.add_subplot(gs[0, 0:-2], projection='3d', 
                          xlabel='x (mm)', ylabel='y (mm)')
    ax1.view_init(10, -60) # set the camera orientation
    # set interval for major tick on axis
    ax1.set_title(glob_title_1)
    ax1.xaxis.set_major_locator(MultipleLocator(500))
    ax1.yaxis.set_major_locator(MultipleLocator(500))
    #ax1.zaxis.set_major_locator(MultipleLocator(10000))
    ax1.plot_wireframe(Q[0], Q[1], U, rstride=1, cstride=1)
    
    # create a 2 draw spreading over the 2 last columns 
    ax2 = fig.add_subplot(gs[0, 3:], aspect='equal', 
                          xlabel='x (mm)', ylabel='y (mm)')
    # set interval for major tick on axis
    ax2.set_title(glob_title_2)
    ax2.xaxis.set_major_locator(MultipleLocator(500))
    ax2.yaxis.set_major_locator(MultipleLocator(500))
    ax2.yaxis.tick_right()
    ax2.quiver(Q[0], Q[1], E[0], E[1], scale=25*np.max(np.hypot(E[0],E[1])))
        
    plt.show() # display figure on screen
    
    return fig

def U_mesh_att(X, Y, goal):
    x_goal = goal[0]
    y_goal = goal[1]
    kappa = goal[2]
    return kappa * np.hypot(X-x_goal, Y-y_goal)
      
def E_mesh_att(X, Y, goal):
    x_goal = goal[0]
    y_goal = goal[1]
    kappa = goal[2]
    Distance_to_goal = np.hypot(X-x_goal, Y-y_goal)
    return -kappa/Distance_to_goal * (X-x_goal, Y-y_goal)
        
def E_attrative(x, y, goal):
    x_goal = goal[0]
    y_goal = goal[1]
    kappa_goal = goal[2]
    tmp = - kappa_goal / math.hypot(x - x_goal, y - y_goal)
    Ex, Ey = tmp * (x - x_goal), tmp * (y - y_goal)
    return np.array([Ex, Ey])

def U_mesh_rep(X, Y, obstacle):
    x_obstacle = obstacle[0]
    y_obstacle = obstacle[1]
    radius_obstable = obstacle[2]
    kappa_obstable = obstacle[3]
    Dist_to_center = np.hypot(X-x_obstacle, Y-y_obstacle)
    return np.where(Dist_to_center > radius_obstable, 
                    kappa_obstable/Dist_to_center, 
                    kappa_obstable/radius_obstable)

def E_mesh_rep(X, Y, obstacle):
    x_obstacle = obstacle[0]
    y_obstacle = obstacle[1]
    radius_obstable = obstacle[2]
    kappa_obstable = obstacle[3]
    Dist_to_center = np.hypot(X-x_obstacle, Y-y_obstacle)
    return np.where(Dist_to_center > radius_obstable,
                    (kappa_obstable/Dist_to_center**3)*(X-x_obstacle, Y-y_obstacle),
                    0)

def E_repulsive(x, y, obstacle_list):
    
    Ex, Ey = 0, 0
    
    for obstacle in obstacle_list:
    
        x_obstable = obstacle[0]
        y_obstable = obstacle[1]
        radius_obstable = obstacle[2]
        kappa_obstable = obstacle[3]
        
        distance_obstacle = math.hypot(x - x_obstable, y - y_obstable)
        
        if distance_obstacle > radius_obstable: # 2 fois radius ?
            tmp = kappa_obstable/distance_obstacle**3
            Ex += tmp * (x - x_obstable)
            Ey += tmp * (y - y_obstable)
            
    return np.array([Ex, Ey])

# =============================
#
#   Start of main programme
#
# =============================

# Field size (units are millimeters)
field_xmin, field_xmax = -2720./2, 2720./2
field_ymin, field_ymax = -1990./2, 1990./2
field_length = field_xmax - field_xmin
robot_radius = 90.

# Creation of a 100 mm x 100 mm grid for q locations
X = np.arange(field_xmin, field_xmax, 100)
Y = np.arange(field_ymin, field_ymax, 100)
Q = np.meshgrid(X, Y)

goal = (500., -200., 2./field_length) # x_goal, y_goal, kappa
# obstacle_list = [ (x_goal, y_goal, radius, kappa), ... ]
obstacle_list = [(-700., -300., 1*robot_radius, .5*robot_radius),
                 (   0.,  100., 1*robot_radius, .5*robot_radius)]

# ------------------------------
#     Attractive potential
# ------------------------------

Uatt = U_mesh_att(Q[0], Q[1], goal)
Eatt = E_mesh_att(Q[0], Q[1], goal)
Eatt_norm = Eatt / goal[2]

glob_title = 'Potentiel attractif conique '
glob_title += '$U_{att}(q) = \kappa \, || \overrightarrow{q_{obstacle}q} ||$'
glob_title_1 = 'Champ de potentiel $U_{att}(x,y)$'
glob_title_2 = 'Champ électrique $\overrightarrow{E}_{att}(x,y)$'
fig = draw_U_and_E(Q, Uatt, Eatt)
fig.savefig('potentiel_attractif_conique.png', dpi=300, format='png')
print('Eatt[0] maxi =', np.max(Eatt[0]))
    
# ------------------------------
#     Repulsif potentials
# ------------------------------
Urep_list, Erep_list, Erep_norm_list = list(), list(), list()
n = 1

for obstacle in obstacle_list:
    qobstacle  = (obstacle[0], obstacle[1])
    Robstacle = obstacle[2]
    kappa = obstacle[3]
    
    Urep_list.append( U_mesh_rep(Q[0], Q[1], obstacle) )
    Erep_list.append( E_mesh_rep(Q[0], Q[1], obstacle) )
    Erep_norm_list.append( Erep_list[-1]/np.amax(np.absolute(Erep_list[-1])) )
    
    glob_title = 'Potentiel répulsif en loi inverse '
    glob_title += '$U_{rep}(q) = \kappa \,/\, ||\overrightarrow{q_{obstacle}q}||$'
    glob_title += f' : obstacle en ({obstacle[0]:n}, {obstacle[1]:n})'
    glob_title_1 = 'Champ de potentiel $U_{rep}(x,y)$'
    glob_title_2 = 'Champ électrique $\overrightarrow{E}_{rep}(x,y)$'
    fig = draw_U_and_E(Q, Urep_list[-1], Erep_list[-1])
    fig.savefig(f'potentiel_repulsif_1surR_{n}.png', dpi=300, format='png')
    n += 1
    print('Eatt[0] maxi =', np.max(Erep_list[-1][0]))


# ------------------------------
#   Cumulative potential
# ------------------------------

Ucumul = Uatt
for U in Urep_list:
    Ucumul += U
Ecumul = Eatt_norm
for E in Erep_norm_list:
    Ecumul += E
fig = draw_U_and_E(Q, Ucumul, Ecumul)
fig.savefig('superposition.png', dpi=300, format='png')

plt.close('all')
fig,ax = plt.subplots(1)

#fig = plt.figure(figsize=plt.figaspect(3/2)*1) 
#ax = fig.add_axes(aspect='equal', xlabel='x (mm)', ylabel='y (mm)')
#set interval for major tick on axis
ax.set_title("title")
ax.xaxis.set_major_locator(MultipleLocator(500))
ax.yaxis.set_major_locator(MultipleLocator(500))
ax.yaxis.tick_right()
ax.set_aspect('equal')
ax.quiver(Q[0], Q[1], Ecumul[0], Ecumul[1], scale = 25)
print('Eatt[0] cumul =', np.max(Ecumul[0]))

for obstacle in obstacle_list:
    circ = patches.Circle((obstacle[0], obstacle[1]), obstacle[2])
    ax.add_patch(circ)
#plt.show() # display figure on screen
    

# --------------------------------

t, dt = 0., 1./5 # Date de départ et incrément de temps en seconde
Qr = np.array([-1300., 500.]) # Coordonnées du robot en millimètre
vr_norm = 200 # Norme de la vitesse du robot en mètre par seconde
Path = [[Qr[0]],[Qr[1]]]

while t < 12. :
    t += dt
    E = E_attrative(Qr[0], Qr[1], goal) + E_repulsive(Qr[0], Qr[1], obstacle_list)
    Qr = Qr + (vr_norm * dt * E / np.linalg.norm(E))
    print(Qr)
    Path[0].append(Qr[0])
    Path[1].append(Qr[1])
    
ax.plot(Path[0], Path[1], color='red')


plt.show() # display figure on screen
 

print(Path)

# ----------
# exemple de mesh grid
X = np.arange(-20, 30, 10) # X = array([-20, -10,   0,  10,  20])
Y = np.arange(-10, 20, 10) # Y = array([-10,   0,  10])
Q = np.meshgrid(X, Y)

