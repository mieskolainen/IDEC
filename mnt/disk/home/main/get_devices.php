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

session_start();

if ($_SESSION['c']==1) {

	$fp = fopen ( "/mnt/mmc/cluster.data", 'r' );
	
	echo sprintf("<pre><p><b>Cluster\tt(s)\tP(W)\tFreq\tE(kWh)</b></p></pre>", $line[0], $line[1], $line[2], $line[3]);

	$total_events = 0;
	$total_energy = 0;
	
	while( $line = fgetcsv ($fp, 100, ",") ) {

		echo sprintf("<pre><p>%d\t%d\t%d\t%d\t%0.2f</p></pre>", $line[0], $line[1], $line[2], $line[3], $line[1]*$line[2]*$line[3]/1000/3600);
		
		$total_events += $line[3];
		$total_energy += $line[1]*$line[2]*$line[3]/1000/3600;
	
	}
	
	echo sprintf("<pre><p><b>Total\t...\t...\t%d\t%0.2f</b></p></pre>", $total_events, $total_energy);
	
	fclose ( $fp );	
}

?>
