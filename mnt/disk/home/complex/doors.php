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

<script type="text/javascript">

new Ajax.PeriodicalUpdater("front_io_div", "/apps/command.php?com=front_io", {   
 asynchronous: true,  
 frequency : 4,   
 decay : 1   
});

new Ajax.PeriodicalUpdater("garage_input_div", "/apps/command.php?com=garage_input", {   
 asynchronous: true,  
 frequency : 4,   
 decay : 1   
});
</script> 
    
<div id="bigbox_div">

        <input type="submit" class="push_button" value="FRONT" onclick="ajaxUpdater('front_output_div','/apps/command.php?com=front_push_output')" />
      <p>
      <div id="front_output_div">Waiting for a command...
      </div>
      </p>
      <div id="smallbox_div">
      <div id="front_io_div">Loading...</div>
      </div>
      <p>
        <input type="submit" class="push_button" value="GARAGE" onclick="ajaxUpdater('garage_output_div','/apps/command.php?com=garage_push_output')" />
      </p>
      <div id="garage_output_div">Waiting for a command...
      </div>
	  <div id="smallbox_div">
      <div id="garage_input_div">Loading...</div>
      </div>
      <div align="right"><img src="/images/rotate2.gif"/></div>
    </div> 
    
<br/>
