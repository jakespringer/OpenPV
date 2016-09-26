/*
 * LIF.cpp
 *
 *  Created on: Dec 21, 2010
 *      Author: Craig Rasmussen
 */

#include "HyPerLayer.hpp"
#include "LIF.hpp"

#include "../include/pv_common.h"
#include "../include/default_params.h"
#include "../io/fileio.hpp"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void LIF_update_state_arma(
    const int nbatch,
    const int numNeurons,
    const float time,
    const float dt,

    const int nx,
    const int ny,
    const int nf,
    const int lt,
    const int rt,
    const int dn,
    const int up,

    LIF_params * params,
    taus_uint4 * rnd,

    float * V,
    float * Vth,
    float * G_E,
    float * G_I,
    float * G_IB,
    float * GSynHead,
    float * activity);

void LIF_update_state_beginning(
    const int nbatch,
    const int numNeurons,
    const float time,
    const float dt,

    const int nx,
    const int ny,
    const int nf,
    const int lt,
    const int rt,
    const int dn,
    const int up,

    LIF_params * params,
    taus_uint4 * rnd,

    float * V,
    float * Vth,
    float * G_E,
    float * G_I,
    float * G_IB,
    float * GSynHead,
    float * activity);

void LIF_update_state_original(
    const int nbatch,
    const int numNeurons,
    const float time,
    const float dt,

    const int nx,
    const int ny,
    const int nf,
    const int lt,
    const int rt,
    const int dn,
    const int up,

    LIF_params * params,
    taus_uint4 * rnd,

    float * V,
    float * Vth,
    float * G_E,
    float * G_I,
    float * G_IB,
    float * GSynHead,
    float * activity);


namespace PV
{

LIF::LIF() {
   initialize_base();
   // this is the constructor to be used by derived classes, it does not produce
   // a function class but is asking for an init by the derived class
}

LIF::LIF(const char * name, HyPerCol * hc) {
   initialize_base();
   initialize(name, hc, "LIF_update_state");
}

LIF::~LIF() {
   if (numChannels > 0) {
      // conductances allocated contiguously so this frees all
      free(G_E);
   }
   free(Vth);
   delete randState;
   free(methodString);
}

int LIF::initialize_base() {
   numChannels = 3;
   randState = NULL;
   Vth = NULL;
   G_E = NULL;
   G_I = NULL;
   G_IB = NULL;
   methodString = NULL;

   return PV_SUCCESS;
}

// Initialize this class
int LIF::initialize(const char * name, HyPerCol * hc, const char * kernel_name) {
   HyPerLayer::initialize(name, hc);
   clayer->params = &lParams;

   return PV_SUCCESS;
}
// Set Parameters
//

int LIF::ioParamsFillGroup(enum ParamsIOFlag ioFlag)
{
   HyPerLayer::ioParamsFillGroup(ioFlag);

   ioParam_Vrest(ioFlag);
   ioParam_Vexc(ioFlag);
   ioParam_Vinh(ioFlag);
   ioParam_VinhB(ioFlag);
   ioParam_VthRest(ioFlag);
   ioParam_tau(ioFlag);
   ioParam_tauE(ioFlag);
   ioParam_tauI(ioFlag);
   ioParam_tauIB(ioFlag);
   ioParam_tauVth(ioFlag);
   ioParam_deltaVth(ioFlag);
   ioParam_deltaGIB(ioFlag);

   // NOTE: in LIFDefaultParams, noise ampE, ampI, ampIB were
   // ampE=0*NOISE_AMP*( 1.0/TAU_EXC )
   //       *(( TAU_INH * (V_REST-V_INH) + TAU_INHB * (V_REST-V_INHB) ) / (V_EXC-V_REST))
   // ampI=0*NOISE_AMP*1.0
   // ampIB=0*NOISE_AMP*1.0
   // 

   ioParam_noiseAmpE(ioFlag);
   ioParam_noiseAmpI(ioFlag);
   ioParam_noiseAmpIB(ioFlag);
   ioParam_noiseFreqE(ioFlag);
   ioParam_noiseFreqI(ioFlag);
   ioParam_noiseFreqIB(ioFlag);

   ioParam_method(ioFlag);
   return 0;
}
void LIF::ioParam_Vrest(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "Vrest", &lParams.Vrest, (float) V_REST); }
void LIF::ioParam_Vexc(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "Vexc", &lParams.Vexc, (float) V_EXC); }
void LIF::ioParam_Vinh(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "Vinh", &lParams.Vinh, (float) V_INH); }
void LIF::ioParam_VinhB(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "VinhB", &lParams.VinhB, (float) V_INHB); }
void LIF::ioParam_VthRest(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "VthRest", &lParams.VthRest, (float) VTH_REST); }
void LIF::ioParam_tau(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "tau", &lParams.tau, (float) TAU_VMEM); }
void LIF::ioParam_tauE(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "tauE", &lParams.tauE, (float) TAU_EXC); }
void LIF::ioParam_tauI(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "tauI", &lParams.tauI, (float) TAU_INH); }
void LIF::ioParam_tauIB(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "tauIB", &lParams.tauIB, (float) TAU_INHB); }
void LIF::ioParam_tauVth(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "tauVth", &lParams.tauVth, (float) TAU_VTH); }
void LIF::ioParam_deltaVth(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "deltaVth", &lParams.deltaVth, (float) DELTA_VTH); }
void LIF::ioParam_deltaGIB(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "deltaGIB", &lParams.deltaGIB, (float) DELTA_G_INHB); }
void LIF::ioParam_noiseAmpE(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "noiseAmpE", &lParams.noiseAmpE, 0.0f); }
void LIF::ioParam_noiseAmpI(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "noiseAmpI", &lParams.noiseAmpI, 0.0f); }
void LIF::ioParam_noiseAmpIB(enum ParamsIOFlag ioFlag) { parent->parameters()->ioParamValue(ioFlag, name, "noiseAmpIB", &lParams.noiseAmpIB, 0.0f); }

