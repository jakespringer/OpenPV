/*
 * Arguments.cpp
 *
 *  Created on: Sep 21, 2015
 *      Author: pschultz
 */

#include <cstdlib>
#ifdef PV_USE_OPENMP_THREADS
#include <omp.h>
#endif
#include "Arguments.hpp"
#include "io/io.hpp"
#include "utils/PVAssert.hpp"
#include "utils/PVLog.hpp"
#include <istream>

namespace PV {

Arguments::Arguments(std::istream &configStream, bool allowUnrecognizedArguments) {
   initialize_base();
   initialize(configStream, allowUnrecognizedArguments);
}

int Arguments::initialize_base() {
   clearState();
   return PV_SUCCESS;
}

int Arguments::initialize(std::istream &configStream, bool allowUnrecognizedArguments) {
   resetState(configStream, allowUnrecognizedArguments);
   return PV_SUCCESS;
}

bool Arguments::setRequireReturnFlag(bool val) {
   mRequireReturnFlag = val;
   return mRequireReturnFlag;
}
void Arguments::setOutputPath(char const *val) {
   mOutputPath = val ? val : "";
}
void Arguments::setParamsFile(char const *val) {
   mParamsFile = val ? val : "";
}
void Arguments::setLogFile(char const *val) {
   mLogFile = val ? val : "";
}
void Arguments::setGPUDevices(char const *val) {
   mGpuDevices = val ? val : "";
}
unsigned int Arguments::setRandomSeed(unsigned int val) {
   mRandomSeed = val;
   return mRandomSeed;
}
void Arguments::setWorkingDir(char const *val) {
   mWorkingDir = val ? val : "";
}
bool Arguments::setRestartFlag(bool val) {
   mRestartFlag = val;
   return mRestartFlag;
}
void Arguments::setCheckpointReadDir(char const *val) {
   mCheckpointReadDir = val ? val : "";
}
bool Arguments::setUseDefaultNumThreads(bool val) {
   mUseDefaultNumThreads = val;
   if (val) {
      mNumThreads = -1;
   }
   return mUseDefaultNumThreads;
}
int Arguments::setNumThreads(int val) {
   mNumThreads           = val;
   mUseDefaultNumThreads = false;
   return mNumThreads;
}
int Arguments::setNumRows(int val) {
   mNumRows = val;
   return mNumRows;
}
int Arguments::setNumColumns(int val) {
   mNumColumns = val;
   return mNumColumns;
}
int Arguments::setBatchWidth(int val) {
   mBatchWidth = val;
   return mBatchWidth;
}
bool Arguments::setDryRunFlag(bool val) {
   mDryRunFlag = val;
   return mDryRunFlag;
}

void Arguments::resetState(std::istream &configStream, bool allowUnrecognizedArguments) {
   delete mConfigFromStream;
   mConfigFromStream = new ConfigParser(configStream, allowUnrecognizedArguments);
   resetState();
}

void Arguments::resetState() {
   mRequireReturnFlag = mConfigFromStream->getRequireReturn();
   mRestartFlag = mConfigFromStream->getRestart();
   mDryRunFlag = mConfigFromStream->getDryRun();
   mRandomSeed = mConfigFromStream->getRandomSeed();
   mNumThreads = mConfigFromStream->getNumThreads();
   mUseDefaultNumThreads = mConfigFromStream->getUseDefaultNumThreads();
   mNumRows = mConfigFromStream->getNumRows();
   mNumColumns = mConfigFromStream->getNumColumns();
   mBatchWidth = mConfigFromStream->getBatchWidth();
   mOutputPath = mConfigFromStream->getOutputPath();
   mParamsFile = mConfigFromStream->getParamsFile();
   mLogFile = mConfigFromStream->getLogFile();
   mGpuDevices = mConfigFromStream->getGpuDevices();
   mWorkingDir = mConfigFromStream->getWorkingDir();
   mCheckpointReadDir = mConfigFromStream->getCheckpointReadDir();
}

void Arguments::clearState() {
   mRequireReturnFlag = false;
   mRestartFlag = false;
   mDryRunFlag = false;
   mRandomSeed = 0U;
   mNumThreads = 0;
   mUseDefaultNumThreads = false;
   mNumRows = 0;
   mNumColumns = 0;
   mBatchWidth = 0;
   mOutputPath.clear();
   mParamsFile.clear();
   mLogFile.clear();
   mGpuDevices.clear();
   mWorkingDir.clear();
   mCheckpointReadDir.clear();
}

int Arguments::printState() const {
   if (mRequireReturnFlag) {
      InfoLog().printf("RequireReturn:true\n");
   }
   if (!mOutputPath.empty()) {
      InfoLog().printf("OutputPath:%s\n", mOutputPath.c_str());
   }
   if (!mParamsFile.empty()) {
      InfoLog().printf("ParamsFile:%s\n", mParamsFile.c_str());
   }
   if (!mLogFile.empty()) {
      InfoLog().printf("LogFile:%s\n", mLogFile.c_str());
   }
   if (!mGpuDevices.empty()) {
      InfoLog().printf("GPUDevices:%s\n", mGpuDevices.c_str());
   }
   if (mRandomSeed) {
      InfoLog().printf("RandomSeed:%u\n", mRandomSeed);
   }
   if (!mWorkingDir.empty()) {
      InfoLog().printf("WorkingDirectory:%s\n", mWorkingDir.c_str());
   }
   pvAssert(!mRestartFlag || mCheckpointReadDir.empty());
   if (mRestartFlag) {
      InfoLog().printf("Restart:true\n");
   }
   if (!mCheckpointReadDir.empty()) {
      InfoLog().printf("CheckpointReadDirectory:%s\n", mCheckpointReadDir.c_str());
   }
   if (mUseDefaultNumThreads) {
      InfoLog().printf("NumThreads:-\n");
   }
   else if (mNumThreads >= 0) {
      InfoLog().printf("NumThreads:%d\n", mNumThreads);
   }
   if (mNumRows) {
      InfoLog().printf("NumRows:%d\n", mNumRows);
   }
   if (mNumColumns) {
      InfoLog().printf("NumColumns:%d\n", mNumColumns);
   }
   if (mBatchWidth) {
      InfoLog().printf("BatchWidth:%d\n", mBatchWidth);
   }
   if (mDryRunFlag) {
      InfoLog().printf("DryRun:true\n");
   }
   return PV_SUCCESS;
}

Arguments::~Arguments() {
}

} /* namespace PV */
