// soyo -> esp direction
//----------------------
// A6 01 01 21 00 00 E3 00 3B 00 D7 64 01 D3 AF
// sy          op vv vv aa aa VV VV hz tt tt cc

// status data
// Byte Len  Payload                Content          
// 0     1   0xA6                   Header
// 1&2   2   0x00 0x84              Output Power         
// 3     1   0x91                   Operation mode (High nib), Frame function (Low nib)
// 4     1   0x40                   Error and status bitmask
// 5+6   2   0x01 0xC5              Battery voltage
// 7+6   2   0x00 0xDB              Battery current
// 9+10  2   0x00 0xF7              Grid voltage
// 11    1   0x63                   Grid frequency
// 12+13 2   0x02 0xBC              Temperature

// settings data
// Byte Len  Payload                Content       
// 0     1   0xA6                   Header
// 1     1   0x00                   Unknown
// 2     1   0x72
// 3     1   0x93                   Operation mode (High nib), Frame function (Low nib)
// 4     1   0x40                   Operation status bitmask
// 5     1   0xD4                   Device model
// 6     1   0x30                   Device type
// 7     1   0x2C                   Starting voltage  
// 8     1   0x2B                   Shutdown voltage
// 9     2   0x00 0xFA              Grid voltage  
// 11    1   0x64                   Grid frequency
// 12    1   0x5A                   Battery output power in 10W
// 13    1   0x03                   Delay in seconds

// read settings response: 53  10 B2 53 2 D4 30 2E 2C 0 DF 64 A 8

void copy_msg_soyo_esp()
{
  byte msgty = se[3] & 0x0F;
  if (msgty == 0x01)
  {
    sd.reqpower = (int)((se[1]<<8)+se[2]);
    sd.opmode   = se[3];
// high nibble  Limiter bit, Standby bit, PV mode bit, Battery mode bit
    sd.errstat  = se[4];
// 2=normal, 1=startip, 0=idle, 42=liniter+normal?
    sd.V_input  = 0.1 * (float)((se[5]<<8)+se[6]);
    sd.A_input  = 0.1 * (float)((se[7]<<8)+se[8]);
    sd.V_main   =       (float)((se[9]<<8)+se[10]);
    sd.net_HZ   = 0.5 * (float)se[11];
    sd.temp     = 0.1 * (float)((se[12]<<8)+se[13]) - 30.0;
  }
  else if (msgty == 0x03)
  {
    sd.dev_model = se[5]; //0xD4 = 212 -> 220V, 112 -> 110V, 210 -> 1000W
    sd.dev_type  = se[6]; //built for bat volt
    sd.startvolt = se[7];
    sd.stopvolt  = se[8];
    sd.bat_power = 10 * se[12];
    sd.delaysec  = se[13];   
  }
  else if (msgty == 0x00) // old 24V soyo
  {
    // 64 B7 60 2 0 0 0 0 0 DC 64 1 EB
    sd.reqpower = (int)((se[1]<<8)+se[2]);
    sd.opmode   = se[3];
// high nibble  Limiter bit, Standby bit, PV mode bit, Battery mode bit
    sd.errstat  = se[4];
    sd.V_input  = 0.1 * (float)((se[5]<<8)+se[6]);
    sd.A_input  = 0.1 * (float)((se[7]<<8)+se[8]);
    sd.V_main   =       (float)((se[9]<<8)+se[10]);
    sd.net_HZ   = 0.5 * (float)se[11];
    sd.temp     = 0.1 * (float)((se[12]<<8)+se[13]) - 30.0;  
  }
  else errmsgct++;
}

byte chksum_se(byte * data)
{
  byte cs = 0;
  for (int i=1;i<=13;i++) cs+=data[i];
  return 0xFF - cs;
}

// esp -> soyo direction
//----------------------
// idle msg 
// 55 01 00 00 00 FE
// sy id b1 b3
// sy id bp
// sy id xx xx bs
// b1 startvolt in 0.1V
// b2 stopvolt = 0.1V
// bp batpower in 10W
// bs 2=bat pow, 1= pv
// id 01=norm, 02 modeset, 03 limiter, 0A=batset, 12=powerset

byte chksum_es(byte * data)
{
  byte cs = 0;
  for (int i=1;i<=4;i++) cs+=data[i];
  return 0xFF - cs;
}

//55 01 00 00 00 FE idle msg back
//55 02 02 00 02 F9 battery set
//55 02 02 00 01 FC battery not set
//55 0A FE 00 00 F7 battery start volt 25.4 
//55 0A FD 03 00 F4 battery end volt 3 
//55 12 02 00 00 EB battery power 20W

// Limiter ?
// 55:03:17:18:02:CB # Constant power mode
// 55:03:23:64:01:74 # off
// 55:03:23:64:10:65 # Limiter mode

// bat max power
//Set Bat CP Mode Power to 40W  >>> 55:13:04:64:00:84
//Set Bat CP Mode Power to 600W >>> 55:13:3C:64:00:4C

void msg_esp_soyo(byte com)
{
  es[0]  = 0x55; // sync
  es[1]  = com;
  if (com == 0x01) //norm
  {
    es[2]  = 0x00;
    es[3]  = 0x00;
    es[4]  = 0x00;
  }
  else if (com == 0x02) // mode
  {
    es[2]  = 2;
    es[3]  = 0;
    if (batmode) es[4] = 2; // 01=PV 02=Bat
    else         es[4] = 1;
  }
  else if (com == 0x03) // mode
  {
    es[2]  = 0;
    es[3]  = 0;
    if (limit) es[4] = 0x10;
    //if (off) es[4] = 1; // 01=PV 02=Bat
    //if (constantpower) es[4] = 2; // 01=PV 02=Bat
    else 
    {
      if (batmode) es[4] = 2; // 01=PV 02=Bat
      else         es[4] = 1;
    }
  }
  else if (com == 0x0A) // bat
  {
    es[2]  = sd.startvolt;
    es[3]  = sd.stopvolt;
    es[4]  = 0;
  }
  else if (com == 0x12) // pow
  {
    es[2]  = sd.bat_power/10;
    es[3]  = 0;
    es[4]  = 0;
  }
  es[5] = chksum_es((byte*)&es);
}

byte chksum_lim(byte * data)
{
  byte cs = 0;
  for (int i=1;i<=7;i++) cs+=data[i];
  return 0xFF - cs;
}

void sendlimit(uint16_t pwrval)
{
  lim[0] = 36;
  lim[1] = 86;
  lim[2] = 0;
  lim[3] = 33;
  lim[4] = pwrval>>8;
  lim[5] = pwrval&0xFF;
  lim[6] = 128;
  lim[7] = chksum_lim((byte*)&lim);  
}
