/*
 * PlasticConnTestProbe.hpp
 *
 *  Created on: Mar 10, 2009
 *      Author: garkenyon
 */

#ifndef PLASTICCONNTESTPROBE_HPP_
#define PLASTICCONNTESTPROBE_HPP_

#include "../PetaVision/src/io/KernelProbe.hpp"

namespace PV {

class PlasticConnTestProbe: public KernelProbe {
public:
   PlasticConnTestProbe(const char * probename, HyPerCol * hc);

   virtual int outputState(double timed);

   virtual ~PlasticConnTestProbe();

protected:
   int initialize(const char * probename, HyPerCol * hc);

protected:
   bool errorPresent;
};

}

#endif /* PLASTICCONNTESTPROBE_HPP_ */
