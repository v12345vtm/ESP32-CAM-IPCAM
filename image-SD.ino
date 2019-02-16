/*
   ESP32-CAM board uses  the SD card to the following pins:
  https://i.imgur.com/Pxa26nc.png
  edit by v12345vtm : https://www.youtube.com/user/v12345vtm
https://www.youtube.com/watch?v=PhrSWB4qWXg


   SD Card | ESP32    |esp32-cam
      D2       -          -
      D3       SS         gpio13
      CMD      MOSI       gpio15
      VSS      GND        gnd
      VDD      3.3V       3.3v
      CLK      SCK         gpio14
      VSS      GND          gnd
      D0       MISO         gpio2
      D1       -        -




  WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
             or another board which has PSRAM enabled



  the html -code is open and is easy to update or modify  on the other tab page = app_httpd.cpp

*/


#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h" //disable brownour problems
#include "soc/rtc_cntl_reg.h"  //disable brownour problems

#include "FS.h" // used by SDcard
#include "SD.h" // used by SDcard
#include "SPI.h" // used by SDcard
#include <WiFi.h> //used for internet time

#define CAMERA_MODEL_AI_THINKER

const char* ssid = "WiFi ";
const char* password = "pass";

const char* ntpServer = "pool.ntp.org"; //internet time server
const long  gmtOffset_sec = 3600;//internet time server
const int   daylightOffset_sec = 3600;//internet time server

void printLocalTime()
{ //internet time server
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}


#if defined(CAMERA_MODEL_AI_THINKER) //zie esp32-cam schema v1.6.pdf
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define init_xclk_freq_hz      20000000
#define init_pixel_format        PIXFORMAT_JPEG
#define init_frame_size          FRAMESIZE_UXGA
#define init_jpeg_quality      20
#define init_fb_count           2
#else
#error "Camera model not selected"
#endif

bool gelukt  = false; //for debug in mainloop 1image
bool webserverenabled = false ; //debug webserver on or off
bool testSDkaart = true; //sd card tests

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char * path) {
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file) {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++) {
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}
void startCameraServer();

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  SPI.begin(14, 2, 15, 13); //used by SDcard
  // yield();
  Serial.println("SPI(begin) success");

  camera_config_t config;  // see esp_camera.h constructor
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = init_pixel_format;
  config.frame_size = init_frame_size;
  config.jpeg_quality = init_jpeg_quality;
  config.fb_count = init_fb_count;
  //if(psramFound()){       Serial.println("psram included on ESP32-CAM"); }

  // camera init to esp_camera.h
  esp_err_t err = esp_camera_init(&config); //see : esp_camera.h




  if (err != ESP_OK) {
    Serial.printf("bad contact in connector or Camera init failed with error 0x%x", err);
    return;
  }

  //weg mss weg ??drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_UXGA);//




  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");



  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  if (webserverenabled){

  startCameraServer();
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect , de stream zit op een andere poortkanaal 9601 ");
  Serial.print("stream Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println(":9601/stream ");
  Serial.print("image Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("/capture ");

  }
  else
  {
     Serial.println("webserverenabled : uitgezet via bool webserverenabled ");
    }
  



  /////////////SD card init
  //SD.begin(13);

  if (testSDkaart ) {
     Serial.println("SD CARD enabled");
    if (!SD.begin(13)) {
      Serial.println("Card Mount Failed if no begin13");
      return;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
      Serial.println("No SD card attached");
      return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
      Serial.println("MMC");
    } else if (cardType == CARD_SD) {
      Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    //    listDir(SD, "/", 0);
    //    createDir(SD, "/mydir");
    //    listDir(SD, "/", 0);
    //    removeDir(SD, "/mydir");
    //    listDir(SD, "/", 2);
    //    writeFile(SD, "/hello.txt", "Hello ");
    //    appendFile(SD, "/hello.txt", "ESP32-CAM-SD-v12345vtm!\n");
    //    readFile(SD, "/hello.txt");
    //    deleteFile(SD, "/foo.txt");
    //    renameFile(SD, "/hello.txt", "/foo.txt");
    //    readFile(SD, "/foo.txt");
    //    testFileIO(SD, "/test.txt");
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

    // SPI.end() ;


  }

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  printLocalTime();



  //get  1 image without webserverpages
  if (!gelukt) {

    camera_fb_t * foto = NULL;
    foto = esp_camera_fb_get(); //methode in esp_camera.h to get image from am
    if (!foto  ) {
      Serial.println("mainloop manual Camera capture niet gelukt");
    }
    else
    {
      Serial.println("mainloop :manual Camera gelukt");
      gelukt = true;

    }


  }



  /////////////////////////////////////////////


}
