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

extern "C" {
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/un.h>
	#include <pthread.h>
	#include <sched.h>
	#include <fcntl.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <time.h>
	#include <math.h>
}

#include "boxfunctions.h"
#include "ipac.h"
#include "io.h"


IO::IO() : alive(true) {
	// avataan tietokanta
	if (openDatabase(NIALM_DB_FILE, &nialm_db, 30000) == false) {
		#ifdef DEBUG
		cout << "opendatabase nialm.db error" << endl;
		throw "error";
		#endif
	}
	// avataan tietokanta
	if (openDatabase(CONFIG_DB_FILE, &config_db, 30000) == false) {
		#ifdef DEBUG
		cout << "opendatabase config.db error" << endl;
		throw "error";
		#endif
	}
	
	// luetaan sensoritiedot
	if (read_io() == false) {
		#ifdef DEBUG
		cout << "read_io error" << endl;
		throw "error";
		#endif
	}
	
	// alustetaan sensoritiedot
	read_db();
	
	// alustetaan lukot
	pthread_mutex_init(&iolock, NULL);
	
	// attribute initialisize
	pthread_attr_init(&thread_io_attr);
	pthread_attr_init(&thread_server_attr);
	
	// do not inherit parent scheduler
	pthread_attr_setinheritsched(&thread_io_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setinheritsched(&thread_server_attr, PTHREAD_EXPLICIT_SCHED);
	
	// thread schedulers
	pthread_attr_setschedpolicy(&thread_io_attr, SCHED_RR);
	pthread_attr_setschedpolicy(&thread_server_attr, SCHED_RR);
	
	// Sched policy Sruct
	struct sched_param thread_io_param;
	struct sched_param thread_server_param;
	
	// setting the priority
	thread_io_param.sched_priority = 50;
	thread_server_param.sched_priority = 50;
	pthread_attr_setschedparam(&thread_io_attr, &thread_io_param);
	pthread_attr_setschedparam(&thread_server_attr, &thread_server_param);
	
	// setting system competition scope
	pthread_attr_setscope(&thread_io_attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setscope(&thread_server_attr, PTHREAD_SCOPE_SYSTEM);
	
	// create thread
	if (pthread_create(&thread_server, &thread_server_attr, 
	    &serverThread, (void *) this ) != 0) {
		#ifdef DEBUG
		cout << "thread_server create error" << endl;
		throw "error";
		#endif
		
	}
	// create thread
	if (pthread_create(&thread_io, &thread_io_attr, 
		&ioThread, (void *) this ) != 0) {
		#ifdef DEBUG
		cout << "thread_io create error" << endl;
		throw "error";
		#endif
	}
}

IO::~IO() {
}

void IO::stop() {
	// suljetaan tietokanta
	closeDatabase(&nialm_db);
	closeDatabase(&config_db);
	
	// destroy pthread attributes
	pthread_attr_destroy(&thread_io_attr);
	pthread_attr_destroy(&thread_server_attr);
	
	// kill running tasks
	alive = false;
	
	// odotetaan hetki
	sleep(2);
	
	// tuhotaan threadit
	pthread_cancel(thread_server);
	pthread_cancel(thread_io);
}

void* IO::serverThread(void* threadArg) {

	// palvellaan loopissa
	((IO *) threadArg) -> servering();
   
	pthread_exit(NULL);
}

void* IO::ioThread(void *threadArg) {
	
	// kerätään pulsseja loopissa
	((IO *) threadArg) -> pulsing();
	
	pthread_exit(NULL);
	 	
}


void IO::minute(int minute, int hour) {

	// =======================================================================
	pthread_mutex_lock(&iolock);
	// =======================================================================

	for (unsigned int id = 0; id < ioVect.size(); ++id) {
		
		ioVect[id].devicePulses_minute = ioVect[id].devicePulses_today 
									   - ioVect[id].devicePulses_minus_minute;
		
		ioVect[id].devicePulses_minus_minute = ioVect[id].devicePulses_today;
		
		//PÄIVÄ VAIHTUU
		if (hour==0 && minute==0) {
			
			ioVect[id].devicePulses_today = 0;		
			ioVect[id].devicePulses_minus_minute = 0;
					
		}
	}
	
	// =======================================================================
	pthread_mutex_unlock(&iolock);
	// =======================================================================
}

void IO::servering() {

    int s, s2 = 0;
    socklen_t t, len = 0; //Socket address length type
    
    FILE *sockin;
    FILE *sockout;
    struct sockaddr_un local, remote;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        //perror("socket error");
        pthread_exit(NULL);
    }
	
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(s, (struct sockaddr *)&local, len) == -1) {
        //perror("bind error");
        pthread_exit(NULL);
    }

    if (listen(s, 5) == -1) {
        //perror("listen error");
        pthread_exit(NULL);
    }
    
    char line[80]; //puskuri fgets funktiolle
    
	int type = 0;
	int deviceId = 0;
	
    while (alive == true) {
	
		#ifdef DEBUG
        printf("Waiting for a connection...\n");
		#endif
		
        t = sizeof(remote);
		
        if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1) {
			#ifdef DEBUG
            printf("accept error");
			#endif
            pthread_exit(NULL);
        }

        //
     	// We'll use stdio for reading the socket.
     	//
		sockin = fdopen(s2, "r");
		sockout = fdopen(s2, "w");

		#ifdef DEBUG
        printf("Socket connected.\n");
		#endif
        
        fgets(line, 80, sockin); //luetaan ensimmäinen rivi, jotta tiedetään mikä tieto tulossa
 		sscanf( line, "type=%d&deviceId=%d", &type, &deviceId ); //skannataan bufferi-rivi
 		
		if (type == 1) { //live_power READ, ASIAKAS haluaa tietoa serveriltä
			
 			fprintf(sockout, "%d,%d,%d", ioVect[deviceId].deviceValueCalc, 
									  ioVect[deviceId].devicePulses_today,
									  ioVect[deviceId].deviceMeterConst);
			fflush(sockout);
		}
		
 		else if (type == 2) { // ORIGINAL X
		
 			int i = 0;
			
	 		// =======================================================================
    		pthread_mutex_lock(&iolock);
			// =======================================================================
    		
    		for (int k = BUFFERLENGTH - 1; k >= 0; --k) {
	    		
				//lasketaan pointterin paikka, sama kuin modulo(buffercounter-k, BUFFERLENGTH) 2 potensseilla
    			i = (ioVect[deviceId].buffercounter - k)&(BUFFERLENGTH - 1); 
				
				// Tulostetaan vain eri suuri kuin nolla
    			if ( ioVect[deviceId].x[i].Value != 0 ) {
					
					fprintf(sockout, "%d,%d\n", ioVect[deviceId].x[i].Epoch, ioVect[deviceId].x[i].Value);
					fflush(sockout);
				}
			}
			
			// =======================================================================
    		pthread_mutex_unlock(&iolock);
			// =======================================================================
		}
		
		else if (type == 22) { // FILTERED Y
		
 			int i = 0;
			
	 		// =======================================================================
    		pthread_mutex_lock(&iolock);
			// =======================================================================
    		
    		for (int k = BUFFERLENGTH - 1; k >= 0; --k) {
	    		
				//lasketaan pointterin paikka, sama kuin modulo(buffercounter-k, BUFFERLENGTH) 2 potensseilla
    			i = (ioVect[deviceId].buffercounter - k)&(BUFFERLENGTH - 1); 
				
				// Tulostetaan vain eri suuri kuin nolla
    			if ( ioVect[deviceId].y[i].Value != 0 ) {
					
					fprintf(sockout, "%d,%d\n", ioVect[deviceId].y[i].Epoch, ioVect[deviceId].y[i].Value);
					fflush(sockout);
				}
			}
			
			// =======================================================================
    		pthread_mutex_unlock(&iolock);
			// =======================================================================
		}
		
		else if (type == 3) { //FFT, ASIAKAS haluaa tietoa serveriltä
		
 			int i = 0;
			
	 		// =======================================================================
    		pthread_mutex_lock(&iolock);
			// =======================================================================
    		
			// tämän pitää nimenomaan tulostaa myös käynnistyksen aikaiset nolla-alkiot
			// jotta FFT algoritmi saa 2 potenssin mukaisen taulukon
			
    		for (int k =  BUFFERLENGTH2 - 1; k >= 0; --k) {
	    		
				//lasketaan pointterin paikka, sama kuin modulo(buffercounter-k, BUFFER2LENGTH) 2 potensseilla
    			i = (ioVect[deviceId].buffercounter2 - k)&(BUFFERLENGTH2 - 1);
				
				fprintf(sockout, "%d\n", ioVect[deviceId].y_interp[i].Value);
				fflush(sockout);
			}
			
			// =======================================================================
    		pthread_mutex_unlock(&iolock);
			// =======================================================================
		}
		
		else if (type == 4) { // Nialm
		
			fprintf(sockout, "%d,%d", ioVect[deviceId].delta.Epoch, 
									  ioVect[deviceId].delta.Value);
			fflush(sockout);
		}
				
		fclose(sockin);	//muuten tulee muistivuoto
        fclose(sockout);//muuten tulee muistivuoto
		
        close(s2); //suljetaan socket       
   	}
}

