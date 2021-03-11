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

#include <vector>
#include <iostream>

extern "C" {
	#include <stdio.h>
	#include <pthread.h>
	#include <sched.h>
	#include <string.h>
	
	// ONEWIRE C
	#include <stdlib.h>
	#include <ctype.h>
	#include <unistd.h>
	#include <getopt.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/time.h>
	#include <time.h>
	#include <fcntl.h>
	#include <strings.h>
	#include <stdint.h>
	
	// DALLAS SEMICONDUCTOR
	#include "userial/ownet.h"
	#include "userial/owproto.h"
	#include "userial/ad26.h"
}

#include "boxfunctions.h"
#include "ow.h"

// Sarjaportin lukitus
#ifdef LOCKDEV
#include <lockdev.h>
#endif


OW::OW() : alive(true) {
	
	// avataan tietokanta
	if (openDatabase(CONFIG_DB_FILE, &config_db, 30000) == false) {
		#ifdef DEBUG
		cout << "openDatabase CONFIG_DB error " << endl;
		throw "error";
		#endif
	}
	
	// luetaan sensoritiedot
	if (read_ow() == false) {
		#ifdef DEBUG
		cout << "read_ow error" << endl;
		throw "error";
		#endif
	}
	
	// alustetaan lukot
	pthread_mutex_init(&owlock, NULL);
	
	// attribute initialisize
	pthread_attr_init(&thread_ow_attr);
	
	// do not inherit parent scheduler
	pthread_attr_setinheritsched(&thread_ow_attr, PTHREAD_EXPLICIT_SCHED);
	
	// thread schedulers
	pthread_attr_setschedpolicy(&thread_ow_attr, SCHED_OTHER);
	
	// Sched policy Sruct
	struct sched_param thread_ow_param;
	
	// setting the priority	
	thread_ow_param.sched_priority = 0;
	pthread_attr_setschedparam(&thread_ow_attr, &thread_ow_param);
	
	// setting system competition scope
	pthread_attr_setscope(&thread_ow_attr, PTHREAD_SCOPE_SYSTEM);
	
	// create thread
	if (pthread_create(&thread_ow, &thread_ow_attr, 
	    &owThread, (void *) this) != 0) {
		#ifdef DEBUG
		cout << "pthread thread_ow create error" << endl;
		throw "error";
		#endif
	}
	
}

OW::~OW() {
}

void OW::stop() {
	// suljetaan tietokanta
	closeDatabase(&config_db);
	
	// destroy pthread attributes
	pthread_attr_destroy(&thread_ow_attr);
	
	// kill running tasks
	alive = false;
	
	// odotetaan hetki
	sleep(2);
	
	// tuhotaan threadit
	pthread_cancel(thread_ow);
}

void* OW::owThread(void *threadArg) {

	OW* ow;
	ow = (OW*) threadArg;
	
	while (ow->alive == true) {
	
		// Luetaan arvot
		ow->runOnewire();
		
		// Talletetaan tietokantaan
		ow->live_db_write();
	}
	pthread_exit(NULL);
}

int OW::read_size() {
	
	sqlite3_stmt* stmt = NULL;
	char sql_count1[] = "SELECT COUNT(deviceId) FROM OW_DATA";
	
	//HAETAAN sensorien lukumäärä
	int SIZE = 0;
	int rc = sqlite3_prepare_v2(config_db, sql_count1, strlen(sql_count1), &stmt, NULL);
	
	if( rc != SQLITE_OK ){
					
		#ifdef DEBUG
		cerr << "SQL error in preparing OW_DATA row count" << endl;
		#endif
		
		return SIZE;
					
	} else {
		sqlite3_step(stmt);	
		SIZE = sqlite3_column_int(stmt,0);	
	}
	
	sqlite3_finalize(stmt);
	return SIZE;	
}

bool OW::read_ow() {

	sqlite3_stmt* stmt = NULL;	
	
	//HAETAAN sensorien arvot
	char sql_read[] = "SELECT deviceId, deviceActive, deviceHex, deviceType, deviceLog, deviceAlarm, deviceLow_limit, deviceHigh_limit, deviceCalibration FROM OW_DATA";	
	
	//valmistellaan käsky
	int rc = sqlite3_prepare_v2(config_db, sql_read, strlen(sql_read), &stmt, NULL);
	
	if( rc!=SQLITE_OK ){
		
		#ifdef DEBUG
		cerr << "SQL error in preparing reading OW_DATA config table" << endl;
		#endif
		
		return false;
					
	} else {
		
		// muutetaan koko vastaamaan nykyistä
		owVect.resize(read_size());
		
		unsigned int i = 0;
		
   		while (sqlite3_step(stmt) == SQLITE_ROW) { //suoritetaan käsky
		 
			// =======================================================================
			pthread_mutex_lock(&owlock);
			// =======================================================================
						
			owVect[i].deviceId  = sqlite3_column_int(stmt,0);
		
			owVect[i].deviceActive = sqlite3_column_int(stmt,1);
	
			sprintf(owVect[i].deviceHex, "%s", sqlite3_column_text(stmt,2));
		
			sprintf(owVect[i].deviceType, "%s", sqlite3_column_text(stmt,3));
			
			
			sprintf(owVect[i].deviceLogNameCalc, "ow_%d", owVect[i].deviceId);
		
			owVect[i].deviceLog = sqlite3_column_int(stmt,4);
			
			owVect[i].deviceAlarm  = sqlite3_column_int(stmt,5);
		
			owVect[i].deviceLow_limit = sqlite3_column_double(stmt,6);
		
			owVect[i].deviceHigh_limit = sqlite3_column_double(stmt,7);
		
			owVect[i].deviceCalibration = sqlite3_column_double(stmt,8);
			
			// =======================================================================
			pthread_mutex_unlock(&owlock);
			// =======================================================================
			
			++i;
			
		} //while ends
									
	} //else ends
	
	sqlite3_finalize(stmt);
	
	return true;
	
}

void OW::db_write(int epoch) {

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
		
		char *zErrMsg;
		
		// =======================================================================
		// LUODAAN SARAKKEET
		// =======================================================================
		
		for (unsigned int i = 0; i < owVect.size(); ++i) {
			
			char sql_alter1[150]="ALTER TABLE d";
			get_date_string(sql_alter1, 1);
			strcat(sql_alter1, " ADD ");
			strcat(sql_alter1, owVect[i].deviceLogNameCalc);
			strcat(sql_alter1, " DOUBLE");
			
			//SUORITETAAN SARAKKEEN LUONTI
			int rc = sqlite3_exec(db, sql_alter1, NULL, NULL, &zErrMsg);
			
			if( rc!=SQLITE_OK ){
				
				#ifdef DEBUG
				fprintf(stderr, "SQL error 5: %s\n", zErrMsg);
				#endif
				
				sqlite3_free(zErrMsg);
			}
		}

		// =======================================================================
		// KIRJOITETAAN MUIDEN SARAKKEIDEN ARVOT
		// =======================================================================
		
		sqlite3_stmt* stmt = NULL;
		
		char update_string[30] = "UPDATE d";
		get_date_string(update_string, 1);
		
		for (unsigned int i = 0; i < owVect.size(); ++i) {
		
			// =======================================================================
			pthread_mutex_lock(&owlock);	
			// =======================================================================
			
			char sql_update1[150];
			
			sprintf(sql_update1, "%s SET %s = '%0.2f' WHERE epoch LIKE '%d'", 
					update_string, owVect[i].deviceLogNameCalc, 
					owVect[i].deviceValueCalc, epoch);
					
			//laite on aktiivinen JA halutaan tallentaa logi JA saatu arvo on kunnollinen
			if ( (owVect[i].deviceActive == 1) && (owVect[i].deviceLog == 1) &&
				 (owVect[i].deviceValid == 1) ) {	

				// =======================================================================
				pthread_mutex_unlock(&owlock);
				// =======================================================================
				
				//valmistellaan käsky
				int rc = sqlite3_prepare_v2(db, sql_update1, strlen(sql_update1), &stmt, NULL);
				
				if( rc!=SQLITE_OK ){		
					
					#ifdef DEBUG
					fprintf(stderr, "SQL error 9\n");
					#endif
						
				} else { //ajetaan käsky
					rc = sqlite3_step(stmt);
				}
				
				sqlite3_finalize(stmt);
				
			} else { //ei tehdä muuta kuin avataan lukko
				// =======================================================================
				pthread_mutex_unlock(&owlock);
				// =======================================================================
			}
		}
	}
	// END
	sqlite3_exec(db, "END;", NULL, NULL, &zErrMsg);
	
	// Suljetaan tietokanta
	closeDatabase(&db);
}

