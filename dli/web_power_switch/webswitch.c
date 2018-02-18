#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <curl/curl.h>

#include "common.h"
#include "logger.h"

/* #define DEBUG 1 */

static int doCurlAction(int portNum, int dur_action);
static int doCurlVerify(int ipAddr, int portnum, int swState);
extern void     msDelay(int);

char *plibCurlCodes[] = {
	"OK",
	"Unsupported Protocol",
	"Failed Init",
	"URL MalFormat",
	"URL MalFormat user",
	"Couldn't resolve proxy",
	"Couldn't resolve host",
	"Couldn't connect",
	" "
};

/*
curl_version_info_data *curl_version_info( CURLversion type );
*/
size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data);

struct MemoryStruct {
	char *memory;
	size_t size;
};


/*
 * Ping verify - debug
 */
bool pingVerify(void)
{
	int pingRetval;
	bool bRetVal = TRUE;

	pingRetval = system("ping -c1 -s32 10.0.42.71 > /dev/null");
	if(pingRetval != 0)
		bRetVal = FALSE;

	return(bRetVal);
}

/* Verify micro is working */

bool microCheck(void)
{
	int i;
	bool bRetVal = TRUE;
	int pingErrCnt = 0;

	for(i = 0; i < 5; i++) {
		if( system("ping -c1 -s32 10.0.42.71 > /dev/null") != 0) {
			pingErrCnt++;
		}
	}

	/* Criteria for failure is ping failure 4 or 5 times */
	if(pingErrCnt > 3)
		bRetVal = FALSE;

	return(bRetVal);
}


/* Verify webswitch connection is working */

bool webSwitchCheck(void)
{
	int i;
	bool bRetVal = TRUE;
	int pingErrCnt = 0;

	printf("Debug: webSwitchCheck\n"); fflush(stdout);

	for(i = 0; i < 5; i++) {
		if( system("ping -c1 -s32 10.0.42.54 > /dev/null") != 0) {
			pingErrCnt++;
		}
	}

	/* Criteria for failure is ping failure 4 or 5 times */
	if(pingErrCnt > 3)
		bRetVal = FALSE;

	return(bRetVal);
}

/*
 * curlErrHandler - curl library error handler
 */
void curlErrHandler(char *funcName, char *libName, CURLoption option, CURLcode curlResult, char *memoryAlloc)
{
	char logBuf[LOG_BUF_SIZE];
	char *pErrorStr;

	if(curlResult > 0 && curlResult < 8) {
		pErrorStr = plibCurlCodes[curlResult];
	} else {
		pErrorStr = plibCurlCodes[8];
	}

	sprintf(logBuf, "%s: libcall: %s, option 0x%02x failed with error (0x%02x) - %s",
		  funcName, libName, option, curlResult, pErrorStr);
	puts(logBuf);
	logger_puts(logBuf, LOGGER_ERRORFILE);

	if(memoryAlloc)
		free(memoryAlloc);
}

int TurnHeater(int on_off)
{
	int retVal=-1;
	int portNum = 1;
	char logBuf[LOG_BUF_SIZE];

	if(doCurlVerify( 54, portNum, on_off)) {
		sprintf(logBuf,"Heater on port %d already in %s state",
			   portNum,
				on_off ? "On" : "Off");
		puts(logBuf);
		logger_puts(logBuf, LOGGER_ERRORFILE);

	}

	if(TESTONLY) {
		printf("TurnHeater TestOnly, port number %d to %s\n",
		       portNum,
		       on_off ? "On" : "Off");
	} else {
		retVal = doCurlAction(portNum, on_off);
		if(retVal !=0) {
			logger_puts("webswitch: doCurlAction error", LOGGER_ERRORFILE);
		}
	}
	return(retVal);
}

