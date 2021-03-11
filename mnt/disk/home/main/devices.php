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
	require_once( "/disk/home/classes/MAIN.class.php");
	
	//luodaan asetusoliot
	$clusterObj = new CLUSTER();
?>

<br/>
<br/>
<div id="bigbox_dark_div">
	<div id="ContentFlow_DEFAULT" class="ContentFlow" useAddOns="DEFAULT">
		<div class="flow">
			<?php				
				$clusterObj->printCarusel();
			?>
		</div>
			
		<div class="globalCaption">
		</div>
		
		<div class="scrollbar">
			<div class="slider">
				<div class="position">
				</div>
			</div>
		</div>
	</div>
</div>

<div id="bigbox_dark_div"> 
	<div id="disaggregated_div" style="width:600px;height:450px"></div>  
</div>

<script type="text/javascript">

	// GET javascript object
	<?php
		$clusterObj->printJs();
	?>
	
	$j(function () {
		$j.plot($j("#disaggregated_div"), data.items, { series: {
			pie: {
				show: true,
				stroke: {
					color: '#FFF',
					width: 1
				},
				label: {
					show: true,
					background: {
						color: "#000",
						opacity: 0.55
					},
					threshold: 0.02
				},
				highlight: {
					opacity: 0.55
				}
			},
			legend: {
				show: true,
				color: "#000",
				position: "ne"
			}
		}}); 
	});

</script>

