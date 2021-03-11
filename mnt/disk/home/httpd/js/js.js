// deltaBox - Algorithmic Non-Intrusive Load Monitoring
// 
// Copyright (c) 2008-2009 Mikael Mieskolainen.
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// 
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

var previousPoint = null; //tooltip variable

function ajaxUpdater(id,url) {
    new Ajax.Updater(id,url,{asynchronous:true});   
}

function popUp(URL,ARG) {
	day = new Date();
	id = day.getTime();
	eval("page" + id + " = window.open(URL, '" + id + "', ARG);");
}

//   LIMIT MULTIPLE SELECTED OPTIONS SCRIPT
var selectedOptions = [];

function countSelected(select,maxNumber) {
	
	for(var i = 0; i < select.options.length; ++i) {
		
	 	if (select.options[i].selected && !new RegExp(i,'g').test(selectedOptions.toString())) {
	    	selectedOptions.push(i);
		}
	
	 	if (!select.options[i].selected && new RegExp(i,'g').test(selectedOptions.toString())) {
	  		selectedOptions = selectedOptions.sort(function(a,b){return a-b});
	  		  
	   		for(var j = 0; j < selectedOptions.length; ++j){
		   		
	     		if(selectedOptions[j] == i){
	        		selectedOptions.splice(j,1);
	     		}
	   		}
	 	}
	
		if (selectedOptions.length > maxNumber) {
			document.getElementById('alert_box').innerHTML = ("<font color=#CC0000>Maximum number of sensors!</font>");
			select.options[i].selected = false;
			selectedOptions.pop();
			document.body.focus();
		}
	}
}

function setResolution(chooser) {
	
	var deviceType = new Object();

	deviceType["generic"] = [{value:"1", text:"1 min"},
							 {value:"6", text:"6 min"},
							 {value:"30", text:"30 min"},
							 {value:"1440", text:"24 hour"},
							 {value:"43200", text:"1 month"}];
	
	deviceType["power"]  = deviceType["generic"];
	deviceType["energy"] = deviceType["generic"];
	deviceType["temp"]	 = deviceType["generic"];
	
	deviceType["factor"] = [{value:"1440", text:"24 hour"}];
	
    var f_resoChooser = chooser.form.elements["f_reso"];
    
    // empty previous settings
    f_resoChooser.options.length = 0;
    
    // haetaan alasvetovalikon f_type nykyinen arvo
    var choice = chooser.form.elements["f_type"].options[chooser.form.elements["f_type"].selectedIndex].value;
    var db = deviceType[choice];
	
    date_diff = calcDateDiff(); //päivien lukumäärä
   
    if (choice != "factor") {
	    
	    var j = 0;
	    
		if (date_diff  <= 3) {			

			// insert default first item   
		    // loop through array of the hash table entry, and populate options
		    
			//db.length - 1 --> jätetään kuukausireso pois
			
		    for (i = 0; i < db.length - 1; ++i) {
		        f_resoChooser.options[j] = new Option(db[i].text, db[i].value);
		        ++j;
		    }
		 	 
	    }
		
	    if ( (3 < date_diff && date_diff <=7) ) {
		    
		    for (i = 1; i < db.length - 1; ++i) {
		    	f_resoChooser.options[j] = new Option(db[i].text, db[i].value);
		    	++j;
		    }	    		  
	    }
	    
	    if ( (7 < date_diff && date_diff <=14) ) {
		    
		    for (i = 2; i < db.length - 1; ++i) {
		    	f_resoChooser.options[j] = new Option(db[i].text, db[i].value);
		    	++j;
		    }	    		  
	    }
	    
	   	if ( (14 < date_diff && date_diff <=28) ) {
		   	
		   	for (i = 3; i < db.length - 1; ++i) {
		        f_resoChooser.options[j] = new Option(db[i].text, db[i].value);
		        ++j;
		    }
	    }
	    
	    if (28 < date_diff ) {
		    
		    for (i = 3; i < db.length; ++i) {
		        f_resoChooser.options[j] = new Option(db[i].text, db[i].value);
		        ++j;
		    }
	    }
    
	} else if (choice == "factor") {	
		f_resoChooser.options[0] = new Option(db[0].text, db[0].value);
	}
}

function setSensors(chooser) {       	
	       	
	// Valittu tyyppi
	var f_type = chooser.form.elements["f_type"].options[chooser.form.elements["f_type"].selectedIndex].value;
	var sensor_idChooser = chooser.form.elements["sensor_id"];
	
	// empty previous settings
	sensor_idChooser.options.length = 0;
	
	var db = deviceName[f_type];
	
	for (i = 0; i < db.length; ++i) {
		 sensor_idChooser.options[i] = new Option(db[i].text, db[i].value);
		 
		 if (i == 0) {	//valitaan oletuksena ensimmäinen arvo
			 sensor_idChooser.options[i].selected = true;
		 }
	}
	
	if ( sensor_idChooser.options.length == 1 ) {
		sensor_idChooser.size = 1;	//näytetään kerralla vain yksi arvo
	} else {
		sensor_idChooser.size = 3; //muuten näytetään kolme
	}	
	
	setResolution(chooser);
}

