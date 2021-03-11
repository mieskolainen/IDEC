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
	#include <stdlib.h>
	#include <time.h>
	#include <string.h>
}

using namespace std;

#include "boxfunctions.h"

Common::Common() {

	// Avataan tietokanta
	if (openDatabase(MAIN_DB_FILE, &db, 30000) == false) {
		//throw exception
	}
	
	// Luetaan asetukset tekstitiedostosta
	if (read_config_file(Conf) == false) { 
		//throw exception
	}

}

Common::~Common() {

	//Suljetaan tietokanta
	closeDatabase(&db);
	
}

//testi stringi
//const char* testi = "time=01&f_reso=60&date1=13-11-2009&date2=15-11-2009&si=333970e3746d36d304b2920e39a1e337&f_type=temp,temp,temp&sensor_id=1,2,3";

void Common::Cgi() {

	string start_date;
	string end_date;
	
	string f_type;
	string sensor_id;
	
	string session_id;

	// asetetaan CGI query
	string query_string(getenv("QUERY_STRING"));
	
	//tähän tallennetaan erotetut kentät, esim. "date1=11-11-2009"
	vector<string> fields;
	
	//pilkotaan tiedot
	chopString(query_string, "&", fields);
	
	//sijoitetaan tiedot
	for (unsigned int i = 0; i < fields.size(); ++i) {
	
		int pos = fields[i].find("="); // "=" merkki toimii erottimena
		string name = fields[i].substr(0, pos); // haetaan kentän nimi
		string value = fields[i].substr(pos + 1, fields[i].length()); // haetaan arvo
		
		if (name == "time") {
			//only to prevent GET-caching in browsers
		} else if (name == "f_reso") {
			Rundata.f_reso = str2int(value);
		} else if (name == "date1") {
			start_date = value;
		} else if (name == "date2") {
			end_date = value;
		} else if (name == "si") {
			session_id = value;
		} else if (name == "f_type") {
			f_type = value;
		} else if (name == "sensor_id") {
			sensor_id = value;
		}
		
	}
	
	// testataan löytyykö sessiontiedosto
	if ( test_session(session_id.c_str()) == false) {
		cerr << "SI error" << endl; //ei löytynyt /tmp/sess_tiedostoa
		// throw exception
	}
	
	//haetaan päivämäärät
	genDateArr(dateArr, start_date, end_date);	
	
	//rakennetaan vektori
	if (buildGraphsVector(f_type, sensor_id) == false) {
		cerr << "vector build error" << endl;
		//throw exception
	}
	
}


bool Common::buildGraphsVector(string& f_type, string& sensor_id) {

	vector<string> type_fields;
	vector<string> sensor_fields;
	
	//pilkotaan tiedot
	chopString(f_type, ",", type_fields);
	chopString(sensor_id, ",", sensor_fields);
	
	//virheellinen syöte
	if (type_fields.size() != sensor_fields.size()) {
		return false;
	} else { //muutetan vectorin koko
		Graphs.resize(type_fields.size());
	}
	
	for (unsigned int i = 0; i < Graphs.size(); ++i) {
		Graphs[i].f_type = type_fields[i];
		Graphs[i].sensor_id = str2int(sensor_fields[i]);
	}
	
	//asetustietokanta
  	sqlite3 *config_db = NULL;
  	
	//avataan tietokanta
	if (openDatabase(CONFIG_DB_FILE, &config_db, 5000)==false) {
		return false;
	}
	
  	sqlite3_stmt* stmt = NULL;
  	
  	for (unsigned int i = 0; i < Graphs.size(); ++i) {
	  	
	  	if ( Graphs[i].f_type == "energy" ) {
		
		  	char sql[150];
		  	sprintf(sql, "SELECT deviceMeterConst FROM io_data WHERE deviceId='%d'", Graphs[i].sensor_id);	  	
		  	
		  	//valmistellaan käsky
			int rc = sqlite3_prepare_v2(config_db, sql, strlen(sql), &stmt, NULL);
			
			if( rc!=SQLITE_OK ){		
				//printf("SQL error code: %d\n", rc);
			} else {
			
				//ajetaan käsky
				sqlite3_step(stmt);
				
				Graphs[i].meterConst = sqlite3_column_double(stmt,0);
			}
			
			sqlite3_finalize(stmt);
	  	
  		}
		
	}
	
	closeDatabase(&config_db);
	
	for (unsigned int g = 0; g < Graphs.size() ; ++g) {
		
		if ( Graphs[g].f_type == "power" ) {
			Graphs[g].f_unit = "W";
		}
		else if ( Graphs[g].f_type == "energy" ) {
			Graphs[g].f_unit = "kWh";
		}		
		else if ( Graphs[g].f_type == "temp" ) {
			Graphs[g].f_unit = "&deg;C";
		}
		else if ( Graphs[g].f_type == "factor" ) {
			Graphs[g].f_unit = "kWh/&deg;C";
		}
		
	}
	
	return true;

}



