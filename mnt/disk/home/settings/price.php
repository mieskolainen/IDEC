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
	$priceObj = new PRICE();

?>



<div id="bigbox_div">    
    <form method="post" action="">
      <table width="654" border="0">
        <tr>
          <td>Day Price/kWh (c)</td>
          <td><input type="text" name="power_day_price" id="power_day_price" value="<?php echo $priceObj->power_day_price; ?>"/></td>
          <td>Default: 9.50</td>
        </tr>
        <tr>
          <td>Night Price/kWh (c)</td>
          <td><input type="text" name="power_night_price" id="power_night_price" value="<?php echo $priceObj->power_night_price; ?>"/></td>
          <td>Default: 8.00</td>
        </tr>
        <tr>
          <td>Daytime start</td>
          <td><input type="text" name="power_day_start" id="power_day_start" value="<?php echo $priceObj->power_day_start; ?>"/></td>
          <td>Default: 7</td>
        </tr>
        <tr>
          <td>Night-time start</td>
          <td><input type="text" name="power_night_start" id="power_night_start" value="<?php echo $priceObj->power_night_start; ?>"/></td>
          <td>Default: 22</td>
        </tr>
        <tr>
          <td>&nbsp;</td>
          <td><input type="submit" class="push_button" name="submit_power" value="Save" /></td>
          <td>&nbsp;</td>
        </tr>
      </table>
</form>
</div>

<?php

	//päivitysteksti
	echo ($priceObj->update_message);

?>

