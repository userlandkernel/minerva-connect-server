#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "core.h"

bool sVerbose = false;
char *sIP = NULL;
char *sPort = "80";

void usage(const int argc, const char* argv[]) {

	printf("MVC-Server v1.0-alpha (Azure-Blue)\n");
	printf("------------------------------------\n");
	printf("  -p\t\t Port to serve on\n");
	printf("  -s\t\t IP to bind to\n");
	printf("  -v\t\t Verbose logging\n");
}

int main(const int argc, const char* argv[])  {

	sIP = getenv("VHOST") ? getenv("VHOST") : "0.0.0.0";
	sPort = getenv("VPORT") ? getenv("VPORT") : "80";
	sVerbose = (bool)(getenv("VERBOSE") ? 1 : 0);


	if(sVerbose) {
		printf("Starting mvc-server on port %s (%s)...\n", sPort, sIP);
	}

	return mvcs_start(sIP, sPort, sVerbose);
}
