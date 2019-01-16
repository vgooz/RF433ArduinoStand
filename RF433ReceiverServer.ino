//P0(SDA), P2(SCL)

#include <VirtualWire.h>
#include <TinyWireS.h>

#define pinLED   1
#define pinRX    3
#define pinRXINT 4
#define repeatTX 33
#define I2C_ADDR 9

volatile unsigned long timeRXINT = 0;

uint8_t prevSender = 0;
uint8_t prevSession = 0;
uint8_t prevIteration = 0;
uint8_t pkgReceived = 0;
uint8_t pkgLost = 0;

struct packagerx
{
  uint8_t sender;
  uint8_t session;
  uint8_t iteration;
  int16_t data1;
  int16_t data2;
  int16_t data3;
};

struct packagei2c
{
  uint8_t sender;
  uint8_t session;
  uint8_t iteration;
  uint8_t received;
  uint8_t lost;
  int16_t data1;
  int16_t data2;
  int16_t data3;
};

typedef struct packagerx PackageRX;
PackageRX dataRX;

typedef struct packagei2c PackageI2C;
PackageI2C dataI2C;

uint8_t buflenRX = sizeof(dataRX);
uint8_t buf[sizeof(dataRX)];
uint8_t buflenI2C = sizeof(dataI2C);

void setup() {
  DDRB |= (1 << PB1);      //replaces pinMode(pinLED, OUTPUT);
  DDRB |= (1 << PB4);      //replaces pinMode(pinRXIN, OUTPUT);
  vw_set_rx_pin(pinRX);
  vw_setup(2048);
  vw_rx_start();
  TinyWireS.begin(I2C_ADDR);
  TinyWireS.onRequest(requestEvent);
  setup_watchdog(7);      //set watchdog time in 2sec
}

ISR(WDT_vect) {
  if (timeRXINT > 0 && millis() - timeRXINT > 2000)
  {
      PORTB &= ~(1 << PB1);    //replaces digitalWrite(pinLED, LOW);
      PORTB &= ~(1 << PB4);    //replaces digitalWrite(pinLED, LOW);   
      timeRXINT = 0;
  }
}

//****************************************************************
// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii)
{
 byte bb;
 int ww;
 if (ii > 9 ) ii=9;
 bb=ii & 7;
 if (ii > 7) bb|= (1<<5);
 bb|= (1<<WDCE);
 ww=bb;

 MCUSR &= ~(1<<WDRF);
 // start timed sequence
 WDTCR |= (1 << WDCE) | (1 << WDE);
 // set new watchdog timeout value
 WDTCR = bb;
 WDTCR |= _BV(WDIE);
}

void requestEvent()
{
  dataI2C.received = pkgReceived;
  dataI2C.lost = pkgLost;
  uint8_t buf[buflenI2C];
  CopyPackageI2CtoBuffer(buf);
  for (int i=0; i<buflenI2C; i++) TinyWireS.send(buf[i]);
}

void loop()
{
  vw_wait_rx();
  if (vw_get_message(buf, &buflenRX)) // Non-blocking
  {
    CopyBufferToPackageRX(buf);
    if (dataRX.sender != prevSender || dataRX.session != prevSession)
    {
      PORTB |= (1 << PB1);      //replaces digitalWrite(pinLED, HIGH);
      PORTB |= (1 << PB4);      //replaces digitalWrite(pinRXINT, HIGH);
      timeRXINT = millis();
      prevSender = dataRX.sender;
      prevSession = dataRX.session;
      prevIteration = 0;
      pkgReceived = 0;
      pkgLost = 0;
      dataI2C.sender = dataRX.sender;
      dataI2C.session = dataRX.session;
      dataI2C.iteration = dataRX.iteration;
      dataI2C.data1 = dataRX.data1;
      dataI2C.data2 = dataRX.data2;
      dataI2C.data3 = dataRX.data3;
    }
    pkgReceived++;
    if (prevIteration < dataRX.iteration && dataRX.iteration - prevIteration > 1) pkgLost = dataRX.iteration - prevIteration - 1;
    else if (prevIteration > dataRX.iteration && repeatTX - prevIteration + dataRX.iteration > 1) pkgLost = repeatTX - prevIteration + dataRX.iteration - 1;
    prevIteration = dataRX.iteration;
  }
}

void CopyBufferToPackageRX(uint8_t *arr)
{
  dataRX.sender = arr[0];   
  dataRX.session = arr[1];
  dataRX.iteration = arr[2];
  dataRX.data1 = arr[3] | arr[4] << 8;
  dataRX.data2 = arr[5] | arr[6] << 8;
  dataRX.data3 = arr[7] | arr[8] << 8;
}

void CopyPackageI2CtoBuffer(uint8_t *arr)
{
  arr[0] = dataI2C.sender;
  arr[1] = dataI2C.session;
  arr[2] = dataI2C.iteration;
  arr[3] = dataI2C.received;
  arr[4] = dataI2C.lost;
  arr[5] = dataI2C.data1 & 0xff;
  arr[6] = (dataI2C.data1 >> 8) & 0xff;
  arr[7] = dataI2C.data2 & 0xff;
  arr[8] = (dataI2C.data2 >> 8) & 0xff;
  arr[9] = dataI2C.data3 & 0xff;
  arr[10] = (dataI2C.data3 >> 8) & 0xff;
}
