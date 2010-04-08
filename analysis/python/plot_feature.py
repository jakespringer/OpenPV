"""
Plot a color map of features of given phase
"""
import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
import matplotlib.cm as cm
import PVReadWeights as rw

def format_coord(x, y):
    col = int(x+0.5)
    row = int(y+0.5)
    if col>=0 and col<numcols and row>=0 and row<numrows:
        z = M[row,col]
        return 'x=%1.4f, y=%1.4f, z=%1.4f'%(x, y, z)
    else:
        return 'x=%1.4f, y=%1.4f'%(x, y)

"""
Show how to modify the coordinate formatter to report the image "z"
value of the nearest pixel given x and y
"""

if len(sys.argv) < 3:
   print "usage: plot_feature filename phase"
   sys.exit()

phase = int(sys.argv[2])

w = rw.PVReadWeights(sys.argv[1])


# feature for given phase
#
f = np.zeros(w.patchSize)
f[ range(phase, w.patchSize, w.nxp) ] = 1.0
f = w.normalize(f)

numrows = w.nyGlobal
numcols = w.nxGlobal

M = np.zeros(w.numPatches)

for i in range(len(M)):
   p = w.normalize( w.next_patch() )
   M[i] = np.sum(p * f)

M = M.reshape( (numrows,numcols) )

# print averaged projection over column
#
s = np.zeros(numcols)
maxs = 0.0
maxcol = 0
for col in range(numcols):
   s[col] = np.sum(M[:,col])
   if s[col] > maxs: maxs = s[col]; maxcol = col
print "(maxcol, maxsum) = (", maxcol, ",", maxs/numrows, ")"

fig = plt.figure()

ax = fig.add_subplot(2,1,1)
ax.set_xlabel('Kx GLOBAL')
ax.set_ylabel('Ky GLOBAL')
ax.set_title( 'Feature Projection: phase=%d' %(phase) )
ax.format_coord = format_coord
ax.imshow(M, cmap=cm.jet, interpolation='nearest', vmin=0., vmax=1.)


ax = fig.add_subplot(2,1,2)
ax.set_ylabel('Ky Avg Projection')
ax.plot(s, 'o')

plt.show()
