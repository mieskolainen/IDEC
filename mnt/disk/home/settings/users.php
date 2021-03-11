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
	$usersObj = new USERS();
	
?>


<div id="bigbox_div"> 

<form method="post" action="">
<h3>Change user settings</h3>
<table width="440" border="0">
  <tr>
    <td height="121"><table width="248" border="0">
      <tr>
        <td width="62">Users</td>
        <td width="58">Permissions</td>
        <td width="58">&nbsp;</td>
        </tr>
      <tr>
        <td><select name="old_user" size=5 id="old_user">
          	
        	<?php
        		//tulostetaan käyttäjät
				$usersObj->printUsers();	
			?>
        </select></td>
        <td><select disabled name="permissions" size=5 id="permissions">
        
        	<?php
        		//tulostetaan oikeudet
        		$usersObj->printPermissions();
        	?>
		
        </select></td>
        <td><input type="submit" class="push_button" name="delete_user" value="Delete" /></td>
        </tr>
    </table>
      </td>
  </tr>
  <tr>
    <td width="434" height="59"><table width="420" border="0">
      <tr>
        <td width="220">New password</td>
        <td width="190"><input type="password" name="new_pass" /></td>
      </tr>
      <tr>
        <td>Confirm password</td>
        <td><input type="password" name="new_pass_confirm" /></td>
      </tr>
      <tr>
        <td>&nbsp;</td>
        <td><input type="submit" class="push_button" name="change_password" value="Change Password" /></td>
      </tr>
      <tr>
        <td>&nbsp;</td>
        <td>&nbsp;</td>
      </tr>
      <tr>
        <td>New permissions</td>
        <td>C
          <input name="new_complex" type="checkbox" id="new_complex" value="1" />
          |   
          T
          <input name="new_tools" type="checkbox" id="new_tools" value="1" />
          | S
          <input name="new_settings" type="checkbox" id="new_settings" value="1" /></td>
      </tr>
      <tr>
        <td>&nbsp;</td>
        <td><input type="submit"  class="push_button" name="change_permissions" value="Change Permissions" /></td>
      </tr>
    </table></td>
    </tr>
</table>
</form>


<h3>Add User</h3>
<form method="post" action="">
<table width="440" border="0">
        <tr>
          <td width="220">Username</td>
          <td width="210"><input type="text" name="user" /></td>
        </tr>
        <tr>
          <td>Password</td>
          <td><input type="password" name="pass" /></td>
        </tr>
        <tr>
          <td>Confirm Password</td>
          <td><input type="password" name="pass_confirm" /></td>
        </tr>
        <tr>
          <td>Permissions</td>
          <td>
	C
  <input name="check_complex" type="checkbox" id="check_complex" value="1" />
  |   
  T
  <input name="check_tools" type="checkbox" id="check_tools" value="1" />
 | S
 <input name="check_settings" type="checkbox" id="check_settings" value="1" /></td>
        </tr>
        
        <tr>
          <td>&nbsp;</td>
          <td><input type="submit" class="push_button" name="add_user" value="Apply" /></td>
        </tr>
    </table>
</form>
</div>

<?php

	//päivitysteksti
	echo ($usersObj->update_message);

?>
