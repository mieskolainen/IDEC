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

<div id="bigbox_div">

<div id="power_graph"><div id="chart_small_div" ></div></div>

<div id="powercalc_div"></div>

<br/>

<div id="arrow_div">
	<a HREF="/apps/pop.php" title="Power [W]" onclick="Modalbox.show(this.href, {title: this.title, width: 950}); return false;">
		<img src="/images/arrow.gif" border="0" align="right"/>
	</a>
</div>
</div>

<div id="css_bar_div" ></div>

<br/>
<br/>
<div id="bayes_bar_div" style="font-size: 1.05em; background-image: url(/images/transparent.png); width: 70%"></div>

<script type="text/javascript">

	var RightNow = new Date(); //date object		
	var msecs = RightNow.getTime(); //time to prevent GET caching
	
	// TÄMÄ PITÄÄ MUUTTAA DYNAAMISEKSI
	var deviceId = 0;
	
	var socket_client_url = "/apps/socket_client.cgi?time="+msecs+"&type=1"+"&si="+si+"&deviceId="+deviceId;	
	
	new Ajax.PeriodicalUpdater("powercalc_div", socket_client_url, {  //for Power live data ajax update 
	 	asynchronous: true,
	 	frequency : 3,   
	 	decay : 1  
	});
	
	// -------------------------------------------------------------------------
	
	var css_bar_url = "/apps/command.php?com=css_bar&deviceId="+deviceId;
	
	new Ajax.PeriodicalUpdater("css_bar_div", css_bar_url, {   
		asynchronous: true, 
		frequency : 120,  
		decay : 1
	});
	
	// -------------------------------------------------------------------------
	
	var bayes_bar_url = "/apps/socket_client.cgi?time="+msecs+"&type=4"+"&si="+si+"&deviceId="+deviceId;
	
	new Ajax.PeriodicalUpdater("bayes_bar_div", bayes_bar_url, {   
		asynchronous: true, 
		frequency : 10,
		decay : 1
	});
	
	function dateFormatter(stamp, type) {    
		
	    var d = new Date(stamp); //date object
	
	    if (type == "live") {
			return $j.plot.formatDate(d, "%H:%M:%S");
		}
		if (type == "week") {
			return $j.plot.formatDate(d, "%d/%m/%y");
		}
	}
	
	//Loading animation

	var loaded = false; 
	
	function startLoading() {
	    loaded = false; //variable for fast loadings, to prevent unnecessary animation
	    window.setTimeout('showLoadingImage()', 1000);     
	}
	
	function stopLoading() {
	    //Element.hide('loading_box');
	    Effect.Fade('loading_box');
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
	
	//chart_week();
	
	window.setInterval("chart_small_live(2, '#chart_small_div')", 2000);

</script>


