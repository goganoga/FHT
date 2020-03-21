/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 29.11.19
*  Copyright (C) goganoga 2019
***************************************/
#include <string>
#include <map>

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