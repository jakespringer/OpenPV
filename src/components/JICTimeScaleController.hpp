#ifndef _JICTIMESCALECONTROLLER_HPP_
#define _JICTIMESCALECONTROLLER_HPP_

#include "AdaptiveTimeScaleController.hpp"

namespace PV {

// Just-In-Case TimeScaleController
// The intended behavior of this controller is to operate as if it doesn't exist
// in the vast majority of cases, except when the dyanmical system appears about
// to explode, at which point the timescale controller kicks in.
class JICTimeScaleController : public AdaptiveTimeScaleController {
  public:
   JICTimeScaleController(
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
         double resetThresh);

   virtual std::vector<double>
   calcTimesteps(double timeValue, std::vector<double> const &rawTimeScales) override;

  protected:
   double mKneeThresh = 1.0;
   double mKneeSlope  = 1.0;
   double mResetThresh = 0.0;
};
}

#endif
