/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 29.11.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef WS_WEBSOCKET_H
#define WS_WEBSOCKET_H
#include "Tools.h"
#include "Frame.h"
#include "openssl/sha.h"
#include "Base64.h"
#include <iostream>
#include <string>
#include <vector>

struct wsRequest {
    std::string generateWebsocketResponse();
    bool parseWebsocketRequest(const char *src);
    std::string generateKey(const std::string &key);
    enum step {
            ZERO,  //before websocket handshake
            ONE,   //0-2 bytes, fin, opcode, mask, payload length
            TWO,   //extended payload length
            THREE, //masking-key
            FOUR,  //payload data
            UNKNOWN
    };
    std::string req_;
    std::string connection_;
    std::string upgrade_;
    std::string host_;
    std::string origin_;
    std::string cookie_;
    std::string secWebsocketKey_;
    std::string secWebsocketVersion_;
};

#endif
