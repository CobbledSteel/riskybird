#include "uav_pid.h"
#include <stdint.h>
#include <Arduino.h>



UAV_PID::UAV_PID(void) {

    base_speed = 0.6;

    Kp_roll = 0.0;
    Kd_roll = 0.0;
    Ki_roll = 0.00;

    Kp_pitch = 0.0;
    Kd_pitch = 0.0;
    Ki_pitch = 0.00;

    Kp_yaw = 0.1;
    Kd_yaw = 0.0;
    Ki_yaw = 0.0;

    Kp_alt = 0.3;
    Kd_alt = 0;
    Ki_alt = 0;
}

void UAV_PID::init() {
  pidRoll = new PID(&state_roll, &out_roll, &set_roll, Kp_roll, Ki_roll, Kd_roll, DIRECT);
  pidPitch = new PID(&state_pitch, &out_pitch, &set_pitch, Kp_pitch, Ki_pitch, Kd_pitch, DIRECT);
  pidYaw = new PID(&state_yaw, &out_yaw, &set_yaw_err, Kp_yaw, Ki_yaw, Kd_yaw, DIRECT);
  pidAlt = new PID(&state_alt, &out_alt, &set_alt, Kp_alt, Ki_alt, Kd_alt, DIRECT);

  pidRoll->SetMode(AUTOMATIC);
  pidPitch->SetMode(AUTOMATIC);
  pidYaw->SetMode(AUTOMATIC);
  pidAlt->SetMode(AUTOMATIC);

  pidRoll->SetOutputLimits(-100, 100);
  pidPitch->SetOutputLimits(-100, 100);
  pidYaw->SetOutputLimits(-100, 100);
  pidAlt->SetOutputLimits(-100, 100);


  for (int i = 0; i < 4; i++) {
    pinMode(motor_pin[i], OUTPUT);
    analogWrite(motor_pin[i], 0);
  }
}

void UAV_PID::adjustAngleDifference(float new_angle) {
    float difference = set_yaw_abs - new_angle;

    while (difference < -180) difference += 360;
    while (difference > 180) difference -= 360;

    state_yaw = difference;
}

void UAV_PID::compute(void) {
  pidRoll->Compute();
  pidPitch->Compute();
  pidYaw->Compute();
  pidAlt->Compute();

  motor_speeds[0] = base_speed + out_alt + out_pitch + out_roll + out_yaw;
  motor_speeds[1] = base_speed + out_alt - out_pitch + out_roll - out_yaw;
  motor_speeds[2] = base_speed + out_alt - out_pitch - out_roll + out_yaw;
  motor_speeds[3] = base_speed + out_alt + out_pitch - out_roll - out_yaw;

  for(uint8_t i = 0; i < 4; i++) {
    motor_speeds[i] = constrain(motor_speeds[i]*255, 0.0, 255.0);
  }
  
}

void UAV_PID::updateSetpoints(double roll, double pitch, double yaw, double alt) {
  set_roll = roll;
  set_pitch = pitch;
  set_yaw_abs = yaw;
  set_yaw_err = 0;
  set_alt = alt;
}

void UAV_PID::updateState(double roll, double pitch, double yaw, double alt) {
  state_roll = roll;
  state_pitch = pitch;
  adjustAngleDifference(yaw);
  state_alt = alt;
}

void UAV_PID::setMotors(void) {
  for (int i = 0; i < 4; i++) {
    analogWrite(motor_pin[i], motor_speeds[i]);
  }


}

void UAV_PID::printMotors(void) {
  Serial.print("PID: \tB: ");
  Serial.print(base_speed);
  Serial.print("\tR: ");
  Serial.print(out_roll);
  Serial.print("\tP: ");
  Serial.print(out_pitch);
  Serial.print("\tY: ");
  Serial.print(out_yaw);
  Serial.print("\t:A ");
  Serial.println(out_alt);
  base_speed + out_alt + out_pitch + out_roll + out_yaw;
  Serial.print("Front: \t");
  Serial.print(motor_speeds[3] / 255);
  Serial.print("\t");
  Serial.println(motor_speeds[0] / 255);
  Serial.print("Back: \t");
  Serial.print(motor_speeds[2] / 255);
  Serial.print("\t");
  Serial.println(motor_speeds[1] / 255);
}

void UAV_PID::stopMotors(void) {
  for (int i = 0; i < 4; i++) {
    analogWrite(motor_pin[i], 0);
  }
}