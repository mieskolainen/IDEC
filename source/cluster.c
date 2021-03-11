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

// ------------------------------------------------------------------
// NIALM
// 
// HUOM!! c[i].prob on todennäköisyys positiivisille ja neg. erikseen
// ------------------------------------------------------------------

#include <vector>
#include <string>
#include <complex>
#include <iostream>
#include <cmath>

extern "C" {
	#include <stdio.h>
	#include <signal.h>
	#include <sys/time.h>
	#include <fcntl.h>
	#include <time.h>
	#include <string.h>
}

#include "sqlite3.h"

using namespace std;

#include "boxfunctions.h"


struct edge {
	int dP;     // teho
	int power;
	int epoch;  // aikaleima
	bool s;     // onko löydetty aiemmin
	int N;      // mihin klusteriin kuuluu
	
	// konstruktori
	edge(int dP_init, int epoch_init, int power_init, int s_init, int N_init) : 
	     dP(dP_init), epoch(epoch_init), power(power_init), s(s_init), N(N_init) {} 
};

struct match {
	int period; // aikaleima
	int N;      // mihin klusteriin kuuluu
	
	// konstruktori
	match(int period_init, int N_init) : period(period_init), N(N_init) {} 
};

struct device {
	string name;
	double energy;
	
	// konstruktori
	device(string name_init) : name(name_init), energy(0.0) {} 
};


//pienin teho, joka noteerataan (W)
const int PIENIN_TEHO = 42;

//suurin teho, joka noteerataan (W)
const int SUURIN_TEHO = 30000;

//TARKKA laskuri tietue
struct timeval timer;

string start_date;
string end_date;
int days;

// Funktioiden prototyypit

bool comparecluster(const cluster& a, const cluster& b);

void load_data(vector<edge>& data, vector<edge>& data_pos, vector<edge>& data_neg);

void genetic(vector<vector<int> >& devices, vector<cluster>& clust);

void Mutate(vector<vector<int> >& c, vector<double>& Q);

void Cross(vector<vector<int> >& c, vector<double>& Q);

double meanQuality(vector<double>& Q);

double bestQuality(vector<double>& Q);

void quality(vector<int>& viite, vector<cluster>& clust, vector<vector<int> >& c, vector<double>& Q);

void c_means(vector<edge>& x, vector<cluster>& c, int C);

bool c_meansEigen(vector<edge>& x, vector<cluster>& c);

void fuzzy_c_means(vector<edge>& x, vector<cluster>& c, int C);

bool fuzzy_c_meansEigen(vector<edge>& x, vector<cluster>& c, vector<vector<double> >& U);

void em(vector<edge>& x, vector<cluster>& c, int C);

void savetodb(vector<vector<int> >& devices, vector<cluster>& clust, vector<vector<int> >& A);

int nearest(int dP, const vector<cluster>& clust);

void createTransMat(vector<vector<int> >& A, vector<edge>& raw, vector<cluster>& cluster_pos, vector<cluster>& cluster_neg);

void calcEnergy(vector<edge>& raw);

void calcPeriods(vector<edge>& edges, vector<cluster>& clust, int type);

double dB(vector<cluster>& clust, int n);

// DEFINE LOCK FILE
#define LOCK_FILE "/var/run/cluster.pid"


int main(int argc, char* argv[]) {
	
	// Tarkistetaan onko annettu parametreja
	if (argc != 5) {
		cerr << "Error: too little parameters" << endl;
		return EXIT_FAILURE;
	}
	
	// Ajoparametri
	string cmd_param(argv[1]);
	string cluster_type(argv[2]);
	string cluster_count(argv[3]);
	string cluster_start(argv[4]);
	
	// muutetaan parametria
	start_date = cluster_start;
	
	if (cluster_type != "soft" && cluster_type != "hard" && cluster_type != "em") {
		cerr << "Error: unknown clustering type" << endl;
		return EXIT_FAILURE;
	}
	
	// -------------------------------------------------------------------
	// Järjestelmäprosessin alustuskäskyt
	{
		pid_t id;
		id = fork();
		
		// FORK 1
		if (id < 0) return EXIT_FAILURE; // fork error 
		if (id > 0) return EXIT_FAILURE; // parent exits 
		
		// CHILD (daemon) continues -->
		
		// Obtain a new process group
		if (setsid() < 0) return EXIT_FAILURE; 
		
		// FORK 2
		if (id < 0) return EXIT_FAILURE; // fork error 
		if (id > 0) return EXIT_FAILURE; // parent exits
		
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
		
		if (lfp < 0) return EXIT_FAILURE; // can not open
		
		if (lockf(lfp,F_TLOCK,0) < 0) return EXIT_FAILURE; // can not lock	
		// --> there is already instance running
		
		sprintf(str, "%d\n", getpid());
		write(lfp, str,strlen(str)); // record pid to lockfile
		
	}
	
   	gettimeofday(&timer, NULL);   //haetaan päälaskuri
	srand( timer.tv_usec );       //sekoitetaan pseudorandomgeneraattori
	
	// -------------------------------------------------------------------
	// ladataan kerätty raakadata
	// -------------------------------------------------------------------
	vector<edge> raw;
	vector<edge> raw_pos;
	vector<edge> raw_neg; 
	
	cout << "Loading data..." << endl;
	load_data(raw, raw_pos, raw_neg);
	
	if (cmd_param == "--init") {
	
		// -------------------------------------------------------------------
		// laitteiden klusterointi
		// -------------------------------------------------------------------
		vector<cluster> cluster_pos;
		vector<cluster> cluster_neg;
		
		// Klustereiden määrä
		int C = str2int(cluster_count);
		
		cout << "Clustering " << cluster_type << " ..." << endl;
		
		if (cluster_type == "soft") {
			fuzzy_c_means(raw_pos, cluster_pos, C);
			fuzzy_c_means(raw_neg, cluster_neg, C);
			
		} else if (cluster_type == "hard") {
			c_means(raw_pos, cluster_pos, C);
			c_means(raw_neg, cluster_neg, C);
			
		} else if (cluster_type == "em") {
			em(raw_pos, cluster_pos, C);
			em(raw_neg, cluster_neg, C);
			
		}
		
		// -------------------------------------------------------------------
		// Keskimääräisten jaksonaikojen haku
		// -------------------------------------------------------------------
		cout << "Calculating periods..." << endl;
		calcPeriods(raw_pos, cluster_pos, 1);
		calcPeriods(raw_neg, cluster_neg, -1);
		
		// -------------------------------------------------------------------
		// luodaan Nc X Nc siirtymätodennäköisyysmatriisi A
		// -------------------------------------------------------------------
		vector<vector<int> > A(cluster_pos.size() + cluster_neg.size(), 
					vector<int>(cluster_pos.size() + cluster_neg.size(),0));
		
		//cout << "Creating transition matrix A..." << endl;
		/*createTransMat(A, raw, cluster_pos, cluster_neg);
		
		
		// Tulostetaan matriisi
		for (unsigned int j = 0; j < A[0].size(); ++j) {
			cout << j << "  ";
		}
		for (unsigned int i = 0; i < A.size(); ++i) {
			cout << endl << i << "  ";
			for (unsigned int j = 0; j < A[i].size(); ++j) {
				cout << A[i][j] << "  ";
			}
		}*/
		
		// -------------------------------------------------------------------
		// Geneettinen algoritmi laitteiden luontiin
		// -------------------------------------------------------------------
		vector<vector<int> > devices;
		
		// yhdistetään vektorit
		cluster_pos.insert(cluster_pos.end(), cluster_neg.begin(), cluster_neg.end());
		
		cout << "Starting genetic algorithm..." << endl;
		genetic(devices, cluster_pos);
		
		// Tallennetaan tulokset tietokantaan
		savetodb(devices, cluster_pos, A);
		
	}
	
	if (cmd_param == "--init" || cmd_param == "--energy") {
	
		// ------------------------------------------------------------
		// Energy Consumption calculation
		// ------------------------------------------------------------	
		cout << "Energy consumption calculation..." << endl;
		calcEnergy(raw);
	}
	
	return EXIT_SUCCESS;
}


