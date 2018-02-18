/* This is probably wrong
 *
 * Commands:
 * FIRST channel commands:
 * OFF command : FF 01 00 (HEX) or 255 1 0 (DEC)
 * ON command : FF 01 01 (HEX) or 255 1 1 (DEC)
 *
 * Needs ftdi library:
 *     apt-get install libftdi1 libftdi-dev libusb-dev libusb-1.0-0 libusb-1.0-0-dev
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdint.h>

#include "relay_drv.h"

#define PROG_VERSION "1.0"
#define DEFAULT_RELAY_ACTION "UNDEFINED"
static void usage(void);
const char *getprogname(void);
void strToUpper(char *sPtr);
void readall_relays(void);

extern char *__progname;
int gverbose_flag = false;
int DebugFlag = 0;


int main(int argc, char *argv[])
{
	/* required by crelay */
	relay_info_t *relay_info;
	char cname[MAX_RELAY_CARD_NAME_LEN];
	char com_port[MAX_COM_PORT_NAME_LEN];
	uint8_t num_relays=FIRST_RELAY;
	char* serial=NULL;
	relay_state_t rstate;
	int err, i;

	int relay_num = 1; /* default to first relay */
	char *relay_action=DEFAULT_RELAY_ACTION;
	int retval =  EXIT_SUCCESS;

	/* For command line parsing */
	int next_option;
	extern int optind;

	/* short options */
	static const char *short_options = "ivh";

	/* long options */
	static struct option long_options[] = {
		/* These options set a flag. */
		{"verbose",       no_argument,  &gverbose_flag, true},
		{"help",          no_argument,  NULL, 'h'},
		{"info",          no_argument,  NULL, 'i'},
		{NULL, no_argument, NULL, 0} /* array termination */
	};

	/*
	 * Get config from command line
	 */

	opterr = 0;
	optind = 0;

	while( (next_option = getopt_long (argc, argv, short_options, long_options, &optind)) != -1 ) {

		switch (next_option) {
			case 'i':
				/* Detect all usb devices connected to the system */
				if (crelay_detect_all_relay_cards(&relay_info) == -1) {
					printf("No compatible usb device detected.\n");
					return -1;
				}
				printf("\nDetected relay cards:\n");
				i = 1;
				while (relay_info->next != NULL) {
					crelay_get_relay_card_name(relay_info->relay_type, cname);
					printf("  #%d\t%s (serial %s)\n", i++ ,cname, relay_info->serial);
					relay_info = relay_info->next;
					// TODO: free relay_info list memory
				}

				exit(EXIT_SUCCESS);

				;
			case 'v': /* set verbose flag */
				gverbose_flag = true;
				break;
			case 'h':
				usage();  /* does not return */
				break;
			case '?':
				if (isprint (optopt)) {
					fprintf (stderr, "%s: Unknown option `-%c'.\n",
						getprogname(), optopt);
				} else {
					fprintf (stderr,"%s: Unknown option character `\\x%x'.\n",
						getprogname(), optopt);
				}
				/* fall through */
			default:
				usage();  /* does not return */
				break;
		}
	}

	if( argc < 2 ) {
		readall_relays();
		exit(EXIT_SUCCESS);
	}

	if(optind < argc) {

		relay_num = atoi(argv[optind]);
		if(gverbose_flag) {
			printf("Set relay number to: %d\n", relay_num);
		}
		optind++;
	}
	if(optind < argc) {

		relay_action = argv[optind];
		strToUpper(relay_action);
		printf("Set relay %d state to: %s\n", relay_num, relay_action);
		optind++;
	}
	/* returns with com_port & num_relays */
	if (crelay_detect_relay_card(com_port, &num_relays, serial, NULL) == -1)
	{
		uid_t uid=geteuid();

		printf("No compatible device detected for user id: %d.\n", uid);

		if(geteuid() != 0)
		{
			printf("\nWarning: this program is currently not running with root privileges !\n");
			printf("Therefore it might not be able to access your relay card communication port.\n");
			printf("Consider invoking the program from the root account or use \"sudo ...\"\n");
		}

		exit(EXIT_FAILURE);
	}

	if(gverbose_flag) {
		printf("Detected %d relays, type: %s\n", num_relays, com_port);
	}

	err = 0;
	if (strcmp(relay_action, "OFF") == 0) {
		err = crelay_set_relay(com_port, relay_num, OFF, serial);
	} else if (strcmp(relay_action, "ON") == 0) {
		err = crelay_set_relay(com_port, relay_num, ON, serial);
	} else {
		/* GET current relay state */
		if (crelay_get_relay(com_port, relay_num, &rstate, serial) == 0) {
			printf("Relay %d is %s\n", relay_num, (rstate==ON)?"on":"off");
		} else {
			printf("Failure getting relay state\n");
			retval = EXIT_FAILURE;
		}
	}
	if(err) {
		printf("Error setting relay: %d to %s\n", relay_num, relay_action);
		retval = EXIT_FAILURE;
	}

	return(retval);
}

void readall_relays(void)
{
	char com_port[MAX_COM_PORT_NAME_LEN];
	uint8_t num_relays=FIRST_RELAY;
	char* serial=NULL;
	relay_state_t rstate[8];
	int i;

	/* returns with com_port & num_relays */
	if (crelay_detect_relay_card(com_port, &num_relays, serial, NULL) == -1)
	{
		uid_t uid=geteuid();

		printf("No compatible device detected for user id: %d.\n", uid);

		return;
	}
	for (i = 0; i < num_relays; i++) {
		/* GET current relay state */
		if (crelay_get_relay(com_port, i+1, &rstate[i], serial) == 0) {
		} else {
			printf("Failure getting relay state for relay %d\n", i+1);
		}
	}

	for(i = 0; i < num_relays; i++) {
		printf("%4d ", i+1);
	}
	printf("\n");

	for(i = 0; i < num_relays; i++) {
		printf(" %4s", (rstate[i]==ON)?"on":"off");
	}
	printf("\n");
}

void strToUpper(char *strPtr)
{
	while(*strPtr != '\0') {
		*strPtr = toupper((unsigned char)*strPtr);
		strPtr++;
	}
}

const char *getprogname(void)
{
	return __progname;
}

/*
 * Print usage information and exit
 *  - does not return
 */
static void usage(void)
{
	printf("Usage:  %s [-i] [<relay number>] [on|off] \n", getprogname());
	printf("  Version: %s\n", PROG_VERSION);
	printf("\n");
	printf("  relay number 1-8\n");
	printf("  no args          Display state of all relays\n");
	printf("  -i  --info       Display info on device\n");
	printf("  -v  --verbose    Print verbose messages\n");
	printf("  -h  --help       Display this usage info\n");

	exit(EXIT_SUCCESS);
}
