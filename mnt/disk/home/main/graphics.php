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

<div id="chart_energy_div" style="width:57em; height:29em;" align="center"></div>
<div id="choices" style="margin-top:1.0em;margin-bottom:0.5em">	</div>
<div id="overview" style="margin-top:0.5em;margin-bottom:1em;width:21em;height:4.6em" ></div>
<div id="numerical_div" style="margin-top:1.4em;margin-bottom:1em;width:36em" ></div>

<div id="dateform" style="margin-left:2.0em">

<form name="dform" id="dform" action="#" method="get" >
  <table width="519" border="0" align="center">
    <tr>
      <td width="92"><table width="89" border="0">
        <tr>
          <td width="60"><input class="date_input" type="text" id="f_date_c1" onchange="setResolution(this);" size="10" maxlength="10" readonly="1"/></td>
          <td width="19"><img src="/js/calendar/img.gif" alt="" id="f_trigger_c1" style="cursor: pointer; border: 1px solid red;" title="Date selector"
      onmouseover="this.style.background='red';" onmouseout="this.style.background=''" /></td>
        </tr>
      </table></td>
      <td width="6">-</td>
      <td width="92"><table width="89" border="0">
        <tr>
          <td width="60"><input class="date_input" type="text" id="f_date_c2" onchange="setResolution(this);" size="10" maxlength="10" readonly="1"/></td>
          <td width="19"><img src="/js/calendar/img.gif" alt="" id="f_trigger_c2" style="cursor: pointer; border: 1px solid red;" title="Date selector"
	onmouseover="this.style.background='red';" onmouseout="this.style.background=''" /></td>
        </tr>
      </table></td>
      <td width="69"><select name="f_type" id="f_type" onchange="setSensors(this);" >
        <option value="power">Power (W)</option>
        <option value="energy" selected="selected">Energy (kWh)</option>
		<option value="factor">Relative (kWh/&deg;C)</option>
        <option value="temp">Temperature (&deg;C)</option>
      </select></td>
      
      <td width="69">
      
      <select id='sensor_id' name='sensor_id' multiple='multiple' onchange="countSelected(this,10);" >
      </select>
      
      </td>
      
      <td width="131"><table width="128" border="0">
        <tr>
             <td width="39">Resolution</td>
             <td width="79"><select name="f_reso" id="f_reso" >
               <option value="1" >1 min</option>
               <option value="5" selected="selected">5 min</option>
               <option value="60">1 hour</option>
               <option value="1440">24 hour</option>
             </select></td>
           </tr>
         </table></td>
      <td width="32"><input type="button" class="push_button" value="Get" onclick="getGraphics();" align="right"/></td>
    </tr>
  </table>
</form>
</div>

<div id="alert_box" align="center"></div>
<div id="loading_box" style="display:none" align="center"></div>

</div>

<script type="text/javascript">

	
	//Device name object
	var deviceName = new Object();	//IO ja ONEWIRE devices
	
	<?php
	
		//haetaan luokat
		require_once( "/disk/home/classes/MAIN.class.php");

		//luodaan asetusoliot
		$jsObj = new JS();
		
		// IO
		$jsObj->printJsObj("io_data", "energy");
		
		// ONEWIRE
		$jsObj->printJsObj("ow_data", "temp");
		
	?>
	
	// aliakset
	deviceName["power"]  = deviceName["energy"];
	deviceName["factor"] = deviceName["energy"];
	
	//calendar setup
	Calendar.setup({
	    inputField     :    "f_date_c1",     // id of the input field
	    ifFormat       :    "%d-%m-%Y",      // format of the input field
	    button         :    "f_trigger_c1",  // trigger for the calendar (button ID)
	    align          :    "Tl",           // alignment (defaults to "Bl")
	    firstDay	   :    1,
	    singleClick    :    true
	});
	
	Calendar.setup({
	    inputField     :    "f_date_c2",     // id of the input field
	    ifFormat       :    "%d-%m-%Y",      // format of the input field
	    button         :    "f_trigger_c2",  // trigger for the calendar (button ID)
	    align          :    "Tl",           // alignment (defaults to "Bl")
	    firstDay	   :    1,
	    singleClick    :    true
	}); 
	
	
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
	
	function dateFormatter(stamp, type) {
        
		var d = new Date(stamp); //date object
		
		var indeksi1 = document.dform.f_reso.selectedIndex; 
		var f_reso = document.dform.f_reso.options[indeksi1].value; //luetaan millä lailla muotoillaan timestamp
		
		if (f_reso == 0) {
	    	return $j.plot.formatDate(d, "%H:%M:%S");
		}
		if (f_reso == 1 || f_reso == 6 || f_reso == 30 || f_reso == 60) {
	    	return $j.plot.formatDate(d, "%H:%M");
		}
		if (f_reso == 1440) {
			return $j.plot.formatDate(d, "%d/%m/%y");
		}
		if (f_reso == 43200) {
			return $j.plot.formatDate(d, "%m/%y");
		}
	}
	
	
	//First page load
	document.dform.f_date_c1.value = format_date(0);
	document.dform.f_date_c2.value = format_date(0);
	
	setSensors(document.getElementById('f_type'));
	
	// haetaan grafiikat
	getGraphics();

</script>