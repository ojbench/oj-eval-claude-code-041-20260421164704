#include <iostream>
#include "map.hpp"
#include <string>

int main() {
    sjtu::map<int, std::string> m;
    m.insert(sjtu::pair<int, std::string>(1, "one"));
    m.insert(sjtu::pair<int, std::string>(2, "two"));
    m.insert(sjtu::pair<int, std::string>(3, "three"));

    for (auto it = m.begin(); it != m.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }

    auto it = m.find(2);
    if (it != m.end()) {
        std::cout << "Found 2: " << it->second << std::endl;
        m.erase(it);
    }

    std::cout << "After erase:" << std::endl;
    for (auto it = m.begin(); it != m.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }

    return 0;
}
