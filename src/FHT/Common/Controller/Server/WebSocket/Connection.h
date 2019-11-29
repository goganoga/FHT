/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 29.11.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef WS_CONNECTION_H
#define WS_CONNECTION_H
#include <stdlib.h>
#include <iostream>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/bufferevent.h>
#include "Websocket.h"
#include <functional>

struct wsConnect{
    wsConnect(evhttp_connection* conev);
    ~wsConnect();
    void acceptWebSocketRequest();
    void responsedWebSocketRequest();
    void wsServerExit();
    void wsFrameReceverLoop();
    void wsServerStart();
    int32_t writeFrameData(const wsFrameBuffer *fb);

    enum CBTYPE {
            HANDSHAKE,
            FRAME_RECV,
            WRITE,
            CLOSE,
            PING
    };
    wsRequest::step step_;
    std::string wsReqStr_;
    std::string wsRespStr_;
    uint64_t ntoread_;
    std::function<void()> handshakeHendlerFunction;
    std::function<void()> wsFrameReceverHendlerFunction;
    std::function<void()> writeHendlerFunction;
    std::function<void()> closeHendlerFunction;
    std::function<void()> pingHendlerFunction;
    std::unique_ptr<wsFrame> frame_ = nullptr; //current frame
    bufferevent* bev_ = nullptr;
    std::unique_ptr<evhttp_connection, decltype(&evhttp_connection_free)> conev_;
};

void wsConnectSetHendler(wsConnect *conn, wsConnect::CBTYPE cbtype, std::function<void()> cb);
void requestReadHendler(struct bufferevent *bev, void *ctx);
void responseWriteHendler(struct bufferevent *, void *ctx);
void wsFrameReadHendler(struct bufferevent *bev, void *ctx);
void writeHendler(struct bufferevent *bev, void *ctx);
void closeHendler(struct bufferevent *bev, short what, void *ctx);


#endif
