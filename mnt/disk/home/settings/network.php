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
	$netObj = new NETWORK();

?>


<div id="bigbox_div">    
    <form method="post" action="">
      <table width="654" border="0">
        <tr>
          <td width="220">Host Name</td>
          <td width="210"><input type="text" name="hostname" id="hostname" value="<?php echo $netObj->hostname; ?>"/></td>
          <td width="210">Default: deltaBox</td>
        </tr>
        <tr>
        <tr>
          <td>IP address</td>
          <td><input type="text" name="ip_address" id="ip_address" value="<?php echo $netObj->ip_address; ?>"/></td>
          <td>Default: 192.168.1.51</td>
        </tr> 
          <tr>
            <td>Subnet mask</td>
            <td><input type="text" name="subnet_mask" id="subnet_mask" value="<?php echo $netObj->subnet_mask; ?>"/></td>
          <td>Default: 255.255.255.0</td>
        </tr>        
        <tr>
          <td>Gateway</td>
          <td><input type="text" name="gateway" id="gateway" value="<?php echo $netObj->gateway; ?>"/></td>
          <td>Default: 192.168.1.1</td>
        </tr>
        <tr>
          <td>DNS Server</td>
          <td><input type="text" name="dns_server" id="dns_server" value="<?php echo $netObj->dns_server; ?>"/></td>
          <td>Default: 192.168.1.1</td>
        </tr>    
          <tr>
            <td>&nbsp;</td>
            <td><input type="submit" class="push_button" name="submit_network" value="Save" /></td>
          <td>&nbsp;</td>
        </tr>
      </table>
</form>
</div>

<?php

	//päivitysteksti
	echo ($netObj->update_message);

?>


