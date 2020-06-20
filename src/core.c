/**
 * Minerva Server - Core
**/

#include <plist/plist.h>
#include "core.h"
#include "config.h"
#include "variables.h"


char* MVCSIPv4String(struct sockaddr_in addr) {
	char* str = malloc(INET_ADDRSTRLEN);
	bzero(str, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &addr.sin_addr, str, INET_ADDRSTRLEN); 
	return str;
}


MVCSClient* MVCSClientCreate(int fd, struct sockaddr_in addr, socklen_t len) {
	MVCSClient* client = malloc(sizeof(MVCSClient));
	bzero(client, sizeof(MVCSClient));
	client->fd = fd;
	memcpy(&client->addr, &addr, sizeof(struct sockaddr_in));
	client->len = len;
	return client;
}

char* MVCSStatusString(unsigned int statusCode) {
	switch(statusCode) {
		case 200:
			return "OK";
		case 400:
			return "Bad request";
		case 404:
			return "Not found";
		case 403:
			return "Forbidden";
		case 418:
			return "I'm a teapot";
		case 500:
			return "Internal server error";
		default:
			return "Unknown";
	}
}

void MVCSRespond(MVCSClient* client, unsigned int statusCode, const char* contentType, char *content, unsigned int contentLength) {
	
	FILE* stream = fdopen(client->fd, "r+");
	fprintf(stream, "HTTP/1.1 %d %s\n", statusCode, MVCSStatusString(statusCode));
	fprintf(stream, "Server: %s\n", MVCS_SERVER_TITLE);
	fprintf(stream, "Content-Type: %s\n", contentType);
	fprintf(stream, "Content-Length: %d\n\r\n", contentLength);

	if(content) {
		fprintf(stream, "%s", content);
	}

	fclose(stream);
}

void MVCSHTTPBadRequest(MVCSClient* client) {
	char* err = "Minerva Connect Server - Bad request (400).<br/>";
	MVCSRespond(client, 400, "text/html", err, strlen(err));
}

void MVCSHTTPBadRequestCustom(MVCSClient* client, const char* err) {
	MVCSRespond(client, 400, "text/html", (char*)err, strlen(err));
}

void MVCSHTTPInternalError(MVCSClient* client) {
	char* err = "Minerva Connect Server - Internal server error (500).<br/>";
	MVCSRespond(client, 500, "text/html", err, strlen(err));
}

void MVCSHTTPInternalErrorCustom(MVCSClient* client, const char* err) {
	MVCSRespond(client, 500, "text/html", (char*)err, strlen(err));
}

void MVCSHTTPNotFound(MVCSClient* client) {
	char* err = "Minerva Connect Server - Document not found (404).<br/>";
	MVCSRespond(client, 404, "text/html", err, strlen(err));
}


void MVCSHTTPNotFoundCustom(MVCSClient* client, const char* err) {
	MVCSRespond(client, 404, "text/html", (char*)err, strlen(err));
}
void MVCSHTTPForbidden(MVCSClient* client) {
	char* err = "Minerva Connect Server - Forbidden (403).<br/>";
	MVCSRespond(client, 403, "text/html", err, strlen(err));
}

void MVCSHTTPForbiddenCustom(MVCSClient* client, const char* err) {
	MVCSRespond(client, 403, "text/html", (char*)err, strlen(err));
}

