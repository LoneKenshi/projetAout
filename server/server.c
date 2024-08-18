#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

#define BUFFER_SIZE 1024

// pour compiler: gcc -o server server.c -lcrypto -lssl

void handle_error(int sockid, int client_socket, char* buffer)
{
    if(sockid >= 0)
    {
        close(sockid);
    }
    if(client_socket >= 0)
    {
        close(client_socket);
    }
    if(buffer != NULL)
    {
        free(buffer);
    }

    fprintf(stderr, "Error occurred.\n");
    exit(EXIT_FAILURE);
}

void printHex(unsigned char *buffer, size_t size)
{
    for(size_t i = 0; i < size; ++i)
        printf("%02x", buffer[i]);
    printf("\n");
}

unsigned int calculateFileHash(FILE *file, unsigned char *hash)
{
    const EVP_MD *md = EVP_get_digestbyname("md5");
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

    if(!mdctx)
    {
        handle_error(-1, -1, NULL);
    }
    if (1 != EVP_DigestInit_ex(mdctx, md, NULL))
    {
        handle_error(-1, -1, NULL);
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
    {
        if (1 != EVP_DigestUpdate(mdctx, buffer, bytes_read))
        {
            handle_error(-1, -1, buffer);
        }
    }

    unsigned int hashLen;
    if (1 != EVP_DigestFinal(mdctx, hash, &hashLen))
    {
        handle_error(-1, -1, NULL);
    }

    EVP_MD_CTX_free(mdctx);
    return hashLen;
}

int main(int argc, char *argv[])
{
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sockid = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    char *buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));

    int bind_result = bind(sockid, (struct sockaddr*) &server_addr, sizeof(server_addr));

    if(bind_result < 0)
    {
        printf("Error during binding\n");
        handle_error(sockid, -1, buffer);
    }

    printf("Listening on %s:%d\n", server_ip, server_port);
    if (listen(sockid, 1) != 0)
    {
        printf("Error during listen()\n");
        handle_error(sockid, -1, buffer);
    }

    while(1) // Main loop to handle multiple clients
    {
        int len = sizeof(client_addr);
        int client_socket = accept(sockid, (struct sockaddr* )&client_addr, &len);

        if(client_socket < 0)
        {
            printf("Error during accept()\n");
            handle_error(sockid, -1, buffer);
        }

        printf("Accepted connection from %s:%d\n",
                        inet_ntoa(client_addr.sin_addr), // adresse IP client
                        ntohs(client_addr.sin_port)); // port

        int n = recv(client_socket, buffer, BUFFER_SIZE, 0); // client envoie un nom de fichier
        if (n <= 0)
        {
            printf("Error receiving filename\n");
            handle_error(sockid, client_socket, buffer);
        }
        buffer[n] = '\0'; // Null-terminate the received string

        // on vérifie si le fichier existe
        if(access(buffer, F_OK) != 0)
        {
            printf("This file does not exist\n");
            close(client_socket);
            continue;
        }

        char *fileName = buffer;
        FILE *file = fopen(fileName, "rb");
        if(file == NULL)
        {
            perror("Unable to open file");
            close(client_socket);
            continue;
        }

        // Pour trouver la taille du fichier (en Bytes)
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        printf("Size of file: %ld bytes\n", fileSize);

        // Calculate and send the hash
        unsigned char fileHash[EVP_MAX_MD_SIZE];
        unsigned int fileHashSize = calculateFileHash(file, fileHash);

        // Send the hash to the client
        send(client_socket, fileHash, fileHashSize, 0);
        printf("This is the message that was sent: ");
        printHex(fileHash, fileHashSize);

        // Reset file pointer to beginning for sending
        fseek(file, 0, SEEK_SET);

        // Send the file contents to the client in chunks
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
        {
            if (send(client_socket, buffer, bytes_read, 0) == -1)
            {
                handle_error(sockid, client_socket, buffer);
            }
        }

        printf("File sent successfully\n");

        // Clean up for this client
        fclose(file);
        close(client_socket);
    }

    free(buffer);
    close(sockid);

    return 0;
}
