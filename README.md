# 🖥 System Monitor Tool (C++)

A lightweight, terminal-based *System Monitor* written in C++ for Linux and WSL environments.  
It displays *real-time CPU and Memory usage, **process lists, and allows users to **kill processes* directly from the interface — similar to a simplified version of htop.

---

## 🚀 Features

- 📊 *Live CPU and Memory usage*
- 🕒 *System uptime display*
- 🧾 *Process table* with:
  - PID  
  - Process name  
  - CPU (%)  
  - Memory usage (MB)
- ⚡ *Interactive controls*:
  - q → Quit the monitor  
  - k → Kill a process by PID
- 🧠 *Smart updates* (refreshes automatically every second)
- 💀 *Process termination check* (verifies if killed successfully)
- 🎨 *Colored and formatted terminal output*

---

## 🧩 Example Output
<img width="1920" height="1080" alt="Screenshot (1859)" src="https://github.com/user-attachments/assets/284a4e15-6ea2-4c83-a64a-7ea16e177d45" />

<img width="1920" height="1080" alt="Screenshot (1857)" src="https://github.com/user-attachments/assets/c70e3047-cbeb-4a58-845c-b758d97c7a19" />

---

## ⚙ Requirements

- *Operating System:* Linux or WSL (Windows Subsystem for Linux)  
- *Compiler:* g++ (GCC 9 or higher recommended)
- *C++ Standard:* C++17 or newer  
- *Libraries used:*
  - <iostream>
  - <iomanip>
  - <unistd.h>
  - <dirent.h>
  - <signal.h>
  - <fstream>
  - <sstream>
  - <sys/sysinfo.h>
  - <termios.h>
  - <vector>
  - <chrono>, <thread>

---

## 🧰 Installation & Build

### 1. Clone this repository
```bash
git clone https://github.com/<your-username>/system_monitor.git
cd system_monitor
```
### 2. Compile the code
```bash
g++ -std=c++17 system_moniter.cpp -o system_monitor
```
### 3. Run the program
```bash
./system_monitor
```

---

### 🎮 Controls
- 1. q : Quit the monitor
- 2. k : Kill a process(with a PID)
 
---

### 🔪 Killing a Process

When you press k, you’ll be prompted:
```bash
Enter PID to kill:
```
Type the PID of the process and press Enter.
The tool will attempt to terminate the process using SIGTERM and verify if it exited successfully.

---

### ⚠ Notes

Killing system-critical processes may cause your OS or WSL session to become unstable — use with caution.
The tool may require sudo privileges to terminate certain processes.
Works best in Linux terminals (may render slightly differently in Windows Terminal).

---

### 🧑‍💻 Author

Sohenna Choudhury
📧 sohennachoudhury@gmail.com
