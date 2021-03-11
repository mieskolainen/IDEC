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
#include <algorithm>

extern "C" {
	#include <stdio.h>
	#include <time.h>
	#include <string.h>
}

using namespace std;

#include "boxfunctions.h"

const unsigned int CALCULATION_LIMIT = 31; //DAYS

//peritään kantaluokka Common
class Statistics : public Common {
	public:
		//rakentaja
		Statistics();
		
		// Ajaa varsinaisen käskyn
		void runQuery();
		
		//purkaja
		~Statistics();
		
	private:		
		void readDb(DATE& dateObj, unsigned int days);
		void printOut(int g);
		
		vector<unsigned short int> day_energy;
		vector<unsigned short int> night_energy;
		
		unsigned long int day_pulses;
		unsigned long int day_n;
		
		unsigned long int night_pulses;
		unsigned long int night_n;
		
};

int main(void) {

	// Tarkistus, löytyykö CGI string
	if (getenv("QUERY_STRING") != NULL) { 
		
		// luodaan olio
		Statistics stat;
		
		// CGI-alustus
		stat.Cgi();
		
		// haetaan data
		stat.runQuery();
		
		return EXIT_SUCCESS;
		
	} else {
	
		cerr << "No QUERY_STRING" << endl;	
		return EXIT_FAILURE;
		
	}
	
}

//Rakentaja
Statistics::Statistics() : day_pulses(0), day_n(0), night_pulses(0), night_n(0) {
}

// Purkaja
Statistics::~Statistics() {

}

void Statistics::runQuery() {

	//HTML HEADER
	cout << "Content-type:text/html;charset=UTF-8\r\n\r\n" << endl;

	//käydään läpi joka anturi
	for (unsigned int g = 0; g < Graphs.size(); ++g) {
	
		//käydään läpi joka päivä
		for (unsigned int x = 0; x < dateArr.size(); ++x) {
			readDb(dateArr[x], dateArr.size()); //luetaan tietokantaa			
		}
		
		//Tulostetaan tiedot
		printOut(g);
		
		//nollataan tiedot
		day_energy.clear();
		night_energy.clear();
	
	}
	
}

void Statistics::readDb(DATE& dateObj, unsigned int days) {

	//LUETAAN KYSEISEN PÄIVÄT ARVOT
	sqlite3_stmt* stmt = NULL;
	
	char sql_read[50] = "SELECT epoch, energy_0 FROM d";
	date_string(sql_read, dateObj.day, dateObj.month, dateObj.year);
	
	//valmistellaan käsky
	int rc = sqlite3_prepare_v2(db, sql_read, strlen(sql_read), &stmt, NULL);	
	
	if( rc!=SQLITE_OK){	
	
		//tapahtui virhe	
		//printf("SQL error code: %d\n", rc);

	} else {
	
		struct tm *aika; //päivämäärätietue
		time_t line_epoch = 0;
		int hour = 0;
		
		//suoritetaan käskyä loopissa
		int i = 0;
		
		while(sqlite3_step(stmt) == SQLITE_ROW) {
				
			if ( i % 30 == 0 ) { //NOPEUTUS, luetaan vain joka kolmaskymmenes aikaleima
				
				line_epoch = sqlite3_column_int(stmt,0);				
				aika = localtime(&line_epoch);
				hour = aika->tm_hour; //luetaan nykyinen tunti
			}
			
			//yösähkö
			if (hour >= Conf.power_night_start  || hour < Conf.power_day_start) {	
			
				if (days <= CALCULATION_LIMIT) {
					night_energy.push_back(sqlite3_column_int(stmt,1));
				}
				night_pulses += sqlite3_column_int(stmt,1);
				++night_n;
				
			} else { //päiväsähkö
			
				if (days <= CALCULATION_LIMIT) {
					day_energy.push_back(sqlite3_column_int(stmt,1));
				}
				day_pulses += sqlite3_column_int(stmt,1);
				++day_n;
			}
			
			++i;
		}	
		
	}
	
	sqlite3_finalize(stmt);
}

