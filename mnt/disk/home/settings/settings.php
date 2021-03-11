<!-- 
 deltaBox - Algorithmic Non-Intrusive Load Monitoring
 
 Copyright (c) 2008-2009 Mikael Mieskolainen.
 Licensed under the MIT License <http:opensource.org/licenses/MIT>.
 
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
-->

<div class="topbar">
<ul>
<?php
	$php_sivu = $_GET['p'];
	$php_alasivu = $_GET['dp'];
	
	if ($php_sivu=="settings") {
		
		if ($php_alasivu=="io") {
	  			echo "<li><a href='/index.php?p=settings&dp=io' id='set'>I/O</a></li>";
			} else {
				echo "<li><a href='/index.php?p=settings&dp=io'  >I/O</a></li>";
		}
		if ($php_alasivu=="ow") {
	  			echo "<li><a href='/index.php?p=settings&dp=ow' id='set'>Onewire</a></li>";
			} else {
				echo "<li><a href='/index.php?p=settings&dp=ow'  >Onewire</a></li>";
		}
		if ($php_alasivu=="dev") {
	  			echo "<li><a href='/index.php?p=settings&dp=dev' id='set'>Devices</a></li>";
			} else {
				echo "<li><a href='/index.php?p=settings&dp=dev'  >Devices</a></li>";
		}
		if ($php_alasivu=="price") {
	  			echo "<li><a href='/index.php?p=settings&dp=price' id='set'>Price</a></li>";
			} else {
				echo "<li><a href='/index.php?p=settings&dp=price'  >Price</a></li>";
		}
		if ($php_alasivu=="users") {
				echo "<li><a href='/index.php?p=settings&dp=users' id='set'>Users</a></li>";
			} else {
				echo "<li><a href='/index.php?p=settings&dp=users'  >Users</a></li>";
		}
		if ($php_alasivu=="network") {
				echo "<li><a href='/index.php?p=settings&dp=network'  id='set'>Network</a></li>";
			} else {
				echo "<li><a href='/index.php?p=settings&dp=network' >Network</a></li>";
		}
		if ($php_alasivu=="timezone") {
				echo "<li><a href='/index.php?p=settings&dp=timezone'  id='set'>Timezone</a></li>";
			} else {
				echo "<li><a href='/index.php?p=settings&dp=timezone'  >Timezone</a></li>";
		}
	}	
	
	?>
</ul>    
</div> <!-- end #topbar -->

</div> <!-- end #main_bar_div -->  

<div id="sidebar2_div">
	<?php
	
  		$help_section = $_GET['p'];
  	
    	echo "<h4><a href=\"javascript:popUp('/help/help_large.php?section=$help_section', 'toolbar=no,location=yes,statusbar=no,menubar=no,scrollbars=yes,width=400,height=950,top=0,left=0')\">Click for help...</a></h4>";
    
   		require_once("/disk/home/httpd/help/help_small.php");
	   		
	?>
	<div id="RSS_bar_div">
	
	</div>
        
</div> <!-- end #sidebar2 --> 

<div id="mainContent_div">
<?php
	
	if ($php_alasivu == "io") {
		require_once( "/disk/home/settings/io.php" );
	}
	if ($php_alasivu == "ow") {
		require_once( "/disk/home/settings/ow.php" );
	}
	if ($php_alasivu == "dev") {
		require_once( "/disk/home/settings/dev.php" );
	}	
	if ($php_alasivu == "price") {
		require_once( "/disk/home/settings/price.php" );
	}			
	if ($php_alasivu == "users") {
		require_once( "/disk/home/settings/users.php" );
	}	
	if ($php_alasivu == "network") {
		require_once( "/disk/home/settings/network.php" );
	}
	if ($php_alasivu == "timezone") {
		require_once( "/disk/home/settings/timezone.php" );
	}
?>
</div> <!-- end #maincontent -->