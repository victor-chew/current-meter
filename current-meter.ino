/*
 * current-meter.ino
 *
 * Copyright 2018 Victor Chew
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Library required: ESP8266 and ESP32 OLED Driver for SSD1306 Display (by Daniel Eichhorn, Fabrice Weinberg)

#include <Wire.h>
#include "SSD1306Wire.h"

const byte INT_PIN = D6;
const float INT_TO_COULUMB = 0.614439;

bool blinker = false;
unsigned long total_time = 0;
char time_str[64] = "  00:00:00", current_str[64] = "  0.00mA";
volatile bool trigger = false;
volatile unsigned long num_interrupts = 0, time1 = 0, time2 = 0;

SSD1306Wire screen(0x3c, SDA, SCL);

void ICACHE_RAM_ATTR handleInterrupt() {
  if (time1 == 0) {
    time1 = millis();
  } else {
    ++num_interrupts;
    time2 = millis();
    trigger = true;
  }
}

void updateScreen() {

  blinker = !blinker;
  String msg =  "Elapsed time:";
  if (blinker) msg += " *";

  screen.clear();
  screen.setFont(ArialMT_Plain_10);
  screen.drawString(16, 16*0, msg);
  screen.drawString(16, 16*2, "Avg current draw:");
  screen.setFont(ArialMT_Plain_16);
  screen.drawString(16, 16*1, time_str);
  screen.drawString(16, 16*3, current_str);
  screen.display();
}

void setup() {
  Serial.begin(115200);
  screen.init();
  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), handleInterrupt, FALLING);
}

void loop() {
  if (trigger) {
    trigger = false;
    total_time += time2 - time1;
    time1 = time2;
    float ma_avg = total_time == 0 ? 0.0 : (num_interrupts * INT_TO_COULUMB) / total_time * 1000.0 * 1000.0;
    int ss = total_time / 1000;
    int mm = ss / 60; ss -= mm * 60;
    int hh = mm / 60; mm -= hh * 60;
    sprintf(time_str, "  %02d:%02d:%02d", hh, mm ,ss);
    sprintf(current_str, "  %.2fmA", ma_avg);
  }
  updateScreen();
  delay(1000);
}
