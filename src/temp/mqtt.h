#ifndef MQTT_H
#define MQTT_H

#include <string>

class Mqtt {
	bool cleansession;
	struct mosquitto *mosq;
	string clientId;

	public:
		static void init(void);
		Mqtt(std::string clientId, void *obj, bool cleansession);
		~Mqtt(void);
		string get_clientid(void);
		int set_credentials(std::string username, std::string password);
		int connect(std::string host, int port, int keepalive);
		int publish(int *msg_id, std::string topic, int payloadlen, uint8_t *payload, int qos, bool retain);
		int disconnect(void);
};

#endif /* Guard */
