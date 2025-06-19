#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHT10.h>
#include <Adafruit_SGP30.h>
#include <BH1750.h>

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

#define DBG_OUT  USBSerial //Serial
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






/************************* MQTT Broker Setup *********************************/

#define MQTT_SERVER      "mqtt.recreacloud.com"

// Using port 8885 for MQTTS
#define MQTT_SERVERPORT  8885

//Account Configuration
//
#define MQTT_USERNAME "1966189FC8C6"
#define MQTT_PASSWORD "b1d33f5e3a46"

/************ Global State (you don't need to change this!) ******************/

// mqtt root CA
const char* mqtt_root_ca = \
"-----BEGIN CERTIFICATE-----\n"
"MIIEGTCCAwGgAwIBAgIUb5JLVp76y4iP//JVD6qq+O7jagQwDQYJKoZIhvcNAQEL\n"
"BQAwgZoxCzAJBgNVBAYTAkVTMQ8wDQYDVQQIDAZNYWRyaWQxDzANBgNVBAcMBk1h\n"
"ZHJpZDEVMBMGA1UECgwMQ2xvdWQgU3R1ZGlvMRYwFAYDVQQLDA1JVCBEZXBhcnRt\n"
"ZW50MRUwEwYDVQQDDAxDbG91ZCBTdHVkaW8xIzAhBgkqhkiG9w0BCQEWFHN1cHBv\n"
"cnRAY2xvdWQuc3R1ZGlvMCAXDTI0MDUyNzEwMzQwMFoYDzIwNTQwNTIwMTAzNDAw\n"
"WjCBmjELMAkGA1UEBhMCRVMxDzANBgNVBAgMBk1hZHJpZDEPMA0GA1UEBwwGTWFk\n"
"cmlkMRUwEwYDVQQKDAxDbG91ZCBTdHVkaW8xFjAUBgNVBAsMDUlUIERlcGFydG1l\n"
"bnQxFTATBgNVBAMMDENsb3VkIFN0dWRpbzEjMCEGCSqGSIb3DQEJARYUc3VwcG9y\n"
"dEBjbG91ZC5zdHVkaW8wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDk\n"
"sTG9c83aQ7eGkItvhNkTjHjnuKRigJNIh6u8daIdA6OIAZFrj7i2ViooH+k275vW\n"
"6Cvhv1ditZ/hqyg3GJXv6GPF0+7kHf3twD7NOL36JxAISnVEa1bS81E1bGTxpRyK\n"
"YeJm1ZDlKx49DpHR6LDF9c5TVne4Y5Ns0kT7DJmsb1Lyrndr7jPynQNFKIhgE8OR\n"
"S0cmnWDqCvYO/goCoJGuE/tq6QjZyRGPa2zONODAvb17iHfNWCDag0zZd6SblnzA\n"
"jNRkO+UQzoCo2jRltc2fk4Tkh7bCDPv/BxoBUXiOD/rqAVib6huGCjTKVImKFbD2\n"
"3rruRslpTqHTObbSid7TAgMBAAGjUzBRMB0GA1UdDgQWBBSEc0QkyrJ/tlAq+ers\n"
"7miwe4dXvTAfBgNVHSMEGDAWgBSEc0QkyrJ/tlAq+ers7miwe4dXvTAPBgNVHRMB\n"
"Af8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQAkbZfbmkNP6w4lbi7fkPOOYdii\n"
"muTz/1tEzLhhU+Nb0TcQMW4krIjaLRM0T6DZx4HirRhYS+lh9WZg1AtP5qES6jXF\n"
"6TLBzBv4H5ejT9zJ7tuGich2L+gC4z3yQJ9ZRaHmjWBIwmfskgJ+911GT2C7eQkP\n"
"oj7J6kb6nqrnIkGPxHCXRJraHzHVGjwHAjSOGNNBnL4z2gBhhJuDSe9jqnSYfoB8\n"
"tTwyNgwDt0zag6M9ovpElpc6gkJqKyXKJ7lZjjGixmojeRCIlRJY/vVFZKBT9qG+\n"
"dYt5AVWXsAhOBQOjLbEekeeLWzKa8es9t/yOZx37K2yQSxyNzpU6PoL3gLzp\n"
"-----END CERTIFICATE-----\n";