void LIF::ioParam_noiseFreqE(enum ParamsIOFlag ioFlag) {
   parent->parameters()->ioParamValue(ioFlag, name, "noiseFreqE", &lParams.noiseFreqE, 250.0f);
   if (ioFlag==PARAMS_IO_READ) {
      float dt_sec = 0.001f * (float)parent->getDeltaTime();// seconds
      if (dt_sec * lParams.noiseFreqE  > 1.0f) {
         lParams.noiseFreqE  = 1.0f / dt_sec;
      }
   }
}

void LIF::ioParam_noiseFreqI(enum ParamsIOFlag ioFlag) {
   parent->parameters()->ioParamValue(ioFlag, name, "noiseFreqI", &lParams.noiseFreqI, 250.0f);
   if (ioFlag==PARAMS_IO_READ) {
      float dt_sec = 0.001f * (float)parent->getDeltaTime();// seconds
      if (dt_sec * lParams.noiseFreqI  > 1.0f) {
         lParams.noiseFreqI  = 1.0f / dt_sec;
      }
   }
}

void LIF::ioParam_noiseFreqIB(enum ParamsIOFlag ioFlag) {
   parent->parameters()->ioParamValue(ioFlag, name, "noiseFreqIB", &lParams.noiseFreqIB, 250.0f);
   if (ioFlag==PARAMS_IO_READ) {
      float dt_sec = 0.001f * (float)parent->getDeltaTime();// seconds
      if (dt_sec * lParams.noiseFreqIB > 1.0f) {
         lParams.noiseFreqIB = 1.0f / dt_sec;
      }
   }
}

void LIF::ioParam_method(enum ParamsIOFlag ioFlag) {
   // Read the integration method: one of 'arma' (preferred), 'beginning' (deprecated), or 'original' (deprecated).
   const char * default_method = "arma";
   parent->parameters()->ioParamString(ioFlag, name, "method", &methodString, default_method, true/*warnIfAbsent*/);
   if (ioFlag != PARAMS_IO_READ) {
      return;
   }

   assert(methodString);
   if (methodString[0] == '\0') {
      free(methodString);
      methodString = strdup(default_method);
      if (methodString==NULL) {
         pvError().printf("%s: unable to set method string: %s\n", getDescription_c(), strerror(errno));
      }
   }
   method = methodString ? methodString[0] : 'a'; // Default is ARMA; 'beginning' and 'original' are deprecated.
   if (method != 'o' && method != 'b' && method != 'a') {
      if (getParent()->columnId()==0) {
         pvErrorNoExit().printf("LIF::setLIFParams error.  Layer \"%s\" has method \"%s\".  Allowable values are \"arma\", \"beginning\" and \"original\".", name, methodString);
      }
      MPI_Barrier(parent->getCommunicator()->communicator());
      exit(EXIT_FAILURE);
   }
   if (method != 'a') {
      if (getParent()->columnId()==0) {
         pvWarn().printf("LIF layer \"%s\" integration method \"%s\" is deprecated.  Method \"arma\" is preferred.\n", name, methodString);
      }
   }
}

int LIF::setActivity() {
   pvdata_t * activity = clayer->activity->data;
   memset(activity, 0, sizeof(pvdata_t) * clayer->numExtendedAllBatches);
   return 0;
}