void OW::live_db_write() {

	sqlite3* live_db;

	// Avataan tietokanta
	if (openDatabase(LIVE_DB_FILE, &live_db, 30000) == false) {
		#ifdef DEBUG
		cout << "openDatabase LIVE_DB error " << endl;
		throw "error";
		#endif
	}
	
	char *zErrMsg;
	
	// BEGIN
	int rc = sqlite3_exec(live_db, "BEGIN;", NULL, NULL, &zErrMsg);
	{
		// --------------------------------------------
		// Tiputetaan taulukko
		
		char sql_drop_table[] = "DROP TABLE ow_live";
		rc = sqlite3_exec(live_db, sql_drop_table, NULL, NULL, &zErrMsg);
		
		if( rc != SQLITE_OK ){
			#ifdef DEBUG
			fprintf(stderr, "SQL error OW::live_db() %s\n", zErrMsg);
			#endif
			sqlite3_free(zErrMsg);
		}
		
		// --------------------------------------------
		// Luodaan taulukko
		
		char sql_create_table[] = "CREATE TABLE ow_live (deviceId INTEGER, deviceValueCalc DOUBLE)";
		rc = sqlite3_exec(live_db, sql_create_table, NULL, NULL, &zErrMsg);
		
		if( rc != SQLITE_OK ){
			#ifdef DEBUG
			fprintf(stderr, "SQL error OW::live_db() %s\n", zErrMsg);
			#endif
			sqlite3_free(zErrMsg);
		}
		
		// --------------------------------------------
		// Luodaan rivit
		
		// =======================================================================
		pthread_mutex_lock(&owlock);	
		// =======================================================================
		
		for (unsigned int i = 0; i < owVect.size(); ++i) {
			
			if (owVect[i].deviceActive == 1) { // merkitään vain aktiivisten
												  // sensorien tiedot
			
				char sql_insert_row[200];
				sprintf(sql_insert_row, "INSERT INTO ow_live VALUES (%d, %0.2f)", 
						owVect[i].deviceId, owVect[i].deviceValueCalc);
				
				rc = sqlite3_exec(live_db, sql_insert_row, NULL, NULL, &zErrMsg);
				
				if( rc != SQLITE_OK ){
					#ifdef DEBUG
					fprintf(stderr, "SQL error OW::live_db() %s\n", zErrMsg);
					#endif
					sqlite3_free(zErrMsg);
				}
			}
		}
		
		// =======================================================================
		pthread_mutex_unlock(&owlock);	
		// =======================================================================
	}
	
	// END
	rc = sqlite3_exec(live_db, "END;", NULL, NULL, &zErrMsg);
	
	// Suljetaan tietokanta
	closeDatabase(&live_db);
}


char serial_port[40];               // Path to the serial port
char serial_dev[40];				// Device name without /dev/
char conf_file[1024];				// Configuration File

int	read_time;						// Pause during read
int num_cs = 0;         			// Number of sensors on coupler

struct _coupler *coupler_top = NULL;// Linked list of couplers

unsigned char Last2409[9];          // Last selected coupler

const char dtlib[] = "DS9097";		// Library Used

int	global_msec = 10;				// For ReadCOM delay
int	global_msec_max = 15;


// -----------------------------------------------------------------------
//  Return the family name passed to it
// -----------------------------------------------------------------------

char * OW::device_name( unsigned int family ) {
  switch( family ) {
    case 0x01:
      return "DS2401/DS1990A Serial Number iButton";
      
    case 0x02:
      return "DS1425/DS1991 MultiKey iButton";
      
    case 0x04:
      return "DS2402/DS1994 4K NVRAM memory, clock, timer";

    case 0x05:
      return "DS2405 Addressable Switch";

    case 0x06:
      return "DS1993 4K NVRAM Memory";
      
    case 0x08:
      return "DS1992 1K NVRAM Memory";
  
    case 0x09:
      return "DS2502/DS1982 1Kbit Add only memory";
  
    case 0x0A:
      return "DS1995 16K NVRAM Memory";

    case 0x0B:
      return "DS2505/DS1985 16K EPROM Memory"; 
      
    case 0x0C:
      return "DS1996/x2/x4 64K to 256K NVRAM Memory"; 
      
    case 0x0F:
      return "DS2506/DS1986 64K EEPROM Memory";
      
    case 0x10:
      return "DS1820/DS18S20/DS1920 Temperature Sensor";
    
    case 0x12:
      return "DS2406/2407 Dual Addressable Switch + 1Kbit memory";
    
    case 0x14:
      return "DS2430A/DS1971 256bit EEPROM iButton";
    
    case 0x18:
      return "DS1963S SHA iButton";
      
    case 0x1A:
      return "DS1963L 4kBit MONETARY iButton";
      
    case 0x1C:
      return "DS2422 1Kbit RAM + Counter";
      
    case 0x1D:
      return "DS2423 4Kbit RAM + Counter";
      
    case 0x1F:
      return "DS2409 MicroLAN Coupler";
      
    case 0x20:
      return "DS2450 Quad A/D Converter";
      
    case 0x21:
      return "DS1921/H/Z Thermochron iButton";
      
    case 0x22:
      return "DS1822 Econo-Temperature Sensor";

    case 0x23:
      return "DS2433/DS1973 4K EEPROM Memory";
    
    case 0x24:
      return "DS1425/DS1904 Real Time Clock";
      
    case 0x26:
      return "DS2438 Temperature, A/D Battery Monitor";
      
    case 0x27:
      return "DS2417 Real Time Clock with Interrupt";

    case 0x28:
      return "DS18B20 Temperature Sensor";

    case 0x29:
      return "DS2408 8-Channel Addressable Switch";

    case 0x2C:
      return "DS2890 Single Channel Digital Potentiometer";
      
    case 0x30:
      return "DS2760 Temperature, Current, A/D";
      
    case 0x33:
      return "DS2432/DS1961S 1K EEPROM with SHA-1 Engine";

    case 0x3A:
      return "DS2413 Dual Channel Addressable Switch";

    case 0x41:
      return "DS1923 Hygrochron Temperature/Humidity Logger with 8kB Data Log Memory";

    case 0x42:
      return "DS28EA00 Temperature Sensor with Sequence Detect and PIO";

    case 0x82:
      return "DS1425 Multi iButton";
      
    case 0x84:
      return "DS1427 TIME iButton";
      
    case 0x89:
      return "DS2502/1982 1024bit UniqueWare Add Only Memory";
    
    case 0x8B:
      return "DS2505/1985 16Kbit UniqueWare Add Only Memory";
      
    case 0x8F:
      return "DS2506/1986 64Kbit UniqueWare Add Only Memory";

    case 0x91:
      return "DS1981 512-bit EEPROM Memory UniqueWare Only";
      
    case 0x96:
      return "DS1955/DS1957B Java Cryptographic iButton";
  
    default:
      return "Unknown Family Code";
  }
}

// -----------------------------------------------------------------------
//  Free up all memory used by the coupler list
// -----------------------------------------------------------------------
 
void OW::free_coupler( int free_only ) {
	
	unsigned char   a[3];
	struct _coupler *current;
	
	current = coupler_top;
	
	while( current != 0) {
		// Turn off the Coupler
		if ( !free_only )
			SetSwitch1F(0, current->SN, ALL_LINES_OFF, 0, a, TRUE);
		
		// Free up the serial number list arrays
		if( current->num_main > 0 )
			free( current->main );
		
		if( current->num_aux > 0 )
			free( current->aux );
		
		// Point to the next in the list
		coupler_top = current->next;
		
		// Free up the current entry
		free( current );
		
		current = coupler_top;
	} // Coupler free loop
	
	// Make sure its null
	coupler_top = NULL;
}


// -----------------------------------------------------------------------
//   Convert degrees C to degrees F
// -----------------------------------------------------------------------
float OW::c2f( float temp ) {
	return 32 + ((temp*9)/5);
}


// -----------------------------------------------------------------------
//   Compare two serial numbers and return 1 of they match
//
//   The second one has an additional byte indicating the main (0) or aux (1)
//   branch.
// -----------------------------------------------------------------------
int OW::cmpSN( unsigned char *sn1, unsigned char *sn2, int branch ) {

	for( int i = 0; i < 8; i++ ) {
		if( sn1[i] != sn2[i] ) {
			return 0;
		}
	}
  
	if( branch != sn2[8] ) {
		return 0;
	}

	// Everything Matches
	return 1;
}  


// -----------------------------------------------------------------------
// Show the verbose contents of the scratchpad
// -----------------------------------------------------------------------
void OW::show_scratchpad( unsigned char *scratchpad, int sensor_family ) {

	switch( sensor_family ) {

	  case DS1820_FAMILY:
		printf("  Temperature   : 0x%02X\n", scratchpad[1] );
		printf("  Sign          : 0x%02X\n", scratchpad[2] );
		printf("  TH            : 0x%02X\n", scratchpad[3] );
		printf("  TL            : 0x%02X\n", scratchpad[4] );
		printf("  Remain        : 0x%02X\n", scratchpad[7] );
		printf("  Count Per C   : 0x%02X\n", scratchpad[8] );
		printf("  CRC           : 0x%02X\n", scratchpad[9] );
		break;

	  case DS18B20_FAMILY:
	  case DS1822_FAMILY:
	  case DS28EA00_FAMILY:
		printf( "  Temp. LSB     : 0x%02X\n", scratchpad[1] );
		printf( "  Temp. MSB     : 0x%02X\n", scratchpad[2] );
		printf( "  TH            : 0x%02X\n", scratchpad[3] );
		printf( "  TL            : 0x%02X\n", scratchpad[4] );
		printf( "  Config Reg.   : 0x%02X\n", scratchpad[5] );
		printf( "  CRC           : 0x%02X\n", scratchpad[9] );
		break;

	  case DS2422_FAMILY:
	  case DS2423_FAMILY:
	  
		break;  
	} // sensor_family switch

  // Dump the complete contents of the scratchpad
  for( int i = 0; i < 10; i++ ) {
	printf( "scratchpad[%d] = 0x%02X\n", i, scratchpad[i] );
  }
  
}


