/*
WGSLIB - Web GeoStatistics Library
URL: http://wgslib.com

(c) 2014, Pericles Lopes Machado <pericles.raskolnikoff@gmail.com
*/

#ifndef _WGSLGRID_H_
#define _WGSLGRID_H_

#include <common.h>
#include <vector>
#include <map>
#include <algorithm>

typedef std::vector<double> WSGLProperty;
typedef std::pair<double, double> Point;
typedef std::vector<Point> Points;

class WGSLIB_DECL WGSLGrid {
    public:
        WGSLGrid(int n_dims, int n_rows, int n_props);
        ~WGSLGrid();

        void set_location(int row, double x, double y, double z);
        void set_property(int i, int row,  double value);       
        double get_value(int i, int row);

        WSGLProperty& get_property(int i);
        void clear_property(int i);
        
    private:
        std::vector<double> X_;
        std::vector<double> Y_;
        std::vector<double> Z_;

        std::vector<WSGLProperty> props_;

        int n_dims_;
        
};

#endif // _WGSLGRID_H_

