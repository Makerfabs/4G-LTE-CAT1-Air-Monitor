#ifndef CONFIG_H
#define CONFIG_H

#define AP_SSID "Makerfabs-"
#define AP_PWD "12345678"

#define DEFAULT_SENSOR_ID "AirM01"
#define DEFAULT_SLEEP_TIME "3600"

#define SUCCESS 1
#define ERROR 0

#define NVS_DATA_LENGTH 20

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

#define DEBUG true

#define POWER_3V3 21
#define POWER_1V8 48

#define SDA 17
#define SCL 18

#define BUTTON_PIN 8
#define LED_PIN 47

#define BAT_PIN 14

#define IO_RXD2 1
#define IO_TXD2 2

#define IO_GSM_PWRKEY 42
#define IO_GSM_RST 41
#define IO_GSM_DTR 39


#endif