void Statistics::printOut(int g) {
	
	// =======================================================================
	// lasketaan TEHOJEN keskiarvo, mediaani, keskihajonta
	// =======================================================================
	
	double power_mean = 0;
	double power_day_mean = 0;
	double power_night_mean = 0;
	
	double power_sigma = 0;
	double power_day_sigma = 0;
	double power_night_sigma = 0;
	
	double power_median = 0;
	double power_day_median = 0;
	double power_night_median = 0;
	
	double day_sum_of_squares = 0;
	double night_sum_of_squares = 0;
	
	if ( night_n > 0) {
	
		//KESKIARVO
		power_night_mean = (night_pulses/Graphs[g].meterConst*60000)/night_n;
		
		if (dateArr.size() <= CALCULATION_LIMIT) {
		
			for (unsigned int i=0; i < night_n; ++i) {			
				night_sum_of_squares += pow(night_energy[i]/Graphs[g].meterConst*60000, 2);
			}
			
			//KESKIHAJONTA
			double N = night_n;
			power_night_sigma = sqrt(night_sum_of_squares/N - pow(power_night_mean,2));
			
			//MEDIAANI
			sort(night_energy.begin(), night_energy.end());
			power_night_median = night_energy[night_energy.size()/2]/Graphs[g].meterConst*60000;
		
		}
	}
	
	if ( day_n > 0 ) {
	
		//KESKIARVO
		power_day_mean = (day_pulses/Graphs[g].meterConst*60000)/day_n;
		
		if (dateArr.size() <= CALCULATION_LIMIT) {
		
			for (unsigned int i=0; i < day_n; ++i) {	
				day_sum_of_squares += pow(day_energy[i]/Graphs[g].meterConst*60000, 2);
			}
			
			//KESKIHAJONTA
			double N = day_n;
			power_day_sigma = sqrt(day_sum_of_squares/N - pow(power_day_mean,2));
			
			//MEDIAANI
			sort(day_energy.begin(), day_energy.end());
			power_day_median = day_energy[day_energy.size()/2]/Graphs[g].meterConst*60000;
		
		}
	}
	
	if (day_n > 0 || night_n > 0) { //jos kertynyt pulsseja
	
		//KESKIARVO
		power_mean = ((day_pulses + night_pulses)/Graphs[g].meterConst*60000) / ( day_n + night_n );
		
		if ( dateArr.size() <= CALCULATION_LIMIT ) {
		
			//KESKIHAJONTA
			double N = day_n + night_n;
			power_sigma = sqrt( (night_sum_of_squares+day_sum_of_squares)/N - pow(power_mean,2));
			
			//MEDIAANI
			vector<short int> total_energy;
		
			//yhdistetään vektorit
			total_energy.insert(total_energy.end(), night_energy.begin(), night_energy.end());
			total_energy.insert(total_energy.end(), day_energy.begin(), day_energy.end());
			
			sort(total_energy.begin(), total_energy.end());
			power_median = total_energy[total_energy.size()/2]/Graphs[g].meterConst*60000;
			
		}
		
	}

	// =======================================================================
	// Tulostetaan taulukot
	// =======================================================================
	
	printf( "<table id='mytable'>" );
	
	cout << "<caption><H4>[" << dateArr[0].day << "." << dateArr[0].month << "." << dateArr[0].year
		 << "-" << dateArr[dateArr.size()-1].day << "." << dateArr[dateArr.size()-1].month 
		 << "." << dateArr[dateArr.size()-1].year << "]</H4></caption>";
	
	printf("<tr>" );
	printf( "<th class='nobg'></th>");
	printf( "<th>ENERGY</th>");
	printf( "<th>MEAN (<span style='text-decoration:overline'>x</span>) </th>");
	if ( dateArr.size() <= CALCULATION_LIMIT ) {
		printf( "<th>MEDIAN</th>");
		printf( "<th>STD (&#963)</th>");
	}
	printf( "<th >COSTS</th>");
	printf( "</tr>");
	
	printf( "<tr>");
	printf( "<th class='spec'>DAY</th>");
	printf("<td>%0.3f kWh</td>",day_pulses/Graphs[g].meterConst);
	printf("<td>%0.0f W</td>", power_day_mean);
	if ( dateArr.size() <= CALCULATION_LIMIT ) {
		printf("<td>%0.0f W</td>", power_day_median);
		printf("<td>%0.0f W</td>", power_day_sigma);
	}	
	printf("<td>%0.2f eur [%0.2f cent/kWh]</td>", day_pulses*Conf.power_day_price/Graphs[g].meterConst*0.01, Conf.power_day_price);
	printf( "</tr>");
	
	printf( "<tr>");
	printf( "<th class='spec'>NIGHT</b></th>");
	printf("<td>%0.3f kWh</td>",night_pulses/Graphs[g].meterConst);
	printf("<td>%0.0f W</td>", power_night_mean);
	if ( dateArr.size() <= CALCULATION_LIMIT ) {
		printf("<td>%0.0f W</td>", power_night_median);
		printf("<td>%0.0f W</td>", power_night_sigma); 	
	}
	printf("<td>%0.2f eur [%0.2f cent/kWh]</td>", night_pulses*Conf.power_night_price/Graphs[g].meterConst*0.01, Conf.power_night_price);
	printf( "</tr>");
	
	printf( "<tr>");
	printf( "<th class='specalt'>TOTAL</th>");
	printf("<td class='alt'>%0.3f kWh</td>",(day_pulses + night_pulses)/Graphs[g].meterConst);
	printf("<td class='alt'>%0.0f W</td>", power_mean);
	if ( dateArr.size() <= CALCULATION_LIMIT ) {
		printf("<td class='alt'>%0.0f W</td>", power_median);
		printf("<td class='alt'>%0.0f W</td>", power_sigma);
	}	
	printf("<td class='alt'>%0.2f eur</td>", (day_pulses * Conf.power_day_price/Graphs[g].meterConst*0.01 + night_pulses * Conf.power_night_price/Graphs[g].meterConst*0.01 ));
	printf( "</tr>" );
	
	printf( "</table>" );
	
}

