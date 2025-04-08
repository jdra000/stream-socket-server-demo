
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT "3490"
#define MAXDATASIZE 100

// desgined for IPv4 and IPv6
void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr); 
}
int main(int argc, char *argv[]){
   int sockfd, numbytes;
   char buf[MAXDATASIZE];

   struct addrinfo hints, *res, *p;

   if(argc != 2){
       fprintf(stderr, "usage: clienthostname\n");
       exit(1);
   }

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;

   int status;
   if((status = getaddrinfo(argv[1], PORT, &hints, &res)) != 0){
       fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
       return 1;
   }

   // loop through results and connect to the first we can
   for(p = res; p!=NULL; p=p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }

        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
   }

   freeaddrinfo(res);

   if(p == NULL){
       fprintf(stderr, "client: failed to connect\n");
       return 2;
   }

   struct sockaddr_storage server_addr;
   socklen_t addr_size = sizeof server_addr;

   if(getpeername(sockfd, ((struct sockaddr *)&server_addr), &addr_size) == -1){
       perror("client: getting info about server");
   }
   char s [INET_ADDRSTRLEN];  
   inet_ntop(server_addr.ss_family, get_in_addr((struct sockaddr *)&server_addr), 
           s, sizeof s);
   printf("client: connecting to %s\n", s);

   if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
       perror("recv");
       exit(1);
   }

   buf[numbytes] = '\0';
   printf("client: received %s\n", buf);
   close(sockfd);
   return 0;
}
