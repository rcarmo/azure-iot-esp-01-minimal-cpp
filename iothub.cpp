//
// IoTHub wrapper
//
// Rui Carmo
// https://github.com/rcarmo
//

#include "iothub.h"
#include <WiFiClientSecure.h> // TLS sockets
#include "base64.h"
#include "sha256.h"


// Perform "safe" URL escaping, like the Golang and Python defaults
String urlEscape(String msg) {
    String safe = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.-~";
    String hex = "0123456789ABCDEF";
    String escapedMsg = "";

    for(int i=0; i < msg.length(); i++) {
        if(safe.indexOf(msg[i]) >= 0) {
          escapedMsg += msg[i];
        } else {
            escapedMsg += '%';
            escapedMsg += hex[(msg[i] & 0xF0) >> 4];
            escapedMsg += hex[msg[i] & 0x0F];
        }
    }
    escapedMsg.replace(' ', '+');
    return escapedMsg;
} // urlEscape


IoTHub::IoTHub(String host, String fingerPrint, String accessKey) {
    hostName = host;
    hostFingerPrint = fingerPrint;
    sharedAccessKey = accessKey;
} // IoTHub


String IoTHub::sharedAccessSignature(String uri, time_t epoch) {    
  time_t sasExpiryTime = epoch + IOTHUB_SAS_TOKEN_TTL;

  String stringToSign = urlEscape(uri.c_str()) + "\n" + sasExpiryTime;

  // convert the sharedAccessKey back into its component bits
  int keyLength = sharedAccessKey.length();
  int decodedKeyLength = base64_dec_len((char*)sharedAccessKey.c_str(), keyLength);
  char decodedKey[decodedKeyLength];
  base64_decode(decodedKey, (char*)sharedAccessKey.c_str(), keyLength);

  // compute an HMAC of the URI + timestamp using the key
  Sha256.initHmac((uint8_t*)decodedKey, decodedKeyLength);
  Sha256.print(stringToSign);
  char* sign = (char*) Sha256.resultHmac(); // this is statically allocated, so no leakage
  
  // base64 encode the HMAC
  int encodedSignLen = base64_enc_len(HASH_LENGTH);
  char encodedSign[encodedSignLen];
  base64_encode(encodedSign, sign, HASH_LENGTH); 
  
  return "sr=" + urlEscape(uri.c_str()) + "&sig="+ urlEscape(encodedSign) + "&se=" + sasExpiryTime;
} // sharedAccessSignature


void IoTHub::sendMessage(String deviceId, String payload, time_t epoch) {

    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
    Serial.print("Connecting to ");
    Serial.println(hostName);
    if (!client.connect(hostName.c_str(), 443)) {
        Serial.println("Connection failed.");
        return;
    }

    // Certificate validation
    if(hostFingerPrint.length()) {
        if (client.verify(hostFingerPrint.c_str(), hostName.c_str())) {
            Serial.println("Certificate matches.");
        } else {
            Serial.println("Certificate doesn't match, aborting.");
            return;
        }
    }

    String uri = "/devices/" + deviceId + "/messages/events?api-version=" + IOTHUB_API_VERSION;

    client.print(String("POST ") + uri + " HTTP/1.1\r\n" +
                "Host: " + hostName + "\r\n" +
                "Content-Type: application/json\r\n" +
                "Authorization: SharedAccessSignature " + sharedAccessSignature(hostName + uri, epoch) + "\r\n" +
                "User-Agent: ESP-01-Minimal-IoTHub-Client\r\n" +
                "Content-Length: " + String(payload.length()) + "\r\n" +
                "Connection: close\r\n\r\n" +
                payload);

#ifdef DEBUG
    Serial.println("request sent");
    Serial.println("==========");
#endif
    // read the entire reply
    while (client.connected()) {
        String line = client.readStringUntil('\n');
#ifdef DEBUG
        Serial.println(line);
#endif
    }
#ifdef DEBUG
    Serial.println("==========");
    Serial.println("closing connection");
#endif
} // sendMessage

