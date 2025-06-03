#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

#if defined(_WIN32)
    #define PLATFORM_WINDOWS 1
#else
    #define PLATFORM_WINDOWS 0
#endif

const std::string HOME = std::getenv(PLATFORM_WINDOWS ? "USERPROFILE" : "HOME");
const std::string GUV_HOME = HOME + "/.guv";
const std::string ENVS_PATH = GUV_HOME + "/envs";

// ANSI colors
const std::string GREEN = "\033[1;32m";
const std::string RED   = "\033[1;31m";
const std::string CYAN  = "\033[1;36m";
const std::string RESET = "\033[0m";

// Check if 'uv' is available
bool uvExists() {
#if PLATFORM_WINDOWS
    return std::system("where uv > nul 2>&1") == 0;
#else
    return std::system("which uv > /dev/null 2>&1") == 0;
#endif
}

// Prompt user to install UV
void promptUVInstall() {
    std::cerr << RED << "'uv' is not installed or not in PATH.\n" << RESET;
    std::cerr << CYAN << "Install it with:\n";
    if (PLATFORM_WINDOWS)
        std::cerr << "  python -m pip install uv\n";
    else
        std::cerr << "  curl -Ls https://astral.sh/uv/install.sh | sh\n";
    std::cerr << RESET;
}

// Help
void printHelp() {
    std::cout << CYAN << "GUV - Global UV Environment Manager\n\n" << RESET;
    std::cout << "Usage:\n";
    std::cout << "  guv venv <envname>         Create new environment\n";
    std::cout << "  guv clone <src> <dst>      Clone environment\n";
    std::cout << "  guv reset <envname>        Delete specific environment\n";
    std::cout << "  guv reset --all            Delete all environments\n";
    std::cout << "  guv activate <envname>     Show activation command\n";
    std::cout << "  guv list                   List all environments\n";
    std::cout << "  guv config                 Config (not implemented yet)\n";
    std::cout << "  guv help                   Show this help message\n";
}

// List envs
void listEnvs() {
    if (!fs::exists(ENVS_PATH)) {
        std::cout << RED << "No environments directory at " << ENVS_PATH << RESET << "\n";
        return;
    }

    bool found = false;
    for (const auto& entry : fs::directory_iterator(ENVS_PATH)) {
        if (fs::is_directory(entry)) {
            std::string envName = entry.path().filename().string();
            std::cout << "  - " << envName << "\n";

            std::string scriptPath = entry.path().string() +
                                     (PLATFORM_WINDOWS ? "/Scripts/Activate.ps1" : "/bin/activate");

            if (PLATFORM_WINDOWS) {
                std::cout << "    PowerShell: Invoke-Expression -Command \"& '" << scriptPath << "'\"\n";
            } else {
                std::cout << "    Bash: source \"" << scriptPath << "\"\n";
            }

            found = true;
        }
    }

    if (!found)
        std::cout << RED << "No environments found.\n" << RESET;
}

// Make env
void makeVenv(const std::string& name) {
    if (!uvExists()) {
        promptUVInstall();
        return;
    }

    std::string envPath = ENVS_PATH + "/" + name;
    if (fs::exists(envPath)) {
        std::cerr << RED << "Environment already exists: " << name << RESET << "\n";
        return;
    }

    fs::create_directories(ENVS_PATH);
    std::string cmd = "uv venv \"" + envPath + "\"";
    std::cout << GREEN << "Creating venv at " << envPath << RESET << "\n";
    if (std::system(cmd.c_str()) != 0)
        std::cerr << RED << "Failed to create virtual environment." << RESET << "\n";
}

// Delete single or all
void resetEnv(const std::string& arg) {
    if (arg == "--all") {
        if (!fs::exists(ENVS_PATH)) {
            std::cout << RED << "Nothing to reset. No environments found.\n" << RESET;
            return;
        }

        for (const auto& entry : fs::directory_iterator(ENVS_PATH)) {
            if (fs::is_directory(entry)) {
                fs::remove_all(entry.path());
                std::cout << GREEN << "Deleted: " << entry.path().filename().string() << RESET << "\n";
            }
        }
    } else {
        std::string envPath = ENVS_PATH + "/" + arg;
        if (!fs::exists(envPath)) {
            std::cerr << RED << "Environment not found: " << arg << RESET << "\n";
            return;
        }
        fs::remove_all(envPath);
        std::cout << GREEN << "Deleted environment: " << arg << RESET << "\n";
    }
}

