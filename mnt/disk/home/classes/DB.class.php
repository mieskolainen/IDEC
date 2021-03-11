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

	class DB
	{
		
		public function openDatabase($database_file)
		{
			//Avataan SQLite tietokanta			
			try {
				$db = new PDO($database_file);				
			}
			catch ( PDOException $e ) {
				die ( $e->getMessage() );
			}
			
			return $db;
		}
		
		public function readDatabase($db, $sql_query )
		{
		
			//valmistellaan käsky
			if ( $sql = $db->prepare($sql_query) ) {
			
				//suoritetaan käsky
				$sql->execute();
				
				// palautettava taulukko
				$result = NULL;
				
				// fetch-metodi palauttaa aina uuden rivin taulukkona
				while($row = $sql->fetch() ) {					
					$result[] = $row;					
				}
				
				return $result;
				
			} else {					
				return (NULL);
			}
		}
		
		public function writeDatabase($db, $sql_query )
		{
			
			if ( $sql = $db->prepare($sql_query) ) {
						
				//suoritetaan käsky
				$sql->execute();
			
			} else { 
				return (NULL);
			}
		}
		
	}
		
?>
