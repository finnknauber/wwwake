#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>


#define dcf77 PB12
#define dcfEnable PB13
#define buzzer PA8
#define encoder_switch PB15
#define encoderA PA10
#define encoderB PA2 //PA9
#define BATLEVEL PB1
#define LED PA1
#define CHRG PB4
#define STDBY PB3
#define RX PA3
#define TX PA2

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define selectionPage -1
#define alarmPage 0
#define vibrationPage 1
#define settingsPage 2


Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RTC_DS1307 rtc;


const unsigned int arrow_locations[4] = {16, 56, 96};

// DCF77
volatile unsigned long long currentBuf = 0;
volatile unsigned long long dataBuf = 0;
volatile unsigned long last_high_time = 0;
volatile unsigned long last_dcf_call = 0;
volatile unsigned long dcf77_last_received = 0;
volatile unsigned long dcf77_last_timestamp = 0;
volatile int counter = 0;
int dcfYear = 1969;
int dcfMonth = 1;
int dcfDayOfMonth = 1;
int dcfDayOfWeek = 3;
int dcfHour = 4;
int dcfMinute = 20;
String time_str = "";

// Pages
int currentLowerPage = 0;
int currentPage = -1;

int wake_hour = 0;
int wake_minute = 0;
int vibration_pattern = 1;

unsigned long last_user_input = 0;
bool pressed = false;
bool long_press_registered = false;
bool active_user = false;
bool always_on = true;
bool dimmed = false;


// Encoder
unsigned long switch_last_pressed = 0;
unsigned long last_encoder_turn = 0;
unsigned long last_encoder_callA = 0;
unsigned long last_encoder_callB = 0;
volatile int encoderLocation = 0;
bool allowEncoderOverflow = true;
int minLimitEncoder = 0;
int maxLimitEncoder = 2;


const unsigned char settings_bitmap [] PROGMEM = {
    0x00, 0x03, 0xC0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x07, 0xE0, 0x00,
    0x03, 0x87, 0xE1, 0xC0, 0x07, 0xFF, 0xFF, 0xE0, 0x0F, 0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0xFF, 0xFF, 0xF0, 0x07, 0xF8, 0x1F, 0xE0, 0x07, 0xE0, 0x07, 0xE0, 0x07, 0xC0, 0x03, 0xE0,
    0x07, 0xC0, 0x03, 0xE0, 0x7F, 0x80, 0x01, 0xFE, 0xFF, 0x80, 0x01, 0xFF, 0xFF, 0x80, 0x01, 0xFF,
    0xFF, 0x80, 0x01, 0xFF, 0xFF, 0x80, 0x01, 0xFF, 0x7F, 0x80, 0x01, 0xFE, 0x07, 0xC0, 0x03, 0xE0,
    0x07, 0xC0, 0x03, 0xE0, 0x07, 0xE0, 0x07, 0xE0, 0x07, 0xF8, 0x1F, 0xE0, 0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0xFF, 0xF0, 0x07, 0xFF, 0xFF, 0xE0, 0x03, 0x87, 0xE1, 0xC0,
    0x00, 0x07, 0xE0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x03, 0xC0, 0x00
};


const unsigned char alarm_bitmap [] PROGMEM = {
    0x07, 0x00, 0x00, 0x60, 0x1F, 0xC0, 0x03, 0xF8, 0x3F, 0x80, 0x01, 0xFE, 0x7F, 0x00, 0x00, 0xFE,
    0xFE, 0x07, 0xE0, 0x7F, 0xFC, 0x3F, 0xFC, 0x3F, 0xF0, 0xFF, 0xFF, 0x0F, 0xE1, 0xFC, 0x1F, 0x87,
    0x43, 0xE0, 0x07, 0xC2, 0x07, 0x80, 0x01, 0xE0, 0x07, 0x01, 0x80, 0xF0, 0x0E, 0x01, 0x80, 0x70,
    0x1E, 0x01, 0xC0, 0x78, 0x1C, 0x01, 0xC0, 0x38, 0x1C, 0x01, 0xC0, 0x38, 0x38, 0x01, 0xC0, 0x1C,
    0x38, 0x01, 0xC0, 0x1C, 0x38, 0x01, 0xC0, 0x1C, 0x38, 0x01, 0xE0, 0x1C, 0x38, 0x01, 0xF0, 0x1C,
    0x38, 0x00, 0xF0, 0x1C, 0x1C, 0x00, 0x70, 0x38, 0x1C, 0x00, 0x00, 0x38, 0x1E, 0x00, 0x00, 0x78,
    0x0E, 0x00, 0x00, 0x70, 0x0F, 0x00, 0x00, 0xF0, 0x07, 0x80, 0x01, 0xE0, 0x07, 0xE0, 0x07, 0xF0,
    0x0F, 0xF8, 0x1F, 0xF8, 0x1E, 0xFF, 0xFF, 0x78, 0x3C, 0x3F, 0xFC, 0x3C, 0x18, 0x07, 0xE0, 0x18
};


