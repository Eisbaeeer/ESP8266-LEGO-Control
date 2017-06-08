/*--------------------------------------------------
This is a LEGO controller.
Used parts:
- Nodemcu 1.0 ESP-12E module
- 5V stepper 28byj-48
- Motor driver ULN2003

Author: Eisbaeeer
License: http://creativecommons.org/licenses/by-nc/3.0/
licensed under the Creative Commons - Attribution - Non-Commercial license

Version:
0.2 
- This is the initial version of LEGO controller

0.3 
- Adding D-Pad (directional pad) for vehicles

0.3 2017-06-08
- Bugfix: D-Pad

###################################################
Source of project:
HTTP 1.1 Webserver as AccessPoint for ESP8266 
for ESP8266 adapted Arduino IDE

by Stefan Thesen 08/2015 - free for anyone

Does HTTP 1.1 with defined connection closing.
Handles empty requests in a defined manner.
Handle requests for non-exisiting pages correctly.

This demo allows to switch two functions:
Function 1 creates serial output and toggels GPIO2
Function 2 just creates serial output.

Serial output can e.g. be used to steer an attached
Arduino, Raspberry etc.
--------------------------------------------------*/

#include <ESP8266WiFi.h>

const char* ssid = "LEGO-Controller";
const char* password = "12345678";  // set to "" for open access point w/o password

unsigned long ulReqcount;


// Create an instance of the server on Port 80
WiFiServer server(80);   // the server listen on: 192.168.4.1

// Global vars
  
  int motor1 = 0;  //control var for motor1
  int motor2 = 0;  //control var for motor2


  const int motorPin1 = 0;    // Blue   - In 1
  const int motorPin2 = 2;    // Pink   - In 2
  const int motorPin3 = 4;    // Yellow - In 3
  const int motorPin4 = 5;    // Orange - In 4
                              // Red    - pin 5 (VCC)

  const int motorPin5 = 12;    // Blue   - In 1
  const int motorPin6 = 13;    // Pink   - In 2
  const int motorPin7 = 14;    // Yellow - In 3
  const int motorPin8 = 16;    // Orange - In 4
                               // Red    - pin 5 (VCC)

void setup() 
{
  // setup globals
  ulReqcount=0; 
    
  //--- INIT hardware pins
  pinMode(motorPin1,  OUTPUT);  // Stepper1 IN1
  pinMode(motorPin2,  OUTPUT);  //          IN2
  pinMode(motorPin3,  OUTPUT);  //          IN3
  pinMode(motorPin4,  OUTPUT);  //          IN4
  pinMode(motorPin5, OUTPUT);   // Stepper2 IN1
  pinMode(motorPin6, OUTPUT);   //          IN2
  pinMode(motorPin7, OUTPUT);   //          IN3
  pinMode(motorPin8, OUTPUT);   //          IN4

  //--- set pins to 0
  stopMotor1();
  stopMotor2();
  
  // start serial
  Serial.begin(9600);
  delay(1);
  Serial.println("");
  Serial.println("LEGO controller ver.0.1");
  
  // AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  server.begin();
}

