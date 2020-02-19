#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h>
/*
 *
    Create a socket with the socket() system call.
    !Take own ip and set fix port

    Bind the socket to an address using the bind() system call. 
    For a server socket on the Internet, 
    an address consists of a port number on the host machine.

    Listen for connections with the listen() system call.

    Accept a connection with the accept() system call. 
    This call typically blocks until a client connects with the server.

    !Fork a child process
    Send and receive data using the read() and write() system calls.
    !OR use send receive
*/

#define PORT "9797" 
#define BACKLOG 10

void sigchld_handler(int s) {
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
   if (sa->sa_family == AF_INET) {
      return &(((struct sockaddr_in*)sa)->sin_addr); 
    }

   return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void) {
    //fd for my sock and new for send
    int sockfd, new_fd;

    //address infos for server
    struct addrinfo hints, *servinfo, *p;

    //socket addr storage for conneting entity
    struct sockaddr_storage cli_addr;

    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    //clear hints struct
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; //will take my own IP

    if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "error in getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }
    freeaddrinfo(servinfo);

    if (p = NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    printf("server: waiting for connections\n");

    while(1) {
        sin_size = sizeof cli_addr;
        new_fd = accept(sockfd, (struct sockaddr*)&cli_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(cli_addr.ss_family,
                get_in_addr((struct sockaddr*)&cli_addr),
                s, sizeof s);
        printf("server: got conncetion from %s\n", s);
        
        if(!fork()) {
            close(sockfd);
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }       

    return 0;
                
}
//
//TODO figure out why IPPROTO_TCP undefined
//http://beej.us/guide/bgnet/html/#audience
