#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHT10.h>
#include <Adafruit_SGP30.h>
#include <BH1750.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Makerfabs";  
const char* password = "20160704";  
const char* serverName = "https://api.datacake.co/integrations/api/3fe9f124-9c7c-4ddf-88fa-7aa14fde3cac/";    //you HTTP endpoint URL

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

#define DBG_OUT USBSerial // Serial
HardwareSerial mySerial2(2);

Adafruit_AHT10 aht;
Adafruit_SGP30 sgp;
BH1750 lightMeter;

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
    DBG_OUT.begin(115200);
    mySerial2.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    pin_init();
    sensor_init();
}

void loop()
{
  sensor_read();
  value_report();
  if (WiFi.status() == WL_CONNECTED) //If the WIFI connection is successful
  {    
    HTTPClient http;

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> jsonDoc;
    jsonDoc["device"] = "e570efbf-cca7-4fe8-8670-909b1e01d689";   //you Serial Number
    jsonDoc["Temperature"] = temperature;
    jsonDoc["Humidity"] = humidity;
    jsonDoc["Tvoc"] = tvoc;    
    jsonDoc["CO2"] = eCO2;
    jsonDoc["H2"] = H2;
    jsonDoc["Ethanol"] = Ethanol;
    jsonDoc["Light"] = lux;
    jsonDoc["Bat"] = bat_vol;

    String requestBody;
    serializeJson(jsonDoc, requestBody);

    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) 
    {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else 
    {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();   
  }

  delay(60000); // Send data every 60 seconds
  
}

void pin_init()
{
    pinMode(POWER_3V3, OUTPUT);
    pinMode(POWER_1V8, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BAT_PIN, INPUT);

    digitalWrite(POWER_3V3, HIGH);
    digitalWrite(POWER_1V8, HIGH);
    digitalWrite(LED_PIN, HIGH);

    pinMode(IO_GSM_RST, OUTPUT);
    pinMode(IO_GSM_PWRKEY, OUTPUT);

    digitalWrite(IO_GSM_RST, LOW);
    digitalWrite(IO_GSM_PWRKEY, LOW);
    delay(100);
    digitalWrite(IO_GSM_PWRKEY, HIGH);
    delay(2000);
    digitalWrite(IO_GSM_PWRKEY, LOW);
}

void sensor_init()
{
    DBG_OUT.println("Sensor init begin.");

    Wire.begin(SDA, SCL);

    if (!aht.begin())
        DBG_OUT.println("SGP30 not found.");
    else
        init_flag_aht10 = 1;

    if (!sgp.begin())
        Serial.println("SGP30 not found.");
    else
        init_flag_SGP30 = 1;

    if (!lightMeter.begin())
        DBG_OUT.println("BH1750 not found.");
    else
        init_flag_BH1750 = 1;

    DBG_OUT.println("Sensor init over.");
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
    count++;
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
        DBG_OUT.println("Measurement failed");
        return;
    }
    tvoc = sgp.TVOC;
    eCO2 = sgp.eCO2;

    if (!sgp.IAQmeasureRaw())
    {
        DBG_OUT.println("Raw Measurement failed");
        return;
    }
    H2 = sgp.rawH2;
    Ethanol = sgp.rawEthanol;
}

void read_BH1750()
{
    lux = lightMeter.readLightLevel();
}

void read_bat() //
{
    bat_vol = 3.3 * analogRead(BAT_PIN) / 4096 * 2 + 0.3;
}

void value_report()
{
    DBG_OUT.println("\n-------------------------------------");
    DBG_OUT.printf("Num:\t%d\n", count);

    DBG_OUT.print("Temperature: ");
    DBG_OUT.print(temperature);
    DBG_OUT.println(" degrees C");
    DBG_OUT.print("Humidity: ");
    DBG_OUT.print(humidity);
    DBG_OUT.println("% rH");

    DBG_OUT.print("TVOC ");
    DBG_OUT.print(tvoc);
    DBG_OUT.print(" ppb\t");
    DBG_OUT.print("eCO2 ");
    DBG_OUT.print(eCO2);
    DBG_OUT.println(" ppm");

    DBG_OUT.print("Raw H2 ");
    DBG_OUT.print(H2);
    DBG_OUT.print(" \t");
    DBG_OUT.print("Raw Ethanol ");
    DBG_OUT.print(Ethanol);
    DBG_OUT.println("");

    DBG_OUT.print("Light: ");
    DBG_OUT.print(lux);
    DBG_OUT.println(" lx");

    DBG_OUT.print("Bat: ");
    DBG_OUT.print(bat_vol);
    DBG_OUT.println(" V");

    DBG_OUT.println("\n-------------------------------------");

    // 14:37:57.103 -> Num:	5
    // 14:37:57.103 -> Temperature: 28.98 degrees C
    // 14:37:57.103 -> Humidity: 44.40% rH
    // 14:37:57.103 -> TVOC 0.00 ppb	eCO2 400.00 ppm
    // 14:37:57.103 -> Raw H2 13978.00 	Raw Ethanol 19738.00
    // 14:37:57.103 -> Light: 80.00 lx
    // 14:37:57.103 -> Bat: 4.26 V
}





