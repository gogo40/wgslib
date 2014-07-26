/*
WGSLIB - Web GeoStatistics Library
URL: http://wgslib.com

(c) 2014, Pericles Lopes Machado <pericles.raskolnikoff@gmail.com
*/

function plot(labels, data, ymin, ymax) {
    
    var options = {
        series: {
            lines: {
                show: true
            },
            points: {
                show: true
            },
            tooltip: true,
            tooltipOpts: {
                content: "'%s' of %x.1 is %y.4",
                shifts: {
                    x: 0,
                    y: 0
                }
            }
        },
        grid: {
            hoverable: true //IMPORTANT! this is needed for tooltip to work
        },
        yaxis: {
            min: ymin,
            max: ymax
        }
    };

    var info = [];

    for (var i = 0; i < data.length; ++i) {
        info.push({data: data[i], label: labels[i] });
    }

     $("#variogram_plot_region").focus();
    var plotObj = $.plot($("#variogram_chart"), info, options);
    $("#variogram_plot_region").show();
}

function get_directions(num_directions, table)
{
    var num_dirs = Number(num_directions.val());
    var dirs = [];

    for (var i = 0; i < num_dirs; ++i) {
        dirs.push([Number($("#dir_x_"+i).val()), Number($("#dir_y_"+i).val()), Number($("#dir_z_"+i).val())]);
    }
    
    return dirs; 
}

function print_variogram_csv()
{
    var props_selected = $("#select_props_value").val();
    var props = JSON.parse($("#props_name").val());

    var text = "";
    for (var i = 0; i < props_selected.length; ++i) {
        var prop = props[Number(props_selected[i])];
        if (i > 0) text += " ,\t";
        text += "\"" + prop + "\""; 
    }
    text += "\n";

    var variog = JSON.parse($("#var_output").val());
    var dir_variog = Number($("#id_var_direction").val());

    for (var p = 0; p < variog[dir_variog][0].length; ++p) {
        for (var i = 0; i < variog[dir_variog].length; ++i) {
            if (i == 0) text +=  variog[dir_variog][i][p][0];
            text += " ,\t";
            text += variog[dir_variog][i][p][1];            
        }
        text += "\n";
    }
    $("#grid_text").text(text);
    console.log(text);
}

function plot_this_variogram()
{
    var props_selected = $("#select_props_value").val();
    var props = JSON.parse($("#props_name").val());

    for (var i = 0; i < props_selected.length; ++i) {
        props_selected[i] = props[Number(props_selected[i])];
    }

    var variog = JSON.parse($("#var_output").val());
    var ymin = 0;
    var ymax = 0;
    var f = true;

    var dir_variog = Number($("#id_var_direction").val());

    for (var i = 0; i < variog[dir_variog].length; ++i) {
        for (var p = 0; p < variog[dir_variog][i].length; ++p) {
            if (f) {    
                f = false; ymin = ymax = variog[dir_variog][i][p][1]; 
            } else {
                if (variog[dir_variog][i][p][1] < ymin) {
                    ymin = variog[dir_variog][i][p][1];
                }
                if (variog[dir_variog][i][p][1] > ymax) {
                    ymax = variog[dir_variog][i][p][1];
                }
            }  
        }
    }
   
    plot(props_selected, 
    variog[dir_variog], 
    ymin, ymax);

    print_variogram_csv();
}

function print_variograms(props_selected, props_name, directions)
{
    var options = "<select id=\"id_var_direction\" onchange=\"plot_this_variogram()\">";
    for (var i = 0; i < directions.length; ++i) {
        options += "<option value=\""+i+"\"> ["+directions[i][0]+"|"+directions[i][1]+"|"+directions[i][2]+"]</option>";
    }
    options += "</select>";
    $("#variograms").html(options);
}


function call_variogram(output)
{
    var props_selected = $("#select_props_value").val();

    if (props_selected === null) {
        alert("No property selected!");        
        return;
    }
    
	$(output).text("Computing...");
	
	//var url = "http://wgslib.com:8080";
	//var url = "http://143.54.155.233:8080";
    var url = "http://localhost:8080";

    for (var i = 0; i < props_selected.length; ++i) {
        props_selected[i] = Number(props_selected[i]);
    }

	var request = {};
    var directions = get_directions($("#num_directions"), $("#variogram_dirs_table"));
	request.method = "compute_variograms";
	request.params = {
            'X_prop'         : Number($("#x_prop_value").val()),
            'Y_prop'         : Number($("#y_prop_value").val()),
            'Z_prop'         : Number($("#z_prop_value").val()),
            'directions'     : directions,
            "dimensions"     : [Number($("#dx").val()), Number($("#dy").val()), Number($("#dz").val())],
            'grid_name'      : $("#grid_name").val(),            
            'num_lags'       : Number($("#num_lags").val()),
            'props'          : JSON.parse($("#props").val()),                        
            'props_name'     : JSON.parse($("#props_name").val()),
            'props_selected' : props_selected
        };
	request.id = 1;
	request.jsonrpc = "2.0";

	function displayResult(response) {
        var props_selected = $("#select_props_value").val();
        var props = JSON.parse($("#props_name").val());

        for (var i = 0; i < props_selected.length; ++i) {
            props_selected[i] = props[Number(props_selected[i])];
        }

		if (response.result) {
            console.log(response.result);
            print_variograms(props_selected, props, get_directions($("#num_directions"), $("#variogram_dirs_table")));
            $("#var_output").val(JSON.stringify(response.result));
			plot_this_variogram();        
		} else if (response.error) {
			alert("Error: " + response.error.message);
		}
	};

	$.post(url, JSON.stringify(request), displayResult, "json");
}  


function update_variogram_directions(num_directions, table_output)
{
    var num_dirs = Number(num_directions.val());
    console.log(num_dirs);

    var table = '<div class="table-responsive">';
    table += '<table class="table table-striped table-bordered table-hover" id="variogram_dirs_table">';
    table += '<thead>';
    table += '<tr>';
    table += '<th>X</th>';
    table += '<th>Y</th>';
    table += '<th>Z</th>';
    table += '</tr>';
    table += '</thead>';
    table += '<tbody>';

    for (var i = 0; i < num_dirs; ++i) {
        table += '<tr class="odd gradeX">';
        table += '<td> <input type="text" id="dir_x_'+ i +'" value="0" /></td>';
        table += '<td> <input type="text" id="dir_y_'+ i +'" value="0" /></td>';
        table += '<td> <input type="text" id="dir_z_'+ i +'" value="0" /> </td>';
    }
    
    table += '</tr>';
    table += '</tbody>';                                    

    table += '</table>';                                
    table += '</div>';
    table_output.html(table);    
}

