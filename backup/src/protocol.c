// protocol.c
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "protocol.h"
#include "rawsocket.h"

uint8_t calculate_crc(const Frame* frame) {
    uint8_t crc = 0;
    const uint8_t* data = (const uint8_t*)frame;
    for (size_t i = 0; i < sizeof(Frame) - 1; i++) {
        crc ^= data[i];
    }
    return crc;
}

Frame create_frame(uint8_t size, uint8_t sequence, MessageType type, const uint8_t* data) {
    Frame frame = {0};
    frame.init_marker = INIT_MARKER;
    frame.size = size & 0x3F;  // Ensure size is 6 bits
    frame.sequence = sequence & 0x1F;  // Ensure sequence is 5 bits
    frame.type = type & 0x1F;  // Ensure type is 5 bits
    
    if (data && size > 0) {
        memcpy(frame.data, data, size);
    }
    
    frame.crc = calculate_crc(&frame);
    return frame;
}

int send_frame(int socket, const Frame* frame) {
    return send(socket, frame, sizeof(Frame), 0);
}

int receive_frame(int socket, Frame* frame) {
    int received = recv(socket, frame, sizeof(Frame), 0);
    if (received == sizeof(Frame)) {
        uint8_t calculated_crc = calculate_crc(frame);
        if (calculated_crc != frame->crc) {
            fprintf(stderr, "CRC mismatch\n");
            return -1;
        }
        return 0;
    }
    return -1;
}