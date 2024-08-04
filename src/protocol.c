// protocol.c
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
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

int send_frame_with_timeout(int socket, const Frame* frame, int timeout_sec) {
    fd_set write_fds;
    struct timeval timeout;
    
    FD_ZERO(&write_fds);
    FD_SET(socket, &write_fds);
    
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;
    
    int result = select(socket + 1, NULL, &write_fds, NULL, &timeout);
    if (result > 0) {
        return send(socket, frame, sizeof(Frame), 0);
    } else if (result == 0) {
        return -1; // Timeout
    } else {
        return -2; // Error
    }
}

int receive_frame_with_timeout(int socket, Frame* frame, int timeout_sec) {
    fd_set read_fds;
    struct timeval timeout;
    
    FD_ZERO(&read_fds);
    FD_SET(socket, &read_fds);
    
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;
    
    int result = select(socket + 1, &read_fds, NULL, NULL, &timeout);
    if (result > 0) {
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
    } else if (result == 0) {
        return -1; // Timeout
    } else {
        return -2; // Error
    }
}

int send_and_wait_ack(int socket, const Frame* frame, int max_retries) {
    for (int retry = 0; retry < max_retries; retry++) {
        if (send_frame_with_timeout(socket, frame, TIMEOUT_SEC) < 0) {
            fprintf(stderr, "Failed to send frame, retrying...\n");
            continue;
        }
        
        Frame ack_frame;
        if (receive_frame_with_timeout(socket, &ack_frame, TIMEOUT_SEC) == 0) {
            if (ack_frame.type == ACK && ack_frame.sequence == frame->sequence) {
                return 0; // Success
            }
        }
        
        fprintf(stderr, "No ACK received, retrying...\n");
    }
    
    return -1; // Max retries reached
}

int send_data_with_sliding_window(int socket, const uint8_t* data, size_t data_size, uint8_t* sequence) {
    size_t window_start = 0;
    size_t window_end = 0;
    size_t next_to_send = 0;
    
    while (window_start < data_size) {
        // Send frames within the window
        while (next_to_send < data_size && next_to_send < window_start + WINDOW_SIZE) {
            size_t chunk_size = (data_size - next_to_send < MAX_DATA_SIZE) ? data_size - next_to_send : MAX_DATA_SIZE;
            Frame data_frame = create_frame(chunk_size, *sequence, DATA, &data[next_to_send]);
            if (send_frame_with_timeout(socket, &data_frame, TIMEOUT_SEC) < 0) {
                fprintf(stderr, "Failed to send data frame\n");
                return -1;
            }
            (*sequence)++;
            next_to_send += chunk_size;
            window_end = next_to_send;
        }
        
        // Wait for ACKs
        Frame ack_frame;
        if (receive_frame_with_timeout(socket, &ack_frame, TIMEOUT_SEC) == 0) {
            if (ack_frame.type == ACK) {
                window_start = (ack_frame.sequence + 1) * MAX_DATA_SIZE;
            } else if (ack_frame.type == NACK) {
                next_to_send = ack_frame.sequence * MAX_DATA_SIZE;
            }
        } else {
            // Timeout, go back N
            next_to_send = window_start;
        }
    }
    
    return 0;
}