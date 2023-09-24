// soyo -> esp direction

byte se[15]; // sync 0xA6

struct soyo_esp_data
{
  int   reqpower;
  byte  opmode;
  byte  errstat;
  float V_input;
  float A_input;
  float V_main;
  float net_HZ;
  float temp;
  //-----
  byte  dev_model;
  byte  dev_type;
  byte  startvolt;
  byte  stopvolt;
  int   bat_power;
  byte  delaysec;
};
soyo_esp_data sd;

bool batmode;
bool limit;

// esp -> soyo direction

// opmode 11-pvlimit, 01-pv, 12-batlimit, 02batconst
// msgid 0B-switchtomode, 03-readsettings, 01-info

// sync = 0x55
// msgtype 01=norm
// x x x

// 02 modeset
// x
// x
// mode 01=PV, 02=bat

// 0A batset 
// bat start volt in 0.1V
// bat stop volt in 0.1V ?

// 12 powerset
// batpow in 10W step

byte es[6]; 
byte lim[8];
