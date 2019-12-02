/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 29.11.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Frame.h"
#include <cstring>

wsFrame::wsFrame() {}

wsFrame::~wsFrame() {
    if (payload_data_) {
        delete[] payload_data_;
    }
}

bool wsFrame::isFrameValid() {
    if (fin_ <= 1 && opcode_ <= 0xf && mask_ == 1) {
        return true;
    }
    return false;
}
bool wsFrame::parseFrameHeader(const char *buf) {
    if (!buf) {
        return false;
    }
    unsigned char c1 = *buf;
    unsigned char c2 = *(buf + 1);
    fin_ = (c1 >> 7) & 0xff;
    opcode_ = c1 & 0x0f;
    mask_ = (c2 >> 7) & 0xff;
    payload_len_ = c2 & 0x7f;
    return true;
}

bool wsFrame::unmaskPayloadData() {
    if (payload_data_ && payload_len_ > 0) {
        for (int32_t i = 0; i < payload_len_; ++i) {
            *(payload_data_ + i) = *(payload_data_ + i) ^ *(masking_key_ + i % 4);
        }
        return true;
    }
    return false;
}

wsFrameBuffer::wsFrameBuffer(uint8_t fin_, uint8_t opcode_, uint64_t payload_len_, const char* payload_data_) {
    frameBuffer(fin_, opcode_, payload_len_, payload_data_);
}

wsFrameBuffer::wsFrameBuffer(const wsFrame* frame) {
    if (!frame || frame->fin_ > 1 || frame->opcode_ > 0xf || frame->mask_ > 1) {
        return;
    }
    frameBuffer(frame->fin_, frame->opcode_, frame->payload_len_, frame->payload_data_);
}

void wsFrameBuffer::frameBuffer(uint8_t fin_, uint8_t opcode_, uint64_t payload_len_, const char *payload_data_) {
    if (fin_ > 1 || opcode_ > 0xf) {
        return;
    }

    uint8_t mask_ = 0; //must not mask_ at server endpoint
    char masking_key_[4] = {0}; //no need at server endpoint

    char *p = nullptr; //buffer
    uint64_t len = 0; //buffer length

    unsigned char c1 = 0x00;
    unsigned char c2 = 0x00;
    c1 = c1 | (fin_ << 7); //set fin_
    c1 = c1 | opcode_; //set opcode_
    c2 = c2 | (mask_ << 7); //set mask_

    if (!payload_data_ || payload_len_ == 0) {
        if (mask_ == 0) {
            p = new char[2];
            *p = c1;
            *(p + 1) = c2;
            len = 2;
        } else {
            p = new char[2 + 4];
            *p = c1;
            *(p + 1) = c2;
            memcpy(p + 2, masking_key_, 4);
            len = 2 + 4;
        }
    } else if (payload_data_ && payload_len_ <= 125) {
        if (mask_ == 0) {
            p = new char[2 + payload_len_];
            *p = c1;
            *(p + 1) = c2 + (char)payload_len_;
            memcpy(p + 2, payload_data_, payload_len_);
            len = 2 + payload_len_;
        } else {
            p = new char[2 + 4 + payload_len_];
            *p = c1;
            *(p + 1) = c2 + (char)payload_len_;
            memcpy(p + 2, masking_key_, 4);
            memcpy(p + 6, payload_data_, payload_len_);
            len = 2 + 4 + payload_len_;
        }
    } else if (payload_data_ && payload_len_ >= 126 && payload_len_ <= 65535) {
        if (mask_ == 0) {
            p = new char[4 + payload_len_];
            *p = c1;
            *(p + 1) = c2 + 126;
            uint16_t tmplen = myhtons((uint16_t)payload_len_);
            memcpy(p + 2, &tmplen, 2);
            memcpy(p + 4, payload_data_, payload_len_);
            len = 4 + payload_len_;
        } else {
            p = new char[4 + 4 + payload_len_];
            *p = c1;
            *(p + 1) = c2 + 126;
            uint16_t tmplen = myhtons((uint16_t)payload_len_);
            memcpy(p + 2, &tmplen, 2);
            memcpy(p + 4, masking_key_, 4);
            memcpy(p + 8, payload_data_, payload_len_);
            len = 4 + 4 + payload_len_;
        }
    } else if (payload_data_ && payload_len_ >= 65536) {
        if (mask_ == 0) {
            p = new char[10 + payload_len_];
            *p = c1;
            *(p + 1) = c2 + 127;
            uint64_t tmplen = myhtonll(payload_len_);
            memcpy(p + 2, &tmplen, 8);
            memcpy(p + 10, payload_data_, payload_len_);
            len = 10 + payload_len_;
        } else {
            p = new char[10 + 4 + payload_len_];
            *p = c1;
            *(p + 1) = c2 + 127;
            uint64_t tmplen = myhtonll(payload_len_);
            memcpy(p + 2, &tmplen, 8);
            memcpy(p + 10, masking_key_, 4);
            memcpy(p + 14, payload_data_, payload_len_);
            len = 10 + 4 + payload_len_;
        }
    }

    if (p && len > 0) {
        data_ = p;
        len_ = len;
    }
}

wsFrameBuffer::~wsFrameBuffer(){
    if (data_) {
        delete[] data_;
    }
}