// Funktio FFT laskee syötevektorissa "data" olevien lukujen
// Fourier-muunnoksen käyttäen nopeaa Fourier-muunnosta.
// Syötevektorin alkioiden määrä on oltava muotoa 2^n (n=0,1,2,...).
// Funktio palauttaa muunnetun vektorin.

ComplexVector FFT(ComplexVector data) {

   int N = data.size();
   
   ComplexVector even;
   ComplexVector odd;
   ComplexVector evenResult;
   ComplexVector oddResult;
   ComplexVector finalResult (N);
   
   // Määritellään pii, imaginääriyksikkö ja ykkösen N:s juuri.
   double pi = 3.14159265358979;
   complex<double> I (0,1);
   complex<double> w_N = exp(2*pi*I / static_cast<double>(N));
   
   // Jos muunnettavan vektorin dimensio on 1, rekursio päättyy.
   if (N == 1) {
      return data;
   }
   
   // Jaetaan syöte paritonindeksisiin (odd) ja 
   // parillisindeksisiin (even) alkioihin.
   for (int k = 0; k < N; k += 2) {
      even.push_back (data [k]);
      odd.push_back  (data [k+1]);
   }
   
   // Lasketaan näin saatujen N/2-ulotteisten 
   // vektoreiden (even ja odd) Fourier-muunnokset.
   evenResult = FFT (even);
   oddResult  = FFT (odd);
   
   // Lasketaan lopullinen tulos yhdistämällä jonot
   // kaavalla (3.4).
   for (int k = 0; k < N; ++k) {
      finalResult[k] = evenResult [k % (N/2)] 
         + pow (w_N, -k) * oddResult [k % (N/2)];
   }
   
   return finalResult;
}

//Aliohjelma pilkkoo merkkijonon erotinmerkin perusteella vector taulukkoon
void chopString(const string& chopstr, const char* erotin, vector<string>& fields) {

   //alkupiste, josta lähdetään pilkkomaan kenttiä
   string::size_type alkukohta = 0;

   //käydään läpi kaikki annetut kentät merkkijonossa
   while (true) {
      //Etsitään alkukohtaa seuraava erotin
      string::size_type erotinkohta = chopstr.find(erotin, alkukohta);

      //Ei löydetty erotinta --> viimeinen kenttä kyseessä
      if ( erotinkohta == string::npos ) {
         fields.push_back(chopstr.substr(alkukohta));
         break;
         //Kenttä sijaitsee alkukohdan ja erottimen välissä
      } else {
         fields.push_back(chopstr.substr(alkukohta, erotinkohta-alkukohta));
         alkukohta = erotinkohta + 1;
      }
   }
}

int str2int(const string& str) {

	//muunnetaan tyyppiä tietovirran avulla (STRING TO INT)
	int integer = 0;
	istringstream tempstream(str);
	tempstream >> integer;

    return integer;
}

string int2str(const int i) {

	std::string s;
	std::stringstream out;
	out << i;
	s = out.str();
	
	return s;
}

float str2float(const string& str) {

	//muunnetaan tyyppiä tietovirran avulla (FLOAT TO INT)
	float val = 0.0;
	istringstream tempstream(str);
	tempstream >> val;

    return val;
}

