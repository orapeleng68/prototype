#include <Wire.h>
#include <U8g2lib.h> // Library for 128x64 LCD Display
#include <TinyGPS++.h>
#include <Keypad.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <math.h>

// LCD setup for 128x64 resolution
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 22, /* data=*/ 21, /* reset=*/ U8X8_PIN_NONE);

// Keypad setup
const byte ROW_NUM = 4;
const byte COL_NUM = 4;
char keys[ROW_NUM][COL_NUM] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pin_rows[ROW_NUM] = {25, 26, 27, 14};
byte pin_column[COL_NUM] = {12, 13, 23, 22};
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COL_NUM);

// GPS setup
TinyGPSPlus gps;
HardwareSerial mySerial(1);

const char* ssid = "YourSSID";
const char* password = "YourPassword";

int screenState = 0; // 0 = GPS, 1 = Weather, 2 = Credits, 3 = Compass

// === Welcome Screen 1 ===
void WelcomeWindow() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(30, 30, "LOBSEC@2025");
    u8g2.drawStr(6, 40, "BOTSAT1 RECEIVER");
    u8g2.setFont(u8g2_font_ncenR08_tr);
    u8g2.drawStr(20, 50, "www.lobsec.ac.bw");
    u8g2.drawRFrame(0, 0, 128, 64, 4);
  } while (u8g2.nextPage());
  delay(3000);
}

// === Welcome Screen 2 (Menu) ===
void menuWindow() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenR08_tr);
    u8g2.drawStr(4, 10, "Select Option:");
    u8g2.drawStr(4, 26, "[A] GPS Data");
    u8g2.drawStr(4, 36, "[B] Weather Info");
    u8g2.drawStr(4, 48, "[C] Credits");
    u8g2.drawStr(4, 60, "[D] Compass");
  } while (u8g2.nextPage());
  delay(100);
}

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 16, 17);
  u8g2.begin();

  // Show welcome screens before proceeding
  WelcomeWindow();
  menuWindow();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    if (key == 'A') screenState = 0;
    else if (key == 'B') screenState = 1;
    else if (key == 'C') screenState = 2;
    else if (key == 'D') screenState = 3;
  }

  if (screenState == 0) displayGPSData();
  if (screenState == 1) displayWeatherData();
  if (screenState == 2) displayCredits();
  if (screenState == 3) displayCompass();

  delay(500);
}

void displayGPSData() {
  while (mySerial.available() > 0) gps.encode(mySerial.read());

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(0, 10);
  u8g2.print("GPS Data");

  if (gps.location.isValid()) {
    u8g2.setCursor(0, 20);
    u8g2.print("Lat: ");
    u8g2.print(gps.location.lat(), 6);

    u8g2.setCursor(0, 30);
    u8g2.print("Lon: ");
    u8g2.print(gps.location.lng(), 6);

    u8g2.setCursor(0, 40);
    u8g2.print("Alt: ");
    u8g2.print(gps.altitude.meters(), 1);
    u8g2.print("m");

    u8g2.setCursor(0, 50);
    u8g2.print("Speed: ");
    u8g2.print(gps.speed.kmph(), 1);
    u8g2.print("km/h");

    u8g2.setCursor(80, 50);
    u8g2.print("Sat: ");
    u8g2.print(gps.satellites.value());
  } else {
    u8g2.setCursor(0, 20);
    u8g2.print("Waiting for GPS...");
  }

  u8g2.sendBuffer();
}

void displayWeatherData() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(0, 10);
  u8g2.print("Weather Info");

  String weather = getWeatherData();
  u8g2.setCursor(0, 20);
  u8g2.print(weather);

  u8g2.sendBuffer();
}

String getWeatherData() {
  HTTPClient http;
  String weatherInfo = "";

  String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?q=YourCity&appid=YOUR_API_KEY&units=metric";
  http.begin(weatherUrl);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String payload = http.getString();
    int tempIndex = payload.indexOf("\"temp\":") + 7;
    int tempEnd = payload.indexOf(",", tempIndex);
    String temperature = payload.substring(tempIndex, tempEnd);
    weatherInfo = "Temp: " + temperature + "°C";
  }
  
  http.end();
  return weatherInfo;
}

void displayCredits() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(10, 10);
  u8g2.print("Project by:");

  u8g2.setCursor(10, 20);
  u8g2.print("Orapeleng, Nicole,");

  u8g2.setCursor(10, 30);
  u8g2.print("Pako, Tshiamo,");

  u8g2.setCursor(10, 40);
  u8g2.print("Messiah, Neo,");

  u8g2.setCursor(10, 50);
  u8g2.print("Thousand");

  u8g2.sendBuffer();
}

void displayCompass() {
  while (mySerial.available() > 0) gps.encode(mySerial.read());

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(40, 10);
  u8g2.print("Compass");

  float heading = gps.course.deg();
  if (!gps.course.isValid()) {
    heading = 0;
  }

  drawCompassNeedle(heading);
  u8g2.sendBuffer();
}

void drawCompassNeedle(float heading) {
  int centerX = 64;
  int centerY = 40;
  int radius = 20;

  float angle = heading * M_PI / 180.0;
  int needleX = centerX + radius * sin(angle);
  int needleY = centerY - radius * cos(angle);

  u8g2.drawCircle(centerX, centerY, radius);
  u8g2.drawLine(centerX, centerY, needleX, needleY);

  u8g2.setCursor(40, 60);
  u8g2.print("Head: ");
  u8g2.print(heading, 1);
  u8g2.print("°");
}
 