const unsigned char vibration_bitmap [] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0xFF, 0xFF, 0x00, 0x01, 0xFF, 0xFF, 0x00, 0x01, 0x80, 0x03, 0x00, 0x21, 0x80, 0x03, 0x04,
    0x71, 0x80, 0x03, 0x0E, 0x39, 0x80, 0x03, 0x1C, 0x39, 0x80, 0x03, 0x1C, 0x71, 0x80, 0x03, 0x0E,
    0xE1, 0x80, 0x03, 0x07, 0x71, 0x80, 0x03, 0x0E, 0x39, 0x80, 0x03, 0x1C, 0x71, 0x80, 0x03, 0x0E,
    0xE1, 0x80, 0x03, 0x07, 0x71, 0x80, 0x03, 0x0E, 0x39, 0x80, 0x03, 0x1C, 0x31, 0x80, 0x03, 0x0C,
    0x61, 0x80, 0x03, 0x06, 0x01, 0x80, 0x03, 0x00, 0x01, 0x80, 0x03, 0x00, 0x01, 0x80, 0x03, 0x00,
    0x01, 0x80, 0x03, 0x00, 0x01, 0x80, 0x03, 0x00, 0x01, 0x80, 0x03, 0x00, 0x01, 0xFF, 0xFF, 0x00,
    0x01, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


const unsigned char arrow_bitmap [] PROGMEM = {
    0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
    0x31, 0x8C, 0x39, 0x9C, 0x1D, 0xB8, 0x0F, 0xF0, 0x07, 0xE0, 0x01, 0x80,
};

const unsigned char wwwake_bitmap [] PROGMEM = {
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
    time_str = "";
    if (now.hour() < 10) {
        time_str += "0";
    }
    time_str += now.hour();

    // only blink when time signal available
    if (millis() - dcf77_last_timestamp > 65000 or dcf77_last_timestamp == 0) {
        time_str += ":";
    }
    else {
        if (now.second() % 2 == 1) {
            time_str += " ";
        }
        else {
            time_str += ":";
        }
    }


    if (now.minute() < 10) {
        time_str += "0";
    }
    time_str += now.minute();

    if (center) {
        oled.setTextSize(3);
        oled.setCursor((SCREEN_WIDTH - 90) / 2, (SCREEN_HEIGHT - 24) / 2);
    }
    else {
        oled.setTextSize(2);
        oled.setCursor((SCREEN_WIDTH - 60) / 2, 0);
    }
    oled.println(time_str);

    delay(3);
}

void displaySettings() {
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
        if (always_on) {
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
        if (wake_minute < 10) {
            oled.print("0");
        }
        oled.println(wake_minute);
    }
    else {
        if (wake_hour < 10) {
            oled.print("0");
        }
        oled.print(wake_hour);
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
        case alarmPage:
            displayAlarm();
            break;
        case vibrationPage:
            displayVibration();
            break;
        case settingsPage:
            displaySettings();
            break;
        case selectionPage:
            displayClock(false);
            minLimitEncoder = 0; maxLimitEncoder = 2; allowEncoderOverflow = true;
            oled.drawBitmap(arrow_locations[encoderLocation], 16, arrow_bitmap, 16, 14, WHITE);
            // draw Settings icon
            oled.drawBitmap(8, 32, alarm_bitmap, 32, 32, WHITE);
            // draw Alarm icon
            oled.drawBitmap(48, 32, vibration_bitmap, 32, 32, WHITE);
            // draw vibration Pattern icon
            oled.drawBitmap(88, 32, settings_bitmap, 32, 32, WHITE);
            break;
        default:
            currentPage = selectionPage;
            break;
    }
}


void checkActive() {
    if (millis() - last_user_input < 15000) {
        if (millis() > 15000 or last_user_input != 0) {
            if (not active_user) {
                active_user = true;
                encoderLocation = 0;
            }
        }
    }
    else {
        if (active_user) {
            active_user = false;
            currentPage = selectionPage;
        }
    }
}


