#ifndef __uav_pid_h__
#define __uav_pid_h__

#include <stdint.h>
#include <PID_v1.h>

class UAV_PID {
  private:
    double Kp_roll, Kd_roll, Ki_roll;
    double Kp_pitch, Kd_pitch, Ki_pitch;
    double Kp_yaw, Kd_yaw, Ki_yaw;
    double Kp_alt, Kd_alt, Ki_alt;

    double set_roll, set_pitch, set_alt;
    double set_yaw_abs, set_yaw_err;
    double state_roll, state_pitch, state_yaw, state_alt;
    double out_roll, out_pitch, out_yaw, out_alt;

    double base_speed;

    double motor_speeds[4];
    uint8_t motor_pin[5] = {2, 3, 4, 5};

    PID * pidRoll;
    PID * pidPitch;
    PID * pidYaw;
    PID * pidAlt;


  public:
    UAV_PID(void);
    void init(void);
    void compute(void);
    void adjustAngleDifference(float new_angle);
    void updateSetpoints(double roll, double pitch, double yaw, double alt);
    void updateState(double roll, double pitch, double yaw, double alt);
    void setMotors(void);
    void printMotors(void);
    void stopMotors(void);

};

#endif