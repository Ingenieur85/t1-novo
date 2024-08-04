#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "protocol.h"
#include "rawsocket.h"

#define VIDEO_DIR "videos/"

int main() {
    int socket = cria_raw_socket(INTERFACE_NAME);
    if (socket < 0) {
        fprintf(stderr, "Failed to create raw socket\n");
        return 1;
    }

    printf("Server listening on interface %s\n", INTERFACE_NAME);

    while (1) {
        Frame request_frame;
        if (receive_frame_with_timeout(socket, &request_frame, TIMEOUT_SEC) == 0) {
            if (request_frame.type == DOWNLOAD) {
                char filename[64] = {0};
                strncpy(filename, (char*)request_frame.data, request_frame.size);
                
                char filepath[128];
                snprintf(filepath, sizeof(filepath), "%s%s", VIDEO_DIR, filename);
                
                FILE* fp = fopen(filepath, "rb");
                if (!fp) {
                    fprintf(stderr, "Failed to open file: %s\n", filepath);
                    Frame error_frame = create_frame(1, request_frame.sequence, ERROR, (const uint8_t*)"\x02");
                    send_frame_with_timeout(socket, &error_frame, TIMEOUT_SEC);
                    continue;
                }
                
                fseek(fp, 0, SEEK_END);
                size_t file_size = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                
                // Send file descriptor
                Frame descriptor_frame = create_frame(sizeof(size_t), request_frame.sequence, FILE_DESCRIPTOR, (const uint8_t*)&file_size);
                if (send_and_wait_ack(socket, &descriptor_frame, MAX_RETRIES) < 0) {
                    fprintf(stderr, "Failed to send file descriptor\n");
                    fclose(fp);
                    continue;
                }
                
                // Send file data
                uint8_t buffer[MAX_DATA_SIZE];
                size_t bytes_read;
                uint8_t sequence = 0;
                
                while ((bytes_read = fread(buffer, 1, MAX_DATA_SIZE, fp)) > 0) {
                    Frame data_frame = create_frame(bytes_read, sequence, DATA, buffer);
                    if (send_and_wait_ack(socket, &data_frame, MAX_RETRIES) < 0) {
                        fprintf(stderr, "Failed to send data frame\n");
                        break;
                    }
                    sequence++;
                }
                
                fclose(fp);
                printf("File sent: %s\n", filename);
            }
        }
    }

    close(socket);
    return 0;
}