function calcDateDiff() {

	var t1 = document.dform.f_date_c1.value; 
	var t2 = document.dform.f_date_c2.value;
	
	//var t1="10-10-2006";
	//var t2="15-10-2006";
	 
	//Total time for one day
	var one_day=1000*60*60*24; 
	
	//Here we need to split the inputed dates to convert them into standard format
	//for furter execution
	var x = t1.split("-");     
	var y = t2.split("-");
	
	//date format(Fullyear,month,date) 
	var date1 = new Date(x[2],(x[1]-1),x[0]);
	var date2 = new Date(y[2],(y[1]-1),y[0]);
	var month1 = x[1] - 1;
	var month2 = y[1] - 1;
	        
	//Calculate difference between the two dates, and convert to days      
	Diff=Math.ceil((date2.getTime()-date1.getTime())/(one_day));
	
	//Diff gives the diffrence between the two dates.	
	return Diff;
}

function getGraphics() {
	
	// Valinta
	var start_date = document.dform.f_date_c1.value; 
	var end_date = document.dform.f_date_c2.value;	
	
	var d1 = start_date.slice(0,2);
	var m1 = start_date.slice(3,5);
	var y1 = start_date.slice(6,10);
	
	var d2 = end_date.slice(0,2);
	var m2 = end_date.slice(3,5);
	var y2 = end_date.slice(6,10);
	
    var date1_obj = new Date(y1,m1-1,d1); //new date object
    var date2_obj = new Date(y2,m2-1,d2);
    
    var today_obj = new Date(system_year, system_month-1, system_day);
    
    if ( date1_obj.getTime() > today_obj.getTime() ) { //milliseconds difference
			
		document.getElementById('alert_box').innerHTML = ("<font color=#CC0000>Start date is in future!</font>");
		return -1;

	} else if ( date2_obj.getTime() > today_obj.getTime() ) { //milliseconds difference
			
		document.getElementById('alert_box').innerHTML = ("<font color=#CC0000>End date is in future!</font>");
		return -1;

	} else if ( date1_obj.getTime() > date2_obj.getTime() ) { //milliseconds difference
			
		document.getElementById('alert_box').innerHTML = ("<font color=#CC0000>Start date is bigger than end date!</font>");
		return -1;

	} else {
		document.getElementById('alert_box').innerHTML = "";
	}
	
	var RightNow = new Date();
	var msecs = RightNow.getTime(); //time to prevent GET caching
	var f_reso = document.dform.f_reso.options[document.dform.f_reso.selectedIndex].value;

	var selectedSensors = new Array();	//selected sensors_id's array [0], [3], [9] ...
	var selectedSensorsNames = new Array();	//selected sensors_id's array names like outdoor, heater, indoor...
	var selectedTypes = new Array();	//selected sensors_id's array
		
	function loopSelected() {
		
		var selObj = document.getElementById('sensor_id');
		var i;
		var count = 0;
		
		for (i = 0; i < selObj.options.length; ++i) {
			
			if (selObj.options[i].selected) {
				
				  selectedSensors[count] = selObj.options[i].value;
				  selectedSensorsNames[count] = selObj.options[i].text;
				  selectedTypes[count] = document.dform.f_type.options[document.dform.f_type.selectedIndex].value;
				  ++count;				  
			}
		}
	}  //get selected sensor_id's
	
	loopSelected();
	
	var RightNow = new Date();
	var msecs = RightNow.getTime(); //time to prevent GET caching

	var dataurl= "/apps/get_file_archive.cgi?"
	
	dataurl += "time=" + msecs;
	dataurl += "&f_reso=" + f_reso;
	dataurl += "&f_number=" + selectedSensors.length;
	dataurl += "&date1=" + start_date;
	dataurl += "&date2=" + end_date;
	dataurl += "&si=" + si;	
	
	dataurl += "&f_type=";
	
	for (i=0; i <selectedSensors.length; ++i) {
		
		dataurl += selectedTypes[i];
		
		if (i != (selectedSensors.length - 1)) {
			dataurl += ",";	//erotellaan pilkulla eri arvot
		}
	}
	
	dataurl += "&sensor_id=";
	
	for (i = 0; i < selectedSensors.length; ++i) {
		
		dataurl += selectedSensors[i];
		
		if (i != (selectedSensors.length - 1)) {	
			dataurl += ",";	//erotellaan pilkulla eri arvot
		}
	}
	
	$j(document).ajaxSend(function(evt, request, settings){ //document may be eq. #bigbox_div
		   		startLoading();
	});	
	
	$j.getJSON(dataurl, function (jsondata) {
		
			$j(document).ajaxStop(function(evt, request, settings){
		   		stopLoading();
		 	});
		 		
			var datasets = jsondata;
     		
			//tarkistetaan löytyikö dataa
			var datasets_is_null = true;
			
			$j.each(datasets, function(indeksi, alkio) {
				if ( alkio.data.length > 0 ) {
					datasets_is_null = false;
				}
			});
			
     		if ( datasets_is_null == true ) {	
	     		stopLoading();				
				document.getElementById('alert_box').innerHTML = ("<font color=#CC0000>Null data!</font>");
				return -1;			
     		}
		    	
		    // hard-code color indices to prevent them from shifting as
		    var i = 0;
			
			// Merkitään värit
			var colorValues = new Array();
			
			colorValues[0] = "#c43e3e";
			colorValues[1] = "#66cc66";
			colorValues[2] = "#5b93bf";
			colorValues[3] = "#663333";
			colorValues[4] = "#FFCC00";
			colorValues[5] = "#336600";
			colorValues[6] = "#996600";
			colorValues[7] = "#FFcc00";
			colorValues[8] = "#003300";
			colorValues[9] = "#3366CC";
			
		    $j.each(datasets, function(key, val) {
		        val.color = colorValues[i];
		        ++i;
		    });
			
	    	var options;	    
	    	var overview_options;		    
	    	var tolppa = f_reso*51000; //palkin leveys
		    	
    		if ( (selectedTypes[0]=="energy") || (selectedTypes[0]=="factor") ) {
			
				var sensorValues = new Array();
				
				$j.each(datasets, function(indeksi, alkio) {	//käydään läpi olio					
				
					var Max = -10000000;
			    	var Min = 10000000;
			    	
			    	var Sum = 0;
			    	var Mean = 0;
				    
			    	for (var j = 0; j < alkio.data.length; ++j) { //x-value, y-value
					
			        	if ( alkio.data[j][1]>Max ) {
				        	Max = alkio.data[j][1];
			        	}
			        	if ( alkio.data[j][1]<Min ) {
				        	Min = alkio.data[j][1];
			        	}
			        	
			        	Sum += alkio.data[j][1];		        
			    	}
				    
			    	Mean = (Sum/alkio.data.length).toFixed(2); //Mean Mass
			    	
			    	Min = Min.toFixed(2);			    	
			    	Max = Max.toFixed(2);
			    	Sum = Sum.toFixed(2);
				    					  	  			      
					sensorValues[indeksi] = {color: alkio.color, unit: alkio.unit, id: alkio.label, minVal: Min, meanVal: Mean, maxVal: Max, sum: Sum}; 		    	
			    	
			    	f_unit = alkio.unit;	//pop up tooltipille
				});
					
			    
			    var numerical_table = "<table id='mytable' align='center'><tr><th>ID</th><th>MIN</th><th>MEAN (<span style='text-decoration:overline'>x</span>)</th><th>MAX</th>";
				
				if (selectedTypes[0] == "energy") { // lisätään summasolu
					numerical_table += "<th>SUM (&#931x)</th></tr>";
				}
				
		    	numerical_table += "</tr>";
				
		    	$j.each(sensorValues, function(indeksi, alkio) {	//käydään läpi olio
					
					numerical_table = numerical_table + "<tr>" + "<td>" + '<span style="color:' + alkio.color + ';background:' + alkio.color + ';border:1px solid ' + alkio.color + '">--</span>' + '<input type="checkbox" name="' + indeksi + '" checked="checked" >' + selectedSensorsNames[indeksi] + ' ' + '</input>' + "</td>" + "<td>";					
					numerical_table += alkio.minVal + " " + alkio.unit + "</td>" + "<td>" + alkio.meanVal + " " + alkio.unit + "</td>" + "<td>" + alkio.maxVal + " " + alkio.unit + "</td>";
					
					if (selectedTypes[0] == "energy") { // lisätään summasolun tiedot
						numerical_table += "</td><td>" + alkio.sum + " " + alkio.unit + "</td>";
					}
					
				});
				
				numerical_table = numerical_table + "</table>";
				
				document.getElementById('numerical_div').innerHTML = numerical_table;				
		    
			    options = {
			        xaxis: { mode: "time", timeformat: "%H:%M<br>%d/%m" },
			        yaxis: { min: 0 }, 
			        selection: { mode: "x" },
			        grid: { hoverable: true, autoHighlight: true, clickable: false, markings: weekendAreas },
			        legend: { show: false },
			        series: {
				        bars: { show: true, align: "left", barWidth: tolppa }
			        }
			    };
			    
			    overview_options = {  
			        xaxis: { ticks: [], mode: "time" },
			        yaxis: { ticks: [], min: 0},
			        selection: { mode: "x" },
			        legend: { show: false },
			        series: {
				        bars: { show: true, align: "left", barWidth: tolppa }
			        }
			    };		    
			}
		
			else if ( (selectedTypes[0]=="power") || (selectedTypes[0]=="temp") ) {
				
				var sensorValues = new Array();
			
				$j.each(datasets, function(indeksi, alkio) {	//käydään läpi olio	
				
					var Max = -10000000;
			    	var Min = 10000000;
			    	
			    	var MeanSum = 0;
			    	var Mean = 0;
			    	
			    	var Last = 0;
				    
			    	for (var j = 0; j < alkio.data.length; ++j) { //x-value, y-value
					
			        	if ( alkio.data[j][1] > Max ) {
				        	Max = alkio.data[j][1];
			        	}
			        	if ( alkio.data[j][1] < Min ) {
				        	Min = alkio.data[j][1];
			        	}
			        	
			        	MeanSum += alkio.data[j][1];
			    	}
			    	
					var decimals = 0;
					
					if (selectedTypes[0]=="temp") {
						decimals = 1;
					}
					
			    	Last = (alkio.data[alkio.data.length-1][1]).toFixed(decimals);				    
			    	Mean = (MeanSum/alkio.data.length).toFixed(decimals); //Mean
			    	Min = Min.toFixed(decimals);			    	
			    	Max = Max.toFixed(decimals);	    	
			    	
			    	sensorValues[indeksi] = {color: alkio.color, unit: alkio.unit, id: alkio.label, minVal: Min, meanVal: Mean, maxVal: Max, lastVal: Last}; 
			    				    	
			    	f_unit = alkio.unit;	//pop up tooltipille				    
		    	});
		    	
		    	
		    	var numerical_table = "<table id='mytable'><tr><th>ID</th><th>MIN</th><th>MEAN (<span style='text-decoration:overline'>x</span>)</th><th>MAX</th><th>LAST</th></tr>";				    	
		    	
		    	$j.each(sensorValues, function(indeksi, alkio) {	//käydään läpi olio	    
				    
					numerical_table = numerical_table + "<tr>" + "<td>" + '<span style="color:' + alkio.color + ';background:' + alkio.color + ';border:1px solid ' + alkio.color + '">--</span>' + '<input type="checkbox" name="' + indeksi + '" checked="checked" >' + selectedSensorsNames[indeksi] + ' ' + '</input>' + "</td>" + "<td>";		
					numerical_table += alkio.minVal + " " + alkio.unit + "</td>" + "<td>" + alkio.meanVal + " " + alkio.unit + "</td>" + "<td>" + alkio.maxVal + " " + alkio.unit;	
					numerical_table += "</td>"  + "<td>" + alkio.lastVal + " " + alkio.unit +  "</td>";
					
				});
				
				numerical_table = numerical_table + "</table>";
				
				document.getElementById('numerical_div').innerHTML = numerical_table;
		    
			    options = {
			        xaxis: { mode: "time", timeformat: "%H:%M<br>%d/%m" },
			        selection: { mode: "xy" },
			        grid: { hoverable: true, autoHighlight: true, clickable: false, markings: weekendAreas },
			        legend: { show: false },
			        series: {
				    	lines: { show: true },
		       			points: { show: false }
			        }
			    };
			    
			    overview_options = {
			        xaxis: { ticks: [], mode: "time" },
			        selection: { mode: "xy" },
			        legend: { show: false },
			        series: {
				    	lines: { show: true },
		       			points: { show: false }
			        }
			    };		    
			}
		    
			var choiceContainer = $j("#numerical_div");
		    
			//CHECK checkboxes   
			choiceContainer.find("input").click(plotAccordingToChoices);
			
			var plot;		    
		    var overview;
		    
		    function plotAccordingToChoices() {
			    
			    var data = [];
	    
		        choiceContainer.find("input:checked").each(function () {
		            var key = $j(this).attr("name");
		            if (key && datasets[key])
		                data.push(datasets[key]);
		        });
		        		
		        if (data.length > 0) {		        
		            plot = $j.plot($j("#chart_energy_div"), data, options);		            
		            overview = $j.plot($j("#overview"), data, overview_options);		        
		        }
		        
		        // Now connect the two
				
				$j("#chart_energy_div").bind("plotselected", function (event, ranges) {
					
					// clamp the zooming to prevent eternal zoom
					if (ranges.xaxis.to - ranges.xaxis.from < 0.00001)
						ranges.xaxis.to = ranges.xaxis.from + 0.00001;
					if (ranges.yaxis.to - ranges.yaxis.from < 0.00001)
						ranges.yaxis.to = ranges.yaxis.from + 0.00001;
						
					// do the zooming
					plot = $j.plot($j("#chart_energy_div"), data,
								  $j.extend(true, {}, options, {
									  xaxis: { min: ranges.xaxis.from, max: ranges.xaxis.to },
									  yaxis: { min: ranges.yaxis.from, max: ranges.yaxis.to }                        
								  }));
			
					// don't fire event on the overview to prevent eternal loop
					overview.setSelection(ranges, true);
				});
				
				$j("#overview").bind("plotselected", function (event, ranges) {
					plot.setSelection(ranges);
					  
				});
				
				$j("#chart_energy_div").bind("plothover", function (event, pos, item) {
			
				   if (item) {
					   if (previousPoint != item.datapoint) {
						   previousPoint = item.datapoint;
								
						   $j("#tooltip").remove();
						   var x = parseInt(item.datapoint[0].toFixed(2));
						   var y = item.datapoint[1].toFixed(2);                  
								   
						   showTooltip(item.pageX, item.pageY,  y + " " + f_unit, x, selectedTypes[0]);
					   }
					}
					else {
					   $j("#tooltip").remove();
					   previousPoint = null;            
					}
				   
				});
		    
		    } //plotAccordingToChoises ends
			
			plotAccordingToChoices();
			
		}); //Ajax.Request ends 

} //getGraphics ends

