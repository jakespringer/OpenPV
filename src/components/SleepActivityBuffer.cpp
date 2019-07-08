/*
 * SleepActivityBuffer.cpp
 *
 * Created on: Jul 1, 2019
 *     Author: jspringer
 */

#include "SleepActivityBuffer.hpp"
#include <cmath>

namespace PV {

SleepActivityBuffer::SleepActivityBuffer(char const *name, PVParams *params, Communicator const *comm) {
  HyPerActivityBuffer::initialize(name, params, comm);
}

SleepActivityBuffer::~SleepActivityBuffer() {
}

void SleepActivityBuffer::ioParam_sleepOffset(enum ParamsIOFlag ioFlag) {
  parameters()->ioParamValue(
          ioFlag, getName(), "sleepOffset", &mSleepOffset, mSleepOffset, true);
}

void SleepActivityBuffer::ioParam_sleepDuration(enum ParamsIOFlag ioFlag) {
  parameters()->ioParamValue(
          ioFlag, getName(), "sleepDuration", &mSleepDuration, mSleepDuration, true);
}

void SleepActivityBuffer::ioParam_sleepCycleLen(enum ParamsIOFlag ioFlag) {
  parameters()->ioParamValue(
          ioFlag, getName(), "sleepCycleLen", &mSleepCycleLen, mSleepCycleLen, true);
}

void SleepActivityBuffer::ioParam_sleepValueA(enum ParamsIOFlag ioFlag) {
  parameters()->ioParamValue(
          ioFlag, getName(), "sleepValueA", &mSleepValueA, mSleepValueA, true);
}

int SleepActivityBuffer::ioParamsFillGroup(enum ParamsIOFlag ioFlag) {
  int status = HyPerActivityBuffer::ioParamsFillGroup(ioFlag);

  ioParam_sleepOffset(ioFlag);
  ioParam_sleepDuration(ioFlag);
  ioParam_sleepCycleLen(ioFlag);
  ioParam_sleepValueA(ioFlag);

  return status;
}

void SleepActivityBuffer::updateBufferCPU(double simTime, double deltaTime) {
   float *A           = mBufferData.data();
   float const *V     = mInternalState->getBufferData();
   int const nbatch   = getLayerLoc()->nbatch;
   int const nx       = getLayerLoc()->nx;
   int const ny       = getLayerLoc()->ny;
   int const nf       = getLayerLoc()->nf;
   PVHalo const *halo = &getLayerLoc()->halo;

   int const numNeuronsAcrossBatch = mInternalState->getBufferSizeAcrossBatch();
   pvAssert(V != nullptr);
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (int k = 0; k < numNeuronsAcrossBatch; k++) {
      int kExt = kIndexExtendedBatch(k, nbatch, nx, ny, nf, halo->lt, halo->rt, halo->dn, halo->up);
      if (fmod(simTime + mSleepOffset, mSleepCycleLen) < mSleepDuration - deltaTime / 2.0) {
         A[kExt]  = V[k];
      } else {
         A[kExt]  = 0.0f;    
      }
   }
}

}
