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
#include <signal.h>

#include <iostream>
using namespace std;

int g_client1_sockfd = 0;
int g_client2_sockfd = 0;
int g_client3_sockfd = 0;

bool check_socket_connection(int server_sockfd)
{
    unsigned char recv_buf[2];
    socklen_t client_len;
    struct sockaddr_in client_address;
    int client_sockfd;

    while(1)
    {
        /*  Accept a connection.  */
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

        read(client_sockfd, &recv_buf, 2);
        printf("recv_buf = %x %x\n", recv_buf[0], recv_buf[1]);

        if(0xa6 == (recv_buf[0]&0xff))
        {
            if(0x51 == (0xff & recv_buf[1]))
            {
                if(g_client1_sockfd == 0)
                {
                    g_client1_sockfd = client_sockfd;
                }
                else if(g_client1_sockfd > 0 && g_client1_sockfd != client_sockfd)
                {
                    close(g_client1_sockfd);
                    g_client1_sockfd = client_sockfd;
                }
                else{}
                
            }
            else if(0x52 == (0xff & recv_buf[1]))
            {
                if(g_client2_sockfd == 0)
                {
                    g_client2_sockfd = client_sockfd;
                }
                else if(g_client2_sockfd > 0 && g_client2_sockfd != client_sockfd)
                {
                    close(g_client2_sockfd);
                    g_client2_sockfd = client_sockfd;
                }
                else{}
            }
            else if(0x53 == (0xff & recv_buf[1]))
            {
                if(g_client3_sockfd == 0)
                {
                    g_client3_sockfd = client_sockfd;
                }
                else if(g_client3_sockfd > 0 && g_client3_sockfd != client_sockfd)
                {
                    close(g_client3_sockfd);
                    g_client3_sockfd = client_sockfd;
                }
                else{}
            }
            else
            {
                cout << "\t *** unexpected client request.";
            }
        } 
        else{}
    }
}


void clients_handler()
{
    /*  We can now read/write to client on client_sockfd.  */
    while(1)
    {    
        if(g_client1_sockfd)
        {
            char send_buf[10] = "Hi, Tony!";
            write(g_client1_sockfd, &send_buf, 10);
            cout << "send command to client 1 ------ client_sockfd =  " << g_client1_sockfd << endl;
        }

        if(g_client2_sockfd)
        {
            char send_buf[10] = "Hi, Lee!";
            write(g_client2_sockfd, &send_buf, 10);
            cout << "send command to client 2 ------ client_sockfd =  " << g_client2_sockfd << endl;
        }

        if(g_client3_sockfd)
        {
            char send_buf[10] = "Hi, Nola!";
            write(g_client3_sockfd, &send_buf, 10);
            cout << "send command to client 3 ------ client_sockfd =  " << g_client3_sockfd << endl;
        }

	    sleep(1);
    }   
}

int main()
{
    signal(SIGPIPE, SIG_IGN);

    int server_sockfd, client_sockfd;
    socklen_t server_len, client_len;
    struct sockaddr_in server_address;
    
    const char* ip = "127.0.0.1";
    int port = 10005;

    /*  Remove any old socket and create an unnamed socket for the server.  */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /*  Name the socket.  */
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(port);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    /*  Create a connection queue and wait for clients.  */
    listen(server_sockfd, 5);

    printf("socket server is listenning on %s:%d\n", ip, port);

    std::thread th1(check_socket_connection, server_sockfd);
    //th1.detach();

    std::thread th2(clients_handler);
    //th2.detach();

    //th1.join();
    //th2.join();

    while(1){}

    return 0;
}


/*  check link: http://www.2cto.com/os/201510/445453.html
 *  int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,struct timeval *timeout);
 */ 

/**************************************************************************
 *  read_timeout: read timeout detection, does not include read operation
 *  return value: 0, success; -1, fail and set errno = ETIMEDOUT
*/
int read_timeout(int fd, int sec)
{
    int ret = 0;
    if(sec > 0)
    {
        fd_set read_set;
        FD_ZERO(&read_set);     /* clear all bits within read_set */
        FD_SET(fd, &read_set);  /* set cooresponding bits within read_set */

        struct timeval wait_time;
        wait_time.tv_sec = sec;
        wait_time.tv_usec = 0;
        do
        {
            ret = select(fd+1, &read_set, NULL, NULL, &wait_time);
        }
        while(ret < 0 && errno == EINTR); /* if interrupted, restart select */

        if(ret == 0)
        {
            ret = -1;
            errno = ETIMEDOUT;
        }
        else if(ret == 1)
        {
            ret = 0;
        }
        else {}
    }

    return ret;
}

/**************************************************************************
 *  write_timeout: write_timeout detection, does not include write operation
 *  return value: 0, success; -1, fail and set errno = ETIMEDOUT
*/
int write_timeout(int fd, int sec)  
{
    int ret = 0;  
    if (sec > 0)  
    {  
        fd_set write_set;  
        FD_ZERO(&write_set);      /* clear all the bits */ 
        FD_SET(fd,&write_set);    /* set cooresponding bits */
   
        struct timeval wait_time;  
        wait_time.tv_sec = sec;  
        wait_time.tv_usec = 0;  
        do 
        {  
            ret = select(fd+1,NULL,&write_set,NULL,&wait_time);  
        } 
        while(ret < 0 && errno == EINTR); /* waiting for signal */ 
   
        if(ret == 0)       /* within wait_time, no events came */ 
        {  
            ret = -1;   
            errno = ETIMEDOUT;  
        }  
        else if(ret == 1)  /* within wait_time, event came */ 
        {
            ret = 0;    /* return success */
        } 
        else {}
    }  
   
    return ret;  
}

