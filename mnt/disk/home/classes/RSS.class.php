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

	class RSS
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
		
		public function getFeed($max_events)
		{
			return $this->getDetails() . $this->getItems($max_events);
		}
		
		private function getDetails()
		{
			
			$now = date("D, d M Y H:i:s O");
			
			$details = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" .
            			"<rss version=\"2.0\">\n" .
                		"<channel>\n" .
	                	"\t<title>deltaBox RSS</title>\n" .
	               		"\t<link>http://zoomzoom.ath.cx/RSS.php</link>\n" .
	                	"\t<description>RSS feed</description>\n" .
						"\t<language>en-us</language>\n" .
						"\t<pubDate>" . $now . "</pubDate>\n" .
					    "\t<lastBuildDate>" . $now . " </lastBuildDate>\n";
			
			return $details;
		}
		
		private function getItems($max_events) {			
			
			//päivämäärä
			$j = date("d"); $i = date("m"); $h = date("Y");
	        
			//SQL komento
			$sql_query = "SELECT epoch, power, dP FROM d" . $j . $i . $h . " ORDER BY epoch desc LIMIT " . $max_events;
			
			//haetaan tiedot taulukkoon
			$result = $this->dbObj->readDatabase($this->nialm_db, $sql_query);
				
			$size = count($result); //taulukon koko
			
			if ($size > 0) {
			
				$items = ""; //item lohkot
				
					for ($i = 0; $i < $size; ++$i) {
						
						$epoch_long = date("D, d M Y H:i:s O", $result[$i]['epoch']);
						$epoch_short = date("H:i:s", $result[$i]['epoch']);
						
						$total_load = $result[$i]['power'] + $result[$i]['dP'];
						
						$event_type = "";
						
						if ($result[$i]['dP'] > 0) {
							$event_type = "up: ";
						} else {
							$event_type = "down: ";
						}
						
					    $items .= "\t<item>\n" .
					    				"\t\t<title>Total load: " . $total_load . "W" . " (" . $epoch_short . ")</title>\n" .
					                	"\t\t<link>http://192.168.1.51/RSS.php</link>\n" .
										"\t\t<description>Load went " . $event_type . $result[$i]['dP'] . " W. Total load after event: " . $total_load . " W (" . $epoch_long . ")" . "</description>\n" .
										"\t\t<pubDate>" . $epoch_long . "</pubDate>\n" .
										"\t\t<guid></guid>\n" .
					              "\t</item>\n";
				               
			        }
			    
				
				
				$items .= "</channel>\n".
					"</rss>\n";
							
				return $items;
			
			}
			
		}
	}
		
?>