void IO::pulsing() {

    //seconds from epoch
    time_t now;
	
	//Luetaan epoch
	time(&now);
	
    //sekoitetaan pseudorandomgeneraattori
	srand(now); 
    
    //timespec struct for nanosleep 
    struct timespec ts;
    
    //nanosleep values
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
	
	//IO tarkastus muuttuja	
    int tila = 0;

    //avataan gpio laite
    int fd = open("/dev/gpio",O_RDWR);      
	
	while (alive == true) {
	
		// Generoidaan CPU aikaa (muuten järjestelmä jäätyy)
		nanosleep(&ts, NULL);
		
		// luetaan uusin aika
		time(&now);
		
		for (unsigned int id = 0; id < ioVect.size(); ++id) { // käydään läpi kaikki sensorit
			
			//GET state of pulse sensor
			tila = ioctl(fd, GPIO_IOCTL_GET, &ioVect[id].devicePort);
			
			if ( tila == 0 && ioVect[id].devicePortState == 1) { // Ei mennä samalla
																 // pulssilla			
				//kerätään pulssi
				pulsecollect(id);
				
				//Suoritetaan laskenta
				if ( ioVect[id].i == ioVect[id].e  ) {
				
					pulsecalculate(id, now);
					live_write(id);
					interpolate(id);
					
					if (id == 0) { // vain pääanturille
						edge_detect(id);
					}
				}
			}
			
			ioVect[id].devicePortState = tila;	// asetetaan tila
		}
		
		if (alive == false) { 	// ohjelma sammuu
			close(fd); 		// Suljetaan GPIO laite
			return;			// Palataan pois
		}
	}
}

