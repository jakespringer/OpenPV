#include "JICTimeScaleController.hpp"

namespace PV {

JICTimeScaleController::JICTimeScaleController(
      char const *name,
      int batchWidth,
      double baseMax,
      double baseMin,
      double tauFactor,
      double growthFactor,
      bool writeTimeScaleFieldnames,
      Communicator const *comm,
      double kneeThresh,
      double kneeSlope,
      double resetThresh)
      : AdaptiveTimeScaleController(
              name,
              batchWidth,
              baseMax,
              baseMin,
              tauFactor,
              growthFactor,
              writeTimeScaleFieldnames,
              comm) {
   mKneeThresh = kneeThresh;
   mKneeSlope  = kneeSlope;
   mResetThresh = resetThresh;
}

std::vector<double>
JICTimeScaleController::calcTimesteps(double timeValue, std::vector<double> const &rawTimeScales) {

   mOldTimeScaleInfo             = mTimeScaleInfo;
   mTimeScaleInfo.mTimeScaleTrue = rawTimeScales;
   for (int b = 0; b < mBatchWidth; b++) {
      double E_dt         = mTimeScaleInfo.mTimeScaleTrue[b];
      double E_0          = mOldTimeScaleInfo.mTimeScaleTrue[b];
      double dE_dt_scaled = (E_0 - E_dt) / mTimeScaleInfo.mTimeScale[b];

      if (E_dt == E_0) {
         continue;
      }

      if ((dE_dt_scaled < mResetThresh) || (E_0 <= 0) || (E_dt <= 0)) {
         mTimeScaleInfo.mTimeScale[b]    = mBaseMin;
         mTimeScaleInfo.mTimeScaleMax[b] = mBaseMax;
      }
      else {
         double tau_eff_scaled = E_0 / dE_dt_scaled;

         // dt := mTimeScaleMaxBase * tau_eff
         mTimeScaleInfo.mTimeScale[b] = mTauFactor * tau_eff_scaled;
         if (mTimeScaleInfo.mTimeScale[b] >= mTimeScaleInfo.mTimeScaleMax[b]) {
            mTimeScaleInfo.mTimeScale[b]    = mTimeScaleInfo.mTimeScaleMax[b];
            mTimeScaleInfo.mTimeScaleMax[b] = (1 + mGrowthFactor) * mTimeScaleInfo.mTimeScaleMax[b];
         }
      }
   }

   std::vector<double> timeScales(mTimeScaleInfo.mTimeScale);

   for (std::size_t i = 0; i < timeScales.size(); ++i) {
      // Scale timescalemax if it's above the knee
      if (mTimeScaleInfo.mTimeScaleMax[i] > mKneeThresh) {
         mTimeScaleInfo.mTimeScaleMax[i] =
               mOldTimeScaleInfo.mTimeScaleMax[i]
               + (mTimeScaleInfo.mTimeScaleMax[i] - mOldTimeScaleInfo.mTimeScaleMax[i])
                       * mKneeSlope;
         // Cap our timescale to the newly calculated max
         timeScales[i] = std::min(timeScales[i], mTimeScaleInfo.mTimeScaleMax[i]);
      }
   }
   return timeScales;
}
}
