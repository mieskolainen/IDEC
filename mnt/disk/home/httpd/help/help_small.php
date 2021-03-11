<?php

session_start(); 
if (isset ($_SESSION['un'])) {

   	$main_page = $_GET['p']; //embedded help
   	$under_page = $_GET['dp']; //embedded help
   	
   	if ($main_page=="power") {
	   	
	   	if ($under_page=="live") {

		   	echo "<p>Relative consumption measures your house and living style energy efficiency!</p>";
		
		}
		
		if ($under_page=="statistics") {

		   	echo "<p>Instant feedback helps you to change your customs. Check your actualized costs in realtime! </p>";
		
		}
		
		if ($under_page=="graphics") {

		   	echo "<p>You can zoom your view, by selecting area with mouse!</p>";
		   	echo "<p>For performance and stability, use only Firefox, Opera, Chrome or Safari!</p>";
		   	echo "<p>Press CTRL when selecting multiple measurements!</p>";
		
		}
		
		if ($under_page=="forecast") {

		   	echo "<p>Save energy by keeping air conditioner clean and at the right speed!</p>";
		
		}

	}		
	
	if ($main_page=="complex") { 

		echo "<p>No instructions.</p>";

	}
	
	if ($main_page=="tools") { 

		echo "<p>Use drop down menu to view system logs.</p>";
		echo "<p>System info shows statistics of flash SD-card.</p>";

	}
	
	if ($main_page=="settings") { 

		echo "<p>No instructions.</p>";

	}	
		
}	
?>