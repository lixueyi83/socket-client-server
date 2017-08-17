/*  Make the necessary includes and set up the variables.  */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int server_sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    char send_buf[10] = "pong";
    char read_buf[10];

/*  Create a socket for the client.  */

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

/*  Name the socket, as agreed with the server.  */

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(10002);
    len = sizeof(address);

/*  Now connect our socket to the server's socket.  */

    result = connect(server_sockfd, (struct sockaddr *)&address, len);

    if(result == -1) 
    {
        perror("oops: client3");
        exit(1);
    }

/*  We can now read/write via server_sockfd.  */
    while(1)
    {
	    static int cnts = 0; 
	    cnts++;
	    printf("client is connected with server_sockfd = %d ---------- %d\n", server_sockfd, cnts);

        read(server_sockfd, &read_buf, 2);
        printf("\nclient-read: %x %x\n", 0xff & read_buf[0], 0xff & read_buf[1]);

        if(0xaa == (0xff & read_buf[0]) && 0x55 == (0xff & read_buf[1]))
        {
            printf("client send heart beat cmd ACK 0xa5\n");
            unsigned char cmd_ack = 0xa5;
            write(server_sockfd, &cmd_ack, 1);
        }

        read(server_sockfd, &read_buf, 10);
	    printf("client-rcvd msg: %s\n", read_buf);
        write(server_sockfd, &send_buf, 10);
        printf("client-send msg: %s\n", send_buf);
    }
    close(server_sockfd);
    exit(0);
}