void activateEnv(const std::string& name) {
    std::string script = ENVS_PATH + "/" + name +
                         (PLATFORM_WINDOWS ? "/Scripts/Activate.ps1" : "/bin/activate");

    if (!fs::exists(script)) {
        std::cerr << RED << "No activate script found: " << script << RESET << "\n";
        return;
    }

    std::cout << CYAN << "To activate the environment '" << name << "' in PowerShell:\n" << RESET;
    if (PLATFORM_WINDOWS) {
        std::cout << "  Invoke-Expression -Command \"& '" << script << "'\"\n";
    } else {
        std::cout << "  source \"" << script << "\"\n";
    }
}



// Clone env via freeze/install
void cloneEnv(const std::string& src, const std::string& dest) {
    if (!uvExists()) {
        promptUVInstall();
        return;
    }

    std::string srcPath = ENVS_PATH + "/" + src;
    std::string dstPath = ENVS_PATH + "/" + dest;
    std::string tempFile = GUV_HOME + "/temp.txt";

    if (!fs::exists(srcPath)) {
        std::cerr << RED << "Source env doesn't exist: " << src << RESET << "\n";
        return;
    }
    if (fs::exists(dstPath)) {
        std::cerr << RED << "Destination already exists: " << dest << RESET << "\n";
        return;
    }

    std::string actScript = srcPath + (PLATFORM_WINDOWS ? "/Scripts/activate" : "/bin/activate");
    std::string freezeCmd = (PLATFORM_WINDOWS ? "cmd /c \"" : "bash -c 'source ") + actScript +
                             (PLATFORM_WINDOWS ? " && " : " && ") + "uv pip freeze > \"" + tempFile + "\"" +
                             (PLATFORM_WINDOWS ? "\"" : "'");

    std::string venvCmd = "uv venv \"" + dstPath + "\"";
    std::string instScript = dstPath + (PLATFORM_WINDOWS ? "/Scripts/activate" : "/bin/activate");
    std::string installCmd = (PLATFORM_WINDOWS ? "cmd /c \"" : "bash -c 'source ") + instScript +
                             (PLATFORM_WINDOWS ? " && " : " && ") + "uv pip install -r \"" + tempFile + "\"" +
                             (PLATFORM_WINDOWS ? "\"" : "'");

    std::cout << CYAN << "Freezing packages from '" << src << "'..." << RESET << "\n";
    std::system(freezeCmd.c_str());

    std::cout << CYAN << "Creating '" << dest << "'..." << RESET << "\n";
    std::system(venvCmd.c_str());

    std::cout << CYAN << "Installing into '" << dest << "'..." << RESET << "\n";
    std::system(installCmd.c_str());

    fs::remove(tempFile);
    std::cout << GREEN << "Successfully cloned " << src << " → " << dest << RESET << "\n";
}

// CLI Dispatcher
void run(int argc, char** argv) {
    if (argc < 2) {
        printHelp();
        return;
    }

    std::string cmd = argv[1];

    if (cmd == "venv" && argc >= 3) {
        makeVenv(argv[2]);
    } else if (cmd == "clone" && argc >= 4) {
        cloneEnv(argv[2], argv[3]);
    } else if (cmd == "reset" && argc >= 3) {
        resetEnv(argv[2]);
    } else if (cmd == "activate" && argc >= 3) {
        activateEnv(argv[2]);
    } else if (cmd == "list") {
        listEnvs();
    } else if (cmd == "config") {
        std::cout << CYAN << "Config system not implemented.\n" << RESET;
    } else if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        printHelp();
    } else if (cmd == "version") {
        std::cout << "guv version 0.2.1\n";
    } else {
        std::cerr << RED << "Unknown or invalid command.\n" << RESET;
        printHelp();
    }
}

int main(int argc, char** argv) {
    run(argc, argv);
    return 0;
}