function getStatistics(log) { 
	
    // Valinta
	var start_date = "null";
	var end_date = "null";
	
	if (log == 1) {
		start_date = document.dform1.f_date_c11.value; 
		end_date = document.dform1.f_date_c12.value;
	}
	if (log == 2) {
		start_date = document.dform2.f_date_c21.value; 
		end_date = document.dform2.f_date_c22.value;
	}
	
	var d1 = start_date.slice(0,2);
	var m1 = start_date.slice(3,5);
	var y1 = start_date.slice(6,10);
	
	var d2 = end_date.slice(0,2);
	var m2 = end_date.slice(3,5);
	var y2 = end_date.slice(6,10);
	
    var date1_obj = new Date(y1,m1-1,d1); //new date object
    var date2_obj = new Date(y2,m2-1,d2);
    
    var today_obj = new Date(system_year, system_month-1, system_day);
    
    if ( date1_obj.getTime() > today_obj.getTime() ) { //milliseconds difference
			
		document.getElementById('alert_box').innerHTML = ("<font color=#CC0000>Start date is in future!</font>");		
		return -1;

	} else if ( date2_obj.getTime() > today_obj.getTime() ) { //milliseconds difference
			
		document.getElementById('alert_box').innerHTML = ("<font color=#CC0000>End date is in future!</font>");
		return -1;

	} else if ( date1_obj.getTime() > date2_obj.getTime() ) { //milliseconds difference
			
		document.getElementById('alert_box').innerHTML = ("<font color=#CC0000>Start date is bigger than end date!</font>");
		return -1;
		
	} else {
		document.getElementById('alert_box').innerHTML = "";
	}
	
	var RightNow = new Date();	
	var msecs = RightNow.getTime(); //time to prevent GET caching
	
	var dataurl = "/apps/get_statistics.cgi?"
	var sensor_id = 0;
	
	dataurl += "time=" + msecs;
	dataurl += "&date1=" + start_date;
	dataurl += "&date2=" + end_date;
	dataurl += "&si=" + si;
	dataurl += "&sensor_id=" + sensor_id;
	
	var log_div = "empty";
	
	if (log == 1) {
		log_div = "log1_div";
	}
	if (log == 2) {
		log_div = "log2_div";
	}
	
	new Ajax.Updater(log_div, dataurl, {
		asynchronous: true,
		onCreate: startLoading,
		onComplete: stopLoading
	});
	
	/*
	//vaihtoehtoinen tapa
	Modalbox.show(arvo, {title: "Statistics", width: 440});
	*/
}

