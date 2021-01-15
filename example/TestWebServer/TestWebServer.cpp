/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifdef __linux__ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#endif

#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <thread>

#include "FHT/CoreFHT.h"
#include "FHT/Config/Config.h"
#include "iTest.h"

int main(int argc, char* argv[]) {
    std::string conf_path("config.json");
    if (argc > 1) {
        if (argv[1] == "-c" || argv[1] == "--config" || argv[2]) {
            conf_path = std::string(argv[2]);
        } else {
            std::cout << "-c | --config <path_to_config>               path to file config.json" << std::endl;
            return 1;
        }
    }

    CoreFHT::InitCore(FHT::GetFHTConfig(conf_path));
    
    FHT::iTest* Test = FHT::iTest::Run.get(); // init global class

    std::cout << "For exit press Ctrl + C" << std::endl;
    for(;;) std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}