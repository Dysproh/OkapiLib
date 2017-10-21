#include "chassis/chassisController.h"
#include "util/timer.h"
#include <cmath>

namespace okapi {
  void ChassisControllerPid::driveStraight(const int itarget) {
    const auto encStartVals = model->getEncoderVals();
    float distanceElapsed = 0, angleChange = 0, lastDistance = 0;
    unsigned long prevWakeTime = millis();

    distancePid.reset();
    anglePid.reset();
    distancePid.setTarget(itarget);
    anglePid.setTarget(0);

    bool atTarget = false;
    const int atTargetDistance = 15;
    const int threshold = 2;

    Timer atTargetTimer;

    const int timeoutPeriod = 250;

    std::valarray<int> encVals{0, 0};
    float distOutput, angleOutput;

    while (!atTarget) {
      encVals = model->getEncoderVals() - encStartVals;
      distanceElapsed = (encVals[0] + encVals[1]) / 2.0;
      angleChange = encVals[0] - encVals[1];

      distOutput = distancePid.loop(distanceElapsed);
      angleOutput = anglePid.loop(angleChange);
      model->driveVector(distOutput, angleOutput);

      if (std::abs(itarget - distanceElapsed) <= atTargetDistance)
        atTargetTimer.placeHardMark();
      else if (std::abs(distanceElapsed - lastDistance) <= threshold)
        atTargetTimer.placeHardMark();
      else
        atTargetTimer.clearHardMark();

      lastDistance = distanceElapsed;

      if (atTargetTimer.getDtFromHardMark() >= timeoutPeriod)
        atTarget = true;

      taskDelayUntil(&prevWakeTime, 15);
    }

    model->driveForward(0);
  }

  void ChassisControllerPid::pointTurn(float idegTarget) {
    const auto encStartVals = model->getEncoderVals();
    float angleChange = 0, lastAngle = 0;
    unsigned long prevWakeTime = millis();

    while (idegTarget > 180)
      idegTarget -= 360;
    while (idegTarget <= -180)
      idegTarget += 360;

    anglePid.reset();
    anglePid.setTarget(idegTarget);

    bool atTarget = false;
    const int atTargetAngle = 10;
    const int threshold = 2;

    Timer atTargetTimer;

    const int timeoutPeriod = 250;

    std::valarray<int> encVals{0, 0};

    while (!atTarget) {
      encVals = model->getEncoderVals() - encStartVals;
      angleChange = encVals[1] - encVals[0];

      model->turnClockwise(anglePid.loop(angleChange));

      if (std::abs(idegTarget - angleChange) <= atTargetAngle)
        atTargetTimer.placeHardMark();
      else if (std::abs(angleChange - lastAngle) <= threshold)
        atTargetTimer.placeHardMark();
      else
        atTargetTimer.clearHardMark();

      lastAngle = angleChange;

      if (atTargetTimer.getDtFromHardMark() >= timeoutPeriod)
        atTarget = true;

      taskDelayUntil(&prevWakeTime, 15);
    }

    model->driveForward(0);
  }

  void ChassisControllerMP::driveStraight(const int itarget) {
    controller.setTarget(itarget);

    const auto encStartVals = model->getEncoderVals();
    float distanceElapsed = 0;
    unsigned long prevWakeTime = millis();

    std::valarray<int> encVals{0, 0};

    while (!controller.isComplete()) {
      encVals = model->getEncoderVals() - encStartVals;
      distanceElapsed = (encVals[0] + encVals[1]) / 2.0;

      model->driveForward(controller.loop(distanceElapsed));

      taskDelayUntil(&prevWakeTime, 15);
    }

    model->driveForward(0);
  }

  void ChassisControllerMP::pointTurn(float idegTarget) {
    controller.setTarget(idegTarget); //TODO: Might not be able to use the same params for turning

    const auto encStartVals = model->getEncoderVals();
    float angleChange = 0;
    unsigned long prevWakeTime = millis();

    std::valarray<int> encVals{0, 0};

    while (!controller.isComplete()) {
      encVals = model->getEncoderVals() - encStartVals;
      angleChange = encVals[1] - encVals[0];

      model->turnClockwise(controller.loop(angleChange));

      taskDelayUntil(&prevWakeTime, 15);
    }

    model->driveForward(0);
  }
}