int CheckHeater(int on_off)
{
	int portNum = 1;
	char logBuf[LOG_BUF_SIZE];
	int retVal=FALSE;

	if(doCurlVerify( 54, portNum, on_off)) {
		sprintf(logBuf,"Heater on port %d already in %s state",
				portNum,
				on_off ? "On" : "Off");
		puts(logBuf);
		logger_puts(logBuf, LOGGER_ERRORFILE);
		retVal = TRUE;

	} else {
#ifdef DEBUG
		sprintf(logBuf,"WARNING: Heater on port %d NOT in %s state",
				portNum,
				on_off ? "On" : "Off");
		puts(logBuf);
		logger_puts(logBuf, LOGGER_ERRORFILE);
#endif
		retVal = FALSE;
	}

	return(retVal);
}
/*
 * curlaction - use curl to turn web switch on or off
 *
 * micro is plugged into web switch
 */

static int doCurlAction(int portNum, int dur_action)
{
	struct MemoryStruct chunk;
	CURL *curl_handle;
	CURLcode curlResult;
	char urlString[64];
	char logBuf[LOG_BUF_SIZE];

	int ipAddr = 54;
	char strAction[8]="NULL";
	int  retCode = 0;	/* init return value */


	chunk.memory = NULL;  /* we expect realloc(NULL, size) to work */
	chunk.size = 0;	      /* no data at this point */

	if(dur_action == 0 || dur_action == 1) {
		sprintf(strAction, "%s", dur_action ?"ON" : "OFF");
	} else {
		sprintf(logBuf, "curlaction: invalid action arg 0x%02x", dur_action);
		puts(logBuf);
		logger_puts(logBuf, LOGGER_ERRORFILE);
		retCode=-2;
		goto doCurlAction_Exit;
	}

	/* action applies to a specific port */
	sprintf(urlString,"http://10.0.42.%d/outlet?%d=%s",
		  ipAddr, portNum, strAction);

	sprintf(logBuf, "doCurlAction: portnum %d, action: %d, url: %s\n", portNum, dur_action, urlString);
	logger_puts(logBuf, LOGGER_T_STDOUT| LOGGER_ERRORFILE);

	curl_handle = curl_easy_init();

	if(curl_handle) {

		curlResult = curl_easy_setopt(curl_handle, CURLOPT_USERPWD, "gunn:9sydhobbs");
		if(curlResult != 0) {
			curlErrHandler("curlaction", "curl_easy_setopt", CURLOPT_USERPWD, curlResult, chunk.memory);
			retCode=-3;
			goto doCurlAction_Exit;
		}

		/* specify URL to get */
		curlResult = curl_easy_setopt(curl_handle, CURLOPT_URL, urlString);
		if(curlResult != 0) {
			curlErrHandler("curlaction", "curl_easy_setopt", CURLOPT_URL, curlResult, chunk.memory);
			retCode=-3;
			goto doCurlAction_Exit;
		}

		/* send all data to this function  */
		curlResult = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		if(curlResult != 0) {
			curlErrHandler("curlaction", "curl_easy_setopt", CURLOPT_WRITEFUNCTION, curlResult, chunk.memory);
			retCode=-3;
			goto doCurlAction_Exit;
		}

		/* we pass our 'chunk' struct to the callback function */
		curlResult = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
		if(curlResult != 0) {
			curlErrHandler("curlaction", "curl_easy_setopt", CURLOPT_WRITEDATA, curlResult, chunk.memory);
			retCode=-3;
			goto doCurlAction_Exit;
		}

		/* get it */
#if 0 /* DEBUG */
		printf("Enter curl_easy_perform ...\n");
#endif /* DEBUG */

		curlResult = curl_easy_perform(curl_handle);

#if 0 /* DEBUG */
		printf("Exit 0x%02x\n", curlResult); /* DEBUG */
#endif /* DEBUG */

		if(curlResult != 0) {
			curlErrHandler("curlaction", "curl_easy_perform", 0, curlResult, chunk.memory);
			retCode=-3;
			goto doCurlAction_Exit;
		}

		/* always cleanup */
		curl_easy_cleanup(curl_handle);

		/*
		 * Now, our chunk.memory points to a memory block that is chunk.size
		 * bytes big and contains the remote file.
		 * Don't do anything with it, just keeps the console
		 * for looking messy.
		 */

	} else {
		curlErrHandler("curlaction", "curl_easy_init", 0, 0, chunk.memory);
		retCode=-3;
		goto doCurlAction_Exit;
	}

	/* return 0 on Success */

doCurlAction_Exit:
	if(chunk.memory)
		free(chunk.memory);

#if 0
	sprintf(logBuf, "doCurlAction: Exit");
	logger_puts(logBuf, LOGGER_T_STDOUT| LOGGER_ERRORFILE);
#endif
	return(retCode);
}

