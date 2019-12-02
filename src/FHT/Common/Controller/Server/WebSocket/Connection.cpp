/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 29.11.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Connection.h"
#include "User.h"
#include <cstring>

wsConnect::wsConnect(evhttp_connection* conev):
    frame_(std::make_unique<wsFrame>()),
    bev_(evhttp_connection_get_bufferevent(conev)),
    conev_(conev, &evhttp_connection_free){
    wsReqStr_ = "";
    wsRespStr_ = "";
    step_ = wsRequest::step::ZERO;
    ntoread_ = 0;
}
wsConnect::~wsConnect(){
    handshakeHendlerFunction = nullptr;
    wsFrameReceverHendlerFunction = nullptr;
    writeHendlerFunction = nullptr;
    pingHendlerFunction = nullptr;
    closeHendlerFunction = nullptr;
    conev_.reset();
    frame_.reset();
}

void wsConnect::wsServerStart() {
    if (bev_) {
        acceptWebSocketRequest();
    } else {
        wsServerExit();
    }
}

void wsConnect::wsServerExit() {
    if (closeHendlerFunction) {
        closeHendlerFunction();
    }
}

void wsConnect::acceptWebSocketRequest() {
    if (bev_) {
        bufferevent_setcb(bev_, requestReadHendler, responseWriteHendler, closeHendler, this);
        bufferevent_setwatermark(bev_, EV_READ, 1, 1);
        bufferevent_setwatermark(bev_, EV_WRITE, 0, 0);
        bufferevent_enable(bev_, EV_READ);
    } else {
        wsServerExit();
    }
}

void wsConnect::responsedWebSocketRequest() {
    if (bev_) {
        wsRequest ws_req;
        ws_req.parseWebsocketRequest(wsReqStr_.c_str());
        wsRespStr_ = ws_req.generateWebsocketResponse(); //generate response
        if (!wsRespStr_.empty()) {
            bufferevent_write(bev_, wsRespStr_.c_str(), wsRespStr_.length());
        } else {
            wsServerExit();
        }
    } else {
        wsServerExit();
    }
}

void wsConnect::wsFrameReceverLoop() {
    if (bev_) {
        step_ = wsRequest::step::ONE;
        ntoread_ = 2;
        bufferevent_setcb(bev_, wsFrameReadHendler, writeHendler, closeHendler, this);
        bufferevent_setwatermark(bev_, EV_READ, ntoread_, ntoread_);
        bufferevent_setwatermark(bev_, EV_WRITE, 0, 0);
        bufferevent_enable(bev_, EV_READ);
    } else {
        wsServerExit();
    }
}

int32_t wsConnect::writeFrameData(const wsFrameBuffer *fb) {
    return bufferevent_write(bev_, fb->data_, fb->len_) ? 500 : 200;
}

void requestReadHendler(struct bufferevent *bev_, void *ctx) {
    wsConnect *conn = (wsConnect*)ctx;
    if (conn && conn->bev_) {
        char c;
        bufferevent_read(bev_, &c, 1);
        conn->wsReqStr_ += c;
        //size_t n = conn->wsReqStr_.size();
        //if (n >= 4 && conn->wsReqStr_.substr(n - 4) == "\r\n\r\n") {
            bufferevent_disable(conn->bev_, EV_READ); //stop reading before a valid handshake
            conn->responsedWebSocketRequest(); //send websocket response
        //}
    } else {
        conn->wsServerExit();
    }
}

void responseWriteHendler(struct bufferevent *, void *ctx) {
    wsConnect *conn = (wsConnect*)ctx;
    if (conn && conn->bev_) {
        if (conn->handshakeHendlerFunction) {
            conn->handshakeHendlerFunction();
        }
        conn->wsFrameReceverLoop(); //frame receive loop
    } else {
        conn->wsServerExit();
    }
}

