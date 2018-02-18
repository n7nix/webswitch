#define LOG_BUF_SIZE 128

void logger_puts (char *buf, int fileType);

/* define bit positions for file to write */
#define LOGGER_STDOUT		0x01
#define LOGGER_ERRORFILE	0x02
#define LOGGER_DATAFILE		0x04
#define LOGGER_MAXMINFILE	0x08
#define LOGGER_T_STDOUT		0x10

#define LOGERRORFILE_NAME "cronswitch_err.log"
#define LOGDATAFILE_NAME "cronswitch_data.log"
#define LOGMAXMINFILE_NAME "cronswitch_maxmin.log"

#if defined(LINUX)
#define LOGFILE_DIR "/home/gunn/var/log/"
#elif defined(FREEBSD)
#define LOGFILE_DIR "/usr/home/gunn/var/log/"
#else /* NOT SUPPORTED */
#error OS not supported
#endif

void logger_updaterrd(char *dbName, float rrdvalue);
void logger_updatefile(char *buf, char *fileName);
void logger_puts (char *buf, int fileTypeFlag);
