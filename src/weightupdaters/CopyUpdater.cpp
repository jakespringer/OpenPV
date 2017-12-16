/*
 * CopyUpdater.cpp
 *
 *  Created on: Dec 15, 2017
 *      Author: Pete Schultz
 */

#include "CopyUpdater.hpp"
#include "columns/HyPerCol.hpp"
#include "columns/ObjectMapComponent.hpp"
#include "connections/HyPerConn.hpp"
#include "utils/MapLookupByType.hpp"
#include "utils/TransposeWeights.hpp"

namespace PV {

CopyUpdater::CopyUpdater(char const *name, HyPerCol *hc) { initialize(name, hc); }

int CopyUpdater::initialize(char const *name, HyPerCol *hc) {
   return BaseWeightUpdater::initialize(name, hc);
}

void CopyUpdater::ioParam_plasticityFlag(enum ParamsIOFlag ioFlag) {
   // During the CommunicateInitInfo stage, plasticityFlag will be copied from
   // the original connection's updater.
}

int CopyUpdater::communicateInitInfo(std::shared_ptr<CommunicateInitInfoMessage const> message) {
   int status        = PV_SUCCESS;
   auto componentMap = message->mHierarchy;
   mCopyWeightsPair  = mapLookupByType<CopyWeightsPair>(componentMap, getDescription());
   pvAssert(mCopyWeightsPair);
   if (!mCopyWeightsPair->getInitInfoCommunicatedFlag()) {
      return PV_POSTPONE;
   }
   mCopyWeightsPair->needPre();

   char const *originalConnName = mCopyWeightsPair->getOriginalConnName();
   pvAssert(originalConnName != nullptr and originalConnName[0] != '\0');

   auto hierarchy           = message->mHierarchy;
   auto *objectMapComponent = mapLookupByType<ObjectMapComponent>(hierarchy, getDescription());
   HyPerConn *originalConn  = objectMapComponent->lookup<HyPerConn>(std::string(originalConnName));
   pvAssert(originalConn);
   auto *originalWeightUpdater = originalConn->getComponentByType<BaseWeightUpdater>();
   if (originalWeightUpdater and !originalWeightUpdater->getInitInfoCommunicatedFlag()) {
      return PV_POSTPONE;
   }
   mPlasticityFlag = originalWeightUpdater ? originalWeightUpdater->getPlasticityFlag() : false;

   auto *originalWeightsPair = originalConn->getComponentByType<WeightsPair>();
   pvAssert(originalWeightsPair);
   if (!originalWeightsPair->getInitInfoCommunicatedFlag()) {
      return PV_POSTPONE;
   }
   originalWeightsPair->needPre();
   mOriginalWeights = originalWeightsPair->getPreWeights();
   pvAssert(mOriginalWeights);

   status = BaseWeightUpdater::communicateInitInfo(message);
   if (status != PV_SUCCESS) {
      return status;
   }

   if (mPlasticityFlag) {
      mCopyWeightsPair->getPreWeights()->setWeightsArePlastic();
   }
   mWriteCompressedCheckpoints = mCopyWeightsPair->getWriteCompressedCheckpoints();

   return status;
}

int CopyUpdater::registerData(Checkpointer *checkpointer) {
   int status = BaseWeightUpdater::registerData(checkpointer);
   if (status != PV_SUCCESS) {
      return status;
   }
   std::string nameString = std::string(name);
   checkpointer->registerCheckpointData(
         nameString,
         "lastUpdateTime",
         &mLastUpdateTime,
         (std::size_t)1,
         true /*broadcast*/,
         false /*not constant*/);
   return status;
}

void CopyUpdater::updateState(double simTime, double dt) {
   pvAssert(mCopyWeightsPair and mCopyWeightsPair->getPreWeights());
   if (mOriginalWeights->getTimestamp() > mCopyWeightsPair->getPreWeights()->getTimestamp()) {
      mCopyWeightsPair->copy();
      mCopyWeightsPair->getPreWeights()->setTimestamp(simTime);
      mLastUpdateTime = simTime;
   }
}

} // namespace PV
