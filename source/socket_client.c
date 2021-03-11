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
	#include <sys/un.h>
	#include <stdio.h>
	#include <time.h>
	#include <sys/socket.h>
}

using namespace std;

#include "boxfunctions.h"


class Socket {
	public:
		//rakentaja
		Socket();
		
		//purkaja
		~Socket();	
		
	protected:
		void openSocket();
		void closeSocket();
		
		//SOCKET VARIABLES
		int s;
		socklen_t len; //Socket address length type
    
		FILE* sockin;
		FILE* sockout;
		struct sockaddr_un remote;

};


//peritään Socket luokka
class Live : public Socket {
	public:
		//rakentaja
		Live();
		
		void Cgi();
		void runQuery();
		
	private:
		void type1();
		void type2();
		void type3();
		void type4();
		
		CONFIG_DATA Conf; //Määritetään asetusmuuttujien tietue
		
		time_t nyt; //SEKUNTI laskuri reaaliajan lukemiseen
		struct tm* aika; //päivämäärätietue
		
		// ilmaisee haluttua toimintoa
		int type;
		
		// haluttu sensori
		int deviceId ;
};


int main() {

	// Tarkistus, löytyykö CGI string
	if (getenv("QUERY_STRING") != NULL) { 
		
		// luodaan olio
		Live live;
		
		// CGI-alustus
		live.Cgi();
		
		// haetaan data
		live.runQuery();
		
		return EXIT_SUCCESS;
		
	} else {
		cerr << "No QUERY_STRING" << endl;	
		return EXIT_FAILURE;
	}
}


//rakentaja
Socket::Socket() : s(0), len(0){
	openSocket();
}

Socket::~Socket() {
	closeSocket();
}

void Socket::openSocket() {

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        cerr << "socket error" << endl;
        // throw error
    }

    // Trying to connect
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        cerr << "connect error" << endl;
        // throw error
    }

    // Connected
    
    //We'll use stdio for reading the socket
	sockout = fdopen(s, "w");
	sockin  = fdopen(s, "r");
}

void Socket::closeSocket() {

	fclose(sockin);	 //muuten tulee muistivuoto
	fclose(sockout); //muuten tulee muistivuoto
		
	close(s); 		 //suljetaan socket
}

//rakentaja
Live::Live() : type(0), deviceId(0) {

	//luetaan asetukset
	if (read_config_file(Conf) == false) { 
		//throw exception 
	}
}

void Live::Cgi() {
	
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
		} else if (name == "type") {
			type = str2int(value);
		} else if (name == "si") {
			session_id = value;
		} else if (name == "deviceId") {
			deviceId = str2int(value);
		}
		
	}
	
	// testataan löytyykö sessiontiedosto
	if ( test_session(session_id.c_str()) == false) {
		cerr << "SI error" << endl; //ei löytynyt /tmp/sess_tiedostoa
		// throw exception
	}
	
}

void Live::runQuery() {
	
	//HTML HEADER
	cout << "Content-type:text/html;charset=UTF-8\r\n\r\n" << endl;
	
	if (type == 1) {
		type1();
	} else if (type == 2 || type == 22) {
		type2();
	} else if (type == 3) {
		type3();
	} else if (type == 4) {
		type4();
	}
}

void Live::type1() {
	
	char line[80]; //Puskuri fgets funktiolle tulevasta datasta
	
	int power = 0;
	int pulses_today = 0;
	int meterConst = 0;
	
	double temp_out = getSensorValue(deviceId);
	
	//ilmoitetaan, että halutaan power_live
	fprintf(sockout, "type=%d&deviceId=%d\n", type, deviceId); 
	fflush(sockout);
	
	fgets( line, 80, sockin); //luetaan ensimmäinen rivi
	sscanf( line, "%d,%d,%d", &power, &pulses_today, &meterConst); //skannataan bufferi-rivi	
	
	int mean_power_today = 0;
	double seconds_today = 0;
	double kwh_today = 0.0;
	double cost_current = 0.0;
	double power_price_now = 0.0;
	
	//Luetaan sekunnin vuodesta 1970
	time(&nyt);
	
	//Muutetaan kuluneet sekunnit tm tietueen kenttiin sopiviin muotoihin			
	aika = localtime(&nyt);
	
	int hour = aika->tm_hour; //luetaan nykyinen tunti
	
	if (hour >= Conf.power_night_start || hour < Conf.power_day_start ) {	//yösähkö
		cost_current = power*Conf.power_night_price*0.00001;
		power_price_now = Conf.power_night_price;		 
	} else {	//päiväsähkö
		cost_current = power*Conf.power_day_price*0.00001;
		power_price_now = Conf.power_day_price;
	}	
	
	if ( pulses_today != 0 ) { //jos kertynyt pulsseja tälle päivälle
	
		seconds_today = (aika->tm_hour)*3600+(aika->tm_min)*60+(aika->tm_sec);
		kwh_today = pulses_today/static_cast<double>(meterConst); //lasketaan päiväkulutusta vastaava kWh arvo, mittarin pulssimäärätiedon avulla		
		mean_power_today = static_cast<int>(kwh_today*3600/seconds_today*1000);
		
	}
	
	////////////TEMPERATURE
	
	printf ("<table id='mytable'><tr>");
	
	printf ("<th class='nobg'>POWER</th><td class='alt2'>%i W</td>", power);
	
	printf ("</tr><tr>");
	
	printf ("<th>MEAN POWER TODAY</th><td>%i W</td>", mean_power_today);
	
	printf ("</tr><tr>");
	
	printf ("<th>CONSUMPTION TODAY</th><td >%0.3f kWh</td>", kwh_today);
	
	printf ("</tr><tr>");
	
	printf ("<th><b>CURRENT COSTS<b></th><td ><b>%0.2f eur/h</b> [%0.2f cent/kWh]</td>", cost_current, power_price_now);
	
	printf ("</tr><tr>");
	
	printf ("<th><b>OUTDOOR TEMP</th><td><b>%0.1lf &deg;C</td>", temp_out);
	
	printf ("</tr></table>");

}

