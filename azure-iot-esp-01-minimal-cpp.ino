//
// Minimal Azure IoT Hub client for the ESP-01
//
// Rui Carmo
// https://github.com/rcarmo
//

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "config.h"
#include "iothub.h"
#include "dht.h"

void setupNetwork() {
    Serial.begin(115200);
    Serial.println();
    Serial.print("connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
} // setupNetwork


time_t ntpTime() {
    time_t epoch;
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    delay(500);

    while (true) {
        epoch = time(NULL);

        if (epoch == 0) {
            Serial.println("NTP timed out. Waiting 2 seconds to retry.");
            delay(2000);
        } else {
            Serial.print("Clock set via NTP. Epoch: ");
            Serial.println(epoch);
            break;
        }
    }
    return epoch;
} // ntpTime


String buildPayload() {
    DHT dht;
    dht.setup(DHT_PIN);

    delay(dht.getMinimumSamplingPeriod());
    String measurements = "{\"device\":\""    + WiFi.macAddress() +
                        "\",\"vcc\":"         + String(ESP.getVcc()/1000.0) +
                          ",\"temperature\":" + String(dht.getTemperature()) +
                          ",\"humidity\":"    + String(dht.getHumidity()) + "}";
    return measurements;
} // buildPayload


void setup() {
    setupNetwork();

#ifdef USE_DEEP_SLEEP
    loop();
    ESP.deepSleep(1000000 * SLEEP_INTERVAL, WAKE_RF_DEFAULT);
#endif
} // setup


void loop() {
    WiFiUDP Udp;

#ifdef USE_IOT_HUB
    time_t epoch = ntpTime();
        
    IoTHub client(HOSTNAME, SSL_FINGERPRINT, SHARED_ACCESS_KEY);
#endif
    String payload = buildPayload();
    Serial.println(payload);

#ifdef USE_MULTICAST
    // Send payload as multicast packet for local monitoring
    Udp.beginPacketMulticast(MULTICAST_ADDRESS, MULTICAST_UDP_PORT, WiFi.localIP());
    Udp.write(payload.c_str());
    Udp.endPacket();
#endif

#ifdef USE_IOT_HUB
    // Send to IoTHub
    client.sendMessage(DEVICE_ID, payload, epoch);
#endif
#ifndef USE_DEEP_SLEEP
    delay(1000 * SLEEP_INTERVAL);
#endif
} // loop
