#include "websocket.h"
#include <map>
#ifdef _WIN32

#define snprintf _snprintf
#define strcasecmp _stricmp 
#else
#include <strings.h>
#endif

int32_t parse_websocket_request(const char *s_req, ws_req_t *ws_req) {
	if (!s_req || !ws_req) {
		return -1;
	}
	std::string str(s_req);
	std::map<std::string, std::string> map;
	std::string key;
	std::string value;
	for (int i = 0; i < str.size(); i++) {
		if (str[i] == '\r' && str[i + 1] == '\n') {
			map.emplace(key, value);
			++i;
			key.clear();
			value.clear();
		} else if(str[i] == ':' && key.empty()){
			if (str[i+1] == ' ') {
				i++;
			}
			key = value;
			value.clear();
		} else {
		value += str[i];
		}
	}
	if (auto a = map.find("Connection"); a != map.end()) {
		ws_req->connection = a->second;
	}
	if (auto a = map.find("Upgrade"); a != map.end()) {
		ws_req->upgrade = a->second;
	}
	if (auto a = map.find("Host"); a != map.end()) {
		ws_req->host = a->second;
	}
	if (auto a = map.find("Origin"); a != map.end()) {
		ws_req->origin = a->second;
	}
	if (auto a = map.find("Cookie"); a != map.end()) {
		ws_req->cookie = a->second;
	}
	if (auto a = map.find("Sec-WebSocket-Key"); a != map.end()) {
		ws_req->sec_websocket_key = a->second;
	}
	if (auto a = map.find("Sec-WebSocket-Version"); a != map.end()) {
		ws_req->sec_websocket_version = a->second;
	}
	return 0;
}


void print_websocket_request(const ws_req_t *ws_req) {
	if (ws_req) {
		if (!ws_req->req.empty()) {
			fprintf(stdout, "%s\r\n", ws_req->req.c_str());
		}
		if (!ws_req->connection.empty()) {
			fprintf(stdout, "Connection: %s\r\n", ws_req->connection.c_str());
		}
		if (!ws_req->upgrade.empty()) {
			fprintf(stdout, "Upgrade: %s\r\n", ws_req->upgrade.c_str());
		}
		if (!ws_req->host.empty()) {
			fprintf(stdout, "Host: %s\r\n", ws_req->host.c_str());
		}
		if (!ws_req->origin.empty()) {
			fprintf(stdout, "Origin: %s\r\n", ws_req->origin.c_str());
		}
		if (!ws_req->cookie.empty()) {
			fprintf(stdout, "Cookie: %s\r\n", ws_req->cookie.c_str());
		}
		if (!ws_req->sec_websocket_key.empty()) {
			fprintf(stdout, "Sec-WebSocket-Key: %s\r\n", ws_req->sec_websocket_key.c_str());
		}
		if (!ws_req->sec_websocket_version.empty()) {
			fprintf(stdout, "Sec-WebSocket-Version: %s\r\n", ws_req->sec_websocket_version.c_str());
		}
		fprintf(stdout, "\r\n");
	}
}


string generate_websocket_response(const ws_req_t *ws_req) {
	string resp;
	if (ws_req) {
		resp += "HTTP/1.1 101 WebSocket Protocol\r\n";
		resp += "Connection: Upgrade\r\n";
		resp += "Upgrade: websocket\r\n";
		resp += "Server: AI Math\r\n";
		resp += "Sec-WebSocket-Accept: " + generate_key(ws_req->sec_websocket_key) + "\r\n";
		resp += "\r\n";
	}
	return resp;
}


string generate_key(const string &key) {
	//sha-1
	string tmp = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	unsigned char md[SHA_DIGEST_LENGTH] = {0};
	SHA1((const unsigned char*)tmp.data(), tmp.size(), md);

	//base64 encode
	string res = base64_encode(md, 20);

	return res;
}


int32_t parse_frame_header(const char *buf, frame_t *frame) {
	if (!buf || !frame) {
		return -1;
	}
	unsigned char c1 = *buf;
	unsigned char c2 = *(buf + 1);
	frame->fin = (c1 >> 7) & 0xff;
	frame->opcode = c1 & 0x0f;
	frame->mask = (c2 >> 7) & 0xff;
	frame->payload_len = c2 & 0x7f;
	return 0;
}


int32_t unmask_payload_data(frame_t *frame) {
	if (frame && frame->payload_data && frame->payload_len > 0) {
		for (int32_t i = 0; i < frame->payload_len; ++i) {
			*(frame->payload_data + i) = *(frame->payload_data + i) ^ *(frame->masking_key + i % 4);
		}
		return 0;
	}
	return -1;
}


int32_t unmask_payload_data(const char *masking_key, char *payload_data, uint32_t payload_len) {
	if (!masking_key || !payload_data || payload_len == 0) {
		return -1;
	}
	for (int32_t i = 0; i < payload_len; ++i) {
		*(payload_data + i) = *(payload_data + i) ^ *(masking_key + i % 4);
	}
	return 0;
}
