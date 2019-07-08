/*
 * L0NormAdaptiveLCAProbe.hpp
 *
 *  Created on: Jun 28, 2019
 *      Author: jspringer
 */

#ifndef L0NORMADAPTIVELCAPROBE_HPP_
#define L0NORMADAPTIVELCAPROBE_HPP_

#include "AbstractNormProbe.hpp"
#include "components/AdaptiveThresholdActivityBuffer.hpp"

namespace PV {

/**
 * A layer probe for returning the number of elements in its target layer's
 * activity buffer
 * above a certain threshold (often referred to as the L0-norm).
 */
class L0NormAdaptiveLCAProbe : public AbstractNormProbe {
  public:
   L0NormAdaptiveLCAProbe(const char *name, PVParams *params, Communicator const *comm);
   virtual ~L0NormAdaptiveLCAProbe();

   virtual Response::Status
   communicateInitInfo(std::shared_ptr<CommunicateInitInfoMessage const> message) override;

  protected:
   L0NormAdaptiveLCAProbe();
   void initialize(const char *name, PVParams *params, Communicator const *comm);
   virtual double getValueInternal(double timevalue, int index) override;

   virtual int ioParamsFillGroup(enum ParamsIOFlag ioFlag) override;
   /**
    * List of parameters for the L0NormProbe class
    * @name L0NormProbe Parameters
    * @{
    */

   /**
    * @brief nnzThreshold: The threshold for computing the L0-norm.
    * getValue(t, index) returns the number of targetLayer neurons whose
    * absolute value is greater than nnzThreshold.
    */
   virtual void ioParam_nnzThreshold(enum ParamsIOFlag ioFlag);
   /** @} */

   /**
    * Overrides AbstractNormProbe::setNormDescription() to set normDescription to
    * "L0-norm".
    * Return values and errno are set by a call to setNormDescriptionToString.
    */
   virtual int setNormDescription() override;

  private:
   int initialize_base() { return PV_SUCCESS; }

  protected:
   float nnzThreshold;
   AdaptiveThresholdActivityBuffer* activityBuffer = nullptr;
}; // end class L0NormProbe

} // end namespace PV

#endif /* L0NORMPROBE_HPP_ */
