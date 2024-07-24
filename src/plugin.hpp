#pragma once

#include <reframework/API.hpp>

extern "C" __declspec(dllexport) void reframework_plugin_required_version(REFrameworkPluginVersion *version);

extern "C" __declspec(dllexport) bool reframework_plugin_initialize(const REFrameworkPluginInitializeParam *param);
