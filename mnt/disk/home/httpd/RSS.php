<?php

	error_reporting(E_ALL);
	
	header("Content-Type: application/xml; charset=UTF-8");
	
	require_once("/home/classes/RSS.class.php");
	
	$rssObj = new RSS();
	echo $rssObj->getFeed(25);
?>
