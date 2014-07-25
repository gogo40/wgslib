//jsonrpcstub -s -c -o /Users/cinemast/Desktop spec.json MyStub

#include <jsonrpc/connectors/httpserver.h>
#include <jsonrpc/rpc.h>
#include <iostream>

#include "abstractwgslibstubserver.h"

#ifdef __linux

#include <unistd.h>

#elif _WIN32 || WIN32

#include <Windows.h>

#else
#include <unistd.h>

#endif

void task_sleep(int time)
{
    global_lock();
#ifdef  __linux

          usleep(time * 1000);

#elif WIN32 || _WIN32

          Sleep(time);
#else
    usleep(time * 1000);
#endif
    global_unlock();
}

using namespace jsonrpc;
using namespace std;

class WGSLibHttpServer : public HttpServer
{
    public: 
          MyHttpServer(int port, bool enableSpecification = true, const std::string& sslcert = "", int threads = 50)
		: HttpServer(port, enableSpecification, sslcert, threads) {}

	  bool virtual SendResponse(const std::string& response,
                    void* addInfo = NULL);
};

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

class WGSLibStubServer : public AbstractWGSLibStubServer
{
    public:
        WGSLibStubServer();

        virtual Json::Value compute_variograms(const int& X_prop, const int& Y_prop, const int& Z_prop, const Json::Value& directions, const double& dx, const double& dy, const double& dz, const std::string& grid_name, const int& num_lags, const Json::Value& props, const Json::Value& props_name, const Json::Value& props_selected);
        virtual void notifyServer();
};


WGSLibStubServer::WGSLibStubServer() :
    AbstractWGSLibStubServer(new MyHttpServer(8080))
{
}

void WGSLibStubServer::notifyServer()
{
    cout << "Server got notified" << endl;
}

Json::Value WGSLibStubServer::compute_variograms(const int& X_prop, const int& Y_prop, const int& Z_prop, const Json::Value& directions, const double& dx, const double& dy, const double& dz, const std::string& grid_name, const int& num_lags, const Json::Value& props, const Json::Value& props_name, const Json::Value& props_selected)
{
    

    
}

///////////////////////////////////////////////
int main()
{
    WGSLibStubServer s;
    s.StartListening();
    while(true) {
         task_sleep(1000);
    }
    s.StopListening();
    return 0;
}

