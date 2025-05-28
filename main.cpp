#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <limits>

namespace fs = std::filesystem;

struct Opt {
    std::string target_dir_str;
    fs::path target_dir_path;
    bool dry_run = false;
    bool verbose = false;
    bool interactive = false;
    int min_depth = 0; // all (root is 0)
    int max_depth = std::numeric_limits<int>::max();
};



void help(const char* prog_name) {
    std::cout << "Empty Folder Nuker\n"
              << "Recursively finds and deletes empty dirs\n\n"
              << "Usage: " << prog_name << " <dir> [options]\n\n"
              << "Args:\n"
              << "  <dir>         The starting dir to scan.\n\n"
              << "Options:\n"
              << "  --help              Show this help msg and exits\n"
              << "  --dry-run           Show what would be deleted without actually deleting yet\n"
              << "  --verbose, -v       Print more info about actions taken\n"
              << "  --interactive, -i   Ask for confirmation before deleting each one (kinda bad for larger amounts)\n"
              << "  --min-depth <N>     Only consider folders at or deeper than N (root is 0)\n"
              << "  --max-depth <N>     Only consider folders at or \"superficial\" than N\n";

    
}


bool nukeEmptyRecursive(const fs::path& dir_path, const Opt& opts, const int current_depth) {
    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
        if (opts.verbose) {
            std::cerr << "Warn: Path is not a dir or doesnt exist: " << dir_path << std::endl;
        }
        return false;
    }

    bool currentDirEempty = true;
    std::vector<fs::path> subDirLater;

    try {
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (fs::is_directory(entry.path())) {
                if (current_depth + 1 <= opts.max_depth) {
                    if (!nukeEmptyRecursive(entry.path(), opts, current_depth + 1)) {
                        // if subdir was not nuked, then this current dir is kinda not empty
                        currentDirEempty = false;
                    }
                }
                else {
                    currentDirEempty = false;
                }
            }
            else { // file most likely
                currentDirEempty = false;
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Err accessing dir " << dir_path << ": " << e.what() << std::endl;
        return false;
    }


    // after processing children in hirarchy, check if this dir is now empty + depth matches
    if (currentDirEempty && current_depth >= opts.min_depth) {
        bool confirmd = true;
        if (opts.interactive && !opts.dry_run) {
            std::cout << "Delete '" << dir_path.string() << "'? [y/N]: ";
            std::string response_str;
            std::getline(std::cin, response_str);
            char response = 'n';
            if (!response_str.empty()) {
                response = static_cast<char>(tolower(response_str[0]));
            }
            if (response != 'y') {
                confirmd = false;
                if (opts.verbose) {
                    std::cout << "Skipped (interactive): " << dir_path.string() << std::endl;
                }
            }
        }

        if (confirmd) {
            if (opts.dry_run) {
                std::cout << "[DRY RUN] Would delete empty dir: " << dir_path.string() << std::endl;
            }
            else {
                try {
                    if (fs::remove(dir_path)) {
                        if (opts.verbose) {
                            std::cout << "Deleted: " << dir_path.string() << std::endl;
                        } else {
                             std::cout << dir_path.string() << std::endl;
                        }
                    } else {
                        if (opts.verbose) {
                             std::cerr << "Warn: Failed to delete (or already gone?): " << dir_path.string() << std::endl;
                        }
                        return false;
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Err deleting dir " << dir_path.string() << ": " << e.what() << std::endl;
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    return false;
}


int main(const int argc, char* argv[]) {
    Opt o;
    const std::vector<std::string> args(argv + 1, argv + argc);

    if (args.empty()) {
        help(argv[0]);
        return 1;
    }
    for (size_t i = 0; i < args.size(); ++i) {
        if (const std::string& arg = args[i]; arg == "--help") {
            help(argv[0]);
            return 0;
        }
        else if (arg == "--dry-run") {
            o.dry_run = true;
        }
        else if (arg == "--verbose" || arg == "-v") {
            o.verbose = true;
        }
        else if (arg == "--interactive" || arg == "-i") {
            o.interactive = true;
        }
        else if (arg == "--min-depth") {
            if (i + 1 < args.size()) {
                try {
                    o.min_depth = std::stoi(args[++i]);
                    if (o.min_depth < 0) {
                         std::cerr << "Err: --min-depth cant be < 0" << std::endl;
                         return 1;
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Err: Invalid value for --min-depth: " << args[i] << "\t" << e.what() << std::endl;
                    return 1;
                }
            }
            else {
                std::cerr << "Err: --min-depth requires an arg" << std::endl;
                return 1;
            }
        } else if (arg == "--max-depth") {
            if (i + 1 < args.size()) {
                try {
                    o.max_depth = std::stoi(args[++i]);
                     if (o.max_depth < 0) {
                         std::cerr << "Err: --max-depth cant be < 0." << std::endl;
                         return 1;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Err: Invalid value for --max-depth: " << args[i] << "\t" << e.what() << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Err: --max-depth requires an arg" << std::endl;
                return 1;
            }
        } else if (arg.rfind("--", 0) == 0) {
            std::cerr << "Err: Idk option: " << arg << std::endl;
            help(argv[0]);
            return 1;
        } else {
            if (o.target_dir_str.empty()) {
                o.target_dir_str = arg;
            } else {
                std::cerr << "Err: Multiple target dirs inputed, only 1 is currently allowed (Ill implement in future if I need it)" << std::endl;
                help(argv[0]);
                return 1;
            }
        }
    }

    if (o.target_dir_str.empty()) {
        std::cerr << "Err: Target dir not specified!!!" << std::endl;
        help(argv[0]);
        return 1;
    }
    if (o.min_depth > o.max_depth) {
        std::cerr << "Err: --min-depth (" << o.min_depth<< ") cant be greater than --max-depth (" << o.max_depth << ")!!!" << std::endl;
        return 1;
    }
    
    o.target_dir_path = fs::absolute(o.target_dir_str);

    if (!fs::exists(o.target_dir_path)) {
        std::cerr << "Err: Target dir does not exist: " << o.target_dir_path.string() << std::endl;
        return 1;
    }
    if (!fs::is_directory(o.target_dir_path)) {
        std::cerr << "Err: Target path is not a dir: " << o.target_dir_path.string() << std::endl;
        return 1;
    }
    if (o.verbose) {
        std::cout << "Starting scan in: " << o.target_dir_path.string() << std::endl;
        std::cout << "Options: "<< (o.dry_run ? "DryRun " : "")/*<< (o.verbose ? "Verbose " : "")*/<< (o.interactive ? "Interactive " : "")<< "MinDepth=" << o.min_depth << " "<< "MaxDepth=" << (o.max_depth == std::numeric_limits<int>::max() ? "INF" : std::to_string(o.max_depth))<< std::endl;
    }





    
    nukeEmptyRecursive(o.target_dir_path, o, 0);
    if (o.verbose) {
        std::cout << "Scan done :) " << std::endl;
    }
    return 0;
}