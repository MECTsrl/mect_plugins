#include <stdio.h>
#define __USE_XOPEN
#define _GNU_SOURCE
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#ifdef STANDALONE
#define LOG_PRINT(level, format, args...)
#else
#include "app_logprint.h"
#endif

#define MAX_FIELDS_NB 128
#define LINE_SIZE 1024
#define SIGN_APP "/usr/bin/sign"
#define UNDEFINED "UNDEFINED"

#define LINE2STR(line) \
{ \
    if (strchr(line, '\r')) \
    { \
        *strchr(line, '\r') = '\0'; \
    } \
    else if (strchr(line, '\n')) \
    { \
        *strchr(line, '\n') = '\0'; \
    } \
}

char FieldsMap[MAX_FIELDS_NB][LINE_SIZE];
int  FilterFlags[MAX_FIELDS_NB];

int LoadFilterFields(const char * fieldsfile, char * titleline, int * filterflags)
{
	char field[LINE_SIZE];
	char token[LINE_SIZE];
	int j = 0;
	int k = 0;
	char * p = NULL;
	int found = 0;

	FILE * fpfieldsfile = fopen (fieldsfile, "r");
	if (fpfieldsfile == NULL)
	{
        LOG_PRINT(info_e, "cannot open field file '%s'\n", fieldsfile);
		return 1;
	}

	LOG_PRINT(info_e, "fieldsfile %s\n", fieldsfile);

	while (fgets(field, LINE_SIZE, fpfieldsfile) != NULL)
	{
        LINE2STR(field);
        found = 0;
		for (j = 0, p = titleline; j < MAX_FIELDS_NB && p != NULL; j++, p = strchr(p, ';'))
		{
			if (*p == ';')
			{
				p++;
			}
			if (*p =='\0')
			{
				break;
			}
			if (j > MAX_FIELDS_NB)
			{
                LOG_PRINT(info_e, "Too many fields %d vs %d\n", j, MAX_FIELDS_NB);
				return 1;
			}
			strcpy(token, p);
			if (strstr(token, ";") != NULL)
			{
				*strstr(token, ";") = '\0';
			}
			for (k = strlen(token) - 1; k > 0 && isspace(token[k]); k--);
			token[k + 1] = '\0';
			for (k = 0; (unsigned int) k < strlen(token) && isspace(token[k]); k++);
			if (k > 0)
			{
				strcpy(token, &(token[k]));
			}
			if (strcmp(field, token) == 0)
			{
                LOG_PRINT(error_e, "########## add new field %d %s\n", j, field);
                filterflags[j] = 1;
				strcpy(FieldsMap[j], token);
				found = 1;
			}
            else
            {
                LOG_PRINT(error_e, "########## skipping field %d %s\n", j, field);
                filterflags[j] = 1;
            }
		}
		if (found == 0)
		{
			for (; j < MAX_FIELDS_NB && FieldsMap[j][0] != '\0'; j++);
			if (j < MAX_FIELDS_NB)
			{
				strcpy(FieldsMap[j], field);
                LOG_PRINT(info_e, "add new field %d %s\n", j, field);
                filterflags[j] = 1;
			}
		}
	}
	fclose(fpfieldsfile);
	return 0;
}

