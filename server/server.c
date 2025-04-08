#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT "3490" // the port users will be connecting to
#define BACKLOG 10

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr); 
}

int main(void){
    int sockfd, newfd;
    struct addrinfo hints, *res, *p;

    struct sockaddr_storage their_addr; // connector's address information
    socklen_t addr_size;

    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    int status;
    if((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    // loop through all results and bind to the first we can 
    for(p = res; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
           perror("server: socket");
           continue;
        }

        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;

    }

    freeaddrinfo(res); // done with this structure
    
    if(p == NULL){
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if(listen(sockfd, BACKLOG) == -1){
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    
    char s [INET6_ADDRSTRLEN]; 
    while(1){
        addr_size = sizeof their_addr;
        newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if(newfd == -1){
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
                    s, sizeof s);
        printf("server: got connection from: %s\n", s);

        if(fork() == 0){
            close(sockfd);
            if(send(newfd, "Hello world", 13, 0) == -1){
                perror("send");
            }
            close(newfd);
            exit(0);
        }
        close(newfd);
    }
    return 0;

}
