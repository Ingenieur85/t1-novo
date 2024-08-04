// protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define INTERFACE_NAME "lo"
#define INIT_MARKER 0x7E
#define MAX_DATA_SIZE 63

typedef struct __attribute__((packed)) {
    uint8_t init_marker;
    uint8_t size : 6;
    uint8_t sequence : 5;
    uint8_t type : 5;
    uint8_t data[MAX_DATA_SIZE];
    uint8_t crc;
} Frame;

typedef enum {
    ACK = 0,
    NACK = 1,
    LIST = 10,
    DOWNLOAD = 11,
    DISPLAY = 16,
    FILE_DESCRIPTOR = 17,
    DATA = 18,
    END_TX = 30,
    ERROR = 31
} MessageType;

Frame create_frame(uint8_t size, uint8_t sequence, MessageType type, const uint8_t* data);
uint8_t calculate_crc(const Frame* frame);
int send_frame(int socket, const Frame* frame);
int receive_frame(int socket, Frame* frame);

#endif