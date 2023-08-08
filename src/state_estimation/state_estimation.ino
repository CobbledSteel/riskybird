#include "FastIMU.h"
#include "Madgwick.h"
#include "tof.h"
#include "uav_pid.h"

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Preferences.h>
#include <WiFi.h>





#define IMU_ADDRESS 0x68    //Change to the address of the IMU
#define PERFORM_CALIBRATION //Comment to disable startup calibration
#define SERIAL_BUFFER_LEN 256
// #define REAL



#define PIN 8
#define NUMPIXELS 2
#define BRIGHTNESS 20

const char* ssid = "SSID";
const char* password = "PASSWD";

char buf[SERIAL_BUFFER_LEN];

WiFiServer server(80);

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

Preferences preferences;

ToF tof;
MPU9250 IMU;               //Change to the name of any supported IMU!

// Currently supported IMUS: MPU9255 MPU9250 MPU6886 MPU6500 MPU6050 ICM20689 ICM20690 BMI055 BMX055 BMI160 LSM6DS3 LSM6DSL

calData calib = { 0 };  //Calibration data
AccelData IMUAccel;    //Sensor data
GyroData IMUGyro;
MagData IMUMag;
Madgwick filter;

UAV_PID pid;

hw_timer_t * timer = NULL;

uint32_t count;
unsigned long current_time;

struct Quaternion_t {
  float w, x, y, z;
};

struct Euler_t {
  float roll, pitch, yaw;
};

unsigned long old_isr_time;
unsigned long isr_time;

void IRAM_ATTR onTimer() {
  // Your interrupt handling code here
  // for(uint8_t i = 0; i < NUM_TOF; i++) {
  //   tof_data[i] = tof.tof_sensors[i].read();
  //   if (tof.tof_sensors[i].timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  // }
}

void parseSerialObservation(double* z, double* qw, double* qx, double* qy, double* qz, double* rollrate, double* pitchrate, double* yawrate) {
    char* idx;
    int newlinesCount = 0;  // Counter for newlines
    int i = 0;  // Index for inputString

    // Wait until three newlines are received or buffer is full
    while (newlinesCount < 3 && i < SERIAL_BUFFER_LEN - 1) {
        if (Serial.available()) {
            char inChar = (char)Serial.read();
            if (inChar == '\n') {
                newlinesCount++;
            }
            buf[i++] = inChar;
        }
    }
    buf[i] = '\0';
    idx = strstr(buf, "x:");
    double x,y;
    if (idx != NULL) {
        sscanf(idx, "x: [%lf %lf %lf]", &x, &y, z);
    }

    // Serial.print("x: ");
    // Serial.print(x);
    // Serial.print("\ty: ");
    // Serial.print(y);
    // Serial.print("\tz: ");
    // Serial.print(*z);

    idx = strstr(buf, "theta:");
    if (idx != NULL) {
        sscanf(idx, "theta: [%lf %lf %lf %lf]", qw, qx, qy, qz);
    }

    // Serial.print("\tqw: ");
    // Serial.print(*qw);
    // Serial.print("\tqx: ");
    // Serial.print(*qx);
    // Serial.print("\tqy: ");
    // Serial.print(*qy);
    // Serial.print("\tqz: ");
    // Serial.print(*qz);

    idx = strstr(buf, "omega:");
    if (idx != NULL) {
        sscanf(idx, "omega: [%lf %lf %lf]", rollrate, pitchrate, yawrate);
    }


    // Serial.print("\trollrate: ");
    // Serial.print(*rollrate);
    // Serial.print("\tpitchrate: ");
    // Serial.print(*pitchrate);
    // Serial.print("\tyawrate: ");
    // Serial.println(*yawrate);
}

void saveCalibrationData(calData& data) {
  Serial.println("Saving calibration preferences");
  Serial.print("Valid: ");
  Serial.println(data.valid);
	preferences.begin("riskybird", false);
	preferences.putBool("valid", data.valid);
	preferences.putBytes("accelBias", data.accelBias, sizeof(data.accelBias));
	preferences.putBytes("gyroBias", data.gyroBias, sizeof(data.gyroBias));
	preferences.putBytes("magBias", data.magBias, sizeof(data.magBias));
	preferences.putBytes("magScale", data.magScale, sizeof(data.magScale));
	preferences.end();
}

bool loadCalibrationData(calData& data) {
  Serial.println("Loading calibration preferences");
	preferences.begin("riskybird", true);
	data.valid = preferences.getBool("valid", false); // second parameter is default value
	if (!data.valid) {
		// data is not valid, don't load
    Serial.println("Invalid calibration.");
		preferences.end();
		return false;
	}
	preferences.getBytes("accelBias", data.accelBias, sizeof(data.accelBias));
	preferences.getBytes("gyroBias", data.gyroBias, sizeof(data.gyroBias));
	preferences.getBytes("magBias", data.magBias, sizeof(data.magBias));
	preferences.getBytes("magScale", data.magScale, sizeof(data.magScale));
	preferences.end();
	return true;
}

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

