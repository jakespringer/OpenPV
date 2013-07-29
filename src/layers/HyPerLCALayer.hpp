/*
 * HyPerLCALayer.hpp
 *
 *  Created on: Jan 24, 2013
 *      Author: garkenyon
 */

#ifndef HYPERLCALAYER_HPP_
#define HYPERLCALAYER_HPP_

#include "ANNLayer.hpp"

namespace PV {

class HyPerLCALayer: public PV::ANNLayer {
public:
   HyPerLCALayer(const char * name, HyPerCol * hc, int numChannels);
   HyPerLCALayer(const char * name, HyPerCol * hc);
   virtual ~HyPerLCALayer();

   //Overwriting HyPerLayer's window methods
   virtual int getNumWindows();
   virtual bool inWindowExt(int windowId, int neuronIdxExt);
   virtual bool inWindowRes(int windowId, int neuronIdxRes);
   //calcWindow used in WindowSystemTest, so needs to be public
   int calcWindow(int globalExtX, int globalExtY);
protected:
   HyPerLCALayer();
   int initialize(const char * name, HyPerCol * hc, int numChannels);
   virtual int doUpdateState(double time, double dt, const PVLayerLoc * loc, pvdata_t * A,
         pvdata_t * V, int num_channels, pvdata_t * gSynHead, bool spiking,
         unsigned int * active_indices, unsigned int * num_active);
   virtual float getChannelTimeConst(enum ChannelType channel_type){return tauMax;};
private:
   pvdata_t tauMax;
   pvdata_t tauMin;
   pvdata_t dtTau;
   float slopeErrorStd;
   double errorStd;
   int initialize_base();
   int numWindowX;
   int numWindowY;
   bool windowSymX;
   bool windowSymY;
};

} /* namespace PV */
#endif /* HYPERLCALAYER_HPP_ */