function chart_live() {
	
	var RightNow = new Date();
	var msecs = RightNow.getTime(); //time to prevent GET caching

	var datatiedosto = "/apps/socket_client.cgi?time=" + msecs + "&type=2" + "&si=" + si;
	
	$j.get(datatiedosto, function(strdata, textStatus) {
		
		if ( strdata.length < 1 ) { //CONNECTION ERROR OR NULL DATA		
			return -1;
		}
		
		var d_live = []; //data taulukko

        var splitdata = strdata.split(",");
        splitdata.pop(); //poistetaan yksi turha alkio lopusta, johtuu viimeisestä "," merkistä     
		
        jmax = splitdata.length;
        
        var x_arvo = 0; //int
        var y_arvo = 0; //int
        
        for(var j = 0; j < (jmax - 1); j += 2) { //x-value, y-value
        
        	x_arvo = parseInt(splitdata[j]);
	        y_arvo = parseInt(splitdata[j+1]);
			
            d_live.push([x_arvo, y_arvo]); 
        } 
        
        //create object
        var data = [ { data: d_live } ];
        
        //add color
	    $j.each(data, function(key, val) {
	        val.color = "rgb(255, 0, 0)";
	    });
        
        var options = {
			xaxis: { ticks:4, mode: "time", timeformat: "%H:%M" },
		    yaxis: { tickFormatter: function (v, axis) { return v.toFixed(axis.tickDecimals) +" W" }},
	        grid: { hoverable: true, autoHighlight: true, clickable: false},
	        series: {
		        lines: { show: true , lineWidth: 2},
		        shadowSize: 0
	        }
        };
		
        if (data.length > 0) {
			try {
				$j.plot( $j("#chart_live_div"), data, options );
			} catch (e) {
				// do nothing, käyttäjä shadowbox ikkunan
			}
   		}
		
	    $j("#chart_live_div").bind("plothover", function (event, pos, item) {
		
	       if (item) {
	            if (previousPoint != item.datapoint) {
	                previousPoint = item.datapoint;
	                
	                $j("#tooltip").remove();
	                var x = parseInt(item.datapoint[0].toFixed(2));
	                var y = parseInt(item.datapoint[1].toFixed(2));                  
	                   
	                showTooltipSpecial(item.pageX, item.pageY,  y + " W", x, "live");
	            }
	        }
	        else {
	            $j("#tooltip").remove();
	            previousPoint = null;            
	        }
			
	    });   
    
	});

} //chart_live() function ends

