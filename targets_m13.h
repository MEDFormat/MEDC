
// Make this file the first include
#ifndef TARGETS_IN_m13
#define TARGETS_IN_m13

//**********************************************************************************//
//************************************  Target OS  *********************************//
//**********************************************************************************//

// Target OS Options: LINUX_m13, MACOS_m13, or WINDOWS_m13
// Define one of these here
#define MACOS_m13
// #define LINUX_m13
// #define WINDOWS_m13

#ifdef WINDOWS_m13
	#define _CRT_SECURE_NO_WARNINGS
	#define NEED_WIN_SOCKETS_m13  // define this if winsock2.h needed in application, otherwise comment it out
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
// #define DATABASE_m13  // MED database (DB) functions (postgres only at this time)
// #define MATLAB_m13  // for screen output & exit functions
// #define MATLAB_PERSISTENT_m13	// For persistent memory between mex calls.
					// NOTE: it may be more convenient to define MATLAB_PERSISTENT_m13
					// only within mex functions that use it, rather than here

#ifdef MATLAB_PERSISTENT_m13
	#define MATLAB_m13
#endif

// If using persistent memory, do something like the following:
//
//	
//	SESSION_m13	*sess = NULL;  // Global session pointer
//
// 	#ifdef MATLAB_PERSISTENT_m13
//	void mexExitFcn(void) {
//
//		// free session separately to close files
//		free_session_m13(sess, TRUE_m13);
//
//		// free everything else (if using AT_DEBUG mode)
//		AT_free_all_m13();
//
//		// free globals (most already freed by AT_free_all_m13)
//		free_globals_m13(TRUE_m13);
//	}
// 	#endif
//
//	void mexFunction(...) {
//		...
// 		#ifdef MATLAB_PERSISTENT_m13
//		globals_m13 = <passed_value>;  // possibly NULL
//		sess = <passed_value>;  // possibly NULL
//		mexAtExit(mexExitFcn);  // register on every entry into mex
// 		#endif
//
//		if (globals_m13 == NULL)
//			initialize_medlib_m13(FALSE_m13, FALSE_m13);  // always NULL until library initialized
//		...
//	}



//**********************************************************************************//
//***********************************  Debug Modes  ********************************//
//**********************************************************************************//

// #define AT_DEBUG_m13  // uncomment to use allocation tracking (and recompile medlib_m13.c)
// #define FN_DEBUG_m13  // uncomment to enable function stack tracking (and recompile medlib_m13.c)


#endif  // TARGETS_IN_m13
