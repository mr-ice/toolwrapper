#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <sstream>
#include <iterator>
#include <fstream>

namespace fs = std::__fs::filesystem;

static bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

static bool startsWith(const std::string& str, const std::string& prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

std::string readWrapperConfig(fs::path path) {
    // fs::path path(path_arg);
    std::cout << "Found path = " << path.string() << std::endl;
    std::ifstream file(path.string());
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("PATH=") == 0) {
            size_t start_pos = line.find_first_of("=") + 1;
            size_t end_pos = line.find_first_of("#", start_pos);
            // Remove any leading spaces before the comment
            while (std::isspace(line[end_pos-1])) {
                --end_pos;
            }
            std::string found_path = line.substr(start_pos, end_pos-start_pos);
            return found_path;
        }
    }
    return "";
}

int main(int argc, char* argv[]) {
    // Get the name of the program being called, without any path elements
    std::string program_name = fs::path(argv[0]).filename().string();

    // Get the path of the current program, and replace "bin" with "config"
    fs::path program_path = fs::absolute(argv[0]).parent_path();

    // strip any trailing '.'
    if (endsWith(program_path, ".")) {
        program_path = program_path.parent_path();
    }

    // replace the last part of the path with 'config'
    program_path.replace_filename("config");
    std::cerr << "Using program_path = " << program_path << std::endl;

    // Add program_path to WRAPPER_PATH
    const char* wrapper_path = std::getenv("WRAPPER_PATH");
    if (!wrapper_path) {
        std::cerr << "WRAPPER_PATH environment variable is not set" << std::endl;
        wrapper_path = "";
    }

    // Split the WRAPPER_PATH on ':'
    // add the elements to paths if they are absolute directories that exist
    std::vector<fs::path> paths;
    std::string wrapper_str(wrapper_path);
    size_t start = 0;
    size_t end = wrapper_str.find(':');
    while (end != std::string::npos) {
        fs::path dir_path(wrapper_str.substr(start, end - start));
        if (dir_path.is_absolute() && fs::exists(dir_path) && fs::is_directory(dir_path)) {
            // std::cerr << dir_path.string() << " is acceptable" << std::endl;
            paths.push_back(dir_path);
        } else {
            // std::cerr << dir_path.string() << " is not absolute, a directory, or existing... skipping" << std::endl;
        }
        start = end + 1;
        end = wrapper_str.find(':', start);
    }
    fs::path dir_path(wrapper_str.substr(start));
    if (dir_path.is_absolute() && fs::exists(dir_path) && fs::is_directory(dir_path)) {
        // std::cerr << dir_path.string() << " is acceptable" << std::endl;
        paths.push_back(dir_path);
    } else {
        // std::cerr << dir_path.string() << " is not absolute, a directory, or existing... skipping" << std::endl;
    }
    // append the previously determined program_path
    paths.push_back(program_path);

    // Print out the list of paths
    // std::cout << "Search Directories" << std::endl;
    // for (const auto& path : paths) {
    //     std::cout << "  " << fs::path(path).string() << std::endl;
    // }

    // Look through the directories, if you find program_name, read it for
    // PATH=
    for (const auto& dir : paths) {
        std::cout << "checking " << dir << " for " <<  program_name << std::endl;
        fs::path path = dir / program_name;
        if (fs::exists(path) && fs::is_regular_file(path)) {
            std::string new_dir = readWrapperConfig(path.string());
            fs::path new_path(new_dir);
            new_path = new_path / program_name;
            // Found the program config, read the PATH value from the file
            std::string old_path = std::getenv("PATH");
            if (old_path.empty()) {
                std::cerr << "PATH environment variable not found, aborting" << std::endl;
                return 1;
            }
            std::string merged_path = new_path.string() + ":" + old_path;
            setenv("PATH", merged_path.c_str(), 1);
            std::cout << "path is " << path << std::endl;
            execv(new_path.c_str(), argv);
            perror("execv");
            return 1;
        }
    }
    std::cerr << "Could not find executable " << program_name << " in WRAPPER_PATH and program path." << std::endl;
    return 1;
}
