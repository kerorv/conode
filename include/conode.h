#pragma once

#ifdef _WIN32
	#if defined(LIBCONODE_EXPORTS)
		#define CONODE_API __declspec(dllexport)
	#else
		#define CONODE_API __declspec(dllimport)
	#endif
#else
	#define CONODE_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

CONODE_API void* conode_start();
CONODE_API void conode_stop(void* handle);

#ifdef __cplusplus
}
#endif