double dB(vector<cluster>& c, int n) {

	// Davies-Boulding laatuindeksi
	// DB_i = max(DB_ij) missä j=1...n
	vector<double> DB_i(n, 0.0);
	
	// DB_ij = {sc(C_i) + sc(C_j)}/cd(C_i,C_j)
	
	// where  sc(x): Average distance of samples (belonging to x cluster)
	//              to center of x cluster
	//
	// and    cd(x,y): The distance between the centers of x and y clusters.
	
	// The average value of the cluster indices is defined as D-B index
	// of all clusters
	// DB = 1/n*sum(DB_i) missä i=1...n
	
	for (int i = 0; i < n; ++i) {
		
		vector<double> DB_ij;
		
		for (int j = 0; j < n; ++j) {
			if (i != j) {
				double scCi = c[i].dP_STD;			
				double scCj = c[j].dP_STD;
				double cdCiCj = abs(c[i].dP - c[j].dP);
				
				DB_ij.push_back((scCi + scCj)/cdCiCj);
			}
		}
		// Find max
		double max = 0.0;
		for (int x = 0; x < DB_ij.size(); ++x) {
			if (DB_ij[x] > max) {
				max = DB_ij[x];
			}
		}
		// D-B index for the ith cluster
		DB_i[i] = max;
	}
	
	double DB_i_sum = 0.0;	
	for (int i = 0; i < n; ++i) {
		DB_i_sum += DB_i[i];
	}
	
	return (DB_i_sum / static_cast<double>(n)); // paluuarvo	
}


void calcPeriods(vector<edge>& edges, vector<cluster>& clust, int type) {
	
	//matchatyt vektorit
	vector<match> matched;
	
	for (int i = 0; i < edges.size() - 1; ++i) { // käydään läpi kaikki arvot	
		for (int j = i + 1; j < edges.size(); ++j) {
			if ( (edges[j].N == edges[i].N) && (edges[j].s == false) ) {
				matched.push_back(match(edges[j].epoch - edges[i].epoch, edges[i].N));
				edges[j].s = true; //leimataan
				break;
			}
		}
	}
	
	// Keskiarvo
	for (int i = 0; i < clust.size(); ++i) {
				
		int sum = 0;
		int k = 0;
		
		for (int j = 0; j < matched.size(); ++j) {
			if (matched[j].N == i) {
				sum += matched[j].period;
				++k;
			}
		}
		
		if (k > 0) { // type on positiivinen nousevilla reunoilla, negatiivinen laskevilla
			clust[i].period = static_cast<int>(sum/(static_cast<double>(k)))*type;
		}
	}
}


void calcEnergy(vector<edge>& raw) {

	// clusterit
	vector<cluster> clust;
	
	// laitteet
	vector<device> devices;
	
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
			devices.push_back(nameStr);
		}	
	}
	
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


	// Energy calculation
	// ------------------------------------------------------------
	vector<int> series;
	
	double prob = 0.0; // todennäköisyys (ei tehdä mitään)
	
	for (int i = 0; i < raw.size(); ++i) {
		cout << "State: " << i << "/" << raw.size() << endl;
		series.push_back(bayes(raw[i].dP, clust, prob, false)); // haetaan todennäköisin laite
	}
	
	for (int round = 0; round < 3; ++round ) { // Käydään data useasti läpi
		
		for (int i = 0; i < raw.size() - 1; ++i) {
			
			cout << "State X: " << i << "/" << raw.size() << endl;
			
			if (raw[i].dP > 0 && raw[i].s == false) { // positiivinen reuna
			
				if (series[i] == -1) { // tuntematon laite kyseessä			
					// do nothing
				} else {	
					// xx aikaraja haussa
					for (int j = i + 1; j < raw.size() && raw[j].epoch - raw[i].epoch < clust[series[i]].period*2; ++j) { // etsitään vastaava negatiivinen reuna
						
						if (raw[j].dP < 0 && raw[j].s == false && clust[series[i]].deviceId == clust[series[j]].deviceId) {

							devices[clust[series[i]].deviceId].energy += abs(raw[j].dP)*(raw[j].epoch - raw[i].epoch);
							raw[j].s = true; // Leimataan käytetyksi
							raw[i].s = true;
							
							break; // lopetetaan tämä looppi
						} else if (raw[j].dP > 0 && raw[j].s == false && clust[series[i]].deviceId == clust[series[j]].deviceId) { // löytyi vastaava positiivinen reuna uudelleen
							break;
						}
					}
				}
			}
		}
	}
	
	// -------------------------------------------------------------------
	// printataan statistiikka
	// -------------------------------------------------------------------
	
	char print_file[] = "/tmp/cluster.data";
	
	FILE* fp;
	
	fp = fopen(print_file, "w");
	
	for (int i = 0; i < devices.size(); ++i) { // taulukon rivit
		fprintf(fp, "%0.2f\n", devices[i].energy/3600000.0); // laitteen energia
	}
	
	vector<int> lowest;
	
	for (int round = 0; round < 20; ++round) {
		
		int min_power = 100000000;
		int min_i = 0;
		
		for (int i = 0; i < raw.size(); ++i) {
			if (raw[i].power < min_power) {
				min_power = raw[i].power;
				min_i = i;
			}
		}
		
		lowest.push_back(raw[min_i].power);
		raw.erase(raw.begin() + min_i);
	}
	
	int base_power = 0;
	for (int i = 0; i < lowest.size(); ++i) {
		base_power += lowest[i];
	}
	base_power /= lowest.size();
	
	double base_load_energy = (raw[raw.size()-1].epoch - raw[0].epoch)/ 3600.0 * base_power / 1000.0;
	
	fprintf(fp, "%0.2f\n", base_load_energy); // pohjaenergia
	
	fclose(fp); // suljetaan tiedostokahva

}


