
const char *ssid = "";
const char *password = "";

// Find this awsEndpoint in the AWS Console: Manage - Things, choose your thing
// choose Interact, its the HTTPS Rest endpoint
const char *awsEndpoint = "";
const char *thingName = "";
const char *thingMqttTopicIn = "";
const char *thingMqttTopicOut = "";

// AWS device certificate and private key:
// xxxxxxxxxx-certificate.pem.crt
static const char deviceCertificatePemCrt[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

// xxxxxxxxxx-private.pem.key
static const char devicePrivatePemKey[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)EOF";

// This is the AWS IoT CA Certificate from:
// https://docs.aws.amazon.com/iot/latest/developerguide/managing-device-certs.html#server-authentication
// This one in here is the 'RSA 2048 bit key: Amazon Root CA 1' which is valid
// until January 16, 2038 so unless it gets revoked you can leave this as is:
static const char awsCaPemCrt[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";
