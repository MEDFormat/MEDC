
// Make this file the first include
#ifndef TARGETS_IN_m12
#define TARGETS_IN_m12

//**********************************************************************************//
//************************************  Target OS  *********************************//
//**********************************************************************************//

// Target OS Options: LINUX_m10, MACOS_m10, or WINDOWS_m10
// Define one of these here
#define MACOS_m12
// #define LINUX_m12
// #define WINDOWS_m12

#ifdef WINDOWS_m12
	#define _CRT_SECURE_NO_WARNINGS
	#define NEED_WIN_SOCKETS_m12  // define this if winsock2.h needed in application, otherwise comment it out
#endif

#ifdef LINUX_m12
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif
#endif


//**********************************************************************************//
//********************************  Include DHN Keys  ******************************//
//**********************************************************************************//

// #define DHNKEYS_d12


//**********************************************************************************//
//******************************  Target Applications  *****************************//
//**********************************************************************************//

// Target Application Options: DATABASE_m12, MATLAB_m12, MATLAB_PERSISTENT_m12,
// Define one of these here if appropriate
// #define DATABASE_m12  // MED database (DB) functions (postgres only at this time)
// #define MATLAB_m12  // for screen output & exit functions
// #define MATLAB_PERSISTENT_m12	// For persistent memory between mex calls.
				// NOTE: it may be more convenient to define MATLAB_PERSISTENT_m12
				// only within mex functions that use it, rather than here

	// If using persistent memory, do something like the following:
	//
	//	// Global session pointer
	//	SESSION_m12	*sess = NULL;
	//
	// 	#ifdef MATLAB_PERSISTENT_m12
	//	void mexExitFcn(void) {
	//
	//		// free session seperately to close files
	//		free_session_m12(sess, TRUE_m12);
	//
	//		// free everything else
	//		AT_free_all_m12();
	//
	//		// free globals (most already freed by AT_free_all_m12)
	//		free_globals_m12(TRUE_m12);
	//	}
	// 	#endif
	//
	//	void mexFunction(...) {
	//		...
	// 		#ifdef MATLAB_PERSISTENT_m12
	//		globals_m12 = <passed_value>;  // possibly NULL
	//		sess = <passed_value>;  // possibly NULL
	//		mexAtExit(mexExitFcn);  // register on every entry into mex
	// 		#endif
	//
	//		if (globals_m12 == NULL)
	//			initialize_medlib_m12(FALSE_m12, FALSE_m12);  // always NULL until library initialized
	//		...
	//	}


#ifdef MATLAB_PERSISTENT_m12
	#define MATLAB_m12
#endif


//**********************************************************************************//
//***********************************  Debug Modes  ********************************//
//**********************************************************************************//

// #define AT_DEBUG_m12  // uncomment for debug behavior in allocation tracking (and recompile medlib_m12.c)


#endif  // TARGETS_IN_m12