void createTransMat(vector<vector<int> >& A, vector<edge>& raw,
                    vector<cluster>& cluster_pos, vector<cluster>& cluster_neg) {
	
	// Matrix A of state transition probaliities
	// 
	//         pos_1 pos_2 pos_3 ... neg_1 neg_2 neg_3
	// pos_1 |----------------------------------------| rivisumma = 1
	// pos_2 |                                    	  |
	// pos_3 |									      |
	// .     |									      |
	// .     |									      |
	// neg_1 |									      |
	// neg_2 |									      |
	// neg_3 |									      |
	//       |----------------------------------------|
	
	// Sijoitetaan siirtymät matriisiin
	int M = 0;
	int N = 0;
	
	// ei käytetä varsinaisesti mihinkään
	double prob = 0.0;
	
	// Tässä haetaan ensimmäinen arvo (algoritmin aloitus)
	if (raw[0].dP > 0) { 
		M = bayes(raw[0].dP, cluster_pos, prob, true); // positiivinen arvo
	} else {
		M = bayes(raw[0].dP, cluster_neg, prob, true) + cluster_pos.size(); 
		// + cluster_pos.size() shiftaa negatiiviset positiivisten klustereiden perään
	}

	for (int i = 1; i < raw.size(); ++i) { // käydään läpi kaikki arvot
	
		if (raw[i].dP > 0) { // positiivinen
			N = bayes(raw[i].dP, cluster_pos, prob, true);
		} else { // negatiivinen
			N = bayes(raw[i].dP, cluster_neg, prob, true) + cluster_pos.size();
		}
		
		// Lisätään yksi arvo lisää
		++A.at(M).at(N);
		
		M = N; // nykyinen sarake -> seuraava rivi
	}
	
	// muunnetaan matriisin sisältö prosenteiksi riveittäin
	for (int row = 0; row < A.size(); ++row) {
		
		int rivi_alkiot = 0;
		
		for (int col = 0; col < A[row].size(); ++col) {
			rivi_alkiot += A[row][col];
		}
		
		if (rivi_alkiot > 0) { // jos rivillä alkioita
			for (int col = 0; col < A[row].size(); ++col) {
				A[row][col] = A[row][col]*100/rivi_alkiot;
			}
		}
	}
}


int nearest(int dP, const vector<cluster>& clust) {

	int K_index = 0;
	double min = 0;
	
	for (int k = 0; k < clust.size(); ++k) {
		if (abs(dP - clust[k].dP) < min || k == 0) {
			K_index = k;
			min = abs(dP - clust[k].dP);
		}
	}
	
	return K_index; // palautetaan lähin klusteri
}


bool comparecluster(const cluster& a, const cluster& b) {
	if (a.N >= b.N) {
		return true;
	} else {
		return false;
	}
}


void genetic(vector<vector<int> >& devices, vector<cluster>& clust) {
	
	// lisätään pari MAHDOTONTA LAITETTA algoritmia varten
	clust.push_back(cluster(1e6, 1e6, 1e6, 1000000, -1, 1000000));
	clust.push_back(cluster(1e6, 1e6, 1e6, 1000000, -1, 1000000));
	
	int Nc = clust.size();
	int Nv = Nc/2 + 5; // populaation koko
	
	//luodaan binäärinen kerroinmatriisi c, dim(Nv X Nc)  
	vector<vector<int> > c(Nv, vector<int>(Nc, 0));
	
	//luodaan laatuvektori
	vector<double> Q(Nv, 0);
	
	//viittaus todellisiin klustereiden indekseihin
	vector<int> viite(Nc,0);
	
	for (int i = 0; i < Nc; ++i) {
		viite[i] = i;
	}
	
	//luodaan alkupopulaatio satunnaisesti
	
	for (int j = 0; j < Nv; ++j) { //rivit
		for (int r = 0; r < Nc; ++r) { //sarakkeet
			double x = closed_interval_rand(0.0, 1.0); //haetaan satunnaisesti 0 tai 1
			c[j][r] = static_cast<int>(round(x));
		}
	}
	
	//lasketaan laatu
	quality(viite, clust, c, Q);
	
	//paras vaihtoehto tähän mennessä kerroinmatriisille c on alkupopulaatio
	vector<vector<int> > cBest = c;
	
	//paras laatuvektori (tämän voi toki aina laskea matriisista c)
	vector<double> QBest = Q;
	
	//laaduntarkasteluAPUvektori
	vector<int> Sum(Nv, 0);
	
	// Tätä jatketaan kunnes ehto toteutuu (Nc > xx)
	for (int rounds = 0; Nc > 2; ++rounds) {
		
		// Cross/Mutate
		rounds % 2 == 0 ? Cross(c, Q) : Mutate(c, Q);
		
		//lasketaan uusi laatu
		quality(viite, clust, c, Q);
		
		//lasketaan onko samoja rivejä (vertaillaan tehojen summia)
		int max_same_rows = 2;
		int same_rows = 0;
		
		for (int i = 0; i < Nv; ++i) {	
			for (int j = 0; j < Nv; ++j) {
				if (c[i] == c[j] && i != j) {
					++same_rows;  //löydettiin vastaava
				}
			}
		}
		
		same_rows /= 2;
		
		//Pyritään minimoimaan keskiarvoa tai parasta yksilöä
		if ( (meanQuality(Q) < meanQuality(QBest) || 
		      bestQuality(Q) < bestQuality(QBest)) && same_rows <= max_same_rows ) {
			
			cBest = c;
			QBest = Q;
			cout << "Round: " << rounds << ", Q: " << meanQuality(QBest) << endl;
			
		} else { //Palautetaan vanha ratkaisu		
			c = cBest;
			Q = QBest;
		}
		
		if (rounds % 1000 == 0) { // tulostetaan tiedot
			/* 
			cout << endl;
			
			//tulostetaan kaikki clusterit
			for (int r = 0; r < clust.size(); ++r) {				
				cout << "C" << r << "\t Power: " << clust[r].dP << "\t Period: " << clust[r].period
					 << "\t Members: " << clust[r].N << endl;
			}
			
			cout << endl;
			
			// tulostetaan ylärivi kerroinmatriisille c
			for (int r = 0; r < Nc; ++r) {
				if (viite[r] < 10) {
					cout << "0" << viite[r] << " ";
				} else {
					cout << viite[r] << " ";
				}
			}
			
			cout << endl;
			
			//tulostetaan matriisi
			for (int j = 0; j < Nv; ++j) {		
				for (int r = 0; r < Nc; ++r) {
					cout << c[j][r] << "  ";
				}
				
				double rowsum = 0;
				for (int r = 0; r < Nc; ++r) {
					rowsum += c[j][r]*clust[viite[r]].dP;
				}
				cout << " Power sum: " << rowsum << " W" << ", Q: " << Q[j] << endl;
			}
			
			cout << endl;
			
			//tulostetaan löydetyt laitteet
			for (int i = 0; i < devices.size(); ++i) {	
				cout << "Device[" << i << "]: ";
				for (int d = 0; d < devices[i].size(); ++d) {
					cout << devices[i][d] << " ";
				}
				cout << endl;
			}
			
			cout << endl;
			*/
		}
		
		if (rounds == 20000) { // nyt poistetaan paras laite
			
			double min = 1e6;
			int best = 0;
			
			for (int i = 0; i < Q.size(); ++i) { // haetaan paras laite
				if (Q[i] < min) {
					min = Q[i];
					best = i;
				}
			}
			
			// lisätään laitevektoriin uusi laite (sen indeksit)
			vector<int> device;
			
			// uusi viitevectori
			vector<int> newviite;
			
			for (int i = 0; i < Nc; ++i) {
				if (c[best][i] == 1) {
					device.push_back(viite[i]);   // tämä indeksi kuuluu uuteen laitteeseen
				} else {
					newviite.push_back(viite[i]); // tämä ei kuulu uuteen laitteeseen
				}
			}
			
			Nc -= viite.size() - newviite.size(); // poistetaan kyseinen määrä tiloja		
			viite = newviite; // sijoitetaan uusi viitevectori
			devices.push_back(device); // lisätään laite
			
			//nollataan kerroinmatriisi
			c.clear();
			
			//lisätään uudet rivivektorit
			for (int i = 0; i < Nv; ++i) {
				c.push_back(vector<int>(Nc,0));
			}
			
			//luodaan alkupopulaatio satunnaisesti
			
			for (int j = 0; j < Nv; ++j) { //rivit
				for (int r = 0; r < Nc; ++r) { //sarakkeet
					double x = closed_interval_rand(0.0, 1.0); //haetaan satunnaisesti 0 tai 1
					c[j][r] = static_cast<int>(round(x));
				}
			}
			
			//paras vaihtoehto tähän mennessä kerroinmatriisille c on alkupopulaatio
			cBest = c;
			
			//lasketaan laatu
			quality(viite, clust, c, Q);
			
			//laatu tähän mennessä
			QBest = Q;
			
			rounds = -1;
		}
	}
	
	//poistetaan mahdottomat laitteet
	clust.pop_back();
	clust.pop_back();
}


