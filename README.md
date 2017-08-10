# socket-client-server

Simple examples of creating socket server and multiple clients in different ways, either by utilizing 
select which is more focuses on client request and server handle the request and then close the connection
or by creating corresponding thread to accept the connection request from different clients simultaneously 
and will always maintain the connection with those clients. The later more focuses on sending command from 
server to the clients while keeping the socket connection all the time. 
