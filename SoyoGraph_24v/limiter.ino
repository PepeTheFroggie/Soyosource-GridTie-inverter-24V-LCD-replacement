#include <SoftwareSerial.h>

#define txpin 12// gpio12
SoftwareSerial mySerial(-1, txpin); // RX, TX

byte lim[8];

void limiterInit()
{
  pinMode(txpin, OUTPUT);
  mySerial.enableRx(false);
  mySerial.begin(4800); 
  mySerial.println("Hi ");     
}

byte chksum_lim(byte * data)
{
  byte cs = 0;
  for (int i=1;i<7;i++) cs+=data[i];
  return 0xFF - cs;
}

void SendLimit(int pwrval)
{
  uint16_t pwr = constrain(pwrval,0,600);
  lim[0] = 36;
  lim[1] = 86;
  lim[2] = 0;
  lim[3] = 33;
  lim[4] = pwr>>8;
  lim[5] = pwr&0xFF;
  lim[6] = 128;
  lim[7] = chksum_lim(lim);  
  mySerial.write(lim,8);
}
