#ifndef UTILITI_H
#define UTILITI_H

#ifdef __cplusplus
extern "C" {
#endif

int atoh(const char * string);
char *strtok_csv(char *string, const char *separators, char **savedptr);

#ifdef __cplusplus
}
#endif

#endif // UTILITI_H
