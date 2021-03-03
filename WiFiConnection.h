#ifndef WiFiConnection_h
#define WiFiConnection_h

#include <Arduino.h>
#include <AtWiFi.h>
#include "TFT_eSPI.h"
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "Debug.h"
//#include <MySQL_Connection.h>
//#include <MySQL_Cursor.h>

extern TFT_eSPI tft;
extern char *ssidload;
extern char *passload; 
extern char *local_IPload;
extern char *gatewayload;
extern char *subnetload;
extern char *primaryDNSload;
extern char *secondaryDNSload;

IPAddress local_IP;
IPAddress gateway;
IPAddress subnet;
IPAddress primaryDNS;
IPAddress secondaryDNS;


void connWifi()
{
  local_IP.fromString(String(local_IPload));
  gateway.fromString(String(gatewayload));
  subnet.fromString(String(subnetload));
  primaryDNS.fromString(String(primaryDNSload));
  secondaryDNS.fromString(String(secondaryDNSload));

//  DPRINT("DNSP = ");
//  DPRINTLN(primaryDNS);
//  DPRINT("DNSs = ");
//  DPRINTLN(secondaryDNS);
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    DPRINTLN("STA Failed to configure");
    tft.setFreeFont(FSSB9);
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_BLACK);
    tft.drawString("WiFi Failed to configure", 50, 60);
  }

  DPRINT("Connecting WiFi: ");
  tft.setFreeFont(FSSB9);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.drawString("Connecting to WiFi", 50, 60);
  tft.drawString(String(ssidload), 50, 80);
  DPRINTLN(ssidload);

  WiFi.begin(ssidload, passload);
  tft.setCursor(50, 110);
  int h = 0;
  for (h = 0; h < 40; h++)
  {
    delay(500);
    tft.setTextColor(TFT_BLACK);

    tft.print(".");
    DPRINT(".");
    if (WiFi.status() == WL_CONNECTED)
    {
      tft.fillScreen(TFT_WHITE);
      tft.drawString("Connected", 50, 60);
      delay(2000);
      DPRINTLN("");
      DPRINTLN("WiFi connected!");
      DPRINT("IP address: ");
      DPRINTLN(WiFi.localIP());
      DPRINT("Subnet Mask: ");
      DPRINTLN(WiFi.subnetMask());
      DPRINT("Gateway IP: ");
      DPRINTLN(WiFi.gatewayIP());
      DPRINT("DNS: ");
      DPRINTLN(WiFi.dnsIP());
      break;
    }
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    yield();
    tft.fillScreen(TFT_WHITE);
    tft.drawString("Wifi connection failed", 30, 60); /* code */
    DPRINTLN("");
    DPRINTLN("Wifi connection failed");
    delay(2000);
  }
}

void connWifiSilent()
{
  DPRINT("Connecting WiFi ");
  DPRINTLN(ssidload);
  WiFi.begin(ssidload, passload);
  int h = 0;
  for (h = 0; h < 15; h++)
  {
    delay(500);
    DPRINT(".");
    if (WiFi.status() == WL_CONNECTED)
    {
      DPRINTLN("");
      DPRINTLN("WiFi connected!");
      // DPRINT("IP address: ");
      // DPRINTLN(WiFi.localIP());
      // DPRINT("Subnet Mask: ");
      // DPRINTLN(WiFi.subnetMask());
      // DPRINT("Gateway IP: ");
      // DPRINTLN(WiFi.gatewayIP());
      // DPRINT("DNS: ");
      // DPRINTLN(WiFi.dnsIP());
      break;
    }
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    yield();
    DPRINTLN("");
    DPRINTLN("Wifi connection failed");
  }
}

//*************************************************
// converts the dBm to a range between 0 and 100%
//*************************************************

int8_t getWifiQuality()
{
  int32_t dbmM = WiFi.RSSI();
  if (dbmM <= -100)
  {
    return 0;
  }
  else if (dbmM >= -50)
  {
    return 100;
  }
  else
  {
    return 2 * (dbmM + 100);
  }
}
void drawWifiQualityBoot()
{
  int8_t quality = getWifiQuality();
  tft.fillRect(214, 30, 106, 20, ILI9341_BLACK);
  tft.setFreeFont(FM9);
  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(70, 105);
  tft.print("WiFi BOOT");
  tft.setCursor(20, 135);
  tft.print("WAITING PLEASE");
  delay(1000);
  DPRINTLN("WiFi Boot");
}
void drawWifiQualityNotConfigured()
{
  int8_t quality = getWifiQuality();
  tft.fillRect(0, 40, 320, 180, ILI9341_BLACK);
  tft.setFreeFont(FM9);
  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(35, 105);
  tft.print("WiFi NOT CONFIGURATION");
  tft.setCursor(15, 135);
  tft.print("WAITING FOR CONFIGURATION");
  DPRINTLN("WiFi Not Configurate");
}
void drawWifiQualityAPMODE()
{
  int8_t quality = getWifiQuality();
  tft.fillRect(0, 40, 320, 180, ILI9341_BLACK);
  tft.setFreeFont(FM9);
  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(70, 105);
  tft.print("DEVICE IN AP MODE");
  tft.setCursor(48, 135);
  tft.print("AWAITING CONNECTION");
  tft.setCursor(62, 165);
  tft.print("FOR CONFIGURATION");
  DPRINTLN("Wifi in AP MODE");
}
void drawWifiQualityCONNETTING()
{
  int8_t quality = getWifiQuality();
  tft.fillRect(214, 30, 106, 24, ILI9341_BLACK);
  tft.setFreeFont(FM9);
  tft.setTextColor(ILI9341_CYAN);
  tft.drawString("WAIT AP", 220, 40, GFXFF);
  DPRINTLN("WIFI WAIT CONNECTING");
}
void drawWifiQualityONLINE()
{
  int8_t quality = getWifiQuality();
  tft.fillRect(214, 30, 106, 24, ILI9341_BLACK);
  tft.setFreeFont(FM9);
  tft.setTextColor(ILI9341_CYAN);
  tft.drawString("WIFI:" + String(quality) + "%", 220, 40, GFXFF);
}

void drawWifiQualityND()
{
  int8_t quality = getWifiQuality();
  tft.fillRect(214, 32, 106, 24, ILI9341_BLACK);
  tft.setFreeFont(FM9);
  tft.setFreeFont(FM9);
  tft.setTextColor(ILI9341_CYAN);
  tft.drawString("WIFI N/D", 220, 40, GFXFF);
  DPRINTLN("WIFI N/D");
}
#endif
