#include <iostream>
#include <fstream>
#include "load_config.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << argv[1] << std::endl;
        return 1;
    }

    auto keyValueMap = load_config(file);
    file.close(); // Don't forget to close the file stream when done!

    // Print the key-value pairs
    for (const auto& pair : keyValueMap) {
        std::cout << pair.first << " = ";
        std::visit([](const auto& value) {
            std::cout << value;
        }, pair.second);
        std::cout << std::endl;
    }

    return 0;
}
