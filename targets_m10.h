
// Make this file the first include
#ifndef TARGETS_IN_m10
#define TARGETS_IN_m10

//**********************************************************************************//
//************************************  Target OS  *********************************//
//**********************************************************************************//

// Target OS Options: LINUX_m10, MACOS_m10, or WINDOWS_m10
// Define one of these here
#define MACOS_m10
// #define LINUX_m10
// #define WINDOWS_m10

#ifdef WINDOWS_m10
  #define _CRT_SECURE_NO_WARNINGS
  #define WIN_NEED_SOCKETS_m10  // define this if winsock2.h needed in application, otherwise comment it out
#endif

#ifdef LINUX_m10
  #ifndef _GNU_SOURCE
    #define _GNU_SOURCE
  #endif
#endif

//**********************************************************************************//
//*******************************  Target Application  *****************************//
//**********************************************************************************//

// Target Application Options: MATLAB_m10
// Define one of these here if appropriate
// #define MATLAB_m10  // for screen output functions & exit()

#endif  // TARGETS_IN_m10