int IO::read_size() {
	
	sqlite3_stmt* stmt = NULL;	
	char sql_count2[] = "SELECT COUNT(deviceId) FROM IO_DATA";
	
	//HAETAAN sensorien lukumäärä
	int SIZE = 0;
	
	int rc = sqlite3_prepare_v2(config_db, sql_count2, strlen(sql_count2), &stmt, NULL);
	if( rc!=SQLITE_OK ){
			
		#ifdef DEBUG	
		fprintf(stderr, "SQL error in preparing IO_DATA row count\n");
		#endif
		
		return SIZE;
					
	} else {
		sqlite3_step(stmt);	
		SIZE = sqlite3_column_int(stmt,0);
	}
			
    sqlite3_finalize(stmt); 
	return SIZE;
}

bool IO::read_io() {

	sqlite3_stmt* stmt = NULL; 
    
	//HAETAAN PORTTI TIEDOT
	char sql_read[] = "SELECT deviceId, deviceActive, devicePort, deviceTypePulse, deviceTypeCalc, deviceLog, deviceMeterConst, deviceCalcTime, deviceDelta,deviceAlarm, deviceLow_limit, deviceHigh_limit FROM IO_DATA";
	    
	//valmistellaan käsky
	int rc = sqlite3_prepare_v2(config_db, sql_read, strlen(sql_read), &stmt, NULL);
	
	if( rc != SQLITE_OK ){
			
			#ifdef DEBUG	
			fprintf(stderr, "SQL error in preparing OW_DATA config table\n");
			#endif
			
			return false;
					
	} else {
		
		// muutetaan koko vastaamaan nykyistä tietokantaa
		ioVect.resize(read_size());
		
		unsigned int i = 0;
		
		while (sqlite3_step(stmt) == SQLITE_ROW) { //suoritetaan käsky
			
			// =======================================================================
			pthread_mutex_lock(&iolock);
			// =======================================================================
			
			//haetaan arvot
		    
	    	ioVect[i].deviceId = sqlite3_column_int(stmt,0);
	    	
	    	ioVect[i].deviceActive = sqlite3_column_int(stmt,1);
	
			ioVect[i].devicePort = sqlite3_column_int(stmt,2);											
			
			sprintf(ioVect[i].deviceTypePulse, "%s", sqlite3_column_text(stmt,3) );
			
			sprintf(ioVect[i].deviceTypeCalc, "%s", sqlite3_column_text(stmt,4) );
			
			
			sprintf(ioVect[i].deviceLogNamePulse, "%s_%d", ioVect[i].deviceTypePulse, ioVect[i].deviceId);
			
			sprintf(ioVect[i].deviceLogNameCalc, "%s_%d", ioVect[i].deviceTypeCalc, ioVect[i].deviceId);
						
			
			ioVect[i].deviceLog = sqlite3_column_int(stmt,5);
			
			ioVect[i].deviceMeterConst  = sqlite3_column_int(stmt,6);
			
			ioVect[i].deviceCalcTime = sqlite3_column_int(stmt,7);
			
			ioVect[i].deviceDelta = sqlite3_column_int(stmt,8);

			ioVect[i].deviceAlarm = sqlite3_column_int(stmt,9);
			
			ioVect[i].deviceLow_limit = sqlite3_column_double(stmt,10);
			
			ioVect[i].deviceHigh_limit = sqlite3_column_double(stmt,11);
			
			// =======================================================================
			pthread_mutex_unlock(&iolock);
			// =======================================================================
						
			++i;
			
		} //while ends
			
	} //else ends
	
	sqlite3_finalize(stmt);
	
    return true;
}

