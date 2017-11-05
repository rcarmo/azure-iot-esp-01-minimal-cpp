//
// IoTHub wrapper
//
// Rui Carmo
// https://github.com/rcarmo
//

#include <Arduino.h>

#define IOTHUB_SAS_TOKEN_TTL 3600
#define IOTHUB_API_VERSION "2016-11-14"

class IoTHub {
    public:
        IoTHub(String host, String fingerPrint, String accessKey);
        void sendMessage(String deviceId, String payload, time_t epoch);

        String hostName;
        String hostFingerPrint;
        String sharedAccessKey;

    private:
        String sharedAccessSignature(String uri, time_t epoch);
};