function chart_small_live(chart_type, chart_div) {
	
	var RightNow = new Date();
	var msecs = RightNow.getTime(); //time to prevent GET caching
	
	var datatiedosto = "/apps/socket_client.cgi?time=" + msecs + "&type=" + chart_type + "&si=" + si;
		
	$j.get(datatiedosto, function(strdata, textStatus) {
		
		if ( strdata.length < 1 ) { //CONNECTION ERROR OR NULL DATA
			return -1;
		}
		
		var d_live = []; //data taulukko
		
        var splitdata = strdata.split(",");
        splitdata.pop(); //poistetaan yksi turha alkio lopusta, johtuu viimeisestä "," merkistä     
		
        jmax = splitdata.length;
        
        var x_arvo = 0; //int
        var y_arvo = 0; //int
        
        for(var j = 0; j < (jmax - 1); j += 2) { //x-value, y-value
        
        	x_arvo = parseInt(splitdata[j]);
	        y_arvo = parseInt(splitdata[j+1]);
			
            d_live.push([x_arvo, y_arvo]); 
        }   
            
        //create object
        var data = [ { data: d_live } ];
        
        //add color
	    $j.each(data, function(key, val) {
	        val.color = "rgb(255, 0, 0)";
	    });
        
        var options = {
			xaxis: { ticks: 2, mode: "time", timeformat: "%H:%M" },
		    yaxis: { tickFormatter: function (v, axis) { return v.toFixed(axis.tickDecimals) +" W" }},
	        grid: { hoverable: true, autoHighlight: true, clickable: false},
	        series: {
		        lines: { show: true , lineWidth: 2},
		        shadowSize: 0
	        }
        };

        if (data.length > 0) {
            $j.plot( $j(chart_div), data, options );
   		}
	    
	    $j(chart_div).bind("plothover", function (event, pos, item) {      
	        
	       if (item) {
		        
		        if (previousPoint != item.datapoint) {
			        previousPoint = item.datapoint;
	                    
	                $j("#tooltip").remove();
	                var x = parseInt(item.datapoint[0].toFixed(2));
	                var y = parseInt(item.datapoint[1].toFixed(2));                  
	                       
	                showTooltip(item.pageX, item.pageY,  y + " W", x, "live");
	            }
	            
	       } else {
	            $j("#tooltip").remove();
	            previousPoint = null;                   
	       }
	       
	     });    
     });    
    
} //function chart_small_live() ends

