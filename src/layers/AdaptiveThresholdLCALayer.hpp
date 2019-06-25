/*
 * HyPerLCALayer.hpp
 *
 *  Created on: Jan 24, 2013
 *      Author: garkenyon
 */

#ifndef ADAPTIVETHRESHOLDLCALAYER_HPP__
#define ADAPTIVETHRESHOLDLCALAYER_HPP__

#include "HyPerLayer.hpp"

namespace PV {

class AdaptiveThresholdLCALayer : public HyPerLayer {
  public:
   AdaptiveThresholdLCALayer(const char *name, PVParams *params, Communicator const *comm);
   virtual ~AdaptiveThresholdLCALayer();

  protected:
   AdaptiveThresholdLCALayer() {}

   void initialize(const char *name, PVParams *params, Communicator const *comm);

   virtual LayerInputBuffer *createLayerInput() override;

   virtual ActivityComponent *createActivityComponent() override;
};

} // end namespace PV

#endif
