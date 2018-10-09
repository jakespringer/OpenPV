/*
 * Retina.cpp
 *
 *  Created on: Jul 29, 2008
 *
 */

#include "Retina.hpp"
#include "HyPerLayer.hpp"
#include "include/default_params.h"
#include "io/io.hpp"
#include "io/randomstateio.hpp"
#include "utils/cl_random.h"

#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Retina_spiking_update_state(
      const int nbatch,
      const int numNeurons,
      const double timed,
      const double dt,
      const int nx,
      const int ny,
      const int nf,
      const int lt,
      const int rt,
      const int dn,
      const int up,
      Retina_params *params,
      taus_uint4 *rnd,
      float const *GSynHead,
      float *activity,
      float *timeSinceLast);

void Retina_nonspiking_update_state(
      const int nbatch,
      const int numNeurons,
      const double timed,
      const double dt,
      const int nx,
      const int ny,
      const int nf,
      const int lt,
      const int rt,
      const int dn,
      const int up,
      Retina_params *params,
      float const *GSynHead,
      float *activity);

namespace PV {

Retina::Retina() {
   initialize_base();
   // Default constructor to be called by derived classes.
   // It doesn't call Retina::initialize; instead, the derived class
   // should explicitly call Retina::initialize in its own initialization,
   // the way that Retina::initialize itself calls HyPerLayer::initialization.
   // This way, virtual methods called by initialize will be overridden
   // as expected.
}

Retina::Retina(const char *name, HyPerCol *hc) {
   initialize_base();
   initialize(name, hc);
}

Retina::~Retina() { delete randState; }

int Retina::initialize_base() {
   randState                     = NULL;
   spikingFlag                   = true;
   rParams.abs_refractory_period = 0.0f;
   rParams.refractory_period     = 0.0f;
   rParams.beginStim             = 0.0f;
   rParams.endStim               = -1.0;
   rParams.burstDuration         = 1000.0;
   rParams.burstFreq             = 1.0f;
   rParams.probBase              = 0.0f;
   rParams.probStim              = 1.0f;
   return PV_SUCCESS;
}

int Retina::initialize(const char *name, HyPerCol *hc) {
   int status = HyPerLayer::initialize(name, hc);
   mLayerInput->requireChannel(CHANNEL_EXC);
   mLayerInput->requireChannel(CHANNEL_INH);
   pvAssert(mLayerInput->getNumChannels() == mNumRetinaChannels);

   return status;
}

InternalStateBuffer *Retina::createInternalState() { return nullptr; }

Response::Status
Retina::communicateInitInfo(std::shared_ptr<CommunicateInitInfoMessage const> message) {
   auto status = HyPerLayer::communicateInitInfo(message);
   if (!Response::completed(status)) {
      return status;
   }
   return status;
}

Response::Status Retina::allocateDataStructures() {
   auto status = HyPerLayer::allocateDataStructures();
   if (!Response::completed(status)) {
      return status;
   }

   assert(!parameters()->presentAndNotBeenRead(name, "spikingFlag"));
   if (spikingFlag) {
      // // a random state variable is needed for every neuron/clthread
      const PVLayerLoc *loc = getLayerLoc();
      // Allocate extended loc
      randState = new Random(loc, true);
      allocateExtendedBuffer(&mSinceLastSpike, "SinceLastSpike");
      status = Response::SUCCESS;
   }

   return status;
}

Response::Status Retina::initializeState(std::shared_ptr<InitializeStateMessage const> message) {
   pvAssert(mInternalState == nullptr); // Retina does not use V.
   if (triggerLayer and triggerBehaviorType == UPDATEONLY_TRIGGER) {
      if (!triggerLayer->getInitialValuesSetFlag()) {
         return Response::POSTPONE;
      }
      mDeltaUpdateTime = triggerLayer->getDeltaUpdateTime();
   }
   else {
      setNontriggerDeltaUpdateTime(message->mDeltaTime);
   }
   setRetinaParams(message->mDeltaTime);
   if (spikingFlag) {
      for (int k = 0; k < getNumExtendedAllBatches(); k++) {
         mSinceLastSpike[k] = 10 * rParams.abs_refractory_period; // allow neuron to fire at t=0
      }
   }
   updateState(0.0, message->mDeltaTime);
   mLastUpdateTime  = message->mDeltaTime; // Retina ignores these values, since it updates every
   mLastTriggerTime = message->mDeltaTime; // timestep, but this stays consistent with HyPerLayer.
   return Response::SUCCESS;
}

int Retina::ioParamsFillGroup(enum ParamsIOFlag ioFlag) {
   int status = HyPerLayer::ioParamsFillGroup(ioFlag);
   ioParam_spikingFlag(ioFlag);
   ioParam_foregroundRate(ioFlag);
   ioParam_backgroundRate(ioFlag);
   ioParam_beginStim(ioFlag);
   ioParam_endStim(ioFlag);
   ioParam_burstFreq(ioFlag);
   ioParam_burstDuration(ioFlag);
   ioParam_refractoryPeriod(ioFlag);
   ioParam_absRefractoryPeriod(ioFlag);

   return status;
}

void Retina::ioParam_spikingFlag(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(ioFlag, name, "spikingFlag", &spikingFlag, true);
}

void Retina::ioParam_foregroundRate(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(ioFlag, name, "foregroundRate", &probStimParam, 1.0f);
}

void Retina::ioParam_backgroundRate(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(ioFlag, name, "backgroundRate", &probBaseParam, 0.0f);
}

void Retina::ioParam_beginStim(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(ioFlag, name, "beginStim", &rParams.beginStim, 0.0);
}

void Retina::ioParam_endStim(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(ioFlag, name, "endStim", &rParams.endStim, (double)FLT_MAX);
   if (ioFlag == PARAMS_IO_READ && rParams.endStim < 0)
      rParams.endStim = FLT_MAX;
}

void Retina::ioParam_burstFreq(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(ioFlag, name, "burstFreq", &rParams.burstFreq, 1.0f);
}

void Retina::ioParam_burstDuration(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(ioFlag, name, "burstDuration", &rParams.burstDuration, 1000.0f);
}

void Retina::ioParam_refractoryPeriod(enum ParamsIOFlag ioFlag) {
   assert(!parameters()->presentAndNotBeenRead(name, "spikingFlag"));
   if (spikingFlag) {
      parameters()->ioParamValue(
            ioFlag, name, "refractoryPeriod", &rParams.refractory_period, mDefaultRefractoryPeriod);
   }
}

void Retina::ioParam_absRefractoryPeriod(enum ParamsIOFlag ioFlag) {
   assert(!parameters()->presentAndNotBeenRead(name, "spikingFlag"));
   if (spikingFlag) {
      parameters()->ioParamValue(
            ioFlag,
            name,
            "absRefractoryPeriod",
            &rParams.abs_refractory_period,
            mDefaultAbsRefractoryPeriod);
   }
}

int Retina::setRetinaParams(double deltaTime) {

   float dt_sec   = (float)deltaTime * 0.001f; // convert millisectonds to seconds
   float probStim = probStimParam * dt_sec;
   if (probStim > 1.0f) {
      probStim = 1.0f;
   }
   float probBase = probBaseParam * dt_sec;
   if (probBase > 1.0f) {
      probBase = 1.0f;
   }

   // default parameters
   //
   rParams.probStim = probStim;
   rParams.probBase = probBase;

   return 0;
}

Response::Status Retina::readStateFromCheckpoint(Checkpointer *checkpointer) {
   pvAssert(mInitializeFromCheckpointFlag);
   auto status = HyPerLayer::readStateFromCheckpoint(checkpointer);
   if (!Response::completed(status)) {
      return status;
   }
   readRandStateFromCheckpoint(checkpointer);
   return Response::SUCCESS;
}

void Retina::readRandStateFromCheckpoint(Checkpointer *checkpointer) {
   checkpointer->readNamedCheckpointEntry(
         std::string(name), std::string("rand_state.pvp"), false /*not constant*/);
}

Response::Status
Retina::registerData(std::shared_ptr<RegisterDataMessage<Checkpointer> const> message) {
   auto status = HyPerLayer::registerData(message);
   if (!Response::completed(status)) {
      return status;
   }
   if (spikingFlag) {
      pvAssert(randState != nullptr);
      auto *checkpointer = message->mDataRegistry;
      checkpointRandState(checkpointer, "rand_state", randState, true /*extended*/);
      checkpointPvpActivityFloat(checkpointer, "SinceLastSpike", getActivity(), true /*extended*/);
   }
   return Response::SUCCESS;
}

//! Updates the state of the Retina
/*!
 * REMARKS:
 *      - mSinceLastSpike[] buffer holds the time since a neuron last spiked.
 *      - not used if nonspiking
 *      - it sets the probStim and probBase.
 *              - probStim = noiseOnFreq * dt_sec * (phiExc - phiInh); the last ()  is V[k];
 *              - probBase = noiseOffFreq * dt_sec;
 *              .
 *      - activity[] is set to 0 or 1 depending on the return of spike()
 *      - this depends on the last time a neuron spiked as well as on V[]
 *      at the location of the neuron. This V[] is set by calling updateImage().
 *      - V points to the same memory space as data in the Image so that when Image
 *      is updated, V gets updated too.
 *      .
 *      .
 *
 *
 */
Response::Status Retina::updateState(double timed, double dt) {
   const int nx     = getLayerLoc()->nx;
   const int ny     = getLayerLoc()->ny;
   const int nf     = getLayerLoc()->nf;
   const int nbatch = getLayerLoc()->nbatch;

   float const *GSynHead = mLayerInput->getBufferData();
   float *activity       = mActivity->getActivity();

   if (spikingFlag == 1) {
      Retina_spiking_update_state(
            nbatch,
            getNumNeurons(),
            timed,
            dt,
            nx,
            ny,
            nf,
            getLayerLoc()->halo.lt,
            getLayerLoc()->halo.rt,
            getLayerLoc()->halo.dn,
            getLayerLoc()->halo.up,
            &rParams,
            randState->getRNG(0),
            GSynHead,
            activity,
            mSinceLastSpike);
   }
   else {
      Retina_nonspiking_update_state(
            nbatch,
            getNumNeurons(),
            timed,
            dt,
            nx,
            ny,
            nf,
            getLayerLoc()->halo.lt,
            getLayerLoc()->halo.rt,
            getLayerLoc()->halo.dn,
            getLayerLoc()->halo.up,
            &rParams,
            GSynHead,
            activity);
   }

#ifdef DEBUG_PRINT
   char filename[132];
   sprintf(filename, "r_%d.tiff", (int)(2 * timed));
   this->writeActivity(filename, timed);

   DebugLog(debugRetina);
   debugRetina().printf("----------------\n");
   for (int k = 0; k < 6; k++) {
      debugRetina().printf("host:: k==%d h_exc==%f h_inh==%f\n", k, phiExc[k], phiInh[k]);
   }
   debugRetina().printf("----------------\n");

#endif // DEBUG_PRINT
   return Response::SUCCESS;
}

} // namespace PV

