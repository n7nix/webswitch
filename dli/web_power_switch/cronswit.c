/*
 * File: cronswit.c
 *
 *  $Id: EXP$
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#include "common.h"

#define T_SIZE 256

#include "logger.h"

#define CRONSWIT_VERSION "1.1"
/* #define RRDDIR "/home/gunn/var/lib/rrdlocweather/db/local" */
/* #define RRDDIR "/home/gunn/var/lib/rrdlocweather/db/local" */
/* #define RRDDIR "/media/disk/rrd/var/lib/cbw/rrdtemp" */

#define RRDDIR "/media/backup/rrd/var/lib/cbw/rrdtemp"
#define RRD_DATABASE_LOWPUMP "lowpumphouse"
#define RRD_DATABASE_HIGHPUMP "highpumphouse"
#define UPPER_TEMP 42
#define LOWER_TEMP 36

int TurnHeater(int on_off);
int CheckHeater(int on_off);
int read_rrdb(char *dbName);
static void usage(char *progname);
const char *getprogname(void);

extern char *__progname;

int main(int argc, char **argv)
{
	int opt;
	extern int optind;
	extern char *optarg;
        int lowpumpTemp;
        int highpumpTemp;
	char buf[512];
	int uppertemp = UPPER_TEMP;
	int lowertemp = LOWER_TEMP;

	printf("cronswit ver: %s %s\n",
	       CRONSWIT_VERSION,
	       TESTONLY ? "Test Only" : "Real version");

     /*
      *  parse the command line options and take appropriate
      *	 actions
      */
	while ((opt = getopt(argc, argv, "thu:l:")) != -1) {
		switch (opt) {
			case 't': /* Temperature check */
                                lowpumpTemp = read_rrdb(RRD_DATABASE_LOWPUMP);
                                highpumpTemp = read_rrdb(RRD_DATABASE_HIGHPUMP);
				printf("%s: temperature sensors: low: %d, high: %d  (limits: %d, %d)\n",
				       getprogname(), lowpumpTemp, highpumpTemp, uppertemp, lowertemp);
				exit(0);
				break;
			case 'h': /* help */
				usage(argv[0]);
				exit(0);
				break;
			/* upper set point value */
			case 'u':
				uppertemp = atoi(optarg);
				break;
			/* lower set point value */
			case 'l':
				lowertemp = atoi(optarg);
				break;
			case ':':
			case '?':
			default:
				printf("ERROR: invalid option usage");
				usage(argv[0]);
				/*NOTREACHED*/
				exit(2);
		}
	}

        lowpumpTemp = read_rrdb(RRD_DATABASE_LOWPUMP);
        highpumpTemp = read_rrdb(RRD_DATABASE_HIGHPUMP);

	if( lowpumpTemp !=0 && highpumpTemp != 0) {

		if( (lowpumpTemp > uppertemp)  && (highpumpTemp > uppertemp) ) {
			TurnHeater(0);

			/* write to log file */
			sprintf(buf, "cronswit: Heater off, Pump House sensors: low %d, high %d (limit=%d)",
					lowpumpTemp, highpumpTemp, uppertemp);
			logger_updatefile(buf, LOGERRORFILE_NAME);
			printf("%s\n", buf);

		} else if( (lowpumpTemp <= lowertemp) || (highpumpTemp <= lowertemp) ) {
			TurnHeater(1);

			/* write to log file */
			sprintf(buf, "cronswit: Heater on, Pump House sensors: low %d, high %d (limit=%d)",
					lowpumpTemp, highpumpTemp, lowertemp);
			logger_updatefile(buf, LOGERRORFILE_NAME);
			printf("%s\n", buf);


		} else {
			CheckHeater(1);
#if 1
			/* write to log file */
			sprintf(buf, "cronswit: No action taken, Pump House sensors: low %d, high %d (limits: %d, %d)",
					lowpumpTemp, highpumpTemp, uppertemp, lowertemp);
/*			logger_updatefile(buf, LOGERRORFILE_NAME); */
			printf("%s\n", buf);
#endif
		}
	} else {
		logger_puts("cronswit: Error reading rrdb", LOGGER_ERRORFILE);
	}

	return(0);
}

