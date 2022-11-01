#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>


#define LED PC13 // PA1
#define BATLEVEL PB1
#define TX PA2
#define RX PA3
#define CHARGE PB4
#define STANDBY PB3
#define ENCODERA PA10
#define ENCODERB PA2 //PA9
#define BUZZER PA8
#define ENCODERSWITCH PB15

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SELECTION_PAGE -1
#define ALARM_PAGE 0
#define VIBRATION_PAGE 1
#define SETTINGS_PAGE 2


Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RTC_DS1307 rtc;




// Display Helpers
const unsigned int arrowLocations[4] = {16, 56, 96};
String timeString = "";
int currentLowerPage = 0;
int currentPage = -1;

// Wake Data
int wakeHour = 0;
int wakeMinute = 0;
int vibrationPattern = 1;

// User Input
unsigned long lastUserInput = 0;
bool pressed = false;
bool longPressRegistered = false;
bool activeUser = false;

// Settings
bool soundOn = true;
bool alwaysOn = true;
bool dimmed = false;


// Encoder
unsigned long switchLastPressed = 0;
unsigned long lastEncoderTurn = 0;
unsigned long lastEncoderCallA = 0;
unsigned long lastEncoderCallB = 0;
volatile int encoderLocation = 0;
bool allowEncoderOverflow = true;
int minLimitEncoder = 0;
int maxLimitEncoder = 2;



const unsigned char settingsBitmap [] PROGMEM = {
    0x00, 0x03, 0xC0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x07, 0xE0, 0x00,
    0x03, 0x87, 0xE1, 0xC0, 0x07, 0xFF, 0xFF, 0xE0, 0x0F, 0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0xFF, 0xFF, 0xF0, 0x07, 0xF8, 0x1F, 0xE0, 0x07, 0xE0, 0x07, 0xE0, 0x07, 0xC0, 0x03, 0xE0,
    0x07, 0xC0, 0x03, 0xE0, 0x7F, 0x80, 0x01, 0xFE, 0xFF, 0x80, 0x01, 0xFF, 0xFF, 0x80, 0x01, 0xFF,
    0xFF, 0x80, 0x01, 0xFF, 0xFF, 0x80, 0x01, 0xFF, 0x7F, 0x80, 0x01, 0xFE, 0x07, 0xC0, 0x03, 0xE0,
    0x07, 0xC0, 0x03, 0xE0, 0x07, 0xE0, 0x07, 0xE0, 0x07, 0xF8, 0x1F, 0xE0, 0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0xFF, 0xF0, 0x07, 0xFF, 0xFF, 0xE0, 0x03, 0x87, 0xE1, 0xC0,
    0x00, 0x07, 0xE0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x03, 0xC0, 0x00
};


const unsigned char alarmBitmap [] PROGMEM = {
    0x07, 0x00, 0x00, 0x60, 0x1F, 0xC0, 0x03, 0xF8, 0x3F, 0x80, 0x01, 0xFE, 0x7F, 0x00, 0x00, 0xFE,
    0xFE, 0x07, 0xE0, 0x7F, 0xFC, 0x3F, 0xFC, 0x3F, 0xF0, 0xFF, 0xFF, 0x0F, 0xE1, 0xFC, 0x1F, 0x87,
    0x43, 0xE0, 0x07, 0xC2, 0x07, 0x80, 0x01, 0xE0, 0x07, 0x01, 0x80, 0xF0, 0x0E, 0x01, 0x80, 0x70,
    0x1E, 0x01, 0xC0, 0x78, 0x1C, 0x01, 0xC0, 0x38, 0x1C, 0x01, 0xC0, 0x38, 0x38, 0x01, 0xC0, 0x1C,
    0x38, 0x01, 0xC0, 0x1C, 0x38, 0x01, 0xC0, 0x1C, 0x38, 0x01, 0xE0, 0x1C, 0x38, 0x01, 0xF0, 0x1C,
    0x38, 0x00, 0xF0, 0x1C, 0x1C, 0x00, 0x70, 0x38, 0x1C, 0x00, 0x00, 0x38, 0x1E, 0x00, 0x00, 0x78,
    0x0E, 0x00, 0x00, 0x70, 0x0F, 0x00, 0x00, 0xF0, 0x07, 0x80, 0x01, 0xE0, 0x07, 0xE0, 0x07, 0xF0,
    0x0F, 0xF8, 0x1F, 0xF8, 0x1E, 0xFF, 0xFF, 0x78, 0x3C, 0x3F, 0xFC, 0x3C, 0x18, 0x07, 0xE0, 0x18
};


