#include <Arduino.h>
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"
#include "TFT_eSPI.h"
#include <SPI.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include <millisDelay.h>
#include <Wire.h>
#include <Timezone_Generic.h> // https://github.com/khoih-prog/Timezone_Generic
#include <DS323x_Generic.h>   // https://github.com/khoih-prog/DS323x_Generic
#include <SoftwareSerial.h>
#include "Debug.h"
#include "DefineDir.h"
#include <string.h>
#include <stdio.h>
#include <ArduinoJson.h>
#include "ReadWriteFile.h"
#include "LogoISD.h"

//**************************************************
//               Global Variable
//**************************************************
String VerString = Ver;
char VER_FW[14] = "";
bool DBsyncStartup = false;
bool ServerOnOff = false;
bool LastServerOnOff = true;
bool lastDrawLogo = true;
bool startup = true;
//**************************************************
//               NTP & RTC Settings
//**************************************************

const char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server
//const char timeServer[] = "ntp1.inrim.it";
byte j = 0;
// declare a time object
DS323x rtc;
unsigned long devicetime;
Timezone myTZ;
TimeChangeRule myDST;
TimeChangeRule mySTD;
TimeChangeRule *tcr; //pointer to the time change rule, use to get TZ abbrev
unsigned int localPort = 2390;
const int NTP_PACKET_SIZE = 48;     // NTP timestamp is in the first 48 bytes of the message
const int UDP_TIMEOUT = 2000;       // timeout in miliseconds to wait for an UDP packet to arrive
byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets

char daysOfTheWeek[7][12] = {"Dom", "Lun", "Mar", "Mer", "Gio", "Ven", "Sab"};
char daysOfTheWeekN[7][12] = {"7", "1", "2", "3", "4", "5", "6"};
//**********  END NTP SETTINGS  ********************

//**************************************************
//                    BADGE VAR
//**************************************************
byte RFIDcardNum[4];
byte evenBit = 0;
byte oddBit = 0;
byte evenBit2 = 0;
byte oddBit2 = 0;
byte isData0Low = 0;
byte isData1Low = 0;
byte isData2Low = 0;
byte isData3Low = 0;
int recvBitCount = 0;
byte isCardReadOver = 0;
long rfidValue = 0; //rfidValue: used to store the value obtained from the RFID tag
String rfidCognome = "";
String rfidNome = "";
String rfidUIDdec = "";
int rfidIdBadge = 0;
long BufrfidValue = 0;
int prev_badge = 0;
byte isCardRead = 0;
String direction = "";
int entrata_uscita;
int indiceTabBadge = 0;
bool LoadDBBadge = 0;
int indiceTabProfili = 0;
bool LoadDBProfili = 0;
int indiceTabTime = 0;
bool LoadDBTime = 0;
int tabellaBadgelength = 0;
int tabellaDesProfililength = 0;
int tabellaDesTimelength = 0;
int field = 0;
int fieldP = 0;
int fieldT = 0;
byte idProfilo = 0;
boolean isProfiloOk = 0;
boolean isBadgeOk = 0;
boolean isGateOk = 0;
byte tabFromDB = 0;
bool loadCFGfromSD = false;
long cfgTime = 0;
long cfgTimeLast = 0;
long cfgBadge = 0;
long cfgBadgeLast = 0;
long cfgDesProfili = 0;
long cfgDesProfiliLast = 0;
long cfgDesTime = 0;
long cfgDesTimeLast = 0;
long cfgTabel = 0;
long cfgTabelLast = 0;
boolean waitingForKey = true; //waitingForKey: is used to identify if we are still waiting for a person to present a valid key or not
bool BufBadgePresent = 1;
//**********  END BADGE VAR     ********************

//**************************************************
//                CLOCK VAR
//**************************************************
char barra = '/';    // costante utilizzato in diverse punti del programma / constant
char duepunti = ':'; // costante utilizzato in diverse parti del programma / constant
char virgola = ',';  // costante utilizzata in diverse parti del programma / constant
char tratto = ' - ';
int tempolettura = 3500; // tempo di lettura del messaggio su lcd (3 sec.) /time to read lcd msg
byte digits = 0;
byte digitsm = 0;
byte digitss = 0;
byte mese = 0;        // zona di memorizzazione del mese corrente / current month
byte secondoprec = 0; // secondo evidenziato su display lcd /second on lcd
//**********  END CLOCK VAR     ********************

//**************************************************
//           INTERRUPT e Serial VAR VAR
//**************************************************
volatile byte state_5S_PRESS = HIGH;
volatile byte state_a = HIGH;
volatile byte do_break = 0;
SoftwareSerial rfid(3, 2);  // RX Yellow Growe,TX White Growe
SoftwareSerial rfid2(1, 0); // RX Yellow Growe,TX White Growe
//**********  END INTERRUPT VAR ********************

//**************************************************
//             TIMER VAR
//**************************************************
#define N_ELEM 11
int prog[N_ELEM] = {2500, 60000, 500, 500, 500, 9000000, 0, 10000, 1500, 1500, 1500};
int timer[N_ELEM] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 0 = timer diplay message 1 =UpdateTransit 2 = beep
unsigned long t1, dt;

millisDelay updateNTP;     // the update delay object. used for ntp periodic update.
millisDelay updateDate;    // the update delay object. used for Date periodic update.
millisDelay updateOra;     // the update delay object. used for hours periodic update.
millisDelay updateCfg;     // the update delay object. used to periodically check for a new configuration.
millisDelay updateServer;  // the update delay object. used to periodically check for a server connection and display it.
millisDelay tftTimer;      // the update delay object. used to turn off the display
millisDelay updateConnDB;  // the update delay object. used to check if the DB is reachable
millisDelay badgePrevius;  // the update delay object. used for the timeout of the lock reading the same badge
millisDelay ResetETHDelay; // the update delay object. used for repeating the hardware reset of the ethernet shield
int updateConnDBDelay = 5000;
int updateServerDelay = 20000;
int updateResetETHDelay = 5000;
bool ETHStartReset = false;
bool ETHEndReset = false;
bool ETHReseted = false;
bool ETH_INIT = false;
bool ETH_begin = false;
//**********  END TIMER VAR     ********************

//**************************************************
//             SCREEN VAR
//**************************************************
int xlogo = 35;
int ylogo = 55;
int xServer = 13;
int yServer = 222;
int xCleanServer = xServer - 5;
int yCleanServer = yServer - 15;
int WCleanServer = 182;
int HCleanServer = 20;
int xcur = 25;
int ycur = 100;
int xpos = 210;
int ypos = 13;
bool statoLCD_BACKLIGHT = true;
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
int xBoxWt = 0;
int yBoxWt = 38;
int HBoxWt = tft.width() - yBoxWt - (HCleanServer + 10);
int WBoxWt = tft.height() - xBoxWt;
//**********  END SCREEN VAR     ********************

//***************************************************
//              ETHERNET VAR
//***************************************************
byte EthernetStatus = 0;
boolean ReteConnected = 0;
boolean ETHStartup = 0;
int LastLinkStatus = 0;
int LinkStatus = 0;
//**********  END ETHERNET VAR     ******************

//***************************************************
//              DB VAR
//***************************************************
bool DBConnected = false;
int dnsReturn = 0;
char hostname[STRING_LEN];
int server_port = 3306;
char user[] = "ACAP100";          // MySQL user login username
char password[] = "CA$area@2020"; // MySQL user login password
String tabellacodici[3300];       /* table of codes of valid cards max 550 badges */
String tabellaDesProfili[400];    /* valid card profile table  */
String tabellaDesTime[400];       /* table of time slots of valid cards  */
bool UpdatedTransit = 0;
bool UpdateDBok = 0;
//**********  END DB VAR          *******************

//***************************************************
//              FILE NAME CONSTANT
//***************************************************
// SD library uses 8.3 filenames
const char *filenameCFG = "/cfgACAP.jsn";       // Basic configuration file stored on SD
const char *filenameCFGBack = "/cfgACAP.bak";   // Configuration file used as a backup before replacing it with the new one you downloaded
const char *filenameCFG2 = "/cfgACAP2.jsn";     // Secondary configuration file
const char *filenameCFG2Back = "/cfgACAP2.bak"; // Secondary configuration file used as a backup before replacing it with the new one you downloaded
const char *filenameBadge1 = "/BADGE1.csv";     // Badge list 1 it was necessary to subdivide the badge list into several fali in order not to load too long a string during the reading phase
const char *filenameBadge2 = "/BADGE2.csv";     // Badge list 2
const char *filenameBadge3 = "/BADGE3.csv";     // Badge list 3
const char *filenameProfili = "/PROFILI.csv";   // File containing the profiles of the valid badges associated with the readers
const char *filenameDesTime = "/DESTIME.csv";   // File containing the time slots of the valid badges associated with the readers
const char *filenameAccessBuf = "/Accessi.csv"; // Badge transit buffer file
const char *filenameLog = "/Log.txt";           // Log buffer file
//**********  END FILE NAME CONSTANT ****************

#ifdef NoWiFi
#include "defines.h"
#include <MySQL_Generic_Ethernet.h>
#include "Dns.h"

//***************************************************
//         Ethernet Shield Settings
//***************************************************
byte macAdd[6];
IPAddress myIP;
IPAddress myMASK;
IPAddress myDNS;
IPAddress myGW;
// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
//******  END Ethernet Shield Settings  ************
MySQL_Connection conn((Client *)&client);
#else

// Implement functions for WiFi here

#endif

IPAddress server_ip; //(109, 168, 40, 35);

//************************************************
// json file structure of configuration variables
//************************************************
struct Config
{
  byte idGate;
  char GateName[STRING_LEN_LONG];
  char ssidSD[STRING_LEN];
  char passSD[STRING_LEN];
  char MacAddress[STRING_LEN];
  char local_IP[STRING_LEN];
  char gateway[STRING_LEN];
  char subnet[STRING_LEN];
  char primaryDNS[STRING_LEN];
  //char secondaryDNS[STRING_LEN];
  char DBAddressParam[STRING_LEN_LONG];
  int DBPortParam;
  int BeepOnOff;
  char DBUserParam[STRING_LEN];
  char DBPswParam[STRING_LEN];
  int SyncDB;
  byte SingleGate;
  int TimeGate1;
  int TimeGate2;
  int tftTimeout;
  int NTPUpDate;
  int TransitUpdate;
  int Log;
  int DelLog;
  char DataBaseName[STRING_LEN_LONG];
};

Config config;
// Variables used to compare the variations loaded from the DB
char *GateNameLoad = "";
char GateNameOld[STRING_LEN];
char *ssidload = "";
char ssidOld[STRING_LEN];
char *passload = "";
char passOld[STRING_LEN];
char *MacAddressload = "";
char MacAddressOld[STRING_LEN];
char *local_IPload = "";
char local_IPOld[STRING_LEN];
char *gatewayload = "";
char gatewayOld[STRING_LEN];
char *subnetload = "";
char subnetOld[STRING_LEN];
char *primaryDNSload = "";
char primaryDNSOld[STRING_LEN];
char *secondaryDNSload = "";
char secondaryDNSOld[STRING_LEN];
char DBAddressParamold[STRING_LEN_LONG];
char *DBUserParamload = "";
char DBUserParamOLD[STRING_LEN];
char *DBPswParamload = "";
char DBPswParamlOld[STRING_LEN];
//**********  END json variable setting *************

