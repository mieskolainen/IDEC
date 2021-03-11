<?php
	session_start();
	
	if (!isset ($_SESSION['un'])) {
		header("location:logout.php");
	}
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

<link rel="shortcut icon" href="/images/favicon.png" />

<title>Help instructions</title>

<link href="/css/style.css" rel="stylesheet" type="text/css" />


</head>

<body class="oneColElsCtrHdr">


<div id="container">
  <div id="header">
    <h1 align="center">Help</h1>
    
    <div align="center">
      <!-- end #header -->
    </div>
  </div>
  <div id="mainContent">
    <?php  	
   	$large_help_section = $_GET['section']; //large help page (diskreetti)  	
	
	if ($large_help_section=="power") { 

		echo "<H3>Live Data</H3>";
		echo "<p>Power means your current energy consumption rate. Energy is ability to do work. </p>";
		echo "<p>1 kWh = 3.6 MJ (million joule)</p>";
		echo "<p>1 W = 1 J/s (joule per second)</p>";
		
		echo "<H3>Relative Consumption</H3>";
		echo "<p>All used electric energy transforms to heat. Relative consumption measures how much energy you had to use to heat your house 1 &deg;C above outdoor temp.</p>";
		
		echo "<H3>Statistics</H3>";
		echo "<p>Numerical information about your energy usage. Mean power is arithmetic average of calculated period. Costs are based on day and night prices of energy.</p>";
		
		echo "<H3>Graphics</H3>";
		echo "<p>Different type of graphs are available. Instant power shows history of instantaneous power stamps.</p>";
		
		echo "<H3>Forecast</H3>";
		echo "<p>System calculates forecast based on your consumption, outdoor temperature and 30 years history of outdoor temperature in your living area.</p>";
		
	}
	
	if ($large_help_section=="complex") { 

		echo "<H3>Video</H3>";
		echo "<p>...</p>";
		echo "<H3>Door Control</H3>";
		echo "<p>...</p>";

	}
	
	if ($large_help_section=="tools") { 

		echo "<H3>Logs</H3>";
		echo "<p>...</p>";
		echo "<H3>System info</H3>";
		echo "<p>...</p>";

	}
	
	if ($large_help_section=="settings") { 

		echo "<H3>Calculation settings</H3>";
		echo "<p>...</p>";
		echo "<H3>Web access</H3>";
		echo "<p>...</p>";

	}
	
	echo "<H3><font color=#CC0000>Can't see any graphs!</font></H3>";
	echo "<p>Turn on full javascript support from your browser.</p>";

?>
      <!-- end #mainContent -->
    
  </div>
  <div id="footer">
    <p align="center">&infin</p>
  <!-- end #footer --></div>
<!-- end #container --></div>
 </body>
</html>