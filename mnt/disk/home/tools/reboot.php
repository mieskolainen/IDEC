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

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

<title>System Reboot</title>

<link rel="shortcut icon" href="/images/favicon.png" />

<script src="/js/prototype.js" type="text/javascript"></script>

<title>Help instructions</title>

<link href="/css/style.css" rel="stylesheet" type="text/css" />

</head>

<body class="oneColElsCtrHdr">

<script type="text/javascript"> 

function reboot() {
	
	var command = "/apps/command.php?com=reboot";
	
	new Ajax.Updater("null", command, {
		asynchronous: true		
	});
}

var t = 120;

function update() {
	
	if (t>0) {
	
		var text = "<p>Wait for " + t + " seconds...</p>";
	
		document.getElementById('mainContent').innerHTML = text;
	
		t -= 1;
	
	} else {
		
		location.href = "/logout.php";
			
	}	
}

window.setInterval(update, 1000); //update

<?php

if ( isset ($_POST['submit']) ) {
	
	if ( $_POST['reboot']=="yes" ) {
		
		echo ("reboot();");
		
	}			
}

?>

</script>

<div id="container">
  <div id="header">
    <h1 align="center">System Reboot</h1>
    
    <div align="center">
      <!-- end #header -->
    </div>
  </div>
  <div id="mainContent" align="center">
	<p>Wait for 120 seconds...</p>
  </div>
  <div id="footer">
    <p align="center">MMM Box</p>
  <!-- end #footer --></div>
<!-- end #container --></div>

 </body>
</html>
