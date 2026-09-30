#include "driver/twai.h"
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

#define RX_PIN 18
#define TX_PIN 19
#define CAN_TX_ID 0x100
#define CAN_RX_ID 0x101  // incoming "set time" message

unsigned long lastCANTxMillis = 0;
#define CAN_TX_INTERVAL_MS 1000

// ---------------- CAN send ----------------
void sendCAN(uint32_t id, const uint8_t *data, uint8_t len) {
  if (len > 8) len = 8;

  twai_message_t msg = {};
  msg.identifier = id;
  msg.data_length_code = len;
  msg.flags = TWAI_MSG_FLAG_NONE;  // standard 11-bit ID

  memcpy(msg.data, data, len);

  esp_err_t err = twai_transmit(&msg, pdMS_TO_TICKS(100));
  if (err == ESP_OK) {
    Serial.printf("Sent ID=0x%03X\n", id);
  } else {
    Serial.printf("TX failed: %s\n", esp_err_to_name(err));
  }
}

// ---------------- CAN receive / set time ----------------
void handleCANMessage(const twai_message_t &msg) {
  if (msg.identifier != CAN_RX_ID) return;
  if (msg.data_length_code < 6) {
    Serial.println("SET TIME msg too short, ignoring");
    return;
  }

  int year  = msg.data[0] + 2000;
  int month = msg.data[1];
  int day   = msg.data[2];
  int hour  = msg.data[3];
  int minute = msg.data[4];
  int second = msg.data[5];

  // Basic sanity check before adjusting the RTC
  if (month < 1 || month > 12 || day < 1 || day > 31 ||
      hour > 23 || minute > 59 || second > 59) {
    Serial.println("SET TIME msg out of range, ignoring");
    return;
  }

  rtc.adjust(DateTime(year, month, day, hour, minute, second));
  Serial.printf("RTC set via CAN: %04d-%02d-%02d %02d:%02d:%02d\n",
                year, month, day, hour, minute, second);
}

void serviceCANReceive() {
  twai_message_t msg;
  // Non-blocking poll; process anything waiting
  while (twai_receive(&msg, 0) == ESP_OK) {
    handleCANMessage(msg);
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("RTC <-> CAN starting");

  // RTC
  Wire.begin(22, 23);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // CAN
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("TWAI driver install failed");
    while (1);
  }
  if (twai_start() != ESP_OK) {
    Serial.println("TWAI start failed");
    while (1);
  }

  Serial.println("CAN driver started");
}

// ---------------- Loop ----------------
void loop() {
  unsigned long now = millis();

  // Check for incoming "set time" messages
  serviceCANReceive();

  // Periodically broadcast current time
  if (now - lastCANTxMillis >= CAN_TX_INTERVAL_MS) {
    lastCANTxMillis = now;

    DateTime t = rtc.now();

    uint8_t data[8] = {
      (uint8_t)(t.year() - 2000),
      (uint8_t)t.month(),
      (uint8_t)t.day(),
      (uint8_t)t.hour(),
      (uint8_t)t.minute(),
      (uint8_t)t.second(),
      0,
      0
    };

    sendCAN(CAN_TX_ID, data, 8);

    Serial.printf("%04d-%02d-%02d %02d:%02d:%02d\n",
                  t.year(), t.month(), t.day(), t.hour(), t.minute(), t.second());
  }
}