void Mutate(vector<vector<int> >& c, vector<double>& Q) {
	
	// Matriisin c
	int Nc = c[0].size(); // sarakkeet
	int Nv = c.size();    // rivit
		
	//Etsitään x suurinta arvoa (heikointa yksilöä)
	int x = static_cast<int>(Nc/2.0);
	
	//Suurimmat arvot (heikoimmat yksilöt)
	vector<int> max (x, 0);
	double previousVal = 10000000.0;
	
	for (int i = 0; i < max.size(); ++i) {
	
		double maxVal = 0.0;
		
		for (int j = 0; j < Nv; ++j ) {
			if (Q[j] > maxVal && Q[j] < previousVal) {
				max[i] = j;    // binaarimatriisin rivi j
				maxVal = Q[j]; // suurin löydetty
			}
		}
		previousVal = maxVal;
	}
	
	//Yhden alleelin MUTAATIO vain heikoille kromosomeille
	for (int j = 0; j < max.size(); ++j) {
	
		//todennäköisyys
		double mutationProb = 0.03;
		
		if (closed_interval_rand(0.0, 1.0) < mutationProb) {
			int randcolumn = rand() % (Nc - 1);
			c[max[j]][randcolumn] == 1 ? c[max[j]][randcolumn] = 0 : c[max[j]][randcolumn] = 1;
		}
	}
}


void Cross(vector<vector<int> >& c, vector<double>& Q) {
	
	// Matriisin c
	int Nc = c[0].size(); // sarakkeet
	int Nv = c.size();    // rivit
	
	//Etsitään x suurinta arvoa (heikointa yksilöä)
	int x = static_cast<int>(Nc/2.0);
	
	//Suurimmat arvot (heikoimmat yksilöt)
	vector<int> max (x, 0);
	double previousVal = 10000000.0;
	
	for (int i = 0; i < max.size(); ++i) {
	
		double maxVal = 0.0;
		
		for (int j = 0; j < Nv; ++j ) {
			if (Q[j] > maxVal && Q[j] < previousVal) {
				max[i] = j;    // binaarimatriisin rivi j
				maxVal = Q[j]; // suurin löydetty
			}
		}
		previousVal = maxVal;
	}
		
	// Luodaan uusi populaatio risteyttämällä parhaita yksilöitä
	// (siis niitä, jotka eivät kuuluu max vectoriin)
	for (int i = 0; i < max.size(); ++i) {
		
		//haetaan sattumanvaraisesti kaksi vanhempikromosomia
		int parent1 = 0; int parent2 = 0;
		
		bool OK = true;
		
		do {
			OK = true;
			parent1 = rand() % (Nv - 1); //haetaan luku väliltä  0...Nv-1
			
			for (int x = 0; x < max.size(); ++x) {
				if (parent1 == max[x]) { 
					OK = false; // epäkelpo, aloitetaan alusta
					break;
				}
			}
		} while (OK != true); // poistutaan kun ehto on epätosi
		
		do {
			OK = true;
			parent2 = rand() % (Nv - 1); //haetaan luku väliltä  0...Nv-1
			
			for (int x = 0; x < max.size(); ++x) {
				if (parent2 == max[x] || parent2 == parent1) { 
					OK = false; // epäkelpo, aloitetaan alusta
					break;
				}
			}
		} while (OK != true); // poistutaan kun ehto on epätosi
		
		//SATTUMANVARAINEN CROSSOVER kromosomin kahden pisteen välillä
		int crosspoint1 = rand() % static_cast<int>(Nc-1-Nc/2); 
		int crosspoint2 = crosspoint1 + rand() % static_cast<int>(Nc-1-Nc/2); 
		
		//Tehdään CROSSOVER
		for (int r = 0; r < crosspoint1; ++r) {	
			c[max[i]][r] = c[parent1][r];	
		}
		for (int r = crosspoint1; r < crosspoint2; ++r) {
			c[max[i]][r] = c[parent2][r];		
		}
		for (int r = crosspoint2; r < Nc; ++r) {
			c[max[i]][r] = c[parent1][r];		
		}
		
	}
}


