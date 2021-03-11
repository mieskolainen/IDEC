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
	
	if ($php_sivu=="complex") {
		
		if ($php_alasivu=="cam") {
	  			echo "<li><a href='/index.php?p=complex&dp=cam' id='set'>Cam</a></li>";
			} else {
				echo "<li><a href='/index.php?p=complex&dp=cam'  >Cam</a></li>";
		}
		if ($php_alasivu=="doors") {
				echo "<li><a href='/index.php?p=complex&dp=doors' id='set'>Doors</a></li>";
			} else {
				echo "<li><a href='/index.php?p=complex&dp=doors'  >Doors</a></li>";
		}
		if ($php_alasivu=="sensors") {
				echo "<li><a href='/index.php?p=complex&dp=sensors' id='set'>Sensors</a></li>";
			} else {
				echo "<li><a href='/index.php?p=complex&dp=sensors'  >Sensors</a></li>";
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
	
	if ($php_alasivu == "cam") {
		require_once( "/disk/home/complex/cam.php" );
	}
	if ($php_alasivu == "doors") {
		require_once( "/disk/home/complex/doors.php" );
	}
	if ($php_alasivu == "sensors") {
		require_once( "/disk/home/complex/sensors.php" );
	}
	
?>
</div> <!-- end #maincontent -->