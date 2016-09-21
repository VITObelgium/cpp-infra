#pragma once

#ifndef WIN32
#define OPAQ_DLL_API extern "C"
#define OPAQ_DLL_CLASS extern "C"
#else
#define OPAQ_DLL_API extern "C" __declspec(dllexport)
#define OPAQ_DLL_CLASS extern "C" __declspec(dllexport)
#endif
