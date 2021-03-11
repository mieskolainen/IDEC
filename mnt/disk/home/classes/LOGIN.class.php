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

	class LOGIN
	{
		public function getIpSpace() 
		{	
			//C-class ip script
			$IP_addr = explode('.', $_SERVER['REMOTE_ADDR']); //write $IP sub number array
			array_pop($IP_addr); //remote last sub number
			$IP_addr = implode('.', $IP_addr); //write $IP string
			
			if ($IP_addr=="192.168.10") {
				$this->homeUser();			
			}
		}
		
		private function homeUser()
		{
			
			session_start();
			
			// this sets variables in the session
			$_SESSION['un']='1';
			$_SESSION['c']='1';
			$_SESSION['t']='0';
			$_SESSION['s']='1';	
			
			$this->passMatch();
			
		}
		
		public function checkLogin()
		{
			
			$loggedin = FALSE;
			
			$fp = fopen ( '/disk/users.txt', 'r' );
			
			while ( $line = fgetcsv ($fp, 100, " ")) {
			
				if ( ($line[0] == $_POST['username']) AND ($line[1] == md5($_POST['password']) )) {
				
					$loggedin = TRUE;
					
					session_start(); 
					
					// this sets variables in the session
					$_SESSION['un']='1';
					$_SESSION['c']=$line[2];
					$_SESSION['t']=$line[3];
					$_SESSION['s']=$line[4];
					
					fclose ( $fp );
					
					break;
				
				}
			
			}
			
			if ($loggedin) {					
				$this->passMatch();
			
			} else { //pass didnt match		
				$this->passError();			
			}

			
		}
		
		private function passMatch()
		{
			
			//users_ip to log
			$IP=$_SERVER['REMOTE_ADDR'];
			$logged_string = "$IP | " . date("d.m.y H:i:s") . " | " . $_POST['username'] . " | " . $_SERVER['HTTP_USER_AGENT'];
			$file = fopen("/tmp/user_ip.log", "a");
			fputs($file, $logged_string, strlen($logged_string));
			fputs($file, "\n");
			fclose($file);
			
			//move to front page
			header("location:/index.php?p=main&dp=live");
			
		}
		
		private function passError()
		{
			//users_ip to log
			$IP=$_SERVER['REMOTE_ADDR'];
			$logged_string = "$IP | " . date("d.m.y H:i:s") . " | " . $_POST['username'] . " | " . $_SERVER['HTTP_USER_AGENT'] . " | " . "MATCH ERROR!";
			$file = fopen("/tmp/user_ip.log", "a");
			fputs($file, $logged_string, strlen($logged_string));
			fputs($file, "\n");
			fclose($file);
			
			//move to login page
			header("location:/login.php?p=error");	
			
		}
		
		
	}
		
?>
