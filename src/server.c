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

    printf("Server listening on interface %s\n", INTERFACE);

    while (1) {
        Frame frame;
        if (recv_frame(socket, &frame) == 0) {
            printf("Received frame:\n");
            print_frame(&frame);

            // Echo the frame back
            send_frame(socket, &frame);
        }
    }

    return 0;
}