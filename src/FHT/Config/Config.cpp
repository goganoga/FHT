/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 17.01.21
*  Copyright (C) goganoga 2021
***************************************/
#include "FHT/CoreFHT.h"
#include "FHT/Config/Config.h"
#include "FHT/Common/Controller/Controller.h"
#include "FHT/Interface/DBFacade/iDBFacade.h"
#include "FHT/LoggerStream.h"

#ifdef __linux__ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#endif

#include <fstream>

namespace FHT {
 
#ifdef __linux__ 
    void skeleton_daemon()
    {
        pid_t pid, sid;
        pid = fork();
        if (pid < 0)
            exit(EXIT_FAILURE);
        if (pid > 0)
            exit(EXIT_SUCCESS);
        umask(0);
        sid = setsid();
        if (sid < 0)
            exit(EXIT_FAILURE);
        if ((chdir("/")) < 0)
            exit(EXIT_FAILURE);
        for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
            close(x);
        openlog("FHT", LOG_PID, LOG_DAEMON);
        syslog(LOG_NOTICE, "Framework Handler Task core daemon runing.");
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << METHOD_NAME << "Framework Handler Task core daemon runing.";

    }
#else
    void skeleton_daemon() {}
#endif
}
