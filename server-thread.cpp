/*  Make the necessary includes and set up the variables.  */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <thread> 

#include <iostream>
using namespace std;


bool check_socket_connection(int client_sockfd)
{
    bool ret = false;
    char recv_buf = 0;
    unsigned char heart_beat_cmd[2] = {0xaa,0x55};

    write(client_sockfd, &heart_beat_cmd, 2);
    read(client_sockfd, &recv_buf, 1);

    if(0xa5 == (recv_buf&0xff))
    {
        cout << "heart_beat_cmd ACK 0xa5 rcvd." << endl;
        ret = true;
    }
    else 
    {
        ret = false;
    }

    return ret;
}


void client_handler(int client_sockfd)
{
	char recv_buf[10];
	char send_buf[10] = "ping";

    /*  We can now read/write to client on client_sockfd.  */
    while(1)
    {    
	    static int cnts = 0; 
	    cnts++;
	    printf("server is connected with client_sockfd = %d ---------- %d\n", client_sockfd, cnts);

	    char cmd;
	    memset(recv_buf, 0, sizeof(char));

        if(!check_socket_connection(client_sockfd))
        {
            cout << "socket connection failed." << endl;
	        close(client_sockfd);
            return; 
        }

    	write(client_sockfd, &send_buf, 10);
	    printf("server-send msg: %s\n", send_buf);
	    read(client_sockfd, &recv_buf, 10);
	    printf("server-rcvd msg: %s\n", recv_buf);
	
	    sleep(1);
    }   
    close(client_sockfd);
}

int main()
{
    int server_sockfd, client_sockfd;
    socklen_t server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    /*  Remove any old socket and create an unnamed socket for the server.  */

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /*  Name the socket.  */

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(10002);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    /*  Create a connection queue and wait for clients.  */

    listen(server_sockfd, 5);
    
    while(1) 
    {
        printf("socket server waiting for connection request from socket clients...\n");

        /*  Accept a connection.  */

        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

        cout << "new client with client_sockfd " << client_sockfd << " is connected." << endl;

        std::thread th(client_handler, client_sockfd);
        th.detach();
    }

    return 0;
}