function FFT_live() {
	
	var RightNow = new Date();
	var msecs = RightNow.getTime(); //time to prevent GET caching

	var datatiedosto = "/apps/socket_client.cgi?time=" + msecs + "&type=3" + "&si=" + si;
		
	$j.get(datatiedosto, function(strdata, textStatus) {
		
		if ( strdata.length < 1 ) { //CONNECTION ERROR OR NULL DATA
			return -1;
		}
		
		var d_live = []; //data taulukko

        var splitdata = strdata.split(",");
        splitdata.pop(); //poistetaan yksi turha alkio lopusta, johtuu viimeisestä "," merkistä
		
		// jätetään DC-komponentti näyttämättä gaafissa ja näytetään tekstinä
		var dc = parseInt(splitdata[1]);
		var window = splitdata.length - 2;
		
		document.getElementById('alert_box').innerHTML = '<br/>DC-component: ' + dc + 
														 ' W, FFT length: ' + window;
		
		//käydään läpi saadut tiedot
        for(var j = 2; j < (splitdata.length - 1); j += 2) { //x-value, y-value
            d_live.push([ splitdata[j], splitdata[j + 1] ]);        
        }

        //create object
        var data = [ { data: d_live } ];
        
        //add color
	    $j.each(data, function(key, val) {
	        val.color = "rgb(255, 0, 0)";
	    });
		
        var options = {
		    xaxis: { tickFormatter: function (v, axis) { return v.toFixed(axis.tickDecimals) +" mHz" }},
			yaxis: { tickFormatter: function (v, axis) { return v.toFixed(axis.tickDecimals) +" W" }},
	        grid: { hoverable: true, autoHighlight: true, clickable: false},
	        series: {
		        bars: { show: true }
	        }
        };

        if (data.length > 0) {
            $j.plot( $j("#FFT_div"), data, options );
   		}
	    
	    $j("#FFT_div").bind("plothover", function (event, pos, item) {      
	        
	       if (item) {
		        
		        if (previousPoint != item.datapoint) {
			        previousPoint = item.datapoint;
	                    
	                $j("#tooltip").remove();
					//T = 1 / f, 1000 jako, koska kyseessä mHz
	                var T = (1/(item.datapoint[0]/1000)).toFixed(1);
					var f = parseInt((item.datapoint[0]).toFixed(2));
	                var H = parseInt(item.datapoint[1].toFixed(2));          
	                
	                showFFTTooltip(item.pageX, item.pageY, "<b>Amplitude |H(f)| = </b>" + H + " W", "<b>Frequency f = </b>" + f + " mHz", "<b> Period T = 1/f = </b>" + T + " sec");
	            }
	            
	       } else {
	            $j("#tooltip").remove();
	            previousPoint = null;
	       }
	       
	     });    
     });    
    
} //function FFT_live() ends

