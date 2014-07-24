/*
WGSLIB - Web GeoStatistics Library
URL: http://wgslib.com

(c) 2014, Pericles Lopes Machado <pericles.raskolnikoff@gmail.com
*/

function process_grid(text, output, grid_name_output, x_output, y_output, z_output, props_output, props_value_output, props_name_output)
{
    var lines = text.split("\n");

    if (lines.length < 2) return false;

    var grid_name = lines[0];
    var n_columns = Number(lines[1]);

    if (lines.length < n_columns + 2) return false;

    grid_name_output.val(grid_name);

    var x_html;
    var y_html;
    var z_html;
    var select_props;
    var props = [];

    x_html = "<select id=\"x_prop_value\" class=\"form-control\">";
    y_html = "<select id=\"y_prop_value\" class=\"form-control\">";
    z_html = "<select id=\"z_prop_value\" class=\"form-control\">";
    select_props = "<select multiple class=\"form-control\" id=\"select_props_value\">";

    var id = 0;

    z_html += "<option value=\"-1\"> - </option>";
    
    for (var i = 2; i < n_columns + 2; ++i) {
        props.push(lines[i]);
        var option = "<option value=\"" + id + "\"> " + lines[i] + "</option>";
        
        x_html += option;
        y_html += option;
        z_html += option;
        select_props += option;

        ++id;
    } 

    x_html += "</select>";
    y_html += "</select>";
    z_html += "</select>";
    select_props += "</select>";

    x_output.html(x_html);
    y_output.html(y_html);
    z_output.html(z_html);
    props_output.html(select_props);
    


    var props_values = []
    for (var i = n_columns + 2; i < lines.length; ++i) {
        var values = lines[i].split(/[ \t\n]+/);
        if (values.length < n_columns) {
            output.append("Warning: Line " + i + ": wrong number of parameters!\n");
        } else { 
            for (var j = 0; j <  n_columns; ++j) {
                values[j] = Number(values[j]);
            }
            props_values.push(values);
        }
    }

    props_value_output.val(JSON.stringify(props_values));
    props_name_output.val(JSON.stringify(props));
    
    output.append("Grid loaded!\n");
    output.append("-------------------------------------\n");

    return true;
}


