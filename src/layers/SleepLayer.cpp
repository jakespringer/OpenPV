/*
 * SleepLayer.cpp
 *
 *  Created on: Jul 1, 2019
 *      Author: jspringer
 */

#include "SleepLayer.hpp"
#include "components/SleepActivityBuffer.hpp"
#include "components/GSynAccumulator.hpp"
#include "components/HyPerActivityComponent.hpp"
#include "components/HyPerInternalStateBuffer.hpp"

namespace PV {

SleepLayer::SleepLayer(const char *name, PVParams *params, Communicator const *comm) {
   initialize(name, params, comm);
}

SleepLayer::~SleepLayer() {}

void SleepLayer::initialize(const char *name, PVParams *params, Communicator const *comm) {
   HyPerLayer::initialize(name, params, comm);
}

ActivityComponent *SleepLayer::createActivityComponent() {
   return new HyPerActivityComponent<GSynAccumulator, HyPerInternalStateBuffer, SleepActivityBuffer>(
         getName(), parameters(), mCommunicator);
}

} // end namespace PV
