/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 29.11.19
*  Copyright (C) goganoga 2019
***************************************/
#include "User.h"

wsUser::wsUser(evhttp_connection *conev):wsConn_(std::make_shared<wsConnect>(conev)){
    id_ = 0;
    msg_ = "";
}
wsUser::~wsUser(){
    readBind_ = nullptr;
    if (wsConn_) {
        wsConn_.reset();
    }
    if (closeBind_)
        closeBind_();
    closeBind_ = nullptr;
}

void wsUser::frameRead() {
    if (wsConn_->frame_->payload_len_ > 0) {
        msg_ += std::string(wsConn_->frame_->payload_data_, wsConn_->frame_->payload_len_);
	}
    if (wsConn_->frame_->fin_ == 1) {
        if (readBind_)
            readBind_(msg_);
        msg_ = "";
	}
}
