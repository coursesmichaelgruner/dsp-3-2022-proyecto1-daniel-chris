/** @file simple_client.c
 *
 * @brief This simple client demonstrates the most basic features of JACK
 * as they would be used by many applications.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h> /* Functions to parse the command line arguments */

#include "project.h"
#include <jack/jack.h>

jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *client;

const int channels       = 2;
const int bytesPerSample = 2;

int Fs;
int verbose;
int capture=0;

struct dataType {
  int wnd;           /* Counter for current window in buffer */
  int wndSize;       /* Total window size in samples         */
  int rawBufferSize; /* Size of buffer with raw data from file in samples */
  float* rawBuffer;  /* Buffer with raw data from file       */
};

int readFile(float* buffer,int samples) {
  const int bytes = samples*2; /*sizeof(float);*/
  char raw[bytes];
  int rc = read(0,raw,bytes);
  if (rc == 0) {
    fprintf(stderr, "end of file on input\n");
    return 0;
  } else if (rc != bytes) {
    memset(raw+rc,0,bytes-rc);
  }
  short* ptr = (short*)raw; /* see the memory as short ints */ 
  const short* eptr = ptr+samples;
  while (ptr!=eptr) {
    (*buffer++) = (float)(*ptr++)/32768;
  }
  return 1;
}


/**
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 *
 * This client does nothing more than copy data from its input
 * port to its output port. It will exit when stopped by 
 * the user (e.g. using Ctrl-C on a unix-ish operating system)
 */
int process_(jack_nframes_t nframes, void *arg) {
	jack_default_audio_sample_t *in, *out;
	
  if (capture!=0) {
    in = jack_port_get_buffer (input_port, nframes);
  } else {
    /* 
     * The arg is a pointer to an integer holding if the input should be
     * taken from a file, overwriting the captured data
     */
    struct dataType* dataPtr = (struct dataType*)arg;

    // update underlying buffer reading process
    dataPtr->wndSize = nframes;
    int currentBuffer = dataPtr->wnd;
    int next = currentBuffer+1;
    if ((next+1)*dataPtr->wndSize > dataPtr->rawBufferSize) {
      next=0;
    }
    dataPtr->wnd = next; /* indicate the underlying process to keep reading */

    /* transfer the data to the input buffer */
    in = (float*)(dataPtr->rawBuffer + dataPtr->wndSize*currentBuffer);
  } /* end of else capture */

	out = jack_port_get_buffer (output_port, nframes);
	
  return process(Fs,nframes,in,out);
}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void
jack_shutdown (void *arg)
{
	exit (1);
}

