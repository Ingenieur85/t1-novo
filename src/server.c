// server.c
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

    printf("Server listening on interface %s\n", INTERFACE_NAME);

    while (1) {
        Frame received_frame;
        if (receive_frame(socket, &received_frame) == 0) {
            printf("Received frame: type=%d, size=%d, data=%.*s\n",
                   received_frame.type, received_frame.size,
                   received_frame.size, received_frame.data);

            // Echo the frame back to the client
            Frame response_frame = create_frame(received_frame.size, received_frame.sequence,
                                                DATA, received_frame.data);
            if (send_frame(socket, &response_frame) < 0) {
                fprintf(stderr, "Failed to send response frame\n");
            }
        }
    }

    close(socket);
    return 0;
}