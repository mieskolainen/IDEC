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
#include <cstdlib>

extern "C" {
	#include <stdio.h>
	#include <time.h>
	#include <string.h>
}

using namespace std;

#include "boxfunctions.h"

// peritään kantaluokka Common
class Archive : public Common {
	public:
		// Rakentaja
		Archive();
		
		// Ajaa varsinaisen käskyn
		void runQuery();
		
		// Purkaja
		~Archive();
		
	private:
	
		struct SENSOR {
			//konstruktori
			SENSOR(int epoch, double value) : epoch(epoch), value(value) {}

			unsigned int epoch;
			double value;
		};
	
		// Tietokantaa lukevat metodit
		void vectorRead(vector<SENSOR>& data_vector, DATE& dateObj, GRAPH& graph);
		void read_factor(DATE& dateObj, GRAPH& graph);
		
		// Ohjausmetodi
		void readPrintData(int g);
		
		// Tietokantaa tulostavat metodit
		void vectorPrint(vector<SENSOR>& data_vector, GRAPH& graph);
		
		//päivämäärätietue
		struct tm* aika;
		
		//time zone in seconds from GMT; EST=-18000, WET=3600
		time_t line_epoch;
		
};


int main() {
	
	// Tarkistus, löytyykö CGI string
	if (getenv("QUERY_STRING") != NULL) { 
		
		// luodaan olio
		Archive arkisto;
		
		// CGI-alustus
		arkisto.Cgi();
		
		// haetaan data
		arkisto.runQuery();
			
		return EXIT_SUCCESS;
		
	} else {
		cerr << "No QUERY_STRING" << endl;	
		return EXIT_FAILURE;
	}
}

// Rakentaja
Archive::Archive() {

}

// Purkaja
Archive::~Archive() {

	//Suljetaan tietokanta
	closeDatabase(&db);
	
}

void Archive::runQuery() {

	//HTML HEADER
	cout << "Content-type:text/html;charset=UTF-8\r\n\r\n" << endl;
	
	//JSON FIRST START
	cout << "[ ";
    
	// Käydään läpi jokainen graafi
    for (unsigned int g = 0; g < Graphs.size() ; ++g) {
	
		//JSON START
		cout << "{ label: '[" << Graphs[g].sensor_id 
			 << "]', unit: '" << Graphs[g].f_unit << "', data: [ ";
		
		// Käydään läpi jokainen päivä
		readPrintData(g);
		
		//JSON END
		cout << " ] } ";
		
		//ei vielä viimeinen graafi
		if (g != (Graphs.size() - 1) ) {	
			cout << " , ";
		}
	}
	
	//PRINT JSON LAST end
	cout << " ] ";
	
}

void Archive::readPrintData(int g) {

	//NOLLATAAN runtime muuttuja
	Rundata.eka = 0;
	
	vector<SENSOR> month_vector;
	
	int last_month = dateArr[0].month;

	//käydään läpi joka päivä
	for (unsigned int x = 0; x < dateArr.size(); ++x) {

		vector<SENSOR> day_vector;
		
		if (Graphs[g].f_type != "factor") { // suoritetaan toiminnot
			if (dateArr[x].month != last_month || x == (dateArr.size() - 1) ) {
				vectorPrint(month_vector, Graphs[g]);
				month_vector.clear();
			}
			if (Rundata.f_reso <= 1440) { //päiväresoluutio ja pienempi
				vectorRead(day_vector, dateArr[x], Graphs[g]);
				vectorPrint(day_vector, Graphs[g]);
			} else { //kuukausiresoluutio
				vectorRead(month_vector, dateArr[x], Graphs[g]);
			}
			
		} else {
			read_factor(dateArr[x], Graphs[g]);
		}
		
		//tallennetaan nykyinen kuukausi
		last_month = dateArr[x].month;
	}
}

void Archive::vectorRead(vector<SENSOR>& data_vector, DATE& dateObj, GRAPH& graph) {

	//LUETAAN KYSEISEN PÄIVÄT ARVOT
	sqlite3_stmt* stmt = NULL;
	
	char sql_read[70];
	
	if (graph.f_type == "power") {
		sprintf(sql_read, "SELECT epoch, power_%d FROM d", graph.sensor_id);
		date_string(sql_read, dateObj.day, dateObj.month, dateObj.year);
		sprintf(sql_read, "%s WHERE power_%d IS NOT NULL", sql_read, graph.sensor_id);
	} else if (graph.f_type == "energy") {
		sprintf(sql_read, "SELECT epoch, energy_%d FROM d", graph.sensor_id);
		date_string(sql_read, dateObj.day, dateObj.month, dateObj.year);
		sprintf(sql_read, "%s WHERE energy_%d IS NOT NULL", sql_read, graph.sensor_id);
	} else if (graph.f_type == "temp") {
		sprintf(sql_read, "SELECT epoch, ow_%d FROM d", graph.sensor_id);
		date_string(sql_read, dateObj.day, dateObj.month, dateObj.year);
		sprintf(sql_read, "%s WHERE ow_%d IS NOT NULL", sql_read, graph.sensor_id);
	}
	
	//valmistellaan käsky
	int rc = sqlite3_prepare_v2(db, sql_read, strlen(sql_read), &stmt, NULL);	
	
	if( rc != SQLITE_OK){
		//printf("SQL error code: %d\n", rc);	
	} else {
		while(sqlite3_step(stmt) == SQLITE_ROW) { //suoritetaan käskyä loopissa
		
			//lisätään data päivävektoriin
			data_vector.push_back(SENSOR(sqlite3_column_int(stmt,0), 
										 sqlite3_column_double(stmt,1)));
		}
	}
	
	sqlite3_finalize(stmt);
}


