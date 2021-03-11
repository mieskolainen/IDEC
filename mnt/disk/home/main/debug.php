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

<div id="headerbox_div">x(n), y(n)</div>
<div id="bigbox_div">
	<div id="center_div" align="center">
		<div id="chart_x_div" ></div>
		<div id="chart_y_div" ></div>
	</div>
</div>


<div id="headerbox_div">FFT{x(n)}</div>
<div id="bigbox_div">
	<div id="center_div" align="center">
		<div id="FFT_div" >
			<div id="loading_box" align="center"><img src="/images/loading2.gif"></div>
		</div>
	</div>
	<div id="alert_box" align="center"></div>
</div>


<br/>

<script type="text/javascript">
	
	function dateFormatter(stamp, type) {    
		
	    var d = new Date(stamp); //date object
	
	    if (type == "live") {
			return $j.plot.formatDate(d, "%H:%M:%S");
		}
		if (type == "week") {
			return $j.plot.formatDate(d, "%d/%m/%y");
		}
	}
	
	FFT_live(); //first page load
	
	window.setInterval("FFT_live()", 15000);
	window.setInterval("chart_small_live(2, '#chart_x_div')", 2000);
	window.setInterval("chart_small_live(22, '#chart_y_div')", 2000);
	
</script>
