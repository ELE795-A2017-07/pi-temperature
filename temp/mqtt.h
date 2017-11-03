#ifndef MQTT_H
#define MQTT_H

#define MQTT_CLIENT_ID "dioo-test"
#if 0
const char MQTT_HOST[] = "207.162.8.230";
const int MQTT_PORT = 8080;
#else
const char MQTT_HOST[] = "hexadecimal";
const int MQTT_PORT = 1884;
#endif

const int MQTT_KEEPALIVE = 0;

class Mqtt {
	bool cleansession;
	struct mosquitto *mosq;

	public:
static void init(void);
	Mqtt(string clientId, void *obj, bool cleansession);
	~Mqtt(void);
	int set_credentials(string username, string password);
	int connect(string host, int port, int keepalive);
	int publish(int *msg_id, string topic, int payloadlen, void *payload, int qos, bool retain);
	int disconnect(void);
}

#endif /* Guard */