int Extract(FILE * fpin, FILE * fpout, int skipline, int * filterflags, const char * datein, const char * timein, const char * datefin, const char * timefin)
{
	struct tm time;
	time_t datetimein, datetimefin;
	char token[LINE_SIZE];
	char line[LINE_SIZE];
	char * p;
	int j,k;
	int first = 0;
	int firstline = 1;

	if (fpin == NULL || fpout == NULL)
	{
		return 1;
	}

	if (datein != NULL && timein != NULL && datefin != NULL && timefin != NULL)
	{
		/* download logs from datein timein to datefin timefin */
		sprintf(token, "%s %s", datein, timein);
		if(strptime(token, "%Y/%m/%d %H:%M:%S", &time) == NULL)
		{
            LOG_PRINT(info_e, "invalid time '%s'\n", token);
			return 1;
		}
		datetimein = mktime(&time);

		sprintf(token, "%s %s", datefin, timefin);
		if(strptime(token, "%Y/%m/%d %H:%M:%S",&time) == NULL)
		{
            LOG_PRINT(info_e, "invalid time '%s'\n", token);
			return 1;
		}
		datetimefin = mktime(&time);
	}
	else
	{
        LOG_PRINT(info_e, "invalid paramenters\n");
		return 1;
	}

	if (skipline == 1 && fgets(line, LINE_SIZE, fpin) != NULL)
	{
		while (fgets(line, LINE_SIZE, fpin) != NULL)
		{
            LINE2STR(line);
			strcpy(token, line);
			p = strchr(token, ';');
			if (p == NULL)
			{
                LOG_PRINT(info_e, "invalid time '%s'\n", token);
				return 1;
			}
			*p = ' ';
			p = strchr(token, ';');
			if (p == NULL)
			{
                LOG_PRINT(info_e, "invalid time '%s'\n", token);
				return 1;
			}
			*p = '\0';

			if(strptime(token, "%Y/%m/%d %H:%M:%S",&time) == NULL)
			{
                LOG_PRINT(info_e, "invalid time '%s'\n", token);
				return 1;
			}
			if (difftime(mktime(&time), datetimefin) <= 0)
			{
				if (difftime(mktime(&time), datetimein) >= 0)
				{
					first = 1;
					for (j = 0, p = line; j < MAX_FIELDS_NB && p != NULL; j++, p = strchr(p, ';'))
					{
						if (*p == ';')
						{
							p++;
						}
						if (*p =='\0')
						{
							break;
						}

						if (filterflags[j] == 1)
						{
							strcpy(token, p);
							if (strstr(token, ";") != NULL)
							{
								*strstr(token, ";") = '\0';
							}
							for (k = strlen(token) - 1; k > 0 && isspace(token[k]); k--);
							token[k + 1] = '\0';
							for (k = 0; (unsigned int) k < strlen(token) && isspace(token[k]); k++);
							if (k > 0)
							{
								strcpy(token, &(token[k]));
							}
							if (first == 0)
							{
								fprintf(fpout, "; ");
							}
							fprintf(fpout, "%s", token);
							first = 0;
						}
					}
					for (; j < MAX_FIELDS_NB && FieldsMap[j][0] != '\0'; j++)
					{
						fprintf(fpout, "; ");
						if (firstline == 1 && skipline == 0)
						{
							fprintf(fpout, "%s", FieldsMap[j]);
						}
						else
						{
							fprintf(fpout, "%s", UNDEFINED);
						}
					}
					fprintf(fpout, "\n");
				}
			}
			else
			{
				break;
			}
			firstline = 0;
		}
	}
	return 0;
}
#ifdef STANDALONE
int main (int argc, char * argv[])
#else
int StoreFilter ( char * outFileName, const char * logdir, const char * outdir, const char * fieldsfile, const char * datein, const char * timein, const char * datefin, const char * timefin)
#endif
{
    int retval;
	LOG_PRINT(info_e, "fieldsfile %s logdir %s\n", fieldsfile, logdir);
    FILE * fpin = NULL, * fpout = NULL;
#ifdef STANDALONE
	char * outdir = NULL, * logdir = NULL, * fieldsfile = NULL;
	char * datein = NULL, * timein = NULL, * datefin = NULL, * timefin = NULL;
    char outFileName[LINE_SIZE] = "";
#endif
	char * p = NULL;
	char token[LINE_SIZE] = "";
#ifdef SIGN_APP
	char command[LINE_SIZE] = "";
#endif
	char line[LINE_SIZE] = "";
	char tmp[LINE_SIZE] = "";
	char titleline[LINE_SIZE] = "";
	char inFullPathFileName[LINE_SIZE] = "";
    char outFullPathFileName[LINE_SIZE] = "";
    char *outBasename;
	int i, j, k;
    struct dirent **filelist = NULL;
	int fcount = -1;
	struct tm mytime;
	time_t datetimein, datetimefin;
	int first = 1;
	int firstline = 0;

#ifdef STANDALONE
	switch (argc)
	{
		case 4:
			logdir     = argv[1];
			outdir     = argv[2];
			fieldsfile = argv[3];
			break;
		case 5:
			logdir     = argv[1];
			outdir     = argv[2];
			fieldsfile = argv[3];
			datein     = argv[4];
			break;
		case 6:
			logdir     = argv[1];
			outdir     = argv[2];
			fieldsfile = argv[3];
			datein     = argv[4];
			datefin    = argv[5];
			break;
		case 8:
			logdir     = argv[1];
			outdir     = argv[2];
			fieldsfile = argv[3];
			datein     = argv[4];
			timein     = argv[5];
			datefin    = argv[6];
			timefin    = argv[7];
			break;
		default:
            LOG_PRINT(info_e, "%d vs 8\n", argc);
            LOG_PRINT(info_e, "Usage: %s <logdir> <outdir> <fieldsfile>\n", argv[0]);
            LOG_PRINT(info_e, "   Or: %s <logdir> <outdir> <fieldsfile> <datein>\n", argv[0]);
            LOG_PRINT(info_e, "   Or: %s <logdir> <outdir> <fieldsfile> <datein> <datefin>\n", argv[0]);
            LOG_PRINT(info_e, "   Or: %s <logdir> <outdir> <fieldsfile> <datein> <timein> <datefin> <timefin>\n", argv[0]);
			return 1;
			break;
	}
#endif

	fcount = scandir(logdir, &filelist, 0, alphasort);

    LOG_PRINT(info_e, "fieldsfile %s logdir %s fcount %d\n", fieldsfile, logdir, fcount);

    if (fcount < 0) {
		perror(logdir);
        retval = 1;
        goto exit_function;
    }

    bzero(FieldsMap, MAX_FIELDS_NB * LINE_SIZE * sizeof(char));
    bzero(FilterFlags, MAX_FIELDS_NB * sizeof(int));

    for (i = fcount - 1; i >= 0; i--)
	{
		/* skip the '.' and '..' files */
		if (strcmp(filelist[i]->d_name, ".") != 0 && strcmp(filelist[i]->d_name, "..") != 0)
		{
			sprintf(inFullPathFileName, "%s/%s", logdir, filelist[i]->d_name);

            LOG_PRINT(info_e, "%s\n", inFullPathFileName);
            fpin = fopen(inFullPathFileName, "r");
			if (fpin)
			{
				/* extract the selected columns from the actual log file */
				if (fgets(titleline, LINE_SIZE, fpin) == NULL)
				{
                    LINE2STR(titleline);
                    LOG_PRINT(info_e, "Cannot read title line from '%s'\n", inFullPathFileName);
                    fclose(fpin);
                    continue;
                }
				else
                {
                    fclose(fpin);
                    break;
				}
			}
		}
	}

    /* load the filter file */
    if (i < 0 || fpin == NULL)
    {
        LOG_PRINT(info_e, "no file %s.\n", fieldsfile);
        retval = 2;
        goto exit_function;
    }

    LOG_PRINT(info_e, "fieldsfile %s logdir %s fcount %d\n", fieldsfile, logdir, fcount);

    if (strlen(titleline) == 0)
	{
        LOG_PRINT(info_e, "Cannot read title line\n");
        retval = 3;
        goto exit_function;
    }

	/* load the filter file */
    if (LoadFilterFields(fieldsfile, titleline, FilterFlags) != 0)
	{
        LOG_PRINT(info_e, "invalid fields file '%s'\n", fieldsfile);
        retval = 5;
        goto exit_function;
    }

    /* daily logs */
    if (datein != NULL && timein == NULL && timefin == NULL)
	{
        LOG_PRINT(info_e, "DAILY\n");
        if (datefin == NULL)
		{
			datefin = datein;
		}
		sprintf(tmp, "%s 23:59:59", datefin);
		if(strptime(tmp, "%Y/%m/%d %H:%M:%S", &mytime) == NULL)
		{
            LOG_PRINT(info_e, "invalid time '%s'\n", datefin);
            retval = 6;
            goto exit_function;
        }
		datetimefin = mktime(&mytime);

		sprintf(tmp, "%s 00:00:00", datein);
		if(strptime(tmp, "%Y/%m/%d %H:%M:%S", &mytime) == NULL)
		{
            LOG_PRINT(info_e, "invalid time '%s'\n", datein);
            retval = 7;
            goto exit_function;
        }
		datetimein = mktime(&mytime);

		while (difftime(datetimefin, datetimein) > 0)
		{
			strftime(tmp, sizeof(tmp), "%Y_%m_%d", &mytime);
			sprintf(inFullPathFileName, "%s/%s.log", logdir, tmp);
			fpin = fopen(inFullPathFileName, "r");
			if (fpin)
			{
				/*extracting the basename without extension*/
				outBasename = strrchr(fieldsfile, '/' ) + 1;
				strcpy(token, outBasename);
				outBasename = strchr(token, '.');
				*outBasename ='\0';

                sprintf(outFileName, "%s_%s.log", tmp, token );
				LOG_PRINT(info_e, "outFileName  '%s'\n", outFileName);
                sprintf(outFullPathFileName, "%s/%s", outdir, outFileName );
                fpout = fopen(outFullPathFileName, "w");
				if (fpout == NULL)
				{
                    LOG_PRINT(info_e, "Cannot open '%s'\n", outFullPathFileName);
                    retval = 8;
                    goto exit_function;
                }
				fprintf(stdout, "%s\n", outFullPathFileName);

				/* extract the selected columns from the actual log file */
				while (fgets(line, LINE_SIZE, fpin) != NULL)
				{
                    LINE2STR(line);
                    first = 1;
					for (j = 0, p = line; j < MAX_FIELDS_NB && p != NULL; j++, p = strchr(p, ';'))
					{
						if (*p == ';')
						{
							p++;
						}
						if (*p =='\0')
						{
							break;
						}

						if (FilterFlags[j] == 1)
						{
							strcpy(token, p);
							if (strstr(token, ";") != NULL)
							{
								*strstr(token, ";") = '\0';
							}
							for (k = strlen(token) - 1; k > 0 && isspace(token[k]); k--);
							token[k + 1] = '\0';
							for (k = 0; (unsigned int) k < strlen(token) && isspace(token[k]); k++);
							if (k > 0)
							{
								strcpy(token, &(token[k]));
							}
							if (first == 0)
							{
								fprintf(fpout, "; ");
							}
							fprintf(fpout, "%s", token);
							first = 0;
						}
					}
					fprintf(fpout, "\n");
				}
				fclose(fpout);
				fclose(fpin);
#ifdef SIGN_APP
				/* create the sign file for the actual extracted log file */
                sprintf(command, "%s %s | cut -d\\  -f1 > %s.sign", SIGN_APP, outFullPathFileName, outFullPathFileName);
				if (system(command) != 0)
				{
                    LOG_PRINT(info_e, "cannot create sign file '%s.sign'\n", outFullPathFileName);
                    retval = 9;
                    goto exit_function;
                }
				fprintf(stdout, "%s.sign\n", outFullPathFileName);
#endif
			}
			else
			{
                LOG_PRINT(info_e, "Warning cannot open field file '%s'\n", inFullPathFileName);
			}
			mytime.tm_mday++;
			datetimein = mktime(&mytime);
		}
	}
	/* all logs */
	else if (datein == NULL && timein == NULL && datefin == NULL && timefin == NULL)
	{
        LOG_PRINT(info_e, "ALL\n");
        for(i = 0; i < fcount; i++)
		{
			/* skip the '.' and '..' files */
			if (strcmp(filelist[i]->d_name, ".") != 0 && strcmp(filelist[i]->d_name, "..") != 0)
			{
				sprintf(inFullPathFileName, "%s/%s", logdir, filelist[i]->d_name);
				fpin = fopen(inFullPathFileName, "r");
				if (fpin)
				{
					/*extracting the basename without extension*/
					outBasename = strrchr(fieldsfile, '/' ) + 1;
					strcpy(token, outBasename);
					outBasename = strchr(token, '.');
					*outBasename ='\0';
					sprintf(outFileName, "%s_%s.log", filelist[i]->d_name, token );
					LOG_PRINT(info_e, "outFileName  '%s'\n", outFileName);
					sprintf(outFullPathFileName, "%s/%s", outdir, outFileName);
					fpout = fopen(outFullPathFileName, "w");
					if (fpout == NULL)
					{
                        LOG_PRINT(info_e, "Cannot open '%s'\n", outFullPathFileName);
						fclose(fpin);
                        retval = 10;
                        goto exit_function;
                    }
					fprintf(stdout, "%s\n", outFullPathFileName);

					/* extract the selected columns from the actual log file */
					while (fgets(line, LINE_SIZE, fpin) != NULL)
					{
                        LINE2STR(line);
						first = 1;
						for (j = 0, p = line; j < MAX_FIELDS_NB && p != NULL; j++, p = strchr(p, ';'))
						{
							if (*p == ';')
							{
								p++;
							}
							if (*p =='\0')
							{
								break;
							}

							if (FilterFlags[j] == 1)
							{
								strcpy(token, p);
								if (strstr(token, ";") != NULL)
								{
									*strstr(token, ";") = '\0';
								}
								for (k = strlen(token) - 1; k > 0 && isspace(token[k]); k--);
								token[k + 1] = '\0';
								for (k = 0; (unsigned int) k < strlen(token) && isspace(token[k]); k++);
								if (k > 0)
								{
									strcpy(token, &(token[k]));
								}
								if (first == 0)
								{
									fprintf(fpout, "; ");
								}
								fprintf(fpout, "%s", token);
								first = 0;
							}
						}
						if (first == 0)
						{
							fprintf(fpout, "\n");
						}
					}
					fclose(fpout);
					fclose(fpin);

#ifdef SIGN_APP
					/* create the sign file for the actual extracted log file */
                    sprintf(command, "%s %s | cut -d\\  -f1 > %s.sign", SIGN_APP, outFullPathFileName, outFullPathFileName);
					if (system(command) != 0)
					{
                        LOG_PRINT(info_e, "cannot create sign file '%s.sign'\n", outFullPathFileName);
                        retval = 11;
                        goto exit_function;
                    }
					fprintf(stdout, "%s.sign\n", outFullPathFileName);
#endif
				}
			}
		}
	}
	/* time filtered */
	else if (datein != NULL && timein != NULL && datefin != NULL && timefin != NULL)
	{
        LOG_PRINT(info_e, "TIME\n");
        int skipline = 0;

		/* extract the log file */
		sprintf(tmp, "%s 00:00:00", datefin);
		if(strptime(tmp, "%Y/%m/%d %H:%M:%S", &mytime) == NULL)
		{
            LOG_PRINT(info_e, "invalid time '%s'\n", datefin);
            retval = 12;
            goto exit_function;
        }
		datetimefin = mktime(&mytime);

		sprintf(tmp, "%s 00:00:00", datein);
		if(strptime(tmp, "%Y/%m/%d %H:%M:%S", &mytime) == NULL)
		{
            LOG_PRINT(info_e, "invalid time '%s'\n", datein);
            retval = 13;
            goto exit_function;
        }
		datetimein = mktime(&mytime);

		/* cut field extension */
		if (strchr(fieldsfile, '.'))
		{
			*strchr(fieldsfile, '.') = '\0';
		}
		outBasename = strrchr(fieldsfile, '/' ) + 1;
		sprintf(outFileName, "%s.log", outBasename);
		sprintf(outFullPathFileName, "%s/%s", outdir, outFileName);
		LOG_PRINT(info_e, "outFileName  '%s'\n", outFileName);
		fpout = fopen(outFullPathFileName, "w");
		if (fpout == NULL)
		{
            LOG_PRINT(info_e, "cannot open file '%s'\n", outFullPathFileName);
            retval = 14;
            goto exit_function;
        }
		fprintf(stdout, "%s\n", outFullPathFileName);

		while (difftime(datetimefin, datetimein) >  - 24*60*60)
		{
			strftime(tmp, sizeof(tmp), "%Y_%m_%d", &mytime);
			sprintf(inFullPathFileName, "%s/%s.log", logdir, tmp);
			fpin = fopen(inFullPathFileName, "r");
			if (fpin)
			{
				if (skipline == 0)
				{

					skipline = 1;
					for (j = 0, p = titleline; j < MAX_FIELDS_NB && p != NULL; j++, p = strchr(p, ';'))
					{
						if (*p == ';')
						{
							p++;
						}
						if (*p =='\0')
						{
							break;
						}

						if (FilterFlags[j] == 1)
						{
							strcpy(token, p);
                            LOG_PRINT(info_e, "token %s, p %s", token, p);
							if (strstr(token, ";") != NULL)
							{
								*strstr(token, ";") = '\0';
							}
							for (k = strlen(token) - 1; k > 0 && isspace(token[k]); k--);
							token[k + 1] = '\0';
							for (k = 0; (unsigned int) k < strlen(token) && isspace(token[k]); k++);
							if (k > 0)
							{
								strcpy(token, &(token[k]));
							}
							fprintf(fpout, "%s", token);
							if (firstline == 0)
							{
								fprintf(fpout, "; ");
							}
							
                            LOG_PRINT(info_e, "su file %s\n", token);
							firstline = 0;
						}
					}
					fprintf(fpout, "\n");
					
					//occorre filtrare anche la riga di titolo
					//fprintf(fpout, "%s", titleline);
				}

				/* open the log file and filter by time and by field */
				if (Extract(fpin, fpout, skipline, FilterFlags, datein, timein, datefin, timefin) != 0)
				{
                    LOG_PRINT(info_e, "Error\n");
                    retval = 15;
                    goto exit_function;
                }
			}
			else
			{
                LOG_PRINT(info_e,"Cannot open %s\n", inFullPathFileName);
			}
			if(fpin)
				fclose(fpin);

            mytime.tm_mday++;
			datetimein = mktime(&mytime);
		}
		fclose(fpout);
#ifdef SIGN_APP
        /* create the sign file for the actual extracted log file */
        sprintf(command, "%s %s | cut -d\\  -f1 > %s.sign", SIGN_APP, outFullPathFileName, outFullPathFileName);
        if (system(command) != 0)
        {
            LOG_PRINT(info_e, "cannot create sign file '%s.sign'\n", outFullPathFileName);
            retval = 16;
            goto exit_function;
        }
        fprintf(stdout, "%s.sign\n", outFullPathFileName);
#endif
	}
	else
	{
        LOG_PRINT(info_e, "invalid paramenter\n");
        retval = 17;
        goto exit_function;
    }
    LOG_PRINT(info_e, "END\n");
    retval = 0;

exit_function:
    if (fcount >= 0) {
        int n;
        for (n = 0; n < fcount; ++n) {
            free(filelist[n]);
        }
        free(filelist);
    }
    return retval;
}


