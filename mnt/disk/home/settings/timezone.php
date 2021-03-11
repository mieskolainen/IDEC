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

<?php

	//haetaan luokat
	require_once("/home/classes/SETTINGS.class.php");

	//luodaan asetusolio
	$tzObj = new TIMEZONE();
	
?>



<div id="bigbox_div">    
    <form method="post" action="">
      <table width="654" border="0">
        <tr>
          <td width="220">Time Zone</td>
          <td width="210"><select name="time_zone" id="time_zone">
            <?php
            
            	//haetaan aikavyöhykkeet
				$tzObj->printTimezone();
				
            ?>
          </select></td>
          <td width="210">Default: Europe/Helsinki</td>
        </tr>
        <tr>
        
        <tr>
          <td>Server1 IP/Name</td>
          <td><input name="NTP_server1" type="text" id="NTP_server1" value="<?php echo $tzObj->NTP_server1; ?>" /></td>
          <td>Default: ntp1.funet.fi</td>
        </tr>
        <tr>
          <td>Server2 IP/Name</td>
          <td><input name="NTP_server2" type="text" id="NTP_server2" value="<?php echo $tzObj->NTP_server2; ?>" /></td>
          <td>Default: ntp1.eunet.fi</td>
        </tr>
        <tr>
          <td>Server3 IP/Name</td>
          <td><input name="NTP_server3" type="text" id="NTP_server3" value="<?php echo $tzObj->NTP_server3; ?>" /></td>
          <td>Default: time1.mikes.fi</td>
        </tr>
        <tr>
          <td>Server4 IP/Name</td>
          <td><input name="NTP_server4" type="text" id="NTP_server4" value="<?php echo $tzObj->NTP_server4; ?>" /></td>
          <td>Default: ntp2.eunet.fi</td>
        </tr> 
                  
          <tr>
            <td>&nbsp;</td>
            <td><input type="submit" class="push_button" name="submit_time" value="Save" id="submit_time" /></td>
          <td>&nbsp;</td>
        </tr>
      </table>
</form>
</div>

<?php

	//päivitysteksti
	echo ($tzObj->update_message);

?>