// -----------------------------------------------------------------------
//   Read the temperature from one sensor
//
//   Return the high-precision temperature value
//
// -----------------------------------------------------------------------
int OW::read_temperature( int sensor_family, double& value ) {
    
	unsigned char lastcrc8;
	unsigned char scratchpad[30];   // Scratchpad block from the sensor
	
	int strong_err;					// Error with strong pullup?        
	float hi_precision;				// High precision temperature
	
	int tries = 0;					// Tries
	int ds1820_try = 0; 			// Allow ds1820 glitch 1 time 
	int ds18s20_try = 0; 			// Allow DS18S20 error 1 time
	float temp_c = 0;				// Calculated temperature in Centigrade
	
	for( tries = 0; tries < MAX_READ_TRIES; ++tries) {
	
		if( owAccess(0) ) {
			// Convert Temperature
			if( !owWriteBytePower( 0, 0x44 ) ) {
				return FALSE;
			}

			// Sleep for conversion second
			msDelay( read_time );

			// Turn off the strong pullup
			if( owLevel( 0, MODE_NORMAL ) != MODE_NORMAL ) {
				strong_err = 2;
			}

			// Now read the scratchpad from the device
			if( owAccess(0) ) {
			
				// Use Read_Scratchpad instead?
				// Build a block for the Scratchpad read
				scratchpad[0] = 0xBE;
				for( int j = 1; j < 10; j++ ) {
					scratchpad[j] = 0xFF;
				}
				// Send the block
				if( owBlock( 0, FALSE, scratchpad, 10 ) ) {
				
					// Calculate the CRC 8 checksum on the received data
					setcrc8(0, 0);
					for( int j = 1; j < 10; j++ ) {
						lastcrc8 = docrc8( 0, scratchpad[j] );
					}

					// If the CRC8 is valid then calculate the temperature
					if( lastcrc8 == 0x00 ) {

						// DS1822 and DS18B20 use a different calculation
						if( (sensor_family == DS18B20_FAMILY) ||
							(sensor_family == DS1822_FAMILY) ||
							(sensor_family == DS28EA00_FAMILY) ||
							(sensor_family == DS1923_FAMILY) ) {
							short int temp2 = (scratchpad[2] << 8) | scratchpad[1];
							temp_c = temp2 / 16.0;
						}

						// Handle the DS1820 and DS18S20
						if( sensor_family == DS1820_FAMILY ) {

							// Check for DS1820 glitch condition
							// COUNT_PER_C - COUNT_REMAIN == 1

							if( ds1820_try == 0 ) {
								if( (scratchpad[7] - scratchpad[6]) == 1 ) {
									ds1820_try = 1;
									continue;
								} // DS1820 error
							} // ds1820_try

							// Check for DS18S20 Error condition
							// LSB == 0xAA
							// MSB == 0x00
							// COUNT_REMAIN == 0x0C
							// COUNT_PER_C == 0x10

							if( ds18s20_try == 0 ) {
								if( (scratchpad[4]==0xAA) &&
									(scratchpad[3]==0x00) &&
									(scratchpad[7]==0x0C) &&
									(scratchpad[8]==0x10) ) {
									
									ds18s20_try = 1;
									continue;
								} // DS18S20 error condition
							} // ds18s20_try

							// Convert data to temperature
							if( scratchpad[2] == 0 ) {
								temp_c = (int) scratchpad[1] >> 1;
							} else {
								temp_c = -1 * (int) (0x100-scratchpad[1]) >> 1;
							} // Negative temp calculation
							
							temp_c -= 0.25;
							hi_precision = (int) scratchpad[8] - (int) scratchpad[7];
							hi_precision = hi_precision / (int) scratchpad[8];
							temp_c = temp_c + hi_precision;
						
						} // DS1820_FAMILY
						
						// Show the scratchpad if verbose is selected
						show_scratchpad( scratchpad, sensor_family );
						
						// Output the value
						value = temp_c;
						
						// Good conversion finished
						return TRUE;

					} else {
						fprintf( stderr, "CRC Failed. CRC is %02X instead of 0x00\n", lastcrc8 );
						show_scratchpad( scratchpad, sensor_family );
						
					} // CRC 8 is OK
				} // Scratchpad Read
			} // owAccess failed
		} // owAccess failed
    
		// Failed to read, reset the network, delay and try again
		owTouchReset(0);
		msDelay( read_time );
	
	} // for tries < 3
	
	// Failed, no good reads after MAX_READ_TRIES
	return FALSE;
}


// -----------------------------------------------------------------------
//  Select the indicated device, turning on any required couplers
// -----------------------------------------------------------------------
int OW::read_device( struct _roms *sensor_list, int sensor, double& value ) {

	unsigned char   TempSN[8];
	unsigned char   a[3];

	unsigned int s;
	int status = 0;
	int sensor_family;
	
	struct _coupler *c_ptr;          // Coupler linked list

	// Tell the sensor to do a temperature conversion

	// Sort out how to address the sensor.
	// If sensor < num_sensors then it can be directly addressed
	// if sensor >= num_sensors then the coupler must first be
	// addressed and the correct branch turned on.

	if( sensor < sensor_list->max ) {

		// Address the sensor directly
		owSerialNum( 0, &sensor_list->roms[sensor*8], FALSE );

	} else {
		// Step through the coupler list until the right sensor is found.
		// Sensors are in order.

		s = sensor - sensor_list->max;
		c_ptr = coupler_top;

		while( c_ptr ) {

			if( s < c_ptr->num_main ) {
				// Found the right area

				// Is this coupler & branch already on?
				if( !cmpSN( c_ptr->SN, Last2409, 0 ) ) {
					// Turn on the main branch
					if(!SetSwitch1F(0, c_ptr->SN, DIRECT_MAIN_ON, 0, a, TRUE)) {
						printf("Setting Switch to Main ON state failed\n");
						return FALSE;
					}
					// Remember the last selected coupler & Branch
					memcpy( &Last2409, &c_ptr->SN, 8 );
					Last2409[8] = 0;
				}

				// Select the sensor
				owSerialNum( 0, &c_ptr->main[s*8], FALSE );
				break;

			} else {

				s -= c_ptr->num_main;

				if( s < c_ptr->num_aux ) {
					// Found the right area

					// Is this coupler & branch already on?
					if( !cmpSN( c_ptr->SN, Last2409, 1 ) ) {        
						// Turn on the aux branch
						if(!SetSwitch1F(0, c_ptr->SN, AUXILARY_ON, 2, a, TRUE)) {
							printf("Setting Switch to Aux ON state failed\n");
							return FALSE;
						}
						// Remember the last selected coupler & Branch
						memcpy( &Last2409, &c_ptr->SN, 8 );
						Last2409[8] = 1;
					} // Last2409 check

					// Select the sensor
					owSerialNum( 0, &c_ptr->aux[s*8], FALSE );
					break;          
				}
			}
			
			s -= c_ptr->num_aux;
			c_ptr = c_ptr->next;
		}    
	}

	// Get the Serial # selected
	owSerialNum( 0, &TempSN[0], TRUE );
	sensor_family = TempSN[0]; // This is used as switch value

	switch( sensor_family ) {
	
	/*
	case DS28EA00_FAMILY:
	case DS2413_FAMILY:
		if( (opts & OPT_DS2438) || (sensor_family==DS2413_FAMILY) ) { // read PIO
			status = read_pio_ds28ea00( sensor_family, sensor );
		break;
	}
	*/
	// else - drop through to DS1822
	case DS1820_FAMILY:
	case DS1822_FAMILY:
	case DS18B20_FAMILY:
		status = read_temperature( sensor_family, value ); // also for DS28EA00
		break;

	/*case DS1923_FAMILY:
		status = read_temperature_DS1923( sensor_family, sensor );
		break;      

	case DS2422_FAMILY:
	case DS2423_FAMILY:
		status = read_counter( sensor_family, sensor );
		break;

	case DS2438_FAMILY:
		// What type is it?
		for( int page=3; page<8; page++) {
			get_ibl_type( 0, page, 0);
		}
		if( opts & OPT_DS2438 ) {
			status = read_ds2438( sensor_family, sensor );
		} else {
			status = read_humidity( sensor_family, sensor );
		}
		break;*/
	}

  return status;
}


// -----------------------------------------------------------------------
//   Print out a serial number
// ----------------------------------------------------------------------- 
void OW::printSN( unsigned char *TempSN, int crlf ) {

	// Print the serial number   
	for( int y = 0; y < 8; y++ ) {
		printf("%02X", TempSN[y] );
	}
	if( crlf ) {
		printf("\n");
	}
	
}


// -----------------------------------------------------------------------
//   Compare 2 serial numbers (8 bytes)
//   
//   Return:
//     -1 if the 2nd is < 1st
//     0 if equal
//     1 if the 2nd is > 1st
// -----------------------------------------------------------------------
int OW::sercmp( unsigned char *sn1, unsigned char *sn2 ) {

    for (int i=0; i<8; i++) {
        if (sn2[i] < sn1[i]) {
            return -1;
		}
		if (sn2[i] > sn1[i]) {
            return 1;
		}
    }
    
    return 0;
}


