#ifndef LOAD_CONFIG_H
#define LOAD_CONFIG_H

#include <fstream>
#include <string>
#include <map>
#include <variant>

std::map<std::string, std::variant<int, float, bool, std::string>> load_config(std::ifstream& file);

#endif // LOAD_CONFIG_H
