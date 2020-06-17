#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h> 

typedef struct MVCSClient {
	int fd;
	struct sockaddr_in addr;
	socklen_t len;
} MVCSClient;

typedef enum mvc_err {
	MVC_ERR_OK = 0,
	MVC_ERR_FAIL = 1,
	MVC_ERR_UNKNOWN = 2
} mvc_err_t;

int mvcs_start(const char *ip, const char* port, const bool verbose);