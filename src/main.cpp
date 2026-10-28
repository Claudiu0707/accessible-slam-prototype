#include <Arduino.h>
#include <Wire.h>

const uint8_t MPU_addr          = 0x68;
const uint8_t PWR_MGMT_ADDR     = 0x6B;
const uint8_t GYRO_CONFIG_ADDR  = 0x1B;
const uint8_t ACCEL_CONFIG_ADDR = 0x1C; 

const uint8_t A_XOUT_H = 0x3B;
const uint8_t A_XOUT_L = 0x3C;
const uint8_t A_YOUT_H = 0x3D;
const uint8_t A_YOUT_L = 0x3E;
const uint8_t A_ZOUT_H = 0x3F;
const uint8_t A_ZOUT_L = 0x40;

const uint8_t G_XOUT_H = 0x43;
const uint8_t G_XOUT_L = 0x44;
const uint8_t G_YOUT_H = 0x45;
const uint8_t G_YOUT_L = 0x46;
const uint8_t G_ZOUT_H = 0x47;
const uint8_t G_ZOUT_L = 0x48;

int8_t XAxisH = 0;
int8_t XAxisL = 0;
int8_t YAxisH = 0;
int8_t YAxisL = 0;
int8_t ZAxisH = 0;
int8_t ZAxisL = 0;

int16_t XAxisFull = 0;
int16_t YAxisFull = 0;
int16_t ZAxisFull = 0;

void setup() {
  Serial.begin(115200); 
  Wire.begin();

  Wire.beginTransmission(MPU_addr);
  Wire.write(PWR_MGMT_ADDR);
  Wire.write(0x00);             // Internal 20MHz clock used
  Wire.endTransmission(false);

  delay(50);

  Wire.beginTransmission(MPU_addr);
  Wire.write(GYRO_CONFIG_ADDR);
  Wire.write(0x10);             // 1000 dps used
  Wire.endTransmission(false);

  delay(50);

  Wire.beginTransmission(MPU_addr);
  Wire.write(ACCEL_CONFIG_ADDR); // accelerometer scale 2g
  Wire.write(0x00);

  Wire.endTransmission(true);
  delay(50);
}

void loop() {
  Serial.println("======== START PACKAGE ========");

  Wire.beginTransmission(MPU_addr);
  Wire.write(A_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 6, true);
  
  XAxisFull =  (Wire.read() << 8 | Wire.read());
  YAxisFull =  (Wire.read() << 8 | Wire.read());
  ZAxisFull =  (Wire.read() << 8 | Wire.read());

  Wire.endTransmission(false);

  float XAxisFinal = (float) XAxisFull / 16384.0;
  float YAxisFinal = (float) YAxisFull / 16384.0;
  float ZAxisFinal = (float) ZAxisFull / 16384.0;

  Serial.print("X Axis = "); Serial.print(XAxisFinal); Serial.println("g");
  Serial.print("Y Axis = "); Serial.print(YAxisFinal); Serial.println("g");
  Serial.print("Z Axis = "); Serial.print(ZAxisFinal); Serial.println("g");

  Wire.endTransmission(false);
  Wire.write(G_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 6, true);

  int16_t XGyroFull = Wire.read() << 8 | Wire.read();
  int16_t YGyroFull = Wire.read() << 8 | Wire.read();
  int16_t ZGyroFull = Wire.read() << 8 | Wire.read();

  Wire.endTransmission(false);

  float XGyroFinal = (float)XGyroFull/32.8;
  float YGyroFinal = (float)YGyroFull/32.8;
  float ZGyroFinal = (float)ZGyroFull/32.8;

  Serial.print("X Axis = "); Serial.print(XGyroFinal); Serial.println(" deg/s"); 
  Serial.print("Y Axis = "); Serial.print(YGyroFinal); Serial.println(" deg/s");
  Serial.print("Z Axis = "); Serial.print(ZGyroFinal); Serial.println(" deg/s");

  Serial.println("======== END PACKAGE ========\n\n");
  delay(500);
}
