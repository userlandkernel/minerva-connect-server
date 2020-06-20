all:
	make server

server:
	clang server.c variables.c core.c config.c rsasec.c -o ../bin/server -lpthread -lplist-2.0 -lssl -lcrypto

clean:
	rm -f ../bin/server
