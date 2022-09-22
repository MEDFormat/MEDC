
// Make this file the first include
#ifndef TARGETS_IN_m11
#define TARGETS_IN_m11

//**********************************************************************************//
//************************************  Target OS  *********************************//
//**********************************************************************************//

// Target OS Options: LINUX_m10, MACOS_m10, or WINDOWS_m10
// Define one of these here
#define MACOS_m11
// #define LINUX_m11
// #define WINDOWS_m11

#ifdef WINDOWS_m11
	#define _CRT_SECURE_NO_WARNINGS
	#define NEED_WIN_SOCKETS_m11  // define this if winsock2.h needed in application, otherwise comment it out
#endif

#ifdef LINUX_m11
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif
#endif

//**********************************************************************************//
//*******************************  Target Application  *****************************//
//**********************************************************************************//

// Target Application Options: MATLAB_m11
// Define one of these here if appropriate
// #define MATLAB_m11  // for screen output functions & exit()

//**********************************************************************************//
//***********************************  Debug Modes  ********************************//
//**********************************************************************************//

// #define FN_DEBUG_m11  // uncomment to show function entries (and recompile medlib_m11.c)
// #define AT_DEBUG_m11  // uncomment for debug behavior in allocation tracking (and recompile medlib_m11.c)


#endif  // TARGETS_IN_m11
