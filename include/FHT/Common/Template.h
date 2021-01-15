/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 29.11.19
*  Copyright (C) goganoga 2019
***************************************/
#include <string>
#include <map>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

template <typename T, typename ...A>
auto map_find(std::map<std::string, std::string> const& map, T const arg0, A const ...args) {
    std::vector<T> v{ arg0, args... };
    for (T &i : v) {
        std::string t{ i };
        auto a = map.find(t);
        if (a != map.end()) {
            return a;
        };
    }
    return map.end();
}
namespace FHT {
    static std::string guid(std::string ab) { //uuid_v5
        boost::uuids::nil_generator gen_nil;
        boost::uuids::name_generator gen(gen_nil());
        boost::uuids::uuid u = gen(ab.c_str());
        std::string tmp = boost::uuids::to_string(u);
        return tmp;
    }
}