void quality(vector<int>& viite, vector<cluster>& clust, 
             vector<vector<int> >& c, vector<double>& Q) {
	
	// Matriisin c
	int Nc = c[0].size(); // sarakkeet
	int Nv = c.size();    // rivit
	
	//luodaan laatufunktiot geneettistä algoritmia varten
	double Q1 = 0; double Q2 = 0; double Q3 = 0;
	
	//suhteellinen h(Cr)
	vector<double> h(Nc,0);
	
	// biasointi vakiot (mitä suurempia luku, sitä suurempi painoarvo)
	// y3 muuttuu dynaamisesti Nc:n pienentyessä
	double y1 = 2.0; double y2 = 1.0; double y3 = static_cast<double>(Nc/2.0);
	
	//aputermit
	double lasku = 0.0;
	double osoittaja = 0.0;
	double max = 0.0;
	
	//h(Cr)
	for (int col = 0; col < Nc; ++col) {
		lasku += clust[viite[col]].N;
	}
	for (int col = 0; col < Nc; ++col) { // esiintymisprosentit
		h[col] = clust[viite[col]].N / lasku;
	}
	
	//Lasketaan laatu riveittäin
	for (int row = 0; row < Nv; ++row) {
		
		// valittujen alkioiden lkm
		double N = 0;
		
		// ---------------------------------------------------------------
		// Rankaisee jos alle yksi positiivinen tai alle yksi negatiivinen termi
		// tai ratkaisu käyttää epäkelpoja alleeleja
		// ---------------------------------------------------------------
		double Qextra = 0.0;
		int pos = 0; int neg = 0;
		
		for (int col = 0; col < Nc; ++col) {
			if (c[row][col] == 1 && clust[viite[col]].dP > 0) {
				++pos;
			} else if (c[row][col] == 1 && clust[viite[col]].dP < 0) {
				++neg;
			}
			if (c[row][col] == 1 && clust[viite[col]].dP == 1e6) { // käytössä epäkelpo alleeli
				Qextra += 1e6;
			}
		}
		if (pos < 1 || neg < 1 ) { // rankaisu täysin epäkelvosta kromosomista
			Qextra += 1e6;
		}
		
		N = static_cast<double>(pos+neg);
		
		// ---------------------------------------------------------------
		// Q1, painottaa tehojen summan minimointia
		// ---------------------------------------------------------------
		osoittaja = 0; lasku = 0; max = 0.0;
		
		for (int col = 0; col < Nc; ++col) {
		
			lasku = c[row][col]*clust[viite[col]].dP;
			osoittaja += lasku;
			
			if ( abs(lasku) > max) {
				max = abs(lasku);
			}
		}
		
		max > 0 ? Q1 = abs(osoittaja)/max : Q1 = 0; // normeeraus
		
		// ---------------------------------------------------------------
		// Q2, painottaa esiintymislukumäärään tasapainoa
		// ---------------------------------------------------------------
		osoittaja = 0; lasku = 0; max = 0;
		
		for (int col = 0; col < Nc; ++col) {
			
			lasku = c[row][col]*clust[viite[col]].N*sgn(clust[viite[col]].dP);
			osoittaja += lasku;
			
			if ( abs(lasku) > max) {
				max = abs(lasku);
			}
		}
		
		max > 0 ? Q2 = abs(osoittaja)/max : Q2 = 0; // normeeraus
		
		// ---------------------------------------------------------------
		// Q3, painottaa termien minimointia
		// ---------------------------------------------------------------
		
		Q3 = N / static_cast<double>(Nc); // normeeraus
		
		// ---------------------------------------------------------------
		// Kokonais Q
		// ---------------------------------------------------------------
		
		Q[row] = y1*Q1 + y2*Q2 + y3*Q3 + Qextra;
	}
}


double meanQuality(vector<double>& Q) {

	double sum = 0.0;
	
	for (int j = 0; j < Q.size(); ++j) {		
		sum += Q[j];
	}
	
	return (sum/static_cast<double>(Q.size()));
}


double bestQuality(vector<double>& Q) {

	double best = 1e6;
	
	for (int j = 0; j < Q.size(); ++j) {		
		if (Q[j] < best) { //minimointi
			best = Q[j];
		}
	}
	
	return best;
}


void load_data(vector<edge>& data, vector<edge>& data_pos, vector<edge>& data_neg) {
	
	char *zErrMsg = 0;
	sqlite3* nialm_db; //SQLITE3 tietokantakahva
	
	// Avataan tietokanta
	openDatabase(NIALM_DB_FILE, &nialm_db, 30000);
	
	//sisältää päivämäärät
	vector<DATE> dateArr;
	
	time_t rawtime;
	struct tm* timeinfo;
	
	//get current time
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	
	int year  = timeinfo->tm_year + 1900;
	int month = timeinfo->tm_mon  + 1;
	int day   = timeinfo->tm_mday;
	
	//luodaan päivämäärät
	//start_date = "1-2-2010";
	end_date = int2str(day) + "-" + int2str(month) + "-" + int2str(year);
	
	genDateArr(dateArr, start_date, end_date);
	
	//globaalimuuttuja
	days = dateArr.size();
	
	//käydään läpi joka päivä
	for (int x = 0; x < dateArr.size(); ++x) {
		
		char sql_query[50] = "SELECT epoch, power, dP FROM d";	
		date_string(sql_query, dateArr[x].day, dateArr[x].month, dateArr[x].year);
		
		//LUETAAN KYSEISEN PÄIVÄT ARVOT
		sqlite3_stmt* stmt = NULL;
	
		//valmistellaan käsky
		int rc = sqlite3_prepare_v2(nialm_db, sql_query, strlen(sql_query), &stmt, NULL);	
		
		if( rc != SQLITE_OK){
			fprintf(stderr, "SQL error: %s, %s\n", zErrMsg, sql_query);
		} else {
			//logi rivin bufferi  			
    		int epoch = 0;   		
    		int power = 0;
    		int dP = 0;

			//suoritetaan käskyä loopissa
			while(sqlite3_step(stmt) == SQLITE_ROW) {
					
				epoch = sqlite3_column_int(stmt,0);
				power = sqlite3_column_int(stmt,1);
				dP = sqlite3_column_int(stmt,2);
				
				if ( (abs(dP) > PIENIN_TEHO) && (abs(dP) < SUURIN_TEHO) ) {
					
					// kaikki reunat
					data.push_back(edge(dP, epoch, power, false, 0));
					
					if (dP > 0) { //positiiviset reunat
						data_pos.push_back(edge(dP, epoch, power, false, 0));
					} else {      //negatiiviset reunat
						data_neg.push_back(edge(dP, epoch, power, false, 0));
					}
				}
			}
			
			sqlite3_finalize(stmt);
		}
	}
	// Suljetaan tietokanta
	closeDatabase(&nialm_db);
}


