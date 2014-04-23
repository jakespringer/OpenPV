/*
 * MLPErrorLayer.hpp
 *
 *  Created on: Mar 21, 2014
 *      Author: slundquist
 */

#ifndef MLPERRORLAYER_HPP_
#define MLPERRORLAYER_HPP_

#include "ANNLayer.hpp"
#include "MLPForwardLayer.hpp"

namespace PV {

class MLPErrorLayer: public PV::ANNLayer {
public:
   MLPErrorLayer(const char * name, HyPerCol * hc);
   virtual ~MLPErrorLayer();
   virtual int communicateInitInfo();
   virtual int allocateDataStructures();
protected:
   MLPErrorLayer();
   virtual int initialize(const char * name, HyPerCol * hc);
   virtual int allocateV();
   virtual int initializeV();
   virtual int checkpointRead(const char * cpDir, double * timed);
   virtual int checkpointWrite(const char * cpDir);
   virtual int ioParamsFillGroup(enum ParamsIOFlag ioFlag);
   virtual void ioParam_eta(enum ParamsIOFlag ioFlag);
   virtual void ioParam_Vrest(enum ParamsIOFlag ioFlag);
   virtual void ioParam_VthRest(enum ParamsIOFlag ioFlag);
   virtual void ioParam_SigmoidAlpha(enum ParamsIOFlag ioFlag);
   virtual void ioParam_ForwardLayername(enum ParamsIOFlag ioFlag);
   virtual int updateState(double time, double dt);
private:
   int initialize_base();
   float * bias;
   float eta; //A sort of dw in mlp
   float Vrest;
   float VthRest;
   float sigmoid_alpha;
   char * forwardLayername;
   MLPForwardLayer* forwardLayer;
};

} /* namespace PV */
#endif /* ANNERRORLAYER_HPP_ */
