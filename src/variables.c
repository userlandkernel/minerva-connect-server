
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *mvcs_var_pgp_authcode(void) {
    const int len = 40;
    char* s = malloc(41);
    bzero(s, 40);
    srand((unsigned int)time(NULL)); // I'm aware this is a vulnerable random approach where the random isn't random enough
    srand(rand());

    static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    s[len] = 0;
    return s;
}

char* mvcs_var_year(void) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char* year = malloc(64);
	bzero(year, 64);
	snprintf(year, 64, "%d", tm.tm_year + 1900);
	return year;
}

char *replacevar(const char *s, const char *oldW, const char *newW) 
{ 
    char *result; 
    int i, cnt = 0; 
    int newWlen = strlen(newW); 
    int oldWlen = strlen(oldW); 
  
    // Counting the number of times old word 
    // occur in the string 
    for (i = 0; s[i] != '\0'; i++) 
    { 
        if (strstr(&s[i], oldW) == &s[i]) 
        { 
            cnt++; 
  
            // Jumping to index after the old word. 
            i += oldWlen - 1; 
        } 
    } 
  
    // Making new string of enough length 
    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1); 
  
    i = 0; 
    while (*s) 
    { 
        // compare the substring with the result 
        if (strstr(s, oldW) == s) 
        { 
            strcpy(&result[i], newW); 
            i += newWlen; 
            s += oldWlen; 
        } 
        else
            result[i++] = *s++; 
    } 
  
    result[i] = '\0'; 
    return result; 
} 
