#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h> 
#include <arpa/inet.h>
#include <string.h>

#define BUFFER_SIZE 1024

unsigned char* binaryToHex(unsigned char *buffer, size_t size){
    unsigned char *hexString;
    for(size_t i=0; i<size; ++i){
        sprintf(&hexString[i * 2], "%02x", buffer[i]);
    }
    // printf("The hex is: %s", hexString);
    return hexString;
}

int main(int argc, char *argv[])
{
    char *buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));
    int sockid;
    int server_port = 8888;
    char *server_ip = "127.0.0.1";
    char *msg = argv[1]; // le nom de fichier

    sockid = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    connect(sockid, (struct sockaddr *)&server_addr, sizeof(server_addr));

    send(sockid, (char *)msg, strlen(msg), 0); 

    recv(sockid, buffer, BUFFER_SIZE, 0); // le hash
    unsigned char *fileHash = buffer;
    unsigned char *fileHashHex = binaryToHex(fileHash, strlen(fileHash));

    printf("The message was received from the server: %s", fileHashHex);
    
    free(buffer);
    char *buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));
    recv(sockid, buffer, BUFFER_SIZE, 0); // le fichier

    // // File receive bloc

    // char fileMsg[] = "HASHOK;SENDFILE";
    // send(sockid, (char *)fileMsg, strlen(fileMsg), 0);
    // recv(sockid, (char *)buffer2, sizeof(int), MSG_WAITFORONE);
    // printf("\nLa taille: %d", buffer2);

    close(sockid);
}