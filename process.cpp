#include "infra/process.h"
#include "infra/exception.h"

#ifdef WIN32
#include <Windows.h>
#endif

namespace infra {

int runProcess(std::string_view cmdLine)
{
#ifdef WIN32
    STARTUPINFO si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::string cmd(cmdLine);

    // Start the child process.
    if (!CreateProcess(nullptr,          // No module name (use command line)
                       &cmd[0],          // Command line
                       nullptr,          // Process handle not inheritable
                       nullptr,          // Thread handle not inheritable
                       FALSE,            // Set handle inheritance to FALSE
                       CREATE_NO_WINDOW, // Hide the window
                       nullptr,          // Use parent's environment block
                       nullptr,          // Use parent's starting directory
                       &si,              // Pointer to STARTUPINFO structure
                       &pi)              // Pointer to PROCESS_INFORMATION structure
    ) {
        throw RuntimeError("Failed to create process ({}).", GetLastError());
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    if (GetExitCodeProcess(pi.hProcess, &exitCode) != FALSE) {
        if (STILL_ACTIVE == exitCode) {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            throw RuntimeError("Process still running");
        }
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return int(exitCode);
#else
    (void)cmdLine;
    throw RuntimeError("runProcess not implemented");
#endif
}
}
