all:
	make clean
	make server

server:
	clang server.c variables.c core.c -o ../bin/server -lpthread

clean:
	rm -f ../bin/server
