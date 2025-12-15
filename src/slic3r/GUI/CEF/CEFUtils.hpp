#pragma once

#include <string>

namespace Slic3r { namespace GUI { namespace CEFUtils {

bool Initialize(int argc, char** argv);
void Shutdown();
void DoMessageLoopWork();
std::string GetResourcesPath();

}}} // namespace Slic3r::GUI::CEFUtils
