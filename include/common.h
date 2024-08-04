#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>

#define INTERFACE "lo"
#define START_MARKER 0x7E
#define MAX_DATA_SIZE 63
#define FRAME_HEADER_SIZE 3  // 1 byte for start marker, 2 bytes for size+sequence+type
#define CRC8_POLYNOMIAL 0x07

typedef struct {
    uint8_t start_marker;
    uint8_t size_seq;  // 6 bits for size, 2 bits for part of sequence
    uint8_t seq_type;  // 3 bits for rest of sequence, 5 bits for type
    uint8_t data[MAX_DATA_SIZE];
    uint8_t crc;
} __attribute__((packed)) Frame;

enum MessageType {
    ACK = 0,
    NACK = 1,
    LIST = 10,
    DOWNLOAD = 11,
    DISPLAY = 16,
    FILE_DESCRIPTOR = 17,
    DATA = 18,
    END_TX = 30,
    ERROR = 31
};

// Function prototypes
void init_crc8_table(void);
void create_frame(Frame *frame, uint8_t size, uint8_t sequence, uint8_t type, const uint8_t *data);
uint8_t calculate_crc(const uint8_t *data, size_t len);
int send_frame(int socket, const Frame *frame);
int recv_frame(int socket, Frame *frame);
void print_frame(const Frame *frame);

#endif // COMMON_H