void MVCSHTTPRouter(MVCSClient* client, char *HTTP_PROTOCOL, char *HTTP_METHOD, char *HTTP_URI, char* HTTP_QUERY_STRING) {

	// Unsupport post requests
	if(!strcmp(HTTP_METHOD, "POST")) {
		return MVCSHTTPBadRequestCustom(client, "Minerva Connect Server currently does not support http post requests.\n");
	}

	// Mark all requests that are not GET as invalid
	else if(strcmp(HTTP_METHOD, "GET")) {
		return MVCSHTTPBadRequest(client);
	}

	// Get current directory
	char cwd[PATH_MAX];
   	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("getcwd() error");
		MVCSHTTPInternalError(client);
		return;
	}

	// Get full document path
	char DOCUMENT_PATH[PATH_MAX];
	int len = strlen(cwd);
	len += strlen("/html/");
	len += strlen(HTTP_URI);
	snprintf(DOCUMENT_PATH, len, "%s/html%s", cwd, HTTP_URI);

	struct stat statbuf;
    if (stat(DOCUMENT_PATH, &statbuf) != 0) {
       	char MESSAGE[strlen(DOCUMENT_PATH)+strlen(" does not exist.<br/>")+1];
		snprintf(MESSAGE, sizeof(MESSAGE), "%s does not exist.<br/>", HTTP_URI);
		return MVCSHTTPNotFoundCustom(client, MESSAGE);
    }
   	
   	if(S_ISDIR(statbuf.st_mode)) {
   		len += strlen("/index.mvcs");
   		snprintf(DOCUMENT_PATH, len, "%s/html%s/index.mvcs", cwd, HTTP_URI);
   	}

	// Get file extension
	const char* extension = "";
	const char *dot = strrchr(DOCUMENT_PATH, '.');
    if(!dot || dot == DOCUMENT_PATH) {
    	extension = "";
    } else {
    	extension = dot+1;
    }
    

	// Read the file
	FILE* fp = fopen(DOCUMENT_PATH, "rb");
	if(!fp) {
		char MESSAGE[strlen(DOCUMENT_PATH)+strlen("Minerva Connect Server -  does not exist.<br/>")+1];
		snprintf(MESSAGE, sizeof(MESSAGE), "Minerva Connect Server - %s does not exist.<br/>", HTTP_URI);
		return MVCSHTTPNotFoundCustom(client, MESSAGE);
	}

	fseek(fp, 0, SEEK_END);
	size_t contentLength = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	printf("Serving file %s (%zu bytes)...\n", DOCUMENT_PATH, contentLength);

	char content[contentLength+1];
	bzero(content, contentLength+1);

	fread(content, contentLength+1, 1, fp);
	fclose(fp);

    if(!strcmp(extension, "html")) {
    	return MVCSRespond(client, 200, "text/html", content, contentLength);
    }
    else if(!strcmp(extension, "mvcs")) {

    	char* tmp = NULL;
    	char *replaced = NULL;

    	// Replace PGP auth message variables
    	replaced = replacevar(content, "$$SERVER_PGP_AUTH_MESSAGE$$", mvcs_var_pgp_authcode());

    	// Backup pointer to prevent memleak
    	tmp = replaced;

    	// Replace year variable
    	replaced = replacevar(replaced, "$$YEAR$$", mvcs_var_year());

    	// Free the old string
    	if(tmp) {
    		free(tmp);
    		tmp = NULL;
    	}

    	// Back up pointer to prevent memleak
    	tmp = replaced;

    	//  Replace title variable
    	replaced = replacevar(replaced, "$$TITLE$$", MVCS_SERVER_TITLE);

    	// Free the old string
    	if(tmp) {
   			free(tmp);
   			tmp = NULL;
    	}

    	MVCSRespond(client, 200, "text/html", replaced, strlen(replaced));

    	// Free the replaced string

    	if(replaced) {
    		free(replaced);
    		replaced = NULL;
    	}
    	

    	return;
    }
    else if(!strcmp(extension, "css") || !strcmp(HTTP_URI, "/")) {
    	return MVCSRespond(client, 200, "text/css", content, contentLength);
    }
    else if(!strcmp(extension, "png") || !strcmp(HTTP_URI, "/")) {
    	return MVCSRespond(client, 200, "image/png", content, contentLength);
    }
    if(!strcmp(extension, "jpg") || !strcmp(HTTP_URI, "/")) {
    	return MVCSRespond(client, 200, "image/jpeg", content, contentLength);
    }
    else {
    	return MVCSRespond(client, 200, "application/octet-stream", content, contentLength);
    }

    MVCSRespond(client, 500, "text/html", "Debug hack", strlen("Debug hack"));

}

