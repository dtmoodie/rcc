#pragma once


#if (defined WIN32 || defined _WIN32 || defined WINCE || defined __CYGWIN__) && defined RuntimeObjectSystem_EXPORTS
#  define RCC_EXPORTS __declspec(dllexport)
#elif defined __GNUC__ && __GNUC__ >= 4
#  define RCC_EXPORTS __attribute__ ((visibility ("default")))
#else
#  define RCC_EXPORTS
#endif