const unsigned char vibrationBitmap [] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0xFF, 0xFF, 0x00, 0x01, 0xFF, 0xFF, 0x00, 0x01, 0x80, 0x03, 0x00, 0x21, 0x80, 0x03, 0x04,
    0x71, 0x80, 0x03, 0x0E, 0x39, 0x80, 0x03, 0x1C, 0x39, 0x80, 0x03, 0x1C, 0x71, 0x80, 0x03, 0x0E,
    0xE1, 0x80, 0x03, 0x07, 0x71, 0x80, 0x03, 0x0E, 0x39, 0x80, 0x03, 0x1C, 0x71, 0x80, 0x03, 0x0E,
    0xE1, 0x80, 0x03, 0x07, 0x71, 0x80, 0x03, 0x0E, 0x39, 0x80, 0x03, 0x1C, 0x31, 0x80, 0x03, 0x0C,
    0x61, 0x80, 0x03, 0x06, 0x01, 0x80, 0x03, 0x00, 0x01, 0x80, 0x03, 0x00, 0x01, 0x80, 0x03, 0x00,
    0x01, 0x80, 0x03, 0x00, 0x01, 0x80, 0x03, 0x00, 0x01, 0x80, 0x03, 0x00, 0x01, 0xFF, 0xFF, 0x00,
    0x01, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


const unsigned char arrowBitmap [] PROGMEM = {
    0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
    0x31, 0x8C, 0x39, 0x9C, 0x1D, 0xB8, 0x0F, 0xF0, 0x07, 0xE0, 0x01, 0x80,
};

const unsigned char wwwakeBitmap [] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00,
    0x01, 0xF0, 0xF0, 0x7B, 0xC1, 0xE1, 0xF7, 0x87, 0xC3, 0xE3, 0xF8, 0x1F, 0x0F, 0x83, 0xF8, 0x00,
    0x00, 0xF1, 0xF8, 0xFB, 0xC3, 0xE1, 0xE7, 0x87, 0xC3, 0xC7, 0xFE, 0x1F, 0x1F, 0x0F, 0xFC, 0x00,
    0x00, 0xF1, 0xF8, 0xF3, 0xC3, 0xE1, 0xE7, 0x8F, 0xC7, 0xCF, 0xFE, 0x1F, 0x3E, 0x0F, 0xFE, 0x00,
    0x00, 0xF1, 0xF9, 0xF1, 0xE7, 0xE3, 0xC7, 0x8F, 0xC7, 0x8F, 0x1F, 0x1F, 0x7C, 0x1F, 0x1F, 0x00,
    0x00, 0xF3, 0xF9, 0xE1, 0xE7, 0xF3, 0xC7, 0x8F, 0xCF, 0x8E, 0x0F, 0x1F, 0xF8, 0x3E, 0x0F, 0x00,
    0x00, 0xF3, 0xF9, 0xE1, 0xEF, 0xF7, 0x83, 0x9F, 0xCF, 0x00, 0x7F, 0x1F, 0xF8, 0x3E, 0x0F, 0x00,
    0x00, 0xF7, 0xBB, 0xC1, 0xEE, 0xF7, 0x83, 0xDD, 0xCF, 0x07, 0xFF, 0x1F, 0xF8, 0x3F, 0xFF, 0x00,
    0x00, 0x77, 0x3B, 0xC1, 0xEE, 0x7F, 0x03, 0xFD, 0xFE, 0x0F, 0xFF, 0x1F, 0xFC, 0x3F, 0xFF, 0x00,
    0x00, 0x7F, 0x3F, 0x81, 0xFE, 0x7F, 0x03, 0xF9, 0xFE, 0x1F, 0x8F, 0x1F, 0xFC, 0x3C, 0x00, 0x00,
    0x00, 0x7F, 0x3F, 0x80, 0xFC, 0x7E, 0x03, 0xF9, 0xFC, 0x1F, 0x0F, 0x1F, 0xBE, 0x3E, 0x0F, 0x00,
    0x00, 0x7E, 0x3F, 0x00, 0xFC, 0x7E, 0x03, 0xF0, 0xFC, 0x1F, 0x1F, 0x1F, 0x3E, 0x1F, 0x1F, 0x00,
    0x00, 0x7E, 0x3F, 0x00, 0xF8, 0x7C, 0x01, 0xF0, 0xF8, 0x1F, 0xFF, 0x1F, 0x1F, 0x0F, 0xFE, 0x00,
    0x00, 0x7C, 0x1E, 0x00, 0xF8, 0x7C, 0x01, 0xF0, 0xF8, 0x0F, 0xFF, 0x1F, 0x0F, 0x8F, 0xFE, 0x00,
    0x00, 0x7C, 0x1E, 0x00, 0xF8, 0x78, 0x01, 0xE0, 0xF0, 0x07, 0xEF, 0x1F, 0x0F, 0x83, 0xF8, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x44, 0x02, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x64, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x06, 0xE4, 0x46, 0x00, 0xC3, 0x40, 0x12, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x02, 0xAC, 0xA6, 0x9B, 0xE6, 0xC1, 0x13, 0x60, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x07, 0x02, 0xB8, 0x77, 0x1B, 0x24, 0x41, 0x12, 0x20, 0xE0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x99, 0xB7, 0x9B, 0x24, 0xC1, 0x92, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x19, 0xB6, 0x9B, 0x27, 0xC0, 0xF3, 0xE0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0xC0, 0x00, 0x20, 0x40, 0x02, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00
};



