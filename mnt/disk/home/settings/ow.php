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
	$owObj = new OW();

?>



<div id="bigbox_div">
 <form method="post" action="">
	<div id="ow_data_div">
		<?php
				
			//printataan OW-taulukot
			$owObj->printData();
				
		?>
   </div>
</form>

<h3>Cross-Swap devices</h3>


<form method="post" action="">
	<div id="swap_data_div">
      <table id='mytable'>
      	<tr>
      		<th>DEVICE A</th>
      		<th></th>
      		<th>DEVICE B</th>
      	</tr>
      
<?php    

	//cross-swap data
	echo ($owObj->printSwapData());
		
?>
      
        <tr>
          <td>&nbsp;</td>
          <td align='center'><input type="submit" class="push_button" name="swap_data" value="Cross" /></td>
          <td>&nbsp;</td>
        </tr>
        
      </table>  
   </div>
</form>

<br/>
<div id="refresh_button" align="right">
	<input type="button" class="push_button" value="Refresh" onclick="ajaxUpdater('ow_data_div','/apps/command.php?com=ow_data')" />
</div>
</div>

<?php

	//päivitysteksti
	echo ($owObj->update_message);

?>