/**************************************************************************
 *  accept_timeout: accept_timeout detection
 *  return value: 0, success; -1, fail and set errno = ETIMEDOUT
*/
int accept_timeout(int fd, struct sockaddr_in *addr, int sec)  
{  
    int ret = 0;  
    if (sec > 0)  
    {  
        fd_set accept_set;  
        FD_ZERO(&accept_set);  
        FD_SET(fd,&accept_set);    
   
        struct timeval wait_time;  
        wait_time.tv_sec = sec;  
        wait_time.tv_usec = 0;  

        do 
        {  
            ret = select(fd+1,&accept_set,NULL,NULL,&wait_time);  
        }  
        while(ret < 0 && errno == EINTR);  
   
        if(ret == 0)       /* within wait_time, no events came */  
        {   
            errno = ETIMEDOUT;  
            return -1;
        }  
        else if(ret == -1)
        {
            return -1;  
        } 
        else {}
    }
   
    /**select正确返回: 
        表示有select所等待的事件发生:对等方完成了三次握手, 
        客户端有新的链接建立,此时再调用accept就不会阻塞了 
    */ 
    socklen_t socklen = sizeof(struct sockaddr_in);  
    if (addr != NULL)  
    {
        ret = accept(fd,(struct sockaddr *)addr,&socklen);  
    }
    else 
    {
        ret = accept(fd,NULL,NULL);  
    }
   
    return ret;  
}

#if 0
/**************************************************************************
 *  connection_timeout: connection_timeout detection
 *  return value: 0, success; -1, fail and set errno = ETIMEDOUT
*/
 /* activate_nonblock - 设置IO为非阻塞模式 
 * fd: 文件描述符 
 */
void  activate_nonblock( int  fd) 
{ 
     int  ret; 
     int  flags = fcntl(fd, F_GETFL); 
     if  (flags == - 1 ) 
        ERR_EXIT( "fcntl error" ); 
 
    flags |= O_NONBLOCK; 
    ret = fcntl(fd, F_SETFL, flags); 
     if  (ret == - 1 ) 
        ERR_EXIT( "fcntl error" ); 
} 
 
/* deactivate_nonblock - 设置IO为阻塞模式 
 * fd: 文件描述符 
 */
void  deactivate_nonblock( int  fd) 
{ 
     int  ret; 
     int  flags = fcntl(fd, F_GETFL); 
     if  (flags == - 1 ) 
        ERR_EXIT( "fcntl error" ); 
 
    flags &= ~O_NONBLOCK; 
    ret = fcntl(fd, F_SETFL, flags); 
     if  (ret == - 1 ) 
        ERR_EXIT( "fcntl error" ); 
} 
 
/* connect_timeout - 带超时的connect 
 * fd: 套接字 
 * addr: 输出参数，返回对方地址 
 * wait_seconds: 等待超时秒数，如果为0表示正常模式 
 * 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT 
 */
int  connect_timeout( int  fd,  struct  sockaddr_in *addr,  unsigned   int  wait_seconds) 
{ 
     int  ret; 
    socklen_t addrlen =  sizeof ( struct  sockaddr_in); 
 
     if  (wait_seconds >  0 ) 
        activate_nonblock(fd); 
 
    ret = connect(fd, ( struct  sockaddr *)addr, addrlen); 
     if  (ret <  0  && errno == EINPROGRESS) 
    { 
 
        fd_set connect_fdset; 
         struct  timeval timeout; 
        FD_ZERO(&connect_fdset); 
        FD_SET(fd, &connect_fdset); 
 
        timeout.tv_sec = wait_seconds; 
        timeout.tv_usec =  0 ; 
 
         do
        { 
             /* 一旦连接建立，套接字就可写 */
            ret = select(fd +  1 ,  NULL , &connect_fdset,  NULL , &timeout); 
        } 
         while  (ret <  0  && errno == EINTR); 
 
         if  (ret ==  0 ) 
        { 
            errno = ETIMEDOUT; 
             return  - 1 ; 
        } 
         else   if  (ret <  0 ) 
             return  - 1 ; 
 
         else   if  (ret ==  1 ) 
        { 
             /* ret返回为1，可能有两种情况，一种是连接建立成功，一种是套接字产生错误 
             * 此时错误信息不会保存至errno变量中（select没出错）,因此，需要调用 
             * getsockopt来获取 */
             int  err; 
            socklen_t socklen =  sizeof (err); 
             int  sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen); 
             if  (sockoptret == - 1 ) 
                 return  - 1 ; 
             if  (err ==  0 ) 
                ret =  0 ; 
             else
            { 
                errno = err; 
                ret = - 1 ; 
            } 
        } 
    } 
 
     if  (wait_seconds >  0 ) 
        deactivate_nonblock(fd); 
 
     return  ret; 
}

#endif 

/*---------------------------------------------------------------------------------------------------------

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
    
--------------------------------------------------------------------------------------------------------------*/
