/*
 * HyPerLCALayer.cpp
 *
 *  Created on: Jan 24, 2013
 *      Author: garkenyon
 */

#include "AdaptiveThresholdLCALayer.hpp"
#include "components/AdaptiveThresholdActivityBuffer.hpp"
#include "components/GSynAccumulator.hpp"
#include "components/HyPerActivityComponent.hpp"
#include "components/HyPerLCAInternalStateBuffer.hpp"
#include "components/LayerInputBuffer.hpp"

namespace PV {

AdaptiveThresholdLCALayer::AdaptiveThresholdLCALayer(const char *name, PVParams *params, Communicator const *comm) {
   initialize(name, params, comm);
}

AdaptiveThresholdLCALayer::~AdaptiveThresholdLCALayer() {}

void AdaptiveThresholdLCALayer::initialize(const char *name, PVParams *params, Communicator const *comm) {
   HyPerLayer::initialize(name, params, comm);
}

LayerInputBuffer *AdaptiveThresholdLCALayer::createLayerInput() {
   return new LayerInputBuffer(name, parameters(), mCommunicator);
}

ActivityComponent *AdaptiveThresholdLCALayer::createActivityComponent() {
   return new HyPerActivityComponent<GSynAccumulator,
                                     HyPerLCAInternalStateBuffer,
                                     AdaptiveThresholdActivityBuffer>(getName(), parameters(), mCommunicator);
}

} // end namespace PV
