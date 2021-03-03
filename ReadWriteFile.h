
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"
#include "SFUD/Seeed_SFUD.h"
#include "DEBUG.h"
#include "DefineDir.h"

//#define NDEBUGRWF // Comment this line to enable serial debugging

// #ifndef NDEBUG
// #define DSTART(speed) Serial.begin(speed)
// #define DPRINT(...) Serial.print(__VA_ARGS__)
// #define DPRINTLN(...) Serial.println(__VA_ARGS__)
// #define DSWRITE(...) Serial.write(__VA_ARGS__)

// #else
// #define DSTART(...) \
//     do              \
//     {               \
//     } while (0)
// #define DPRINT(...) \
//     do              \
//     {               \
//     } while (0)
// #define DPRINTLN(...) \
//     do                \
//     {                 \
//     } while (0)
// #define DSWRITE(...) \
//     do               \
//     {                \
//     } while (0)
// #undef DSERIAL
// #endif

// #ifdef _SAMD21_
// #define SDCARD_SS_PIN 1
// #define SDCARD_SPI SPI
// #endif

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    DPRINT("Listing directory: ");
    DPRINTLN(dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        DPRINTLN("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        DPRINTLN("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            DPRINT("  DIR : ");
            DPRINTLN(file.name());
            if (levels)
            {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else
        {
            DPRINT("  FILE: ");
            DPRINT(file.name());
            DPRINT("  SIZE: ");
            DPRINTLN(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char *path)
{
    DPRINT("Creating Dir: ");
    DPRINTLN(path);
    if (fs.mkdir(path))
    {
        DPRINTLN("Dir created");
    }
    else
    {
        DPRINTLN("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char *path)
{
    DPRINT("Removing Dir: ");
    DPRINTLN(path);
    if (fs.rmdir(path))
    {
        DPRINTLN("Dir removed");
    }
    else
    {
        DPRINTLN("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char *path)
{
    DPRINT("Reading Dir: ");
    DPRINTLN(path);
    File file = fs.open(path);
    if (!file)
    {
        DPRINTLN("Failed to open file for reading");
        return;
    }

    DPRINT("Read from file: ");
    DPRINTLN("");
    while (file.available())
    {
        Serial.write(file.read());
    }
    DPRINTLN(" ");
    file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
    DPRINT("Writing file: ");
    DPRINTLN(path);
    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        DPRINTLN("Failed to open file for writing");
        return;
    }

    if (file.print(message))
    {
        DPRINTLN("File written");
    }
    else
    {
        DPRINTLN("Write failed");
    }

    file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
    //DPRINT("Appending to file: ");
    //DPRINTLN(path);

    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        DPRINTLN("Failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        //DPRINTLN("Message appended");
    }
    else
    {
        DPRINTLN("Append failed");
    }
    file.close();
}

bool renameFile(fs::FS &fs, const char *path1, const char *path2)
{
    DPRINT("Renaming file ");
    DPRINT(path1);
    DPRINT(" to ");
    DPRINTLN(path2);
    if (fs.rename(path1, path2))
    {
        DPRINTLN("File renamed");
        return true;
    }
    else
    {
        DPRINTLN("Rename failed");
        return false;
    }
}

bool deleteFile(fs::FS &fs, const char *path)
{
    File file = fs.open(path);
    delay(300);
    file.close();
    DPRINT("Deleting file: ");
    DPRINTLN(path);
    delay(300);
    if (fs.remove(path))
    {
        DPRINTLN("File deleted");
        return true;
    }
    else
    {
        DPRINTLN("Delete failed");
        return false;
    }
}

bool testFileIO(fs::FS &fs, const char *path)
{
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = micros();
    uint32_t end = start;
    if (file)
    {
        len = file.size();
        size_t flen = len;
        start = micros();
        while (len)
        {
            size_t toRead = len;
            if (toRead > 512)
            {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = micros() - start;
        DPRINT("Test File: ");
        DPRINT(path);
        DPRINT(" len= ");
        DPRINT(flen);
        DPRINT(" bytes read for ");
        DPRINT(end);
        DPRINTLN(" ns");
        file.close();
        return true;
    }
    else
    {
        DPRINTLN("Failed to open file for reading");
        return false;
    }
}

int FileLen(fs::FS &fs, const char *path)
{
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = micros();
    uint32_t end = start;
    if (file)
    {
        len = file.size();
        size_t flen = len;
        start = micros();
        while (len)
        {
            size_t toRead = len;
            if (toRead > 512)
            {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = micros() - start;
        DPRINT("Test File: ");
        DPRINT(path);
        DPRINT(" len= ");
        DPRINT(flen);
        DPRINT(" bytes read for ");
        DPRINT(end);
        DPRINTLN(" ns");
        file.close();
        return flen;
    }
    else
    {
        DPRINTLN("Failed to open file for reading");
        return false;
    }
}