///////////////////////////////////////////////////////
//
// implementation of Retina kernels
//

/*
 * Spiking method for Retina
 * Returns 1 if an event should occur, 0 otherwise. This is a stochastic model.
 *
 * REMARKS:
 *      - During abs_refractory_period a neuron does not spike
 *      - The neurons that correspond to stimuli (on Image pixels)
 *        spike with probability probStim.
 *      - The neurons that correspond to background image pixels
 *        spike with probability probBase.
 *      - After abs_refractory_period the spiking probability
 *        grows exponentially to probBase and probStim respectively.
 *      - The burst of the retina is periodic with period T set by
 *        T = 1000/burstFreq in milliseconds
 *      - When the time t is such that mT < t < mT + burstDuration, where m is
 *        an integer, the burstStatus is set to 1.
 *      - The burstStatus is also determined by the condition that
 *        beginStim < t < endStim. These parameters are set in the input
 *        params file params.stdp
 *      - sinAmp modulates the spiking probability only when burstDuration <= 0
 *        or burstFreq = 0
 *      - probSpike is set to probBase for all neurons.
 *      - for neurons exposed to Image on pixels, probSpike increases with probStim.
 *      - When the probability is negative, the neuron does not spike.
 *
 * NOTES:
 *      - time is measured in milliseconds.
 *
 */