function showFFTTooltip(x, y, H, f, T) {

	$j('<div id="tooltip">' + H + '<br/>' + f + '<br/>' + T + '</div>').css( {
		'position': 'absolute',
		'display': 'none',
		'top': y - 70,
		'left': x - 5,
		'color': '#000000',
		'border': '1px solid #fdd',
		'padding': '2px',
		'background-color': '#ff0',
		'opacity': '0.85'
	}).appendTo("body").show();
  
}

function chart_week() {	

	var RightNow = new Date();
	var msecs = RightNow.getTime(); //time to prevent GET caching	
	var f_reso = 1440;	
	var f_number = 1;
	var start_date = format_date(-14);	
	var end_date = format_date(0);
	var f_type = "mass";
	var sensor_id = 0;
	
	var dataurl= "/apps/get_file_archive.cgi?"
	
	dataurl += "time=" + msecs;
	dataurl += "&f_reso=" + f_reso;
	dataurl += "&f_number=" + f_number;
	dataurl += "&date1=" + start_date;
	dataurl += "&date2=" + end_date;
	dataurl += "&si=" + si;	
	dataurl += "&f_type=" + f_type;
	dataurl += "&sensor_id=" + sensor_id;
		
	//loading animation
	startLoading();
	
	$j.getJSON(dataurl, function (jsondata) {
		
   			var data = jsondata;
   			
		    $j.each(data, function(key, val) {
		        val.color = "rgb(230, 200, 0)";
		    });
		    
		    //loading animation
   			stopLoading();
   			
	        var tolppa = f_reso * 51000; //palkin leveys
	        
	        var options = {
		        	yaxis: { min: 0, ticks: 3, tickFormatter: function (v, axis) { return v.toFixed(axis.tickDecimals) + " kWh" }},
	                xaxis: { mode: "time", timeformat: "%d.%m", minTickSize: [1, "day"]},
	                grid: { hoverable: true, autoHighlight: true, clickable: false, markings: weekendAreas },
	                legend: { show: false },
	                series: {
	               		bars: { show: true, align: "left", barWidth: tolppa}	
        			}
	        };
	
	        if (data.length > 0) {
	            $j.plot( $j("#chart_week_div"), data, options );
	   		}
		    
		    $j("#chart_week_div").bind("plothover", function (event, pos, item) {
		
		        if (item) {
		            if (previousPoint != item.datapoint) {
		                previousPoint = item.datapoint;
		                
		                $j("#tooltip").remove();
		                var x = parseInt(item.datapoint[0].toFixed(1));
		                var y = item.datapoint[1].toFixed(1);                  
		                   
		                showTooltip(item.pageX, item.pageY,  y + " kWh", x, "week");
		            }
		        }
		        else {
		            $j("#tooltip").remove();
		            previousPoint = null;            
		        }
		        
		    });


		}); //Ajax.Request ends 

} //function chart_week() ends

