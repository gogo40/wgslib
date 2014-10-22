/*
WGSLIB - Web GeoStatistics Library
URL: http://wgslib.com

(c) 2014, Pericles Lopes Machado <pericles.raskolnikoff@gmail.com
*/

#include "wgslibserver.h"

int main()
{
    WGSLibStubServer s(8383);
    s.StartListening();
    while(s.isRunning()) {
         task_sleep(1000);
    }
    s.StopListening();
    return 0;
}

