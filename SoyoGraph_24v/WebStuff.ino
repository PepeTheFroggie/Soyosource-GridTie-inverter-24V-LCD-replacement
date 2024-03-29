
void handleNotFound() 
{
  server.send(404, "text/plain", "Not Here");
}

void handleSettings() 
{
  String out;
  char temp[400];
  
  for (uint8_t i = 0; i < server.args(); i++)
  {
    if      (server.argName(i) == "rd")  IsRead = true;  
    else if (server.argName(i) == "BatStartV") {sd.startvolt=server.arg(i).toInt(); IsBat=true;} 
    else if (server.argName(i) == "BatStopV")  {sd.stopvolt =server.arg(i).toInt(); IsBat=true;}
    else if (server.argName(i) == "BatPowerW") {sd.bat_power=server.arg(i).toInt(); IsPow=true;}
    else if (server.argName(i) == "pv")  pvmode = true;
    else if (server.argName(i) == "bat") pvmode = false;
    else if (server.argName(i) == "bcp") batcp  = true;
    else if (server.argName(i) == "ncp") batcp  = false;
    else if (server.argName(i) == "lim") limit  = true; 
    else if (server.argName(i) == "nli") limit  = false; 
    //constpower
  }

  out += "<body><center>";
            
  out += "<h1>Soyo Settings</h1>";         
  out += "<br>\n";
  
  sprintf(temp,"devmodel %d &nbsp\n",sd.dev_model);
  out += temp;
  sprintf(temp,"devbatvolt %d <br>\n",sd.dev_type);
  out += temp;
  if (batcp)  out += "BatCP &nbsp\n";
  if (pvmode) out += "PV &nbsp\n";
  if (limit) out += "limit &nbsp\n";
  out += "<br>\n";
    
  out += "<form action=\"/settings\">\n";
  out += "Bat restart Volt : ";
  sprintf(temp,"<input type=\"number\" name=\"BatStartV\" value=%d>&nbsp\n",sd.startvolt);
  out += temp;
  out += "Bat stop Volt : ";
  sprintf(temp,"<input type=\"number\" name=\"BatStopV\" value=%d>&nbsp\n",sd.stopvolt);
  out += temp;
  out += "<input type=\"submit\" value=\"Write Bat Volt\"><br>\n";
  out += "</form>\n";
  out += "<form action=\"/settings\">\n";
  out += "Bat Power Watt: ";
  sprintf(temp,"<input type=\"number\" name=\"BatPowerW\" step=\"10\" value=%d>&nbsp\n",sd.bat_power);
  out += temp;
  out += "<input type=\"submit\" value=\"Write Power\"><br>\n";
  out += "</form>\n";
  sprintf(temp,"startdelay %d <br>\n",sd.delaysec);
  out += temp;
  out += "<br>\n";

  out += "<a href='/settings?bat=1'>batmode</a>";
  out += "&nbsp\n";
  out += "<a href='/settings?pv=1'>pvmode</a>";
  out += "<br>\n";
  out += "<a href='/settings?ncp=1'>noCP</a>";
  out += "&nbsp\n";
  out += "<a href='/settings?bcp=1'>batCP</a>";
  out += "<br>\n";
  out += "<a href='/settings?nli=1'>nolimit</a>";
  out += "&nbsp\n";
  out += "<a href='/settings?lim=1'>limit</a>";
  out += "<br><br>\n";
  
  out += "<button onclick=\"window.location.href='/settings?rd=1\';\">Read/Write Values</button>\n";
  out += "<br><br>\n";
  
  out += "<a href='/'>Go Back</a>";
  
  out += "</center></body></html>";
  server.send(200, "text/html", out);
}

