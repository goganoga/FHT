/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 29.11.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTWSFRAME_H
#define FHTWSFRAME_H
#include <iostream>
#include "Tools.h"

struct wsFrame {
    wsFrame();
    ~wsFrame();
    bool isFrameValid();
    bool parseFrameHeader(const char *buf);
    bool unmaskPayloadData();
    uint8_t fin_ = 0;
    uint8_t opcode_ = 0;
    uint8_t mask_ = 0;
    uint64_t payload_len_ = 0;
    unsigned char masking_key_[4] = { 0 };
    char *payload_data_ = nullptr;

};
struct wsFrameBuffer{
    wsFrameBuffer(uint8_t fin, uint8_t opcode, uint64_t payload_len, const char *payload_data);
    wsFrameBuffer(const wsFrame *frame);
    ~wsFrameBuffer();
    void frameBuffer(uint8_t fin, uint8_t opcode, uint64_t payload_len, const char* payload_data);
    char *data_ = nullptr;
    uint64_t len_ = 0;
};
#endif //FHTWSFRAME_H