void init_imu() {
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
  if(!loadCalibrationData(calib)) {
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
    saveCalibrationData(calib);
  }
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

  #endif
  
  //delay(1000);
  IMU.init(calib, IMU_ADDRESS);
  IMU.setIMUGeometry(3);
}

void setup() {
  pixels.begin(); // Start NeoPixel LEDs
  setLED(0, 0, 0, 0);
  statusActive();
  delay(200);

  Wire.begin(6,7);
  Wire.setClock(400000); //400khz clo
  Serial.begin(115200);
  
  while (!Serial) {
    ;
  }
  #ifdef REAL
    Serial.println("Booted up!");
  #endif
  setLED(0, 0, 0, 0 );
  #ifdef REAL
    init_imu();
    filter.begin(2.0f);
    tof.init_tof();
  #endif
  // Initialize the hardware timer
  timer = timerBegin(0, 80, true); // 80 is the prescaler (with 80MHz CPU clock this makes it 1 MHz)
  
  // Attach the interrupt handler routine to the timer
  timerAttachInterrupt(timer, &onTimer, true);
  
  // Set the timer to trigger every 50 ms
  timerAlarmWrite(timer, 50000, true); // In microseconds

  // Enable the timer
  timerAlarmEnable(timer);
  pid.init();
  pid.updateSetpoints(0,0,0,0.5);
  statusReady();
}

void loop() {
  double z, qw, qx, qy, qz, rollrate, pitchrate, yawrate;
  double roll, pitch, yaw;
  #ifdef REAL
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
  
    Quaternion_t q = {filter.getQuatW(), filter.getQuatX(), filter.getQuatY(), filter.getQuatZ()};
    Euler_t e = quaternionToEuler(q);

    roll = e.roll;
    pitch = e.pitch;
    yaw = e.yaw;

    rollrate = IMUGyro.gyroX;
    pitchrate = IMUGyro.gyroY;
    yawrate = IMUGyro.gyroZ;

    float roll_in_degrees = degrees(e.roll);
    float pitch_in_degrees = degrees(e.pitch);
    float yaw_in_degrees = degrees(e.yaw);

    uint16_t tof_data[5];
    for(uint8_t i = 0; i < NUM_TOF; i++) {
      tof_data[i] = tof.tof_sensors[i].read(false);
      if (tof.tof_sensors[i].timeoutOccurred()) { Serial.print(" TIMEOUT"); }
    }
    z = tof_data[0] / 1000.0;
  #else
    parseSerialObservation(&z, &qw, &qx, &qy, &qz, &rollrate, &pitchrate, &yawrate);
    Quaternion_t q = {qw, qx, qy ,qz};
    Euler_t e = quaternionToEuler(q);

    roll = e.roll;
    pitch = e.pitch;
    yaw = e.yaw;
  #endif

  
  // if (count == 1000) {
  //   pid.updateSetpoints(0,0,yaw,0.5);
  // }
  pid.updateState(roll, pitch, yaw, z);
  pid.updateStateRate(rollrate, pitchrate, yawrate);
  pid.compute();

  #ifdef REAL
    if (count < 1200 && count >= 1000) {
      pid.setMotors();
    } else {
      pid.stopMotors();
    }

    if(count % 50 == 0) {
      unsigned long old_time = current_time;
      current_time = millis();

      Serial.print("Roll: ");
      Serial.print(roll_in_degrees);
      Serial.print("\tPitch: ");
      Serial.print(pitch_in_degrees);
      Serial.print("\tYaw: ");
      Serial.print(yaw_in_degrees);
      Serial.print("\t");

      Serial.print("dRoll: ");
      Serial.print(IMUGyro.gyroX);
      Serial.print("\tdPitch: ");
      Serial.print(IMUGyro.gyroY);
      Serial.print("\tdYaw: ");
      Serial.print(IMUGyro.gyroZ);
      Serial.print("\tToF:\t");
      

      for (uint8_t i = 0; i < NUM_TOF; i++) {
        Serial.print(tof_data[i]); 
        Serial.print('\t');
      }
      Serial.print("(");
      Serial.print(current_time - old_time);
      Serial.print("ms)");
      Serial.println();
      pid.printMotors();
      current_time = millis();
    }
  #else
    double m0, m1, m2, m3;
    pid.getMotors(&m0, &m1, &m2, &m3);
    // sprintf(buf, "motors: [%e, %e, %e, %e]\n", m1, m0, m3, m2);
    sprintf(buf, "motors: [%e, %e, %e, %e]\n", m0, m1, m2, m3);

    Serial.print(buf);
  #endif

  count++;
  //delay(50);
}
