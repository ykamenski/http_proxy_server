###############################################################################
# Project2, CPSC 5510 Networking, Seattle University
# Zach, David, Yevgeni Kamenski
# 
# Makefile - compiles the proxy program
# 
# This is free and unencumbered software released into the public domain.
###############################################################################

VPATH = src include
CPPFLAGS += -I include

EXECUTABLES=proxy
SOURCES:= HttpRequest.cpp\
          HttpResponse.cpp\
          AddrInfo.cpp\
          ListenConnection.cpp\
          ClientConnection.cpp\
          LineScanner.cpp\
          HttpHeaderBuilder.cpp\
          ForwardSocketData.cpp\
          HttpUri.cpp\
          helpers.cpp\
          proxy.cpp
          
OBJECTS=$(SOURCES:.cpp=.o)
CFLAGS=-Wall -pthread -std=c++1y

.PHONY: all
all: optimized

proxy: $(OBJECTS)
	g++ $(CFLAGS) $(CPPFLAGS) $(OBJECTS) -o $@

.INTERMEDIATE: optimized
optimized: proxy
optimized: CFLAGS+=-O3

.INTERMEDIATE: debug
debug: proxy
debug: CFLAGS += -g -D_DEBUG

# Header file dependencies
AddrInfo.o: AddrInfo.h
ListenConnection.o: ListenConnection.h
helpers.o: helpers.h

%.o : %.cpp
	g++ -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean: 
	rm -f $(EXECUTABLES) $(OBJECTS)