static inline float calcBurstStatus(double timed, Retina_params *params) {
   float burstStatus;
   if (params->burstDuration <= 0 || params->burstFreq == 0) {
      burstStatus = cosf(2.0f * PI * (float)timed * params->burstFreq / 1000.0f);
   }
   else {
      burstStatus = fmodf((float)timed, 1000.0f / params->burstFreq);
      burstStatus = burstStatus < params->burstDuration;
   }
   burstStatus *= (int)((timed >= params->beginStim) && (timed < params->endStim));
   return burstStatus;
}

static inline int
spike(float timed,
      float dt,
      float timeSinceLast,
      float stimFactor,
      taus_uint4 *rnd_state,
      float burst_status,
      Retina_params *params) {
   float probSpike;

   // input parameters
   //
   float probBase = params->probBase;
   float probStim = params->probStim * stimFactor;

   // see if neuron is in a refractory period
   //
   if (timeSinceLast < params->abs_refractory_period) {
      return 0;
   }
   else {
      float delta   = timeSinceLast - params->abs_refractory_period;
      float refract = 1.0f - expf(-delta / params->refractory_period);
      refract       = (refract < 0) ? 0 : refract;
      probBase *= refract;
      probStim *= refract;
   }

   probSpike = probBase;

   probSpike += probStim * burst_status; // negative prob is OK
   // probSpike is spikes per millisecond; conversion to expected number of spikes in dt takes place
   // in setRetinaParams

   *rnd_state     = cl_random_get(*rnd_state);
   int spike_flag = (cl_random_prob(*rnd_state) < probSpike);
   return spike_flag;
}

