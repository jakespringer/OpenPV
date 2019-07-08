/*
 * SleepActivityBuffer.hpp
 *
 * Created on: Jul 1, 2019
 *     Author: jspringer
 */

#ifndef SLEEPACTIVITYBUFFER_HPP_
#define SLEEPACTIVITYBUFFER_HPP_

#include "HyPerActivityBuffer.hpp"

namespace PV {

class SleepActivityBuffer : public HyPerActivityBuffer {
  protected:
    virtual void ioParam_sleepOffset(enum ParamsIOFlag ioFlag);
    virtual void ioParam_sleepDuration(enum ParamsIOFlag ioFlag);
    virtual void ioParam_sleepCycleLen(enum ParamsIOFlag ioFlag);
    virtual void ioParam_sleepValueA(enum ParamsIOFlag ioFlag);

  public:
    SleepActivityBuffer(char const *name, PVParams *params, Communicator const *comm);

    virtual ~SleepActivityBuffer();

    float getSleepOffset() const { return mSleepOffset; }
    float getSleepDuration() const { return mSleepDuration; }
    float getSleepCycleLen() const { return mSleepCycleLen; }
    float getSleepValueA() const { return mSleepValueA; }

  protected:
    SleepActivityBuffer() {}

    virtual int ioParamsFillGroup(enum ParamsIOFlag ioFlag) override;

    virtual void updateBufferCPU(double simTime, double deltaTime) override;

  protected:
    float mSleepOffset = 0;
    float mSleepDuration = 0;
    float mSleepCycleLen = 0;
    float mSleepValueA = 0;
};

}

#endif
