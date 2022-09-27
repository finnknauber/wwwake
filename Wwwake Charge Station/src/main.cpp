#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>


#define dcf77 PB12
#define dcfEnable PB13
#define buzzer PA8
#define encoder_switch PB15
#define encoderA PA9
#define encoderB PA10

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RTC_DS1307 rtc;


const unsigned char clock_bitmap [] PROGMEM = {
    0x00, 0x0F, 0xF0, 0x00, 0x00, 0x7F, 0xFE, 0x00, 0x01, 0xFF, 0xFF, 0x00, 0x03, 0xF0, 0x0F, 0xC0,
    0x07, 0x80, 0x03, 0xE0, 0x0F, 0x00, 0x00, 0xF0, 0x1E, 0x01, 0x80, 0x78, 0x3C, 0x03, 0x80, 0x38,
    0x38, 0x03, 0x80, 0x1C, 0x70, 0x03, 0x80, 0x1C, 0x70, 0x03, 0x80, 0x0E, 0xF0, 0x03, 0x80, 0x0E,
    0xE0, 0x03, 0x80, 0x0E, 0xE0, 0x03, 0x80, 0x07, 0xE0, 0x03, 0x80, 0x07, 0xE0, 0x03, 0xC0, 0x07,
    0xE0, 0x03, 0xE0, 0x07, 0xE0, 0x00, 0xF8, 0x07, 0xE0, 0x00, 0x7C, 0x07, 0xE0, 0x00, 0x1E, 0x0E,
    0xF0, 0x00, 0x0E, 0x0E, 0x70, 0x00, 0x00, 0x0E, 0x78, 0x00, 0x00, 0x1C, 0x38, 0x00, 0x00, 0x3C,
    0x3C, 0x00, 0x00, 0x38, 0x1E, 0x00, 0x00, 0x78, 0x0F, 0x00, 0x01, 0xF0, 0x07, 0xC0, 0x03, 0xE0,
    0x03, 0xF0, 0x1F, 0xC0, 0x01, 0xFF, 0xFF, 0x00, 0x00, 0x7F, 0xFC, 0x00, 0x00, 0x0F, 0xE0, 0x00
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
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80,
    0x00, 0x00, 0xF8, 0x78, 0x3D, 0xE0, 0xF0, 0xFB, 0xC3, 0xE1, 0xF1, 0xFC, 0x0F, 0x87, 0xC1, 0xFC,
    0x78, 0xFC, 0x7D, 0xE1, 0xF0, 0xF3, 0xC3, 0xE1, 0xE3, 0xFF, 0x0F, 0x8F, 0x87, 0xFE, 0x78, 0xFC,
    0x79, 0xE1, 0xF0, 0xF3, 0xC7, 0xE3, 0xE7, 0xFF, 0x0F, 0x9F, 0x07, 0xFF, 0x78, 0xFC, 0xF8, 0xF3,
    0xF1, 0xE3, 0xC7, 0xE3, 0xC7, 0x8F, 0x8F, 0xBE, 0x0F, 0x8F, 0x79, 0xFC, 0xF0, 0xF3, 0xF9, 0xE3,
    0xC7, 0xE7, 0xC7, 0x07, 0x8F, 0xFC, 0x1F, 0x07, 0x79, 0xFC, 0xF0, 0xF7, 0xFB, 0xC1, 0xCF, 0xE7,
    0x80, 0x3F, 0x8F, 0xFC, 0x1F, 0x07, 0x7B, 0xDD, 0xE0, 0xF7, 0x7B, 0xC1, 0xEE, 0xE7, 0x83, 0xFF,
    0x8F, 0xFC, 0x1F, 0xFF, 0x3B, 0x9D, 0xE0, 0xF7, 0x3F, 0x81, 0xFE, 0xFF, 0x07, 0xFF, 0x8F, 0xFE,
    0x1F, 0xFF, 0x3F, 0x9F, 0xC0, 0xFF, 0x3F, 0x81, 0xFC, 0xFF, 0x0F, 0xC7, 0x8F, 0xFE, 0x1E, 0x00,
    0x3F, 0x9F, 0xC0, 0x7E, 0x3F, 0x01, 0xFC, 0xFE, 0x0F, 0x87, 0x8F, 0xDF, 0x1F, 0x07, 0x3F, 0x1F,
    0x80, 0x7E, 0x3F, 0x01, 0xF8, 0x7E, 0x0F, 0x8F, 0x8F, 0x9F, 0x0F, 0x8F, 0x3F, 0x1F, 0x80, 0x7C,
    0x3E, 0x00, 0xF8, 0x7C, 0x0F, 0xFF, 0x8F, 0x8F, 0x87, 0xFF, 0x3E, 0x0F, 0x00, 0x7C, 0x3E, 0x00,
    0xF8, 0x7C, 0x07, 0xFF, 0x8F, 0x87, 0xC7, 0xFF, 0x3E, 0x0F, 0x00, 0x7C, 0x3C, 0x00, 0xF0, 0x78,
    0x03, 0xF7, 0x8F, 0x87, 0xC1, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x22, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x32, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x72, 0x23,
    0x00, 0x61, 0xA0, 0x09, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x56, 0x53, 0x4D, 0xF3,
    0x60, 0x89, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x5C, 0x3B, 0x8D, 0x92, 0x20, 0x89,
    0x10, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xCC, 0xDB, 0xCD, 0x92, 0x60, 0xC9, 0x10, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8C, 0xDB, 0x4D, 0x93, 0xE0, 0x79, 0xF0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x80, 0x60, 0x00, 0x10, 0x20, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

const unsigned int arrow_locations[4] = {16, 56, 96};

volatile unsigned long long currentBuf = 0;
volatile unsigned long long dataBuf = 0;
volatile unsigned long last_high_time = 0;
volatile unsigned long last_dcf_call = 0;
volatile unsigned long dcf77_last_received = 0;
volatile unsigned long dcf77_last_timestamp = 0;
unsigned long switch_last_pressed = 0;
unsigned long last_encoder_turn = 0;
unsigned long last_user_input = 0;
volatile int counter = 0;

int wake_hour = 0;
int wake_minute = 0;
int vibration_pattern = 1;

int dcfYear = 1969;
int dcfMonth = 1;
int dcfDayOfMonth = 1;
int dcfDayOfWeek = 3;
int dcfHour = 4;
int dcfMinute = 20;
String time_str = "";

int encoder_location = 0;
int currentPage = -1;
int currentLowerPage = 0;


bool pressed = false;
bool long_press_registered = false;
bool active_user = false;
bool always_on = true;
bool dimmed = false;



void limitEncoder(int minLimit, int maxLimit) {
    if (encoder_location < minLimit) {
        encoder_location = minLimit;
    }
    if (encoder_location > maxLimit) {
        encoder_location = maxLimit;
    }
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


void displayPages() {
    oled.setTextSize(1);
    if (currentPage == 0) {
        oled.setCursor(13, 0);
        oled.println("Always On Display");
        oled.setTextSize(2);
        if (currentLowerPage == 0) {
            if (encoder_location % 2 == 0) {
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
            if (encoder_location % 2 == 0) {
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

    else if (currentPage == 1) {
        oled.setCursor(22, 0);
        oled.println("Set Alarm Time");
        oled.setTextSize(3);
        oled.setCursor((SCREEN_WIDTH - 90) / 2, (SCREEN_HEIGHT - 24) / 2);
        if (currentLowerPage == 0) {
            limitEncoder(0, 23);
            if (encoder_location < 10) {
                oled.print("0");
            }
            oled.print(encoder_location);
            oled.print(":");
            if (wake_minute < 10) {
                oled.print("0");
            }
            oled.println(wake_minute);
        }
        else {
            limitEncoder(0, 59);
            if (wake_hour < 10) {
                oled.print("0");
            }
            oled.print(wake_hour);
            oled.print(":");
            if (encoder_location < 10) {
                oled.print("0");
            }
            oled.println(encoder_location);
        }
    }

    else if (currentPage == 2) {
        limitEncoder(1, 5);
        oled.setCursor(1, 0);
        oled.println("Set Vibration Pattern");
        oled.setTextSize(4);
        oled.setCursor(54, 24);
        oled.println(encoder_location);
    }

    else if (currentPage == -1) {
        limitEncoder(0, 2);
        displayClock(false);
        oled.drawBitmap(arrow_locations[encoder_location], 16, arrow_bitmap, 16, 14, WHITE);
        // draw Clock icon
        oled.drawBitmap(8, 32, clock_bitmap, 32, 32, WHITE);
        // draw Alarm icon
        oled.drawBitmap(48, 32, alarm_bitmap, 32, 32, WHITE);
        // draw vibration Pattern icon
        oled.drawBitmap(88, 32, vibration_bitmap, 32, 32, WHITE);
    }
    else {
        currentPage = -1;
    }
}


void checkActive() {
    if (millis() - last_user_input < 15000) {
        if (millis() > 15000 or last_user_input != 0) {
            if (not active_user) {
                active_user = true;
                encoder_location = 0;
            }
        }
    }
    else {
        if (active_user) {
            active_user = false;
            currentPage = -1;
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
    if (active_user and currentPage == -1) {
        currentPage = encoder_location;
        encoder_location = 0;
        currentLowerPage = 0;
        if (currentPage == 0) {
            encoder_location = always_on ? 0 : 1;
        }
        else if (currentPage == 1) {
            encoder_location = wake_hour;
        }
        else if (currentPage == 2) {
            encoder_location = vibration_pattern;
        }
    }
    else if (active_user and currentPage == 0) {
        if (currentLowerPage == 0) {
            if (encoder_location % 2 == 0) {
                always_on = true;
            }
            else {
                always_on = false;
            }
            encoder_location = dimmed ? 0 : 1;
            currentLowerPage = 1;
        }
        else {
            encoder_location = 0;
            currentLowerPage = 0;
            currentPage = -1;
        }
    }

    else if (active_user and currentPage == 1) {
        if (currentLowerPage == 0) {
            wake_hour = encoder_location;
            encoder_location = wake_minute;
            currentLowerPage = 1;
        }
        else {
            wake_minute = encoder_location;
            encoder_location = 0;
            currentLowerPage = 0;
            currentPage = -1;
        }
    }
    else if (active_user and currentPage == 2) {
        vibration_pattern = encoder_location;
        encoder_location = 0;
        currentLowerPage = 0;
        currentPage = -1;
    }
    else if (active_user) {
        currentPage = -1;
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
            if (active_user and currentPage == -1) {
                active_user = false;
                last_user_input = 0;
            }
            else if (active_user and currentPage != -1) {
                currentPage = -1;
                encoder_location = 0;
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
                last_user_input = millis();
                handleShortPress();
            }
        }
    }
}


void encoder_turn_call() {
    if (millis() - last_encoder_turn > 50) {
        if (digitalRead(encoderA) == LOW and digitalRead(encoderB) == LOW) {
            encoder_location++;
            last_user_input = millis();
        }
        else if (digitalRead(encoderA) == LOW and digitalRead(encoderB) == HIGH) {
            while (digitalRead(encoderB) == HIGH) {}
            if (digitalRead(encoderA) == LOW and digitalRead(encoderB) == LOW) {
                encoder_location--;
                last_user_input = millis();
            }
        }

        last_encoder_turn = millis();
    }
}


void checkTime() {
    /*if (dcf77_last_received != 0) {
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
                Serial.print("Set Time to ");
                Serial.print(dcfHour);
                Serial.print(":");
                Serial.println(dcfMinute);
            }
            else {
                dcf77_last_timestamp = 0; Serial.println("Timestamp out of Date!");
            }
        }
        else {
            dcf77_last_timestamp = 0; Serial.println("Time Parity Incorrect!");
        }
        dataBuf = 0;
        dcf77_last_received = 0;
    }*/
}

void DCF77_ISR() {
    /*// debouncing
    if (millis() >= last_dcf_call + 50) {
        // new minute starting + new signal start
        if (millis() - last_dcf_call > 1500 and millis() > 10000) {
            Serial.print(counter);
            Serial.println(" Bits received (59 expected)");
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
            digitalWrite(PC13, !digitalRead(PC13));
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
    }*/
}



void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");
    time_str.reserve(15);

    pinMode(dcf77, INPUT);
    pinMode(dcfEnable, OUTPUT);
    pinMode(buzzer, OUTPUT);
    pinMode(encoder_switch, INPUT);
    pinMode(PC13, OUTPUT);
    pinMode(encoderA, INPUT_PULLUP);
    pinMode(encoderB, INPUT_PULLUP);

    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed");
        playErrorSound();
        // TODO Error handling
        delay(100);
        while (true);
    }
    delay(10);
    oled.clearDisplay();
    oled.setTextColor(WHITE);
    oled.setTextSize(2);
    oled.drawBitmap(0, 0, wwwake_bitmap, 128, 64, WHITE);
    oled.display();
    playStartSound();
    delay(1000);

    attachInterrupt(digitalPinToInterrupt(encoderA), encoder_turn_call, FALLING);
    attachInterrupt(digitalPinToInterrupt(dcf77), DCF77_ISR, CHANGE);
    digitalWrite(buzzer, LOW);
    delay(500);

    // SETUP RTC MODULE
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        playErrorSound();
        // TODO Error handling
        delay(100);
        while (true);
    }
    delay(1500);
    digitalWrite(dcfEnable, LOW);
}


void loop() {
    checkActive();
    checkEncoderButton();
    //checkTime();

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
    delay(10);
}