// -----------------------------------------------------------------------
//   Find all the supported temperature sensors on the bus, searching down
//   DS2409 hubs on the main bus (but not on other hubs).
//   -----------------------------------------------------------------------
int OW::Init1WireLan( struct _roms *sensor_list, vector<string>& sensor_vector ) {

	unsigned char TempSN[8];
	unsigned char InfoByte[3];
	int result;
	
	unsigned int found_sensors = 0;
	struct _coupler *c_ptr;         // Coupler pointer
	struct _coupler *coupler_end;   // end of the list

	// Free up anything that was read from .digitemprc
	if( sensor_list->roms != NULL ) {
		free( sensor_list->roms );
		sensor_list->roms = NULL;
	}
	
	sensor_list->max = 0;
	num_cs = 0;

	// Free up the coupler list
	free_coupler(0);

	// Initalize the coupler pointer
	coupler_end = coupler_top;

	printf("Turning off all DS2409 Couplers\n");

	result = owFirst( 0, TRUE, FALSE );
	
	while(result) {
		owSerialNum( 0, TempSN, TRUE );

		printf(".");
		fflush(stdout);

		if( TempSN[0] == SWITCH_FAMILY ) {
			// Turn off the Coupler
			if(!SetSwitch1F(0, TempSN, ALL_LINES_OFF, 0, InfoByte, TRUE)) {
				fprintf( stderr, "Setting Coupler to OFF state failed\n");
				free_coupler(0);
				return -1;
			}
		}
		result = owNext( 0, TRUE, FALSE );
	} // HUB OFF search

	printf("\n");

	printf("Searching the 1-Wire LAN\n");

	// Find any DS2409 Couplers and turn them all off
	result = owFirst( 0, TRUE, FALSE );

	while(result) {
		owSerialNum( 0, TempSN, TRUE );

		if( TempSN[0] == SWITCH_FAMILY ) {
			// Print the serial number
			printSN( TempSN, 0 );
			printf(" : %s\n", device_name( TempSN[0]) );

			// Save the Coupler's serial number
			// Create a new entry in the coupler linked list
			if( (c_ptr = (_coupler*) malloc( sizeof( struct _coupler ) ) ) == NULL ) {
				fprintf( stderr, "Failed to allocate %d bytes for coupler linked list\n", (int) sizeof( struct _coupler ) );
				free_coupler(0);
				return -1;
			}

			// Write the serial number to the new list entry
			owSerialNum( 0, c_ptr->SN, TRUE );

			c_ptr->next = NULL;
			c_ptr->num_main = 0;
			c_ptr->num_aux = 0;
			c_ptr->main = NULL;
			c_ptr->aux = NULL;

			if( coupler_top == NULL ) {
				// First coupler, add it to the top of the list
				coupler_top = c_ptr;
				coupler_end = c_ptr;
			} else {
				// Add the new coupler to the end of the list, point to new end
				coupler_end->next = c_ptr;
				coupler_end = c_ptr;        
			}
			
		} else if ((TempSN[0] == DS1820_FAMILY) ||
				   (TempSN[0] == DS1822_FAMILY) ||
				   (TempSN[0] == DS28EA00_FAMILY) ||
				   (TempSN[0] == DS18B20_FAMILY) ||
				   (TempSN[0] == DS1923_FAMILY) ||
				   (TempSN[0] == DS2406_FAMILY) ||
				   (TempSN[0] == DS2413_FAMILY) ||
				   (TempSN[0] == DS2422_FAMILY) ||
				   (TempSN[0] == DS2423_FAMILY) ||
				   (TempSN[0] == DS2438_FAMILY)) {
				  
			// Print the serial number
			printSN( TempSN, 0 );
			printf(" : %s\n", device_name( TempSN[0]) );

			found_sensors = 1;
			// Count the sensors detected
			sensor_list->max++;

			// Allocate enough space for the new serial number
			if( (sensor_list->roms = (unsigned char*) realloc( sensor_list->roms, sensor_list->max * 8 ) ) == NULL ) {

				fprintf( stderr, "Failed to allocate %d bytes for sensor_list\n", sensor_list->max * 8 );
				
				if( sensor_list->roms ) {
					free( sensor_list->roms );
					sensor_list->roms = NULL;
				}
				return -1;
			}
			owSerialNum( 0, &sensor_list->roms[(sensor_list->max-1)*8], TRUE );
		}
		result = owNext( 0, TRUE, FALSE );
	}

	// Now go through each coupler branch and search there
	c_ptr = coupler_top;

	while( c_ptr ) {
		// Search the Main branch
		result = owBranchFirst( 0, c_ptr->SN, FALSE, TRUE );

		while(result) {
			owSerialNum( 0, TempSN, TRUE );

			// Check to see if it is a temperature sensor or a PIO device
			if( (TempSN[0] == DS1820_FAMILY) ||
				(TempSN[0] == DS1822_FAMILY) ||
				(TempSN[0] == DS28EA00_FAMILY) ||
				(TempSN[0] == DS18B20_FAMILY)||
				(TempSN[0] == DS1923_FAMILY) ||
				(TempSN[0] == DS2406_FAMILY) ||
				(TempSN[0] == DS2413_FAMILY) ||
				(TempSN[0] == DS2422_FAMILY) ||
				(TempSN[0] == DS2423_FAMILY) ||
				(TempSN[0] == DS2438_FAMILY) ) {

				// Print the serial number
				printSN( TempSN, 0 );
				printf(" : %s\n", device_name( TempSN[0]) );

				found_sensors = 1;

				// Count the number of sensors on the main branch
				c_ptr->num_main++;

				// Allocate enough space for the new serial number
				if( (c_ptr->main = (unsigned char*) realloc( c_ptr->main, c_ptr->num_main * 8 ) ) == NULL ) {

					fprintf( stderr, "Failed to allocate %d bytes for main branch\n", c_ptr->num_main * 8 );
					free_coupler(0);

					if( sensor_list->roms ) {
						free( sensor_list->roms );
						sensor_list->roms = NULL;
					}

					return -1;
				}

				owSerialNum( 0, &c_ptr->main[(c_ptr->num_main-1)*8], TRUE );
			} // Add serial number to list

			// Find the next device on this branch
			result = owBranchNext(0, c_ptr->SN, FALSE, TRUE );

		} // Main branch loop

		// Search the Aux branch
		result = owBranchFirst( 0, c_ptr->SN, FALSE, FALSE );

		while(result) {

			owSerialNum( 0, TempSN, TRUE );

			if( (TempSN[0] == DS1820_FAMILY) ||
				(TempSN[0] == DS1822_FAMILY) ||
				(TempSN[0] == DS28EA00_FAMILY) ||
				(TempSN[0] == DS18B20_FAMILY)||
				(TempSN[0] == DS1923_FAMILY) ||
				(TempSN[0] == DS2406_FAMILY) ||
				(TempSN[0] == DS2413_FAMILY) ||
				(TempSN[0] == DS2422_FAMILY) ||
				(TempSN[0] == DS2423_FAMILY) ||
				(TempSN[0] == DS2438_FAMILY) ) {

				// Print the serial number
				printSN( TempSN, 0 );
				printf(" : %s\n", device_name( TempSN[0]) );

				found_sensors = 1;
				// Count the number of sensors on the aux branch
				c_ptr->num_aux++;

				// Allocate enough space for the new serial number
				if( (c_ptr->aux = (unsigned char*) realloc( c_ptr->aux, c_ptr->num_aux * 8 ) ) == NULL ) {

					fprintf( stderr, "Failed to allocate %d bytes for aux branch\n", c_ptr->num_main * 8 );
					free_coupler(0);

					if( sensor_list->roms ) {
						free( sensor_list->roms );
						sensor_list->roms = NULL;
					}

					return -1;
				}

				owSerialNum( 0, &c_ptr->aux[(c_ptr->num_aux-1)*8], TRUE );
			} // Add serial number to list

			// Find the next device on this branch
			result = owBranchNext(0, c_ptr->SN, FALSE, FALSE );
		} // Aux branch loop

		c_ptr = c_ptr->next;
	}  // Coupler loop


	//  Did the search find any sensors? Even if there was an error it may
	//  have found some valid sensors

	if( found_sensors ) {

		// Was anything found on the main branch?
		if( sensor_list->max > 0 ) {

			for( int x = 0; x < sensor_list->max; x++ ) {
				printf("ROM #%d : ", x );
				printSN( &sensor_list->roms[x*8], 1 );
			}

		} // num_sensors check

		// Was anything found on any DS2409 couplers?
		c_ptr = coupler_top;

		while( c_ptr ) {
			// Check the main branch
			if( c_ptr->num_main > 0 ) {
				for( unsigned int x = 0; x < c_ptr->num_main; x++ ) {    
					printf("ROM #%d : ", sensor_list->max+num_cs++ );
					printSN( &c_ptr->main[x*8], 1 );
				}
			}

			// Check the aux branch
			if( c_ptr->num_aux > 0 ) {
				for( unsigned int x = 0; x < c_ptr->num_aux; x++ ) {    
					printf("ROM #%d : ", sensor_list->max+num_cs++ );
					printSN( &c_ptr->aux[x*8], 1 );
				}
			}

			// Next Coupler
			c_ptr = c_ptr->next;
			
		} // Coupler list loop

		// Write the new list of sensors to the current directory
		createVector( sensor_list, sensor_vector );
	}
	
	return 0;
}


int OW::buildSensorList( struct _roms *sensor_list, vector<string>& sensor_vector  ) {
	
	// Luetaan olioon nyt sensoreiden tietokannan uusimmat tiedot
	read_ow();
	
	// Rakennetaan sensor_list ja sensor vector uudestaan
	// mutta nyt olion omilla tiedoilla
	
	// Vapautetaan muisti
	if( sensor_list->roms != NULL ) {
		free( sensor_list->roms );
		sensor_list->roms = NULL;
	}
	
	// Make sure the structure is erased
	bzero( &*sensor_list, sizeof( struct _roms ) );
	
	// Tyhjennetään vectori
	sensor_vector.clear();
	
	// kerätään aktiiviset sensorit
	vector<int> sensor_ids;
	
	for (unsigned int i = 0; i < owVect.size(); ++i) {
		if (owVect[i].deviceActive == 1) {
			sensor_ids.push_back(owVect[i].deviceId);
		}
	}
	
	// Varataan muistia sensori taulukolle, ROM tiedot tulevat
	// peräkkäin taulukkoon eli sensorit sijaitsevat peräkkäin taulukossa
	if( sensor_ids.size() > 0 ) {
		if( ( sensor_list->roms = (unsigned char*) malloc( sensor_ids.size() * 8 ) ) == NULL ) {
			fprintf( stderr, "Error reserving memory for %d sensors\n", sensor_ids.size() );
			return -1;
		}
		sensor_list->max = sensor_ids.size(); 
	}
	
	/*
	char* sensor_roms[6] = {"0x10 0x96 0x27 0xA5 0x01 0x08 0x00 0xA4",
								  "0x10 0x11 0x29 0xA5 0x01 0x08 0x00 0x69",
								  "0x10 0x5C 0x10 0xA5 0x01 0x08 0x00 0x21",
								  "0x10 0x6E 0x23 0xA5 0x01 0x08 0x00 0x68",
								  "0x10 0xF9 0x18 0xA5 0x01 0x08 0x00 0xA8",
								  "0x10 0xBF 0x26 0xA5 0x01 0x08 0x00 0x49"};*/
	
	char* ptr; // merkkijonon pilkkomiseen tarkoitettu merkkijonon osoitin
	
	// Käydään läpi kaikki sensorit
	for ( unsigned int i = 0; i < sensor_ids.size(); ++i) {
			
		// Sijoitetaan hex ROM väliaikaismuuttujaan
		char temp_string[40];
		strcpy(temp_string, owVect[sensor_ids[i]].deviceHex);
		
		// pilkotaan merkkijono
		ptr = strtok(temp_string, " ");
		
		// Luetaan 64 bittinen eli 8 tavuinen yhden sensorin tiedot sisään
		for( int x = 0; x < 8; ++x ){
		
			// string to long integer, i on taulukon indeksissä, 
			// jotta sensorit tulevat peräkkäin kasvavasti taulukkoon
			sensor_list->roms[(i * 8) + x] = strtol( ptr, (char **)NULL, 0 );
			
			// jatketaan merkkijonon pilkkomista edellisen lopetuskohdasta
			ptr = strtok( NULL, " ");
		}
		
		// lisätään sensor_vectoriin
		sensor_vector.push_back(owVect[sensor_ids[i]].deviceHex);
	}
	
	// kaikki OK
	return 0;
}


