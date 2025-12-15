#!/usr/bin/env python3
import re

gui_app_cpp_path = "src/slic3r/GUI/GUI_App.cpp"
gui_app_hpp_path = "src/slic3r/GUI/GUI_App.hpp"

cpp_patches = [
    {
        "name": "Add CEF include to GUI_App.cpp",
        "after_line": '#include "DeviceCore/DevManager.h"',
        "insert": """
#ifdef SLIC3R_CEF_ENABLED
#include "CEF/CEFUtils.hpp"
#endif
"""
    },
    {
        "name": "Initialize CEF in on_init_inner",
        "after_line": "    wxInitAllImageHandlers();",
        "insert": """#ifdef SLIC3R_CEF_ENABLED
    int argc = wxApp::argc;
    char** argv = new char*[argc];
    for (int i = 0; i < argc; i++) {
        wxString arg = wxApp::argv[i];
        argv[i] = strdup(arg.mb_str());
    }

    if (!CEFUtils::Initialize(argc, argv)) {
        for (int i = 0; i < argc; i++) {
            free(argv[i]);
        }
        delete[] argv;
        wxLogError("Failed to initialize CEF");
    } else {
        m_cef_timer = new wxTimer();
        m_cef_timer->Bind(wxEVT_TIMER, [](wxTimerEvent&) {
            CEFUtils::DoMessageLoopWork();
        });
        m_cef_timer->Start(16);
    }

    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    delete[] argv;
#endif
"""
    },
    {
        "name": "Shutdown CEF in OnExit",
        "after_line": "int GUI_App::OnExit()\n{",
        "insert": """#ifdef SLIC3R_CEF_ENABLED
    if (m_cef_timer) {
        m_cef_timer->Stop();
        delete m_cef_timer;
        m_cef_timer = nullptr;
    }
    CEFUtils::Shutdown();
#endif

"""
    }
]

hpp_patches = [
    {
        "name": "Add CEF timer member to GUI_App.hpp",
        "after_line": "    bool             m_show_gcode_window{true};",
        "insert": """
#ifdef SLIC3R_CEF_ENABLED
    wxTimer* m_cef_timer{nullptr};
#endif
"""
    }
]

def read_file(path):
    with open(path, 'r') as f:
        return f.read()

def write_file(path, content):
    with open(path, 'w') as f:
        f.write(content)

def apply_patches(content, patches):
    for patch in patches:
        if "after_line" in patch:
            if "\n" in patch["after_line"]:
                lines = content.split('\n')
                search_lines = patch["after_line"].split('\n')
                for i in range(len(lines) - len(search_lines) + 1):
                    match = True
                    for j, search_line in enumerate(search_lines):
                        if lines[i + j].strip() != search_line.strip():
                            match = False
                            break
                    if match:
                        lines.insert(i + len(search_lines), patch["insert"])
                        print(f"Applied patch: {patch['name']}")
                        break
                content = '\n'.join(lines)
            else:
                pattern = re.escape(patch["after_line"])
                replacement = patch["after_line"] + patch["insert"]
                content = re.sub(pattern, replacement, content, count=1)
                print(f"Applied patch: {patch['name']}")

    return content

if __name__ == "__main__":
    print("Patching GUI_App.cpp...")
    cpp_content = read_file(gui_app_cpp_path)
    modified_cpp = apply_patches(cpp_content, cpp_patches)
    write_file(gui_app_cpp_path, modified_cpp)
    print("GUI_App.cpp patched successfully.\n")

    print("Patching GUI_App.hpp...")
    hpp_content = read_file(gui_app_hpp_path)
    modified_hpp = apply_patches(hpp_content, hpp_patches)
    write_file(gui_app_hpp_path, modified_hpp)
    print("GUI_App.hpp patched successfully.\n")

    print("Done! All patches applied successfully.")
    print("\nNOTE: These scripts modify GUI_App.cpp and GUI_App.hpp.")
    print("Review the changes before committing.")