void setup()
{

    DBG_OUT.begin(115200);
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
                DBG_OUT.println("Button press down.");
                break;
            }
        }
        else
        {
            sensor_read();
            value_report();
            delay(1000);
        }
    }

    at_init();
    // http_request();

    mqtts_request();
}

void loop()
{
    // put your main code here, to run repeatedly:
    while (DBG_OUT.available() > 0)
    {
        mySerial2.write(DBG_OUT.read());
        yield();
    }
    while (mySerial2.available() > 0)
    {
        DBG_OUT.write(mySerial2.read());
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
        DBG_OUT.println("SGP30 not found.");
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

void read_bat()
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

void at_init()
{
    DBG_OUT.println(F("void at_init()"));

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
    sendData("AT+HTTPINIT", 1000, DEBUG);
    // sendData("AT+HTTPPARA=\"URL\",\"http://www.baidu.com\"", 1000, DEBUG);
    sendData("AT+HTTPPARA=\"URL\",\"http://api.thingspeak.com/update?api_key=5WOQ0JZURMWBTRF4&field1=123&field2=456\"", 1000, DEBUG);
    sendData("AT+HTTPACTION=0", 1000, DEBUG);
    sendData("AT+CSQ", 1000, DEBUG);
    sendData("AT+HTTPHEAD", 1000, DEBUG);
    sendData("AT+HTTPREAD=0,500", 1000, DEBUG);
    sendData("AT+HTTPTERM", 1000, DEBUG);
}

void mqtts_request()
{
    //MQTTS test
    sendData("AT+CSQ", 1000, DEBUG);
    sendData("AT+CPSI?", 1000, DEBUG);
    

    sendData("AT+CCERTLIST", 1000, DEBUG);
    sendData("AT+CSSLCFG=\"sslversion\",0,4", 1000, DEBUG);
    

    sendData("AT+CSSLCFG=\"authmode\",0,1", 1000, DEBUG);
    sendData("AT+CSSLCFG=\"cacert\",0,\"mqtt_cert82.pem\"", 1000, DEBUG);
    sendData("AT+CMQTTSTART", 1000, DEBUG);

    sendData("AT+CMQTTACCQ=0,\"1966189FC8C6\",1", 1000, DEBUG);
    sendData("AT+CMQTTSSLCFG=0,0", 1000, DEBUG);
    sendData("AT+CMQTTWILLTOPIC=0,27", 1000, DEBUG);
    sendData("1966189FC8C6/uplink/willmsg", 1000, DEBUG);
    sendData("AT+CMQTTWILLMSG=0,17,1", 1000, DEBUG);
    sendData("{\"msg\":\"Hello06\"}", 1000, DEBUG);
    sendData("AT+CMQTTCONNECT=0,\"tcp://mqtt.recreacloud.com:8885\",60,1,1966189FC8C6,b1d33f5e3a46", 1000, DEBUG);
    delay(10000);
    sendData("AT+CMQTTTOPIC=0,36", 1000, DEBUG);
    sendData("1966189FC8C6/uplink/device_address01", 1000, DEBUG);
    sendData("AT+CMQTTPAYLOAD=0,17", 1000, DEBUG);
    sendData("{\"msg\":\"maker06\"}", 1000, DEBUG);
    sendData("AT+CMQTTPUB=0,1,60", 1000, DEBUG);
    //delay(10000);
    sendData("AT+CMQTTSUB=0,36,1", 1000, DEBUG);

    sendData("1966189FC8C6/uplink/device_address01", 1000, DEBUG);

    //sendData("AT+CMQTTDISC=0,120", 1000, DEBUG);
    //sendData("AT+CMQTTREL=0", 1000, DEBUG);
    //sendData("AT+CMQTTSTOP", 1000, DEBUG);

    sendData("AT+CSQ", 1000, DEBUG);

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
        DBG_OUT.print(response);
    }
    return response;
}