// send an NTP request to the time server at the given address
void sendNTPpacket(const char *address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

unsigned long getNTPtime()
{
  // module returns a unsigned long time valus as secs since Jan 1, 1970
  // unix time or 0 if a problem encounted
  bool connectionOn;
#ifdef NoWiFi
  if (Ethernet.hardwareStatus() != EthernetNoHardware)
  {
    connectionOn = true;
  }
  else
  {
    connectionOn = false;
  }

#else
// Implement functions for WiFi here
#endif
  //only send data when connected
  if (connectionOn == true)
  {
    //initializes the UDP state
    //This initializes the transfer buffer
    Udp.begin(localPort);
    delay(10);
    sendNTPpacket(timeServer); // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(500);
    if (Udp.parsePacket())
    {
      DPRINTLN("udp packet received");
      DPRINTLN("");
      // We've received a packet, read the data from it
      Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, extract the two words:
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;
      // adjust time for timezone offset in secs +/- from UTC
      // WA time offset from UTC is +8 hours (28,800 secs)
      // + East of GMT
      // - West of GMT
      //unsigned long tzOffset = config.tzOffset;
      // WA local time
      unsigned long adjustedTime;
      time_t epoch_t = epoch;
      setTime(epoch_t);
      rtc.now(DateTime((uint32_t)epoch));
      return adjustedTime = epoch;
    }
    else
    {
      // were not able to parse the udp packet successfully
      // clear down the udp connection
      DPRINTLN("were not able to parse the udp packet successfully");
      Udp.stop();
      return 0; // zero indicates a failure
    }
    // not calling ntp time frequently, stop releases resources
    Udp.stop();
  }
  else
  {
    // network not connected
    return 0;
  }
}

bool DBConnect() //Function for connecting to the DB
{
  if (updateConnDB.justFinished())
  {
    //updateConnDB.restart();
    if (conn.connected())
    {
      updateConnDBDelay = 5000;
      updateConnDB.start(updateConnDBDelay);
    }
    else
    {
      if (updateConnDBDelay >= 1800000)
      {
        updateConnDBDelay = 1800000;
      }
      else
      {
        updateConnDBDelay = updateConnDBDelay + 1000;
        updateConnDB.start(updateConnDBDelay);
      }
      DPRINT("updateConnDBDelay= ");
      DPRINTLN(updateConnDBDelay);
      yield();
      conn.close();
      delay(50);
      if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
      {
        delay(500);

        DPRINT("try to Connected DB= ");
        DPRINTLN(config.DBAddressParam);
        DPRINT("DBConnected = ");
        DPRINTLN(DBConnected);
        if (config.Log == 1)
        {
          writeLog("Try to Connected DB");
        }
        return true;
      }
      else
      {
        DPRINTLN("Connection DB failed.");
        updateConnDBDelay = updateConnDBDelay + 1000;
        updateConnDB.start(updateConnDBDelay);
        DPRINT("updateConnDBDelay= ");
        DPRINTLN(updateConnDBDelay);
        if (config.Log == 1)
        {
          writeLog("Connection DB failed");
          return false;
        }
        DBConnected = false;
      }
    }
  }
}

bool BufAccessToDB()
{
  if ((ReteConnected == true) && (dnsReturn == true))
  {
    yield();
    conn.close();
    delay(50);
    if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
    {
      delay(500);
      DPRINTLN("Read SD");
      tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
      tft.setTextColor(TFT_BLACK);
      tft.setFreeFont(FF5);
      tft.setCursor((320 - tft.textWidth("sul Server attendere prego")) / 2, 90);
      tft.print(("Carico archivio transiti"));
      tft.setCursor((320 - tft.textWidth("sul Server attendere prego")) / 2, 110);
      tft.print(("sul Server attendere prego"));
      String message;
      File myFile = SD.open((filenameAccessBuf));
      if (myFile)
      {
        while (myFile.available())
        {
          message = myFile.readString();
          DPRINTLN(message);
        }
        // close the file:
        myFile.close();
      }
      else
      {
        // if the file didn't open, print an error:
        DPRINTLN("====================================================");
        DPRINTLN(String("             Opening file= ") + filenameAccessBuf + String(" error"));
        DPRINTLN("====================================================");
      }
      int PuntMessage = 0;
      int indexMessage = message.indexOf("\n");
      int indice = 0;
      while (indexMessage > 0)
      {
        String tokenA = message.substring(indexMessage, PuntMessage);
        PuntMessage = (indexMessage + 1);
        indexMessage = message.indexOf("\n", indexMessage + 1);
        int PuntTokenA = 0;
        int indexTokenA = tokenA.indexOf(",");
        int indiceToken = 0;
        int idGateSD = 0;
        long rfidValueSD = 0;
        int entrata_uscitaSD = 0;
        int ValidoSD = 0;
        String TimeSD = "";
        int rfidIdBadgeSD = 0;
        while (indexTokenA > 0)
        {
          String SubtokenA = tokenA.substring(indexTokenA, PuntTokenA);
          // DPRINT("SubStringA");
          // DPRINTLN(SubtokenA);
          PuntTokenA = (indexTokenA + 1);
          indexTokenA = tokenA.indexOf(",", indexTokenA + 1);
          //DPRINT("indice = "+String(indiceToken));
          switch (indiceToken)
          {
          case 0:
            idGateSD = SubtokenA.toInt();
            break;
          case 1:
            rfidValueSD = SubtokenA.toInt();
            break;
          case 2:
            entrata_uscitaSD = SubtokenA.toInt();
            break;
          case 3:
            ValidoSD = SubtokenA.toInt();
            break;
          case 4:
            TimeSD = SubtokenA;
            break;
          case 5:
            rfidIdBadgeSD = SubtokenA.toInt();
            break;
          default:
            break;
          }
          indiceToken++;
          int xcursore = ((320 - tft.textWidth("sul Server attendere prego")) / 2);
          int ycursor = 140;
          tft.fillRect(xcursore + (tft.textWidth("Invio transito nr. ")), ycursor - 21, 80, 25, TFT_WHITE);
          tft.setCursor(xcursore, ycursor);
          tft.print(("Invio transito nr. "));
          tft.print(indice / 6);
          indice++;
        }
        if (!conn.connected())
        {
          conn.close();
          delay(50);
          if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
          {
            delay(500);
            DPRINTLN("Try to Connected for update transit");
            DPRINTLN(config.DBAddressParam);
            DPRINT("DBConnected = ");
            DPRINTLN(DBConnected);
            if (config.Log == 1)
            {
              writeLog("Try to Connected for update transit");
            }
          }
          else
          {
            DPRINTLN("Connection for update transit failed.");
            if (config.Log == 1)
            {
              writeLog("Connection for update transit failed");
            }
            DBConnected = false;
            return false;
          }
        }
        if (conn.connected())
        {
          if (!Updatetransit(idGateSD, rfidValueSD, entrata_uscitaSD, ValidoSD, TimeSD, rfidIdBadgeSD))
          {
            DBConnected = false;
            if (statoLCD_BACKLIGHT == LOW)
            {
              digitalWrite(LCD_BACKLIGHT, HIGH);
              tftTimer.restart();
            }
            int xcursore = ((320 - tft.textWidth("sul Server attendere prego")) / 2);
            int ycursor = 140;
            tft.fillRect(xcursore + (tft.textWidth("Errore invio transito")), ycursor - 21, 80, 25, TFT_WHITE);
            timer[0] = prog[0]; // reset timer Display
            lastDrawLogo = 0;
            tft.setFreeFont(FF5);
            return false;
          }
        }
        else
        {
          DPRINTLN("\nConnect failed BufAccToDB");
          DBConnected = false;
          if (statoLCD_BACKLIGHT == LOW)
          {
            tftTimer.restart();
          }
          drawLogo();
          return false;
        }
      }
    }
  }
  else
  {
    DPRINTLN("\nConnect failed BufAccToDB");
    if (config.Log == 1)
    {
      writeLog("Connect failed BufAccToDB");
    }
    DBConnected = false;
    if (statoLCD_BACKLIGHT == LOW)
    {
      tftTimer.restart();
    }
    drawLogo();
    return false;
  }
  if (statoLCD_BACKLIGHT == LOW)
  {
    tftTimer.restart();
  }
  drawLogo();
  //conn.close();
  return true;
}

bool BufLogToDB()
{
  if ((ReteConnected == true) && (dnsReturn == true) && (DBConnected == true))
  {
    DPRINTLN("read SD");
    tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
    tft.setTextColor(TFT_BLACK);
    tft.setFreeFont(FF5);
    tft.setCursor((320 - tft.textWidth("sul Server attendere prego")) / 2, 90);
    tft.print(("Carico archivio Log"));
    tft.setCursor((320 - tft.textWidth("sul Server attendere prego")) / 2, 110);
    tft.print(("sul Server attendere prego"));
    String messageSD;
    File myFile = SD.open((filenameLog));
    if (myFile)
    {
      while (myFile.available())
      {
        messageSD = myFile.readString();
      }
      // close the file:
      myFile.close();
    }
    else
    {
      // if the file didn't open, print an error:
      DPRINTLN("====================================================");
      DPRINTLN(String("             Opening file= ") + filenameLog + String(" error"));
      DPRINTLN("====================================================");
    }
    int PuntMessage = 0;
    int indexMessage = messageSD.indexOf("\n");
    int indice = 0;
    while (indexMessage > 0)
    {
      String tokenA = messageSD.substring(indexMessage, PuntMessage);
      PuntMessage = (indexMessage + 1);
      indexMessage = messageSD.indexOf("\n", indexMessage + 1);
      int PuntTokenA = 0;
      int indexTokenA = tokenA.indexOf(",");
      int indiceToken = 0;
      int idGateSD = 0;
      String Date = "";
      String Message = "";
      while (indexTokenA > 0)
      {
        String SubtokenA = tokenA.substring(indexTokenA, PuntTokenA);
        PuntTokenA = (indexTokenA + 1);
        indexTokenA = tokenA.indexOf(",", indexTokenA + 1);
        switch (indiceToken)
        {
        case 0:
          idGateSD = SubtokenA.toInt();
          break;
        case 1:
          Date = SubtokenA;
          break;
        case 2:
          Message = SubtokenA;
          break;
        default:
          break;
        }
        indiceToken++;
        int xcursore = ((320 - tft.textWidth("sul Server attendere prego")) / 2);
        int ycursor = 140;
        tft.fillRect(xcursore + (tft.textWidth("Invio Log nr. ")), ycursor - 21, 80, 25, TFT_WHITE);
        tft.setCursor(xcursore, ycursor);
        tft.print(("Invio Log nr. "));
        tft.print(indice / 3);
        DPRINT("Send field n. ");
        DPRINT(indiceToken);
        DPRINT(" Log nr. ");
        DPRINTLN((indice / 3) + 1);

        if (indice >= 150)
        {
          DPRINT("indice= ");
          DPRINTLN(indice);
          DPRINTLN("File too large upload end");
          drawLogo();
          return true;
        }
        indice++;
      }
      DPRINT("indice = ");
      DPRINTLN(indice);
      if ((indice / 3 == 1) || (!conn.connected()))
      {
        DPRINT("indice = ");
        DPRINTLN(indice / 3);
        conn.close();
        delay(50);
        if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        //if (conn.connect(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam))
        {
          delay(500);

          DPRINTLN("DB Connected for update log");
          DPRINTLN(config.DBAddressParam);
          DPRINT("DBConnected = ");
          DPRINTLN(DBConnected);
          if (config.Log == 1)
          {
            writeLog("DB Connected for update log");
          }
        }
        else
        {
          DPRINTLN("Connection for update log failed.");
          if (config.Log == 1)
          {
            writeLog("Connection for update log failed");
          }
          DBConnected = false;
          return false;
        }
      }
      if (conn.connected())
      {
        if (!UpdateLog(idGateSD, Date, Message))
        {
          DBConnected = false;
          if (statoLCD_BACKLIGHT == LOW)
          {
            tftTimer.restart();
          }
          drawLogo();
          DPRINTLN("UpdateLog Failed");
          if (config.Log == 1)
          {
            writeLog("UpdateLog Failed");
          }
          return false;
          //}
        }
      }
      else
      {
        DPRINTLN("\nConnect failed BufLogToDB");
        DBConnected = false;
        if (statoLCD_BACKLIGHT == LOW)
        {
          tftTimer.restart();
        }
        drawLogo();
        return false;
      }
    }
  }
  else
  {
    DPRINTLN("\nConnect failed BufLogToDB");
    if (config.Log == 1)
    {
      writeLog("Connect failed BufLogToDB");
    }
    DBConnected = false;
    if (statoLCD_BACKLIGHT == LOW)
    {
      tftTimer.restart();
    }
    drawLogo();
    return false;
  }
  if (statoLCD_BACKLIGHT == LOW)
  {
    tftTimer.restart();
  }
  drawLogo();
  //conn.close();
  return true;
}

void infoScreen()
{
  analogWrite(WIO_BUZZER, 0);
  digitalWrite(beeper, HIGH);
  DPRINTLN("Infoscreen");
  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();
  tft.setTextSize(0);
  tft.setTextColor(ILI9341_CYAN, TFT_BLACK);
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setTextSize(1 / 1.5);
  int xstart = 10;
  int xstartTab = 135;
  int yStart = 20;
  int nextRow = 20;
  tft.setCursor(xstart, yStart);
  tft.print("Gate ID: ");
  tft.setCursor(xstartTab, yStart);
  tft.print(config.idGate);
  tft.setCursor(xstart, yStart + nextRow);
  tft.print("Gate Name: ");
  tft.setCursor(xstartTab, yStart + nextRow);
  tft.print(config.GateName);
  tft.setCursor(xstart, yStart + nextRow * 2);
  tft.print("IP Address:");
  tft.setCursor(xstartTab, yStart + nextRow * 2);
  tft.print(config.local_IP);
  tft.setCursor(xstart, yStart + nextRow * 3);
  tft.print("GW Address:");
  tft.setCursor(xstartTab, yStart + nextRow * 3);
  tft.print(config.gateway);
  tft.setCursor(xstart, yStart + nextRow * 4);
  tft.print("Mac Address:");
  tft.setCursor(xstartTab, yStart + nextRow * 4);
  tft.print(config.MacAddress);
  tft.setCursor(xstart, yStart + nextRow * 5);
  tft.print("DB Address:");
  tft.setCursor(xstartTab, yStart + nextRow * 5);
  tft.print(config.DBAddressParam);
  tft.setCursor(xstart, yStart + nextRow * 6);
  tft.print("tftTimeout:");
  tft.setCursor(xstartTab, yStart + nextRow * 6);
  tft.print(config.tftTimeout);
  tft.setCursor(xstart, yStart + nextRow * 7);
  tft.print("NTPUpDate: ");
  tft.setCursor(xstartTab, yStart + nextRow * 7);
  tft.print(config.NTPUpDate);
  tft.setCursor(xstart, yStart + nextRow * 8);
  tft.print("TransitUp: ");
  tft.setCursor(xstartTab, yStart + nextRow * 8);
  tft.print(config.TransitUpdate);
  tft.setCursor(xstart, yStart + nextRow * 9);
  tft.print("Log: ");
  tft.setCursor(xstartTab, yStart + nextRow * 9);
  tft.print(config.Log);
  tft.setCursor(xstartTab + 28, yStart + nextRow * 9);
  tft.print("-  UpDate Log: ");
  tft.setCursor(xstartTab + 28 + tft.textWidth("-  UpDate Log:   "), yStart + nextRow * 9);
  tft.print(config.DelLog);
  tft.setCursor(xstart, yStart + nextRow * 10);
  tft.print("Firmware Ver: ");
  tft.setCursor(xstartTab, yStart + nextRow * 10);
  tft.print(String(VER_FW));
  statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
  if (statoLCD_BACKLIGHT == LOW)
  {
    digitalWrite(LCD_BACKLIGHT, HIGH);
    tftTimer.restart();
  }
  delay(8000);
}

void BadgeFromSDtoLocal()
{
  // To avoid that the file loaded on string is too large, the badge list has been divided into three files

  DPRINTLN("Loading Badge from SD to RAM");
  String message;  // message read from the first file
  String message2; // message read from the second file
  String message3; // message read from the third file

  tabellaBadgelength = 0;
  File myFile = SD.open((filenameBadge1));
  if (myFile)
  {
    while (myFile.available())
    {
      message = myFile.readString();
    }
    // close the file:
    myFile.close();
  }
  else
  {
    // if the file didn't open, print an error:
    DPRINTLN("====================================================");
    DPRINTLN(String("             Opening file= ") + filenameBadge1 + String(" error"));
    DPRINTLN("====================================================");
  }
  message = message + ",";
  DPRINT("message Badge= ");
  DPRINTLN(message);
  DPRINT("Lunghezza= ");
  DPRINTLN(message.length());
  int PuntMessage = 0;
  int indexMessage = message.indexOf(",");
  int indice = 0;
  while (indexMessage > 0)
  {
    String tokenM = message.substring(indexMessage, PuntMessage);
    PuntMessage = (indexMessage + 1);
    indexMessage = message.indexOf(",", indexMessage + 1);
    tabellacodici[indice] = tokenM;
    // DPRINT("tokemM= ");
    // DPRINTLN(tokenM);
    indice++;
  }

  File myFile2 = SD.open((filenameBadge2));
  if (myFile2)
  {
    while (myFile2.available())
    {
      message2 = myFile2.readString();
    }
    // close the file:
    myFile2.close();
  }
  else
  {
    // if the file didn't open, print an error:
    DPRINTLN("====================================================");
    DPRINTLN(String("             Opening file= ") + filenameBadge2 + String(" error"));
    DPRINTLN("====================================================");
  }
  indice--;
  message2 = message2 + ",";
  DPRINT("message2 Badge= ");
  DPRINTLN(message2);
  DPRINT("Lunghezza= ");
  DPRINTLN(message2.length());
  int PuntMessage2 = 0;
  int indexMessage2 = message2.indexOf(",");
  while (indexMessage2 > 0)
  {
    String tokenM2 = message2.substring(indexMessage2, PuntMessage2);
    PuntMessage2 = (indexMessage2 + 1);
    indexMessage2 = message2.indexOf(",", indexMessage2 + 1);
    tabellacodici[indice] = tokenM2;
    indice++;
  }

  File myFile3 = SD.open((filenameBadge3));
  if (myFile3)
  {
    while (myFile3.available())
    {
      message3 = myFile3.readString();
    }
    // close the file:
    myFile3.close();
  }
  else
  {
    // if the file didn't open, print an error:
    DPRINTLN("====================================================");
    DPRINTLN(String("             Opening file= ") + filenameBadge3 + String(" error"));
    DPRINTLN("====================================================");
  }
  indice--;
  message3 = message3 + ",";
  DPRINT("message3 Badge= ");
  DPRINTLN(message3);
  DPRINT("Lunghezza= ");
  DPRINTLN(message3.length());
  int PuntMessage3 = 0;
  int indexMessage3 = message3.indexOf(",");
  while (indexMessage3 > 0)
  {
    String tokenM3 = message3.substring(indexMessage3, PuntMessage3);
    PuntMessage3 = (indexMessage3 + 1);
    indexMessage2 = message2.indexOf(",", indexMessage3 + 1);
    tabellacodici[indice] = tokenM3;
    indice++;
  }

  tabellaBadgelength = (indice / 6)+1;
  field = 6;

  DPRINT("tabellaBadgelength= ");
  DPRINTLN(tabellaBadgelength);
  DPRINT("field= ");
  DPRINTLN(field);
}

void ProfiliFromSDtoLocal()
{
  DPRINTLN("load profiles from SD to RAM");
  String message;
  tabellaDesProfililength = 0;
  File myFile = SD.open((filenameProfili));
  if (myFile)
  {
    while (myFile.available())
    {
      message = myFile.readString();
    }
    // close the file:
    myFile.close();
  }
  else
  {
    // if the file didn't open, print an error:
    DPRINTLN("====================================================");
    DPRINTLN(String("             Opening file= ") + filenameProfili + String(" error"));
    DPRINTLN("====================================================");
  }
  message = message + ",";
  DPRINT("message profili= ");
  DPRINTLN(message);
  DPRINT("Lunghezza= ");
  DPRINTLN(message.length());
  int PuntMessage = 0;
  int indexMessage = message.indexOf(",");
  int indice = 0;
  while (indexMessage >= 0)
  {
    String tokenM = message.substring(indexMessage, PuntMessage);
    PuntMessage = (indexMessage + 1);
    indexMessage = message.indexOf(",", indexMessage + 1);
    tabellaDesProfili[indice] = tokenM;
    indice++;
  }
  tabellaDesProfililength = (indice) / 2;
  fieldP = 2;
  DPRINT("tabellaDesProfililength= ");
  DPRINTLN(tabellaDesProfililength);
  DPRINT("fieldP= ");
  DPRINTLN(fieldP);
}

void DesTimeFromSDtoLocal()
{
  DPRINTLN("load time bands from SD to RAM");
  String message;
  tabellaDesTimelength = 0;
  File myFile = SD.open((filenameDesTime));
  if (myFile)
  {
    while (myFile.available())
    {
      message = myFile.readString();
    }
    // close the file:
    myFile.close();
  }
  else
  {
    // if the file didn't open, print an error:
    DPRINTLN("====================================================");
    DPRINTLN(String("             Opening file= ") + filenameDesTime + String(" error"));
    DPRINTLN("====================================================");
  }
  message = message + ",";
  DPRINT("message DesTime= ");
  DPRINTLN(message);
  DPRINT("Length= ");
  DPRINTLN(message.length());
  int PuntMessage = 0;
  int indexMessage = message.indexOf(",");
  int indice = 0;
  while (indexMessage >= 0)
  {
    String tokenM = message.substring(indexMessage, PuntMessage);
    PuntMessage = (indexMessage + 1);
    indexMessage = message.indexOf(",", indexMessage + 1);
    tabellaDesTime[indice] = tokenM;
    indice++;
  }
  tabellaDesTimelength = (indice) / 4;
  fieldT = 4;
  DPRINT("tabellaDesTimelength= ");
  DPRINTLN(tabellaDesTimelength);
  DPRINT("fieldT= ");
  DPRINTLN(fieldT);
}

bool BadgeFromDBtoLocal(bool loopRunning)
{
  DPRINTLN("====================================================");
  DPRINTLN("              Running SELECT BADGE                  ");
  analogWrite(WIO_BUZZER, 0);
  digitalWrite(beeper, HIGH);
  String bufBadge = "";
  // Initiate the query class instance
  MySQL_Query query_mem = MySQL_Query(&conn);
  byte xcur = 25;
  byte ycur = 80;
  tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
  tft.setFreeFont(FF5);
  DPRINTLN("Update badge table");
  if (loopRunning == 1)
  {
    tft.drawRect(xcur - 15, ycur - 30, 300, 150, ILI9341_BLUE);
  }
  tft.setCursor(xcur, ycur + 20);
  tft.print("Attendere prego");
  tft.setCursor(xcur, ycur + 40);
  tft.print("Aggiornamento Badge");
  tft.setCursor(xcur, ycur + 60);
  tft.print("nr.");
  // Execute the query
  char INSEERTqueryBadge[] = "SELECT %s.Badge.CodiceBadge,  %s.Badge.IdBadge, %s.Badge.Cognome, %s.Badge.Nome, %s.Badge.IdProfilo, %s.Badge.Uid_Dec FROM %s.Badge WHERE %s.Badge.Valido = 1 ORDER BY IdBadge LIMIT %d, %d";
  char queryBadge[300];
  int SubSTRQ = 9;
  int ciclo = 0;
  int badge_module = 55;
  int badgetot = badge_module * SubSTRQ;
  int startBadge = 0;
  int stopBadge = 140;
  int tab = 0;
  for (ciclo = 0; ciclo < SubSTRQ; ciclo++)
  {
    switch (ciclo)
    {
    case 0:
      startBadge = 0;
      stopBadge = badge_module;
      break;
    case 1:
      startBadge = (badge_module);
      stopBadge = badge_module;
      break;
    case 2:
      startBadge = (badge_module * ciclo);
      stopBadge = badge_module;
      break;
    case 3:
      startBadge = (badge_module * ciclo);
      stopBadge = badge_module;
      break;
    case 4:
      startBadge = (badge_module * ciclo);
      stopBadge = badge_module;
      break;
    case 5:
      startBadge = (badge_module * ciclo);
      stopBadge = badge_module;
      break;
    case 6:
      startBadge = (badge_module * ciclo);
      stopBadge = badge_module;
      break;
    case 7:
      startBadge = (badge_module * ciclo);
      stopBadge = badge_module;
      break;
    case 8:
      startBadge = (badge_module * ciclo);
      stopBadge = badge_module;
      break;
    case 9:
      startBadge = (badge_module * ciclo);
      stopBadge = badge_module;
      break;
    default:
      break;
    }
    DPRINT("startBadge= ");
    DPRINTLN(startBadge);
    DPRINT("stopBadge= ");
    DPRINTLN(stopBadge);

    snprintf(queryBadge, sizeof(queryBadge), INSEERTqueryBadge, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, startBadge, stopBadge);
    delay(1);
    DPRINT("queryBadge= ");
    DPRINTLN(queryBadge);
    DPRINT("CicloQry= ");
    DPRINTLN(ciclo);
    if (!conn.connected())
    {
      DBConnect();
      delay(500);
    }
    if (conn.connected())
    {
      delay(50);
      if (!query_mem.execute(queryBadge))
      {
        DPRINTLN("====================================================");
        DPRINTLN("               Query Badge error ");
        DPRINTLN("====================================================");
        DPRINTLN("Query Badge error");
        if (config.Log == 1)
        {
          writeLog("Query Badge error");
        }
        DBConnected = false;
        ServerOnOff = false;
        query_mem.close();
        if (loopRunning == 1)
        {
          tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
          if (statoLCD_BACKLIGHT == LOW)
          {
            tftTimer.restart();
          }
          drawLogo();
        }
        LoadDBBadge = false;
        return false;
      }
      else
      {
        DBConnected = true;
        ServerOnOff = true;
        // Fetch the columns and print them
        column_names *cols = query_mem.get_columns();
        // Read the rows and print them
        row_values *row = NULL;
        //int tab = startBadge;
        bufBadge = "";
        do
        {
          row = query_mem.get_next_row();
          if (row != NULL)
          {
            tabellaBadgelength++;
            field = cols->num_fields;

            for (int f = 0; f < cols->num_fields; f++)
            {
              if (f < cols->num_fields)
              {

                tabellacodici[tab] = (row->values[f]);
                tab++;
              }
              delay(1);
            }
            tft.fillRect(xcur + tft.textWidth("nr. ") - 1, ycur + 47, 55, 25, TFT_WHITE);
            tft.setCursor(xcur + tft.textWidth("nr. "), ycur + 60);
            tft.print(tab / field);
          }
        } while (row != NULL);
        query_mem.close();
      }
    }
    tabellaBadgelength = tab / field;
  }
  if (DBConnect)
  {
    ciclo = 0;
    float StopTBLBDL = tabellaBadgelength / (badge_module + 1);
    int SubSTR = 0;
    switch (tabellaBadgelength / (badge_module + 1))
    {
    case 0:
      SubSTR = 1;
      break;
    case 1:
      SubSTR = 2;
      break;
    case 2:
      SubSTR = 3;
      break;
    case 3:
      SubSTR = 4;
      break;
    case 4:
      SubSTR = 5;
      break;
    case 5:
      SubSTR = 6;
      break;
    case 6:
      SubSTR = 7;
      break;
    case 7:
      SubSTR = 8;
      break;
    case 8:
      SubSTR = 9;
      break;
    case 9:
      SubSTR = 10;
      break;
    default:
      break;
    }

    for (ciclo == 0; ciclo < SubSTR; ciclo++)
    {
      bufBadge = "";
      DPRINTLN("======================================================");
      DPRINT("          Scrivo File Badge Ciclo= ");
      DPRINTLN(ciclo);
      DPRINTLN("======================================================");
      switch (ciclo)
      {
      case 0:
        startBadge = 0;
        if (StopTBLBDL < 1)
        {
          stopBadge = tabellaBadgelength * field;
        }
        else
        {
          stopBadge = badge_module * field;
        }
        break;
      case 1:
        startBadge = badge_module * field;
        if (StopTBLBDL < 2)
        {
          stopBadge = tabellaBadgelength * field;
        }
        else
        {
          stopBadge = badge_module * 2 * field;
        }
        break;
      case 2:
        startBadge = badge_module * 2 * field;
        if (StopTBLBDL < 3)
        {
          stopBadge = tabellaBadgelength * field;
        }
        else
        {
          stopBadge = badge_module * 3 * field;
        }
        break;
      case 3:
        startBadge = badge_module * 3 * field;
        if (StopTBLBDL < 4)
        {
          stopBadge = tabellaBadgelength * field;
        }
        else
        {
          stopBadge = badge_module * 4 * field;
        }
        break;
      case 4:
        startBadge = badge_module * 4 * field;
        if (StopTBLBDL < 5)
        {
          stopBadge = tabellaBadgelength * field;
        }
        else
        {
          stopBadge = badge_module * 5 * field;
        }
        break;
      case 5:
        startBadge = badge_module * 5 * field;
        if (StopTBLBDL < 6)
        {
          stopBadge = tabellaBadgelength * field;
        }
        else
        {
          stopBadge = badge_module * 6 * field;
        }
        break;
      case 6:
        startBadge = badge_module * 6 * field;
        if (StopTBLBDL < 7)
        {
          stopBadge = tabellaBadgelength * field;
        }
        else
        {
          stopBadge = badge_module * 7 * field;
        }
        break;
      case 7:
        startBadge = badge_module * 7 * field;
        if (StopTBLBDL < 8)
        {
          stopBadge = tabellaBadgelength * field;
        }
        else
        {
          stopBadge = badge_module * 8 * field;
        }
        break;
      case 8:
        startBadge = badge_module * 8 * field;
        if (StopTBLBDL < 9)
        {
          stopBadge = tabellaBadgelength * field;
        }
        else
        {
          stopBadge = badge_module * 9 * field;
        }
        break;
      case 9:
        startBadge = badge_module * 9 * field;
        if (StopTBLBDL < 10)
        {
          stopBadge = tabellaBadgelength * field;
        }
        else
        {
          stopBadge = badge_module * 10 * field;
        }
        break;
      default:
        break;
      }
      for (int fbadg = startBadge; fbadg < stopBadge; fbadg++)
      {
        bufBadge = bufBadge + tabellacodici[fbadg];
        if (fbadg != (stopBadge))
        {
          bufBadge = bufBadge + ",";
        }
      }
      DPRINT("BUfBadge=  ");
      DPRINTLN(bufBadge);
      tft.fillRect(xcur + tft.textWidth("nr. ") - 1, ycur + 47, 55, 25, TFT_WHITE);
      tft.setCursor(xcur + tft.textWidth("nr. "), ycur + 60);
      tft.print(tabellaBadgelength);
      DPRINTLN("write badge table to SD");
      tft.setCursor(xcur, ycur + 85);
      tft.print("Aggiorno file badge su SD");
      delay(100);
      if (ciclo == 0 || ciclo == 1 || ciclo == 2 || ciclo == 3)
      {
        if (testFileIO(SD, filenameBadge1))
        {
          if (ciclo == 0)
          {
            if (deleteFile(SD, filenameBadge1) == true)
            {
              delay(300);
              writeFile(SD, filenameBadge1, bufBadge.c_str());
            }
            else
            {
              if (loopRunning == 1)
              {
                tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
                if (statoLCD_BACKLIGHT == LOW)
                {
                  tftTimer.restart();
                }
                drawLogo();
              }
              DPRINTLN("File delete error");
              return false;
            }
          }
          else if (ciclo == 1 || ciclo == 2 || ciclo == 3)
          {
            appendFile(SD, filenameBadge1, bufBadge.c_str());
          }
        }
        else
        {
          appendFile(SD, filenameBadge1, bufBadge.c_str());
        }
      }
      if (ciclo == 4 || ciclo == 5 || ciclo == 6 || ciclo == 7)
      {
        if (testFileIO(SD, filenameBadge2))
        {
          if (ciclo == 4)
          {
            if (deleteFile(SD, filenameBadge2) == true)
            {
              delay(300);
              writeFile(SD, filenameBadge2, bufBadge.c_str());
            }
            else
            {
              if (loopRunning == 1)
              {
                tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
                if (statoLCD_BACKLIGHT == LOW)
                {
                  tftTimer.restart();
                }
                drawLogo();
              }
              DPRINTLN("File delete error");
              return false;
            }
          }
          else if (ciclo == 5 || ciclo == 6 || ciclo == 7)
          {
            appendFile(SD, filenameBadge2, bufBadge.c_str());
          }
        }
        else
        {
          appendFile(SD, filenameBadge2, bufBadge.c_str());
        }
      }
      if (ciclo == 8 || ciclo == 9 || ciclo == 10)
      {
        if (testFileIO(SD, filenameBadge3))
        {
          if (ciclo == 8)
          {
            if (deleteFile(SD, filenameBadge3) == true)
            {
              delay(300);
              writeFile(SD, filenameBadge3, bufBadge.c_str());
            }
            else
            {
              if (loopRunning == 1)
              {
                tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
                if (statoLCD_BACKLIGHT == LOW)
                {
                  tftTimer.restart();
                }
                drawLogo();
              }
              DPRINTLN("File delete error");
              return false;
            }
          }
          else if (ciclo == 9 || ciclo == 10)
          {
            appendFile(SD, filenameBadge3, bufBadge.c_str());
          }
        }
        else
        {
          appendFile(SD, filenameBadge3, bufBadge.c_str());
        }
      }
    }
    if (loopRunning == 1)
    {
      tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
      if (statoLCD_BACKLIGHT == LOW)
      {
        tftTimer.restart();
      }
      drawLogo();
    }
    //conn.close();
    return true;
  }
}

bool ProfiliFromDBtoLocal(bool loopRunning)
{
  DPRINTLN("====================================================");
  DPRINTLN("             Runnig SELECT Profili");
  DPRINTLN("====================================================");
  analogWrite(WIO_BUZZER, 0);
  digitalWrite(beeper, HIGH);
  String bufProfili = "";
  MySQL_Query query_mem = MySQL_Query(&conn);
  byte xcur = 25;
  byte ycur = 80;
  tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
  tft.setFreeFont(FF5);
  DPRINTLN("Update Profiles");
  if (loopRunning == 1)
  {
    tft.drawRect(xcur - 15, ycur - 30, 300, 150, ILI9341_BLUE);
  }
  tft.setCursor(xcur, ycur + 20);
  tft.print("Attendere prego");
  tft.setCursor(xcur, ycur + 40);
  tft.print("Aggiorn. Profilo");
  tft.setCursor(xcur, ycur + 60);
  tft.print("nr.");
  // // Execute the query
  char INSERTqueryDesProfili[] = "SELECT %s.DesProfili.IdProfilo, %s.DesProfili.DayNumber FROM %s.DesProfili";
  char queryDesProfili[150];
  snprintf(queryDesProfili, sizeof(queryDesProfili), INSERTqueryDesProfili, config.DataBaseName, config.DataBaseName, config.DataBaseName);
  delay(5);
  if (!query_mem.execute(queryDesProfili))
  {
    DPRINTLN("====================================================");
    DPRINTLN("               Query Profili error ");
    DPRINTLN("====================================================");
    DBConnected = false;
    ServerOnOff = false;
    query_mem.close();
    //conn.close();
    if (loopRunning == 1)
    {
      tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
      if (statoLCD_BACKLIGHT == LOW)
      {
        tftTimer.restart();
      }
      drawLogo();
    }
    LoadDBProfili = false;
    return false;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    // // Fetch the columns and print them
    column_names *colsP = query_mem.get_columns();

    // Read the rows and print them
    row_values *rowP = NULL;

    int tabP = 0;
    bufProfili = "";
    do
    {
      rowP = query_mem.get_next_row();
      if (rowP != NULL)
      {
        tabellaDesProfililength++;
        fieldP = colsP->num_fields;

        for (int fP = 0; fP < colsP->num_fields; fP++)
        {
          if (fP < colsP->num_fields)
          {
            tabellaDesProfili[tabP] = (rowP->values[fP]);
            tabP++;
          }
        }
        tft.fillRect(xcur + tft.textWidth("nr. ") - 1, ycur + 47, 55, 25, TFT_WHITE);
        tft.setCursor(xcur + tft.textWidth("nr. "), ycur + 60);
        tft.print(tabellaDesProfililength);
      }
    } while (rowP != NULL);
    query_mem.close();
    //conn.close();
    for (int fProf = 0; fProf < tabellaDesProfililength * fieldP; fProf++)
    {
      bufProfili = bufProfili + tabellaDesProfili[fProf];
      if (fProf != (tabellaDesProfililength * fieldP) - 1)
      {
        bufProfili = bufProfili + ",";
      }
    }
    DPRINT("tabellaDesProfililength= ");
    DPRINTLN(tabellaDesProfililength);
    DPRINT("fieldP= ");
    DPRINTLN(fieldP);
    delay(30);
    if (testFileIO(SD, filenameProfili))
    {
      if (deleteFile(SD, filenameProfili) == true)
      {
        delay(200);
        writeFile(SD, filenameProfili, bufProfili.c_str());
      }
      else
      {
        if (loopRunning == 1)
        {
          tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
          if (statoLCD_BACKLIGHT == LOW)
          {
            tftTimer.restart();
          }
          drawLogo();
        }

        return false;
      }
    }
    else
    {
      writeFile(SD, filenameProfili, bufProfili.c_str());
    }

    if (loopRunning == 1)
    {
      tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
      if (statoLCD_BACKLIGHT == LOW)
      {
        tftTimer.restart();
      }
      drawLogo();
    }

    return true;
  }
}

bool DesTimeFromDBtoLocal(bool loopRunning)
{
  DPRINTLN("====================================================");
  DPRINTLN("              Running SELECT DES TIME               ");
  DPRINTLN("====================================================");
  analogWrite(WIO_BUZZER, 0);
  digitalWrite(beeper, HIGH);
  String bufTime = "";
  MySQL_Query query_memT = MySQL_Query(&conn);
  byte xcur = 25;
  byte ycur = 80;
  tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
  tft.setFreeFont(FF5);
  DPRINTLN("update DesTime");

  if (loopRunning == 1)
  {
    tft.drawRect(xcur - 15, ycur - 30, 300, 150, ILI9341_BLUE);
  }
  tft.setCursor(xcur, ycur + 20);
  tft.print("Attendere prego");
  tft.setCursor(xcur, ycur + 40);
  tft.println(("Aggiorn. fascia oraria"));
  tft.setCursor(xcur, ycur + 60);
  tft.print("nr.");
  char INSERTqueryDesTime[] = "SELECT %s.DesTime.StartTime, %s.DesTime.StopTime, %s.DesTime.IdDefProfili, %s.DesTime.IdGate FROM %s.DesTime ";
  char queryDesTime[200];
  snprintf(queryDesTime, sizeof(queryDesTime), INSERTqueryDesTime, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName);
  delay(5);
  // // Execute the query
  DPRINTLN("queryDesTime:");
  DPRINTLN(queryDesTime);
  if (!query_memT.execute(queryDesTime))
  {
    DPRINTLN("====================================================");
    DPRINTLN("               Query DES TIME error ");
    DPRINTLN("====================================================");
    DBConnected = false;
    ServerOnOff = false;
    if (config.Log == 1)
    {
      writeLog("Query DES TIME error");
    }
    query_memT.close();
    //conn.close();
    if (loopRunning == 1)
    {
      tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
      if (statoLCD_BACKLIGHT == LOW)
      {
        tftTimer.restart();
      }
      drawLogo();
    }
    LoadDBTime = false;
    return false;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    // // Fetch the columns and print them
    column_names *colsT = query_memT.get_columns();
    // Read the rows and print them
    row_values *rowT = NULL;
    int tabT = 0;
    bufTime = "";
    do
    {
      rowT = query_memT.get_next_row();
      if (rowT != NULL)
      {
        tabellaDesTimelength++;
        fieldT = colsT->num_fields;
        for (int fT = 0; fT < colsT->num_fields; fT++)
        {
          if (fT < colsT->num_fields)
          {
            tabellaDesTime[tabT] = (rowT->values[fT]);
            tabT++;
          }
        }
        tft.fillRect(xcur + tft.textWidth("nr. ") - 1, ycur + 47, 55, 25, TFT_WHITE);
        tft.setCursor(xcur + tft.textWidth("nr. "), ycur + 60);
        tft.print(tabellaDesTimelength);
      }
    } while (rowT != NULL);
    query_memT.close();
    //conn.close();
    for (int fTime = 0; fTime < tabellaDesTimelength * fieldT; fTime++)
    {
      bufTime = bufTime + tabellaDesTime[fTime];
      if (fTime != (tabellaDesTimelength * fieldT) - 1)
      {
        bufTime = bufTime + ",";
      }
    }
    DPRINT("tabellaDesTimelength= ");
    DPRINTLN(tabellaDesTimelength);
    DPRINT("fieldT= ");
    DPRINTLN(fieldT);
    if (testFileIO(SD, filenameDesTime))
    {
      if (deleteFile(SD, filenameDesTime) == true)
      {
        delay(200);
        writeFile(SD, filenameDesTime, bufTime.c_str());
        if (loopRunning == 1)
        {
          tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
          if (statoLCD_BACKLIGHT == LOW)
          {
            tftTimer.restart();
          }
          drawLogo();
        }
        else
        {
          tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
          if (loopRunning == 1)
          {
            tft.drawRect(xcur - 15, ycur - 30, 300, 150, ILI9341_BLUE);
          }
          tft.setCursor(xcur, ycur + 5);
          tft.print("Attendere prego");
          tft.setCursor(xcur, ycur + 25);
          tft.println(("Verifico agg. Configuraz."));
        }
        return true;
      }
      else
      {
        if (loopRunning == 1)
        {
          tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
          if (statoLCD_BACKLIGHT == LOW)
          {
            tftTimer.restart();
          }
          drawLogo();
          return false;
        }
        else
        {
          tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
          if (loopRunning == 1)
          {
            tft.drawRect(xcur - 15, ycur - 30, 300, 150, ILI9341_BLUE);
          }
          tft.setCursor(xcur, ycur + 15);
          tft.print("errore cancellazione.");
          tft.setCursor(xcur, ycur + 32);
          tft.println(("File. Configuraz. KO"));
        }
      }
    }
    else
    {
      writeFile(SD, filenameDesTime, bufTime.c_str());
    }
    if (loopRunning == 1)
    {
      tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
      if (statoLCD_BACKLIGHT == LOW)
      {
        tftTimer.restart();
      }
      drawLogo();
      return false;
    }
  }
}

void UpdateFW()
{
  DPRINTLN("====================================================");
  DPRINTLN("              Running UPDATE FW                     ");
  DPRINTLN("====================================================");
  // Initiate the query class instance
  MySQL_Query query_memUP = MySQL_Query(&conn);
  // Execute the query
  char queryINSERTDATA[] = "UPDATE %s.gate SET VerFW = '%s' WHERE IdGate = %d";
  char queryVerFW[80];
  snprintf(queryVerFW, sizeof(queryVerFW), queryINSERTDATA, config.DataBaseName, VER_FW, config.idGate);
  delay(5);
  DPRINT("queryVerFW= ");
  DPRINTLN(queryVerFW);
  if (!query_memUP.execute(queryVerFW))
  {
    DPRINTLN("====================================================");
    DPRINTLN(("               Query UPDATE FW error "));
    DPRINTLN("====================================================");
    DBConnected = false;
    ServerOnOff = false;
    query_memUP.close();
    return;
  }
  else
  {
    DPRINTLN("Quering UPDATE FW OK");
    DBConnected = true;
    ServerOnOff = true;
    query_memUP.close();
    //conn.close();
  }
}

bool KeepAlive()
{
  DPRINTLN("====================================================");
  DPRINTLN("              Running UPDATE KeepAlive              ");
  DPRINTLN("====================================================");

  analogWrite(WIO_BUZZER, 0);
  digitalWrite(beeper, HIGH);
  // Initiate the query class instance
  MySQL_Query query_memKA = MySQL_Query(&conn);
  // Execute the query
  char WRITE_DATA_UpKA[] = "UPDATE %s.chkbadgevar SET KeepAlive = '%s' WHERE %s.chkbadgevar.IdChk = %d";
  char queryUpKA[135];
  DateTime now = rtc.now();
  time_t utc = now.get_time_t();
  time_t local = myTZ.toLocal(utc, &tcr);
  //String TimeNow = String(now.year(), DEC) + "-" + String(now.month(), DEC) + "-" + String(now.day(), DEC) + " " + String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC);
  String TimeNow = printLocalTime(local);
  char charBufTimeNow[50] = "";
  TimeNow.toCharArray(charBufTimeNow, 50);
  //DPRINTLN(TimeNow);
  snprintf(queryUpKA, sizeof(queryUpKA), WRITE_DATA_UpKA, config.DataBaseName, charBufTimeNow, config.DataBaseName, config.idGate);
  delay(5);
  DPRINT("queryUpKA= ");
  DPRINTLN(queryUpKA);
  if (!query_memKA.execute(queryUpKA))
  {
    DPRINTLN("====================================================");
    DPRINTLN(("               Query UPDATE KeepAlive error "));
    DPRINTLN("====================================================");
    DBConnected = false;
    ServerOnOff = false;
    query_memKA.close();
    //conn.close();
    return false;
  }
  else
  {
    DPRINTLN("Quering UPDATE KeepAlive OK");
    DBConnected = true;
    ServerOnOff = true;
    query_memKA.close();
    //conn.close();
    return true;
  }
}

bool Updatetransit(int idGate, long RFIDVALUE, int ENTRATA_USCITA, int VALIDO, String Time, int RfidIdBadge)
{
  DPRINTLN("====================================================");
  DPRINTLN("              Running UPDATE Transit                ");
  DPRINTLN("====================================================");
  analogWrite(WIO_BUZZER, 0);
  digitalWrite(beeper, HIGH);
  // DateTime now = rtc.now();
  // Initiate the query class instance
  MySQL_Query query_memUPT = MySQL_Query(&conn);
  // char queryINSERTDATATRANSIT[] = "INSERT %s.transazioni SET device = %d, CodiceBadge = %d, entrata_uscita = %d, Valido = %d, date= FROM_UNIXTIME (%d), IdBadge = %d";
  char queryINSERTDATATRANSIT[] = "INSERT %s.transazioni SET device = %d, CodiceBadge = %d, entrata_uscita = %d, Valido = %d, date= '%s', IdBadge = %d";
  DPRINT("Time= ");
  DPRINTLN(Time);
  char queryInsertTransit[200];
  char charBufDate[50];
  Time.toCharArray(charBufDate, 50);

  snprintf(queryInsertTransit, 150, queryINSERTDATATRANSIT, config.DataBaseName, idGate, RFIDVALUE, ENTRATA_USCITA, VALIDO, charBufDate, RfidIdBadge);
  delay(5);
  DPRINT("queryInsertTransit= ");
  DPRINTLN(queryInsertTransit);
  if (!query_memUPT.execute(queryInsertTransit))
  {
    DPRINTLN("====================================================");
    DPRINTLN(("               Query UPDATE Transit error "));
    DPRINTLN("====================================================");
    DBConnected = false;
    ServerOnOff = false;
    if (config.Log == 1)
    {
      writeLog("Query UPDATE Transit error");
    }
    query_memUPT.close();
    //conn.close();
    return false;
  }
  else
  {
    DPRINTLN("Quering UPDATE Transit OK");
    DBConnected = true;
    ServerOnOff = true;
    query_memUPT.close();
    return true;
  }
}

bool UpdateLog(int idGate, String DateSD, String MessageSD)
{
  DPRINTLN("====================================================");
  DPRINTLN("              Running UPDATE Log                    ");
  DPRINTLN("====================================================");
  analogWrite(WIO_BUZZER, 0);
  digitalWrite(beeper, HIGH);
  // DateTime now = rtc.now();
  // Initiate the query class instance
  MySQL_Query query_memUPL = MySQL_Query(&conn);
  char queryInsertLog[140];
  char queryINSERTDATALOG[] = "INSERT %s.Log SET Gate = %d, Date = '%s', Message = '%s'";
  char charBufDate[50];
  DateSD.toCharArray(charBufDate, 50);
  char charBufMessage[250];
  MessageSD.toCharArray(charBufMessage, 250);
  snprintf(queryInsertLog, 250, queryINSERTDATALOG, config.DataBaseName, idGate, charBufDate, charBufMessage);
  delay(20);
  DPRINT("queryInsertLog= ");
  DPRINTLN(queryInsertLog);
  if (!query_memUPL.execute(queryInsertLog))
  {
    DPRINTLN("====================================================");
    DPRINTLN(("               Query UPDATE Transit error "));
    DPRINTLN("====================================================");
    DBConnected = false;
    ServerOnOff = false;
    if (config.Log == 1)
    {
      writeLog("Query UPDATE Log error");
    }
    query_memUPL.close();
    //conn.close();
    return false;
  }
  else
  {
    DPRINTLN("Quering UPDATE Log OK");
    DBConnected = true;
    ServerOnOff = true;
    query_memUPL.close();
    return true;
  }
}

void readBadgeTable()
{
  DPRINTLN("\nTabella codici");
  DPRINTLN("tabellaBadgelength * field=" + String(tabellaBadgelength * field));
  for (int t = 0; t < (tabellaBadgelength * field); t++)
  {
    DPRINT(tabellacodici[t]);
    DPRINT(",");
    if ((t + 1) % field == 0)
    {
      DPRINTLN("");
    }
  }
  DPRINTLN("end read badge table");
}

void readProfilesTable()
{
  DPRINTLN("\nTabella Profili");
  DPRINTLN("tabellaDesProfililength =" + String(tabellaDesProfililength * fieldP));
  for (int t = 0; t < (tabellaDesProfililength * fieldP); t++)
  {
    DPRINT(tabellaDesProfili[t]);
    DPRINT(",");
    if ((t + 1) % fieldP == 0)
    {
      DPRINTLN("");
    }
  }
}

void readTimeTable()
{
  DPRINTLN("\ntabellaDesTime =" + String(tabellaDesTimelength * fieldT));
  for (int t = 0; t < (tabellaDesTimelength * fieldT); t++)
  {
    DPRINT(tabellaDesTime[t]);
    DPRINT(",");
    if ((t + 1) % fieldT == 0)
    {
      DPRINTLN("");
    }
  }
}

void drawLogo()
{
  tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK);
  tft.setFreeFont(FF5);
  tft.drawXBitmap(xlogo, ylogo, LogoISD, LogoISD_width, LogoISD_height, TFT_BLACK);
  tft.setCursor(xlogo + 85, ylogo + 55);
  tft.print("Avvicinare Badge");
  tft.setCursor(xlogo + 85, ylogo + 75);
  tft.print("al lettore");
}