int LIF::communicateInitInfo() {
   int status = HyPerLayer::communicateInitInfo();

   return status;
}

int LIF::allocateDataStructures() {
   int status = HyPerLayer::allocateDataStructures();

   // // a random state variable is needed for every neuron/clthread
   randState = new Random(getLayerLoc(), false/*isExtended*/);
   if (randState == NULL) {
      pvError().printf("LIF::initialize:  %s unable to create object of Random class.\n", getDescription_c());
   }

   int numNeurons = getNumNeuronsAllBatches();
   assert(Vth); // Allocated when HyPerLayer::allocateDataStructures() called allocateBuffers().
   for (size_t k = 0; k < numNeurons; k++){
      Vth[k] = lParams.VthRest; // lParams.VthRest is set in setLIFParams
   }
   return status;
}

int LIF::allocateBuffers() {
   int status = allocateConductances(numChannels);
   assert(status==PV_SUCCESS);
   Vth = (pvdata_t *) calloc((size_t) getNumNeuronsAllBatches(), sizeof(pvdata_t));
   if(Vth == NULL) {
      pvError().printf("%s: rank %d process unable to allocate memory for Vth: %s\n",
            getDescription_c(), parent->columnId(), strerror(errno));
   }
   return HyPerLayer::allocateBuffers();
}

int LIF::allocateConductances(int num_channels) {
   assert(num_channels>=3); // Need exc, inh, and inhb at a minimum.
   const int numNeurons = getNumNeuronsAllBatches();
   G_E = (pvdata_t *) calloc((size_t) (getNumNeuronsAllBatches()*numChannels), sizeof(pvdata_t));
   if(G_E == NULL) {
      pvError().printf("%s: rank %d process unable to allocate memory for %d conductances: %s\n",
            getDescription_c(), parent->columnId(), num_channels, strerror(errno));
   }

   G_I  = G_E + 1*numNeurons;
   G_IB = G_E + 2*numNeurons;
   return PV_SUCCESS;
}

int LIF::readStateFromCheckpoint(const char * cpDir, double * timeptr) {
   int status = HyPerLayer::readStateFromCheckpoint(cpDir, timeptr);
   status = readVthFromCheckpoint(cpDir, timeptr);
   status = readG_EFromCheckpoint(cpDir, timeptr);
   status = readG_IFromCheckpoint(cpDir, timeptr);
   status = readG_IBFromCheckpoint(cpDir, timeptr);
   status = readRandStateFromCheckpoint(cpDir, timeptr);

   return PV_SUCCESS;
}

int LIF::readVthFromCheckpoint(const char * cpDir, double * timeptr) {
   char * filename = parent->pathInCheckpoint(cpDir, getName(), "_Vth.pvp");
   int status = readBufferFile(filename, parent->getCommunicator(), timeptr, &Vth, 1, /*extended*/true, getLayerLoc());
   assert(status==PV_SUCCESS);
   free(filename);
   return status;
}

int LIF::readG_EFromCheckpoint(const char * cpDir, double * timeptr) {
   char * filename = parent->pathInCheckpoint(cpDir, getName(), "_G_E.pvp");
   int status = readBufferFile(filename, parent->getCommunicator(), timeptr, &G_E, 1, /*extended*/true, getLayerLoc());
   assert(status==PV_SUCCESS);
   free(filename);
   return status;
}

int LIF::readG_IFromCheckpoint(const char * cpDir, double * timeptr) {
   char * filename = parent->pathInCheckpoint(cpDir, getName(), "_G_I.pvp");
   int status = readBufferFile(filename, parent->getCommunicator(), timeptr, &G_I, 1, /*extended*/true, getLayerLoc());
   assert(status==PV_SUCCESS);
   free(filename);
   return status;
}

int LIF::readG_IBFromCheckpoint(const char * cpDir, double * timeptr) {
   char * filename = parent->pathInCheckpoint(cpDir, getName(), "_G_IB.pvp");
   int status = readBufferFile(filename, parent->getCommunicator(), timeptr, &G_IB, 1, /*extended*/true, getLayerLoc());
   assert(status==PV_SUCCESS);
   free(filename);
   return status;
}

int LIF::readRandStateFromCheckpoint(const char * cpDir, double * timeptr) {
   char * filename = parent->pathInCheckpoint(cpDir, getName(), "_rand_state.bin");
   int status = readRandState(filename, parent->getCommunicator(), randState->getRNG(0), getLayerLoc(), false /*extended*/); // TODO Make a method in Random class
   assert(status==PV_SUCCESS);
   free(filename);
   return status;
}