void c_means(vector<edge>& x, vector<cluster>& c, int C) {
	
	// C-means klusterointi
	// C = klustereiden määrä
	
	// Datan määrä
	int N = x.size();
	
	// tätä minimoidaan alkuiteroinnissa
	double min_DB = 1e6;
	
	// halutut parannusratkaisut alkuratkaisun hauassa - 1
	int iter = 3;
	
	// Alkuiterointi "parhaan" aloitusratkaisun löytämiseksi
	for (int alfa = 0; alfa < iter; /* empty */) { // halutaan x parannusta iteroinnissa
		
		// Varsinaisten C-means iterointikierrosten lkm parannushaussa
		int rounds = 1;
		
		// iterointikierrokset
		if (alfa < (iter - 1)) {
			c.clear(); // TÄRKEÄ - nollataan klusterivektori
			
			for (int i = 0; i < C; ++i) {
				int random = rand()%(N - 1); // Haetaan luku väliltä  0...x.size()-1
				c.push_back(cluster(x[random].dP, 0, 0));
			}
		} else { // viimeinen kierros
			rounds = 100; // lopulliset iterointikierrokset
			alfa = iter;
			
			// Nollataan viimeksi kertyneet arvot (lasketaan iteroinnin päätyttyä)
			for (int i = 0; i < c.size(); ++i) {
				c[i].N = 0;
			}
		}
		
		// Lasketaan klustericentroidit
		for (int r = 0; r < rounds; ++r) {	// how many rounds to iterate
			for (int i = 0; i < N; ++i) { // through all the data
				
				double min = 0;
				double distance = 0;
				
				// || [x(i)]-[u(i)] ||^2 , sijoitetaan arvot klustereihin, komponenteittain minimoidaan etäisyyttä
				// eli sqrt[(x1-u1)^2 + (x2-u2)^2 + ... + (xn-un)^2]
				for (int j = 0; j < C; ++j) { // through all clusters
					
					distance  = abs(x[i].dP - c[j].dP);
					
					if (distance < min || j == 0) {
						min = distance; //new minimum	
						x[i].N = j; //kuuluu nyt uuteen klusteriin			
					}
				}
			}
			
			// Lasketaan klustereiden uudet keskipisteet (=keskiarvo komponenteittain)		
			for (int j = 0; j < C; ++j) {
			
				double dP_sum = 0; 
				double f = 0;
				
				for (int i = 0; i < N; ++i) {	
					if (x[i].N == j) {
						dP_sum += x[i].dP;
						++f;
					}
				}
				
				if (f > 0) { // Lasketaan keskiarvot vain, jos klusteriin kuuluu alkioita
					c[j].dP = dP_sum / f;			
				}
				
				//cout << "c[" << j << "]: " << c[j].dP << endl;
			}
		}
		
		// Lasketaan ominaissuureet kuten mediaanit, keskihajonta yms.
		if ( c_meansEigen(x, c) == false ) {
		
			// Davies-Bould index
			double DB = dB(c, C);
			
			// Katsotaan löytyikö parempi
			if (DB < min_DB) {
				min_DB = DB;
				++alfa;
			}
		}
	}
}


bool c_meansEigen(vector<edge>& x, vector<cluster>& c) {

	bool zeroflag = false; // onko täysin tyhjiä klustereita

	//lasketaan ja merkitään alkioiden lkm per clusteri
	for (int i = 0; i < x.size(); ++i) {
		++c[x[i].N].N;
	}
	
	//lasketaan todennäköisyydet
	for (int i = 0; i < c.size(); ++i) {
		c[i].prob = c[i].N / static_cast<double>(x.size());
	}
	
	//Lasketaan klusterikeskusten keskihajonnat
	for (int k = 0; k < c.size(); ++k) {
	
		vector<double> tempdP;
		
		for (int i = 0; i < x.size(); ++i) {	
			if (x[i].N == k) {
				tempdP.push_back(x[i].dP);
			}
		}
		
		if (tempdP.size() > 0) { // klusteriin on liitetty alkioita
		
			// keskihajonnat, supistettu kaava (wikipedia)
			double sum_of_squares = 0;
			
			for (int i = 0; i < tempdP.size(); ++i) {			
				sum_of_squares += tempdP[i]*tempdP[i];
			}
			
			c[k].dP_STD = static_cast<int>( sqrt((sum_of_squares - 
									tempdP.size()*c[k].dP*c[k].dP)/tempdP.size()) );
			
		} else {	
			zeroflag = true;
		}
	}
	
	return zeroflag;
}


void fuzzy_c_means(vector<edge>& x, vector<cluster>& c, int C) {
	
	// Fuzzy-c-means klusterointi
	// C = klustereiden määrä
	
	// Datan määrä
	int N = x.size();
	
	// Fuzzy matrix U, joka on (N x C)
	vector<vector<double> >U (N, vector<double>(C, 0));
	
	// tätä minimoidaan alkuiteroinnissa
	double min_DB = 1e6;
	
	// halutut parannusratkaisut alkuratkaisun hauassa - 1
	int iter = 3;
	
	// Alkuiterointi "parhaan" aloitusratkaisun löytämiseksi
	for (int alfa = 0; alfa < iter; /* empty */) { // halutaan x parannusta iteroinnissa
		
		// Varsinaisten C-means iterointikierrosten lkm parannushaussa
		int rounds = 1;
		
		// iterointikierrokset
		if (alfa < (iter - 1)) {
			// TÄRKEÄ - nollataan klusterivektori
			c.clear();
			
			for (int i = 0; i < C; ++i) {
				int random = rand() % (N - 1); // Haetaan luku väliltä  0...N-1
				double dither = closed_interval_rand(0.0, 10.0); // estetään Nan ongelmat fuzzy algoritmissa
				c.push_back(cluster(x[random].dP + dither, 0, 0));
			}
		} else { // viimeinen kierros
			rounds = 50; // lopulliset iterointikierrokset
			alfa = iter;
			
			// Nollataan viimeksi kertyneet arvot (lasketaan iteroinnin päätyttyä)
			for (int i = 0; i < c.size(); ++i) {
				c[i].N = 0;
			}
		}
		
		// Lasketaan klustericentroidit
		for (int r = 0; r < rounds; ++r) {	// how many rounds to iterate
			
			// Päivitetään U matriisi
			// U_ij = 1 / sum_k=1...C (||x_i - c_j||/||x_i - c_k||)^(2/(m-1))
			for (int i = 0; i < N; ++i) { // through all the data
				for (int j = 0; j < C; ++j) { // through all clusters
					
					double sum = 0.0;
					
					for (int k = 0; k < C; ++k) {
						double calc = abs(x[i].dP - c[j].dP) / (abs(x[i].dP - c[k].dP));
						sum += pow(calc, 2.0);				
					}
					
					U[i][j] = 1.0 / sum;
					
					//cout << "U[" << i << "][" << j << "]: " << U[i][j] << " x[i].dP: " << x[i].dP << endl;
				}
			}
			
			// Lasketaan klustereiden uudet keskipisteet
			// Cj = sum_i=1...N( u_ij^m * xj ) / sum_i=1...N ( u_ij^m )
			
			for (int j = 0; j < C; ++j) {
			
				double dP_sum = 0.0; 
				
				for (int i = 0; i < N; ++i) {
					dP_sum += pow(U[i][j], 2.0)*x[i].dP;
				}
				
				double nimittaja = 0.0;
				
				for (int i = 0; i < N; ++i) {
					nimittaja += pow(U[i][j], 2.0);
				}			
				if (nimittaja > 0.0) {
					c[j].dP = dP_sum / nimittaja;
				}
				
				//cout << "c[" << j << "].dP:" << c[j].dP << endl;
			}
		}

		// Lasketaan ominaissuureet kuten mediaanit, keskihajonta yms.
		if ( fuzzy_c_meansEigen(x, c, U) == false ) {
		
			// Davies-Bould index
			double DB = dB(c, C);
			
			// Katsotaan löytyikö parempi
			if (DB < min_DB) {
				min_DB = DB;
				++alfa;
			}
		}
	}
}


