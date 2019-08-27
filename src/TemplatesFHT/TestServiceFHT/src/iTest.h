/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTITEST_H
#define FHTITEST_H
#include <memory>
namespace FHT {
	struct iTest {
		static std::shared_ptr<iTest> Run;
		virtual ~iTest() = default;
	};
}
#endif //FHTITEST_H
