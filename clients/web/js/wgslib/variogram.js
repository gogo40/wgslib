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
                    x: -60,
                    y: 25
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

function call_variogram(output)
{
    var props_selected = $("#select_props_value").val();

    for (var i  = 0; i < props_selected.length; ++i) {
        props_selected[i] = Number(props_selected[i]);
    }

    var var_params = {
            'grid_name'      : $("#grid_name").val(),
            'props_name'     : JSON.parse($("#props_name").val()),
            'props'          : JSON.parse($("#props").val()),
            'X_prop'         : Number($("#x_prop_value").val()),
            'Y_prop'         : Number($("#y_prop_value").val()),
            'Z_prop'         : Number($("#z_prop_value").val()),
            'dx'             : Number($("#dx").val()),
            'dy'             : Number($("#dy").val()),
            'dz'             : Number($("#dz").val()),
            'props_selected' : props_selected,
            'num_lags'       : Number($("#num_lags").val()),
            'directions'     : get_directions($("#num_directions"), $("#variogram_dirs_table"))
        };

    console.log(var_params);

	$(output).text("Computing...");
	
	//var url = "http://wgslib.com:8080";
	//var url = "http://143.54.155.233:8080";
    var url = "http://localhost:8080";

	var request = {};
	request.method = "compute_variograms";
	request.params = {'name' : 'teste WGSLIB'};
	request.id = 1;
	request.jsonrpc = "2.0";

	function displayResult(response) {
		console.log(response);

        plot(["var_1", "var_2", "var_3"], 
             [ [[1, 1], [2, 2], [3, 1]],
               [[1, 1], [2, 2], [3, 4]],
               [[1, 4], [2, 2], [3, 6]]  ], 
             -10, 10);

		if (response.result) {
			$(output).text(response.result);
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
