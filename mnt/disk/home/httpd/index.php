<?php
	//error_reporting(E_ALL);
	
	session_start(); 
	
	$sivu = $_GET['p'];
	
	if (!isset ($_SESSION['un'])) {
	
		header("location:/logout.php");
	}
	  
	//block from user writing these pages directly
	
	if ($sivu=="complex" && $_SESSION['c']!=1) {
	
		header("location:/logout.php");
	}
	
	if ($sivu=="tools" && $_SESSION['t']!=1) {
	
		header("location:/logout.php");
	}
	
	if ($sivu=="settings" && $_SESSION['s']!=1) {
	
		header("location:/logout.php");
	}

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="refresh" content="3600;url=/logout.php"; charset="utf-8"/>

<link rel="shortcut icon" href="/images/favicon.png" />

<!--[if IE]><script src="/js/jquery/excanvas.js" type="text/javascript"></script><![endif]-->

<script src="/js/js.js" type="text/javascript"></script>
<script src="/js/jquery/jquery.js" type="text/javascript"></script>
<script src="/js/jquery/jquery.flot.js" type="text/javascript"></script>
<script src="/js/jquery/jquery.flot.selection.js" type="text/javascript"></script>
<script src="/js/jquery/jquery.flot.pie.js" type="text/javascript"></script>
<script src="/ContentFlow/contentflow.js" type="text/javascript"></script>

<SCRIPT language="JavaScript"> 
	var $j = jQuery.noConflict();
</script>

<script src="/js/prototype.js" type="text/javascript"></script>
<script src="/js/scriptaculous/scriptaculous.js" type="text/javascript"></script>
<script src="/js/modalbox.js" type="text/javascript"></script>
<script src="/js/calendar/calendar.js" type="text/javascript"></script>
<script src="/js/calendar/lang/calendar-en.js" type="text/javascript"></script>
<script src="/js/calendar/calendar-setup.js" type="text/javascript"></script>

<link rel="stylesheet" href="/css/modalbox.css" type="text/css" />
<link rel="stylesheet" href="/js/calendar/calendar-blue.css" type="text/css" />
<link rel="stylesheet" href="/css/style.css" type="text/css" />

<SCRIPT language="JavaScript">

<?php
$sesid= session_id();

printf("var si = \"%s\";\n", $sesid);

printf("var system_day = %d;\n", date("j"));
printf("var system_month = %d;\n", date("n"));
printf("var system_year = %d;\n", date("Y"));

?>

new Ajax.PeriodicalUpdater("uptime_div", "/apps/command.php?com=uptime", {   
 	asynchronous: true, 
 	frequency : 5,   
 	decay : 1   
});

</script>

<title>&#948-Box</title>

</head>

<body class="thrColFixHdr">


  <div id="header_div">
    <h1>&#948-Box</h1>
  </div>
  <div id="container_div">
<div id="main_bar_div">


        <div class="toptopbar">
  <ul>
      <?php

    	$php_sivu = $_GET['p'];
   		
	if ($php_sivu=="main") {
  		echo "<li><a href='/index.php?p=main&dp=live' id='set'>Main</a></li>";
	} else {
		echo "<li><a href='/index.php?p=main&dp=live'>Main</a></li>";
	}
	
	if ($_SESSION['c']==1) {
		
		if ($php_sivu=="complex") {
			echo "<li><a href='/index.php?p=complex&dp=cam' id='set'>Complex</a></li>";
		} else {
			echo "<li><a href='/index.php?p=complex&dp=cam'>Complex</a></li>";
		}
	}
	
	if ($_SESSION['t']==1) {
		if ($php_sivu=="tools") {
			echo "<li><a href='/index.php?p=tools&dp=logs' id='set'>Tools</a></li>";
		} else {
			echo "<li><a href='/index.php?p=tools&dp=logs'>Tools</a></li>";
		}
	}
	
	if ($_SESSION['s']==1) {
		if ($php_sivu=="settings") {
  			echo "<li><a href='/index.php?p=settings&dp=io' id='set'>Settings</a></li>";
		} else {
			echo "<li><a href='/index.php?p=settings&dp=io'>Settings</a></li>";
		}
	}
	
	echo "<li><a href='/logout.php'><span style='color:#CC0000'>Log Out</span></a></li>";
  ?>

	</ul>
	
		</div>

<?php

	$p = $_GET['p'];
	
	$page = "/disk/home/" . $p . "/" . $p . ".php";

	require_once($page);
	
?>

  <p>
    <!-- This clearing element should immediately follow the #mainContent div in order to force the #container div to contain all child floats -->
  </p>
  
  <p><br class="clearfloat" />
  </p>
  		<div id="footer_div">
  			<!-- <div id="cpu_div"></div> -->
        	<div id="uptime_div"></div>
  		</div>
  
<!-- end #container --></div>
</body>
</html>
