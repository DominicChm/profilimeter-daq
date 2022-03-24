// SET PINS HERE!!!

// Shock pot gets power from digital pins - Don't worry about hooking it up to power
// White has to be an analog pin!!!
#define PIN_SHOCKPOT_WHITE A17
#define PIN_SHOCKPOT_RED A16
#define PIN_SHOCKPOT_BLACK A15

// Hall effect signal (should be white...?)
// Connect black to teesny 3.5 ground, and red to teensy Vin pin.
#define PIN_HALL_SIGNAL 44

#include <Arduino.h>
#include <SdFat.h>
#include <jled.h>
#include "util.h"

enum {
    OK,
    SD_FAIL,
    FILE_FAIL,
} init_state;

SdFs sd;
FsFile file;
JLed led(LED_BUILTIN);

char filename[32];
bool hall_flag = false;

struct {
    uint32_t hall_ticks;
    uint16_t pot_val;
} data;

void isr_hall() {
    hall_flag = true;
}

void setup() {
    pinMode(PIN_SHOCKPOT_BLACK, OUTPUT);
    pinMode(PIN_SHOCKPOT_RED, OUTPUT);
    pinMode(PIN_SHOCKPOT_WHITE, INPUT);

    pinMode(PIN_HALL_SIGNAL, INPUT);

    digitalWrite(PIN_SHOCKPOT_BLACK, LOW);
    digitalWrite(PIN_SHOCKPOT_RED, HIGH);

    // Init SD card and files.
    if (!sd.begin(BUILTIN_SDCARD)) {
        if (sd.sdErrorCode()) {
            if (sd.sdErrorCode() == SD_CARD_ERROR_ACMD41) Serial.println("Try power cycling the SD card.");
            sd.printSdError(&Serial);
        }
        init_state = SD_FAIL;
        led.Blink(1000, 200).Forever();
        return;
    }

    select_next_filename(filename, &sd);

    if (!file.open(filename, O_RDWR | O_CREAT)) {
        led.Blink(200, 1000).Forever();
        return;
    }

    file.printf("Hall ticks, Pot Value\n");

    led.On().Forever();

    attachInterrupt(digitalPinToInterrupt(PIN_HALL_SIGNAL), isr_hall, CHANGE);

    Serial.println("Init successful! Logging...");
}

void loop() {
    if (init_state == OK && hall_flag) {
        hall_flag = false;

        data.hall_ticks++;
        data.pot_val = analogRead(PIN_SHOCKPOT_WHITE);

        file.printf("%d, %d\n", data.hall_ticks, data.pot_val);

        file.flush();
        file.truncate();
        file.sync();
    }

    led.Update();
}