void getData() 
{
  String out;
  char temp[400];
  
  out = "<html>\
         <head>\
           <meta http-equiv='refresh' content='10'/>\
           <title>SoyoPower</title>\
         </head>";
  out += "<body>\
          <center>\
            <h1>Soyo Power Monitor</h1>";

  sprintf(temp,"msg %d &nbsp\n",msgct);
  out += temp;
  sprintf(temp,"errmsg %d &nbsp\n",errmsgct);
  out += temp;
  sprintf(temp,"cserr %d <br>\n",cserr);
  out += temp;
  sprintf(temp,"power %d W<br><br>\n",(int)(sd.V_input*sd.A_input));
  out += temp;
  sprintf(temp,"opmode %d &nbsp",sd.opmode>>4);
  out += temp;
  byte locmode = sd.opmode>>4;
  if (locmode & 0b0001) out += "cp ";   //0001
  if (locmode & 0b0010) out += "pv ";    //0010
  if (locmode & 0b0100) out += "stdby "; //0100
  if (locmode & 0b1000) out += "limit "; //1000   
  out += "<br>\n";

  sprintf(temp,"frame %x <br>\n",sd.opmode&0x0F);
  out += temp;
  sprintf(temp,"status %x &nbsp\n",sd.errstat);
  out += temp;
  byte locstat = sd.errstat;
  if (locstat & 0b00000010) out += "DC low ";    
  if (locstat & 0b00000100) out += "DC high "; 
  if (locstat & 0b00001000) out += "AC high ";    
  if (locstat & 0b00010000) out += "AC low ";   
  if (locstat & 0b00100000) out += "OverHeat ";  
  if (locstat & 0b01000000) out += "Limiter "; 
  out += "<br>\n";

  sprintf(temp,"V input %.1f <br>\n",sd.V_input);
  out += temp;
  sprintf(temp,"A input %.1f <br>\n",sd.A_input);
  out += temp;
  sprintf(temp,"V main %.1f <br>\n",sd.V_main);
  out += temp;
  sprintf(temp,"HZ main %.1f <br>\n",sd.net_HZ);
  out += temp;
  sprintf(temp,"temp %.2f <br><br>\n",sd.temp);
  out += temp;
  sprintf(temp,"Shelly AC power %d <br>\n",pwrsum);
  out += temp;
  sprintf(temp,"Requested power %d <br>\n",sd.req_power);
  out += temp;
  if (DoLimit) out += "Limiter ON<br>\n";
  else         out += "Limiter OFF<br>\n";

  out += "<br>\n";
  out += "<button onclick=\"window.location.href='/settings?rd=1\';\">Settings</button>\n";
  out += "<br><br>\n";
  out += "<a href='/pwr.svg'>Graph</a>";
  out += "<br><br>\n";
  out += "<a href='/up'>Upload</a>";
  out += "<br><br>\n";
  out += "<a href='/limit'>Limiter</a>";
  out += "<br><br>\n";
  
  sprintf(temp,"%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x<br>\n",se[1],se[2],se[3],se[4],se[5],se[6],se[7],se[8],se[9],se[10],se[11],se[12],se[12]); 
  out += temp;

  out += "</center></body></html>";
  
  server.send(200, "text/html", out);
}

#define num_datasets 720 // every minute, 12h 
#define num_sensors_max 6
#define num_sensors 3 
String colors[num_sensors_max] = {"red","green","magenta","orange","lime","yellow"};

struct wp
{
  uint8_t val[num_sensors];
  int8_t hour;
};

wp wp_arr[num_datasets];
uint16_t datasetpt = 0;

void clearwp(){ for(int i=0;i<num_datasets;i++) wp_arr[i].hour = -1; }

void storewp(float V, float A, int pwr, int8_t hour)
{
  wp_arr[datasetpt].hour = hour;
  wp_arr[datasetpt].val[0] = (uint8_t)(5.0*V); // 0..50V
  wp_arr[datasetpt].val[1] = (uint8_t)(0.25*A*V); // 0..1000w
  if (pwr < 0) pwr = 0; 
  wp_arr[datasetpt].val[2] = (uint8_t)(pwr/4);
  datasetpt++;
  if (datasetpt >= num_datasets) datasetpt = 0;
}