void playErrorSound() {
    if (soundOn) {
        tone(BUZZER, 4000);
        delay(500);
        tone(BUZZER, 1000);
        delay(700);
        noTone(BUZZER);
        digitalWrite(BUZZER, LOW);
    }
}

void playShortPress() {
    if (soundOn) {
        tone(BUZZER, 4000);
        delay(50);
        noTone(BUZZER);
        digitalWrite(BUZZER, LOW);
    }
}

void playLongPress() {
    if (soundOn) {
        tone(BUZZER, 4000);
        delay(250);
        noTone(BUZZER);
        digitalWrite(BUZZER, LOW);
    }
}

void playStartSound() {
    if (soundOn) {
        tone(BUZZER, 2500);
        delay(500);
        tone(BUZZER, 3000);
        delay(500);
        tone(BUZZER, 4000);
        delay(500);
        noTone(BUZZER);
        digitalWrite(BUZZER, LOW);
    }
}


int limitEncoder() {
    if (encoderLocation < minLimitEncoder) {
        if (allowEncoderOverflow) {
            encoderLocation = maxLimitEncoder;
        }
        else {
            encoderLocation = minLimitEncoder;
        }
    }
    if (encoderLocation > maxLimitEncoder) {
        if (allowEncoderOverflow) {
            encoderLocation = minLimitEncoder;
        }
        else {
            encoderLocation = maxLimitEncoder;
        }
    }
    return encoderLocation;
}


void displayClock(bool center) {
    DateTime now = rtc.now();
    timeString = "";
    if (now.hour() < 10) {
        timeString += "0";
    }
    timeString += now.hour();

    if (now.second() % 2 == 1) {
        timeString += " ";
    }
    else {
        timeString += ":";
    }


    if (now.minute() < 10) {
        timeString += "0";
    }
    timeString += now.minute();

    if (center) {
        oled.setTextSize(3);
        oled.setCursor((SCREEN_WIDTH - 90) / 2, (SCREEN_HEIGHT - 24) / 2);
    }
    else {
        oled.setTextSize(2);
        oled.setCursor((SCREEN_WIDTH - 60) / 2, 0);
    }
    oled.println(timeString);

    delay(3);
}


void displaySettings() {
    // Always On bool
    // Brightness Bool
    // Buzzer bool
    // set time
    oled.setCursor(13, 0);
    oled.println("Always On Display");
    oled.setTextSize(2);
    if (currentLowerPage == 0) {
        if (encoderLocation % 2 == 0) {
            oled.setCursor(52, 12);
            oled.println("On");
        }
        else {
            oled.setCursor(46, 12);
            oled.println("Off");
        }
        oled.setTextSize(1);
        oled.setCursor(10, 36);
        oled.println("Display Brightness");
        oled.setTextSize(2);
        if (dimmed) {
            oled.setCursor(28, 48);
            oled.println("Dimmed");
        }
        else {
            oled.setCursor(40, 48);
            oled.println("Full");
        }
    }
    else {
        if (alwaysOn) {
            oled.setCursor(52, 12);
            oled.println("On");
        }
        else {
            oled.setCursor(46, 12);
            oled.println("Off");
        }
        oled.setTextSize(1);
        oled.setCursor(10, 36);
        oled.println("Display Brightness");
        oled.setTextSize(2);
        if (encoderLocation % 2 == 0) {
            oled.setCursor(28, 48);
            oled.println("Dimmed");
            oled.dim(true);
            dimmed = true;
        }
        else {
            oled.setCursor(40, 48);
            oled.println("Full");
            oled.dim(false);
            dimmed = false;
        }
    }
}


