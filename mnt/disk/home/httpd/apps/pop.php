<div id="center_column" align="center">
<div id="chart_live_div" style="width:900px; height:460px;" align="center"></div>

</div>

<script type="text/javascript">

	function dateFormatter(stamp, type) {	 
		       
	    var d = new Date(stamp); //date object 
	    
	    if (type == "live" ) { 	
			return $j.plot.formatDate(d, "%H:%M:%S");
		}
	}
	
	chart_live();
	
	window.setInterval("chart_live()", 3000);

</script>
