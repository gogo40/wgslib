/*
WGSLIB - Web GeoStatistics Library
URL: http://wgslib.com

(c) 2014, Pericles Lopes Machado <pericles.raskolnikoff@gmail.com
*/

//jsonrpcstub -s -c -o /Users/cinemast/Desktop spec.json MyStub
#include "wgslibserver.h"
#include "fftvarmapcalc.h"
#include "grid.h"

#include <cmath>

WGSLIB_DECL void task_sleep(int time)
{
#ifdef  __linux
    usleep(time * 1000);
#elif WIN32 || _WIN32
    Sleep(time);
#else
    usleep(time * 1000);
#endif
}

/*HTTP SERVER*/

WGSLibHttpServer::WGSLibHttpServer(int port, bool enableSpecification, const std::string& sslcert, int threads)
    : HttpServer(port, enableSpecification, sslcert, threads) 
{}


bool WGSLibHttpServer::SendResponse(const std::string& response, void* addInfo)
{
	struct mg_connection* conn = (struct mg_connection*) addInfo;
	if (mg_printf(conn, "HTTP/1.1 200 OK\r\n"
		      "Content-Type: application/json\r\n"
		      "Content-Length: %d\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
		      "\r\n"
		      "%s",(int)response.length(), response.c_str()) > 0)
	{
	    return true;
	}
	else
	{
	    return false;
	}
}

/*RPC SERVER*/

WGSLibStubServer::WGSLibStubServer() :
    AbstractWGSLibStubServer(new WGSLibHttpServer(8080))
{
    is_running_ = true;
}

#define VARIOGRAM_MAX_SIZE 5000000

