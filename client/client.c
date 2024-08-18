#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define HASH_SIZE 16
#define FILE_NOT_FOUND_MSG "Error: File not found"

unsigned char* binaryToHex(unsigned char *buffer, size_t size) {
    unsigned char *hexString = (unsigned char*)malloc(size*2+1);
    for(size_t i = 0; i < size; ++i) {
        sprintf(&hexString[i * 2], "%02x", buffer[i]);
    }
    hexString[size*2] = '\0';
    return hexString;
}

int main(int argc, char *argv[]) {
    if(argc != 4)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));
    int sockid;

    char *server_ip = argv[1]; // l'ip du serveur
    int server_port = atoi(argv[2]);
    char *msg = argv[3]; // le nom de fichier

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

    int bytesRead = recv(sockid, buffer, BUFFER_SIZE, 0);
    if (bytesRead <= 0) {
        perror("Error receiving response");
        exit(EXIT_FAILURE);
    }

    // Check if the received message is the file-not-found error
    if (strncmp(buffer, FILE_NOT_FOUND_MSG, strlen(FILE_NOT_FOUND_MSG)) == 0) {
        printf("Server error: %s\n", buffer);
        close(sockid);
        free(buffer);
        exit(EXIT_FAILURE);
    }

    // Otherwise, assume it's the hash
    unsigned char *fileHashHex = binaryToHex((unsigned char*)buffer, HASH_SIZE);
    printf("The hash received from the server: %s\n", fileHashHex);
    free(fileHashHex);

    FILE *file = fopen("file2.txt", "wb");

    if (file == NULL)
    {
        perror("File open error");
        exit(EXIT_FAILURE);
    }

    // Continue to receive the file content
    while((bytesRead = read(sockid, buffer, BUFFER_SIZE)) > 0) 
    {
        fwrite(buffer, sizeof(char), bytesRead, file);
    }

    if(bytesRead < 0)
        perror("File receive error");
    else
        printf("File received successfully\n");

    fclose(file);
    free(buffer);
    close(sockid);

    return 0;
}