void IO::read_db() {
	
	// Avataan tietokanta
	sqlite3* db;
	
	// avataan tietokanta
	if (openDatabase(MAIN_DB_FILE, &db, 30000) == false) {
		#ifdef DEBUG
		cout << "openDatabase MAIN_DB error " << endl;
		throw "error";
		#endif
	}
	
	//char *zErrMsg;
	int rc = 0;
	
	//LUETAAN TÄMÄN PÄIVÄN PULSSIT ERI PORTEILLE
	
    for (unsigned int id = 0; id <ioVect.size(); ++id) {

		int pulses_today = 0;
		
		sqlite3_stmt* stmt = NULL;
		
		char sql_read[50] = "select "; //MUISTA tyhjä merkki lopussa
		strcat(sql_read, ioVect[id].deviceLogNamePulse); //esim. energy_0, water_1	
		strcat(sql_read," from d"); //MUISTA tyhjä merkki alussa
		
		get_date_string(sql_read, 0);
	
		//valmistellaan käsky
		rc = sqlite3_prepare_v2(db, sql_read, strlen(sql_read), &stmt, NULL);
		if( rc!=SQLITE_OK ){
			
			#ifdef DEBUG		
			fprintf(stderr, "SQL error in reading IO_DATA table database\n");
			#endif
					
		} else {
							
			//suoritetaan käskyä loopissa
			while(sqlite3_step(stmt) == SQLITE_ROW) {				
				pulses_today += sqlite3_column_int(stmt,0);
	
			}
	
			ioVect[id].devicePulses_today = pulses_today;
			ioVect[id].devicePulses_minus_minute = pulses_today;
		
		}
		
		sqlite3_finalize(stmt);	
	}
	
	// Suljetaan tietokanta
	closeDatabase(&db);
}

void IO::pulsecollect(int id) {
	// ==========================================================
	// timerV:n alkiot
	// ----------------------------------------------------------
	// Ajastimen arvo (s) table[0]=table[2]-table[1]	 	[0]
	// t1													[1]
	// t2													[2]
	// Pulssin jaksonaika (s)								[3]
	// Edellisen pulssin jaksonaika (s)		 				[4]
	// Edellinen ajastimen arvo (s)							[5]
	// ----------------------------------------------------------
	
	// Haetaan päälaskuri		
    gettimeofday(&ioVect[id].timer, NULL);
    
    ioVect[id].timerV[2].tv_sec  = ioVect[id].timer.tv_sec;
    ioVect[id].timerV[2].tv_usec = ioVect[id].timer.tv_usec;
	
    if (ioVect[id].i == -1) { //eka käynnistys
	    ioVect[id].timerV[1].tv_sec  = ioVect[id].timerV[2].tv_sec;
		ioVect[id].timerV[1].tv_usec = ioVect[id].timerV[2].tv_usec;
    }
	
    ioVect[id].timerV[0].tv_sec  = ioVect[id].timerV[2].tv_sec
								 - ioVect[id].timerV[1].tv_sec;
									 
	ioVect[id].timerV[0].tv_usec = ioVect[id].timerV[2].tv_usec
								 - ioVect[id].timerV[1].tv_usec; 
    
    ++ioVect[id].i;
    
    // =======================================================================
   	pthread_mutex_lock(&iolock);
	// =======================================================================
	
    ++ioVect[id].devicePulses_today;  // Lisätään pulssilaskuriin arvo
    
    // =======================================================================
   	pthread_mutex_unlock(&iolock);
	// =======================================================================
    
    // Yksittäisen pulssin jaksonaika T (s)
	ioVect[id].timerV[3].tv_sec  = ioVect[id].timerV[0].tv_sec
								 - ioVect[id].timerV[5].tv_sec;
									 
	ioVect[id].timerV[3].tv_usec = ioVect[id].timerV[0].tv_usec
							     - ioVect[id].timerV[5].tv_usec;
	
	//Tallennetaan arvo
   	ioVect[id].timerV[5].tv_sec  = ioVect[id].timerV[0].tv_sec;
	ioVect[id].timerV[5].tv_usec = ioVect[id].timerV[0].tv_usec; 
		
	if (ioVect[id].i == 1) { //TÄMÄ MÄÄRÄÄ laskentajaksojenmäärän
		
		double period = ioVect[id].timerV[3].tv_sec 
		              + ioVect[id].timerV[3].tv_usec/1000000.0;
		
    	ioVect[id].e = static_cast<int>(ioVect[id].deviceCalcTime/period); 
    	
    	if (ioVect[id].e == 0) { //pyöristys ylös ( pienet tehot ja kun Calctime==0)
	    	ioVect[id].e = 1;
    	}
	}
	
    // Tarkastelu nopeita muutoksia ioten, 
	// jos pulssi poikkeaa yli power_deltan % 
	
	double period = fabs(ioVect[id].timerV[3].tv_sec
				       - ioVect[id].timerV[4].tv_sec
				       + (ioVect[id].timerV[3].tv_usec
					    - ioVect[id].timerV[4].tv_usec)/1000000.0);
	
	if ( ( period*100/(ioVect[id].timerV[4].tv_sec
	       + ioVect[id].timerV[4].tv_usec/1000000.0) ) > ioVect[id].deviceDelta ) {
		
		if (ioVect[id].i == 1) {
			
			#ifdef DEBUG
			printf( "Delta 1 happened \n");
			#endif
			
			//nollataan tilanne  
			--ioVect[id].i;	//eli i=0
			
			//uusi startti laskurille tästä hetkestä
			ioVect[id].timerV[1].tv_sec  = ioVect[id].timerV[2].tv_sec;
			ioVect[id].timerV[1].tv_usec = ioVect[id].timerV[2].tv_usec;
			ioVect[id].timerV[5].tv_sec  = 0;
			ioVect[id].timerV[5].tv_usec = 0;
			
		}
		
		if (ioVect[id].i > 1) {
			
			#ifdef DEBUG
			printf( "Delta 2 happened \n");
			#endif
			
			//vähennetään viimeisin pulssi
			--ioVect[id].i; 
			ioVect[id].timerV[0].tv_sec  -= ioVect[id].timerV[3].tv_sec;
			ioVect[id].timerV[0].tv_usec -= ioVect[id].timerV[3].tv_usec;
			
			ioVect[id].e = ioVect[id].i;		
		}
	}
	
	//Tallennetaan pulssin jaksonaika
	ioVect[id].timerV[4].tv_sec  = ioVect[id].timerV[3].tv_sec;
	ioVect[id].timerV[4].tv_usec = ioVect[id].timerV[3].tv_usec;
    
}

