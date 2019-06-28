#include "JICTimeScaleProbe.hpp"
#include "components/JICTimeScaleController.hpp"

namespace PV {

JICTimeScaleProbe::JICTimeScaleProbe(
      char const *name,
      PVParams *params,
      Communicator const *comm) {
   initialize(name, params, comm);
}

int JICTimeScaleProbe::ioParamsFillGroup(enum ParamsIOFlag ioFlag) {
   int status = AdaptiveTimeScaleProbe::ioParamsFillGroup(ioFlag);
   ioParam_kneeThresh(ioFlag);
   ioParam_kneeSlope(ioFlag);
   ioParam_resetThresh(ioFlag);
   return status;
}

void JICTimeScaleProbe::ioParam_kneeThresh(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(ioFlag, name, "kneeThresh", &mKneeThresh, mKneeThresh);
}

void JICTimeScaleProbe::ioParam_kneeSlope(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(ioFlag, name, "kneeSlope", &mKneeSlope, mKneeSlope);
}

void JICTimeScaleProbe::ioParam_resetThresh(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(ioFlag, name, "resetThresh", &mResetThresh, mResetThresh);
}

void JICTimeScaleProbe::allocateTimeScaleController() {
   mAdaptiveTimeScaleController = new JICTimeScaleController(
         getName(),
         getNumValues(),
         mBaseMax,
         mBaseMin,
         tauFactor,
         mGrowthFactor,
         mWriteTimeScaleFieldnames,
         mCommunicator,
         mKneeThresh,
         mKneeSlope,
         mResetThresh);
}
}

