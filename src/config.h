#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <plist/plist.h>

typedef struct MVCSConfig {
	plist_t server;
	plist_t doctype;
	plist_t template;
} MVCSConfig;

#define MVCS_SERVER_TITLE "Minerva Connect Server"
#define MVCS_HTTP404_FILE "404.html"
#define MVCS_HTTP500_FILE "500.html"
#define MVCS_HTTP403_FILE "403.html"