void IO::pulsecalculate(int id, int epoch) {
	
	//haetaan päälaskuri
    gettimeofday(&ioVect[id].timer, NULL);
    	    			
	ioVect[id].timerV[1].tv_sec  = ioVect[id].timer.tv_sec;
	ioVect[id].timerV[1].tv_usec = ioVect[id].timer.tv_usec;
	
	// =======================================================================
   	pthread_mutex_lock(&iolock);
	// =======================================================================
    
    //Itse lasku
	double jakaja = (ioVect[id].timerV[0].tv_sec
				     + ioVect[id].timerV[0].tv_usec/1000000.0)*ioVect[id].deviceMeterConst;
	
	double tulos = (ioVect[id].i*3600/jakaja)*1000;
	
	// Triangular-PDF dithering
	double dither = (closed_interval_rand(-0.002*tulos, 0.002*tulos)
				   + closed_interval_rand(-0.002*tulos, 0.002*tulos))/2.0;
	
    ioVect[id].deviceValueCalc = static_cast<int>(tulos + dither);
    ioVect[id].deviceEpoch = epoch;
    
    // =======================================================================
   	pthread_mutex_unlock(&iolock);
	// =======================================================================
    
    //nollataan tarvittavat
    ioVect[id].e = 0;
	ioVect[id].i = 0;
	ioVect[id].timerV[5].tv_sec  = 0;
	ioVect[id].timerV[5].tv_usec = 0;
    
}

void IO::live_write(int id) {
	
	// =======================================================================
   	pthread_mutex_lock(&iolock);
	// =======================================================================
	
	//kasvatetaan osoittimen arvoa, TÄRKEÄ ETTÄ ON TÄSSÄ EKANA,
	//EIKÄ LOPUSSA, koska sotkee muuten indeksoinnin
	
	++ioVect[id].buffercounter;

	//siirretään taulukon alkuun
	if (ioVect[id].buffercounter == BUFFERLENGTH) {
		ioVect[id].buffercounter = 0;
	}
	
	//uusin arvo osoittimen määräämään kohtaan taulukossa
	ioVect[id].x[ioVect[id].buffercounter].Value = ioVect[id].deviceValueCalc;
	ioVect[id].x[ioVect[id].buffercounter].Epoch = ioVect[id].deviceEpoch;
	ioVect[id].x[ioVect[id].buffercounter].Check = false;
	
	// ----------------------------------------------
	// Haetaan circular-buffer arvot
	// ----------------------------------------------
	const int N = 11;    // Suotimen pituus
	int x[N] = {0};      // Suodatettava signaali
	int viive = 0;       // Viiven määrä tappeina
	
	for (int k = 0; k < N; ++k) {
		x[k] = ioVect[id].x[(ioVect[id].buffercounter - k - viive)&(BUFFERLENGTH - 1)].Value;
	}
	
	// -------------------------------------------------------------------
	// MEDIAANI SUODATUS
	// -------------------------------------------------------------------
	sort(&x[0], &x[N]);
	int y = x[(N-1)/2];
	
	// -------------------------------------------------------------------
	// Ulostulo
	// -------------------------------------------------------------------
	ioVect[id].y[ioVect[id].buffercounter].Value = y;
	ioVect[id].y[ioVect[id].buffercounter].Epoch = ioVect[id].deviceEpoch;
	
	if (y > 0) { // Käynnistysarvoja ei merkitä leimatuiksi
		ioVect[id].y[ioVect[id].buffercounter].Check = false;
	}
	
	
	// -------------------------------------------------------------------
	// Adaptiivinen suodatus LMS
	// -------------------------------------------------------------------
	// Suodatetaan viivästettyä versiota (x)
	// Referenssisignaalina on viivästämätön kohinainen signaali (ref)
	/*
	const double u = 0.00000001;       // suppenemisparametri
	static double w[N] = {0.0};   // suotimen kertoimet, alustus vain ekalla kerralla
	double y = 0;				  // suotimen ulostulo
	double e = 0;				  // erotussignaali
	double ref = ioVect[id].x[(ioVect[id].buffercounter)&(BUFFERLENGTH - 1)].Value; // referenssi
	
	cout << "ref:" << ref << endl;
	
	// Lasketaan konvoluutio painojen kanssa
	for (int i = 0; i < N; ++i) {
		y += w[i]*x[i];
	}
	
	cout << "y: " << y << endl;
	
	// Lasketaan erosignaali
	e = ref - y;
	
	cout << "e: " << e << endl;
	
	// Päivitetään painoja
	for (int i = 0; i < N; ++i) {
		w[i] = w[i] + 2*u*e*x[i];
		cout << "w[" << i << "]: " << w[i] << endl;
	}
	
	// suotimen_ulostulo
	ioVect[id].y[ioVect[id].buffercounter].Value = static_cast<int>(y);
	ioVect[id].y[ioVect[id].buffercounter].Epoch = ioVect[id].deviceEpoch;
	
	if (y > 0) { // Käynnistysarvoja ei merkitä leimatuiksi
		ioVect[id].y[ioVect[id].buffercounter].Check = false;
	}
	*/
	// =======================================================================
    pthread_mutex_unlock(&iolock);
	// =======================================================================
	
}

