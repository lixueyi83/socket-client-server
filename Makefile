# Makefile for Beginning Linux Programming 3rd Edition
# Chapter 15

CLIENTS=client
SERVERS=server
INFO=getname getdate getdate-udp
SELECT=select

CC=g++
CPPFLAGS=-g -std=c++11 -pthread
LDFLAGS=-g

#ALL= $(CLIENTS) $(SERVERS) $(INFO) $(SELECT)
ALL= server client  

all: $(ALL)

clean:
	@rm -f $(ALL) *~ *.o