int OW::createVector( struct _roms *sensor_list, vector<string>& sensor_vector ) {
	
	// Luodaan vektori
	unsigned char tempSN[8]; 	 // for ROM
	char hex[5];				 // hex string
	
	for( int x = 0; x < sensor_list->max; ++x ) {
	
		string sensor_rom;		 // ROM stringinä "0x00 0x12 ... 0x33"
		
		for( int y = 0; y < 8; ++y ) { //luodaan ROM
			tempSN[y] = sensor_list->roms[(x * 8) + y];
		}
		
		for( int y = 0; y < 8; ++y) {
			sprintf(hex, "0x%02X", tempSN[y]);		
			sensor_rom.append(hex);
			if (y < 7) {
				sensor_rom.append(" ");
			}
		}
		
		// Lisätään vectoriin
		
		sensor_vector.push_back(sensor_rom);
	}
	
	return 0;
}


bool OW::write_ow(vector<string>& sensor_vector) {
	
	char *zErrMsg;
	sqlite3_stmt* stmt = NULL;

	// BEGIN
	sqlite3_exec(config_db, "BEGIN;", NULL, NULL, &zErrMsg);
	{
		// Ajetaan aluksi kaikki oletuksena ei-aktiivisiksi
		string sql_update("UPDATE ow_data SET deviceActive=0");
		
		int rc = sqlite3_exec(config_db, sql_update.c_str(), NULL, NULL, &zErrMsg);
		
		if( rc!=SQLITE_OK ) {
			
			#ifdef DEBUG
			fprintf(stderr, "SQL error in OW::write_ow() %s \n", zErrMsg);
			#endif
			
			sqlite3_free(zErrMsg);
			
			return false;
		}
		
		// Käydään läpi kaikki vektorit
		
		for (unsigned int i = 0; i < sensor_vector.size(); ++i) {
		
			string sql_query("SELECT deviceId FROM ow_data WHERE deviceHex='");
			sql_query.append(sensor_vector[i].c_str()); // hex
			sql_query.append("'"); // loppuhipsu
			
			//valmistellaan käsky
			int rc = sqlite3_prepare_v2(config_db, sql_query.c_str(), strlen(sql_query.c_str()), &stmt, NULL);
			
			if( rc != SQLITE_OK ){
			
				#ifdef DEBUG
				cerr << "SQL error X2 in OW::write_ow() " << endl;
				#endif
				
			} else {
				
				if ( sqlite3_step(stmt) == SQLITE_ROW ) { // vanha anturi kyseessä, päivitetään aktiiviseksi
					
					// haetaan arvo
					int id = sqlite3_column_int(stmt,0);
					
					char sql_update[200];
					sprintf(sql_update, "UPDATE ow_data SET deviceActive=1 WHERE deviceId=%d", id);
					
					int rc = sqlite3_exec(config_db, sql_update, NULL, NULL, &zErrMsg);
					
					if( rc!=SQLITE_OK ) {
						
						#ifdef DEBUG
						fprintf(stderr, "SQL error in OW::write_ow() sql_update %s \n", zErrMsg);
						#endif
						
						sqlite3_free(zErrMsg);
						
						return false;
					}
					
				} else { // UUSI anturi kyseessä
					
					char sql_insert[300];
					sprintf(sql_insert, "INSERT INTO ow_data (deviceActive, deviceHex, deviceName, deviceType, deviceLog) VALUES (%d, '%s', '%s', '%s', %d)", 
							1, sensor_vector[i].c_str(), "NEW_sensor!", "temp", 0);
					
					int rc = sqlite3_exec(config_db, sql_insert, NULL, NULL, &zErrMsg);
					
					if( rc != SQLITE_OK ) {
						
						#ifdef DEBUG
						fprintf(stderr, "SQL error X4 in OW::write_ow() sql_insert %s \n", zErrMsg);
						#endif
						
						sqlite3_free(zErrMsg);
						
						return false;
					}
				}
				
			}
			
			sqlite3_finalize(stmt);
		}
	}
	// END
	sqlite3_exec(config_db, "END;", NULL, NULL, &zErrMsg);
	
	return true;
}


// -----------------------------------------------------------------------
//  Onewire main routine
// -----------------------------------------------------------------------
void OW::runOnewire() {
	
	// Attached Roms
	struct _roms sensor_list;
	vector<string> sensor_vector;
	
	// ----------------------------------------------
	// Settings
	// ----------------------------------------------
	strcpy( serial_port, "/dev/ttyS2" );
	strcpy( serial_dev, "ttyS2");
	read_time = 1000; // millisekuntteja
	// ----------------------------------------------
	
	// Make sure the structure is erased
	bzero( &sensor_list, sizeof( struct _roms ) );

	// Check to make sure we have permission to access the port
	if( access( serial_port, R_OK|W_OK ) < 0 ) {
		fprintf( stderr, "Error, you don't have +rw permission to access serial port: %s\n", serial_port );

		if( sensor_list.roms != NULL ) {
			free( sensor_list.roms );
			sensor_list.roms = NULL;
		}

		if( coupler_top != NULL ) {
			free_coupler(1);
		}
		
		return;
	}
	
	#ifdef LOCKDEV
	// Lock our use of the serial port, exit if it is in use
	// First turn serial_port into just the final device name
	if( !(p = strrchr( serial_port, '/' )) ) {
		fprintf( stderr, "Error getting serial device from %s\n", serial_port );
		
		if( sensor_list.roms != NULL ) {
			free( sensor_list.roms );
			sensor_list.roms = NULL;
		}

		if( coupler_top != NULL ) {
			free_coupler(1);
		}
		
		return;
	}
  
	strncpy( serial_dev, p+1, sizeof(serial_dev)-1 );

	if( (pid = dev_lock( serial_dev )) != 0 ) {
  
		if( pid == -1 ) {
			fprintf( stderr, "Error locking %s. Do you have permission to write to /var/lock?\n", serial_dev );
		} else {
			fprintf( stderr, "Error, %s is locked by process %d\n", serial_dev, pid );
		}
      
		if( sensor_list.roms != NULL ) {
			free( sensor_list.roms );
			sensor_list.roms = NULL;
		}

		if( coupler_top != NULL ) {
			free_coupler(1);
		}

		return;
	}
	#endif		// LOCKDEV
	
	
	// Yhdistetään Onewireverkkoon
	if( !owAcquire( 0, serial_port) ) {
	
		// Error connecting, print the error
		OWERROR_DUMP(stdout);
		
		// Free memory
		if( sensor_list.roms != NULL ) {
			free( sensor_list.roms );
			sensor_list.roms = NULL;
		}
	
		if( coupler_top != NULL ) {
			free_coupler(1);
		}
		
		#ifdef LOCKDEV
		dev_unlock( serial_dev, 0 );
		#endif
		
		return;
	}
	
	// Luetaan verkko
	Init1WireLan(&sensor_list, sensor_vector);
	
	// Talletetaan tiedot tietokantaan
	write_ow(sensor_vector);

	// Rakennetaan uusi lista
	buildSensorList(&sensor_list, sensor_vector);
	
	// Luetaan olioon nyt aktiivisten sensorien tiedot
	
	int sensor = 0;
	for (unsigned int i = 0; i < owVect.size(); ++i) {
	
		// tähän luetaan sensorin arvo
		double value = 85.0;
		
		// luetaan aktiivisten arvot
		if (owVect[i].deviceActive == 1) {
			
			// luetaan arvo, aktiiviset sensorit ovat nyt samassa 
			// järjestyksessä sensor_listissä sekä oliossa
			read_device( &sensor_list, sensor, value );
			
			// =======================================================================
			pthread_mutex_lock(&owlock);
			// =======================================================================
			
			// tallennetaan arvo
			owVect[i].deviceValueCalc = value;
			
			// tarkistetaan arvon kunnollisuus
			if ( fabs(value - 85.0) < epsilon ) {
				owVect[i].deviceValid = 0;
			} else {
				owVect[i].deviceValid = 1;
			}
			
			// =======================================================================
			pthread_mutex_unlock(&owlock);
			// =======================================================================
			
			++sensor;
		}
	}
	
	// 5. Vapautetaan onewire verkko
	owRelease(0);
	
	#ifdef LOCKDEV
	dev_unlock( serial_dev, 0 );
	#endif
	
	// Vapautetaan muisti
	if( sensor_list.roms != NULL ) {
		free( sensor_list.roms );
		sensor_list.roms = NULL;
	}
	if( coupler_top != NULL ) {
		free_coupler(0);
	}
	
	return;
}


//
//
//
//
// THESE ARE
// NOT IMPLEMENTED OR ARE UNNECESSARY ...
//
//
//
//
//