void drawGraph() 
{
  String out;
  out.reserve(3800);
  int8_t oldhour = -1;
  char temp[200];

  out += "<svg viewBox=\"0 0 740 240\" xmlns=\"http://www.w3.org/2000/svg\">\n";
  out += "<rect x=\"10\" y=\"0\" width=\"720\" height=\"240\" stroke=\"black\" fill=\"none\"/>\n";
      
  for (int i=0;i<num_datasets;i++)
  {
    int8_t newhour;
    uint16_t pos = datasetpt + i;
    if (pos >= num_datasets) pos -= num_datasets;
    newhour = wp_arr[pos].hour;
    if ((newhour != oldhour) && (oldhour >= 0))
    {
      if ((newhour == 12) || (newhour == 0))  // 12, 24
        sprintf(temp,"<line x1=\"%d\" y1=\"240\" x2=\"%d\" y2=\"210\" stroke=\"black\" />\n",10+i,10+i);
      else if ((newhour == 6) || (newhour == 18)) // 6, 18
        sprintf(temp,"<line x1=\"%d\" y1=\"240\" x2=\"%d\" y2=\"220\" stroke=\"black\" />\n",10+i,10+i);
      else 
        sprintf(temp,"<line x1=\"%d\" y1=\"240\" x2=\"%d\" y2=\"230\" stroke=\"black\" />\n",10+i,10+i);
      out += temp;        
    }
    oldhour = newhour;
  }

  for (int k=0;k<num_sensors;k++)
  {
    out += "<polyline points=\"";
    // datasetpt is now, show the past
    for (int i=0;i<num_datasets;i++)
    {
      uint16_t pos = datasetpt + i;
      if (pos >= num_datasets) pos -= num_datasets;
      if (wp_arr[pos].hour >= 0)
      {
        int tempdata = wp_arr[pos].val[k];
        sprintf(temp,"%d,%d ",10+i,240-tempdata);
        out += temp;
      }
    }
    out += "\" fill=\"none\" stroke=\""; out += colors[k]; out += "\" />\n"; 
  }
  
  //strsize = out.length();  
  out += "</svg>\n";
  
  server.send(200, "image/svg+xml", out);
}

void limiter() 
{
  String out;
  char temp[100];
  
  for (uint8_t i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "trgt") sd.target_power = server.arg(i).toInt();
    if (server.argName(i) == "min")  sd.min_power = server.arg(i).toInt();
    if (server.argName(i) == "manu") sd.manual_power = server.arg(i).toInt();
    if (server.argName(i) == "limon") DoLimit = true;
    if (server.argName(i) == "limof") DoLimit = false;
  }  
  out += "<body><center>\n";
            
  out += "<h1>Limiter</h1>";         
  out += "<br>\n";
  
  out += "<form action=\"/limit\">\n";
  out += "Power Limiter Min Watt: ";
  sprintf(temp,"<input type=\"number\" name=\"min\" step=\"10\" value=%d>&nbsp\n",sd.min_power);
  out += temp;
  out += "<input type=\"submit\" value=\"Write\"><br>\n";
  out += "</form>\n";

  out += "<form action=\"/limit\">\n";
  out += "Power Limiter Target Watt: ";
  sprintf(temp,"<input type=\"number\" name=\"trgt\" step=\"10\" value=%d>&nbsp\n",sd.target_power);
  out += temp;
  out += "<input type=\"submit\" value=\"Write\"><br>\n";
  out += "</form>\n";

  out += "<form action=\"/limit\">\n";
  out += "Power Limiter Manual (set 0 for off) Watt: ";
  sprintf(temp,"<input type=\"number\" name=\"manu\" step=\"10\" value=%d>&nbsp\n",sd.manual_power);
  out += temp;
  out += "<input type=\"submit\" value=\"Write\"><br>\n";
  out += "</form>\n";

  out += "<a href='/limit?limon=1'>limit on</a>&nbsp\n";
  out += "<a href='/limit?limof=1'>limit off</a><br><br>\n";
  

  out += "<a href='/'>Back</a>";
  out += "<br><br>\n";

  out += "</center></body></html>";
  server.send(200, "text/html", out);
}
