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

	class USERS
	{
		
		//viesti joka printataan sivun lopussa, jos tehtiin muutoksia
		public $update_message = NULL;
		
		function __construct()
		{					
			//paivitetaan uusi data
			$this->deleteUser();
			$this->addUser();
			$this->changePassword();
			$this->changePermissions();		
		}
		
		public function printUsers()
		{			
			$fp = fopen ( '/disk/users.txt', 'r' );
			while ( $line = fgetcsv ($fp, 100, " ")) {			
				echo sprintf("<OPTION>%s</OPTION>", $line[0]);
			}
			
			fclose ( $fp );			
		}
		
		public function printPermissions()
		{			
			$fp = fopen ( '/disk/users.txt', 'r' );
			while ( $line = fgetcsv ($fp, 100, " ")) {		
				echo sprintf("<OPTION>Complex [%s] - Tools [%s] - Settings [%s]</OPTION>", $line[2], $line[3], $line[4]);			
			}
			
			fclose ( $fp );			
		}

		
		private function deleteUser() 
		{
			
		//////////////////////////////USER delete script
		
			if (isset ($_POST['delete_user'])) {
			
				$problem = FALSE;
				
				if (empty ($_POST['old_user'])) {
				  	$problem = TRUE;
					$this->update_message = '<p>Please select a user to remove!</p>';
				
				}
				
				if (($_POST['old_user'])=="admin") {
				  	$problem = TRUE;
					$this->update_message = '<p>You cannot remove admin!</p>';
				
				}
				
				if ( $problem==FALSE) {
					
					//luetaan tiedosto ja kirjoitetaan vå­©aikaistiedosto ilman poistettavaa kåº´tå«¤äŠ					
					$userfile = fopen ( '/disk/users.txt', 'r' );
					$tempfile = fopen ( '/tmp/1234.tmp', 'w' );
					
					while ( $line = fgetcsv ($userfile, 100, " ")) {
						
						$row = $line[0] . " " . $line[1] . " " . $line[2] . " " . $line[3] . " " . $line[4] . "\n";			
							
						if ($line[0]!=$_POST['old_user']) {		
							fwrite ( $tempfile, $row);
						}
				
					}
					
					fclose ( $userfile );
					fclose ( $tempfile );
					
					//kirjoitetaan user tiedosto vå­©aikaistiedostosta
				
					$tempfile = fopen ( '/tmp/1234.tmp', 'r' );	
					$userfile = fopen ( '/disk/users.txt', 'w' );
					
					while ( $line = fgetcsv ($tempfile, 100, " ")) {
						
						$row = $line[0] . " " . $line[1] . " " . $line[2] . " " . $line[3] . " " . $line[4] . "\n";			
								
						fwrite ( $userfile, $row);
				
					}
					
					fclose ( $userfile );
					fclose ( $tempfile );
					
					//remove tempfile
					
					exec ("rm /tmp/1234.tmp");
				
				
					$this->update_message = '<p>User has been removed!</p>';
				
				
				}
			
			}
			
		}

		
		private function addUser() 
		{
			
			///////////////////////ADD User script
			
			if (isset ($_POST['add_user'])) {
						
				$check_complex = 0;
				$check_tools = 0;
				$check_settings = 0;
				$problem = FALSE;
				
				if ($_POST['check_complex']==1) {
					$check_complex = 1;
				}
				if ($_POST['check_tools']==1) {
					$check_tools = 1;
				}
				if ($_POST['check_settings']==1) {
					$check_settings = 1;
				}
				
				
				if (empty ($_POST['user'])) {
				  	$problem = TRUE;
					$this->update_message = '<p>Please enter a username!</p>';
				
				}
				
				if (empty ($_POST['pass'])) {
				  	$problem = TRUE;
					$this->update_message = '<p>Please enter a password</p>';
				
				}
				
				if (($_POST['pass_confirm'])!=($_POST['pass'])) {
				  	$problem = TRUE;
					$this->update_message = '<p>Password/Confirm Password mismatch!</p>';
				
				}
				
				$user_max = 5;
				$i = 0;
				
				$problem2 = FALSE;
				
				//tarkistetaan ettei ylitetä¡«åº´tå«©en maksimimå¥²å¤
				
				if ($fp = fopen ( '/disk/users.txt', 'r+' )) {  //Open the file!
				
					while ( $line = fgetcsv ($fp, 100, " ")) {
						
							++$i;
							if ($i==$user_max) {
								$this->update_message = '<p>Error, maximum amount of users exists!</p>';
								$problem2 = TRUE;
							}
							
							
					}
				}
				
				if ( $problem==FALSE && $problem2==FALSE) {
					
					$user_exist=FALSE;
					 
					if ($fp = fopen ( '/disk/users.txt', 'r+' )) {  //Open the file!
					
						while ( $line = fgetcsv ($fp, 100, " ")) {
							
							if ($line[0]==$_POST['user']) {		
								$user_exist=TRUE;
								
								$this->update_message = '<p>Error, User exists!</p>';
								
							}
						
						}
					
						if ($user_exist==FALSE) {
							
							$data = $_POST['user'] . " " . md5($_POST['pass']) . " " . $check_complex . " " . $check_tools . " " . $check_settings . "\n";
					
							// Write data and close file!
							
							$this->update_message = '<p>User has been added!</p>';
						
							fwrite ( $fp, $data );
					
						}
					
						fclose ( $fp );
					
					} else {
					
					   	$this->update_message = '<p>An error occured!</p>';
					   	
					}
				}		
			}	
		}

		
		//////////////////////////////USER change password script
		private function changePassword() 
		{
			
			if (isset ($_POST['change_password'])) {
				
				$check_complex = 0;
				$check_tools = 0;
				$check_settings = 0;
				$problem = FALSE;
				
				if (empty ($_POST['old_user'])) {
				  	$problem = TRUE;
					$this->update_message = '<p>Please select a user!</p>';
				
				}
				
				if (empty ($_POST['new_pass'])) {
				 	$problem = TRUE;
					$this->update_message = '<p>Empty password error!</p>';
				
				}
				
				if (($_POST['new_pass'])!=($_POST['new_pass_confirm'])) {
				  	$problem = TRUE;
					$this->update_message = '<p>Password / Confirm password mismatch!</p>';
				
				}
				
				if ( $problem==FALSE) {	
					
					//luetaan tiedosto ja kirjoitetaan valiakaistiedosto ilman etta kayttajalle vaihdetaan salasana
					
					$userfile = fopen ( '/disk/users.txt', 'r' );
					$tempfile = fopen ( '/tmp/1234.tmp', 'w' );
					
					while ( $line = fgetcsv ($userfile, 100, " ")) {
						
						$row = $line[0] . " " . $line[1] . " " . $line[2] . " " . $line[3] . " " . $line[4] . "\n";		
							
						if ($line[0]!=$_POST['old_user']) {		//muut kayttajat
							fwrite ( $tempfile, $row);
						}
					
						if ($line[0]==$_POST['old_user']) {		//kayttaja jolle vaihdetaan salasana
							$check_complex = $line[2];
							$check_tools = $line[3];
							$check_settings = $line[4];
							
							$row = $_POST['old_user'] . " " . md5($_POST['new_pass']) . " " . $check_complex . " " . $check_tools . " " . $check_settings . "\n";
							fwrite ($tempfile, $row);
						}
						
					}
					
					fclose ( $userfile );
					fclose ( $tempfile );
					
					//kirjoitetaan user tiedosto valiakaistiedostosta
				
					$tempfile = fopen ( '/tmp/1234.tmp', 'r' );	
					$userfile = fopen ( '/disk/users.txt', 'w' );
					
					while ( $line = fgetcsv ($tempfile, 100, " ")) {
						
						$row = $line[0] . " " . $line[1] . " " . $line[2] . " " . $line[3] . " " . $line[4] . "\n";			
							
						fwrite ( $userfile, $row);
				
					}	
					
					fclose ( $userfile );
					fclose ( $tempfile );
					
					//remove temp file
					exec ("rm /tmp/1234.tmp");
					
					$this->update_message = '<p>Password has been changed!</p>';
				
				}			
			}
		}
		
		//////////////////////////////USER change permissions script
		
		private function changePermissions()
		{
		
			if (isset ($_POST['change_permissions'])) {
				
				$new_complex = 0;
				$new_tools = 0;
				$new_settings = 0;
				$problem = FALSE;
				
				if ($_POST['new_complex']==1) {
					$new_complex = 1;
				}
				if ($_POST['new_tools']==1) {
					$new_tools = 1;
				}
				if ($_POST['new_settings']==1) {
					$new_settings = 1;
				}
				
				if (empty ($_POST['old_user'])) {
				  	$problem = TRUE;
				  
					$this->update_message = '<p>Please select a user!</p>';
				
				}
				
				if (($_POST['old_user'])=="admin") {
				  	$problem = TRUE;
				  
					$this->update_message = '<p>Cannot change admin permissions!</p>';
				
				}
				
				if ( $problem==FALSE) {	
					
					//luetaan tiedosto ja kirjoitetaan valiaikaistiedosto ilman kayttajalle vaihdetaan oikeuksia
					
					$userfile = fopen ( '/disk/users.txt', 'r' );
					$tempfile = fopen ( '/tmp/1234.tmp', 'w' );
					
					while ( $line = fgetcsv ($userfile, 100, " ")) {
						
						$row = $line[0] . " " . $line[1] . " " . $line[2] . " " . $line[3] . " " . $line[4] . "\n";		
							
						if ($line[0]!=$_POST['old_user']) {		//muut kåº´tå«¤t
							fwrite ( $tempfile, $row);
						}
					
						if ($line[0]==$_POST['old_user']) {		//kayttaja jolle vaihdetaan oikeudet
							
							$row = $line[0] . " " . $line[1] . " " . $new_complex . " " . $new_tools . " " . $new_settings . "\n";
							fwrite ($tempfile, $row);
						}
						
					}
					
					fclose ( $userfile );
					fclose ( $tempfile );
					
					//kirjoitetaan user tiedosto valiaikaistiedostosta
				
					$tempfile = fopen ( '/tmp/1234.tmp', 'r' );	
					$userfile = fopen ( '/disk/users.txt', 'w' );
					
					while ( $line = fgetcsv ($tempfile, 100, " ")) {
						
						$row = $line[0] . " " . $line[1] . " " . $line[2] . " " . $line[3] . " " . $line[4] . "\n";			
							
						fwrite ( $userfile, $row);
				
					}	
					
					fclose ( $userfile );
					fclose ( $tempfile );
					
					//remove tempfile
					exec ("rm /tmp/1234.tmp");
				
					$this->update_message = '<p>Permissions has been changed!</p>';
				
				}
			}
		}
	}
	
	class PRICE
	{		
		//viesti joka printataan sivun lopussa, jos tehtiin muutoksia
		public $update_message = NULL;
		
		function __construct()
		{							
			//päivitetään uusi data
			$this->submitData();
			
			//tulostetaan data
			$this->printData();		
			
		}
		
		private function submitData()
		{
			
			if (isset ($_POST['submit_power'])) {
	
				$problem = FALSE;
				
				if (($_POST['power_day_price'])<7) {
					$problem = TRUE;
					echo '<p>Bad energy day price!</p>';
				}
				if (($_POST['power_night_price'])<5) {
					$problem = TRUE;
					echo '<p>Bad energy night price!</p>';
				}
				if (($_POST['power_day_start'])>9) {
					$problem = TRUE;
					echo '<p>Bad daytime start!</p>';
				}
				if (($_POST['power_night_start'])<21) {
					$problem = TRUE;
					echo '<p>Bad nightime start!</p>';
				}
				
				if ( !$problem ) {
				 
					if ($fp = fopen ( '/disk/powercalc.conf', 'w' )) {  //Open the file!
					
						$row[0] = "power_day_price" . " " . $_POST['power_day_price'] . "\n";
						$row[1] = "power_night_price" . " " . $_POST['power_night_price'] . "\n";
						$row[2] = "power_day_start" . " " . $_POST['power_day_start'] . "\n";
						$row[3] = "power_night_start" . " " . $_POST['power_night_start'] . "\n";
						
						// Write data and close file!
						
						for ($i=0 ; $i < sizeof($row) ; $i++) {
							fwrite ( $fp, $row[$i] );
						}
						fclose ( $fp );
						
						$this->update_message = '<p>Settings updated!</p>';
					
					}
				}
			}
		}
		
		public $power_day_price=0;
		public $power_night_price=0;
		public $power_day_start=0;
		public $power_night_start=0;
			
		private function printData()
		{
			
			$fp = fopen ( '/disk/powercalc.conf', 'r' );
			
			while ( $line = fgetcsv ($fp, 100," ")) {
			
				if ($line[0] == "power_day_price") {	
					$this->power_day_price=$line[1];
				}
				if ($line[0] == "power_night_price") {	
					$this->power_night_price=$line[1];
				}
				if ($line[0] == "power_day_start") {	
					$this->power_day_start=$line[1];
				}
				if ($line[0] == "power_night_start") {	
					$this->power_night_start=$line[1];
				}
			
			}
			fclose ( $fp );
		}	
	}
	
	
	
	class TIMEZONE
	{
		//viesti joka printataan sivun lopussa, jos tehtiin muutoksia
		public $update_message = NULL;
		
		function __construct()
		{							
			//päivitetään uusi data
			$this->submitData();
			
			//tulostetaan data
			$this->printData();
		}
		
		private function submitData()
		{
			if (isset ($_POST['submit_time'])) {
				
				$problem = FALSE;
				
				if (empty ($_POST['NTP_server1']) || empty ($_POST['NTP_server2']) || empty ($_POST['NTP_server3']) || empty ($_POST['NTP_server4'])) {
				  $problem = TRUE;
					echo '<p>Error! Empty NTP_server Server address!</p>';
				
				}
				
				if ( !$problem ) {
				 
					if ($fp = fopen ( '/disk/web_interface.conf', 'w' )) {  //Open the file!
					
						$row1 = "time_zone " . $_POST['time_zone'] . "\n";
						$row2 = "NTP_server1 " . $_POST['NTP_server1'] . "\n";
						$row3 = "NTP_server2 " . $_POST['NTP_server2'] . "\n";
						$row4 = "NTP_server3 " . $_POST['NTP_server3'] . "\n"; 
						$row5 = "NTP_server4 " . $_POST['NTP_server4'] . "\n";
						
						// Write data and close file!
						
						fwrite ( $fp, $row1 );
						fwrite ( $fp, $row2 );
						fwrite ( $fp, $row3 );
						fwrite ( $fp, $row4 );
						fwrite ( $fp, $row5 );
						fclose ( $fp );
						
						$this->update_message = '<p>Time settings updated!</p>';
						
					}
				}
			}
		}
			
		
		public $time_zone="null";
		public $NTP_server1="null";
		public $NTP_server2="null";
		public $NTP_server3="null";
		public $NTP_server4="null";
			
		private function printData()
		{	
			//luetaan muuttujat lomakkeeseen
			$fp = fopen ( '/disk/web_interface.conf', 'r' );
			
			while ( $line = fgetcsv ($fp, 100," ")) {
			
				if ($line[0] == "time_zone") {					
					$this->time_zone=$line[1];
				}
				
				if ($line[0] == "NTP_server1") {				
					$this->NTP_server1=$line[1];				
				}
				
				if ($line[0] == "NTP_server2") {					
					$this->NTP_server2=$line[1];				
				}
				
				if ($line[0] == "NTP_server3") {					
					$this->NTP_server3=$line[1];				
				}
				
				if ($line[0] == "NTP_server4") {					
					$this->NTP_server4=$line[1];				
				}				
			}			
		}	
		
		//kesken
		public function printTimezone()
		{
			
			$selected = "";
          
          	$value_array = array("Europe/Helsinki", "Europe/London", "Europe/Rome");
          						
          	$name_array = array("Europe/Helsinki", "Europe/London", "Europe/Rome");    
          						      
          						
          	for ($i=0;$i<sizeof($value_array);++$i) {
          	
	          	if ($time_zone==$value_array[$i]) {
		          	$selected = "selected='selected'";
	          	} else {
		          	$selected = "";
	          	}
	          	
 				echo sprintf("<option value='%s' %s>%s</option> \n", $value_array[$i], $selected, $name_array[$i]);
 			
			}
		}	
	}
	
	class NETWORK
	{
		
		//viesti joka printataan sivun lopussa, jos tehtiin muutoksia
		public $update_message = NULL;
		
		function __construct()
		{									
			//päivitetään uusi data
			$this->submitData();
			
			//tulostetaan data
			$this->printData();
		}
		
		private function submitData()
		{	
			
			if (isset ($_POST['submit_network'])) {
		
				$problem = FALSE;
				
				if (empty ($_POST['hostname'])) {
					$problem = TRUE;
					echo '<p>Error! Empty hostname!</p>';
				}
				
				if (($_POST['ip_address'])!="192.168.1.51") {
					$problem = TRUE;
					echo '<p>Bad ip-address!</p>';
				}
				
				if (($_POST['subnet_mask'])!="255.255.255.0") {
					$problem = TRUE;
					echo '<p>Bad subnet mask!</p>';
				}
				
				if (($_POST['gateway'])!="192.168.1.5") {
					$problem = TRUE;
					echo '<p>Bad gateway IP!</p>';
				}
				
				if (($_POST['dns_server'])!="192.168.1.5") {
					$problem = TRUE;
					echo '<p>Bad dns server IP</p>';
				}
				
				if ( !$problem ) {
				 
					if ($fp = fopen ( '/disk/eth0.conf', 'w' )) {  //Open the file!
					
						$row1 = "hostname " . $_POST['hostname'] . "\n";
						$row2 = "ifconfig eth0 " . $_POST['ip_address'] . " " . "netmask " . $_POST['subnet_mask'] . "\n";
						$row3 = "route add default gw " . $_POST['gateway'] . "\n";
						$row4 = "nameserver" . " " . $_POST['dns_server'] . "\n";
						
						// Write data and close file!
						
						fwrite ( $fp, $row1 );
						fwrite ( $fp, $row2 );
						fwrite ( $fp, $row3 );
						fclose ( $fp );
						
						if ($fp2 = fopen ( '/etc/resolv.conf', 'w' )) {  //Open the file!
						
						fwrite ( $fp2, $row4 );
						fclose ( $fp2 );
						}
						
						$this->update_message = "<p>Settings updated!</p>";
						
						//run new settings
						exec ("/disk/eth0.conf");
					
					}
				}			
			}			
		}
		
		//default values
		public $hostname="delta-Box";
		public $ip_address="192.168.1.1";
		public $subnet_mask="255.255.255.0";
		public $gateway="192.168.1.0";
		public $dns_server="192.168.1.0";
		
		private function printData()
		{ 
			
			$fp = fopen ( '/disk/eth0.conf', 'r' );
			
			while ( $line = fgetcsv ($fp, 100," ")) {
			
				if ($line[0] == "hostname") {				
					$this->hostname=$line[1];				
				}
				
				if ($line[0] == "ifconfig") {				
					$this->ip_address=$line[2];
					$this->subnet_mask=$line[4];			
				}
				
				if ($line[0] == "route") {			
					$this->gateway=$line[4];				
				}			
			}
			
			fclose ( $fp );
			
			if ($fp2 = fopen ( '/etc/resolv.conf', 'r' )) {
			
				$line = fgetcsv ($fp2, 100," ");
				
				$this->dns_server = $line[1];
				
				fclose ( $fp2 );
			
			}		
		}
	}
	
	
	class OW
	{
			
		private $dbObj;	//tietokanta olio
		private $config_db; //tietokanta kahva
		
		//viesti joka printataan sivun lopussa, jos tehtiin muutoksia
		public $update_message = NULL;
		
		function __construct()
		{			
			//luodaan tietokanta olio
			$this->dbObj = new DB();
			
			//avataan tietokanta kahva
			$this->config_db = $this->dbObj->openDatabase("sqlite:/mnt/mmc/config.db");
			
			//paivitetaan uusi data
			$this->submitData();
			
			//paivitetaan cross-swap
			$this->crossSwap();
		}
		
		function __destruct()
		{
			//NOLLATAAN tietokanta objekti
			unset($this->config_db);
		}
		
		
		private function submitData() {
			
			if (isset ($_POST['submit_data'])) {
				
				//Haetaan lampoanturien tiedot
				$sql_query = "SELECT deviceId FROM ow_data";
				
				//haetaan tiedot taulukkoon
				$row = $this->dbObj->readDatabase($this->config_db, $sql_query);
	
				for ($i=0; $i<$_POST['device_count'] ; ++$i) {	
					 
					$deviceName = $_POST["deviceName_" . $row[$i]['deviceId']];
							
					if ($deviceName == NULL) {			
						$deviceName = "not_defined";				
					}
			
					if ( isset ($_POST["deviceLog_" . $row[$i]['deviceId']]) ) {						
						$deviceLog = $_POST["deviceLog_" . $row[$i]['deviceId']];	
					}	else {						
						$deviceLog = 0;
					}
					
					$sql_query = sprintf("UPDATE ow_data SET deviceName='%s', deviceLog='%d' WHERE deviceId='%d'", $deviceName, $deviceLog, $row[$i]['deviceId']);
					
					//kirjoitetaan tietokanta
					$this->dbObj->writeDatabase($this->config_db, $sql_query);		
				
				}
				
				$this->update_message = "<p>Device Settings updated!</p>";		
			}
		}
		
		private function crossSwap() {
			
			if (isset ($_POST['swap_data'])) {
	
				$deviceId_A = $_POST['deviceId_A'];
				$deviceId_B = $_POST['deviceId_B'];				
			
				//HAETAAN LAITTEEN(A) TIEDOT	
				$sql_query = sprintf("SELECT deviceActive, deviceHex, deviceType, deviceName, deviceLog FROM ow_data WHERE deviceId='%d'", $deviceId_A);
				
				//haetaan tiedot taulukkoon
				$row_A = $this->dbObj->readDatabase($this->config_db, $sql_query);
				
				//HAETAAN LAITTEEN(B) TIEDOT	
				$sql_query = sprintf("SELECT deviceActive, deviceHex, deviceType, deviceName, deviceLog FROM ow_data WHERE deviceId='%d'", $deviceId_B);
				
				//haetaan tiedot taulukkoon
				$row_B = $this->dbObj->readDatabase($this->config_db, $sql_query);						
				
				//SIJOITETAAN A:han B:n arvot
				$sql_writeA = sprintf("UPDATE ow_data SET deviceActive='%d', deviceHex='%s', deviceType='%s', deviceName='%s', deviceLog='%d' WHERE deviceId='%d'", 
									$row_B[0]['deviceActive'] , $row_B[0]['deviceHex'], $row_B[0]['deviceType'], $row_B[0]['deviceName'], $row_B[0]['deviceLog'], $deviceId_A);
				
				//kirjoitetaan tietokanta
				$this->dbObj->writeDatabase($this->config_db, $sql_writeA);	
				
				//SIJOITETAAN B:hen A:n arvot	
				$sql_writeB = sprintf("UPDATE ow_data SET deviceActive='%d', deviceHex='%s', deviceType='%s', deviceName='%s', deviceLog='%d' WHERE deviceId='%d'", 
									$row_A[0]['deviceActive'], $row_A[0]['deviceHex'], $row_A[0]['deviceType'], $row_A[0]['deviceName'], $row_A[0]['deviceLog'],  $deviceId_B);
				
				//kirjoitetaan tietokanta
				$this->dbObj->writeDatabase($this->config_db, $sql_writeB);	

				$this->update_message = "<p>Cross-Swap done!</p>";			
			}
		}
		
		public function printData()
		{
			
			//Haetaan onewire anturien tiedot
			$sql_query = "SELECT deviceId, deviceActive, deviceHex, deviceType, deviceName, deviceLog FROM ow_data";
				
			//haetaan tiedot taulukkoon
			$row = $this->dbObj->readDatabase($this->config_db, $sql_query);

			//taulukon alku
			echo ("<table id='mytable'><tr><th>ID</th><th>ACTIVE</th><th>HEX</th><th>TYPE</th><th>NAME</th><th>LOG</th></tr>");
			
			for ($i = 0; $i < count($row); ++$i) {
					
					echo ("<tr>");			
					echo sprintf("<td><input maxlength='2' size='2' disabled='disabled' name='deviceId_%d' id='deviceId_%d' value='%d'/></td>", $row[$i]["deviceId"], $row[$i]["deviceId"], $row[$i]["deviceId"] );
					
					$deviceActiveChecked = "";
							
					if ($row[$i]["deviceActive"]==1) {
						$deviceActiveChecked= "checked='checked'";
					}
					
					echo sprintf("<td><input disabled='disabled' type='checkbox' %s name='deviceActive_%d' id='deviceActive_%d' value='1'/></td>", $deviceActiveChecked, $row[$i]["deviceId"], $row[$i]["deviceId"] );
					echo sprintf("<td><input size='40' disabled='disabled' type='text' name='deviceHex_%d' id='deviceHex_%d' value='%s'/></td>", $row[$i]["deviceId"], $row[$i]["deviceId"], $row[$i]["deviceHex"] );
					echo sprintf("<td><input size='8' disabled='disabled' type='text' name='deviceType_%d' id='deviceType_%d' value='%s'/></td>", $row[$i]["deviceId"], $row[$i]["deviceId"], $row[$i]["deviceType"] );
					echo sprintf("<td><input maxlength='12' size='12' type='text' name='deviceName_%d' id='deviceName_%d' value='%s'/></td>", $row[$i]["deviceId"], $row[$i]["deviceId"], $row[$i]["deviceName"] );
					
					$deviceLogChecked = "";
							
					if ($row[$i]["deviceLog"]==1) {
						$deviceLogChecked= "checked='checked'";
					}
					
					echo sprintf("<td><input type='checkbox' %s name='deviceLog_%d' id='deviceLog_%d' value='1' /></td>", $deviceLogChecked, $row[$i]["deviceId"], $row[$i]["deviceId"] );	
					echo ("</tr>");
					
			}
				
			echo sprintf("<input type='hidden' name='device_count' id='device_count' value='%d'/>", count($row));
			
			//taulukon loppu
			echo ("<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td><input type='submit' class='push_button' name='submit_data' value='Save' /></td><td>&nbsp;</td></tr></table>");
		
		}
		
		public function printSwapData() {
			
			//Haetaan onewire anturien tiedot
			$sql_query = "SELECT deviceId FROM ow_data";
				
			//haetaan tiedot taulukkoon
			$row = $this->dbObj->readDatabase($this->config_db, $sql_query);

			//VALINTA A
			echo ("<tr><td align='center'><SELECT name='deviceId_A' id='deviceId_A'>");
			
			for ($i = 0; $i < count($row); ++$i) {	
				echo sprintf("<OPTION value='%d'>%d</OPTION>", $row[$i]["deviceId"], $row[$i]["deviceId"]);	
			}
			
			echo("</SELECT></td>");
			
			echo("<td align='center'>X</td>");
			
			//VALINTA B
			echo ("<td align='center'><SELECT name='deviceId_B' id='deviceId_B'>");
			
			for ($i = 0; $i < count($row); ++$i) {
				echo sprintf("<OPTION value='%d'>%d</OPTION>", $row[$i]["deviceId"], $row[$i]["deviceId"]);
			}
			
			echo("</SELECT></td></tr>");
		}
	}

	
	class IO
	{
		
		private $dbObj;	//tietokanta olio
		private $config_db; //tietokanta kahva
		
		//viesti joka printataan sivun lopussa, jos tehtiin muutoksia
		public $update_message = NULL;
		
		function __construct()
		{			
			//luodaan tietokanta olio
			$this->dbObj = new DB();
			
			//avataan tietokanta kahva
			$this->config_db = $this->dbObj->openDatabase("sqlite:/mnt/mmc/config.db");
			
			//paivitetaan uusi data
			$this->submitData();
		}
		
		function __destruct()
		{
			//NOLLATAAN tietokanta objekti
			unset($this->config_db);
		}

		public function printData() {
	      
			//Haetaan lampoanturien tiedot
			$sql_query = "SELECT deviceId, deviceActive, devicePort, deviceTypePulse, deviceLog, deviceName, deviceMeterConst, deviceCalcTime, deviceDelta  FROM io_data";
						
			//haetaan tiedot taulukkoon
			$row = $this->dbObj->readDatabase($this->config_db, $sql_query);

			for ($i = 0; $i < count($row); ++$i) {
				
				echo ("<tr>");			
				echo sprintf("<td><input size='2' disabled='disabled' name='deviceId_%d' id='deviceId_%d' value='%d'/></td>", $row[$i]["deviceId"], $row[$i]["deviceId"], $row[$i]["deviceId"] );
				
				$deviceActiveChecked = "";
						
				if ($row[$i]["deviceActive"]==1) {
					$deviceActiveChecked= "checked='checked'";
				}
				
				echo sprintf("<td><input type='checkbox' %s name='deviceActive_%d' id='deviceActive_%d' value='1'/></td>", $deviceActiveChecked, $row[$i]["deviceId"], $row[$i]["deviceId"] );
				echo sprintf("<td><input maxlength='2' size='2'  type='text' name='devicePort_%d' id='devicePort_%d' value='%s'/></td>", $row[$i]["deviceId"], $row[$i]["deviceId"], $row[$i]["devicePort"] );
				
				$energyChecked = "";
				$waterChecked = "";
				$heatChecked = "";
				$oilChecked = "";
						
				if ($row[$i]["deviceTypePulse"]=="energy") {
					$energyChecked = "selected='selected'";
				}
				else if ($row[$i]["deviceTypePulse"]=="water") {
					$waterChecked = "selected='selected'";
				}
				else if	($row[$i]["deviceTypePulse"]=="heat") {
					$heatChecked = "selected='selected'";
				}
				else if	($row[$i]["deviceTypePulse"]=="oil") {
					$oilChecked = "selected='selected'";
				}
				
				echo sprintf("<td><SELECT name='deviceTypePulse_%d' id='deviceTypePulse_%d'><OPTION %s value='energy'>energy</OPTION><OPTION %s value='water'>water</OPTION>
				<OPTION %s value='heat'>heat</OPTION></OPTION><OPTION %s value='oil'>oil</OPTION></SELECT></td>", $row[$i]["deviceId"], $row[$i]["deviceId"], $energyChecked, $waterChecked, $heatChecked, $oilChecked);
				
				echo sprintf("<td><input maxlength='15' size='15' type='text' name='deviceName_%d' id='deviceName_%d' value='%s'/></td>", $row[$i]["deviceId"], $row[$i]["deviceId"], $row[$i]["deviceName"] );
				echo sprintf("<td><input maxlength='6' size='6' type='text' name='deviceMeterConst_%d' id='deviceMeterConst_%d' value='%d'/></td>", $row[$i]["deviceId"], $row[$i]["deviceId"], $row[$i]["deviceMeterConst"] );
				echo sprintf("<td><input maxlength='3' size='3' type='text' name='deviceCalcTime_%d' id='deviceCalcTime_%d' value='%d'/></td>", $row[$i]["deviceId"], $row[$i]["deviceId"], $row[$i]["deviceCalcTime"] );
				echo sprintf("<td><input maxlength='3' size='3' type='text' name='deviceDelta_%d' id='deviceDelta_%d' value='%d'/></td>", $row[$i]["deviceId"], $row[$i]["deviceId"], $row[$i]["deviceDelta"] );
								
				$deviceLogChecked = "";
						
				if ($row[$i]["deviceLog"]==1) {
					$deviceLogChecked= "checked='checked'";
				}
				
				echo sprintf("<td><input type='checkbox' %s name='deviceLog_%d' id='deviceLog_%d' value='1' /></td>", $deviceLogChecked, $row[$i]["deviceId"], $row[$i]["deviceId"] );	
				echo ("</tr>");
					
			}
				
			//tulostetaan lopuksi laitteiden lukumå¥²ä¡°iilotettuna
			echo sprintf("<input type='hidden' name='device_count' id='device_count' value='%d'/>", count($row) );
		}
	
		private function submitData() {
			
			if (isset ($_POST['submit_data'])) {
				
				//Haetaan ensimmaiseksi laitteiden ID:T
				$sql_query = "SELECT deviceId FROM io_data";
						
				//haetaan tiedot taulukkoon
				$row = $this->dbObj->readDatabase($this->config_db, $sql_query);
				
				for ($i=0; $i<$_POST['device_count'] ; ++$i) {
					
					if ( isset ($_POST["deviceActive_" . $row[$i]['deviceId']]) ) {						
						$deviceActive = $_POST["deviceActive_" . $row[$i]['deviceId']];	
					}	else {						
						$deviceActive = 0;
					}
					
					$devicePort = $_POST["devicePort_" . $row[$i]['deviceId'] ];
							
					if ($devicePort == NULL) {			
						$devicePort = 8;				
					}
					
					$deviceTypePulse = $_POST["deviceTypePulse_" . $row[$i]['deviceId'] ];
					
					$deviceName = $_POST["deviceName_" . $row[$i]['deviceId'] ];
							
					if ($deviceName == NULL) {			
						$deviceName = "not_defined";				
					}
					
					$deviceMeterConst = $_POST["deviceMeterConst_" . $row[$i]['deviceId'] ];
					
					if ($deviceMeterConst == NULL) {			
						$deviceMeterConst = 1000;						
					}
					
					$deviceCalcTime = $_POST["deviceCalcTime_" . $row[$i]['deviceId'] ];
					
					if ($deviceCalcTime == NULL) {			
						$deviceCalcTime = 7;						
					}
					
					$deviceDelta = $_POST["deviceDelta_" . $row[$i]['deviceId'] ];
					
					if ($deviceDelta == NULL) {			
						$deviceDelta = 25;						
					}
					
					if ( isset ($_POST["deviceLog_" . $row[$i]['deviceId']]) ) {						
						$deviceLog = $_POST["deviceLog_" . $row[$i]['deviceId']];	
					}	else {						
						$deviceLog = 0;
					}
					
					$sql_query = sprintf("UPDATE io_data SET deviceActive='%d', devicePort='%d', deviceTypePulse='%s', deviceName='%s', deviceMeterConst='%d', deviceCalcTime='%d', deviceDelta='%d', deviceLog='%d' WHERE deviceId='%d'", 
										$deviceActive, $devicePort, $deviceTypePulse, $deviceName, $deviceMeterConst, $deviceCalcTime, $deviceDelta, $deviceLog, $row[$i]['deviceId']);
									
					//suoritetaan kasky
					$this->dbObj->writeDatabase($this->config_db, $sql_query);
							
				}
				
				$this->update_message = "<p>Settings updated!</p>";
			}
		}
	}
	
?>
