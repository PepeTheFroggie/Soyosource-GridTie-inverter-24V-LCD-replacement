// soyo -> esp direction

byte se[15]; // sync 0xA6

struct soyo_esp_data
{
  int   x_power;
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
  int   min_power = 50;
  int   target_power = 50;
  int   req_power = 0;
  int   manual_power = 0;
  byte  delaysec;
};
soyo_esp_data sd;

bool pvmode;
bool stby;
bool limit;
bool batcp;

// esp -> soyo direction

#define STATUS_COMMAND  0x01
#define SETTINGS_COMMAND 0x03
#define REBOOT_COMMAND 0x11
#define WRITE_SETTINGS_COMMAND 0x0B

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
