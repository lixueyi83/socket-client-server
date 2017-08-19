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

/*

Select:

    Quite often when you’re writing Linux applications, you may need to examine the state of a number of
    inputs to determine the next action to take. For example, a communication program such as a terminal
    emulator needs to read the keyboard and the serial port effectively at the same time. In a single-user sys-
    tem, it might be acceptable to run in a “busy wait” loop, repeatedly scanning the input for data and
    reading it if it arrives. This behavior is expensive in terms of CPU time.

    The select system call allows a program to wait for input to arrive (or output to complete) on a num-
    ber of low-level file descriptors at once. This means that the terminal emulator program can block until
    there is something to do. Similarly, a server can deal with multiple clients by waiting for a request on
    many open sockets at the same time.

    The select function operates on data structures, fd_set , that are sets of open file descriptors. A number
    of macros are defined for manipulating these sets:

            #include <sys/types.h>
            #include <sys/time.h>

            void FD_ZERO(fd_set *fdset);
            void FD_CLR(int fd, fd_set *fdset);
            void FD_SET(int fd, fd_set *fdset);
            int FD_ISSET(int fd, fd_set *fdset);

    As suggested by their names, FD_ZERO initializes an fd_set to the empty set, FD_SET and FD_CLR set
    and clear elements of the set corresponding to the file descriptor passed as fd , and FD_ISSET returns
    nonzero if the file descriptor referred to by fd is an element of the fd_set pointed to by fdset . The
    maximum number of file descriptors in an fd_set structure is given by the constant FD_SETSIZE.

    The select function can also use a timeout value to prevent indefinite blocking. The timeout value is
    given using a struct timeval . This structure, defined in sys/time.h , has the following members:

            struct timeval 
            {
                time_t tv_sec;
                long   tv_usec;
            }

    The time_t type is defined in sys/types.h as an integral type.

    The select system call has the following prototype:
    
    #include <sys/types.h>
    #include <sys/time.h>

    int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout);

    A call to select is used to test whether any one of a set of file descriptors is ready for reading or 
    writing or has an error condition pending and will optionally block until one is ready

    The nfds argument specifies the number of file descriptors to be tested, and descriptors from 0 to nfds-1 
    are considered. Each of the three descriptor sets may be a null pointer, in which case the associated test
    isn’t carried out.

    The select function will return if any of the descriptors in the readfds set are ready for reading, if any
    in the writefds set are ready for writing, or if any in errorfds have an error condition. If none of these
    conditions apply, select will return after an interval specified by timeout . If the timeout parameter is
    a null pointer and there is no activity on the sockets, the call will block forever.

    When select returns, the descriptor sets will have been modified to indicate which descriptors are
    ready for reading or writing or have errors. You should use FD_ISSET to test them, to determine the
    descriptor(s) needing attention. You can modify the timeout value to indicate the time remaining until
    the next timeout, but this behavior isn’t specified by X/Open. In the case of a timeout occurring, all
    descriptor sets will be empty.

    The select call returns the total number of descriptors in the modified sets. It returns –1 on failure,
    setting errno to describe the error. Possible errors are EBADF for invalid descriptors, EINTR for return
    due to interrupt, and EINVAL for bad values for nfds or timeout .
*/

int socket_main(const char* ip_addr, int port);

using namespace std;
char qr_code[50] = "Where there is a will, there is a way.";
const char reply[50] = "Where there is a will, there is a way.";

int main()
{
	const char* ip_addr = "127.0.0.1";
	int port = 10002;
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
        //timeout = {0};      /* just for the sake of removing compile error */

        printf("server waiting\n");
        result = select(FD_SETSIZE, &testfds, (fd_set *)0, (fd_set *)0, (struct timeval *)&timeout);

        switch(result) 
        {
            case 0:
                printf("select timeout\n");
                return 0;

            case -1:
                perror("select faild or error happened");
                exit(1);
        }

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
                        printf("server sent: %s\n", reply);
                        write(fd, reply, 50);
                    }
                }
            }
        }
    }
}