/*
// -----------------------------------------------------------------------
//   Walk the entire connected 1-wire LAN and display the serial number
//   and type of device.
// ----------------------------------------------------------------------- 
int OW::Walk1Wire() {

	unsigned char TempSN[8];
	unsigned char InfoByte[3];
	short result;
	struct _roms  coupler_list;	// Attached Roms

	bzero( &coupler_list, sizeof( struct _roms ) );

	// Find any DS2409 Couplers and turn them all off.
	// This WILL NOT WORK if there is a coupler attached to the
	// bus of another coupler. DigiTemp only supports couplers
	// on the main 1-Wire LAN.
	
	// We also don't record any couplers in this loop because if
	// one was one and we detected a branch that is closed off
	// after it is turned off we will end up with multiple copies
	// of the same couplers.

	printf("Turning off all DS2409 Couplers\n");

	result = owFirst( 0, TRUE, FALSE );

	while(result) {

		owSerialNum( 0, TempSN, TRUE );	
		printf(".");
		fflush(stdout);

		if( TempSN[0] == SWITCH_FAMILY ) {

			// Turn off the Coupler
			if(!SetSwitch1F(0, TempSN, ALL_LINES_OFF, 0, InfoByte, TRUE)) {
				fprintf( stderr, "Setting Coupler to OFF state failed\n");

				if( coupler_list.roms != NULL ) {
					free( coupler_list.roms );
				}

				return -1;
			}
		}
		
		result = owNext( 0, TRUE, FALSE );
	} // HUB search

	printf("\n");

	// Now we know all the couplers on the main LAN are off, we
	// can now start mapping the 1-Wire LAN

	printf("Devices on the Main LAN\n");

	result = owFirst( 0, TRUE, FALSE );

	while (result) {
		owSerialNum( 0, TempSN, TRUE );

		// Print the serial number   
		printSN( TempSN, 0 );
		printf(" : %s\n", device_name( TempSN[0]) );

		if( TempSN[0] == SWITCH_FAMILY ) {

			// Save the Coupler's serial number so we can explore it later
			// Count the sensors detected
			coupler_list.max++;
			
			// Allocate enough space for the new serial number
			if( (coupler_list.roms = (unsigned char*) realloc( coupler_list.roms, coupler_list.max * 8 ) ) == NULL ) {

				fprintf( stderr,"Failed to allocate %d bytes for coupler_list\n", coupler_list.max * 8 );

				if( coupler_list.roms != NULL ) {
					free( coupler_list.roms );
				}

				return -1;
			}

			owSerialNum( 0, &coupler_list.roms[(coupler_list.max-1)*8], TRUE );

			// Turn off the Coupler
			if(!SetSwitch1F(0, TempSN, ALL_LINES_OFF, 0, InfoByte, TRUE)) {

				fprintf(stderr, "Setting Switch to OFF state failed\n");

				if( coupler_list.roms != NULL ) {
					free( coupler_list.roms );
				}

				return -1;
			}
		}

		result = owNext( 0, TRUE, FALSE );
	} // HUB search

	printf("\n");

	// If there were any 2409 Couplers present walk their trees too
	if( coupler_list.max > 0 ) {

		for(int x = 0; x < coupler_list.max; x++ ) {

			printf("\nDevices on Main Branch of Coupler : ");
			printSN( &coupler_list.roms[x*8], 1 );

			result = owBranchFirst( 0, &coupler_list.roms[x * 8], FALSE, TRUE );

			while (result) {
				owSerialNum( 0, TempSN, TRUE );

				// Print the serial number   
				printSN( TempSN, 0 );
				printf(" : %s\n", device_name( TempSN[0]) );

				result = owBranchNext(0, &coupler_list.roms[x * 8], FALSE, TRUE );
			} // Main branch loop

			printf("\n");
			printf("Devices on Aux Branch of Coupler : ");
			printSN( &coupler_list.roms[x*8], 1 );

			result = owBranchFirst( 0, &coupler_list.roms[x * 8], FALSE, FALSE );

			while (result) {
				owSerialNum( 0, TempSN, TRUE );

				// Print the serial number   
				printSN( TempSN, 0 );
				printf(" : %s\n", device_name( TempSN[0]) );

				result = owBranchNext(0, &coupler_list.roms[x * 8], FALSE, FALSE );
			} // Aux Branch loop
		}  // Coupler loop
	} // num_couplers check

	if( coupler_list.roms != NULL )
	free( coupler_list.roms );
	
	return 0;
}
*/


// -----------------------------------------------------------------------
//   Write a .digitemprc file, it contains:
//   
//   TTY <serial>
//   LOG <logfilepath>
//   READ_TIME <time in mS>
//   LOG_TYPE <from -o>
//   LOG_FORMAT <format string for temperature logging and printing>
//   CNT_FORMAT <format string for counter logging and printing>
//   SENSORS <number of ROM lines>
//   Multiple ROM x <serial number in bytes> lines
//
//   v 2.3 additions:
//   Multiple COUPLER x <serial number in decimal> lines
//   CROM x <COUPLER #> <M or A> <Serial number in decimal>
//
//   v 2.4 additions:
//   All serial numbers are now in Hex.  Still can read older decimal
//     format. 
//   Added 'ALIAS # <string>'  
// -----------------------------------------------------------------------
/*
int OW::write_rcfile( char *fname, struct _roms *sensor_list ) {

	FILE *fp;
	struct _coupler *c_ptr;

	if( ( fp = fopen( fname, "wb" ) ) == NULL ) {
		return -1;
	}

	fprintf( fp, "TTY %s\n", serial_port );
	
	if( log_file[0] != 0 ) {
		fprintf( fp, "LOG %s\n", log_file );
	}
	
	fprintf( fp, "READ_TIME %d\n", read_time );		// mSeconds

	fprintf( fp, "LOG_TYPE %d\n", log_type );
	fprintf( fp, "LOG_FORMAT \"%s\"\n", temp_format );
	fprintf( fp, "CNT_FORMAT \"%s\"\n", counter_format );
	fprintf( fp, "HUM_FORMAT \"%s\"\n", humidity_format );

	fprintf( fp, "SENSORS %d\n", sensor_list->max );

	for( int x = 0; x < sensor_list->max; x++ ) {
		fprintf( fp, "ROM %d ", x );

		for( int y = 0; y < 8; y++ ) {
			fprintf( fp, "0x%02X ", sensor_list->roms[(x * 8) + y] );
		}
		
		fprintf( fp, "\n" );
	}

	// If any DS2409 Couplers were found, write out their information too
	// Write out the couplers first
	c_ptr = coupler_top;
	int x =  0;
	
	while( c_ptr ) {
		fprintf( fp, "COUPLER %d ", x );
		for( int y = 0; y < 8; y++ ) {
			fprintf( fp, "0x%02X ", c_ptr->SN[y] );
		}
		fprintf( fp, "\n" );
		++x;
		c_ptr = c_ptr->next;
	} // Coupler list

	// Sendor # ID for coupler starts at num_sensors
	num_cs = 0;  

	// Start at the top of the coupler list
	c_ptr = coupler_top;
	x =  0;
	
	while( c_ptr ) {
		// Print the devices on this coupler's main branch
		if( c_ptr->num_main > 0 ) {
			for( int i = 0; i < c_ptr->num_main; i++ ) {
				fprintf( fp, "CROM %d %d M ", sensor_list->max+num_cs++, x );

				for( int y = 0; y < 8; y++ ) {
					fprintf( fp, "0x%02X ", c_ptr->main[(i * 8) + y] );
				}
				fprintf( fp, "\n" );
			}
		}

		// Print the devices on this coupler's aux branch
		if( c_ptr->num_aux > 0 ) {
			for( int i = 0; i < c_ptr->num_aux; i++ ) {
				fprintf( fp, "CROM %d %d A ", sensor_list->max+num_cs++, x );

				for( int y = 0; y < 8; y++ ) {
					fprintf( fp, "0x%02X ", c_ptr->aux[(i * 8) + y] );
				}
				fprintf( fp, "\n" );
			}
		}

		++x;
		c_ptr = c_ptr->next;
		
	} // Coupler list

	fclose( fp );
	
	printf( "Wrote %s\n", fname);
	
	return 0;
}
*/