int LIF::checkpointWrite(const char * cpDir) {
   HyPerLayer::checkpointWrite(cpDir);
   Communicator * icComm = parent->getCommunicator();
   double timed = (double) parent->simulationTime();
   int filenamesize = strlen(cpDir)+1+strlen(name)+16;
   // The +1 is for the slash between cpDir and name; the +16 needs to be large enough to hold the suffix (e.g. _rand_state.bin) plus the null terminator
   char * filename = (char *) malloc( filenamesize*sizeof(char) );
   assert(filename != NULL);
   int chars_needed;

   chars_needed = snprintf(filename, filenamesize, "%s/%s_Vth.pvp", cpDir, name);
   assert(chars_needed < filenamesize);
   writeBufferFile(filename, icComm, timed, &Vth, 1, /*extended*/false, getLayerLoc());

   chars_needed = snprintf(filename, filenamesize, "%s/%s_G_E.pvp", cpDir, name);
   assert(chars_needed < filenamesize);
   writeBufferFile(filename, icComm, timed, &G_E, 1, /*extended*/false, getLayerLoc());

   chars_needed = snprintf(filename, filenamesize, "%s/%s_G_I.pvp", cpDir, name);
   assert(chars_needed < filenamesize);
   writeBufferFile(filename, icComm, timed, &G_I, 1, /*extended*/false, getLayerLoc());

   chars_needed = snprintf(filename, filenamesize, "%s/%s_G_IB.pvp", cpDir, name);
   assert(chars_needed < filenamesize);
   writeBufferFile(filename, icComm, timed, &G_IB, 1, /*extended*/false, getLayerLoc());

   chars_needed = snprintf(filename, filenamesize, "%s/%s_rand_state.bin", cpDir, name);
   assert(chars_needed < filenamesize);
   writeRandState(filename, parent->getCommunicator(), randState->getRNG(0), getLayerLoc(), false /*extended*/, parent->getVerifyWrites()); // TODO Make a method in Random class

   free(filename);
   return PV_SUCCESS;
}

int LIF::updateState(double time, double dt)
{
   int status = 0;
   update_timer->start();

   const int nx = clayer->loc.nx;
   const int ny = clayer->loc.ny;
   const int nf = clayer->loc.nf;
   const PVHalo * halo = &clayer->loc.halo;
   const int nbatch = clayer->loc.nbatch;

   pvdata_t * GSynHead   = GSyn[0];
   pvdata_t * activity = clayer->activity->data;

   switch (method) {
   case 'a':
      LIF_update_state_arma(nbatch, getNumNeurons(), time, dt, nx, ny, nf, halo->lt, halo->rt, halo->dn, halo->up, &lParams, randState->getRNG(0), clayer->V, Vth,
            G_E, G_I, G_IB, GSynHead, activity);
      break;
   case 'b':
      LIF_update_state_beginning(nbatch, getNumNeurons(), time, dt, nx, ny, nf, halo->lt, halo->rt, halo->dn, halo->up, &lParams, randState->getRNG(0), clayer->V, Vth,
            G_E, G_I, G_IB, GSynHead, activity);
      break;
   case 'o':
      LIF_update_state_original(nbatch, getNumNeurons(), time, dt, nx, ny, nf, halo->lt, halo->rt, halo->dn, halo->up, &lParams, randState->getRNG(0), clayer->V, Vth,
            G_E, G_I, G_IB, GSynHead, activity);
      break;
   default:
      assert(0);
      break;
   }
   update_timer->stop();
   return status;
}

float LIF::getChannelTimeConst(enum ChannelType channel_type)
{
   clayer->params = &lParams;
   float channel_time_const = 0.0f;
   switch (channel_type) {
   case CHANNEL_EXC:
      channel_time_const = lParams.tauE;
      break;
   case CHANNEL_INH:
      channel_time_const = lParams.tauI;
      break;
   case CHANNEL_INHB:
      channel_time_const = lParams.tauIB;
      break;
   default:
      channel_time_const = 0.0f;
      break;
   }
   return channel_time_const;
}

int LIF::findPostSynaptic(int dim, int maxSize, int col,
      // input: which layer, which neuron
      HyPerLayer *lSource, float pos[],

      // output: how many of our neurons are connected.
      // an array with their indices.
      // an array with their feature vectors.
      int* nNeurons, int nConnectedNeurons[], float *vPos)
{
   return 0;
}

} // namespace PV

///////////////////////////////////////////////////////
//
// implementation of LIF kernels
//

