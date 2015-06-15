/** 
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief HMI Maty to manage a configuration file
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "app_cfg_file.h"

/**
 * load a configuration file with the format
 *      <configuration>: <value>
 *
 * @param filename configuration file name
 *
 * @return	0 ok, -1 config file not found
 *
 */
int readCfgVal(const char * filename, const char * valuename, char * valueread)
{
	FILE *fp;
	char _label [MAX_LINE] = "";
	char _value [MAX_LINE] = "";
	char line [MAX_LINE] = "";
	fp = fopen(filename, "r");
	if (!fp)
	{
		/*
		   printf("canno open cfg fle '%s'\n", filename);
		 */
		return -1;
	}
	while (fgets(line, MAX_LINE, fp) != NULL)
	{

		if (line[0] != '#')
		{
			_label[0] = '\0';
			_value[0] = '\0';
			sscanf(line, "%s %s", _label, _value);
			if (_label[0] != '\0')
			{
				if (strcmp(_label, valuename) == 0)
				{
					strcpy(valueread, _value);
					fclose(fp);
					return 0;
				}
			}
		}
	}
	fclose(fp);
	return 1;
}

/**
 * update a configuration file
 *
 * @param filename configuration file name
 * @param label parameter name to add or update
 * @param value parameter value to add or update
 *
 * @return	0 added, 1 updated, <0 error
 *
 */
int writeCfgVal(const char * filename, const char * label, const char * value)
{
	FILE *fp, * fptmp;
	char _label [MAX_LINE] = "";
	char _value [MAX_LINE] = "";
	char line [MAX_LINE] = "";
	int found = 0;

	fp = fopen(filename, "r");
	if (!fp)
	{
		fp = fopen(filename, "w");
		fclose(fp);
		fp = fopen(filename, "r");
		if (!fp)
		{
			printf("cannot open cfg fle '%s'\n", filename);
			return -1;
		}
	}
	fptmp = tmpfile();
	if (!fptmp)
	{
		/*
		   printf("canno open cfg fle '%s'\n", filename);
		 */
		fclose(fp);
		return -2;
	}
	while (fgets(line, MAX_LINE, fp) != NULL)
	{

		if (line[0] != '#')
		{
			_label[0] = '\0';
			_value[0] = '\0';
			sscanf(line, "%s %s", _label, _value);
			if (_label[0] != '\0')
			{
				if (strcmp(_label, label) == 0)
				{
					strcpy(_value, value);
					found = 1;
				}
				fprintf(fptmp,"%s %s\n", _label, _value);
			}
			else
			{
				fprintf(fptmp,"%s", line);
			}
		}
		else
		{
			fprintf(fptmp,"%s", line);
		}
	}
	if (found == 0)
	{
		fprintf(fptmp,"%s %s\n", label, value);
	}

	fclose(fp);
	rewind(fptmp);
	fp = fopen(filename, "w");
	if (!fp)
	{
		/*
		   printf("cannot open cfg fle '%s'\n", filename);
		 */
		return -1;
	}
	while (fgets(line, MAX_LINE, fptmp) != NULL)
	{
		fprintf(fp, "%s", line);
	}

	fclose(fptmp);
	fclose(fp);
	sync();

	return found;
}

/**
 * load a configuration file with the format with a bash syntax
 *      <configuration>: <value>
 *
 * @param filename configuration file name
 *
 * @return	0 ok, -1 config file not found
 *
 */
int readCfgValBash(const char * filename, const char * valuename, char * valueread)
{
	FILE *fp;
	char label [MAX_LINE] = "";
	char value [MAX_LINE] = "";
	char line [MAX_LINE] = "";
	fp = fopen(filename, "r");
	if (!fp)
	{
		printf("canno open cfg fle '%s'\n", filename);
		return -1;
	}
	while (fgets(line, MAX_LINE, fp) != NULL)
	{
		if (line[0] != '#' && strchr(line, '=') != NULL)
		{
			label[0] = '\0';
			strcpy(value, strchr(line, '=') + 1);
			if (strchr(value, '\n') != NULL)
			{
				*strchr(value, '\n') = '\0';
			}
			*strchr(line, '=') = '\0';
			strcpy(label, line);
			// printf("line '%s' label '%s' value '%s' valuename '%s'\n", line, label, value, valuename);
			if (label[0] != '\0')
			{
				if (strcmp(label, valuename) == 0)
				{
					strcpy(valueread, value);
					fclose(fp);
					return 0;
				}
			}
		}
	}
	fclose(fp);
	return 1;
}

