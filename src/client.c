#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "protocol.h"
#include "rawsocket.h"

#define VIDEO_FILE "videos/apollo11.mp4"

int main() {
    int socket = cria_raw_socket(INTERFACE_NAME);
    if (socket < 0) {
        fprintf(stderr, "Failed to create raw socket\n");
        return 1;
    }

    static uint8_t sequence = 0;

    // Request video file
    Frame request_frame = create_frame(strlen(VIDEO_FILE), sequence++, DOWNLOAD, (const uint8_t*)VIDEO_FILE);
    if (send_and_wait_ack(socket, &request_frame, MAX_RETRIES) < 0) {
        fprintf(stderr, "Failed to send video request\n");
        close(socket);
        return 1;
    }

    // Receive file descriptor
    Frame descriptor_frame;
    if (receive_frame_with_timeout(socket, &descriptor_frame, TIMEOUT_SEC) < 0 || descriptor_frame.type != FILE_DESCRIPTOR) {
        fprintf(stderr, "Failed to receive file descriptor\n");
        close(socket);
        return 1;
    }

    size_t file_size;
    memcpy(&file_size, descriptor_frame.data, sizeof(size_t));
    printf("Receiving video file of size %zu bytes\n", file_size);

    // Receive file data
    uint8_t* file_buffer = malloc(file_size);
    if (!file_buffer) {
        fprintf(stderr, "Failed to allocate memory for file\n");
        close(socket);
        return 1;
    }

    size_t received = 0;
    while (received < file_size) {
        Frame data_frame;
        if (receive_frame_with_timeout(socket, &data_frame, TIMEOUT_SEC) == 0 && data_frame.type == DATA) {
            memcpy(file_buffer + received, data_frame.data, data_frame.size);
            received += data_frame.size;
            
            // Send ACK
            Frame ack_frame = create_frame(0, data_frame.sequence, ACK, NULL);
            send_frame_with_timeout(socket, &ack_frame, TIMEOUT_SEC);
        } else {
            fprintf(stderr, "Failed to receive data frame\n");
            break;
        }
    }

    if (received == file_size) {
        printf("Video file received successfully\n");
        // Save the file or play it
        FILE* fp = fopen("received_video.mp4", "wb");
        if (fp) {
            fwrite(file_buffer, 1, file_size, fp);
            fclose(fp);
            printf("Video saved as received_video.mp4\n");
        } else {
            fprintf(stderr, "Failed to save video file\n");
        }
    } else {
        fprintf(stderr, "Incomplete video file received\n");
    }

    free(file_buffer);
    close(socket);
    return 0;
}