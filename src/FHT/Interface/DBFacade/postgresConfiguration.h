/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 09.06.20
*  Copyright (C) goganoga 2020
***************************************/
#ifndef FHTIBDFACADEPOSTGRESCONFIGURATION_H
#define FHTIBDFACADEPOSTGRESCONFIGURATION_H
#include <string>
namespace FHT {
    namespace iDBFacade {
        struct Configuration {
            std::string m_host;
            std::string m_name;
            std::string m_user;
            std::string m_pass;
            int m_port = 5432;
            int m_worker = 10;
        };
    }
}
#endif //FHTIBDFACADEPOSTGRESCONFIGURATION_H
