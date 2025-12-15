#include "CEFUtils.hpp"
#include "CEFApp.hpp"
#include <cef_app.h>
#include <cef_task.h>
#include <wrapper/cef_helpers.h>
#include <memory>
#include <cstdlib>

#ifdef __WXMSW__
#include <windows.h>
#include <shlobj.h>
#elif defined(__WXOSX__)
#include <mach-o/dyld.h>
#include <libgen.h>
#include <unistd.h>
#include <wrapper/cef_library_loader.h>
#include "CEFMacApplication.hpp"
#else
#include <unistd.h>
#include <libgen.h>
#endif

namespace Slic3r { namespace GUI { namespace CEFUtils {

namespace {
    CefRefPtr<CEFApp> g_app;
    bool g_initialized = false;
#ifdef __WXOSX__
    std::unique_ptr<CefScopedLibraryLoader> g_library_loader;
#endif
}

bool Initialize(int argc, char** argv) {
    if (g_initialized) {
        return true;
    }

#ifdef __WXOSX__
    if (!CEFMacApplication::SwizzleNSApplication()) {
        return false;
    }

    g_library_loader = std::make_unique<CefScopedLibraryLoader>();
    if (!g_library_loader->LoadInMain()) {
        g_library_loader.reset();
        return false;
    }
#endif

#ifdef __WXMSW__
    CefMainArgs main_args(GetModuleHandle(nullptr));
#else
    CefMainArgs main_args(argc, argv);
#endif

    g_app = new CEFApp();

    int exit_code = CefExecuteProcess(main_args, g_app.get(), nullptr);
    if (exit_code >= 0) {
        std::exit(exit_code);
    }

    CefSettings settings;
    settings.no_sandbox = true;
    settings.external_message_pump = true;
    settings.multi_threaded_message_loop = false;


#ifdef __WXMSW__
    settings.log_severity = LOGSEVERITY_WARNING;
#else
    settings.log_severity = LOGSEVERITY_ERROR;
#endif

    std::string resources_path = GetResourcesPath();

#ifdef __WXOSX__
    char exe_path[1024];
    uint32_t exe_size = sizeof(exe_path);
    std::string frameworks_path;
    std::string macos_dir_str;

    if (_NSGetExecutablePath(exe_path, &exe_size) == 0) {
        char exe_path_copy[1024];
        strncpy(exe_path_copy, exe_path, sizeof(exe_path_copy) - 1);
        exe_path_copy[sizeof(exe_path_copy) - 1] = '\0';
        char* macos_dir = dirname(exe_path_copy);
        macos_dir_str = std::string(macos_dir);
        frameworks_path = macos_dir_str + "/../Frameworks";

        char contents_path[1024];
        snprintf(contents_path, sizeof(contents_path), "%s/..", macos_dir);
        char* contents_dir = realpath(contents_path, nullptr);
        if (contents_dir) {
            char app_bundle_path[1024];
            snprintf(app_bundle_path, sizeof(app_bundle_path), "%s/..", contents_dir);
            char* app_dir = realpath(app_bundle_path, nullptr);
            if (app_dir) {
                CefString(&settings.main_bundle_path) = std::string(app_dir);
                free(app_dir);
            }
            free(contents_dir);
        }

        std::string helper_path = macos_dir_str + "/../Frameworks/OrcaSlicer Helper.app/Contents/MacOS/OrcaSlicer Helper";
        char* resolved_helper = realpath(helper_path.c_str(), nullptr);
        if (resolved_helper) {
            CefString(&settings.browser_subprocess_path) = std::string(resolved_helper);
            free(resolved_helper);
        } else {
            CefString(&settings.browser_subprocess_path) = std::string(exe_path);
        }
    } else {
        frameworks_path = resources_path + "/../Frameworks";
    }
    std::string framework_path = frameworks_path + "/Chromium Embedded Framework.framework";
    CefString(&settings.framework_dir_path) = framework_path;
#endif

#ifdef __WXOSX__
    CefString(&settings.resources_dir_path) = framework_path + "/Resources";
    CefString(&settings.locales_dir_path) = framework_path + "/Resources";
#else
    CefString(&settings.resources_dir_path) = resources_path;
    CefString(&settings.locales_dir_path) = resources_path + "/locales";
#endif

    bool result = CefInitialize(main_args, settings, g_app.get(), nullptr);
    if (result) {
        g_initialized = true;
    }

    return result;
}

void Shutdown() {
    if (g_initialized) {
        CefShutdown();
        g_app = nullptr;
        g_initialized = false;
#ifdef __WXOSX__
        g_library_loader.reset();
#endif
    }
}

void DoMessageLoopWork() {
    if (g_initialized) {
        CefDoMessageLoopWork();
    }
}

std::string GetResourcesPath() {
#ifdef __WXMSW__
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string exe_path(path);
    size_t last_slash = exe_path.find_last_of("\\/");
    return exe_path.substr(0, last_slash) + "\\resources";
#elif defined(__WXOSX__)
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        char* dir = dirname(path);
        return std::string(dir) + "/../Resources";
    }
    return "./resources";
#else
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        char* dir = dirname(path);
        return std::string(dir) + "/resources";
    }
    return "./resources";
#endif
}

}}} // namespace Slic3r::GUI::CEFUtils
