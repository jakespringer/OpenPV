"""
Make a histogram of normally distributed random numbers and plot the
analytic PDF over it
"""
import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
import matplotlib.cm as cm
import matplotlib.image as mpimg
import PVReadWeights as rw
import PVReadSparse as rs
import math

"""
mi=mpimg.imread(sys.argv[3])
imgplot = plt.imshow(mi, interpolation='Nearest')
imgplot.set_cmap('hot')
plt.show()
"""

def nearby_neighbor(kzPre, zScaleLog2Pre, zScaleLog2Post):
   a = math.pow(2.0, (zScaleLog2Pre - zScaleLog2Post))
   ia = a

   if ia < 2:
      k0 = 0
   else:
      k0 = ia/2 - 1

   if a < 1.0 and kzPre < 0:
      k = kzPre - (1.0/a) + 1
   else:
      k = kzPre

   return k0 + (a * k)


def zPatchHead(kzPre, nzPatch, zScaleLog2Pre, zScaleLog2Post):
   a = math.pow(2.0, (zScaleLog2Pre - zScaleLog2Post))

   if a == 1:
      shift = -(0.5 * nzPatch)
      return shift + nearby_neighbor(kzPre, zScaleLog2Pre, zScaleLog2Post)

   shift = 1 - (0.5 * nzPatch)

   if (nzPatch % 2) == 0 and a < 1:
      kpos = (kzPre < 0)

      if kzPre < 0:
         kpos = -(1+kzPre)
      else:
         kpos = kzPre

      l = (2*a*kpos) % 2
      if kzPre < 0:
         shift -= l == 1
      else:
         shift -= l == 0
   elif (nzPatch % 2) == 1 and a < 1:
      shift = -(0.5 * nzPatch)

   neighbor = nearby_neighbor(kzPre, zScaleLog2Pre, zScaleLog2Post)

   if nzPatch == 1:
      return neighbor

   return shift + neighbor
"""
a = zPatchHead(int(sys.argv[1]), 5, -math.log(4, 2), -math.log(1, 2))
print a
print int(a)
sys.exit()
"""



vmax = 100.0 # Hz
space = 1
extended = False


w = rw.PVReadWeights(sys.argv[1])
wOff = rw.PVReadWeights(sys.argv[2])


nx  = w.nx
ny  = w.ny
nxp = w.nxp
nyp = w.nyp


nx_im = nx * (nxp + space) + space
ny_im = ny * (nyp + space) + space

predub = np.zeros(((nx*nx),(nxp * nxp)))
predubOff = np.zeros(((nx*nx),(nxp * nxp)))


numpat = w.numPatches
print "numpat = ", numpat

for k in range(numpat):
   p = w.next_patch()
   pOff = wOff.next_patch()

   predub[k] = p
   predubOff[k] = pOff

print "weights done"

#print "p = ", P
#if k == 500:
#   sys.exit()


#end fig loop




activ = rs.PVReadSparse(sys.argv[3], extended)


end = int(sys.argv[4])
step = int(sys.argv[5])
begin = int(sys.argv[6])

count = 0
for end in range(begin+step, end, step):
   A = activ.avg_activity(begin, end)


   this = 7 + count
   count += 1
   print "this = ", this
   print "file = ", sys.argv[this]
   print
   numrows, numcols = A.shape

   min = np.min(A)
   max = np.max(A)

   s = np.zeros(numcols)
   for col in range(numcols):
       s[col] = np.sum(A[:,col])
   s = s/numrows

   b = np.reshape(A, (len(A)* len(A)))

   c = np.shape(b)[0]
   
   mi=mpimg.imread(sys.argv[this])


   print "a w start"
   rr = nx / 64
   im = np.zeros((64, 64))


   for yi in range(len(A)):
      for xi in range(len(A)):
         x = int(zPatchHead(int(xi), 5, -math.log(rr, 2), -math.log(1, 2)))
         y = int(zPatchHead(int(yi), 5, -math.log(rr, 2), -math.log(1, 2)))
         if (65-nxp) > x >= 0 and (65-nxp) > y >= 0:
            if A[yi, xi] > 0:
               patch = predub[yi * (nx) + xi]
               patchOff = predubOff[yi * (nx) + xi]

               patch = np.reshape(patch, (nxp, nxp))
               patchOff = np.reshape(patchOff, (nxp, nxp))
               for yy in range(nyp):
                  for xx in range(nxp):
                     im[y + yy, x + xx] += patch[yy, xx] * A[yi, xi]
                     im[y + yy, x + xx] -= patchOff[yy, xx] * A[yi, xi]

   
   im = np.ravel(im)
   mi = np.ravel(mi)
   print "mi = ", np.max(mi)

   print "im shape = ", np.shape(im)

   
   tnum = 64*64

   rocim = np.zeros(((tnum*10), 2)) 
   print "rocim shape = ", np.shape(rocim)
   countw = 0
   countg = 0
   for g in range(10):
      w = (g+1) / 10.

      immax=np.max(im) * w
      #print "immax = ", immax
      pos = 0
      fpos = 0

      for i in range(tnum):
         if im[i] >= immax:
            if mi[i] == 1:
               rocim[countg,0] = 1
               rocim[countg,1] = w
               pos+=1
            elif mi[i] == 0:
               rocim[countg,0] = -1
               rocim[countg,1] = w
               fpos+=1
            else:
               print "stopped"
               sys.exit()
            countg+=1

      #print "pos = ", pos
      #print "fpos = ", fpos
   #print "g count = ", countg
   rocim = rocim[0:countg]

   np.savetxt("roc-info%o.txt" %(count), rocim, fmt='%f', delimiter = ';')

  
  
  
  