void Archive::vectorPrint(vector<SENSOR>& data_vector, GRAPH& graph) {
	
	double loop_sum = 0.0;
	double double_i = 0.0;
	int loop_i = 0;
	
	for (unsigned int i = 0; i < data_vector.size(); ++i) {
	
		loop_sum += data_vector[i].value;
		++double_i;
		++loop_i;
		
		if ( (( (loop_i) % Rundata.f_reso == 0) && Rundata.f_reso < 1440) || (i == data_vector.size() - 1) ) {
		
			(Rundata.eka == 1) ? printf(",") : Rundata.eka = 1;

			if (graph.f_type == "energy") {
				line_epoch = data_vector[i - (loop_i - 1)].epoch; // tässä otetaan huomioon myös kesä/talviaika
				aika = localtime(&line_epoch);
				printf("[%ld000,%0.3f]", data_vector[i - (loop_i - 1)].epoch + aika->tm_gmtoff - 30, loop_sum/graph.meterConst);			
			} else {
				line_epoch = data_vector[i - loop_i/2].epoch; // tässä otetaan huomioon myös kesä/talviaika
				aika = localtime(&line_epoch);
				printf("[%ld000,%0.3f]", data_vector[i - loop_i/2].epoch + aika->tm_gmtoff, loop_sum/double_i);
			}
			
			loop_sum = 0.0;
			double_i = 0.0;
			loop_i = 0;
		}
	}
}

void Archive::read_factor(DATE& dateObj, GRAPH& graph) {
	
	// Staattiset muuttujat (alustus nollaksi tapahtuu vain kerran)
	static int calculated_days = 0;
	static double previous_day_mean_out_temp = 0.0;
	static double previous_day_mean_in_temp = 0.0;
	
	// Looppi muuttujat
	int pulses = 0;	
	int i = 0;
	
	double double_i = 0.0; 
	double temp_out_sum = 0.0; 
	double temp_in_sum = 0.0;
	
	// LUETAAN KYSEISEN PÄIVÄT ARVOT
	sqlite3_stmt* stmt = NULL;
	
	char sql_read[50] = "SELECT epoch, energy_0, ow_0, ow_2 FROM d";
	date_string(sql_read, dateObj.day, dateObj.month, dateObj.year);
	
	// valmistellaan käsky
	int rc = sqlite3_prepare_v2(db, sql_read, strlen(sql_read), &stmt, NULL);	
	
	if( rc != SQLITE_OK){	
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
	} else {
		
		++calculated_days;
		
		//suoritetaan käskyä loopissa
		while(sqlite3_step(stmt) == SQLITE_ROW) {
			
			if (i == 0) { 
				line_epoch = sqlite3_column_int(stmt,0); 
			}
			pulses += sqlite3_column_int(stmt,1);
			temp_out_sum += sqlite3_column_double(stmt,2);
			temp_in_sum += sqlite3_column_double(stmt,3);
					
			++i;
			++double_i;
		}
		
		sqlite3_finalize(stmt);
		
		aika = localtime(&line_epoch);		
		
		if (calculated_days >= 2) {
		
			(Rundata.eka == 1) ? printf(",") : Rundata.eka = 1;
			
			printf("[%ld000,%0.1f]", line_epoch + aika->tm_gmtoff - 30,
					(pulses/graph.meterConst)/(previous_day_mean_in_temp - previous_day_mean_out_temp) );
		}
		
		previous_day_mean_out_temp = temp_out_sum / double_i;
		previous_day_mean_in_temp = temp_in_sum / double_i;
	}
}

/*
void csvPrint(vector<ENERGY>& day_vector) {

	// Avataan tiedosto kirjoitettavaksi
	string tiedosto = "/tmp/power.csv";
	
	ofstream textfile(tiedosto.c_str(), ios::app);
	
	if (textfile.is_open()) {

		// Käydään läpi päiväkirjamerkinnät
		for (unsigned int i = 0; i < day_vector.size(); ++i) {
	
			// Kirjoitetaan rivi
			textfile << day_vector[i].epoch << "," << day_vector[i].value << endl;
			
		}
	
		// purkaja hoitaisi muutenkin
		textfile.close();
	  
	} else {
		//cerr << "Virhe: tiedostoa ei voida avata kirjoitettavaksi" << endl;
	}
	
}
*/
