/*
 * LineSegments.cpp
 *
 *  Created on: Jan 26, 2009
 *      Author: rasmussn
 */

#include "SubunitConn.hpp"
#include "../io/io.h"
#include <assert.h>
#include <string.h>

namespace PV {

/**
 * This connection is for retina to layer with 4 x 16 features.  The post-synaptic layer
 * exhaustively computes presence of a hierarchy of 4 x 2x2 (on/off) patch of pixels
 * (embedded in a 3x3 pixel patch).
 */
SubunitConn::SubunitConn(const char * name, HyPerCol * hc, HyPerLayer * pre, HyPerLayer * post)
//    : HyPerConn(name, hc, pre, post)
{
   this->connId = hc->numberOfConnections();
   this->name   = strdup(name);
   this->parent = hc;

   // default for this connection is 1 weight patch
   this->numAxonalArborLists = pre->clayer->numFeatures;

   initialize(NULL, pre, post, CHANNEL_EXC);

   hc->addConnection(this);
}

/**
 * Initialize weights to all values of 4 x 2x2 patch
 */
int SubunitConn::initializeWeights(const char * filename)
{
   assert(post->clayer->numFeatures == 4*16);

   const int arbor = 0;
   const int numPatches = numberOfWeightPatches(arbor);
   for (int i = 0; i < numPatches; i++) {
      weights(wPatches[arbor][i]);
   }

   return 0;
}

int SubunitConn::weights(PVPatch * wp)
{
   pvdata_t * w = wp->data;

   const int nx = (int) wp->nx;
   const int ny = (int) wp->ny;
   const int nf = (int) wp->nf;

   const int sx = (int) wp->sx;  assert(sx == nf);
   const int sy = (int) wp->sy;  assert(sy == nf*nx);
   const int sf = (int) wp->sf;  assert(sf == 1);

   assert(nx == 3);
   assert(ny == 3);
   assert(nf == 4*16);

   // TODO - already initialized to zero (so delete)
   for (int k = 0; k < nx*ny*nf; k++) {
      w[k] = 0.0;
   }

   for (int f = 0; f < nf; f++) {
      int i0, j0;
      int kf = f / 16;
      if (kf == 0) {i0 = 0; j0 = 0;}
      if (kf == 1) {i0 = 1; j0 = 0;}
      if (kf == 2) {i0 = 0; j0 = 1;}
      if (kf == 3) {i0 = 1; j0 = 1;}

      kf = f % 16;

      for (int j = 0; j < 2; j++) {
         for (int i = 0; i < 2; i++) {
            int n = i + 2*j;
            int r = kf >> n;
            r = 0x1 & r;
            w[(i+i0)*sx + (j+j0)*sy + f*sf] = r;
         }
      }
   }

   // normalize
   for (int f = 0; f < nf; f++) {
      float sum = 0;
      for (int i = 0; i < nx*ny; i++) sum += w[f + i*nf];

      if (sum == 0) continue;

      float factor = 1.0/sum;
      for (int i = 0; i < nx*ny; i++) w[f + i*nf] *= factor;
   }

   return 0;
}

} // namespace PV
