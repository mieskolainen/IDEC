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

<script>

	function dateFormatter(stamp, type) {    
		
	    var d = new Date(stamp); //date object
	
		if (type == "year") {
			return $j.plot.formatDate(d, "%m/%y");
		}
		
	}

	//Loading animation
	
	var loaded = false; 
	
	function startLoading() {
	    loaded = false; //variable for fast loadings, to prevent unnecessary animation
	    window.setTimeout('showLoadingImage()', 10);     
	}
	
	function stopLoading() {
	    Element.hide('loading_box');
	    //Effect.Fade('loading_box');
	    loaded = true;
	}
	
	function showLoadingImage() {
	    var el = document.getElementById("loading_box");
	    if (el && !loaded) {
	        el.innerHTML = '<img src="/images/loading2.gif">';
	        Effect.Appear('loading_box');
	        //$ ( 'loading_box' ).show ( );
	        
	    }
	}
	
	startLoading();
	
</script>


<div id="headerbox_div">Forecast</div>

<div id="bigbox_div">

<div id="loading_box" style="display:none" align="center"></div>


<?php
	
	require_once("/disk/home/classes/POWER.class.php");
	
	$forecastObj = new FORECAST();
	
	$deviceId = 0;
	$forecastObj->getForecast($deviceId);

?>


</div>


