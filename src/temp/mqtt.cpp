#include <mosquitto.h>

#include "mqtt.h"

void Mqtt::init(void) {
	mosquitto_lib_init();
}

Mqtt::Mqtt(std::string clientId, void *obj, bool cleansession) {
	this->cleansession = cleansession;
	this->clientId = clientId;

	#if LIBMOSQUITTO_VERSION_NUMBER <= 15000
		this->mosq = mosquitto_new(clientId.c_str(), obj);
	#elif LIBMOSQUITTO_VERSION_NUMBER <= 1004010
		this->mosq = mosquitto_new(clientId.c_str(), cleansession, obj);
	#else
	#error mosquitto_new not implemented for this libmosquitto version
	#endif
}

Mqtt::~Mqtt(void) {
	this->disconnect();
	mosquitto_destroy(this->mosq);
}

std::string Mqtt::get_clientid(void) {
	return this->clientId;
}

int Mqtt::set_credentials(std::string username, std::string password) {
	int ret;

	ret = mosquitto_username_pw_set(this->mosq, username.c_str(), password.c_str());

	return ret;
}

int Mqtt::connect(std::string host, int port, int keepalive) {
	int ret;

	#if LIBMOSQUITTO_VERSION_NUMBER <= 15000
		ret = mosquitto_connect(this->mosq, host.c_str(), port, keepalive, this->cleansession);
	#elif LIBMOSQUITTO_VERSION_NUMBER <= 1004010
		ret = mosquitto_connect(this->mosq, host.c_str(), port, keepalive);
	#else
	#error mosquitto_connect not implemented for this libmosquitto version
	#endif

	return ret;
}

int Mqtt::publish(int32_t *msg_id, std::string topic, int payloadlen, uint8_t *payload, int qos, bool retain) {
	int ret;
	
	#if LIBMOSQUITTO_VERSION_NUMBER <= 15000
		{
			uint16_t mid;
			ret = mosquitto_publish(this->mosq, &mid, topic.c_str(), payloadlen, payload, qos, retain);
			if (msg_id != nullptr) {
				*msg_id = int32_t(mid);
			}
		}
	#elif LIBMOSQUITTO_VERSION_NUMBER <= 1004010
		ret = mosquitto_publish(this->mosq, msg_id, topic.c_str(), payloadlen, payload, qos, retain);
	#else
	#error mosquitto_publish not implemented for this libmosquitto version
	#endif

	return ret;
}

int Mqtt::disconnect(void) {
	int ret;
	
	ret = mosquitto_disconnect(this->mosq);

	return ret;
}

#if 0
	#if LIBMOSQUITTO_VERSION_NUMBER <= 15000
	#elif LIBMOSQUITTO_VERSION_NUMBER <= 1004010
	#else
	#error X not implemented for this libmosquitto version
	#endif
#endif
