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
	$ioObj = new IO();

?>


<div id="bigbox_div">
 <form method="post" action="">
	<div id="io_data_div">
      <table id='mytable'>
      	<tr>
      		<th>ID</td>
      		<th>ACTIVE</th>
      		<th>PORT</th>
      		<th>TYPE</th>
      		<th>NAME</th>
      		<th>METER</th>
      		<th>CALCTIME</th>
      		<th>DELTA</th>
      		<th>LOG</th>
      	</tr>
      
      <?php	
		
      		//printataan IO taulukot		
     		$ioObj->printData();
	
      ?>
      
        <tr>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          <td><input type="submit" class="push_button" name="submit_data" value="Save" /></td>
          <td>&nbsp;</td>
        </tr>
        
      </table>  
   </div>
</form>

<!--
<h3>Add a Device</h3>

 <form method="post" action="">
	<div id="add_device_div">
      <table id='mytable'>
      	<tr>
      		<th>Id</td>
      		<th>Active</th>
      		<th>Port</th>
      		<th>Type</th>
      		<th>Name</th>
      		<th>Meter</th>
      		<th>CalcTime</th>
      		<th>Delta</th>
      		<th>Log</th>
      	</tr>
        <tr>
          <td><input size='2' name='deviceId' id='deviceId' value='99'/></td>
          <td><input type='checkbox' name='deviceActive' id='deviceActive' value='1'/></td>
          <td><input maxlength='2' size='2' type='text' name='devicePort' id='devicePort' value='8'/></td>
          <td><SELECT name='deviceTypePulse' id='deviceTypePulse'><OPTION value='energy'>energy</OPTION><OPTION value='water'>water</OPTION>
			<OPTION value='heat'>heat</OPTION></OPTION><OPTION value='oil'>oil</OPTION></SELECT></td>
          <td><input maxlength='15' size='15' type='text' name='deviceName' id='deviceName' value='null_name'/></td>
          <td><input maxlength='6' size='6' type='text' name='deviceMeterConst' id='deviceMeterConst' value='1000'/></td>
          <td><input maxlength='3' size='3' type='text' name='deviceCalcTime' id='deviceCalcTim' value='7'/></td>
          <td><input maxlength='3' size='3' type='text' name='deviceDelta' id='deviceDelta' value='25'/></td>
          <td><input type='checkbox' name='deviceLog' id='deviceLog' value='1' /></td>
        </tr>
        <tr>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          <td>&nbsp;</td>
          <td><input type="submit" class="push_button" name="new_device" value="Save" /></td>
          <td>&nbsp;</td>
        </tr>
        
      </table>  
   </div>
</form>

-->

</div>

<?php

	//päivitysteksti
	echo ($ioObj->update_message);

?>
