/*  Make the necessary includes and set up the variables.  */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <thread> 

#include <iostream>
using namespace std;


typedef enum
{
	CMD_TYPE_1,
	CMD_TYPE_2,
	CMD_TYPE_3,
	CMD_TYPE_NONE
}CMD_TYPE;

CMD_TYPE pbl_cmd = CMD_TYPE_1;

typedef struct
{
	short cmd_type;
	short x;
	short y;
	short z;
}CMD_1;

CMD_1 ptz_cmd = {CMD_TYPE_1, 200, 200, 200};

typedef struct
{
	short cmd_type;
	short index;
	short begin;
	short end;
	short color;
}CMD_2;

CMD_2 led_cmd = {CMD_TYPE_2, 0, 10, 15, 2};

typedef struct
{
	char cmd_type;
	char cmd;
}CMD_3;

CMD_3 show_cmd = {CMD_TYPE_3, 1};

char read_buf[10];
char send_buf[10] = "ping";

void wait_for_paramters()
{
	int cmd;
	
	cout << "enter pbl cmd: 0 for PTZ, 1 for LED, 2 for SHOW: ";
	cin >> cmd;
	pbl_cmd = (CMD_TYPE)cmd;
	
   switch(pbl_cmd)
   {
   		case CMD_TYPE_1:
   		{
   			int x,y,z;
			cout << "enter x, y, z: " << endl;
			cin >> x >> y >> z;
			ptz_cmd.cmd_type = pbl_cmd;
			ptz_cmd.x = x;
			ptz_cmd.y = y;
			ptz_cmd.z = z;
			
			cout << "ptz_cmd: " << ptz_cmd.cmd_type << " " 
				<< ptz_cmd.x << " " 
				<< ptz_cmd.y << " "
				<< ptz_cmd.z << endl;
   			break;
   		}
   			
   		case CMD_TYPE_2:
   		{
			short index, begin, end, color;
			cout << "enter index, begin-led, end-led, color: " << endl;
			cin >> index >> begin >> end >> color;
			
			led_cmd.cmd_type = pbl_cmd;
			led_cmd.index = index;
			led_cmd.begin = begin;
			led_cmd.end = end;
			led_cmd.color = color;
			
			cout << "led_cmd: " << led_cmd.cmd_type << " " 
				<< led_cmd.index << " "
				<< led_cmd.begin << " "
				<< led_cmd.end << " "
				<< led_cmd.color << endl;
   			break;
   		}
   			
   		case CMD_TYPE_3:
   		{
			cout << "enter show command: 0:off; 1:on: " << endl;
			cin >> show_cmd.cmd;
   			break;
   		}
   			
   		default:  
   		{     		
   			cout << "cmd invalid" << endl;	         			
   			pbl_cmd = CMD_TYPE_NONE;
   			break;
   		}
   	}
}



void client_handler(int client_sockfd)
{
    /*  We can now read/write to client on client_sockfd.  */
    while(1)
    {   
#if 1
        wait_for_paramters();
       		       
        switch(pbl_cmd)
        {
           	case CMD_TYPE_1:
           	{
           		write(client_sockfd, &ptz_cmd, sizeof(CMD_1));
           		unsigned char* p1 = (unsigned char*)&ptz_cmd;
           		for(int i=0; i<sizeof(CMD_1); i++)
           			printf("server-send: %d\n", p1[i]);
           		break;
           	}
           			
           	case CMD_TYPE_2:
           	{
           		write(client_sockfd, &led_cmd, sizeof(CMD_2));
           		unsigned char* p2 = (unsigned char*)&led_cmd;
           		for(int i=0; i<sizeof(CMD_2); i++)
           			printf("server-send: %d\n", p2[i]);
           		break;
           	}
           			
           	case CMD_TYPE_3:
           	{
           		write(client_sockfd, &show_cmd, sizeof(CMD_3));
           		unsigned char* p3 = (unsigned char*)&show_cmd;
           		for(int i=0; i<sizeof(CMD_3); i++)
           			printf("server-send: %d\n", p3[i]);
           		break;
           	}
           			
           	default:       			         			
           		pbl_cmd = CMD_TYPE_NONE;
           		break;
        }
#endif 
        write(client_sockfd, &send_buf, 10);
        printf("server-send: %s\n", send_buf);
        read(client_sockfd, &read_buf, 10);
        printf("server-rcvd: %s\n", read_buf);
    }   
    //close(client_sockfd);
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
    server_address.sin_addr.s_addr = inet_addr("192.168.1.144");
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

        std::thread th(client_handler, client_sockfd);
        th.detach();
    }

    return 0;
}


