#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <unistd.h>
#include <csignal>
#include <termios.h>
#include <sys/ioctl.h>
#include <chrono>
#include <thread>
#include <cerrno>
#include <limits>


#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define CLEAR_SCREEN "\033[H\033[J"

struct ProcessInfo {
    int pid;
    std::string name;
    double cpuUsage;
    double memUsage;
};


void setNonBlockingInput(bool enable) {
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

bool kbhit() {
    int bytesWaiting;
    ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
    return bytesWaiting > 0;
}


double getCPUUsage() {
    static long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
    double percent;

    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);
    std::istringstream ss(line);

    std::string cpu;
    long long user, nice, system, idle, iowait, irq, softirq, steal;
    ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    long long totalUser = user;
    long long totalUserLow = nice;
    long long totalSys = system;
    long long totalIdle = idle + iowait;

    if (lastTotalUser == 0 && lastTotalUserLow == 0 && lastTotalSys == 0 && lastTotalIdle == 0) {
        lastTotalUser = totalUser;
        lastTotalUserLow = totalUserLow;
        lastTotalSys = totalSys;
        lastTotalIdle = totalIdle;
        return 0.0;
    }

    long long total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
                      (totalSys - lastTotalSys);
    long long totalTime = total + (totalIdle - lastTotalIdle);

    percent = (totalTime == 0) ? 0 : (100.0 * total / totalTime);

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    return percent;
}

double getMemoryUsage() {
    std::ifstream file("/proc/meminfo");
    std::string label;
    long memTotal = 0, memAvailable = 0;

    while (file >> label) {
        if (label == "MemTotal:")
            file >> memTotal;
        else if (label == "MemAvailable:") {
            file >> memAvailable;
            break;
        } else
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    if (memTotal == 0)
        return 0.0;
    return 100.0 * (memTotal - memAvailable) / memTotal;
}


std::vector<ProcessInfo> getProcesses() {
    std::vector<ProcessInfo> processes;
    DIR *dir = opendir("/proc");
    if (!dir)
        return processes;

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (!isdigit(entry->d_name[0]))
            continue;

        int pid = atoi(entry->d_name);
        std::string path = std::string("/proc/") + entry->d_name + "/comm";
        std::ifstream file(path);
        if (!file)
            continue;

        std::string name;
        std::getline(file, name);

        std::string statusPath = std::string("/proc/") + entry->d_name + "/status";
        std::ifstream statusFile(statusPath);
        double memUsage = 0.0;
        std::string label;
        while (statusFile >> label) {
            if (label == "VmRSS:") {
                long memKb;
                statusFile >> memKb;
                memUsage = memKb / 1024.0;
                break;
            }
            statusFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        processes.push_back({pid, name, 0.0, memUsage});
    }
    closedir(dir);
    return processes;
}


std::string colorForUsage(double value) {
    if (value < 50) return GREEN;
    if (value < 80) return YELLOW;
    return RED;
}

int main() {
    setNonBlockingInput(true);
    auto startTime = std::chrono::steady_clock::now();
    pid_t myPID = getpid();

    while (true) {
        std::cout << CLEAR_SCREEN;

        double cpuUsage = getCPUUsage();
        double memUsage = getMemoryUsage();

        std::cout << BOLD << CYAN << "==== System Monitoring Tool ====" << RESET << "\n";
        std::cout << "CPU: " << colorForUsage(cpuUsage) << std::fixed << std::setprecision(1) << cpuUsage << "%" << RESET
                  << " | MEM: " << colorForUsage(memUsage) << memUsage << "%" << RESET << " | ";

        auto now = std::chrono::steady_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        int hrs = uptime / 3600;
        int mins = (uptime % 3600) / 60;
        int secs = uptime % 60;
        std::cout << CYAN << "Uptime: " << std::setfill('0') << std::setw(2) << hrs << ":"
          << std::setw(2) << mins << ":" << std::setw(2) << secs << RESET << "\n";
std::cout << std::setfill(' ');  

        std::cout << YELLOW << "Press q to quit, k to kill process" << RESET << "\n\n";

        auto processes = getProcesses();
        std::cout << BOLD << std::left << std::setw(10) << "PID"
                  << std::setw(25) << "Process Name"
                  << std::setw(10) << "CPU(%)"
                  << std::setw(10) << "MEM(MB)" << RESET << "\n";
        std::cout << "-----------------------------------------------\n";

        for (auto &p : processes) {
            std::string color = (p.pid == myPID) ? CYAN : GREEN;
            std::cout << color << std::left << std::setw(10) << p.pid
                      << std::setw(25) << p.name
                      << std::setw(10) << std::fixed << std::setprecision(2) << p.cpuUsage
                      << std::setw(10) << p.memUsage << RESET << "\n";
        }

    
        if (kbhit()) {
            char ch = getchar();
            if (ch == 'q') {
                setNonBlockingInput(false);
                break;
            } else if (ch == 'k') {
                setNonBlockingInput(false);
                std::cout << "\nEnter PID to kill: ";
                int pid;
                if (!(std::cin >> pid)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    std::cout << RED << "Invalid input.\n" << RESET;
                } else {
                    if (kill(pid, SIGTERM) == 0) {
                        sleep(1);
                        if (kill(pid, 0) == -1 && errno == ESRCH) {
                            std::cout << GREEN << "Process " << pid << " successfully terminated.\n" << RESET;
                        } else {
                            char confirm;
                            std::cout << YELLOW << "Process still exists. Force kill with SIGKILL? (y/n): " << RESET;
                            std::cin >> confirm;
                            if (confirm == 'y' || confirm == 'Y') {
                                if (kill(pid, SIGKILL) == 0)
                                    std::cout << RED << "Process " << pid << " force killed.\n" << RESET;
                                else
                                    std::perror("Force kill failed");
                            }
                        }
                    } else {
                        std::perror("Failed to terminate process");
                    }
                }
                std::cout << CYAN << "Press Enter to refresh..." << RESET;
                std::cin.ignore(10000, '\n');
                std::cin.get();
                setNonBlockingInput(true);
                continue;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    setNonBlockingInput(false);
    return 0;
}