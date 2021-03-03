#include <stdio.h>
#define __USE_XOPEN
#define _GNU_SOURCE
#include <time.h>

#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef STANDALONE

#define LOG_PRINT(level, format, args...) \
{ \
    fprintf (stderr, "[%s:%s:%d]", __FILE__, __func__, __LINE__); \
    fprintf (stderr, format , ## args); \
    fflush(stderr); \
}
#define CROSS_TABLE        LOCAL_ETC_DIR"/Crosstable.csv"
#define TAG_LEN   (31 + 1)
#define DB_SIZE_ELEM 5472
#define TAG_STORED_SLOW    'S'
#define TAG_STORED_FAST    'F'
#define TAG_STORED_ON_VAR  'V'
#define TAG_STORED_ON_SHOT 'X'

#else

#include "app_logprint.h"
#include "utility.h"
#include "common.h"

#endif

#define STR_LEN 256
#define SEPARATOR ";"
#define ALL_VARIABLE "Total.csv"

struct dirent **filelist = NULL;
int fcount = -1;
int actualfcount = -1;

char ** logHeader = NULL;
int logHeaderSize = 0;

char ** ctHeader = NULL;
int ctHeaderSize = 0;

char ** filterHeader = NULL;
int filterHeaderSize = 0;

char ** outstruct = NULL;

int maplog2header[DB_SIZE_ELEM];

char ** allocMatrix(int m , int n)
{
    int i;
    char ** matrix = calloc(m, sizeof (char **));
    for (i = 0; i < m; i++)
    {
        matrix[i] = calloc(n, sizeof (char *));
    }
    return matrix;
}

void freeMatrix(char ** matrix , int m)
{
    int i;
    for (i = 0; i < m; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
}

#ifdef STANDALONE
char *strtok_csv(char *string, const char *separators, char **savedptr)
{
    char *p, *s;

    if (separators == NULL || savedptr == NULL) {
        return NULL;
    }
    if (string == NULL) {
        p = *savedptr;
        if (p == NULL) {
            return NULL;
        }
    } else {
        p = string;
    }

    s = strstr(p, separators);
    if (s == NULL) {
        *savedptr = NULL;
        s = p + strlen(p);
    }
    else
    {
        *s = 0;
        *savedptr = s + 1;
    }

    // remove spaces at head
    while (p < s && isspace(*p)) {
        ++p;
    }
    // remove spaces at tail
    --s;
    while (s > p && isspace(*s)) {
        *s = 0;
        --s;
    }
    return p;
}
#endif

void finishLogRead()
{
    int i =  0;
    for (i = 0; i < DB_SIZE_ELEM; i++)
    {
        maplog2header[i] = -1;
    }
    if (ctHeader)
    {
        freeMatrix(ctHeader, DB_SIZE_ELEM);
        ctHeaderSize = 0;
        ctHeader = NULL;
    }
    if (filterHeader)
    {
        if (outstruct)
        {
            freeMatrix(outstruct, filterHeaderSize);
            outstruct = NULL;
        }
        freeMatrix(filterHeader, DB_SIZE_ELEM);
        filterHeaderSize = 0;
        filterHeader = NULL;
    }
    if (logHeader)
    {
        freeMatrix(logHeader, DB_SIZE_ELEM);
        logHeaderSize = 0;
        logHeader = NULL;
    }
    for(i = 0; i < fcount; i++)
    {
            free(filelist[i]);
    }

    free(filelist);
    filelist = NULL;
    fcount = 0;
}

int getStoreCtHeader(char ** header)
{
    FILE * fp;

    fp = fopen(CROSS_TABLE, "r");
    if (fp == NULL)
    {
        LOG_PRINT(error_e, "Cannot open the CROSS_TABLE '%s'\n", CROSS_TABLE);
        return -1;
    }

    char line[LINE_SIZE];
    char * p, * r;

    strcpy(header[0], "date");
    strcpy(header[1], "time");

    int j = 2;
    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        /* Enable */
        p = strtok_csv(line, SEPARATOR, &r);
        if (p != NULL && atoi(p) > 0)
        {
            /* Store */
            p = strtok_csv(NULL, SEPARATOR, &r);
            if (p != NULL)
            {
                switch (p[0])
                {
                    case TAG_STORED_SLOW:
                    case TAG_STORED_FAST:
                    case TAG_STORED_ON_VAR:
                    case TAG_STORED_ON_SHOT:
                        p = strtok_csv(NULL, SEPARATOR, &r);
                        if (p != NULL)
                        {
                            strcpy(header[j], p);
                            LOG_PRINT(verbose_e, "header[%d] '%s'\n", j, header[j]);
                            j++;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
    fclose(fp);
    return j;
}

int getStoreFilter(const char * storeFilterFile, char ** header)
{
    FILE * fp;
    /* if there is a filter file, extract list of stored variable into the store filter file */
    /* if the store filter file have more variable than CROSS_TABLE stored variable
     * give an error and ignore it
     */
    ctHeader = allocMatrix(DB_SIZE_ELEM, TAG_LEN);
    if (ctHeader == NULL)
    {
        LOG_PRINT(error_e, "Cannot allocate ctHeader\n");
        return -1;
    }

    int ctHeaderSize = getStoreCtHeader(ctHeader);
    if (ctHeaderSize <= 0)
    {
        LOG_PRINT(info_e, "Cannot find stored variable\n");
        return -1;
    }

    if (storeFilterFile != NULL && storeFilterFile[0] != '\0')
    {
        fp = fopen(storeFilterFile, "r");
        if (fp == NULL)
        {
            LOG_PRINT(error_e, "Cannot open the file '%s'\n", storeFilterFile);
            return -1;
        }

        char line[STR_LEN];
        int j = 0;
        char * p;
        while (fgets(line, LINE_SIZE, fp) != NULL)
        {
            if (line[0] == '\0')
            {
                /* skipping empty line */
                continue;
            }
            p = strchr(line, '\r');
            if (p)
            {
                *p = '\0';
            }
            p = strchr(line, '\n');
            if (p)
            {
                *p = '\0';
            }
            int found = 0;
            int i;
            for (i = 0; i < ctHeaderSize; i++)
            {
                if (strcmp(ctHeader[i], line) == 0)
                {
                    strcpy(header[j], line);
                    LOG_PRINT(verbose_e, "header[%d] %s\n",j, header[j]);
                    found = 1;
                    j++;
                }
            }
            if (found == 0)
            {
                LOG_PRINT(error_e, "Cannot find variable to dump\n");
                fclose(fp);
                return -1;
            }
        }
        fclose(fp);
        return j;
    }
    /* else if there isn't a filter file put all the stored variable into the CROSS_TABLE into the filter */
    /* for each log file into the time filter interval,
     * extract the header */
    else
    {
        int i;
        for (i = 0; i < ctHeaderSize; i++)
        {
            strcpy(header[i], ctHeader[i]);
        }
        LOG_PRINT(verbose_e, "No store filter \n");
        return ctHeaderSize;
    }
}

int initLogRead(const char * logdir, const char * storeFilterFile, time_t ti, time_t tf, FILE ** fpin)
{
    struct tm tfile;
    time_t t;

    finishLogRead();

    fcount = scandir(logdir, &filelist, 0, alphasort);

    if (fcount <= 0)
    {
        // no log file
        LOG_PRINT(error_e, "Cannot find any store file into '%s'\n", logdir);
        return -1;
    }

    for (actualfcount = 0; actualfcount < fcount; actualfcount++)
    {
        /* skip the '.' and '..' files */
        if (strcmp(filelist[actualfcount]->d_name, ".") == 0 || strcmp(filelist[actualfcount]->d_name, "..") == 0)
        {
            continue;
        }
        // yyyy_MM_dd_HH_mm_ss.log
        // transform yyyy_MM_dd_HH_mm_ss into time_t
        strptime (filelist[actualfcount]->d_name, "%Y_%m_%d_%H_%M_%S.log", &tfile);
        tfile.tm_isdst = 0;
        t = mktime(&tfile);

        LOG_PRINT(verbose_e, "actualfcount %d fcount %d\n", actualfcount, fcount);
        if ( t > ti || actualfcount == fcount - 1)
        {
            if (actualfcount < fcount)
            {
                LOG_PRINT(verbose_e, "t > ti\n");
                actualfcount = (actualfcount > 2) ? actualfcount - 1 : actualfcount;
            }
            LOG_PRINT(verbose_e, "found file '%s'\n", filelist[actualfcount]->d_name);

            filterHeader = allocMatrix(DB_SIZE_ELEM, TAG_LEN);
            if (filterHeader == NULL)
            {
                LOG_PRINT(error_e, "Cannot allocate filterHeader\n");
                return -1;
            }
            filterHeaderSize = getStoreFilter(storeFilterFile, filterHeader);
            if (filterHeaderSize <= 0)
            {
                LOG_PRINT(error_e, "Cannot find variable to dump\n");
                return -1;
            }

            /* read the store filter and intersecate it with ctHeader */

            logHeader = allocMatrix(DB_SIZE_ELEM, TAG_LEN);
            if (logHeader == NULL)
            {
                LOG_PRINT(error_e, "Cannot allocate logHeader\n");
                return -1;
            }

            outstruct = allocMatrix(filterHeaderSize, TAG_LEN);
            if (outstruct == NULL)
            {
                LOG_PRINT(error_e, "Cannot allocate outstruct\n");
                return -1;
            }

            *fpin = NULL;

            return 0;
        }
    }

    LOG_PRINT(verbose_e, "actualfcount %d fcount %d\n", actualfcount, fcount);
    return -1;
}

int getStoreLogHeader(FILE * fp, char ** logHeader)
{
    char line[LINE_SIZE];
    char * p, * r;
    if (fgets(line, LINE_SIZE, fp) == NULL)
    {
        LOG_PRINT(error_e, "Cannot get data\n");
        return -1;
    }
    int i = 0;
    p = strtok_csv(line, SEPARATOR, &r);
    do
    {
        /* fill outstruct  in function of header */
        strcpy(logHeader[i], p);
        i++;
    }
    while ((p = strtok_csv(NULL, SEPARATOR, &r)));
    return i;
}

int getLogRead(const char * logdir, time_t ti, time_t tf, FILE ** fpin, char ** outstruct)
{
    static struct stat file_stat_old;
    char line[LINE_SIZE];
    int getting_news = 0;
    if (fcount <= 0 || filelist == NULL || outstruct == NULL)
    {
        LOG_PRINT(error_e, "no data to read\n");
        return -1;
    }
    if (*fpin == NULL || feof(*fpin))
    {
        char fullpath[FILENAME_MAX];
        if (*fpin != NULL) // i.e. feof()
        {
            if (actualfcount + 1 < fcount)
            {
                fclose(*fpin);
                *fpin = NULL;
                actualfcount ++;
                sprintf(fullpath,"%s/%s", logdir, filelist[actualfcount]->d_name);
                if (stat(fullpath, &file_stat_old) != 0) {
                    LOG_PRINT(error_e, "cannot stat '%s'\n",fullpath);
                    return 1;
                }

                *fpin = fopen(fullpath, "r");
                if (*fpin == NULL)
                {
                    LOG_PRINT(error_e, "cannot open '%s'\n",fullpath);
                    return -1;
                }
                LOG_PRINT(verbose_e, "open '%s'\n",fullpath);
            }
            else
            {
                /* here the code to reopen the last log file and reload the news */
                sprintf(fullpath,"%s/%s", logdir, filelist[actualfcount]->d_name);
                LOG_PRINT(verbose_e, "check for some news in '%s'\n", fullpath);
                struct stat file_stat;
                if (stat(fullpath, &file_stat) != 0) {
                    LOG_PRINT(error_e, "cannot stat '%s'\n",fullpath);
                    return 1;
                }
                if (file_stat.st_mtime > file_stat_old.st_mtime)
                {
                    fpos_t pos;
                    /* save the actual position */
                    if (fgetpos(*fpin, &pos))
                    {
                        LOG_PRINT(error_e, "cannot get the actual position\n");
                        return -1;
                    }
                    LOG_PRINT(verbose_e, "news in '%s': %ld %ld\n", fullpath, file_stat.st_mtime, file_stat_old.st_mtime);
                    memcpy(&file_stat_old, &file_stat, sizeof(file_stat));
                    /* close and re open the file */
                    fclose(*fpin);
                    *fpin = fopen(fullpath, "r");
                    if (*fpin == NULL)
                    {
                        LOG_PRINT(error_e, "cannot open file '%s'\n", fullpath);
                        return -1;
                    }
                    LOG_PRINT(verbose_e, "open '%s'\n",fullpath);
                    /* go to the actual position */
                    if (fsetpos(*fpin, &pos))
                    {
                        LOG_PRINT(error_e, "cannot set the actual position\n");
                        return -1;
                    }
                    /* check if the file is not finished */
                    if (feof(*fpin))
                    {
                        LOG_PRINT(verbose_e, "File finished\n");
                        return -1;
                    }
                    getting_news = 1;
                }
                else
                {
                    /* no news, finish */
                    LOG_PRINT(verbose_e, "no news: finished\n");
                    return -1;
                }
            }
        }
        else
        {
            sprintf(fullpath,"%s/%s", logdir, filelist[actualfcount]->d_name);
            if (stat(fullpath, &file_stat_old) != 0) {
                LOG_PRINT(error_e, "cannot stat '%s'\n",fullpath);
                return 1;
            }

            *fpin = fopen(fullpath, "r");
            if (*fpin == NULL)
            {
                LOG_PRINT(error_e, "cannot open file '%s'\n", fullpath);
                return -1;
            }
            LOG_PRINT(verbose_e, "open '%s'\n",fullpath);
        }
        if (getting_news == 0)
        {
        //LOG_PRINT(error_e, "Opening new file '%s'\n", fullpath);
        int i,j;
        int variablesFound = 0;
        logHeaderSize = getStoreLogHeader(*fpin, logHeader);

        for (i = 0; i < DB_SIZE_ELEM; i++)
        {
            maplog2header[i] = -1;
        }

        /* mette in ct header il numero di colonna dei log header */
        for (i = 0; i < logHeaderSize; i++)
        {
            for (j = 0; j < filterHeaderSize; j++)
            {
                if (strcmp(logHeader[i], filterHeader[j]) == 0)
                {
                    maplog2header[i] = j;
                    variablesFound++;
                    break;
                }
            }
            if (j >= filterHeaderSize)
            {
                LOG_PRINT(warning_e, "Parsing file '%s': Cannot find '%s' into the CT\n", fullpath, logHeader[i]);
            }
        }
        if (variablesFound == 0)
        {
            LOG_PRINT(error_e, "no variables found\n");
            return -1;
        }
        }
    }
    if (fgets(line, LINE_SIZE, *fpin) != NULL)
    {
        struct tm tfile;
        time_t t;
        char * p, * r;
        char tmp[256];

        /*date*/
        p = strtok_csv(line, SEPARATOR, &r);
        if (p == NULL)
        {
            LOG_PRINT(error_e, "Cannot get date token\n");
            return -1;
        }
        strcpy(outstruct[0], p);
        sprintf (tmp, "%s_", p);

        /*time*/
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL)
        {
            LOG_PRINT(error_e, "Cannot get time token\n");
            return -1;
        }
        strcpy(outstruct[1], p);
        strcat (tmp, p);

        strptime (tmp, "%Y/%m/%d_%H:%M:%S", &tfile);
        tfile.tm_isdst = 0;
        t = mktime(&tfile);
        if ( ti > t )
        {
            /* too early */
            LOG_PRINT(verbose_e, "early\n");
            return 1;
        }
        if ( tf < t )
        {
            /* too late */
            LOG_PRINT(verbose_e, "late %ld %ld %ld\n", ti, t, tf);
            return -1;
        }

        int i = 2;
        while ((p = strtok_csv(NULL, SEPARATOR, &r)))
        {
            /* fill outstruct  in function of header */
            if (maplog2header[i]>=0)
            {
                if (maplog2header[i] < filterHeaderSize)
                {
                    strcpy(outstruct[maplog2header[i]], p);
                }
            }
            i++;
        }
        return 0;
    }
    LOG_PRINT(verbose_e, "done\n");
    return 1;
}

int dumpLogHeder(FILE * fpout)
{
    int i;
    if (fpout == NULL)
    {
        return -1;
    }
    for (i = 0; i < filterHeaderSize; i++)
    {
        if (i > 0)
        {
            fprintf(fpout, ";");
        }
        fprintf(fpout, "%s", filterHeader[i]);
    }
    fprintf(fpout, "\n");
    return 0;
}

int dumpLogRead(FILE * fpout, char ** outstruct)
{
    int i;
    if (fpout == NULL)
    {
        return -1;
    }
    if (filterHeaderSize <= 0)
    {
        return -1;
    }
    for (i = 0; i < filterHeaderSize; i++)
    {
        if (i > 0)
        {
            fprintf(fpout, ";");
        }
        fprintf(fpout, "%s", outstruct[i]);
    }
    fprintf(fpout, "\n");
    return 0;
}


#ifdef STANDALONE
int main (int argc, char * argv[])
{
#ifdef STANDALONE
    char * outdir = NULL, * logdir = NULL, * fieldsfile = NULL;
    char * datein = NULL, * timein = NULL, * datefin = NULL, * timefin = NULL;
    char outFileName[LINE_SIZE] = "";
#else
    outFileName[0] = '\0';
#endif
    char outFullPathFileName[FILENAME_MAX];
    struct tm tmp_ti, tmp_tf;
    time_t ti, tf;
    char tmp[256];
    FILE * fpin = NULL, * fpout = NULL;

#ifdef STANDALONE
    char empty[2] = "";
    switch (argc)
    {
        case 4:
            logdir     = argv[1];
            outdir     = argv[2];
            if (strcmp(argv[3], ALL_VARIABLE) == 0)
            {
                fieldsfile = empty;
            }
            break;
        case 5:
            logdir     = argv[1];
            outdir     = argv[2];
            if (strcmp(argv[3], ALL_VARIABLE) == 0)
            {
                fieldsfile = empty;
            }
            datein     = argv[4];
            break;
        case 6:
            logdir     = argv[1];
            outdir     = argv[2];
            if (strcmp(argv[3], ALL_VARIABLE) == 0)
            {
                fieldsfile = empty;
            }
            datein     = argv[4];
            datefin    = argv[5];
            break;
        case 8:
            logdir     = argv[1];
            outdir     = argv[2];
            if (strcmp(argv[3], ALL_VARIABLE) == 0)
            {
                fieldsfile = empty;
            }
            datein     = argv[4];
            timein     = argv[5];
            datefin    = argv[6];
            timefin    = argv[7];
            break;
        default:
            LOG_PRINT(verbose_e, "%d vs 8\n", argc);
            LOG_PRINT(verbose_e, "Usage: %s <logdir> <outdir> <fieldsfile>\n", argv[0]);
            LOG_PRINT(verbose_e, "   Or: %s <logdir> <outdir> <fieldsfile> <datein>\n", argv[0]);
            LOG_PRINT(verbose_e, "   Or: %s <logdir> <outdir> <fieldsfile> <datein> <datefin>\n", argv[0]);
            LOG_PRINT(verbose_e, "   Or: %s <logdir> <outdir> <fieldsfile> <datein> <timein> <datefin> <timefin>\n", argv[0]);
            return 1;
            break;
    }
#endif

    if (fieldsfile && fieldsfile[0] != '\0')
    {
        sprintf(outFileName, "%s/%s_%s-%s_%s.log", fieldsfile, datein, timein, datefin, timefin);
    }
    else
    {
        sprintf(outFileName, "%s/%s-%s_%s.log", datein, timein, datefin, timefin);
    }
    char * p = NULL;
    while((p = strchr(outFileName, '/')))
    {
        *p = '_';
    }
    while((p = strchr(outFileName, ':')))
    {
        *p = '_';
    }
    sprintf(outFullPathFileName, "%s/%s", outdir, outFileName );

    fpout = fopen(outFullPathFileName, "w");
    if (fpout == NULL)
    {
        LOG_PRINT(verbose_e, "Cannot open '%s'\n", outFullPathFileName);
        return -1;
    }

    sprintf (tmp, "%s_%s", datein, timein);
    strptime (tmp, "%Y/%m/%d_%H:%M:%S", &tmp_ti);
    tmp_ti.tm_isdst = 0;
    ti = mktime(&tmp_ti);
    LOG_PRINT(error_e, "%ld [%s]\n", ti, tmp);
    strftime (tmp, 32, "%Y/%m/%d %H:%M:%S", localtime(&ti));
    LOG_PRINT(verbose_e, "ti '%s'\n", tmp);
    sprintf (tmp, "%s_%s", datefin, timefin);
    strptime (tmp, "%Y/%m/%d_%H:%M:%S", &tmp_tf);
    tmp_tf.tm_isdst = 0;
    tf = mktime(&tmp_tf);
    LOG_PRINT(error_e, "%ld [%s]\n", tf, tmp);
    strftime (tmp, 32, "%Y/%m/%d %H:%M:%S", localtime(&tf));
    LOG_PRINT(verbose_e, "tf '%s'\n", tmp);

    // datein timein storefile
    if (initLogRead(logdir, fieldsfile, ti, tf, &fpin) != 0)
    {
        LOG_PRINT(verbose_e, "no file to dump\n");
    }

    if (dumpLogHeder(fpout) != 0)
    {
        LOG_PRINT(error_e, "\n");
        return -1;
    }

    int retval = 0;
    while ( (retval = getLogRead(logdir, ti, tf, &fpin, outstruct)) >= 0)
    {
        if (retval == 0)
        {
            if (dumpLogRead(fpout, outstruct) != 0)
            {
                LOG_PRINT(error_e, "\n");
                return -1;
            }
        }
    }

    fclose(fpout);
    return 0;
}
#endif
