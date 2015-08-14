/*
 * AbstractNormProbe.cpp
 *
 *  Created on: Aug 11, 2015
 *      Author: pschultz
 */

#include "AbstractNormProbe.hpp"
#include "../columns/HyPerCol.hpp"

namespace PV {

AbstractNormProbe::AbstractNormProbe() : LayerProbe() {
   initAbstractNormProbe_base();
}

AbstractNormProbe::AbstractNormProbe(const char * probeName, HyPerCol * hc) : LayerProbe()
{
   initAbstractNormProbe_base();
   initAbstractNormProbe(probeName, hc);
}

AbstractNormProbe::~AbstractNormProbe() {
}

int AbstractNormProbe::initAbstractNormProbe(const char * probeName, HyPerCol * hc) {
   return initialize(probeName, hc);
}

int AbstractNormProbe::getValues(double timevalue, std::vector<double> * values) {
   if (values==NULL) { return PV_FAILURE; }
   int nBatch = getParent()->getNBatch();
   values->resize(nBatch); // Should we test if values->size()==nBatch before resizing?
   for (int b=0; b<nBatch; b++) {
      values->at(b) = getValueInternal(timevalue, b);
   }
   MPI_Allreduce(MPI_IN_PLACE, &values->front(), nBatch, MPI_DOUBLE, MPI_SUM, getParent()->icCommunicator()->communicator());
   return PV_SUCCESS;
}
   
double AbstractNormProbe::getValue(double timevalue, int index) {
   if (index>=0 && index < getParent()->getNBatch()) {
      double norm = getValueInternal(timevalue, index);
      MPI_Allreduce(MPI_IN_PLACE, &norm, 1, MPI_DOUBLE, MPI_SUM, getParent()->icCommunicator()->communicator());
      return norm;
   }
   else {
      return std::numeric_limits<double>::signaling_NaN();
   }
}

int AbstractNormProbe::outputState(double timevalue) {
   std::vector<double> values;
   getValues(timevalue, &values);
   assert(values.size()==getParent()->getNBatch());
   if (outputstream!=NULL) {
      int nBatch = getParent()->getNBatch();
      int nk = getTargetLayer()->getNumGlobalNeurons();
      for (int b=0; b<nBatch; b++) {
         fprintf(outputstream->fp, "%st = %6.3f b = %d numNeurons = %8d norm             = %f\n",
               getMessage(), timevalue, b, nk, values[b]);
      }
      fflush(outputstream->fp);
   }
   return PV_SUCCESS;
}

}  // end namespace PV
