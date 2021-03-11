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

#ifndef BOXFUNCTIONS_H
#define BOXFUNCTIONS_H

#include <vector>
#include <iostream>
#include <complex>
#include <algorithm>

extern "C" {
	#include "sqlite3.h"
	#include "sys/time.h"
}

using namespace std;

// DEBUG PÄÄLLE/POIS
//#define DEBUG

// SOCKET ADDRESS
#define SOCK_PATH "/tmp/mmm_socket"  // addr to connect

// DATABASE FILES
#define MAIN_DB_FILE    "/mnt/mmc/powercalc.db"
#define NIALM_DB_FILE   "/mnt/mmc/nialm.db"
#define CONFIG_DB_FILE  "/mnt/mmc/config.db"
#define LIVE_DB_FILE 	"/tmp/live.db"

const int BUFFERLENGTH = 512;
const int BUFFERLENGTH2 = 256;

const double pi = 3.1415926535;
const double neper = 2.7182818284;
const double epsilon = 1e-6;

struct cluster {
	double dP; 		// tehojen keskiarvo
	double dP_STD; 	// tehojen keskihajonta
	double prob;	// todennäköisyys
	
	int period;
	int deviceId;   // mihin laitteeseen liitetty
	int N;
	
	//konstruktori
	cluster(double dP_init, double dP_STD_init, double prob_init) : 
		    dP(dP_init), dP_STD(dP_STD_init), prob(prob_init), period(0), deviceId(-1), N(0) {}
	
	//konstruktori
	cluster(double dP_init, double dP_STD_init, double prob_init, int period_init, int deviceId_init, int N_init ) :
	        dP(dP_init), dP_STD(dP_STD_init), prob(prob_init), period(period_init), deviceId(deviceId_init), N(N_init) {}
};

class X_DATA {

  public:
	X_DATA() : deviceId(0), deviceActive(0), deviceLog(0), 
			   deviceValid(0), deviceAlarm(0), deviceLow_limit(0),
			   deviceHigh_limit(0), deviceCalibration(0), deviceEpoch(0) {}
			   
	int deviceId; 				// yksilöllinen numero
	
	int deviceActive; 			// käytössäkö
	char deviceName[30];		// tietoa anturista: esim: vesimittari
	char deviceLogNameCalc[30];	// TÄTÄ EI OLE TIETOKANNASSA
	
	int deviceLog; 			// tallennetaanko tietokantaan
	int deviceValid; 			// luettu arvo kunnollinen
	
	int deviceAlarm; 			// varoitus päälläkö
	double deviceLow_limit;
	double deviceHigh_limit;
	
	double deviceCalibration; 	// kalibrointi offset
	
	int deviceEpoch; 	// viimeisimmän arvon aikaleima
};

class IO_DATA : public X_DATA { // ajonaikaisten muuttujien tietue

  public:
	IO_DATA() :  devicePort(0), devicePortState(0), deviceMeterConst(1000), 
				 deviceCalcTime(3), deviceDelta(20), devicePulses_today(0),
				 devicePulses_minus_minute(0), devicePulses_minute(0),
				 deviceValueCalc(0), i(-1), e(-100), buffercounter(0),
				 buffercounter2(0) {
	
		for (int k = 0; k < 6; ++k) {
			timerV[k].tv_sec = 0; // alustetaan taulukko
			timerV[k].tv_usec = 0;
		}
	}
	
	int devicePort; 			// porttinumero
	int devicePortState;		// portin tila
		
	char deviceTypePulse[20]; 	// esim. energy
	char deviceTypeCalc[20]; 	// esim. power
	char deviceLogNamePulse[30];// TÄTÄ EI OLE TIETOKANNASSA
	
	int deviceMeterConst; 		// pulses/kWh or litra or ...
	int deviceCalcTime; 		// kuinka kauan haetaan pulsseja
	int deviceDelta; 			// muutosprosentti	
	
	int devicePulses_today;
	int devicePulses_minus_minute;
	int devicePulses_minute;
	
	int deviceValueCalc; 		// laskennallinen arvo
	
	int i;
	int e;
	
