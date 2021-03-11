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
    
  <?php
  phpinfo();
    
  echo "<h3>Storage Space</h3>";
  exec ("df", $output1);
  
  $rows1 = count($output1);
  
  for ($i=0;$i<$rows1;++$i) {
	
	  echo "<pre>";
	  echo ($output1[$i]);
	  echo "</pre>";
	  
  }
  
  echo "<br/>";
  echo "<h3>Memory Usage</h3>";
  
  exec ("free", $output2);
  
  $rows1 = count($output2);
  
  for ($i=0;$i<$rows1;++$i) {
	
	  echo "<pre>";
	  echo ($output2[$i]);
	  echo "</pre>";
	  
  }
  
  echo "<br/>";
  echo "<h3>Processes</h3>";
  
  exec ("ps", $output3);
  
  $rows3 = count($output3);
  
  for ($i=0;$i<$rows3;++$i) {
	
	  echo "<pre>";
	  echo ($output3[$i]);
	  echo "</pre>";
	  
  }
  
  
  ?>

  </div>
  
      <div id="headerbox_div">Reboot</div>
    <div id="bigbox_div">
    
      <p>Click button to reboot system:</p>
      <form action="/tools/reboot.php" name="reboot_form" id="reboot_form" method="POST">
        <input type="hidden" name="reboot" value="yes"/>
        <input type="submit" class="push_button" name="submit" value="Reboot"/>
      </form>
      <div id="reboot_div">

      </div>
      
    </div>  
     <br/> 