// -----------------------------------------------------------------------
//   Read a .digitemprc file from the current directory
//
//   The rc file contains:
//   
//   TTY <serial>
//   LOG <logfilepath>
//   READ_TIME <time in mS>
//   LOG_TYPE <from -o>
//   LOG_FORMAT <format string for temperature logging and printing>
//   CNT_FORMAT <format string for counter logging and printing>
//   SENSORS <number of ROM lines>
//   Multiple ROM x <serial number in bytes> lines
//
//   v 2.3 additions:
//   Multiple COUPLER x <serial number in decimal> lines
//   CROM x <COUPLER #> <M or A> <Serial number in decimal>
//   
// -----------------------------------------------------------------------
/*
int OW::read_rcfile( char *fname, struct _roms *sensor_list ) {

	FILE	*fp;
	char	temp[1024];
	char	*ptr;
	int	sensors;
	struct _coupler *c_ptr;
	struct _coupler *coupler_end;

	sensors = 0;
	num_cs = 0;
	c_ptr = coupler_top;
	coupler_end = coupler_top;

	if( ( fp = fopen( fname, "r" ) ) == NULL ) {
		// No rcfile to read, could be part of an -i so don't die
		return 1;
	}

	while( fgets( temp, 80, fp ) != 0 ) {
		if( (temp[0] == '\n') || (temp[0] == '#') ) {
			continue;
		}
		  
		ptr = strtok( temp, " \t\n" );

		if( strncasecmp( "TTY", ptr, 3 ) == 0 ) {
			ptr = strtok( NULL, " \t\n" );
			strncpy( serial_port, ptr, sizeof(serial_port)-1 );
		} else if( strncasecmp( "LOG_TYPE", ptr, 8 ) == 0 ) {
			ptr = strtok( NULL, " \t\n");
			log_type = atoi( ptr );
		} else if( strncasecmp( "LOG_FORMAT", ptr, 10 ) == 0 ) {
			ptr = strtok( NULL, "\"\n");
			strncpy( temp_format, ptr, sizeof(temp_format)-1 );
		} else if( strncasecmp( "CNT_FORMAT", ptr, 10 ) == 0 ) {
			ptr = strtok( NULL, "\"\n");
			strncpy( counter_format, ptr, sizeof(counter_format)-1 );
		} else if( strncasecmp( "HUM_FORMAT", ptr, 10 ) == 0 ) {
			ptr = strtok( NULL, "\"\n");
			strncpy( humidity_format, ptr, sizeof(humidity_format)-1 );
		} else if( strncasecmp( "LOG", ptr, 3 ) == 0 ) {
			ptr = strtok( NULL, " \t\n" );
			strncpy( log_file, ptr, sizeof(log_file)-1 );
		} else if( strncasecmp( "FAIL_TIME", ptr, 9 ) == 0 ) {

		} else if( strncasecmp( "READ_TIME", ptr, 9 ) == 0 ) {
			ptr = strtok( NULL, " \t\n");
			read_time = atoi( ptr );
		} else if( strncasecmp( "SENSORS", ptr, 7 ) == 0 ) {
			ptr = strtok( NULL, " \t\n" );
			sensors = atoi( ptr );

			if( sensors > 0 ) {
				// Reserve some memory for the list array
				if( ( sensor_list->roms = (unsigned char*) malloc( sensors * 8 ) ) == NULL ) {
					fprintf( stderr, "Error reserving memory for %d sensors\n", sensors );
					return -1;
				}
				sensor_list->max = sensors; 
			}
		} else if ( strncasecmp( "ROM", ptr, 3 ) == 0 ) {
			// Main LAN sensors
			ptr = strtok( NULL, " \t\n" );
			sensors = atoi( ptr );

			// Read the 8 byte ROM address
			for( int x = 0; x < 8; x++ ){
				ptr = strtok( NULL, " \t\n" );
				sensor_list->roms[(sensors * 8) + x] = strtol( ptr, (char **)NULL, 0 );
			}
		} else if( strncasecmp( "COUPLER", ptr, 7 ) == 0 ) {
			// DS2409 Coupler list, they are ALWAYS in order, so ignore the
			// coupler # and create the list in the order found

			// Allocate space for this coupler
			// Create a new entry in the coupler linked list
			if( (c_ptr = (_coupler*) malloc( sizeof( struct _coupler ) ) ) == NULL ) {
				fprintf( stderr, "Failed to allocate %d bytes for coupler linked list\n", (int) sizeof( struct _coupler ) );
				free_coupler(0);
				if( sensor_list != NULL )
					free(sensor_list);
				#ifndef OWUSB
				owRelease(0);
				#else
				owRelease(0, temp );
				fprintf( stderr, "USB ERROR: %s\n", temp );
				#endif // OWUSB
				return -1;
			}

			c_ptr->next = NULL;
			c_ptr->num_main = 0;
			c_ptr->num_aux = 0;
			c_ptr->main = NULL;
			c_ptr->aux = NULL;

			if( coupler_top == NULL ) {
				// First coupler, add it to the top of the list
				coupler_top = c_ptr;
				coupler_end = c_ptr;
			} else {
				// Add the new coupler to the list, point to new end
				coupler_end->next = c_ptr;
				coupler_end = c_ptr;
			}

			// Ignore the coupler # 
			ptr = strtok( NULL, " \t\n" );

			// Read the 8 byte ROM address
			for( int x = 0; x < 8; x++ ) {
				ptr = strtok( NULL, " \t\n" );
				c_ptr->SN[x] = strtol( ptr, (char **)NULL, 0);
			}
			
		} else if( strncasecmp( "CROM", ptr, 4 ) == 0 ) {
			// Count the number of coupler connected sensors
			num_cs++;

			// DS2409 Coupler sensors    
			// Ignore sensor #, they are all created in order
			ptr = strtok( NULL, " \t\n" );

			// Get the coupler number, and set the pointer to the right
			// coupler

			ptr = strtok( NULL, " \t\n" );
			int x = atoi(ptr);
			c_ptr = coupler_top;
			while( c_ptr && (x > 0) ) {
				c_ptr = c_ptr->next;
				x--;
			}

			// Make sure we are pointing to something
			if( c_ptr ) {
				// Main/Aux branch
				ptr = strtok( NULL, " \t\n" );

				if( *ptr == 'M' ) {
					// Add to the main list
					c_ptr->num_main++;

					// Allocate enough space for the new serial number
					if( (c_ptr->main = (unsigned char*) realloc( c_ptr->main, c_ptr->num_main * 8 ) ) == NULL ) {
						fprintf( stderr, "Failed to allocate %d bytes for main branch\n", c_ptr->num_main * 8 );
						free_coupler(0);
						if( sensor_list != NULL )
						free( sensor_list );
						#ifndef OWUSB
						owRelease(0);
						#else
						owRelease(0, temp );
						fprintf( stderr, "USB ERROR: %s\n", temp );
						#endif // OWUSB
						return -1;
					}

					// Add the serial number to the list
					for( x = 0; x < 8; x++ ) {
						ptr = strtok( NULL, " \t\n" );
						c_ptr->main[((c_ptr->num_main-1)*8)+x] = strtol( ptr, (char **)NULL,0);
					}  
				} else {
					// Add to the aux list
					c_ptr->num_aux++;

					// Allocate enough space for the new serial number
					if( (c_ptr->aux = (unsigned char*) realloc( c_ptr->aux, c_ptr->num_aux * 8 ) ) == NULL ) {
						fprintf( stderr, "Failed to allocate %d bytes for aux branch\n", c_ptr->num_aux * 8 );
						free_coupler(0);
						if( sensor_list != NULL )
							free( sensor_list );
						#ifndef OWUSB
						owRelease(0);
						#else
						owRelease(0, temp );
						fprintf( stderr, "USB ERROR: %s\n", temp );
						#endif // OWUSB
						return -1;
					} // Allocate more aux space

					// Add the serial number to the list
					for( int x = 0; x < 8; x++ ) {
						ptr = strtok( NULL, " \t\n" );
						c_ptr->aux[((c_ptr->num_aux-1)*8)+x] = strtol( ptr, (char **)NULL, 0 );
					} // aux serial number loop
					
				} // Main/Aux branch check
			} // c_ptr Pointing somewhere check
			
		} else {
			fprintf( stderr, "Error reading rcfile: %s\n", fname );
			free( sensor_list );
			fclose( fp );
			return -1;
		}
	}

	fclose( fp ); 

	return 0;
}
*/


// -----------------------------------------------------------------------
// Get 2800 Pio
// -----------------------------------------------------------------------
/*
unsigned short int OW::Get_2800_Pio(int portnum) {
	unsigned short int pio = -1;

	if(owAccess(portnum)) {
		// read pio command
		owWriteByte(portnum, 0xf5);
		pio=owReadByte(portnum);
	}
    if(owAccess(portnum)) {
		return (pio);
	} else {
		return -1;
	}
}

*/

// -----------------------------------------------------------------------
//  Read the DS28ea00 temperature or PIO by Tomasz R. Surmacz
//  (tsurmacz@ict.pwr.wroc.pl)
// -----------------------------------------------------------------------
   /*
int OW::read_pio_ds28ea00( int sensor_family, int sensor )
{
  unsigned char pio;
  char		temp[1024],
  		    time_format[160];
  time_t	mytime;

  
  if ( (sensor_family == DS28EA00_FAMILY) || (sensor_family == DS2413_FAMILY) )
  {
    pio = Get_2800_Pio(0);

	if ( ((pio ^ (pio>>4)) &0xf) != 0xf) {
	  // upper nibble should be complement of lower nibble
          // sprintf( temp, "Sensor %d Read Error (%02x)\n", sensor, pio );
      fprintf(stderr, "Sensor %d Read Error (%02x)\n", sensor,  pio );
	  return FALSE;
	}


    mytime = time(NULL);
    if( mytime )
    {
      // Log the temperature
      switch( log_type )
      {
        // Multiple Centigrade temps per line 
		case 3:
        case 2:     sprintf( temp, "\t%02x", pio );
                    break;

        default:    
                    sprintf( time_format, "%%b %%d %%H:%%M:%%S Sensor %d PIO: %02x, PIO-A: %s PIO-B: %s", sensor, pio, (pio&0x01)?"ON ":"OFF", (pio&0x04)?"ON ":"OFF" );
                    // Handle the time format tokens
                    strftime( temp, 1024, time_format, localtime( &mytime ) );
                    strcat( temp, "\n" );
                    break;
      } // switch( log_type )
    } else {
      sprintf( temp, "Time Error\n" );
    }

    // Log it to stdout, logfile or both
    log_string( temp );
  }

  return FALSE;
}
*/


// -----------------------------------------------------------------------
//   Read the current counter values
// -----------------------------------------------------------------------
   /*
int OW::read_counter( int sensor_family, int sensor )
{
  char          temp[1024];        // For output string
  unsigned char TempSN[8];
  int           page;
  unsigned long counter_value;
  
  if( sensor_family == DS2422_FAMILY )
  {
    // Read Pages 2, 3
    for( page=2; page<=3; page++ )
    {
      if( ReadCounter( 0, page, &counter_value ) )
      {
        // Log the counter
        switch( log_type )
        {
          // Multiple Centigrade temps per line
          case 2:
          case 3:     sprintf( temp, "\t%ld", counter_value );
                      log_string( temp );
                      break;

          default:    owSerialNum( 0, &TempSN[0], TRUE );
                      log_counter( sensor, page-2, counter_value, TempSN );
                      break;
        } // switch( log_type )
      }
    }    
  } else if( sensor_family == DS2423_FAMILY ) {
    // Read Pages 14, 15
    for( page=14; page<=15; page++ )
    {
      if( ReadCounter( 0, page, &counter_value ) )
      {
        // Log the counter
        switch( log_type )
        {
          // Multiple Centigrade temps per line
          case 2:
          case 3:     sprintf( temp, "\t%ld", counter_value );
                      log_string( temp );
                      break;

          default:    owSerialNum( 0, &TempSN[0], TRUE );
                      log_counter( sensor, page-14, counter_value, TempSN );
                      break;
        } // switch( log_type )
      }
    }    
  }

  return FALSE;
}*/