/*
 *  Curlverify - verify state of a single port
 *
 *  Return codes:
 *    0 port not in state to check
 *    1 port is in the state to check
 *   -1 invalid port number arg
 *   -2 invalid action arg
 *   -3 curl library call error
 *   -4 curl memory or search error
 */
static int doCurlVerify(int ipAddr, int portnum, int swState)
{
	CURL *curl_handle;
	CURLcode curlResult;
	char urlString[64];
	char logBuf[LOG_BUF_SIZE];
	struct MemoryStruct chunk;
	int portNum=9;
	int swStateNow=-1;

	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	portNum = portnum;

#ifdef DEBUG
	printf("Entering doCurlVerify ipAddr = 0x%d, portnum = 0x%x, state = 0x%x\n",
	       ipAddr, portnum, swState);
#endif /* DEBUG */

	/* sanity check port number 1-8 or a for all ports*/
	if(portNum > 0 && portNum < 9) {
		/* action applies to a specific port */


	} else {
		sprintf(logBuf, "curlverify: invalid port number arg 0x%02x", portNum);
		puts(logBuf);
		logger_puts(logBuf, LOGGER_ERRORFILE);

		return (-1);
	}

	/* sanity check action */
	if(swState != 0 && swState != 1) {
		sprintf(logBuf, "curlverify: invalid action arg 0x%02x", swState);
		puts(logBuf);
		logger_puts(logBuf, LOGGER_ERRORFILE);

		return (-2);
	}

	sprintf(urlString,"http://10.0.42.%d/index.htm",ipAddr);

#ifdef DEBUG /* DEBUG only */
	printf("State : port %d, %d\n", portNum, swState);
	printf("Sending URL cmd --> %s\n", urlString);
#endif	/* end DEBUG */


	curl_handle = curl_easy_init();
	if(curl_handle) {
		curlResult = curl_easy_setopt(curl_handle, CURLOPT_USERPWD, "gunn:9sydhobbs");
		if(curlResult != 0) {
			curlErrHandler("curlverify",
				       "curl_easy_setopt",
				       CURLOPT_USERPWD,
				       curlResult,
				       chunk.memory);
			return(-3);
		}

		/* specify URL to get */
		curlResult = curl_easy_setopt(curl_handle, CURLOPT_URL, urlString);
		if(curlResult != 0) {
			curlErrHandler("curlverify",
				       "curl_easy_setopt",
				       CURLOPT_URL,
				       curlResult,
				       chunk.memory);
			return(-3);
		}

		/* send all data to this function  */
		curlResult = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		if(curlResult != 0) {
			curlErrHandler("curlverify",
				       "curl_easy_setopt",
				       CURLOPT_WRITEFUNCTION,
				       curlResult,
				       chunk.memory);
			return(-3);
		}

			/* we pass our 'chunk' struct to the callback
			 * function */
		curlResult = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
		if(curlResult != 0) {
			curlErrHandler("curlverify",
				       "curl_easy_setopt",
				       CURLOPT_WRITEDATA,
				       curlResult,
				       chunk.memory);
			return(-3);
		}

		/* get it */
		curlResult = curl_easy_perform(curl_handle);
		if(curlResult != 0) {
			curlErrHandler("curlverify",
				       "curl_easy_perform",
				       0,
				       curlResult,
				       chunk.memory);
			return(-3);
		}


		/* always cleanup */
		curl_easy_cleanup(curl_handle);

			/*
			 * Now, our chunk.memory points
			 *  to a memory block that is chunk.size
			 *  bytes big and contains the remote file.
			 */
		{
					/* Search for this string:
					 * <td>Outlet 1 - Bus A</td><td>
					 */
			int j;
			char *pMem = chunk.memory;
			char *strPtr;
			char strBuf[32];
			char strSearch1[]="Outlet";
#if 0
			char strSearch2[]="- Bus ";
#endif
			char strSearch3[]="font color=";

			if(pMem == NULL) {
				sprintf(logBuf, "curlverify: Error: about to pass null memory ptr to strstr %p 0x%02zx\n",
					  chunk.memory, chunk.size);
				puts(logBuf);
				logger_puts(logBuf, LOGGER_ERRORFILE);

				if(chunk.memory) {
					free(chunk.memory);
				}
				return(-4);

			}
#if 0
			sprintf(strBuf, "%s %d %s",strSearch1, portNum, strSearch2);
#else
			sprintf(strBuf, "%s %d",strSearch1, portNum);

#endif
			strPtr = strstr(pMem, strBuf);

			if(strPtr == NULL) {
				sprintf(logBuf, "curlverify: String search failed(1): %s", strBuf);
				puts(logBuf);
				logger_puts(logBuf, LOGGER_ERRORFILE);

				if(chunk.memory) {
					free(chunk.memory);
				}
				return(-4);

			}

			strPtr = strstr(strPtr, strSearch3);
			if(strPtr == NULL){
				sprintf(logBuf, "curlverify: String search failed(2): %s", strSearch3);
				puts(logBuf);
				logger_puts(logBuf, LOGGER_ERRORFILE);

				if(chunk.memory) {
					free(chunk.memory);
				}
				return(-4);

			}
			strPtr = strpbrk(strPtr, ">");
			if(strPtr == NULL) {
				sprintf(logBuf, "curlverify: String search failed(3): strPtr = NULL");
				puts(logBuf);
				logger_puts(logBuf, LOGGER_ERRORFILE);

				if(chunk.memory) {
					free(chunk.memory);
				}
				return(-4);

			}
					/*
					 *	Final Step:
					 *	  get string pointer past the > to point at ON or OFF
					 */
			strPtr++;
			if(strncmp(strPtr, "OFF",3) == 0) {
				swStateNow = 0;
			} else if (strncmp(strPtr, "ON",2) == 0) {
				swStateNow = 1;
			} else { /* Failed to find ON or OFF string */
				sprintf(logBuf, "curlverify: string search failed on outlet %d, expected ON or OFF\n", portNum);
				puts(logBuf);
				logger_puts(logBuf, LOGGER_ERRORFILE);

							/* dump some of
							 * the buffer
							 * for debug */
				for(j = 0; j < 10; j++) {
					printf("%c", *strPtr++);
				}
				printf("\n");

				if(chunk.memory) {
					free(chunk.memory);
				}
				return(-4);

			}
		}
	} else {
		curlErrHandler("curlverify",
			       "curl_easy_init",
			       0,
			       0,
			       chunk.memory);
		return(-3);
	}

	/*
	 * You should be aware of the fact that at this point we might have an
	 * allocated data block, and nothing has yet deallocated that data. So when
	 *you're done with it, you should free() it as a nice application.
	 */
	if(chunk.memory)
		free(chunk.memory);

	return (swState == swStateNow);
}

/*
 * myrealalloc
 */
void *myrealloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */
	if(ptr)
		return realloc(ptr, size);
	else
		return malloc(size);
}

/*
 * WriteMemoryCallback
 */
size_t
   WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)data;

	mem->memory = (char *)myrealloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory) {
		memcpy(&(mem->memory[mem->size]), ptr, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
	} else {
		printf("WriteMemoryCallback: Error: myrealloc returned NULL ptr for size %zd\n",
		       mem->size + realsize + 1);
	}

	return realsize;
}
