#ifndef CALL_HOME_H
#define CALL_HOME_H

#include "struct.h"

namespace CallHome
{
    RetResult start();


    // TODO: Temp public for testing
    RetResult handle_remote_control();
    RetResult handle_client_attributes();
    RetResult handle_logs();
    RetResult handle_telemetry();
    RetResult submit_ipfs();
}

#endif