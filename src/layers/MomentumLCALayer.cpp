/*
 * MomentumLCALayer.cpp
 *
 *  Created on: Mar 15, 2016
 *      Author: slundquist
 */

#include "MomentumLCALayer.hpp"
#include <iostream>

#ifdef PV_USE_CUDA

#include "../cudakernels/CudaUpdateStateFunctions.hpp"

#endif

void MomentumLCALayer_update_state(
      const int nbatch,
      const int numNeurons,
      const int nx,
      const int ny,
      const int nf,
      const int lt,
      const int rt,
      const int dn,
      const int up,
      const int numChannels,
      float *V,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      const bool selfInteract,
      double *dtAdapt,
      const float tau,
      const float LCAMomentumRate,
      float *GSynHead,
      float *activity,
      float *prevDrive);

namespace PV {

MomentumLCALayer::MomentumLCALayer() { initialize_base(); }

MomentumLCALayer::MomentumLCALayer(const char *name, HyPerCol *hc) {
   initialize_base();
   initialize(name, hc);
}

MomentumLCALayer::~MomentumLCALayer() {}

int MomentumLCALayer::initialize_base() {
   timeConstantTau = 1.0;
   LCAMomentumRate = 0;
   // Locality in conn
   selfInteract = true;
   return PV_SUCCESS;
}

int MomentumLCALayer::initialize(const char *name, HyPerCol *hc) {
   HyPerLCALayer::initialize(name, hc);

   return PV_SUCCESS;
}

Response::Status MomentumLCALayer::allocateDataStructures() {
   auto status = HyPerLCALayer::allocateDataStructures();
   allocateRestrictedBuffer(&prevDrive, "prevDrive of LCA layer");

   // Initialize buffer to 0
   for (int i = 0; i < getNumNeuronsAllBatches(); i++) {
      prevDrive[i] = 0;
   }

#ifdef PV_USE_CUDA
   if (mUpdateGpu) {
      d_prevDrive->copyToDevice(prevDrive);
   }
#endif

   return status;
}

int MomentumLCALayer::ioParamsFillGroup(enum ParamsIOFlag ioFlag) {
   int status = HyPerLCALayer::ioParamsFillGroup(ioFlag);
   ioParam_LCAMomentumRate(ioFlag);

   return status;
}

void MomentumLCALayer::ioParam_LCAMomentumRate(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(
         ioFlag, name, "LCAMomentumRate", &LCAMomentumRate, LCAMomentumRate, true /*warnIfAbsent*/);
}

#ifdef PV_USE_CUDA
int MomentumLCALayer::allocateUpdateKernel() {
   PVCuda::CudaDevice *device = mCudaDevice;
   d_prevDrive = device->createBuffer(getNumNeuronsAllBatches() * sizeof(float), &getDescription());
   // Set to temp pointer of the subclass
   PVCuda::CudaUpdateMomentumLCALayer *updateKernel =
         new PVCuda::CudaUpdateMomentumLCALayer(device);
   // Set arguments
   const PVLayerLoc *loc = getLayerLoc();
   const int nx          = loc->nx;
   const int ny          = loc->ny;
   const int nf          = loc->nf;
   const int num_neurons = nx * ny * nf;
   const int nbatch      = loc->nbatch;
   const int lt          = loc->halo.lt;
   const int rt          = loc->halo.rt;
   const int dn          = loc->halo.dn;
   const int up          = loc->halo.up;
   const int numChannels = mLayerInput->getNumChannels();
   pvAssert(mInternalState);
   PVCuda::CudaBuffer *cudaBuffer = mInternalState->getCudaBuffer();
   pvAssert(cudaBuffer);
   assert(d_prevDrive);
   const float Vth         = this->VThresh;
   const float AMax        = this->AMax;
   const float AMin        = this->AMin;
   const float AShift      = this->AShift;
   const float VWidth      = this->VWidth;
   const bool selfInteract = this->selfInteract;
   const float tau         = timeConstantTau
                     / (float)parent->getDeltaTime(); // TODO: eliminate need to call parent method
   PVCuda::CudaBuffer *d_GSyn     = getDeviceGSyn();
   PVCuda::CudaBuffer *d_activity = getDeviceActivity();

   size_t size = loc->nbatch * sizeof(double);
   d_dtAdapt   = device->createBuffer(size, &getDescription());

   size        = (size_t)numVertices * sizeof(*verticesV);
   d_verticesV = device->createBuffer(size, &getDescription());
   d_verticesA = device->createBuffer(size, &getDescription());
   d_slopes    = device->createBuffer(size + sizeof(*slopes), &getDescription());

   d_verticesV->copyToDevice(verticesV);
   d_verticesA->copyToDevice(verticesA);
   d_slopes->copyToDevice(slopes);

   // Set arguments to kernel
   updateKernel->setArgs(
         nbatch,
         num_neurons,
         nx,
         ny,
         nf,
         lt,
         rt,
         dn,
         up,
         numChannels,
         cudaBuffer,
         d_prevDrive,
         numVertices,
         d_verticesV,
         d_verticesA,
         d_slopes,
         selfInteract,
         d_dtAdapt,
         tau,
         LCAMomentumRate,
         d_GSyn,
         d_activity);

   // set updateKernel to krUpdate
   krUpdate = updateKernel;
   return PV_SUCCESS;
}

Response::Status MomentumLCALayer::updateStateGpu(double time, double dt) {
   if (triggerLayer != NULL) {
      Fatal().printf("HyPerLayer::Trigger reset of V does not work on GPUs\n");
   }
   // Copy over d_dtAdapt
   d_dtAdapt->copyToDevice(deltaTimes());

   // Change dt to match what is passed in
   PVCuda::CudaUpdateMomentumLCALayer *updateKernel =
         dynamic_cast<PVCuda::CudaUpdateMomentumLCALayer *>(krUpdate);
   assert(updateKernel);
   runUpdateKernel();

   return Response::SUCCESS;
}
#endif

Response::Status MomentumLCALayer::updateState(double time, double dt) {
   const PVLayerLoc *loc = getLayerLoc();
   float *A              = mActivity->getActivity();
   float *V              = getV();
   int num_channels      = mLayerInput->getNumChannels();
   float *gSynHead       = mLayerInput->getLayerInput();
   int nx                = loc->nx;
   int ny                = loc->ny;
   int nf                = loc->nf;
   int num_neurons       = nx * ny * nf;
   int nbatch            = loc->nbatch;
   // Only update when the probe updates

   MomentumLCALayer_update_state(
         nbatch,
         num_neurons,
         nx,
         ny,
         nf,
         loc->halo.lt,
         loc->halo.rt,
         loc->halo.dn,
         loc->halo.up,
         num_channels,
         V,
         numVertices,
         verticesV,
         verticesA,
         slopes,
         selfInteract,
         deltaTimes(),
         timeConstantTau / (float)dt,
         LCAMomentumRate,
         gSynHead,
         A,
         prevDrive);
   return Response::SUCCESS;
}

Response::Status
MomentumLCALayer::registerData(std::shared_ptr<RegisterDataMessage<Checkpointer> const> message) {
   auto status = HyPerLCALayer::registerData(message);
   if (!Response::completed(status)) {
      return status;
   }
   auto *checkpointer = message->mDataRegistry;
   checkpointPvpActivityFloat(checkpointer, "prevDrive", prevDrive, false /*not extended*/);
   return Response::SUCCESS;
}

Response::Status MomentumLCALayer::processCheckpointRead() {
   auto status = HyPerLCALayer::processCheckpointRead();
   if (!Response::completed(status)) {
      return status;
   }
#ifdef PV_USE_CUDA
   // Copy prevDrive onto GPU
   if (mUpdateGpu) {
      d_prevDrive->copyToDevice(prevDrive);
      mCudaDevice->syncDevice();
      return Response::SUCCESS;
   }
   else {
      return Response::NO_ACTION;
   }
#else
   return Response::NO_ACTION;
#endif
}

Response::Status MomentumLCALayer::prepareCheckpointWrite() {
#ifdef PV_USE_CUDA
   // Copy prevDrive from GPU
   if (mUpdateGpu) {
      d_prevDrive->copyFromDevice(prevDrive);
      mCudaDevice->syncDevice();
      return Response::SUCCESS;
   }
   else {
      return Response::NO_ACTION;
   }
#else
   return Response::NO_ACTION;
#endif
}

} // end namespace PV

void MomentumLCALayer_update_state(
      const int nbatch,
      const int numNeurons,
      const int nx,
      const int ny,
      const int nf,
      const int lt,
      const int rt,
      const int dn,
      const int up,
      const int numChannels,

      float *V,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      const bool selfInteract,
      double *dtAdapt,
      const float tau,
      const float LCAMomentumRate,
      float *GSynHead,
      float *activity,
      float *prevDrive) {
   updateV_MomentumLCALayer(
         nbatch,
         numNeurons,
         numChannels,
         V,
         GSynHead,
         activity,
         prevDrive,
         numVertices,
         verticesV,
         verticesA,
         slopes,
         dtAdapt,
         tau,
         LCAMomentumRate,
         selfInteract,
         nx,
         ny,
         nf,
         lt,
         rt,
         dn,
         up);
}
