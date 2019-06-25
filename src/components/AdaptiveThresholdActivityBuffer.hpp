/*
 * AdaptiveThresholdActivityBuffer.hpp
 *
 *  Created on: Jun 20, 2019
 *      Author: jspringer
 */

#ifndef ADAPTIVETHRESHOLDACTIVITYBUFFER_HPP_
#define ADAPTIVETHRESHOLDACTIVITYBUFFER_HPP_

#include "components/HyPerActivityBuffer.hpp"

namespace PV {

/**
 * A component to contain the internal state (membrane potential) of a HyPerLayer.
 */
class AdaptiveThresholdActivityBuffer : public HyPerActivityBuffer {
  protected:
   /**
    * List of parameters used by the AdaptiveThresholdActivityBuffer class
    * @name AdaptiveThresholdLCALayer Parameters
    * @{
    */

   virtual void ioParam_thresholdWindow(enum ParamsIOFlag ioFlag);
   virtual void ioParam_displayPeriod(enum ParamsIOFlag ioFlag);
   virtual void ioParam_simTimeOffset(enum ParamsIOFlag ioFlag);

   /** @} */
  public:
   AdaptiveThresholdActivityBuffer(char const *name, PVParams *params, Communicator const *comm);

   virtual ~AdaptiveThresholdActivityBuffer();
  protected:
   AdaptiveThresholdActivityBuffer() {}

   void initialize(char const *name, PVParams *params, Communicator const *comm);

   virtual void setObjectType() override;

   virtual int ioParamsFillGroup(enum ParamsIOFlag ioFlag) override;

   virtual Response::Status allocateDataStructures() override;

   Response::Status registerData(std::shared_ptr<RegisterDataMessage<Checkpointer> const> message) override;

   Response::Status readStateFromCheckpoint(Checkpointer *checkpointer) override;

   virtual void allocateThresholds();

   /**
    * Copies V to A, and then applies the parameters (either verticesV, verticesA, slopes; or
    * VThresh, AMax, AMin, AShift, VWidth) to the activity buffer.
    */
   virtual void updateBufferCPU(double simTime, double deltaTime) override;

   static void applyThresholds(
        int nbatch,
        int numNeurons,
        float const* V,
        float const* thresholds,
        float *A,
        int nx,
        int ny,
        int nf,
        int lt,
        int rt,
        int dn,
        int up);

   static void updateThresholds(
        int nbatch,
        int numNeurons,
        float const* V,
        float thresholdTimewindow,
        float *thresholds,
        float *A,
        int nx,
        int ny,
        int nf,
        int lt,
        int rt,
        int dn,
        int up,
        Communicator const* icComm,
        int numProcesses);

// TODO: Implement CUDA support for this Activity Buffer correctly
#ifdef PV_USE_CUDA
   virtual void allocateUpdateKernel() override;

   virtual Response::Status copyInitialStateToGPU() override;

   virtual void updateBufferGPU(double simTime, double deltaTime) override;

   //void runKernel();
#endif // PV_USE_CUDA

  protected:
    float* mThresholds                = nullptr;
    float mThresholdTimewindow        = FLT_MAX;
    double mDisplayPeriod             = DBL_MAX;
    double mSimTimeOffset             = 0.;
    int mNumProcesses                 = -1;

#ifdef PV_USE_CUDA
   PVCuda::CudaBuffer *mCudaThresholds = nullptr;
#endif // PV_USE_CUDA
};

class CheckpointEntryAdaptiveThresholds : public CheckpointEntry {
  public:
    CheckpointEntryAdaptiveThresholds(
            std::string const &name, 
            MPIBlock const *mpiBlock,
            float* thresholds,
            int numThresholds)
        : CheckpointEntry(name, mpiBlock), mThresholdsPtr(thresholds), mNumThresholds(numThresholds) {}
    CheckpointEntryAdaptiveThresholds(
            std::string const &objName,
            std::string const &dataName,
            MPIBlock const *mpiBlock,
            float* thresholds,
            int numThresholds)
        : CheckpointEntry(objName, dataName, mpiBlock), mThresholdsPtr(thresholds), mNumThresholds(numThresholds) {}
    virtual void write(std::string const &checkpointDirectory, double simTime, bool verifyWritesFlag) const override;
    virtual void read(std::string const &checkpointDirectory, double *simTimePtr) const override;
    virtual void remove(std::string const &checkpointDirectory) const override;

  private:
    float* mThresholdsPtr = nullptr;
    int mNumThresholds = 0;
};

} // namespace PV

#endif // ADAPTIVETHRESHOLDACTIVITYBUFFER_HPP
