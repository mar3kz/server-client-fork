#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>

#define PORT "9000"
#define SIZE len

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("priste zadejte text");
        return EXIT_FAILURE;
    }

    struct addrinfo hints, *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;

    if ( getaddrinfo(NULL, PORT, &hints, &server_info) != 0) {
        perror("getaddrinfo() selhal");
        return EXIT_FAILURE;
    }

    int socketfd;
    if ( ( socketfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol) ) == -1) {
        perror("socket() selhal");
        return EXIT_FAILURE;
    }

    socklen_t length = sizeof (*(struct sockaddr *)server_info->ai_addr);
    if ( connect(socketfd, (struct sockaddr *)server_info->ai_addr, length) == -1 ) { 
        perror("connect() selhal");
        return EXIT_FAILURE;
    }

    char *message = argv[1];
    size_t bytes_sent = 0;

    int len;
    recv(socketfd, (void *)&len, sizeof(int), 0);

    int buffer_length = strlen(message);
    send(socketfd, (void *)&buffer_length, sizeof(int), 0);
    while (bytes_sent < strlen(message)) {
        size_t sent_now = send(socketfd, message + bytes_sent, strlen(message) - bytes_sent, 0);

        if (sent_now == -1) {
            printf("probehl error pri posilani, je mozne, ze uz je poslane");
            return EXIT_FAILURE;
        }
        bytes_sent += sent_now;
    }
    printf("\nvse odeslano ze strany clienta, poslano: %s, PID: %d\n", message, getpid());

    char received[SIZE + 1];
    size_t rec = 0;

    while(rec != len) {
        size_t rec_now = recv(socketfd, received + rec, SIZE - rec, 0);

        if (rec_now == -1) {
            perror("recv() selhal");
            exit(EXIT_FAILURE);
        }
        else if (rec_now == 0) {
            perror("client ukoncil spojeni drive nez se to vse poslalo");
            break;
        }
        rec += rec_now;
    }

    received[rec] = '\0';
    printf("Poslane ze serveru: %s", received);

    close(socketfd);

    return EXIT_SUCCESS;
}