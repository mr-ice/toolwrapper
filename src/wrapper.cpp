#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <sstream>
#include <iterator>
#include <fstream>
#include <sys/wait.h>
#include "load_config.h"

#ifdef TARGET_OS_X
namespace fs = std::__fs::filesystem;
#else
namespace fs = std::filesystem;
#endif // ifdef TARGET_OS_X

#ifndef CONFIGDIRNAME
#define CONFIGDIRNAME "config"
#endif // ifndev CONFIGDIRNAME

#ifndef TOOLPATHENV
#define TOOLPATHENV "WRAPPER_PATH"
#endif // ifdndef TOOLPATHENV

#ifndef PATHENV
#define PATHENV "PATH"
#endif // ifndef PATHENV

#ifndef EQUALS
#define EQUALS "="
#endif // ifndef EQUALS

#ifndef PATHEQ
#define PATHEQ PATHENV EQUALS
#endif // ifndef PATHEQ

// checks (returns true) if suffix is at the end of str
static bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

void writeLogToFile(const std::string& filename, const std::string& message) {
    std::ofstream logfile(filename, std::ios::app);
    if (logfile.is_open()) {
        logfile << message << std::endl;
        logfile.close();
    } else {
        std::cerr << "Error: Unable to open log file " << filename << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Get the name of the program being called, without any path elements
    if (argc < 1) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string program_name = fs::path(argv[0]).filename().string();

    // Get the path of the current program, and replace "bin" with "config"
    fs::path program_path = fs::absolute(argv[0]).parent_path();

    // strip any trailing '.'
    if (endsWith(program_path, ".")) {
        program_path = program_path.parent_path();
    }

    // replace the last part of the path with 'config'
    program_path.replace_filename(CONFIGDIRNAME);

    // Add program_path to WRAPPER_PATH
    const char* wrapper_path = std::getenv(TOOLPATHENV);
    if (!wrapper_path) {
        wrapper_path = "";
    }

    // Split the WRAPPER_PATH on ":"
    // Add the elements to paths if they are absolute directories that exist
    std::vector<fs::path> paths;
    std::string wrapper_str(wrapper_path);
    size_t start = 0;
    size_t end = wrapper_str.find(":");
    while (end != std::string::npos) {
        fs::path dir_path(wrapper_str.substr(start, end - start));
        if (dir_path.is_absolute() && fs::exists(dir_path) && fs::is_directory(dir_path)) {
            paths.push_back(dir_path);
        }
        start = end + 1;
        end = wrapper_str.find(":", start);
    }
    fs::path dir_path(wrapper_str.substr(start));
    if (dir_path.is_absolute() && fs::exists(dir_path) && fs::is_directory(dir_path)) {
        paths.push_back(dir_path);
    }

    // append the previously determined program_path
    paths.push_back(program_path);

    // Look through the directories, if you find program_name, read it for PATH=
    for (const auto& dir : paths) {
        fs::path path = dir / program_name;
        if (fs::exists(path) && fs::is_regular_file(path)) {
            std::ifstream configFile(path.string());
            auto configMap = load_config(configFile);

            std::string new_dir;
            if (configMap.find("PATH") != configMap.end()) {
                new_dir = std::get<std::string>(configMap["PATH"]);
            }

            fs::path new_path(new_dir);
            fs::path new_program_name = new_path / program_name;

            // Fork the program
            pid_t pid = fork();
            if (pid == -1) {
                std::cerr << "Error: Fork failed" << std::endl;
                return 1;
            } else if (pid == 0) { // Child process
                // Exec the program
                execv(new_program_name.c_str(), argv);
                perror("execv");  // This should not be reached
                return 1;
            } else { // Parent process
                // Check if LOG key exists and its value starts with "file:"
                if (configMap.find("LOG") != configMap.end()) {
                    std::string logValue = std::get<std::string>(configMap["LOG"]);
                    if (logValue.find("file:") == 0) {
                        std::string filename = logValue.substr(logValue.find(":") + 1);
                        std::string message = fs::absolute(argv[0]).string() +
                        " executed " +
                        new_program_name.string() +
                        " through " +
                        path.string() +
                        " using WRAPPER_PATH=" +
                        std::getenv(TOOLPATHENV);
                        writeLogToFile(filename, message);
                    }
                }
                return 0;
            }
        }
    }
    std::cerr << "Could not find executable " << program_name << " in " <<
        TOOLPATHENV << " and program path." << std::endl;
    return 1;
}