int read_rrdb(char *dbName)
{
	int lastTemp=0;
	int sysRetVal;
	char sysCallBuf[128];
	char rrdBuf[512];
	FILE *fp;

	sprintf(sysCallBuf, "rrdtool info %s/%s.rrd | grep -i last_ds | cut -d'%s' -f2 > rrdval.txt",
		  RRDDIR, dbName, "\"");

	sysRetVal = system(sysCallBuf);


	if(sysRetVal != 0) {
		printf("Syscall failed: %s\n", sysCallBuf);
		return(-1);
	}



	if( ( fp = fopen( "rrdval.txt", "r" ) ) != NULL ) {
		if(fgets(rrdBuf, 80,fp ) !=0 ) {
			lastTemp =  atoi(rrdBuf);
			if(TESTONLY) {
				printf("rrdBuf: %s %d\n", rrdBuf, lastTemp);
			}
		} else {
			printf("Nothing in file\n");
		}

	} else {
		printf("Failed to open rrdval.txt\n");
	}


	return(lastTemp);
}

void logger_updatefile(char *buf, char *fileName)
{
	char dir_filename[128];
	char strfTime[T_SIZE];
	struct tm loctime;
	time_t now;
	FILE *fp;

	sprintf(dir_filename, "%s%s",
		  LOGFILE_DIR,
		  fileName);

	if((fp = fopen(dir_filename, "a")) == NULL)
	{
		printf("Error: opening log file: %s: %s\n",
		       dir_filename, strerror(errno));
	} else {

		/* get current time */
		now = time(NULL);
		localtime_r(&now, &loctime);
#if 0
		/* make a time string of Month day hour:minute:sec am/pm */
		strftime(strfTime, T_SIZE, "%b %d %I:%M %p - ", &loctime);

		/* make a time string with preferred data & time of
		 * locale */
		strftime(strfTime, T_SIZE, "%c - ", &loctime);

#else
		/* make a time string of Month day hour:minute yr */
		strftime(strfTime, T_SIZE, "%b %d:%y %H:%M:%S - ", &loctime);
#endif

		fputs(strfTime, fp);
		fputs(buf, fp);
		fputs("\n", fp);
		fclose(fp);

	}
}

void logger_puts (char *buf, int fileTypeFlag)
{
	char datebuf[T_SIZE];
	time_t now;
	struct tm loctime;
	int fileFlag = fileTypeFlag;
	int fileBit = 0x01;

	while(fileFlag) {
		switch(fileFlag & fileBit) {

			case LOGGER_STDOUT:
				puts(buf);
				break;

			case LOGGER_T_STDOUT:
				/* get current time -> time_t (seconds since epoch) */
				now = time(NULL);
				localtime_r(&now, &loctime);
				if(now) {
					strftime( datebuf,
						T_SIZE, "%b %d %H:%M:%S",
						&loctime);
					printf("%s: ", datebuf);
				}


				puts(buf);
				break;

			case LOGGER_ERRORFILE:
				logger_updatefile(buf, LOGERRORFILE_NAME);
				break;

			case LOGGER_DATAFILE:
				logger_updatefile(buf, LOGDATAFILE_NAME);
				break;

			case LOGGER_MAXMINFILE:
				logger_updatefile(buf, LOGMAXMINFILE_NAME);
				break;

			default:
				break;

		}
		fileFlag &= ~fileBit;
		fileBit = fileBit << 1;
	}
}

const char *getprogname(void)
{
	return __progname;
}

/*
 * Print usage information and exit
 *  -does not return
 */
static void
   usage(char *progname)
{
	printf("Usage: %s [options]\n", progname);
	printf("  -h        Display this usage info\n");
	printf("  -u <val>  Set upper temperature value\n");
	printf("  -l <val>  Set lower temperature value\n");
	exit(0);
}