void wsFrameReadHendler(struct bufferevent *bev_, void *ctx) {
    wsConnect *conn = (wsConnect*)ctx;
    if (!conn || !conn->bev_) {
        conn->wsServerExit();
        return;
    }
    switch (conn->step_) {
    case wsRequest::step::ONE: {
            std::unique_ptr<char> tmp(new char[conn->ntoread_]);
            bufferevent_read(bev_, tmp.get(), conn->ntoread_);
            //parse header
            if (conn->frame_->parseFrameHeader(tmp.get()) == 0) {
                if (conn->frame_->payload_len_ <= 125) {
                    conn->step_ = wsRequest::step::THREE;
                    conn->ntoread_ = 4;
                    bufferevent_setwatermark(bev_, EV_READ, conn->ntoread_, conn->ntoread_);
                } else if (conn->frame_->payload_len_ == 126) {
                    conn->step_ = wsRequest::step::TWO;
                    conn->ntoread_ = 2;
                    bufferevent_setwatermark(bev_, EV_READ, conn->ntoread_, conn->ntoread_);
                } else if (conn->frame_->payload_len_ == 127) {
                    conn->step_ = wsRequest::step::TWO;
                    conn->ntoread_ = 8;
                    bufferevent_setwatermark(bev_, EV_READ, conn->ntoread_, conn->ntoread_);
                }
            }
            //TODO
            //validate frame header
            if (!conn->frame_->isFrameValid()) {
                return;
            }
            break;
        }
    case wsRequest::step::TWO: {
            std::unique_ptr<char> tmp(new char[conn->ntoread_]);
            bufferevent_read(bev_, tmp.get(), conn->ntoread_);
            if (conn->frame_->payload_len_ == 126) {
                conn->frame_->payload_len_ = ntohs(*(uint16_t*)tmp.get());
            } else if (conn->frame_->payload_len_ == 127) {
                conn->frame_->payload_len_ = myntohll(*(uint64_t*)tmp.get());
            }
            conn->step_ = wsRequest::step::THREE;
            conn->ntoread_ = 4;
            bufferevent_setwatermark(bev_, EV_READ, conn->ntoread_, conn->ntoread_);
            break;
        }
    case wsRequest::step::THREE: {
            std::unique_ptr<char> tmp(new char[conn->ntoread_]);
            bufferevent_read(bev_, tmp.get(), conn->ntoread_);
            memcpy(conn->frame_->masking_key_, tmp.get(), conn->ntoread_);
            if (conn->frame_->payload_len_ > 0) {
                conn->step_ = wsRequest::step::FOUR;
                conn->ntoread_ = conn->frame_->payload_len_;
                bufferevent_setwatermark(bev_, EV_READ, conn->ntoread_, conn->ntoread_);
            } else if (conn->frame_->payload_len_ == 0) {
                /*recv a whole frame*/
                if (conn->frame_->mask_ == 0) {
                    //recv an unmasked frame
                }
                if (conn->frame_->fin_ == 1 && conn->frame_->opcode_ == 0x8) {
                    //0x8 denotes a connection close
                    std::unique_ptr<wsFrameBuffer> fb(new wsFrameBuffer(1, 8, 0, nullptr));
                    conn->writeFrameData(fb.get());
                    break;
                } else if (conn->frame_->fin_ == 1 && conn->frame_->opcode_ == 0x9) {
                    //0x9 denotes a ping
                    //TODO
                    //make a pong
                } else {
                    //execute custom operation
                    if (conn->wsFrameReceverHendlerFunction) {
                        conn->wsFrameReceverHendlerFunction();
                    }
                }
                conn->step_ = wsRequest::step::ONE;
                conn->ntoread_ = 2;
                bufferevent_setwatermark(bev_, EV_READ, conn->ntoread_, conn->ntoread_);
            }
            break;
        }
    case wsRequest::step::FOUR:{
            if (conn->frame_->payload_len_ > 0) {
                if (conn->frame_->payload_data_) {
                    delete[] conn->frame_->payload_data_;
                    conn->frame_->payload_data_ = nullptr;
                }
                conn->frame_->payload_data_ = new char[conn->frame_->payload_len_];
                bufferevent_read(bev_, conn->frame_->payload_data_, conn->frame_->payload_len_);
                conn->frame_->unmaskPayloadData();
            }
            if (conn->frame_->fin_ == 1 && conn->frame_->opcode_ == 0x8) {
                //0x8 denotes a connection close
                std::unique_ptr<wsFrameBuffer> fb(new wsFrameBuffer(1, 8, 0, nullptr));
                conn->writeFrameData(fb.get());
                break;
            } else if (conn->frame_->fin_ == 1 && conn->frame_->opcode_ == 0x9) {
                //0x9 denotes a ping
                //TODO
                //make a pong
            } else {
                //execute custom operation
                if (conn->wsFrameReceverHendlerFunction) {
                    conn->wsFrameReceverHendlerFunction();
                }
            }
            if (conn->frame_->opcode_ == 0x1) { //0x1 denotes a text frame
            }
            if (conn->frame_->opcode_ == 0x2) { //0x1 denotes a binary frame
            }
            conn->step_ = wsRequest::step::ONE;
            conn->ntoread_ = 2;
            bufferevent_setwatermark(bev_, EV_READ, conn->ntoread_, conn->ntoread_);
            break;
        }
    default:
        std::cout << "FATAL STEP" << std::endl;
        break;
    }

}

void wsConnectSetHendler(wsConnect *conn, wsConnect::CBTYPE cbtype, std::function<void()> cb) {
    if (conn) {
        switch (cbtype) {
        case wsConnect::CBTYPE::HANDSHAKE:
            conn->handshakeHendlerFunction = cb;
            break;
        case wsConnect::CBTYPE::FRAME_RECV:
            conn->wsFrameReceverHendlerFunction = cb;
            break;
        case wsConnect::CBTYPE::WRITE:
            conn->writeHendlerFunction = cb;
            break;
        case wsConnect::CBTYPE::CLOSE:
            conn->closeHendlerFunction = cb;
            break;
        case wsConnect::CBTYPE::PING:
            conn->pingHendlerFunction = cb;
            break;
        }
    }
}

void writeHendler(struct bufferevent *bev_, void *ctx) {
    wsConnect *conn = (wsConnect*)ctx;
    if (conn) {
        if (conn->writeHendlerFunction) {
            conn->writeHendlerFunction();
        }
    }
}

void closeHendler(struct bufferevent *bev_, short what, void *ctx) {
    ((wsConnect*)ctx)->wsServerExit();
}
