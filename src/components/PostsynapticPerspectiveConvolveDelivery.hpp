/*
 * PostsynapticPerspectiveConvolveDelivery.hpp
 *
 *  Created on: Aug 24, 2017
 *      Author: Pete Schultz
 */

#ifndef POSTSYNAPTICPERSPECTIVECONVOLVEDELIVERY_HPP_
#define POSTSYNAPTICPERSPECTIVECONVOLVEDELIVERY_HPP_

#include "components/HyPerDelivery.hpp"

namespace PV {

/**
 * The delivery class for HyPerConns using the postsynaptic perspective on the CPU,
 * with accumulate type "convolve".
 */
class PostsynapticPerspectiveConvolveDelivery : public HyPerDelivery {
  protected:
   /**
    * List of parameters needed from the PostsynapticPerspectiveConvolveDelivery class
    * @name PostsynapticPerspectiveConvolveDelivery Parameters
    * @{
    */

   /**
    * @brief receiveGpu: PostsynapticPerspectiveConvolveDelivery always sets receiveGpu to false.
    * The receiveGpu=true cases is handled by the PostsynapticPerspectiveGPU class.
    */
   virtual void ioParam_receiveGpu(enum ParamsIOFlag ioFlag);
   /** @} */ // End of list of BaseDelivery parameters.

  public:
   PostsynapticPerspectiveConvolveDelivery(char const *name, HyPerCol *hc);

   virtual ~PostsynapticPerspectiveConvolveDelivery();

   /**
    * The method that delivers presynaptic activity to the given postsynaptic channel.
    * It loops over presynaptic neurons, skipping over any whose activity is zero
    * (to take advantage of sparsity). Each neuron then modifies the region of the post channel
    * that the weights argument specifies for that pre-synaptic neuron.
    *
    * If OpenMP is used, we parallelize over the presynaptic neuron. To avoid the
    * possibility of collisions where more than one pre-neuron writes to the
    * same post-neuron, we internally allocate multiple buffers the size of the post channel,
    * and accumulate them at the end.
    *
    * The postWeights argument is not used.
    */
   virtual void deliver() override;

   virtual void deliverUnitInput(float *recvBuffer) override;

   /**
    * Returns the value of the receiveGpu parameter
    */
   bool getReceiveGpu() const { return mReceiveGpu; }

  protected:
   PostsynapticPerspectiveConvolveDelivery();

   int initialize(char const *name, HyPerCol *hc);

   virtual int ioParamsFillGroup(enum ParamsIOFlag ioFlag) override;

   virtual int allocateDataStructures() override;

   void allocateThreadGSyn();

   // Data members
  protected:
   std::vector<std::vector<float>> mThreadGSyn;
}; // end class PostsynapticPerspectiveConvolveDelivery

} // end namespace PV

#endif // POSTSYNAPTICPERSPECTIVECONVOLVEDELIVERY_HPP_
