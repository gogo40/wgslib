/*
WGSLIB - Web GeoStatistics Library
URL: http://wgslib.com

(c) 2014, Pericles Lopes Machado <pericles.raskolnikoff@gmail.com
*/

#ifndef _WGSLIBSERVER_H_
#define _WGSLIBSERVER_H_

#include <common.h>

#include <jsonrpccpp/server/connectors/httpserver.h>
#include <jsonrpccpp/server/rpcprotocolserver.h>
#include <iostream>
#include <atomic>

#include "abstractwgslibstubserver.h"

#ifdef __linux
    #include <unistd.h>
#elif _WIN32 || WIN32
    #include <Windows.h>
#else
    #include <unistd.h>
#endif

WGSLIB_DECL void task_sleep(int time);

class WGSLIB_DECL WGSLibHttpServer : public jsonrpc::HttpServer
{
    public: 
        WGSLibHttpServer(int port, bool enableSpecification = true, const std::string& sslcert = "", int threads = 50);
        bool virtual SendResponse(const std::string& response,
                void* addInfo = NULL);
};

class WGSLIB_DECL WGSLibStubServer : public AbstractWGSLibStubServer
{
    public:
        WGSLibStubServer(int port);

        virtual Json::Value compute_variograms(const int& X_prop, const int& Y_prop, const int& Z_prop, const Json::Value& dimensions, const Json::Value& directions, const std::string& grid_name, const int& num_lags, const Json::Value& props, const Json::Value& props_name, const Json::Value& props_selected);
        virtual void notifyServer();

        bool isRunning();
    private:
        std::atomic<bool> is_running_;
        jsonrpc::HttpServer http_server_;
};

#endif // _WGSLIBSERVER_H_

