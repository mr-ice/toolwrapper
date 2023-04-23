#include <iostream>
// #include <string>
#include <vector>
// #include <filesystem>
#include <unistd.h>
// #include <sstream>
// #include <iterator>
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

int main(int argc, char* argv[]) {
    // Get the name of the program being called, without any path elements
    std::string program_name = fs::path(argv[0]).filename().string();
    std::cerr << "Using program_name=" << program_name.c_str() << std::endl;

    // Get the path of the current program, and replace "bin" with "config"
    fs::path program_path = fs::absolute(argv[0]).parent_path();
    std::cerr << "Before searching for '.', program_path = " << program_path << std::endl;

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
    std::string path_str(wrapper_path);
    size_t start = 0;
    size_t end = path_str.find(':');
    while (end != std::string::npos) {
        fs::path dir_path(path_str.substr(start, end - start));
        if (dir_path.is_absolute() && fs::exists(dir_path) && fs::is_directory(dir_path)) {
            // std::cerr << dir_path.string() << " is acceptable" << std::endl;
            paths.push_back(dir_path);
        } else {
            // std::cerr << dir_path.string() << " is not absolute, a directory, or existing... skipping" << std::endl;
        }
        start = end + 1;
        end = path_str.find(':', start);
    }
    fs::path dir_path(path_str.substr(start));
    if (dir_path.is_absolute() && fs::exists(dir_path) && fs::is_directory(dir_path)) {
        // std::cerr << dir_path.string() << " is acceptable" << std::endl;
        paths.push_back(dir_path);
    } else {
        // std::cerr << dir_path.string() << " is not absolute, a directory, or existing... skipping" << std::endl;
    }
    // append the previously determined program_path
    paths.push_back(program_path);

    // Print out the list of paths
    std::cout << "Search Directories" << std::endl;
    for (const auto& path : paths) {
        std::cout << "  " << fs::path(path).string() << std::endl;
    }

    // Look for the program_name in the directories
    for (const auto& dir : paths) {
        std::cout << "checking " << dir << " for " <<  program_name << std::endl;
        fs::path path = dir / program_name;
        if (fs::exists(path) && fs::is_regular_file(path)) {
            // Found the program config, read the PATH value from the file
            std::ifstream file(path.string());
            std::string line;
            while (std::getline(file, line)) {
                if (line.find("PATH=") == 0) {
                    // Modify the PATH environment variable and execute the program
                    std::string new_path = line.substr(5);
                    std::string old_path = std::getenv("PATH");
                    std::string merged_path = new_path + ":" + old_path;
                    setenv("PATH", merged_path.c_str(), 1);
                    std::cout << "path is " << path << std::endl;
                    execv(path.c_str(), argv);
                    perror("execv");
                    return 1;
                }
            }
            std::cerr << "Could not find PATH= line in " << path << std::endl;
            return 1;
        }
    }

    // std::cerr << "Could not find executable " << program_name << " in WRAPPER_PATH and program path." << std::endl;
    // return 1;
}
