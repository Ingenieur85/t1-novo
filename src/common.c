#include <string.h>
#include <stdio.h>
#include "common.h"

static uint8_t crc8_table[256];

void init_crc8_table(void) {
    for (int i = 0; i < 256; i++) {
        uint8_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            } else {
                crc <<= 1;
            }
        }
        crc8_table[i] = crc;
    }
}

void create_frame(Frame *frame, uint8_t size, uint8_t sequence, uint8_t type, const uint8_t *data) {
    frame->start_marker = START_MARKER;
    frame->size_seq = (size << 2) | (sequence >> 3);
    frame->seq_type = ((sequence & 0x07) << 5) | type;
    
    if (data && size > 0) {
        memcpy(frame->data, data, size);
    }
    
    frame->crc = calculate_crc(frame);
}

uint8_t calculate_crc(const Frame *frame) {
    uint8_t crc = 0;
    const uint8_t *data = (const uint8_t *)frame;
    
    for (int i = 0; i < FRAME_HEADER_SIZE + (frame->size_seq >> 2); i++) {
        crc = crc8_table[crc ^ data[i]];
    }
    
    return crc;
}

int send_frame(int socket, const Frame *frame) {
    struct sockaddr_ll dest_addr = {0};
    dest_addr.sll_family = AF_PACKET;
    dest_addr.sll_protocol = htons(ETH_P_ALL);
    dest_addr.sll_ifindex = if_nametoindex(INTERFACE);

    size_t frame_size = FRAME_HEADER_SIZE + (frame->size_seq >> 2) + 1; // +1 for CRC
    ssize_t bytes_sent = sendto(socket, frame, frame_size, 0,
                                (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    if (bytes_sent == -1) {
        perror("Error sending frame");
        return -1;
    }

    return 0;
}

int recv_frame(int socket, Frame *frame) {
    ssize_t bytes_received = recv(socket, frame, sizeof(Frame), 0);

    if (bytes_received == -1) {
        perror("Error receiving frame");
        return -1;
    }

    if (frame->start_marker != START_MARKER) {
        fprintf(stderr, "Invalid start marker\n");
        return -1;
    }

    uint8_t received_crc = frame->crc;
    frame->crc = 0;
    uint8_t calculated_crc = calculate_crc(frame);

    if (received_crc != calculated_crc) {
        fprintf(stderr, "CRC mismatch\n");
        return -1;
    }

    return 0;
}

void print_frame(const Frame *frame) {
    printf("Frame:\n");
    printf("  Start Marker: 0x%02X\n", frame->start_marker);
    printf("  Size: %d\n", frame->size_seq >> 2);
    printf("  Sequence: %d\n", ((frame->size_seq & 0x03) << 3) | (frame->seq_type >> 5));
    printf("  Type: %d\n", frame->seq_type & 0x1F);
    printf("  Data: ");
    for (int i = 0; i < (frame->size_seq >> 2); i++) {
        printf("%02X ", frame->data[i]);
    }
    printf("\n");
    printf("  CRC: 0x%02X\n", frame->crc);
}