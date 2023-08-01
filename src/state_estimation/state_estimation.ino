#include "FastIMU.h"
#include "Madgwick.h"
#include "tof.h"
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#define IMU_ADDRESS 0x68    //Change to the address of the IMU
#define PERFORM_CALIBRATION //Comment to disable startup calibration


#define PIN 8
#define NUMPIXELS 2
#define BRIGHTNESS 20

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

ToF tof;
MPU9250 IMU;               //Change to the name of any supported IMU!

// Currently supported IMUS: MPU9255 MPU9250 MPU6886 MPU6500 MPU6050 ICM20689 ICM20690 BMI055 BMX055 BMI160 LSM6DS3 LSM6DSL

calData calib = { 0 };  //Calibration data
AccelData IMUAccel;    //Sensor data
GyroData IMUGyro;
MagData IMUMag;
Madgwick filter;

struct Quaternion_t {
  float w, x, y, z;
};

struct Euler_t {
  float roll, pitch, yaw;
};

void setLED(int i, int r, int g, int b) {
  pixels.setPixelColor(i, pixels.Color(r,g,b));
  pixels.show();
}

void statusActive() {
  setLED(1, BRIGHTNESS*0.8, BRIGHTNESS*0.6, 0);
}

void statusError() {
  setLED(1, BRIGHTNESS, 0, 0);
}

void statusReady() {
  setLED(1, 0, BRIGHTNESS, 0);
}

Euler_t quaternionToEuler(Quaternion_t q) {
  Euler_t e;

  // Roll (x-axis rotation)
  float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
  float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
  e.roll = atan2(sinr_cosp, cosr_cosp);

  // Pitch (y-axis rotation)
  float sinp = 2 * (q.w * q.y - q.z * q.x);
  if (abs(sinp) >= 1)
    e.pitch = copysign(M_PI / 2, sinp); // Use 90 degrees if out of range
  else
    e.pitch = asin(sinp);

  // Yaw (z-axis rotation)
  float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
  float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
  e.yaw = atan2(siny_cosp, cosy_cosp);

  return e;
}


void setup() {
  pixels.begin(); // Start NeoPixel LEDs
  setLED(0, 0, 0, 0);
  statusActive();

  Wire.begin(6,7);
  Wire.setClock(400000); //400khz clo
  Serial.begin(115200);
  
  while (!Serial) {
    ;
  }
  Serial.println("Booted up!");

  
  int err = IMU.init(calib, IMU_ADDRESS);
  if (err != 0) {
    Serial.print("Error initializing IMU: ");
    Serial.println(err);
    statusError();
    while (true) {
      ;
    }
  }

  if (err != 0) {
    Serial.print("Error Setting range: ");
    Serial.println(err);
    statusError();
    while (true) {
      ;
    }
  }

#ifdef PERFORM_CALIBRATION
  Serial.println("FastIMU Calibrated Quaternion example");
  if (IMU.hasMagnetometer()) {
    delay(1000);
    setLED(0, BRIGHTNESS, 0 , BRIGHTNESS);
    Serial.println("Move IMU in figure 8 pattern until done.");
    delay(3000);
    IMU.calibrateMag(&calib);
    Serial.println("Magnetic calibration done!");
  }
  else {
    delay(1000);
  }
  Serial.println("Keep IMU level.");
  setLED(0, BRIGHTNESS, BRIGHTNESS , 0 );
  delay(5000);
  IMU.calibrateAccelGyro(&calib);
  Serial.println("Calibration done!");
  Serial.println("Accel biases X/Y/Z: ");
  Serial.print(calib.accelBias[0]);
  Serial.print(", ");
  Serial.print(calib.accelBias[1]);
  Serial.print(", ");
  Serial.println(calib.accelBias[2]);
  Serial.println("Gyro biases X/Y/Z: ");
  Serial.print(calib.gyroBias[0]);
  Serial.print(", ");
  Serial.print(calib.gyroBias[1]);
  Serial.print(", ");
  Serial.println(calib.gyroBias[2]);
  if (IMU.hasMagnetometer()) {
    Serial.println("Mag biases X/Y/Z: ");
    Serial.print(calib.magBias[0]);
    Serial.print(", ");
    Serial.print(calib.magBias[1]);
    Serial.print(", ");
    Serial.println(calib.magBias[2]);
    Serial.println("Mag Scale X/Y/Z: ");
    Serial.print(calib.magScale[0]);
    Serial.print(", ");
    Serial.print(calib.magScale[1]);
    Serial.print(", ");
    Serial.println(calib.magScale[2]);
  }
  setLED(0, 0, 0, 0 );
  delay(5000);
  IMU.init(calib, IMU_ADDRESS);
  IMU.setIMUGeometry(3);

  filter.begin(0.2f);
#endif
  tof.init_tof();
  statusReady();
}

void loop() {
  IMU.update();
  IMU.getAccel(&IMUAccel);
  IMU.getGyro(&IMUGyro);
  if (IMU.hasMagnetometer()) {
    IMU.getMag(&IMUMag);
    filter.update(IMUGyro.gyroX, IMUGyro.gyroY, IMUGyro.gyroZ, IMUAccel.accelX, IMUAccel.accelY, IMUAccel.accelZ, IMUMag.magX, IMUMag.magY, IMUMag.magZ);
  }
  else {
    filter.updateIMU(IMUGyro.gyroX, IMUGyro.gyroY, IMUGyro.gyroZ, IMUAccel.accelX, IMUAccel.accelY, IMUAccel.accelZ);
  }
  // Serial.print("QW: ");
  // Serial.print(filter.getQuatW());
  // Serial.print("\tQX: ");
  // Serial.print(filter.getQuatX());
  // Serial.print("\tQY: ");
  // Serial.print(filter.getQuatY());
  // Serial.print("\tQZ: ");
  // Serial.println(filter.getQuatZ());

  Quaternion_t q = {filter.getQuatW(), filter.getQuatX(), filter.getQuatY(), filter.getQuatZ()};
  Euler_t e = quaternionToEuler(q);

  float roll_in_degrees = degrees(e.roll);
  float pitch_in_degrees = degrees(e.pitch);
  float yaw_in_degrees = degrees(e.yaw);

  Serial.print("Roll: ");
  Serial.print(roll_in_degrees);
  Serial.print("\tPitch: ");
  Serial.print(pitch_in_degrees);
  Serial.print("\tYaw: ");
  Serial.println(yaw_in_degrees);

  for (uint8_t i = 0; i < NUM_TOF; i++)
  {
    Serial.print(tof.tof_sensors[i].read());
    if (tof.tof_sensors[i].timeoutOccurred()) { Serial.print(" TIMEOUT"); }
    Serial.print('\t');
  }
  Serial.println();
  delay(50);
}