bool fuzzy_c_meansEigen(vector<edge>& x, vector<cluster>& c, vector<vector<double> >& U) {

	bool zeroflag = false; // onko täysin tyhjiä klustereita
	
	//lasketaan ja merkitään alkioiden lkm per clusteri (defuzzy)
	for (int i = 0; i < x.size(); ++i) {
		double max = 0;
		
		for (int k = 0; k < c.size(); ++k) {
			if (U[i][k] > max) {
				max = U[i][k];
				x[i].N = k;
			}
		}
		++c[x[i].N].N; // lisätään lkm
	}
	
	//lasketaan todennäköisyydet
	for (int i = 0; i < c.size(); ++i) {
		c[i].prob = c[i].N / static_cast<double>(x.size());
	}
	
	//Lasketaan klusterikeskusten keskihajonnat
	for (int k = 0; k < c.size(); ++k) {
		
		vector<double> tempdP;
		
		for (int i = 0; i < x.size(); ++i) {	
			if (x[i].N == k) {
				tempdP.push_back(x[i].dP);
			}
		}
		
		if (tempdP.size() > 0) { // klusteriin on liitetty alkioita
			
			// keskihajonnat, supistettu kaava (wikipedia)
			double sum_of_squares = 0;
			
			for (int i = 0; i < tempdP.size(); ++i) {			
				sum_of_squares += pow(tempdP[i], 2.0);
			}
			
			c[k].dP_STD = static_cast<int>( sqrt((sum_of_squares - 
									tempdP.size()*pow(c[k].dP, 2.0))/tempdP.size()) );
			
		} else {
			zeroflag = true;
		}
	}
	
	return zeroflag;
}


void em(vector<edge>& x, vector<cluster>& c, int C) {
	
	// Fuzzy-c-means klusterointi
	// C = klustereiden määrä
	
	// Datan määrä
	int N = x.size();
	
	// Fuzzy matrix (p)ij, joka on (N x C)
	vector<vector<double> >p (static_cast<int>(N), vector<double>(C, 0));
	
	// tätä minimoidaan alkuiteroinnissa
	double min_DB = 1e6;
	
	// halutut parannusratkaisut alkuratkaisun hauassa - 1
	int iter = 3;
	
	// Alkuiterointi "parhaan" aloitusratkaisun löytämiseksi
	for (int alfa = 0; alfa < iter; /* empty */) { // halutaan x parannusta iteroinnissa
		
		// Varsinaisten C-means iterointikierrosten lkm parannushaussa
		int rounds = 1;
		
		// iterointikierrokset
		if (alfa < (iter - 1)) {
			// -------------------------------------------------------------------
			// Alusta uj, sigma_j, P(wj)
			//--------------------------------------------------------------------
			c.clear();
			
			for (int i = 0; i < C; ++i) {
				int random = rand() % (x.size() - 1); // Haetaan luku väliltä  0...N-1
				
				double dP_dither = closed_interval_rand(0.0, 10.0);
				double dP_STD = closed_interval_rand(1.0, 15.0);
				
				c.push_back( cluster(x[random].dP + dP_dither, dP_STD, 1.0/static_cast<double>(C) ) );
			}
			
		} else { // viimeinen kierros
			rounds = 50; // lopulliset iterointikierrokset
			alfa = iter;
		}
		
		for (int r = 0; r < rounds; ++r) {
			
			// -------------------------------------------------------------------
			// E-askel, posterior todennäköisyydet, että xi kuuluu luokkaan wj
			// -------------------------------------------------------------------
			
			for (unsigned int i = 0; i < x.size(); ++i) {
				
				// p(x) = sum[k=1...C]( p(x|Wk)*P(Wk) )
				// missä p(x|Wk) = 1/(sqrt(2*pi)*sigma_k)*e(-(x-mju_k)^2/(2*sigma_k^2))
				
				double nimittaja = 0.0;

				for (unsigned int k = 0; k < C; ++k) {
					
					double eksponent = - pow(static_cast<double>(x[i].dP - c[k].dP), 2.0)
									 / (2.0*pow(static_cast<double>(c[k].dP_STD), 2.0));
					
					nimittaja += (1.0/(sqrt(2.0*pi)*c[k].dP_STD))
							   * pow(neper, eksponent) * c[k].prob;
				}
				
				// lopullinen lasku: P(Wj|x) = p(x|Wj)P(Wj)/p(x)
				// missä p(x) laskettiin ylempänä ja
				// missä p(x|Wj) = 1/(sqrt(2*pi)*sigma_j)*e(-(x-mju_j)^2/(2*sigma_j^2))
				
				for (unsigned int j = 0; j < C; ++j) {
					
					double eksponent = - pow(static_cast<double>(x[i].dP - c[j].dP), 2.0)
									   / (2.0*pow(static_cast<double>(c[j].dP_STD), 2.0));
					
					double osoittaja = (1.0/(sqrt(2.0*pi)*c[j].dP_STD))
									  * pow(neper, eksponent) * c[j].prob;
					
					p[i][j] = osoittaja/nimittaja;
					
					if ( isnan(p[i][j]) ) { //Nan
						p[i][j] = 0.001;
					}
					
					//cout << "r: " <<  r << ", p[" << i << "][" << j << "]: " << p[i][j] << endl;
				}
			}
			
			// -------------------------------------------------------------------
			// M-askel, Lasketaan uudet parametrien arvot
			// -------------------------------------------------------------------
			
			for (unsigned int j = 0; j < C; ++j) {
				
				double sum = 0;
				// sum[i=1...N](pij)
				for (unsigned int i = 0; i < N; ++i) {
					sum += p[i][j];
				}
				
				//P(t+1)(wj) = (1/N)*sum[i=1...N](pij)
				c[j].prob = sum/static_cast<double>(N);
				
				//uj(t+1)
				double osoittaja = 0.0;
				for (unsigned int i = 0; i < N; ++i) {
					osoittaja += p[i][j]*x[i].dP;
				}
				
				c[j].dP = osoittaja/sum;
				
				//sigma(j)(t+1)
				osoittaja = 0.0;
				for (unsigned int i = 0; i < N; ++i) {
					osoittaja += p[i][j]*pow(x[i].dP - c[j].dP, 2.0);
				}
				
				c[j].dP_STD = sqrt(osoittaja/sum);
				
				//cout << "j: " << j << ", P(Wj): " << c[j].prob << ", uj:" << c[j].dP << ", sigma_j: " << c[j].dP_STD << endl;
			}
		}
		
		// Davies-Bould index
		double DB = dB(c, C);
		
		// Katsotaan löytyikö parempi
		if (DB < min_DB) {
			min_DB = DB;
			++alfa;
		}
	}
	
	//lasketaan ja merkitään alkioiden lkm per clusteri (defuzzy)
	for (int i = 0; i < x.size(); ++i) {
		double max = 0;
		
		for (int j = 0; j < c.size(); ++j) {
			if (p[i][j] > max) {
				max = p[i][j];
				x[i].N = j;
			}
		}
		++c[x[i].N].N; // lisätään lkm
	}
	
}


