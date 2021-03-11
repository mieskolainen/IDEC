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

#ifndef OW_HH
#define OW_HH

#include <pthread.h>

#include "boxfunctions.h"

// Family codes for supported devices
#define DS1820_FAMILY	0x10
#define DS1822_FAMILY	0x22
#define DS18B20_FAMILY	0x28
#define DS28EA00_FAMILY 0x42
#define DS1923_FAMILY   0x41
#define DS2406_FAMILY   0x12
#define DS2422_FAMILY	0x1C
#define DS2423_FAMILY	0x1D
#define DS2438_FAMILY   0x26
// and non-supported yet, but coming soon
#define DS2408_FAMILY	0x29
#define DS2413_FAMILY	0x3A

// Coupler related definitions
#define SWITCH_FAMILY      0x1F
#define MAXDEVICES         15
#define ALL_LINES_OFF      0
#define DIRECT_MAIN_ON     1
#define AUXILARY_ON        2
#define STATUS_RW          3

// Number of tries to read a sensor before giving up
#define MAX_READ_TRIES	3

struct _roms {
	unsigned char *roms;		// Array of 8 bytes
	int max;                    // Maximum number
};

struct _coupler {
	unsigned char SN[8];		// Serial # of this Coupler
	unsigned int num_main;		// # of devices on main
	unsigned int num_aux;		// # of devices on aux
	
	unsigned char *main;		// Array of 8 byte serial nums
	unsigned char *aux;			// Array of 8 byte serial nums
	
	struct _coupler *next;
};

class OW {
  public:
	OW();
	~OW();
	
	void stop();
	
    int read_size();
	
	bool write_ow(vector<string>& sensor_vector);
	bool read_ow();
	void db_write(int epoch);
	void live_db_write();
	
	// ONEWIRE
	void runOnewire();
	
	char *device_name( unsigned int family );
	int file_exists (char * fileName);
	int createVector( struct _roms *sensor_list, vector<string>& sensor_vector );
	
	int buildSensorList( struct _roms *sensor_list, vector<string>& sensor_vector );
	int read_rcfile( char *fname, struct _roms *sensor_list );
	
	void free_coupler( int free_only );
	float c2f( float temp );
	
	int cmpSN( unsigned char *sn1, unsigned char *sn2, int branch );
	void show_scratchpad( unsigned char *scratchpad, int sensor_family );
	int read_temperature( int sensor_family, double& value );
	int read_device( struct _roms *sensor_list, int sensor, double& value );
	void printSN( unsigned char *TempSN, int crlf );
	int sercmp( unsigned char *sn1, unsigned char *sn2 );
	int Init1WireLan( struct _roms *sensor_list, vector<string>& sensor_vector );
	int Walk1Wire();
	
	//int read_ds2406( int sensor_family, int sensor );
	//int read_counter( int sensor_family, int sensor );
	//int read_ds2438( int sensor_family, int sensor );
	//int read_humidity( int sensor_family, int sensor );
	//int read_temperature_DS1923( int sensor_family, int sensor );
	//unsigned short int Get_2800_Pio(int portnum);
	//int read_pio_ds28ea00( int sensor_family, int sensor );
	//int get_ibl_type(int portnum, unsigned char page, int offset); // From ds2438.c
	
  private:
	static void *owThread(void *threadarg);

	sqlite3 *config_db;
	
    pthread_mutex_t owlock;			// mutex
	pthread_t thread_ow;			// thread labels
	pthread_attr_t thread_ow_attr;  // thread attributes

	vector<OW_DATA> owVect;
	
	bool alive;						// object alive sign for thread
};

#endif







