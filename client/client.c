#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>

#define BUFFER_SIZE 1024
#define HASH_SIZE 16
#define FILE_NOT_FOUND_MSG "Error: File not found"

unsigned char* binaryToHex(unsigned char *buffer, size_t size) {
    unsigned char *hexString = (unsigned char*)malloc(size * 2 + 1);
    for (size_t i = 0; i < size; ++i) {
        sprintf(&hexString[i * 2], "%02x", buffer[i]);
    }
    hexString[size * 2] = '\0';
    return hexString;
}

void calculateFileHash(const char *fileName, unsigned char *hash, unsigned int *hashLen) {
    const EVP_MD *md = EVP_get_digestbyname("md5");
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    FILE *file = fopen(fileName, "rb");
    
    if (mdctx == NULL || file == NULL) {
        perror("Error initializing hash or opening file");
        exit(EXIT_FAILURE);
    }
    
    if (1 != EVP_DigestInit_ex(mdctx, md, NULL)) {
        perror("Error initializing digest");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (1 != EVP_DigestUpdate(mdctx, buffer, bytes_read)) {
            perror("Error updating digest");
            exit(EXIT_FAILURE);
        }
    }

    if (1 != EVP_DigestFinal(mdctx, hash, hashLen)) {
        perror("Error finalizing digest");
        exit(EXIT_FAILURE);
    }

    EVP_MD_CTX_free(mdctx);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));
    int sockid;

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    char *msg = argv[3];

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

    int bytesRead = recv(sockid, buffer, HASH_SIZE, 0);
    if (bytesRead <= 0) {
        perror("Error receiving response");
        exit(EXIT_FAILURE);
    }

    if (strncmp(buffer, FILE_NOT_FOUND_MSG, strlen(FILE_NOT_FOUND_MSG)) == 0) {
        printf("Server error: %s\n", buffer);
        close(sockid);
        free(buffer);
        exit(EXIT_FAILURE);
    }

    unsigned char receivedHash[HASH_SIZE];
    memcpy(receivedHash, buffer, HASH_SIZE);

    FILE *file = fopen("file2.txt", "wb");
    if (file == NULL) {
        perror("File open error");
        exit(EXIT_FAILURE);
    }

    while ((bytesRead = read(sockid, buffer, BUFFER_SIZE)) > 0) {
        fwrite(buffer, sizeof(char), bytesRead, file);
    }

    if (bytesRead < 0) {
        perror("File receive error");
    } else {
        printf("File received successfully\n");
    }

    fclose(file);

    unsigned char calculatedHash[HASH_SIZE];
    unsigned int hashLen;

    calculateFileHash("file2.txt", calculatedHash, &hashLen);

    if (hashLen != HASH_SIZE) {
        fprintf(stderr, "Hash length mismatch\n");
        exit(EXIT_FAILURE);
    }

    if (memcmp(receivedHash, calculatedHash, HASH_SIZE) == 0) {
        printf("The file hash matches the received hash.\n");
    } else {
        printf("The file hash does not match the received hash.\n");
    }

    free(buffer);
    close(sockid);

    return 0;
}
