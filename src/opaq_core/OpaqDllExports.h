#pragma once

#ifndef WIN32
#define OPAQ_DLL_API extern "C" __attribute__((visibility("default")))
#define OPAQ_DLL_CLASS extern "C" __attribute__((visibility("default")))
#else
#define OPAQ_DLL_API extern "C" __declspec(dllexport)
#define OPAQ_DLL_CLASS extern "C" __declspec(dllexport)
#endif