void displayAlarm() {
    oled.setCursor(22, 0);
    oled.println("Set Alarm Time");
    oled.setTextSize(3);
    oled.setCursor((SCREEN_WIDTH - 90) / 2, (SCREEN_HEIGHT - 24) / 2);
    if (currentLowerPage == 0) {
        minLimitEncoder = 0; maxLimitEncoder = 23; allowEncoderOverflow = true;
        int currentLocation = encoderLocation;
        if (currentLocation < 10) {
            oled.print("0");
        }
        oled.print(currentLocation);
        oled.print(":");
        if (wakeMinute < 10) {
            oled.print("0");
        }
        oled.println(wakeMinute);
    }
    else {
        if (wakeHour < 10) {
            oled.print("0");
        }
        oled.print(wakeHour);
        oled.print(":");
        minLimitEncoder = 0; maxLimitEncoder = 59; allowEncoderOverflow = true;
        int currentLocation = encoderLocation;
        if (currentLocation < 10) {
            oled.print("0");
        }
        oled.println(currentLocation);
    }
}


void displayVibration() {
    oled.setCursor(1, 0);
    oled.println("Set Vibration Pattern");
    oled.setTextSize(4);
    oled.setCursor(54, 24);
    minLimitEncoder = 0; maxLimitEncoder = 4; allowEncoderOverflow = false;
    oled.println(encoderLocation+1);
}


void displayPages() {
    oled.setTextSize(1);
    switch (currentPage) {
        case ALARM_PAGE:
            displayAlarm();
            break;
        case VIBRATION_PAGE:
            displayVibration();
            break;
        case SETTINGS_PAGE:
            displaySettings();
            break;
        case SELECTION_PAGE:
            displayClock(false);
            minLimitEncoder = 0; maxLimitEncoder = 2; allowEncoderOverflow = true;
            oled.drawBitmap(arrowLocations[encoderLocation], 16, arrowBitmap, 16, 14, WHITE);
            // draw Settings icon
            oled.drawBitmap(8, 32, alarmBitmap, 32, 32, WHITE);
            // draw Alarm icon
            oled.drawBitmap(48, 32, vibrationBitmap, 32, 32, WHITE);
            // draw vibration Pattern icon
            oled.drawBitmap(88, 32, settingsBitmap, 32, 32, WHITE);
            break;
        default:
            currentPage = SELECTION_PAGE;
            break;
    }
}


void checkActive() {
    if (millis() - lastUserInput < 15000) {
        if (millis() > 15000 or lastUserInput != 0) {
            if (not activeUser) {
                activeUser = true;
                encoderLocation = 0;
            }
        }
    }
    else {
        if (activeUser) {
            activeUser = false;
            currentPage = SELECTION_PAGE;
        }
    }
}


void handleShortPress() {
    switch (currentPage) {
        case SELECTION_PAGE:
            currentLowerPage = 0;
            currentPage = encoderLocation;
            encoderLocation = 0;
            switch (currentPage) {
                case SETTINGS_PAGE:
                    encoderLocation = alwaysOn ? 0 : 1;
                    break;

                case ALARM_PAGE:
                    encoderLocation = wakeHour;
                    break;

                case VIBRATION_PAGE:
                    encoderLocation = vibrationPattern;
                    break;
                
                default:
                    currentPage = SELECTION_PAGE;
                    break;
            }
            break;

        case ALARM_PAGE:
            if (currentLowerPage == 0) {
                wakeHour = encoderLocation;
                encoderLocation = wakeMinute;
                currentLowerPage = 1;
            }
            else {
                wakeMinute = encoderLocation;
                encoderLocation = 0;
                currentLowerPage = 0;
                currentPage = SELECTION_PAGE;
            }
            break;

        case VIBRATION_PAGE:
            vibrationPattern = encoderLocation;
            encoderLocation = 0;
            currentLowerPage = 0;
            currentPage = SELECTION_PAGE;
            break;

        case SETTINGS_PAGE:
            if (currentLowerPage == 0) {
                if (encoderLocation % 2 == 0) {
                    alwaysOn = true;
                }
                else {
                    alwaysOn = false;
                }
                encoderLocation = dimmed ? 0 : 1;
                currentLowerPage = 1;
            }
            else {
                encoderLocation = 0;
                currentLowerPage = 0;
                currentPage = SELECTION_PAGE;
            }
            break;

        default:
            currentPage = SELECTION_PAGE;
            break;
    }
}


