all:
	make server

server:
	clang server.c variables.c core.c config.c -o ../bin/server -lpthread -lplist

clean:
	rm -f ../bin/server
