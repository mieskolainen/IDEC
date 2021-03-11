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
	
	if ($php_sivu=="tools") {
		
		if ($php_alasivu=="logs") {
	  			echo "<li><a href='/index.php?p=tools&dp=logs' id='set'>Logs</a></li>";
			} else {
				echo "<li><a href='/index.php?p=tools&dp=logs'  >Logs</a></li>";
		}
		if ($php_alasivu=="syst") {
				echo "<li><a href='/index.php?p=tools&dp=syst' id='set'>System</a></li>";
			} else {
				echo "<li><a href='/index.php?p=tools&dp=syst'  >System</a></li>";
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
	
	if ($php_alasivu == "logs") {
		require_once( "/disk/home/tools/logs.php" );
	}
	if ($php_alasivu == "syst") {
		require_once( "/disk/home/tools/syst.php" );
	}
?>
</div> <!-- end #maincontent -->