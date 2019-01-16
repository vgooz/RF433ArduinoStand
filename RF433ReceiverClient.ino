//A4(SDA), A5(SCL)
#include <Wire.h>

#define pinLED    13
#define pinRXINT  2
#define I2C_ADDR  9

struct package
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

typedef struct package Package;
Package data;

uint8_t buflen = sizeof(data);

unsigned long timeLED = 0;
volatile unsigned long timeRX = 0;

void setup() {
  pinMode(pinLED, OUTPUT);
  pinMode(pinRXINT, INPUT_PULLUP);
  Wire.begin();
  attachInterrupt(0, onPackageReceived, RISING);
  Serial.begin(9600);
}

void onPackageReceived() {
  timeRX = millis();
}

void loop()
{
  if (timeRX > 0 && millis() - timeRX > 3000)
  {
    timeRX = 0;
    Serial.println("Package Interrupt!");
    digitalWrite(pinLED, HIGH);
    timeLED = millis();

    Wire.beginTransmission(I2C_ADDR);
    if (Wire.endTransmission() == 0)
    {
      Wire.requestFrom(I2C_ADDR, buflen);      // Request N bytes from slave
      uint8_t bytes = Wire.available();
       
      if (bytes == buflen)
      {
        uint8_t buf[sizeof(data)];
        for (int i = 0; i < buflen; i++) buf[i] = Wire.read();
        CopyBufferToPackage(buf);
        Serial.print("Received package! buflen: ");
        Serial.print(buflen);
        Serial.print("; bytes: ");
        Serial.print(bytes);
        Serial.print("; Sender: ");
        Serial.print(data.sender);
        Serial.print("; Session: ");
        Serial.print(data.session);
        Serial.print("; Iteration: ");
        Serial.print(data.iteration);
        Serial.print("; Received: ");
        Serial.print(data.received);
        Serial.print("; Lost: ");
        Serial.print(data.lost);
        Serial.print("; Data1: ");
        Serial.print(data.data1);
        Serial.print("; Data2: ");
        Serial.print(data.data2);
        Serial.print("; Data3: ");
        Serial.print(data.data3);
        Serial.print("; Quality: ");
        if (data.received + data.lost > 0)
          Serial.print(((float)data.received / (float)(data.received + data.lost)) * 100.0);
        else
          Serial.print("0.00");
        Serial.println("%");
      }
      else
      {
        Serial.print("Package data is corrupted! Byte received: ");
        Serial.println(bytes);
      }
    }
    else
    {
      Serial.println("No Slave Device found!");
    }
  }
  if (timeLED > 0 && millis() - timeLED > 500)
  {
    digitalWrite(pinLED, LOW);
    timeLED = 0;
  }
}

void CopyBufferToPackage(uint8_t *arr)
{
  data.sender = arr[0];   
  data.session = arr[1];
  data.iteration = arr[2];
  data.received = arr[3];
  data.lost = arr[4];
  data.data1 = arr[5] | arr[6] << 8;
  data.data2 = arr[7] | arr[8] << 8;
  data.data3 = arr[9] | arr[10] << 8;
}
