# Makefile for Beginning Linux Programming 3rd Edition
# Chapter 15

CLIENTS=client-thread client-select client-thread-1 client-thread-2 client-thread-3 
SERVERS=server-thread server-select server-thread-cmd  
INFO=getname getdate getdate-udp
SELECT=select

CC=g++
CPPFLAGS=-g -std=c++11 -pthread
LDFLAGS=-g

ALL= $(CLIENTS) $(SERVERS)  $(SELECT)

all: $(ALL)

clean:
	@rm -f $(ALL) *~ *.o