//
// update the state of a retinal layer (spiking)
//
//    assume called with 1D kernel
//
void Retina_spiking_update_state(
      const int nbatch,
      const int numNeurons,
      const double timed,
      const double dt,

      const int nx,
      const int ny,
      const int nf,
      const int lt,
      const int rt,
      const int dn,
      const int up,

      Retina_params *params,
      taus_uint4 *rnd,
      float const *GSynHead,
      float *activity,
      float *timeSinceLast) {

   float const *phiExc = &GSynHead[CHANNEL_EXC * nbatch * numNeurons];
   float const *phiInh = &GSynHead[CHANNEL_INH * nbatch * numNeurons];
   for (int b = 0; b < nbatch; b++) {
      taus_uint4 *rndBatch      = rnd + b * nx * ny * nf;
      float const *phiExcBatch  = phiExc + b * nx * ny * nf;
      float const *phiInhBatch  = phiInh + b * nx * ny * nf;
      float *timeSinceLastBatch = timeSinceLast + b * (nx + lt + rt) * (ny + up + dn) * nf;
      float *activityBatch      = activity + b * (nx + lt + rt) * (ny + up + dn) * nf;
      int k;
      float burst_status = calcBurstStatus(timed, params);
      for (k = 0; k < nx * ny * nf; k++) {
         int kex = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
         //
         // kernel (nonheader part) begins here
         //
         // load local variables from global memory
         //
         taus_uint4 l_rnd      = rndBatch[k];
         float l_phiExc        = phiExcBatch[k];
         float l_phiInh        = phiInhBatch[k];
         float l_timeSinceLast = timeSinceLastBatch[kex] + (float)dt;
         float l_activ;
         l_activ = (float)spike(
               (float)timed,
               (float)dt,
               l_timeSinceLast,
               (l_phiExc - l_phiInh),
               &l_rnd,
               burst_status,
               params);
         l_timeSinceLast = (l_activ > 0.0f) ? 0.0f : l_timeSinceLast;
         // store local variables back to global memory
         //
         rndBatch[k]        = l_rnd;
         timeSinceLast[kex] = l_timeSinceLast;
         activityBatch[kex] = l_activ;
      }
   }
}

//
// update the state of a retinal layer (non-spiking)
//
//    assume called with 1D kernel
//
void Retina_nonspiking_update_state(
      const int nbatch,
      const int numNeurons,
      const double timed,
      const double dt,

      const int nx,
      const int ny,
      const int nf,
      const int lt,
      const int rt,
      const int dn,
      const int up,

      Retina_params *params,
      float const *GSynHead,
      float *activity) {
   int k;
   float burstStatus = calcBurstStatus(timed, params);

   float const *phiExc = &GSynHead[CHANNEL_EXC * nbatch * numNeurons];
   float const *phiInh = &GSynHead[CHANNEL_INH * nbatch * numNeurons];

   for (int b = 0; b < nbatch; b++) {
      float const *phiExcBatch = phiExc + b * nx * ny * nf;
      float const *phiInhBatch = phiInh + b * nx * ny * nf;
      float *activityBatch     = activity + b * (nx + lt + rt) * (ny + up + dn) * nf;
      for (k = 0; k < nx * ny * nf; k++) {
         int kex = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
         //
         // kernel (nonheader part) begins here
         //

         // load local variables from global memory
         //
         float l_phiExc = phiExcBatch[k];
         float l_phiInh = phiInhBatch[k];
         float l_activ;
         // adding base prob should not change default behavior
         l_activ = burstStatus * params->probStim * (l_phiExc - l_phiInh) + params->probBase;
         // store local variables back to global memory
         //
         activityBatch[kex] = l_activ;
      }
   }
}
