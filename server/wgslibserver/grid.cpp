/*
WGSLIB - Web GeoStatistics Library
URL: http://wgslib.com

(c) 2014, Pericles Lopes Machado <pericles.raskolnikoff@gmail.com
*/

#include "grid.h"

 
WGSLGrid::WGSLGrid(int n_dims, int n_rows, int n_props)
: X_(n_rows), Y_(n_rows), Z_(n_rows), props_(n_props), n_dims_(n_dims)
{
    for (auto v : props_) {
        v.resize(n_rows);
    }
}

WGSLGrid::~WGSLGrid()
{
}

void WGSLGrid::set_location(int row, double x, double y, double z)
{
    X_[row] = x;
    Y_[row] = y;
    Z_[row] = z;
}

void WGSLGrid::set_property(int i, int row, double value)
{
    props_[i][row] = value;
}       

double WGSLGrid::get_value(int i, int row)
{
    return props_[i][row];
}

WSGLProperty& WGSLGrid::get_property(int i)
{
    return props_[i];
}

void WGSLGrid::clear_property(int i)
{
    props_[i].clear();
}

