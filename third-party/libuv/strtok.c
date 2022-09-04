
#include <stdlib.h>
#include "strtok.h"
char* uv__strtok(char* str, const char* sep, char** itr) { const char* sep_itr; char* tmp; char* start; if (str == NULL) start = tmp = *itr; else start = tmp = str; if (tmp == NULL) return NULL; while (*tmp != '\0') { sep_itr = sep; while (*sep_itr != '\0') { if (*tmp == *sep_itr) { *itr = tmp + 1; *tmp = '\0'; return start; } sep_itr++; } tmp++; } *itr = NULL; return start;}