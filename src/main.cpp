#include <Arduino.h>
#include "crsf.h"

// set for XAIO C3
#define RXD2 20 // 16 esp32
#define TXD2 21 // 17 esp32
#define SBUS_BUFFER_SIZE 25
uint8_t _rcs_buf[25] {};
uint16_t _raw_rc_values[RC_INPUT_MAX_CHANNELS] {};
uint16_t _raw_rc_count{};

int aileronsPin = 2;
int elevatorPin = 3;
int throttlePin = 4;
int rudderPin = 5;

int aileronsPWMChannel = 1;
int elevatorPWMChannel = 2;
int throttlePWMChannel = 3;
int rudderPWMChannel = 4;


int curTicks = 0;
int curCnt = 0;

void SetServoPos(float percent, int pwmChannel)
{
    // 50 cycles per second 1,000ms / 50 = 100 /5 = 20ms per cycle
    // 1ms / 20ms = 1/20 duty cycle
    // 2ms / 20ms = 2/20 = 1/10 duty cycle
    // using 16 bit resolution for PWM signal convert to range of 0-65536 (0-100% duty/on time)
    // 1/20th of 65536 = 3276.8
    // 1/10th of 65536 = 6553.6

    uint32_t duty = map(percent, 0, 100, 3276.8, 6553.6);

    ledcWrite(pwmChannel, duty);
}

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  // Serial.begin(460800);  // production output speed if needed
  Serial.begin(115200); // Much slower than CRSF don't output in production!
  // ESP32 uses Serial2
  // ESP32 C3 uses Serial0
  // needs some conditional compile stuff for other boards
  Serial0.begin(420000, SERIAL_8N1, RXD2, TXD2);
  // Serial.println("Serial Txd is on pin: "+String(TX));
  // Serial.println("Serial Rxd is on pin: "+String(RX));
  
  ledcSetup(aileronsPWMChannel,50,16);
  ledcSetup(elevatorPWMChannel,50,16);
  ledcSetup(throttlePWMChannel,50,16);
  ledcSetup(rudderPWMChannel,50,16);

  ledcAttachPin(aileronsPin, aileronsPWMChannel);
  ledcAttachPin(elevatorPin, elevatorPWMChannel);
  ledcAttachPin(throttlePin, throttlePWMChannel);
  ledcAttachPin(rudderPin, rudderPWMChannel);
}

void loop() { //Choose Serial1 or Serial2 as required
  // verify we are looping, slowly!
  // curTicks += 1;
  // if(curTicks > 100000) {
  //   curTicks = 0;
  //   curCnt += 1;
  //   Serial.print("looping - ");
  //   Serial.println(curCnt);
  // }

  while (Serial0.available()) {
    // Serial.print("Serial data available");
    size_t numBytesRead = Serial0.readBytes(_rcs_buf, SBUS_BUFFER_SIZE);
    if(numBytesRead > 0)
    {
      crsf_parse(&_rcs_buf[0], SBUS_BUFFER_SIZE, &_raw_rc_values[0], &_raw_rc_count, RC_INPUT_MAX_CHANNELS );
      Serial.print("Channel 1: ");
      Serial.print(_raw_rc_values[0]);
      Serial.print("\tChannel 2: ");
      Serial.print(_raw_rc_values[1]);
      Serial.print("\tChannel 3: ");
      Serial.print(_raw_rc_values[2]);
      Serial.print("\tChannel 4: ");
      Serial.print(_raw_rc_values[3]);
      Serial.print("\tChannel 5: ");
      Serial.println(_raw_rc_values[4]);

      int aileronsMapped = map(_raw_rc_values[0], 1000, 2000, 0, 100);
      int elevatorMapped = map(_raw_rc_values[1], 1000, 2000, 0, 100);
      int throttleMapped = map(_raw_rc_values[2], 1000, 2000, 0, 100);
      int rudderMapped = map(_raw_rc_values[3], 1000, 2000, 0, 100);
      int switchMapped = map(_raw_rc_values[4], 1000, 2000, 0, 100);

      SetServoPos(aileronsMapped, aileronsPWMChannel);
      SetServoPos(elevatorMapped, elevatorPWMChannel);
      SetServoPos(throttleMapped, throttlePWMChannel);
      SetServoPos(rudderMapped, rudderPWMChannel);
    }
  }
}