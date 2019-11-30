/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 29.11.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTWSUSER_H
#define FHTWSUSER_H
#include <event2/http.h>
#include "Connection.h"
#include <functional>

struct wsUser {
    ~wsUser();
    wsUser(evhttp_connection *conev);
    void frameRead();
    uint32_t id_;
    std::shared_ptr<wsConnect> wsConn_;
    std::string msg_;
    std::function<void(void)> closeBind_;
    std::function<void(std::string)> readBind_;
};

#endif //FHTWSUSER_H