void IO::interpolate(int id) {
	
	// =======================================================================
   	pthread_mutex_lock(&iolock);
	// =======================================================================
	
	int x0 = ioVect[id].x_interp[ioVect[id].buffercounter2].Epoch;
	int y0 = ioVect[id].x_interp[ioVect[id].buffercounter2].Value;
	
	int x1 = ioVect[id].deviceEpoch;
	int y1 = ioVect[id].deviceValueCalc;
	
	int points = 0;
	
	if (x0 != 0) { // huomioidaan käynnistys
		points = x1 - x0 + 1;
	} else {
		points = 2;
		x0 = x1 - 1;
		y0 = y1;
	}
	
	for (int i = 1; i < points; ++i) {
		
		++ioVect[id].buffercounter2;
		
		//siirretään taulukon alkuun
		if (ioVect[id].buffercounter2 == BUFFERLENGTH2) {
			ioVect[id].buffercounter2 = 0;
		}
		
		int x = x0 + i;								// uusi aikaleima
		int y = y0 + (x - x0)*(y1 - y0)/(x1 - x0);  // uusi interpoloitu arvo
		
		//uusin arvo osoittimen määräämään kohtaan taulukossa
		ioVect[id].x_interp[ioVect[id].buffercounter2].Value = y;
		ioVect[id].x_interp[ioVect[id].buffercounter2].Epoch = x;
		ioVect[id].x_interp[ioVect[id].buffercounter2].Check = false;
		
		// ----------------------------------------------
		// DC-block suodatus y[n] = x[n] - x[n-1]
		// ----------------------------------------------
		
		//int index = (ioVect[id].buffercounter2 - 1)&(BUFFERLENGTH2 - 1); //indeksi arvolle x[n-1]
		
		//uusin arvo osoittimen määräämään kohtaan taulukossa
		ioVect[id].y_interp[ioVect[id].buffercounter2].Value = y;// - ioVect[id].x_interp[index].Value;
		ioVect[id].y_interp[ioVect[id].buffercounter2].Epoch = x;
		ioVect[id].y_interp[ioVect[id].buffercounter2].Check = false;
	}
	
	// =======================================================================
    pthread_mutex_unlock(&iolock);
	// =======================================================================
}


