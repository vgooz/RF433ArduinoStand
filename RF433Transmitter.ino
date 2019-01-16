#include <VirtualWire.h>

#define pinLED  1
#define pinTX   3
#define repeatTX 33

struct package
{
  uint8_t sender;
  uint8_t session;
  uint8_t iteration;
  int16_t data1;
  int16_t data2;
  int16_t data3;
};

typedef struct package Package;
Package data;
uint8_t buflen = sizeof(data);
uint8_t buf[sizeof(data)];

void setup() {
  pinMode(pinLED, OUTPUT);
  pinMode(pinTX, OUTPUT);
  data.sender = 111;
  data.data1 = 0;
  data.data2 = 0;
  data.data3 = 0;
  vw_set_tx_pin(pinTX);
  vw_setup(2048);
  randomSeed(analogRead(0));
}

void loop() {
  digitalWrite(pinLED, HIGH);
  while (data.iteration < repeatTX)
  {
    CopyPackagetoBuffer(buf);
    vw_send((uint8_t *)&buf, buflen);
    vw_wait_tx();
    data.iteration++;
    data.data3++;
  }
  digitalWrite(pinLED, LOW);
  data.session++;
  data.iteration = 0;
  data.data1 = random(32767);
  data.data2 = random(32767);
  delay(5000);
}

void CopyPackagetoBuffer(uint8_t *arr)
{
  arr[0] = data.sender;
  arr[1] = data.session;
  arr[2] = data.iteration;
  arr[3] = data.data1 & 0xff;
  arr[4] = (data.data1 >> 8) & 0xff;
  arr[5] = data.data2 & 0xff;
  arr[6] = (data.data2 >> 8) & 0xff;
  arr[7] = data.data3 & 0xff;
  arr[8] = (data.data3 >> 8) & 0xff;
}
