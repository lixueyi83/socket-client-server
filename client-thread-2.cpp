/*  Make the necessary includes and set up the variables.  */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>


int main()
{
    signal(SIGPIPE, SIG_IGN);
    
    int server_sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    char read_buf[10];

/*  Create a socket for the client.  */

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

/*  Name the socket, as agreed with the server.  */

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(10005);
    len = sizeof(address);

/*  Now connect our socket to the server's socket.  */

    result = connect(server_sockfd, (struct sockaddr *)&address, len);

    if(result == -1) 
    {
        perror("oops: client3");
        exit(1);
    }

    unsigned char id[2] = {0xa6, 0x52};
    write(server_sockfd, &id, 2);

/*  We can now read/write via server_sockfd.  */
    while(1)
    {
        static int cnts = 0; 
        cnts++;

        read(server_sockfd, &read_buf, 10);
        printf("client-rcvd msg: ' %s ' from server_sockfd %d --------%d\n", read_buf, server_sockfd, cnts);
    }
    close(server_sockfd);
    exit(0);
}