void playErrorSound() {
    tone(buzzer, 4000);
    delay(500);
    tone(buzzer, 1000);
    delay(700);
    noTone(buzzer);
    digitalWrite(buzzer, LOW);
}

void playShortPress() {
    tone(buzzer, 4000);
    delay(50);
    noTone(buzzer);
    digitalWrite(buzzer, LOW);
}

void playLongPress() {
    tone(buzzer, 4000);
    delay(250);
    noTone(buzzer);
    digitalWrite(buzzer, LOW);
}

void playStartSound() {
    tone(buzzer, 2500);
    delay(500);
    tone(buzzer, 3000);
    delay(500);
    tone(buzzer, 4000);
    delay(500);
    noTone(buzzer);
    digitalWrite(buzzer, LOW);
}


void handleShortPress() {
    switch (currentPage) {
        case selectionPage:
            currentLowerPage = 0;
            currentPage = encoderLocation;
            encoderLocation = 0;
            switch (currentPage) {
                case settingsPage:
                    encoderLocation = always_on ? 0 : 1;
                    break;

                case alarmPage:
                    encoderLocation = wake_hour;
                    break;

                case vibrationPage:
                    encoderLocation = vibration_pattern;
                    break;
                
                default:
                    currentPage = selectionPage;
                    break;
            }
            break;

        case alarmPage:
            if (currentLowerPage == 0) {
                wake_hour = encoderLocation;
                encoderLocation = wake_minute;
                currentLowerPage = 1;
            }
            else {
                wake_minute = encoderLocation;
                encoderLocation = 0;
                currentLowerPage = 0;
                currentPage = selectionPage;
            }
            break;

        case vibrationPage:
            vibration_pattern = encoderLocation;
            encoderLocation = 0;
            currentLowerPage = 0;
            currentPage = selectionPage;
            break;

        case settingsPage:
            if (currentLowerPage == 0) {
                if (encoderLocation % 2 == 0) {
                    always_on = true;
                }
                else {
                    always_on = false;
                }
                encoderLocation = dimmed ? 0 : 1;
                currentLowerPage = 1;
            }
            else {
                encoderLocation = 0;
                currentLowerPage = 0;
                currentPage = selectionPage;
            }
            break;

        default:
            currentPage = selectionPage;
            break;
    }
}


void checkEncoderButton() {
    if (digitalRead(encoder_switch) == LOW) {
        if (not pressed) {
            switch_last_pressed = millis();
            pressed = true;
        }
        else if (millis() - switch_last_pressed > 500 and not long_press_registered) {
            long_press_registered = true;
            playLongPress();
            last_user_input = millis();
            if (active_user and currentPage == selectionPage) {
                active_user = false;
                last_user_input = 0;
            }
            else if (active_user and currentPage != selectionPage) {
                currentPage = selectionPage;
                encoderLocation = 0;
                currentLowerPage = 0;
            }
        }
    }

    else if (digitalRead(encoder_switch) == HIGH) {
        if (pressed) {
            pressed = false;
            long_press_registered = false;
            if (millis() - switch_last_pressed <= 500) {
                playShortPress();
                digitalWrite(RX, !digitalRead(RX));

                last_user_input = millis();
                if (active_user) {
                    handleShortPress();
                }
            }
        }
    }
}


void encoder_turn_callA() {
    if (millis() - last_encoder_callA > 15) {
        if (digitalRead(encoderA) == LOW and digitalRead(encoderB) == LOW) {
            if (millis() - last_encoder_turn > 25) {
                encoderLocation++;
                limitEncoder();
                last_user_input = millis();
                last_encoder_turn = millis();
            }
        }
        last_encoder_callA = millis();
    }
}


void encoder_turn_callB() {
    if (millis() - last_encoder_callB > 15) {
        if (digitalRead(encoderA) == LOW and digitalRead(encoderB) == LOW) {
            if (millis() - last_encoder_turn > 25) {
                encoderLocation--;
                limitEncoder();
                last_user_input = millis();
                last_encoder_turn = millis();
            }
        }
        last_encoder_callB = millis();
    }
}