Json::Value WGSLibStubServer::compute_variograms(const int& X_prop, const int& Y_prop, const int& Z_prop, const Json::Value& dimensions, const Json::Value& directions, const std::string& grid_name, const int& num_lags, const Json::Value& props, const Json::Value& props_name, const Json::Value& props_selected)
{
    std::cout<< "Compute variogram\n";
    std::cout << "Grid_name: " << grid_name << std::endl;
    std::cout << "props_name: " << props_name << std::endl;

    Json::Value response;

    if (props.size() > VARIOGRAM_MAX_SIZE) return response;

    double dx = dimensions[0u].asDouble();
    double dy = dimensions[1u].asDouble();
    double dz = dimensions[2u].asDouble();

    if (dx < 1e-6) dx = 1;
    if (dy < 1e-6) dy = 1;
    if (dz < 1e-6) dz = 1;

    int n_dims = 3;

    if (Z_prop < 0) {
        n_dims = 2;
    }

    std::vector<int> dirs[2][2][2];
    std::vector<std::vector<Points> > variogs(directions.size());

    for (int i = 0 ; i < directions.size(); ++i) {
        variogs[i].resize(props_selected.size());
    }

    for (int i = 0; i < directions.size(); ++i) {
        bool inv_x = (directions[i][0u].asDouble() < 0);
        bool inv_y = (directions[i][1u].asDouble() < 0);
        bool inv_z = (directions[i][2u].asDouble() < 0);

        dirs[inv_x][inv_y][inv_z].push_back(i);
    }


    for(int ip = 0; ip < props_selected.size(); ++ip) {
        int p = props_selected[ip].asInt();
        double Xmin = 0, Ymin = 0, Zmin = 0;
        double Xmax = 0, Ymax = 0, Zmax = 0;

        for (int i = 0; i < props.size(); ++i) {
            double X = props[i][X_prop].asDouble();
            double Y = props[i][Y_prop].asDouble();
            double Z = 0;
            if (Z_prop > 0) Z = props[i][Z_prop].asDouble();

            if (i == 0) {
                Xmin = Xmax = X;
                Ymin = Ymax = Y;
                Zmin = Zmax = Z;
            } else {
                if (X > Xmax) Xmax = X;
                if (Y > Ymax) Ymax = Y;
                if (Z > Zmax) Zmax = Z;

                if (X < Xmin) Xmin = X;
                if (Y < Ymin) Ymin = Y;
                if (Z < Zmin) Zmin = Z;
            }
        }

        int Nx = static_cast<int>(ceil((Xmax - Xmin) / dx)) + 1;
        int Ny = static_cast<int>(ceil((Ymax - Ymin) / dy)) + 1;
        int Nz = static_cast<int>(ceil((Zmax - Zmin) / dz)) + 1;

        if (Nx * Ny * Nz + 1 > VARIOGRAM_MAX_SIZE) return response;
        std::vector<double> data(Nx * Ny * Nz + 1);
        std::vector<int> hp(Nx * Ny * Nz + 1, 0);

        for (int id = 0; id < 8; ++id) {
            bool inv_x = (id & 0x1) > 0;
            bool inv_y = (id & 0x2) > 0;
            bool inv_z = (id & 0x4) > 0;


            if (dirs[inv_x][inv_y][inv_z].size() > 0) {
                for (int i = 0; i < props.size(); ++i) {
                    double X = props[i][X_prop].asDouble();
                    double Y = props[i][Y_prop].asDouble();
                    double Z = 0;
                    if (Z_prop > 0) Z = props[i][Z_prop].asDouble();

                    double v = props[i][p].asDouble();

                    int ix = static_cast<int>((X - Xmin) / dx);
                    int iy = static_cast<int>((Y - Ymin) / dy);
                    int iz = static_cast<int>((Z - Zmin) / dz);

                    ix = (inv_x)?(Nx - ix - 1):(ix);
                    iy = (inv_y)?(Ny - iy - 1):(iy);
                    iz = (inv_z)?(Nz - iz - 1):(iz);

                    data[(ix * Ny + iy) * Nz + iz] = v;
                    hp[(ix * Ny + iy) * Nz + iz] = 1;

                }
                VarOut varmap(data.size());
                fft_varmap_3d(varmap, data, hp, Nx, Ny, Nz);

                for (int j = 0; j < dirs[inv_x][inv_y][inv_z].size(); ++j) {
                    int d = dirs[inv_x][inv_y][inv_z][j];
                    int ix = 0, iy = 0, iz = 0;
                    for (int l = 0; l < num_lags; ++l) {


                        if (ix > -1 && ix < Nx && iy > -1 && iy < Ny && iz > -1 && iz < Nz) {
                            double x = ix * dx;
                            double y = iy * dy;
                            double z = iz * dz;

                            double lag = sqrt(x * x + y * y + z * z);
                            double NI = varmap.ni[(ix * Ny + iy) * Nz + iz];
                            double v = varmap.varmap[(ix * Ny + iy) * Nz + iz];

                            if (NI > 1e-4) {
                                variogs[d][ip].push_back(Point(lag, v));
                            }
                        }

                        ix += abs(directions[d][0u].asInt());
                        iy += abs(directions[d][1u].asInt());
                        iz += abs(directions[d][2u].asInt());
                    }

                }
            }
        }
    }


    double vmin = 0, vmax = 0;
    bool first = true;
    int idir = 0, iprop = 0, ivar = 0;
    for (auto var_dir: variogs) {
        iprop = 0;
        for (auto var_prop: var_dir) {
            ivar = 0;
            for (auto p: var_prop) {
                response["data"][idir][iprop][ivar][0u] = p.first;
                response["data"][idir][iprop][ivar][1u] = p.second;

                if (first) {
                    first = false;
                    vmin = vmax = p.second;
                } else {
                    if (p.second < vmin) vmin = p.second;
                    if (p.second > vmax) vmax = p.second;
                }
                ++ivar;
            }
            ++iprop;
        }
        ++idir;
    }

    response["range"][0u] = vmin;
    response["range"][1u] = vmax;

    return response;
}


void WGSLibStubServer::notifyServer()
{
    is_running_ = false;
}

bool WGSLibStubServer::isRunning()
{
    bool r = is_running_;
    return r;
}

