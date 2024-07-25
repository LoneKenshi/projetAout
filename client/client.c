#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h> // for close()
#include <arpa/inet.h> // for init_addr
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    char buffer[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];
    int sockid;
    int server_port = 8888;
    char *server_ip = "127.0.0.1";

    sockid = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    char *msg = argv[1];

    connect(sockid, (struct sockaddr *)&server_addr, sizeof(server_addr));

    send(sockid, (const char *)msg, strlen(msg),0);

    int n = recv(sockid, (char *)buffer2, BUFFER_SIZE -1, MSG_WAITALL);
    buffer2[n] = '\0';

    printf("The message was received from the server: %s", buffer2);
    puts(buffer);

    close(sockid);
}