#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netdb.h> 
#include <ifaddrs.h> 
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

# define BACKLOG 5
# define PORT "9000"
# define SIZE len_received

void print_info(struct addrinfo *getInfoResultPointer);
void handle_client_fork(int communication_socket, int socketfd);

int main()
{
    struct addrinfo hints, *server_info; 
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0; 
    hints.ai_flags = AI_PASSIVE; 
    hints.ai_canonname = NULL;

    int result;
    if ( result = getaddrinfo(NULL, PORT, &hints, &server_info) != 0) {
        perror("selhalo getaddrinfo");
        exit(1);
    }

    print_info(server_info);
    int socketfd;

    if ( (socketfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol) ) == -1) {
        perror("socket() selhala");
        exit(1);
    }

    int option_value = 1;
    if ( setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value)) == -1) {
        perror("setsockopt() selhal");
        return EXIT_FAILURE;
    }

    if ( bind( socketfd, server_info->ai_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind() selhal");
        exit(1);
    }

    if ( listen( socketfd, BACKLOG) == -1) {
        perror("listen() selhal");
        exit(1);
    }

    while(1) {
        struct sockaddr_storage client_info;

        int comfd;
        socklen_t length_of_struct = sizeof(struct sockaddr_storage);
        if ( ( comfd = accept(socketfd, (struct sockaddr *)&client_info, &length_of_struct) ) == -1) { // informace o userovi (client)
            printf("%d", errno);
            perror("accept() selhal");
            return EXIT_FAILURE;
        }

        pid_t child_process = fork(); 

        if (child_process == 0) {
            handle_client_fork(comfd, socketfd);

            return EXIT_SUCCESS;
        }
        else if (child_process == -1) {
            perror("novy proces se nevytvoril, chyba!");
            continue;
        }
        else {
            close(comfd);
        }
    }

    close(socketfd);

    return EXIT_SUCCESS;
}
void print_info(struct addrinfo *getInfoResultPointer) {
    while (getInfoResultPointer != NULL) {
        printf("Address information - flags: %d\n", getInfoResultPointer->ai_flags);
        printf("Address information - IP family: %d\n", getInfoResultPointer->ai_family);
        printf("Address information - socket type: %d\n", getInfoResultPointer->ai_socktype);
        printf("Address information - protocol: %d\n", getInfoResultPointer->ai_protocol);
        printf("Address information - length of sockaddr struct: %u\n", (unsigned int)getInfoResultPointer->ai_addrlen);
        printf("Address information - canonname: %s\n", getInfoResultPointer->ai_canonname);

        struct sockaddr_storage uschovna;
        memcpy(&uschovna, getInfoResultPointer->ai_addr, sizeof(struct sockaddr));

        switch(getInfoResultPointer->ai_family) {
            case AF_INET:
                char ip4_address_readable[INET_ADDRSTRLEN];
                if (inet_ntop(AF_INET, &((( struct sockaddr_in *)&uschovna)->sin_addr), ip4_address_readable, INET_ADDRSTRLEN) == NULL) {
                    perror("převedeni IP adresy se nepovedlo");
                }
                printf("Address information - IP address IPv4: %s", ip4_address_readable);
                break;

            case AF_INET6:

                char ip6_address_readable[INET6_ADDRSTRLEN];
                if (inet_ntop(AF_INET, &((( struct sockaddr_in6 *)&uschovna)->sin6_addr), ip6_address_readable, INET6_ADDRSTRLEN) == NULL) {
                    perror("převedeni IP adresy se nepovedlo");
                }
                printf("Address information - IP address IPv6: %s", ip6_address_readable);
                break;

            default:
                char chyba[10];
                strcpy(chyba, "chyba");
                printf(chyba);
                break;
        }

        struct ifaddrs *ifaddr;
        if ( getifaddrs(&ifaddr) == -1) {
            perror("neslo ziskat vsechna sitova rozhrani");
            exit(1);
        }

        printf("\nprotoze se pouziva 0.0.0.0, tak tady jsou vsechny sitove rozhrani:\n\n");

        while ( ifaddr != NULL) {
            struct sockaddr_storage uschovna;
            memcpy(&uschovna, ifaddr->ifa_addr, sizeof(struct sockaddr));

            switch(uschovna.ss_family)
            {
                case AF_INET:
                    if (getInfoResultPointer->ai_family == AF_INET) {
                        char ip4_address_readable[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &( ( (struct sockaddr_in *)&uschovna)->sin_addr) , ip4_address_readable, INET_ADDRSTRLEN);
                        printf("%s %s\n", ifaddr->ifa_name, ip4_address_readable);
                    }   
                    break;
                case AF_INET6:
                    if (getInfoResultPointer->ai_family == AF_INET6) {
                        char ip6_address_readable[INET6_ADDRSTRLEN];
                        inet_ntop(AF_INET6, &( ( (struct sockaddr_in6 *)&uschovna)->sin6_addr), ip6_address_readable, INET6_ADDRSTRLEN);
                        printf("%s %s\n", ifaddr->ifa_name, ip6_address_readable);
                    }
                    break;
            }
            ifaddr = ifaddr->ifa_next;
        }
        getInfoResultPointer = getInfoResultPointer->ai_next;
    }
}
void handle_client_fork(int communication_socket, int socketfd) {
    char message[] = "Ahoj, posilam ti aaaaaa ze serveru!";

    int bytes_sent = 0;
    int len = strlen(message);

    if ( send(communication_socket, (const void *)&len, sizeof(len), 0) == -1) {
        perror("nejde efektivne poslat zpravu ze strany serveru!");
        exit(EXIT_FAILURE);
    }

    while(bytes_sent < len) {
        int sent_now = send(communication_socket, message + bytes_sent, len - bytes_sent, 0);

        if (sent_now == -1) {
            perror("send() selhala");
            exit(1);
        }
        bytes_sent += sent_now;
    }

    int len_received;
    recv(communication_socket, (void *)&len_received, sizeof(int), 0);

    char message_rec[SIZE + 1];
    size_t rec = 0;
    while (rec != len_received) {
        size_t rec_now = recv(communication_socket, (void *)message_rec + rec, SIZE - rec, 0);

        if (rec_now == -1) {
            exit(EXIT_FAILURE);
        }
        else if (rec_now == 0) {
            perror("client ukoncil spojeni drive nez se to vse poslalo");
            break;
        }
        rec += rec_now;
    }
    message_rec[rec] = '\0';

    printf("\nposlano ze strany clienta: %s\n", message_rec);
    fflush(stdout);

    close(communication_socket);
    close(socketfd);
}
