/*  For our final example, server5.c, 
    we include the sys/time.h and sys/ioctl.h headers in place of signal.h
    in our last program and declare some extra variables to deal with select.  */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <thread>
#include <iostream>
#include <cstring>

int socket_main(const char* ip_addr, int port);

using namespace std;
char qr_code[50] = {0};

int main()
{
	const char* ip_addr = "127.0.0.1";
	int port = 9734;
	std::thread t(socket_main, ip_addr, port);
	t.join();
	//socket_main(ip_addr, port);
}

int socket_main(const char* ip_addr, int port)
{
    int server_sockfd, client_sockfd;
    socklen_t server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int result;
    fd_set readfds, testfds;
    struct timeval timeout;

/*  Create and name a socket for the server.  */

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip_addr);
    server_address.sin_port = htons(port);
    server_len = sizeof(server_address);

    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

/*  Create a connection queue and initialize readfds to handle input from server_sockfd.  */

    listen(server_sockfd, 5);

    FD_ZERO(&readfds);
    FD_SET(server_sockfd, &readfds);

    std::cout << "\n\nCoRos Socket Server is now listening on: " << ip_addr << ":" << port << endl;;

/*  Now wait for clients and requests.
    Since we have passed a null pointer as the timeout parameter, no timeout will occur.
    The program will exit and report an error if select returns a value of less than 1.  */

    while(1) 
    {
        int fd;
        int nread;
        
        testfds = readfds;
        timeout.tv_sec = 2;
        timeout.tv_usec = 500000;
        timeout = {0}; // just for the sake of removing compile error

        //printf("server waiting\n");
        result = select(FD_SETSIZE, &testfds, (fd_set *)0, 
            (fd_set *)0, (struct timeval *)0);

        if(result < 1) 
        {
            perror("server5");
            exit(1);
        }
        else if(result == 0)
            printf("timeout\n");
        else{}

/*  Once we know we've got activity,
    we find which descriptor it's on by checking each in turn using FD_ISSET.  */

        for(fd = 0; fd < FD_SETSIZE; fd++) 
        {
            if(FD_ISSET(fd,&testfds)) 
            {

/*  If the activity is on server_sockfd, it must be a request for a new connection
    and we add the associated client_sockfd to the descriptor set.  */

                if(fd == server_sockfd) 
                {
                    client_len = sizeof(client_address);
                    client_sockfd = accept(server_sockfd, 
                        (struct sockaddr *)&client_address, &client_len);
                    FD_SET(client_sockfd, &readfds);
                    printf("adding client on fd %d\n", client_sockfd);
                }

/*  If it isn't the server, it must be client activity.
    If close is received, the client has gone away and we remove it from the descriptor set.
    Otherwise, we 'serve' the client as in the previous examples.  */

                else 
                {
                    ioctl(fd, FIONREAD, &nread);

                    if(nread == 0) 
                    {
                        close(fd);
                        FD_CLR(fd, &readfds);
                        printf("-----------------------------------\n");
                    }
                    else 
                    {
                        memset(qr_code, 0, 50);
                        read(fd, qr_code, 50);
                        printf("server rcvd: %s\n", qr_code);
                        printf("server sent: %s\n", qr_code);
                        write(fd, qr_code, 50);
                    }
                }
            }
        }
    }
}

