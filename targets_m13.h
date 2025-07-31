
// Make this file the first include
#ifndef TARGETS_IN_m13
#define TARGETS_IN_m13

//**********************************************************************************//
//************************************  Target OS  *********************************//
//**********************************************************************************//

// Target OS Options. Define one of these here:
#define MACOS_m13		// uncomment to compile for MacOS
// #define LINUX_m13		// uncomment to compile for Linux
// #define WINDOWS_m13		// uncomment to compile for Windows

#ifdef WINDOWS_m13
	#define _CRT_SECURE_NO_WARNINGS
	#define WIN_SOCKETS_m13  // define this if winsock2.h needed in application, otherwise comment it out
#endif

#ifdef LINUX_m13
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif
#endif


//**********************************************************************************//
//******************************  Target Applications  *****************************//
//**********************************************************************************//

// Target Application Options: DATABASE_m13, MATLAB_m13, MATLAB_PERSISTENT_m13,
// Define one of these here if appropriate
// #define DATABASE_m13			// MED database (DB) functions (postgres only at this time)
// #define MATLAB_m13			// for screen output & exit functions
// #define MATLAB_PERSISTENT_m13	// for persistent memory between mex calls

// ensure MATLAB_m13 is defined if MATLAB_PERSISTENT_m13 is defined
#ifdef MATLAB_PERSISTENT_m13
	#ifndef MATLAB_m13
		#define MATLAB_m13
	#endif
#endif


//**********************************************************************************//
//***********************************  Debug Modes  ********************************//
//**********************************************************************************//

// #define FT_DEBUG_m13  // uncomment to enable function tracking (and recompile medlib_m13.c)
// #define AT_DEBUG_m13  // uncomment to use allocation tracking (and recompile medlib_m13.c)
// #define AT_CHECK_OVERWRITES_m13  // uncomment to check if memory was written past requested extents
				 // catches small overwrites that remain within the system allocated extents, and thus would not cause an error
				 // detected on free, does not track where overwrite occurred (for this functionality use valgrind or similar)

// ensure AT_DEBUG_m13 is defined if AT_CHECK_OVERWRITES_m13 is defined
#ifdef AT_CHECK_OVERWRITES_m13
	#ifndef AT_DEBUG_m13
		#define AT_DEBUG_m13
	#endif
#endif


#endif  // TARGETS_IN_m13
