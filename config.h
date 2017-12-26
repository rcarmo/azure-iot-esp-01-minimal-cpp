// Settings
#define WIFI_SSID "Pocket IoT"
#define WIFI_PSK "Squirrels"

// Enable VCC monitoring
ADC_MODE(ADC_VCC);

// "hostname=myiothub.azure-devices.net;DeviceId=esp01sensor;SharedAccessKey=foobar";
String HOSTNAME="myiothub.azure-devices.net";
String DEVICE_ID="esp01sensor";
String SHARED_ACCESS_KEY="foobar";
String SSL_FINGERPRINT=""; // fill this in to validate certificate

IPAddress MULTICAST_ADDRESS(224,0,0,42);
const int MULTICAST_UDP_PORT=4200;
const int DHT_PIN=2; // GPIO2
const int SLEEP_INTERVAL=5;

#define USE_DEEP_SLEEP
#define USE_MULTICAST
#define USE_IOT_HUB
