/*
 * L0NormAdaptiveLCAProbe.cpp
 *
 *  Created on: Jun 28, 2019
 *      Author: jspringer
 */

#include "L0NormAdaptiveLCAProbe.hpp"
#include "layers/HyPerLayer.hpp"
#include "components/ANNActivityBuffer.hpp"
#include "components/AdaptiveThresholdActivityBuffer.hpp"
#include "layers/AdaptiveThresholdLCALayer.hpp"

namespace PV {

L0NormAdaptiveLCAProbe::L0NormAdaptiveLCAProbe() : AbstractNormProbe() { initialize_base(); }

L0NormAdaptiveLCAProbe::L0NormAdaptiveLCAProbe(const char *name, PVParams *params, Communicator const *comm)
      : AbstractNormProbe() {
   initialize_base();
   initialize(name, params, comm);
}

L0NormAdaptiveLCAProbe::~L0NormAdaptiveLCAProbe() {}

Response::Status
L0NormAdaptiveLCAProbe::communicateInitInfo(std::shared_ptr<CommunicateInitInfoMessage const> message) {
    auto status = AbstractNormProbe::communicateInitInfo(message);
    if (!Response::completed(status)) {
        return status;
    }
    assert(targetLayer);
    auto* activityComponent = targetLayer->getComponentByType<ActivityComponent>();
    FatalIf(
            activityComponent == nullptr,
            "%s: targetLayer \"%s\" does not have an activity component.\n",
            getDescription_c(),
            getTargetName());
    activityBuffer = activityComponent->getComponentByType<AdaptiveThresholdActivityBuffer>();
    FatalIf(
            activityBuffer == nullptr,
            "%s: targetLayer \"%s\" does not have an AdaptiveThresholdActivityBuffer component.\n",
            getDescription_c(),
            getTargetName());
}

void L0NormAdaptiveLCAProbe::initialize(const char *name, PVParams *params, Communicator const *comm) {
   AbstractNormProbe::initialize(name, params, comm);
}

int L0NormAdaptiveLCAProbe::ioParamsFillGroup(enum ParamsIOFlag ioFlag) {
   int status = AbstractNormProbe::ioParamsFillGroup(ioFlag);
   ioParam_nnzThreshold(ioFlag);
   return status;
}

void L0NormAdaptiveLCAProbe::ioParam_nnzThreshold(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(ioFlag, getName(), "nnzThreshold", &nnzThreshold, (float)0);
}

double L0NormAdaptiveLCAProbe::getValueInternal(double timevalue, int index) {
   PVLayerLoc const *loc = getTargetLayer()->getLayerLoc();
   if (index < 0 || index >= loc->nbatch) {
      return PV_FAILURE;
   }
   int const nx             = loc->nx;
   int const ny             = loc->ny;
   int const nf             = loc->nf;
   PVHalo const *halo       = &loc->halo;
   int const lt             = halo->lt;
   int const rt             = halo->rt;
   int const dn             = halo->dn;
   int const up             = halo->up;
   double sum                  = 0;
   auto *publisherComponent = getTargetLayer()->getComponentByType<BasePublisherComponent>();
   int const numExtended    = (nx + lt + rt) * (ny + dn + up) * nf;
   float const *aBuffer     = publisherComponent->getLayerData() + index * numExtended;
   float const* thresholdsPtr = activityBuffer->getThresholds(); 

   if (getMaskLayerData()) {
      PVLayerLoc const *maskLoc  = getMaskLayerData()->getLayerLoc();
      int const maskLt           = maskLoc->halo.lt;
      int const maskRt           = maskLoc->halo.rt;
      int const maskDn           = maskLoc->halo.dn;
      int const maskUp           = maskLoc->halo.up;
      int const maskNumExtended  = getMaskLayerData()->getNumExtended();
      float const *maskLayerData = getMaskLayerData()->getLayerData() + index * maskNumExtended;
      if (maskHasSingleFeature()) {
         assert(getTargetLayer()->getNumNeurons() == nx * ny * nf);
         int nxy = nx * ny;
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for reduction(+ : sum)
#endif // PV_USE_OPENMP_THREADS
         for (int kxy = 0; kxy < nxy; kxy++) {
            int kexMask = kIndexExtended(kxy, nx, ny, 1, maskLt, maskRt, maskDn, maskUp);
            if (maskLayerData[kexMask]) {
               int featureBase = kxy * nf;
               for (int f = 0; f < nf; f++) {
                  int kex   = kIndexExtended(featureBase++, nx, ny, nf, lt, rt, dn, up);
                  float val = fabsf(aBuffer[kex]);
                  sum += (val > nnzThreshold) * thresholdsPtr[kxy] * thresholdsPtr[kxy] * 0.5;
               }
            }
         }
      }
      else {
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for reduction(+ : sum)
#endif // PV_USE_OPENMP_THREADS
         for (int k = 0; k < getTargetLayer()->getNumNeurons(); k++) {
            int kex     = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
            int kexMask = kIndexExtended(k, nx, ny, nf, maskLt, maskRt, maskDn, maskUp);
            if (maskLayerData[kexMask]) {
               float val = fabsf(aBuffer[kex]);
               sum += (val > nnzThreshold) * thresholdsPtr[k] * thresholdsPtr[k] * 0.5;
            }
         }
      }
   }
   else {
      if (publisherComponent->getSparseLayer()) {
         PVLayerCube cube   = publisherComponent->getPublisher()->createCube();
         long int numActive = cube.numActive[index];
         int numItems       = cube.numItems / cube.loc.nbatch;
         SparseList<float>::Entry const *activeList =
               &((SparseList<float>::Entry *)cube.activeIndices)[index * numItems];
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for reduction(+ : sum)
#endif // PV_USE_OPENMP_THREADS
         for (int k = 0; k < numActive; k++) {
            int extIndex     = activeList[k].index;
            int resIndex     = kIndexRestricted(extIndex, nx, ny, nf, halo->lt, halo->rt, halo->dn, halo->up);
            int inRestricted = !extendedIndexInBorderRegion(
                  extIndex, nx, ny, nf, halo->lt, halo->rt, halo->dn, halo->up);
            float val = inRestricted * fabsf(aBuffer[extIndex]);
            sum += (val > nnzThreshold) * thresholdsPtr[resIndex] * thresholdsPtr[resIndex] * 0.5;
         }
      }
      else {
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for reduction(+ : sum)
#endif // PV_USE_OPENMP_THREADS
         for (int k = 0; k < getTargetLayer()->getNumNeurons(); k++) {
            int kex   = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
            float val = fabsf(aBuffer[kex]);
            sum += (val > nnzThreshold) * thresholdsPtr[k] * thresholdsPtr[k] * 0.5;
         }
      }
   }

   return (double)sum;
}

int L0NormAdaptiveLCAProbe::setNormDescription() { return setNormDescriptionToString("L0-norm"); }

} // end namespace PV
