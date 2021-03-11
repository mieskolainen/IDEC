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

<script type="text/javascript">

	new Ajax.PeriodicalUpdater("RSS_bar_div", "/apps/command.php?com=sidebar", {   
 	asynchronous: true, 
 	frequency : 10,   
 	decay : 1
	});


</script>

<div class="topbar">
<ul>
<?php
	$php_sivu = $_GET['p'];
	$php_alasivu = $_GET['dp'];
	
	if ($php_sivu=="main") {
		
		if ($php_alasivu=="live") {
	  			echo "<li><a href='/index.php?p=main&dp=live' id='set'>Live</a></li>";
			} else {
				echo "<li><a href='/index.php?p=main&dp=live'  >Live</a></li>";
		}
		if ($php_alasivu=="graphics") {
				echo "<li><a href='/index.php?p=main&dp=graphics'  id='set'>Graphics</a></li>";
			} else {
				echo "<li><a href='/index.php?p=main&dp=graphics' >Graphics</a></li>";
		}
		if ($php_alasivu=="devices") {
	  			echo "<li><a href='/index.php?p=main&dp=devices' id='set'>Devices</a></li>";
			} else {
				echo "<li><a href='/index.php?p=main&dp=devices'  >Devices</a></li>";
		}
		if ($php_alasivu=="statistics") {
				echo "<li><a href='/index.php?p=main&dp=statistics' id='set'>Statistics</a></li>";
			} else {
				echo "<li><a href='/index.php?p=main&dp=statistics'  >Statistics</a></li>";
		}
		if ($php_alasivu=="debug") {
				echo "<li><a href='/index.php?p=main&dp=debug' id='set'>Debug</a></li>";
			} else {
				echo "<li><a href='/index.php?p=main&dp=debug'  >Debug</a></li>";
		}/*
		if ($php_alasivu=="forecast") {
				echo "<li><a href='/index.php?p=main&dp=forecast'  id='set'>Forecast</a></li>";
			} else {
				echo "<li><a href='/index.php?p=main&dp=forecast'  >Forecast</a></li>";
		}*/

	}	
	
	?>
</ul>    
</div> <!-- end #topbar -->

</div> <!-- end #main_bar_div -->


<div id="sidebar2_div">
	
	<br/>
	<br/>
	
	<!--
	<?php
	
  		$help_section = $_GET['p'];
  	
    	echo "<h4><a href=\"javascript:popUp('/help/help_large.php?section=$help_section', 'toolbar=no,location=yes,statusbar=no,menubar=no,scrollbars=yes,width=400,height=950,top=0,left=0')\">Click for help...</a></h4>";
    
   		//require_once("/disk/home/httpd/help/help_small.php");
	   		
	?>
	-->
	<div id="RSS_bar_div">
	
	</div>
        
</div> <!-- end #sidebar2 --> 
	

	<div id="mainContent_div">

	<?php
	
		if ($php_alasivu == "live") {
			require_once( "/disk/home/main/live.php" );
		}		
		if ($php_alasivu == "devices") {
			require_once( "/disk/home/main/devices.php" );
		}
		if ($php_alasivu == "statistics") {
			require_once( "/disk/home/main/statistics.php" );
		}	
		if ($php_alasivu == "graphics") {
			require_once( "/disk/home/main/graphics.php" );
		}
		if ($php_alasivu == "debug") {
			require_once( "/disk/home/main/debug.php" );
		}/*
		if ($php_alasivu == "forecast") {
			require_once( "/disk/home/main/forecast.php" );
		}*/

		
	?>

	</div> <!-- end #maincontent -->


