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
    
      <p>Select log file and press button:</p>
      <form name="lomake" id="lomake">
        <select name="valinta">
          <option value = "/apps/command.php?com=get_log&log_type=user_ip.log">User IP</option>
          <option value = "/apps/command.php?com=get_log&log_type=lighttpd_error.log">Lighttpd</option>
          <option value = "/apps/command.php?com=get_log&log_type=door_control.log">Door Control</option>
          <option value = "/apps/command.php?com=get_log&log_type=sensors.log">Sensors</option>
        </select>
        <input type="button" class="push_button" value="Get" onclick="GetLog()" />
        <input type="button" class="push_button" value="Clear" onclick="ClearLog()" />
      </form>
      <div id="log_div">
           <textarea name='log_area' cols="95" rows="30" id='log_area'></textarea>
      </div>
     
      <p>
        <script type="text/javascript"> 

function GetLog() 
{ 
    // Valinta
    var indeksi = document.lomake.valinta.selectedIndex; 

    // Sijoitetaan valinnan arvo kohtaan 
    var arvo = document.lomake.valinta.options[indeksi].value;
    
    arvo += "&clear=false";
	
	new Ajax.Updater('log_div', arvo, {
		asynchronous: true		
	});

}

function ClearLog() 
{ 
    // Valinta
    var indeksi = document.lomake.valinta.selectedIndex; 

    // Sijoitetaan valinnan arvo kohtaan 
    var arvo = document.lomake.valinta.options[indeksi].value;
    
    arvo += "&clear=true";
	
	new Ajax.Updater('log_div', arvo, {
		asynchronous: true		
	});

}

//first start 
GetLog();

      </script>
      </p>
 </div>
