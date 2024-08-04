#include <stdio.h>
#include <string.h>
#include "common.h"

extern int cria_raw_socket(char *nome_interface_rede);

int main() {
    init_crc8_table();  // Initialize CRC table

    int socket = cria_raw_socket(INTERFACE);
    if (socket == -1) {
        fprintf(stderr, "Failed to create raw socket\n");
        return 1;
    }

    const char *test_message = "Hello, Server!";
    Frame frame;
    create_frame(&frame, strlen(test_message), 0, DATA, (const uint8_t *)test_message);

    printf("Sending frame:\n");
    print_frame(&frame);

    if (send_frame(socket, &frame) == 0) {
        printf("Frame sent successfully\n");

        // Wait for echo
        Frame echo_frame;  // Changed variable name from recv_frame to echo_frame
        if (recv_frame(socket, &echo_frame) == 0) {
            printf("Received echo:\n");
            print_frame(&echo_frame);
        }
    } else {
        fprintf(stderr, "Failed to send frame\n");
    }

    return 0;
}