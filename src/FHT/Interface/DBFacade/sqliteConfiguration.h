/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 25.10.20
*  Copyright (C) goganoga 2020
***************************************/
#ifndef FHTIBDFACADESQLITECONFIGURATION_H
#define FHTIBDFACADESQLITECONFIGURATION_H
#include <string>
namespace FHT {
    namespace iDBFacade {
        struct Configuration {
            std::string m_name;
            std::string m_pass;
        };
    }
}
#endif //FHTIBDFACADESQLITECONFIGURATION_H