String printLocalTime(time_t t)
{
  DateTime now = rtc.now();
  time_t utc = now.get_time_t();
  time_t local = myTZ.toLocal(utc, &tcr);
  char bufLocalTime[25];
  sprintf(bufLocalTime, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
          year(t), month(t), day(t), hour(t), minute(t), second(t));
  return bufLocalTime;
}

void printDateTime(time_t t, const char *tz)
{
  char buf[32];
  char m[4]; // temporary storage for month string
  strcpy(m, monthShortStr(month(t)));
  sprintf(buf, "%.2d:%.2d:%.2d %s %.2d %s %d %s",
          hour(t), minute(t), second(t), dayShortStr(weekday(t)), day(t), m, year(t), tz);
  DPRINTLN(buf);
}

boolean checkCard() //rivedere da quì con file r-controllo-accessi-rfid-password-sd.ino
{
  waitingForKey = true;
  isProfiloOk = 0;
  isBadgeOk = 0;
  String gateTotOk = "";
  DPRINTLN("looking for a badge code");
  DPRINT("table badge length = ");
  DPRINTLN(tabellaBadgelength * field);
  for (indiceTabBadge = 0; indiceTabBadge <= tabellaBadgelength * field; indiceTabBadge += field) //scorro tabella Badge
  {
    if (atoi((tabellacodici[indiceTabBadge]).c_str()) == rfidValue) // ricerco badge letto nella tabella
    {
      DPRINT("Found Code badge= ");
      DPRINTLN((tabellacodici[indiceTabBadge]).c_str());
      isBadgeOk = 1; // set Badge ok = 1 if found a code
      rfidIdBadge = (tabellacodici[indiceTabBadge + 1]).toInt();
      rfidCognome = tabellacodici[indiceTabBadge + 2];
      DPRINT("rfidCognome= ");
      DPRINTLN(rfidCognome);
      rfidNome = tabellacodici[indiceTabBadge + 3];
      String ProfiloSelect = tabellacodici[indiceTabBadge + 4]; // save the corresponding profile
      rfidUIDdec = tabellacodici[indiceTabBadge + 5];

      DPRINT("rfidUIDdec= ");
      DPRINTLN(rfidUIDdec);

      DateTime now = rtc.now(); // store the current date and time
      time_t utc = now.get_time_t();
      time_t local = myTZ.toLocal(utc, &tcr);
      byte hh = (hour(local)); // I store the time in the variable
      byte mm = (minute(local));
      String DofW = daysOfTheWeekN[now.dayOfTheWeek()]; // store the day of the week in the variable DofW

      for (indiceTabProfili = 0; indiceTabProfili < tabellaDesProfililength * fieldP; indiceTabProfili++) // I scroll through the DesProfili table
      {
        if ((indiceTabProfili == 0) || (indiceTabProfili % fieldP == 0)) // search only in the last field
        {
          if (atoi((tabellaDesProfili[indiceTabProfili]).c_str()) == atoi(ProfiloSelect.c_str())) // search for all occurrences of the selected profile
          {
            if (atoi((tabellaDesProfili[indiceTabProfili + 1]).c_str()) == atoi(DofW.c_str())) // If the current day of the week is present in one of the occurrences, I go ahead
            {
              DPRINT("Profilo =");
              DPRINT(ProfiloSelect);
              DPRINTLN(" time ok");

              for (indiceTabTime = 0; indiceTabTime < tabellaDesTimelength * fieldT; indiceTabTime++) // I scroll through the DesTime table
              {
                if ((indiceTabTime == 0) || (indiceTabTime % fieldT == 0))
                {
                  if (atoi(tabellaDesTime[indiceTabTime + 2].c_str()) == atoi(ProfiloSelect.c_str()))
                  {
                    String tabDesTimeStartTot = tabellaDesTime[indiceTabTime];
                    String tabDesTimeStopTot = tabellaDesTime[indiceTabTime + 1];
                    gateTotOk = tabellaDesTime[indiceTabTime + 3];
                    int PTimeStart = 0;
                    int PTimeStop = 0;
                    int indexTimeStart = tabDesTimeStartTot.indexOf(":");
                    int indexTimeStop = tabDesTimeStopTot.indexOf(":");
                    byte indice1 = 0;
                    byte indice2 = 0;
                    int hhTabStart = 0;
                    int mmTabStart = 0;
                    int hhTabStop = 0;
                    int mmTabStop = 0;
                    while (indexTimeStart > 0)
                    {
                      String token1 = tabDesTimeStartTot.substring(indexTimeStart, PTimeStart);
                      PTimeStart = indexTimeStart + 1;
                      indexTimeStart = tabDesTimeStartTot.indexOf(":", indexTimeStart + 1);
                      if (indice1 == 0)
                      {
                        hhTabStart = atoi(token1.c_str());
                      }
                      else if (indice1 == 1)
                      {
                        mmTabStart = atof(token1.c_str());
                      }
                      indice1++;
                    }
                    while (indexTimeStop > 0)
                    {
                      String token2 = tabDesTimeStopTot.substring(indexTimeStop, PTimeStop);
                      PTimeStop = indexTimeStop + 1;
                      indexTimeStop = tabDesTimeStopTot.indexOf(":", indexTimeStop + 1);
                      if (indice2 == 0)
                      {
                        hhTabStop = atoi(token2.c_str());
                      }
                      else if (indice2 == 1)
                      {
                        mmTabStop = atof(token2.c_str());
                      }
                      indice2++;
                    }
                    if (hh >= hhTabStart && hh <= hhTabStop)
                    {
                      if (hh == hhTabStart)
                      {
                        if (mm >= mmTabStart)
                        {
                          isProfiloOk = 1;
                        }
                        else
                        {
                          isProfiloOk = 0;
                        }
                      }
                      else
                      {
                        if (hh == hhTabStop)
                        {
                          if (mm <= mmTabStop)
                          {
                            isProfiloOk = 1;
                          }
                          else
                          {
                            isProfiloOk = 0;
                          }
                        }
                        else
                        {
                          isProfiloOk = 1;
                        }
                      }
                    }
                    else
                    {
                      isProfiloOk = 0;
                    }
                    int PGate = 0;
                    int indexGate = gateTotOk.indexOf(";");
                    byte indice = 0;
                    while (indexGate > 0)
                    {
                      String token = gateTotOk.substring(indexGate, PGate);
                      PGate = indexGate + 1;
                      indexGate = gateTotOk.indexOf(";", indexGate + 1);
                      indice++;
                      if (config.idGate == atoi(token.c_str()))
                      {
                        DPRINTLN("idGate Ok");
                        waitingForKey = false;
                        isGateOk = 1;
                        break;
                      }
                      else
                      {
                        waitingForKey = true;
                        isGateOk = 0;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      break;
    }
  }
  if (isBadgeOk == 0 || isProfiloOk == 0 || isGateOk == 0)
  {
    waitingForKey = true;
  }
  if (waitingForKey)
  {
    return false;
  }
  else
  {
    return true;
  }
}

byte checkParity()
{
  int i = 0;
  int evenCount = 0;
  int oddCount = 0;
  for (i = 0; i < 8; i++)
  {
    if (RFIDcardNum[2] & (0x80 >> i))
    {
      evenCount++;
    }
  }
  for (i = 0; i < 4; i++)
  {
    if (RFIDcardNum[1] & (0x80 >> i))
    {
      evenCount++;
    }
  }
  for (i = 4; i < 8; i++)
  {
    if (RFIDcardNum[1] & (0x80 >> i))
    {
      oddCount++;
    }
  }
  for (i = 0; i < 8; i++)
  {
    if (RFIDcardNum[0] & (0x80 >> i))
    {
      oddCount++;
    }
  }
  if (((evenCount % 2 == evenBit) || (evenCount % 2 == evenBit2)) && ((oddCount % 2 != oddBit) || (oddCount % 2 != oddBit2)))
  {
    DPRINT("evenCount= ");
    DPRINTLN(evenCount % 2);
    DPRINT("oddCount= ");
    DPRINTLN(oddCount % 2);
    DPRINTLN("checkParity() = true");
    return 1;
  }
  else
  {
    DPRINT("evenCount= ");
    DPRINTLN(evenCount % 2);
    DPRINT("oddCount= ");
    DPRINTLN(oddCount % 2);
    DPRINTLN("checkParity() = false");
    resetData();
    rfidValue = 0;
    return 0;
  }
}

void resetData()
{
  RFIDcardNum[0] = 0;
  RFIDcardNum[1] = 0;
  RFIDcardNum[2] = 0;
  RFIDcardNum[3] = 0;
  evenBit = 0;
  oddBit = 0;
  recvBitCount = 0;
  isData0Low = 0;
  isData1Low = 0;
  isData2Low = 0;
  isData3Low = 0;
  isCardReadOver = 0;
  BufrfidValue = 0;
}

void ISRreceiveData0() // handle interrupt0
{
  recvBitCount++;
  isData0Low = 1;
}

void ISRreceiveData1() // handle interrupt1
{
  recvBitCount++;
  isData1Low = 1;
}

void ISRreceiveData2() // handle interrupt2
{
  recvBitCount++;
  isData2Low = 1;
}

void ISRreceiveData3() // handle interrupt3
{
  recvBitCount++;
  isData3Low = 1;
}

void esponidata()
{
  // esposizione dei dati provenienti dal RTC /shows data from RTC
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setCursor(13, 25);
  tft.fillRect(9, 9, 160, 25, TFT_WHITE);
  DateTime now = rtc.now();
  time_t utc = now.get_time_t();
  time_t local = myTZ.toLocal(utc, &tcr);
  tft.print(daysOfTheWeek[now.dayOfTheWeek()]);
  tft.print(" ");
  tft.print(day(local));
  tft.print(barra);
  tft.print(month(local));
  tft.print(barra);
  tft.print(year(local));
}

//************ routine di esposizione dell'ora ************
//************** hour exposition routine **************
void esponiora()
{
  //now = rtc.now();
  DateTime now = rtc.now();
  time_t utc = now.get_time_t();
  time_t local = myTZ.toLocal(utc, &tcr);
  if (!((second(local)) == secondoprec))
    ;
  {
    secondoprec = (second(local));
    tft.print(hour(local));
    tft.print(duepunti);
    digits = (minute(local));
    printDigits(); // routine esposizione zeri non significativi / non significant zeroes
  }
}

void esponidataCiclo()
{
  xpos = 13;
  ypos = 10; // Top left corner ot clock text, about half way down
  // esposizione dei dati provenienti dal timer di arduino /shows data from arduino timer
  DateTime now = rtc.now();
  time_t utc = now.get_time_t();
  time_t local = myTZ.toLocal(utc, &tcr);
  tft.print(daysOfTheWeek[now.dayOfTheWeek()]);
  tft.print(" ");
  tft.print(day(local));
  tft.print(barra);
  tft.print(month(local));
  tft.print(barra);
  tft.print(year(local));
}

void esponioraciclo()
{
  DateTime now = rtc.now();
  time_t utc = now.get_time_t();
  time_t local = myTZ.toLocal(utc, &tcr);
  byte omm = 99, oss = 99;
  byte xcolon = 0, ysecs = 0, hh = 0, mm = 0, ss = 0;
  hh = hour(local);
  mm = minute(local);
  ss = second(local);
  if (ss == 60)
  {
    omm = mm;
  }
  xpos = 220;
  ypos = 25;
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setFreeFont(FF5);
  //       // Update digital time
  if (omm != mm)
  { // Redraw hours and minutes time every minute
    omm = mm;
    //***Draw hours and minutes
    tft.fillRect(xpos, ypos - 20, 90, 25, TFT_WHITE);
    tft.setCursor(xpos, ypos);
    digits = hh;
    printDigits();
    tft.print(duepunti);
    digits = mm;
    printDigits();
    tft.print(duepunti);
  }
  if (oss != ss)
  { // Redraw seconds time every second
    oss = ss;
    digits = ss;
    printDigits();
  }
}

//****routine visualizzazione minuti e secondi comprensivi di zeri non significativi********
// ***** minutes and seconds, with non significant zeros treatment *************************
void printDigits()
{
  if (digits < 10)
    tft.print('0');
  tft.print(digits);
}

void writeTransit(long rfidValueSD, int entrata_uscitaSD, int Valido, int rfidIdBadgeSD)
{
  DateTime now = rtc.now();
  time_t utc = now.get_time_t();
  time_t local = myTZ.toLocal(utc, &tcr);
  DPRINTLN(printLocalTime(local));
  File AccessSD = SD.open(filenameAccessBuf, FILE_APPEND); //File in scrittura / open writing mode
  if (AccessSD)                                            // Se il file è stato aperto correttamente / if sd ok
  {
    AccessSD.print(config.idGate);
    AccessSD.print(virgola);
    AccessSD.print(rfidValueSD);
    AccessSD.print(virgola);
    AccessSD.print(entrata_uscitaSD);
    AccessSD.print(virgola);
    AccessSD.print(Valido);
    AccessSD.print(virgola);
    AccessSD.print(printLocalTime(local)); //write Timestamp
    AccessSD.print(virgola);
    AccessSD.print(rfidIdBadgeSD);
    AccessSD.print(virgola);
    AccessSD.print("\n");
    AccessSD.close(); // Chiude il file su sd / close file on sd
  }
  else
  {
    DPRINTLN("SD error");
  }
}

bool writeLog(String message)
{
  DateTime now = rtc.now();
  time_t utc = now.get_time_t();
  time_t local = myTZ.toLocal(utc, &tcr);
  File LogSD = SD.open(filenameLog, FILE_APPEND); //File in scrittura / open writing mode
  if (LogSD)                                      // Se il file è stato aperto correttamente / if sd ok
  {
    LogSD.print(config.idGate);
    LogSD.print(virgola);
    LogSD.print(printLocalTime(local));
    LogSD.print(virgola);
    LogSD.print(message);
    LogSD.print(virgola);
    LogSD.print("\n");
    LogSD.close(); // Chiude il file su sd / close file on sd
    return true;
  }
  else
  {
    DPRINTLN("SD error");
    return false;
  }
}

void key_5S_PRESS_input()
{
  state_5S_PRESS = !state_5S_PRESS;
  do_break = 1;
}

void key_a_input()
{
  state_a = !state_a;
  do_break = 1;
}

bool loadConfig(fs::FS &fs, int FNum, const char *path)
{
  // Open file for reading
  DPRINT("Reading Dir: ");
  DPRINTLN(path);
  File file = fs.open(path);
  if (!file)
  {
    DPRINTLN("Failed to open file for reading");
    return false;
  }
  DPRINT("Read from file: ");
  DPRINTLN("");
  const int capacity = 2024;
  // Allocate a temporary JsonDocument
  StaticJsonDocument<capacity> doc;
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    DPRINTLN(("Failed to read file, using default configuration"));
    tft.setFreeFont(FSSB9);
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("Failed to read file", 50, 60);
  }
  else
  {
    if (FNum == 0)
    {
      // Copy values from the JsonDocument to the Config
      config.idGate = doc["idGate"] | 0,
      strlcpy(config.MacAddress,
              doc["MacAddress"] | "MacAdd. not cfg",
              sizeof(config.MacAddress));
      strlcpy(config.local_IP,
              doc["local_IP"] | "local_IP not cfg",
              sizeof(config.local_IP));
      strlcpy(config.gateway,
              doc["gateway"] | "gateway not cfg",
              sizeof(config.gateway));
      strlcpy(config.subnet,
              doc["subnet"] | "subnet not cfg",
              sizeof(config.subnet));
      strlcpy(config.primaryDNS,
              doc["primaryDNS"] | "8.8.8.8",
              sizeof(config.primaryDNS));
      // strlcpy(config.secondaryDNS,
      //         doc["secondaryDNS"] | "8.8.4.4",
      //         sizeof(config.secondaryDNS));
      strlcpy(config.DBAddressParam,
              doc["DBAddressParam"] | "mysql01.areasrl.com",
              sizeof(config.DBAddressParam));
      strlcpy(config.DataBaseName,
              doc["DataBaseName"] | "acap100",
              sizeof(config.DataBaseName));
      config.DBPortParam = doc["DBPortParam"] | 3306;
      strlcpy(config.DBUserParam,
              doc["DBUserParam"] | "DBUser not cfg",
              sizeof(config.DBUserParam));
      strlcpy(config.DBPswParam,
              doc["DBPswParam"] | "DBPassword not cfg",
              sizeof(config.DBPswParam));
      config.SingleGate = doc["SingleGate"] | 0;
      DPRINT("Configuration ");
      DPRINT(FNum);
      DPRINTLN(" loaded");
    }
    else if (FNum == 1)
    {
      // // Copy values from the JsonDocument to the Config
      strlcpy(config.GateName,                      // <- destination
              doc["GateName"] | "GateName not cfg", // <- source
              sizeof(config.GateName));             // <- destination's capacity
      config.BeepOnOff = doc["BeepOnOff"] | 180;
      config.SyncDB = doc["SyncDB"] | 10;
      config.TimeGate1 = doc["TimeGate1"] | 10;
      config.TimeGate2 = doc["TimeGate2"] | 10;
      config.tftTimeout = doc["tftTimeout"] | 0;
      config.NTPUpDate = doc["NTPUpDate"] | 4;
      config.TransitUpdate = doc["TransitUpdate"] | 300;
      config.Log = doc["Log"] | 0;
      config.DelLog = doc["DelLog"] | 0;
      DPRINTLN("Configuration " + String(FNum) + " loaded");
    }
  }
  file.close();
  return true;
}

void printVar()
{
  DPRINT("\nID Gate: ");
  DPRINTLN(config.idGate);
  DPRINT("Gate Name: ");
  DPRINTLN(config.GateName);
  // DPRINT("WiFiEnable: ");
  // DPRINTLN(config.WiFiEnable);
  DPRINT("ssId = ");
  DPRINTLN(config.ssidSD);
  DPRINT("pass= ");
  DPRINTLN(config.passSD);
  DPRINT("MacAddress= ");
  DPRINTLN(config.MacAddress);
  DPRINT("LocalIP = ");
  DPRINTLN(config.local_IP);
  DPRINT("gateway = ");
  DPRINTLN(config.gateway);
  DPRINT("Subnet = ");
  DPRINTLN(config.subnet);
  DPRINT("Primary DNS = ");
  DPRINTLN(config.primaryDNS);
  // DPRINT("Secondary DNS = ");
  // DPRINTLN(config.secondaryDNS);
  DPRINT("DBAddressParam = ");
  DPRINTLN(config.DBAddressParam);
  DPRINT("DBPortParam = ");
  DPRINTLN(config.DBPortParam);
  DPRINT("BeepOnOff = ");
  DPRINTLN(config.BeepOnOff);
  DPRINT("DBUserParam = ");
  DPRINTLN(config.DBUserParam);
  DPRINT("DBPswParam = ");
  DPRINTLN(config.DBPswParam);
  DPRINT("SyncDB = ");
  DPRINTLN(config.SyncDB);
  DPRINT("SingleGate = ");
  DPRINTLN(config.SingleGate);
  DPRINT("TimeGate1 = ");
  DPRINTLN(config.TimeGate1);
  DPRINT("TimeGate2 = ");
  DPRINTLN(config.TimeGate2);
  DPRINT("tftTimeout = ");
  DPRINTLN(config.tftTimeout);
  DPRINT("NTPUpDate= ");
  DPRINTLN(config.NTPUpDate);
  DPRINT("TransitUpdate= ");
  DPRINTLN(config.TransitUpdate);
  DPRINT("Log= ");
  DPRINTLN(config.Log);
  DPRINT("DelLog= ");
  DPRINTLN(config.DelLog);
  DPRINT("Database= ");
  DPRINTLN(config.DataBaseName);
}

bool writeFilejson(fs::FS &fs, const char *pathF, int FNum, const char *message)
{

  DPRINT("Writing file: ");
  DPRINTLN(pathF);
  File fileF = fs.open(pathF, FILE_WRITE);
  if (!fileF)
  {
    DPRINTLN("Failed to open file for writing");
    return false;
  }
  const int capacity = 1256;
  StaticJsonDocument<capacity> docW;

  // Set the values in the document
  if (FNum == 0)
  {
    docW["idGate"] = config.idGate;
    docW["MacAddress"] = config.MacAddress;
    docW["local_IP"] = config.local_IP;
    docW["gateway"] = config.gateway;
    docW["subnet"] = config.subnet;
    docW["primaryDNS"] = config.primaryDNS;
    // docW["secondaryDNS"] = config.secondaryDNS;
    docW["DBAddressParam"] = config.DBAddressParam;
    docW["DBPortParam"] = config.DBPortParam;
    docW["DBUserParam"] = config.DBUserParam;
    docW["DBPswParam"] = config.DBPswParam;
    docW["SingleGate"] = config.SingleGate;
    docW["DataBaseName"] = config.DataBaseName;
  }
  else if (FNum == 1)
  {
    docW["GateName"] = config.GateName;
    docW["BeepOnOff"] = config.BeepOnOff;
    docW["SyncDB"] = config.SyncDB;
    docW["TimeGate1"] = config.TimeGate1;
    docW["TimeGate2"] = config.TimeGate2;
    docW["tftTimeout"] = config.tftTimeout;
    docW["NTPUpDate"] = config.NTPUpDate;
    docW["TransitUpdate"] = config.TransitUpdate;
    docW["Log"] = config.Log;
    docW["DelLog"] = config.DelLog;
  }
  // Serialize JSON to file
  if (serializeJson(docW, fileF) == 0)
  {
    DPRINTLN(("Failed to write to file"));
    return false;
  }
  delay(50);
  fileF.close();
  return true;
}

void parseBytes(const char *str, char sep, byte *bytes, int maxBytes, int base)
{
  for (int i = 0; i < maxBytes; i++)
  {
    bytes[i] = strtoul(str, NULL, base); // Convert byte
    str = strchr(str, sep);              // Find next separator
    if (str == NULL || *str == '\0')
    {
      break; // No more separators, exit
    }
    str++; // Point to next character after separator
  }
}

void cfgnetload()
{
  GateNameLoad = config.GateName;
  ssidload = config.ssidSD;
  passload = config.passSD;
  local_IPload = config.local_IP;
  gatewayload = config.gateway;
  subnetload = config.subnet;
  primaryDNSload = config.primaryDNS;
  // secondaryDNSload = config.secondaryDNS;
}

void getCfgVar()
{
  DPRINTLN("====================================================");
  DPRINTLN(("              Running SELECT getCfgVar                  "));

  DPRINTLN("Load ChangeConfig from DB");
  row_values *row = NULL;
  //char head_count [30];
  MySQL_Query query_memLC = MySQL_Query(&conn);

  char GET_DATA_CFG[] = "SELECT %s.gate.ChangeCFG FROM %s.gate WHERE %s.gate.idGate = %d";
  char queryCGF[128];
  snprintf(queryCGF, sizeof(queryCGF), GET_DATA_CFG, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.idGate);
  delay(5);
  //Execute the query
  if (!query_memLC.execute(queryCGF))
  {
    DPRINTLN("Query getCfgVar error");
    DBConnected = false;
    ServerOnOff = false;
    if (config.Log == 1)
    {
      writeLog("Query getCfgVar error");
    }
    query_memLC.close();
    //conn.close();
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    // Fetch the columns (required) but we don't use them.
    column_names *cols = query_memLC.get_columns();

    // Read the row (we are only expecting the one)
    do
    {
      row = query_memLC.get_next_row();
      if (row != NULL)
      {
        //head_count = atof(row->values[0]);
        cfgTime = atol(row->values[0]);
      }
    } while (row != NULL);
    // Now we close the cursor to free any memory
    //query_memLC->close();
    query_memLC.close();
    DPRINT("cfgTime = ");
    //conn.close();
    DPRINTLN(cfgTime);
  }
}

void getBadgeVar()
{
  DPRINTLN("====================================================");
  DPRINTLN(("              Running SELECT getBadgeVar                  "));

  DPRINTLN("Load ChangeBadge from DB");
  row_values *row = NULL;
  //char head_count [30];
  MySQL_Query query_memBC = MySQL_Query(&conn);

  char GET_DATA_CFGB[] = "SELECT %s.chkbadgevar.ChangeBadge FROM %s.chkbadgevar WHERE %s.chkbadgevar.IdChk = %d";
  char queryCFGB[128];
  snprintf(queryCFGB, sizeof(queryCFGB), GET_DATA_CFGB, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.idGate);
  delay(5);
  //Execute the query
  if (!query_memBC.execute(queryCFGB))
  {
    DPRINTLN("Query getBadgeVar error");
    DBConnected = false;
    ServerOnOff = false;
    if (config.Log == 1)
    {
      writeLog("Query getBadgeVar error");
    }
    query_memBC.close();
    //conn.close();
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    // Fetch the columns (required) but we don't use them.
    column_names *cols = query_memBC.get_columns();

    // Read the row (we are only expecting the one)
    do
    {
      row = query_memBC.get_next_row();
      if (row != NULL)
      {
        //head_count = atof(row->values[0]);
        cfgBadge = atol(row->values[0]);
      }
    } while (row != NULL);
    // Now we close the cursor to free any memory
    query_memBC.close();
    DPRINT("cfgBadge = ");
    DPRINTLN(cfgBadge);
  }
}

void getDesProfiliVar()
{
  DPRINTLN("====================================================");
  DPRINTLN(("              Running SELECT getDesProfiliVar                  "));

  DPRINTLN("Load ChangeDesProfili from DB");
  row_values *row = NULL;
  MySQL_Query query_memPC = MySQL_Query(&conn);

  char GET_DATA_CFGB[] = "SELECT %s.chkbadgevar.ChangeDesProfili FROM %s.chkbadgevar WHERE %s.chkbadgevar.IdChk = %d";
  char queryCFGB[128];
  snprintf(queryCFGB, sizeof(queryCFGB), GET_DATA_CFGB, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.idGate);
  delay(5);
  //Execute the query
  if (!query_memPC.execute(queryCFGB))
  {
    DPRINTLN("====================================================");
    DPRINTLN("Query getDesProfiliVar error");
    DPRINTLN("====================================================");

    DBConnected = false;
    ServerOnOff = false;
    if (config.Log == 1)
    {
      writeLog("Query getDesProfiliVar error");
    }
    query_memPC.close();
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    // Fetch the columns (required) but we don't use them.
    column_names *cols = query_memPC.get_columns();
    // Read the row (we are only expecting the one)
    do
    {
      row = query_memPC.get_next_row();
      if (row != NULL)
      {
        //head_count = atof(row->values[0]);
        cfgDesProfili = atol(row->values[0]);
      }
    } while (row != NULL);
    // Now we close the cursor to free any memory
    query_memPC.close();
    //conn.close();
    DPRINT("cfgDesProfili = ");
    DPRINTLN(cfgDesProfili);
  }
}

void getDesTimeVar()
{
  DPRINTLN("====================================================");
  DPRINTLN(("              Running SELECT getDesTimeVar                  "));

  DPRINTLN("Load ChangeDesTime from DB");
  row_values *row = NULL;
  //char head_count [30];
  MySQL_Query query_memTC = MySQL_Query(&conn);

  char GET_DATA_CFGT[] = "SELECT %s.chkbadgevar.ChangeDesTime FROM %s.chkbadgevar WHERE %s.chkbadgevar.IdChk = %d";
  char queryCFGT[128];
  snprintf(queryCFGT, sizeof(queryCFGT), GET_DATA_CFGT, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.idGate);
  delay(5);
  //Execute the query
  if (!query_memTC.execute(queryCFGT))
  {
    DPRINTLN("Query getDesTimeVar error");
    if (config.Log == 1)
    {
      writeLog("Query getDesTimeVar error");
    }
    DBConnected = false;
    ServerOnOff = false;
    query_memTC.close();
    //conn.close();
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    // Fetch the columns (required) but we don't use them.
    column_names *cols = query_memTC.get_columns();
    // Read the row (we are only expecting the one)
    do
    {
      row = query_memTC.get_next_row();
      if (row != NULL)
      {
        //head_count = atof(row->values[0]);
        cfgDesTime = atol(row->values[0]);
      }
    } while (row != NULL);
    // Now we close the cursor to free any memory
    query_memTC.close();
    //conn.close();
    DPRINT("cfgDesTime = ");
    DPRINTLN(cfgDesTime);
  }
}

void getChangeTabelVar()
{
  DPRINTLN("====================================================");
  DPRINTLN("       Running SELECT getChangeTabelVar             ");
  DPRINTLN("====================================================");

  DPRINTLN("Load ChangeTabel from DB");
  row_values *row = NULL;
  MySQL_Query query_memTC = MySQL_Query(&conn);
  char GET_DATA_CFGT[] = "SELECT %s.chkbadgevar.ChangeTabel FROM %s.chkbadgevar WHERE %s.chkbadgevar.IdChk = %d";
  char queryCFGT[128];
  snprintf(queryCFGT, sizeof(queryCFGT), GET_DATA_CFGT, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.idGate);
  delay(5);
  //Execute the query
  if (!query_memTC.execute(queryCFGT))
  {
    DPRINTLN("Query getChangeTabelVar error");
    if (config.Log == 1)
    {
      writeLog("Query getChangeTabelVar error");
    }
    DBConnected = false;
    ServerOnOff = false;
    query_memTC.close();
    //conn.close();
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    // Fetch the columns (required) but we don't use them.
    column_names *cols = query_memTC.get_columns();
    // Read the row (we are only expecting the one)
    do
    {
      row = query_memTC.get_next_row();
      if (row != NULL)
      {
        //head_count = atof(row->values[0]);
        cfgTabel = atol(row->values[0]);
      }
    } while (row != NULL);
    // Now we close the cursor to free any memory
    query_memTC.close();
    DPRINT("cfgTabel = ");
    DPRINTLN(cfgTabel);
  }
}

bool getCfgVarLast(bool loopRunning)
{

  DPRINTLN("====================================================");
  DPRINTLN("         Running SELECT getCfgVarLast               ");
  DPRINTLN("====================================================");

  DPRINTLN("I get LastChangeCFG from DB");
  byte xcur = 25;
  byte ycur = 80;
  row_values *row = NULL;
  MySQL_Query query_memGV = MySQL_Query(&conn);

  char GET_DATA_CFG[] = "SELECT %s.gate.LastChangeCFG FROM %s.gate WHERE %s.gate.idGate = %d";
  char queryCGF[128];
  snprintf(queryCGF, sizeof(queryCGF), GET_DATA_CFG, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.idGate);
  delay(5);
  //Execute the query
  if (!query_memGV.execute(queryCGF))
  {
    DPRINTLN("Query getCfgVarLast error");
    if (config.Log == 1)
    {
      writeLog("Query getCfgVarLast error");
    }
    DBConnected = false;
    ServerOnOff = false;
    query_memGV.close();
    //conn.close();
    return false;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    // Fetch the columns (required) but we don't use them.
    column_names *colsGV = query_memGV.get_columns();

    // Read the row (we are only expecting the one)
    do
    {
      row = query_memGV.get_next_row();
      if (row != NULL)
      {
        cfgTimeLast = atol(row->values[0]);
      }
    } while (row != NULL);
    // Now we close the cursor to free any memory
    query_memGV.close();
    DPRINT("cfgTimeLast = ");
    DPRINTLN(cfgTimeLast);
    if (loopRunning == 0)
    {
      tft.setCursor(xcur, ycur + 45);
      tft.println(("Tabella Configuraz."));
    }
    return true;
  }
}

void getBadgeVarLast(bool loopRunning)
{

  DPRINTLN("====================================================");
  DPRINTLN("         Running SELECT getBadgeVarLast             ");
  DPRINTLN("====================================================");

  DPRINTLN("I get LastChangeBadge from DB");
  byte xcur = 25;
  byte ycur = 80;
  row_values *row = NULL;
  MySQL_Query query_memBVL = MySQL_Query(&conn);

  char GET_DATA_CFGBL[] = "SELECT %s.chkbadgevar.LastChangeBadge FROM %s.chkbadgevar WHERE %s.chkbadgevar.IdChk = %d";
  char queryCGFBL[128];
  snprintf(queryCGFBL, sizeof(queryCGFBL), GET_DATA_CFGBL, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.idGate);
  delay(5);
  //Execute the query
  if (!query_memBVL.execute(queryCGFBL))
  {
    DPRINTLN("Query getBadgeVarLast error");
    if (config.Log == 1)
    {
      writeLog("Query getBadgeVarLast error");
    }
    DBConnected = false;
    ServerOnOff = false;
    query_memBVL.close();
    //conn.close();
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    // Fetch the columns (required) but we don't use them.
    column_names *colsGV = query_memBVL.get_columns();

    // Read the row (we are only expecting the one)
    do
    {
      row = query_memBVL.get_next_row();
      if (row != NULL)
      {
        cfgBadgeLast = atol(row->values[0]);
      }
    } while (row != NULL);
    // Now we close the cursor to free any memory
    query_memBVL.close();
    DPRINT("cfgBadgeLast = ");
    DPRINTLN(cfgBadgeLast);
    if (loopRunning == 0)
    {
      tft.setCursor(xcur, ycur + 65);
      tft.println(("Tabella Badge."));
    }
  }
}

void getDesProfiliVarLast(bool loopRunning)
{

  DPRINTLN("====================================================");
  DPRINTLN("         Running SELECT getDesProfiliVarLast        ");
  DPRINTLN("====================================================");

  DPRINTLN("I get LastChangeDesProfili from DB");
  byte xcur = 25;
  byte ycur = 80;
  row_values *row = NULL;
  MySQL_Query query_memPVL = MySQL_Query(&conn);

  char GET_DATA_CFGPL[] = "SELECT %s.chkbadgevar.LastChangeDesProfili FROM %s.chkbadgevar WHERE %s.chkbadgevar.IdChk = %d";
  char queryCGFPL[128];
  snprintf(queryCGFPL, sizeof(queryCGFPL), GET_DATA_CFGPL, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.idGate);
  delay(5);
  //Execute the query
  if (!query_memPVL.execute(queryCGFPL))
  {
    DPRINTLN("Query getDesProfiliVarLast error");
    if (config.Log == 1)
    {
      writeLog("Query getDesProfiliVarLast error");
    }
    DBConnected = false;
    ServerOnOff = false;
    query_memPVL.close();
    //conn.close();
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    // Fetch the columns (required) but we don't use them.
    column_names *colsGV = query_memPVL.get_columns();

    // Read the row (we are only expecting the one)
    do
    {
      row = query_memPVL.get_next_row();
      if (row != NULL)
      {
        cfgDesProfiliLast = atol(row->values[0]);
      }
    } while (row != NULL);
    // Now we close the cursor to free any memory
    query_memPVL.close();
    DPRINT("cfgDesProfiliLast = ");
    DPRINTLN(cfgDesProfiliLast);
    if (loopRunning == 0)
    {
      tft.setCursor(xcur, ycur + 85);
      tft.println(("Tabella Profili."));
    }
  }
}

void getDesTimeVarLast(bool loopRunning)
{

  DPRINTLN("====================================================");
  DPRINTLN("         Running SELECT getDesTimeVarLast           ");
  DPRINTLN("====================================================");

  DPRINTLN("I get LastChangeDesTime from DB");
  byte xcur = 25;
  byte ycur = 80;
  row_values *row = NULL;
  MySQL_Query query_memTVL = MySQL_Query(&conn);

  char GET_DATA_CFGTL[] = "SELECT %s.chkbadgevar.LastChangeDesTime FROM %s.chkbadgevar WHERE %s.chkbadgevar.IdChk = %d";
  char queryCGFTL[128];
  snprintf(queryCGFTL, sizeof(queryCGFTL), GET_DATA_CFGTL, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.idGate);
  delay(5);
  //Execute the query
  if (!query_memTVL.execute(queryCGFTL))
  {
    DPRINTLN("Query getDesTimeVarLast error");
    if (config.Log == 1)
    {
      writeLog("Query getDesTimeVarLast error");
    }
    DBConnected = false;
    ServerOnOff = false;
    query_memTVL.close();
    //conn.close();
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    // Fetch the columns (required) but we don't use them.
    column_names *colsGV = query_memTVL.get_columns();

    // Read the row (we are only expecting the one)
    do
    {
      row = query_memTVL.get_next_row();
      if (row != NULL)
      {
        cfgDesTimeLast = atol(row->values[0]);
      }
    } while (row != NULL);
    // Now we close the cursor to free any memory
    query_memTVL.close();
    DPRINT("cfgDesTimeLast = ");
    DPRINTLN(cfgDesTimeLast);
    if (loopRunning == 0)
    {
      tft.setCursor(xcur, ycur + 105);
      tft.println(("Tabella Fasce Orarie."));
    }
  }
}

void getChangeTabelVarLast(int loopRunning)
{

  DPRINTLN("====================================================");
  DPRINTLN("        Running SELECT getChangeTabelVarLast        ");
  DPRINTLN("====================================================");

  //DPRINTLN("I get LastChangeTabel from DB");
  byte xcur = 25;
  byte ycur = 80;
  row_values *row = NULL;
  MySQL_Query query_memTVL1 = MySQL_Query(&conn);

  char GET_DATA_CFGTL[] = "SELECT %s.chkbadgevar.LastChangeTabel, %s.chkbadgevar.ChangeTabel FROM %s.chkbadgevar WHERE %s.chkbadgevar.IdChk = %d";
  char queryCGFTL[200];
  snprintf(queryCGFTL, sizeof(queryCGFTL), GET_DATA_CFGTL, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.idGate);
  delay(5);
  //Execute the query
  if (!query_memTVL1.execute(queryCGFTL))
  {
    switch (loopRunning)
    {
    case 0:
      DPRINTLN("==============================================");
      DPRINTLN("Query getChangeTabelVarLast error in  SETUP");
      DPRINTLN("==============================================");
      break;
    case 1:
      DPRINTLN("==============================================");
      DPRINTLN("Query getChangeTabelVarLast error in  LOOP1");
      DPRINTLN("==============================================");
      break;
    case 2:
      DPRINTLN("==============================================");
      DPRINTLN("Query getChangeTabelVarLast error in  LOOP2");
      DPRINTLN("==============================================");
      break;
    default:
      break;
    }
    DBConnected = false;
    ServerOnOff = false;
    query_memTVL1.close();
    //conn.close();
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    // Fetch the columns (required) but we don't use them.
    column_names *colsGV = query_memTVL1.get_columns();

    // Read the row (we are only expecting the one)
    do
    {
      row = query_memTVL1.get_next_row();
      if (row != NULL)
      {
        cfgTabelLast = atol(row->values[0]);
        cfgTabel = atol(row->values[1]);
      }
    } while (row != NULL);
    // Now we close the cursor to free any memory
    query_memTVL1.close();
    DPRINT("cfgTabelLast = ");
    DPRINTLN(cfgTabelLast);
    DPRINT("cfgTabel = ");
    DPRINTLN(cfgTabel);
    if (loopRunning == 0)
    {
      tft.setCursor(xcur, ycur + 125);
      tft.println(("Check Tabelle."));
    }
  }
}

void WriteCfgNewToDB()
{
  DPRINTLN("====================================================");
  DPRINTLN("          Running UPDATE WriteCfgNewToDB            ");
  DPRINTLN("====================================================");

  row_values *row = NULL;
  MySQL_Query query_memNG = MySQL_Query(&conn);
  char WRITE_DATA_CFGNew[] = "UPDATE %s.gate SET LastChangeCFG = %d WHERE %s.gate.idGate = %d";
  char queryCfgnew[128];
  DPRINT("Update lastChangeCFG with cfgTime =");
  DPRINTLN(cfgTime);
  snprintf(queryCfgnew, sizeof(queryCfgnew), WRITE_DATA_CFGNew, config.DataBaseName, cfgTime, config.DataBaseName, config.idGate);
  delay(5);
  DPRINT("queryCfgnew");
  DPRINTLN(queryCfgnew);

  if (!query_memNG.execute(queryCfgnew))
  {
    DPRINTLN("==============================================");
    DPRINTLN("Query WriteCfgNewToDB error");
    DPRINTLN("==============================================");
    DBConnected = false;
    ServerOnOff = false;
    query_memNG.close();
    //conn.close();
    UpdateDBok = false;
    DPRINTLN("UpdateDBok=false");
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    query_memNG.close();
    UpdateDBok = false;
    DPRINTLN("UpdateDBok=true");
  }
}

void WriteCfgBadgeNewToDB()
{
  DPRINTLN("====================================================");
  DPRINTLN("          Running UPDATE WriteCfgBadgeNewToDB       ");
  DPRINTLN("====================================================");

  row_values *row = NULL;
  MySQL_Query query_memNG = MySQL_Query(&conn);
  char WRITE_DATA_CFGNew[] = "UPDATE %s.chkbadgevar SET LastChangeBadge = %d WHERE %s.chkbadgevar.IdChk = %d";
  char queryCfgnew[128];
  snprintf(queryCfgnew, sizeof(queryCfgnew), WRITE_DATA_CFGNew, config.DataBaseName, cfgBadge, config.DataBaseName, config.idGate);
  delay(5);
  if (!query_memNG.execute(queryCfgnew))
  {
    DPRINTLN("==============================================");
    DPRINTLN("Query WriteCfgBadgeNewToDB error");
    DPRINTLN("==============================================");
    DBConnected = false;
    ServerOnOff = false;
    query_memNG.close();
    //conn.close();
    UpdateDBok = false;
    DPRINTLN("UpdateDBok=false");
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    query_memNG.close();
    UpdateDBok = false;
    //conn.close();
    DPRINTLN("UpdateDBok=true");
  }
}

void WriteCfgProfiliNewToDB()
{
  DPRINTLN("====================================================");
  DPRINTLN("       Running UPDATE WriteCfgProfiliNewToDB        ");
  DPRINTLN("====================================================");

  row_values *row = NULL;
  MySQL_Query query_memNG = MySQL_Query(&conn);
  char WRITE_DATA_CFGNew[] = "UPDATE %s.chkbadgevar SET LastChangeDesProfili = %d WHERE %s.chkbadgevar.IdChk = %d";
  char queryCfgnew[128];
  snprintf(queryCfgnew, sizeof(queryCfgnew), WRITE_DATA_CFGNew, config.DataBaseName, cfgDesProfili, config.DataBaseName, config.idGate);
  delay(5);
  if (!query_memNG.execute(queryCfgnew))
  {
    DPRINTLN("==============================================");
    DPRINTLN("Query WriteCfgProfiliNewToDB error");
    DPRINTLN("==============================================");
    DBConnected = false;
    ServerOnOff = false;
    query_memNG.close();
    //conn.close();
    UpdateDBok = false;
    DPRINTLN("UpdateDBok=false");
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    query_memNG.close();
    UpdateDBok = false;
    DPRINTLN("UpdateDBok=true");
  }
}

void WriteCfgTimeNewToDB()
{
  DPRINTLN("====================================================");
  DPRINTLN("        Running UPDATE WriteCfgTimeNewToDB          ");
  DPRINTLN("====================================================");

  row_values *row = NULL;
  MySQL_Query query_memNG = MySQL_Query(&conn);
  char WRITE_DATA_CFGNew[] = "UPDATE %s.chkbadgevar SET LastChangeDesTime = %d WHERE %s.chkbadgevar.IdChk = %d";
  char queryCfgnew[128];
  snprintf(queryCfgnew, sizeof(queryCfgnew), WRITE_DATA_CFGNew, config.DataBaseName, cfgDesTime, config.DataBaseName, config.idGate);
  delay(5);
  DPRINTLN("queryCfgnew ");
  DPRINTLN(queryCfgnew);

  if (!query_memNG.execute(queryCfgnew))
  {
    DPRINTLN("==============================================");
    DPRINTLN("Query WriteCfgTimeNewToDB error");
    DPRINTLN("==============================================");
    DBConnected = false;
    ServerOnOff = false;
    query_memNG.close();
    //conn.close();
    UpdateDBok = false;
    DPRINTLN("UpdateDBok=false");
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    query_memNG.close();
    UpdateDBok = true;
    DPRINTLN("UpdateDBok=true");
  }
}

void WriteChangeTabelNewToDB()
{
  DPRINTLN("====================================================");
  DPRINTLN("       Running UPDATE WriteChangeTabelNewToDB       ");
  DPRINTLN("====================================================");

  row_values *row = NULL;
  MySQL_Query query_memNG = MySQL_Query(&conn);
  char WRITE_DATA_CFGNew[] = "UPDATE %s.chkbadgevar SET LastChangeTabel = %d WHERE %s.chkbadgevar.IdChk = %d";
  char queryCfgnew[128];
  snprintf(queryCfgnew, sizeof(queryCfgnew), WRITE_DATA_CFGNew, config.DataBaseName, cfgTabel, config.DataBaseName, config.idGate);
  delay(5);
  if (!query_memNG.execute(queryCfgnew))
  {
    DPRINTLN("==============================================");
    DPRINTLN("Query WriteChangeTabelNewToDB error");
    DPRINTLN("==============================================");
    DBConnected = false;
    ServerOnOff = false;
    query_memNG.close();
    //conn.close();
    return;
  }
  else
  {
    DBConnected = true;
    ServerOnOff = true;
    query_memNG.close();
  }
}

bool getCfgfromDB(bool loopRunning)
{

  DPRINTLN("====================================================");
  DPRINTLN("           Running SELECT getCfgfromDB2             ");
  DPRINTLN("====================================================");

  MySQL_Query query_mem2 = MySQL_Query(&conn);
  row_values *rowC2 = NULL;
  char GET_DATA_CFG2[] = "SELECT  %s.Config.DeviceName, %s.Config.BeepOnOff, %s.Config.SyncDB, %s.Config.TimeGate1, %s.Config.TimeGate2, %s.Config.tftTimeout, %s.Config.NTPUpDate, %s.Config.TransitUpdate, %s.Config.Log, %s.Config.DelLog FROM %s.Config WHERE %s.Config.Gate = %d";
  char queryCGFFROMDB2[400];
  int id = config.idGate;
  snprintf(queryCGFFROMDB2, sizeof(queryCGFFROMDB2), GET_DATA_CFG2, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, id);
  delay(5);
  //Execute the query
  DPRINT("queryCGFFROMDB2= ");
  DPRINTLN(queryCGFFROMDB2);
  if (!query_mem2.execute(queryCGFFROMDB2))
  {
    DPRINTLN("==============================================");
    DPRINTLN("Query CFG2 error");
    DPRINTLN("==============================================");
    if (config.Log == 1)
    {
      writeLog("Query CFG2 error");
    }
    DBConnected = false;
    ServerOnOff = false;
    query_mem2.close();
    //conn.close();
    ;
    return false;
  }
  else
  {
    DPRINTLN("Query 2 ok");
    DBConnected = true;
    ServerOnOff = true;
    // Fetch the columns
    column_names *colscfg2 = query_mem2.get_columns();
    do
    {
      rowC2 = query_mem2.get_next_row();
      if (rowC2 != NULL)
      {
        for (int f = 0; f < colscfg2->num_fields; f++)
        {
          //DPRINTLN(rowC2->values[f]);
          strcpy(config.GateName, rowC2->values[0]);
          config.BeepOnOff = atoi(rowC2->values[1]);
          config.SyncDB = atoi(rowC2->values[2]);
          config.TimeGate1 = atoi(rowC2->values[3]);
          config.TimeGate2 = atoi(rowC2->values[4]);
          config.tftTimeout = atoi(rowC2->values[5]);
          config.NTPUpDate = atoi(rowC2->values[6]);
          config.TransitUpdate = atoi(rowC2->values[7]);
          config.Log = atoi(rowC2->values[8]);
          config.DelLog = atoi(rowC2->values[9]);
          delay(5);
        }
        //delay(5);
      }
    } while (rowC2 != NULL);
    query_mem2.close();
    tftTimer.start(config.tftTimeout * 1000);
    updateCfg.start(config.SyncDB * 1000);
    prog[1] = config.TransitUpdate * 1000;
    delay(50);
    if ((writeFilejson(SD, filenameCFG2Back, 1, "{\n")) == true)
    {
      delay(300);
      deleteFile(SD, filenameCFG2);
      delay(300);
      if ((renameFile(SD, filenameCFG2Back, filenameCFG2)) == true)
      {
        DPRINTLN("File cfgACAP2.jsn Renamed ok");
      }
      else
      {
        DPRINTLN("Renamed Failed try again one time");
        if ((renameFile(SD, filenameCFG2Back, filenameCFG2)) == true)
        {
          DPRINTLN("File cfgACAP2.jsn Renamed ok");
        }
        else
        {
          DPRINTLN("Raneme cfgACAP2.jsn Failed Definitive");
          if (loopRunning == 0)
          {
            tft.setFreeFont(FF5);
            byte xcur = 25;
            byte ycur = 80;
            tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
            tft.setCursor(xcur, ycur + 20);
            tft.println(("Carico Tab. Configuraz."));
            tft.setCursor(xcur, ycur + 45);
            tft.println(("Salv. Config. 2 fallita"));
            delay(1500);
            return false;
          }
        }
      }
      if (loopRunning == 0)
      {
        tft.setFreeFont(FF5);
        byte xcur = 25;
        byte ycur = 80;
        tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
        tft.setCursor(xcur, ycur + 20);
        tft.println(("Carico Tab. Configuraz."));
        tft.setCursor(xcur, ycur + 45);
        tft.println(("Configuraz. 2 Ricevuta"));
        delay(1000);
      }
    }
    else
    {
      DPRINTLN("Writing file cfgACAP2.jsn Json Failed");
      if (loopRunning == 0)
      {
        tft.setFreeFont(FF5);
        byte xcur = 25;
        byte ycur = 80;
        tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
        tft.setCursor(xcur, ycur + 20);
        tft.println(("Carico Tab. Configuraz."));
        tft.setCursor(xcur, ycur + 45);
        tft.println(("Scrit. file conf. 2 errata"));
        delay(1000);
        return false;
      }
    }
    return true;
  }
}

bool getCfgNetfromDB(int loopRunning)
{
  DPRINTLN("====================================================");
  DPRINTLN("           Running SELECT getCfgNetfromDB1          ");
  DPRINTLN("====================================================");

  MySQL_Query query_mem = MySQL_Query(&conn);
  char GET_DATA_CFG[] = "SELECT acap100.Config.MacAddress, acap100.Config.local_IP, acap100.Config.gateway, acap100.Config.subnet, acap100.Config.primaryDNS FROM acap100.Config WHERE acap100.Config.Gate = % d";

  char queryCGFFROMDB[250];
  int id = config.idGate;
  snprintf(queryCGFFROMDB, sizeof(queryCGFFROMDB), GET_DATA_CFG, id);
  //Execute the query
  DPRINT("queryCGFFROMDB= ");
  DPRINTLN(queryCGFFROMDB);
  if (!query_mem.execute(queryCGFFROMDB))
  {
    DPRINTLN("==============================================");
    DPRINTLN("Query CFGNet1 error");
    DPRINTLN("==============================================");
    tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
    tft.setCursor(xcur, ycur + 20);
    tft.println(("Query CFGNet1 error"));
    delay(1500);
    if (config.Log == 1)
    {
      writeLog("Query CFGNet1 error");
    }
    DBConnected = false;
    ServerOnOff = false;
    query_mem.close();
    //conn.close();
    return false;
  }
  else
  {
    DPRINTLN("Query CFGNet1 ok");
    tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
    tft.setCursor(xcur, ycur + 20);
    tft.println(("Query CFGNet1 eseguita"));
    delay(1500);
    DBConnected = true;
    ServerOnOff = true;
  }
  // Fetch the columns
  column_names *colscfg = query_mem.get_columns();
  row_values *row = NULL;
  do
  {
    row = query_mem.get_next_row();
    if (row != NULL)
    {
      for (int f = 0; f < colscfg->num_fields; f++)
      {
        DPRINT("f= ");
        DPRINT(f);
        DPRINT(" Value= ");
        DPRINTLN(row->values[f]);
        strcpy(config.MacAddress, row->values[0]);
        strcpy(config.local_IP, row->values[1]);
        strcpy(config.gateway, row->values[2]);
        strcpy(config.subnet, row->values[3]);
        strcpy(config.primaryDNS, row->values[4]);
        delay(10);
      }
      delay(10);
    }
  } while (row != NULL);
  query_mem.close();
  delay(50);

  DPRINTLN("====================================================");
  DPRINTLN("           Running SELECT getCfgNetfromDB2          ");
  DPRINTLN("====================================================");

  char GET_DATA_CFG2[] = "SELECT %s.Config.DBAddressParam, %s.Config.DBPortParam, %s.Config.DBUserParam, %s.Config.DBPswParam, %s.Config.DataBaseName, %s.Config.SingleGate FROM %s.Config WHERE %s.Config.Gate = % d";
  char queryCGFFROMDB2[250];
  snprintf(queryCGFFROMDB2, sizeof(queryCGFFROMDB2), GET_DATA_CFG2, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, config.DataBaseName, id);
  //Execute the query
  DPRINT("queryCGFFROMDB2= ");
  DPRINTLN(queryCGFFROMDB2);
  if (!query_mem.execute(queryCGFFROMDB2))
  {
    DPRINTLN("==============================================");
    DPRINTLN("Query CFGNet2 error");
    DPRINTLN("==============================================");
    tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
    tft.setCursor(xcur, ycur + 20);
    tft.println(("Query CFGNet2 error"));
    delay(1500);
    if (config.Log == 1)
    {
      writeLog("Query CFGNet2 error");
    }
    DBConnected = false;
    ServerOnOff = false;
    query_mem.close();
    //conn.close();
    return false;
  }
  else
  {
    DPRINTLN("Query CFGNet2 ok");
    tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
    tft.setCursor(xcur, ycur + 20);
    tft.println(("Query CFGNet2 eseguita"));
    delay(1500);
    DBConnected = true;
    ServerOnOff = true;
  }
  // Fetch the columns
  colscfg = query_mem.get_columns();
  row = NULL;
  do
  {
    row = query_mem.get_next_row();
    if (row != NULL)
    {
      for (int f = 0; f < colscfg->num_fields; f++)
      {
        strcpy(config.DBAddressParam, row->values[0]);
        config.DBPortParam = atoi(row->values[1]);
        strcpy(config.DBUserParam, row->values[2]);
        strcpy(config.DBPswParam, row->values[3]);
        strcpy(config.DataBaseName, row->values[4]);
        config.SingleGate = atoi(row->values[5]);
        delay(10);
      }
      delay(10);
    }
  } while (row != NULL);
  query_mem.close();
  //conn.close();
  delay(50);

  if (loopRunning == 0)
  {
    tft.setFreeFont(FF5);
    byte xcur = 25;
    byte ycur = 80;
    tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
    tft.setCursor(xcur, ycur + 20);
    tft.println(("Carico Tab. Configuraz."));
    tft.setCursor(xcur, ycur + 45);
    tft.println(("Configuraz. 1 Ricevuta"));
    tft.setCursor(xcur, ycur + 60);
    tft.println(("DB Server:"));
    tft.setCursor(xcur, ycur + 75);
    tft.println((config.DBAddressParam));
    delay(1500);
  }
  if ((writeFilejson(SD, filenameCFGBack, 0, "{\n")) == true)
  {
    delay(300);
    deleteFile(SD, filenameCFG);
    delay(300);
    if ((renameFile(SD, filenameCFGBack, filenameCFG)) == true)
    {
      DPRINTLN("File ccfgACAP.jsn Renamed ok");
    }
    else
    {
      DPRINTLN("Renamed Failed try again one time");
      if ((renameFile(SD, filenameCFGBack, filenameCFG)) == true)
      {
        DPRINTLN("File ccfgACAP.jsn Renamed ok");
      }
      else
      {
        DPRINTLN("Raneme Failed Definitive");
        tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
        tft.setCursor(xcur, ycur + 20);
        tft.println(("File ccfgACAP.jsn not renamed"));
        delay(1500);
      }
    }
  }
  else
  {
    DPRINTLN("Writing ccfgACAP.jsn Json file Failed");
  }
}

void setup()
{
  tft.init();
  tft.setRotation(1);
  Wire.begin();
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(FF5);
  tft.fillScreen(TFT_WHITE);
  tft.setCursor((320 - tft.textWidth("Inizializzazione Device")) / 2, 120);
  tft.print(("Inizializzazione Device"));
  Serial.begin(115200);
#ifndef NDEBUG
  while (!Serial)
  {
  }; // wait for serial port to connect
#endif
  pinMode(WIO_BUZZER, OUTPUT);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(ResetETH, OUTPUT);
  digitalWrite(ResetETH, LOW);
  //delay(10000);

  pinMode(rele1, OUTPUT);
  digitalWrite(rele1, LOW);

  pinMode(rele2, OUTPUT);
  digitalWrite(rele2, LOW);
  delay(50);
  pinMode(beeper, OUTPUT);
  digitalWrite(beeper, HIGH);

  digitalWrite(ResetETH, HIGH);
  //delay(200);

  tft.setCursor((320 - tft.textWidth("Inizializzazione Device")) / 2, 145);
  DPRINTLN("Eth shield startup delay");
  for (int t = 0; t < 4; t++)
  {
    tft.print(".");
    DPRINT(".");
    delay(50);
  }

  myDST = (TimeChangeRule){
      "CEST", Last, Sun, Mar, 2, 120};
  mySTD = (TimeChangeRule){
      "CET", Last, Sun, Oct, 3, 60};
  myTZ.init(myDST, mySTD);
  Udp.begin(localPort);
  rtc.attach(Wire);

#if USE_W5500
  VerString += ".W.";
#elif USE_ENC28J60
  VerString += ".E.";
#endif

#if Mensa
  VerString += "M";
#elif PMensa
  VerString += "PM";
#elif Presenze
  VerString += "P";
#elif CA
  VerString += "CA";
#endif
#ifdef Beta
  VerString += ".beta";
#endif
  VerString.toCharArray(VER_FW, 20);

  MYSQL_LOGERROR3(("Board :"), BOARD_NAME, (", setCsPin:"), USE_THIS_SS_PIN);
  DPRINTLN("\nEthernet shield inizialize");
  Ethernet.init(USE_THIS_SS_PIN);

  //Just info to know how to connect correctly
  MYSQL_LOGERROR(F("========================================="));
  MYSQL_LOGERROR(F("Currently Used SPI pinout:"));
  MYSQL_LOGERROR1(F("MOSI:"), MOSI);
  MYSQL_LOGERROR1(F("MISO:"), MISO);
  MYSQL_LOGERROR1(F("SCK:"), SCK);
  MYSQL_LOGERROR1(F("SS:"), SS);
  MYSQL_LOGERROR(F("========================================="));

  Serial.println("============================================");
  Serial.print("Sketch: ");
  Serial.println(__FILE__);
  Serial.print("Uploaded: ");
  Serial.println(__DATE__);
  Serial.print("Ver FW: ");
  Serial.println(VER_FW);

#if Presenze || PMensa || CA
  rfid.begin(9600); // the SoftSerial baud rate
#endif

  rfid2.begin(9600); // the SoftSerial baud rate

#if Mensa
  attachInterrupt(digitalPinToInterrupt(WIO_5S_UP), key_a_input, CHANGE);
#elif Presenze
  attachInterrupt(digitalPinToInterrupt(WIO_5S_UP), key_a_input, CHANGE);
#elif PMensa
  attachInterrupt(digitalPinToInterrupt(WIO_5S_UP), key_a_input, CHANGE);
#elif CA
  attachInterrupt(digitalPinToInterrupt(WIO_5S_UP), key_a_input, CHANGE);
#endif

  attachInterrupt(digitalPinToInterrupt(0), ISRreceiveData0, FALLING); //data0/tx is connected to pin 2, which results in INT 0
  attachInterrupt(digitalPinToInterrupt(1), ISRreceiveData1, FALLING); //data1/rx yellow is connected to pin 3, which results in INT 1

#if Presenze || PMensa || CA
  attachInterrupt(digitalPinToInterrupt(2), ISRreceiveData2, FALLING); //data2/tx is connected to pin 5, which results in INT 0
  attachInterrupt(digitalPinToInterrupt(3), ISRreceiveData3, FALLING); //data3/rx yellow is connected to pin 7, which results in INT 1
#endif

  digitalWrite(LCD_BACKLIGHT, HIGH);
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(FF5);
  DPRINTLN("Boot WioT application");

  tft.fillScreen(TFT_WHITE);
  tft.setCursor((320 - tft.textWidth("Inizializzazione Device")) / 2, 120);
  tft.print(("Inizializzazione Device"));
  while (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI, 4000000UL))
  {
    DPRINTLN("Card Mount Failed");
    tft.fillScreen(TFT_WHITE);
    tft.setCursor((320 - tft.textWidth("SD Card Mount Failed")) / 2, 120);
    tft.print(("SD Card Mount Failed"));
    delay(2000);
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    DPRINTLN("No SD card attached");
    tft.fillScreen(TFT_WHITE);
    tft.setCursor((320 - tft.textWidth("No SD card attached")) / 2, 120);
    tft.print(("No SD card attached"));
    delay(2000);
    return;
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  DPRINT("SD Card Size: ");
  DPRINT((uint32_t)cardSize);
  DPRINTLN("MB");
  if (config.Log == 1)
  {
    writeLog("Boot Device");
  }
  attachInterrupt(digitalPinToInterrupt(WIO_5S_PRESS), key_5S_PRESS_input, CHANGE);

  if (!loadConfig(SD, 0, filenameCFG))
  {
    DPRINTLN("Configuration file not found on the SD");
    DPRINTLN("it is not possible to continue");
    tft.fillScreen(TFT_WHITE);
    tft.setCursor((320 - tft.textWidth("non e' possibile continuare")) / 2, 90);
    tft.print(("File di configurazione"));
    tft.setCursor((320 - tft.textWidth("non e' possibile continuare")) / 2, 110);
    tft.print(("non trovato sulla SD"));
    tft.setCursor((320 - tft.textWidth("non e' possibile continuare")) / 2, 130);
    tft.print(("non e' possibile continuare"));
    while (true)
    {
      delay(1);
    }
  }
  else if (!loadConfig(SD, 1, filenameCFG2))
  {
    DPRINTLN("Secondary configuration file not found on the SD");
    DPRINTLN("it is not possible to continue");
    tft.fillScreen(TFT_WHITE);
    tft.setCursor((320 - tft.textWidth("non e' possibile continuare")) / 2, 90);
    tft.print(("File di configurazione 2"));
    tft.setCursor((320 - tft.textWidth("non e' possibile continuare")) / 2, 110);
    tft.print(("non trovato sulla SD"));
    tft.setCursor((320 - tft.textWidth("non e' possibile continuare")) / 2, 130);
    tft.print(("non e' possibile continuare"));
    while (true)
    {
      delay(1);
    }
  }
  else
  {
    tft.fillScreen(TFT_WHITE);
    tft.setCursor((320 - tft.textWidth("Caricamento Configurazione")) / 2, 110);
    tft.print(("Caricamento Configurazione"));
    tft.setCursor((320 - tft.textWidth("Caricamento Configurazione")) / 2, 130);
    tft.print(("dalla SD Card effettuato"));
    if (config.Log == 1)
    {
      writeLog("Config loaded from SD");
    }
  }

  strncpy(hostname, config.DBAddressParam, 128);
  server_port = config.DBPortParam;
  DBUserParamload = config.DBUserParam;
  DBPswParamload = config.DBPswParam;
  delay(1500);

#ifdef NoWiFi
  parseBytes(config.MacAddress, '-', macAdd, 6, 16);
  myIP.fromString(String(config.local_IP));
  myMASK.fromString(String(config.subnet));
  myDNS.fromString(String(config.primaryDNS));
  myGW.fromString(String(config.gateway));

  DPRINT("myIP= ");
  DPRINTLN(myIP);
  DPRINT("myMASK= ");
  DPRINTLN(myMASK);
  DPRINT("myDNS= ");
  DPRINTLN(myDNS);
  DPRINT("myGW= ");
  DPRINTLN(myGW);
  tft.fillScreen(TFT_WHITE);
  tft.setFreeFont(FF5);
  tft.setCursor((320 - tft.textWidth("Avvio Scheda di Rete.")) / 2, 100);
  tft.print("Avvio Scheda di Rete.");
  tft.setCursor((320 - tft.textWidth("Avvio Scheda di Rete.")) / 2, 125);
  DPRINTLN("Ethernet Shield startup.");
  for (int t = 0; t < 10; t++)
  {
    tft.print(".");
    DPRINT(".");
    delay(50);
  }
  Ethernet.begin(macAdd, myIP, myDNS, myGW, myMASK);
  DPRINT("\nTry to Connected with IP address: ");
  DPRINTLN(Ethernet.localIP());

  delay(200);
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    DPRINTLN("Ethernet shield was not found.  Sorry, can't run without hardware. :( System halt");
    //no point in carrying on, so do nothing forevermore:
    tft.fillScreen(TFT_WHITE);
    tft.setFreeFont(FF5);
    tft.setCursor((320 - tft.textWidth("Ethernet shield not found.")) / 2, 100);
    tft.print("Ethernet shield not found.");
    tft.setCursor((320 - tft.textWidth("Ethernet shield not found.")) / 2, 120);
    tft.println("Sorry, can't run without");
    tft.setCursor((320 - tft.textWidth("Ethernet shield not found.")) / 2, 160);
    tft.print("     :(  System halt");
    while (true)
    {
      delay(1);
    }
  }
  if (Ethernet.linkStatus() == LinkOFF)
  {
    DPRINTLN("Ethernet cable disconnect (setup)");
    tft.fillScreen(TFT_WHITE);
    tft.setCursor((320 - tft.textWidth("Cavo Ethernet disconnesso.")) / 2, 110);
    tft.println(("Cavo Ethernet disconnesso."));
    tft.setCursor((320 - tft.textWidth("Cavo Ethernet disconnesso.")) / 2, 130);
    tft.println(("In attesa connesione."));
    delay(2000);
    ReteConnected = false;
    ETHStartup = false;
    if (config.Log == 1)
    {
      writeLog("Ethernet cable disconnect (setup)");
    }
  }
  else
  {
    ReteConnected = true;
    ETHStartup = true;
    DNSClient dns;
    delay(200);
    dns.begin(Ethernet.dnsServerIP());
    dnsReturn = dns.getHostByName(hostname, server_ip);
    if (dnsReturn == 1)
    {
      DPRINT("Host =");
      DPRINT(hostname);
      DPRINT(" Ip resolved = ");
      DPRINTLN(server_ip);
    }
    else
    {
      DPRINTLN("DB Host Ip not resolved");
      DPRINTLN("set default");
      server_ip.fromString("109.168.40.35");
      config.DBPortParam = 3306;
      if (config.Log == 1)
      {
        writeLog("DB Host Ip not resolved");
      }
    }
  }
#else
// Implement functions for WiFi here
#endif

  if (digitalRead(WIO_5S_PRESS) == LOW)
  {
    DPRINTLN("Tasto 5S Pressed");
    tft.fillScreen(TFT_WHITE);
    tft.setCursor((320 - tft.textWidth("Carico Conf.1 dal Server.")) / 2, 120);
    tft.println(("Carico Conf.1 dal Server."));
    delay(1000);
    conn.close();
    delay(500);
    if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
    {
      delay(1000);

      getCfgNetfromDB(0);
      conn.close();
      tft.fillScreen(TFT_WHITE);
      tft.setCursor((320 - tft.textWidth("Conf.1  Caricata dal Server.")) / 2, 120);
      tft.println(("Conf.1  Caricata dal Server."));
      readFile(SD, filenameCFG);
      delay(1500);
    }
    conn.close();
    delay(200);
    if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
    {
      delay(500);

      WriteCfgNewToDB();
    }
    NVIC_SystemReset();
  }
  if ((ReteConnected == true) && (dnsReturn == 1))
  {

    for (j = 0; j < 3; j++)
    {
      devicetime = getNTPtime();
      // check if NTP response
      if (devicetime == 0 || j == 2)
      {
        DPRINTLN("Failed to get time from network time server.");
        tft.fillScreen(TFT_WHITE);
        tft.setCursor((320 - tft.textWidth("Failed to get time")) / 2, 120);
        tft.print(("Failed to get time"));
        tft.setCursor((320 - tft.textWidth("Failed to get time")) / 2, 145);
        tft.print(("from network."));
        delay(600);
      }
      else
      {
        tft.fillScreen(TFT_WHITE);
        tft.setCursor((320 - tft.textWidth("Orario NTP Server Ricevuto")) / 2, 120);
        tft.print(("Orario NTP Server Ricevuto"));
        DateTime now = rtc.now();
        time_t utc = now.get_time_t();
        time_t local = myTZ.toLocal(utc, &tcr);
        DPRINTLN("NTP Server time ok");
        DPRINTLN(printLocalTime(local));
        delay(1000);
        tft.fillScreen(TFT_WHITE);
        break;
      }
    }
    tft.fillScreen(TFT_WHITE);
    tft.setCursor((320 - tft.textWidth("Conn. al Server in corso.")) / 2, 100);
    tft.println(("Conn. al Server in corso."));
    if (config.Log == 1)
    {
      writeLog("Conn. al Server in corso");
    }
    yield();
    cfgnetload();
    yield();
    conn.close();
    delay(50);
    if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
    {
      delay(500);

      getCfgVar();
      tft.setCursor((320 - tft.textWidth("Conn. al Server in corso.")) / 2, 120);
      tft.println("Tabella 1");
    }
    else
    {
      DBConnected = false;
      tft.setCursor((320 - tft.textWidth("Conn. al Server in corso.")) / 2, 140);
      tft.println("Connessione fallita ");
      DPRINTLN("Connection to DB Failed");
      if (config.Log == 1)
      {
        writeLog("Connection to DB Failed");
      }
    }
    if (DBConnected)
    {
      if (config.Log == 1)
      {
        writeLog("Server connected");
      }
      //conn.close();
      delay(50);
      // if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
      // {
      //   delay(500);

      getBadgeVar();
      tft.setCursor((320 - tft.textWidth("Conn. al Server in corso.")) / 2, 140);
      tft.println("Tabella 2");
      // }
      // else
      // {
      //   DBConnected = false;
      // }
      // conn.close();
      delay(50);
      // if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
      // {
      //   delay(500);
      getDesProfiliVar();
      tft.setCursor((320 - tft.textWidth("Conn. al Server in corso.")) / 2, 160);
      tft.println("Tabella 3");
      // }
      // else
      // {
      //   DBConnected = false;
      // }
      // conn.close();
      delay(50);
      // if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
      // {
      //   delay(500);
      getDesTimeVar();
      tft.setCursor((320 - tft.textWidth("Conn. al Server in corso.")) / 2, 180);
      tft.println("Tabella 4");
      // }
      // else
      // {
      //   DBConnected = false;
      // }
      // conn.close();
      delay(50);
      // if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
      // {
      //   delay(500);
      getChangeTabelVar();
      tft.setCursor((320 - tft.textWidth("Conn. al Server in corso.")) / 2, 200);
      tft.println("Tabella 5");
      // }
      // else
      // {
      //   DBConnected = false;
      // }
      // conn.close();
      delay(50);
      // if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
      // {
      //   delay(500);
      LoadDBProfili = ProfiliFromDBtoLocal(0);
      if (LoadDBProfili)
      {
        // conn.close();
        delay(50);
        // if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        // {
        // delay(500);
        WriteCfgProfiliNewToDB();
        // }
        // else
        // {
        //   DBConnected = false;
        // }
      }
      // }
      // else
      // {
      //   DBConnected = false;
      // }
      // conn.close();
      delay(50);
      // if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
      // {
      //   delay(500);
      LoadDBTime = DesTimeFromDBtoLocal(0);
      if (LoadDBTime)
      {
        // conn.close();
        delay(50);
        // if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        // {
        //   delay(500);
        WriteCfgTimeNewToDB();
        // }
        // else
        // {
        //   DBConnected = false;
        // }
      }
      // }
      // else
      // {
      //   DBConnected = false;
      // }
      // conn.close();
      delay(50);
      // if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
      // {
      //   delay(500);
      UpdateFW();
      // }
      // else
      // {
      //   DBConnected = false;
      // }
      DBsyncStartup = true;
      yield();
      DPRINTLN("Run DB Connect");
      //conn.close();
      delay(50);
      // if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
      // {
      //   delay(500);
      if (getCfgfromDB(0))
      {
        conn.close();
        delay(50);
        if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        {
          delay(500);
          WriteCfgNewToDB();
        }
        else
        {
          DBConnected = false;
        }
      }
      // }
      // conn.close();
      delay(50);
      // if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
      // {
      LoadDBBadge = BadgeFromDBtoLocal(0);
      if (LoadDBBadge)
      {
        // conn.close();
        delay(50);
        // if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        // {
        //   delay(500);
        WriteCfgBadgeNewToDB();
        // }
        // else
        // {
        //   DBConnected = false;
        // }
      }
      // }
      // else
      // {
      //   DBConnected = false;
      // }
    }
    if (LoadDBProfili == 0)
    {
      ProfiliFromSDtoLocal();
      readProfilesTable();
    }
    if (LoadDBTime == 0)
    {
      DesTimeFromSDtoLocal();
      readTimeTable();
    }
    if (LoadDBBadge == 0)
    {
      BadgeFromSDtoLocal();
      readBadgeTable();
    }
    if ((LoadDBProfili == 1) && (LoadDBTime == 1) && (LoadDBBadge == 1))
    {
      tft.fillScreen(TFT_WHITE);
      tft.setCursor((320 - tft.textWidth("Connessione al DataBase ok")) / 2, 110);
      tft.println(("Connessione al DataBase ok"));
      tft.setCursor((320 - tft.textWidth("Connessione al DataBase ok")) / 2, 130);
      tft.println(("Tabelle CFG Ricevute"));
      if (config.Log == 1)
      {
        writeLog("CFG Tables Received in SETUP");
      }
      delay(1000);
    }
    else
    {
      tft.fillScreen(TFT_WHITE);
      tft.setCursor((320 - tft.textWidth("Connessione al DataBase KO")) / 2, 110);
      tft.println(("Connessione al DataBase KO"));
      tft.setCursor((320 - tft.textWidth("Connessione al DataBase KO")) / 2, 130);
      tft.println(("Carico Tabelle da SD"));
      if (config.Log == 1)
      {
        writeLog("Connessione al DataBase KO Carico Tabelle da SD");
      }
      delay(1500);
    }
  }
  else
  {
    tft.fillScreen(TFT_WHITE);
    tft.setCursor((320 - tft.textWidth("Server non raggiungibile")) / 2, 110);
    tft.println(("Server non raggiungibile"));
    tft.setCursor((320 - tft.textWidth("Modalita' off-line")) / 2, 130);
    tft.println(("Modalita' off-line"));
    DBsyncStartup = false;
    DPRINTLN("================================================");
    DPRINTLN("                off-line Mode                   ");
    DPRINTLN("================================================");

    if (config.Log == 1)
    {
      writeLog("Server unreachable Off-line mode");
    }
    delay(300);
    BadgeFromSDtoLocal();
    readBadgeTable();
    delay(300);
    ProfiliFromSDtoLocal();
    //readProfilesTable();
    delay(300);
    DesTimeFromSDtoLocal();
    //readTimeTable();
    if (config.Log == 1)
    {
      writeLog("Conf from SD is ok (Off Line)");
    }
  }
  printVar();

  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(FF5);
  tft.setCursor((320 - tft.textWidth("Avvio programmi")) / 2, 120);
  tft.print(F("Avvio programmi")); // good day
  delay(tempolettura / 2);
  tft.fillScreen(TFT_WHITE);
  tft.setCursor(10, 25);
  digitalWrite(LCD_BACKLIGHT, HIGH);
  tft.fillScreen(TFT_WHITE);
  tft.setFreeFont(FF5);

  esponidata();
  drawLogo();
  delay(300);

  if (config.Log == 1)
  {
    writeLog("Setup end");
  }
  state_5S_PRESS = HIGH;

  prog[5] = prog[1] * 3;
  prog[6] = prog[1] * 4;
  updateConnDB.start(3000);
  updateNTP.start(config.NTPUpDate * 60 * 60 * 1000); // update time via ntp
  updateOra.start(1000);
  updateDate.start(1 * 60 * 60 * 1000);
  updateCfg.start(config.SyncDB * 1000);
  prog[1] = config.TransitUpdate * 1000;
  updateServer.start(updateServerDelay);
  badgePrevius.start(5000);
  ResetETHDelay.start(updateResetETHDelay);
  tftTimer.start(config.tftTimeout * 1000);
  if (config.DelLog == 1)
  {
    if (testFileIO(SD, filenameLog))
    {
      DPRINTLN("File Log present");
      int flenlog = FileLen(SD, filenameLog);
      if (flenlog > 15000)
      {
        deleteFile(SD, filenameLog);
        DPRINTLN("File Log too large deleted");
      }
      else
      {

        delay(200);

        if (BufLogToDB())
        {
          DPRINTLN("Ritorno da BufLogToDB");
          delay(300);
          deleteFile(SD, filenameLog);
          DPRINTLN("File Log deleted");
          if (config.Log == 1)
          {
            writeLog("Log update on DB");
          }
        }
        else
        {
          DPRINTLN("==============================================");
          DPRINTLN("          Query Insert Log error              ");
          DPRINTLN("==============================================");
          if (config.Log == 1)
          {
            writeLog("LogToDB Error");
          }
        }
      }
    }
    else
    {
      DPRINTLN("Log File not present");
      BufBadgePresent = 0;
    }
    timer[6] = 120000;
  }

  t1 = millis();
}

void loop()
{
  if (updateServerDelay >= 40000 && ETHStartReset == false)
  {
    ETHStartReset = true;
    //
    do
    {
      client.flush();
    } while (client.available());
    client.stop();
    //EthernetClient client;
    delay(1000);
    digitalWrite(ResetETH, LOW);
    DPRINTLN("Reset Network Card in loop");
    if (config.Log == 1)
    {
      writeLog("Reset Network Card in loop");
    }
    // Wire.begin();
    ETHReseted = true;
    // ETH_INIT = false;
    timer[8] = 1000;
    // timer[9] = 4000;
    // timer[10] = 10000;
    // conn.close();
    // delay(500);
    // digitalWrite(ResetETH, HIGH);
    // delay(1000);
    // Ethernet.init(USE_THIS_SS_PIN);
    // delay(3000);
    // Ethernet.begin(macAdd, myIP, myDNS, myGW, myMASK);
    // DPRINTLN("Network card configured");
    // delay(3000);

    // DPRINTLN(Ethernet.localIP());
  }
  if (ETHEndReset)
  {
    conn.close();

    ETHEndReset = false;
    //client.stop();
    MySQL_Connection conn((Client *)&client);
    delay(5000);
    DPRINTLN("ETHEndReset");
    if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL) // check if server is on
    //if (conn.connect(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam)) // check if server is on
    {
      delay(700);

      if (KeepAlive() == true)
      {
        updateServerDelay = 20000;
        updateServer.start(updateServerDelay);
        DPRINT("updateServerDelay= ");
        DPRINTLN(updateServerDelay);
        if (config.Log == 1)
        {
          writeLog("KeepAlive OK");
          timer[5] = prog[5]; // reset timer KeepAlive
        }
      }
      else
      {
        if (config.Log == 1)
        {
          writeLog("KeepAlive KO");
          timer[5] = 1000;
        }
      }
    }
    else
    {
      updateServerDelay = 300000;
      updateServer.start(updateServerDelay);
      DPRINT("updateServerDelay= ");
      DPRINTLN(updateServerDelay);
      DBConnected = false;
    }
  }

  dt = millis() - t1;
  if (dt > 10) // Update timer
  {
    for (int t = 0; t < N_ELEM; t++)
    {
      if (timer[t] > 0)
      {
        timer[t] = timer[t] - 10;
      }
    }
    t1 = millis();
  }
  if ((timer[0] == 0) && (lastDrawLogo == 0)) // Display Timer
  {
    lastDrawLogo = 1;
    drawLogo();
    if (statoLCD_BACKLIGHT == LOW)
    {
      tftTimer.restart();
    }
  }
  if ((timer[1] == 0) && (BufBadgePresent == 1) && (DBConnected == true)) // Update Badge Timer
  {
    if (statoLCD_BACKLIGHT == LOW)
    {
      digitalWrite(LCD_BACKLIGHT, HIGH);
    }
    if (testFileIO(SD, filenameAccessBuf))
    {
      DPRINTLN("AccessBuf File present");
      delay(600);
      if (BufAccessToDB())
      {
        deleteFile(SD, filenameAccessBuf);
        BufBadgePresent = 0;
        if (config.Log == 1)
        {
          writeLog("Transit on DB Updated");
        }
      }
      else
      {
        DPRINTLN("==============================================");
        DPRINTLN("          Query Insert Transit error          ");
        DPRINTLN("==============================================");
        if (config.Log == 1)
        {
          writeLog("BufAccessToDB Error");
        }
      }
    }
    else
    {
      DPRINTLN("AccessBuf File not present");
      BufBadgePresent = 0;
    }
    if (statoLCD_BACKLIGHT == LOW)
    {
      tftTimer.restart();
    }
  }

  if (timer[2] == 0) // Retet Buzzer Timer
  {
    analogWrite(WIO_BUZZER, 0);
    digitalWrite(beeper, HIGH);
  }
  if (timer[3] == 0)
  {
    digitalWrite(rele1, LOW);
  }
  if (timer[4] == 0)
  {
    digitalWrite(rele2, LOW);
  }
  if (timer[5] == 0 && DBConnected == true && ReteConnected == true)
  {
    timer[5] = prog[5]; // reset timer KeepAlive
    DPRINTLN("KeepALive connection");
    conn.close();
    delay(50);
    if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
    {
      delay(500);

      if (KeepAlive() == true)
      {
        if (config.Log == 1)
        {
          writeLog("KeepAlive OK");
          timer[5] = prog[5]; // reset timer KeepAlive
        }
      }
      else
      {
        if (config.Log == 1)
        {
          writeLog("KeepAlive KO");
          timer[5] = 1000;
        }
      }
    }
    else
    {
      DBConnected = false;
    }
  }
  if (timer[6] == 0 && DBConnected == true && ReteConnected == true)
  {
    timer[6] = prog[6]; // reset timer updateLog
    if (config.DelLog == 1)
    {
      if (testFileIO(SD, filenameLog))
      {
        DPRINTLN("Log File present");
        int flenlog = FileLen(SD, filenameLog);
        if (flenlog > 15000)
        {
          deleteFile(SD, filenameLog);
          DPRINTLN("File Log too large deleted");
        }
        else
        {

          delay(600);
          if (BufLogToDB())
          {
            delay(300);
            deleteFile(SD, filenameLog);
            DPRINTLN("File Log deleted");
            if (config.Log == 1)
            {
              writeLog("update Log on DB");
            }
          }
          else
          {
            DPRINTLN("==============================================");
            DPRINTLN("          Query Insert Log error");
            DPRINTLN("==============================================");
            if (config.Log == 1)
            {
              writeLog("LogToDB Error");
            }
          }
        }
      }
      else
      {
        DPRINTLN("Log File not present");
        BufBadgePresent = 0;
      }
    }
    if (statoLCD_BACKLIGHT == LOW)
    {
      tftTimer.restart();
    }
    drawLogo();
  }
  // Reset previus badge
  if (badgePrevius.justFinished())
  {
    badgePrevius.restart();
    prev_badge = 0;
  }

  /* Update Display Server on-line/off-line */
  /******************************************/
  if (updateServer.justFinished() || startup)
  {
    startup = false;
    //updateServer.restart();

    DPRINTLN("updateServer.justFinished in Agg. Display");
    DPRINT("updateConnDBDelay= ");
    DPRINTLN(updateConnDBDelay);

    if ((DBConnected == true) && (ReteConnected == true))
    {
      tft.fillRect(xCleanServer, yCleanServer, WCleanServer, HCleanServer, ILI9341_WHITE);
      tft.setCursor(xServer, yServer);
      tft.setTextColor(TFT_BLACK);
      tft.print(F("Server on-line"));
      DPRINTLN("updateServer.justFinished in Agg. Display e DBConnected");
      updateServerDelay = 20000;
      updateServer.start(updateServerDelay);
    }
    else
    {
      updateServer.restart();
      if ((ReteConnected == true))
      {
        if ((ETHStartup == true))
        {
          tft.fillRect(xCleanServer, yCleanServer, WCleanServer, HCleanServer, ILI9341_WHITE);
          tft.setCursor(xServer, yServer);
          tft.setTextColor(TFT_RED);
          tft.print(F("Server off-line"));
          tft.setTextColor(TFT_BLACK);
          if (config.Log == 1)
          {
            writeLog("Server off-line");
          }
          if (dnsReturn == 0)
          {
            DNSClient dns;
            dns.begin(Ethernet.dnsServerIP());
            dnsReturn = dns.getHostByName(hostname, server_ip);
          }
          if (devicetime == 0)
          {
            for (j = 0; j < 3; j++)
            {
              devicetime = getNTPtime();
              // check if NTP response
              if (devicetime == 0 || j == 2)
              {
                DPRINTLN("Failed to get time from network time server after recovery network.");
                DBConnected == false;
                if (updateServerDelay >= 900000)
                {
                  updateServerDelay = 900000;
                  updateServer.start(updateServerDelay);
                }
                else
                {
                  updateServerDelay = updateServerDelay * 2;
                  updateServer.start(updateServerDelay);
                  DPRINT("updateServerDelay= ");
                  DPRINTLN(updateServerDelay);
                }
                if (config.Log == 1)
                {
                  writeLog("Failed to get time from network time server after recovery network");
                }
              }
              else
              {
                DateTime now = rtc.now();
                time_t utc = now.get_time_t();
                time_t local = myTZ.toLocal(utc, &tcr);
                DPRINTLN("rtc time updated after recovery network.");
                // get and print the adjusted rtc time
                DPRINT("Adjusted RTC time is: ");
                printDateTime(local, tcr->abbrev);
                if (config.Log == 1)
                {
                  String txtlog = ("rtc time updated after recovery network. " + printLocalTime(local));
                  writeLog(txtlog);
                }
                esponidata();
                break;
              }
            }
          }
          if (devicetime >= 1)
          {
            DPRINT("Nel updateserver Host =");
            DPRINTLN(hostname);
            DPRINT("Host resolved = ");
            DPRINTLN(server_ip);
            conn.close();
            delay(50);
            if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
            {
              DPRINTLN("LANCIO UPDATEFW");

              UpdateFW();
              conn.close();
              delay(50);
              if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
              {

                delay(500);
                getCfgVarLast(1);
              }
              if (!conn.connected())
              {
                DBConnect();
                delay(500);
              }
              if (conn.connected())
              {
                getBadgeVarLast(1);
              }
              if (!conn.connected())
              {
                DBConnect();
                delay(500);
              }
              if (conn.connected())
              {
                delay(200);
                getDesProfiliVarLast(1);
              }
              if (!conn.connected())
              {
                DBConnect();
                delay(500);
              }
              if (conn.connected())
              {
                delay(200);
                getDesTimeVarLast(1);
              }
              if (!conn.connected())
              {
                DBConnect();
                delay(500);
              }
              if (conn.connected())
              {
                delay(200);
                getChangeTabelVarLast(1);
              }
              if (conn.connected())
              {
                tft.fillRect(xCleanServer, yCleanServer, WCleanServer, HCleanServer, ILI9341_WHITE);
                tft.setCursor(xServer, yServer);
                tft.setTextColor(TFT_BLACK);
                tft.print(F("Server on-line"));
                updateServerDelay = 20000;
                updateServer.start(updateServerDelay);
                if (config.Log == 1)
                {
                  writeLog("Server on-line");
                }
              }
            }
            else
            {
              DBConnected = false;
              DPRINTLN("Failed to connect");
              if (updateServerDelay >= 900000)
              {
                updateServerDelay = 900000;
                updateServer.start(updateServerDelay);
              }
              else
              {
                updateServerDelay = updateServerDelay * 2;
                updateServer.start(updateServerDelay);
                DPRINT("updateServerDelay= ");
                DPRINTLN(updateServerDelay);
              }
            }
          }
        }
        else
        {
          DPRINTLN("Ethernet fail");
          DPRINT("ReteConnected = ");
          DPRINT(ReteConnected);
          DPRINT(" ETHStartup= ");
          DPRINTLN(ETHStartup);
          if (updateServerDelay >= 900000)
          {
            updateServerDelay = 900000;
            updateServer.start(updateServerDelay);
          }
          else
          {
            updateServerDelay = updateServerDelay * 2;
            updateServer.start(updateServerDelay);
            DPRINT("updateServerDelay= ");
            DPRINTLN(updateServerDelay);
          }
        }
      }
    }
  }
  /* I turn on / off the display backlight    */
  /********************************************/
  if (state_5S_PRESS == LOW)
  {
    while (digitalRead(WIO_5S_PRESS) == LOW)
      ;
    state_5S_PRESS = 1;
    while (1)
    {
      statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
      digitalWrite(LCD_BACKLIGHT, !statoLCD_BACKLIGHT);
      tftTimer.restart();
      DPRINTLN("TFT LED change switch");
      if (do_break == 1)
      {
        do_break = 0;
        break;
      }
    }
  }

  if (state_a == LOW)
  {
#if Mensa
    while (digitalRead(WIO_5S_UP) == LOW)
      ;
#elif Presenze
    while (digitalRead(WIO_5S_UP) == LOW)
      ;
#elif PMensa
    while (digitalRead(WIO_5S_UP) == LOW)
      ;
#elif CA
    while (digitalRead(WIO_5S_UP) == LOW)
      ;
#endif
    state_a = HIGH;
    while (1)
    {
      infoScreen();
      if (do_break == 1)
      {
        do_break = 0;
        tft.fillScreen(ILI9341_WHITE);
        esponidata();
        drawLogo();
        break;
      }
    }
  }

  if (config.tftTimeout > 0)
  {
    if (tftTimer.justFinished())
    {
      DPRINTLN("Turn off the display for timeout");
      digitalWrite(LCD_BACKLIGHT, LOW);
      tftTimer.restart();
    }
  }

  /*       Update time from NTP      */
  /************************************/
  if ((updateNTP.justFinished()) && (ReteConnected == true) && (DBConnected == true))
  {
    for (j = 0; j < 4; j++)
    {
      devicetime = getNTPtime();
      if (devicetime == 0 || j == 3)
      {
        DPRINTLN("Failed to get time from network time server in the updateNTP");
        DPRINTLN("updateNTP= 60");
        updateNTP.start(60 * 1000);
        if (config.Log == 1)
        {
          writeLog("Failed to get time from network time server in the updateNTP");
        }
      }
      else
      {
        DPRINTLN("rtc time updated in the updateNTP.");
        // get and print the adjusted rtc time
        DateTime now = rtc.now();
        time_t utc = now.get_time_t();
        time_t local = myTZ.toLocal(utc, &tcr);
        DPRINT("Adjusted RTC time is: ");
        DPRINTLN(printLocalTime(local));
        DPRINTLN("updateNTP= " + String(config.NTPUpDate) + "* 60 * 60 * 1000");
        updateNTP.start(config.NTPUpDate * 60 * 60 * 1000); // update time via ntp
        if (config.Log == 1)
        {
          String txtlog = ("rtc time updated in the updateNTP. " + printLocalTime(local));
          writeLog(txtlog);
        }
        esponidata();
        break;
      }
    }
  }

  /*   Clock Update Display    */
  /*****************************/
  if (updateDate.justFinished())
  {
    // repeat timer
    updateDate.restart();
    esponidata();
  }
  if (updateOra.justFinished())
  {
    // repeat timer
    esponioraciclo();

    updateOra.restart();
  }
  /* Check if any configuration updates are available on the server */
  if ((ReteConnected == true) && (dnsReturn == true) && (DBConnected == true))
  {
    if (updateCfg.justFinished())
    {
      // repeat timer
      updateCfg.restart();
      DPRINTLN("Connessione per updateCFG");
      conn.close();
      delay(50);
      if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
      {
        delay(500);

        getChangeTabelVarLast(2);
        DPRINT("updateConnDBDelay= ");
        DPRINTLN(updateConnDBDelay);
      }
      else
      {
        DBConnected = false;
      }
      if (LoadDBProfili == 0)
      {
        statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
        if (statoLCD_BACKLIGHT == LOW)
        {
          digitalWrite(LCD_BACKLIGHT, HIGH);
          tftTimer.restart();
        }
        DPRINTLN("Connection for LoadDBProfili");
        conn.close();
        delay(50);
        if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        {
          delay(500);

          if (LoadDBProfili = ProfiliFromDBtoLocal(1))
          {
            if ((config.Log == 1) && (LoadDBProfili == 1))
            {
              writeLog("LoadDBProfili ok I come from LoadDBProfili = 0");
            }
          }
        }
        else
        {
          DBConnected = false;
        }
      }
      if (LoadDBTime == 0)
      {
        statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
        if (statoLCD_BACKLIGHT == LOW)
        {
          digitalWrite(LCD_BACKLIGHT, HIGH);
          tftTimer.restart();
        }
        DPRINTLN("Connection for LoadDBTime");
        conn.close();
        delay(50);
        if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        {
          delay(500);

          if (LoadDBTime = DesTimeFromDBtoLocal(1))
          {
            if ((config.Log == 1) && (LoadDBTime == 1))
            {
              writeLog("LoadDBTime ok I come from LoadDBTime = 0");
            }
          }
        }
        else
        {
          DBConnected = false;
        }
      }
      if (LoadDBBadge == 0)
      {
        statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
        if (statoLCD_BACKLIGHT == LOW)
        {
          digitalWrite(LCD_BACKLIGHT, HIGH);
          tftTimer.restart();
        }
        DPRINTLN("Connection for LoadDBBadge");
        conn.close();
        delay(50);
        if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        {
          delay(500);

          if (LoadDBBadge = BadgeFromDBtoLocal(1))
          {
            if ((config.Log == 1) && (LoadDBBadge == 1))
            {
              writeLog("LoadDBBadge ok I come from LoadDBBadge = 0");
            }
          }
        }
        else
        {
          DBConnected = false;
        }
      }
      if (cfgTabel != cfgTabelLast && DBConnected == true)
      {
        tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
        tft.drawRect(xcur - 15, ycur - 60, 300, 150, ILI9341_BLUE);
        tft.setCursor(xcur, ycur + 20);
        tft.print("Attendere prego");
        tft.setCursor(xcur, ycur + 40);
        tft.println(("Aggiorn. Tabelle"));
        DPRINTLN("Connessione per cfgTabLast");
        conn.close();
        delay(50);
        if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        {

          delay(500);
          getCfgVar();
        }
        else
        {
          DBConnected = false;
          drawLogo();
        }
        conn.close();
        delay(50);
        if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        {
          delay(500);

          getCfgVarLast(1);
        }
        else
        {
          DBConnected = false;
        }
        if (config.Log == 1)
        {
          writeLog("New configuration present");
        }
        if (cfgTime != cfgTimeLast && DBConnected == true)
        {
          conn.close();
          delay(50);
          if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
          {
            delay(500);

            WriteCfgNewToDB();
          }
          else
          {
            DBConnected = false;
          }
          conn.close();
          delay(50);
          if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
          {
            delay(500);

            WriteChangeTabelNewToDB();
          }
          else
          {
            DBConnected = false;
          }
          DPRINTLN("CFG2 Changed reboot");
          if (config.Log == 1)
          {
            writeLog("CFG2 Changed reboot");
          }
          delay(1500);
          NVIC_SystemReset();
        }
        else
        {
          DPRINTLN("Network configuration not changed");
        }
        conn.close();
        delay(50);
        if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        {
          delay(500);

          getBadgeVarLast(1);
        }
        else
        {
          DBConnected = false;
        }
        if (!conn.connected())
        {
          DBConnect();
          delay(500);
        }
        if (conn.connected())
        {
          getBadgeVar();
        }
        if (cfgBadge != cfgBadgeLast && DBConnected == true)
        {
          UpdateDBok = false;
          if (config.Log == 1)
          {
            writeLog("New Badge configuration present");
          }
          statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
          if (statoLCD_BACKLIGHT == LOW)
          {
            digitalWrite(LCD_BACKLIGHT, HIGH);
          }
          conn.close();
          delay(50);
          if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
          {
            delay(500);

            WriteCfgBadgeNewToDB();
          }
          else
          {
            DBConnected = false;
          }
          conn.close();
          delay(50);
          if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
          {
            delay(500);

            LoadDBBadge = BadgeFromDBtoLocal(1);
            if ((config.Log == 1) && (LoadDBProfili == 1))
            {
              writeLog("Badge Table Updated");
            }
          }
          else
          {
            DBConnected = false;
          }
        }
        else
        {
          UpdateDBok = true;
        }
        conn.close();
        delay(50);
        if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        {
          delay(500);

          getDesProfiliVarLast(1);
        }
        else
        {
          DBConnected = false;
        }
        if (!conn.connected())
        {
          DBConnect();
          delay(500);
        }
        if (conn.connected())
        {
          getDesProfiliVar();
        }
        if (cfgDesProfili != cfgDesProfiliLast && DBConnected == true)
        {
          UpdateDBok = false;
          if (config.Log == 1)
          {
            writeLog("New Profiles configuration present");
          }
          statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
          if (statoLCD_BACKLIGHT == LOW)
          {
            digitalWrite(LCD_BACKLIGHT, HIGH);
          }
          conn.close();
          delay(50);
          if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
          {
            delay(500);

            WriteCfgProfiliNewToDB();
          }
          else
          {
            DBConnected = false;
          }
          conn.close();
          delay(50);
          if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
          {
            delay(500);

            LoadDBProfili = ProfiliFromDBtoLocal(1);
          }
          else
          {
            DBConnected = false;
          }
          if ((config.Log == 1) && (LoadDBProfili == 1))
          {
            writeLog("Profiles Table Updated");
          }
        }
        else
        {
          UpdateDBok = true;
        }
        conn.close();
        delay(50);
        if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
        {
          delay(500);

          getDesTimeVarLast(1);
        }
        else
        {
          DBConnected = false;
        }
        if (!conn.connected())
        {
          DBConnect();
          delay(500);
        }
        if (conn.connected())
        {

          getDesTimeVar();
        }
        if (cfgDesTime != cfgDesTimeLast && DBConnected == true)
        {
          UpdateDBok = false;
          if (config.Log == 1)
          {
            writeLog("New DesTime configuration present");
          }
          statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
          if (statoLCD_BACKLIGHT == LOW)
          {
            digitalWrite(LCD_BACKLIGHT, HIGH);
          }
          conn.close();
          delay(50);
          if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
          {
            delay(500);

            WriteCfgTimeNewToDB();
          }
          else
          {
            DBConnected = false;
          }
          conn.close();
          delay(50);
          if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
          {
            delay(500);

            LoadDBTime = DesTimeFromDBtoLocal(1);
          }
          else
          {
            DBConnected = false;
          }
          if ((config.Log == 1) && (LoadDBTime == 1))
          {
            writeLog("Time Slots Table Updated");
          }
        }
        else
        {
          UpdateDBok = true;
        }
        if (UpdateDBok == true)
        {
          conn.close();
          delay(50);
          if (conn.connectNonBlocking(server_ip, config.DBPortParam, config.DBUserParam, config.DBPswParam) != RESULT_FAIL)
          {
            delay(500);
            WriteChangeTabelNewToDB();
          }
          else
          {
            DBConnected = false;
          }
        }
        drawLogo();
      }
    }
  }

#ifdef NoWiFi
  /*     Check if the ethernet link is on     */
  /********************************************/
  LinkStatus = Ethernet.linkStatus();
  if (timer[7] == 0 && ETHReseted == false && ETH_INIT == false)
  {
    timer[7] = prog[7];
    if (LinkStatus == 2)
    {

      DPRINTLN("Ethernet cable is not connected in link status.");
      tft.fillRect(xCleanServer, yCleanServer, WCleanServer, HCleanServer, ILI9341_WHITE);
      tft.setCursor(xServer, yServer);
      tft.setTextColor(TFT_RED);
      tft.print(("Server off-line"));
      ReteConnected = false;
      tft.fillRect(xCleanServer + WCleanServer + 40, yServer - 20, 80, 25, ILI9341_WHITE);
      tft.setCursor(xCleanServer + WCleanServer + 40, yServer);
      tft.print(("ETH OFF"));
      if (ResetETHDelay.justFinished())
      {
        if (updateResetETHDelay >= 1800000)
        {
          updateResetETHDelay = 1800000;
        }
        else
        {
          updateResetETHDelay = updateResetETHDelay * 2;
        }
        ResetETHDelay.start(updateResetETHDelay);
        DPRINT("updateResetETHDelay= ");
        DPRINTLN(updateResetETHDelay);
        //conn.close();
        do
        {
          client.flush();
        } while (client.available());
        client.stop();
        //EthernetClient client;
        delay(1000);
        digitalWrite(ResetETH, LOW);

        DPRINTLN("Reset Network Card in loop 3");
        if (config.Log == 1)
        {
          writeLog("Reset Network Card in loop 3");
        }
        // Wire.begin();
        ETHReseted = true;
        // ETH_INIT = false;
        timer[8] = 1000;
        // timer[9] = 4000;
        // timer[10] = 10000;
        // delay(500);
        // digitalWrite(ResetETH, HIGH);
        // delay(1000);
        // Ethernet.init(USE_THIS_SS_PIN);
        // delay(3000);
        // Ethernet.begin(macAdd, myIP, myDNS, myGW, myMASK);
        // DPRINTLN("Network card configured");
        // delay(3000);
        // DPRINTLN(Ethernet.localIP());
      }
    }
    else if (LinkStatus == 1)
    {
      if (ETHEndReset)
      {
        ETHEndReset = false;
        ETHStartReset = false;
        DPRINTLN("Reset end in loop3");
      }
      tft.fillRect(xCleanServer + WCleanServer + 40, yServer - 20, 80, 25, ILI9341_WHITE);
      tft.setCursor(xCleanServer + WCleanServer + 40, yServer);
      tft.setTextColor(TFT_BLACK);
      tft.print(("ETH ON"));
      ETHStartup = true;
      ReteConnected = true;
      updateResetETHDelay = 5000;
      ResetETHDelay.start(updateResetETHDelay);
      if (DBsyncStartup == false)
      {
        timer[1] = 0;
        BufBadgePresent = 1;
        DBsyncStartup = 1;
      }
    }
  }
  if (timer[8] == 0 && ETHReseted == true) // delay ResetETH = LOW
  {
    DPRINTLN("ETH go on");
    digitalWrite(ResetETH, HIGH);
    ETHReseted = false;
    timer[9] = 1000;
    ETH_INIT = true;
  }
  if (timer[9] == 0 && ETH_INIT == true) // delay for init ethernet shield
  {
    MYSQL_LOGERROR3(("Board :"), BOARD_NAME, (", setCsPin:"), USE_THIS_SS_PIN);
    DPRINTLN("\nEthernet shield inizialize");
    Ethernet.init(USE_THIS_SS_PIN);
    MYSQL_LOGERROR(F("========================================="));
    MYSQL_LOGERROR(F("Currently Used SPI pinout:"));
    MYSQL_LOGERROR1(F("MOSI:"), MOSI);
    MYSQL_LOGERROR1(F("MISO:"), MISO);
    MYSQL_LOGERROR1(F("SCK:"), SCK);
    MYSQL_LOGERROR1(F("SS:"), SS);
    MYSQL_LOGERROR(F("========================================="));
    ETH_INIT = false;
    ETH_begin = true;
    timer[10] = 1000;
  }
  if (timer[10] == 0 && ETH_begin) // delay for startup Ethernet shield
  {
    ETH_begin = false;
    Ethernet.begin(macAdd, myIP, myDNS, myGW, myMASK);
    DPRINTLN("Network card configured");
    ETHEndReset = true;
    LinkStatus = Ethernet.linkStatus();
    if (LinkStatus == true)
    {
      ReteConnected = true;
    }
    DPRINT("LinkStatus= ");
    DPRINTLN(LinkStatus);
    DPRINT("updateServerDelay= ");
    DPRINTLN(updateServerDelay);
    //     ETHReseted = false;
  }
  // }
#else
// Implement functions for WiFi here
#endif

  /*       Check if there is an interrupt from a badge present        */
  /********************************************************************/
  if (isData0Low || isData1Low)
  {
    // DPRINTLN("read card number bit");
    if (1 == recvBitCount)
    { //even bit
      evenBit = (1 - isData0Low) & isData1Low;
    }
    else if (recvBitCount >= 26)

    { //odd bit
      oddBit = (1 - isData0Low) & isData1Low;
      isCardReadOver = 1;
      delay(10);
    }
    else
    {
      //only if isData1Low = 1, card bit could be 1
      RFIDcardNum[2 - (recvBitCount - 2) / 8] |= (isData1Low << (7 - (recvBitCount - 2) % 8));
    }
    //reset data0 and data1
    isData0Low = 0;
    isData1Low = 0;
    isData2Low = 0;
    isData3Low = 0;
#if Mensa
    direction = "Presente";
    entrata_uscita = 3;
#elif PMensa
    direction = "Presente";
    entrata_uscita = 3;
#elif Presenze
    direction = "ENTRATA";
    entrata_uscita = 3;
#elif CA
    direction = "ENTRATA";
    entrata_uscita = 1;
#endif
  }

  if (isData2Low || isData3Low)
  {
    // DPRINTLN("read card number bit");
    if (1 == recvBitCount)
    { //even bit
      evenBit2 = (1 - isData2Low) & isData3Low;
    }
    else if (recvBitCount >= 26)
    //else if (recvBitCount >= 42)
    { //odd bit
      oddBit2 = (1 - isData2Low) & isData3Low;
      isCardReadOver = 1;
      delay(10);
    }
    else
    {
      //only if isData3Low = 1, card bit could be 1
      RFIDcardNum[2 - (recvBitCount - 2) / 8] |= (isData3Low << (7 - (recvBitCount - 2) % 8));
    }
    //reset data0 and data1
    isData0Low = 0;
    isData1Low = 0;
    isData2Low = 0;
    isData3Low = 0;
#if Mensa
    direction = "Presente";
    entrata_uscita = 3;
#elif PMensa
    direction = "Presente";
    entrata_uscita = 3;
#elif Presenze
    direction = "USCITA";
    entrata_uscita = 3;
#elif CA
    direction = "USCITA";
    entrata_uscita = 2;
#endif
  }

  if (isCardReadOver)
  {
    if (checkParity())
    {
      // DPRINTLN("\n****************after checkParity**********");
      // DPRINT("RFIDcardNum HEX= ");
      // DPRINT(RFIDcardNum[4], HEX);
      // DPRINT(RFIDcardNum[3], HEX);
      // DPRINT(RFIDcardNum[2], HEX);   //High byte
      // DPRINT(RFIDcardNum[1], HEX);   //Midle byte
      // DPRINTLN(RFIDcardNum[0], HEX); //Low byte
      rfidValue = 0;
      rfidIdBadge = 0;
      rfidValue = (*((long *)RFIDcardNum));
      DPRINT("rfidValue= ");
      DPRINTLN(rfidValue);
      DateTime now = rtc.now();
      time_t utc = now.get_time_t();
      time_t local = myTZ.toLocal(utc, &tcr);
      DPRINT("Badge read = ");
      DPRINT(rfidValue);
      DPRINT(" ");
      DPRINT(direction);
      DPRINT(" entrata_uscita= ");
      DPRINT(entrata_uscita);
      DPRINT(" -at time- ");
      printDateTime(local, tcr->abbrev);
    }
    resetData();
    if (rfidValue != 0)
    {
      if (prev_badge != rfidValue)
      {
        if (checkCard())
        {
          tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_WHITE);
          tft.setFreeFont(FF6);
          DPRINTLN("Authorized Badge");
          DPRINT("Surname= ");
          DPRINTLN(rfidCognome);
          DPRINT("Name= ");
          DPRINTLN(rfidNome);
          tft.setTextColor(ILI9341_BLUE);
          byte xcur = 25;
          byte ycur = 80;
          tft.drawRect(xcur - 15, ycur - 30, 300, 150, ILI9341_BLUE);
          tft.setCursor(xcur, ycur);
          tft.print("Badge Autorizzato");
          tft.setCursor(xcur, ycur + 25);
          tft.print(rfidCognome);
          tft.setCursor(xcur, ycur + 50);
          tft.print(rfidNome);
          tft.setCursor(xcur, ycur + 100);
          tft.print(rfidUIDdec);
          statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
          if (statoLCD_BACKLIGHT == LOW)
          {
            digitalWrite(LCD_BACKLIGHT, HIGH);
            //tftTimer.restart();
          }
          resetData();
          tft.setCursor(xcur, ycur + 75);
          esponiora();
          tft.print(" - ");
          tft.print(direction);
          if (config.BeepOnOff != 0)
          {
            analogWrite(WIO_BUZZER, 128);
            digitalWrite(beeper, LOW);
            timer[2] = 300;
          }
#ifdef CA
          if (config.SingleGate == 1)
          {
            digitalWrite(rele1, HIGH);
            timer[3] = (config.TimeGate1);
          }
          else if (config.SingleGate == 0)
          {
            if (entrata_uscita == 0)
            {
              digitalWrite(rele1, HIGH);
              timer[3] = (config.TimeGate1);
            }
            else if (entrata_uscita == 1)
            {
              digitalWrite(rele2, HIGH);
              timer[4] = (config.TimeGate2);
            }
          }
#endif
          BufBadgePresent = 1;
          timer[1] = prog[1]; // reset timer updatetransit
          timer[5] = prog[5]; // reset timer KeepAlive
          timer[6] = prog[6]; // reset timer updateLog
          timer[0] = prog[0]; // reset timer Display
          writeTransit(rfidValue, entrata_uscita, 1, rfidIdBadge);
          lastDrawLogo = 0;
          tft.setFreeFont(FF5);
        }
        else
        {
          statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
          tft.setTextColor(ILI9341_WHITE);
          tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_RED);
          tft.setFreeFont(FF6);
          byte xcur = 25;
          byte ycur = 80;
          tft.drawRect(xcur - 15, ycur - 30, 300, 150, ILI9341_WHITE);
          tft.setCursor(xcur, ycur);
          DPRINT("isBadgeOk= ");
          DPRINTLN(isBadgeOk);
          DPRINT("isProfiloOk= ");
          DPRINTLN(isProfiloOk);
          DPRINT("isGateOk= ");
          DPRINTLN(isGateOk);
          if (isBadgeOk == 0)
          {
            DPRINTLN("Invalid badge");
            tft.print("Badge non valido");
            tft.setCursor(xcur, ycur + 25);
            statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
            DPRINTLN("Invalid badge= " + String(rfidValue));
            DPRINTLN("rfidIdBadge= " + String(rfidIdBadge));
            writeTransit(rfidValue, entrata_uscita, 0, rfidIdBadge);
            if (config.BeepOnOff != 0)
            {
              analogWrite(WIO_BUZZER, 128);
              digitalWrite(beeper, LOW);
              timer[2] = 2000;
            }
            if (statoLCD_BACKLIGHT == LOW)
            {
              digitalWrite(LCD_BACKLIGHT, HIGH);
            }
          }
          if (isGateOk == 0 && isBadgeOk == 1)
          {
            DPRINTLN("Gate not authorized");
            tft.print("Gate non autoriz.");
            tft.setCursor(xcur, ycur + 25);
            tft.print(rfidCognome);
            tft.setCursor(xcur, ycur + 50);
            tft.print(rfidNome);
            tft.setCursor(xcur, ycur + 100);
            tft.print(rfidUIDdec);
            statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
            writeTransit(rfidValue, entrata_uscita, 2, rfidIdBadge);
            if (config.BeepOnOff != 0)
            {
              analogWrite(WIO_BUZZER, 128);
              digitalWrite(beeper, LOW);
              timer[2] = 2000;
            }
            if (statoLCD_BACKLIGHT == LOW)
            {
              digitalWrite(LCD_BACKLIGHT, HIGH);
            }
          }
          else if (isBadgeOk == 1 && isProfiloOk == 0)
          {
            DPRINTLN("Out of time slot badge");
            tft.print("Badge Fuori Fascia");
            tft.setCursor(xcur, ycur + 25);
            tft.print(rfidCognome);
            tft.setCursor(xcur, ycur + 50);
            tft.print(rfidNome);
            tft.setCursor(xcur, ycur + 100);
            tft.print(rfidUIDdec);
            writeTransit(rfidValue, entrata_uscita, 3, rfidIdBadge);
          }
          if (config.BeepOnOff != 0)
          {
            analogWrite(WIO_BUZZER, 128);
            digitalWrite(beeper, LOW);
            timer[2] = 2000;
          }
          statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
          if (statoLCD_BACKLIGHT == LOW)
          {
            digitalWrite(LCD_BACKLIGHT, HIGH);
          }
          DateTime now = rtc.now();
          time_t utc = now.get_time_t();
          time_t local = myTZ.toLocal(utc, &tcr);
          resetData();
          tft.setCursor(xcur, ycur + 75);
          esponiora();
          if ((isBadgeOk != 0) && (isProfiloOk != 0) && (isGateOk != 0))
          {
            tft.print(" - ");
            tft.print(direction);
            if (config.BeepOnOff != 0)
            {
              analogWrite(WIO_BUZZER, 128);
              digitalWrite(beeper, LOW);
              timer[2] = 2000;
            }
          }
          BufBadgePresent = 1;
          timer[1] = prog[1]; // reset timer updatetransit
          timer[5] = prog[5]; // reset timer KeepAlive
          timer[6] = prog[6]; // reset timer updateLog
          timer[0] = prog[0]; // reset timer Display
          lastDrawLogo = 0;
          tft.setFreeFont(FF5);
          statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
          if (statoLCD_BACKLIGHT == LOW)
          {
            digitalWrite(LCD_BACKLIGHT, HIGH);
          }
        }
        prev_badge = rfidValue;
        badgePrevius.restart();
      }
    }
    else
    {
      tft.fillRect(xBoxWt, yBoxWt, WBoxWt, HBoxWt, ILI9341_RED);
      tft.setFreeFont(FF5);
      DPRINTLN("Bad badge reading");
      tft.setTextColor(ILI9341_WHITE);
      byte xcur = 25;
      byte ycur = 80;
      tft.drawRect(xcur - 15, ycur - 30, 300, 150, ILI9341_WHITE);
      tft.setCursor(xcur, ycur + 40);
      tft.print("Lettura Badge non valida");
      tft.setCursor(xcur, ycur + 60);
      tft.print("Rileggere Prego");
      if (config.BeepOnOff != 0)
      {
        analogWrite(WIO_BUZZER, 128);
        digitalWrite(beeper, LOW);
        timer[2] = 2000;
      }
      statoLCD_BACKLIGHT = digitalRead(LCD_BACKLIGHT);
      if (statoLCD_BACKLIGHT == LOW)
      {
        digitalWrite(LCD_BACKLIGHT, HIGH);
      }
      if (timer[0] == 0)
      {
        timer[0] = prog[0];
        lastDrawLogo = 0;
      }
      tft.setFreeFont(FF5);
    }
  }
}
