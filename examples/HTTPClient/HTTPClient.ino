/*
* This example demonstrates how to use HTTP client API to send data to a web application at regular intervals.
* 2 analogical pin A1 and A2 and 2 digital input pin D4 and D5 are monitored each time interval TIMEINT
* and sent to a program on REMOTEIP and port PORT. 
* Data are sent as a record format:
*    valAna1 valAna2 valDig4 valDig4 
* PC program receives and stores on file. 
* Beside, because the lack of real time clock on Arduino, PC program has to put a timestamp too.
*
* Console prints are done just for controll (you can delete)
* Library error management have reset policy at present.
* So, socket problem (pc program stop link) or net connection lost produces automatic reset.
*
* A simply java program (SocketLog.jar) is provided to test this example.
* Author: Daniele Denaro
*/

#include <HTTPlib.h>             // include library

/********* Definitions (adapt to your environment) **********************/

#define ACCESSPOINT  "D-Link-casa"       // access point name
#define PASSWORD     ""                  // password if WAP
#define REMOTEIP     "192.168.1.2"       // PC computer address
#define PORT         8080                // application port

#define TIMEINT      10                  // time interval in seconds
#define D4            4                  // input (pullup) pin D4
#define D8            8                  // input (pullup) pin D8
#define AN1           1                  // input analogical A1
#define AN2           2                  // input analogical A2

/*************************************************************************/

char ip[16];                   // buffer for local ip address
int fc=0;                      // flag connection
boolean fs=0;                  // flag socket open
int csocket;                   // client socket handle

HTTP WIFI;                    //instance of MWiFi library

void setup() 
{
  Serial.begin(9600);
  pinMode(D4,INPUT_PULLUP);
  pinMode(D8,INPUT_PULLUP);

  WIFI.begin();                // startup wifi shield
  
  WIFI.setNetMask("255.255.255.0");  //modify default
  WIFI.setGateway("0.0.0.0");        //modify default
  
  if (PASSWORD==""){WIFI.ConnSetOpen(ACCESSPOINT);}         // if passw= empty string connect in open mode
  else             {WIFI.ConnSetWPA(ACCESSPOINT,PASSWORD);} // else connect in WAP mode
  
  netConnection();                   // connection to access point   
 
  if (fc==1)  socketConnection();        // if connect to access point try to open socket
  //end of startup 
} 

void netConnection()
{
    int i;for(i=0;i<5;i++) {fc=WIFI.Connect(); if(fc) break;}             // try to connect for 5 times
    if (!fc)     {delay(60000);wdt_enable(WDTO_1S);}                      // reset if no connection 
    WIFI.getIP(ip);                                                       // get dynamic ip
    Serial.println("Net connected!");
}

void socketConnection()
{
    Serial.print("Start socket connection to ");Serial.print(REMOTEIP);
    Serial.print(":");Serial.println(PORT);
    csocket=WIFI.openSockTCP(REMOTEIP,PORT);                             // try to connect to remote ip 
    if (csocket==255)
     {fs=false;Serial.println("Socket not available!");return;}              // socket not valid. No link! 
    else 
     {fs=true;Serial.println("Socket connected!");}                         // flag socket link open
}
  
 char query[128];                                     // buffer for sending data with GET method 
 char rec[64];                                        // buffer for sending data with POST method
 
void loop() 
{
  if(fs)                                          // if socket connected
  {
    int an1=analogRead(AN1);char sa1[6];sprintf(sa1,"%d",an1);                 // read values
    int an2=analogRead(AN2);char sa2[6];sprintf(sa2,"%d",an2);
    int d1=digitalRead(D4);char sd1[3];sprintf(sd1,"%d",d1);
    int d2=digitalRead(D8);char sd2[3];sprintf(sd2,"%d",d2);
// with method GET 
/*
    WIFI.addParameter(query,128,"/TestWIFIArduino/TestClient",NULL);
    WIFI.addParameter(query,128,"A1",sa1);
    WIFI.addParameter(query,128,"A2",sa2);
    WIFI.addParameter(query,128,"D1",sd1);
    WIFI.addParameter(query,128,"D2",sd2);
    WIFI.sendRequestGET(csocket,query);
*/
// with method POST
    sprintf(rec,"%d %d %d %d",an1,an2,d1,d2);      // prepare record coding values to string
    WIFI.sendRequestPOST(csocket,"/TestWIFIArduino/TestClient",rec);
    int i;char *resp=NULL;
    for (i=0;i<10;i++)                           // receive response
    {
      delay(100);
      resp=WIFI.getResponse(csocket);
      if (resp!=NULL) break;
    }
    if (resp!=NULL) Serial.println(resp);
    else Serial.println("Timeout response");
  }
  else socketConnection();                        // if socket not yet open try to open
  delay(TIMEINT*1000);                            // wait for TIMEINT seconds
}