inline
float LIF_Vmem_derivative(
      const float Vmem,
      const float G_E,
      const float G_I,
      const float G_IB,
      const float V_E,
      const float V_I,
      const float V_IB,
      const float Vrest,
      const float tau) {
   float totalconductance = 1.0f + G_E + G_I + G_IB;
   float Vmeminf = (Vrest + V_E*G_E + V_I*G_I + V_IB*G_IB)/totalconductance;
   return totalconductance*(Vmeminf-Vmem)/tau;
}
//
// update the state of a retinal layer (spiking)
//
//    assume called with 1D kernel
//
// LIF_update_state_original uses an Euler scheme for V where the conductances over the entire timestep are taken to be the values calculated at the end of the timestep
// LIF_update_state_beginning uses a Heun scheme for V, using values of the conductances at both the beginning and end of the timestep.  Spikes in the input are applied at the beginning of the timestep.
// LIF_update_state_arma uses an auto-regressive moving average filter for V, applying the GSyn at the start of the timestep and assuming that tau_inf and V_inf vary linearly over the timestep.  See van Hateren, Journal of Vision (2005), p. 331.
//
void LIF_update_state_original(
    const int nbatch,
    const int numNeurons,
    const float time, 
    const float dt,

    const int nx,
    const int ny,
    const int nf,
    const int lt,
    const int rt,
    const int dn,
    const int up,

    LIF_params * params,
    taus_uint4 * rnd,
    float * V,
    float * Vth,
    float * G_E,
    float * G_I,
    float * G_IB,
    float * GSynHead,
    float * activity)
{
   int k;

   const float exp_tauE    = expf(-dt/params->tauE);
   const float exp_tauI    = expf(-dt/params->tauI);
   const float exp_tauIB   = expf(-dt/params->tauIB);
   const float exp_tauVth  = expf(-dt/params->tauVth);

   const float dt_sec = 0.001f * dt;   // convert to seconds


   for (k = 0; k < nx*ny*nf*nbatch; k++) {
      int kex = kIndexExtendedBatch(k, nbatch, nx, ny, nf, lt, rt, dn, up);

      //
      // kernel (nonheader part) begins here
      //

      // local param variables
      float tau, Vrest, VthRest, Vexc, Vinh, VinhB, deltaVth, deltaGIB;

      const float GMAX = 10.0f;

      // local variables
      float l_activ;

      taus_uint4 l_rnd = rnd[k];

      float l_V   = V[k];
      float l_Vth = Vth[k];

      float l_G_E  = G_E[k];
      float l_G_I  = G_I[k];
      float l_G_IB = G_IB[k];

      float * GSynExc = &GSynHead[CHANNEL_EXC*nbatch*numNeurons];
      float * GSynInh = &GSynHead[CHANNEL_INH*nbatch*numNeurons];
      float * GSynInhB = &GSynHead[CHANNEL_INHB*nbatch*numNeurons];
      float l_GSynExc  = GSynExc[k];
      float l_GSynInh  = GSynInh[k];
      float l_GSynInhB = GSynInhB[k];
      
      // temporary arrays
      float tauInf, VmemInf;

      //
      // start of LIF2_update_exact_linear
      //

      // define local param variables
      //
      tau   = params->tau;
      Vexc  = params->Vexc;
      Vinh  = params->Vinh;
      VinhB = params->VinhB;
      Vrest = params->Vrest;

      VthRest  = params->VthRest;
      deltaVth = params->deltaVth;
      deltaGIB = params->deltaGIB;

      // add noise
      //

      l_rnd = cl_random_get(l_rnd);
      if (cl_random_prob(l_rnd) < dt_sec*params->noiseFreqE) {
         l_rnd = cl_random_get(l_rnd);
         l_GSynExc = l_GSynExc + params->noiseAmpE*cl_random_prob(l_rnd);
      }

      l_rnd = cl_random_get(l_rnd);
      if (cl_random_prob(l_rnd) < dt_sec*params->noiseFreqI) {
         l_rnd = cl_random_get(l_rnd);
         l_GSynInh = l_GSynInh + params->noiseAmpI*cl_random_prob(l_rnd);
      }

      l_rnd = cl_random_get(l_rnd);
      if (cl_random_prob(l_rnd) < dt_sec*params->noiseFreqIB) {
         l_rnd = cl_random_get(l_rnd);
         l_GSynInhB = l_GSynInhB + params->noiseAmpIB*cl_random_prob(l_rnd);
      }

      l_G_E  = l_GSynExc  + l_G_E *exp_tauE;
      l_G_I  = l_GSynInh  + l_G_I *exp_tauI;
      l_G_IB = l_GSynInhB + l_G_IB*exp_tauIB;
      
      l_G_E  = (l_G_E  > GMAX) ? GMAX : l_G_E;
      l_G_I  = (l_G_I  > GMAX) ? GMAX : l_G_I;
      l_G_IB = (l_G_IB > GMAX) ? GMAX : l_G_IB;

      tauInf  = (dt/tau) * (1.0f + l_G_E + l_G_I + l_G_IB);
      VmemInf = (Vrest + l_G_E*Vexc + l_G_I*Vinh + l_G_IB*VinhB)
              / (1.0f + l_G_E + l_G_I + l_G_IB);

      l_V = VmemInf + (l_V - VmemInf)*expf(-tauInf);

      //
      // start of LIF2_update_finish
      //

      l_Vth = VthRest + (l_Vth - VthRest)*exp_tauVth;

      //
      // start of update_f
      //

      bool fired_flag = (l_V > l_Vth);

      l_activ = fired_flag ? 1.0f                 : 0.0f;
      l_V     = fired_flag ? Vrest                : l_V;
      l_Vth   = fired_flag ? l_Vth + deltaVth     : l_Vth;
      l_G_IB  = fired_flag ? l_G_IB + deltaGIB    : l_G_IB;

      //
      // These actions must be done outside of kernel
      //    1. set activity to 0 in boundary (if needed)
      //    2. update active indices
      //

      // store local variables back to global memory
      //
      rnd[k] = l_rnd;

      activity[kex] = l_activ;
      
      V[k]   = l_V;
      Vth[k] = l_Vth;

      G_E[k]  = l_G_E;
      G_I[k]  = l_G_I;
      G_IB[k] = l_G_IB;

      GSynExc[k]  = 0.0f;
      GSynInh[k]  = 0.0f;
      GSynInhB[k] = 0.0f;

   } // loop over k
}

