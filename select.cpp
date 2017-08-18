/*  Begin as usual with the includes and declarations
    and then initialize inputs to handle input from the keyboard.  */

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>


/*---------------------------------------------------------------------------------------------------------

Select:

    select() is level-triggered, not edge-triggered. When you pass it a set of file descriptors, 
    it will come back telling you which ones are readable/writable/exceptional at the moment, 
    not just the ones that have changed state recently.

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

const int std_input_fd    = 0;
const int std_output_fd   = 1;
const int std_error_fd    = 2;

int main()
{
    char buffer[128];
    int result, nread;

    fd_set inputs, testfds;
    struct timeval timeout;

    FD_ZERO(&inputs);
    FD_SET(0,&inputs);

    /*  
    *   Wait for input on stdin for a maximum of 2.5 seconds.  
    */
    while(1) 
    {
        testfds = inputs;
        timeout.tv_sec = 2;
        timeout.tv_usec = 500000;

        /*
        *   wait at most 2.5 seconds and then return
        */
        result = select(FD_SETSIZE, &testfds, (fd_set *)0, (fd_set *)0, &timeout); 

        /*  
        *   After this time, we test result. If there has been no input, the program loops again. 
        *   If there has been an error, the program exits.  
        */
        switch(result) 
        {
            case 0:
                printf("select timeout\n");
                break;

            case -1:
                perror("select faild or error happened");
                exit(1);

            /*  If, during the wait, we have some action on the file descriptor,
            *   we read the input on stdin and echo it whenever an <end of line> character is received,
            *   until that input is Ctrl-D.  
            */
            default:
                if(FD_ISSET(std_input_fd,&testfds)) 
                {
                    ioctl(std_input_fd,FIONREAD,&nread);
                    if(nread == 0) 
                    {
                        printf("keyboard done\n");
                        exit(0);
                    }
                    nread = read(std_input_fd,buffer,nread);
                    buffer[nread] = 0;
                    printf("read %d from keyboard: %s", nread, buffer);
                }
                break;
        }
    }
}

