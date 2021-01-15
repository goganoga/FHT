/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include <iostream>

#include "FHT/CoreFHT.h"
#include "FHT/Config/Config.h"

int main(int argc, char* argv[]) {
    std::string conf_path("configSQL.json");
    if (argc > 1) {
        if (argv[1] == "-c" || argv[1] == "--config" || argv[2]) {
            conf_path = std::string(argv[2]);
        } else {
            std::cout << "-c | --config <path_to_config>               path to file config.json" << std::endl;
            return 1;
        }
    }
    
    CoreFHT::InitCore(FHT::GetFHTConfig(conf_path));

    //emulation work in programm
    auto result = CoreFHT::QueryDB->Query("select version()");
    std::cout << "Connection version PostgreSQL: " << result["version"][0] << std::endl;

    return 0;
}