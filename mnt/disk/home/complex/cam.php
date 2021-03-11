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

<div id="bigbox_dark_div">
<br/>
<br/>
    <div id="small_cam" align="center">
       <SCRIPT LANGUAGE="JavaScript">
	

  if ((navigator.appName == "Microsoft Internet Explorer")&&(navigator.platform != "MacPPC")&&(navigator.platform != "Mac68k")) {
                document.write("<OBJECT ID=\"AxisCamControl\" CLASSID=\"CLSID:917623D1-D8E5-11D2-BE8B-00104B06BDE3\" WIDTH=\"640\" HEIGHT=\"480\" CODEBASE=\"http://zoomzoom.ath.cx:81/activex/AxisCamControl.cab#Version=2,23,0,0\">");
                document.write("<PARAM NAME=DisplaySoundPanel VALUE=0>");
                document.write("<PARAM NAME=URL VALUE=\"http://zoomzoom.ath.cx:81/axis-cgi/mjpg/video.cgi?camera=&resolution=640x480\">");
	    
                document.write("</OBJECT>");
	
  } else {
    theDate = new Date();
    var output = "<img SRC=\"http://zoomzoom.ath.cx:81/axis-cgi/mjpg/video.cgi?camera=&resolution=640x480&";
    output += theDate.getTime()
    output += "\" ALT=\"Press Reload if no image is displayed\">";
    document.write(output);
  }

</SCRIPT>
</div>
</div>
    

<br/>
