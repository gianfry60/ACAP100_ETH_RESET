#ifndef FSSelect_h
#define FSSelect_h

#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"
//#include "SFUD/Seeed_SFUD.h"
#include "Debug.h"
#include "TFT_eSPI.h"

uint8_t cardType = 0;
uint8_t cardSize = 0;
uint8_t flashType = 0;
uint8_t flashSize = 0;

extern TFT_eSPI tft;

void SDStart()
{
#define DEV SD
SPIFLASH.end();
#undef USESPIFLASH

    while (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI, 4000000UL))
    {
        DPRINTLN("Card Mount Failed");
        tft.fillScreen(TFT_WHITE);
        tft.setTextColor(TFT_BLACK);
        tft.drawString("Card Mount Failed", 50, 60);
        delay(1500);
        tft.fillScreen(TFT_BLACK);
        NVIC_SystemReset();
        return;
    }
    cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        DPRINTLN("No SD card attached");
        tft.fillScreen(TFT_WHITE);
        tft.setTextColor(TFT_BLACK);
        tft.drawString("No SD card attached", 40, 60);
        delay(1500);
        tft.fillScreen(TFT_BLACK);
        NVIC_SystemReset();
        return;
    }
    cardSize = SD.cardSize() / (1024 * 1024);
    DPRINT("\nSD Card Size: ");
    DPRINT((uint32_t)cardSize);
    DPRINTLN("MB");
}

void FLASHStart()
{
#undef DEV SD
SD.end();

#define USESPIFLASH
#define DEV SPIFLASH
    while (!SPIFLASH.begin(104000000UL))
    {
        DPRINTLN("Flash Mount Failed");
        tft.fillScreen(TFT_WHITE);
        tft.setTextColor(TFT_BLACK);
        tft.drawString("Flash Mount Failed", 40, 60);
        delay(1500);
        tft.fillScreen(TFT_BLACK);
        NVIC_SystemReset();
        return;
    }
    flashType = SPIFLASH.flashType();
    if (flashType == FLASH_NONE)
    {
        DPRINTLN("No Flash attached");
        tft.fillScreen(TFT_WHITE);
        tft.setTextColor(TFT_BLACK);
        tft.drawString("No Flash attached", 50, 60);
        delay(1500);
        tft.fillScreen(TFT_BLACK);
        NVIC_SystemReset();
        return;
    }
    flashSize = SPIFLASH.flashSize() / (1024 * 1024);
    DPRINT("\nFlash Size: ");
    DPRINT((uint32_t)flashSize);
    DPRINTLN("MB");
}
#endif
