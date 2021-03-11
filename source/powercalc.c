/*
deltaBox - Algorithmic Non-Intrusive Load Monitoring

Copyright (c) 2008-2009 Mikael Mieskolainen.
Licensed under the MIT License <http://opensource.org/licenses/MIT>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include <cstdlib>

extern "C" {
	#include <stdio.h>
	#include <signal.h>
	#include <sys/time.h>
	#include <fcntl.h>
	#include <time.h>
	#include <string.h>
}

#include "ow.h"
#include "io.h"
#include "boxfunctions.h"

// DEFINE LOCK FILE
#define LOCK_FILE "/var/run/powercalc.pid"

// This flag controls termination of the main loop.
volatile sig_atomic_t MAIN_LOOP_ALIVE = 1;


void signal_handler(int sig);


int main() {

	// ====================================================
	// Järjestelmäprosessin alustuskäskyt
	{
		pid_t id;
		id = fork();
		
		// FORK 1
		if (id<0) return EXIT_FAILURE; // fork error 
		if (id>0) return EXIT_FAILURE; // parent exits 
		
		// CHILD (daemon) continues -->
		
		// Obtain a new process group
		if (setsid() < 0) return EXIT_FAILURE; 
		
		// FORK 2
		if (id<0) return EXIT_FAILURE; // fork error 
		if (id>0) return EXIT_FAILURE; // parent exits
		
		// Loput alustuskäskyt
			chdir("/"); // change running directory
		
		#ifndef DEBUG
		closeall(0); //close all open Filedescriptors
		open("/dev/null",O_RDWR); // open stdin -> /dev/null	
		dup(0); // stdout
		dup(0); // stderr
		#endif
		
		umask(0); // set newly created file permissions
		
		// ----------------------------------------------------
		
		int lfp = 0;
		char str[10];
		
		lfp = open(LOCK_FILE,O_RDWR|O_CREAT,0640);
		
		if (lfp<0) return EXIT_FAILURE; // can not open
		
		if (lockf(lfp,F_TLOCK,0)<0) return EXIT_FAILURE; // can not lock	
		// --> there is already instance running
		
		sprintf(str,"%d\n",getpid());
		write(lfp,str,strlen(str)); // record pid to lockfile
		
		// ----------------------------------------------------
		
		signal(SIGHUP, signal_handler); // catch hangup signal
		signal(SIGTERM, signal_handler); // catch kill signal
		
		// ----------------------------------------------------
		
		// All memory currently allocated and all memory that
		// gets allocated in the future will be locked
		
		/*
		 //tässä PITÄISI huomioida säikeiden pinokoot -> MAHDOLLINEN ONGELMA
		if ( mlockall(MCL_CURRENT|MCL_FUTURE) == -1 ) {
			return EXIT_FAILURE;
		}
		*/
	}
	// ====================================================
    
	// Aikaloopin muuttujat	
	time_t now; struct tm *mainTime;
	time(&now); mainTime = localtime(&now);
    int write_min = mainTime->tm_min;
	
    // ----------------------------------------------------
	
	// Luodaan oliot
	IO io;
	OW ow;
	
	while (MAIN_LOOP_ALIVE == 1) { //MAIN LOOP
	
		// Luetaan epoch
		time(&now);
	
		// Muutetaan kuluneet sekunnit tietueeseen
		mainTime = localtime(&now);
		
		// viiden sekunnin välein
		if ( (now % 5) == 0 ) {
			// haetaan uusimmat asetukset
			if (io.read_io() == false) {
				#ifdef DEBUG
				cout << "Error in config.db (IO) file" << endl;
				#endif
			}
		}
		
		// minuutin välein
		if ( mainTime->tm_min != write_min ) {
			
			write_min = mainTime->tm_min;
	    	
			// käydään olio läpi
			io.minute(mainTime->tm_min, mainTime->tm_hour);
			
		    // kirjoitetaan tietokanta (HUOM. NIMENOMAAN TÄSSÄ JÄRJESTYKSESSÄ)
			io.db_write(now);
			ow.db_write(now);
		}
		
		usleep(300000); //300ms
	}
	
	// Tuhotaan oliot ja niiden threadit
	io.stop();
	ow.stop();
	
	return EXIT_SUCCESS;
}

void signal_handler(int sig) { 
// signal handler function
	switch(sig){
		case SIGHUP:
			//(RE READ CONFIGURATION)
			break;		
		case SIGTERM:
			MAIN_LOOP_ALIVE = 0;
			break;
	}
}