void genDateArr(vector<DATE>& dateArr, const string& start_date, const string& end_date) {
	
	vector<string> start_fields;
	vector<string> end_fields;
	
	chopString(start_date, "-", start_fields);
	chopString(end_date, "-", end_fields);
	
	int d1 = str2int(start_fields[0]);
	int m1 = str2int(start_fields[1]);
	int y1 = str2int(start_fields[2]);
	
	int d2 = str2int(end_fields[0]);
	int m2 = str2int(end_fields[1]);
	int y2 = str2int(end_fields[2]);
	
	int startday = 0; 	int endday = 0;
	int startmonth = 0; 	int endmonth = 0;
	int startyear = y1; 	int endyear = y2;
	
	//kuukausien pv taulukko
	int dates[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	for (int h = startyear; h < endyear + 1; ++h) { //year loop
		
		//haku saman vuoden sisältä
		if (h==endyear && startyear==endyear) { 
			startmonth=m1;
			endmonth=m2;
		}		
		
		//aloitusvuosi ja viimeinen vuosi on eri kuin aloitusvuosi
		if (h==startyear && startyear!=endyear) { 
			startmonth=m1;
			endmonth=12;
		}
		
		//välivuodet useamman vuoden hauassa
		if ( h>startyear && h<endyear ) { 
			startmonth=1;
			endmonth=12;	
		}
		
		//siirrytään viimeiselle vuodelle usamman vuoden hauassa
		if (h==endyear && startyear!=endyear) { 
			startmonth=1;
			endmonth=m2;
		}
		
		//Tarkastellaan karkausvuosi
		if ( (h % 4 == 0 && h % 100 != 0) || (h % 400 == 0) ) {  
			dates[2]=29;
		}
		
		for (int i = startmonth; i < endmonth + 1; ++i) { //month loop
		
			startday=1;	
			endday=dates[i];
			
			//aloituskuukausi aloitusvuodelta
			if (i==startmonth && h==startyear) { 
				startday=d1;
			}
				
			//siirrytään viimeiselle kuulle
			if (i==endmonth && h==endyear) { 
				endday=d2;
			}
				
			for (int j = startday; j < endday + 1; ++j) { //day loop
			
				dateArr.push_back(DATE(j, i, h));
			
			} //day for-loop
			
		} //month for-loop
	
	} //year for-loop	
	
}

bool openDatabase(const char* dbFilename, sqlite3** dbFile, int timeout) {

	// Avataan yhteys tietokantaan
    int rc = sqlite3_open(dbFilename, &*dbFile);
    
    if( rc ) {
	    
        // sqlite3_errmsg hakee esille virhesanoman
        printf("Can't open database!\n%s", sqlite3_errmsg(*dbFile) );
        sqlite3_close(*dbFile);
        
        return false;
    }
	
    sqlite3_busy_timeout(*dbFile, timeout);
	
    return true;
}

void closeDatabase(sqlite3** dbFile) {
	
	//suljetaan tietokanta
	sqlite3_close(*dbFile);
	
}

bool test_session(const char* session_id) {
	
	char session_file[50]="/tmp/sess_"; //php session tiedosto

	strcat(session_file, session_id); //liitetään merkkijono perään
	
	FILE *session_fp; 
	session_fp = fopen(session_file, "r");   
    	
   	if (session_fp == NULL) {       
        return false;
	} else {
		fclose(session_fp);
		return true;
	}
}

bool read_config_file(CONFIG_DATA& Conf) {
	
	//luetaan asetukset	    
    char name[30];
	float value = 0.0;
	
	FILE* config_file;   	
	char config_buff[50]; //bufferi   	
	config_file = fopen("/disk/powercalc.conf", "r");
	
	if (config_file==NULL) {		
    	return false;
	} else { 
    	     		
		while ( fgets( config_buff, sizeof config_buff, config_file ) != NULL ) {
	 			 
			if ( sscanf( config_buff, "%s %f", name, &value ) == 2 ) {
	
				if (strcmp(name, "power_day_price")==0) {
	    			Conf.power_day_price = value;
				}
				else if (strcmp(name, "power_night_price")==0) {
	    			Conf.power_night_price = value;
				}
				else if (strcmp(name, "power_day_start")==0) {	        			
	    			Conf.power_day_start = (int) value;
				}
				else if (strcmp(name, "power_night_start")==0) {	        			
	    			Conf.power_night_start = (int) value;	
				}
			
		 	}
			 	
		}
		
	    fclose(config_file);
	    
	    return true;
    
	}
	
}


void date_string(char* log_file, int day, int month, int year) {
	
	char day_s[3];
	char month_s[3];
	char year_s[5];
	
	if (day < 10) {	
		sprintf(day_s, "0%d", day); //int to string			
	} else {
		sprintf(day_s, "%d", day); //int to string
	}
	
	if (month < 10) {		
		sprintf(month_s, "0%d", month);
	} else {
		sprintf(month_s, "%d", month);
	}
	
	sprintf(year_s, "%d", year); 
	
	strcat(&*log_file, day_s); //liitetään merkkijono perään	
	strcat(&*log_file, month_s); 	
	strcat(&*log_file, year_s);
	
}


int bayes(double dP, vector<cluster>& clust, double& prob, bool norm) {
	
	int K_index = 0;
	double max = 0.0;
	double nimittaja = 1.0;
	
	// kaikille yhteinen nimittaja 
	// (sanotaan myös normeerausvakioksi, ei välttämättä käytetä aina lainkaan
	//  esim. puheentunnistuksessa)
	//
	// p(x) = sum[k=1...C]( p(x|Wk)*P(Wk) )
	// missä p(x|Wk) = 1/(sqrt(2*pi)*sigma_k)*e(-(x-mju_k)^2/(2*sigma_k^2))
	
	if (norm == true) { // halutaan normeeraus
		
		nimittaja = 0.0;
	
		for (unsigned int k = 0; k < clust.size(); ++k) {	
		
			if (sgn(dP) == sgn(clust[k].dP)) { // käydään läpi vain joko neg/pos
			
				double eksponent = - pow(static_cast<double>(dP - clust[k].dP), 2.0)
								 / (2.0*pow(static_cast<double>(clust[k].dP_STD), 2.0));
				
				// TÄMÄ pitää vielä korjata cluster ohjelmassa
				if (clust[k].dP_STD < 0.5) {
					clust[k].dP_STD = 1;
				}
				
				nimittaja += (1.0/(sqrt(2.0*pi)*clust[k].dP_STD))
						   * pow(neper, eksponent) * clust[k].prob;
			}
		}
	}
	
	// lopullinen lasku: P(Wj|x) = p(x|Wj)P(Wj)/p(x)
	// missä p(x) laskettiin ylempänä ja
	// missä p(x|Wj) = 1/(sqrt(2*pi)*sigma_j)*e(-(x-mju_j)^2/(2*sigma_j^2))
	
	for (unsigned int j = 0; j < clust.size(); ++j) {
		
		if (sgn(dP) == sgn(clust[j].dP)) { // käydään läpi vain joko neg/pos
		
			double eksponent = - pow(static_cast<double>(dP - clust[j].dP), 2.0)
							   / (2.0*pow(static_cast<double>(clust[j].dP_STD), 2.0));
			
			double osoittaja = (1.0/(sqrt(2.0*pi)*clust[j].dP_STD)) 
							  * pow(neper, eksponent) * clust[j].prob;
			
			double prob = osoittaja/nimittaja;
			
			if (prob > max) { // haetaan suurin todennäköisyys
				max = prob;   // uusi maksimi
				K_index = j;
			}
		}
	}
	
	// todennäköisyys
	prob = max;
	
	return K_index;
}


double getSensorValue(int deviceId) {

	sqlite3_stmt* stmt = NULL;
	
	// Tietokanta
	sqlite3* live_db;
	
	// avataan tietokanta
	if (openDatabase(LIVE_DB_FILE, &live_db, 30000) == false) {
		#ifdef DEBUG
		cout << "openDatabase LIVE_DB error " << endl;
		throw "error";
		#endif
	}
	
	double deviceValueCalc = -100.0;
	
	// luetaan lämpötila
	char sql_query[200];
	sprintf(sql_query, "SELECT deviceValueCalc FROM ow_live WHERE deviceId=%d", deviceId);
	
	//valmistellaan käsky
	int rc = sqlite3_prepare_v2(live_db, sql_query, strlen(sql_query), &stmt, NULL);
	
	if( rc != SQLITE_OK ){
		#ifdef DEBUG
		cerr << "SQL error in OW::write_ow() " << endl;
		#endif
	} else {
		if ( sqlite3_step(stmt) == SQLITE_ROW ) { // haluttu anturi löytyi
		
			deviceValueCalc = sqlite3_column_double(stmt,0);
			
		}
	}
	
	sqlite3_finalize(stmt);
	
	closeDatabase(&live_db);
	
	return deviceValueCalc;
}

void get_date_string(char *log_file, int shift) {

	// =======================================================================
	//ERI AIKATIETUE, JOTTEI SOTKETA PÄÄOHJELMAN VASTAAVAA (THREAD SAFETY)
	
	//seconds from epoch
    time_t now;
	//päivämäärätietue
	struct tm *stringTime;
	 	
	//Luetaan epoch
	time(&now);
	//Muutetaan kuluneet sekunnit tm tietueen kenttiin sopiviin muotoihin	
	stringTime = localtime(&now);
	// =======================================================================
	
	// jotta pulssiarvo oikealle päivälle -> 
	// aikaleima 00.00:n arvo edellisen päivän tiedostoon
	if ( stringTime->tm_hour==0 && stringTime->tm_min==0 && shift==1) {
		now -= 60; //vähennetään minuutti	
		stringTime = localtime(&now);	    	
	}
	
	char day_s[3], month_s[3], year_s[5];
	
	int day, month, year = 0;
	
	day = stringTime->tm_mday;
	month = stringTime->tm_mon+1;
	year = stringTime->tm_year+1900;
	
	if (day<10) {	
		sprintf(day_s, "0%d", day); //int to string			
	} else {
		sprintf(day_s, "%d", day); //int to string
	}
		
	if (month<10) {		
		sprintf(month_s, "0%d", month);
	} else {
		sprintf(month_s, "%d", month);
	}
		
	sprintf(year_s, "%d", year); 

	strcat(&*log_file, day_s); //liitetään merkkijono perään	
	strcat(&*log_file, month_s); 	
	strcat(&*log_file, year_s);
}

void closeall(int fd) {
	// close all FDs >= a specified value
	
    int fdlimit = sysconf(_SC_OPEN_MAX);

    while (fd < fdlimit) {
      close(fd++);
  	}
}




