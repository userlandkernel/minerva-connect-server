#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <plist/plist.h>
#include "config.h"
#include "rsasec.h"

MVCSConfig MVCS_CONFIG = {};


plist_t MVCSLoadConfFile(const char *name) {

	plist_t conf = NULL;

	// Get current directory
	char cwd[PATH_MAX];
	bzero(cwd, PATH_MAX);
	getcwd(cwd, PATH_MAX);


	// Calculate full path size
	size_t len = strlen(cwd);
	len += strlen("/conf/");
	len += strlen(name);
	len++;

	// May not exceed OS limit
	if(len > PATH_MAX) {
		return NULL;
	}

	// Build full path
	char confpath[len];
	bzero(confpath, len);
	snprintf(confpath, len, "%s/conf/%s", cwd, name);

	// Open the config file
	FILE *fp = fopen(confpath, "rb");
	if(!fp) {
		fprintf(stderr, "Failed to open config file: %s\n", confpath);
		return NULL;
	}

	// Get the size of the file
	size_t size = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Allocate buffer for the file size
	char buffer[size+1];

	// Read config into buffer
	fread(buffer, size, 1, fp);

	// Close file again
	fclose(fp);

	// Parse config
	plist_from_memory(buffer, size, &conf);

	if(!conf) {
		fprintf(stderr, "Failed to parse config: %s\n", confpath);
		return NULL;
	}

	return conf;
}


bool MVCSLoadServerConf(void) {

	// Load server configuration
	MVCS_CONFIG.server = MVCSLoadConfFile("server.plist");
	if(!MVCS_CONFIG.server)
		return false;

	// Validate that hosting key exists
	plist_t _hosting = plist_dict_get_item(MVCS_CONFIG.server, "HOSTING");
	if(!_hosting) {
		fprintf(stderr, "server.plist is invalid. (reason = missing required dictionary key 'HOSTING'\n");
		return false;
	}

	// Validate that srvhost key exists
	plist_t _srvhost = plist_dict_get_item(_hosting, "SRVHOST");
	if(!_srvhost) {
		fprintf(stderr, "server.plist is invalid. (reason = missing required entry 'SRVHOST' for key 'HOSTING'\n");
		return false;
	}

	plist_t _srvport = plist_dict_get_item(_hosting, "SRVPORT");
	if(!_srvport) {
		fprintf(stderr, "server.plist is invalid. (reason = missing required entry 'SRVPORT' for key 'HOSTING'\n");
		return false;
	}

	return true;
}


bool MVCSLoadDoctypeConf(void) {

	// Load doctype handler configuration
	MVCS_CONFIG.doctype = MVCSLoadConfFile("doctypes.plist");
	if(!MVCS_CONFIG.doctype)
		return false;

	// Validate that doctype key exists
	plist_t _doctype = plist_dict_get_item(MVCS_CONFIG.doctype, "DOCTYPE");
	if(!_doctype) {
		fprintf(stderr, "doctypes.plist is invalid. (reason = missing required dictionary key 'DOCTYPE'\n");
		return false;
	}

	return true;
}

bool MVCSLoadTemplateConf(void) {

	// Load template configuration
	MVCS_CONFIG.template = MVCSLoadConfFile("template.plist");
	if(!MVCS_CONFIG.template)
		return false;
	return true;
}

bool __attribute__((constructor)) MVCSLoadConf(void) {
	return MVCSLoadServerConf() && MVCSLoadDoctypeConf() && MVCSLoadTemplateConf();
}
