<?php
	require_once("/home/classes/LOGIN.class.php");	
		
	$loginObj = new LOGIN();
	$loginObj->getIpSpace();
	
	if (isset ($_POST['submit'])) {
		
		$loginObj->checkLogin();
		
	}
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

<link rel="shortcut icon" href="/images/favicon.png" />

<title>&#948-Box</title>

<link href="/css/style.css" rel="stylesheet" type="text/css" />

</head>

<body class="oneColElsCtrHdr">

<div id="container">
  <div id="header">
    <h1 align="center">Login</h1>
    
    <div align="center">
      <!-- end #header -->
    </div>
  </div>
  <div id="mainContent" align="center">
  <p></p>
  <form method="post" action="" >

		<table bgcolor="#ffffff" align="center" border="0"><tr><td width="82" align="right">Username:</td>
		<td width="117" align="left"><input name="username" tabindex="1" type="text" size="12" />
		</td>
		</tr>
		
		<tr><td width="82" align="right">Password:</td>
		<td align="left"><input name="password" tabindex="2" type="password" size="12" />
		</td>
		</tr>
		</table>
		
		<input type="submit" class="push_button" name="submit" value="Login">
		<p></p>

</form>

<?php

	if ($_GET['p']=="error") {
	
		echo "<p>Wrong username or password!</p>";
	        
	}
	
?>

<!-- end #mainContent -->
  </div>
  <div id="footer">
    <p align="center">&#948-Box</p>
  <!-- end #footer --></div>
<!-- end #container --></div>
 </body>
</html>