	struct DATA_HISTORY {
		// rakentaja
		DATA_HISTORY() : Epoch(0), Value(0), Check(true) {} ;
		
		int Epoch;
		int Value;
		bool Check;
	};
	
	DATA_HISTORY x[BUFFERLENGTH]; 	 // Laskettujen arvojen taulukko
	DATA_HISTORY y[BUFFERLENGTH]; 	 // Laskettujen arvojen taulukko
	DATA_HISTORY x_interp[BUFFERLENGTH2];   // Interpoloitujen arvojen taulukko
	DATA_HISTORY y_interp[BUFFERLENGTH2]; 	// Laskettujen arvojen taulukko
	
	int buffercounter;		// circular buffer pointer counter
	int buffercounter2;
	
	DATA_HISTORY delta; // tallennetaan viimeisin muutos tehossa
	
	timeval timerV[6];
	struct timeval timer; 	// TARKKA laskuri tietue
};

class OW_DATA : public X_DATA { //ajonaikaisten muuttujien tietue

  public:
	OW_DATA() : deviceValueCalc(0.0) {}
  
	char deviceHex[60]; 	// dallas koodi
	char deviceType[20]; 	// tyyppi esim. temp
	double deviceValueCalc; // saatu arvo
};

struct RUNTIME_DATA {	
	//rakentaja
	RUNTIME_DATA() : f_reso(0), eka(0) {}
					 
	int f_reso;
	int eka;
};

struct GRAPH {
	//rakentaja
	GRAPH() : f_type("null"), f_unit("null"),
		      sensor_id(0), meterConst(1000.0) {}
	
	string f_type;	
	string f_unit;
	int sensor_id;
	double meterConst;
};

struct CONFIG_DATA {
	//rakentaja
	CONFIG_DATA() : power_day_price(0.0), power_night_price(0.0), 
					power_day_start(0), power_night_start(0) {}
	
	double power_day_price;	
	double power_night_price;	
	int power_day_start;	
	int power_night_start;
};

struct DATE {
	// rakentaja
	DATE(int day, int month, int year) : day(day), month(month), year(year) {} 

	int day;
	int month;
	int year;
};

class Common {
	public:
		//rakentaja
		Common();
		
		void Cgi();
				
		//purkaja
		~Common();
		
	protected:
		sqlite3* db; //SQLITE3 tietokantakahva
		
		vector<GRAPH> Graphs; //graafityypit
		vector<DATE> dateArr; //sisältää päivämäärät
		
		RUNTIME_DATA Rundata; //Määritetään ajonaikaisten muuttujien tietue 
		CONFIG_DATA Conf; 	  //Määritetään asetusmuuttujien tietue
		
		// rakentaa vectorin
		bool buildGraphsVector(string& f_type, string& sensor_id);	

};

// Määritellään oma kompleksivektorityyppi
typedef vector < complex <double> > ComplexVector;

//Funktioiden prototyypit
ComplexVector FFT(ComplexVector data);

void chopString(const string& chopstr, const char* erotin, vector<string>& fields);
int str2int(const string& str);
string int2str(const int i);
float str2float(const string& str);

void genDateArr(vector<DATE>& dateArr, const string& start_date, const string& end_date);

bool openDatabase(const char* dbFilename, sqlite3** dbFile, int timeout);
void closeDatabase(sqlite3** dbFile);

bool test_session(const char* session_id);
bool read_config_file(CONFIG_DATA& Var);

void date_string(char* log_file, int day, int month, int year);
void get_date_string(char* log_file, int shift);

double getSensorValue(int deviceId);
int bayes(double dP, vector<cluster>& clust, double& prob, bool norm);

void closeall(int fd);

inline double closed_interval_rand(double x0, double x1) {
	return x0 + (x1 - x0) * rand() / (static_cast<double>(RAND_MAX));
}

inline int modulo(int a, int b) {
  	int result = a % b; 
	if (result < 0) result += b;
  	return result;
}

inline double sgn(double x) {
	if (x >= 0.0) {
		return 1.0;
	} else {
		return -1.0;
	}
}

#endif
