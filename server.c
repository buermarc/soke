#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
/*
 *
    Create a socket with the socket() system call.

    Bind the socket to an address using the bind() system call. 
    For a server socket on the Internet, 
    an address consists of a port number on the host machine.

    Listen for connections with the listen() system call.

    Accept a connection with the accept() system call. 
    This call typically blocks until a client connects with the server.

    Send and receive data using the read() and write() system calls.
*/

int main(void) {
    int socketfd;
    //TODO figure out why IPPROTO_TCP undefined
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        fprintf(stderr, "failed to create socket errno is: %d\n", errno);

    struct sockaddr saddr;
    saddr.sa_family = 0;
    //TODO how to init
    *saddr.sa_data = (char)0;
    if (bind(socketfd, &saddr, sizeof(saddr)) == -1) 
        fprintf(stderr, "failed to bind socket errno is: %d\n", errno);
}