void Live::type2() {
	
	char line[80]; //Puskuri fgets funktiolle tulevasta datasta
	
	//ilmoitetaan, että halutaan power_graph
	fprintf(sockout, "type=%d&deviceId=%d\n", type, deviceId); 
	fflush(sockout);	
	
	//Luetaan sekunnin vuodesta 1970		
	time(&nyt);

	//Muutetaan kuluneet sekunnit tm tietueen kenttiin sopiviin muotoihin			
	aika = localtime(&nyt);
		
	int epoch = 0;
	int power = 0;
	int epoch_off = aika->tm_gmtoff; // time zone in seconds from GMT; EST=-18000, WET=3600, automatic DST (DAY LIGHT SAVING)
		
	while (fgets(line, 80, sockin) != NULL) {
		
		//scan  line
		sscanf( line, "%d,%d", &epoch, &power ); //skannataan bufferi-rivi ja tallennetaan muuttujien arvot
							
		printf("%d000,%d,", epoch + epoch_off, power); //000 javascript epoch format
	}

}


void Live::type3() {

	bool isvalid = false;
	complex<double> z; // kompleksiluku
	ComplexVector data; // sisältää FFT muunnettavan datan
	
	vector<double> raw_data;
	vector<double> hann_window;
	
	do {
		
		char line[80]; //Puskuri fgets funktiolle tulevasta datasta
		
		//ilmoitetaan, että halutaan FFT data
		fprintf(sockout, "type=%d&deviceId=%d\n", type, deviceId); 
		fflush(sockout);
		
		data.clear(); //tyhjennetään
		raw_data.clear();
		hann_window.clear();
		
		double power = 0; // tämä tulee socketin kautta

		while(fgets(line, 80, sockin) != NULL) {
			
			//skannataan bufferi-rivi ja tallennetaan muuttujien arvot
			sscanf( line, "%lf", &power ); 
			raw_data.push_back(power);
		}
		
		// ikkunointi Hann ikkunalla
		/*for (unsigned int n = 0; n < raw_data.size(); ++n ) {
			raw_data[n] *= 0.5*(1 - cos(2.0*pi*n/(static_cast<double>(raw_data.size() - 1.0))));
		}*/
		
		// muunnos complex vectoriin
		for (unsigned int n = 0; n < raw_data.size(); ++n) {
			z = raw_data[n]; //tekee implisiittisen tyyppimuunnoksen
			data.push_back(z);
		}
		
		// ---------------------------------------------------------------
		
		// Lasketaan montako bittiä annetussa luvussa on päällä.

		int PowersOfTwo = 0;

		for (unsigned int k = 0; k < 8 * sizeof( data.size() ); ++k)  {
			PowersOfTwo += ( data.size() >> k) & 1;
		}

		// Jos luvussa on vain yksi bitti päällä, 
		// se on kakkosen potenssi.
		
		if (PowersOfTwo == 1) {  // onnistunut socket pyyntö
			isvalid = true;
		} else {			
			isvalid = false;     // tapahtunut virhe socketin tiedonsiirrossa
								 // ei ole siirtynyt kakkosen potensseina
		}
		
	} while (isvalid == false);
	
	// Zero Padding --> lisätään nollia aikatason signaalin perään 
	//  --> interpolointi taajuustasossa
	
	z = 0; //kompleksiluku
	
	unsigned int orig_size = data.size();
	unsigned int L = 1; //interpolointi kerroin
	
	while (data.size() != L*orig_size) {
		data.push_back(z);
	}

	// =======================================================================
		
	ComplexVector result; // sisältää FFT muunnetun datan
	
	// Lasketaan annetun vektorin Fourier-muunnos.
	result = FFT(data);
	
	// =======================================================================
	
	//tulostetaan taajuudet
	double Fs = 1.0; 	      // näytteenottotaajuus (Hz)
	double N = result.size(); // length of FFT
	double Fo = Fs/N;		  // frequency resolution
	double X = 0; 		      // x-akseli (taajuus)
	
	// nollataajuus (DC-mean)
	printf("%0.1f,%0.1f,", 0.0, abs(result[0])*L / N );
	
	// loput taajuudet
	for (unsigned int i = 1; i < (result.size() / 2 + 1); ++i) {
	
		X += Fo; // liikutaan oikealle x-akselilla
		
		// abs(result[i])*2 / N <-- kahdella kertominen, jotta
		// negatiivisten taajuuksien amplitudit tulee summattua
		// kerrotaan 1000:lla --> milliHz
		printf("%0.1f,%0.1f,", X*1000, abs(result[i])*2/ (L*N) );
		
	}
	
}


