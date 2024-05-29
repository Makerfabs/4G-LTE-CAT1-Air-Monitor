#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHT10.h>
#include <Adafruit_SGP30.h>
#include <BH1750.h>

#include "config.h"
#include "web_server.h"

// Sensor Config
char sensor_id[NVS_DATA_LENGTH];
char sleep_time[NVS_DATA_LENGTH];

RTC_DATA_ATTR int bootCount = 0;

Adafruit_AHT10 aht;
Adafruit_SGP30 sgp;
BH1750 lightMeter;
HardwareSerial mySerial2(2);

float temperature = 0.0;
float humidity = 0.0;
float tvoc = 0.0;
float eCO2 = 0.0;
float H2 = 0.0;
float Ethanol = 0.0;
float lux = 0.0;
float bat_vol = 0.0;
int count = 0;

int init_flag_aht10 = 0;
int init_flag_SGP30 = 0;
int init_flag_BH1750 = 0;

void setup()
{
    pin_init();

    Serial.begin(115200);
    mySerial2.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);

    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));
    print_wakeup_reason();

    at_init();

    // For test
    while (0)
    {
        at_send_task("AAAA");
        delay(10000);
    }

    sensor_init();

    // Button Check
    wifi_init();

    delay(5000);
}

void loop()
{
    Serial.println("Reading 20 times...");
    // Send three time and sleep

    if (0) // For test
    {
        read_bat();
        value_report();
        check_count(&count);
        record_count(++count);

        String temp = "";
        temp = json_msg_create();
        at_send_task(temp);
        delay(2000);
    }
    else
        for (int i = 0; i < 20; i++)
        {
            sensor_read();
            value_report();

            String temp = "";
            if (i > 18)
            {
                check_count(&count);
                record_count(++count);
                temp = json_msg_create();
                at_send_task(temp);
            }
            delay(2000);
        }

    // Sleep
    digitalWrite(POWER_3V3, 0);
    digitalWrite(POWER_1V8, 0);

    // Close AT
    digitalWrite(IO_GSM_PWRKEY, HIGH);
    delay(5000);
    digitalWrite(IO_GSM_PWRKEY, LOW);

    int sleep_s = atoi(sleep_time);
    esp_sleep_enable_timer_wakeup(sleep_s * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(sleep_s) + " Seconds");

    Serial.println("Going to sleep now");
    Serial.flush();
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
}

// Hardware init

void pin_init()
{
    pinMode(POWER_3V3, OUTPUT);
    pinMode(POWER_1V8, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BAT_PIN, INPUT);

    pinMode(IO_GSM_RST, OUTPUT);
    pinMode(IO_GSM_PWRKEY, OUTPUT);

    digitalWrite(POWER_3V3, HIGH);
    digitalWrite(POWER_1V8, HIGH);
    digitalWrite(LED_PIN, HIGH);

    digitalWrite(IO_GSM_RST, HIGH);
    delay(1000);
    digitalWrite(IO_GSM_RST, LOW);
    delay(500);
    digitalWrite(IO_GSM_PWRKEY, HIGH);
    delay(1000);
    digitalWrite(IO_GSM_PWRKEY, LOW);
}

void at_init()
{
    Serial.println(F("void at_init()"));

    sendData("AT", 1000, DEBUG);
    delay(1000);
    sendData("AT+SIMCOMATI", 1000, DEBUG);
    delay(1000);
    sendData("AT+CICCID", 1000, DEBUG);
    sendData("AT+CNUM", 1000, DEBUG);
    sendData("AT+COPS?", 1000, DEBUG);
    sendData("AT+CPSI?", 1000, DEBUG);
    sendData("AT+CSQ", 1000, DEBUG);
}

void at_send_task(String data)
{
    Serial.println("void at_send_task(String data)");
    Serial.println(data);
}

// Soil Sensor
void sensor_init()
{
    // SDA SCL
    Wire.begin(SDA, SCL);

    if (!aht.begin())
        Serial.println("SGP30 not found.");
    else
        init_flag_aht10 = 1;

    if (!sgp.begin())
        Serial.println("SGP30 not found.");
    else
        init_flag_SGP30 = 1;

    if (!lightMeter.begin())
        Serial.println("BH1750 not found.");
    else
        init_flag_BH1750 = 1;
}

void sensor_read()
{
    if (init_flag_aht10)
        read_aht10();
    if (init_flag_SGP30)
        read_SGP30();
    if (init_flag_BH1750)
        read_BH1750();
    read_bat();
}

void value_report()
{
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" degrees C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("% rH");

    Serial.print("TVOC ");
    Serial.print(tvoc);
    Serial.print(" ppb\t");
    Serial.print("eCO2 ");
    Serial.print(eCO2);
    Serial.println(" ppm");

    Serial.print("Raw H2 ");
    Serial.print(H2);
    Serial.print(" \t");
    Serial.print("Raw Ethanol ");
    Serial.print(Ethanol);
    Serial.println("");

    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");

    Serial.print("Bat: ");
    Serial.print(bat_vol);
    Serial.println(" V");
}

// Lora send task
String json_msg_create()
{
    String temp = "{";
    temp = temp + "\"ID\":\"" + sensor_id + "\",\"COUNT\":" + count + ",\"SLEEP\":" + sleep_time + ",";
    temp = temp + "\"bat\":" + bat_vol + ",";
    temp = temp + "\"temp\":" + temperature + ",";
    temp = temp + "\"humi\":" + humidity + ",";
    // temp = temp + "\"tvoc\":" + tvoc + ",";
    temp = temp + "\"eco2\":" + eCO2 + ",";
    // temp = temp + "\"h2\":" + H2 + ",";
    // temp = temp + "\"Ethanol\":" + Ethanol + ",";
    temp = temp + "\"lux\":" + lux + "}";

    return temp;
}

uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                                                                // [mg/m^3]
    return absoluteHumidityScaled;
}

void read_aht10()
{
    sensors_event_t humi, temp;
    aht.getEvent(&humi, &temp); // populate temp and humidity objects with fresh data
    temperature = temp.temperature;
    humidity = humi.relative_humidity;
}

void read_SGP30()
{
    sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

    if (!sgp.IAQmeasure())
    {
        Serial.println("Measurement failed");
        return;
    }
    tvoc = sgp.TVOC;
    eCO2 = sgp.eCO2;

    if (!sgp.IAQmeasureRaw())
    {
        Serial.println("Raw Measurement failed");
        return;
    }
    H2 = sgp.rawH2;
    Ethanol = sgp.rawEthanol;
}

void read_BH1750()
{
    lux = lightMeter.readLightLevel();
}

void read_bat()
{
    bat_vol = 3.3 * analogRead(BAT_PIN) / 4096 * 2 + 0.3;
}

void print_wakeup_reason()
{
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
}

float c2f(float c_temp)
{
    return c_temp * 9.0 / 5.0 + 32;
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    mySerial2.println(command);
    long int time = millis();
    while ((time + timeout) > millis())
    {
        while (mySerial2.available())
        {
            char c = mySerial2.read();
            response += c;
        }
    }
    if (debug)
    {
        Serial.print(response);
    }
    return response;
}