void checkTime() {
    if (dcf77_last_received != 0) {
        if (bitRead(dataBuf, 0) == 0) {
            dcfYear = 2000 + ((dataBuf >> 54) & 0x0F) * 10 + ((dataBuf >> 50) & 0x0F);  // year = bit 50-57
            dcfMonth = ((dataBuf >> 49) & 0x01) * 10 + ((dataBuf >> 45) & 0x0F);        // month = bit 45-49
            dcfDayOfMonth = ((dataBuf >> 40) & 0x03) * 10 + ((dataBuf >> 36) & 0x0F);   // day of the month = bit 36-41
            dcfDayOfWeek = (dataBuf >> 42) & 0x07;                                      // day of the week = bit 42-44
            dcfHour = ((dataBuf >> 33) & 0x03) * 10 + ((dataBuf >> 29) & 0x0F);         // hour = bit 29-34
            dcfMinute = ((dataBuf >> 25) & 0x07) * 10 + ((dataBuf >> 21) & 0x0F);       // minute = 21-27
            if (millis() - dcf77_last_received < 60000) {
                int dcfSeconds = (millis() - dcf77_last_received) / 1000;
                rtc.adjust(DateTime(dcfYear, dcfMonth, dcfDayOfMonth, dcfHour, dcfMinute, dcfSeconds));
            }
            else {
                dcf77_last_timestamp = 0;
            }
        }
        else {
            dcf77_last_timestamp = 0;
        }
        dataBuf = 0;
        dcf77_last_received = 0;
    }
}

void DCF77_ISR() {
    // debouncing
    if (millis() >= last_dcf_call + 50) {
        // new minute starting + new signal start
        if (millis() - last_dcf_call > 1500 and millis() > 10000) {
            if (counter == 59 and dcf77_last_received == 0) {
                for (int i = 0; i < 59; i++) {
                    dataBuf = dataBuf << 1;
                    dataBuf |= bitRead(currentBuf, i);
                }
                dcf77_last_received = millis();
                dcf77_last_timestamp = millis();
            }
            counter = 0;
            currentBuf = 0;
        }


        // signal incoming
        if (digitalRead(dcf77)) {
            digitalWrite(LED, !digitalRead(LED));
            last_high_time = millis();
        }

        // signal readout
        else {
            unsigned long high_time = millis() - last_high_time;
            currentBuf = currentBuf << 1;
            if (high_time > 150 and high_time < 250) {
                currentBuf |= 1;
            }

            last_high_time = millis();
            counter += 1;
        }
        last_dcf_call = millis();
    }
}



void setup() {
    time_str.reserve(15);
    analogReadResolution(12);

    pinMode(CHRG, INPUT_PULLUP);
    pinMode(STDBY, INPUT_PULLUP);
    pinMode(buzzer, OUTPUT);
    pinMode(encoder_switch, INPUT);
    pinMode(encoderA, INPUT_PULLUP);
    pinMode(encoderB, INPUT_PULLUP);
    pinMode(RX, OUTPUT);
    pinMode(LED, OUTPUT);
    digitalWrite(RX, LOW);


    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        playErrorSound();
        // TODO Error handling
        delay(100);
        while (true);
    }
    delay(10);
    oled.clearDisplay();
    oled.setTextColor(WHITE);
    oled.setTextSize(2);
    oled.drawBitmap(0, 15, wwwake_bitmap, 128, 35, WHITE);
    oled.display();
    playStartSound();
    digitalWrite(LED, LOW);
    delay(500);
    digitalWrite(LED, HIGH);

    attachInterrupt(digitalPinToInterrupt(encoderA), encoder_turn_callA, FALLING);
    attachInterrupt(digitalPinToInterrupt(encoderB), encoder_turn_callB, FALLING);
    digitalWrite(dcfEnable, HIGH);
    attachInterrupt(digitalPinToInterrupt(dcf77), DCF77_ISR, CHANGE);
    digitalWrite(buzzer, LOW);
    delay(500);


    // SETUP RTC MODULE
    if (!rtc.begin()) {
        playErrorSound();
        // TODO Error handling
        delay(100);
        while (true);
    }
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    delay(1000);
}


void loop() {
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.print("CHARGE: ");
    oled.println(digitalRead(CHRG));
    oled.print("STANDBY: ");
    oled.println(digitalRead(STDBY));
    oled.println("Bat-Level");
    // digitalWrite(RX, LOW);
    delay(30);
    float value = 0.0;
    for (int i=0; i<10; i++) {
        value = value + (float(analogRead(BATLEVEL))/4096) * 3.3 * 2;
        delay(5);
    }
    // digitalWrite(RX, HIGH);
    float median = (float(value)/10);
    oled.println(median);
    // oled.fillScreen(WHITE);
    oled.display();
    delay(50);


    checkEncoderButton();
    /*checkActive();
    // checkTime();


    // draw Interface
    oled.clearDisplay();
    if (active_user) {
        displayPages();
        delay(5);
    }
    else if (always_on) {
        displayClock(true);
    }


    oled.display();
    delay(10);*/
}

