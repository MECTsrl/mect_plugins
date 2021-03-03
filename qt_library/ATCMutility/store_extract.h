#ifndef EXTRACT_H
#define EXTRACT_H

#ifdef __cplusplus
extern "C" {
#endif

extern char ** filterHeader;
extern int filterHeaderSize;

extern char ** outstruct;

void finishLogRead();
int initLogRead(const char * logdir, const char * storeFilterFile, time_t ti, time_t tf, FILE ** fpin);
int getLogRead(const char * logdir, time_t ti, time_t tf, FILE ** fpin, char ** outstruct);
int dumpLogHeder(FILE * fpout);
int dumpLogRead(FILE * fpout, char ** outstruct);

#ifdef __cplusplus
}
#endif
#endif
