#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h> // for close()
#include <arpa/inet.h> // for init_addr
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

#define BUFFER_SIZE 1024

// pour compiler: gcc -o <executable> <fichier_source> -lcrypto -lssl

void handle_error()
{
    fprintf(stderr, "Error occured.\n");
    exit(EXIT_FAILURE);
}

void printHex(unsigned char *buffer, size_t size)
{
    for(size_t i = 0; i < size; ++i)
        printf("%02x", buffer[i]);
    printf("\n");
}

void binaryToHex(unsigned char *buffer, size_t size){
    unsigned char *hexString;
    for(size_t i=0; i<size; ++i){
        sprintf(&hexString[i * 2], "%02x", buffer[i]);
    }
    printf("The hex is: %s", hexString);
}

unsigned int calculateHash(const char* message, unsigned char *hash)
{
    // OpenSSL_add_all_digests();
    const EVP_MD *md = EVP_get_digestbyname("md5");
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx)
    {
        handle_error();
    }
    if (1 != EVP_DigestInit_ex(mdctx, md, NULL))
    {
        handle_error();
    }
    if (1 != EVP_DigestUpdate(mdctx, message, strlen(message)))
    {
        handle_error();
    }

    // unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;

    if (1 != EVP_DigestFinal(mdctx, hash, &hashLen)) //EVP_DigestFinal_ex(); #TODO
    {
        handle_error();
    }

    printf("\nhashLen: %u\nEVP_MAX_MD_SIZE: %d\n", hashLen, EVP_MAX_MD_SIZE);
    // printHex(hash, hashLen);
    EVP_MD_CTX_free(mdctx);
    return hashLen;
}

int main(void)
{
    int server_port = 8888;
    char server_ip[] = "127.0.0.1";

    int sockid = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // char buffer[BUFFER_SIZE];
    char *buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));

    int n, len, client_socket;

    int bind_result = bind(sockid, (struct sockaddr *) &server_addr, sizeof(server_addr));

    if(bind_result<0)
    {
        printf("Error during binding\n");
    }
    else
    {
        printf("Listening on %s:%d\n", server_ip, server_port);
        n = listen(sockid, 1);

        if(n < 0)
        {
            printf("Error during listen()\n");
            handle_error();
        }

        len = sizeof(client_addr);
        client_socket = accept(sockid, (struct sockaddr* )&client_addr, &len);
        
        if(client_socket < 0)
        {
            printf("Error during accept()\n");
            handle_error();
        }

        printf("Accept connection from %s:%d\n",
                        inet_ntoa(client_addr.sin_addr),
                        client_addr.sin_port);
        
        n = recv(client_socket, buffer, BUFFER_SIZE, MSG_WAITFORONE);
        buffer[n] = '\0';

        if(access(buffer, F_OK) != 0)
        {
            printf("This file does not exists\n");
            handle_error();
        }

        printf("\nMessage of size %d received: %s\n",n, buffer);   

        const char *fileName = buffer;
        FILE *file = fopen(fileName, "rb");

        if(file == NULL){
            perror("Unable to open file");
            return EXIT_FAILURE;
        }

        // Pour trouver la taille du fichier en B
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        printf("Size of file: %d octets", fileSize);

        char fileBuffer[fileSize];        
        size_t bytesRead = fread(fileBuffer, 1, fileSize, file);
        if (bytesRead != fileSize) 
        {
            perror("Error reading file");
            fclose(file);
            return EXIT_FAILURE;
        }

        // Null-terminate the buffer to ensure it is a valid C string
        fileBuffer[fileSize] = '\0';

        // Calculate and print the hash

        unsigned char fileHash[EVP_MAX_MD_SIZE];
        unsigned int fileHashSize;
        fileHashSize = calculateHash(fileBuffer, fileHash);

        binaryToHex(fileHash, fileHashSize);

        fclose(file);
        free(buffer);

        // char *hello = "Hello from the server";
        // send(client_socket, fileHash, fileHashSize, 0);
        // printf("\nThis is the message that was sent: ");
        // printHex(fileHash, fileHashSize);
    }

    close(client_socket);
    close(sockid);
}