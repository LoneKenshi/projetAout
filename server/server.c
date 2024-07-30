#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
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
    for(size_t i=0; i<size; ++i)
        printf("%02x", buffer[i]);
    printf("\n");
}

unsigned char* binaryToHex(unsigned char *buffer, size_t size){
    unsigned char *hexString;
    for(size_t i=0; i<size; ++i){
        sprintf(&hexString[i * 2], "%02x", buffer[i]);
    }
    return hexString;
}

unsigned int calculateHash(const char* message, unsigned char *hash)
{
    // OpenSSL_add_all_digests();
    const EVP_MD *md = EVP_get_digestbyname("md5");
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

    if(!mdctx)
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

    // printf("\nhashLen: %u\nEVP_MAX_MD_SIZE: %d\n", hashLen, EVP_MAX_MD_SIZE);
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

    char *buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));

    int n, len, client_socket;
    int bind_result = bind(sockid, (struct sockaddr*) &server_addr, sizeof(server_addr));

    if(bind_result<0)
    {
        printf("Error during binding\n");
        handle_error();
    }
    else
    {
        printf("Listening on %s:%d\n", server_ip, server_port);
        n = listen(sockid, 1);

        if(n != 0)
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
                        inet_ntoa(client_addr.sin_addr), // adresse IP client
                        client_addr.sin_port); // port
        
        // recv() ou read() ?
        n = recv(client_socket, buffer, BUFFER_SIZE, 0); // client envoie un nom de fichier

        // on verifie si le fichier existe
        if(access(buffer, F_OK) != 0)
        {
            printf("This file does not exists\n");
            handle_error();
        }

        // printf("\nTaille de message: %d\nMessage: %s\nLongueur de message: %d",
        //         n, buffer, strlen(buffer));

        char *fileName = buffer;
        FILE *file = fopen(fileName, "rb");
        if(file == NULL){
            perror("Unable to open file");
            return EXIT_FAILURE;
            // TODO: utiliser la fonction handle_errors()
        }

        // Pour trouver la taille du fichier (en Bytes)
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        printf("Size of file: %ld bytes", fileSize);

        char *fileBuffer = (char *) malloc(fileSize);
        if(fileBuffer == NULL){ // Le cas ou fichier est trop grand
            fclose(file);
            close(sockid);
            handle_error();
        }

        size_t bytesRead = fread(fileBuffer, 1, fileSize, file);
        // if(bytesRead != fileSize){
        //     perror("Error reading file");
        //     fclose(file);
        //     fclose(sockid);
        //     return EXIT_FAILURE;
        // } // est-ce que ce bloc fait qqch?

        // Calculate and print the hash
        unsigned char fileHash[EVP_MAX_MD_SIZE];
        unsigned int fileHashSize;
        fileHashSize = calculateHash(fileBuffer, fileHash);

        unsigned char *fileHashHex = binaryToHex(fileHash, fileHashSize);
        // printf("\nLe hash: %s", fileHashHex);
        
        send(client_socket, fileHash, fileHashSize, 0);
        printf("\nThis is the message that was sent: ");
        printHex(fileHash, fileHashSize);
        printf("\n");

        send(client_socket, file, fileSize, 0);

        // File send block

        fclose(file);
        free(buffer);

        // recv(client_socket, fileBuffer, fileSize, MSG_WAITALL);
    }
    close(client_socket);
    close(sockid);
}