// -----------------------------------------------------------------------
//   Read the DS2406
//   General Purpose PIO
//	 by Tomasz R. Surmacz (tsurmacz@ict.pwr.wroc.pl)
//   !!!! Not finished !!!!
//   Needs an output format string system. Hard-coded for the moment.
// -----------------------------------------------------------------------
   /*
int OW::read_ds2406( int sensor_family, int sensor )
{
  int		pio;
  char		temp[1024],
  		    time_format[160];
  time_t	mytime;

  
  if( sensor_family == DS2406_FAMILY )
  {
    // Read Vdd
    pio = PIO_Reading(0, 0);

    if (pio==-1) {
	printf(" PIO DS2406 sensor %d CRC failed\n", sensor);
	return FALSE;
    }
    mytime = time(NULL);
    if( mytime )
    {
      // Log the temperature
      switch( log_type )
      {
        // Multiple Centigrade temps per line
        case 2:     sprintf( temp, "\t%02x,%02x", pio>>8, pio&0xff );
                    break;

        // Multiple Fahrenheit temps per line
        case 3:     sprintf( temp, "\t%02x,%02x", pio>>8, pio&0xff);
                    break;

        default:    
                    sprintf( time_format, "%%b %%d %%H:%%M:%%S Sensor %d PIO: %02x,%02x, PIO-A: %s%s", sensor, pio>>8, pio&0xff,
			((pio&0x1000)!=0)? // Port A latch: there was a change
				(((pio&0x0400)!=0)?
					"ON"	// and the current state is ON
					:"on")
				:"off",	// the current state is off, no change
			((pio&0x4000)!=0)? // we have 2 ports if bit is 1
				( ((pio&0x2000)!=0)?
					(((pio&0x0800)!=0)? // the latch says 1
						" PIO-B: ON" // and state too
						:" PIO-B: on")
					:" PIO-B: off") // the latch said no
				:
				"")
			;
                    // Handle the time format tokens
                    strftime( temp, 1024, time_format, localtime( &mytime ) );
                    strcat( temp, "\n" );
                    break;
      } // switch( log_type )
    } else {
      sprintf( temp, "Time Error\n" );
    }

    // Log it to stdout, logfile or both
    log_string( temp );
  }

  return TRUE;
}
*/


// -----------------------------------------------------------------------
//  Read the DS2438
//   General Purpose A/D
//   VDD
//   Temperature
//   ...
//
//   !!!! Not finished !!!!
//   Needs an output format string system. Hard-coded for the moment.
// -----------------------------------------------------------------------
   /*
int OW::read_ds2438( int sensor_family, int sensor )
{
  double	temperature;
  float		vdd,
  		ad;
  char		temp[1024],
  		time_format[160];
  time_t	mytime;
  int           cad = 0;

  
  if( sensor_family == DS2438_FAMILY )
  {
    temperature = Get_Temperature(0);

    // Read Vdd
    vdd = Volt_Reading(0, 1, &cad);

    // Read A/D
    ad = Volt_Reading(0, 0, NULL);

    mytime = time(NULL);
    if( mytime )
    {
      // Log the temperature
      switch( log_type )
      {
        // Multiple Centigrade temps per line
        case 2:     sprintf( temp, "\t%3.2f", temperature );
                    break;

        // Multiple Fahrenheit temps per line
        case 3:     sprintf( temp, "\t%3.2f", c2f(temperature) );
                    break;

        default:    
                    sprintf( time_format, "%%b %%d %%H:%%M:%%S Sensor %d VDD: %0.2f AD: %0.2f CAD: %d C: %0.2f", sensor, vdd, ad, cad, temperature );
                    // Handle the time format tokens
                    strftime( temp, 1024, time_format, localtime( &mytime ) );
                    strcat( temp, "\n" );
                    break;
      } // switch( log_type )
    } else {
      sprintf( temp, "Time Error\n" );
    }

    // Log it to stdout, logfile or both
    log_string( temp );
  }

  return FALSE;
}*/


// -----------------------------------------------------------------------
// Read Humidity
// -----------------------------------------------------------------------

/* -----------------------------------------------------------------------
   (This routine is modified from code by Eric Wilde)

   Read the humidity from one sensor (e.g. the AAG TAI8540x).
 
   Log the temperature value and relative humidity.
 
   Calculated using formula cribbed from the Dallas source code (gethumd.c),
   DS2438 data sheet and HIH-3610 data sheet.
 
   Sensors like the TAI8540x use a DS2438 battery monitor to sense temperature
   and convert humidity readings from a Honeywell HIH-3610.  The DS2438
   scratchpad is:
 
   Status/config = scratchpad[2]
   Temp LSB      = scratchpad[3]
   Temp MSB      = scratchpad[4]
   Voltage LSB   = scratchpad[5]
   Voltage MSB   = scratchpad[6]
   CRC           = scratchpad[10]
 
                            Temp LSB
   temp = (Temp MSB * 32) + -------- * 0.03125
                                8
 
   The temperature is a two's complement signed number.
 
   voltage = ((Voltage MSB * 256) + Voltage LSB) / 100
 
   There are two voltages that must be read to get an accurate humidity
   reading.  The supply voltage (VDD) is read to determine what voltage the
   humidity sensor is running at (this affects the zero offset and slope of
   the humidity curve).  The sensor voltage (VAD) is read to get the humidity
   value.  Here is the formula for the humidity (temperature and voltage
   compensated):
    
              ((VAD/VDD) - 0.16) * 161.29
   humidity = ---------------------------
               1.0546 - (0.00216 * temp)
 
   The humidity sensor is linear from approx 10% to 100% R.H.  Accuracy is
   approx 2%.

   !!!! Not Finished !!!!
   ----------------------------------------------------------------------- */
   /*
int OW::read_humidity( int sensor_family, int sensor )
{
  double	temp_c;				// Converted temperature in degrees C
  float		sup_voltage,		// Supply voltage in volts
			hum_voltage,		// Humidity sensor voltage in volts
			humidity;			// Calculated humidity in %RH
  unsigned char	TempSN[8];
  int		try;  
	
  for( try = 0; try < MAX_READ_TRIES; try++ )
  {
    // Read Vdd, the supply voltage
    if( (sup_voltage = Volt_Reading(0, 1, NULL)) != -1.0 )
    {
      // Read A/D reading from the humidity sensor
      if( (hum_voltage = Volt_Reading(0, 0, NULL)) != -1.0 )
      {
        // Read the temperature
        temp_c = Get_Temperature(0);

        // Convert the measured voltage to humidity
        humidity = (((hum_voltage/sup_voltage) - 0.16) * 161.29)
                      / (1.0546 - (0.00216 * temp_c));
	if( humidity > 100.0 ) 
	  humidity = 100.0;
	else if( humidity < 0.0 )
	  humidity = 0.0;

        // Log the temperature and humidity
        owSerialNum( 0, &TempSN[0], TRUE );
        log_humidity( sensor, temp_c, humidity, TempSN );

        // Good conversion finished
        return TRUE;
      }
    }

    owTouchReset(0);
    msDelay(read_time);
  }

  return FALSE;
}
*/


// -----------------------------------------------------------------------
//   Read the DS1923 Hygrochton Temperature/Humidity Logger
// -----------------------------------------------------------------------
   /*
int OW::read_temperature_DS1923( int sensor_family, int sensor )
{
  unsigned char TempSN[8],
  		block2[2];
  int try;                     // Number of tries at reading device 
  int b;
  int pre_t;
  float temp_c;
  int ival;
  float adval;
  float humidity;

  for( try = 0; try < MAX_READ_TRIES; try++ )
  {
    if( owAccess(0) )
    {
      // Force Conversion
      if( !owWriteByte( 0, 0x55 ) || !owWriteByte( 0, 0x55 ))
      {
        return FALSE;
      }
      // TODO CRC checking and read the addresses 020Ch to 020Fh (results)i
       // and the Device Sample Counter at address 0223h to 0225h. 
       // If the count has incremented, the command was executed successfully.
       

      // Sleep for conversion (spec says it takes max 666ms
      // Q. Is it possible to poll?
      msDelay( 666 );
      
      // Now read the memory 0x20C:0x020F
      if( owAccess(0) )
      {
        if( !owWriteByte( 0, 0x69 ) )
        {
          return FALSE;
        }

        // "Latest Temp" in the memory
        block2[0] = 0x0c;
        block2[1] = 0x02;
 
        // Send the block
        if( owBlock( 0, FALSE, block2, 2 ) )
        {
          if (block2[0] != 0x0c && block2[1] != 0x02) 
            return FALSE;

          // Send dummy password
          for(b = 0; b < 8; ++b) {
            owWriteByte(0, 0x04);
          }

          // Read the temperature
          block2[0] = owReadByte(0);
          block2[1] = owReadByte(0);
          pre_t  = (block2[1]/2)-41;
          temp_c = 1.0f * pre_t + block2[0]/512.0f;

          // Read the humidity
          block2[0] = owReadByte(0);
          block2[1] = owReadByte(0);
          ival = (block2[1]*256 + block2[0])/16;
          adval = 1.0f * ival * 5.02f/4096;
          humidity = (adval-0.958f) / 0.0307f;

          // Log the temperature and humidity
	      // TUTAJ masz wartosci we floatach dla Thermochrona
 	      // sensor to nr sensora z pliku konfiguracyjnego,
 	      // a tempsn to pewnie id urzadzenia 1wire
         
          owSerialNum( 0, &TempSN[0], TRUE );
          log_humidity( sensor, temp_c, humidity, TempSN );

          // Good conversion finished
          return TRUE;
        } // Scratchpad Read
      } // owAccess failed
    } // owAccess failed

    // Failed to read, rest the network, delay and try again
    owTouchReset(0);
    msDelay( read_time );
  } // for try < 3
  
  /// Failed, no good reads after MAX_READ_TRIES
  return FALSE;
}
*/

