/*
WGSLIB - Web GeoStatistics Library
URL: http://wgslib.com

(c) 2014, Pericles Lopes Machado <pericles.raskolnikoff@gmail.com
*/

#ifndef _WGSLIBSERVER_H_
#define _WGSLIBSERVER_H_

#include <jsonrpc/connectors/httpserver.h>
#include <jsonrpc/rpc.h>
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

void task_sleep(int time);

class WGSLibHttpServer : public jsonrpc::HttpServer
{
    public: 
        WGSLibHttpServer(int port, bool enableSpecification = true, const std::string& sslcert = "", int threads = 50);

        bool virtual SendResponse(const std::string& response,
                    void* addInfo = NULL);
};

class WGSLibStubServer : public AbstractWGSLibStubServer
{
    public:
        WGSLibStubServer();

        virtual Json::Value compute_variograms(const int& X_prop, const int& Y_prop, const int& Z_prop, const Json::Value& dimensions, const Json::Value& directions, const std::string& grid_name, const int& num_lags, const Json::Value& props, const Json::Value& props_name, const Json::Value& props_selected);
        virtual void notifyServer();

        bool isRunning();
    private:
        std::atomic<bool> is_running_;
};

#endif // _WGSLIBSERVER_H_

