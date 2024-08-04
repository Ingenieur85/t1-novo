// client.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "protocol.h"
#include "rawsocket.h"

int main() {
    int socket = cria_raw_socket(INTERFACE_NAME);
    if (socket < 0) {
        fprintf(stderr, "Failed to create raw socket\n");
        return 1;
    }

    static uint8_t sequence = 0;

    const char* test_message = "Hello, server!";
    Frame frame_to_send = create_frame(strlen(test_message), sequence++, DATA, (const uint8_t*)test_message);

    printf("Sending frame: type=%d, size=%d, data=%s\n",
           frame_to_send.type, frame_to_send.size, frame_to_send.data);

    if (send_frame(socket, &frame_to_send) < 0) {
        fprintf(stderr, "Failed to send frame\n");
        close(socket);
        return 1;
    }

    Frame received_frame;
    if (receive_frame(socket, &received_frame) == 0) {
        printf("Received response: type=%d, size=%d, data=%.*s\n",
               received_frame.type, received_frame.size,
               received_frame.size, received_frame.data);
    } else {
        fprintf(stderr, "Failed to receive response\n");
    }

    close(socket);
    return 0;
}