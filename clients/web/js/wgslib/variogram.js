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

    for (var p = 0; p < variog['data'][dir_variog][0].length; ++p) {
        for (var i = 0; i < variog['data'][dir_variog].length; ++i) {
            if (i == 0) text +=  variog['data'][dir_variog][i][p][0];
            text += " ,\t";
            text += variog['data'][dir_variog][i][p][1];            
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
    var ymin = variog['range'][0];
    var ymax = variog['range'][1];
    var f = true;

    var dir_variog = Number($("#id_var_direction").val());

   
    plot(props_selected, 
    variog['data'][dir_variog], 
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
    
    var url = "http://localhost:8383";

    var rpcclient = new WGSLibStubClient(url);
	
	var X_prop = Number($("#x_prop_value").val()), 
        Y_prop = Number($("#y_prop_value").val()), 
        Z_prop = Number($("#z_prop_value").val()), 
        dimensions =  [Number($("#dx").val()), Number($("#dy").val()), Number($("#dz").val())], 
        directions = get_directions($("#num_directions"), $("#variogram_dirs_table")), 
        grid_name = $("#grid_name").val(), 
        num_lags = Number($("#num_lags").val()), 
        props = JSON.parse($("#props").val()), 
        props_name = JSON.parse($("#props_name").val()), props_selected = $("#select_props_value").val();

    if (props_selected === null) {
        alert("No property selected!");        
        return;
    }

    $(output).text("Computing...");
    for (var i = 0; i < props_selected.length; ++i) {
        props_selected[i] = Number(props_selected[i]);
    }

    function showError(code, message) {
        alert("Error: " + code + " -> " + message);
    }

	function displayResult(id, response) {
        console.log(id);
        console.log(response);

        var props_selected = $("#select_props_value").val();
        var props = JSON.parse($("#props_name").val());

        for (var i = 0; i < props_selected.length; ++i) {
            props_selected[i] = props[Number(props_selected[i])];
        }

	    console.log(response);
        print_variograms(props_selected, props, get_directions($("#num_directions"), $("#variogram_dirs_table")));
        $("#var_output").val(JSON.stringify(response));
		plot_this_variogram();        
	};

    //rpcclient.notifyServer(showError, showError);
	
    rpcclient.compute_variograms(X_prop, Y_prop, Z_prop, 
    dimensions, directions, grid_name, num_lags, props, props_name, props_selected, 
    displayResult, showError);
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