void LIF_update_state_beginning(
    const int nbatch,
    const int numNeurons,
    const float time,
    const float dt,

    const int nx,
    const int ny,
    const int nf,
    const int lt,
    const int rt,
    const int dn,
    const int up,

    LIF_params * params,
    taus_uint4 * rnd,
    float * V,
    float * Vth,
    float * G_E,
    float * G_I,
    float * G_IB,
    float * GSynHead,
//    float * GSynExc,
//    float * GSynInh,
//    float * GSynInhB,
    float * activity)
{
   int k;

   const float exp_tauE    = expf(-dt/params->tauE);
   const float exp_tauI    = expf(-dt/params->tauI);
   const float exp_tauIB   = expf(-dt/params->tauIB);
   const float exp_tauVth  = expf(-dt/params->tauVth);

   const float dt_sec = 0.001f * dt;   // convert to seconds


   for (k = 0; k < nx*ny*nf*nbatch; k++) {

      int kex = kIndexExtendedBatch(k, nbatch, nx, ny, nf, lt, rt, dn, up);

      //
      // kernel (nonheader part) begins here
      //

      // local param variables
      float tau, Vrest, VthRest, Vexc, Vinh, VinhB, deltaVth, deltaGIB;

      const float GMAX = 10.0f;

      // local variables
      float l_activ;

      taus_uint4 l_rnd = rnd[k];

      float l_V   = V[k];
      float l_Vth = Vth[k];

      // The correction factors to the conductances are so that if l_GSyn_* is the same every timestep,
      // then the asymptotic value of l_G_* will be l_GSyn_*
      float l_G_E  = G_E[k];
      float l_G_I  = G_I[k];
      float l_G_IB = G_IB[k];

      float * GSynExc = &GSynHead[CHANNEL_EXC*nbatch*numNeurons];
      float * GSynInh = &GSynHead[CHANNEL_INH*nbatch*numNeurons];
      float * GSynInhB = &GSynHead[CHANNEL_INHB*nbatch*numNeurons];
      float l_GSynExc  = GSynExc[k];
      float l_GSynInh  = GSynInh[k];
      float l_GSynInhB = GSynInhB[k];

      //
      // start of LIF2_update_exact_linear
      //

      // define local param variables
      //
      tau   = params->tau;
      Vexc  = params->Vexc;
      Vinh  = params->Vinh;
      VinhB = params->VinhB;
      Vrest = params->Vrest;

      VthRest  = params->VthRest;
      deltaVth = params->deltaVth;
      deltaGIB = params->deltaGIB;

      // add noise
      //

      l_rnd = cl_random_get(l_rnd);
      if (cl_random_prob(l_rnd) < dt_sec*params->noiseFreqE) {
         l_rnd = cl_random_get(l_rnd);
         l_GSynExc = l_GSynExc + params->noiseAmpE*cl_random_prob(l_rnd);
      }

      l_rnd = cl_random_get(l_rnd);
      if (cl_random_prob(l_rnd) < dt_sec*params->noiseFreqI) {
         l_rnd = cl_random_get(l_rnd);
         l_GSynInh = l_GSynInh + params->noiseAmpI*cl_random_prob(l_rnd);
      }

      l_rnd = cl_random_get(l_rnd);
      if (cl_random_prob(l_rnd) < dt_sec*params->noiseFreqIB) {
         l_rnd = cl_random_get(l_rnd);
         l_GSynInhB = l_GSynInhB + params->noiseAmpIB*cl_random_prob(l_rnd);
      }

      // The portion of code below uses the newer method of calculating l_V.
      float G_E_initial, G_I_initial, G_IB_initial, G_E_final, G_I_final, G_IB_final;
      float dV1, dV2, dV;

      G_E_initial = l_G_E + l_GSynExc;
      G_I_initial = l_G_I + l_GSynInh;
      G_IB_initial = l_G_IB + l_GSynInhB;

      G_E_initial  = (G_E_initial  > GMAX) ? GMAX : G_E_initial;
      G_I_initial  = (G_I_initial  > GMAX) ? GMAX : G_I_initial;
      G_IB_initial = (G_IB_initial > GMAX) ? GMAX : G_IB_initial;

      G_E_final = G_E_initial*exp_tauE;
      G_I_final = G_I_initial*exp_tauI;
      G_IB_final = G_IB_initial*exp_tauIB;

      dV1 = LIF_Vmem_derivative(l_V, G_E_initial, G_I_initial, G_IB_initial, Vexc, Vinh, VinhB, Vrest, tau);
      dV2 = LIF_Vmem_derivative(l_V+dt*dV1, G_E_final, G_I_final, G_IB_final, Vexc, Vinh, VinhB, Vrest, tau);
      dV = (dV1+dV2)*0.5f;
      l_V = l_V + dt*dV;

      l_G_E = G_E_final;
      l_G_I = G_I_final;
      l_G_IB = G_IB_final;

      l_Vth = VthRest + (l_Vth - VthRest)*exp_tauVth;
      // End of code unique to newer method.

      //
      // start of update_f
      //

      bool fired_flag = (l_V > l_Vth);

      l_activ = fired_flag ? 1.0f                 : 0.0f;
      l_V     = fired_flag ? Vrest                : l_V;
      l_Vth   = fired_flag ? l_Vth + deltaVth     : l_Vth;
      l_G_IB  = fired_flag ? l_G_IB + deltaGIB    : l_G_IB;

      //
      // These actions must be done outside of kernel
      //    1. set activity to 0 in boundary (if needed)
      //    2. update active indices
      //

      // store local variables back to global memory
      //
      rnd[k] = l_rnd;

      activity[kex] = l_activ;

      V[k]   = l_V;
      Vth[k] = l_Vth;

      G_E[k]  = l_G_E;
      G_I[k]  = l_G_I;
      G_IB[k] = l_G_IB;

   } // loop over k
}

