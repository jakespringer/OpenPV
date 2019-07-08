/*
 * SleepLayer.hpp
 *
 *  Created on: Jul 1, 2019
 *      Author: jspringer
 */

#ifndef SLEEPLAYER_HPP__
#define SLEEPLAYER_HPP__

#include "HyPerLayer.hpp"

namespace PV {

class SleepLayer : public HyPerLayer {
  public:
   SleepLayer(const char *name, PVParams *params, Communicator const *comm);
   virtual ~SleepLayer();

  protected:
   SleepLayer() {}

   void initialize(const char *name, PVParams *params, Communicator const *comm);

   virtual ActivityComponent *createActivityComponent() override;
};

} // end namespace PV

#endif // ANNLAYER_HPP_