/**
 * update a configuration file with a bash syntax
 *
 * @param filename configuration file name
 * @param label parameter name to add or update
 * @param value parameter value to add or update
 *
 * @return	0 added, 1 updated, <0 error
 *
 */
int writeCfgValBash(const char * filename, const char * label, const char * value)
{
	FILE *fp, * fptmp;
	char _label [MAX_LINE] = "";
	char _value [MAX_LINE] = "";
	char line [MAX_LINE] = "";
	int found = 0;

	fp = fopen(filename, "r");
	if (!fp)
	{
		fp = fopen(filename, "w");
		fclose(fp);
		fp = fopen(filename, "r");
		if (!fp)
		{
			printf("cannot open cfg fle '%s'\n", filename);
			return -1;
		}
	}
	fptmp = tmpfile();
	if (!fptmp)
	{
		/*
		   printf("canno open cfg fle '%s'\n", filename);
		 */
		fclose(fp);
		return -2;
	}
	while (fgets(line, MAX_LINE, fp) != NULL)
	{

		if (line[0] != '#')
		{
			_label[0] = '\0';
			if (strchr(line, '=')== NULL)
			{
				printf("invalid line '%s'\n", line);
				fclose(fptmp);
				fclose(fp);
				sync();
				return -1;
			}
			strcpy(_value, strchr(line, '=') + 1);
			if (strchr(_value, '\n') != NULL)
			{
				*strchr(_value, '\n') = '\0';
			}
			*strchr(line, '=') = '\0';
			strcpy(_label, line);
			if (_label[0] != '\0')
			{
				if (strcmp(_label, label) == 0)
				{
					strcpy(_value, value);
					found = 1;
				}
				fprintf(fptmp,"%s=%s\n", _label, _value);
			}
			else
			{
				fprintf(fptmp,"%s", line);
			}
		}
		else
		{
			fprintf(fptmp,"%s", line);
		}
	}
	if (found == 0)
	{
		fprintf(fptmp,"%s=%s\n", label, value);
	}

	fclose(fp);
	rewind(fptmp);
	fp = fopen(filename, "w");
	if (!fp)
	{
		/*
		   printf("cannot open cfg fle '%s'\n", filename);
		 */
		return -1;
	}
	while (fgets(line, MAX_LINE, fptmp) != NULL)
	{
		fprintf(fp, "%s", line);
	}

	fclose(fptmp);
	fclose(fp);
	sync();

	return found;
}

/**
 * delete an entry from a configuration file
 *
 * @param filename configuration file name
 * @param label parameter name to add or update
 *
 * @return	0 not found, 1 deleted, <0 error
 *
 */
int deleteCfgVal(const char * filename, const char * label)
{
	FILE *fp, * fptmp;
	char _label [MAX_LINE] = "";
	char _value [MAX_LINE] = "";
	char line [MAX_LINE] = "";
	int found = 0;

	fp = fopen(filename, "r");
	if (!fp)
	{
		fp = fopen(filename, "w");
		fclose(fp);
		fp = fopen(filename, "r");
		if (!fp)
		{

			printf("cannot open cfg fle '%s'\n", filename);
			return -1;
		}
	}
	fptmp = tmpfile();
	if (!fptmp)
	{
		/*
		   printf("canno open cfg fle '%s'\n", filename);
		 */
		fclose(fp);
		return -2;
	}
	while (fgets(line, MAX_LINE, fp) != NULL)
	{

		if (line[0] != '#')
		{
			_label[0] = '\0';
			sscanf(line, "%s %s", _label, _value);
			if (_label[0] != '\0')
			{
				if (strcmp(_label, label) == 0)
				{
					found = 1;
				}
				else
				{
					fprintf(fptmp,"%s %s\n", _label, _value);
				}
			}
			else
			{
				fprintf(fptmp,"%s", line);
			}
		}
		else
		{
			fprintf(fptmp,"%s", line);
		}
	}

	fclose(fp);
	rewind(fptmp);
	fp = fopen(filename, "w");
	if (!fp)
	{
		/*
		   printf("cannot open cfg fle '%s'\n", filename);
		 */
		return -1;
	}
	while (fgets(line, MAX_LINE, fptmp) != NULL)
	{
		fprintf(fp, "%s", line);
	}

	fclose(fptmp);
	fclose(fp);
	sync();

	return found;
}