void IO::edge_detect(int id) {
	// =======================================================================
	// eli skannataan WINDOWS_SIZE viimeistä arvoa
	// esim. käydään läpi n-64, n-63, n-62 ... n-1 eli p(n-t) uusinta tappia,
	// missä n = uusin arvo (viimeisin taulukossa) ja t = askelia taakse päin
	
	const int WINDOW_SIZE = 12;
	
	// pienin muutos, joka tallennetaan
	int dPtres = 0;
	
	// muutoksen jälkeinen staabilius
	int dPtol = 0;
	
	// =======================================================================
   	pthread_mutex_lock(&iolock);
	// =======================================================================
	
	const int space = 10; // liikkumavara
	const int marker = 2;
	
	//pitää sisällään circular bufferin indeksit
	int i[space];
	
	for (int t = WINDOW_SIZE; t >= space; --t) {
	
		for (int z = 0; z < space; ++z) {
		
			// lasketaan circular buffer indeksit hetkelle n-t, n-t+1
			// eli p(n-t-z), missä z = 0,1
			
			//haetaan pointterin paikka, buffercounter = n
			//AND <-> mod(ioVect[id].buffercounter-k, BUFFERLENGTH) 2 potensseilla
			
			i[z] = (ioVect[id].buffercounter - t + z)&(BUFFERLENGTH - 1);
		}
		
		dPtres = abs(25 + ((ioVect[id].y[i[1]].Value + ioVect[id].y[i[0]].Value)/200) );
		dPtol  = abs(10 + ((ioVect[id].y[i[1]].Value + ioVect[id].y[i[0]].Value)/200) );

		int start_difference = abs( ioVect[id].y[i[3]].Value - ioVect[id].y[i[1]].Value);
		int start_stability  = abs( ioVect[id].y[i[1]].Value - ioVect[id].y[i[0]].Value);
		
		// | f(x+1) - f(x) | > threshold
		if ( (start_difference > dPtres) && (start_stability < dPtol) ) {
		
			for (int k = 0; k < space - 4; ++k) {
			
				dPtol  = abs(10 + ((ioVect[id].y[i[3+k]].Value + ioVect[id].y[i[2+k]].Value)/200) );
			
				int end_difference = abs( ioVect[id].y[i[3+k]].Value - ioVect[id].y[i[1]].Value);
				int end_stability = abs( ioVect[id].y[i[3+k]].Value - ioVect[id].y[i[2+k]].Value);
				
				// | f(x+3+k) - f(x+2+k) | < tolerance
				if ( (end_stability < dPtol) && (end_difference > dPtres) ) {
					
					//tarkistetaan onko jo löydetty aiemmin
					bool OK = true;
					
					for (int j = 0; j < marker; ++j) {
						if ( ioVect[id].y[i[j]].Check == true ) {
							OK = false;	//löydetty jo aiemmin
						}
					}
					
					if (OK == true) {
						// merkitään leimatuksi
						for (int s = 0; s < marker; ++s) {
							ioVect[id].y[i[s]].Check = true;
						}
						
						//tallennetaan tietokantaan	(epoch, power, delta)		
						nialm_db_write(ioVect[id].y[i[0]].Epoch,
									   ioVect[id].y[i[0]].Value,
									   ioVect[id].y[i[3+k]].Value - ioVect[id].y[i[1]].Value);

						//tallennetaan muuttujaan delta
						ioVect[id].delta.Epoch = ioVect[id].y[i[0]].Epoch;
						ioVect[id].delta.Value = ioVect[id].y[i[3+k]].Value - ioVect[id].y[i[1]].Value;
					}
				}	
			} // tolerance			
		} // threshold
	} // window
	
	// =======================================================================
   	pthread_mutex_unlock(&iolock);
	// =======================================================================
	
}


void IO::nialm_db_write(int epoch, int power, int dP) {
	
	char *zErrMsg;

    // LUODAAN TÄMÄNPÄIVÄN TAULU JOKATAPAUKSESSA
    char sql_create[50]="CREATE TABLE d";
	get_date_string(sql_create, 0);
	strcat(sql_create, " (epoch INTEGER, power INTEGER, dP INTEGER)");
	
	// SUORITETAAN TAULUN LUONTI
	int rc = sqlite3_exec(nialm_db, sql_create, NULL, NULL, &zErrMsg);
	
	if( rc != SQLITE_OK ){
		
		#ifdef DEBUG
		fprintf(stderr, "SQL nialm error 1: %s\n", zErrMsg);
		#endif
		
		sqlite3_free(zErrMsg);
	}
	
	// =======================================================================
	// SUORITETAAN ARVOJEN SIJOITUS
	
	char insert_string[30] = "INSERT INTO d";
	get_date_string(insert_string, 0);
	
	char insert_epoch[100];	
	sprintf(insert_epoch, "%s (epoch, power, dP) VALUES ('%d', '%d', '%d') ",
			insert_string, epoch, power, dP);
	
	rc = sqlite3_exec(nialm_db, insert_epoch, NULL, NULL, &zErrMsg);
	
	if( rc != SQLITE_OK ){
		
		#ifdef DEBUG
		fprintf(stderr, "SQL nialm error 2: %s\n", zErrMsg);
		#endif
		
		sqlite3_free(zErrMsg);
	}	 	
}

