#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include "suggestions.h"

#define MAX_INPUT 256

struct termios orig_term;

typedef struct {
    const char* suggestion;
    const char* base_input;
} SuggestionResult;

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
    while (end > str && (isspace((unsigned char)*end) || *end == '\r' || *end == '\n')) end--;
    end[1] = '\0';

    if (str[0] == '"' && end[0] == '"' && end > str) {
        memmove(str, str + 1, end - str - 1);
        str[end - str - 1] = '\0';
    }
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

void reset_terminal_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
}

void set_conio_terminal_mode() {
    struct termios new_term;
    tcgetattr(STDIN_FILENO, &orig_term);
    new_term = orig_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    atexit(reset_terminal_mode);
}

SuggestionResult find_suggestion(const char* input) {
    static char word1[MAX_INPUT], word2[MAX_INPUT];
    SuggestionResult result = { NULL, NULL };

    word1[0] = word2[0] = '\0';
    sscanf(input, "%s %s", word1, word2);

    const char** source = suggestions;
    const char* target = input;

    if (strcmp(word1, "gamerule") == 0) {
        if (strlen(word2) == 0 && input[strlen("gamerule")] == ' ') {
            source = gamerules;
            target = "";
        } else if (strlen(word2) > 0) {
            source = gamerules;
            target = word2;
        } else {
            return result;
        }
    } else if (strcmp(word1, "gamemode") == 0) {
        if (strlen(word2) == 0 && input[strlen("gamemode")] == ' ') {
            source = gamemodes;
            target = "";
        } else if (strlen(word2) > 0) {
            source = gamemodes;
            target = word2;
        } else {
            return result;
        }
    }

    size_t len = strlen(target);
    for (int i = 0; source[i]; ++i) {
        if (strncmp(source[i], target, len) == 0) {
            result.suggestion = source[i];
            result.base_input = target;
            return result;
        }
    }

    return result;
}

void draw_input(const char* input, SuggestionResult suggestion) {
    printf("\r\033[K> %s", input);
    if (suggestion.suggestion && strcmp(suggestion.base_input, suggestion.suggestion) != 0) {
        printf("\033[90m%s\033[0m", suggestion.suggestion + strlen(suggestion.base_input));
    }
    fflush(stdout);
}

void custom_input(char* buffer) {
    size_t len = 0;
    buffer[0] = '\0';

    while (1) {
        SuggestionResult suggestion = find_suggestion(buffer);
        draw_input(buffer, suggestion);

        char ch = getchar();
        if (ch == '\n') {
            putchar('\n');
            break;
        } else if (ch == 127 || ch == '\b') {
            if (len > 0) buffer[--len] = '\0';
        } else if (ch == '\t') {
            if (suggestion.suggestion) {
                size_t base_len = strlen(suggestion.base_input);
                size_t sug_len = strlen(suggestion.suggestion);
                if (base_len < sug_len && len + (sug_len - base_len) < MAX_INPUT - 1) {
                    strcpy(buffer + len, suggestion.suggestion + base_len);
                    len = strlen(buffer);
                }
            }
        } else if (isprint(ch) && len < MAX_INPUT - 1) {
            buffer[len++] = ch;
            buffer[len] = '\0';
        }
    }
}

int main() {
    load_env(".env");
    if (port == -1) {
        port = 25575;
    }
    if (password == NULL) {
        printf("Port can be inferred to be 25575, but password must be specified in .env!");
        return 1;
    }

    set_conio_terminal_mode();
    char input[MAX_INPUT];
    custom_input(input);
    printf("You typed: %s\n", input);

    size_t cmd_size = 512 + strlen(input);
    char* command = malloc(cmd_size);
    if (!command) {
        perror("malloc");
        return 1;
    }

    snprintf(command, cmd_size, "mcrcon -H localhost -P %d -p %s \"%s\"", port, password, input);
    int ret = system(command);

    free(command);
    free(password);

    return ret;
}