void checkEncoderButton() {
    if (digitalRead(ENCODERSWITCH) == LOW) {
        if (not pressed) {
            switchLastPressed = millis();
            pressed = true;
        }
        else if (millis() - switchLastPressed > 500 and not longPressRegistered) {
            longPressRegistered = true;
            playLongPress();
            lastUserInput = millis();
            if (activeUser and currentPage == SELECTION_PAGE) {
                activeUser = false;
                lastUserInput = 0;
            }
            else if (activeUser and currentPage != SELECTION_PAGE) {
                currentPage = SELECTION_PAGE;
                encoderLocation = 0;
                currentLowerPage = 0;
            }
        }
    }

    else if (digitalRead(ENCODERSWITCH) == HIGH) {
        if (pressed) {
            pressed = false;
            longPressRegistered = false;
            if (millis() - switchLastPressed <= 500) {
                playShortPress();
                digitalWrite(RX, !digitalRead(RX));

                lastUserInput = millis();
                if (activeUser) {
                    handleShortPress();
                }
            }
        }
    }
}


void encoder_turn_callA() {
    if (millis() - lastEncoderCallA > 15) {
        if (digitalRead(ENCODERA) == LOW and digitalRead(ENCODERB) == LOW) {
            if (millis() - lastEncoderTurn > 25) {
                encoderLocation++;
                limitEncoder();
                lastUserInput = millis();
                lastEncoderTurn = millis();
            }
        }
        lastEncoderCallA = millis();
    }
}


void encoder_turn_callB() {
    if (millis() - lastEncoderCallB > 15) {
        if (digitalRead(ENCODERA) == LOW and digitalRead(ENCODERB) == LOW) {
            if (millis() - lastEncoderTurn > 25) {
                encoderLocation--;
                limitEncoder();
                lastUserInput = millis();
                lastEncoderTurn = millis();
            }
        }
        lastEncoderCallB = millis();
    }
}




void setup() {
    timeString.reserve(15);
    analogReadResolution(12);

    pinMode(CHARGE, INPUT_PULLUP);
    pinMode(STANDBY, INPUT_PULLUP);
    pinMode(ENCODERSWITCH, INPUT);
    pinMode(ENCODERA, INPUT);
    pinMode(ENCODERB, INPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(LED, OUTPUT);


    // TODO Remove this for the new version
    pinMode(RX, OUTPUT);
    pinMode(TX, OUTPUT);
    digitalWrite(RX, LOW);


    // Initialize OLED Screen
    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        playErrorSound();
        // TODO Error handling
        delay(100);
        while (true);
    }
    delay(10);

    // Display Start Screen
    oled.clearDisplay();
    oled.setTextColor(WHITE);
    oled.setTextSize(2);
    oled.drawBitmap(0, 15, wwwakeBitmap, 128, 35, WHITE);
    oled.display();
    playStartSound();
    digitalWrite(LED, LOW);
    delay(500);
    digitalWrite(LED, HIGH);

    // SETUP RTC MODULE
    if (!rtc.begin()) {
        playErrorSound();
        // TODO Error handling
        delay(100);
        while (true);
    }
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    delay(500);

    attachInterrupt(digitalPinToInterrupt(ENCODERA), encoder_turn_callA, FALLING);
    attachInterrupt(digitalPinToInterrupt(ENCODERB), encoder_turn_callB, FALLING);
    delay(250);
}


void loop() {
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.print("CHARGE: ");\
    oled.println(!digitalRead(CHARGE));
    oled.print("STANDBY: ");
    oled.println(!digitalRead(STANDBY));
    oled.println("Bat-Level");
    delay(30);
    float value = 0.0;
    for (int i=0; i<10; i++) {
        value = value + (float(analogRead(BATLEVEL))/4096) * 3.3 * 2;
        delay(5);
    }
    float median = (float(value)/10);
    oled.println(median);
    oled.display();
    delay(50);


    checkEncoderButton();
    /*checkActive();
    // checkTime();


    // draw Interface
    oled.clearDisplay();
    if (activeUser) {
        displayPages();
        delay(5);
    }
    else if (alwaysOn) {
        displayClock(true);
    }


    oled.display();
    delay(10);*/
}

