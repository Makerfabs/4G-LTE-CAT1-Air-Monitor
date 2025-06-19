/***********************************************************************
  Adafruit MQTT Library ESP32 Adafruit IO SSL/TLS example

  Use the latest version of the ESP32 Arduino Core:
    https://github.com/espressif/arduino-esp32

  Works great with Adafruit Huzzah32 Feather and Breakout Board:
    https://www.adafruit.com/product/3405
    https://www.adafruit.com/products/4172

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  Modified by Brent Rubell for Adafruit Industries
  MIT license, all text above must be included in any redistribution
 **********************************************************************/

// Using library WiFi at version 3.1.1 in folder: C:\Users\miand\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.1.1\libraries\WiFi 
// Using library Networking at version 3.1.1 in folder: C:\Users\miand\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.1.1\libraries\Network 
// Using library NetworkClientSecure at version 3.1.1 in folder: C:\Users\miand\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.1.1\libraries\NetworkClientSecure 
// Using library Adafruit MQTT Library at version 2.5.9 in folder: C:\Users\miand\Documents\Arduino\libraries\Adafruit_MQTT_Library 



#include <WiFi.h>
#include "WiFiClientSecure.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID "Makerfabs"
#define WLAN_PASS "20160704"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "mqtt.recreacloud.com"

// Using port 8883 for MQTTS
#define AIO_SERVERPORT  8885

// Adafruit IO Account Configuration
// (to obtain these values, visit https://io.adafruit.com and click on Active Key)
#define AIO_USERNAME "1966189FC8C6"
#define AIO_KEY      "b1d33f5e3a46"

/************ Global State (you don't need to change this!) ******************/

// WiFiFlientSecure for SSL/TLS support
WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// io.adafruit.com root CA
const char* adafruitio_root_ca2 = \
      "-----BEGIN CERTIFICATE-----\n"
      "MIIEjTCCA3WgAwIBAgIQDQd4KhM/xvmlcpbhMf/ReTANBgkqhkiG9w0BAQsFADBh\n"
      "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
      "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
      "MjAeFw0xNzExMDIxMjIzMzdaFw0yNzExMDIxMjIzMzdaMGAxCzAJBgNVBAYTAlVT\n"
      "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
      "b20xHzAdBgNVBAMTFkdlb1RydXN0IFRMUyBSU0EgQ0EgRzEwggEiMA0GCSqGSIb3\n"
      "DQEBAQUAA4IBDwAwggEKAoIBAQC+F+jsvikKy/65LWEx/TMkCDIuWegh1Ngwvm4Q\n"
      "yISgP7oU5d79eoySG3vOhC3w/3jEMuipoH1fBtp7m0tTpsYbAhch4XA7rfuD6whU\n"
      "gajeErLVxoiWMPkC/DnUvbgi74BJmdBiuGHQSd7LwsuXpTEGG9fYXcbTVN5SATYq\n"
      "DfbexbYxTMwVJWoVb6lrBEgM3gBBqiiAiy800xu1Nq07JdCIQkBsNpFtZbIZhsDS\n"
      "fzlGWP4wEmBQ3O67c+ZXkFr2DcrXBEtHam80Gp2SNhou2U5U7UesDL/xgLK6/0d7\n"
      "6TnEVMSUVJkZ8VeZr+IUIlvoLrtjLbqugb0T3OYXW+CQU0kBAgMBAAGjggFAMIIB\n"
      "PDAdBgNVHQ4EFgQUlE/UXYvkpOKmgP792PkA76O+AlcwHwYDVR0jBBgwFoAUTiJU\n"
      "IBiV5uNu5g/6+rkS7QYXjzkwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsG\n"
      "AQUFBwMBBggrBgEFBQcDAjASBgNVHRMBAf8ECDAGAQH/AgEAMDQGCCsGAQUFBwEB\n"
      "BCgwJjAkBggrBgEFBQcwAYYYaHR0cDovL29jc3AuZGlnaWNlcnQuY29tMEIGA1Ud\n"
      "HwQ7MDkwN6A1oDOGMWh0dHA6Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEds\n"
      "b2JhbFJvb3RHMi5jcmwwPQYDVR0gBDYwNDAyBgRVHSAAMCowKAYIKwYBBQUHAgEW\n"
      "HGh0dHBzOi8vd3d3LmRpZ2ljZXJ0LmNvbS9DUFMwDQYJKoZIhvcNAQELBQADggEB\n"
      "AIIcBDqC6cWpyGUSXAjjAcYwsK4iiGF7KweG97i1RJz1kwZhRoo6orU1JtBYnjzB\n"
      "c4+/sXmnHJk3mlPyL1xuIAt9sMeC7+vreRIF5wFBC0MCN5sbHwhNN1JzKbifNeP5\n"
      "ozpZdQFmkCo+neBiKR6HqIA+LMTMCMMuv2khGGuPHmtDze4GmEGZtYLyF8EQpa5Y\n"
      "jPuV6k2Cr/N3XxFpT3hRpt/3usU/Zb9wfKPtWpoznZ4/44c1p9rzFcZYrWkj3A+7\n"
      "TNBJE0GmP2fhXhP1D/XVfIW/h0yCJGEiV9Glm/uGOa3DXHlmbAcxSyCRraG+ZBkA\n"
      "7h4SeM6Y8l/7MBRpPCz6l8Y=\n"
      "-----END CERTIFICATE-----\n";


// io.adafruit.com root CA
const char* adafruitio_root_ca = \
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


/****************************** Feeds ***************************************/

// Setup a feed called 'test' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish test = Adafruit_MQTT_Publish(&mqtt,"1966189FC8C6/uplink/device_address01");

// Setup a feed called 'testRxt' for subscribing to current time
Adafruit_MQTT_Subscribe testRxt = Adafruit_MQTT_Subscribe(&mqtt, "1966189FC8C6/uplink/device_address01");


void testRxtcallback(char *data, uint16_t len) {
  Serial.print("Hey we're in a testRxt callback, the button value is: ");
  Serial.println(data);
}

/*************************** Sketch Code ************************************/

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit IO MQTTS (SSL/TLS) Example"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  delay(1000);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  delay(2000);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Set Adafruit IO's root CA
  client.setCACert(adafruitio_root_ca);

  testRxt.setCallback(testRxtcallback);
  mqtt.subscribe(&testRxt);

}

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // Now we can publish stuff!
  Serial.print(F("\nSending val "));
  Serial.print(x);
  Serial.print(F(" to test feed..."));
  if (! test.publish(x++)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  
  // wait a couple seconds to avoid rate limit
  mqtt.processPackets(2000);
  //delay(2000);

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }

  Serial.println("MQTT Connected!");
}