int
main (int argc, char *argv[])
{
	const char **ports;
	const char *client_name = "simple";
	const char *server_name = NULL;
	jack_options_t options = JackNullOption;
	jack_status_t status;
	
  // We use the standard getopt.h functions here to parse the arguments.
  // Check the documentation of getopt.h for more information on this
  
  // structure for the long options. 
  static struct option lopts[] = {
    {"verbose"  ,no_argument,0,'v'},
    {"help"     ,no_argument,0,'h'},
    {0,0,0,0}
  };

  int optionIdx,c;
  verbose=0;
  while ((c = getopt_long(argc, argv, "vhc",lopts,&optionIdx)) != -1) {
    switch (c) {
    case 'c':
      capture=1;
      break;
    case 'v':
      verbose=1;
      break;
    case 'h':
      printf("Usage: %s [-v|--verbose] [-h|--help] [-c]\n",argv[0]);
      printf("       -c indicates to get the data from the std input\n");
      break;
    default:
      printf("Option '-%c' not recognized.\n",c);
    }
  }


	/* open a client connection to the JACK server */

	client = jack_client_open (client_name, options, &status, server_name);
	if (client == NULL) {
		fprintf (stderr, "jack_client_open() failed, "
			 "status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		exit (1);
	}
	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		fprintf (stderr, "unique name `%s' assigned\n", client_name);
	}

  struct dataType data;
  data.wnd  = 0;
  data.wndSize = 0;
  data.rawBufferSize = 1<<16;

  float localData[data.rawBufferSize];
  data.rawBuffer = localData;
  memset(data.rawBuffer,0,data.rawBufferSize*sizeof(float));

	/* tell the JACK server to call `process()' whenever
   * there is work to be done.
   */
	jack_set_process_callback(client, process_, &data);

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/

	jack_on_shutdown (client, jack_shutdown, 0);

	/* display the current sample rate. 
	 */

  const int Fs = 	jack_get_sample_rate (client);

	printf ("engine sample rate: %d\n",Fs);

	/* create two ports */

	input_port = jack_port_register (client, "input",
                                   JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsInput, 0);
	output_port = jack_port_register (client, "output",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);
  
	if ((input_port == NULL) || (output_port == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		exit (1);
	}
  
  /* Call the user initialization stuff
   */
  init(Fs);

	/* Tell the JACK server that we are ready to roll.  Our
	 * process() callback will start running now. */

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		exit (1);
	}

	/* Connect the ports.  You can't do this before the client is
	 * activated, because we can't make connections to clients
	 * that aren't running.  Note the confusing (but necessary)
	 * orientation of the driver backend ports: playback ports are
	 * "input" to the backend, and capture ports are "output" from
	 * it.
	 */

	ports = jack_get_ports (client, NULL, NULL,
                          JackPortIsPhysical|JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical capture ports\n");
		exit (1);
	}

  /* connect left microphone */
	if (jack_connect (client, ports[0], jack_port_name (input_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

  /* connect left microphone */
	if (jack_connect (client, ports[1], jack_port_name (input_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

	free(ports);
	
	ports = jack_get_ports (client, NULL, NULL,
                          JackPortIsPhysical|JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
		exit (1);
	}

  /* connect left speaker */
	if (jack_connect (client, jack_port_name (output_port), ports[0])) {
		fprintf (stderr, "cannot connect output ports\n");
	}

  /* connect right speaker */
	if (jack_connect (client, jack_port_name (output_port), ports[1])) {
		fprintf (stderr, "cannot connect output ports\n");
	}

	free(ports);

  if (capture != 0) {
    printf("\nCapture from microphone\n\n");
  } else {
    printf("\nProcess from standard input\n\n");
  }

  char* progbar="-/|\\";
  int progidx=0;

	/* keep running until stopped by the user */
  int ok=1;
  int wait=(capture!=0) ? 250000 : 1000;
  int wnd = 0;
  int firstTime = 1;
  int abortRQ=1; /* in negated logic */
  while(ok!=0) {
    if (capture == 0) {
      if (wnd != data.wnd) {
        wnd = data.wnd;

        printf("%i ",wnd);
        fflush(stdout);

        /* we know here the window size */
        int wndSize = data.wndSize;
        int halfBuffer = (data.rawBufferSize/wndSize/2)*wndSize;
        
        /* Already an update was done!  now should we start reading stuff? */
        if (firstTime != 0) {
          printf("\nFirst time: read all\n");
          fflush(stdout);

          firstTime = 0;
          // read for start asap
          abortRQ=readFile(data.rawBuffer+(wnd+1)*wndSize,
                           2*halfBuffer-(wnd+1)*wndSize);

        } else if (wnd == 0) {
          if (abortRQ==0) {
            ok=0;
          } else {
            /* we have to read the second half */
            printf("\nReading second half...");
            fflush(stdout);

            abortRQ=readFile(data.rawBuffer+halfBuffer,halfBuffer);

            printf(" done.\n");
            fflush(stdout);

          }
        } else if (wnd == data.rawBufferSize/wndSize/2) {
          if (abortRQ==0) {
            ok=0;
          } else {
            /* we have to read the first half */
            printf("\nReading first half...");
            fflush(stdout);

            abortRQ=readFile(data.rawBuffer,halfBuffer);

            printf(" done.\n");
            fflush(stdout);

          }
        }
      }
    } else {
      printf("%c\r",progbar[progidx++&3]);
      fflush(stdout);
    }

    usleep (wait); // wait 1 ms

  }

	/* this is never reached but if the program
	   had some other way to exit besides being killed,
	   they would be important to call.
	*/
	jack_client_close (client);
	exit (0);
}
