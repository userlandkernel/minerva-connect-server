all: server
	make clean

server:
	clang server.c variables.c core.c -o ../bin/server -lpthread

clean:
	rm ../bin/server
