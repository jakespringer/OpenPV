#include "AdaptiveThresholdActivityBuffer.hpp"
#include <algorithm>

namespace PV {

AdaptiveThresholdActivityBuffer::AdaptiveThresholdActivityBuffer(char const *name, PVParams *params, Communicator const *comm) {
  initialize(name, params, comm);
}

AdaptiveThresholdActivityBuffer::~AdaptiveThresholdActivityBuffer() {
  free(mThresholds);
}

void AdaptiveThresholdActivityBuffer::initialize(char const *name, PVParams *params, Communicator const *comm) {
  HyPerActivityBuffer::initialize(name, params, comm);
}

void AdaptiveThresholdActivityBuffer::setObjectType() { mObjectType = "AdaptiveThresholdActivityBuffer"; }

int AdaptiveThresholdActivityBuffer::ioParamsFillGroup(enum ParamsIOFlag ioFlag) {
  int status = HyPerActivityBuffer::ioParamsFillGroup(ioFlag);

  ioParam_thresholdWindow(ioFlag);
  ioParam_displayPeriod(ioFlag);
  ioParam_simTimeOffset(ioFlag);
  ioParam_AMax(ioFlag);

  return status;
}

void AdaptiveThresholdActivityBuffer::ioParam_thresholdWindow(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(
         ioFlag,
         name,
         "thresholdTimewindow",
         &mThresholdTimewindow,
         mThresholdTimewindow /*default*/,
         true /*warnIfAbsent*/);
}

void AdaptiveThresholdActivityBuffer::ioParam_displayPeriod(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(
         ioFlag,
         name,
         "displayPeriod",
         &mDisplayPeriod,
         mDisplayPeriod /*default*/,
         true /*warnIfAbsent*/);
}

void AdaptiveThresholdActivityBuffer::ioParam_simTimeOffset(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(
         ioFlag,
         name,
         "simTimeOffset",
         &mSimTimeOffset,
         mSimTimeOffset /*default*/,
         true /*warnIfAbsent*/);
}

void AdaptiveThresholdActivityBuffer::ioParam_AMax(enum ParamsIOFlag ioFlag) {
   parameters()->ioParamValue(
         ioFlag,
         name,
         "AMax",
         &mAMax,
         mAMax /*default*/,
         false /*warnIfAbsent*/);
}

Response::Status AdaptiveThresholdActivityBuffer::allocateDataStructures() {
  allocateThresholds();

  MPI_Comm_size(MPI_COMM_WORLD, &mNumProcesses);
  
  auto status = HyPerActivityBuffer::allocateDataStructures();
  if (!Response::completed(status)) {
      return status;
  }

  return Response::SUCCESS;
}

Response::Status
AdaptiveThresholdActivityBuffer::registerData(std::shared_ptr<RegisterDataMessage<Checkpointer> const> message) {
   auto status = HyPerActivityBuffer::registerData(message);
   if (!Response::completed(status)) {
      return status;
   }
   int numNeurons        = mInternalState->getBufferSize();
//   if (mCheckpointFlag and !mBufferLabel.empty()) {
      auto *checkpointer   = message->mDataRegistry;
      auto checkpointEntry = std::make_shared<CheckpointEntryAdaptiveThresholds>(
         getName(),
         "T",
         getMPIBlock(),
         mThresholds,
         numNeurons);
      bool registerSucceeded =
            checkpointer->registerCheckpointEntry(checkpointEntry, false /*not constant*/);
      FatalIf(
            !registerSucceeded,
            "%s failed to register %s for checkpointing.\n",
            getDescription_c(),
            "T");
//   }
   return Response::SUCCESS;
}

Response::Status AdaptiveThresholdActivityBuffer::readStateFromCheckpoint(Checkpointer *checkpointer) {
   Response::Status status = HyPerActivityBuffer::readStateFromCheckpoint(checkpointer);
   if (!Response::completed(status)) {
      return status;
   }
//   if (mCheckpointFlag and !mBufferLabel.empty()) {
      checkpointer->readNamedCheckpointEntry(std::string(name), "T", false);
//   }
   return Response::SUCCESS;
}

void AdaptiveThresholdActivityBuffer::updateBufferCPU(double simTime, double deltaTime) {
  const PVLayerLoc *loc = getLayerLoc();
  float *A              = getReadWritePointer();
  float const *V        = mInternalState->getBufferData();
  int numNeurons        = mInternalState->getBufferSize();
  int nbatch            = loc->nbatch;

  HyPerActivityBuffer::updateBufferCPU(simTime, deltaTime);

  applyThresholds(
          nbatch,
          numNeurons,
          V,
          mThresholds,
          A,
          loc->nx,
          loc->ny,
          loc->nf,
          loc->halo.lt,
          loc->halo.rt,
          loc->halo.dn,
          loc->halo.up,
          mAMax);

  if (fmod(simTime + mSimTimeOffset, mDisplayPeriod) < deltaTime / 2) {
    updateThresholds(
            nbatch,
            numNeurons,
            V,
            mThresholdTimewindow,
            mThresholds,
            A,
            loc->nx,
            loc->ny,
            loc->nf,
            loc->halo.lt,
            loc->halo.rt,
            loc->halo.dn,
            loc->halo.up,
            mCommunicator,
            mNumProcesses);
  }
}

void AdaptiveThresholdActivityBuffer::applyThresholds(
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
        int up,
        float AMax) {
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
  for (int kbatch = 0; kbatch < numNeurons * nbatch; kbatch++) {
    int b = kbatch / numNeurons;
    int k = kbatch % numNeurons;
    float const *VBatch = V + b * numNeurons;
    float *ABatch = A + b * (nx + lt + rt) * (ny + up + dn) * nf;
    int kex = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
    if (VBatch[k] < std::max(thresholds[k], 0.0f)) {
      ABatch[kex] = 0;
    } else if (VBatch[k] > AMax) {
      ABatch[kex] = AMax;
    }
  }
}

void AdaptiveThresholdActivityBuffer::updateThresholds(
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
        int numProcesses) {

  // NOTE: cannot be parallel because of race condition
  for (int k=0; k < numNeurons; ++k) {
    float aAcc = 0.0f;
    for (int b=0; b < nbatch; ++b) {
      float const *VBatch = V + b * numNeurons;
      float *ABatch = A + b * (nx + lt + rt) * (ny + up + dn) * nf;
      int kex = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
      aAcc += ABatch[kex];
    }
    thresholds[k] = thresholds[k] * (1.f - thresholdTimewindow) + (aAcc * thresholdTimewindow / ((float) nbatch));
  }

//  // TESTING CODE
//  for (int k = 0; k < numNeurons; k++) {
//    thresholds[k] = 1.0;
//  }

#ifdef PV_USE_MPI
  MPI_Comm comm = icComm->globalCommunicator();
  MPI_Allreduce(MPI_IN_PLACE, thresholds, numNeurons, MPI_FLOAT, MPI_SUM, comm);

  for (int i=0; i<numNeurons; ++i) {
    thresholds[i] /= numProcesses;
  }
#endif
}

#ifdef PV_USE_CUDA
void AdaptiveThresholdActivityBuffer::allocateUpdateKernel() {
//    HyPerActivityBuffer::allocateUpdateKernel();
//  PVCuda::CudaDevice *device = mCudaDevice;
//  
//  int const numNeuronsAcrossBatch = mInternalState->getBufferSizeAcrossBatch();
//  std::size_t size = (std::size_t) numNeuronsAcrossBatch * sizeof(*mThresholds);
//  mCudaThresholds = device->createBuffer(size, &getDescription());
}

Response::Status AdaptiveThresholdActivityBuffer::copyInitialStateToGPU() {
//  Response::Status status = HyPerActivityBuffer::copyInitialStateToGPU();
//  if (!Response::completed(status)) {
//    return status;
//  }
//  if (!isUsingGPU()) {
//    return status;
//  }
//
//  mCudaThresholds->copyToDevice(mThresholds);
//
  return Response::SUCCESS;
}

void AdaptiveThresholdActivityBuffer::updateBufferGPU(double simTime, double deltaTime) {
//  pvAssert(isUsingGPU());
//  FatalIf(
//          !mInternalState->isUsingGPU(),
//          getDescription_c(),
//          mIntneralState->getDescription_c());
//
//  runKernel();
//  HyPerActivityBuffer::updateBufferGPU(simTime, deltaTime); 
//  updateBufferCPU(simTime, deltaTime);
}


#endif // PV_USE_CUDA

void AdaptiveThresholdActivityBuffer::allocateThresholds() {
  const PVLayerLoc *loc = getLayerLoc();
  int numNeurons = loc->nx * loc->ny * loc->nf;
  std::size_t size = (std::size_t) numNeurons * sizeof(*mThresholds);
  mThresholds = (float *) malloc(size);
  for (int i=0; i<numNeurons; ++i) {
    mThresholds[i] = 0.5f;
  }
}

void CheckpointEntryAdaptiveThresholds::write(
        std::string const &checkpointDirectory, 
        double simTime, 
        bool verifyWritesFlag) const {
  if (getMPIBlock()->getRank() == 0) {
    std::string path = generatePath(checkpointDirectory, "bin");
    FileStream fileStream{path.c_str(), std::ios_base::out, verifyWritesFlag};
    fileStream.write(mThresholdsPtr, sizeof(*mThresholdsPtr) * mNumThresholds);
    
    path = generatePath(checkpointDirectory, "txt");
    FileStream txtFileStream{path.c_str(), std::ios_base::out, verifyWritesFlag};
    for (std::size_t i=0; i<mNumThresholds; ++i) {
      txtFileStream << mThresholdsPtr[i] << "\n";
    }
  }
}

void CheckpointEntryAdaptiveThresholds::read(
        std::string const &checkpointDirectory, 
        double *simTimePtr) const {
  if (getMPIBlock()->getRank() == 0) {
    std::string path = generatePath(checkpointDirectory, "bin");
    FileStream fileStream{path.c_str(), std::ios_base::in, false};
    fileStream.read(mThresholdsPtr, sizeof(*mThresholdsPtr) * mNumThresholds);
  }

  MPI_Bcast(mThresholdsPtr, mNumThresholds, MPI_FLOAT, 0, getMPIBlock()->getComm());
}

void CheckpointEntryAdaptiveThresholds::remove(std::string const &checkpointDirectory) const {
  deleteFile(checkpointDirectory, "bin");
  deleteFile(checkpointDirectory, "txt");
}

}
