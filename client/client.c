#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define HASH_SIZE 16 // MD5 produces a 16-byte hash

unsigned char* binaryToHex(unsigned char *buffer, size_t size) {
    unsigned char *hexString = (unsigned char*)malloc(size*2+1);
    for(size_t i = 0; i < size; ++i) {
        sprintf(&hexString[i * 2], "%02x", buffer[i]);
    }
    hexString[size*2] = '\0';
    return hexString;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));
    int sockid;
    int server_port = 8888;
    char *server_ip = "127.0.0.1";
    char *msg = argv[1]; // le nom de fichier

    sockid = socket(AF_INET, SOCK_STREAM, 0);
    if (sockid < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(sockid, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    send(sockid, msg, strlen(msg), 0);

    // Receive the hash from the server
    unsigned char hash[HASH_SIZE];
    if (recv(sockid, hash, HASH_SIZE, 0) != HASH_SIZE) {
        perror("Hash receive error");
        exit(EXIT_FAILURE);
    }
    unsigned char *fileHashHex = binaryToHex(hash, HASH_SIZE);
    printf("The hash received from the server: %s\n", fileHashHex);
    free(fileHashHex);

    // Open the file to write the received data
    FILE *file = fopen("file2.txt", "wb");
    if (file == NULL) {
        perror("File open error");
        exit(EXIT_FAILURE);
    }

    int bytesRead;
    while ((bytesRead = read(sockid, buffer, BUFFER_SIZE)) > 0) {
        fwrite(buffer, sizeof(char), bytesRead, file);
    }

    if (bytesRead < 0) {
        perror("File receive error");
    } else {
        printf("File received successfully\n");
    }

    fclose(file);
    free(buffer);
    close(sockid);

    return 0;
}
