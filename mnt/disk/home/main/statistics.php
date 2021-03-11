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

<form name="dform1" id="dform1" action="#" method="get">      
<table width="264" border="0">
<tr>
<td width="96">
    
<table cellspacing="0" cellpadding="0" style="border-collapse: collapse">
 <tr>
 <td width="60"><input class="date_input" type="text" id="f_date_c11" size="10" maxlength="10" readonly="1" /></td> 
 <td width="20"><img src="/js/calendar/img.gif" id="f_trigger_c11" style="cursor: pointer; border: 1px solid red;" title="Date selector"
      onmouseover="this.style.background='red';" onmouseout="this.style.background=''" /></td>
</table></td>

<td width="6"><div align="center">-</div></td>
<td width="112">

<table cellspacing="0" cellpadding="0" style="border-collapse: collapse">
<tr>
<td width="60"><input class="date_input" type="text" id="f_date_c12" size="10" maxlength="10" readonly="1" /></td>
<td width="54"><img src="/js/calendar/img.gif" id="f_trigger_c12" style="cursor: pointer; border: 1px solid red;" title="Date selector"
	onmouseover="this.style.background='red';" onmouseout="this.style.background=''" /></td>
</table></td>
    <td width="32">
      
        <div align="center">
          <input type="button" class="push_button" value="Get" onclick="getStatistics(1)" align="right"/>
        </div></td>
	</tr>
</table>
</form>

<form name="dform2" id="dform2" action="#" method="get">      
<table width="264" border="0">
<tr>
<td width="96">
    
<table cellspacing="0" cellpadding="0" style="border-collapse: collapse">
 <tr>
 <td width="60"><input class="date_input" type="text" id="f_date_c21" size="10" maxlength="10" readonly="1" /></td>
 <td width="20"><img src="/js/calendar/img.gif" id="f_trigger_c21" style="cursor: pointer; border: 1px solid red;" title="Date selector"
      onmouseover="this.style.background='red';" onmouseout="this.style.background=''" /></td>
</table></td>

<td width="6"><div align="center">-</div></td>
<td width="112">

<table cellspacing="0" cellpadding="0" style="border-collapse: collapse">
<tr>
<td width="60"><input class="date_input" type="text" id="f_date_c22" size="10" maxlength="10" readonly="1" /></td>
<td width="54"><img src="/js/calendar/img.gif" id="f_trigger_c22" style="cursor: pointer; border: 1px solid red;" title="Date selector"
	onmouseover="this.style.background='red';" onmouseout="this.style.background=''" /></td>
</table></td>
    <td width="32">
      
        <div align="center">
          <input type="button" class="push_button" value="Get" onclick="getStatistics(2)" align="right"/>
        </div></td>
	</tr>
</table>
</form>

<div id="loading_box" style="display:none"></div>
<div id="alert_box" ></div>
<div id="log1_div"></div>
<br/>
<br/>
<div id="log2_div"></div>
	
</div>

<script type="text/javascript">


	//////////calendar setup
    Calendar.setup({
        inputField     :    "f_date_c11",     // id of the input field
        ifFormat       :    "%d-%m-%Y",      // format of the input field
        button         :    "f_trigger_c11",  // trigger for the calendar (button ID)
        align          :    "Tl",           // alignment (defaults to "Bl")
        firstDay	   :    1,
        singleClick    :    true
    });
    
    Calendar.setup({
        inputField     :    "f_date_c12",     // id of the input field
        ifFormat       :    "%d-%m-%Y",      // format of the input field
        button         :    "f_trigger_c12",  // trigger for the calendar (button ID)
        align          :    "Tl",           // alignment (defaults to "Bl")
        firstDay	   :    1,
        singleClick    :    true
    });
	
	Calendar.setup({
        inputField     :    "f_date_c21",     // id of the input field
        ifFormat       :    "%d-%m-%Y",      // format of the input field
        button         :    "f_trigger_c21",  // trigger for the calendar (button ID)
        align          :    "Tl",           // alignment (defaults to "Bl")
        firstDay	   :    1,
        singleClick    :    true
    });
    
    Calendar.setup({
        inputField     :    "f_date_c22",     // id of the input field
        ifFormat       :    "%d-%m-%Y",      // format of the input field
        button         :    "f_trigger_c22",  // trigger for the calendar (button ID)
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
    
    
	//First page load
	document.dform1.f_date_c11.value = format_date(0);
	document.dform1.f_date_c12.value = format_date(0);
	document.dform2.f_date_c21.value = format_date(-1);
	document.dform2.f_date_c22.value = format_date(-1);
	
	getStatistics(1);
	getStatistics(2);

</script>
