/*
 * TriggerTestLayer.hpp
 * Author: slundquist
 */

#ifndef TRIGGERTESTLAYER_HPP_
#define TRIGGERTESTLAYER_HPP_ 
#include <layers/HyPerLayer.hpp>

namespace PV{

class TriggerTestLayer : public PV::HyPerLayer{
public:
   TriggerTestLayer(const char * name, HyPerCol * hc);
   int virtual updateStateWrapper (double time, double dt);
};

}
#endif /* IMAGETESTPROBE_HPP */