void loop() 
{ 

    //////////////////////
    // call sub programs
    //////////////////////

  // define the speed of motors. Attention, the values should be in this range
  // lowSpeed not upper then 16000
  // highSpeed not lower then 1000

  if      (motor1 == 100) { rightMotor1(2,4,6,8,10,12,14,16);    } 
  else if (motor1 == 80)  { rightMotor1(3,6,9,12,15,18,21,24);    } 
  else if (motor1 == 60)  { rightMotor1(4,8,12,16,20,24,28,32);    } 
  else if (motor1 == 40)  { rightMotor1(5,10,15,20,25,30,35,40);   } 
  else if (motor1 == 20)  { rightMotor1(6,12,18,24,30,36,42,48);   } 
  else if (motor1 == 0)   { stopMotor1();         }
  else if (motor1 == -20)   { leftMotor1(6,12,18,24,30,36,42,48); } 
  else if (motor1 == -40)   { leftMotor1(5,10,15,20,25,30,35,40); } 
  else if (motor1 == -60)   { leftMotor1(4,8,12,16,20,24,28,32);  } 
  else if (motor1 == -80)   { leftMotor1(3,6,9,12,15,18,21,24);  } 
  else if (motor1 == -100)  { leftMotor1(2,4,6,8,10,12,14,16);  }

  if      (motor2 == 100) { rightMotor2(2,4,6,8,10,12,14,16);    } 
  else if (motor2 == 80)  { rightMotor2(3,6,9,12,15,18,21,24);    } 
  else if (motor2 == 60)  { rightMotor2(4,8,12,16,20,24,28,32);    } 
  else if (motor2 == 40)  { rightMotor2(5,10,15,20,25,30,35,40);   } 
  else if (motor2 == 20)  { rightMotor2(6,12,18,24,30,36,42,48);   } 
  else if (motor2 == 0)   { stopMotor2();         }
  else if (motor2 == -20)   { leftMotor2(6,12,18,24,30,36,42,48); } 
  else if (motor2 == -40)   { leftMotor2(5,10,15,20,25,30,35,40); } 
  else if (motor2 == -60)   { leftMotor2(4,8,12,16,20,24,28,32);  } 
  else if (motor2 == -80)   { leftMotor2(3,6,9,12,15,18,21,24);  } 
  else if (motor2 == -100)  { leftMotor2(2,4,6,8,10,12,14,16);  }

  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) 
  {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout) 
  { 
    Serial.println("client connection time-out!");
    return; 
  }
  
  // Read the first line of the request
  String sRequest = client.readStringUntil('\r');
  //Serial.println(sRequest);
  client.flush();
  
  // stop client, if request is empty
  if(sRequest=="")
  {
    Serial.println("empty request! - stopping client");
    client.stop();
    return;
  }
  
  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
  String sPath="",sParam="", sCmd="", buttoncolor = "";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);
    
    // are there parameters?
    if(iEndSpace>0)
    {
      if(iEndQuest>0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }
  
  ///////////////////////////////////////////////////////////////////////////////
  // output parameters to serial, you may connect e.g. an Arduino and react on it
  ///////////////////////////////////////////////////////////////////////////////
  if(sParam.length()>0)
  {
    int iEqu=sParam.indexOf("=");
    if(iEqu>=0)
    {
      sCmd = sParam.substring(iEqu+1,sParam.length());
      Serial.println(sCmd);
    }
  }
  
  
  ///////////////////////////
  // format the html response
  ///////////////////////////
  String sResponse,sPicture,sResponse1,sResponse2,sHeader;
  
  ////////////////////////////
  // 404 for non-matching path
  ////////////////////////////
  if(sPath!="/")
  {
    sResponse="<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";
    
    sHeader  = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length() + sResponse1.length() + sResponse2.length() + sPicture.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  ///////////////////////
  // format the html page
  ///////////////////////
  else
  {
    ulReqcount++;
    sResponse  = "<html><head><title>LEGO ESP8266 Controller</title>";
    //sResponse += "<style> table, th, td { border: 1px solid black;} </style>";
    sResponse += "</head><body>";
    sResponse += "<font color=\"#000000\" face=\"verdana\"><body bgcolor=\"#d0d0f0\">";
    sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponse += "<meta charset=\"utf-8\">";
    sResponse += "<center>";
    sPicture   = "<img align=\"middle\" hspace=\"20\" src=\"data:image/png;base64,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\">";
    sResponse1 = "<font size=\"28\">CONTROL</font><p>";
    
     
    
    //////////////////////
    // react on parameters
    //////////////////////
    if (sCmd.length()>0)
    {
      // write received command to html page
      // sResponse1 += "Kommando: " + sCmd + "<BR>";
      
      // switch the motors
      if(sCmd.indexOf("motor1_100_for")>=0)
      {
        motor1 = 100;
        Serial.println("motor1 turn right with speed: 100%");
      }
      else if(sCmd.indexOf("motor1_80_for")>=0)
      {
        motor1 = 80;
        Serial.println("motor1 turn right with speed: 80%");
      }
      else if(sCmd.indexOf("motor1_60_for")>=0)
      {
        motor1 = 60;
        Serial.println("motor1 turn right with speed: 60%");
      }
      else if(sCmd.indexOf("motor1_40_for")>=0)
      {
        motor1 = 40;
        Serial.println("motor1 turn right with speed: 40%");
      }
      else if(sCmd.indexOf("motor1_20_for")>=0)
      {
        motor1 = 20;
        Serial.println("motor1 turn right with speed: 20%");
      }
      else if(sCmd.indexOf("motor1_stop")>=0)
      {
        motor1 = 0;
        Serial.println("motor1 stop");
      }
      else if(sCmd.indexOf("motor1_20_rev")>=0)
      {
        motor1 = -20;
        Serial.println("motor1 turn left with speed: 20%");
      }
      else if(sCmd.indexOf("motor1_40_rev")>=0)
      {
        motor1 = -40;
        Serial.println("motor1 turn left with speed: 40%");
      }
      else if(sCmd.indexOf("motor1_60_rev")>=0)
      {
        motor1 = -60;
        Serial.println("motor1 turn left with speed: 60%");
      }
      else if(sCmd.indexOf("motor1_80_rev")>=0)
      {
        motor1 = -80;
        Serial.println("motor1 turn left with speed: 80%");
      }
      else if(sCmd.indexOf("motor1_100_rev")>=0)
      {
        motor1 = -100;
        Serial.println("motor1 turn left with speed: 100%");
      }
      else if(sCmd.indexOf("motor2_100_for")>=0)
      {
        motor2 = 100;
        Serial.println("motor2 turn right with speed: 100%");
      }
      else if(sCmd.indexOf("motor2_80_for")>=0)
      {
        motor2 = 80;
        Serial.println("motor2 turn right with speed: 80%");
      }
      else if(sCmd.indexOf("motor2_60_for")>=0)
      {
        motor2 = 60;
        Serial.println("motor2 turn right with speed: 60%");
      }
      else if(sCmd.indexOf("motor2_40_for")>=0)
      {
        motor2 = 40;
        Serial.println("motor2 turn right with speed: 40%");
      }
      else if(sCmd.indexOf("motor2_20_for")>=0)
      {
        motor2 = 20;
        Serial.println("motor2 turn right with speed: 20%");
      }
      else if(sCmd.indexOf("motor2_stop")>=0)
      {
        motor2 = 0;
        Serial.println("motor2 stop");
      }
      else if(sCmd.indexOf("motor2_20_rev")>=0)
      {
        motor2 = -20;
        Serial.println("motor2 turn left with speed: 20%");
      }
      else if(sCmd.indexOf("motor2_40_rev")>=0)
      {
        motor2 = -40;
        Serial.println("motor2 turn left with speed: 40%");
      }
      else if(sCmd.indexOf("motor2_60_rev")>=0)
      {
        motor2 = -60;
        Serial.println("motor2 turn left with speed: 60%");
      }
      else if(sCmd.indexOf("motor2_80_rev")>=0)
      {
        motor2 = -80;
        Serial.println("motor2 turn left with speed: 80%");
      }
      else if(sCmd.indexOf("motor2_100_rev")>=0)
      {
        motor2 = -100;
        Serial.println("motor2 turn left with speed: 100%");
      }

      // D-Pad
      else if(sCmd.indexOf("NORDWEST_FFWD")>=0)
      {
        motor1 = 80;
        motor2 = -100;
        Serial.println("NWFFWD");
      }
      else if(sCmd.indexOf("NORD_FFWD")>=0)
      {
        motor1 = 100;
        motor2 = -100;
        Serial.println("NFFWD");
      }
      else if(sCmd.indexOf("NORDOST_FFWD")>=0)
      {
        motor1 = 100;
        motor2 = -80;
        Serial.println("NOFFWD");
      }
      else if(sCmd.indexOf("NORDWEST_FWD")>=0)
      {
        motor1 = 40;
        motor2 = -80;
        Serial.println("NORDWEST_FWD");
      }
      else if(sCmd.indexOf("NORD_FWD")>=0)
      {
        motor1 = 40;
        motor2 = -40;
        Serial.println("NFWD");
      }
      else if(sCmd.indexOf("NORDOST_FWD")>=0)
      {
        motor1 = 80;
        motor2 = -40;
        Serial.println("NOFWD");
      }
      else if(sCmd.indexOf("WEST_FFWD")>=0)
      {
        motor1 = 0;
        motor2 = -100;
        Serial.println("WFFWD");
      }
      else if(sCmd.indexOf("WEST_FWD")>=0)
      {
        motor1 = 0;
        motor2 = -40;
        Serial.println("WFWD");
      }
      else if(sCmd.indexOf("STOP")>=0)
      {
        motor1 = 0;
        motor2 = 0;
        Serial.println("STOP");
      }
      else if(sCmd.indexOf("OST_FWD")>=0)
      {
        motor1 = 40;
        motor2 = 0;
        Serial.println("OFWD");
      }
      else if(sCmd.indexOf("OST_FFWD")>=0)
      {
        motor1 = 100;
        motor2 = 0;
        Serial.println("OFFWD");
      }
      else if(sCmd.indexOf("SUEDWEST_BAC")>=0)
      {
        motor1 = -40;
        motor2 = 80;
        Serial.println("SWBAC");
      }
      else if(sCmd.indexOf("SUED_BAC")>=0)
      {
        motor1 = -40;
        motor2 = 40;
        Serial.println("SBAC");
      }
      else if(sCmd.indexOf("SUEDOST_BAC")>=0)
      {
        motor1 = -80;
        motor2 = 40;
        Serial.println("SOBAC");
      }
      else if(sCmd.indexOf("SUEDWEST_BAC")>=0)
      {
        motor1 = -80;
        motor2 = 100;
        Serial.println("SWBAC");
      }
      else if(sCmd.indexOf("SUED_FBAC")>=0)
      {
        motor1 = -100;
        motor2 = 100;
        Serial.println("SFBAC");
      }
      else if(sCmd.indexOf("SUEDOST_FBAC")>=0)
      {
        motor1 = -100;
        motor2 = 80;
        Serial.println("SOFBAC");
      }
      else if(sCmd.indexOf("SUEDWEST_FBAC")>=0)
      {
        motor1 = -80;
        motor2 = 100;
        Serial.println("SWFBAC");
      }
    }

  //  output the tables
  //////////////////////////////////

    // table1 begin
    sResponse1 += "<table style=\"with:100%\">";
    sResponse1 += "<tr>";
    sResponse1 += "<th>";

    // table2 begin
    sResponse1 += "<table style=\"with:100%\">";
        
    sResponse1 += "<tr>";
    sResponse1 += "<th>MOTOR 1</th>";
    sResponse1 += "<th></th>";
    sResponse1 += "<th>MOTOR 2</th>";
    sResponse1 += "</tr>";

    sResponse1 += "<tr>";
    sResponse1 += "<th></th>";
    sResponse1 += "<th></th>";
    sResponse1 += "<th></th>";
    sResponse1 += "</tr>";

    
    sResponse1 += "<tr>";
    if (motor1 == 100) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor1_100_for\"><button style=\"background-color:" + buttoncolor + "\">100%</button></a></th>";
    sResponse1 += "<th></th>";
    if (motor2 == 100) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor2_100_for\"><button style=\"background-color:" + buttoncolor + "\">100%</button></a></th>";
    sResponse1 += "</tr>";

    sResponse1 += "<tr>";
    if (motor1 == 80) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor1_80_for\"><button style=\"background-color:" + buttoncolor + "\">80%</button></a></th>";
    sResponse1 += "<th></th>";
    if (motor2 == 80) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor2_80_for\"><button style=\"background-color:" + buttoncolor + "\">80%</button></a></th>";
    sResponse1 += "</tr>";

    sResponse1 += "<tr>";
    if (motor1 == 60) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor1_60_for\"><button style=\"background-color:" + buttoncolor + "\">60%</button></a></th>";
    sResponse1 += "<th></th>";
    if (motor2 == 60) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor2_60_for\"><button style=\"background-color:" + buttoncolor + "\">60%</button></a></th>";
    sResponse1 += "</tr>";

    sResponse1 += "<tr>";
    if (motor1 == 40) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor1_40_for\"><button style=\"background-color:" + buttoncolor + "\">40%</button></a></th>";
    sResponse1 += "<th></th>";
    if (motor2 == 40) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor2_40_for\"><button style=\"background-color:" + buttoncolor + "\">40%</button></a></th>";
    sResponse1 += "</tr>";

    sResponse1 += "<tr>";
    if (motor1 == 20) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor1_20_for\"><button style=\"background-color:" + buttoncolor + "\">20%</button></a></th>";
    sResponse1 += "<th></th>";
    if (motor2 == 20) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor2_20_for\"><button style=\"background-color:" + buttoncolor + "\">20%</button></a></th>";
    sResponse1 += "</tr>";


    sResponse1 += "<tr>";
    if (motor1 == 0) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor1_stop\"><button style=\"background-color:" + buttoncolor + "\">STOP</button></a></th>";
    sResponse1 += "<th></th>";
    if (motor2 == 0) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor2_stop\"><button style=\"background-color:" + buttoncolor + "\">STOP</button></a></th>";
    sResponse1 += "</tr>";

    sResponse1 += "<tr>";
    if (motor1 == -20) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor1_20_rev\"><button style=\"background-color:" + buttoncolor + "\">-20%</button></a></th>";
    sResponse1 += "<th></th>";
    if (motor2 == -20) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor2_20_rev\"><button style=\"background-color:" + buttoncolor + "\">-20%</button></a></th>";
    sResponse1 += "</tr>";

    sResponse1 += "<tr>";
    if (motor1 == -40) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor1_40_rev\"><button style=\"background-color:" + buttoncolor + "\">-40%</button></a></th>";
    sResponse1 += "<th></th>";
    if (motor2 == -40) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor2_40_rev\"><button style=\"background-color:" + buttoncolor + "\">-40%</button></a></th>";
    sResponse1 += "</tr>";

    sResponse1 += "<tr>";
    if (motor1 == -60) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor1_60_rev\"><button style=\"background-color:" + buttoncolor + "\">-60%</button></a></th>";
    sResponse1 += "<th></th>";
    if (motor2 == -60) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor2_60_rev\"><button style=\"background-color:" + buttoncolor + "\">-60%</button></a></th>";
    sResponse1 += "</tr>";

    sResponse1 += "<tr>";
    if (motor1 == -80) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor1_80_rev\"><button style=\"background-color:" + buttoncolor + "\">-80%</button></a></th>";
    sResponse1 += "<th></th>";
    if (motor2 == -80) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor2_80_rev\"><button style=\"background-color:" + buttoncolor + "\">-80%</button></a></th>";
    sResponse1 += "</tr>";

    sResponse1 += "<tr>";
    if (motor1 == -100) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor1_100_rev\"><button style=\"background-color:" + buttoncolor + "\">-100%</button></a></th>";
    sResponse1 += "<th></th>";
    if (motor2 == -100) { buttoncolor = "red"; } else { buttoncolor = ""; }
    sResponse1 += "<th><a href=\"?pin=motor2_100_rev\"><button style=\"background-color:" + buttoncolor + "\">-100%</button></a></th>";
    sResponse1 += "</tr>";

    sResponse1 += "</table>";
    // table2 end

    sResponse1 += "</th>";
    sResponse1 += "<th>";

    // table 3 begin
    sResponse2 = "<table style=\"with:100%\">";
        
    sResponse2 += "<tr>";
    sResponse2 += "<th><a href=\"?pin=NORDWEST_FFWD\"><button>&#8662;</button></a></th>";
    sResponse2 += "<th></th>";
    sResponse2 += "<th><a href=\"?pin=NORD_FFWD\"><button>&#8657;</button></a></th>";
    sResponse2 += "<th></th>";
    sResponse2 += "<th><a href=\"?pin=NORDOST_FFWD\"><button>&#8663;</button></a></th>";
    sResponse2 += "</tr>";
    
    sResponse2 += "<tr>";
    sResponse2 += "<th></th>";
    sResponse2 += "<th><a href=\"?pin=NORDWEST_FWD\"><button>&#8598;</button></a></th>";
    sResponse2 += "<th><a href=\"?pin=NORD_FWD\"><button>&#8593;</button></a></th>";
    sResponse2 += "<th><a href=\"?pin=NORDOST_FWD\"><button>&#8599;</button></a></th>";
    sResponse2 += "<th></th>";
    sResponse2 += "</tr>";

    sResponse2 += "<tr>";
    sResponse2 += "<th><a href=\"?pin=WEST_FFWD\"><button>&#8656;</button></a></th>";
    sResponse2 += "<th><a href=\"?pin=WEST_FWD\"><button>&#8592;</button></a></th>";
    sResponse2 += "<th><a href=\"?pin=STOP\"><button>&#11036</button></a></th>";
    sResponse2 += "<th><a href=\"?pin=OST_FWD\"><button>&#8594;</button></a></th>";
    sResponse2 += "<th><a href=\"?pin=OST_FFWD\"><button>&#8658;</button></a></th>";
    sResponse2 += "</tr>";

    sResponse2 += "<tr>";
    sResponse2 += "<th></th>";
    sResponse2 += "<th><a href=\"?pin=SUEDWEST_BAC\"><button>&#8601;</button></a></th>";
    sResponse2 += "<th><a href=\"?pin=SUED_BAC\"><button>&#8595;</button></a></th>";
    sResponse2 += "<th><a href=\"?pin=SUEDOST_BAC\"><button>&#8600;</button></a></th>";
    sResponse2 += "<th></th>";
    sResponse2 += "</tr>";

    sResponse2 += "<tr>";
    sResponse2 += "<th><a href=\"?pin=SUEDWEST_FBAC\"><button>&#8665;</button></a></th>";
    sResponse2 += "<th></th>";
    sResponse2 += "<th><a href=\"?pin=SUED_FBAC\"><button>&#8659;</button></a></th>";
    sResponse2 += "<th></th>";
    sResponse2 += "<th><a href=\"?pin=SUEDOST_FBAC\"><button>&#8664;</button></a></th>";
    sResponse2 += "</tr>";
 

    sResponse2 += "</table>";
    // table3 end
    
    sResponse2 += "</th>";
    sResponse2 += "</tr>";
    sResponse2 += "</table>";
    // table1 end

    sResponse2 += "<p>\r\n";
    

    sResponse2 += "<FONT SIZE=-2>";
    sResponse2 += "<P>Counter: "; 
    sResponse2 += ulReqcount;
    sResponse2 += "<BR>";
    sResponse2 += "Version 0.2 - Eisbaeeer 05/2017 - <a href=\"https://github.com/Eisbaeeer\">https://github.com/Eisbaeeer<BR>";
    sResponse2 += "</body></html>";
    
    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length() + sResponse1.length() + sResponse2.length() + sPicture.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  
  // Send the response to the client
  client.print(sHeader);
  client.print(sResponse);
  client.print(sPicture);
  client.print(sResponse1);
  client.print(sResponse2);
  
  // and stop the client
  client.stop();
  Serial.println("Client disonnected");

}


////////////////////
// sub programms
////////////////////
void stopMotor1()
{
    digitalWrite(motorPin1,LOW);    digitalWrite(motorPin2,LOW);    digitalWrite(motorPin3,LOW);    digitalWrite(motorPin4,LOW);
}

void stopMotor2()
{
    digitalWrite(motorPin5,LOW);    digitalWrite(motorPin6,LOW);    digitalWrite(motorPin7,LOW);    digitalWrite(motorPin8,LOW);
 }

void rightMotor1(int motor1Speed1,int motor1Speed2,int motor1Speed3,int motor1Speed4,int motor1Speed5,int motor1Speed6,int motor1Speed7,int motor1Speed8)
{ 
  int waitPhase1=millis()%(motor1Speed1+motor1Speed1+motor1Speed1+motor1Speed1+motor1Speed1+motor1Speed1+motor1Speed1+motor1Speed1 );
  if (waitPhase1<motor1Speed1) { digitalWrite(motorPin4, HIGH);  digitalWrite(motorPin3, LOW);  digitalWrite(motorPin2, LOW);  digitalWrite(motorPin1, LOW);  } // 1
  if ((waitPhase1>motor1Speed1) && (waitPhase1<motor1Speed2)) { digitalWrite(motorPin4, HIGH);  digitalWrite(motorPin3, HIGH);  digitalWrite(motorPin2, LOW);  digitalWrite(motorPin1, LOW); } // 2
  if ((waitPhase1>motor1Speed2) && (waitPhase1<motor1Speed3)) { digitalWrite(motorPin4, LOW);  digitalWrite(motorPin3, HIGH);  digitalWrite(motorPin2, LOW);  digitalWrite(motorPin1, LOW);  } // 3
  if ((waitPhase1>motor1Speed3) && (waitPhase1<motor1Speed4)) { digitalWrite(motorPin4, LOW);  digitalWrite(motorPin3, HIGH);  digitalWrite(motorPin2, HIGH);  digitalWrite(motorPin1, LOW); } // 4
  if ((waitPhase1>motor1Speed4) && (waitPhase1<motor1Speed5)) { digitalWrite(motorPin4, LOW);  digitalWrite(motorPin3, LOW);  digitalWrite(motorPin2, HIGH);  digitalWrite(motorPin1, LOW);  } // 5
  if ((waitPhase1>motor1Speed5) && (waitPhase1<motor1Speed6)) { digitalWrite(motorPin4, LOW);  digitalWrite(motorPin3, LOW);  digitalWrite(motorPin2, HIGH);  digitalWrite(motorPin1, HIGH); } // 6
  if ((waitPhase1>motor1Speed6) && (waitPhase1<motor1Speed7)) { digitalWrite(motorPin4, LOW);  digitalWrite(motorPin3, LOW);  digitalWrite(motorPin2, LOW);  digitalWrite(motorPin1, HIGH);  } // 7
  if ((waitPhase1>motor1Speed7) && (waitPhase1<motor1Speed8)) { digitalWrite(motorPin4, HIGH);  digitalWrite(motorPin3, LOW);  digitalWrite(motorPin2, LOW);  digitalWrite(motorPin1, HIGH); } // 8
}

void leftMotor1(int motor1Speed1,int motor1Speed2,int motor1Speed3,int motor1Speed4,int motor1Speed5,int motor1Speed6,int motor1Speed7,int motor1Speed8)
{ 
  int waitPhase1=millis()%(motor1Speed1+motor1Speed1+motor1Speed1+motor1Speed1+motor1Speed1+motor1Speed1+motor1Speed1+motor1Speed1 );
  if (waitPhase1<motor1Speed1) { digitalWrite(motorPin1, HIGH);  digitalWrite(motorPin2, LOW);  digitalWrite(motorPin3, LOW);  digitalWrite(motorPin4, LOW);  }  // 1
  if ((waitPhase1>motor1Speed1) && (waitPhase1<motor1Speed2)) { digitalWrite(motorPin1, HIGH);  digitalWrite(motorPin2, HIGH);  digitalWrite(motorPin3, LOW);  digitalWrite(motorPin4, LOW); } // 2
  if ((waitPhase1>motor1Speed2) && (waitPhase1<motor1Speed3)) { digitalWrite(motorPin1, LOW);  digitalWrite(motorPin2, HIGH);  digitalWrite(motorPin3, LOW);  digitalWrite(motorPin4, LOW);  } // 3
  if ((waitPhase1>motor1Speed3) && (waitPhase1<motor1Speed4)) { digitalWrite(motorPin1, LOW);  digitalWrite(motorPin2, HIGH);  digitalWrite(motorPin3, HIGH);  digitalWrite(motorPin4, LOW); } // 4
  if ((waitPhase1>motor1Speed4) && (waitPhase1<motor1Speed5)) { digitalWrite(motorPin1, LOW);  digitalWrite(motorPin2, LOW);  digitalWrite(motorPin3, HIGH);  digitalWrite(motorPin4, LOW);  } // 5
  if ((waitPhase1>motor1Speed5) && (waitPhase1<motor1Speed6)) { digitalWrite(motorPin1, LOW);  digitalWrite(motorPin2, LOW);  digitalWrite(motorPin3, HIGH);  digitalWrite(motorPin4, HIGH); } // 6
  if ((waitPhase1>motor1Speed6) && (waitPhase1<motor1Speed7)) { digitalWrite(motorPin1, LOW);  digitalWrite(motorPin2, LOW);  digitalWrite(motorPin3, LOW);  digitalWrite(motorPin4, HIGH);  } // 7
  if ((waitPhase1>motor1Speed7) && (waitPhase1<motor1Speed8)) { digitalWrite(motorPin1, HIGH);  digitalWrite(motorPin2, LOW);  digitalWrite(motorPin3, LOW);  digitalWrite(motorPin4, HIGH); } // 8
}

void rightMotor2(int motor2Speed1,int motor2Speed2,int motor2Speed3,int motor2Speed4,int motor2Speed5,int motor2Speed6,int motor2Speed7,int motor2Speed8)
{ 
  int waitPhase2=millis()%(motor2Speed1+motor2Speed1+motor2Speed1+motor2Speed1+motor2Speed1+motor2Speed1+motor2Speed1+motor2Speed1 ); 
  if (waitPhase2<motor2Speed1) { digitalWrite(motorPin8, HIGH);  digitalWrite(motorPin7, LOW);  digitalWrite(motorPin6, LOW);  digitalWrite(motorPin5, LOW); } // 1
  if ((waitPhase2>motor2Speed1) && (waitPhase2<motor2Speed2)) { digitalWrite(motorPin8, HIGH);  digitalWrite(motorPin7, HIGH);  digitalWrite(motorPin6, LOW);  digitalWrite(motorPin5, LOW); } // 2
  if ((waitPhase2>motor2Speed2) && (waitPhase2<motor2Speed3)) { digitalWrite(motorPin8, LOW);  digitalWrite(motorPin7, HIGH);  digitalWrite(motorPin6, LOW);  digitalWrite(motorPin5, LOW);  } // 3
  if ((waitPhase2>motor2Speed3) && (waitPhase2<motor2Speed4)) { digitalWrite(motorPin8, LOW);  digitalWrite(motorPin7, HIGH);  digitalWrite(motorPin6, HIGH);  digitalWrite(motorPin5, LOW); } // 4
  if ((waitPhase2>motor2Speed4) && (waitPhase2<motor2Speed5)) { digitalWrite(motorPin8, LOW);  digitalWrite(motorPin7, LOW);  digitalWrite(motorPin6, HIGH);  digitalWrite(motorPin5, LOW);  } // 5
  if ((waitPhase2>motor2Speed5) && (waitPhase2<motor2Speed6)) { digitalWrite(motorPin8, LOW);  digitalWrite(motorPin7, LOW);  digitalWrite(motorPin6, HIGH);  digitalWrite(motorPin5, HIGH); } // 6
  if ((waitPhase2>motor2Speed6) && (waitPhase2<motor2Speed7)) { digitalWrite(motorPin8, LOW);  digitalWrite(motorPin7, LOW);  digitalWrite(motorPin6, LOW);  digitalWrite(motorPin5, HIGH);  } // 7
  if ((waitPhase2>motor2Speed7) && (waitPhase2<motor2Speed8)) { digitalWrite(motorPin8, HIGH);  digitalWrite(motorPin7, LOW);  digitalWrite(motorPin6, LOW);  digitalWrite(motorPin5, HIGH); } // 8
}

void leftMotor2(int motor2Speed1,int motor2Speed2,int motor2Speed3,int motor2Speed4,int motor2Speed5,int motor2Speed6,int motor2Speed7,int motor2Speed8)
{ 
  int waitPhase2=millis()%(motor2Speed1+motor2Speed1+motor2Speed1+motor2Speed1+motor2Speed1+motor2Speed1+motor2Speed1+motor2Speed1 ); 
  if (waitPhase2<motor2Speed1) { digitalWrite(motorPin5, HIGH);  digitalWrite(motorPin6, LOW);  digitalWrite(motorPin7, LOW);  digitalWrite(motorPin8, LOW); } // 1
  if ((waitPhase2>motor2Speed1) && (waitPhase2<motor2Speed2)) { digitalWrite(motorPin5, HIGH);  digitalWrite(motorPin6, HIGH);  digitalWrite(motorPin7, LOW);  digitalWrite(motorPin8, LOW); } // 2
  if ((waitPhase2>motor2Speed2) && (waitPhase2<motor2Speed3)) { digitalWrite(motorPin5, LOW);  digitalWrite(motorPin6, HIGH);  digitalWrite(motorPin7, LOW);  digitalWrite(motorPin8, LOW);  } // 3
  if ((waitPhase2>motor2Speed3) && (waitPhase2<motor2Speed4)) { digitalWrite(motorPin5, LOW);  digitalWrite(motorPin6, HIGH);  digitalWrite(motorPin7, HIGH);  digitalWrite(motorPin8, LOW); } // 4
  if ((waitPhase2>motor2Speed4) && (waitPhase2<motor2Speed5)) { digitalWrite(motorPin5, LOW);  digitalWrite(motorPin6, LOW);  digitalWrite(motorPin7, HIGH);  digitalWrite(motorPin8, LOW);  } // 5
  if ((waitPhase2>motor2Speed5) && (waitPhase2<motor2Speed6)) { digitalWrite(motorPin5, LOW);  digitalWrite(motorPin6, LOW);  digitalWrite(motorPin7, HIGH);  digitalWrite(motorPin8, HIGH); } // 6
  if ((waitPhase2>motor2Speed6) && (waitPhase2<motor2Speed7)) { digitalWrite(motorPin5, LOW);  digitalWrite(motorPin6, LOW);  digitalWrite(motorPin7, LOW);  digitalWrite(motorPin8, HIGH);  } // 7
  if ((waitPhase2>motor2Speed7) && (waitPhase2<motor2Speed8)) { digitalWrite(motorPin5, HIGH);  digitalWrite(motorPin6, LOW);  digitalWrite(motorPin7, LOW);  digitalWrite(motorPin8, HIGH); } // 8
}
