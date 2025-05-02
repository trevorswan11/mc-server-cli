#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int port = -1;
char* password = NULL;

void trim(char* str) {
    char* end;
    while (isspace((unsigned char)* str)) str++;
    if (*str == 0) {
        str[0] = '\0';
        return;
    }

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)* str)) end--;
    end[1] ='\0';
}

void load_env(const char *filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror(".env");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char* key = line;
        char* value = eq + 1;

        trim(key);
        trim(value);
        
        if (strcmp(key, "RCON_PORT") == 0)
            port = atoi(value);
        else if (strcmp(key, "RCON_PASS") == 0)
            password = strdup(value); 
    }

    fclose(file);
}

int main() {
    load_env(".env");
    if (port == -1 || password == NULL) {
        fprintf(stderr, "Missing required environment variables.\n");
        return 1;
    }
    printf("The port: %d\n", port);
    printf("Your password: %s\n", password);
    
    return 0;
}
