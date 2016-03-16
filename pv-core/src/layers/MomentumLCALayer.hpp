/*
 * MomentumLCALayer.hpp
 *
 *  Created on: Mar 15, 2016
 *      Author: slundquist
 */

#ifndef MOMENTUMLCALAYER_HPP_
#define MOMENTUMLCALAYER_HPP_

#include "HyPerLCALayer.hpp"

namespace PV {

class MomentumLCALayer: public PV::HyPerLCALayer{
public:
   MomentumLCALayer(const char * name, HyPerCol * hc);
   virtual ~MomentumLCALayer();
   virtual int checkpointWrite(const char * cpDir);
   virtual int checkpointRead(const char * cpDir, double * timeptr);

protected:
   MomentumLCALayer();
   int initialize(const char * name, HyPerCol * hc);
   virtual int allocateDataStructures();
   virtual int ioParamsFillGroup(enum ParamsIOFlag ioFlag);

   /**
    * List of parameters needed from the MomentumLCALayer class
    * @name HyPerConn Parameters
    * @{
    */

   virtual void ioParam_LCAMomentumRate(enum ParamsIOFlag ioFlag);

   /** @} */

   virtual int doUpdateState(double time, double dt, const PVLayerLoc * loc, pvdata_t * A,
         pvdata_t * V, int num_channels, pvdata_t * gSynHead);

#ifdef PV_USE_CUDA
   virtual int doUpdateStateGpu(double time, double dt, const PVLayerLoc * loc, pvdata_t * A,
      pvdata_t * V, int num_channels, pvdata_t * gSynHead);
#endif

#if defined(PV_USE_OPENCL) || defined(PV_USE_CUDA)
   virtual int allocateUpdateKernel();
#endif


   float LCAMomentumRate;
   pvdata_t * prevDrive;
   PVCuda::CudaBuffer* d_prevDrive;

private:
   int initialize_base();
};

} /* namespace PV */
#endif /* HYPERLCALAYER_HPP_ */
