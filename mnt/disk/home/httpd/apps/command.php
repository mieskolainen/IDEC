<?php
	//haetaan luokat
	require_once("/disk/home/classes/MAIN.class.php");

	$command = $_GET['com'];
	
	if ($command == "get_log") {
			
		$logitiedosto = $_GET['log_type'];
		
		$clear = $_GET['clear'];
		
		if ($clear == "true") { //tyhjennetään tiedosto, jos clear==yes
			
			if ($fp = @fopen ("/tmp/$logitiedosto", 'w' ) ) {
				fclose ( $fp );
			}
			
		}	
		
		if ( $tiedosto = @file("/tmp/$logitiedosto") ) { //luetaan tiedosto taulukkoon
		
			echo "<textarea name='textarea' id='log_area' cols='100' rows='30'>";
			
			//tulostetaan tiedosto taulukon rivit
			for ($i=0; $i<count($tiedosto); $i++) {
				
				echo  $tiedosto[$i];
				
			}
			
			echo "</textarea>";
		
		} else {
			echo "<p>Empty log file!</p>";
		}
  		
	}
	
	else if ($command == "get_sensors") {
		
		//haetaan luokat
		require_once( "/disk/home/classes/MAIN.class.php");
	
		//luodaan asetusoliot
		$jsObj = new Live();
		
		// Onewire
		$jsObj->printOwData();
	}
	
	else if ($command == "garage_push_output") {
		
		$io1tila=exec("gpioctl -i 1");
		$nyt = date("H:i:s");
		
		echo "Command sent: ".$nyt." ";
		
	    if ($io1tila == "GPIO1 -> State:Low, Mode:Output") {
		    exec("gpioctl -i 1 -s 1");
		    
		    sleep(2);
		    exec("gpioctl -i 1 -s 0");
	    }
	
		$IP=getenv("REMOTE_ADDR");
		$logged_string = "$IP | " . "Garage    | ". date("d.m.y H:i:s");
		$file = fopen("/tmp/door_control.log", "a");
		fputs($file, $logged_string, strlen($logged_string));
		fputs($file, "\r\n");
		fclose($file);
	  		
	}
	
	else if ($command == "garage_input") {
		
		$tila=exec("gpioctl -i 16");
		
	    if ($tila == "GPI16 -> State:Low, Mode:Input") {
		    
			echo "<p></p>";    
		    echo "Garage Door Open!";
	
	    } else {
		    
			echo "<p></p>";    
		    echo "Garage Door closed!";
	
	    }
  		
	}
	
	else if ($command == "front_push_output") {
		
		$tila=exec("gpioctl -i 0");
		
	    if ($tila == "GPIO0 -> State:Low, Mode:Output") {
	    
		    exec("gpioctl -i 0 -s 1");
	    	$komento="F. Unlock";
	    
	    } else {
		    exec("gpioctl -i 0 -s 0");
		    $komento="F. Lock  ";
	    }
	    
		$nyt = date("H:i:s");
		
		echo "Command sent: ".$nyt." ";
		
		$IP=getenv("REMOTE_ADDR");
		$logged_string = "$IP | " . "$komento | ". date("d.m.y H:i:s");
		$file = fopen("/tmp/door_control.log", "a");
		fputs($file, $logged_string, strlen($logged_string));
		fputs($file, "\r\n");
		fclose($file);
  		
	}
	
	else if ($command == "front_io") {
		
		$tila = exec("gpioctl -i 0");
		
	    if ($tila == "GPIO0 -> State:Low, Mode:Output") {
		    
	    	echo "<p></p>";
	    	echo "Locked (Web Control)<br/>";
	
	    } else {
		    
	    	echo "<p></p>";
	    	echo "Unlocked! (Web Control)<br/>";
	
	    }
	    
	    $tila = exec("gpioctl -i 15");
		
	    if ($tila == "GPIO15 -> State:Low, Mode:Input") {
			
		    echo "<p></p>";    
		    echo "Unlocked! (Relay circuit)";
	
	    } else {
		    
			echo "<p></p>";    
		    echo "Locked (Relay circuit)";
	
	    }
		
	}
	
	else if ($command == "ow_data") {
			
		//haetaan luokat
		require_once("/home/classes/SETTINGS.class.php");

		//luodaan asetusolio
		$owObj = new OW();
		
		//printataan OW-taulukot
		$owObj->printData();
  		
	}

	else if ($command == "uptime") {
		
	  	$result = exec ("uptime");
  		echo $result;
  		
	}
	
	else if ($command == "sidebar") {
		
		//luodaan olio
		$nialmObj = new NIALM();
		$nialmObj->printNialm(10);
		
	}		
	
	else if ($command == "css_bar") {
		
		$deviceId = $_GET['deviceId'];
		$liveObj = new LIVE();
		$liveObj->printCssBar($deviceId);
		
	}
	
	else if ($command == "cpu") {
		
	  	$result = exec ("cpu");
  		echo $result;
  		
	}	
	
	else if ($command == "reboot") {
		
		session_destroy();
		
		//käynnistetään kone
		exec ("reboot");
		
	}


?>