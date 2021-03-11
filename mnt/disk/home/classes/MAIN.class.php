<?php
// deltaBox - Algorithmic Non-Intrusive Load Monitoring
// 
// Copyright (c) 2008-2009 Mikael Mieskolainen.
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// 
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
	
	require_once("/home/classes/DB.class.php");
	
	error_reporting(E_ALL);
	
	function dirList($directory) 
	{

		// create an array to hold directory list
		$files = array();
	
		// create a handle for the directory
		$handler = opendir($directory);

		// keep going until all files in directory have been read
		while ($file = readdir($handler)) {

			// if $file isn't this directory or its parent, 
			// add it to the results array
			if ($file != '.' && $file != '..')
				$files[] = $file;
		}
		
		// close the handle
		closedir($handler);

		// done!
		return $files;
	}

	
	function getPriceData(&$power_day_price, &$power_night_price, &$power_day_start, &$power_night_start )
	{
		$fp2 = fopen ( "/disk/powercalc.conf", "r" );
		
		while ( $line = fgetcsv ($fp2, 50, " ")) {
		
			if ($line[0] == "power_day_price") {	
				$power_day_price=$line[1];
			}
			if ($line[0] == "power_night_price") {	
				$power_night_price=$line[1];
			}
			if ($line[0] == "power_day_start") {	
				$power_day_start=$line[1];
			}
			if ($line[0] == "power_night_start") {	
				$power_night_start=$line[1];
			}
		}
		fclose ( $fp2 );
	}
			


	class CLUSTER
	{
		
		private $dbObj;	    //tietokanta olio
		private $nialm_db;  //tietokanta kahva
		
		//viesti joka printataan sivun lopussa, jos tehtiin muutoksia
		public $update_message = NULL;
		
		function __construct()
		{			
			//luodaan tietokanta olio
			$this->dbObj = new DB();
			
			//avataan tietokanta kahva
			$this->nialm_db = $this->dbObj->openDatabase("sqlite:/mnt/mmc/nialm.db");
			
			//paivitetaan uusi data
			$this->submitData();
			
			//paivitetaan uusi data
			$this->addDevice();
			
			//paivitetaan uusi data
			$this->deleteDevice();
			
		}
		
		function __destruct()
		{
			//NOLLATAAN tietokanta objekti
			unset($this->nialm_db);
		}
		
		public function printCarusel()
		{
			//Haetaan onewire anturien tiedot
			$sql_query = "SELECT * FROM devices";
				
			//haetaan tiedot taulukkoon
			$devices = $this->dbObj->readDatabase($this->nialm_db, $sql_query);
			
			for ($i = 0; $i < count($devices); ++$i) {
				
				//haetaan tiedot taulukkoon
				$sql_query = "SELECT * FROM clusters WHERE deviceId='" . $i . "'";
				$device = $this->dbObj->readDatabase($this->nialm_db, $sql_query);
				
				if ($device != NULL) {
					
					echo("<div class='item'>");
					
					$powers = "";
					
					for ($j = 0; $j < count($device); ++$j) { // esim. 1231W | -1213 W
						$powers = $powers . $device[$j]["dP"] . "W";
						if ($j != count($device) - 1) {
							$powers = $powers . " | ";
						}
					}
					
					echo sprintf("<img class='content' src='/covers/%s'/>", $devices[$i]["deviceImage"]);
					echo sprintf("<div class='caption'>%s<br/>%s</div>", $devices[$i]["deviceName"], $powers);
					echo sprintf("<div class='label'>...</div>");
					
					echo("</div>");
				}
			}
		}
		
		public function printJs()
		{
			//Haetaan energiatiedot
			$cluster_data = @file("/tmp/cluster.data"); //luetaan tiedosto taulukkoon
			
			//Haetaan laitetiedot
			$sql_query = "SELECT * FROM devices";
			
			//haetaan tiedot taulukkoon
			$devices = $this->dbObj->readDatabase($this->nialm_db, $sql_query);
			
			//Print javascript object
			echo("var data = {items: [");
			
			for ($i = 0; $i < count($cluster_data) - 1; ++$i) { // -1 koska baseload
			
				$sql_query = "SELECT clusterId FROM clusters WHERE deviceId='" . $i . "'";
				$cluster = $this->dbObj->readDatabase($this->nialm_db, $sql_query);
				
				if ($cluster != NULL) {//haetaan vain mapatyt laitteet
					echo sprintf("{label: '%s', data: %f},", $devices[$i]["deviceName"], $cluster_data[$i]);
				}
			}
			
			echo sprintf("{label: 'Baseload', data: %f},", $cluster_data[count($cluster_data)-1]);
			
			//Print js end
			echo("]};");
			
		}
		
		public function printData()
		{
			
			//Haetaan kuvatiedostot
			$imageFiles = dirList("/home/httpd/covers");
			
			//Haetaan laitteiden tiedot
			$sql_query = "SELECT * FROM devices";
				
			//haetaan tiedot taulukkoon
			$devices = $this->dbObj->readDatabase($this->nialm_db, $sql_query);
			
			//taulukon alku
			echo ("<table id='mytable'>");
			
			//Haetaan clustereiden tiedot
			$sql_query = "SELECT * FROM clusters";
				
			//haetaan tiedot taulukkoon
			$clusters = $this->dbObj->readDatabase($this->nialm_db, $sql_query);
			
			for ($i = 0; $i < count($devices) + 1; ++$i) { // + 1 unmappedia varten
				
				if ($i % 10 == 0) { // Printataan väliin otsikko
					echo ("<tr><th>Device ID</th><th>dP (W)</th><th>dP &#963 (W)</th><th>Period (s)</th><th>Prob</th><th>Name</th><th>Image</th><th>New ID</th></tr>");
				}
				
				//haetaan tiedot taulukkoon
				$sql_query = "";
				
				if ($i < count($devices)) { // normaalit laitteet
					$sql_query = "SELECT * FROM clusters WHERE deviceId='" . $i . "'";
				} else {
					$sql_query = "SELECT * FROM clusters WHERE deviceId='-1'"; // Lopuksi vapaat clusterit
					//Tyhjä rivi
					echo ("<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>");
					echo ("<tr><th>Unmapped</th><th>dP (W)</th><th>dP &#963 (W)</th><th>Period (s)</th><th>Prob</th><th></th><th></th><th>New ID</th></tr>");
				}
				
				$dev = $this->dbObj->readDatabase($this->nialm_db, $sql_query);
				
				// Tyhjät laitteet
				if (count($dev) == 0) {
					
					echo ("<tr>");
					
					// DeviceId
					if ($i < count($devices)) { // normaalit laitteet
						echo sprintf("<td><input maxlength='2' size='2' disabled='disabled' name='deviceId_%d' id='deviceId_%d' value='%d'/></td>", $devices[$i]["deviceId"], $devices[$i]["deviceId"], $devices[$i]["deviceId"]);
					} else { // unmapped
						echo ("<td><input maxlength='2' size='2' disabled='disabled' value='-1'/></td>");
					}
					
					// Cluster values					
					echo ("<td>&nbsp;</td>");
					echo ("<td>&nbsp;</td>");
					echo ("<td>&nbsp;</td>");
					echo ("<td>&nbsp;</td>");
					
					// Devicename
					if ($i < count($devices)) { // normaalit laitteet
						echo sprintf("<td><input maxlength='12' size='12' type='text' name='deviceName_%d' id='deviceName_%d' value='%s'/></td>", $devices[$i]["deviceId"], $devices[$i]["deviceId"], $devices[$i]["deviceName"] );
					} else { // unmapped
						echo ("<td>&nbsp;</td>");
					}
					
					// DeviceImage
					if ($i < count($devices)) { // normaalit laitteet
						echo sprintf("<td><SELECT name='deviceImage_%d' id='deviceImage_%d'>", $devices[$i]["deviceId"], $devices[$i]["deviceId"]);
						
						for ($k = 0; $k < count($imageFiles); ++$k) {
							if ($devices[$i]["deviceImage"] == $imageFiles[$k]) {
								echo sprintf("<option value='%s' SELECTED>%s</option>", $imageFiles[$k], $imageFiles[$k]); // Valittu
							} else {
								echo sprintf("<option value='%s'>%s</option>", $imageFiles[$k], $imageFiles[$k]);
							}
						}
						
						echo("</select></td>");
					} else { // unmapped
						echo ("<td>&nbsp;</td>");
					}
					
					echo("<td>&nbsp;</td>");					
					echo ("</tr>");
				}
				
				for ($j = 0; $j < count($dev); ++$j) {
				
					echo ("<tr>");
					
					// DeviceId
					if ($j == 0 && $i < count($devices)) {
						echo sprintf("<td><input maxlength='2' size='2' disabled='disabled' name='deviceId_%d' id='deviceId_%d' value='%d'/></td>", $dev[$j]["deviceId"], $dev[$j]["deviceId"], $dev[$j]["deviceId"]);
					} else if ($i == count($devices)) {
						echo sprintf("<td><input maxlength='2' size='2' disabled='disabled' name='deviceId_-1' id='deviceId_-1' value='-1'/></td>");
					} else {
						echo ("<td></td>");
					}
					
					// Cluster values
					echo sprintf("<td><input maxlength='4' size='4' disabled='disabled' name='dP_%d' id='dP_%d' value='%d'/></td>", $dev[$j]["clusterId"], $dev[$j]["clusterId"], $dev[$j]["dP"] );
					echo sprintf("<td><input maxlength='4' size='4' disabled='disabled' name='dP_STD_%d' id='dP_STD_%d' value='%d'/></td>", $dev[$j]["clusterId"], $dev[$j]["clusterId"], $dev[$j]["dP_STD"] );
					echo sprintf("<td><input maxlength='5' size='5' disabled='disabled' name='period_%d' id='period_%d' value='%d'/></td>", $dev[$j]["clusterId"], $dev[$j]["clusterId"], $dev[$j]["period"] );
					echo sprintf("<td><input maxlength='3' size='3' disabled='disabled' name='prob_%d' id='prob_%d' value='%0.2f'/></td>", $dev[$j]["clusterId"], $dev[$j]["clusterId"], $dev[$j]["prob"]*100 );
					
					// Image
					if ($j == 0 && $i < count($devices)) {
						echo sprintf("<td><input maxlength='12' size='12' type='text' name='deviceName_%d' id='deviceName_%d' value='%s'/></td>", $dev[$j]["deviceId"], $dev[$j]["deviceId"], $devices[$i]["deviceName"] );
						
						echo sprintf("<td><SELECT name='deviceImage_%d' id='deviceImage_%d'>", $dev[$j]["deviceId"], $dev[$j]["deviceId"]);
						for ($k = 0; $k < count($imageFiles); ++$k) {
							if ($devices[$i]["deviceImage"] == $imageFiles[$k]) {
								echo sprintf("<option value='%s' SELECTED>%s</option>", $imageFiles[$k], $imageFiles[$k]); // Valittu
							} else {
								echo sprintf("<option value='%s'>%s</option>", $imageFiles[$k], $imageFiles[$k]);
							}
						}
						echo("</select></td>");
						
					} else {
						echo ("<td></td>");
						echo ("<td></td>");
					}
					
					// Change Device
					echo sprintf("<td><SELECT name='cluster_deviceId_%d' id='cluster_deviceId_%d'>", $dev[$j]["clusterId"], $dev[$j]["clusterId"]);
					for ($k = -1; $k < count($devices); ++$k) {
						if ($dev[$j]["deviceId"] == $k) {
							echo sprintf("<option value='%s' SELECTED>%s</option>", $k, $k); // Valittu
						} else {
							echo sprintf("<option value='%s'>%s</option>", $k, $k);
						}
					}
					echo("</select></td>");
					
					echo ("</tr>");
				}
			}
			
			echo sprintf("<input type='hidden' name='cluster_count' id='cluster_count' value='%d'/>", count($clusters));
			echo sprintf("<input type='hidden' name='device_count' id='device_count' value='%d'/>", count($devices));
			
			//taulukon loppu
			echo ("<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>
					   </td><td><input type='submit' class='push_button' name='submit_data' value='Save' /></td><td>&nbsp;</td></tr></table>");
					   
			echo ("<br/><br/><input type='submit' class='push_button' name='addDevice' value='Add device' />
							 <input type='submit' class='push_button' name='deleteDevice' value='Del device' />");
		}
		
		private function addDevice() {
			
			if (isset ($_POST['addDevice'])) {		
			
				//Lisätään yksi uusi	
				$sql_query = sprintf("INSERT INTO devices VALUES (%d, 'Device_X%d', 'unknown.jpg')", $_POST['device_count'], $_POST['device_count']);
				
				//kirjoitetaan tietokanta
				$this->dbObj->writeDatabase($this->nialm_db, $sql_query);
				
				$this->update_message = "<p>Add device done!</p>";
			}
		}
		
		private function deleteDevice() {
			
			if (isset ($_POST['deleteDevice'])) {
				
				if ($_POST['device_count'] >= 3) {
				
					//Poistetaan laite
					$sql_query = sprintf("DELETE FROM devices WHERE deviceId='%d'", $_POST['device_count'] - 1);
					
					//kirjoitetaan tietokanta
					$this->dbObj->writeDatabase($this->nialm_db, $sql_query);
					
					//PÄIVITETÄÄN -> unmapped (-1)
					$sql_query = sprintf("UPDATE clusters SET deviceId='-1' WHERE deviceId='%d'", $_POST['device_count'] - 1);
					
					//kirjoitetaan tietokanta
					$this->dbObj->writeDatabase($this->nialm_db, $sql_query);
					
					$this->update_message = "<p>Delete device done!</p>";
				
				} else {
					$this->update_message = "<p>Cannot delete any more!</p>";
				}
			}
		}
		
		private function submitData() {
			
			if (isset ($_POST['submit_data'])) {
				
				//Haetaan ensimmaiseksi clusterit
				$sql_query = "SELECT clusterId, deviceId FROM clusters";
				
				//haetaan tiedot taulukkoon
				$clusters = $this->dbObj->readDatabase($this->nialm_db, $sql_query);
				
				//Haetaan ensimmaiseksi laitteiden ID:T
				$sql_query = "SELECT deviceId, deviceName, deviceImage FROM devices";
				
				//haetaan tiedot taulukkoon
				$devices = $this->dbObj->readDatabase($this->nialm_db, $sql_query);
				
				for ($i = 0; $i < count($clusters) ; ++$i) {
				
					$cluster_deviceId = $_POST["cluster_deviceId_" . $clusters[$i]['clusterId'] ];
					
					if ($cluster_deviceId != $clusters[$i]["deviceId"]) {
						// Päivitetään tietokanta
						$sql_query = sprintf("UPDATE clusters SET deviceId='%d' WHERE clusterId='%d'", $cluster_deviceId, $clusters[$i]['clusterId']);
						
						//suoritetaan kasky
						$this->dbObj->writeDatabase($this->nialm_db, $sql_query);
					}
					
				}
				
				for ($i = 0; $i < count($devices) ; ++$i) {
					
					// Haetaan laitteen uudet tiedot
					if ( isset($_POST["deviceName_" . $devices[$i]['deviceId'] ]) ) { // Onko laite olemassa tällä indeksillä $i
					
						$deviceName = $_POST["deviceName_" . $devices[$i]['deviceId'] ];
						$deviceImage = $_POST["deviceImage_" . $devices[$i]['deviceId'] ];
						
						if ( $deviceName != $devices[$i]['deviceName'] || $deviceImage != $devices[$i]['deviceImage'] ) {
							// Päivitetään tietokanta
							$sql_query = sprintf("UPDATE devices SET deviceName='%s', deviceImage='%s' WHERE deviceId='%d'", 
												 $deviceName, $deviceImage, $devices[$i]['deviceId']);
							
							//suoritetaan kasky
							$this->dbObj->writeDatabase($this->nialm_db, $sql_query);
						}
						
					}
				}
				
				$this->update_message = "<p>Settings updated!</p>";
			}
		}
	}

	
	class FORECAST
	{
		
		private $dbObj;		//tietokanta olio
		
		private $db;    	//tietokanta kahva
		private $config_db; //tietokanta kahva
		
		function __construct()
		{			
			//luodaan tietokanta olio
			$this->dbObj = new DB();
			
			//avataan tietokanta kahva
			$this->db = $this->dbObj->openDatabase("sqlite:/mnt/mmc/powercalc.db");
			
			//avataan tietokanta kahva
			$this->config_db = $this->dbObj->openDatabase("sqlite:/mnt/mmc/config.db");
		}
		
		function __destruct()
		{
			
			//NOLLATAAN tietokanta objekti
			unset($this->db);
			
		}	
		
		private function getDateArray($d1, $m1, $y1, $d2, $m2, $y2, &$dates)
		{
				
			//palautettava taulukko
			$dateArray = array();
			
			//dateArray indeksi
			$x = 0;	
			
			//Loop muuttujat h, i, j
			$h=0; $i=0; $j=0;
			
			$startyear=$y1; 	$endyear=$y2;		
			$startmonth = 0; 	$endmonth=0;				
			$startday = 0; 		$endday=0;
		
			
			for ($h=$startyear ; $h<$endyear+1 ; ++$h) { //year loop
		
				if ($h==$endyear && $startyear==$endyear) { //saman vuoden sisalla
					$startmonth=$m1;
					$endmonth=$m2;
				}
				
				if ($h==$startyear && $startyear!=$endyear) { //aloitusvuosi
					$startmonth=$m1;
					$endmonth=12;
				}
				
				if ( $h > $startyear && $h<$endyear ) { //valivuodet
					$startmonth=1;
					$endmonth=12;
					
				}
				
				if ($h==$endyear && $startyear!=$endyear) { //siirrytaan viimeiselle vuodelle
					$startmonth=1;
					$endmonth=$m2;
				}		
				
				if (($h % 4) == 0){  //Tarkastellaan karkausvuosi
					$dates[2]=29;
				}
		
				
				for ($i=$startmonth ; $i<$endmonth+1 ; ++$i) { //month loop
					
					$startday=1;	
					$endday=$dates[$i];	
						
					if ($i==$startmonth && $h==$startyear) { //aloituskuukausi aloitusvuodelta
						$startday=$d1;
					}	
												
					if ($i==$endmonth && $h==$endyear) { //siirrytaan viimeiselle kuulle
						$endday=$d2;
					}
											
					if ($i<10) {
						$i = "0" . $i;
					}	
					
					for ($j=$startday ; $j<$endday+1 ; ++$j) { //day loop
					
						if ($j<10) {
							$j = "0" . $j;
						}
						
						$dateArray[$x]["day"] = $j;
						$dateArray[$x]["month"] = $i;
						$dateArray[$x]["year"] = $h;
						
						++$x;
													
					}
				}
			}
			
			return $dateArray;
				
		}
		
		public function getForecast($deviceId) 
		{
			flush();
			
			//maaritetaan kantaperiodi ennustukselle
			$epoch1 = date("U") - 86400*6; //minus days
			$epoch2 = date("U") - 86400*1; //minus days
			
			$system_month = date("n");
			$system_year = date("Y");
		
			$d1=date("j", $epoch1); $m1=date("n", $epoch1); $y1=date("Y", $epoch1);
			
			$d2=date("j", $epoch2); $m2=date("n", $epoch2); $y2=date("Y", $epoch2);
			
			//luetaan sahkonhinta ja ajat
			$power_day_price = 0;
			$power_night_price = 0 ;
			$power_day_start = 0;
			$power_night_start = 0;
			
			getPriceData($power_day_price, $power_night_price, $power_day_start, $power_night_start);
			
			// Haetaan tietokannasta
			$sql_query = "SELECT deviceMeterConst FROM io_data WHERE deviceId=" . $deviceId;
			
			// haetaan tiedot taulukkoon
			$row = $this->dbObj->readDatabase($this->config_db, $sql_query);
			$meterConst = $row[0]["deviceMeterConst"];
			
			$room_temperature = 0;
			
			//kuukausien nimet
			$month_name = array(0, "January", "February", "March", "April", "May", "June", 
								   "July", "August", "September", "October", "November", "December");
			
			//oletus astepaivaluvut JYVASKYLA (1970-2000)
			$cd = array(0,0,0,0,0,0,0,0,0,0,0,0,0);
			
			//kuukausien paivien lkm taulukko
			$dates = array(0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31);
			
			// kuukausien kokonaiskulutus [Wh/day]
			$month_calc_mean = array(0,0,0,0,0,0,0,0,0,0,0,0,0);
			
			// kuukausien epoch stamp JAVASCRIPT graafia varten
			$month_stamp = array(0,0,0,0,0,0,0,0,0,0,0,0,0);
			
			$total_day_pulses = 0;
			$total_night_pulses = 0;
			$tick_day_index = 0;
			$tick_night_index = 0;
			
			$tempOutSum = 0;
			$tempOutIndex = 0;
			
			$tempInSum = 0;
			$tempInIndex = 0;
			
			$total_calculated_days = 0;
			
			//haetaan pvm taulukko
			$dateArray = $this->getDateArray($d1, $m1, $y1, $d2, $m2, $y2, &$dates);

			for ($d = 0; $d < count($dateArray); ++$d) {
									
				//SQL komento
				$sql_query = "SELECT epoch, energy_" . $deviceId . ", ow_0, ow_2 FROM d" . $dateArray[$d]["day"] . $dateArray[$d]["month"] . $dateArray[$d]["year"];
				
				//haetaan tiedot taulukkoon
				$result = $this->dbObj->readDatabase($this->db, $sql_query);
				
				$size = count($result);
				
				
				if ( $size > 0) {
					
					$hour = 0;
					
					for ($x = 0; $x < $size; ++$x) {
						
						if ($x % 10 == 0) {
							$hour=date("G", $result[$x]["epoch"]);									
						}

						$tempOutSum += $result[$x]["ow_0"];
        				++$tempOutIndex;
        				
						$tempInSum += $result[$x]["ow_2"];
        				++$tempInIndex;

						if ($hour>=$power_night_start || $hour<$power_day_start ) {	//yosahko					
							$total_night_pulses += $result[$x]["energy_" . $deviceId]; 
							++$tick_night_index;
							
						} else { // paivasahko					
							$total_day_pulses += $result[$x]["energy_" . $deviceId];
							++$tick_day_index;
						}
						
					}

    				++$total_calculated_days;
    					
				}							
								
			}
			
			if ($total_calculated_days>=5) { //jos keråµ´y riittå·¤sti dataa
									
			    //paivan pohjateho
				$pohjateho = ($total_day_pulses/$meterConst*60000)/$tick_day_index; 
		
				//valitaan aloituskuukausi
				$i = (int) $m1;
			
				//lasketaan sisalampotila
				$room_temperature = ($tempInSum/$tempInIndex);
				
				$cd[0]=0; //tyhjä¡¡lkio
				$cd[1]=$room_temperature - -8.5;
				$cd[2]=$room_temperature - -8.7;
				$cd[3]=$room_temperature - -4;
				$cd[4]=$room_temperature - 1.4;
				$cd[5]=$room_temperature - 8.7;
				$cd[6]=$room_temperature - 14;
				$cd[7]=$room_temperature - 16;
				$cd[8]=$room_temperature - 13.7;
				$cd[9]=$room_temperature - 8.2;
				$cd[10]=$room_temperature - 3.2;
				$cd[11]=$room_temperature - -2.2;
				$cd[12]=$room_temperature - -6.4;
				
				//lasketaan lampoarvot
				$cd[$i] = $room_temperature - ($tempOutSum/$tempOutIndex);
					
				//kokopaivakulutuksen aritmeettinen keskiarvo [Wh] talle kuulle
				$month_calc_mean[$i] = ($total_night_pulses+$total_day_pulses)*1000/$meterConst/$total_calculated_days;
	
				for ($k=1 ; $k<=12 ; ++$k) { //lasketaan muille kuukausille ennustukset, oletus että pohjateho puolet
				
					if ($month_calc_mean[$k]==0) { //lasketaan vain tyhjille ennustukset, taten voisi ottaa painotuksen pohjakuukausien aritmeettisen keskiarvon yms.
						$month_calc_mean[$k] = ( ($cd[$k]*$dates[$k])/($cd[$i]*$dates[$i]) )*($month_calc_mean[$i] - ($pohjateho*($power_night_start-$power_day_start)) - ($pohjateho/2)*(24-($power_night_start-$power_day_start)) ) 
						+ ($pohjateho*($power_night_start-$power_day_start)) + ($pohjateho/2)*(24-($power_night_start-$power_day_start));
					}
										
				}
					
			}

						
			echo "<script type='text/javascript'>stopLoading();</script>";
			flush();
		
			$future_kwh_total=0;
			$future_kwh_day_total=0;
			$future_kwh_night_total=0;
		
			$total_price=0;
			$total_day_price=0;
			$total_night_price=0;
			
			if ($month_calc_mean[6]!=0) { //jos on laskettu esim. kesakuulle arvio naytetaan ennustukset
			
				echo "<table id='mytable'>";
			
				echo "<tr>";
				echo "<th >MONTH</th>";
				echo "<th >TOT. ENERGY</th>";
				echo "<th >TOT. COSTS</th>";
				echo "<th >DAY ENERGY</th>";
				echo "<th >DAY COSTS</th>";
				echo "<th >NIGHT ENERGY</th>";
				echo "<th >NIGHT COSTS</th>";
				echo "</tr>";
		
			
			//taman vuoden loppukuut	
			for ($k=$system_month ; $k<=12 ; ++$k) {
								
				$night_price = ($month_calc_mean[$k]-$pohjateho*($power_night_start-$power_day_start))*$power_night_price*$dates[$k]/100000;
				
				$day_price = ($pohjateho*($power_night_start-$power_day_start))*$power_day_price*$dates[$k]/100000;
						
				
				$total_night_price += $night_price;
				$total_day_price += $day_price;
				
				date_default_timezone_set('UTC');
				$month_stamp[$k] = mktime(0,0,0,$k,1,$system_year);
				
				echo "<tr>";
				echo sprintf("<th class='spec'>%s 0%d</th>",$month_name[$k],$system_year-2000);
				echo sprintf("<td>%d kWh</td>",$month_calc_mean[$k]*$dates[$k]/1000);
				echo sprintf("<td>%d eur</td>",$day_price + $night_price);
				
				echo sprintf("<td>%d kWh</td>",$pohjateho*($power_night_start-$power_day_start)*$dates[$k]/1000);
				echo sprintf("<td>%d eur</td>",$day_price);
				
				echo sprintf("<td>%d kWh</td>",( $month_calc_mean[$k]-$pohjateho*($power_night_start-$power_day_start))*$dates[$k]/1000);
				echo sprintf("<td>%d eur</td>",$night_price);
				
				echo "</tr>";
				
		
			}
			
			//JOS mennaan seuraavan vuoden puolelle
			for ($k=1 ; $k<$system_month ; ++$k) {
				$night_price = ($month_calc_mean[$k]-$pohjateho*($power_night_start-$power_day_start))*$power_night_price*$dates[$k]/100000;
				
				$day_price = ($pohjateho*($power_night_start-$power_day_start))*$power_day_price*$dates[$k]/100000;
				
				
				$total_night_price += $night_price;
				$total_day_price += $day_price;
				
				date_default_timezone_set('UTC');
				$month_stamp[$k] = mktime(0,0,0,$k,1,$system_year+1);
				
				echo "<tr>";
				echo sprintf("<th class='spec'>%s 0%d</th>",$month_name[$k],$system_year+1-2000);
				echo sprintf("<td>%d kWh</td>",$month_calc_mean[$k]*$dates[$k]/1000);
				echo sprintf("<td>%d eur</td>",$day_price + $night_price);
				
				echo sprintf("<td>%d kWh</td>",$pohjateho*($power_night_start-$power_day_start)*$dates[$k]/1000);
				echo sprintf("<td>%d eur</td>",$day_price);
				
				echo sprintf("<td>%d kWh</td>",( $month_calc_mean[$k]-$pohjateho*($power_night_start-$power_day_start))*$dates[$k]/1000);
				echo sprintf("<td>%d eur</td>",$night_price);
				
				echo "</tr>";
				
		
			}
			
			for ($k=1 ; $k<=12 ; ++$k) {
				$kwh_night = ( $month_calc_mean[$k]-$pohjateho*($power_night_start-$power_day_start))*$dates[$k]/1000;
				
				$kwh_day = $pohjateho*($power_night_start-$power_day_start)*$dates[$k]/1000;
				
				$future_kwh_night_total += $kwh_night;
				$future_kwh_day_total += $kwh_day;
				
			}
			
			$total_price = $total_night_price + $total_day_price;	
			$future_kwh_total = $future_kwh_night_total + $future_kwh_day_total;
			
			
			echo "<tr>";
			echo "<th class='specalt'>YEAR TOTAL</th>";
			echo sprintf("<td class='alt'>%d kWh</td>",$future_kwh_total);
			echo sprintf("<td class='alt'>%d eur</td>",$total_price);
			
			echo sprintf("<td class='alt'>%d kWh</td>",$future_kwh_day_total);
			echo sprintf("<td class='alt'>%d eur</td>",$total_day_price);
			
			echo sprintf("<td class='alt'>%d kWh</td>",$future_kwh_night_total);
			echo sprintf("<td class='alt'>%d eur</td>",$total_night_price);
				
			
			echo "</tr>";	
			
			echo "</table><br/>";
		
			//tulostetaan grafiikka
			$this->printGraphics($month_stamp, $month_calc_mean, $dates);
			
			} else {
				echo "System must have been running at least for 5 full days!";
			}
			
					
		}
	
		private function printGraphics($month_stamp, $month_calc_mean, $dates) 
		{
			
			echo "<div id='center_div' align='center'><div id='chart_year_div' style='width:53.5em; height:19.3em;' ></div></div>";
			
			echo "<script type='text/javascript'>";
			
				//Javascript taulukko
				echo "var d_year = [";
				
				for ($i=1 ; $i <=11 ; ++$i) {
					echo sprintf("[%d000, %d], ", $month_stamp[$i], $month_calc_mean[$i]*$dates[$i]/1000);
				}
				
				echo sprintf("[%d000, %d]", $month_stamp[12], $month_calc_mean[12]*$dates[12]/1000);
				
				echo "];\n";
				
				echo "chart_forecast();";
			
			echo "</script>";
			
		}
	
	}
	
	class LIVE
	{
				
		private $dbObj;		//tietokanta olio
		
		private $db;    	//tietokanta kahva
		private $live_db;   //tietokanta kahva
		private $config_db; //tietokanta kahva
		
		function __construct()
		{
			//luodaan tietokanta olio
			$this->dbObj = new DB();
			
			//avataan tietokanta kahva
			$this->db = $this->dbObj->openDatabase("sqlite:/mnt/mmc/powercalc.db");
			
			//avataan tietokanta kahva
			$this->live_db = $this->dbObj->openDatabase("sqlite:/tmp/live.db");
			
			//avataan tietokanta kahva
			$this->config_db = $this->dbObj->openDatabase("sqlite:/mnt/mmc/config.db");
			
		}
		
		function __destruct()
		{
			
			//NOLLATAAN tietokanta objekti
			unset($this->db);
			
			//NOLLATAAN tietokanta objekti
			unset($this->live_db);
			
		}
		
		public function printOwData()
		{
			$sql_query = "SELECT deviceId, deviceValueCalc FROM ow_live";
			
			//haetaan tiedot taulukkoon
			$row = $this->dbObj->readDatabase($this->live_db, $sql_query);
			
			for ($i = 0; $i < count($row); ++$i) {			
				echo sprintf("<p>Sensor deviceId[%d]: %0.2f </p>", $row[$i]["deviceId"], $row[$i]["deviceValueCalc"]);
			}
		}
		
		public function printCssBar($deviceId)
		{
			
			//maaritetaan paivamaara
			$epoch_exp = date("U") - 86400*1; //minus days
				
			$d1=date("d", $epoch_exp); $m1=date("m", $epoch_exp); $y1=date("Y", $epoch_exp);
			
			$d2=date("d"); $m2=date("m"); $y2=date("Y");
	        
			//SQL komento
			$sql_exp = "SELECT epoch, energy_" . $deviceId ." FROM d" . $d1 . $m1 . $y1 ;
			$sql_today = "SELECT epoch, energy_" . $deviceId ." FROM d" . $d2 . $m2 . $y2 ;
			
			//haetaan tiedot taulukkoon
			$expResult = $this->dbObj->readDatabase($this->db, $sql_exp);
			
			//haetaan tiedot taulukkoon
			$todayResult = $this->dbObj->readDatabase($this->db, $sql_today);
			
			$night = 0; $morning = 6; $afternoon = 12; $evening = 18;
			
			//luetaan så©«ò®¨©nta ja ajat
			$power_day_price=0;
			$power_night_price=0;
			$power_day_start=0;
			$power_night_start=0;
			
			getPriceData($power_day_price, $power_night_price, $power_day_start, $power_night_start);
			
			// Haetaan tietokannasta
			$sql_query = "SELECT deviceMeterConst FROM io_data WHERE deviceId=" . $deviceId;
			
			// haetaan tiedot taulukkoon
			$row = $this->dbObj->readDatabase($this->config_db, $sql_query);
			$meterConst = $row[0]["deviceMeterConst"];
			
			$todayPriceArr["night"] = 0;
			$todayPriceArr["day"] = 0;
			$expPriceArr["night"] = 0;
			$expPriceArr["day"] = 0;
			
			$expArr["night"] = 0;
			$expArr["morning"] = 0;
			$expArr["afternoon"] = 0;
			$expArr["evening"] = 0;
			
			$todayArr["night"] = 0;
			$todayArr["morning"] = 0;
			$todayArr["afternoon"] = 0;
			$todayArr["evening"] = 0;
			
			$sizeToday = count($todayResult);
			$sizeExp = count($expResult);
			
			//jos riittavasti dataa
			if ($sizeToday > 0 && $sizeExp > 0) {
			
				//kulutusennuste kyseiseen kellonaikaan mennessa		
				$energyExp = 0;
				
				for ($i = 0; $i < $sizeExp; ++$i) {
					
					if ($i % 15 == 0) { //NOPEUTUS, tarkistetaan joka viidestoista minuutti				
						$hour=date("G", $expResult[$i]["epoch"]);
					}
					
					// Hinta				
					if ($hour < $power_day_start || $hour >= $power_night_start) {
						$expPriceArr["night"] += $expResult[$i]["energy_" . $deviceId];
						
					} else if ($hour >= $power_day_start && $hour < $power_night_start) {
						$expPriceArr["day"] += $expResult[$i]["energy_" . $deviceId];
					}
					
					// Kulutus					
					if ($hour < $morning) {
						
						$expArr["night"] += $expResult[$i]["energy_" . $deviceId];
						
					} else if ($hour < $afternoon) {
						
						$expArr["morning"] += $expResult[$i]["energy_" . $deviceId];
					
					} else if ($hour < $evening) {
						
						$expArr["afternoon"] += $expResult[$i]["energy_" . $deviceId];
						
					} else {
						
						$expArr["evening"] += $expResult[$i]["energy_" . $deviceId];
						
					}
					
					if ($i < $sizeToday) {
						
						$energyExp += $expResult[$i]["energy_" . $deviceId];
						
					}
					
				}
				
				for ($i = 0; $i < $sizeToday; ++$i) {
					
					if ($i % 15 == 0) { //NOPEUTUS, tarkistetaan joka viidestoista minuutti		
						$hour=date("G", $todayResult[$i]["epoch"]);
					}
					
					// Hinta			
					if ($hour < $power_day_start || $hour >= $power_night_start) {
						$todayPriceArr["night"] += $todayResult[$i]["energy_" . $deviceId];
						
					} else if ($hour >= $power_day_start && $hour < $power_night_start) {
						$todayPriceArr["day"] += $todayResult[$i]["energy_" . $deviceId];
					}
					
					// Kulutus
					if ($hour < $morning) {			
					
						$todayArr["night"] += $todayResult[$i]["energy_" . $deviceId];		
						
					} else if ($hour < $afternoon) {
						
						$todayArr["morning"] += $todayResult[$i]["energy_" . $deviceId];
					
					} else if ($hour < $evening) {
						
						$todayArr["afternoon"] += $todayResult[$i]["energy_" . $deviceId];
						
					} else {
						
						$todayArr["evening"] += $todayResult[$i]["energy_" . $deviceId];
						
					}
					
				}
				
				//lasketaan kokonaiskulutukset
				$energyExpTotal = $expArr["night"] + $expArr["morning"] + $expArr["afternoon"] + $expArr["evening"];
				$energyToday = $todayArr["night"] + $todayArr["morning"] + $todayArr["afternoon"] + $todayArr["evening"];
				
				//maaritetaan palkkien leveydet
				$idealWidth = 55;
				$todayWidth = 0;
				$expWidth = 0;
				
				//laatikko
				echo "<div style='font-size:1.1em;width:30em;background-image: url(/images/transparent.png)'>\n";
				
				if ( ( ($energyToday / $energyExp) - 1.0 ) > 0.005 ) { 
				
					echo sprintf("<p><b>%0.1f%% over</b> expected usage so far today!</p>\n", ($energyToday / $energyExp)*100 - 100);
				
				} else if ( ( ($energyToday / $energyExp) - 1.0 ) < -0.005 ){		
					
					echo sprintf("<p><b>%0.1f%% under</b> expected usage so far today!</p>\n", 100 - ($energyToday / $energyExp)*100);
					
				} else {
					
					echo sprintf("<p><b>About same</b> as expected usage so far today!</p>\n");
									
				}
				
				echo "</div>\n";
				//laatikko
				
				if ($energyToday < $energyExpTotal) {
					
					$todayWidth = round($energyToday/$energyExpTotal*$idealWidth);
					$expWidth = $idealWidth;
					
				} else { //energyToday >=$energyExpTotal
					
					$todayWidth = $idealWidth;
					$expWidth = round($energyExpTotal/$energyToday*$idealWidth);
					
				}
				
				
				//TODAY
				
				$todayBar = "";
			
				$todayBar .= "<div class='energy_bar_holder'>\n";
				
					$todayBar .= "<div class='energy_bar_start'><b>today:</b></div>\n";
			
				    $todayBar .= sprintf("<div class='energy_bar' style='background-color:#A65E00;width:%0.2fem;'>\n",
										  $todayArr["night"]/$energyToday*$todayWidth );
										  
				    $todayBar .= sprintf("<div class='energy_bar_text' >night</div>\n");
				    $todayBar .= sprintf("<span>00-06<br/><b>%0.2f kWh</b></span></div>\n", $todayArr["night"]/$meterConst );
				     
				    $todayBar .= sprintf("<div class='energy_bar' style='background-color:#FF9000;width:%0.2fem;'>\n",
										  $todayArr["morning"]/$energyToday*$todayWidth );
										  
				    $todayBar .= sprintf("<div class='energy_bar_text' >morning</div>\n");
				    $todayBar .= sprintf("<span>06-12<br/><b>%0.2f kWh</b></span></div>\n", $todayArr["morning"]/$meterConst );
				     
				    $todayBar .= sprintf("<div class='energy_bar' style='background-color:#FFAC40;width:%0.2fem;'>\n",
										  $todayArr["afternoon"]/$energyToday*$todayWidth );
										  
				    $todayBar .= sprintf("<div class='energy_bar_text' >afternoon</div>\n");
				    $todayBar .= sprintf("<span>12-18<br/><b>%0.2f kWh</b></span></div>\n", $todayArr["afternoon"]/$meterConst );
				     
				    $todayBar .= sprintf("<div class='energy_bar' style='background-color:#FFC273;width:%0.2fem;'>\n",
										  $todayArr["evening"]/$energyToday*$todayWidth );
										  
				    $todayBar .= sprintf("<div class='energy_bar_text' >evening</div>\n");
				    $todayBar .= sprintf("<span>18-24<br/><b>%0.2f kWh</b></span></div>\n", $todayArr["evening"]/$meterConst );
					
					$todayBar .= sprintf("<div class='energy_bar_end'><b>%0.1f kWh / %0.1f eur</b></div>\n",
										  $energyToday/$meterConst, ($todayPriceArr["night"]*$power_night_price + $todayPriceArr["day"]*$power_day_price)/($meterConst*100) );
					
				$todayBar .= "</div>\n";
				
				echo($todayBar);
				
				//Exp
				
				$expBar = "";
				
				$expBar .= "<div class='energy_bar_holder'>\n";
				
					$expBar .= "<div class='energy_bar_start'>exp:</div>\n";
			
				    $expBar .= sprintf("<div class='energy_bar' style='background-color:#A65E00;width:%0.2fem;'>\n",
										$expArr["night"]/$energyExpTotal*$expWidth );
				    $expBar .= sprintf("<div class='energy_bar_text' >night</div>\n");
				    $expBar .= sprintf("<span>00-06<br/><b>%0.2f kWh</b></span></div>\n", $expArr["night"]/$meterConst );
				     
				    $expBar .= sprintf("<div class='energy_bar' style='background-color:#FF9000;width:%0.2fem;'>\n",
										$expArr["morning"]/$energyExpTotal*$expWidth );
				    $expBar .= sprintf("<div class='energy_bar_text' >morning</div>\n");
				    $expBar .= sprintf("<span>06-12<br/><b>%0.2f kWh</b></span></div>\n", $expArr["morning"]/$meterConst );
				     
				    $expBar .= sprintf("<div class='energy_bar' style='background-color:#FFAC40;width:%0.2fem;'>\n",
										$expArr["afternoon"]/$energyExpTotal*$expWidth );
				    $expBar .= sprintf("<div class='energy_bar_text' >afternoon</div>\n");
				    $expBar .= sprintf("<span>12-18<br/><b>%0.2f kWh</b></span></div>\n", $expArr["afternoon"]/$meterConst );
				     
				    $expBar .= sprintf("<div class='energy_bar' style='background-color:#FFC273;width:%0.2fem;'>\n",
										$expArr["evening"]/$energyExpTotal*$expWidth );
				    $expBar .= sprintf("<div class='energy_bar_text' >evening</div>\n");
				    $expBar .= sprintf("<span>18-24<br/><b>%0.2f kWh</b></span></div>\n", $expArr["evening"]/$meterConst );
					
					$expBar .= sprintf("<div class='energy_bar_end'>%0.1f kWh / %0.1f eur</div>\n", 
										$energyExpTotal/$meterConst, ($expPriceArr["night"]*$power_night_price + $expPriceArr["day"]*$power_day_price)/($meterConst*100) );
					
				$expBar .= "</div>\n";
				
				echo($expBar);
			
			}
		
		}
		
	}
	
	class NIALM
	{
		
		private $dbObj;	//tietokanta olio
		private $nialm_db; //tietokanta kahva
		
		function __construct()
		{			
			//luodaan tietokanta olio
			$this->dbObj = new DB();
			
			//avataan tietokanta kahva
			$this->nialm_db = $this->dbObj->openDatabase("sqlite:/mnt/mmc/nialm.db");
		}
		
		function __destruct()
		{
			
			//NOLLATAAN tietokanta objekti
			unset($this->nialm_db);
			
		}
		
		public function printNialm($max_events)
		{	
			
			//paivamaara
			$j = date("d"); $i = date("m"); $h = date("Y");
	        
			//SQL komento
			$sql_query = "SELECT epoch, power, dP FROM d" . $j . $i . $h . " ORDER BY epoch desc LIMIT " . $max_events;
			
			//haetaan tiedot taulukkoon
			$result = $this->dbObj->readDatabase($this->nialm_db, $sql_query);
				
			$size = count($result); //taulukon koko
			
			if ($size > 0) {
			
				$items = "<div class='RSS'>"; //item lohkot
				
				for ($i = 0; $i < $size; ++$i) {
						
						$epoch = date("D, d M H:i:s", $result[$i]['epoch']);
						$epoch_small = date("H:i:s", $result[$i]['epoch']);
						
						$total_load = $result[$i]['power']+$result[$i]['dP'];
						
						$event_type = "";
				
						if ($result[$i]['dP'] > 0) {
							$event_type = "up: ";
						} else {
							$event_type = "down: ";
						}
						
					    $items .= "\t<div id='item'>\n" .
					    				"\t\t<div id='title'>" . "Load " . $event_type . $result[$i]['dP'] . " W" . "</div>\n" .
										"\t\t<div id='pubDate'>" . $epoch . "</div>\n" .
					              "\t</div>\n";
			    
				}
				
				$items .= "</div>";
				
				//tulostetaan lista			
				echo $items;
			
			}
			
		}		
		
	}
	
	class JS
	{
		
		private $dbObj;	//tietokanta olio
		private $config_db; //tietokanta kahva
		
		function __construct()
		{			
			//luodaan tietokanta olio
			$this->dbObj = new DB();
			
			//avataan tietokanta kahva
			$this->config_db = $this->dbObj->openDatabase("sqlite:/mnt/mmc/config.db");
		}
		
		function __destruct()
		{
			
			//NOLLATAAN tietokanta objekti
			unset($this->config_db);
			
		}
				
		public function printJsObj($db_table, $deviceType)
		{
			$sql_query = NULL;
			
			//Haetaan ow-anturien tiedot
			if ($db_table == "ow_data")
				$sql_query = "SELECT deviceId, deviceName FROM ow_data WHERE deviceType='" . $deviceType . "'";
				
			//Haetaan io tiedot
			if ($db_table == "io_data")
				$sql_query = "SELECT deviceId, deviceName FROM io_data WHERE deviceTypePulse='" . $deviceType . "'";
			
			//haetaan tiedot taulukkoon
			$row = $this->dbObj->readDatabase($this->config_db, $sql_query);
			
			for ($i = 0; $i < count($row); ++$i) {
				
				if ($i == 0) { //eka arvo
					echo sprintf("deviceName['%s'] = [ {value:'%d', text:'[%d] %s'}",
								 $deviceType, $row[$i]["deviceId"], $row[$i]["deviceId"], $row[$i]["deviceName"]);
				}
				
				else { 	//muut rivit
					echo sprintf(", {value:'%d', text:'[%d] %s'}", $row[$i]["deviceId"], $row[$i]["deviceId"], $row[$i]["deviceName"]);
				}
				
				if ($i == count($row) - 1) { //javascript olion lopetusmerkit
					echo "];\n";
				}
			}
			
		}
		
	}
	
?>
