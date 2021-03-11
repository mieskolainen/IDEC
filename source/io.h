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

#ifndef IO_HH
#define IO_HH

extern "C" {
	#include <pthread.h>
}

#include "boxfunctions.h"

class IO {
  public:
	IO();
	~IO();
	
	void stop();
	
	void pulsing();
	void servering();
	void minute(int minute, int hour);
  
	int read_size();
    bool read_io();
	void read_db();
  
	void pulsecollect(int id);
	void pulsecalculate(int id, int epoch);
	void live_write(int id);
	void interpolate(int id);

	void edge_detect(int id);
	void nialm_db_write(int epoch, int power, int dP);
	
	void db_write(int epoch);
	
  private:
	static void* ioThread(void *threadArg);
	static void* serverThread(void *threadarg);
	
	sqlite3 *nialm_db;
	sqlite3 *config_db;
	
	// mutex
   	pthread_mutex_t iolock;
	
	// thread labels
	pthread_t thread_io;
	pthread_t thread_server;
	
	// thread attributes
	pthread_attr_t thread_io_attr;
	pthread_attr_t thread_server_attr;
	
    vector<IO_DATA> ioVect;
	
	bool alive;
};

#endif