void LIF_update_state_arma(
    const int nbatch,
    const int numNeurons,
    const float time,
    const float dt,

    const int nx,
    const int ny,
    const int nf,
    const int lt,
    const int rt,
    const int dn,
    const int up,

    LIF_params * params,
    taus_uint4 * rnd,
    float * V,
    float * Vth,
    float * G_E,
    float * G_I,
    float * G_IB,
    float * GSynHead,
    float * activity)
{
   int k;

   const float exp_tauE    = expf(-dt/params->tauE);
   const float exp_tauI    = expf(-dt/params->tauI);
   const float exp_tauIB   = expf(-dt/params->tauIB);
   const float exp_tauVth  = expf(-dt/params->tauVth);

   const float dt_sec = 0.001f * dt;   // convert to seconds


   for (k = 0; k < nx*ny*nf*nbatch; k++) {
      int kex = kIndexExtendedBatch(k, nbatch, nx, ny, nf, lt, rt, dn, up);

      //
      // kernel (nonheader part) begins here
      //

      // local param variables
      float tau, Vrest, VthRest, Vexc, Vinh, VinhB, deltaVth, deltaGIB;

      const float GMAX = 10.0;

      // local variables
      float l_activ;

      taus_uint4 l_rnd = rnd[k];

      float l_V   = V[k];
      float l_Vth = Vth[k];

      // The correction factors to the conductances are so that if l_GSyn_* is the same every timestep,
      // then the asymptotic value of l_G_* will be l_GSyn_*
      float l_G_E  = G_E[k];
      float l_G_I  = G_I[k];
      float l_G_IB = G_IB[k];

      float * GSynExc = &GSynHead[CHANNEL_EXC*nbatch*numNeurons];
      float * GSynInh = &GSynHead[CHANNEL_INH*nbatch*numNeurons];
      float * GSynInhB = &GSynHead[CHANNEL_INHB*nbatch*numNeurons];
      float l_GSynExc  = GSynExc[k];
      float l_GSynInh  = GSynInh[k];
      float l_GSynInhB = GSynInhB[k];

      //
      // start of LIF2_update_exact_linear
      //

      // define local param variables
      //
      tau   = params->tau;
      Vexc  = params->Vexc;
      Vinh  = params->Vinh;
      VinhB = params->VinhB;
      Vrest = params->Vrest;

      VthRest  = params->VthRest;
      deltaVth = params->deltaVth;
      deltaGIB = params->deltaGIB;

      // add noise
      //

      l_rnd = cl_random_get(l_rnd);
      if (cl_random_prob(l_rnd) < dt_sec*params->noiseFreqE) {
         l_rnd = cl_random_get(l_rnd);
         l_GSynExc = l_GSynExc + params->noiseAmpE*cl_random_prob(l_rnd);
      }

      l_rnd = cl_random_get(l_rnd);
      if (cl_random_prob(l_rnd) < dt_sec*params->noiseFreqI) {
         l_rnd = cl_random_get(l_rnd);
         l_GSynInh = l_GSynInh + params->noiseAmpI*cl_random_prob(l_rnd);
      }

      l_rnd = cl_random_get(l_rnd);
      if (cl_random_prob(l_rnd) < dt_sec*params->noiseFreqIB) {
         l_rnd = cl_random_get(l_rnd);
         l_GSynInhB = l_GSynInhB + params->noiseAmpIB*cl_random_prob(l_rnd);
      }

      // The portion of code below uses the newer method of calculating l_V.
      float G_E_initial, G_I_initial, G_IB_initial, G_E_final, G_I_final, G_IB_final;
      float tau_inf_initial, tau_inf_final, V_inf_initial, V_inf_final;

      G_E_initial = l_G_E + l_GSynExc;
      G_I_initial = l_G_I + l_GSynInh;
      G_IB_initial = l_G_IB + l_GSynInhB;
      tau_inf_initial = tau/(1+G_E_initial+G_I_initial+G_IB_initial);
      V_inf_initial = (Vrest+Vexc*G_E_initial+Vinh*G_I_initial+VinhB*G_IB_initial)/(1+G_E_initial+G_I_initial+G_IB_initial);

      G_E_initial  = (G_E_initial  > GMAX) ? GMAX : G_E_initial;
      G_I_initial  = (G_I_initial  > GMAX) ? GMAX : G_I_initial;
      G_IB_initial = (G_IB_initial > GMAX) ? GMAX : G_IB_initial;

      G_E_final = G_E_initial*exp_tauE;
      G_I_final = G_I_initial*exp_tauI;
      G_IB_final = G_IB_initial*exp_tauIB;

      tau_inf_final = tau/(1+G_E_final+G_I_final+G_IB_initial);
      V_inf_final = (Vrest+Vexc*G_E_final+Vinh*G_I_final+VinhB*G_IB_final)/(1+G_E_final+G_I_final+G_IB_final);

      float tau_slope = (tau_inf_final-tau_inf_initial)/dt;
      float f1 = tau_slope==0.0f ? expf(-dt/tau_inf_initial) : powf(tau_inf_final/tau_inf_initial, -1/tau_slope);
      float f2 = tau_slope==-1.0f ? tau_inf_initial/dt*logf(tau_inf_final/tau_inf_initial+1.0f) :
                                    (1-tau_inf_initial/dt*(1-f1))/(1+tau_slope);
      float f3 = 1.0f - f1 - f2;
      l_V = f1*l_V + f2*V_inf_initial + f3*V_inf_final;

      l_G_E = G_E_final;
      l_G_I = G_I_final;
      l_G_IB = G_IB_final;

      l_Vth = VthRest + (l_Vth - VthRest)*exp_tauVth;
      // End of code unique to newer method.

      //
      // start of update_f
      //

      bool fired_flag = (l_V > l_Vth);

      l_activ = fired_flag ? 1.0f                 : 0.0f;
      l_V     = fired_flag ? Vrest                : l_V;
      l_Vth   = fired_flag ? l_Vth + deltaVth     : l_Vth;
      l_G_IB  = fired_flag ? l_G_IB + deltaGIB    : l_G_IB;

      //
      // These actions must be done outside of kernel
      //    1. set activity to 0 in boundary (if needed)
      //    2. update active indices
      //

      // store local variables back to global memory
      //
      rnd[k] = l_rnd;

      activity[kex] = l_activ;

      V[k]   = l_V;
      Vth[k] = l_Vth;

      G_E[k]  = l_G_E;
      G_I[k]  = l_G_I;
      G_IB[k] = l_G_IB;
   }

}
