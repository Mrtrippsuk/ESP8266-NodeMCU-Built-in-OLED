//made for esp8266 NodeMCU with bult in oled
//random face emotions but can be controlled by serial
//shows text date and time on yellow section of OLED
// Written by HamHacks
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <time.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// WIFI
const char* ssid = "SSID_HERE";//change to your ssid
const char* password = "HAMHACKS";//change to your password

// Eyes (adjusted down slightly)
int leftEyeX = 40;
int rightEyeX = 88;
int eyeY = 34;

int eyeRadius = 14;
int pupilRadius = 5;

// Movement
float pupilX = 0, pupilY = 0;
float targetX = 0, targetY = 0;

// Timing
unsigned long lastMove = 0;
unsigned long lastBlink = 0;
unsigned long lastMoodChange = 0;
bool blinking = false;

// Emotion
String emotion = "normal";

// Scroll text
String scrollText = "";
int scrollX = 128;

// ---------------- WIFI TIME ----------------
void setupTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

// Format time string
String getTimeString() {
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);

  char buf[32];
  sprintf(buf, "%02d/%02d %02d:%02d ",
          t->tm_mday, t->tm_mon + 1,
          t->tm_hour, t->tm_min);

  return String(buf);
}

// ---------------- PERSONALITY ----------------
void updatePersonality() {
  unsigned long now = millis();

  // Random mood change every ~10–20 sec
  if (now - lastMoodChange > random(10000, 20000)) {
    int r = random(0, 5);
    if (r == 0) emotion = "happy";
    if (r == 1) emotion = "angry";
    if (r == 2) emotion = "tired";
    if (r == 3) emotion = "surprised";
    if (r == 4) emotion = "normal";

    lastMoodChange = now;
  }
}

// ---------------- MOUTH ----------------
void drawMouth() {
  int y = 54;

  if (emotion == "happy")
    display.drawLine(54, y, 74, y + 4, SSD1306_WHITE);

  else if (emotion == "angry")
    display.drawLine(54, y + 4, 74, y, SSD1306_WHITE);

  else if (emotion == "tired")
    display.drawLine(54, y, 74, y, SSD1306_WHITE);

  else if (emotion == "surprised")
    display.drawCircle(64, y, 3, SSD1306_WHITE);

  else
    display.drawLine(56, y + 2, 72, y + 2, SSD1306_WHITE);
}

// ---------------- EYEBROWS ----------------
void drawBrows() {
  if (emotion == "angry") {
    display.drawLine(leftEyeX - 10, eyeY - 16, leftEyeX + 10, eyeY - 8, SSD1306_WHITE);
    display.drawLine(rightEyeX - 10, eyeY - 8, rightEyeX + 10, eyeY - 16, SSD1306_WHITE);
  }
  else if (emotion == "happy") {
    display.drawLine(leftEyeX - 10, eyeY - 14, leftEyeX + 10, eyeY - 16, SSD1306_WHITE);
    display.drawLine(rightEyeX - 10, eyeY - 16, rightEyeX + 10, eyeY - 14, SSD1306_WHITE);
  }
  else if (emotion == "tired") {
    display.drawLine(leftEyeX - 10, eyeY - 12, leftEyeX + 10, eyeY - 12, SSD1306_WHITE);
    display.drawLine(rightEyeX - 10, eyeY - 12, rightEyeX + 10, eyeY - 12, SSD1306_WHITE);
  }
}

// ---------------- SCROLL ----------------
void drawScroll() {
  scrollText = getTimeString() + "HamHacks!";

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(scrollX, 0);
  display.print(scrollText);

  scrollX -= 2;
  if (scrollX < -scrollText.length() * 6)
    scrollX = 128;
}

// ---------------- ANIMATION ----------------
void updateAnimation() {
  unsigned long now = millis();

  if (now - lastMove > random(2000, 4000)) {
    targetX = random(-5, 6);
    targetY = random(-3, 4);
    lastMove = now;
  }

  pupilX += (targetX - pupilX) * 0.08;
  pupilY += (targetY - pupilY) * 0.08;

  if (now - lastBlink > random(3000, 7000)) {
    blinking = true;
    lastBlink = now;
  }

  if (blinking && now - lastBlink > 120)
    blinking = false;
}

// ---------------- DRAW ----------------
void drawFace() {
  display.clearDisplay();

  drawScroll();

  if (!blinking) {
    display.fillCircle(leftEyeX, eyeY, eyeRadius, SSD1306_WHITE);
    display.fillCircle(rightEyeX, eyeY, eyeRadius, SSD1306_WHITE);

    display.fillCircle(leftEyeX + (int)pupilX, eyeY + (int)pupilY, pupilRadius, SSD1306_BLACK);
    display.fillCircle(rightEyeX + (int)pupilX, eyeY + (int)pupilY, pupilRadius, SSD1306_BLACK);
  } else {
    display.fillRect(leftEyeX - eyeRadius, eyeY, eyeRadius * 2, 3, SSD1306_WHITE);
    display.fillRect(rightEyeX - eyeRadius, eyeY, eyeRadius * 2, 3, SSD1306_WHITE);
  }

  drawBrows();
  drawMouth();

  display.display();
}

// ---------------- SERIAL ----------------
void handleSerial() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    emotion = cmd; // quick override
  }
}

// ---------------- SETUP ----------------
void setup() {
  Wire.begin(12, 14);
  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  setupTime();
  randomSeed(analogRead(A0));
}

// ---------------- LOOP ----------------
void loop() {
  handleSerial();
  updatePersonality();
  updateAnimation();
  drawFace();
  delay(20);
}