void MVCSHTTPController(MVCSClient* client) {

	char HTTP_HEADER[8192];
	unsigned int cnt = recv(client->fd, HTTP_HEADER, 8192, 0);

	// Header too small
	if(cnt <= 10)
		return MVCSHTTPBadRequest(client);

	HTTP_HEADER[cnt] = '\0';

	// Get HTTP Method
	char *HTTP_METHOD = strtok(HTTP_HEADER, " \t\r\n");
	if(!HTTP_METHOD) {
		return MVCSHTTPBadRequest(client);
	}
	else {
		printf("METHOD: %s\n", HTTP_METHOD);
	}

	// Get HTTP URI
	char *HTTP_URI = strtok(NULL, " \t");

	// Get HTTP Protocol
	char *HTTP_PROTOCOL = strtok(NULL, " \t\r\n");
	if(!HTTP_PROTOCOL) {
		return MVCSHTTPBadRequest(client);
	} else {
		printf("PROTOCOL: %s\n", HTTP_PROTOCOL);
	}

	// Get HTTP QS
	char *HTTP_QUERY_STRING = NULL;
	if((HTTP_QUERY_STRING = strchr(HTTP_URI, '?'))) {
		*HTTP_QUERY_STRING++ = '\0';
	}
	else {
		HTTP_QUERY_STRING = HTTP_URI - 1;
	}
	if(HTTP_QUERY_STRING) {
		printf("QUERY: %s\n", HTTP_QUERY_STRING);
	}

	HTTP_URI = strtok(HTTP_URI, "?");
	if(HTTP_URI) {
		printf("URI: %s\n", HTTP_URI);
	}


	return MVCSHTTPRouter(client, HTTP_PROTOCOL, HTTP_METHOD, HTTP_URI, HTTP_QUERY_STRING);
}

void MVCSClientThread(MVCSClient* client) {
	MVCSHTTPController(client);
	pthread_exit(NULL);
}

int mvcs_start(const char *ip, const char* port, const bool verbose) {

	int mvcs_fd = -1;

	struct sockaddr_in clientAddr = {};
	socklen_t clientLen = 0;
	int clientFd = 0;

	signal(SIGCHLD,SIG_IGN);

	struct addrinfo hints = {};
	struct addrinfo *res = NULL;
	struct addrinfo *p = NULL;

	// getaddrinfo for host
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	printf("Starting server on port %s:%s...\n", ip, port);

	if( getaddrinfo(NULL, port, &hints, &res) != 00)
	{
		perror("getaddrinfo() error");
		exit(1);
	}
	for(p = res; p; p=p->ai_next)
	{
		int opt = 1;
		mvcs_fd = socket(p->ai_family, p->ai_socktype, 0);
		setsockopt(mvcs_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		if(mvcs_fd == -1) continue;
		if( bind(mvcs_fd, p->ai_addr, p->ai_addrlen) == 0) break;
	}
	if(!p) {
		perror("socket() or bind()");
		exit(1);
	}

	if(res) {
		freeaddrinfo(res);
		res = NULL;
	}

	if( listen(mvcs_fd, 1000000) != 0 )
	{
		perror("listen() error");
		exit(1);
	}

	while(1)
	{
		clientLen = sizeof(clientAddr);
		clientFd = accept(mvcs_fd, (struct sockaddr*)&clientAddr, &clientLen);
		if(clientFd >= 0) {

			MVCSClient* client = MVCSClientCreate(clientFd, clientAddr, clientLen);

			printf("Client connected: %s\n", MVCSIPv4String(client->addr));

			pthread_t clientThread;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_create(&clientThread, &attr, (void*)&MVCSClientThread, client);
			
		}
		else {
			perror("accept() error");
		}
	}
	return 1;
}