void IO::db_write(int epoch) {

	// Avataan tietokanta
	sqlite3* db;
	
	// avataan tietokanta
	if (openDatabase(MAIN_DB_FILE, &db, 30000) == false) {
		#ifdef DEBUG
		cout << "openDatabase MAIN_DB error " << endl;
		throw "error";
		#endif
	}

	char *zErrMsg;
	
	// BEGIN
	sqlite3_exec(db, "BEGIN;", NULL, NULL, &zErrMsg);
	{
		// KOITETAAN LUODA TÄMÄNPÄIVÄN TAULU JOKATAPAUKSESSA
		char sql_create[50]="CREATE TABLE d";
		get_date_string(sql_create, 1);
		strcat(sql_create, " (epoch INTEGER)");
		
		// SUORITETAAN TAULUN LUONTI
		int rc = sqlite3_exec(db, sql_create, NULL, NULL, &zErrMsg);
		
		if( rc != SQLITE_OK ){
			#ifdef DEBUG
			fprintf(stderr, "SQL error 1: %s\n", zErrMsg);
			#endif
			sqlite3_free(zErrMsg);
		}
		
		// =======================================================================
		// LUODAAN SARAKKEET
		// =======================================================================
		
		for (unsigned int i = 0; i < ioVect.size(); ++i) {
			
			char sql_alter1[100] = "ALTER TABLE d";
			get_date_string(sql_alter1, 1);
			strcat(sql_alter1, " ADD ");
			strcat(sql_alter1, ioVect[i].deviceLogNamePulse);
			strcat(sql_alter1, " INTEGER");
			
			//SUORITETAAN SARAKKEEN LUONTI
			rc = sqlite3_exec(db, sql_alter1, NULL, NULL, &zErrMsg);
		
			if( rc!=SQLITE_OK ){
				
				#ifdef DEBUG
				fprintf(stderr, "SQL error 3: %s\n", zErrMsg);
				#endif
				
				sqlite3_free(zErrMsg);
			}
			
			char sql_alter2[100] = "ALTER TABLE d";
			get_date_string(sql_alter2, 1);
			strcat(sql_alter2, " ADD ");
			strcat(sql_alter2, ioVect[i].deviceLogNameCalc);
			strcat(sql_alter2, " INTEGER");
			
			//SUORITETAAN SARAKKEEN LUONTI
			rc = sqlite3_exec(db, sql_alter2, NULL, NULL, &zErrMsg);
		
			if( rc!=SQLITE_OK ){
				
				#ifdef DEBUG
				fprintf(stderr, "SQL error 4: %s\n", zErrMsg);
				#endif
				
				sqlite3_free(zErrMsg);
			}
		}

		
		// =======================================================================
		// SUORITETAAN EPOCH ARVON LUONTI
		// =======================================================================
		
		char insert_string[30] = "INSERT INTO d";
		get_date_string(insert_string, 1);
		
		char insert_epoch[100];	
		sprintf(insert_epoch, "%s (epoch) VALUES ('%d') ", insert_string, epoch);
		
		rc = sqlite3_exec(db, insert_epoch, NULL, NULL, &zErrMsg);

		if( rc!=SQLITE_OK ){
			
			#ifdef DEBUG
			fprintf(stderr, "SQL error 6: %s\n", zErrMsg);
			#endif
			
			sqlite3_free(zErrMsg);
		}

		// =======================================================================
		// KIRJOITETAAN MUIDEN SARAKKEIDEN ARVOT
		// =======================================================================
		
		sqlite3_stmt* stmt = NULL;
		
		char update_string[30] = "UPDATE d";
		get_date_string(update_string, 1);
		
		for (unsigned int i = 0; i < ioVect.size(); ++i) {
		
			// =======================================================================
			pthread_mutex_lock(&iolock);
			// =======================================================================
			
			char sql_update1[150];

			sprintf(sql_update1, "%s SET %s = '%d' WHERE epoch LIKE '%d'",
					update_string, ioVect[i].deviceLogNamePulse, 
					ioVect[i].devicePulses_minute, epoch);
			
			//laite on aktiivinen JA halutaan tallentaa logi
			if ( (ioVect[i].deviceActive == 1) && (ioVect[i].deviceLog == 1) ) {
			
				// =======================================================================
				pthread_mutex_unlock(&iolock);
				// =======================================================================
				
				//valmistellaan käsky
				rc = sqlite3_prepare_v2(db, sql_update1, strlen(sql_update1), &stmt, NULL);
				if( rc!=SQLITE_OK ) {
						
					#ifdef DEBUG
					fprintf(stderr, "SQL error 7\n");	
					#endif
					
				} else { //ajetaan käsky
					rc = sqlite3_step(stmt);	
				}
				
				sqlite3_finalize(stmt);
			
			} else { //ei tehdä muuta kuin avataan lukko
				// =======================================================================
				pthread_mutex_unlock(&iolock);
				// =======================================================================
			}
			
			// =======================================================================
			pthread_mutex_lock(&iolock);
			// =======================================================================
			
			char sql_update2[150];
			
			sprintf(sql_update2, "%s SET %s = '%d' WHERE epoch LIKE '%d'", 
					update_string, ioVect[i].deviceLogNameCalc, 
					ioVect[i].deviceValueCalc, epoch);
			
			// laite on aktiivinen JA halutaan tallentaa logi
			if ( (ioVect[i].deviceActive == 1) && (ioVect[i].deviceLog == 1) ) {
			
				// =======================================================================
				pthread_mutex_unlock(&iolock);
				// =======================================================================
				
				//valmistellaan käsky
				rc = sqlite3_prepare_v2(db, sql_update2, strlen(sql_update2), &stmt, NULL);
				
				if( rc!=SQLITE_OK ) {		
					
					#ifdef DEBUG
					fprintf(stderr, "SQL error 8\n");	
					#endif
					
				} else { //ajetaan käsky
					rc = sqlite3_step(stmt);
				}
				
				sqlite3_finalize(stmt);
			
			} else { //ei tehdä muuta kuin avataan lukko
				// =======================================================================
				pthread_mutex_unlock(&iolock);
				// =======================================================================
			}
		}
	}
	// BEGIN
	sqlite3_exec(db, "END;", NULL, NULL, &zErrMsg);
	
	closeDatabase(&db);
}


