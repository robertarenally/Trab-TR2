CC=g++
CFLAGS= -g -Wall -Werror

all: proxy

proxy: web_proxy_server.cpp
	$(CC) $(CFLAGS) -o proxy_parse.o -c analyzer_web_proxy.cpp
	$(CC) $(CFLAGS) -o proxy.o -c web_proxy_server.cpp
	$(CC) $(CFLAGS) -o proxy proxy_parse.o proxy.o 
	$(CC) -o spider spider.cpp
	$(CC) -o my_wget cliente_recursivo.cpp
	

clean:
	rm -f proxy *.o

tar:
	tar -cvzf ass1.tgz proxy README Makefile analyzer_web_proxy.c analyzer_web_proxy.h
