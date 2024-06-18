#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHT10.h>
#include <Adafruit_SGP30.h>
#include <BH1750.h>

String Apikey = "TE0ELD7W76JLH1DA";

#define DEBUG true

#define LTE_RESET_PIN 41
#define LTE_PWRKEY_PIN 42

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

    Serial.begin(115200);
    mySerial2.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);

    pin_init();

    sensor_init();

    while (1)
    {

        if (digitalRead(BUTTON_PIN) == 0)
        {
            delay(100);
            if (digitalRead(BUTTON_PIN) == 0)
            {
                Serial.println("Button press down.");
                break;
            }
        }
        else
        {
            sensor_read();
            value_report();
            http_request();
            delay(1000);
        }
    }

    at_init();
}

void loop()
{
    // put your main code here, to run repeatedly:
    while (Serial.available() > 0)
    {
        mySerial2.write(Serial.read());
        yield();
    }
    while (mySerial2.available() > 0)
    {
        Serial.write(mySerial2.read());
        yield();
    }
}

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

    pinMode(LTE_RESET_PIN, OUTPUT);
    digitalWrite(LTE_RESET_PIN, LOW);
    pinMode(LTE_PWRKEY_PIN, OUTPUT);
    digitalWrite(LTE_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(LTE_PWRKEY_PIN, HIGH);
    delay(2000);
    digitalWrite(LTE_PWRKEY_PIN, LOW);
}

void sensor_init()
{
    Serial.println("Sensor init begin.");

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

    Serial.println("Sensor init over.");
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

void read_bat()//
{
    bat_vol = 3.3 * analogRead(BAT_PIN) / 4096 * 2 + 0.3;
}

void value_report()
{
    Serial.println("\n-------------------------------------");
    Serial.printf("Num:\t%d\n", count);

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

    Serial.println("\n-------------------------------------");

    // 14:37:57.103 -> Num:	5
    // 14:37:57.103 -> Temperature: 28.98 degrees C
    // 14:37:57.103 -> Humidity: 44.40% rH
    // 14:37:57.103 -> TVOC 0.00 ppb	eCO2 400.00 ppm
    // 14:37:57.103 -> Raw H2 13978.00 	Raw Ethanol 19738.00
    // 14:37:57.103 -> Light: 80.00 lx
    // 14:37:57.103 -> Bat: 4.26 V
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

void http_request()
{
    // Http test
    sendData("AT+HTTPINIT", 2000, DEBUG);
    sendData("AT+HTTPPARA=\"URL\",\"https://api.thingspeak.com/update?api_key=" + Apikey + "&field1=" + (String)temperature + "&field2=" + (String)humidity + "&field3=" + (String)tvoc + "&field4=" + (String)eCO2 + "&field5=" + (String)H2 + "&field6=" + (String)Ethanol + "&field7=" + (String)lux + "&field8=" + (String)bat_vol +"\"\r\n", 2000, DEBUG);
    sendData("AT+HTTPACTION=0", 3000, DEBUG);
    sendData("AT+HTTPTERM", 3000, DEBUG);
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
