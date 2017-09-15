#include <cmath>
#include <API.h>
#include "control/velPid.h"

namespace okapi {
  void VelPid::setGains(const float ikP, const float ikD) {
    kP = ikP;
    kD = ikD * sampleTime / 1000.0;
  }

  void VelPid::setSampleTime(const int isampleTime) {
    if (isampleTime > 0) {
      kD /= isampleTime / (float)sampleTime;
      sampleTime = isampleTime;
    }
  }

  void VelPid::setOutputLimits(float imax, float imin) {
    //Always use larger value as max
    if (imin > imax) {
      const float temp = imax;
      imax = imin;
      imin = temp;
    }

    outputMax = imax;
    outputMin = imin;

    //Fix output
    if (output > outputMax)
      output = outputMax;
    else if (output < outputMin)
      output = outputMin;
  }

  float VelPid::loopVel(const float inewReading) {
    return velMath.loop(inewReading);
  }

  float VelPid::loop(const float inewReading) {
    if (isOn) {
      const long now = millis();
      if (now - lastTime >= sampleTime) {
        const float error = target - inewReading;

        const float derivative = velMath.getDiff(); //Derivative over measurement to eliminate derivative kick on setpoint change

        output += kP * error - kD * derivative;

        if (output > outputMax)
          output = outputMax;
        else if (output < outputMin)
          output = outputMin;

        lastError = error;
        lastTime = now; //Important that we only assign lastTime if dt >= sampleTime
      }

      return output;
    }

    return 0;
  }

  void VelPid::reset() {
    error = 0;
    lastError = 0;
    output = 0;
  }
}