void Live::type4() {
	
	char line[80]; //Puskuri fgets funktiolle tulevasta datasta
	
	int epoch = 0;
	int delta_value = 0;

	//ilmoitetaan, että halutaan power_live
	fprintf(sockout, "type=%d&deviceId=%d\n", type, deviceId); 
	fflush(sockout);
	
	fgets( line, 80, sockin); //luetaan ensimmäinen rivi
	sscanf( line, "%d,%d", &epoch, &delta_value); //skannataan bufferi-rivi	

	// -------------------------------------------------------------------
	
	// clusterit
	vector<cluster> clust;
	
	// laitteet
	vector<string> deviceNames;
	
	// -------------------------------------------------------------------
	
	sqlite3* nialm_db;
	sqlite3_stmt* stmt = NULL; 
	
	// Avataan tietokanta
	if (openDatabase(NIALM_DB_FILE, &nialm_db, 30000) == false) {
		#ifdef DEBUG
		cout << "openDatabase NIALM_DB error " << endl;
		throw "error";
		#endif
	}
    
	//HAETAAN PORTTI TIEDOT
	char sql_read_devices[] = "SELECT deviceName FROM devices";
	    
	//valmistellaan käsky
	int rc = sqlite3_prepare_v2(nialm_db, sql_read_devices, strlen(sql_read_devices), &stmt, NULL);
	
	if( rc != SQLITE_OK ){
			
			#ifdef DEBUG	
			fprintf(stderr, "SQL error in preparing devices table\n");
			#endif
					
	} else {
		
		while (sqlite3_step(stmt) == SQLITE_ROW) { //suoritetaan käsky
			
			char deviceName[50];
			sprintf(deviceName, "%s", sqlite3_column_text(stmt,0) );
			
			string nameStr(deviceName); // alustetaan rakentaja c-merkkijonolla
			deviceNames.push_back(nameStr);
			
		} //while ends
			
	} //else ends
	
	sqlite3_finalize(stmt);

	// --------------------------------------------------------------------
	
	//HAETAAN PORTTI TIEDOT
	char sql_read_clusters[] = "SELECT dP, dP_STD, prob, period, deviceId, N FROM clusters";
	
	//valmistellaan käsky
	rc = sqlite3_prepare_v2(nialm_db, sql_read_clusters, strlen(sql_read_clusters), &stmt, NULL);
	
	if( rc != SQLITE_OK ){			
			#ifdef DEBUG	
			fprintf(stderr, "SQL error in preparing clusters data\n");
			#endif
	} else {		
		while (sqlite3_step(stmt) == SQLITE_ROW) { //suoritetaan käsky
			//haetaan arvot
	    	double dP = sqlite3_column_double(stmt,0);			
	    	double dP_STD = sqlite3_column_double(stmt,1);			
			double prob = sqlite3_column_double(stmt,2);
			int period = sqlite3_column_int(stmt,3);
			int deviceId = sqlite3_column_int(stmt,4);
			int N = sqlite3_column_int(stmt,5);
			
			// lisätään kyseinen rivi vectoriin
			clust.push_back(cluster(dP, dP_STD, prob, period, deviceId, N));
		}			
	}
	
	sqlite3_finalize(stmt);
	
	// Suljetaan tietokanta
	closeDatabase(&nialm_db);

	// ------------------------------------------------------------
	if (clust.size() > 0 && deviceNames.size() > 0 && delta_value != 0) {
	
		double prob = 0.0; // todennäköisyys	
		int clusterId = bayes(static_cast<double>(delta_value), clust, prob, true);

		string deviceName;
		
		if (clust[clusterId].deviceId == -1) { // tuntematon laite kyseessä
			deviceName = "non-mapped";
		} else {
			deviceName = deviceNames[clust[clusterId].deviceId];
		}
		
		printf("Device Recognition: %dW | %s | Probability: %0.0f%%", delta_value, deviceName.c_str(), prob*100 - 1);
	
	} else {
	
		printf("Not enough data for recognition!");
		
	}
	
}