function chart_forecast() {
	
	//flot options
    var options = {
	    xaxis: { mode: "time", timeformat: "%m/%y", minTickSize: [1, "day"]},
	    yaxis: { min: 0, tickFormatter: function (v, axis) { return v.toFixed(axis.tickDecimals) +" kWh" }},
        grid: { hoverable: true, autoHighlight: true, clickable: false },
	    series: {
		    bars: { show: true, align: "center", barWidth: 1000000000}
   	 	}
    };
    
    //create object
    var data = [ { data: d_year } ];
        
    //add color
	$j.each(data, function(key, val) {
        val.color = "rgb(255, 0, 0)";
    });
    
    var plot = $j.plot($j("#chart_year_div"), data, options );
    
    $j("#chart_year_div").bind("plothover", function (event, pos, item) {

        if (item) {
            if (previousPoint != item.datapoint) {
                previousPoint = item.datapoint;
                
                $j("#tooltip").remove();
                var x = parseInt(item.datapoint[0].toFixed(0));
                var y = item.datapoint[1].toFixed(0);                
                   
                showTooltip(item.pageX, item.pageY,  y + " kWh", x, "year");
            }
        }
        
        else {
            $j("#tooltip").remove();
            previousPoint = null;            
        }
        
    });     
    
} //function chart_forecast() ends

function weekendAreas(axes) { // helper for returning the weekends in a period

    var markings = [];
    var d = new Date(axes.xaxis.min);
    // go to the first Saturday
    d.setUTCDate(d.getUTCDate() - ((d.getUTCDay() + 1) % 7))
    d.setUTCSeconds(0);
    d.setUTCMinutes(0);
    d.setUTCHours(0);
    var i = d.getTime();
	
    do {
        // when we don't set yaxis the rectangle automatically
        // extends to infinity upwards and downwards
        markings.push({ xaxis: { from: i, to: i + 2 * 24 * 60 * 60 * 1000 }, color: "#EAEAEA" } );
        i += 7 * 24 * 60 * 60 * 1000;
    } while (i < axes.xaxis.max);
	
    return markings;
}

function showTooltip(x, y, contents, x_value, type) {
	
	var time = dateFormatter(x_value, type); 
	
	$j('<div id="tooltip"><b>' + contents + '</b><br/>' + time +'</div>').css( {
		'position': 'absolute',
		'display': 'none',
		'top': y - 60,
		'left': x - 30,
		'color': '#000000',
		'border': '1px solid #fdd',
		'padding': '2px',
		'background-color': '#ff0',
		'opacity': '0.85'
	}).appendTo("body").show();
	
}

function showTooltipSpecial(x, y, contents, x_value, type) {	
	
    var time = dateFormatter(x_value, type); 

    $j('<div id="tooltip"><b>' + contents + '</b><br/>' + time +'</div>').css( {
	    'font-size': '1.1em',
        'position': 'static',
        'display': 'none',
        'top': y - 50,
        'left': x - 200,
        'color': '#FFFFFF',
        'border': '1px solid #F0F0F0',
        'padding': '0.2em 0.2em 0.2em 0.6em',
        'background-color': '#666666',
        'opacity': '0.95',
        '-moz-border-radius': '7px',
	    '-webkit-border-radius': '7px',
	    '-khtml-border-radius': '7px',
	    'border-radius': '7px'
    }).appendTo("#MB_window").show(); //MODALBOX overlay div

}

function format_date(offset) {	

	//first pageload function
	var theday = 0;
	var themonth = 0;
	var theyear = 0;	
	
	var myDate = new Date(system_year, system_month - 1, system_day);	
	myDate.setDate(myDate.getDate()+offset);

	theyear = myDate.getYear();	

	if (theyear < 1000) {
		theyear += 1900;
	}
	themonth = myDate.getMonth() + 1;	
	
	if (themonth < 10){
		themonth = "0" + themonth;
	}
	theday=myDate.getDate();
	
	if (theday < 10) {
		theday = "0" + theday;		
	}
	
	//set date
	var date = theday + "-" + themonth + "-" + theyear;
	
	return date;
}