void savetodb(vector<vector<int> >& devices, vector<cluster>& cluster, vector<vector<int> >& A) {
	
	sqlite3* nialm_db;

	// Avataan tietokanta
	if (openDatabase(NIALM_DB_FILE, &nialm_db, 30000) == false) {
		#ifdef DEBUG
		cout << "openDatabase LIVE_DB error " << endl;
		throw "error";
		#endif
	}
	
	char *zErrMsg;
	
	// BEGIN
	int rc = sqlite3_exec(nialm_db, "BEGIN;", NULL, NULL, &zErrMsg);
	{
		// --------------------------------------------
		// Tiputetaan taulukko
		
		char sql_drop_table[] = "DROP TABLE clusters";
		rc = sqlite3_exec(nialm_db, sql_drop_table, NULL, NULL, &zErrMsg);
		
		if( rc != SQLITE_OK ){
			#ifdef DEBUG
			fprintf(stderr, "SQL error %s\n", zErrMsg);
			#endif
			sqlite3_free(zErrMsg);
		}
		
		// --------------------------------------------
		// Luodaan taulukko
		
		char sql_create_table[] = "CREATE TABLE clusters (clusterId INTEGER, dP INTEGER, dP_STD INTEGER, prob DOUBLE, period INTEGER, deviceId INTEGER, N INTEGER)";
		rc = sqlite3_exec(nialm_db, sql_create_table, NULL, NULL, &zErrMsg);
		
		if( rc != SQLITE_OK ){
			#ifdef DEBUG
			fprintf(stderr, "SQL error %s\n", zErrMsg);
			#endif
			sqlite3_free(zErrMsg);
		}
		
		// --------------------------------------------
		// Luodaan rivit
		
		for (int i = 0; i < cluster.size(); ++i) {
		
			int deviceId = -1;
			bool found = false;
			
			// haetaan tähän klusteriin liitetty laite
			for (int d = 0; d < devices.size(); ++d) {
				for (int c = 0; c < devices[d].size(); ++c) {
					if (devices[d][c] == i) {
						deviceId = d;
						found = true;
					}
					if (found == true) break;
				}
				if (found == true) break;
			}
			
			char sql_insert_row[200];
			sprintf(sql_insert_row, "INSERT INTO clusters VALUES (%d, %0.0f, %0.0f, %0.6f, %d, %d, %d)",
					i, cluster[i].dP, cluster[i].dP_STD, cluster[i].prob, cluster[i].period, deviceId, cluster[i].N);
			
			rc = sqlite3_exec(nialm_db, sql_insert_row, NULL, NULL, &zErrMsg);
			
			if( rc != SQLITE_OK ){
				#ifdef DEBUG
				fprintf(stderr, "SQL error %s\n", zErrMsg);
				#endif
				sqlite3_free(zErrMsg);
			}
			
		}
		
		// --------------------------------------------
		// Tiputetaan taulukko
		
		char sql_drop_table2[] = "DROP TABLE devices";
		rc = sqlite3_exec(nialm_db, sql_drop_table2, NULL, NULL, &zErrMsg);
		
		if( rc != SQLITE_OK ){
			#ifdef DEBUG
			fprintf(stderr, "SQL error %s\n", zErrMsg);
			#endif
			sqlite3_free(zErrMsg);
		}
		
		// --------------------------------------------
		// Luodaan taulukko
		
		char sql_create_table2[] = "CREATE TABLE devices (deviceId INTEGER, deviceName VARCHAR(15), deviceImage VARCHAR(30))";
		rc = sqlite3_exec(nialm_db, sql_create_table2, NULL, NULL, &zErrMsg);
		
		if( rc != SQLITE_OK ){
			#ifdef DEBUG
			fprintf(stderr, "SQL error %s\n", zErrMsg);
			#endif
			sqlite3_free(zErrMsg);
		}
		
		// --------------------------------------------
		// Luodaan laiterivit
		
		for (int i = 0; i < devices.size(); ++i) {

			char sql_insert_row[200];
			sprintf(sql_insert_row, "INSERT INTO devices VALUES (%d, 'Device_X%d', 'unknown.jpg')", i, i);
			
			rc = sqlite3_exec(nialm_db, sql_insert_row, NULL, NULL, &zErrMsg);
			
			if( rc != SQLITE_OK ){
				#ifdef DEBUG
				fprintf(stderr, "SQL error %s\n", zErrMsg);
				#endif
				sqlite3_free(zErrMsg);
			}
		}
		
		// -------------------------------------------------------
	}
	
	// END
	rc = sqlite3_exec(nialm_db, "END;", NULL, NULL, &zErrMsg);
	
	// Suljetaan tietokanta
	closeDatabase(&nialm_db);
}


