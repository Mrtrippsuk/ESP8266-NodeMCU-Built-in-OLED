#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Eye positions (moved UP again)
int leftEyeX = 40;
int rightEyeX = 88;
int eyeY = 30;

int eyeRadius = 14;
int pupilRadius = 5;

// Smooth movement
float pupilX = 0, pupilY = 0;
float targetX = 0, targetY = 0;

// Timing
unsigned long lastMove = 0;
unsigned long lastBlink = 0;
bool blinking = false;

// Emotion
String emotion = "normal";

// Scroll text
String scrollText = "ESP FACE ONLINE :)   ";
int scrollX = 128;

// ------------------ DRAW MOUTH ------------------
void drawMouth() {
  int y = 52;

  if (emotion == "happy") {
    display.drawLine(54, y, 74, y + 4, SSD1306_WHITE);
  } 
  else if (emotion == "angry") {
    display.drawLine(54, y + 4, 74, y, SSD1306_WHITE);
  } 
  else if (emotion == "tired") {
    display.drawLine(54, y, 74, y, SSD1306_WHITE);
  } 
  else if (emotion == "surprised") {
    display.drawCircle(64, y, 3, SSD1306_WHITE);
  } 
  else {
    display.drawLine(56, y + 2, 72, y + 2, SSD1306_WHITE);
  }
}

// ------------------ EYEBROWS ------------------
void drawBrows() {
  if (emotion == "angry") {
    display.drawLine(leftEyeX - 10, eyeY - 14, leftEyeX + 10, eyeY - 6, SSD1306_WHITE);
    display.drawLine(rightEyeX - 10, eyeY - 6, rightEyeX + 10, eyeY - 14, SSD1306_WHITE);
  }
  else if (emotion == "happy") {
    display.drawLine(leftEyeX - 10, eyeY - 12, leftEyeX + 10, eyeY - 14, SSD1306_WHITE);
    display.drawLine(rightEyeX - 10, eyeY - 14, rightEyeX + 10, eyeY - 12, SSD1306_WHITE);
  }
  else if (emotion == "tired") {
    display.drawLine(leftEyeX - 10, eyeY - 10, leftEyeX + 10, eyeY - 10, SSD1306_WHITE);
    display.drawLine(rightEyeX - 10, eyeY - 10, rightEyeX + 10, eyeY - 10, SSD1306_WHITE);
  }
}

// ------------------ SCROLL BAR ------------------
void drawScrollText() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(scrollX, 0);
  display.print(scrollText);

  scrollX -= 2;
  if (scrollX < -scrollText.length() * 6) {
    scrollX = 128;
  }
}

// ------------------ ANIMATION ------------------
void updateAnimation() {
  unsigned long now = millis();

  // Random look
  if (now - lastMove > random(2000, 4000)) {
    targetX = random(-5, 6);
    targetY = random(-3, 4);
    lastMove = now;
  }

  // Smooth easing
  pupilX += (targetX - pupilX) * 0.08;
  pupilY += (targetY - pupilY) * 0.08;

  // Blink
  if (now - lastBlink > random(3000, 7000)) {
    blinking = true;
    lastBlink = now;
  }

  if (blinking && now - lastBlink > 120) {
    blinking = false;
  }
}

// ------------------ DRAW ------------------
void drawFace() {
  display.clearDisplay();

  // Top scroll (yellow area)
  drawScrollText();

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

// ------------------ SERIAL ------------------
void handleSerial() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "happy") emotion = "happy";
    if (cmd == "angry") emotion = "angry";
    if (cmd == "tired") emotion = "tired";
    if (cmd == "surprised") emotion = "surprised";
    if (cmd == "normal") emotion = "normal";

    if (cmd == "left") targetX = -5;
    if (cmd == "right") targetX = 5;
    if (cmd == "center") targetX = 0;

    if (cmd == "blink") {
      blinking = true;
      lastBlink = millis();
    }

    // Change scroll text
    if (cmd.startsWith("text:")) {
      scrollText = cmd.substring(5) + "   ";
      scrollX = 128;
    }
  }
}

void setup() {
  Wire.begin(12, 14);
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }

  randomSeed(analogRead(A0));
}

void loop() {
  handleSerial();
  updateAnimation();
  drawFace();
  delay(20);
}