	
#ifndef MEDLIB_IN_m13
#define MEDLIB_IN_m13

//**********************************************************************************//
//******************************* MED 1.1.3 C Library ****************************//
//**********************************************************************************//


// Multiscale Electrophysiology Data (MED) Format Software Library, Version 1.1.3
// Written by Matt Stead


// LICENSE & COPYRIGHT:

// MED library source code (medlib) is copyrighted by Dark Horse Neuro Inc, 2021

// Medlib is free software:
// You can redistribute it and/or modify it under the terms of the Gnu General Public License (Gnu GPL),
// version 2, or any later version (as published by the Free Software Foundation).
// The Gnu GPL requires that any object code built and distributed using this software
// is accompanied by the FULL SOURCE CODE used to generate the object code.

// This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the Gnu GPL for more details.

// If you did not receive a copy of the Gnu GPL along with this code, you can find it on the GNU website ( http://www.gnu.org ).
// You may also obtain a copy by writing to the Free Software Foundation, Inc at:
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

// We kindly ask you to acknowledge medlib in any program or publication in which you use it, but you are not required to do so.

// Commercial versions of medlib may be licensed from Dark Horse Neuro Inc, Bozeman, MT, USA.
// Commercially licensed copies do not require object code using medlib to be accompanied by the corresponding full source code.
// Users interested in a commercial license may contact us through the medformat website ( http://www.medformat.org ).


// ACKNOWLEDGEMENTS:

// MED derives from the Multiscale Electrophysiology Format (MEF), versions 1-3.
// Many people contributed to the MEF effort, but special mention is owed to
// Greg Worrell, Casey Stengel, Andy Gardner, Mark Bower, Vince Vasoli, Ben Brinkmann,
// Dan Crepeau, Jan Cimbálnik, Jon Lange, and Jon Halford for their contributions
// in design, coding, testing, implementation, and adoption.

// The encryption / decryption algorithm is the 128-bit AES standard ( http://www.csrc.nist.gov/publications/fips/fips197/fips-197.pdf ).
// AES routines (128 bit only) are included in the library, with attribution, for convenience.

// The hash algorithm is the SHA-256 standard ( http://csrc.nist.gov/publications/fips/fips180-4/fips-180-4.pdf ).
// Basic SHA-256 routines are included in the library, with attribution, for convenience.

// Strings are encoded in the Universal Character Set standard, ISO/IEC 10646:2012 otherwise known as UTF-8.
// ( http://standards.iso.org/ittf/PubliclyAvailableStandards/c056921_ISO_IEC_10646_2012.zip )
// Minimal UTF-8 manipulation routines are included in the library, for convenience.

// Error detection is implemented with 32-bit cyclic redundancy checksums (CRCs).
// Basic CRC-32 manipulation routines are included in the library, with attribution, for convenience.


// USAGE:

// The library is optimized for 64-bit operating systems on 64-bit processors with 64-bit words and addressing.
// However, it can be used with in 32-bit contexts without modification at a performance cost.

// The library is written with tab width = indent width = 8 spaces and a monospaced font.
// Tabs are tabs characters, not spaces.
// Set your editor preferences to these for intended alignment.

// The library utilizes some non-standard structures:
// 	required compiler option (gcc, clang): -fms-extensions
// 	suppress warnings with: -Wno-microsoft-anon-tag


// VERSIONING:

// All functions, constants, macros, and data types defined in the library are tagged
// with the suffix "_mFL" (for "MED major format 'F', library version 'L'").

// MED_FORMAT_VERSION_MAJOR is restricted to single digits 1 through 9
// MED_FORMAT_VERSION_MINOR is restricted to 0 through 254, minor version resets to zero with new major format version
// MED_LIBRARY_VERSION is restricted to 1 through 255, library version resets to one with new major format version

// MED_FULL_FORMAT_NAME == "<MED_VERSION_MAJOR>.<MED_VERSION_MINOR>"
// MED_FULL_LIBRARY_NAME == "<MED_FULL_FORMAT_NAME>.<MED_LIBRARY_VERSION>"
// MED_LIBRARY_TAG == "<MED_VERSION_MAJOR>.<MED_LIBRARY_VERSION>"

// Tag Examples:
// "_m13" indicates "MED format major version 1, library version 3"
// "_m21" indicates "MED format major version 2, library version 1" (for MED 2)
// "_m213" indicates "MED format major version 2, library version 13" (for MED 2)

// All library versions associated with a particular major format version are guaranteed to work on MED files of that major version.
// Minor format versions may add fields to the format in protected regions, but no preexisting fields will be removed or moved.
// Only library versions released on or afer a minor version will make use of new fields, and only if the minor version of the files contains them.
// Backward compatibility will be maintained between major versions if practical.



//**********************************************************************************//
//********************************* Library Includes *****************************//
//**********************************************************************************//

#include "targets_m13.h"

#ifdef WINDOWS_m13
// the following is necessary to include <winsock2.h> (or can define WIN32_LEAN_AND_MEAN, but excludes a lot of stuff)
// winsock2.h has to be included before windows.h, but requires WIN32 to be defined, which is usually defined by windows.h
// WIN_SOCKETS_m13 is defined in "targets.h"
	#ifdef WIN_SOCKETS_m13
		#ifndef WIN32
			#define WIN32
		#endif // WIN32
		#include <winsock2.h>
  		// #pragma comment(lib, "Ws2_32.lib") // link with Ws2_32.lib (required, but repeated below for other libs)
	#endif // WIN_SOCKETS_m13
	#include <windows.h>
	#include <io.h>
	#include <direct.h>
	#include <fileapi.h>
	#include <share.h>
	#include <memoryapi.h>
	#include <synchapi.h>
	#include <stddef.h>
	#include <winsock.h>
	#include <ws2ipdef.h>
	#include <iphlpapi.h>
	#include <ws2tcpip.h>
	#include <process.h>
	#include <processthreadsapi.h>
	#include <synchapi.h>
	#include <sysinfoapi.h>
	#pragma comment(lib, "Ws2_32.lib") // link with Ws2_32.lib
	#pragma comment(lib, "Iphlpapi.lib") // link with Iphlpapi.lib
	#define _USE_MATH_DEFINES // Needed for standard math constants. Must be defined before math.h included.
#endif // WINDOWS_m13
#if defined MACOS_m13 || defined LINUX_m13
	#include <unistd.h>
	#include <dirent.h>
	#include <sys/time.h>
	#include <sys/resource.h>
	#include <sys/mman.h>
	#include <pthread.h>
	#include <sched.h>
	#include <sys/mman.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <ifaddrs.h>
	#include <netdb.h>
	#include <sys/param.h>
	#include <sys/mount.h>
	#include <termios.h>
	#include <sys/wait.h>
	#include <poll.h>
	#include <semaphore.h>
#endif // MACOS_m13 || LINUX_m13
#ifdef MACOS_m13
	#include <malloc/malloc.h>
	#include <sys/sysctl.h>
	#include <util.h>
	#include <mach/thread_act.h>
#endif // MACOS_m13
#ifdef LINUX_m13
	#include <sys/statfs.h>
	#include <sys/sysinfo.h>
	#include <pty.h>
	#include <utmp.h>
#endif // LINUX_m13
#if defined LINUX_m13 || defined WINDOWS_m13
	#include <malloc.h>
#endif // LINUX_m13 || WINDOWS_m13
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <errno.h>
#ifdef MATLAB_m13
	#include "mex.h"
	#include "matrix.h"
#endif // MATLAB_m13
#ifdef DATABASE_m13
	#include <libpq-fe.h> // postgres header
#endif // DATABASE_m13



//**********************************************************************************//
//******************************** Elemental Typedefs ******************************//
//**********************************************************************************//

#ifndef MED_PRIMITIVES_IN_m13
#define MED_PRIMITIVES_IN_m13

#include <stdint.h>

typedef uint8_t		ui1;
typedef char		si1; // Note: the "char" type is not guaranteed to be a signed one-byte integer. If it is not, library will exit during initialization.
typedef int8_t		tern; // ternary type
typedef uint16_t	ui2;
typedef int16_t		si2;
typedef uint32_t	ui4;
typedef int32_t		si4;
typedef uint64_t	ui8;
typedef int64_t		si8;
typedef float		sf4;
typedef double		sf8;
typedef long double	sf16;

// Ternary Defines
#define TRUE_m13	((tern) 1)
#define UNKNOWN_m13	((tern) 0)
#define FALSE_m13	((tern) -1)
#define NOT_SET_m13	UNKNOWN_m13 // common use of tern zero
#define BOOL_FALSE_m13	UNKNOWN_m13

// Ternary Macros
#define TERN_TO_BOOL_m13(x)	(((x) == TRUE_m13) ? TRUE_m13 : BOOL_FALSE_m13) // convert ternary to boolean  (unknown/unset set to false)
#define BOOL_TO_TERN_m13(x)	(((x) == TRUE_m13) ? TRUE_m13 : FALSE_m13) // convert boolean to ternary  (unknown/unset not defined)

// Reserved si8 Values
#define NAN_SI8_m13		((si8) 0x8000000000000000)
#define NEG_INF_SI8_m13		((si8) 0x8000000000000001)
#define POS_INF_SI8_m13		((si8) 0x7FFFFFFFFFFFFFFF)
#define MAX_VAL_SI8_m13		((si8) 0x7FFFFFFFFFFFFFFE)
#define MIN_VAL_SI8_m13		((si8) 0x8000000000000002)

// Reserved ui8 Values
#define POS_INF_UI8_m13		((ui8) 0xFFFFFFFFFFFFFFFF)
#define MAX_VAL_UI8_m13		((ui8) 0x7FFFFFFFFFFFFFFE)
#define MIN_VAL_UI8_m13		((ui8) 0x0000000000000000)

// Reserved si4 Values
#define NAN_SI4_m13		((si4) 0x80000000)
#define NEG_INF_SI4_m13		((si4) 0x80000001)
#define POS_INF_SI4_m13		((si4) 0x7FFFFFFF)
#define MAX_VAL_SI4_m13		((si4) 0x7FFFFFFE)
#define MIN_VAL_SI4_m13		((si4) 0x80000002)

// Reserved ui4 Values
#define POS_INF_UI4_m13		((ui4) 0xFFFFFFFF)
#define MAX_VAL_UI4_m13		((ui4) 0xFFFFFFFE)
#define MIN_VAL_UI4_m13		((ui4) 0x00000000)

// Reserved si2 Values
#define NAN_SI2_m13		((si2) 0x8000)
#define NEG_INF_SI2_m13		((si2) 0x8001)
#define POS_INF_SI2_m13		((si2) 0x7FFF)
#define MAX_VAL_SI2_m13		((si2) 0x7FFE)
#define MIN_VAL_SI2_m13		((si2) 0x8002)

// Reserved ui2 Values
#define POS_INF_UI2_m13		((ui2) 0xFFFF)
#define MAX_VAL_UI2_m13		((ui2) 0xFFFE)
#define MIN_VAL_UI2_m13		((ui2) 0x0000)

#endif // MED_PRIMITIVES_IN_m13


//**********************************************************************************//
//***************************** Pascal Strings (pstr) ******************************//
//**********************************************************************************//

// Enhanced Pascal strings
// Useful for large strings, or strings whose length needs to be known often
// (eventually all medlib string functions will accept these)

#define PSTR_TAG_m13		((si1) 0x80)
#define PSTR_MAX_LEN_m13	((ui4) 0xFFFFFFFF)

// flags (bytes 1-3, byte 0 used for tag)
#define PSTR_FLAG_ALLOCED_m13	((ui4) 1 << 8) // pstr structure was allocated
#define PSTR_FLAG_CONST_m13	((ui4) 1 << 9) // string is const
#define PSTR_FLAG_ASCII_m13	((ui4) 1 << 10) // string is ascii (not sure if useful)

// macros
#define PSTR_m13(x)		(((x) == NULL) ? FALSE_m13 : ((*((ui1 *) (x)) == PSTR_TAG_m13) ? TRUE_m13 : FALSE_m13)) // TRUE_m13 if x is a pstr

// typedefs
typedef struct {
	union {
		si1	tag; // marker for pstr vs *si1 (PSTR_TAG_m13 [== 0x80] is not valid as first byte of any utf8 character, including ascii)
		ui4	flags; // bytes 1-3 used for flags
	};
	ui4	len; // string length in bytes (not necessarily characters), not including terminal zero
	si1	*str; // standard string pointer (terminal zero required)
} pstr;  // (untagged & uncapitalized because it is treated as an elemental type)



//**********************************************************************************//
//***************** Record Structures Integral to the MED Library ******************//
//*************** (prototypes & constants declared in medrec_m13.h) ****************//
//**********************************************************************************//


//*********************************************************************************//
//******************************* Sgmt: Segment Record ****************************//
//*********************************************************************************//

// A segment record is entered at the Session and or Channel Level for each new segment
// The encryption level for these records is typically set to the same as for metadata section 2

// Structures
typedef struct {
	si8 	end_time;
	union {
		si8	start_samp_num; // session-relative (global indexing) (SAMPLE_NUMBER_NO_ENTRY_m13 for variable frequency, session level entries)
		si8	start_frame_num; // session-relative (global indexing) (FRAME_NUMBER_NO_ENTRY_m13 for variable frequency, session level entries)
		si8	start_idx; // generic sample / frame number
	};
	union {
		si8	end_samp_num; // session-relative (global indexing) (SAMPLE_NUMBER_NO_ENTRY_m13 for variable frequency, session level entries)
		si8	end_frame_num; // session-relative (global indexing) (FRAME_NUMBER_NO_ENTRY_m13 for variable frequency, session level entries)
		si8	end_idx; // generic sample / frame number
	};
	si4	seg_num;
	union {
		sf4	samp_freq; // channel sampling frequency (REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m13 in session level records, if sampling frequencies vary across time series channels)
		sf4	frame_rate; // channel frame rate (REC_Sgmt_v10_FRAME_RATE_VARIABLE_m13 in session level records, if frame rates vary across video channels)
		sf4	rate; // generic rate
	};
} REC_Sgmt_v11_m13;
// Description follows sampling frequency / frame rate in structure.
// The description is an aribitrary length array of characters padded (including terminal zero) to 8 byte alignment (total of structure + string).

typedef struct {
	si8 end_time;
	union {
		si8	start_samp_num; // session-relative (global indexing)
		si8	start_frame_num; // session-relative (global indexing)
		si8	start_idx; // generic sample / frame number
	};
	union {
		si8	end_samp_num; // session-relative (global indexing)
		si8	end_frame_num; // session-relative (global indexing)
		si8	end_idx; // generic sample / frame number
	};
	ui8	seg_UID;
	si4	seg_num;
	si4	acq_chan_num; // REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_ALL_CHANNELS_m13 in session level records
	union {
		sf8	samp_freq; // channel sampling frequency (REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m13 in session level records, if sampling frequencies vary across time series channels)
		sf8	frame_rate; // channel frame rate (REC_Sgmt_v10_FRAME_RATE_VARIABLE_m13 in session level records, if frame rates vary across video channels)
		sf8	rate; // generic rate
	};
} REC_Sgmt_v10_m13; // version 1.0 included for backward compatibility
// Description follows sampling frequency / frame rate in structure.
// The description is an aribitrary length array of characters padded (including terminal zero) to 8 byte alignment (total of structure + string).



//*************************************************************************************//
//******************************* Stat: Statistics Record *****************************//
//*************************************************************************************//

typedef struct {
	si4 minimum;
	si4 maximum;
	si4 mean;
	si4 median;
	si4 mode;
	sf4 variance;
	sf4 skewness;
	sf4 kurtosis;
} REC_Stat_v10_m13;



//*************************************************************************************//
//*********************** CMP Structures required in MED section **********************//
//*************************************************************************************//

typedef struct {
 // CMP block header fixed region start
	ui8 block_start_UID;
	ui4 block_CRC;
	ui4 block_flags;
	si8 start_time;
	si4 acquisition_channel_number;
	ui4 total_block_bytes;
 // CMP block encryption start
	ui4 number_of_samples;
	ui2 number_of_records;
	ui2 record_region_bytes;
	ui4 parameter_flags;
	ui2 parameter_region_bytes;
	ui2 protected_region_bytes;
	ui2 discretionary_region_bytes;
	ui2 model_region_bytes;
	ui4 total_header_bytes;
 // CMP block header variable region start
} CMP_FIXED_BH_m13;

typedef struct {
	sf8	user_threshold;
	sf8	algorithm_threshold_LFP;
	sf8	algorithm_threshold_no_filt;
} CMP_VDS_THRESHOLD_MAP_ENTRY_m13;



//**********************************************************************************//
//*********************************** ENCRYPTION ***********************************//
//**********************************************************************************//

// Encryption & Password Constants
#define ENCRYPTION_LEVEL_NO_ENTRY_m13		-128
#define NO_ENCRYPTION_m13			0
#define LEVEL_0_ENCRYPTION_m13			NO_ENCRYPTION_m13
#define LEVEL_1_ENCRYPTION_m13			1
#define LEVEL_2_ENCRYPTION_m13			2
#define ENCRYPTION_NO_ENTRY_m13			-128
#define ENCRYPTION_VARIABLE_m13			ENCRYPTION_NO_ENTRY_m13
#define LEVEL_0_ACCESS_m13			LEVEL_0_ENCRYPTION_m13
#define LEVEL_1_ACCESS_m13			LEVEL_1_ENCRYPTION_m13
#define LEVEL_2_ACCESS_m13			LEVEL_2_ENCRYPTION_m13
#define LEVEL_1_ENCRYPTION_DECRYPTED_m13	-LEVEL_1_ENCRYPTION_m13
#define LEVEL_2_ENCRYPTION_DECRYPTED_m13	-LEVEL_2_ENCRYPTION_m13
#define ENCRYPTION_BLOCK_BYTES_m13		16 // AES-128
#define ENCRYPTION_KEY_BYTES_m13		176 // AES-128 = ((AES_NR + 1) * AES_NK * AES_NB)
#define ENCRYPTION_KEY_BLOCKS_m13		(ENCRYPTION_KEY_BYTES_m13 >> 4)
#define PASSWORD_BYTES_m13			ENCRYPTION_BLOCK_BYTES_m13
#define MAX_PASSWORD_CHARACTERS_m13		PASSWORD_BYTES_m13
#define MAX_ASCII_PASSWORD_STRING_BYTES_m13	(MAX_PASSWORD_CHARACTERS_m13 + 1) // 1 byte per character in ascii plus terminal zero
#define MAX_UTF8_PASSWORD_BYTES_m13		(MAX_PASSWORD_CHARACTERS_m13 * 4) // up to 4 bytes per character in UTF-8
#define MAX_PASSWORD_STRING_BYTES_m13		(MAX_UTF8_PASSWORD_BYTES_m13 + 1) // 1 byte for null-termination
#define PASSWORD_VALIDATION_FIELD_BYTES_m13	PASSWORD_BYTES_m13
#define PASSWORD_HINT_BYTES_m13			256

// Password Data Structure
typedef struct {
	ui1	level_1_encryption_key[ENCRYPTION_KEY_BYTES_m13];
	ui1	level_2_encryption_key[ENCRYPTION_KEY_BYTES_m13];
	si1	level_1_password_hint[PASSWORD_HINT_BYTES_m13];
	si1	level_2_password_hint[PASSWORD_HINT_BYTES_m13];
	ui1	access_level;
	tern	processed;
	tern 	hints_exist;  // if (*level_1_password_hint || *level_2_password_hint)
} PASSWORD_DATA_m13;



//**********************************************************************************//
//********************************* MED Constants **********************************//
//**********************************************************************************//

// Versioning Constants
#define MED_FORMAT_VERSION_MAJOR_m13		1 // restricted to single digits 1 through 9
#define MED_FORMAT_VERSION_MINOR_m13		1 // restricted to 0 through 254, minor version resets to zero with new major format version
#define MED_LIBRARY_VERSION_m13			3 // restricted to 1 through 254, library version resets to one with new major format version
#define MED_VERSION_NO_ENTRY_m13		0xFF
#define MED_FULL_FORMAT_NAME_m13		"\"" ## MED_VERSION_MAJOR_m13 ## "." ## MED_VERSION_MINOR_m13 ## "\""
#define MED_FULL_LIBRARY_NAME_m13		"\"" ## MED_FULL_FORMAT_NAME_m13 ## "." ## MED_LIBRARY_VERSION_m13 ## "\""
#define MED_LIBRARY_TAG_m13			"\"_m" ## MED_VERSION_MAJOR_m13 ## MED_LIBRARY_VERSION_m13 ## "\""

// Miscellaneous Constants
#define NAME_BYTES_m13				256 // utf8[63]
#define SEG_NAME_BYTES_m13 			(NAME_BYTES_m13 + 8)
#define VID_NAME_BYTES_m13 			(SEG_NAME_BYTES_m13 + 8)
#define MAX_NAME_BYTES_m13			VID_NAME_BYTES_m13
#define PATH_BYTES_m13				1024 // utf8[255]
#define INDEX_BYTES_m13				24
#define BIG_ENDIAN_m13				0
#define LITTLE_ENDIAN_m13			1
#define TYPE_BYTES_m13				5
#define TYPE_STRLEN_m13				4
#define UID_BYTES_m13				8
#define UID_NO_ENTRY_m13			0
#define PAD_BYTE_VALUE_m13			0x7e // ascii tilde ("~") as si1
#define FILE_NUMBERING_DIGITS_m13		4
#define RATE_NO_ENTRY_m13			-1.0
#define SAMPLING_FREQUENCY_NO_ENTRY_m13		RATE_NO_ENTRY_m13
#define FRAME_RATE_NO_ENTRY_m13			RATE_NO_ENTRY_m13
#define RATE_VARIABLE_m13			-2.0
#define SAMPLING_FREQUENCY_RATE_VARIABLE_m13	RATE_VARIABLE_m13
#define FRAME_RATE_VARIABLE_m13			RATE_VARIABLE_m13
#define UNKNOWN_NUMBER_OF_ENTRIES_m13		-1
#define SEGMENT_NUMBER_NO_ENTRY_m13		0 // segments number from one
#define FIRST_OPEN_SEG_m13			-1
#define CHANNEL_NUMBER_NO_ENTRY_m13		-1
#define CHANNEL_NUMBER_ALL_CHANNELS_m13		-2
#define DOES_NOT_EXIST_m13			FALSE_m13 // -1
#define EXISTS_ERR_m13				UNKNOWN_m13 // 0
#define FILE_EXISTS_m13				TRUE_m13 // 1
#define DIR_EXISTS_m13				((si1) 2)
#define SIZE_STRING_BYTES_m13			32
#define UNKNOWN_SEARCH_m13			0
#define TIME_SEARCH_m13				1
#define INDEX_SEARCH_m13			2
#define NO_OVERFLOWS_m13			4 // e.g. in find_index_m13(), restrict returned index to valid segment values
#define IPV4_ADDRESS_BYTES_m13			4
#define POSTAL_CODE_BYTES_m13			16
#define LOCALITY_BYTES_m13			64  // ascii[63]
#define THREAD_NAME_BYTES_m13			64
#define SAMPLE_NUMBER_EPS_m13			((sf8) 0.001)
#define FRAME_NUMBER_EPS_m13			((sf8) 0.01)
#define UNMAPPED_CHAN_m13			((si4) -1)
#define NO_FLAGS_m13				((ui8) 0)

#if defined MACOS_m13 || defined LINUX_m13
	#define NULL_DEVICE_m13					"/dev/null"
	#define DIR_BREAK_m13					'/'
	#define GLOBALS_FILE_CREATION_UMASK_DEFAULT_m13		S_IWOTH // removes write permission for "other" (defined in <sys/stat.h>)
#endif // MACOS_m13 || LINUX_m13
#ifdef WINDOWS_m13
	#define NULL_DEVICE_m13					"NUL"
	#define DIR_BREAK_m13					'\\'
	#define GLOBALS_FILE_CREATION_UMASK_DEFAULT_m13		0 // full permissions for everyone (Windows does not support "other" category)
	#define PRINTF_BUF_LEN_m13				1024

 // MacOS / Linux constants for mprotect_m13()
	#define PROT_NONE	((si4) 0) // page can not be accessed
	#define PROT_READ	((si4) 1 << 0) // page can be read
	#define PROT_WRITE	((si4) 1 << 1) // page can be written
	#define PROT_EXEC	((si4) 1 << 2) // page can be executed
#endif

// Pipes
#define READ_END_m13		0
#define WRITE_END_m13		1
#define PIPE_FAILURE_m13	((si4) 255)
#define PIPE_FAILURE_SEND_m13	((si4) -1) // sent from child, received as (si4) ((ui1) PIPE_FAILURE_m13) [== 255]

// Target Value Constants (ui4)
#define NO_IDX_m13			-1 // assigned to signed values (si4 or si8)
#define FIND_DEFAULT_MODE_m13		0
#define FIND_START_m13			(1 << 0)
#define FIND_END_m13			(1 << 1)
#define FIND_CENTER_m13			(1 << 2)
#define FIND_PREVIOUS_m13		(1 << 3)
#define FIND_CURRENT_m13		(1 << 4)
#define FIND_NEXT_m13			(1 << 5)
#define FIND_CLOSEST_m13		(1 << 6)
#define FIND_LAST_BEFORE_m13		(1 << 7)
#define FIND_FIRST_ON_OR_AFTER_m13	(1 << 8)
#define FIND_LAST_ON_OR_BEFORE_m13	(1 << 9)
#define FIND_FIRST_AFTER_m13		(1 << 10)
#define FIND_ABSOLUTE_m13		(1 << 30) // session relative sample numbering
#define FIND_RELATIVE_m13		(1 << 31) // segment relative sample numbering

// Text Color Constant Strings
#ifdef MATLAB_m13 // Matlab doesn't do text coloring this way (can be done with CPRINTF())
	#define TC_BLACK_m13		""
	#define TC_RED_m13		""
	#define TC_GREEN_m13		""
	#define TC_YELLOW_m13		""
	#define TC_BLUE_m13		""
	#define TC_MAGENTA_m13		""
	#define TC_CYAN_m13		""
	#define TC_LIGHT_GRAY_m13	""
	#define TC_DARK_GRAY_m13	""
	#define TC_LIGHT_RED_m13	""
	#define TC_LIGHT_GREEN_m13	""
	#define TC_LIGHT_YELLOW_m13	""
	#define TC_LIGHT_BLUE_m13	""
	#define TC_LIGHT_MAGENTA_m13	""
	#define TC_LIGHT_CYAN_m13	""
	#define TC_WHITE_m13		""
	#define TC_BRIGHT_BLACK_m13	""
	#define TC_BRIGHT_RED_m13	""
	#define TC_BRIGHT_GREEN_m13	""
	#define TC_BRIGHT_YELLOW_m13	""
	#define TC_BRIGHT_BLUE_m13	""
	#define TC_BRIGHT_MAGENTA_m13	""
	#define TC_BRIGHT_CYAN_m13	""
	#define TC_BRIGHT_WHITE_m13	""
	#define TC_RESET_m13		""
 // non-color constants
	#define TC_BOLD_m13		""
	#define TC_BOLD_RESET_m13	""
	#define TC_UNDERLINE_m13	""
	#define TC_UNDERLINE_RESET_m13	""
#else // MATLAB_m13
	#define TC_BLACK_m13		"\033[30m"
	#define TC_RED_m13		"\033[31m"
	#define TC_GREEN_m13		"\033[32m"
	#define TC_YELLOW_m13		"\033[33m"
	#define TC_BLUE_m13		"\033[34m"
	#define TC_MAGENTA_m13		"\033[35m"
	#define TC_CYAN_m13		"\033[36m"
	#define TC_LIGHT_GRAY_m13	"\033[37m"
	#define TC_DARK_GRAY_m13	"\033[90m"
	#define TC_LIGHT_RED_m13	"\033[91m"
	#define TC_LIGHT_GREEN_m13	"\033[92m"
	#define TC_LIGHT_YELLOW_m13	"\033[93m"
	#define TC_LIGHT_BLUE_m13	"\033[94m"
	#define TC_LIGHT_MAGENTA_m13	"\033[95m"
	#define TC_LIGHT_CYAN_m13	"\033[96m"
	#define TC_WHITE_m13		"\033[97m"
	#define TC_BRIGHT_BLACK_m13	"\033[30;1m"
	#define TC_BRIGHT_RED_m13	"\033[31;1m"
	#define TC_BRIGHT_GREEN_m13	"\033[32;1m"
	#define TC_BRIGHT_YELLOW_m13	"\033[33;1m"
	#define TC_BRIGHT_BLUE_m13	"\033[34;1m"
	#define TC_BRIGHT_MAGENTA_m13	"\033[35;1m"
	#define TC_BRIGHT_CYAN_m13	"\033[36;1m"
	#define TC_BRIGHT_WHITE_m13	"\033[37;1m"
	#define TC_RESET_m13		"\033[0m"
 // non-color constants
	#define TC_BOLD_m13		"\033[1m"
	#define TC_BOLD_RESET_m13	"\033[21m"
	#define TC_UNDERLINE_m13	"\033[4m"
	#define TC_UNDERLINE_RESET_m13	"\033[24m"
#endif // not MATLAB_m13

// Time Related Constants
#define TIMEZONE_ACRONYM_BYTES_m13		8 // ascii[7]
#define TIMEZONE_STRING_BYTES_m13		64 // ascii[63]
#define MAXIMUM_STANDARD_UTC_OFFSET_m13		((si4) 86400)
#define MINIMUM_STANDARD_UTC_OFFSET_m13		((si4) -86400)
#define STANDARD_UTC_OFFSET_NO_ENTRY_m13	((si4) 0x7FFFFFFF)
#define MAXIMUM_DST_OFFSET_m13			7200
#define MINIMUM_DST_OFFSET_m13			0
#define DST_OFFSET_NO_ENTRY_m13			-1
#define TIME_STRING_BYTES_m13			128
#define NUMBER_OF_SAMPLES_NO_ENTRY_m13		-1
#define NUMBER_OF_FRAMES_NO_ENTRY_m13		NUMBER_OF_SAMPLES_NO_ENTRY_m13
#define EMPTY_SLICE_m13				-1
#define INDEX_NO_ENTRY_m13			((si8) 0x8000000000000000)
#define SAMPLE_NUMBER_NO_ENTRY_m13		INDEX_NO_ENTRY_m13
#define FRAME_NUMBER_NO_ENTRY_m13		INDEX_NO_ENTRY_m13
#define BEGINNING_OF_INDICES_m13		((si8) 0x0000000000000000)
#define BEGINNING_OF_SAMPLE_NUMBERS_m13		BEGINNING_OF_INDICES_m13
#define BEGINNING_OF_FRAME_NUMBERS_m13		BEGINNING_OF_INDICES_m13
#define END_OF_INDICES_m13			((si8) 0x7FFFFFFFFFFFFFFF)
#define END_OF_SAMPLE_NUMBERS_m13		END_OF_INDICES_m13
#define END_OF_FRAME_NUMBERS_m13		END_OF_INDICES_m13
#define UUTC_NO_ENTRY_m13			((si8) 0x8000000000000000)
#define UUTC_EARLIEST_TIME_m13			((si8) 0x0000000000000000) // 00:00:00.000000 Thursday, 1 Jan 1970, UTC
#define UUTC_LATEST_TIME_m13			((si8) 0x7FFFFFFFFFFFFFFF) // 04:00:54.775808 Sunday, 10 Jan 29424, UTC
#define BEGINNING_OF_TIME_m13			UUTC_EARLIEST_TIME_m13
#define END_OF_TIME_m13				UUTC_LATEST_TIME_m13
#define CURRENT_TIME_m13			((si8) 0xFFFFFFFFFFFFFFFF) // used with time_string_m13() & generate_recording_time_offset_m13()
#define TWENTY_FOURS_HOURS_m13			((si8) 86400000000)
#define Y2K_m13					((si8) 0x00035D013B37E000) // 00:00:00.000000 Saturday, 1 Jan 2000, UTC (946684800000000 decimal)
#define WIN_TICKS_PER_USEC_m13			((si8) 10)
#define WIN_USECS_TO_EPOCH_m13			((si8) 11644473600000000)

// Time Change Code Constants
#define DTCC_VALUE_NOT_OBSERVED_m13			0
#define DTCC_VALUE_NO_ENTRY_m13				-1
#define DTCC_VALUE_DEFAULT_m13				DTCC_VALUE_NO_ENTRY_m13
#define DTCC_DST_END_CODE				-1
#define DTCC_DST_START_CODE				1
#define DTCC_DST_NOT_OBSERVED_CODE			0
#define DTCC_DAY_OF_WEEK_NO_ENTRY			0
#define DTCC_FIRST_RELATIVE_WEEKDAY_OF_MONTH		1
#define DTCC_LAST_RELATIVE_WEEKDAY_OF_MONTH		6
#define DTCC_RELATIVE_WEEKDAY_OF_MONTH_NO_ENTRY		0
#define DTCC_DAY_OF_MONTH_NO_ENTRY			0
#define DTCC_MONTH_NO_ENTRY				-1
#define DTCC_HOURS_OF_DAY_NO_ENTRY			-128
#define DTCC_LOCAL_REFERENCE_TIME			0
#define DTCC_UTC_REFERENCE_TIME				1
#define DTCC_REFERENCE_TIME_NO_ENTRY			-1
#define DTCC_SHIFT_MINUTES_TIME_NO_ENTRY		-128
#define DTCC_START_DATE_NO_ENTRY			-1 // NO_ENTRY indicates it is the only historical rule for this timezone in the table

// Global Defaults
#define GLOBALS_BEHAVIOR_DEFAULT_m13		 		DEFAULT_BEHAVIOR_m13
#define GLOBALS_FILE_LOCK_MODE_DEFAULT_m13			FLOCK_MODE_MED_m13
#define GLOBALS_FILE_LOCK_TIMEOUT_DEFAULT_m13			"1 ms"
#define GLOBALS_ACCESS_TIMES_DEFAULT_m13			FALSE_m13
#define GLOBALS_CRC_MODE_DEFAULT_m13				CRC_CALCULATE_m13
#define GLOBALS_WRITE_SORTED_RECORDS_DEFAULT_m13		TRUE_m13
#define GLOBALS_WRITE_CORRECTED_HEADERS_DEFAULT_m13		TRUE_m13
#define GLOBALS_UPDATE_FILE_SYSTEM_NAMES_DEFAULT_m13		TRUE_m13
#define GLOBALS_UPDATE_HEADER_NAMES_DEFAULT_m13			TRUE_m13
#define GLOBALS_UPDATE_MED_VERSION_DEFAULT_m13			TRUE_m13
#define GLOBALS_UPDATE_PARITY_DEFAULT_m13			TRUE_m13
#define GLOBALS_INCREASE_PRIORITY_DEFAULT_m13			TRUE_m13
#define GLOBALS_BEHAVIOR_STACK_SIZE_INCREMENT_m13		16 // number of behaviors
#define GLOBALS_BEHAVIOR_STACKS_LIST_SIZE_INCREMENT_m13		32 // number of threads
#define GLOBALS_FUNCTION_STACK_SIZE_INCREMENT_m13		16 // number of functions
#define GLOBALS_FUNCTION_STACKS_LIST_SIZE_INCREMENT_m13		32 // number of threads
#define GLOBALS_FLOCK_LIST_SIZE_INCREMENT_m13			512 // number of open files
#define GLOBALS_THREAD_LIST_SIZE_INCREMENT_m13			32 // number of threads
#define GLOBALS_SGMT_LIST_SIZE_INCREMENT_m13			8 // number of rates
#define GLOBALS_PROC_GLOBS_LIST_SIZE_INCREMENT_m13		1 // usually only one, but mechanics in place to make this do more
#define GLOBALS_REFERENCE_CHANNEL_IDX_NO_ENTRY_m13		-1
#define GLOBALS_MMAP_BLOCK_BYTES_NO_ENTRY_m13			((ui4) 0)
#define GLOBALS_MMAP_BLOCK_BYTES_DEFAULT_m13			4096 // 4 KiB
#define GLOBALS_THREADING_DEFAULT_m13				TRUE_m13
#define GLOBALS_AT_LIST_SIZE_INCREMENT_m13			8096

// Global Time Defaults
#define GLOBALS_OBSERVE_DST_DEFAULT_m13				FALSE_m13
#define GLOBALS_RTO_KNOWN_DEFAULT_m13				UNKNOWN_m13
#define GLOBALS_SESSION_START_TIME_DEFAULT_m13			UUTC_NO_ENTRY_m13
#define GLOBALS_SESSION_END_TIME_DEFAULT_m13			UUTC_NO_ENTRY_m13
#define GLOBALS_RECORDING_TIME_OFFSET_DEFAULT_m13		0
#define GLOBALS_RECORDING_TIME_OFFSET_NO_ENTRY_m13		-1 // negative values are not valid
#define GLOBALS_STANDARD_UTC_OFFSET_DEFAULT_m13			0
#define GLOBALS_STANDARD_UTC_OFFSET_NO_ENTRY_m13		86400 // 24 hours, in seconds (valid range -12 to +14 hours)
#define GLOBALS_STANDARD_TIMEZONE_ACRONYM_DEFAULT_m13		"oUTC"
#define GLOBALS_STANDARD_TIMEZONE_STRING_DEFAULT_m13		"Offset Coordinated Universal Time"
#define GLOBALS_DAYLIGHT_TIMEZONE_ACRONYM_DEFAULT_m13		"" // UTC does not observe DST
#define GLOBALS_DAYLIGHT_TIMEZONE_STRING_DEFAULT_m13		"" // UTC does not observe DST

// Hierarchy Type Constants
#define NO_TYPE_CODE_m13			((ui4) 0x00000000) // ui4 (big & little endian)
#define UNKNOWN_TYPE_CODE_m13			NO_TYPE_CODE_m13
#define NO_TYPE_STR_m13				"" // ascii[4]
#define ALL_TYPES_STRING_m13			"allt" // ascii[4]
#define ALL_TYPES_CODE_m13			((ui4) 0x746C6C61) // ui4 (little endian)
// #define ALL_TYPES_CODE_m13			((ui4) 0x616C6C74) // ui4 (big endian)
#define PROC_GLOBS_TYPE_STR_m13			"pglb" // ascii[4]
#define PROC_GLOBS_TYPE_CODE_m13		((ui4) 0x626C6770) // ui4 (little endian)
// #define PROC_GLOBS_TYPE_CODE_m13		((ui4) 0x70676C62) // ui4 (big endian)
#define SESS_TYPE_STR_m13			"medd" // ascii[4]
#define SESS_TYPE_CODE_m13			((ui4) 0x6464656D) // ui4 (little endian)
// #define SESS_TYPE_CODE_m13			((ui4) 0x6D656464) // ui4 (big endian)
#define TS_CHAN_TYPE_STR_m13			"ticd" // ascii[4]
#define TS_CHAN_TYPE_CODE_m13			((ui4) 0x64636974) // ui4 (little endian)
// #define TS_CHAN_TYPE_CODE_m13		((ui4) 0x74696364) // ui4 (big endian)
#define TS_SEG_TYPE_STR_m13			"tisd" // ascii[4]
#define TS_SEG_TYPE_CODE_m13			((ui4) 0x64736974) // ui4 (little endian)
// #define TS_SEG_TYPE_CODE_m13			((ui4) 0x74697364) // ui4 (big endian)
#define TS_METADATA_TYPE_STR_m13		"tmet" // ascii[4]
#define TS_METADATA_TYPE_CODE_m13		((ui4) 0x74656D74) // ui4 (little endian)
// #define TS_METADATA_TYPE_CODE_m13		((ui4) 0x746D6574) // ui4 (big endian)
#define TS_DATA_TYPE_STR_m13			"tdat" // ascii[4]
#define TS_DATA_TYPE_CODE_m13			((ui4) 0x74616474) // ui4 (little endian)
// #define TS_DATA_TYPE_CODE_m13		((ui4) 0x74646174) // ui4 (big endian)
#define TS_INDS_TYPE_STR_m13			"tidx" // ascii[4]
#define TS_INDS_TYPE_CODE_m13			((ui4) 0x78646974) // ui4 (little endian)
// #define TS_INDS_TYPE_CODE_m13		((ui4) 0x74696478) // ui4 (big endian)
#define VID_SEG_TYPE_STR_m13			"visd" // ascii[4]
#define VID_SEG_TYPE_CODE_m13			((ui4) 0x64736976) // ui4 (little endian)
// #define VID_SEG_TYPE_CODE_m13		((ui4) 0x76697364) // ui4 (big endian)
#define VID_CHAN_TYPE_STR_m13			"vicd" // ascii[4]
#define VID_CHAN_TYPE_CODE_m13			((ui4) 0x64636976) // ui4 (little endian)
// #define VID_CHAN_TYPE_CODE_m13		((ui4) 0x76696364) // ui4 (big endian)
#define VID_METADATA_TYPE_STR_m13		"vmet" // ascii[4]
#define VID_METADATA_TYPE_CODE_m13		((ui4) 0x74656D76) // ui4 (little endian)
// #define VID_METADATA_TYPE_CODE_m13		((ui4) 0x766D6574) // ui4 (big endian)
#define VID_DATA_TYPE_STR_m13			"vdat" // ascii[4]				// NOT a file type extension
#define VID_DATA_TYPE_CODE_m13			((ui4) 0x74616476) // ui4 (little endian)	// NOT a file type extension
// #define VID_DATA_TYPE_CODE_m13		((ui4) 0x76646174) // ui4 (big endian)		// NOT a file type extension
#define VID_INDS_TYPE_STR_m13			"vidx" // ascii[4]
#define VID_INDS_TYPE_CODE_m13			((ui4) 0x78646976) // ui4 (little endian)
// #define VID_INDS_TYPE_CODE_m13		((ui4) 0x76696478) // ui4 (big endian)
#define SSR_TYPE_STR_m13			"recd" // ascii[4]
#define SSR_TYPE_CODE_m13			((ui4) 0x64636572) // ui4 (little endian)
// #define SSR_TYPE_CODE_m13			((ui4) 0x72656364) // ui4 (big endian)
#define REC_DATA_TYPE_STR_m13			"rdat" // ascii[4]
#define REC_DATA_TYPE_CODE_m13			((ui4) 0x74616472) // ui4 (little endian)
// #define REC_DATA_TYPE_CODE_m13		((ui4) 0x72646174) // ui4 (big endian)
#define REC_INDS_TYPE_STR_m13			"ridx" // ascii[4]
#define REC_INDS_TYPE_CODE_m13			((ui4) 0x78646972) // ui4 (little endian)
// #define REC_INDS_TYPE_CODE_m13		((ui4) 0x72696478) // ui4 (big endian)

// Channel Types
#define UNKNOWN_CHANNEL_TYPE_m13	NO_TYPE_CODE_m13
#define TS_CHAN_TYPE_m13		TS_CHAN_TYPE_CODE_m13
#define VID_CHAN_TYPE_m13		VID_CHAN_TYPE_CODE_m13

// Reference Channel Types (used in change_index_chan_m13())
#define DEFAULT_CHAN_m13		0
#define DEFAULT_TS_CHAN_m13		1
#define DEFAULT_VID_CHAN_m13		2
#define HIGHEST_RATE_TS_CHAN_m13	3
#define LOWEST_RATE_TS_CHAN_m13		4
#define HIGHEST_RATE_VID_CHAN_m13	5
#define LOWEST_RATE_VID_CHAN_m13	6

// Generate File List flags
 // Path Parts
#define GFL_PATH_m13 			((ui4) 1)
#define GFL_NAME_m13 			((ui4) 2)
#define GFL_EXTENSION_m13 		((ui4) 4)
#define GFL_FULL_PATH_m13 		(GFL_PATH_m13 | GFL_NAME_m13 | GFL_EXTENSION_m13)
#define GFL_PATH_PARTS_MASK_m13 	GFL_FULL_PATH_m13
 // Other Options
#define GFL_FREE_INPUT_LIST_m13		((ui4) 16)
#define GFL_INCLUDE_PARITY_m13		((ui4) 32) // files or directories
#define GFL_INCLUDE_INVISIBLE_m13	((ui4) 64) // files or directories

// System Pipe flags
#define SP_DEFAULT_m13			0 // no flags set (default)
#define SP_TEE_TO_TERMINAL_m13		1 // print buffer(s) to terminal in addition to returning
#define SP_BEHAVIOR_PASSED_m13		2 // behavior passed as first vararg
#define SP_SEPARATE_STREAMS_m13		4 // return seprate "stdout" & "stderr" buffers (buffer = stdout, e_buffer = stderr), otherwise ganged

// Spaces Constants
#define NO_SPACES_m13			((ui4) 0)
#define ESCAPED_SPACES_m13		((ui4) 1)
#define UNESCAPED_SPACES_m13		((ui4) 2)
#define ALL_SPACES_m13			(ESCAPED_SPACES_m13 | UNESCAPED_SPACES_m13)

// Universal Header: File Format Constants
#define UH_OFFSET_m13						0
#define UH_BYTES_m13						1024 // 1 KiB
#define UH_HEADER_CRC_OFFSET_m13				0 // ui4
#define UH_BODY_CRC_OFFSET_m13					4 // ui4
#define UH_HEADER_CRC_START_OFFSET_m13				UH_BODY_CRC_OFFSET_m13
#define UH_BODY_CRC_START_OFFSET_m13				UH_BYTES_m13
#define UH_FILE_END_TIME_OFFSET_m13				8 // si8
#define UH_NUMBER_OF_ENTRIES_OFFSET_m13				16 // si8
#define UH_NUMBER_OF_ENTRIES_NO_ENTRY_m13			-1
#define UH_MAXIMUM_ENTRY_SIZE_OFFSET_m13			24 // ui4
#define UH_MAXIMUM_ENTRY_SIZE_NO_ENTRY_m13			0
#define UH_SEGMENT_NUMBER_OFFSET_m13				28 // si4
#define UH_SEGMENT_NUMBER_NO_ENTRY_m13				SEGMENT_NUMBER_NO_ENTRY_m13
#define UH_SEGMENT_LEVEL_CODE_m13				-1
#define UH_CHANNEL_LEVEL_CODE_m13				-2
#define UH_SESSION_LEVEL_CODE_m13				-3
#define UH_TYPE_STR_OFFSET_m13				32 // ascii[4]
#define UH_TYPE_STR_TERMINAL_ZERO_OFFSET_m13			(UH_TYPE_STR_OFFSET_m13 + 4) // si1
#define UH_TYPE_CODE_OFFSET_m13					UH_TYPE_STR_OFFSET_m13 // ui4
#define UH_TYPE_NO_ENTRY_m13					0 // zero as ui4 or zero-length string as ascii[4]
#define UH_MED_VERSION_MAJOR_OFFSET_m13				37 // ui1
#define UH_MED_VERSION_MAJOR_NO_ENTRY_m13			MED_VERSION_NO_ENTRY_m13
#define UH_MED_VERSION_MINOR_OFFSET_m13				38 // ui1
#define UH_MED_VERSION_MINOR_NO_ENTRY_m13			MED_VERSION_NO_ENTRY_m13
#define UH_BYTE_ORDER_CODE_OFFSET_m13				39 // ui1
#define UH_BYTE_ORDER_CODE_NO_ENTRY_m13				0xFF
#define UH_SESSION_START_TIME_OFFSET_m13			40 // si8
#define UH_FILE_START_TIME_OFFSET_m13				48 // si8
#define UH_SESSION_NAME_OFFSET_m13				56 // utf8[63]
#define UH_CHANNEL_NAME_OFFSET_m13				312 // utf8[63]
#define UH_SUPPLEMENTARY_PROTECTED_REGION_OFFSET_m13		568 // Anonymized Subject ID in MED 1.0
#define UH_SUPPLEMENTARY_PROTECTED_REGION_BYTES_m13		256
#define UH_SESSION_UID_OFFSET_m13				824 // ui8
#define UH_CHANNEL_UID_OFFSET_m13				832 // ui8
#define UH_SEGMENT_UID_OFFSET_m13				840 // ui8
#define UH_FILE_UID_OFFSET_m13					848 // ui8
#define UH_PROVENANCE_UID_OFFSET_m13				856 // ui8
#define UH_LEVEL_1_PASSWORD_VALIDATION_FIELD_OFFSET_m13		864 // ui1
#define UH_LEVEL_2_PASSWORD_VALIDATION_FIELD_OFFSET_m13		880 // ui1
#define UH_LEVEL_3_PASSWORD_VALIDATION_FIELD_OFFSET_m13		896 // ui1
#define UH_VIDEO_DATA_FILE_NUMBER_OFFSET_m13 			912 // ui4, MED 1.1 & above
#define UH_LIVE_OFFSET_m13					916 // tern, MED 1.1 & above
#define UH_ORDERED_OFFSET_m13					917 // tern, MED 1.1 & above
#define UH_EXPANDED_PASSWORDS_OFFSET_m13			918 // tern, MED 1.1 & above
#define UH_ENCRYPTION_ROUNDS_OFFSET_m13				919 // ui1, MED 1.1 & above
#define UH_ENCRYPTION_1_OFFSET_m13				920 // si1, MED 1.1 & above
#define UH_ENCRYPTION_2_OFFSET_m13				921 // si1, MED 1.1 & above
#define UH_ENCRYPTION_3_OFFSET_m13				922 // si1, MED 1.1 & above
#define UH_PROTECTED_REGION_OFFSET_m13				923
#define UH_PROTECTED_REGION_BYTES_m13				53
#define UH_DISCRETIONARY_REGION_OFFSET_m13			976
#define UH_DISCRETIONARY_REGION_BYTES_m13			48

// defaults
#define UH_DATA_ENCRYPTION_DEFAULT_m13				NO_ENCRYPTION_m13 // si1, MED 1.1 & above
#define UH_METADATA_SECTION_2_ENCRYPTION_DEFAULT_m13		LEVEL_1_ENCRYPTION_m13 // si1, MED 1.1 & above
#define UH_METADATA_SECTION_3_ENCRYPTION_DEFAULT_m13		LEVEL_2_ENCRYPTION_m13 // si1, MED 1.1 & above
#define UH_EXPANDED_PASSWORDS_DEFAULT_m13			TRUE_m13
#define UH_ENCRYPTION_ROUNDS_DEFAULT_m13			((ui1) 1)

// Metadata: File Format Constants
#define METADATA_BYTES_m13			15360 // 15 KiB
#define FPS_PROTOTYPE_BYTES_m13			METADATA_BYTES_m13
#define METADATA_FILE_BYTES_m13			(METADATA_BYTES_m13 + UH_BYTES_m13) // 16 KiB
#define METADATA_SECTION_1_OFFSET_m13		1024
#define METADATA_SECTION_1_BYTES_m13		1024 // 1 KiB
#define METADATA_SECTION_2_OFFSET_m13		2048
#define METADATA_SECTION_2_BYTES_m13		10240 // 10 KiB
#define METADATA_SECTION_3_OFFSET_m13		12288
#define METADATA_SECTION_3_BYTES_m13		4096 // 4 KiB

// Metadata: File Format Constants - Section 1 Fields
#define METADATA_LEVEL_1_PASSWORD_HINT_OFFSET_m13		1024 // utf8[63]
#define METADATA_LEVEL_1_PASSWORD_HINT_BYTES_m13		PASSWORD_HINT_BYTES_m13
#define METADATA_LEVEL_2_PASSWORD_HINT_OFFSET_m13		1280 // utf8[63]
#define METADATA_LEVEL_2_PASSWORD_HINT_BYTES_m13		PASSWORD_HINT_BYTES_m13
#define METADATA_ANONYMIZED_SUBJECT_ID_OFFSET_m13		1536 // MED 1.1 & above
#define METADATA_ANONYMIZED_SUBJECT_ID_BYTES_m13		256 // utf8[63]
#define METADATA_SECTION_1_PROTECTED_REGION_OFFSET_m13		1792
#define METADATA_SECTION_1_PROTECTED_REGION_BYTES_m13		128
#define METADATA_SECTION_1_DISCRETIONARY_REGION_OFFSET_m13	1920
#define METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m13	128

// Metadata: File Format Constants - Section 2 Channel Type Independent Fields
#define METADATA_SESSION_DESCRIPTION_OFFSET_m13			2048 // utf8[511]
#define METADATA_SESSION_DESCRIPTION_BYTES_m13			2048
#define METADATA_CHANNEL_DESCRIPTION_OFFSET_m13			4096 // utf8[255]
#define METADATA_CHANNEL_DESCRIPTION_BYTES_m13			1024
#define METADATA_SEGMENT_DESCRIPTION_OFFSET_m13			5120 // utf8[255]
#define METADATA_SEGMENT_DESCRIPTION_BYTES_m13			1024
#define METADATA_EQUIPMENT_DESCRIPTION_OFFSET_m13		6144 // utf8[510]
#define METADATA_EQUIPMENT_DESCRIPTION_BYTES_m13		2044
#define METADATA_ACQUISITION_CHANNEL_NUMBER_OFFSET_m13		8188 // si4
#define METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m13	CHANNEL_NUMBER_NO_ENTRY_m13

// Metadata: File Format Constants - Time Series Section 2 Fields
#define TS_METADATA_REFERENCE_DESCRIPTION_OFFSET_m13			8192 // utf8[255]
#define TS_METADATA_REFERENCE_DESCRIPTION_BYTES_m13			1024
#define TS_METADATA_SAMPLING_FREQUENCY_OFFSET_m13			9216 // sf8
#define TS_METADATA_FREQUENCY_NO_ENTRY_m13				FREQUENCY_NO_ENTRY_m13
#define TS_METADATA_FREQUENCY_VARIABLE_m13				RATE_VARIABLE_m13
#define TS_METADATA_LOW_FREQUENCY_FILTER_SETTING_OFFSET_m13		9224 // sf8
#define TS_METADATA_HIGH_FREQUENCY_FILTER_SETTING_OFFSET_m13		9232 // sf8
#define TS_METADATA_NOTCH_FILTER_FREQUENCY_SETTING_OFFSET_m13		9240 // sf8
#define TS_METADATA_AC_LINE_FREQUENCY_OFFSET_m13			9248 // sf8
#define TS_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_OFFSET_m13	9256 // sf8
#define TS_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m13	0.0
#define TS_METADATA_AMPLITUDE_UNITS_DESCRIPTION_OFFSET_m13		9264 // utf8[31]
#define TS_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m13		128
#define TS_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m13	9392 // sf8
#define TS_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m13	0.0
#define TS_METADATA_TIME_BASE_UNITS_DESCRIPTION_OFFSET_m13		9400 // utf8[31]
#define TS_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m13		128
#define TS_METADATA_SESSION_START_SAMPLE_NUMBER_OFFSET_m13		9528
#define TS_METADATA_SESSION_START_SAMPLE_NUMBER_NO_ENTRY_m13		SAMPLE_NUMBER_NO_ENTRY_m13
#define TS_METADATA_NUMBER_OF_SAMPLES_OFFSET_m13			9536 // si8
#define TS_METADATA_NUMBER_OF_SAMPLES_NO_ENTRY_m13			NUMBER_OF_SAMPLES_NO_ENTRY_m13
#define TS_METADATA_NUMBER_OF_BLOCKS_OFFSET_m13				9544 // si8
#define TS_METADATA_NUMBER_OF_BLOCKS_NO_ENTRY_m13			-1
#define TS_METADATA_MAXIMUM_BLOCK_BYTES_OFFSET_m13			9552 // si8
#define TS_METADATA_MAXIMUM_BLOCK_BYTES_NO_ENTRY_m13			-1
#define TS_METADATA_MAXIMUM_BLOCK_SAMPLES_OFFSET_m13			9560 // ui4
#define TS_METADATA_MAXIMUM_BLOCK_SAMPLES_NO_ENTRY_m13			0xFFFFFFFF
#define TS_METADATA_MAXIMUM_BLOCK_KEYSAMPLE_BYTES_OFFSET_m13		9564 // ui4
#define TS_METADATA_MAXIMUM_BLOCK_KEYSAMPLE_BYTES_NO_ENTRY_m13 		0xFFFFFFFF
#define TS_METADATA_MAXIMUM_BLOCK_DURATION_OFFSET_m13			9568 // sf8
#define TS_METADATA_MAXIMUM_BLOCK_DURATION_NO_ENTRY_m13			-1.0
#define TS_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m13		9576 // si8
#define TS_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m13		-1
#define TS_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_OFFSET_m13		9584 // si8
#define TS_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_NO_ENTRY_m13		-1
#define TS_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_OFFSET_m13		9592 // si8
#define TS_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_NO_ENTRY_m13		-1
#define TS_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_OFFSET_m13		9600 // si8
#define TS_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_NO_ENTRY_m13		-1
#define TS_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m13		9608
#define TS_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m13		1344
#define TS_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m13		10952
#define TS_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m13		1336

// Metadata: File Format Constants - Video Section 2 Fields
#define VID_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m13	8192  // sf8
#define VID_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m13	0.0
#define VID_METADATA_TIME_BASE_UNITS_DESCRIPTION_OFFSET_m13		8200  // utf8[31]
#define VID_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m13		128
#define VID_METADATA_SESSION_START_FRAME_NUMBER_OFFSET_m13		8328
#define VID_METADATA_SESSION_START_FRAME_NUMBER_NO_ENTRY_m13		FRAME_NUMBER_NO_ENTRY_m13
#define VID_METADATA_NUMBER_OF_FRAMES_OFFSET_m13			8336  // si8
#define VID_METADATA_NUMBER_OF_FRAMES_NO_ENTRY_m13			NUMBER_OF_FRAMES_NO_ENTRY_m13
#define VID_METADATA_FRAME_RATE_OFFSET_m13				8344  // sf8
#define VID_METADATA_FRAME_RATE_NO_ENTRY_m13				FRAME_RATE_NO_ENTRY_m13
#define VID_METADATA_FRAME_RATE_VARIABLE_m13				FRAME_RATE_VARIABLE_m13
#define VID_METADATA_NUMBER_OF_CLIPS_OFFSET_m13				8352 // si8
#define VID_METADATA_NUMBER_OF_CLIPS_NO_ENTRY_m13			-1
#define VID_METADATA_MAXIMUM_CLIP_BYTES_OFFSET_m13			8360 // si8
#define VID_METADATA_MAXIMUM_CLIP_BYTES_NO_ENTRY_m13			-1
#define VID_METADATA_MAXIMUM_CLIP_FRAMES_OFFSET_m13			8368 // ui4
#define VID_METADATA_MAXIMUM_CLIP_FRAMES_NO_ENTRY_m13			0xFFFFFFFF
#define VID_METADATA_NUMBER_OF_VIDEO_FILES_OFFSET_m13			8372  // si4
#define VID_METADATA_NUMBER_OF_VIDEO_FILES_NO_ENTRY_m13			-1
#define VID_METADATA_MAXIMUM_CLIP_DURATION_OFFSET_m13			8376 // sf8
#define VID_METADATA_MAXIMUM_CLIP_DURATION_NO_ENTRY_m13			-1.0
#define VID_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m13		8384 // si8
#define VID_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m13		-1
#define VID_METADATA_MAXIMUM_CONTIGUOUS_CLIPS_OFFSET_m13		8392 // si8
#define VID_METADATA_MAXIMUM_CONTIGUOUS_CLIPS_NO_ENTRY_m13		-1
#define VID_METADATA_MAXIMUM_CONTIGUOUS_CLIP_BYTES_OFFSET_m13		8400 // si8
#define VID_METADATA_MAXIMUM_CONTIGUOUS_CLIP_BYTES_NO_ENTRY_m13		-1
#define VID_METADATA_MAXIMUM_CONTIGUOUS_FRAMES_OFFSET_m13		8408 // si8
#define VID_METADATA_MAXIMUM_CONTIGUOUS_FRAMES_NO_ENTRY_m13		-1
#define VID_METADATA_HORIZONTAL_PIXELS_OFFSET_m13			8416  // ui4
#define VID_METADATA_HORIZONTAL_PIXELS_NO_ENTRY_m13			0
#define VID_METADATA_VERTICAL_PIXELS_OFFSET_m13				8420  // ui4
#define VID_METADATA_VERTICAL_PIXELS_NO_ENTRY_m13			0
#define VID_METADATA_VIDEO_FORMAT_OFFSET_m13				8424  // utf8[63]
#define VID_METADATA_VIDEO_FORMAT_BYTES_m13				256
#define VID_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m13		8680
#define VID_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m13		1808
#define VID_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m13		10488
#define VID_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m13		1800

// Metadata: File Format Constants - Section 3 Fields
#define METADATA_RECORDING_TIME_OFFSET_OFFSET_m13		12288 // si8
#define METADATA_RECORDING_TIME_OFFSET_NO_ENTRY_m13		GLOBALS_RECORDING_TIME_OFFSET_NO_ENTRY_m13
#define METADATA_DAYLIGHT_TIME_START_CODE_OFFSET_m13		12296 // DAYLIGHT_TIME_CHANGE_CODE_m13 (si1[8])
#define METADATA_DAYLIGHT_TIME_START_CODE_NO_ENTRY_m13		DTCC_VALUE_NO_ENTRY_m13
#define METADATA_DAYLIGHT_TIME_END_CODE_OFFSET_m13		12304 // DAYLIGHT_TIME_CHANGE_CODE_m13 (si1[8])
#define METADATA_DAYLIGHT_TIME_END_CODE_NO_ENTRY_m13		DTCC_VALUE_NO_ENTRY_m13
#define METADATA_STANDARD_TIMEZONE_ACRONYM_OFFSET_m13		12312 // ascii[7]
#define METADATA_STANDARD_TIMEZONE_ACRONYM_BYTES_m13		TIMEZONE_ACRONYM_BYTES_m13
#define METADATA_STANDARD_TIMEZONE_STRING_OFFSET_m13		12320 // ascii[63]
#define METADATA_STANDARD_TIMEZONE_STRING_BYTES_m13		TIMEZONE_STRING_BYTES_m13
#define METADATA_DAYLIGHT_TIMEZONE_ACRONYM_OFFSET_m13		12384 // ascii[7]
#define METADATA_DAYLIGHT_TIMEZONE_ACRONYM_BYTES_m13		TIMEZONE_ACRONYM_BYTES_m13
#define METADATA_DAYLIGHT_TIMEZONE_STRING_OFFSET_m13		12392 // ascii[63]
#define METADATA_DAYLIGHT_TIMEZONE_STRING_BYTES_m13		TIMEZONE_STRING_BYTES_m13
#define METADATA_SUBJECT_NAME_1_OFFSET_m13			12456 // utf8[31]
#define METADATA_SUBJECT_NAME_BYTES_m13				128
#define METADATA_SUBJECT_NAME_2_OFFSET_m13			12584 // utf8[31]
#define METADATA_SUBJECT_NAME_3_OFFSET_m13			12712 // utf8[31]
#define METADATA_SUBJECT_ID_OFFSET_m13				12840 // utf8[31]
#define METADATA_SUBJECT_ID_BYTES_m13				128
#define METADATA_RECORDING_COUNTRY_OFFSET_m13			12968 // utf8[63]
#define METADATA_RECORDING_TERRITORY_OFFSET_m13			13224 // utf8[63]
#define METADATA_RECORDING_LOCALITY_OFFSET_m13			13480 // utf8[63]
#define METADATA_RECORDING_INSTITUTION_OFFSET_m13		13736 // utf8[63]
#define METADATA_RECORDING_LOCATION_BYTES_m13			256
#define METADATA_GEOTAG_FORMAT_OFFSET_m13			13992 // ascii[31]
#define METADATA_GEOTAG_FORMAT_BYTES_m13			32
#define METADATA_GEOTAG_DATA_OFFSET_m13				14024 // ascii[1023]
#define METADATA_GEOTAG_DATA_BYTES_m13				1024
#define METADATA_STANDARD_UTC_OFFSET_OFFSET_m13			15048 // si4
#define METADATA_STANDARD_UTC_OFFSET_NO_ENTRY_m13		STANDARD_UTC_OFFSET_NO_ENTRY_m13
#define METADATA_SECTION_3_PROTECTED_REGION_OFFSET_m13		15052
#define METADATA_SECTION_3_PROTECTED_REGION_BYTES_m13		668
#define METADATA_SECTION_3_DISCRETIONARY_REGION_OFFSET_m13	15720
#define METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m13	664

// Records: Header Format Constants
#define REC_HDR_BYTES_m13					24
#define REC_HDR_CRC_OFFSET_m13				0 // ui4
#define REC_HDR_CRC_NO_ENTRY_m13			CRC_NO_ENTRY_m13
#define REC_HDR_TOTAL_RECORD_BYTES_OFFSET_m13		4 // ui4
#define REC_HDR_TOTAL_RECORD_BYTES_NO_ENTRY_m13		0
#define REC_HDR_CRC_START_OFFSET_m13			REC_HDR_TOTAL_RECORD_BYTES_OFFSET_m13
#define REC_HDR_START_TIME_OFFSET_m13			8 // si8
#define REC_HDR_START_TIME_NO_ENTRY_m13			UUTC_NO_ENTRY_m13 // si8
#define REC_HDR_TYPE_STR_OFFSET_m13			16	 // ascii[4]
#define REC_HDR_TYPE_STR_TERMINAL_ZERO_OFFSET_m13	(REC_HDR_TYPE_STR_OFFSET_m13 + 4) // si1
#define REC_HDR_TYPE_CODE_OFFSET_m13			REC_HDR_TYPE_STR_OFFSET_m13  // ui4
#define REC_HDR_TYPE_CODE_NO_ENTRY_m13			0	 // ui4
#define REC_HDR_VERSION_MAJOR_OFFSET_m13		21	 // ui1
#define REC_HDR_VERSION_MAJOR_NO_ENTRY_m13		0xFF
#define REC_HDR_VERSION_MINOR_OFFSET_m13		22	 // ui1
#define REC_HDR_VERSION_MINOR_NO_ENTRY_m13		0xFF
#define REC_HDR_ENCRYPTION_LEVEL_OFFSET_m13		23	 // si1
#define REC_HDR_ENCRYPTION_LEVEL_NO_ENTRY_m13		ENCRYPTION_LEVEL_NO_ENTRY_m13

// Record Index: Format Constants
#define REC_IDX_BYTES_m13				INDEX_BYTES_m13
#define REC_IDX_FILE_OFFSET_OFFSET_m13			0 // si8
#define REC_IDX_FILE_OFFSET_NO_ENTRY_m13		-1
#define REC_IDX_START_TIME_OFFSET_m13			8 // si8
#define REC_IDX_START_TIME_NO_ENTRY_m13			UUTC_NO_ENTRY_m13
#define REC_IDX_TYPE_STR_OFFSET_m13			16 // ascii[4]
#define REC_IDX_TYPE_STR_TERMINAL_ZERO_OFFSET_m13	(REC_IDX_TYPE_STR_OFFSET_m13 + 4) // si1
#define REC_IDX_TYPE_CODE_OFFSET_m13			REC_IDX_TYPE_STR_OFFSET_m13  // as ui4
#define REC_IDX_TYPE_CODE_NO_ENTRY_m13			0 // as ui4
#define REC_IDX_VERSION_MAJOR_OFFSET_m13		21 // ui1
#define REC_IDX_VERSION_MAJOR_NO_ENTRY_m13		0xFF
#define REC_IDX_VERSION_MINOR_OFFSET_m13		22 // ui1
#define REC_IDX_VERSION_MINOR_NO_ENTRY_m13		0xFF
#define REC_IDX_ENCRYPTION_LEVEL_OFFSET_m13		23 // si1
#define REC_IDX_ENCRYPTION_LEVEL_NO_ENTRY_m13		ENCRYPTION_LEVEL_NO_ENTRY_m13

// Time Series Index: Format Constants
#define TS_IDX_BYTES_m13				INDEX_BYTES_m13
#define TS_IDX_FILE_OFFSET_OFFSET_m13			0 // si8
#define TS_IDX_FILE_OFFSET_NO_ENTRY_m13			-1
#define TS_IDX_START_TIME_OFFSET_m13			 8 // si8
#define TS_IDX_START_TIME_NO_ENTRY_m13			UUTC_NO_ENTRY_m13
#define TS_IDX_START_SAMPLE_NUMBER_OFFSET_m13		16 // si8
#define TS_IDX_START_SAMPLE_NUMBER_NO_ENTRY_m13		-1

// Video Index: Format Constants
#define VID_IDX_BYTES_m13				INDEX_BYTES_m13
#define VID_IDX_FILE_OFFSET_OFFSET_m13			0 // si8
#define VID_IDX_FILE_OFFSET_NO_ENTRY_m13		-1
#define VID_IDX_START_TIME_OFFSET_m13			8 // si8
#define VID_IDX_START_TIME_NO_ENTRY_m13			UUTC_NO_ENTRY_m13
#define VID_IDX_START_FRAME_OFFSET_m13			16 // ui4
#define VID_IDX_START_FRAME_NO_ENTRY_m13		0xFFFFFFFF
#define VID_IDX_VIDEO_FILE_NUMBER_OFFSET_m13		20 // ui4
#define VID_IDX_VID_FILE_NUMBER_NO_ENTRY_m13		0
#define VID_IDX_TERMINAL_VID_FILE_NUMBER_m13		0xFFFFFFFF

// Prior MED version offsets
#define MED_10_METADATA_SECTION_2_ENCRYPTION_LEVEL_OFFSET_m13		1536 // si1
#define MED_10_METADATA_SECTION_3_ENCRYPTION_LEVEL_OFFSET_m13		1537 // si1
#define MED_10_METADATA_TS_DATA_ENCRYPTION_LEVEL_OFFSET_m13		1538 // si1
#define MED_10_UH_ANONYMIZED_SUBJECT_ID_OFFSET_m13			568 // utf8[63]
#define MED_10_CMP_BF_LEVEL_1_ENCRYPTION_BIT_m13			((ui4) 1 << 4)
#define MED_10_CMP_BF_LEVEL_2_ENCRYPTION_BIT_m13			((ui4) 1 << 4)

// Level Header (LH) Flags Definitions:

// level header flags
// READ == on open: open file & read universal header (applies to data files, index files are always read in full)
// on read: set FPS pointer to section specified by time slice (decrpyting if necessary)
// READ_FULL == on open: read full file (no memory mapping required, & closing)
// MMAP == allocate memory for full file, but only read on demand, (no re-reading occurs, but potentially memory expensive, good paired with VDS)
// ACTIVE == applies only to channels. Mark a CHANNEL as active to return data. Marking a channel as inactive does not free or close it.
// EPHEMERAL DATA == if GENERATE_EPHEMERAL_DATA_m13 is set, ephemeral data is created if it does not exist.
//	If UPDATE_EPHEMERAL_DATA is set, the data is updated whenever the channel or segment open set changes (opening of new elements, not the active status)
//	The UPDATE_EPHEMERAL_DATA bit is set by the lower levels and reset by the higher level once the data has been updated.
//	i.e read_channel_m13() checks the segment bits (e.g. read_segment_m13() opened a new segment) & if update required, it does the channel level update & clears the segment bit.
//	It then sets it's bit to trigger update at the session level. After updating, the session will clear the channel level bit.

// all levels
#define LH_NO_FLAGS_SET_m13			NO_FLAGS_m13
#define LH_FLAGS_SET_m13			((ui8) 1 << 0) // flags have been set (distinguishes empty flags from flags not set)
#define LH_GENERATE_EPHEMERAL_DATA_m13		((ui8) 1 << 1) // implies all level involvement
#define LH_UPDATE_EPHEMERAL_DATA_m13		((ui8) 1 << 2) // signal to higher level from lower level (reset by higher level after update)

// session level
#define LH_EXCLUDE_TS_CHANS_m13			((ui8) 1 << 8) // useful when session directory passed, but don't want time series channels
#define LH_EXCLUDE_VID_CHANS_m13		((ui8) 1 << 9) // useful when session directory passed, but don't want video channels
#define LH_MAP_ALL_TS_CHANS_m13			((ui8) 1 << 10) // useful when time series channels may be added to open session
#define LH_MAP_ALL_VID_CHANS_m13		((ui8) 1 << 11) // useful when video channels may be added to open session

#define LH_READ_SLICE_SESS_RECS_m13		((ui8) 1 << 16) // read full record indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_SESS_RECS_m13		((ui8) 1 << 17) // read full recordindices & data files, close all files
#define LH_MMAP_SESS_RECS_m13			((ui8) 1 << 18) // allocate, but don't read full file

// segmented session records level
#define LH_READ_SLICE_SEG_SESS_RECS_m13		((ui8) 1 << 24) // read full indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_SEG_SESS_RECS_m13		((ui8) 1 << 25) // read full indices file & data files, close all files
#define LH_MMAP_SEG_SESS_RECS_m13		((ui8) 1 << 26) // allocate, but don't read full data file

// channel level
#define LH_CHAN_ACTIVE_m13			((ui8) 1 << 32) // include channel in current read set
#define LH_IDX_CHAN_INACTIVE_m13		((ui8) 1 << 33)
#define LH_MAP_ALL_SEGS_m13			((ui8) 1 << 34) // allocate slots for every segment, regardless of whether required for current read
// (active channels only)
#define LH_READ_SLICE_CHAN_RECS_m13		((ui8) 1 << 40) // read full record indices file (close file); open record data, read universal header, leave open
#define LH_READ_FULL_CHAN_RECS_m13		((ui8) 1 << 41) // read full record indices & data files, close all files
#define LH_MMAP_CHAN_RECS_m13			((ui8) 1 << 42) // allocate, but don't read full file
#define LH_THREAD_SEG_READS_m13			((ui8) 1 << 43) // set if likely to cross many segment boundaries in read (e.g. one channel, long reads or short segments)

// segment level
#define LH_NO_CPS_PTR_RESET_m13			((ui8) 1 << 48) // caller will update pointers
#define LH_NO_CPS_CACHING_m13			((ui8) 1 << 49) // set cps_caching parameter to FALSE
// (active channels only)
#define LH_READ_SLICE_SEG_DATA_m13		((ui8) 1 << 56) // read full metadata & indices files, close files; open data, read universal header, leave open
#define LH_READ_FULL_SEG_DATA_m13		((ui8) 1 << 57) // read full metadata, indices, & data files, close all files
#define LH_MMAP_SEG_DATA_m13			((ui8) 1 << 58) // allocate, but don't read full file
#define LH_READ_SLICE_SEG_RECS_m13		((ui8) 1 << 59) // read full indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_SEG_RECS_m13		((ui8) 1 << 60) // read full indices file & data files, close all files
#define LH_MMAP_SEG_RECS_m13			((ui8) 1 << 61) // allocate, but don't read full file
#define LH_READ_SEG_METADATA_m13		((ui8) 1 << 62) // read segment metadata

// flag groups
#define LH_MAP_ALL_CHANNELS_m13 	 	( LH_MAP_ALL_TS_CHANS_m13 | LH_MAP_ALL_VID_CHANS_m13 )

// reading masks (not to be used as flags: SLICE/FULL mutually exclusive)
#define LH_READ_SESS_RECS_MASK_m13 		( LH_READ_SLICE_SESS_RECS_m13 | LH_READ_FULL_SESS_RECS_m13 )
#define LH_READ_SEG_SESS_RECS_MASK_m13 		( LH_READ_SLICE_SEG_SESS_RECS_m13 | LH_READ_FULL_SEG_SESS_RECS_m13 )
#define LH_READ_CHAN_RECS_MASK_m13 		( LH_READ_SLICE_CHAN_RECS_m13 | LH_READ_FULL_CHAN_RECS_m13 )
#define LH_READ_SEG_RECS_MASK_m13		( LH_READ_SLICE_SEG_RECS_m13 | LH_READ_FULL_SEG_RECS_m13 )
#define LH_READ_SEG_DATA_MASK_m13		( LH_READ_SLICE_SEG_DATA_m13 | LH_READ_FULL_SEG_DATA_m13 )
#define LH_READ_RECS_MASK_m13			( LH_READ_SESS_RECS_MASK_m13 | LH_READ_SEG_SESS_RECS_MASK_m13 | LH_READ_CHAN_RECS_MASK_m13 | LH_READ_SEG_RECS_MASK_m13 )
#define LH_ALL_READ_FLAGS_MASK_m13		( LH_READ_RECS_MASK_m13 | LH_READ_SEG_DATA_MASK_m13 )
#define LH_READ_FULL_RECS_MASK_m13		( LH_READ_FULL_SESS_RECS_m13 | LH_READ_FULL_SEG_SESS_REC_m13 | LH_READ_FULL_CHAN_RECS_m13 | LH_READ_FULL_SEG_RECS_m13 )
#define LH_READ_FULL_FILES_MASK_m13		( LH_READ_FULL_RECORDS_MASK_m13 | LH_READ_FULL_SEGMENT_DATA_m13 )
#define LH_READ_METADATA_MASK_m13		( LH_READ_SEG_DATA_MASK_m13 | LH_READ_SEG_METADATA_m13 | LH_GENERATE_EPHEMERAL_DATA_m13 )
// memory map flags & masks
#define LH_MMAP_ALL_RECS_m13			( LH_MMAP_SESS_RECS_m13 | LH_MMAP_SEG_SESS_RECS_m13 | LH_MMAP_CHAN_RECS_m13 | LH_MMAP_SEG_RECS_m13 )
#define LH_MMAP_ALL_m13				( LH_MMAP_ALL_RECS_m13 | LH_MMAP_SEG_DATA_m13 )
#define LH_ALL_MMAP_FLAGS_MASK_m13		LH_MMAP_ALL_m13

// record reading groups
#define LH_READ_SLICE_ALL_RECS_m13		( LH_READ_SLICE_SESS_RECS_m13 | LH_READ_SLICE_SEG_SESS_RECS_m13 | LH_READ_SLICE_CHAN_RECS_m13 | LH_READ_SLICE_SEG_RECS_m13 )
#define LH_READ_FULL_ALL_RECS_m13		( LH_READ_FULL_SESS_RECS_m13 | LH_READ_FULL_SEG_SESS_RECS_m13 | LH_READ_FULL_CHAN_RECS_m13 | LH_READ_FULL_SEG_RECS_m13 )



//**********************************************************************************//
//************************** File Functions (FILE & FLOCK) *************************//
//**********************************************************************************//

// FILE_m13 is an enhanced FILE pointer for use in medlib functions
// Note: functions requiring FILE pointers accept either, FILE_m13 * or FILE * wherevere appropriate
// These arguments are declared as void pointers use of either without casting or compiler warnings


// Defines
#define FILE_TAG_m13			((ui4) 0x87654321) // ui4 (decimal 2,271,560,481)

#define FILE_PERM_OTH_EXEC_m13		((ui2) 1 << 0) // == S_IXOTH
#define FILE_PERM_OTH_WRITE_m13		((ui2) 1 << 1) // == S_IWOTH
#define FILE_PERM_OTH_READ_m13		((ui2) 1 << 2) // == S_IROTH
#define FILE_PERM_GRP_EXEC_m13		((ui2) 1 << 3) // == S_IXGRP
#define FILE_PERM_GRP_WRITE_m13		((ui2) 1 << 4) // == S_IWGRP
#define FILE_PERM_GRP_READ_m13		((ui2) 1 << 5) // == S_IRGRP
#define FILE_PERM_USR_EXEC_m13		((ui2) 1 << 6) // == S_IXUSR
#define FILE_PERM_USR_WRITE_m13		((ui2) 1 << 7) // == S_IWUSR
#define FILE_PERM_USR_READ_m13		((ui2) 1 << 8) // == S_IRUSR
#define FILE_PERM_STAT_MASK_m13		((ui2) 0x01FF) // lower 9 bits of stat member st_mode (ui2)

// Convenience
#define FILE_PERM_NONE_m13		0
#define FILE_PERM_UG_R_m13		( FILE_PERM_USR_READ_m13 | FILE_PERM_GRP_READ_m13 )
#define FILE_PERM_UG_W_m13		( FILE_PERM_USR_WRITE_m13 | FILE_PERM_GRP_WRITE_m13 )
#define FILE_PERM_UG_RW_m13		( FILE_PERM_UG_R_m13 | FILE_PERM_UG_W_m13 )
#define FILE_PERM_UGO_R_m13		( FILE_PERM_UG_R_m13 | FILE_PERM_OTH_READ_m13 )
#define FILE_PERM_UGO_W_m13		( FILE_PERM_UG_W_m13 | FILE_PERM_OTH_WRITE_m13 )
#define FILE_PERM_UGO_RW_m13		( FILE_PERM_UGO_R_m13 | FILE_PERM_UGO_W_m13 )
#if defined MACOS_m13 || defined LINUX_m13
	#define FILE_PERM_DEFAULT_m13		FILE_PERM_UG_RW_m13 // rw- rw- ---
#endif
#ifdef WINDOWS_m13
	#define FILE_PERM_DEFAULT_m13		FILE_PERM_UGO_RW_m13 // rw- rw- rw-
	#define WN_PERM_MODE_DEFAULT_m13	( (si4) (_S_IREAD | _S_IWRITE) ) // read & write for all (no "other" in Windows); does not replace FILE_PERM_DEFAULT_m13, but should match it
#endif

#define FILE_FLAGS_NONE_m13		((ui2) 0)
#define FILE_FLAGS_ALLOCED_m13		((ui2) 1 << 0) // file structure was allocated
#define FILE_FLAGS_LOCK_m13		((ui2) 1 << 1) // file is subject to locking
#define FILE_FLAGS_STD_STREAM_m13	((ui2) 1 << 2) // file is a standard stream (stdin, stdout, stderr converted to FILE_m13)
#define FILE_FLAGS_MED_m13		((ui2) 1 << 3) // file is a MED file
#define FILE_FLAGS_PARITY_m13		((ui2) 1 << 4) // file is a parity file
#define FILE_FLAGS_READ_m13		((ui2) 1 << 5) // file is open for reading
#define FILE_FLAGS_WRITE_m13		((ui2) 1 << 6) // file is open for writing
#define FILE_FLAGS_APPEND_m13		((ui2) 1 << 7) // file is open in append mode (all writes will append regardless of fp; "append" is treated as a modifier of "write" mode)
#define FILE_FLAGS_LEN_m13		((ui2) 1 << 8) // update len with each operation
#define FILE_FLAGS_POS_m13		((ui2) 1 << 9) // update pos with each operation
#define FILE_FLAGS_TIME_m13		((ui2) 1 << 10) // update access time with each operation (global sets flag here, but flag supersedes global in execution)
#define FILE_FLAGS_DEFAULT_m13		( FILE_FLAGS_LEN_m13 | FILE_FLAGS_POS_m13 )
#define FILE_FLAGS_MODE_MASK_m13	( FILE_FLAGS_READ_m13 | FILE_FLAGS_WRITE_m13 | FILE_FLAGS_APPEND_m13 )

#define FILE_FD_EPHEMERAL_m13		((si4) -2)
#define FILE_FD_CLOSED_m13		((si4) -1)
#define FILE_FD_STDIN_m13		((si4) 0)
#define FILE_FD_STDOUT_m13		((si4) 1)
#define FILE_FD_STDERR_m13		((si4) 2)

// Locking operations (si4 to match standard flock)
// NOTE: locking only possible with FILE_m13 file pointers
#define FLOCK_OPEN_m13			((si4) 1 << 0) // increment open count +/- create lock
#define FLOCK_CLOSE_m13			((si4) 1 << 1) // decrement open count +/- destroy lock
#define FLOCK_LOCK_m13			((si4) 1 << 2) // lock (with mode)
#define FLOCK_UNLOCK_m13		((si4) 1 << 3) // unlock (with mode)
#define FLOCK_READ_m13			((si4) 1 << 4) // read lock mode
#define FLOCK_WRITE_m13			((si4) 1 << 5) // write lock mode
#define FLOCK_NON_BLOCKING_m13		((si4) 1 << 6) // do not block for lock, return FLOCK_LOCKED_m13 immediately
#define FLOCK_TIMEOUT_m13		((si4) 1 << 7) // blocking (looped) or non-blocking (once) sleep time (as nap string) included as vararg

// Return values
#define FLOCK_SUCCESS_m13		((si4) 0) // locking operation succeeded
#define FLOCK_ERR_m13			((si4) -1) // locking operation generated error
#define FLOCK_LOCKED_m13		((si4) -2) // can't unlock due to ownership, or still locked in non-blocking mode

// Convenience
#define FLOCK_READ_LOCK_m13		( FLOCK_LOCK_m13 | FLOCK_READ_m13 )
#define FLOCK_WRITE_LOCK_m13		( FLOCK_LOCK_m13 | FLOCK_WRITE_m13 )
#define FLOCK_READ_LOCK_NB_m13		( FLOCK_LOCK_m13 | FLOCK_READ_m13 | FLOCK_NON_BLOCKING_m13 )
#define FLOCK_WRITE_LOCK_NB_m13		( FLOCK_LOCK_m13 | FLOCK_WRITE_m13 | FLOCK_NON_BLOCKING_m13 )
#define FLOCK_READ_UNLOCK_m13		( FLOCK_UNLOCK_m13 | FLOCK_READ_m13 )
#define FLOCK_WRITE_UNLOCK_m13		( FLOCK_UNLOCK_m13 | FLOCK_WRITE_m13 )

// global default locking modes
// NOTE: individual files can be locked by setting FLOCK_LOCK_m13 bit regardless global mode
#define FLOCK_MODE_NONE_m13		((si1) 0) // do not lock any files
#define FLOCK_MODE_MED_m13		((si1) 1) // lock MED files only
#define FLOCK_MODE_ALL_m13		((si1) 2) // lock all files

// Standard Streams
#define stdin_m13	((FILE_m13 *) stdin)
#define stdout_m13	((FILE_m13 *) stdout)
#define stderr_m13	((FILE_m13 *) stderr)

// Typedefs
typedef struct {
	ui4	tag; // == FILE_MARKER_m13 (marker for FILE_m13 vs FILE)
	ui4	fid; // CRC of file path (can't use file descriptor because not unique if file dup'd)
	si1	path[PATH_BYTES_m13];
	ui2	flags;
	ui2	perms; // system file permissions (lower 9 bits of "st_mode" element of stat structure)
	si4	fd; // system file descriptor
	FILE	*fp; // system FILE pointer
	si8	len; // current file length (-1 indicates not set)
	si8	pos; // file pointer position (relative to start)  (-1 indicates not set)
	si8	acc; // uutc of last file access (open, read, or write functions, if FILE_FLAGS_TIME_m13 bit set)
} FILE_m13;

// Prototypes
FILE_m13 	*FILE_from_std_m13(FILE *std_fp, const si1 *path);
ui4		FILE_id_m13(const si1 *path);
FILE_m13	*FILE_init_m13(void *fp, ...); // varargs(fp == stream): const si1 *path
tern		FILE_is_std_m13(void *fp);
tern		FILE_locking_m13(void *fp, tern heed);  // turn locking on or off for a file (heed or ignore locks on fp)
tern		FILE_show_m13(FILE_m13 *fp);
FILE 		*FILE_std_m13(void *fp);
FILE 		*FILE_to_std_m13(void *fp, si1 *path);



//**********************************************************************************//
//******************************* Processes (PROC) *******************************//
//**********************************************************************************//

// Thread Management Constants
#define PROC_DEFAULT_PRIORITY_m13 		0x7FFFFFFF
#define PROC_MIN_PRIORITY_m13			0x7FFFFFFE
#define PROC_LOW_PRIORITY_m13			0x7FFFFFFD
#define PROC_MEDIUM_PRIORITY_m13		0x7FFFFFFC
#define PROC_HIGH_PRIORITY_m13			0x7FFFFFFB
#define PROC_MAX_PRIORITY_m13			0x7FFFFFFA
#define PROC_UNDEFINED_PRIORITY_m13		0x7FFFFFF9

// PROC_THREAD_INFO_m13 status values
#define PROC_THREAD_WAITING_m13			((ui4) 0)
#define PROC_THREAD_RUNNING_m13			((ui4) 1)
#define PROC_THREAD_SUCCEEDED_m13		((ui4) 2)
#define PROC_THREAD_FAILED_m13			((ui4) 4)
#define PROC_THREAD_SKIPPED_m13			((ui4) 8)
#define PROC_THREAD_FINISHED_m13		( PROC_THREAD_SUCCEEDED_m13 | PROC_THREAD_FAILED_m13 | PROC_THREAD_SKIPPED_m13 )

#define PROC_THREAD_NAME_LEN_DEFAULT_m13	64


typedef ui8	pid_t_m13; // big enough for all OSs, none use signed values
			   // (pid_t_m13 is used for both process and thread IDs throughout the library)

typedef void 	(*sig_handler_t_m13)(si4); // signal handler function pointer

#if defined MACOS_m13 || defined LINUX_m13
	#ifdef MACOS_m13
	typedef	struct {
		si4	cpu_array[64]; // selected cpu numbers (from zero); cast cpu_array to thread_policy_t
		ui4	cpu_count; // number of cpus in cpu_array; cast to mach_msg_type_number_t
	}				cpu_set_t_m13; // max 64 logical cores (in this implementation, no actual limit)
	#endif // MACOS_m13
	#ifdef LINUX_m13
	typedef	cpu_set_t		cpu_set_t_m13; // opaque type (unclear if there is a logical core count limit)
	#endif // LINUX_m13

// pthreads
	typedef	pthread_t		pthread_t_m13; // opaque type (appears to be large structure: 4-8 KB)
	typedef pthread_attr_t		pthread_attr_t_m13;
	typedef void *			pthread_rval_m13;
	typedef pthread_rval_m13 	(*pthread_fn_m13)(void *);
	typedef	pthread_mutex_t		pthread_mutex_t_m13;
	typedef	pthread_mutexattr_t	pthread_mutexattr_t_m13;

// semaphores
	typedef sem_t			sem_t_m13;
#endif // MACOS_m13 || LINUX_m13

#ifdef WINDOWS_m13
	typedef	ui8			cpu_set_t_m13; // max 64 logical cores (defined as DWORD_PTR in Windows, but not used as a pointer; used as ui8)

// pthreads
	typedef	HANDLE			pthread_t_m13; // == void * (opaque type)
	typedef void *			pthread_attr_t_m13;
	typedef ui4 			pthread_rval_m13;
	typedef pthread_rval_m13 	(*pthread_fn_m13)(void *);
	typedef	HANDLE			pthread_mutex_t_m13;
	typedef	SECURITY_ATTRIBUTES	pthread_mutexattr_t_m13;

// semaphores
	typedef	HANDLE			sem_t_m13;
#endif // WINDOWS_m13

// Inverse Semaphores

// function as semaphores that block when count > 0
// function interfaces resemble, but are not the same as sempahore function interfaces
// (implemented with mutex because atomic alone can't guarantee no change between accesses in isem functions)
// (implemented with atomic because mutex alone can't guarantee global value updated between thread accesses)

// if an isem is owned by another thread, accesses that would change the count will block
// (or fail in try functions) until ownership is released or transferred to the calling thread

// example usage: flock reader count => write request blocks in isem until reader count == 0

#define ISEM_SELF_m13		((pid_t_m13) 0xFFFFFFFFFFFFFFFF)

typedef struct {
	_Atomic ui4		count;
	ui4			period;  // checking period, in nanoseconds (max ~4.3 secs); 0 indicates no wait in try functions, default  (10 us) in non-try functions
	const si1 		*name;
	_Atomic pid_t_m13	owner;  // thread id of owning thread, zero when unowned (normal state)
	pthread_mutex_t_m13	mutex;
	_Atomic tern		mutex_initialized;
	tern			free_on_destroy;
} isem_t_m13;

typedef struct {
	pthread_t_m13	thread;
	const si1	*thread_name;
	pthread_fn_m13	thread_f; // thread function pointer
	void		*arg; // function-specific info structure, set by calling function
	si4		priority; // typically PROC_HIGH_PRIORITY_m13
	_Atomic ui4	status;
	_Atomic tern	thread_job; // if FALSE_m13, run in current thread as function
	tern		skip_job;  // if TRUE_m13, don't run this thread (b/c e.g. file missing)
} PROC_THREAD_INFO_m13;

typedef struct {
	const si1	*thread_name;
	pthread_fn_m13	thread_f; // the thread function pointer
	void		*arg; // function-specific info structure, set by calling function
} PROC_MACOS_NAMED_THREAD_INFO_m13;


// Prototypes
// Note: medlib versions of standard pthread functions are prototyped with other standard function versions
tern			PROC_adjust_open_file_limit_m13(si4 new_limit, tern verbose_flag);
tern			PROC_default_threading_m13(void *lh); // lh is an LH_m13 *
tern			PROC_distribute_jobs_m13(PROC_THREAD_INFO_m13 *thread_infos, si4 n_jobs, si4 n_reserved_cores, tern wait_jobs, tern thread_jobs);
cpu_set_t_m13		*PROC_generate_cpu_set_m13(const si1 *affinity_str, cpu_set_t_m13 *cpu_set_p);
pid_t_m13		PROC_id_for_thread_m13(pthread_t_m13 *thread_p);
tern			PROC_increase_process_priority_m13(tern verbose_flag, si4 sudo_prompt_flag, ...); // varargs (sudo_prompt_flag == TRUE_m13): const si1 *exec_name, sf8 timeout_secs
pid_t_m13		PROC_launch_thread_m13(pthread_t_m13 *thread_p, pthread_fn_m13 thread_f, void *arg, si4 priority, const si1 *affinity_str, cpu_set_t_m13 *cpu_set_p, tern detached, const si1 *thread_name);
pthread_rval_m13	PROC_macos_named_thread_m13(void *arg);
tern			PROC_set_thread_affinity_m13(pthread_t_m13 *thread_p, pthread_attr_t_m13 *attributes, cpu_set_t_m13 *cpu_set_p, tern wait_for_lauch);
tern			PROC_show_thread_affinity_m13(pthread_t_m13 *thread_p);
void			PROC_show_thread_list_m13(void);
pthread_t_m13		*PROC_thread_for_id_m13(pid_t_m13 _id);
void			PROC_thread_list_add_m13(pthread_t_m13 *thread_p);
void			PROC_thread_list_remove_m13(pid_t_m13 _id);
pid_t_m13		PROC_thread_parent_id_m13(pid_t_m13 _id);
tern			PROC_wait_jobs_m13(PROC_THREAD_INFO_m13 *jobs, si4 n_jobs);

#if defined MACOS_m13
	// The prototypes of these two functions are commented out in mach/thread_policy.h, so declared here
	kern_return_t	thread_policy_set(thread_t thread, thread_policy_flavor_t flavor, thread_policy_t policy_info, mach_msg_type_number_t count);
	kern_return_t	thread_policy_get(thread_t thread, thread_policy_flavor_t flavor, thread_policy_t policy_info, mach_msg_type_number_t *count, boolean_t *get_default);
#endif // MACOS_m13



//**********************************************************************************//
//************************** Parallel (PAR) Functions ****************************//
//**********************************************************************************//

// Constants
#define PAR_LAUNCHING_m13		1
#define PAR_RUNNING_m13			2
#define PAR_FINISHED_m13		3
#define PAR_DEFAULTS_m13		"defaults"
#define PAR_UNTHREADED_m13		0

// Supported Functions
#define PAR_OPEN_SESSION_M13		1
#define PAR_READ_SESSION_M13		2
#define PAR_OPEN_CHANNEL_M13		3
#define PAR_READ_CHANNEL_M13		4
#define PAR_OPEN_SEGMENT_M13		5
#define PAR_READ_SEGMENT_M13		6
#define PAR_DM_GET_MATRIX_M13		7

// Structures
typedef struct {
	si1		label[64];
	si1		function[64];
	void		*r_val; // pointer to returned data
	pid_t_m13	tid;
	pthread_t_m13	thread_id;
	si4		priority;
	si1		affinity[16];
	si4		detached;
	si4		status;
} PAR_INFO_m13;

typedef struct {
	PAR_INFO_m13	*par_info;
	va_list		args;
} PAR_THREAD_INFO_m13;

// Protoypes
tern			PAR_free_m13(PAR_INFO_m13 **par_info_ptr);
PAR_INFO_m13		*PAR_init_m13(PAR_INFO_m13 *par_info, const si1 *function, const si1 *label, ...); // varargs(label != PAR_DEFAULTS_m13 or NULL): si4 priority, const si1 *affinity, si4 detached
PAR_INFO_m13		*PAR_launch_m13(PAR_INFO_m13 *par_info, ...); // varargs (par_info == NULL): const si1 *function, const si1 *label, si4 priority, const si1 *affinity, si4 detached, <function arguments>
								      // varargs (par_info != NULL): <function arguments>
tern			PAR_show_info_m13(PAR_INFO_m13 *par_info);
pthread_rval_m13	PAR_thread_m13(void *arg);
tern			PAR_wait_m13(PAR_INFO_m13 *par_info, const si1 *interval);



//**********************************************************************************//
//*************************** Parity (PRTY) Functions ****************************//
//**********************************************************************************//

// Flags
#define PRTY_GLB_SESS_REC_DATA_m13	((ui4) 1 << 0)
#define PRTY_GLB_SESS_REC_IDX_m13	((ui4) 1 << 1)
#define PRTY_SEG_SESS_REC_DATA_m13	((ui4) 1 << 2)
#define PRTY_SEG_SESS_REC_IDX_m13	((ui4) 1 << 3)
#define PRTY_TS_CHAN_REC_DATA_m13	((ui4) 1 << 4)
#define PRTY_TS_CHAN_REC_IDX_m13	((ui4) 1 << 5)
#define PRTY_TS_SEG_REC_DATA_m13	((ui4) 1 << 6)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_TS_SEG_REC_IDX_m13		((ui4) 1 << 7)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_TS_SEG_DAT_DATA_m13	((ui4) 1 << 8)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_TS_SEG_DAT_IDX_m13		((ui4) 1 << 9)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_TS_SEG_META_m13		((ui4) 1 << 10)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_VID_CHAN_REC_DATA_m13	((ui4) 1 << 11)
#define PRTY_VID_CHAN_REC_IDX_m13	((ui4) 1 << 12)
#define PRTY_VID_SEG_REC_DATA_m13	((ui4) 1 << 13)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_VID_SEG_REC_IDX_m13	((ui4) 1 << 14)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_VID_SEG_DAT_DATA_m13	((ui4) 1 << 15)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_VID_SEG_DAT_IDX_m13	((ui4) 1 << 16)  // requires segment number (or PRTY_ALL_SEGS_m13)
#define PRTY_VID_SEG_META_m13		((ui4) 1 << 17)  // requires segment number (or PRTY_ALL_SEGS_m13)

#define PRTY_GLB_SESS_RECS_m13		( PRTY_GLB_SESS_REC_DATA_m13 | PRTY_GLB_SESS_REC_IDX_m13 )
#define PRTY_SSR_m13			( PRTY_SEG_SESS_REC_DATA_m13 | PRTY_SEG_SESS_REC_IDX_m13 )
#define PRTY_TS_CHAN_RECS_m13		( PRTY_TS_CHAN_REC_DATA_m13 | PRTY_TS_CHAN_REC_IDX_m13 )
#define PRTY_VID_CHAN_RECS_m13		( PRTY_VID_CHAN_REC_DATA_m13 | PRTY_VID_CHAN_REC_IDX_m13 )
#define PRTY_TS_SEG_RECS_m13		( PRTY_TS_SEG_REC_DATA_m13 | PRTY_TS_SEG_REC_IDX_m13 )
#define PRTY_VID_SEG_RECS_m13		( PRTY_VID_SEG_REC_DATA_m13 | PRTY_VID_SEG_REC_IDX_m13 )
#define PRTY_TS_SEG_DATA_m13		( PRTY_TS_SEG_DAT_DATA_m13 | PRTY_TS_SEG_DAT_IDX_m13 )
#define PRTY_VID_SEG_DATA_m13		( PRTY_VID_SEG_DAT_DATA_m13 | PRTY_VID_SEG_DAT_IDX_m13 )

#define PRTY_SESS_RECS_m13		( PRTY_GLB_SESS_RECS_m13 | PRTY_SSR_m13 )
#define PRTY_CHAN_RECS_m13		( PRTY_TS_CHAN_RECS_m13 | PRTY_VID_CHAN_RECS_m13 )
#define PRTY_SEG_RECS_m13		( PRTY_TS_SEG_RECS_m13 | PRTY_VID_SEG_RECS_m13 )
#define PRTY_SEG_DATA_m13		( PRTY_TS_SEG_DATA_m13 | PRTY_VID_SEG_DATA_m13 )

#define PRTY_TS_CHAN_m13		PRTY_TS_CHAN_RECS_m13
#define PRTY_TS_SEG_m13			( PRTY_TS_SEG_RECS_m13 | PRTY_TS_SEG_DATA_m13 | PRTY_TS_SEG_META_m13 )
#define PRTY_VID_CHAN_m13		PRTY_VID_CHAN_RECS_m13
#define PRTY_VID_SEG_m13		( PRTY_VID_SEG_RECS_m13 | PRTY_VID_SEG_DATA_m13 | PRTY_VID_SEG_META_m13 )

#define PRTY_SESS_m13			( PRTY_SESS_RECS_m13 | PRTY_SSR_m13 )
#define PRTY_CHAN_m13			( PRTY_TS_CHAN_m13 | PRTY_VID_CHAN_m13 )
#define PRTY_SEG_m13			( PRTY_TS_SEG_m13 | PRTY_VID_SEG_m13 )

#define PRTY_ALL_TS_m13			( PRTY_SESS_m13 | PRTY_TS_CHAN_m13 | PRTY_TS_SEG_m13 )
#define PRTY_ALL_VID_m13		( PRTY_SESS_m13 | PRTY_VID_CHAN_m13 | PRTY_VID_SEG_m13 )
#define PRTY_ALL_FILES_m13		( PRTY_ALL_TS_m13 | PRTY_ALL_VID_m13 )
#define PRTY_ALL_SEGS_m13		((si4) -1) // pass as "segment_number" argument to PRTY_write_m13()

// Masks
#define PRTY_TS_MASK_m13		( PRTY_TS_CHAN_m13 | PRTY_TS_SEG_m13 )
#define PRTY_VID_MASK_m13		( PRTY_VID_CHAN_m13 | PRTY_VID_SEG_m13 )

// Parity file array fixed positions
#define PRTY_FILE_CHECK_IDX_m13		0 		  // file to check in first slot
#define PRTY_FILE_DAMAGED_IDX_m13	PRTY_FILE_CHECK_IDX_m13  // damaged file in first slot

// Miscellaneous
#define PRTY_BLOCK_BYTES_DEFAULT_m13	4096 // used in PRTY_CRC_DATA_m13 (must be multiple of 4)
#define PRTY_PCRC_TAG_m13		((ui8) 0x0123456789ABCDEF) // used in PRTY_CRC_DATA_m13


// Structures
typedef struct {
	si1		path[PATH_BYTES_m13];
	si8		len;
	FILE_m13	*fp;
	tern		finished; // data incorporated into parity
} PRTY_FILE_m13;

typedef struct {
	si8		length;
	si8		offset;
} PRTY_BLOCK_m13; // bad block location returned from PRTY_validate_m13()

typedef struct {
	ui1		*parity;
	ui1		*data;
	si8		mem_block_bytes;
	si1		path[PATH_BYTES_m13]; // path to parity file
	PRTY_FILE_m13	*files;
	si4		n_files;
	si4		n_bad_blocks;
	PRTY_BLOCK_m13	*bad_blocks;
} PRTY_m13;

typedef struct {
	ui8		tag; // marker to confirm identity of this structure
	ui8		session_UID; // present in all parity files (same names - in case misplaced)
	ui8		segment_UID; // UID_NO_ENTRY_m13 (zero) in parity data that is session level (same names - in case misplaced)
	ui4		n_blocks; // number of data blocks (& crcs) preceding this structure
	ui4		block_bytes; // data bytes per block (except probably the last), multiple of 4 bytes (defaults to PRTY_BLOCK_BYTES_DEFAULT_m13)
} PRTY_CRC_DATA_m13;

// Parity File Structure:
// 1) parity data
// 2) crc of parity data in blocks // used to confirm that parity data is not itself damaged, & if so, to localize the damage, so that it can hopefully still be used & then rebuilt
// 3) PRTY_CRC_DATA_m13 structure

// Prototypes
tern	PRTY_build_m13(PRTY_m13 *parity_ps);
si4	PRTY_file_compare_m13(const void *a, const void *b);
si1	**PRTY_file_list_m13(const si1 *MED_path, si4 *n_files);
ui4	PRTY_flag_for_path_m13(const si1 *path);
tern	PRTY_is_parity_m13(const si1 *path, tern MED_file);
si8	PRTY_pcrc_block_bytes_m13(FILE_m13 *fp, const si1 *file_path);
si8	PRTY_pcrc_offset_m13(FILE_m13 *fp, const si1 *file_path, PRTY_CRC_DATA_m13 *pcrc);
tern	PRTY_recover_segment_header_fields_m13(const si1 *MED_file, ui8 *segment_uid, si4 *segment_number);
tern	PRTY_repair_file_m13(PRTY_m13 *parity_ps);
tern	PRTY_restore_m13(const si1 *MED_path);
tern	PRTY_set_pcrc_uids_m13(PRTY_CRC_DATA_m13 *pcrc, const si1 *MED_path);
tern	PRTY_show_pcrc_m13(const si1 *file_path);
tern	PRTY_update_m13(void *ptr, si8 n_bytes, si8 offset, void *fp, ...);  // vararg(fp == FILE *): const si1 *path
tern	PRTY_update_pcrc_m13(void *ptr, si8 n_bytes, si8 offset, void *fp, ...);  // vararg(fp == FILE *): const si1 *path)
tern	PRTY_validate_m13(const si1 *file_path, ...); // varargs(file_path == NULL): const si1 *file_path, PRTY_BLOCK_m13 **bad_blocks, si4 *n_bad_blocks, ui4 *n_blocks
tern	PRTY_validate_pcrc_m13(const si1 *file_path, ...); // varargs(file_path == NULL): const si1 *file_path, PRTY_BLOCK_m13 **bad_blocks, si4 *n_bad_blocks, ui4 *n_blocks
tern	PRTY_write_m13(const si1 *sess_path, ui4 flags, si4 segment_number);
tern	PRTY_write_pcrc_m13(const si1 *file_path, ui4 block_bytes);



//**********************************************************************************//
//******************** Runtime Configuration (RC) Functions **********************//
//**********************************************************************************//

// Constants
#define RC_NO_ENTRY_m13		-2
#define RC_ERR_m13 		-1
#define RC_NO_OPTION_m13	0
#define RC_STRING_TYPE_m13 	1
#define RC_FLOAT_TYPE_m13 	2
#define RC_INTEGER_TYPE_m13 	3
#define RC_TERNARY_TYPE_m13 	4
#define RC_UNKNOWN_TYPE_m13 	5

#define RC_STRING_BYTES_m13	256


// Prototypes
si4	RC_read_field_m13(const si1 *field_name, si1 **buffer, tern update_buffer_ptr, si1 *field_value_str, sf8 *float_val, si8 *int_val, tern *TERN_val);
si4	RC_read_field_2_m13(const si1 *field_name, si1 **buffer, tern update_buffer_ptr, void *val, si4 val_type, ...); // vararg (val_type == RC_UNKNOWN_m13): *returned_val_type


//**********************************************************************************//
//************************* Networking (NET) Functions ***************************//
//**********************************************************************************//

// Constants
#define NET_MAC_ADDRESS_BYTES_m13		6
#define NET_MAC_ADDRESS_STR_BYTES_m13		(NET_MAC_ADDRESS_BYTES_m13 * 3) // 6 hex bytes plus colons & terminal zero
#define NET_IPV4_ADDRESS_BYTES_m13		4
#define NET_IPV4_ADDRESS_STR_BYTES_m13		(NET_IPV4_ADDRESS_BYTES_m13 * 4) // 4 dec bytes plus periods & terminal zero


// Structures
typedef struct {
	si1	interface_name[64];
	si1	host_name[256]; // max 253 ascii characters
	union {
		ui1	MAC_address_bytes[8]; // network byte order
		ui8	MAC_address_num; // network byte order
	};
	si1 MAC_address_string[NET_MAC_ADDRESS_STR_BYTES_m13]; // upper case hex with colons
	union {
		ui1	LAN_IPv4_address_bytes[NET_IPV4_ADDRESS_BYTES_m13]; // network byte order
		ui4	LAN_IPv4_address_num; // network byte order
	};
	si1 LAN_IPv4_address_string[NET_IPV4_ADDRESS_STR_BYTES_m13]; // dec with periods
	union {
		ui1	LAN_IPv4_subnet_mask_bytes[NET_IPV4_ADDRESS_BYTES_m13]; // network byte order
		ui4	LAN_IPv4_subnet_mask_num; // network byte order
	};
	si1 LAN_IPv4_subnet_mask_string[NET_IPV4_ADDRESS_STR_BYTES_m13]; // dec with periods
	union {
		ui1	WAN_IPv4_address_bytes[NET_IPV4_ADDRESS_BYTES_m13]; // network byte order
		ui4	WAN_IPv4_address_num; // network byte order
	};
	si1	WAN_IPv4_address_string[NET_IPV4_ADDRESS_STR_BYTES_m13]; // dec with periods
	si4	MTU; // maximum transmission unit
	si1	link_speed[16];
	si1	duplex[16];
	tern	active; // interface status
	tern	plugged_in;
} NET_PARAMS_m13;

// Prototypes
tern		NET_check_internet_connection_m13(void);
tern		NET_domain_to_ip_m13(const si1 *domain_name, si1 *ip);
NET_PARAMS_m13	*NET_get_active_m13(si1 *iface, NET_PARAMS_m13 *np);
tern		NET_get_adapter_m13(NET_PARAMS_m13 *np, tern copy_global);
tern		NET_get_config_m13(NET_PARAMS_m13 *np, tern copy_global);
NET_PARAMS_m13	*NET_get_default_interface_m13(NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_duplex_m13(si1 *iface, NET_PARAMS_m13 *np);
tern		NET_get_ethtool_m13(NET_PARAMS_m13 *np, tern copy_global);
NET_PARAMS_m13	*NET_get_host_name_m13(NET_PARAMS_m13 *np);
void		*NET_get_in_addr_m13(struct sockaddr *sa);
NET_PARAMS_m13	*NET_get_lan_ipv4_address_m13(si1 *iface, NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_link_speed_m13(si1 *iface, NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_mac_address_m13(si1 *iface, NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_mtu_m13(si1 *iface, NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_parameters_m13(si1 *iface, NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_plugged_in_m13(si1 *iface, NET_PARAMS_m13 *np);
NET_PARAMS_m13	*NET_get_wan_ipv4_address_m13(NET_PARAMS_m13 *np);
si1		*NET_iface_name_for_addr_m13(si1 *iface_name, const si1 *iface_addr);
tern		NET_init_tables_m13(void); // set global NET_PARAMS for default internet interface
tern		NET_reset_parameters_m13(NET_PARAMS_m13 *np);
tern		NET_resolve_arguments_m13(si1 *iface, NET_PARAMS_m13 **params_ptr, tern *free_params);
tern		NET_show_parameters_m13(NET_PARAMS_m13 *np);
tern		NET_trim_address_m13(si1 *addr_str);



//**********************************************************************************//
//********************************** MED Errors **********************************//
//**********************************************************************************//


// Behaviors Codes
#define RETURN_ON_FAIL_m13		((ui4) 1 << 0) // return on error; if not set, exit on error
#define IGNORE_ERROR_m13		((ui4) 1 << 1) // do not set error & return (even if RETURN_ON_FAIL_m13 is not set)
#define SUPPRESS_ERROR_OUTPUT_m13	((ui4) 1 << 2)
#define SUPPRESS_WARNING_OUTPUT_m13	((ui4) 1 << 3)
#define SUPPRESS_MESSAGE_OUTPUT_m13	((ui4) 1 << 4)
#define RETRY_ONCE_m13			((ui4) 1 << 5)

// Behavior constants (can't overlap with codes)
#define CURRENT_BEHAVIOR_STACK_m13	((ui4) 1 << 30)
#define CURRENT_BEHAVIOR_m13		((ui4) 1 << 31)

#define DEFAULT_BEHAVIOR_m13		RETURN_ON_FAIL_m13 // (code) return on fail, observe system errors, do not retry, show all output
#define E_BEHAVIOR_m13			((ui4) 0) // (code) exit on fail, observe system errors, do not retry, show all output
#define E_UNKNOWN_LINE_m13		((si4) -1) // usied in signal errors that have no line numbers

// convenience
#define SUPPRESS_OUTPUT_m13		( SUPPRESS_ERROR_OUTPUT_m13 | SUPPRESS_WARNING_OUTPUT_m13 | SUPPRESS_MESSAGE_OUTPUT_m13 )
#define RETURN_QUIETLY_m13		( IGNORE_ERROR_m13 | SUPPRESS_OUTPUT_m13 )  // generally used with handleable or acceptable errors

// error codes
#define E_NUM_CODES_m13		21
typedef enum {
	E_NONE_m13,		// 0, no error
	E_GEN_m13,		// 1, general / unspecified error
	E_SIG_m13,		// 2, system signal
	E_ALLOC_m13,		// 3, allocation error
	E_FGEN_m13,		// 4, general file error
	E_FEXIST_m13,		// 5, file exists error
	E_FOPEN_m13,		// 6, file open error
	E_FREAD_m13,		// 7, file read error
	E_FWRITE_m13,		// 8, file write error
	E_FLOCK_m13,		// 9, file lock error
	E_FMED_m13,		// 10, not med file error
	E_FACC_m13,		// 11, file access / password error
	E_CRYP_m13,		// 12, cryptographic error
	E_MET_m13,		// 13, metadata error
	E_REC_m13,		// 14, records error
	E_NET_m13,		// 15, network error
	E_CMP_m13,		// 16, compression / computational error
	E_PROC_m13,		// 17, process control error
	E_FILT_m13,		// 18, filter error
	E_DB_m13,		// 19, database error
	E_PRTY_m13		// 20, parity error
} E_MED_CODES_m13;

// error string table
#define	E_MAX_STR_LEN_m13		((PATH_BYTES_m13 << 1) + 128)  // enough for two paths plus some text
#define E_MESSAGE_LEN_m13		E_MAX_STR_LEN_m13
#define E_STR_TABLE_ENTRIES_m13		E_NUM_CODES_m13
#define E_STR_TABLE_m13 { \
	"no errors", \
	"general error", \
	"system signal", \
	"memory allocation error", \
	"general file error", \
	"file or directory not found", \
	"file open error", \
	"file read error", \
	"file write error", \
	"file lock error", \
	"not MED file or directory", \
	"access denied", \
	"cryptographic error", \
	"metadata not found", \
	"record error", \
	"network error", \
	"compression / computation error", \
	"process error", \
	"filter error", \
	"database error", \
	"parity error" \
}
#define E_TAG_TABLE_ENTRIES_m13		E_NUM_CODES_m13
#define E_TAG_TABLE_m13 { \
	"E_NONE", \
	"E_GEN", \
	"E_SIG", \
	"E_ALLOC", \
	"E_FGEN", \
	"E_FEXIST", \
	"E_FOPEN", \
	"E_FREAD", \
	"E_FWRITE", \
	"E_FLOCK", \
	"E_FMED", \
	"E_FACC", \
	"E_CRYP", \
	"E_MET", \
	"E_REC", \
	"E_NET", \
	"E_CMP", \
	"E_PROC", \
	"E_FILT", \
	"E_DB", \
	"E_PRTY" \
}

typedef struct {
	_Atomic si4		code;
	si4			signal;
	si4			line;
	const si1		*function;
	si1			message[E_MAX_STR_LEN_m13];
	si1			thread_name[PROC_THREAD_NAME_LEN_DEFAULT_m13];
	_Atomic pid_t_m13	thread_id;
	pthread_mutex_t_m13	mutex; // prevent access while being modified (does not duplicate role of isem)
	isem_t_m13		isem; // inverse semaphore
} ERR_m13;

#define G_set_error_m13(code, message, ...)	G_set_error_exec_m13(__FUNCTION__, __LINE__, code, message, ##__VA_ARGS__) // vararg(code == E_SIG_m13): si4 sig_num (followed by optional formatting string values)
#ifdef MATLAB_m13
	#define eprintf_m13(fmt, ...)		mexPrintf("%s(%d) " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
	#define eprintf_m13(fmt, ...)		do { fprintf_m13(stderr, "%s%s%s(%d)%s " fmt "\n", TC_RED_m13, __FUNCTION__, TC_BLUE_m13, __LINE__, TC_RESET_m13, ##__VA_ARGS__); fflush(stderr); } while(0)
#endif



//**********************************************************************************//
//********************************** MED Macros **********************************//
//**********************************************************************************//

#define METADATA_CODE_m13(x)			( (((x) == TS_METADATA_TYPE_CODE_m13) || ((x) == VID_METADATA_TYPE_CODE_m13)) ? TRUE_m13 : FALSE_m13 )
#define INDICES_CODE_m13(x)			( (((x) == TS_INDS_TYPE_CODE_m13) || ((x) == VID_INDS_TYPE_CODE_m13) || ((x) == REC_INDS_TYPE_CODE_m13)) ? TRUE_m13 : FALSE_m13 )
#define DATA_CODE_m13(x)			( (((x) == TS_DATA_TYPE_CODE_m13) || ((x) == VID_DATA_TYPE_CODE_m13) || ((x) == REC_DATA_TYPE_CODE_m13)) ? TRUE_m13 : FALSE_m13 )
#define RECORD_CODE_m13(x)			( (((x) == REC_INDS_TYPE_CODE_m13) || ((x) == REC_DATA_TYPE_CODE_m13)) ? TRUE_m13 : FALSE_m13 )
#define CHANNEL_CODE_m13(x)			( (((x) == TS_CHAN_TYPE_CODE_m13) || ((x) == VID_CHAN_TYPE_CODE_m13)) ? TRUE_m13 : FALSE_m13 )
#define SEGMENT_CODE_m13(x)			( (((x) == TS_SEG_TYPE_CODE_m13) || ((x) == VID_SEG_TYPE_CODE_m13)) ? TRUE_m13 : FALSE_m13 )
#define PLURAL_m13(x) 				( ((x) == 1) ? "" : "s" )
#define ABS_m13(x)				( ((x) >= 0) ? (x) : -(x) )  // do not increment/decrement in call to ABS (as x occurs thrice)
#define HEX_STR_BYTES_m13(x, y) 		( ((x) << 1) + (((x) - 1) * (y)) + 1 ) // x numerical bytes with y-byte seperators plus terminal zero
#define BIN_STR_BYTES_m13(x, y) 		( ((x) << 3) + (((x) - 1) * (y)) + 1 )  // x numerical bytes with y-byte seperators plus terminal zero
#define REMOVE_DISCONT_m13(x)			( ((x) >= 0) ? (x) : -(x) )  // do not increment/decrement in call to REMOVE_DISCONTINUITY (as x occurs thrice)
#define APPLY_DISCONT_m13(x)			( ((x) <= 0) ? (x) : -(x) )  // do not increment/decrement in call to APPLY_DISCONTINUITY (as x occurs thrice)
#define MAX_OPEN_FILES_m13(n_chans, n_segs)	( (5 * n_chans * n_segs) + (2 * n_segs) + (2 * n_chans) + 5 ) // Note: final +5 == 2 for session level records plus 3 for standard streams (stdin, stdout, & stderr)
// "S" versions are for slice structures (not pointers)
#define MED_VER_1_0_m13(x)			( (x->MED_version_major == 1 && x->MED_version_minor == 0) ? TRUE_m13 : FALSE_m13 )
#define MED_VER_1_0_S_m13(x)			( (x.MED_version_major == 1 && x.MED_version_minor == 0) ? TRUE_m13 : FALSE_m13 )
#define SLICE_IDX_COUNT_m13(x)			( ((x)->end_idx - (x)->start_idx) + 1 )
#define SLICE_IDX_COUNT_S_m13(x)		( ((x).end_idx - (x).start_idx) + 1 )
#define SLICE_SEG_COUNT_m13(x)			( ((x)->end_seg_num - (x)->start_seg_num) + 1 )
#define SLICE_SEG_COUNT_S_m13(x)		( ((x).end_seg_num - (x).start_seg_num) + 1 )
#define SLICE_DUR_m13(x)			( ((x)->end_time - (x)->start_time) + 1 ) // time in usecs
#define SLICE_DUR_S_m13(x)			( ((x).end_time - (x).start_time) + 1 ) // time in usecs



//**********************************************************************************//
//*********************************** Hardware ***********************************//
//**********************************************************************************//

// Structures
typedef struct {
	si8	integer_multiplications_per_sec; // test mimics RED/PRED in operand length, other tests may yield somewhat different results
	si8	integer_divisions_per_sec; // test mimics RED/PRED in operand length, other tests may yield somewhat different results
	sf8	nsecs_per_integer_multiplication; // test mimics RED/PRED in operand length, other tests may yield somewhat different results
	sf8	nsecs_per_integer_division; // test mimics RED/PRED in operand length, other tests may yield somewhat different results
} HW_PERFORMANCE_SPECS_m13;

typedef struct {
	ui1				endianness;
	si4				physical_cores;
	si4				logical_cores;
	tern				hyperthreading;
	sf8				minimum_speed;
	sf8				maximum_speed;
	sf8				current_speed;
	si8				current_speed_uutc;
	HW_PERFORMANCE_SPECS_m13	performance_specs;
	ui8				system_memory_size; // system physical RAM (in bytes)
	ui4				system_page_size; // memory page (in bytes)
	ui8				heap_base_address;
	ui8				heap_max_address;
	si1				cpu_manufacturer[64];
	si1				cpu_model[64];
	si1				machine_serial[56]; // maximum serial number length is 50 characters
	ui4				machine_code; // code based on serial number
} HW_PARAMS_m13;

// Prototypes
ui4	HW_get_block_size_m13(const si1 *volume_path);
tern	HW_get_core_info_m13(void);
tern	HW_get_endianness_m13(void);
tern	HW_get_info_m13(void); // fill whole HW_PARAMS_m13 structure
tern	HW_get_machine_code_m13(void);
tern	HW_get_machine_serial_m13(void);
tern	HW_get_performance_specs_m13(tern get_current);
si1	*HW_get_performance_specs_file_m13(si1 *file);
tern	HW_get_performance_specs_from_file_m13(void);
tern	HW_get_memory_info_m13(void);
tern	HW_init_tables_m13(void);
tern	HW_show_info_m13(void);



//**********************************************************************************//
//********************************** General MED *********************************//
//**********************************************************************************//

// Daylight Change code
typedef union {
	struct {
		si1	code_type; // (DST end / DST Not Observed / DST start) == (-1 / 0 / +1)
		si1	day_of_week; // (No Entry / [Sunday : Saturday]) == (-1 / [0 : 6])
		si1	relative_weekday_of_month; // (No Entry / [First : Fifth] / Last) == (0 / [1 : 5] / 6)
		si1	day_of_month; // (No Entry / [1 : 31]) == (0 / [1 : 31])
		si1	month; // (No Entry / [January : December]) == (-1 / [0 : 11])
		si1	hours_of_day; // [-128 : +127] hours relative to 0:00 (midnight)
		si1	reference_time; // (Local / UTC) == (0 / +1)
		si1	shift_minutes; // [-120 : +120] minutes
	};
	si8 value; // 0 indicates DST is not observed, -1 indicates no entry
} DAYLIGHT_TIME_CHANGE_CODE_m13;

typedef struct {
	si1	country[METADATA_RECORDING_LOCATION_BYTES_m13];
	si1	country_acronym_2_letter[3]; // two-letter acronym; (ISO 3166 ALPHA-2)
	si1	country_acronym_3_letter[4]; // three-letter acronym (ISO-3166 ALPHA-3)
	si1	territory[METADATA_RECORDING_LOCATION_BYTES_m13];
	si1	territory_acronym[TIMEZONE_STRING_BYTES_m13];
	si1	standard_timezone[TIMEZONE_STRING_BYTES_m13];
	si1	standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m13];
	si4	standard_UTC_offset; // seconds
	si1	daylight_timezone[TIMEZONE_STRING_BYTES_m13];
	si1	daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m13];
	si8	daylight_time_start_code; // DAYLIGHT_TIME_CHANGE_CODE_m13 - cast to use other fields
	si8	daylight_time_end_code; // DAYLIGHT_TIME_CHANGE_CODE_m13 - cast to use other fields
	si8	daylight_codes_start_date; // onset of rules encoded in daylight codes (in uutc). DTCC_START_DATE_NO_ENTRY (-1) indicates it is the only historical rule for this timezone in the table.
} TIMEZONE_INFO_m13;

typedef struct {
	si1	table_name[METADATA_RECORDING_LOCATION_BYTES_m13];
	si1	alias[METADATA_RECORDING_LOCATION_BYTES_m13];
} TIMEZONE_ALIAS_m13;

typedef struct {
	tern	conditioned;
	si4	n_segs; // == (si4) UNKNOWN_m13 if segment range is unknown, otherwise == number of segments in slice
	si8 	start_time;
	si8 	end_time;
	union { // session-relative (global indexing)
		si8	start_samp_num;
		si8	start_frame_num;
		si8	start_idx; // generic
	};
	union { // session-relative (global indexing)
		si8	end_samp_num;
		si8	end_frame_num;
		si8	end_idx; // generic
	};
	si4 	start_seg_num;
	si4 	end_seg_num;
} SLICE_m13;

typedef struct {
	si8 	start_time;
	si8 	end_time;
	union { // session-relative (global indexing)
		si8	start_samp_num;
		si8	start_frame_num;
		si8	start_idx; // generic
	};
	union { // session-relative (global indexing)
		si8	end_samp_num;
		si8	end_frame_num;
		si8	end_idx; // generic
	};
	si4 	start_seg_num;
	si4 	end_seg_num;
} CONTIGUON_m13;

typedef struct { // times in uutc
	si8	creation;
	si8	access;
	si8	modification;
} FILE_TIMES_m13;

typedef struct { // fields from ipinfo.io
	TIMEZONE_INFO_m13	timezone_info;
	si1			WAN_IPv4_address[IPV4_ADDRESS_BYTES_m13 * 4];
	si1			locality[LOCALITY_BYTES_m13];
	si1			postal_code[POSTAL_CODE_BYTES_m13];
	si1			timezone_description[METADATA_RECORDING_LOCATION_BYTES_m13];
	sf8			latitude;
	sf8			longitude;
} LOCATION_INFO_m13;

typedef struct {
	ui4			source_type; // type code of source channel, or NO_TYPE_CODE_m13 (session level)
	union {
		sf4		sampling_frequency;
		sf4		frame_rate;
		sf4		rate;
	};
	union {
		si8		n_session_samples;
		si8		n_session_frames;
	};
	struct Sgmt_REC_m13	*Sgmt_recs; // defined below == record header + REC_Sgmt_v11_m13 body (session number of segments in length)
} Sgmt_RECS_ENTRY_m13;

typedef struct { // multiple thread access
	pthread_mutex_t_m13			mutex;
	_Atomic(Sgmt_RECS_ENTRY_m13 *)		entries;
	_Atomic si4				size; // total allocated Sgmt_RECORD_ENTRYs
	_Atomic si4				top_idx; // last non-empty Sgmt_RECORD_ENTRY in list
} Sgmt_RECS_LIST_m13;

typedef struct {
	ui8			UID;
	si1			path[PATH_BYTES_m13]; // path including file system session directory name
	si1			fs_name[NAME_BYTES_m13]; // name from file system
	si1			uh_name[NAME_BYTES_m13]; // name from universal_header (if differs from file system, otherwise empty)
	si8			start_time;
	si8			end_time;
	si4			n_segments; // number of segments in the session, regardless of whether they are mapped
	si4			n_mapped_segments; // may be less than n_session_segments
	si4			first_mapped_segment_number;
	Sgmt_RECS_LIST_m13	*Sgmt_recs_list; // list with one entry for each unique sampling frequency and channel type
	si1			index_channel_name[NAME_BYTES_m13]; // contains user specified value if needed, open_session_m13() matches to session channel
	struct CHAN_m13		*index_channel;
} CURRENT_SESSION_m13; // PROC_GLOBS_m13 element

typedef struct {
	sf8			minimum_sampling_frequency;
	sf8			maximum_sampling_frequency;
	sf8			minimum_frame_rate;
	sf8			maximum_frame_rate;
	tern 			sampling_frequencies_vary;
	tern 			frame_rates_vary;
	struct CHAN_m13		*minimum_sampling_frequency_channel;
	struct CHAN_m13		*maximum_sampling_frequency_channel;
	struct CHAN_m13		*minimum_frame_rate_channel;
	struct CHAN_m13		*maximum_frame_rate_channel;
} ACTIVE_CHANNELS_m13; // PROC_GLOBS_m13 element

typedef struct {
	tern	set;
	tern	RTO_known; // recording time offset
	tern 	observe_DST;
	si8	recording_time_offset;
	si4	standard_UTC_offset;
	si1	standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m13];
	si1	standard_timezone_string[TIMEZONE_STRING_BYTES_m13];
	si1	daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m13];
	si1	daylight_timezone_string[TIMEZONE_STRING_BYTES_m13];
	DAYLIGHT_TIME_CHANGE_CODE_m13	daylight_start_code; // si1[8] / si8
	DAYLIGHT_TIME_CHANGE_CODE_m13	daylight_end_code; // si1[8] / si8
} TIME_CONSTANTS_m13; // PROC_GLOBS_m13 element

typedef struct {
	ui4		mmap_block_bytes; // read size for memory mapped files (process data may be on different volumes)
					 // if files are on different volumes, use/set mmap_block_bytes in FILE_m13 structure
	volatile ui4	med_rand_w; // volatile because, while thread-local, multiple functions access & compiler cannot optimize with local copy
	volatile ui4	med_rand_z; // volatile because, while thread-local, multiple functions access & compiler cannot optimize with local copy
	_Atomic tern	threading; // TRUE_m13 == thread processing where appropriate (atomic b/c other threads may access)
	_Atomic tern	proc_error; // thread-local mechanism for void functions to indicate that an error occurred (can also set global error (atomic b/c other threads may access)
} MISCELLANEOUS_m13; // PROC_GLOBS_m13 element

// All MED File Structures begin with a level header structure
typedef struct LH_m13 { // multiple thread access
	union {
		struct {
			si1 	type_string[TYPE_BYTES_m13];
			ui1 	pad[3]; // enforce 8-byte alignment
		};
		struct {
			ui4 	type_code;
			si1	type_string_terminal_zero; // not used - here for clarity
			tern	allocated; // allocted on heap, independently (not en bloc)  [moved from flags - cleaner code]
			tern	names_differ;
		};
	};
	si1					*path; // NULL in proc_globs
	si1					*name; // NULL in proc_globs
	_Atomic(struct LH_m13 *)		parent; // NULL in proc_globs
	_Atomic(struct PROC_GLOBS_m13 *)	proc_globs; // self in proc_globs
	ui8					flags;
	_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
} LH_m13;

// non-standard structure
// required compiler option (gcc, clang): -fms-extensions
// suppress warnings: -Wno-microsoft-anon-tag
#ifdef __cplusplus
typedef struct PROC_GLOBS_m13 { // multiple thread access
	union {
		LH_m13 	header; // in case just want the level header
		struct { // this struct replaces anonymous LH_m13 in C++
			union {
				struct {
					si1 	type_string[TYPE_BYTES_m13];
					ui1 	pad[3]; // enforce 8-byte alignment
				};
				struct {
					ui4 	type_code;
					si1	type_string_terminal_zero; // not used - here for clarity
					tern	allocated; // allocted on heap, independently (not en bloc)  [moved from flags - cleaner code]
					tern	names_differ;
				};
			};
			si1					*path; // NULL in proc_globs
			si1					*name; // NULL in proc_globs
			_Atomic(LH_m13 *)			parent; // NULL in proc_globs
			_Atomic(struct PROC_GLOBS_m13 *)	proc_globs; // self in proc_globs
			ui8					flags;
			_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
		};
	};
 // Password
	PASSWORD_DATA_m13	password_data;
 // Current Session
	CURRENT_SESSION_m13	current_session;
 // Active Channels
	ACTIVE_CHANNELS_m13	active_channels;
 // Time Constants
	TIME_CONSTANTS_m13	time_constants;
 // Miscellaneous
	MISCELLANEOUS_m13	miscellaneous;
	pid_t_m13		_id; // thread or process id (used if LH_m13 unknown [NULL])
	LH_m13			*child; // hierarchy level immediately below these process globals
	si4			ref_count;
} PROC_GLOBS_m13;
#else // __cplusplus
typedef struct PROC_GLOBS_m13 {
	union {
		LH_m13	header; // in case just want the level header (parent NULL in PROC_GLOBS_m13)
		LH_m13; // anonymous LH_m13
	};
 // Password
	PASSWORD_DATA_m13	password_data;
 // Current Session
	CURRENT_SESSION_m13	current_session;
 // Active Channels
	ACTIVE_CHANNELS_m13	active_channels;
 // Time Constants
	TIME_CONSTANTS_m13	time_constants;
 // Miscellaneous
	MISCELLANEOUS_m13	miscellaneous;
	pid_t_m13		_id; // thread id (used if LH_m13 unknown [NULL])
	LH_m13			*child; // hierarchy level immediately below these process globals
	si4			ref_count; // count of structures & threads currently linked to these process globals
} PROC_GLOBS_m13;
#endif // __cplusplus

// NOTE: placement of LH_m13 in structures allows passing of LH_m13 pointer to functions,
// and based on its content, functions can cast pointer to specific level structures.
// e.g:
// if (lh->type_code == LH_SESS_m13)
//	sess = (SESS_m13 *) lh ;

typedef struct {
	pthread_mutex_t_m13		mutex;
	TIMEZONE_INFO_m13		*timezone_table;
	TIMEZONE_ALIAS_m13		*country_aliases_table;
	TIMEZONE_ALIAS_m13		*country_acronym_aliases_table;
	ui4				**CRC_table;
	ui1				*AES_sbox_table;
	ui1				*AES_rsbox_table;
	ui1				*AES_rcon_table;
	ui4				*SHA_h0_table;
	ui4				*SHA_k_table;
	sf8				*CMP_normal_CDF_table;
	CMP_VDS_THRESHOLD_MAP_ENTRY_m13	*CMP_VDS_threshold_map;
	NET_PARAMS_m13			NET_params; // parameters for default internet interface
	HW_PARAMS_m13			HW_params;
	const si1			**E_strings_table;
	const si1			**E_tags_table;
	#ifdef WINDOWS_m13
	HINSTANCE			hNTdll; // handle to ntdll dylib (used by WN_nap_m13(); only loaded if used)
	#endif
} GLOBAL_TABLES_m13;

typedef struct {
	const si1	*function; // function in which behavior was set
	si4		line; // line at which behavior was set
	ui4		code; // behavior code
} BEHAVIOR_m13;

typedef struct { // single thread access
	pid_t_m13		_id; // thread id
	BEHAVIOR_m13		*behaviors; // current behavior at top of stack (last entry)
	si4			size; // total allocated behaviors
	si4			top_idx; // top of behavior stack
} BEHAVIOR_STACK_m13;

typedef struct { // multiple thread access
	pthread_mutex_t_m13		mutex;
	_Atomic(BEHAVIOR_STACK_m13 **)	stack_ptrs; // secondary indirection to BEHAVIOR_STACK_m13 *
	_Atomic si4			size; // total allocated behavior_stacks
	_Atomic si4			top_idx; // last non-empty behavior_stack in list
} BEHAVIOR_STACK_LIST_m13;

// call with "G_push_function_m13(ui4 behavior)" prototype
#define G_add_behavior_m13(code)		G_add_behavior_exec_m13(__FUNCTION__, __LINE__, code) // call with "G_add_behavior_m13(ui4 behavior)" prototype
#define G_pop_behavior_m13()			G_pop_behavior_exec_m13(__FUNCTION__, __LINE__) // call with "G_pop_behavior_m13(ui4 behavior)" prototype
#define G_push_behavior_m13(code)		G_push_behavior_exec_m13(__FUNCTION__, __LINE__, code) // call with "G_push_behavior_m13(ui4 behavior)" prototype
#define G_remove_behavior_m13(code)		G_remove_behavior_exec_m13(__FUNCTION__, __LINE__, code) // call with "G_remove_behavior_m13(ui4 behavior)" prototype
#define G_behavior_stack_reset_m13(code)	G_behavior_stack_reset_exec_m13(__FUNCTION__, __LINE__, code) // call with "G_behavior_stack_reset_m13(ui4 behavior)" prototype

#define G_push_function_m13() 			G_push_function_exec_m13(__FUNCTION__) // call with "G_push_function_m13(void)" prototype
#define G_pop_function_m13()			G_pop_function_exec_m13(__FUNCTION__, __LINE__) // call with "G_pop_function_m13(void)" prototype


typedef struct {
	const si1	*name;
	si4		return_line;
} FUNCTION_ENTRY_m13;

typedef struct { // single thread access
	pid_t_m13		_id; // thread id
	FUNCTION_ENTRY_m13	*entries;
	si4			size; // total allocated functions
	si4			top_idx; // top of function stack
	si4			err_top_idx; // position in stack on error recursion
	tern			err_chain;  // TRUE_m13 if element of thread chain leading to causal error
} FUNCTION_STACK_m13;

typedef struct { // multiple thread access
	pthread_mutex_t_m13		mutex;
	_Atomic(FUNCTION_STACK_m13 **)	stack_ptrs; // secondary indirection to function stacks
	_Atomic si4			size; // total allocated function_stacks
	_Atomic si4			top_idx; // last non-empty function_stack in list
} FUNCTION_STACK_LIST_m13;

typedef struct { // multiple thread access
	pthread_mutex_t_m13		mutex;
	_Atomic(PROC_GLOBS_m13 **)	proc_globs_ptrs; // secondary indirection to process globals
	_Atomic si4			size; // total allocated proc_globs
	_Atomic si4			top_idx; // last non-empty function_stack in list
} PROC_GLOBS_LIST_m13;

typedef struct { // multiple thread access
	_Atomic pid_t_m13	owner_id; // thread id when writing, zero otherwise (only owner can unlock write)
	isem_t_m13		read_cnt; // number of processes currently reading file
	isem_t_m13		write_cnt; // number of processes currently writing file (0 or 1)
	_Atomic ui4		open_cnt; // number of processes that have file open
	_Atomic ui4		file_id; // CRC of full path
} FLOCK_ENTRY_m13;

typedef struct { // multiple thread access
	pthread_mutex_t_m13		mutex;
	_Atomic(FLOCK_ENTRY_m13	**)	lock_ptrs; // secondary indirection to FLOCK_ENTRY_m13 *
	_Atomic si4			size; // total allocated locks
	_Atomic si4			top_idx; // last non-empty lock in list
} FLOCK_LIST_m13;

typedef struct { // multiple thread access
	_Atomic(void *)		address;
	_Atomic ui8		requested_bytes;
	_Atomic ui8		actual_bytes; // actual bytes allocated => may be more than were requested
	const si1		*alloc_function;
	const si1		*free_function;
	si4			alloc_line;
	si4			free_line;
	si1			alloc_thread_name[PROC_THREAD_NAME_LEN_DEFAULT_m13];  // keep name because thread may be gone
	si1			free_thread_name[PROC_THREAD_NAME_LEN_DEFAULT_m13];  // keep name because thread may be gone
	ui8			alloc_thread_id;
	ui8			free_thread_id;
} AT_ENTRY_m13;

typedef struct { // multiple thread access
	pthread_mutex_t_m13		mutex;
	_Atomic(AT_ENTRY_m13 *)		entries; // no secondary indirection
	_Atomic si8			size; // total allocated entries
	_Atomic si4			top_idx; // last non-empty entry in list
} AT_LIST_m13; // global

typedef struct { // multiple thread access
	_Atomic pid_t_m13	_id; // thread id
	_Atomic pid_t_m13	_pid; // parent thread id
	pthread_t_m13 		thread; // pthread_t_m13 (pointer would have been preferable because a pthread_t can be a few kB on Linux/MacOs, but can't guarantee the original structure still exists)
} THREAD_ENTRY_m13;

typedef struct { // multiple thread access
	pthread_mutex_t_m13		mutex;
	_Atomic(THREAD_ENTRY_m13 *)	entries; // no secondary indirection
	_Atomic si8			size; // total allocated entries
	_Atomic si4			top_idx; // last non-empty entry in list
} THREAD_LIST_m13;

typedef struct {
	si1		path[PATH_BYTES_m13];
	si1		name[NAME_BYTES_m13];
	ui1		version_major;
	ui1		version_minor;
	FILE_TIMES_m13	file_times;
} APP_INFO_m13;

typedef struct {
	pthread_mutex_t_m13		mutex;
// Application Info
	APP_INFO_m13			*app_info;
// Tables
	GLOBAL_TABLES_m13		*tables;
// Behavior Stacks (thread local)
	BEHAVIOR_STACK_LIST_m13		*behavior_stack_list;
// Function Stacks (thread local)
	FUNCTION_STACK_LIST_m13		*function_stack_list;
// Process Globals (thread local)
	PROC_GLOBS_LIST_m13		*proc_globs_list;
// File Locking (global)
	FLOCK_LIST_m13			*file_lock_list;
// Allocation Tracking (global)
	THREAD_LIST_m13			*thread_list;
// Allocation Tracking (global)
	AT_LIST_m13			*AT_list;
// Record Filters (global default)
	si4 				*record_filters; // signed, "NULL terminated" array version of MED record type codes to include or exclude when reading records.
						  // The terminal entry is NO_TYPE_CODE_m13 (== zero). NULL or no filter codes includes all records (== no filters).
						  // filter modes: match positive: include
						  //		 match negative: exclude
						  //		 no match:
						  //			all filters positive: exclude
						  //			else: include
						  // Note: as type codes are composed of ascii bytes values (< 0x80), it is always possible to make them negative without promotion.
 // Miscellaneous
	si1				cwd[PATH_BYTES_m13]; // current working directory (periodically auto-cleared)
	si1				temp_dir[PATH_BYTES_m13]; // system temp directory (periodically auto-cleared)
	si1				temp_file[PATH_BYTES_m13]; // full path to temp file (i.e. incudes temp_dir)
								  	// not thread safe => use G_unique_temp_file_m13() in threaded applications
	ui4				file_creation_umask;
	pid_t_m13			main_id;  // process thread id (not necessarily same as process id)
	ERR_m13				error; // causal error
	tern				threading; // global default, used to set process globals default
	BEHAVIOR_m13			default_behavior;
	si1				file_lock_mode; // enable global file locking
	const si1			*file_lock_timeout; // blocking timeout (as string)  [nap_m13() form]
	ui4				CRC_mode;
	tern				access_times; // record times of each structure & file access
	tern				write_sorted_records; // if records unsorted, sort & re-write
	tern				write_corrected_headers; // if files closed without header update, n_entries may be zero (if TRUE_m13, write corrected header when encountered)
	tern				update_file_system_names; // if session or channel file system name differ from higher level names, rename lower level files & directories
	tern				update_header_names; // if session or channel file system name differs from universal header, update affected universal headers (requires update_file_system_names to be TRUE_m13)
	tern				update_MED_version; // if file MED version is not current, update affected files
	tern				update_parity; // update parity on write, if exists (e.g. updating header names or MED version; best turned off & manually batched on data conversion or acquisition)
	tern				increase_priority; // increase process priority if PROC_increase_priority_m13() is called
} GLOBALS_m13;

// Universal Header Structure
typedef struct {
 // start robust mode region
	ui4		header_CRC; // CRC of the universal header after this field
	ui4		body_CRC; // CRC of the entire file after the universal header
	union { // "segment_end_time" used in code when that is it's meaning, for clarity
			si8	file_end_time;
			si8	segment_end_time;
	};
	si8		n_entries;
	ui4		maximum_entry_size;
 // end robust mode region
	si4		segment_number;
	union {
		struct {
			si1	type_string[TYPE_BYTES_m13];
			ui1	MED_version_major;
			ui1	MED_version_minor;
			ui1	byte_order_code;
		};
		struct {
			ui4	type_code;
			si1	type_string_terminal_zero; // not used - here for clarity
		};
	};
	si8		session_start_time;
	union { // "segment_start_time" used in code when that is it's meaning, for clarity
			si8	file_start_time;
			si8	segment_start_time;
	};
	si1		session_name[NAME_BYTES_m13]; // utf8[63], base name only, no extension
	si1		channel_name[NAME_BYTES_m13]; // utf8[63], base name only, no extension
	ui1		supplementary_protected_region[UH_SUPPLEMENTARY_PROTECTED_REGION_BYTES_m13];
	ui8		session_UID; // session UID of originating data set
	ui8		channel_UID; // channel UID of originating data set
	ui8		segment_UID; // segment UID of originating data set
	ui8		file_UID; // unique to current file
	ui8		provenance_UID; // file UID of originating file
	ui1		level_1_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m13];
	ui1		level_2_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m13];
	ui1		level_3_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m13];
	ui4		video_data_file_number; // MED 1.1 and above
	tern		live; // file currently being acquired
	tern		ordered; // MED 1.1 and above
	tern		expanded_passwords; // MED 1.1 and above
	ui1		encryption_rounds; // MED 1.1 and above
	si1		encryption_1; // MED 1.1 and above
	si1		encryption_2; // MED 1.1 and above
	si1		encryption_3; // MED 1.1 and above
	ui1		protected_region[UH_PROTECTED_REGION_BYTES_m13];
	ui1		discretionary_region[UH_DISCRETIONARY_REGION_BYTES_m13];
} UH_m13;

// Metadata Structures
typedef struct {
	si1	level_1_password_hint[PASSWORD_HINT_BYTES_m13];
	si1	level_2_password_hint[PASSWORD_HINT_BYTES_m13];
	si1	anonymized_subject_ID[METADATA_ANONYMIZED_SUBJECT_ID_BYTES_m13]; // utf8[63], MED 1.1 & above (moved from universal header)
	ui1	protected_region[METADATA_SECTION_1_PROTECTED_REGION_BYTES_m13];
	ui1	discretionary_region[METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m13];
} METADATA_SECTION_1_m13;

typedef struct {
 // channel type independent fields
	si1	session_description[METADATA_SESSION_DESCRIPTION_BYTES_m13]; // utf8[511]
	si1	channel_description[METADATA_CHANNEL_DESCRIPTION_BYTES_m13]; // utf8[255]
	si1	segment_description[METADATA_SEGMENT_DESCRIPTION_BYTES_m13]; // utf8[255]
	si1	equipment_description[METADATA_EQUIPMENT_DESCRIPTION_BYTES_m13]; // utf8[510]
	si4	acquisition_channel_number;
 // channel type specific fields
	si1	reference_description[TS_METADATA_REFERENCE_DESCRIPTION_BYTES_m13]; // utf8[255]
	sf8	sampling_frequency;
	sf8	low_frequency_filter_setting;
	sf8	high_frequency_filter_setting;
	sf8	notch_filter_frequency_setting;
	sf8	AC_line_frequency;
	sf8	amplitude_units_conversion_factor;
	si1	amplitude_units_description[TS_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m13]; // utf8[31]
	sf8	time_base_units_conversion_factor;
	si1	time_base_units_description[TS_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m13]; // utf8[31]
	si8	session_start_sample_number; // session, not segment relative
	si8	number_of_samples;
	si8	number_of_blocks;
	si8	maximum_block_bytes;
	ui4	maximum_block_samples;
	ui4	maximum_block_keysample_bytes;
	sf8	maximum_block_duration;
	si8	number_of_discontinuities;
	si8	maximum_contiguous_blocks;
	si8	maximum_contiguous_block_bytes;
	si8	maximum_contiguous_samples;
	ui1	protected_region[TS_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m13];
	ui1	discretionary_region[TS_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m13];
} TS_METADATA_SECTION_2_m13;

typedef struct {
 // type-independent fields
	si1	session_description[METADATA_SESSION_DESCRIPTION_BYTES_m13];	  // utf8[511]
	si1	channel_description[METADATA_CHANNEL_DESCRIPTION_BYTES_m13];	  // utf8[511]
	si1	segment_description[METADATA_SEGMENT_DESCRIPTION_BYTES_m13];	  // utf8[511]
	si1	equipment_description[METADATA_EQUIPMENT_DESCRIPTION_BYTES_m13];  // utf8[510]
	si4	acquisition_channel_number;
 // type-specific fields
	sf8	time_base_units_conversion_factor;
	si1	time_base_units_description[VID_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m13]; // utf8[31]
	si8	session_start_frame_number; // session, not segment relative
	si8	number_of_frames;
	sf8	frame_rate;
	si8	number_of_clips;
	si8	maximum_clip_bytes;
	ui4	maximum_clip_frames;
	si4	number_of_video_files;
	sf8	maximum_clip_duration;
	si8	number_of_discontinuities;
	si8	maximum_contiguous_clips;
	si8	maximum_contiguous_clip_bytes;
	si8	maximum_contiguous_frames;
	ui4	horizontal_pixels;
	ui4	vertical_pixels;
	si1	video_format[VID_METADATA_VIDEO_FORMAT_BYTES_m13];   // utf8[31]
	ui1	protected_region[VID_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m13];
	ui1	discretionary_region[VID_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m13];
} VID_METADATA_SECTION_2_m13;

// All metadata section substructures are the same sizes
typedef union {
	ui1				section_2[METADATA_SECTION_2_BYTES_m13];
	TS_METADATA_SECTION_2_m13	time_series_section_2;
	VID_METADATA_SECTION_2_m13	video_section_2;
} METADATA_SECTION_2_m13;

typedef struct {
	si8	recording_time_offset;
	DAYLIGHT_TIME_CHANGE_CODE_m13	daylight_time_start_code; // si1[8] / si8
	DAYLIGHT_TIME_CHANGE_CODE_m13	daylight_time_end_code; // si1[8] / si8
	si1	standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m13]; // ascii[8]
	si1	standard_timezone_string[TIMEZONE_STRING_BYTES_m13]; // ascii[31]
	si1	daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m13]; // ascii[8]
	si1	daylight_timezone_string[TIMEZONE_STRING_BYTES_m13]; // ascii[31]
	si1	subject_name_1[METADATA_SUBJECT_NAME_BYTES_m13]; // utf8[31]
	si1	subject_name_2[METADATA_SUBJECT_NAME_BYTES_m13]; // utf8[31]
	si1	subject_name_3[METADATA_SUBJECT_NAME_BYTES_m13]; // utf8[31]
	si1	subject_ID[METADATA_SUBJECT_ID_BYTES_m13]; // utf8[31]
	si1	recording_country[METADATA_RECORDING_LOCATION_BYTES_m13]; // utf8[63]
	si1	recording_territory[METADATA_RECORDING_LOCATION_BYTES_m13]; // utf8[63]
	si1	recording_locality[METADATA_RECORDING_LOCATION_BYTES_m13]; // utf8[63]
	si1	recording_institution[METADATA_RECORDING_LOCATION_BYTES_m13]; // utf8[63]
	si1	geotag_format[METADATA_GEOTAG_FORMAT_BYTES_m13]; // ascii[31]
	si1	geotag_data[METADATA_GEOTAG_DATA_BYTES_m13]; // ascii[1023]
	si4	standard_UTC_offset;
	ui1	protected_region[METADATA_SECTION_3_PROTECTED_REGION_BYTES_m13];
	ui1	discretionary_region[METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m13];
} METADATA_SECTION_3_m13;

#ifdef __cplusplus // c++ does not accept anonymous structures
typedef struct {
	METADATA_SECTION_1_m13		section_1;
	union {
		ui1				section_2[METADATA_SECTION_2_BYTES_m13];
		TS_METADATA_SECTION_2_m13	time_series_section_2;
		VID_METADATA_SECTION_2_m13	video_section_2;
	};
	METADATA_SECTION_3_m13		section_3;
} METADATA_m13;
#else // __cplusplus
typedef struct {
	METADATA_SECTION_1_m13		section_1;
	METADATA_SECTION_2_m13;
	METADATA_SECTION_3_m13		section_3;
} METADATA_m13;
#endif // standard C

// Record Structures
typedef struct REC_HDR_m13 { // struct name for medrec_m13.h interdependency
	ui4		record_CRC;
	ui4		total_record_bytes; // header + body bytes
	si8		start_time;
	union {
		struct {
			si1	type_string[TYPE_BYTES_m13];
			ui1	version_major;
			ui1	version_minor; // minor version == 0
			si1	encryption_level;
		};
		struct {
			ui4	type_code;
			si1	type_string_terminal_zero; // not used - here for clarity
		};
	};
} REC_HDR_m13;

typedef struct {
	si8		file_offset; // never negative: the record indices are not used to indicate discontinuities
	si8		start_time;
	union {
		struct {
			si1	type_string[TYPE_BYTES_m13];
			ui1	version_major;
			ui1	version_minor;
			si1	encryption_level;
		};
		struct {
			ui4	type_code;
			si1	type_string_terminal_zero; // not used - there for clarity
		};
	};
} REC_IDX_m13;

// Time Series Indices Structures
typedef struct {
	si8	file_offset; // negative values indicate discontinuity
	si8	start_time;
	si8	start_samp_num;
} TS_IDX_m13;

// Video Indices Structures
typedef struct {
	si8 	file_offset; // negative values indicate discontinuity
	si8	start_time;
	ui4	start_frame_num;
	ui4	vid_file_num;
} VID_IDX_m13;

typedef struct {
	si8	file_offset; // negative values indicate discontinuity (in time series & video indices)
	si8	start_time;
	ui1	pad[8];
} GEN_IDX_m13;

// All index structures are the same size, and have the same first two fields (hence GEN_IDX_m13)
typedef struct {
	union {
		REC_IDX_m13	record_index;
		TS_IDX_m13	time_series_index;
		VID_IDX_m13	video_index;
		GEN_IDX_m13	generic_index;
	};
} INDEX_m13;



//**********************************************************************************//
//************************** FPS (File Processing Struct) ************************//
//**********************************************************************************//

// Constants
#define FPS_BYTES_NO_ENTRY_m13			((si8) 0) // passed as n_bytes
#define FPS_ITEMS_NO_ENTRY_m13			((si8) 0) // passed as n_items
#define FPS_AUTO_BYTES_m13			((si8) -1) // passed as n_bytes
#define FPS_FULL_FILE_m13			((si8) 0x7FFFFFFFFFFFFFF9) // passed as offset (Note: value positive so not treated as discontinuity)
#define FPS_UH_ONLY_m13				((si8) 0x7FFFFFFFFFFFFFFA) // passed as offset (Note: value positive so not treated as discontinuity)
#define FPS_UH_OFFSET_m13			((si8) 0x7FFFFFFFFFFFFFFB) // passed as offset (Note: value positive so not treated as discontinuity)
#define FPS_APPEND_m13				((si8) 0x7FFFFFFFFFFFFFFC) // passed as offset (Note: value positive so not treated as discontinuity)
#define FPS_REL_START_m13			((si8) 0x7FFFFFFFFFFFFFFD) // passed as offset with rel_bytes vararg (Note: value positive so not treated as discontinuity)
#define FPS_REL_CURR_m13			((si8) 0x7FFFFFFFFFFFFFFE) // passed as offset with rel_bytes vararg (Note: value positive so not treated as discontinuity)
#define FPS_REL_END_m13				((si8) 0x7FFFFFFFFFFFFFFF) // passed as offset with rel_bytes vararg (Note: value positive so not treated as discontinuity)
#define FPS_REL_OFFSET_m13(x)			( ((x) > FPS_APPEND_m13) ? TRUE_m13 : FALSE_m13 )

#define FPS_FILE_LENGTH_UNKNOWN_m13		-1
#define FPS_PROTOTYPE_TYPE_CODE_m13		TS_METADATA_TYPE_CODE_m13 // any metadata type would do
#define FPS_FD_CLOSED_m13			FILE_FD_CLOSED_m13
#define FPS_FD_NO_ENTRY_m13			FILE_FD_NO_ENTRY_m13
#define FPS_FD_EPHEMERAL_m13			FILE_FD_EPHEMERAL_m13

// Directives Flags
// Open Mode Flags: "append" implies "write";
#define FPS_DF_NO_FLAGS_m13			NO_FLAGS_m13
#define FPS_DF_READ_MODE_m13			((ui8) 1 << 0)
#define FPS_DF_WRITE_MODE_m13			((ui8) 1 << 1)
#define FPS_DF_APPEND_MODE_m13			(((ui8) 1 << 2) | FPS_DF_WRITE_MODE_m13)
#define FPS_DF_CREATE_MODE_m13			((ui8) 1 << 3)
#define FPS_DF_TRUNCATE_MODE_m13		((ui8) 1 << 4)
// End Open Mode Flags
#define FPS_DF_CLOSE_AFTER_OP_m13		((ui8) 1 << 5) // close after operation (read / write)
#define FPS_DF_FLUSH_AFTER_WRITE_m13		((ui8) 1 << 6)
#define FPS_DF_UPDATE_UH_m13			((ui8) 1 << 7) // update universal header with write
#define FPS_DF_LEAVE_DECRYPTED_m13		((ui8) 1 << 8) // if data would be encrypted before write, leave data in memory unencrypted
#define FPS_DF_FREE_CPS_m13			((ui8) 1 << 9)
#define FPS_DF_MMAP_m13				((ui8) 1 << 10)

// Open Mode Flag Groups
#define FPS_NO_OPEN_MODE_m13			((ui8) 0)
#define FPS_R_OPEN_MODE_m13			FPS_DF_READ_MODE_m13
#define FPS_RP_OPEN_MODE_m13			( FPS_DF_READ_MODE_m13 | FPS_DF_WRITE_MODE_m13 )
#define FPS_W_OPEN_MODE_m13			( FPS_DF_WRITE_MODE_m13 | FPS_DF_CREATE_MODE_m13 | FPS_DF_TRUNCATE_MODE_m13 ) // open for writing, truncate if exists, create if does not
#define FPS_WP_OPEN_MODE_m13			( FPS_DF_WRITE_MODE_m13 | FPS_DF_READ_MODE_m13 | FPS_DF_CREATE_MODE_m13 | FPS_DF_TRUNCATE_MODE_m13) // open for reading & random writing, truncate if exists, create if does not
#define FPS_WN_OPEN_MODE_m13			( FPS_DF_WRITE_MODE_m13  | FPS_DF_CREATE_MODE_m13 )// open for random writing, create if does not exist
#define FPS_WNP_OPEN_MODE_m13			( FPS_DF_WRITE_MODE_m13 | FPS_DF_READ_MODE_m13  | FPS_DF_CREATE_MODE_m13 ) // open for reading & random writing, create if does not exist
#define FPS_A_OPEN_MODE_m13			( FPS_DF_APPEND_MODE_m13  | FPS_DF_CREATE_MODE_m13 ) // open for appending, create if does not exist
#define FPS_AP_OPEN_MODE_m13			( FPS_DF_APPEND_MODE_m13 | FPS_DF_READ_MODE_m13 | FPS_DF_CREATE_MODE_m13 ) // open for reading & appending, create if does not exist
#define FPS_AC_OPEN_MODE_m13			( FPS_DF_APPEND_MODE_m13 | FPS_DF_CREATE_MODE_m13 | FPS_DF_TRUNCATE_MODE_m13) // open for appending, truncate if exists, create if does not
#define FPS_ACP_OPEN_MODE_m13			( FPS_DF_APPEND_MODE_m13 | FPS_DF_READ_MODE_m13 | FPS_DF_CREATE_MODE_m13 | FPS_DF_TRUNCATE_MODE_m13 ) // open for reading & appending, truncate if exists, create if does not

#define FPS_OPEN_MODE_MASK_m13			( FPS_DF_READ_MODE_m13 | FPS_DF_WRITE_MODE_m13 | FPS_DF_APPEND_MODE_m13 | FPS_DF_TRUNCATE_MODE_m13 )

// Open Mode Strings
// ("c" is for "clobber"; "n" is for "no clobber")
// ("clobber" == "truncate" => can't use 't' because Windows uses it for "text")
#define FPS_R_OPEN_STR_m13			"r" // open for reading, fail if does not exist
#define FPS_RP_OPEN_STR_m13			"r+" // open for reading & random writing, fail if does not exist
#define FPS_W_OPEN_STR_m13			"w" // open for random writing, truncate if exists, create if does not
#define FPS_WP_OPEN_STR_m13			"w+" // open for reading & random writing, truncate if exists, create if does not
#define FPS_WN_OPEN_STR_m13			"wn" // open for random writing, create if does not exist  [truncate removed]
#define FPS_WNP_OPEN_STR_m13			"wn+" // open for reading & random writing, create if does not exist  [truncate removed]
#define FPS_A_OPEN_STR_m13			"a" // open for appending, create if does not exist
#define FPS_AP_OPEN_STR_m13			"a+" // open for reading & appending, create if does not exist
#define FPS_AC_OPEN_STR_m13			"ac" // open for appending, truncate if exists, create if does not  [truncate added]
#define FPS_ACP_OPEN_STR_m13			"ac+" // open for reading & appending, truncate if exists, create if does not  [truncate added]

#define FPS_READ_OPEN_STR_DEFAULT_m13		FPS_R_OPEN_STR_m13
#define FPS_WRITE_OPEN_STR_DEFAULT_m13		FPS_WN_OPEN_STR_m13
#define FPS_OPEN_STR_DEFAULT_m13		FPS_WNP_OPEN_STR_m13 // most flexible

// Directive Defaults
#define FPS_DIRECS_CLOSE_AFTER_OPERATION_DEFAULT_m13		FALSE_m13
#define FPS_DIRECS_FLUSH_AFTER_WRITE_DEFAULT_m13		TRUE_m13
#define FPS_DIRECS_UPDATE_UH_DEFAULT_m13			FALSE_m13
#define FPS_DIRECS_LEAVE_DECRYPTED_DEFAULT_m13			FALSE_m13
#define FPS_DIRECS_FREE_CMP_PROCESSING_STRUCT_DEFAULT_m13	TRUE_m13
#define FPS_DIRECS_MEMORY_MAP_DEFAULT_m13		 	FALSE_m13
#define FPS_DIRECS_READ_OPEN_MODE_DEFAULT_m13			FPS_R_OPEN_MODE_m13
#define FPS_DIRECS_READ_OPEN_STRING_DEFAULT_m13			FPS_R_OPEN_STRING_m13
#define FPS_DIRECS_WRITE_OPEN_MODE_DEFAULT_m13			FPS_WN_OPEN_MODE_m13
#define FPS_DIRECS_WRITE_OPEN_STRING_DEFAULT_m13		FPS_WN_OPEN_STRING_m13
#define FPS_DIRECS_OPEN_MODE_DEFAULT_m13			FPS_DIRECS_READ_OPEN_MODE_DEFAULT_m13  // default to read open (safest)


// Structures
typedef struct {
	ui8	flags;
} FPS_DIRECS_m13; // structure for future directives that may not work well as flags

// Parameters contain "mechanics" of FPS (mostly used internally by library functions)
typedef struct {
	si1			mode_str[6]; // open mode string (Windows systems: 'b' added by fopen_m13(), but not included in this string)
	tern			uh_read; // universal header has been read in
	tern			full_file_read; // full file has been read in
	si8			raw_data_bytes; // bytes in raw data array
	ui1			*raw_data; // universal header followed by data (in standard read - just region requested, in full file & mem map - matches media)
	struct CPS_m13		*cps; // for time series data FPSs
 // FILE_m13 pointer (contains standard FILE pointer)
	FILE_m13		*fp;
 // memory mapping
	ui4			mmap_block_bytes; // read size for memory mapped files (size data may be on different volumes, or even files within the same volume)
	ui4			mmap_n_blocks; // file system block in file == number of bits in bitmap
	ui8			*mmap_block_bitmap; // each bit represents block_bytes bytes; NULL if not memory mapping
} FPS_PARAMS_m13;

#ifdef __cplusplus
typedef struct {
	union {
		LH_m13			header; // in case just want the level header (type == GENERIC_TYPE_CODE_m13 => use universal header to get specific type)
		struct { // this struct replaces anonymous LH_m13 for C++
			union {
				struct {
					si1 	type_string[TYPE_BYTES_m13];
					ui1 	pad[3]; // enforce 8-byte alignment
				};
				struct {
					ui4 	type_code;
					si1	type_string_terminal_zero; // not used - here for clarity
					tern	allocated; // allocted on heap, independently (not en bloc)  [moved from flags - cleaner code]
					tern	names_differ;
				};
			};
			si1					*path; // NULL in proc_globs
			si1					*name; // NULL in proc_globs
			_Atomic(LH_m13 *)			parent; // NULL in proc_globs
			_Atomic(struct PROC_GLOBS_m13 *)	proc_globs; // self in proc_globs
			ui8					flags;
			_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
		};
	};
	si1				local_path[PATH_BYTES_m13]; // full path from root including extension
	si1				local_name[MAX_NAME_BYTES_m13]; // full path from root including extension
	UH_m13				*uh; // points to base of raw_data array (even in video data files)
	FPS_DIRECS_m13	 		direcs;
	FPS_PARAMS_m13	 		params;
	union {			  	// the MED file types (set to point to current data (just read, or to write)
		METADATA_m13		*metadata;
		REC_IDX_m13		*rec_inds;
		ui1			*rec_data;
		TS_IDX_m13		*ts_inds;
		ui1			*ts_data; // compressed data (not modified), CPS block header is modifiable pointer within this array
		VID_IDX_m13		*vid_inds;
		void			*vid_data;
		ui1			*data_ptrs; // generic name for all of the above (dissociable from raw data array / universal header, if needed)
	};
	si8				n_items; // items in current read/write, not necessarily the whole file
} FPS_m13;
#else // __cplusplus
typedef struct {
	union {
		LH_m13			header; // in case just want the level header (type == GENERIC_TYPE_CODE_m13 => use universal header to get specific type)
		LH_m13; 		// anonymous LH_m13
	};
	si1				local_path[PATH_BYTES_m13]; // full path to level (pointed to by level_header path)
	si1				local_name[MAX_NAME_BYTES_m13]; // base name of level (pointed to by level_header name)
	UH_m13				*uh; // points to base of raw_data array
	FPS_DIRECS_m13	 		direcs;
	FPS_PARAMS_m13	 		params;
	union {				// the MED file types (set to point to current data (just read, or to write)
		METADATA_m13		*metadata;
		REC_IDX_m13		*rec_inds;
		ui1			*rec_data;
		TS_IDX_m13		*ts_inds;
		ui1			*ts_data; // compressed data (not modified), CPS block header is modifiable pointer within this array
		VID_IDX_m13		*vid_inds;
		void			*vid_data;
		ui1			*data_ptrs; // generic name for all of the above (dissociable from raw data array / universal header, if needed)
	};
	si8				n_items; // items in current read/write, not necessarily the whole file
} FPS_m13;
#endif // standard C

// Prototypes
FPS_m13		*FPS_clone_m13(FPS_m13 *proto_fps, const si1 *path, si8 n_bytes, si8 copy_bytes, LH_m13 *parent);
tern		FPS_close_m13(FPS_m13 *fps);
si4		FPS_compare_times_m13(const void *a, const void *b);
tern		FPS_free_m13(FPS_m13 **fps);
FPS_m13		*FPS_init_m13(FPS_m13 *fps, const si1 *path, const si1 *mode_str, si8 n_bytes, LH_m13 *parent);
FPS_DIRECS_m13	*FPS_init_direcs_m13(FPS_DIRECS_m13 *direcs);
FPS_PARAMS_m13	*FPS_init_params_m13(FPS_PARAMS_m13 *params);
tern		FPS_is_open_m13(FPS_m13 *fps);
si8		FPS_mmap_read_m13(FPS_m13 *fps, si8 n_bytes);
FPS_m13		*FPS_open_m13(const si1 *path, const si1 *mode, si8 n_bytes, LH_m13 *parent, ...); // varargs(mode empty): const si1 *mode, ui8 fd_flags
FPS_m13 	*FPS_read_m13(FPS_m13 *fps, si8 offset, si8 n_bytes, si8 n_items, void *dest, ...); // varargs(fps invalid): const si1 *path, const si1 *mode, const si1 *password, LH *parent, ui8 lh_flags
												    // varargs(offset == FPS_REL_START/CURR/END): si8 rel_bytes
tern		FPS_realloc_m13(FPS_m13 *fps, si8 n_bytes);
tern		FPS_reopen_m13(FPS_m13 *fps, const si1 *mode);
si8		FPS_resolve_offset_m13(FPS_m13 *fps, si8 offset, ...);  // varargs(offset == FPS_REL_START/CURR/END): si8 rel_bytes
si8		FPS_seek_m13(FPS_m13 *fps, si8 offset, ...); // varargs(offset == FPS_REL_START/CURR/END): si8 rel_bytes
si8		FPS_set_direcs_from_lh_flags_m13(FPS_m13 *fps, ui8 lh_flags);
ui8		FPS_set_open_flags_m13(FPS_m13 *fps, const si1 *mode_str);
si1		*FPS_set_open_string_m13(FPS_m13 *fps, ui8 flags);
tern		FPS_set_pointers_m13(FPS_m13 *fps, si8 offset);
tern		FPS_show_m13(FPS_m13 *fps);
tern		FPS_show_direcs_m13(FPS_m13 *fps);
tern		FPS_show_params_m13(FPS_m13 *fps);
tern		FPS_sort_m13(FPS_m13 **fps_array, si4 n_fps);
tern		FPS_write_m13(FPS_m13 *fps, si8 offset, si8 n_bytes, si8 n_items, void *source, ...); // varargs(offset == FPS_REL_START/CURR/END): si8 rel_bytes



//**********************************************************************************//
//******************************** MED Structures ********************************//
//**********************************************************************************//

// Generally Useful Structures
typedef union {
	si1	ext[8];
	ui4	code;
} EXT_CODE_m13;

#ifdef __cplusplus
typedef struct {
	union {
		REC_HDR_m13	header; // in case just want the record header
		struct { // this replaces anonymous REC_HDR_m13 for C++
			ui4		record_CRC;
			ui4		total_record_bytes; // header + body bytes
			union { // anonymous union
				si8	time;
				si8	end_time;
			};
			union { // anonymous union
				struct {
					si1 type_string[TYPE_BYTES_m13];
					ui1 version_major;
					ui1 version_minor;
					si1 encryption_level;
				};
				struct {
					ui4 type_code;
					si1	type_string_terminal_zero;
				};
			};
		};
	};
	union {
		REC_Sgmt_v11_m13	body; // in case just want the record body
		struct { // this replaces anonymous REC_Sgmt_v11_m13 for C++
			si8		end_time;
			union {
				si8	start_sample_number;
				si8	start_frame_number;
			};
			union {
				si8	end_sample_number;
				si8	end_frame_number;
			};
			si4		segment_number;
			union {
				sf4	samp_freq;
				sf4	frame_rate;
			};
		};
	};
} Sgmt_REC_m13;
#else // __cplusplus
typedef struct {
	union {
		REC_HDR_m13	header; // in case just want the record header
		REC_HDR_m13; // anonymous REC_HDR_m13
	};
	union {
		REC_Sgmt_v11_m13	body; // in case just want the record body
		REC_Sgmt_v11_m13; // anonymous REC_Sgmt_v11_m13
	};
} Sgmt_REC_m13;
#endif // standard C
// NOTE: construction of Sgmt_REC_m13 in this way allows direct reading of Sgmt
// record headers & bodies into this structure (excluding the segment description)

#ifdef __cplusplus
typedef struct {
	union {
		LH_m13		header; // in case just want the level header
		struct { // this struct replaces anonymous LH_m13 for C++
			union {
				struct {
					si1 	type_string[TYPE_BYTES_m13];
					ui1 	pad[3]; // enforce 8-byte alignment
				};
				struct {
					ui4 	type_code;
					si1	type_string_terminal_zero; // not used - here for clarity
					tern	allocated; // allocted on heap, independently (not en bloc)  [moved from flags - cleaner code]
					tern	names_differ;
				};
			};
			si1					*path; // NULL in proc_globs
			si1					*name; // NULL in proc_globs
			_Atomic(LH_m13 *)			parent; // NULL in proc_globs
			_Atomic(struct PROC_GLOBS_m13 *)	proc_globs; // self in proc_globs
			ui8					flags;
			_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
		};
	};
	FPS_m13			*metadata_fps; // also used as prototype
	union {
		FPS_m13		*ts_data_fps;
		FPS_m13		*vid_data_fps;
	};
	union {
		FPS_m13		*ts_inds_fps;
		FPS_m13		*vid_inds_fps;
	};
	FPS_m13			*rec_data_fps;
	FPS_m13			*rec_inds_fps;
	si8			n_contigua;
	CONTIGUON_m13		*contigua;
	si1			local_path[PATH_BYTES_m13]; // full path to segment directory (including segment directory itself)
	si1			fs_name[SEG_NAME_BYTES_m13]; // stored here, no segment_name field in universal header
	si1			uh_name[SEG_NAME_BYTES_m13]; // stored here, no segment_name field in universal header
	SLICE_m13		slice;
} SEG_m13;
#else // __cplusplus
typedef struct {
	union {
		LH_m13		header; // in case just want the level header
		LH_m13; // anonymous LH_m13
	};
	FPS_m13			*metadata_fps; // also used as prototype
	union {
		FPS_m13		*ts_data_fps;
		FPS_m13		*vid_data_fps;
	};
	union {
		FPS_m13		*ts_inds_fps;
		FPS_m13		*vid_inds_fps;
	};
	FPS_m13			*rec_data_fps;
	FPS_m13			*rec_inds_fps;
	si8			n_contigua;
	CONTIGUON_m13		*contigua;
	si1			local_path[PATH_BYTES_m13]; // full path to segment directory (including segment directory itself)
	si1			fs_name[SEG_NAME_BYTES_m13]; // stored here, no segment_name field in universal header
	si1			uh_name[SEG_NAME_BYTES_m13]; // stored here, no segment_name field in universal header
	SLICE_m13		slice;
} SEG_m13;
#endif // standard C

#ifdef __cplusplus
typedef struct CHAN_m13 {
	union {
		LH_m13		header; // in case just want the level header
		struct { // this struct replaces anonymous LH_m13 for C++
			union {
				struct {
					si1 	type_string[TYPE_BYTES_m13];
					ui1 	pad[3]; // enforce 8-byte alignment
				};
				struct {
					ui4 	type_code;
					si1	type_string_terminal_zero; // not used - here for clarity
					tern	allocated; // allocted on heap, independently (not en bloc)  [moved from flags - cleaner code]
					tern	names_differ;
				};
			};
			si1					*path; //  points to local_path
			si1					*name; // points to local_fs_name or local_uh_name
			_Atomic(LH_m13 *)			parent;
			_Atomic(struct PROC_GLOBS_m13 *)	proc_globs;
			ui8					flags;
			_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
		};
	};
	FPS_m13			*metadata_fps; // used as prototype or ephemeral file, does not correspond to stored data
	FPS_m13			*rec_data_fps;
	FPS_m13			*rec_inds_fps;
	SEG_m13			**segs;
	Sgmt_REC_m13		*Sgmt_recs;
	si8			n_contigua;
	CONTIGUON_m13		*contigua;
	si1			local_path[PATH_BYTES_m13]; // full path to channel directory (including channel directory itself)
	si1			fs_name[NAME_BYTES_m13]; // name from file system
	si1			uh_name[NAME_BYTES_m13]; // name from universal header (if differs from file system name)
	SLICE_m13		slice;
} CHAN_m13;
#else // __cplusplus
typedef struct CHAN_m13 {
	union {
		LH_m13		header; // in case just want the level header
		LH_m13; // anonymous LH_m13
	};
	FPS_m13			*metadata_fps; // used as prototype or ephemeral file, does not correspond to stored data
	FPS_m13			*rec_data_fps;
	FPS_m13			*rec_inds_fps;
	SEG_m13			**segs;
	Sgmt_REC_m13		*Sgmt_recs;
	si8			n_contigua;
	CONTIGUON_m13		*contigua;
	si1			local_path[PATH_BYTES_m13]; // full path to channel directory (including channel directory itself)
	si1			fs_name[NAME_BYTES_m13]; // name from file system
	si1			uh_name[NAME_BYTES_m13]; // name from universal header (if differs from file system name, otherwise empty)
	SLICE_m13		slice;
} CHAN_m13;
#endif // standard C

#ifdef __cplusplus
typedef struct {
	union {
		LH_m13	header; // in case just want the level header
		struct { // this struct replaces anonymous LH_m13 in C++
			union {
				struct {
					si1 	type_string[TYPE_BYTES_m13];
					ui1 	pad[3]; // enforce 8-byte alignment
				};
				struct {
					ui4 	type_code;
					si1	type_string_terminal_zero; // not used - here for clarity
					tern	allocated; // allocted on heap, independently (not en bloc)  [moved from flags - cleaner code]
					tern	names_differ;
				};
			};
			si1					*path; // NULL in proc_globs
			si1					*name; // NULL in proc_globs
			_Atomic(LH_m13 *)			parent; // NULL in proc_globs
			_Atomic(struct PROC_GLOBS_m13 *)	proc_globs; // self in proc_globs
			ui8					flags;
			_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
		};
	};
	FPS_m13		**rec_data_fps;
	FPS_m13		**rec_inds_fps;
	Sgmt_REC_m13	*Sgmt_recs;
	si1		local_path[PATH_BYTES_m13]; // full path to segmented session records directory (including directory itself)
	SLICE_m13	slice;
} SSR_m13;
#else // __cplusplus
typedef struct {
	union {
		LH_m13	header; // in case just want the level header
		LH_m13; // anonymous LH_m13
	};
	FPS_m13		**rec_data_fps;
	FPS_m13		**rec_inds_fps;
	Sgmt_REC_m13	*Sgmt_recs;
	si1		local_path[PATH_BYTES_m13]; // full path to channel directory (including channel directory itself)
	si1		fs_name[NAME_BYTES_m13]; // name from file system
	si1		uh_name[NAME_BYTES_m13]; // name from universal header (if differs from file system name)
	SLICE_m13	slice;
} SSR_m13;
#endif // standard C

#ifdef __cplusplus
typedef struct {
	union {
		LH_m13		header; // in case just want the level header
		struct { // this struct replaces anonymous LH_m13 in C++
			union {
				struct {
					si1 	type_string[TYPE_BYTES_m13];
					ui1 	pad[3]; // enforce 8-byte alignment
				};
				struct {
					ui4 	type_code;
					si1	type_string_terminal_zero; // not used - here for clarity
					tern	allocated; // allocted on heap, independently (not en bloc)  [moved from flags - cleaner code]
					tern	names_differ;
				};
			};
			si1					*path; // NULL in proc_globs
			si1					*name; // NULL in proc_globs
			_Atomic(LH_m13 *)			parent; // NULL in proc_globs
			_Atomic(struct PROC_GLOBS_m13 *)	proc_globs; // self in proc_globs
			ui8					flags;
			_Atomic si8				access_time; // uutc of last use of this structure by the calling program (updated by open, read, & write functions)
		};
	};
	FPS_m13			*ts_metadata_fps; // used as prototype or ephemeral file, does not correspond to stored data
	FPS_m13			*vid_metadata_fps; // used as prototype or ephemeral file, does not correspond to stored data
	si4			n_ts_chans;
	CHAN_m13		**ts_chans;
	si4			n_vid_chans;
	CHAN_m13		**vid_chans;
	FPS_m13			*rec_data_fps;
	FPS_m13			*rec_inds_fps;
	SSR_m13			*ssr;
	Sgmt_REC_m13		*Sgmt_recs;
	si8			n_contigua;
	CONTIGUON_m13		*contigua;
	SLICE_m13		slice;
} SESS_m13;
#else // __cplusplus
typedef struct {
	union {
		LH_m13		header; // in case just want the level header
		LH_m13; // anonymous LH_m13
	};
	FPS_m13			*ts_metadata_fps; // used as prototype or ephemeral file, does not correspond to stored data
	FPS_m13			*vid_metadata_fps; // used as prototype or ephemeral file, does not correspond to stored data
	si4			n_ts_chans;
	CHAN_m13		**ts_chans;
	si4			n_vid_chans;
	CHAN_m13		**vid_chans;
	FPS_m13			*rec_data_fps;
	FPS_m13			*rec_inds_fps;
	SSR_m13			*ssr;
	Sgmt_REC_m13		*Sgmt_recs;
	si8			n_contigua;
	CONTIGUON_m13		*contigua;
	SLICE_m13		slice;
} SESS_m13;
#endif // standard C

// Miscellaneous structures that depend on above
typedef struct {
	si1			MED_dir[PATH_BYTES_m13];
	ui8			flags;
	LH_m13			*MED_struct; // SESS_m13, SSR_m13, CHAN_m13, or SEG_m13 pointer (used to pass & return)
	LH_m13			*parent; // SESS_m13 or CHAN_m13 pointer
	SLICE_m13		*slice;
	const si1		*password;
} READ_MED_THREAD_INFO_m13;

typedef struct {
	CHAN_m13	*chan;
	si4		acq_num;
} ACQ_NUM_SORT_m13;



//**********************************************************************************//
//************************** GENERAL (G) MED Functions ***************************//
//**********************************************************************************//


// Prototypes
CHAN_m13		*G_active_channel_m13(SESS_m13 *sess, si1 channel_type);
void			G_add_behavior_exec_m13(const si1 *function, const si4 line, ui4 code);
ui4 			G_add_level_extension_m13(si1 *directory_name);
tern			G_all_zeros_m13(ui1 *bytes, si4 field_length);
CHAN_m13		*G_alloc_channel_m13(CHAN_m13 *chan, FPS_m13 *proto_fps, const si1 *path, LH_m13 *parent, si4 n_segs, tern chan_recs, tern seg_recs);
SEG_m13			*G_alloc_segment_m13(SEG_m13 *seg, FPS_m13 *proto_fps, const si1 *path, LH_m13 *parent, si4 seg_num, tern seg_recs);
SESS_m13		*G_alloc_session_m13(FPS_m13 *proto_fps, const si1 *path, si4 n_ts_chans, si4 n_vid_chans, si4 n_segs, si1 **ts_chan_names, si1 **vid_chan_names, tern sess_recs, tern seg_sess_recs, tern chan_recs, tern seg_recs);
void 			G_apply_recording_time_offset_m13(si8 *time, si8 recording_time_offset);
si1 			*G_base_name_m13(LH_m13 *lh, const si1 *path, si1 *base_name);
BEHAVIOR_STACK_m13	*G_behavior_stack_m13(void);
void			G_behavior_stack_reset_exec_m13(const si1 *function, si4 line, ui4 code);
si1			*G_behavior_string_m13(ui4 behavior_code, si1 *behavior_string);
si8			G_build_contigua_m13(LH_m13 *lh);
Sgmt_REC_m13		*G_build_Sgmt_records_m13(LH_m13 *lh, si4 search_mode, ui4 *source_type);
si8			G_bytes_for_items_m13(FPS_m13 *fps, si8 *n_items, si8 offset);
tern 			G_calculate_indices_CRCs_m13(FPS_m13 *fps);
tern			G_calculate_metadata_CRC_m13(FPS_m13 *fps);
tern			G_calculate_record_data_CRCs_m13(FPS_m13 *fps);
tern			G_calculate_time_series_data_CRCs_m13(FPS_m13 *fps);
tern			G_calculate_video_data_CRCs_m13(FPS_m13 *fps);
CHAN_m13		*G_change_index_chan_m13(SESS_m13 *sess, CHAN_m13 *chan, const si1 *chan_name, si1 chan_type);
ui4			G_channel_type_from_path_m13(const si1 *path);
tern			G_check_char_type_m13(void);
tern			G_check_file_list_m13(si1 **file_list, si4 n_files);
tern			G_check_file_system_m13(const si1 *file_system_path, si4 is_cloud, ...); // varargs (is_cloud == TRUE_m13): const si1 *cloud_directory, const si1 *cloud_service_name, const si1 *cloud_utilities_directory
tern 			G_check_password_m13(const si1 *password);
si4			G_check_segment_map_m13(SLICE_m13 *slice, SESS_m13 *sess);
void			G_clear_error_m13(void);
tern			G_clear_terminal_m13(void);
si4			G_compare_acq_nums_m13(const void *a, const void *b);
si4 			G_compare_record_index_times(const void *a, const void *b);
tern			G_condition_password_m13(const si1 *password, si1 *password_bytes, tern expand_password);
tern			G_condition_slice_m13(SLICE_m13 *slice, LH_m13 *lh);
tern			G_condition_timezone_info_m13(TIMEZONE_INFO_m13 *tz_info);
tern			G_correct_universal_header_m13(FPS_m13 *fps);
ui4			G_current_behavior_m13(void); // returns behavior code
BEHAVIOR_m13		*G_current_behavior_entry_m13(void); // returns pointer to BEHAVIOR_m13 struct (useful for debugging)
si8			G_current_uutc_m13(void);
si4			G_days_in_month_m13(si4 month, si4 year);
tern 			G_decrypt_metadata_m13(FPS_m13 *fps);
tern 			G_decrypt_record_data_m13(FPS_m13 *fps, ...); // varargs (fps == NULL): REC_HDR_m13 *rh, si8 n_records (used to decrypt Sgmt_records arrays)
tern 			G_decrypt_time_series_data_m13(FPS_m13 *fps);
tern 			G_decrypt_video_data_m13(FPS_m13 *fps);
void			G_delete_behavior_stack_m13(void);
void			G_delete_function_stack_m13(void);
si4			G_DST_offset_m13(si8 uutc);
tern			G_encrypt_metadata_m13(FPS_m13 *fps);
tern			G_encrypt_record_data_m13(FPS_m13 *fps);
tern 			G_encrypt_time_series_data_m13(FPS_m13 *fps);
tern			G_enter_ascii_password_m13(si1 *password, const si1 *prompt, tern confirm_no_entry, sf8 timeout_secs, tern create_password);
void			G_error_clear_m13(void);
si4			G_error_code_m13(void);
void 			G_error_message_m13(const si1 *fmt, ...);
si1 			G_exists_m13(const si1 *path);
tern			G_expand_password_m13(const si1 *password, si1 *password_bytes);
si8			G_file_length_m13(FILE_m13 *fp, const si1 *path);
si1			**G_file_list_m13(si1 **file_list, si4 *n_files, const si1 *enclosing_directory, const si1 *name, const si1 *extension, ui4 flags);
FILE_TIMES_m13		*G_file_times_m13(FILE_m13 *fp, const si1 *path, FILE_TIMES_m13 *ft, tern set_time);
tern			G_fill_empty_password_bytes_m13(si1 *password_bytes);
CONTIGUON_m13		*G_find_discontinuities_m13(LH_m13 *lh, si8 *n_contigua);
si8			G_find_index_m13(SEG_m13 *seg, si8 target, ui4 mode);
si1			*G_find_timezone_acronym_m13(si1 *timezone_acronym, si4 standard_UTC_offset, si4 DST_offset);
si1			*G_find_metadata_file_m13(const si1 *path, si1 *md_path);
si8			G_find_record_index_m13(FPS_m13 *rec_inds_fps, si8 target_time, ui4 mode, si8 low_idx);
si8 			G_frame_number_for_uutc_m13(LH_m13 *lh, si8 target_uutc, ui4 mode, ...); // varargs (lh == NULL): si8 ref_frame_number, si8 ref_uutc, sf8 frame_rate
tern			G_free_channel_m13(CHAN_m13 **chan_ptr);
void			G_free_global_tables_m13(void);
void			G_free_globals_m13(tern cleanup_for_exit);
tern			G_free_segment_m13(SEG_m13 **seg_ptr);
tern			G_free_session_m13(SESS_m13 **sess_ptr);
tern			G_free_ssr_m13(SSR_m13 **ssr_ptr);
tern			G_full_path_m13(const si1 *path, si1 *full_path);
FUNCTION_STACK_m13	*G_function_stack_m13(pid_t_m13 _id);
si1			**G_generate_numbered_names_m13(si1 **names, const si1 *prefix, si4 n_names);
tern			G_generate_password_data_m13(FPS_m13 *fps, const si1 *L1_pw, const si1 *L2_pw, const si1 *L3_pw, const si1 *L1_pw_hint, const si1 *L2_pw_hint);
si8			G_generate_recording_time_offset_m13(si8 recording_start_time_uutc);
si1			*G_generate_segment_name_m13(si1 *segment_name, FPS_m13 *fps);
ui8			G_generate_UID_m13(ui8 *uid);
tern			G_include_record_m13(ui4 type_code, si4 *record_filters);
tern			G_init_global_tables_m13(tern init_all_tables);
tern			G_init_globals_m13(tern init_all_tables, const si1 *app_path, ...); // varargs(app_path): ui4 version_major, ui4 version_minor
tern			G_init_medlib_m13(tern init_all_tables, const si1 *app_path, ...); // varargs(app_path): ui4 version_major, ui4 version_minor;
tern			G_init_metadata_m13(FPS_m13 *fps, tern init_for_update);
SLICE_m13		*G_init_slice_m13(SLICE_m13 *slice);
tern			G_init_timezone_tables_m13(void);
tern			G_init_universal_header_m13(FPS_m13 *fps, ui4 type_code, tern generate_file_UID, tern originating_file);
tern			G_is_level_header_m13(void *ptr);
si8			G_items_for_bytes_m13(FPS_m13 *fps, si8 *n_bytes);
ui4			G_level_m13(const si1 *full_file_name, ui4 *input_type_code);
tern			G_location_info_m13(LOCATION_INFO_m13 *loc_info, const si1 *ip_str, const si1 *ipinfo_token, tern set_timezone_globals, tern prompt);
tern			G_MED_file_m13(ui4 type_code);
ui4			G_MED_path_components_m13(const si1 *path, si1 *MED_dir, si1* MED_name);
ui4			G_MED_type_code_from_string_m13(const si1 *string);
const si1		*G_MED_type_string_from_code_m13(ui4 code);
tern			G_merge_metadata_m13(FPS_m13 *md_fps_1, FPS_m13 *md_fps_2, FPS_m13 *merged_md_fps);
tern			G_merge_universal_headers_m13(FPS_m13 *fps_1, FPS_m13 *fps_2, FPS_m13 *merged_fps);
void 			G_message_m13(const si1 *fmt, ...);
CHAN_m13		*G_open_channel_m13(CHAN_m13 *chan, SLICE_m13 *slice, const si1 *chan_path, LH_m13 *parent, ui8 flags, const si1 *password);
pthread_rval_m13	G_open_channel_thread_m13(void *ptr);
tern			G_open_records_m13(LH_m13 *lh, ...);  // varagrgs(level == ssr): si4 seg_num
SSR_m13			*G_open_seg_sess_recs_m13(SESS_m13 *sess);
SEG_m13			*G_open_segment_m13(SEG_m13 *seg, SLICE_m13 *slice, const si1 *seg_path, LH_m13 *parent, ui8 flags, const si1 *password);
pthread_rval_m13	G_open_segment_thread_m13(void *ptr);
SESS_m13		*G_open_session_m13(SESS_m13 *sess, SLICE_m13 *slice, void *file_list, si4 list_len, ui8 flags, const si1 *password, const si1 *index_channel_name);
si8			G_pad_m13(ui1 *buffer, si8 content_len, ui4 alignment);
tern			G_path_parts_m13(const si1 *full_file_name, si1 *path, si1 *name, si1 *extension);
void			G_pop_behavior_exec_m13(const si1 *function, const si4 line);
void			G_pop_function_exec_m13(const si1 *function, const si4 line);
tern			G_proc_error_get_m13(LH_m13 *lh);
void			G_proc_error_set_m13(LH_m13 *lh, tern state);
PROC_GLOBS_m13		*G_proc_globs_m13(LH_m13 *lh);
void			G_proc_globs_delete_m13(LH_m13 *lh);
PROC_GLOBS_m13		*G_proc_globs_find_m13(LH_m13 *lh);
PROC_GLOBS_m13		*G_proc_globs_init_m13(PROC_GLOBS_m13 *pg);
PROC_GLOBS_m13		*G_proc_globs_new_m13(LH_m13 *lh);
tern			G_process_password_data_m13(FPS_m13 *fps, const si1 *unspecified_pw);
tern			G_propagate_flags_m13(LH_m13 *lh, ui8 new_flags);
void			G_push_behavior_exec_m13(const si1 *function, const si4 line, ui4 code);
void			G_push_function_exec_m13(const si1 *function);
tern			G_rates_vary_m13(SESS_m13 *sess);
CHAN_m13		*G_read_channel_m13(CHAN_m13 *chan, SLICE_m13 *slice, ...); // varargs(chan == NULL): const si1 *chan_path, LH_m13 *parent, ui8 lh_flags, const si1 *password, const si1 *index_channel_name
pthread_rval_m13	G_read_channel_thread_m13(void *ptr);
si4			G_read_cs_file_m13(const si1 *cs_file_name, si4 n_available_channels, si4 **map, si4 **reverse_map, si1 ***names, sf8 **decimation_frequencies, ui4 **block_samples, si1 ***descriptions);
LH_m13			*G_read_data_m13(LH_m13 *lh, SLICE_m13 *slice, ...); // varargs(lh == NULL): const si1 *file_list, si4 list_len, ui8 lh_flags, const si1 *password, const si1 *index_channel_name
void			G_read_medlibrc_m13(const si1 *app_path);
si8			G_read_records_m13(LH_m13 *lh, SLICE_m13 *slice, ...); // varargs(level->type_code == LH_SSR_m13): si4 seg_num
SEG_m13			*G_read_segment_m13(SEG_m13 *seg, SLICE_m13 *slice, ...); // varargs(seg == NULL): const si1 *seg_path, LH_m13 *parent, ui8 lh_flags, const si1 *password
pthread_rval_m13	G_read_segment_thread_m13(void *ptr);
SESS_m13		*G_read_session_m13(SESS_m13 *sess, SLICE_m13 *slice, ...); // varargs(sess == NULL): void *file_list, si4 list_len, ui8 lh_flags, const si1 *password
si8			G_read_time_series_data_m13(SEG_m13 *seg, SLICE_m13 *slice);
UH_m13			*G_read_universal_header_m13(const si1 *path, UH_m13 *uh);
tern			G_recover_passwords_m13(const si1 *L3_password, UH_m13* universal_header);
void			G_remove_behavior_exec_m13(const si1 *function, const si4 line, ui4 code);
void			G_remove_recording_time_offset_m13(si8 *time, si8 recording_time_offset);
tern			G_reset_metadata_for_update_m13(FPS_m13 *fps);
si8			G_sample_number_for_uutc_m13(LH_m13 *lh, si8 target_uutc, ui4 mode, ...); // varargs(lh == NULL): si8 ref_sample_number, si8 ref_uutc, sf8 sampling_frequency
si4			G_search_mode_m13(SLICE_m13 *slice);
si4			G_search_Sgmt_records_m13(Sgmt_REC_m13 *Sgmt_records, SLICE_m13 *slice, ui4 search_mode);
si4			G_segment_for_frame_number_m13(LH_m13 *lh, si8 target_sample);
si4			G_segment_for_path_m13(const si1 *path);
si4			G_segment_for_sample_number_m13(LH_m13 *lh, si8 target_sample);
si4			G_segment_for_uutc_m13(LH_m13 *lh, si8 target_time);
si4			G_segment_index_m13(si4 segment_number, LH_m13 *lh);
si4			G_segment_range_m13(LH_m13 *lh, SLICE_m13 *slice);
ui4			*G_segment_video_start_frames_m13(FPS_m13 *vid_inds_fps, ui4 *n_video_files);
tern			G_sendgrid_email_m13(const si1 *sendgrid_key, const si1 *to_email, const si1 *cc_email, const si1 *to_name, const si1 *subject, const si1 *content, const si1 *from_email, const si1 *from_name, const si1 *reply_to_email, const si1 *reply_to_name);
tern			G_session_directory_m13(FPS_m13 *fps);
si1			*G_session_path_for_path_m13(const si1 *path, si1 *sess_path);
si8			G_session_samples_m13(LH_m13 *lh, sf8 rate);
void			G_set_error_exec_m13(const si1 *function, si4 line, si4 code, const si1 *message, ...); // vararg(code == E_SIG_m13): si4 sig_num (followed by optional formatting string values)
tern			G_set_session_globals_m13(const si1 *MED_path, const si1 *password, LH_m13 *lh);
tern			G_set_time_constants_m13(TIMEZONE_INFO_m13 *timezone_info, si8 session_start_time, tern prompt);
Sgmt_REC_m13		*G_Sgmt_records_m13(LH_m13 *lh, si4 search_mode);
ui4			G_Sgmt_records_source_m13(LH_m13 *lh, Sgmt_REC_m13 *Sgmt_recs);
tern			G_show_behavior_m13(ui4 behavior_code);
tern			G_show_contigua_m13(LH_m13 *lh);
tern			G_show_daylight_change_code_m13(DAYLIGHT_TIME_CHANGE_CODE_m13 *code, const si1 *prefix);
tern			G_show_error_m13(void);
tern			G_show_file_times_m13(FILE_TIMES_m13 *ft);
si4			G_show_function_stack_m13(pid_t_m13 _id);
void			G_show_globals_m13(void);
tern			G_show_level_header_m13(LH_m13 *lh);
tern			G_show_level_header_flags_m13(ui8 flags);
tern			G_show_location_info_m13(LOCATION_INFO_m13 *li);
void			G_show_lock_m13(FLOCK_ENTRY_m13 *lock);
tern			G_show_metadata_m13(FPS_m13 *fps, METADATA_m13 *md, ui4 type_code);
tern			G_show_password_data_m13(PASSWORD_DATA_m13 *pwd, si1 pw_level);
tern			G_show_password_hints_m13(PASSWORD_DATA_m13 *pwd, si1 pw_level);
tern			G_show_proc_globs_m13(LH_m13 *lh);
tern			G_show_records_m13(FPS_m13 *rec_data_fps, si4 *record_filters);
tern			G_show_Sgmt_records_m13(LH_m13 *lh, Sgmt_REC_m13 *Sgmt);
tern 			G_show_slice_m13(SLICE_m13 *slice);
tern			G_show_timezone_info_m13(TIMEZONE_INFO_m13 *timezone_entry, tern show_DST_detail);
tern			G_show_universal_header_m13(FPS_m13 *fps, UH_m13 *uh);
void			G_signal_trap_m13(si4 sig_num);
tern			G_sort_channels_by_acq_num_m13(SESS_m13 *sess);
tern			G_sort_records_m13(FPS_m13 *rec_inds_fps, FPS_m13 *rec_data_fps);
tern			G_swap_names_m13(LH_m13 *lh);
tern			G_terminal_entry_m13(const si1 *prompt, si1 type, void *buffer, void *default_input, tern required, tern validate);
tern			G_terminal_password_bytes_m13(const si1 *password, si1 *password_bytes);
tern			G_ternary_entry_m13(const si1 *entry);
tern			G_textbelt_text_m13(const si1 *phone_number, const si1 *content, const si1 *textbelt_key);
void			G_thread_exit_m13(void);
void			G_update_access_time_m13(LH_m13 *lh);
tern			G_update_channel_name_m13(CHAN_m13 *chan);
tern			G_update_channel_name_header_m13(const si1 *path, const si1 *fs_name);
tern			G_update_maximum_entry_size_m13(FPS_m13 *fps, si8 n_bytes, si8 n_items, si8 offset);
tern			G_update_MED_type_m13(const si1 *path); // used by G_update_MED_version_m13()
tern			G_update_MED_version_m13(FPS_m13 *fps);
tern			G_update_session_name_m13(FPS_m13 *fps);
tern			G_update_session_name_header_m13(const si1 *fs_path, const si1 *fs_name, const si1 *uh_name); // used by G_update_session_name_m13()
si8			G_uutc_for_frame_number_m13(LH_m13 *lh, si8 target_frame_number, ui4 mode, ...); // varargs (lh == NULL): si8 ref_frame_number, si8 ref_uutc, sf8 frame_rate
si8			G_uutc_for_sample_number_m13(LH_m13 *lh, si8 target_sample_number, ui4 mode, ...); // varargs (lh == NULL): si8 ref_smple_number, si8 ref_uutc, sf8 sampling_frequency
tern			G_valid_file_code_m13(ui4 file_type_code);
tern			G_valid_level_code_m13(ui4 level_code);
tern			G_valid_tern_m13(tern *val);
tern			G_validate_record_data_CRCs_m13(FPS_m13 *fps);
tern			G_validate_time_series_data_CRCs_m13(FPS_m13 *fps);
tern			G_validate_video_data_CRCs_m13(FPS_m13 *fps);
tern			G_video_data_m13(const si1 *string);
void			G_warning_message_m13(const si1 *fmt, ...);
void			G_write_medlibrc_m13(const si1 *path);



//**********************************************************************************//
//************************ Windows-specific (WN) Functions ***********************//
//**********************************************************************************//

#ifdef WINDOWS_m13

// function typedefs for NTdll dylib()

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID Pointer;
	};
	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef enum _FILE_INFORMATION_CLASS {
	FileDirectoryInformation = 1,
	FileFullDirectoryInformation = 2,
	FileBothDirectoryInformation = 3,
	FileBasicInformation = 4,  // supported by WN_query_file_information_m13()
	FileStandardInformation = 5,
	FileInternalInformation = 6,
	FileEaInformation = 7,
	FileAccessInformation = 8,  // supported by WN_query_file_information_m13()
	FileNameInformation = 9,
	FileRenameInformation = 10,
	FileLinkInformation = 11,
	FileNamesInformation = 12,
	FileDispositionInformation = 13,
	FilePositionInformation = 14,
	FileFullEaInformation = 15,
	FileModeInformation = 16,
	FileAlignmentInformation = 17,
	FileAllInformation = 18,
	FileAllocationInformation = 19,
	FileEndOfFileInformation = 20,
	FileAlternateNameInformation = 21,
	FileStreamInformation = 22,
	FilePipeInformation = 23,
	FilePipeLocalInformation = 24,
	FilePipeRemoteInformation = 25,
	FileMailslotQueryInformation = 26,
	FileMailslotSetInformation = 27,
	FileCompressionInformation = 28,
	FileObjectIdInformation = 29,
	FileCompletionInformation = 30,
	FileMoveClusterInformation = 31,
	FileQuotaInformation = 32,
	FileReparsePointInformation = 33,
	FileNetworkOpenInformation = 34,
	FileAttributeTagInformation = 35,
	FileTrackingInformation = 36,
	FileIdBothDirectoryInformation = 37,
	FileIdFullDirectoryInformation = 38,
	FileValidDataLengthInformation = 39,
	FileShortNameInformation = 40,
	FileIoCompletionNotificationInformation = 41,
	FileIoStatusBlockRangeInformation = 42,
	FileIoPriorityHintInformation = 43,
	FileSfioReserveInformation = 44,
	FileSfioVolumeInformation = 45,
	FileHardLinkInformation = 46,
	FileProcessIdsUsingFileInformation = 47,
	FileNormalizedNameInformation = 48,
	FileNetworkPhysicalNameInformation = 49,
	FileIdGlobalTxDirectoryInformation = 50,
	FileIsRemoteDeviceInformation = 51,
	FileUnusedInformation = 52,
	FileNumaNodeInformation = 53,
	FileStandardLinkInformation = 54,
	FileRemoteProtocolInformation = 55,
	FileRenameInformationBypassAccessCheck = 56,
	FileLinkInformationBypassAccessCheck = 57,
	FileVolumeNameInformation = 58,
	FileIdInformation = 59,
	FileIdExtdDirectoryInformation = 60,
	FileReplaceCompletionInformation = 61,
	FileHardLinkFullIdInformation = 62,
	FileIdExtdBothDirectoryInformation = 63,
	FileDispositionInformationEx = 64,
	FileRenameInformationEx = 65,
	FileRenameInformationExBypassAccessCheck = 66,
	FileDesiredStorageClassInformation = 67,
	FileStatInformation = 68,
	FileMemoryPartitionInformation = 69,
	FileStatLxInformation = 70,
	FileCaseSensitiveInformation = 71,
	FileLinkInformationEx = 72,
	FileLinkInformationExBypassAccessCheck = 73,
	FileStorageReserveIdInformation = 74,
	FileCaseSensitiveInformationForceAccessCheck = 75,
	FileKnownFolderInformation = 76,
	FileStatBasicInformation = 77,
	FileId64ExtdDirectoryInformation = 78,
	FileId64ExtdBothDirectoryInformation = 79,
	FileIdAllExtdDirectoryInformation = 80,
	FileIdAllExtdBothDirectoryInformation = 81,
	FileStreamReservationInformation,
	FileMupProviderInfo,
	FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

typedef struct _FILE_BASIC_INFORMATION {
	LARGE_INTEGER	CreationTime;
	LARGE_INTEGER	LastAccessTime;
	LARGE_INTEGER	LastWriteTime;
	LARGE_INTEGER	ChangeTime;
	ULONG		FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef DWORD	ACCESS_MASK;

typedef struct _FILE_ACCESS_INFORMATION {
  ACCESS_MASK	AccessFlags;
} FILE_ACCESS_INFORMATION, *PFILE_ACCESS_INFORMATION;

// function typedef for WN_query_information_file_m13()
typedef HRESULT (CALLBACK* NTQUERYINFOFILETYPE)(HANDLE, IO_STATUS_BLOCK *, PVOID, ULONG, FILE_INFORMATION_CLASS);

// function typedefs for WN_sleep_m13()
typedef HRESULT (CALLBACK* ZWSETTIMERRESTYPE)(ULONG, BOOLEAN, ULONG *);
typedef HRESULT (CALLBACK* NTDELAYEXECTYPE)(BOOLEAN, LARGE_INTEGER *);



// Prototypes
FILETIME	WN_uutc_to_win_time_m13(si8 uutc);
tern		WN_cleanup_m13(void);
tern		WN_clear_m13(void);
si8		WN_date_to_uutc_m13(sf8 date);
si4 		WN_ls_1d_to_buf_m13(const si1 **dir_strs, si4 n_dirs, tern full_path, si1 **buffer);
si4		WN_ls_1d_to_tmp_m13(const si1 **dir_strs, si4 n_dirs, tern full_path, si1 *temp_file);
tern		WN_init_terminal_m13(void);
void		WN_nap_m13(struct timespec *nap);
void		*WN_query_information_file_m13(void *fp, si4 info_class, void *fi);
tern		WN_reset_terminal_m13(void);
tern		WN_socket_startup_m13(void);
si4		WN_system_m13(const si1 *command);
si8		WN_time_to_uutc_m13(FILETIME win_time);
sf8		WN_uutc_to_date_m13(si8 uutc);
tern		WN_windify_file_paths_m13(si1 *target, const si1 *source);
si1		*WN_windify_format_string_m13(const si1 *fmt);
#endif // WINDOWS_m13

si8		WN_filetime_to_uutc_m13(ui1 *win_filetime); // for conversion of windows file time to uutc on any platform



//**********************************************************************************//
//******************** MED Alignmment Checking (ALCK) Functions ******************//
//**********************************************************************************//

tern	ALCK_all_m13(void);
tern	ALCK_metadata_m13(ui1 *bytes);
tern	ALCK_metadata_section_1_m13(ui1 *bytes);
tern	ALCK_metadata_section_3_m13(ui1 *bytes);
tern	ALCK_record_header_m13(ui1 *bytes);
tern	ALCK_record_indices_m13(ui1 *bytes);
tern	ALCK_time_series_indices_m13(ui1 *bytes);
tern	ALCK_time_series_metadata_section_2_m13(ui1 *bytes);
tern	ALCK_universal_header_m13(ui1 *bytes);
tern	ALCK_video_indices_m13(ui1 *bytes);
tern	ALCK_video_metadata_section_2_m13(ui1 *bytes);



//**********************************************************************************//
//*********************** Allocation Tracking (AT) Functions *********************//
//**********************************************************************************//

#ifdef AT_DEBUG_m13

// NOTE: The AT system keeps track all allocated & freed memory blocks, so the list can grow continuously & may appear like a a very slow memory leak
// Previously freed memory blocks are not replaced in the list so in the case of an attempted double free, it can inform where the block was previously freed.

// Prototypes
ui8	AT_actual_size_m13(void *address);
void	AT_add_entry_m13(const si1 *function, si4 line, void *address, size_t requested_bytes);
tern	AT_freeable_m13(void *address);
tern	AT_remove_entry_m13(const si1 *function, si4 line, void *address);
ui8	AT_requested_size_m13(void *address);
void	AT_show_entries_m13(void);
void	AT_show_entry_m13(void *address);
tern	AT_update_entry_m13(const si1 *function, si4 line, void *orig_address, void *new_address, size_t requested_bytes);

// AT replacement functions for alloc standard functions (these do not need to be called directly)
void	*AT_calloc_m13(const si1 *function, si4 line, size_t n_members, si8 el_size);
void	**AT_calloc_2D_m13(const si1 *function, si4 line, size_t dim1, size_t dim2, si8 el_size);
void	AT_free_m13(const si1 *function, si4 line, void *ptr);
void	AT_free_all_m13(const si1 *function, si4 line);
void	AT_free_2D_m13(const si1 *function, si4 line, void **ptr, size_t dim1);
void	*AT_malloc_m13(const si1 *function, si4 line, si8 n_bytes);
void	**AT_malloc_2D_m13(const si1 *function, si4 line, size_t dim1, si8 dim2_bytes);
void	*AT_realloc_m13(const si1 *function, si4 line, void *ptr, si8 n_bytes);
void	**AT_realloc_2D_m13(const si1 *function, si4 line, void **ptr, size_t curr_dim1, size_t new_dim2, size_t curr_dim2_bytes, si8 new_dim2_bytes);
void	*AT_recalloc_m13(const si1 *function, si4 line, void *ptr, size_t curr_members, size_t new_members, si8 el_size);
void	**AT_recalloc_2D_m13(const si1 *function, si4 line, void **ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, si8 el_size);

// preprocessor directives to replace standard alloc functions with AT versions
#define calloc_m13(a, b)			AT_calloc_m13(__FUNCTION__, __LINE__, a, b)
#define calloc_2D_m13(a, b, c)			AT_calloc_2D_m13(__FUNCTION__, __LINE__, a, b, c)
#define free_m13(a)				AT_free_m13(__FUNCTION__, __LINE__, a)
#define free_all_m13()				AT_free_all_m13(__FUNCTION__, __LINE__)
#define free_2D_m13(a, b)			AT_free_2D_m13(__FUNCTION__, __LINE__, a, b)
#define malloc_m13(a)				AT_malloc_m13(__FUNCTION__, __LINE__, a)
#define malloc_2D_m13(a, b)			AT_malloc_2D_m13(__FUNCTION__, __LINE__, a, b)
#define realloc_m13(a, b)			AT_realloc_m13(__FUNCTION__, __LINE__, a, b)
#define realloc_2D_m13(a, b, c, d, e)		AT_realloc_2D_m13(__FUNCTION__, __LINE__, a, b, c, d, e)
#define recalloc_m13(a, b, c, d)		AT_recalloc_m13(__FUNCTION__, __LINE__, a, b, c, d)
#define recalloc_2D_m13(a, b, c, d, e, f)	AT_recalloc_2D_m13(__FUNCTION__, __LINE__, a, b, c, d, e, f)

#endif // AT_DEBUG_m13



//**********************************************************************************//
//**************************** String (STR) Functions ****************************//
//**********************************************************************************//

// Prototypes
si1		*STR_bin_m13(si1 *str, void *num_ptr, size_t num_bytes, const si1 *byte_separator, tern numeric_order);
const si1	*STR_bool_m13(ui8 val);
wchar_t		*STR_char2wchar_m13(wchar_t *target, const si1 *source);
ui4		STR_check_spaces_m13(const si1 *string);
si4		STR_compare_m13(const void *a, const void *b);
tern		STR_contains_formatting_m13(const si1 *string, si1 *plain_string);
tern		STR_contains_regex_m13(const si1 *string);
si1 		*STR_duration_m13(si1 *dur_str, si8 int_usecs, tern abbreviated, tern two_level);
tern		STR_escape_chars_m13(si1 *string, si1 target_char, si8 buffer_len);
si1		*STR_fixed_width_int_m13(si1 *string, si4 string_bytes, si8 number);
si1		*STR_hex_m13(si1 *str, void *num_ptr, si8 num_bytes, const si1 *byte_separator, tern numeric_order);
tern		STR_is_empty_m13(const si1 *string);
si1		*STR_match_end_m13(const si1 *pattern, const si1 *buffer);
si1		*STR_match_end_bin_m13(const si1 *pattern, const si1 *buffer, si8 buf_len);
si1		*STR_match_line_end_m13(const si1 *pattern, const si1 *buffer);
si1		*STR_match_line_start_m13(const si1 *pattern, const si1 *buffer);
si1		*STR_match_start_m13(const si1 *pattern, const si1 *buffer);
si1		*STR_match_start_bin_m13(const si1 *pattern, const si1 *buffer, si8 buf_len);
si1 		*STR_re_escape_m13(const si1 *str, si1 *esc_str);
tern 		STR_replace_char_m13(si1 c, si1 new_c, si1 *buffer);
si1		*STR_replace_pattern_m13(const si1 *pattern, const si1 *new_pattern, const si1 *buffer, si1 *new_buffer);
si1		*STR_size_m13(si1 *size_str, si8 n_bytes, tern base_two);
tern		STR_sort_m13(si1 **string_array, si8 n_strings);
tern		STR_strip_character_m13(si1 *s, si1 character);
const si1	*STR_tern_m13(tern val, tern colored);
si1		*STR_time_m13(LH_m13 *lh , si8 uutc_time, si1 *time_str, tern fixed_width, tern relative_days, si4 colored_text, ...);
tern		STR_to_lower_m13(si1 *s);
tern		STR_to_title_m13(si1 *s);
tern		STR_to_upper_m13(si1 *s);
tern		STR_unescape_chars_m13(si1 *string, si1 target_char);
si1		*STR_wchar2char_m13(si1 *target, const wchar_t *source);



//**********************************************************************************//
//************************ CMP (COMPRESSION / COMPUTATION) ***********************//
//**********************************************************************************//

// CMP: Miscellaneous Constants
#define CMP_SAMPLE_VALUE_NO_ENTRY_m13		NAN_SI4_m13
#define CMP_SPLINE_TAIL_LEN_m13			6
#define CMP_SPLINE_UPSAMPLE_SF_RATIO_m13	((sf8) 3.0)
#define CMP_MAK_PAD_SAMPLES_m13			3
#define CMP_MAK_INPUT_BUFFERS_m13		8
#define CMP_MAK_IN_Y_BUF			0
#define CMP_MAK_IN_X_BUF			1
#define CMP_MAK_OUTPUT_BUFFERS_m13		4
#define CMP_MAK_OUT_Y_BUF			0
#define CMP_MAK_OUT_X_BUF			1
#define CMP_VDS_INPUT_BUFFERS_m13		(CMP_MAK_INPUT_BUFFERS_m13 + 1)
#define CMP_VDS_OUTPUT_BUFFERS_m13		CMP_MAK_OUTPUT_BUFFERS_m13
#define CMP_VDS_LOWPASS_ORDER_m13		6
#define CMP_VDS_MINIMUM_SAMPLES_m13		10
#define CMP_SELF_MANAGED_MEMORY_m13		-1 // pass CMP_SELF_MANAGED_MEMORY_m13 to CMP_allocate_processing_struct to prevent automatic re-allocation

// CMP: Block Fixed Header Offset Constants
#define CMP_BLOCK_FIXED_HDR_BYTES_m13				56 // fixed region only
#define CMP_BLOCK_START_UID_m13					((ui8) 0x0123456789ABCDEF) // ui8 (decimal 81,985,529,216,486,895)
#define CMP_BLOCK_START_UID_OFFSET_m13				0
#define CMP_BLOCK_CRC_OFFSET_m13				8 // ui4
#define CMP_BLOCK_CRC_NO_ENTRY_m13				CRC_NO_ENTRY_m13
#define CMP_BLOCK_BLOCK_FLAGS_OFFSET_m13			12 // ui4
#define CMP_BLOCK_BLOCK_FLAGS_NO_ENTRY_m13			0
#define CMP_BLOCK_CRC_START_OFFSET_m13				CMP_BLOCK_BLOCK_FLAGS_OFFSET_m13
#define CMP_BLOCK_START_TIME_OFFSET_m13				16 // si8
#define CMP_BLOCK_START_TIME_NO_ENTRY_m13			UUTC_NO_ENTRY_m13
#define CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_OFFSET_m13		24 // si4
#define CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m13	-1
#define CMP_BLOCK_TOTAL_BLOCK_BYTES_OFFSET_m13			28 // ui4
#define CMP_BLOCK_TOTAL_BLOCK_BYTES_NO_ENTRY_m13		0
// CMP Block Encryption Start
#define CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m13			32 // ui4
#define CMP_BLOCK_ENCRYPTION_START_OFFSET_m13			CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m13
#define CMP_BLOCK_NUMBER_OF_SAMPLES_NO_ENTRY_m13		0xFFFFFFFF
#define CMP_BLOCK_NUMBER_OF_RECORDS_OFFSET_m13			36 // ui2
#define CMP_BLOCK_NUMBER_OF_RECORDS_NO_ENTRY_m13		0xFFFF
#define CMP_BLOCK_RECORD_REGION_BYTES_OFFSET_m13		38 // ui2
#define CMP_BLOCK_RECORD_REGION_BYTES_NO_ENTRY_m13		0xFFFF
#define CMP_BLOCK_PARAMETER_FLAGS_OFFSET_m13			40 // ui4
#define CMP_BLOCK_PARAMETER_FLAGS_NO_ENTRY_m13			0
#define CMP_BLOCK_PARAMETER_REGION_BYTES_OFFSET_m13		44 // ui2
#define CMP_BLOCK_PARAMETER_REGION_BYTES_NO_ENTRY_m13		0xFFFF
#define CMP_BLOCK_PROTECTED_REGION_BYTES_OFFSET_m13		46 // ui2
#define CMP_BLOCK_PROTECTED_REGION_BYTES_NO_ENTRY_m13		0xFFFF
#define CMP_BLOCK_DISCRETIONARY_REGION_BYTES_OFFSET_m13		48 // ui2
#define CMP_BLOCK_DISCRETIONARY_REGION_BYTES_NO_ENTRY_m13	0xFFFF
#define CMP_BLOCK_MODEL_REGION_BYTES_OFFSET_m13			50 // ui2
#define CMP_BLOCK_MODEL_REGION_BYTES_NO_ENTRY_m13		0xFFFF
#define CMP_BLOCK_TOTAL_HEADER_BYTES_OFFSET_m13			52 // ui4
#define CMP_BLOCK_TOTAL_HEADER_BYTES_NO_ENTRY_m13		0xFFFF
#define CMP_BLOCK_RECORDS_REGION_OFFSET_m13			56

// CMP: Record Header Offset Constants
#define CMP_REC_HDR_BYTES_m13			8
#define CMP_REC_HDR_TYPE_CODE_OFFSET_m13		0 // ui4
#define CMP_REC_HDR_TYPE_CODE_NO_ENTRY_m13	0
#define CMP_REC_HDR_VERSION_MAJOR_OFFSET_m13	4 // ui1
#define CMP_REC_HDR_VERSION_MAJOR_NO_ENTRY_m13	0xFF
#define CMP_REC_HDR_VERSION_MINOR_OFFSET_m13	5 // ui1
#define CMP_REC_HDR_VERSION_MINOR_NO_ENTRY_m13	0xFF
#define CMP_REC_HDR_TOTAL_BYTES_OFFSET_m13	6 // ui2
#define CMP_REC_HDR_TOTAL_BYTES_NO_ENTRY_m13	0xFFFF // Note maximum CMP record size is 65k - smaller than MED record

// CMP: RED (Range Encoded Derivatives) Model Offset Constants
#define CMP_RED_MODEL_NUMBER_OF_KEYSAMPLE_BYTES_OFFSET_m13 	0 // ui4
#define CMP_RED_MODEL_DERIVATIVE_LEVEL_OFFSET_m13		4 // ui1
#define CMP_RED_MODEL_PAD_BYTES_OFFSET_m13			5 // ui1[3]
#define CMP_RED_MODEL_NUMBER_OF_STATISTICS_BINS_OFFSET_m13	8  // ui2
#define CMP_RED_MODEL_FLAGS_OFFSET_m13				10 // ui2
#define CMP_RED_MODEL_FIXED_HDR_BYTES_m13			12
// RED Model Flags
#define CMP_RED_FLAGS_NO_ZERO_COUNTS_m13			((ui2) 1) // bit 0
#define CMP_RED_FLAGS_POSITIVE_DERIVATIVES_m13			((ui2) 1 << 1) // bit 1
#define CMP_RED_2_BYTE_OVERFLOWS_m13				((ui2) 1 << 2) // bit 2
#define CMP_RED_3_BYTE_OVERFLOWS_m13				((ui2) 1 << 3) // bit 3
#define CMP_RED_OVERFLOW_BYTES_MASK_m13				( CMP_RED_2_BYTE_OVERFLOWS_m13 | CMP_RED_3_BYTE_OVERFLOWS_m13 )

// CMP: PRED (Predictive RED) Model Offset Constants
#define CMP_PRED_MODEL_NUMBER_OF_KEYSAMPLE_BYTES_OFFSET_m13 		0 // ui4
#define CMP_PRED_MODEL_DERIVATIVE_LEVEL_OFFSET_m13 			4 // ui1
#define CMP_PRED_MODEL_PAD_BYTES_OFFSET_m13				5 // ui1[3]
#define CMP_PRED_MODEL_NUMBERS_OF_STATISTICS_BINS_OFFSET_m13		8 // ui2[3]
#define CMP_PRED_MODEL_NUMBER_OF_NIL_STATISTICS_BINS_OFFSET_m13		CMP_PRED_MODEL_NUMBERS_OF_STATISTICS_BINS_OFFSET_m13 // ui2
#define CMP_PRED_MODEL_NUMBER_OF_POS_STATISTICS_BINS_OFFSET_m13		10 // ui2
#define CMP_PRED_MODEL_NUMBER_OF_NEG_STATISTICS_BINS_OFFSET_m13		12 // ui2
#define CMP_PRED_MODEL_FLAGS_OFFSET_m13					14 // ui2
#define CMP_PRED_MODEL_FIXED_HDR_BYTES_m13				16
// PRED Model Flags
#define CMP_PRED_FLAGS_NO_ZERO_COUNTS_m13				((ui2) 1) // bit 0
#define CMP_PRED_FLAGS_BIT_1_m13					((ui2) 1 << 1) // bit 1 Note: this is used for positive derivatives in RED, left empty here to keep bits same
#define CMP_PRED_2_BYTE_OVERFLOWS_m13					((ui2) 1 << 2) // bit 2
#define CMP_PRED_3_BYTE_OVERFLOWS_m13					((ui2) 1 << 3) // bit 3
#define CMP_PRED_OVERFLOW_BYTES_MASK_m13				( CMP_PRED_2_BYTE_OVERFLOWS_m13 | CMP_PRED_3_BYTE_OVERFLOWS_m13 )

// CMP: MBE (Minimal Bit Encoding) Model Offset Constants
#define CMP_MBE_MODEL_MINIMUM_VALUE_OFFSET_m13		0 // si4
#define CMP_MBE_MODEL_BITS_PER_SAMPLE_OFFSET_m13	4 // ui1
#define CMP_MBE_MODEL_DERIVATIVE_LEVEL_OFFSET_m13 	5 // ui1
#define CMP_MBE_MODEL_FLAGS_OFFSET_m13			6 // ui2
#define CMP_MBE_MODEL_FIXED_HDR_BYTES_m13		8
// MBE Model Flags
#define CMP_MBE_FLAGS_PREPROCESSED_MASK_m13		( (ui2) 1 ) // bit 0 - message to MBE_encode()) it will clear it

// CMP: VDS (Vectorized Data Stream) Model Offset Constants
#define CMP_VDS_MODEL_NUMBER_OF_VDS_SAMPLES_OFFSET_m13		0 // ui4
#define CMP_VDS_MODEL_AMPLITUDE_BLOCK_TOTAL_BYTES_OFFSET_m13	4 // ui4
#define CMP_VDS_MODEL_AMPLITUDE_BLOCK_MODEL_BYTES_OFFSET_m13	8 // ui2
#define CMP_VDS_MODEL_TIME_BLOCK_MODEL_BYTES_OFFSET_m13 	10 // ui2
#define CMP_VDS_MODEL_FLAGS_OFFSET_m13				12 // ui4 (more options for VDS)
#define CMP_VDS_MODEL_FIXED_HDR_BYTES_m13			16
// VDS Model Flags
#define CMP_VDS_FLAGS_AMPLITUDE_RED1_m13	((ui4) 1)  // bit 0
#define CMP_VDS_FLAGS_AMPLITUDE_PRED1_m13	((ui4) 1 << 1)  // bit 1
#define CMP_VDS_FLAGS_AMPLITUDE_MBE_m13		((ui4) 1 << 2)  // bit 2
#define CMP_VDS_FLAGS_AMPLITUDE_RED2_m13	((ui4) 1 << 3)  // bit 3
#define CMP_VDS_FLAGS_AMPLITUDE_PRED2_m13	((ui4) 1 << 4)  // bit 4
#define CMP_VDS_FLAGS_TIME_RED1_m13		((ui4) 1 << 5)  // bit 5
#define CMP_VDS_FLAGS_TIME_PRED1_m13		((ui4) 1 << 6)  // bit 6
#define CMP_VDS_FLAGS_TIME_MBE_m13		((ui4) 1 << 7)  // bit 7
#define CMP_VDS_FLAGS_TIME_RED2_m13		((ui4) 1 << 8)  // bit 8
#define CMP_VDS_FLAGS_TIME_PRED2_m13		((ui4) 1 << 9)  // bit 9
#define CMP_VDS_AMPLITUDE_ALGORITHMS_MASK_m13	( CMP_VDS_FLAGS_AMPLITUDE_RED1_m13 | CMP_VDS_FLAGS_AMPLITUDE_PRED1_m13 | CMP_VDS_FLAGS_AMPLITUDE_MBE_m13 | \
						CMP_VDS_FLAGS_AMPLITUDE_RED2_m13 | CMP_VDS_FLAGS_AMPLITUDE_PRED2_m13 )
#define CMP_VDS_TIME_ALGORITHMS_MASK_m13	( CMP_VDS_FLAGS_TIME_RED1_m13 | CMP_VDS_FLAGS_TIME_PRED1_m13 | CMP_VDS_FLAGS_TIME_MBE_m13 | \
						CMP_VDS_FLAGS_TIME_RED2_m13 | CMP_VDS_FLAGS_TIME_PRED2_m13 )
#define CMP_VDS_ALGORITHMS_MASK_m13		( CMP_VDS_AMPLITUDE_ALGORITHMS_m13 | CMP_VDS_TIME_ALGORITHMS_m13 )

// CMP Block Flag Masks
#define CMP_BF_BLOCK_FLAG_BITS_m13	32
#define CMP_BF_DISCONTINUITY_m13	((ui4) 1)  // bit 0
#define CMP_BF_ENCRYPTED_m13		((ui4) 1 << 1)  // bit 1 (block is currently encrypted - get level from universal header)
#define CMP_BF_RED1_ENCODING_m13	((ui4) 1 << 8)  // bit 8
#define CMP_BF_PRED1_ENCODING_m13	((ui4) 1 << 9)  // bit 9
#define CMP_BF_MBE_ENCODING_m13		((ui4) 1 << 10)  // bit 10
#define CMP_BF_VDS_ENCODING_m13		((ui4) 1 << 11)  // bit 11
#define CMP_BF_RED2_ENCODING_m13	((ui4) 1 << 12)  // bit 12 (faster, used as default RED version)
#define CMP_BF_PRED2_ENCODING_m13	((ui4) 1 << 13)  // bit 13 (faster, used as default PRED version)

#define CMP_BF_ALGORITHMS_MASK_m13	( CMP_BF_RED1_ENCODING_m13 | CMP_BF_PRED1_ENCODING_m13 | CMP_BF_MBE_ENCODING_m13 | \
					CMP_BF_VDS_ENCODING_m13 | CMP_BF_RED2_ENCODING_m13 | CMP_BF_PRED2_ENCODING_m13 )
// CMP Parameter Map Indices
#define CMP_PF_INTERCEPT_IDX_m13			((ui4) 0) // bit 0
#define CMP_PF_GRADIENT_IDX_m13				((ui4) 1) // bit 1
#define CMP_PF_AMPLITUDE_SCALE_IDX_m13			((ui4) 2) // bit 2
#define CMP_PF_FREQUENCY_SCALE_IDX_m13			((ui4) 3) // bit 3
#define CMP_PF_NOISE_SCORES_IDX_m13			((ui4) 4) // bit 4

// CMP Parameter Flag Masks
#define CMP_PF_PARAMETER_FLAG_BITS_m13		32
#define CMP_PF_INTERCEPT_m13			((ui4) 1 << CMP_PF_INTERCEPT_IDX_m13) // bit 0
#define CMP_PF_GRADIENT_m13			((ui4) 1 << CMP_PF_GRADIENT_IDX_m13) // bit 1
#define CMP_PF_AMPLITUDE_SCALE_m13		((ui4) 1 << CMP_PF_AMPLITUDE_SCALE_IDX_m13) // bit 2
#define CMP_PF_FREQUENCY_SCALE_m13		((ui4) 1 << CMP_PF_FREQUENCY_SCALE_IDX_m13) // bit 3
#define CMP_PF_NOISE_SCORES_m13			((ui4) 1 << CMP_PF_NOISE_SCORES_IDX_m13) // bit 4

// Compression Modes
#define CMP_COMPRESSION_MODE_NO_ENTRY_m13	((ui1) 0)
#define CMP_DECOMPRESSION_MODE_m13		((ui1) 1)
#define CMP_COMPRESSION_MODE_m13		((ui1) 2)

// Lossy Compression Modes
#define CMP_AMPLITUDE_SCALE_MODE_m13		((ui1) 1)
#define CMP_FREQUENCY_SCALE_MODE_m13		((ui1) 2)

// Compression Algorithms (use CMP block flags codes)
#define CMP_RED1_COMPRESSION_m13	CMP_BF_RED1_ENCODING_m13
#define CMP_RED2_COMPRESSION_m13	CMP_BF_RED2_ENCODING_m13
#define CMP_RED_COMPRESSION_m13		CMP_RED2_COMPRESSION_m13 // use RED v2 as default RED
#define CMP_PRED1_COMPRESSION_m13	CMP_BF_PRED1_ENCODING_m13
#define CMP_PRED2_COMPRESSION_m13	CMP_BF_PRED2_ENCODING_m13
#define CMP_PRED_COMPRESSION_m13	CMP_PRED2_COMPRESSION_m13 // use PRED v2 as default PRED
#define CMP_MBE_COMPRESSION_m13		CMP_BF_MBE_ENCODING_m13
#define CMP_VDS_COMPRESSION_m13		CMP_BF_VDS_ENCODING_m13

// Directives Flags
// Note: data encryption is set & read via universal_header->encryption_1 field (seems like a directive, but then things have to be set in both places)
#define CPS_DF_COMPRESSION_MODE_m13			((ui8) 1 << 0) // unset == decompressio, set == compression
#define CPS_DF_RED1_ALGORITHM_m13			((ui8) 1 << 1)
#define CPS_DF_RED2_ALGORITHM_m13			((ui8) 1 << 2)
#define CPS_DF_RED_ALGORITHM_m13			CPS_DF_RED2_ALGORITHM_m13 // default RED
#define CPS_DF_PRED1_ALGORITHM_m13			((ui8) 1 << 3)
#define CPS_DF_PRED2_ALGORITHM_m13			((ui8) 1 << 4)
#define CPS_DF_PRED_ALGORITHM_m13			CPS_DF_PRED2_ALGORITHM_m13 // default PRED
#define CPS_DF_VDS_ALGORITHM_m13			((ui8) 1 << 5)
#define CPS_DF_MBE_ALGORITHM_m13			((ui8) 1 << 6)
#define CPS_DF_CPS_POINTER_RESET_m13			((ui8) 1 << 7)
#define CPS_DF_CPS_CACHING_m13				((ui8) 1 << 8)
#define CPS_DF_FALL_THROUGH_TO_BEST_ENCODING_m13	((ui8) 1 << 9)
#define CPS_DF_RESET_DISCONTINUITY_m13			((ui8) 1 << 10)
#define CPS_DF_INCLUDE_NOISE_SCORES_m13			((ui8) 1 << 11)
#define CPS_DF_NO_ZERO_COUNTS_m13			((ui8) 1 << 12)
#define CPS_DF_SET_OVERFLOW_BYTES_m13			((ui8) 1 << 13) // user sets value in parameters
#define CPS_DF_FIND_OVERFLOW_BYTES_m13			((ui8) 1 << 14) // determine overflow bytes on a block by block basis
#define CPS_DF_POSITIVE_DERIVATIVES_m13 		((ui8) 1 << 15)
#define CPS_DF_SET_DERIVATIVE_LEVEL_m13			((ui8) 1 << 16)	 // user sets level in parameters
#define CPS_DF_FIND_DERIVATIVE_LEVEL_m13		((ui8) 1 << 17)
#define CPS_DF_CONVERT_TO_NATIVE_UNITS_m13		((ui8) 1 << 18)
// directives flags (lossy)
#define CPS_DF_DETREND_DATA_m13				((ui8) 1 << 32)
#define CPS_DF_REQUIRE_NORMALITY_m13			((ui8) 1 << 33)
#define CPS_DF_RETURN_LOSSY_DATA_m13			((ui8) 1 << 34)
#define CPS_DF_USE_COMPRESSION_RATIO_m13		((ui8) 1 << 35)
#define CPS_DF_USE_MEAN_RESIDUAL_RATIO_m13		((ui8) 1 << 36)
#define CPS_DF_USE_RELATIVE_RATIO_m13			((ui8) 1 << 37)
#define CPS_DF_SET_AMPLITUDE_SCALE_m13			((ui8) 1 << 38) // user sets value in parameters
#define CPS_DF_FIND_AMPLITUDE_SCALE_m13			((ui8) 1 << 39)
#define CPS_DF_SET_FREQUENCY_SCALE_m13			((ui8) 1 << 40) // user sets value in parameters
#define CPS_DF_FIND_FREQUENCY_SCALE_m13			((ui8) 1 << 41)
#define CPS_DF_VDS_SCALE_BY_BASELINE_m13		((ui8) 1 << 42) // increases compression by 15-30%

// masks
#define CPS_DF_ALGORITHM_MASK_m13			( CPS_DF_RED1_ALGORITHM_m13 | CPS_DF_PRED1_ALGORITHM_m13 | CPS_DF_RED2_ALGORITHM_m13 | \
							CPS_DF_PRED2_ALGORITHM_m13 | CPS_DF_VDS_ALGORITHM_m13 | CPS_DF_MBE_ALGORITHM_m13 )

// directive defaults
#define CPS_DIRECTIVES_COMPRESSION_MODE_DEFAULT_m13			FALSE_m13 // TRUE_m13 == compression, FALSE_m13 == decompression
#define CPS_DIRECTIVES_RED_ALGORITHM_DEFAULT_m13			FALSE_m13 // algorithm defaults are mutually exclusive (one, & only one, must be true)
#define CPS_DIRECTIVES_PRED_ALGORITHM_DEFAULT_m13			TRUE_m13 // algorithm defaults are mutually exclusive (one, & only one, must be true)
#define CPS_DIRECTIVES_VDS_ALGORITHM_DEFAULT_m13			FALSE_m13 // algorithm defaults are mutually exclusive (one, & only one, must be true)
#define CPS_DIRECTIVES_MBE_ALGORITHM_DEFAULT_m13			FALSE_m13 // algorithm defaults are mutually exclusive (one, & only one, must be true)
#define CPS_DIRECTIVES_LEVEL_1_ENCRYPTION_DEFAULT_m13			FALSE_m13 // encryption defaults are mutually exclusive (one, & only one, can be true, but neither must be)
#define CPS_DIRECTIVES_LEVEL_2_ENCRYPTION_DEFAULT_m13			FALSE_m13 // encryption defaults are mutually exclusive (one, & only one, can be true, but neither must be)
#define CPS_DIRECTIVES_CPS_POINTER_RESET_DEFAULT_m13			TRUE_m13
#define CPS_DIRECTIVES_CPS_CACHING_DEFAULT_m13				TRUE_m13
#define CPS_DIRECTIVES_FALL_THROUGH_TO_BEST_ENCODING_DEFAULT_m13	TRUE_m13
#define CPS_DIRECTIVES_RESET_DISCONTINUITY_DEFAULT_m13			TRUE_m13
#define CPS_DIRECTIVES_INCLUDE_NOISE_SCORES_DEFAULT_m13			FALSE_m13
#define CPS_DIRECTIVES_NO_ZERO_COUNTS_DEFAULT_m13			FALSE_m13
#define CPS_DIRECTIVES_SET_OVERFLOW_BYTES_DEFAULT_m13			FALSE_m13 // user sets value in parameters
#define CPS_DIRECTIVES_FIND_OVERFLOW_BYTES_DEFAULT_m13			TRUE_m13 // determine overflow bytes on a block by block basis
#define CPS_DIRECTIVES_POSITIVE_DERIVATIVES_DEFAULT_m13 		FALSE_m13
#define CPS_DIRECTIVES_SET_DERIVATIVE_LEVEL_DEFAULT_m13			FALSE_m13 // user sets level in parameters
#define CPS_DIRECTIVES_FIND_DERIVATIVE_LEVEL_DEFAULT_m13		FALSE_m13
#define CPS_DIRECTIVES_CONVERT_TO_NATIVE_UNITS_DEFAULT_m13		TRUE_m13
// directive defaults (lossy)
#define CPS_DIRECTIVES_DETREND_DATA_DEFAULT_m13				FALSE_m13
#define CPS_DIRECTIVES_REQUIRE_NORMALITY_DEFAULT_m13			FALSE_m13
#define CPS_DIRECTIVES_RETURN_LOSSY_DATA_DEFAULT_m13			FALSE_m13
#define CPS_DIRECTIVES_USE_COMPRESSION_RATIO_DEFAULT_m13		FALSE_m13
#define CPS_DIRECTIVES_USE_MEAN_RESIDUAL_RATIO_DEFAULT_m13		TRUE_m13
#define CPS_DIRECTIVES_USE_RELATIVE_RATIO_DEFAULT_m13			FALSE_m13
#define CPS_DIRECTIVES_SET_AMPLITUDE_SCALE_DEFAULT_m13			FALSE_m13 // user sets value in parameters
#define CPS_DIRECTIVES_FIND_AMPLITUDE_SCALE_DEFAULT_m13			FALSE_m13
#define CPS_DIRECTIVES_SET_FREQUENCY_SCALE_DEFAULT_m13			FALSE_m13 // user sets value in parameters
#define CPS_DIRECTIVES_FIND_FREQUENCY_SCALE_DEFAULT_m13			FALSE_m13
#define CPS_DIRECTIVES_VDS_SCALE_BY_BASELINE_DEFAULT_m13		FALSE_m13 // increases compression by 15-30%
#define CPS_DIRECTIVES_ALGORITHM_DEFAULT_m13				CMP_PRED_COMPRESSION_m13
#define CPS_DIRECTIVES_ENCRYPTION_LEVEL_DEFAULT_m13			NO_ENCRYPTION_m13

// parameters defaults
#define CPS_PARAMS_NUMBER_OF_BLOCK_PARAMS_DEFAULT_m13		0
#define CPS_PARAMS_MINIMUM_SAMPLE_VALUE_DEFAULT_m13		CMP_SAMPLE_VALUE_NO_ENTRY_m13
#define CPS_PARAMS_MAXIMUM_SAMPLE_VALUE_DEFAULT_m13		CMP_SAMPLE_VALUE_NO_ENTRY_m13
#define CPS_PARAMS_DISCONTINUITY_DEFAULT_m13			UNKNOWN_m13
#define CPS_PARAMS_DERIVATIVE_LEVEL_DEFAULT_m13			((ui1) 1)
#define CPS_PARAMS_OVERFLOW_BYTES_DEFAULT_m13			4
// parameters defaults (lossy)
#define CPS_PARAMS_GOAL_RATIO_DEFAULT_m13			((sf8) 0.05)
#define CPS_PARAMS_GOAL_TOLERANCE_DEFAULT_m13			((sf8) 0.005)
#define CPS_PARAMS_MAXIMUM_GOAL_ATTEMPTS_DEFAULT_m13		20
#define CPS_PARAMS_MINIMUM_NORMALITY_DEFAULT_m13		((ui1) 128) // range 0-254 (low to high)
#define CPS_PARAMS_AMPLITUDE_SCALE_DEFAULT_m13			((sf4) 1.0)
#define CPS_PARAMS_FREQUENCY_SCALE_DEFAULT_m13			((sf4) 1.0)
#define CPS_PARAMS_VDS_THRESHOLD_DEFAULT_m13			((sf8) 5.0) // generally an integer, but any float value is fine. Range 0.0 to 10.0; default == 5.0 (0.0 == lossless compression)
// parameters defaults (variable region)
#define CMP_USER_NUMBER_OF_RECORDS_DEFAULT_m13			((ui2) 0)
#define CMP_USER_RECORD_REGION_BYTES_DEFAULT_m13		((ui2) 0)
#define CMP_USER_PARAMETER_FLAGS_DEFAULT_m13			((ui4) 0)
#define CMP_PROTECTED_REGION_BYTES_DEFAULT_m13			((ui2) 0)
#define CMP_USER_DISCRETIONARY_REGION_BYTES_DEFAULT_m13		((ui2) 0)

// RED/PRED Codec Constants
#define CMP_SI1_KEYSAMPLE_FLAG_m13 		((si1) 0x80)  // -128 as si1
#define CMP_UI1_KEYSAMPLE_FLAG_m13 		((ui1) 0x80)  // +128 as ui1
#define CMP_POS_DERIV_KEYSAMPLE_FLAG_m13	((ui1) 0x00)  // no zero differences expected in positive derivative model
#define CMP_RED_TOTAL_COUNTS_m13 		((ui4) 0x10000) // 2^16
#define CMP_RED_MAXIMUM_RANGE_m13 		((ui8) 0x1000000000000) // 2^48
#define CMP_RED_RANGE_MASK_m13			((ui8) 0xFFFFFFFFFFFF) // 2^48 - 1
#define CMP_RED_MAX_STATS_BINS_m13 		256
#define CMP_PRED_CATS_m13 			3
#define CMP_PRED_NIL_m13 			0
#define CMP_PRED_POS_m13 			1
#define CMP_PRED_NEG_m13			2

// Macros
#define CMP_MAX_KEYSAMPLE_BYTES_m13(block_samps)		( block_samps * 5 ) // full si4 plus 1 keysample flag byte per sample
#define CMP_MAX_COMPRESSED_BYTES_m13(block_samps, n_blocks)	( ((block_samps * 4) + CMP_BLOCK_FIXED_HDR_BYTES_m13 + 7) * n_blocks )
#define CMP_PRED_CAT_m13(x)					( (x) ? (((x) & 0x80) ? CMP_PRED_NEG_m13 : CMP_PRED_POS_m13) : CMP_PRED_NIL_m13 )
#define CMP_IS_DETRENDED_m13(bh_ptr)				( (bh_ptr->parameter_flags & CMP_PF_INTERCEPT_m13) && (bh_ptr->parameter_flags & CMP_PF_GRADIENT_m13) )
#define CMP_VARIABLE_REGION_BYTES_v1_m13(bh_ptr)		( (ui4) (bh_ptr)->record_region_bytes + (ui4) (bh_ptr)->parameter_region_bytes + \
								(ui4) (bh_ptr)->protected_region_bytes + (ui4) (bh_ptr)->discretionary_region_bytes )
#define CMP_VARIABLE_REGION_BYTES_v2_m13(bh_ptr)		( (ui4) (bh_ptr)->total_header_bytes - ((ui4) CMP_BLOCK_FIXED_HDR_BYTES_m13 + \
								( (ui4) (bh_ptr)->model_region_bytes) )

// Update CPS Pointer Flags
#define CMP_UPDATE_ORIGINAL_PTR_m13		((ui1) 1)
#define CMP_RESET_ORIGINAL_PTR_m13 		((ui1) 2)
#define CMP_UPDATE_BLOCK_HDR_PTR_m13		((ui1) 4)
#define CMP_RESET_BLOCK_HDR_PTR_m13		((ui1) 8)
#define CMP_UPDATE_DECOMPRESSED_PTR_m13		((ui1) 16)
#define CMP_RESET_DECOMPRESSED_PTR_m13		((ui1) 32)

// Binterpolate() center mode codes
#define CMP_CENT_MODE_NONE_m13		0 // extrema only
#define CMP_CENT_MODE_MIDPOINT_m13	1 // best performance if extrema needed: (min + max) / 2
#define CMP_CENT_MODE_MEAN_m13		2 // best performance if extrema not needed
#define CMP_CENT_MODE_MEDIAN_m13	3 // best measure of central tendency
#define CMP_CENT_MODE_FASTEST_m13	4 // CMP_CENT_MODE_MIDPOINT_m13 if extrema requested, CMP_CENT_MODE_MEAN_m13 if not
#define CMP_CENT_MODE_DEFAULT_m13	CMP_CENT_MODE_MEAN_m13

// Normal cumulative distribution function values from -3 to +3 standard deviations in 0.1 sigma steps
#define CMP_NORMAL_CDF_TABLE_ENTRIES_m13 61
#define CMP_NORMAL_CDF_TABLE_m13 {	0.00134989803163010, 0.00186581330038404, 0.00255513033042794, 0.00346697380304067, \
					0.00466118802371875, 0.00620966532577614, 0.00819753592459614, 0.01072411002167580, \
					0.01390344751349860, 0.01786442056281660, 0.02275013194817920, 0.02871655981600180, \
					0.03593031911292580, 0.04456546275854310, 0.05479929169955800, 0.06680720126885810, \
					0.08075665923377110, 0.09680048458561040, 0.11506967022170800, 0.13566606094638300, \
					0.15865525393145700, 0.18406012534676000, 0.21185539858339700, 0.24196365222307300, \
					0.27425311775007400, 0.30853753872598700, 0.34457825838967600, 0.38208857781104700, \
					0.42074029056089700, 0.46017216272297100, 0.50000000000000000, 0.53982783727702900, \
					0.57925970943910300, 0.61791142218895300, 0.65542174161032400, 0.69146246127401300, \
					0.72574688224992600, 0.75803634777692700, 0.78814460141660300, 0.81593987465324100, \
					0.84134474606854300, 0.86433393905361700, 0.88493032977829200, 0.90319951541439000, \
					0.91924334076622900, 0.93319279873114200, 0.94520070830044200, 0.95543453724145700, \
					0.96406968088707400, 0.97128344018399800, 0.97724986805182100, 0.98213557943718300, \
					0.98609655248650100, 0.98927588997832400, 0.99180246407540400, 0.99379033467422400, \
					0.99533881197628100, 0.99653302619695900, 0.99744486966957200, 0.99813418669961600, \
					0.99865010196837000 }

#define CMP_SUM_NORMAL_CDF_m13		((sf8) 30.5)
#define CMP_SUM_SQ_NORMAL_CDF_m13	((sf8) 24.864467406647070)
#define CMP_KS_CORRECTION_m13		((sf8) 0.0001526091333688973)

#define CMP_VDS_THRESHOLD_MAP_TABLE_ENTRIES_m13	101
#define CMP_VDS_THRESHOLD_MAP_TABLE_m13 { \
	{ 0.0, 0.653145437887747, 0.087080239615529 }, \
	{ 0.1, 0.666524244368605, 0.096431833334447 }, \
	{ 0.2, 0.679901382612239, 0.105786886279366 }, \
	{ 0.3, 0.693195278745327, 0.115136328803986 }, \
	{ 0.4, 0.706328010262686, 0.124471091262003 }, \
	{ 0.5, 0.719312446055280, 0.133782104007116 }, \
	{ 0.6, 0.732256199033084, 0.143060297393023 }, \
	{ 0.7, 0.745271324448842, 0.152296601773421 }, \
	{ 0.8, 0.758469877555299, 0.161481947502008 }, \
	{ 0.9, 0.771963913605199, 0.170607264932484 }, \
	{ 1.0, 0.785865487851287, 0.179663484418544 }, \
	{ 1.1, 0.800286655103895, 0.188641536313887 }, \
	{ 1.2, 0.815276097313215, 0.197532350972212 }, \
	{ 1.3, 0.830643391297273, 0.206329712723042 }, \
	{ 1.4, 0.846141732247177, 0.215054845489400 }, \
	{ 1.5, 0.861524315354039, 0.223744284804766 }, \
	{ 1.6, 0.876544335808968, 0.232434732668414 }, \
	{ 1.7, 0.890954988803074, 0.241162891079617 }, \
	{ 1.8, 0.904515693506558, 0.249965462037652 }, \
	{ 1.9, 0.917339694399690, 0.258879147541791 }, \
	{ 2.0, 0.930077675257187, 0.267940649591310 }, \
	{ 2.1, 0.943425290156307, 0.277186670185483 }, \
	{ 2.2, 0.958078193174307, 0.286653911323584 }, \
	{ 2.3, 0.974732038388447, 0.296379075004888 }, \
	{ 2.4, 0.994082479875984, 0.306398209551974 }, \
	{ 2.5, 1.016674233046698, 0.316738820477894 }, \
	{ 2.6, 1.041615782368465, 0.327422417795757 }, \
	{ 2.7, 1.067220754080376, 0.338470387411485 }, \
	{ 2.8, 1.091794360492593, 0.349904115230999 }, \
	{ 2.9, 1.113758598041713, 0.361744987160223 }, \
	{ 3.0, 1.133017949669186, 0.374014389105076 }, \
	{ 3.1, 1.150496180583432, 0.386733706971483 }, \
	{ 3.2, 1.167137104226452, 0.399924326665363 }, \
	{ 3.3, 1.183884534040251, 0.413618810138173 }, \
	{ 3.4, 1.201682114424834, 0.427895185364072 }, \
	{ 3.5, 1.221180179543847, 0.442843040517614 }, \
	{ 3.6, 1.242133433412453, 0.458551963775380 }, \
	{ 3.7, 1.264126230660231, 0.475111543313956 }, \
	{ 3.8, 1.286742925916765, 0.492611367309924 }, \
	{ 3.9, 1.309573914909860, 0.511146276456767 }, \
	{ 4.0, 1.332419792627307, 0.530891512927402 }, \
	{ 4.1, 1.355340749569296, 0.552085150181815 }, \
	{ 4.2, 1.378413034757496, 0.574966933045358 }, \
	{ 4.3, 1.401712891490464, 0.599776606343383 }, \
	{ 4.4, 1.425298489977415, 0.626806804776781 }, \
	{ 4.5, 1.449169878829132, 0.656567276545111 }, \
	{ 4.6, 1.473315445926537, 0.689623472979960 }, \
	{ 4.7, 1.497723579150550, 0.726578911803450 }, \
	{ 4.8, 1.522488258560118, 0.768420064481964 }, \
	{ 4.9, 1.548262083298817, 0.816356801068373 }, \
	{ 5.0, 1.575881693662255, 0.871725955480487 }, \
	{ 5.1, 1.606179052450206, 0.936062669220404 }, \
	{ 5.2, 1.638914040756128, 1.010951239878917 }, \
	{ 5.3, 1.671426199703439, 1.097583980703378 }, \
	{ 5.4, 1.700723162658563, 1.195933599766891 }, \
	{ 5.5, 1.725174105101550, 1.305244713299358 }, \
	{ 5.6, 1.748170849533368, 1.423680806598649 }, \
	{ 5.7, 1.774263115611458, 1.548589345494656 }, \
	{ 5.8, 1.807974531435571, 1.678111066838628 }, \
	{ 5.9, 1.849031541061601, 1.810472754539325 }, \
	{ 6.0, 1.886797330616806, 1.944453521649560 }, \
	{ 6.1, 1.910022474285093, 2.079203902778041 }, \
	{ 6.2, 1.922265774682071, 2.214633042233715 }, \
	{ 6.3, 1.940778181878725, 2.350143024521086 }, \
	{ 6.4, 1.983325396122841, 2.485226121348756 }, \
	{ 6.5, 2.061401451135639, 2.619918574839387 }, \
	{ 6.6, 2.133662260846322, 2.754123018668519 }, \
	{ 6.7, 2.176369286579269, 2.887938280170844 }, \
	{ 6.8, 2.208654721196260, 3.021880758743963 }, \
	{ 6.9, 2.244989253035562, 3.156354273762239 }, \
	{ 7.0, 2.288044683671034, 3.290783349148508 }, \
	{ 7.1, 2.338500365190698, 3.425483693188855 }, \
	{ 7.2, 2.390947777679918, 3.560967005267627 }, \
	{ 7.3, 2.438785285059024, 3.697362380003563 }, \
	{ 7.4, 2.488873238561683, 3.834765208006531 }, \
	{ 7.5, 2.551586167259061, 3.973524655912109 }, \
	{ 7.6, 2.621198805891288, 4.113956660573023 }, \
	{ 7.7, 2.687949002958154, 4.256298709504494 }, \
	{ 7.8, 2.749458372117692, 4.400925898369564 }, \
	{ 7.9, 2.819492290144386, 4.548231341439871 }, \
	{ 8.0, 2.895978757474240, 4.698585849519076 }, \
	{ 8.1, 2.955556103053909, 4.852443976714582 }, \
	{ 8.2, 3.069362319165638, 5.010398618334849 }, \
	{ 8.3, 3.168267272628890, 5.172880252796954 }, \
	{ 8.4, 3.259217694239259, 5.340703709726352 }, \
	{ 8.5, 3.372393284309650, 5.514642980430471 }, \
	{ 8.6, 3.480506090966784, 5.695814408509766 }, \
	{ 8.7, 3.613539318894531, 5.885218071139942 }, \
	{ 8.8, 3.746867732981435, 6.084360685571760 }, \
	{ 8.9, 3.892198202949155, 6.294816359498106 }, \
	{ 9.0, 4.047865691316694, 6.518719318957218 }, \
	{ 9.1, 4.238233461465871, 6.758827410761595 }, \
	{ 9.2, 4.427441777303221, 7.018317189344236 }, \
	{ 9.3, 4.652784008515709, 7.301166878082886 }, \
	{ 9.4, 4.900609519620827, 7.612468057618706 }, \
	{ 9.5, 5.207111696945288, 7.960975856395534 }, \
	{ 9.6, 5.573339028134005, 8.358863805875041 }, \
	{ 9.7, 6.028522124546244, 8.820648191204029 }, \
	{ 9.8, 6.700281313487421, 9.364866314353193 }, \
	{ 9.9, 7.649180239136531, 10.019987941003953 }, \
	{ 10.0, 9.326074985746468, 10.846993908980265 } \
}


// Typedefs & Structures
typedef struct { // requires 4-byte alignment
	ui4	n_keysample_bytes;
	ui1	derivative_level;
	ui1	pad[3];
	ui2	n_statistics_bins;
	ui2	flags;
} CMP_RED_MODEL_FIXED_HDR_m13;

typedef struct { // requires 4-byte alignment
	ui4	n_keysample_bytes;
	ui1	derivative_level;
	ui1	pad[3];
	union {
		ui2	numbers_of_statistics_bins[3];
		struct {
			ui2	n_nil_statistics_bins;
			ui2	n_pos_statistics_bins;
			ui2	n_neg_statistics_bins;
		};
	};
	ui2	flags;
} CMP_PRED_MODEL_FIXED_HDR_m13;

typedef struct { // requires 4-byte alignment
	si4	minimum_value; // of highest derivative
	ui1	bits_per_sample;
	ui1	derivative_level;
	ui2	flags;
} CMP_MBE_MODEL_FIXED_HDR_m13;

typedef struct { // requires 4-byte alignment
	ui4	n_VDS_samples;
	ui4	amplitude_block_total_bytes;
	ui2	amplitude_block_model_bytes;
	ui2	time_block_model_bytes;
	ui4	flags; // potentially more options for VDS, so 32 bits
} CMP_VDS_MODEL_FIXED_HDR_m13;

typedef struct {
	ui4	type_code; // note this is not null terminated and so cannot be treated as a string as in REC_HDR_m13 structure
	ui1	version_major;
	ui1	version_minor;
	ui2	total_bytes; // note maximum record size is 65535 - smaller than in REC_HDR_m13 structure
} CMP_REC_HDR_m13;

// CMP_FIXED_BH_m13 declared above

typedef struct {
	ui4 count;
	union {
		si1 value;
		ui1	pos_value;
	};
} CMP_STATISTICS_BIN_m13;

typedef struct {
	si8	n_buffers;
	si8 	n_elements;
	si8	element_size;
	void	**buffer;
 // used internally
	ui8		total_allocated_bytes;
	tern	locked;
} CMP_BUFFERS_m13;

typedef struct {
	si8	cache_offset;
	ui4	block_samples;
	si4	block_number;
	tern	data_read;
} CMP_CACHE_BLOCK_INFO_m13;

typedef struct NODE_STRUCT_m13 {
	si4	val;
	ui4	count;
	struct NODE_STRUCT_m13	*prev, *next;
} CMP_NODE_m13;

// directives determine behavior of CPS; parameters for directives that require them are in the CPS_PARAMS_m13 structure
typedef struct {
	ui8	flags;
} CPS_DIRECS_m13; // structure for future directives that may not work well as flags

// parameters contain "mechanics" of CPS
typedef struct {
 // cache parameters
	CMP_CACHE_BLOCK_INFO_m13	*cached_blocks;
	si4				cached_block_list_len;
	si4				cached_block_cnt;
	si4				*cache;
	
 // memory parameters
	si8	allocated_block_samples;
	si8	allocated_keysample_bytes;
	si8	allocated_compressed_bytes; // == time series data fps: (raw_data_bytes - UH_BYTES_m13)
	si8	allocated_decompressed_samples;
	
 // compression parameters
	ui1	goal_derivative_level; // used with set_derivative_level directive
	ui1	derivative_level; // goal/actual pairs because not always possible
	ui1	goal_overflow_bytes; // used with set_overflow_bytes directive
	ui1	overflow_bytes; // goal/actual pairs because not always possible
	
 // block parameters
	tern	discontinuity; // set if block is first after a discontinuity, passed in compression, returned in decompression
	ui4	block_start_index; // block relative
	ui4	block_end_index; // block relative
	si4	n_block_parameters;
	ui4	block_parameter_map[CMP_PF_PARAMETER_FLAG_BITS_m13];
	si4	minimum_sample_value; // found on compression, stored for general use (and MBE, if used)
	si4	maximum_sample_value; // found on compression, stored for general use (and MBE, if used)
	si4	minimum_difference_value;
	si4	maximum_difference_value;
	ui2	user_number_of_records; // set by user
	ui2	user_record_region_bytes; // set by user to reserve bytes for records in block header
	ui4	user_parameter_flags; // user bits to be set in parameter flags of block header (library flags will be set automatically)
	ui2	protected_region_bytes; // not currently used, set to zero (allows for future expansion)
	ui2	user_discretionary_region_bytes; // set by user to reserve bytes for discretionary region in header
	ui4	variable_region_bytes; // value calculated and set by library based on parameters & directives
	ui4	n_derivative_bytes; // values in derivative or difference buffer
	
 // lossy compression parameters
	sf8	goal_ratio; // either compression ratio or mean residual ratio
	sf8	actual_ratio; // either compression ratio or mean residual ratio
	sf8	goal_tolerance; // tolerance for lossy compression mode goal, value of <= 0.0 uses default values, which are returned
	si4	maximum_goal_attempts; // maximum loops to attain goal compression
	ui1	minimum_normality; // range 0-254: 0 not normal, 254 perfectly normal, 0xFF no entry
	sf4	amplitude_scale; // used with set_amplitude_scale directive
	sf4	frequency_scale; // used with set_frequency_scale directive
	sf8	VDS_LFP_high_fc; // lowpass filter cutoff for VDS encoding (0.0 for no filter)
	sf8	VDS_threshold; // generally an integer, but float values are fine. Range 0.0 to 10.0; default == 5.0 (0.0 indicates lossless compression)
	sf8	VDS_sampling_frequency; // used to preserve units during LFP filtering, if filtering is not specified, this is not used
	
 // compression arrays
	si1			*keysample_buffer; // passed in both compression & decompression
	si4			*derivative_buffer; // used if needed in compression & decompression, size of maximum block differences
	si4			*detrended_buffer; // used if needed in compression, size of decompressed block
	si4			*scaled_amplitude_buffer; // used if needed in compression, size of decompressed block
	si4			*scaled_frequency_buffer; // used if needed in compression, size of decompressed block
	CMP_BUFFERS_m13		*scrap_buffers; // multipurpose
	CMP_BUFFERS_m13		*VDS_input_buffers;
	CMP_BUFFERS_m13		*VDS_output_buffers;
	struct FILTPS_m13	**filtps;
	si4			n_filtps;
	ui1			*model_region;
	void			*count; // used by RED/PRED encode & decode (ui4 * or ui4 **)
	void			*sorted_count; // used by RED/PRED encode & decode (CMP_STATISTICS_BIN_m13 * or CMP_STATISTICS_BIN_m13 **)
	void			*cumulative_count; // used by RED/PRED encode & decode (ui8 * or ui8 **)
	void			*minimum_range; // used by RED/PRED encode & decode (ui8 * or ui8 **)
	void			*symbol_map; // used by RED/PRED encode & decode (ui1 * or ui1 **)
} CPS_PARAMS_m13;

typedef struct CPS_m13 {
	CPS_DIRECS_m13			direcs;
	CPS_PARAMS_m13			params;
	si4				*input_buffer; // pointer that is updated depending on processing options (e.g. points to detrended data, scaled daata, etc.)
	ui1				*compressed_data; // points to base of FPS time_series_data array, NOT an allocated pointer => do not free; should not be updated
	CMP_FIXED_BH_m13		*block_header; // == FPS time_series_data; points to beginning of current block within compressed_data array, updatable
	si4				*decompressed_data; // returned in decompression or if lossy data requested, used in some compression modes, should not be updated
	si4				*decompressed_ptr; // points to beginning of current block within decompressed_data array, updatable
	si4				*original_data; // passed in compression, should not be updated
	si4				*original_ptr; // points to beginning of current block within original_data array, updatable
	ui1				*block_records; // pointer beginning of records region of block header
	ui4				*block_parameters; // pointer beginning of parameter region of block header
	ui1				*discretionary_region;
} CPS_m13;

// Function Prototypes
CMP_BUFFERS_m13	*CMP_allocate_buffers_m13(CMP_BUFFERS_m13 *buffers, si8 n_buffers, si8 n_elements, si8 element_size, tern zero_data, tern lock_memory);
CPS_m13	*CMP_allocate_CPS_m13(FPS_m13 *fps, ui4 mode, si8 data_samples, si8 compressed_data_bytes, si8 keysample_bytes, ui4 block_samples, CPS_DIRECS_m13 *direcs, CPS_PARAMS_m13 *parameters);
tern	CMP_binterpolate_sf8_m13(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len, ui4 center_mode, tern extrema, sf8 *minima, sf8 *maxima);
tern	CMP_byte_to_hex_m13(ui1 byte, si1 *hex);
sf8	CMP_calculate_mean_residual_ratio_m13(si4 *original_data, si4 *lossy_data, ui4 n_samps);
tern	CMP_calculate_statistics_m13(REC_Stat_v10_m13 *stats_ptr, si4 *data, si8 len, CMP_NODE_m13 *nodes);
tern	CMP_check_block_header_alignment_m13(ui1 *bytes);
tern	CMP_check_CPS_allocation_m13(FPS_m13 *fps);
tern	CMP_check_record_header_alignment_m13(ui1 *bytes);
si4	CMP_compare_sf8_m13(const void *a, const void * b);
si4	CMP_compare_si4_m13(const void *a, const void * b);
si4	CMP_compare_si8_m13(const void *a, const void * b);
si4	CMP_count_bins_m13(CPS_m13 *cps, si4 *deriv_buffer, ui1 n_derivs);
tern	CMP_decode_m13(FPS_m13 *fps);
tern	CMP_decrypt_m13(FPS_m13 *fps); // single block decrypt (see also decrypt_time_series_data_m13)
tern	CMP_detrend_m13(si4 *input_buffer, si4 *output_buffer, si8 len, CPS_m13 *cps);
tern	CMP_detrend_sf8_m13(sf8 *input_buffer, sf8 *output_buffer, si8 len);
ui1	CMP_differentiate_m13(CPS_m13 *cps);
tern	CMP_encode_m13(FPS_m13 *fps, si8 start_time, si4 acquisition_channel_number, ui4 n_samples);
tern	CMP_encrypt_m13(FPS_m13 *fps); // single block encrypt (see also encrypt_time_series_data_m13)
tern	CMP_find_amplitude_scale_m13(CPS_m13 *cps, tern (*compression_f)(CPS_m13 *cps));
si8	*CMP_find_crits_m13(sf8 *data, si8 data_len, si8 *n_crits, si8 *crit_xs);
tern	CMP_find_crits_2_m13(sf8 *data, si8 data_len, si8 *n_peaks, si8 *peak_xs, si8 *n_troughs, si8 *trough_xs);
tern	CMP_find_extrema_m13(si4 *input_buffer, si8 len, si4 *min, si4 *max, CPS_m13 *cps);
tern	CMP_find_frequency_scale_m13(CPS_m13 *cps, tern (*compression_f)(CPS_m13 *cps));
tern	CMP_free_buffers_m13(CMP_BUFFERS_m13 **buffers_ptr);
tern	CMP_free_cps_cache_m13(CPS_m13 *cps);
tern	CMP_free_cps_m13(CPS_m13 **cps_ptr);
sf8	CMP_gamma_cdf_m13(sf8 x, sf8 k, sf8 theta, sf8 offset);
sf8	CMP_gamma_cf_m13(sf8 a, sf8 x, sf8 *g_ln);
sf8	CMP_gamma_inv_cdf_m13(sf8 p, sf8 k, sf8 theta, sf8 offset);
sf8	CMP_gamma_inv_p_m13(sf8 p, sf8 a);
sf8	CMP_gamma_ln_m13(sf8 xx);
sf8	CMP_gamma_p_m13(sf8 a, sf8 x);
sf8	CMP_gamma_ser_m13(sf8 a, sf8 x, sf8 *g_ln);
tern	CMP_generate_lossy_data_m13(CPS_m13 *cps, si4* input_buffer, si4 *output_buffer, ui1 mode);
tern	CMP_generate_parameter_map_m13(CPS_m13 *cps);
ui1	CMP_get_overflow_bytes_m13(CPS_m13 *cps, ui4 mode, ui4 algorithm);
tern	CMP_get_variable_region_m13(CPS_m13 *cps);
tern	CMP_hex_to_int_m13(si1 *hex_str, void *hex_val, si4 val_bytes);
CPS_DIRECS_m13	*CMP_init_direcs_m13(CPS_DIRECS_m13 *direcs, ui1 compression_mode);
CPS_PARAMS_m13	*CMP_init_params_m13(CPS_PARAMS_m13 *params);
tern	CMP_init_tables_m13(void);
tern	CMP_integrate_m13(CPS_m13 *cps);
tern	CMP_lad_reg_2_sf8_m13(sf8 *x_input_buffer, sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
tern	CMP_lad_reg_2_si4_m13(si4 *x_input_buffer, si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
tern	CMP_lad_reg_sf8_m13(sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
tern	CMP_lad_reg_si4_m13(si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
sf8	*CMP_lin_interp_2_sf8_m13(si8 *in_x, sf8 *in_y, si8 in_len, sf8 *out_y, si8 *out_len);
sf8	*CMP_lin_interp_sf8_m13(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len);
si4	*CMP_lin_interp_si4_m13(si4 *in_data, si8 in_len, si4 *out_data, si8 out_len);
tern	CMP_lin_reg_2_sf8_m13(sf8 *x_input_buffer, sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
tern	CMP_lin_reg_2_si4_m13(si4 *x_input_buffer, si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
tern	CMP_lin_reg_sf8_m13(sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
tern	CMP_lin_reg_si4_m13(si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
tern	CMP_lock_buffers_m13(CMP_BUFFERS_m13 *buffers);
tern	CMP_MBE_decode_m13(CPS_m13 *cps);
tern	CMP_MBE_encode_m13(CPS_m13 *cps);
sf8	*CMP_mak_interp_sf8_m13(CMP_BUFFERS_m13 *in_bufs, si8 in_len, CMP_BUFFERS_m13 *out_bufs, si8 out_len);
ui1	CMP_normality_score_m13(si4 *data, ui4 n_samps);
sf8	CMP_p2z_m13(sf8 p);
tern	CMP_PRED1_decode_m13(CPS_m13 *cps);
tern	CMP_PRED2_decode_m13(CPS_m13 *cps);
tern	CMP_PRED1_encode_m13(CPS_m13 *cps);
tern	CMP_PRED2_encode_m13(CPS_m13 *cps);
CPS_m13	*CMP_realloc_cps_m13(FPS_m13 *fps, ui4 compression_mode, si8 data_samples, ui4 block_samples);
sf8	CMP_quantval_m13(sf8 *data, si8 len, sf8 quantile, tern preserve_input, sf8 *buf);
tern	CMP_rectify_m13(si4 *input_buffer, si4 *output_buffer, si8 len);
tern	CMP_RED1_decode_m13(CPS_m13 *cps);
tern	CMP_RED2_decode_m13(CPS_m13 *cps);
tern	CMP_RED1_encode_m13(CPS_m13 *cps);
tern	CMP_RED2_encode_m13(CPS_m13 *cps);
tern	CMP_retrend_si4_m13(si4 *in_y, si4 *out_y, si8 len, sf8 m, sf8 b);
tern	CMP_retrend_2_sf8_m13(sf8 *in_x, sf8 *in_y, sf8 *out_y, si8 len, sf8 m, sf8 b);
si2	CMP_round_si2_m13(sf8 val);
si4	CMP_round_si4_m13(sf8 val);
tern	CMP_scale_amplitude_si4_m13(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor, CPS_m13 *cps);
tern	CMP_scale_frequency_si4_m13(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor, CPS_m13 *cps);
tern	CMP_set_variable_region_m13(CPS_m13 *cps);
tern	CMP_sf8_to_si2_m13(sf8 *sf8_arr, si2 *si2_arr, si8 len, tern round);
tern	CMP_sf8_to_sf4_m13(sf8 *sf8_arr, sf4 *sf4_arr, si8 len, tern round);
tern	CMP_sf8_to_si4_m13(sf8 *sf8_arr, si4 *si4_arr, si8 len, tern round);
tern	CMP_sf8_to_si4_and_scale_m13(sf8 *sf8_arr, si4 *si4_arr, si8 len, sf8 scale);
tern	CMP_show_block_header_m13(LH_m13 *lh , CMP_FIXED_BH_m13 *bh);
tern	CMP_show_block_model_m13(CPS_m13 *cps, tern recursed_call);
tern	CMP_si4_to_sf8_m13(si4 *si4_arr, sf8 *sf8_arr, si8 len);
sf8	*CMP_spline_interp_sf8_m13(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len, CMP_BUFFERS_m13 *spline_bufs);
si4	*CMP_spline_interp_si4_m13(si4 *in_data, si8 in_len, si4 *out_data, si8 out_len, CMP_BUFFERS_m13 *spline_bufs);
sf8	CMP_splope_m13(sf8 *xa, sf8 *ya, sf8 *d2y, sf8 x, si8 lo_pt, si8 hi_pt);
sf8	CMP_trace_amplitude_m13(sf8 *y, sf8 *buffer, si8 len, tern detrend);
si8	CMP_ts_sort_m13(si4 *x, si8 len, CMP_NODE_m13 *nodes, CMP_NODE_m13 *head, CMP_NODE_m13 *tail, si4 return_sorted_ts, ...);
tern	CMP_unlock_buffers_m13(CMP_BUFFERS_m13 *buffers);
tern	CMP_unscale_amplitude_si4_m13(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor);
tern	CMP_unscale_amplitude_sf8_m13(sf8 *input_buffer, sf8 *output_buffer, si8 len, sf8 scale_factor);
tern	CMP_unscale_frequency_si4_m13(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor);
CMP_FIXED_BH_m13	*CMP_update_CPS_pointers_m13(FPS_m13 *fps, ui1 flags);
tern	CMP_VDS_decode_m13(CPS_m13 *cps);
tern	CMP_VDS_encode_m13(CPS_m13 *cps);
tern	CMP_VDS_generate_template_m13(CPS_m13 *cps, si8 data_len);
sf8	CMP_VDS_get_theshold_m13(CPS_m13 *cps);
sf8	CMP_z2p_m13(sf8 z);
tern	CMP_zero_buffers_m13(CMP_BUFFERS_m13 *buffers);



//**********************************************************************************//
//************************************** CRC *************************************//
//**********************************************************************************//

// ATTRIBUTION
//
// Basic CRC-32 manipulation routines
// by Mark Adler (madler@alumni.caltech.edu)
// placed in the public domain 29 Apr 2015
//
// "This library provides general CRC calculation & validation functions and an
// operation to combine the CRCs of two sequences of bytes into a single CRC.
// The routines in this libary only work with the particular CRC-32 polynomial
// provided here."
//
// Minor modifications for compatibility with the MED Library.
// The CRC routines are customized for little endian machines.


// Constants
#define CRC_BYTES_m13		4
#define CRC_TABLES_m13		8
#define CRC_TABLE_ENTRIES_m13	256
#define CRC_POLYNOMIAL_m13	((ui4) 0xEDB88320) // note library CRC routines are customized to this polynomial, it cannot be changed arbitrarily
#define CRC_START_VALUE_m13	((ui4) 0x0)
#define CRC_NO_ENTRY_m13	CRC_START_VALUE_m13

// CRC Modes
#define CRC_IGNORE_m13		((ui4) 0) // ignore CRCs
#define CRC_VALIDATE_m13	((ui4) 1 << 0) // validate on input
#define CRC_CALCULATE_m13	((ui4) 1 << 1) // calculate on output
#define CRC_MODES_ALL_m13	( CRC_VALIDATE_m13 | CRC_VALIDATE_m13 ) // validate on input & calculate on output

// Macros
#define CRC_SWAP32_m13(q)	( (((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + (((q) & 0xff00) << 8) + (((q) & 0xff) << 24) )

// Function Prototypes
ui4	CRC_calculate_m13(const ui1 *block_ptr, si8 block_bytes);
ui4	CRC_combine_m13(ui4 block_1_crc, ui4 block_2_crc, si8 block_2_bytes);
tern	CRC_init_tables_m13(void);
void	CRC_matrix_square_m13(ui4 *square, const ui4 *mat);
ui4	CRC_matrix_times_m13(const ui4 *mat, ui4 vec);
ui4	CRC_update_m13(const ui1 *block_ptr, si8 block_bytes, ui4 current_crc);
tern	CRC_validate_m13(const ui1 *block_ptr, si8 block_bytes, ui4 crc_to_validate);



//**********************************************************************************//
//************************************ UTF-8 *************************************//
//**********************************************************************************//

tern		UTF8_1_byte_char_m13(const si1 *c);
tern		UTF8_2_byte_char_m13(const si1 *c);
tern		UTF8_3_byte_char_m13(const si1 *c);
tern		UTF8_4_byte_char_m13(const si1 *c);
si4		UTF8_char_bytes_m13(const si1 *c);
size_t		UTF8_strchar_m13(const si1 *s);
size_t		UTF8_strlen_m13(const si1 *s);
tern		UTF8_valid_char_m13(const si1 *c);
tern		UTF8_valid_str_m13(const si1 *s);



//**********************************************************************************//
//************************************* AES-128 **********************************//
//**********************************************************************************//

// ATRIBUTION
//
// Advanced Encryption Standard implementation in C.
// By Niyaz PK
// E-mail: niyazlife@gmail.com
// Downloaded from Website: http://www.hoozi.com
//
// "This is the source code for encryption using the latest AES algorithm.
// The AES algorithm is also called Rijndael algorithm. The AES algorithm is
// recommended for non-classified use by the National Institute of Standards
// and Technology (NIST), USA."
//
// For the complete description of the algorithm, see:
// http://www.csrc.nist.gov/publications/fips/fips197/fips-197.pdf
//
// The code in this libaray is set for AES 128-bit encryption / decryption only.
//
// Minor modifications were made for compatibility with the MED Library.

#define AES_KEY_BYTES_m13		PASSWORD_BYTES_m13
#define AES_EXPANDED_KEY_BYTES_m13	ENCRYPTION_KEY_BYTES_m13 // AES-128 == ((AES_NR + 1) * AES_NK * AES_NB)

#define AES_NR_m13		10 // The number of rounds in AES Cipher
#define AES_NK_m13		4 // The number of 32 bit words in the key (128 bits)
#define AES_NB_m13		4 // The number of columns comprising a state in AES
#define AES_NS_m13		( AES_NK_m13 * AES_NB_m13 )  // the number of bytes in the state matrix
#define AES_XTIME_m13(x)	( (x << 1) ^ (((x >> 7) & 1) * 0x1b) ) // AES_XTIME is a macro that finds the product of 0x02 and the argument to AES_XTIME modulo 0x1b
#define AES_MULTIPLY_m13(x, y)	( ((y & 1) * x) ^ ((y >> 1 & 1) * AES_XTIME_m13(x)) ^ ((y >> 2 & 1) * AES_XTIME_m13(AES_XTIME_m13(x))) ^ \
				((y >> 3 & 1) * AES_XTIME_m13(AES_XTIME_m13(AES_XTIME_m13(x)))) ^ ((y >> 4 & 1) * \
				AES_XTIME_m13(AES_XTIME_m13(AES_XTIME_m13(AES_XTIME_m13(x))))) ) // Multiply is a macro used to multiply numbers in the field GF(2^8)

#define AES_SBOX_ENTRIES_m13	256
#define AES_SBOX_m13 {		0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, \
				0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, \
				0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, \
				0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, \
				0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, \
				0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, \
				0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, \
				0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, \
				0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, \
				0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, \
				0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, \
				0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, \
				0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, \
				0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, \
				0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, \
				0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 }

#define AES_RSBOX_ENTRIES_m13	256
#define AES_RSBOX_m13 {		0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, \
				0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb, \
				0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e, \
				0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25, \
				0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92, \
				0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84, \
				0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06, \
				0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b, \
				0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73, \
				0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e, \
				0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b, \
				0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4, \
				0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f, \
				0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef, \
				0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61, \
				0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };

#define AES_RCON_ENTRIES_m13	255
#define AES_RCON_m13 {		0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, \
				0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, \
				0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, \
				0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, \
				0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, \
				0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, \
				0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, \
				0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, \
				0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, \
				0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, \
				0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, \
				0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, \
				0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, \
				0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, \
				0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, \
				0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb };


// Function Prototypes
void	AES_add_round_key_m13(si4 round, ui1 state[][4], ui1 *round_key);
void	AES_cipher_m13(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key);
void	AES_decrypt_m13(ui1 *data, si8 len, const si1 *password, ui1 *expanded_key, ui1 rounds);
void	AES_encrypt_m13(ui1 *data, si8 len, const si1 *password, ui1 *expanded_key, ui1 rounds);
tern	AES_init_tables_m13(void);
void	AES_inv_cipher_m13(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key);
void	AES_inv_mix_columns_m13(ui1 state[][4]);
void	AES_inv_shift_rows_m13(ui1 state[][4]);
void	AES_inv_sub_bytes_m13(ui1 state[][4]);
ui1	*AES_key_expansion_m13(ui1 *round_key, const si1 *key);
void	AES_mix_columns_m13(ui1 state[][4]);
void	AES_partial_decrypt_m13(si4 n_bytes, ui1 *data, ui1 *round_key); // (round_key == NULL) => keyless decryption
void	AES_partial_encrypt_m13(si4 n_bytes, ui1 *data, ui1 *round_key); // (round_key == NULL) => keyless encryption
void	AES_shift_rows_m13(ui1 state[][4]);
void	AES_sub_bytes_m13(ui1 state[][4]);



//***********************************************************************//
//************************** SHA-256 FUNCTIONS ************************//
//***********************************************************************//

// ATTRIBUTION:
//
// Author:	Brad Conte (brad@bradconte.com)
// Disclaimer:	This code is presented "as is" without any guarantees.
// Details:	Implementation of the SHA-256 hashing algorithm.
//		Algorithm specification can be found here:
//	 	http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
//		This implementation uses little endian byte order.
//
// Code:	https://github.com/B-Con/crypto-algorithms/blob/master/sha256.c
//
// Only SHA-256 functions are included in the MED library.
// The version below contains minor modifications for compatibility with the MED Library.


// Constants
#define SHA_HASH_BYTES_m13	32 // 256 bit
#define SHA_LOW_BYTE_MASK_m13	((ui4) 0x000000FF)

#define	SHA_H0_ENTRIES_m13	8
#define	SHA_H0_m13 {		0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 }

#define	SHA_K_ENTRIES_m13	64
#define	SHA_K_m13 {		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, \
 				0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, \
 				0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, \
 				0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, \
 				0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, \
 				0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, \
 				0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, \
				0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 }

// Macros
#define SHA_ROTLEFT_m13(a,b)	( ((a) << (b)) | ((a) >> (32-(b))) )
#define SHA_ROTRIGHT_m13(a,b)	( ((a) >> (b)) | ((a) << (32-(b))) )
#define SHA_CH_m13(x,y,z)	( ((x) & (y)) ^ (~(x) & (z)) )
#define SHA_MAJ_m13(x,y,z)	( ((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)) )
#define SHA_EP0_m13(x)		( SHA_ROTRIGHT_m13(x,2) ^ SHA_ROTRIGHT_m13(x,13) ^ SHA_ROTRIGHT_m13(x,22) )
#define SHA_EP1_m13(x)		( SHA_ROTRIGHT_m13(x,6) ^ SHA_ROTRIGHT_m13(x,11) ^ SHA_ROTRIGHT_m13(x,25) )
#define SHA_SIG0_m13(x)		( SHA_ROTRIGHT_m13(x,7) ^ SHA_ROTRIGHT_m13(x,18) ^ ((x) >> 3) )
#define SHA_SIG1_m13(x)		( SHA_ROTRIGHT_m13(x,17) ^ SHA_ROTRIGHT_m13(x,19) ^ ((x) >> 10) )

// Typedefs & Structures
typedef struct {
	ui1	data[64];
	ui4	state[8];
	ui8	bitlen;
	ui4	datalen;
} SHA_CTX_m13;

// Function Prototypes
void	SHA_finalize_m13(SHA_CTX_m13 *ctx, ui1 *hash);
ui1	*SHA_hash_m13(const ui1 *data, si8 len, ui1 *hash);
void	SHA_init_m13(SHA_CTX_m13 *ctx);
tern	SHA_init_tables_m13(void);
void	SHA_transform_m13(SHA_CTX_m13 *ctx, const ui1 *data);
void	SHA_update_m13(SHA_CTX_m13 *ctx, const ui1 *data, si8 len);



//**********************************************************************************//
//************************************ FILTER ************************************//
//**********************************************************************************//

// ATTRIBUTION
//
// Some of the filter code was adapted from Matlab functions (MathWorks, Inc).
// www.mathworks.com
//
// The c code herein was written entirely from scratch.


// Constants
#define FILT_LOWPASS_TYPE_m13				1
#define FILT_BANDPASS_TYPE_m13				2
#define FILT_HIGHPASS_TYPE_m13				3
#define FILT_BANDSTOP_TYPE_m13				4
#define FILT_TYPE_DEFAULT_m13				FILT_LOWPASS_TYPE_m13
#define FILT_ORDER_DEFAULT_m13				5
#define FILT_PAD_SAMPLES_PER_POLE_m13			3 // minimum == 3
#define FILT_MAX_ORDER_m13				10
#define FILT_BAD_FILTER_m13				-1
#define FILT_BAD_DATA_m13				-2
#define FILT_EPS_SF8_m13				((sf8) 2.22045e-16)
#define FILT_RADIX_m13					((sf8) 2.0)
#define FILT_LINE_NOISE_HARMONICS_DEFAULT_m13		4
#define FILT_ANTIALIAS_FREQ_DIVISOR_DEFAULT_m13		((sf8) 3.5);
#define FILT_UNIT_THRESHOLD_DEFAULT_m13			CPS_PARAMS_VDS_UNIT_THRESHOLD_DEFAULT_m13
#define FILT_NFF_BUFFERS_m13				4
#define FILT_VDS_TEMPLATE_MIN_PS_m13			0 // index of CPS filtps
#define FILT_VDS_TEMPLATE_LFP_PS_m13			1 // index of CPS filtps
#define	FILT_VDS_MIN_SAMPS_PER_CYCLE_m13		((sf8) 4.5) // rolloff starts at ~5 samples per cycle

// Quantfilt Tail Options
#define FILT_TRUNCATE_m13			1
#define FILT_EXTRAPOLATE_m13			2
#define FILT_ZEROPAD_m13			3
#define FILT_DEFAULT_TAIL_OPTION_CODE_m13	FILT_TRUNCATE_m13

// Macros
#define FILT_ABS_m13(x)				( (x) >= ((sf8) 0.0) ? (x) : (-x) )
#define FILT_SIGN_m13(x, y)			( (y) >= ((sf8) 0.0) ? FILT_ABS_m13(x) : -FILT_ABS_m13(x) ) // y = abs(x)
#define FILT_POLES_m13(order, cutoffs)		( order * cutoffs )
#define FILT_FILT_PAD_SAMPLES_m13(poles)	( poles * FILT_PAD_SAMPLES_PER_POLE_m13 * 2 )
#define FILT_OFFSET_ORIG_DATA_m13(filtps)	( filtps->filt_data + (filtps->n_poles * FILT_PAD_SAMPLES_PER_POLE_m13) )

// Typedefs & Structs
typedef struct FILTPS_m13 {
	ui4	behavior;
	si4	order;
	si4	n_poles; // n_poles == order * n_cutoffs
	si4	type;
	sf8	sampling_frequency;
	si8	data_length;
	sf8	cutoffs[2];
	sf8	*numerators; // entries == n_poles + 1
	sf8	*denominators; // entries == n_poles + 1
	sf8	*initial_conditions; // entries == n_poles
	sf8	*orig_data;
	sf8	*filt_data;
	sf8	*buffer;
} FILTPS_m13;

typedef struct {
	sf8	real;
	sf8	imag;
} FILT_COMPLEX_m13;

typedef struct FILT_NODE_STRUCT {
	sf8				val;
	struct FILT_NODE_STRUCT		*prev, *next;
} FILT_NODE_m13;

typedef struct {
	si1		tail_option_code;
	si8		len, span, in_idx, out_idx, oldest_idx;
	sf8		*x, *qx, quantile, low_val_q, high_val_q;
	FILT_NODE_m13	*nodes, head, tail, *oldest_node, *curr_node;
} QUANTFILT_DATA_m13;


// Prototypes
QUANTFILT_DATA_m13	*FILT_alloc_quantfilt_data_m13(si8 len, si8 span);
tern	FILT_balance_m13(sf8 **a, si4 poles);
si4	FILT_butter_m13(FILTPS_m13 *filtps);
void	FILT_complex_div_m13(FILT_COMPLEX_m13 *a, FILT_COMPLEX_m13 *b, FILT_COMPLEX_m13 *quotient);
void	FILT_complex_exp_m13(FILT_COMPLEX_m13 *exponent, FILT_COMPLEX_m13 *ans);
void	FILT_complex_mult_m13(FILT_COMPLEX_m13 *a, FILT_COMPLEX_m13 *b, FILT_COMPLEX_m13 *product);
tern	FILT_elmhes_m13(sf8 **a, si4 poles);
tern	FILT_excise_transients_m13(CPS_m13 *cps, si8 data_len, si8 *n_extrema);
si4	FILT_filtfilt_m13(FILTPS_m13 *filtps);
tern	FILT_free_CPS_m13(CPS_m13 *cps, tern free_orig_data, tern free_filt_data, tern free_buffer);
tern	FILT_free_m13(FILTPS_m13 **filtps_ptr, tern free_orig_data, tern free_filt_data, tern free_buffer);
tern	FILT_free_quantfilt_data_m13(QUANTFILT_DATA_m13 **qd_ptr);
FILTPS_m13 *FILT_init_m13(si4 order, si4 type, sf8 samp_freq, si8 data_len, tern alloc_orig_data, tern alloc_filt_data, tern alloc_buffer, ui4 behavior_on_fail, sf8 cutoff_1, ...);
tern	FILT_generate_initial_conditions_m13(FILTPS_m13 *filtps);
tern	FILT_hqr_m13(sf8 **a, si4 poles, FILT_COMPLEX_m13 *eigs);
tern	FILT_invert_matrix_m13(sf8 **a, sf8 **inv_a, si4 order);
sf8	FILT_line_noise_filter_m13(sf8 *y, sf8 *fy, si8 len, sf8 samp_freq, sf8 line_freq, si8 cycles_per_template, tern calculate_score, tern fast_mode, CMP_BUFFERS_m13 *lnf_buffers);
void	FILT_mat_mult_m13(void *a, void *b, void *product, si4 outer_dim1, si4 inner_dim, si4 outer_dim2);
sf8	*FILT_moving_average_m13(sf8 *x, sf8 *ax, si8 len, si8 span, si1 tail_option_code);
sf8	*FILT_noise_floor_filter_m13(sf8 *data, sf8 *filt_data, si8 data_len, sf8 rel_thresh, sf8 abs_thresh, CMP_BUFFERS_m13 *nff_buffers);
sf8	*FILT_quantfilt_m13(sf8 *x, sf8 *qx, si8 len, sf8 quantile, si8 span, si1 tail_option_code);
QUANTFILT_DATA_m13	*FILT_quantfilt_head_m13(QUANTFILT_DATA_m13 *qd, ...); // varargs: sf8 *x, sf8 *qx, si8 len, sf8 quantile, si8 span, si4 tail_option_code
tern	FILT_quantfilt_mid_m13(QUANTFILT_DATA_m13 *qd);
tern	FILT_quantfilt_tail_m13(QUANTFILT_DATA_m13 *qd);
si4	FILT_sf8_sort_m13(const void *n1, const void *n2);
tern	FILT_show_processing_struct_m13(FILTPS_m13 *filt_ps);
tern	FILT_unsymmeig_m13(sf8 **a, si4 poles, FILT_COMPLEX_m13 *eigs);



//**********************************************************************************//
//******************************* DATA MATRIX (DM) ********************************//
//**********************************************************************************//

//	Extent Mode (EXTMD) Flags:
//
//	DM_EXTMD_ABSOLUTE_LIMITS_m13: all samples that exist between slice is start & end (specified by either time or sample number) are returned
//		Time specified, no padding requested:
//			read session by time
//		Time specified, padding requested:
//			read session by time, discontinuous regions padded
//		Samples specified, no padding requested:
//			read session by sample numbers
//		Samples specified, padding requested:
//			NOTE: There is potential for huge discontinuities being padded in output matrix, so samples are treated as relative.
//			NOTE: If discontinuity does ocurr in the specified slice, a warning message is displayed that relative output was produced.
//			start_time = time of start sample number
//			duration = (end_sample_number - start_sample_number) / ref_chan_samp_freq;
//			end_time = start_time + duration
//			read session by time, discontinuous regions padded
//
//	DM_EXTMD_RELATIVE_LIMITS_m13: slice start is treated as absolute, slice end is treated as indication of desired slice extents as follows:
//		Time specified, no padding requested:
//			duration = (end_time - start_time)
//			start_sample_number = sample number of start time
//			end_sample_number = start_sample_number + (ref_chan_samp_freq * duration)
//			read session by sample numbers
//		Time specified, padding requested:
//			read session by time, discontinuous regions padded
//		Samples specified, no padding requested:
//			read session by sample numbers
//		Samples specified, padding requested:
//			start_time = time of start sample number
//			duration = (end_sample_number - start_sample_number) / ref_chan_samp_freq;
//			end_time = start_time + duration
//			read session by time, discontinuous regions padded
//
//	Matrix output size can be specified by sample count or resultant sampling frequency:
//	DM_EXTMD_SAMP_COUNT_m13: output sample_count specified when passing matrix
//	DM_EXTMD_SAMP_FREQ_m13: output sampling frequency specified when passing matrix ( == valid_samples / valid_time in slice - not affected by discontinuities)
//
//	These two modes are driven by to general use cases:
//	1) want matrix of this specific size (e.g. viewer window where sample count == pixel columns)
//	2) want all channels at the specified sampling frequency (e.g algorithm looking across channels)


// Flag Definitions:
#define DM_NO_FLAGS_m13				NO_FLAGS_m13
#define DM_TYPE_SI2_m13				((ui8) 1 << 1)
#define DM_TYPE_SI4_m13				((ui8) 1 << 2)
#define DM_TYPE_SF4_m13				((ui8) 1 << 3)
#define DM_TYPE_SF8_m13				((ui8) 1 << 4)
#define DM_TYPE_MASK_m13			( DM_TYPE_SI2_m13 | DM_TYPE_SI4_m13 | DM_TYPE_SF4_m13 | DM_TYPE_SF8_m13 )
#define DM_2D_INDEXING_m13			((ui8) 1 << 7)  // include array of pointers so that matrix[x][y] indexing is possible (expensive with large major dinensions)
#define DM_FMT_SAMPLE_MAJOR_m13			((ui8) 1 << 8)
#define DM_FMT_CHANNEL_MAJOR_m13		((ui8) 1 << 9)
#define DM_FMT_MASK_m13				( DM_FMT_SAMPLE_MAJOR_m13 | DM_FMT_CHANNEL_MAJOR_m13 )
#define DM_EXTMD_SAMP_COUNT_m13			((ui8) 1 << 12)
#define DM_EXTMD_SAMP_FREQ_m13			((ui8) 1 << 13)
#define DM_EXTMD_MASK_m13			( DM_EXTMD_SAMP_COUNT_m13 | DM_EXTMD_SAMP_FREQ_m13 )
#define DM_EXTMD_ABSOLUTE_LIMITS_m13		((ui8) 1 << 14)
#define DM_EXTMD_RELATIVE_LIMITS_m13		((ui8) 1 << 15)
#define DM_EXTMD_LIMIT_MASK_m13			( DM_EXTMD_ABSOLUTE_LIMITS_m13 | DM_EXTMD_RELATIVE_LIMITS_m13 )
#define DM_SCALE_m13				((ui8) 1 << 18)  // output multiplied by this number (unless 0 or 1)
#define DM_FILT_LOWPASS_m13			((ui8) 1 << 20)  // low cutoff == vararg 1
#define DM_FILT_HIGHPASS_m13			((ui8) 1 << 21)  // high cutoff == vararg 1
#define DM_FILT_BANDPASS_m13			((ui8) 1 << 22)  // low cutoff == vararg 1, high cutoff == vararg 2
#define DM_FILT_BANDSTOP_m13			((ui8) 1 << 23)  // low cutoff == vararg 1, high cutoff == vararg 2
#define DM_FILT_ANTIALIAS_m13			((ui8) 1 << 24)  // lowpass with high cutoff computed, no varargs
#define DM_FILT_CUTOFFS_MASK_m13		( DM_FILT_LOWPASS_m13 | DM_FILT_HIGHPASS_m13 | DM_FILT_BANDPASS_m13 | DM_FILT_BANDSTOP_m13 )
#define DM_FILT_MASK_m13			( DM_FILT_CUTOFFS_MASK_m13 | DM_FILT_ANTIALIAS_m13 )
#define DM_INTRP_LINEAR_m13			((ui8) 1 << 28)
#define DM_INTRP_MAKIMA_m13			((ui8) 1 << 29)
#define DM_INTRP_SPLINE_m13			((ui8) 1 << 30)
#define DM_INTRP_UP_MAKIMA_DN_LINEAR_m13	((ui8) 1 << 31)
#define DM_INTRP_MAKIMA_UPSAMPLE_SF_RATIO_m13	((sf8) 1.5)
#define DM_INTRP_UP_SPLINE_DN_LINEAR_m13	((ui8) 1 << 32)  // if sampling frequency ratio >= DM_INTRP_SPLINE_UPSAMPLE_SF_RATIO_m13
#define DM_INTRP_SPLINE_UPSAMPLE_SF_RATIO_m13	CMP_SPLINE_UPSAMPLE_SF_RATIO_m13 // require (out_sf / in_sf) be >= this before spline upsampling, if lower use linear (prevents unnatural spline turns)
#define DM_INTRP_BINTRP_MDPT_m13		((ui8) 1 << 33)  // binterpolate with midpoint center mode
#define DM_INTRP_BINTRP_MEAN_m13		((ui8) 1 << 34)  // binterpolate with mean center mode
#define DM_INTRP_BINTRP_MEDN_m13		((ui8) 1 << 35)  // binterpolate with median center mode
#define DM_INTRP_BINTRP_FAST_m13		((ui8) 1 << 36)  // binterpolate with fast center mode
#define DM_INTRP_BINTRP_MASK_d1			( DM_INTRP_BINTRP_MDPT_m13 | DM_INTRP_BINTRP_MEAN_m13 | DM_INTRP_BINTRP_MEDN_m13 | DM_INTRP_BINTRP_FAST_m13 )
#define DM_INTRP_MASK_m13			( DM_INTRP_LINEAR_m13 | DM_INTRP_MAKIMA_m13 | DM_INTRP_SPLINE_m13 | DM_INTRP_UP_MAKIMA_DN_LINEAR_m13 | \
						DM_INTRP_UP_SPLINE_DN_LINEAR_m13 | DM_INTRP_BINTRP_MASK_d1 )
#define DM_TRACE_RANGES_m13			((ui8) 1 << 40)  // return bin minima & maxima (equal in size, type, & format to data matrix)
#define DM_TRACE_EXTREMA_m13			((ui8) 1 << 41)  // return minima & maxima values also (minimum & maximum per channel, same type as data matrix)
#define DM_DETREND_m13				((ui8) 1 << 42)  // detrend traces (and trace range matrices if DM_TRACE_RANGES_m13 is set)
#define DM_DSCNT_CONTIG_m13			((ui8) 1 << 48)  // return contigua
#define DM_DSCNT_NAN_m13			((ui8) 1 << 49)  // fill absent samples with NaNs (locations specified in returned arrays)
#define DM_DSCNT_ZERO_m13			((ui8) 1 << 50)  // fill absent samples with zeros (locations specified in returned arrays)
#define DM_PAD_MASK_m13				( DM_DSCNT_NAN_m13 | DM_DSCNT_ZERO_m13 )
#define DM_DSCNT_MASK_m13			( DM_DSCNT_CONTIG_m13 | DM_PAD_MASK_m13 )

// Non-flag defines
#define DM_MAXIMUM_INPUT_FREQUENCY_m13		((sf8) -3.0) // value chosen to distinguish from FREQUENCY_NO_ENTRY_m13 (-1.0) & RATE_VARIABLE_m13 (-2.0)
#define DM_MAXIMUM_INPUT_COUNT_m13		((si8) -3) // value chosen to parallel DM_MAXIMUM_INPUT_FREQUENCY_m13 & not conflict with NUMBER_OF_SAMPLES_NO_ENTRY_m13 (-1)


// Note: if arrays are allocted as 2D arrays, array[0] is beginning of one dimensional array containing (channel_count * sample_count) values of specfified type
typedef struct {
	si8		channel_count;  // defines dimension of allocated matrix: updated based on active channels
	si8		sample_count;  // defines dimension of allocated matrix: if extent mode (EXTMD) == DM_EXTMD_SAMP_FREQ_m13, resultant sample count filled in
	sf8		sampling_frequency; // defines dimension of allocated matrix: if extent mode (EXTMD) == DM_EXTMD_SAMP_COUNT_m13, resultant sampling frequenc filled in
	sf8		scale_factor;  // optionally passed (can be passed in varargs), always returned
	sf8		filter_low_fc;  // optionally passed (can be passed in varargs), always returned
	sf8		filter_high_fc;  // optionally passed (can be passed in varargs), always returned
	void		*data;	  // alloced / realloced as needed (cast to type * or type **, if DM_2D_INDEXING_m13 is set)
	void		*range_minima;  // alloced / realloced as needed, present if DM_TRACE_RANGES_m13 bit is set, otherwise NULL (cast to type * or type **, if DM_2D_INDEXING_m13 is set)
	void		*range_maxima;  // alloced / realloced as needed, present if DM_TRACE_RANGES_m13 bit is set, otherwise NULL (cast to type * or type **, if DM_2D_INDEXING_m13 is set)
	void		*trace_minima;  // alloced / realloced as needed, present if DM_TRACE_EXTREMA_m13 bit is set, otherwise NULL (cast to type *)
	void		*trace_maxima;  // alloced / realloced as needed, present if DM_TRACE_EXTREMA_m13 bit is set, otherwise NULL (cast to type *)
	si4		n_contigua;
	CONTIGUON_m13	*contigua;  // sample indexes in matrix frame
 // internal processing elements
	si8		valid_sample_count;  // used with padding options
	ui8		flags;
	si8		maj_dim;
	si8		min_dim;
	si8		el_size;
	si8		data_bytes;
	si8		n_proc_bufs;
	CMP_BUFFERS_m13	**in_bufs;
	CMP_BUFFERS_m13	**out_bufs;
	CMP_BUFFERS_m13	**mak_in_bufs;
	CMP_BUFFERS_m13	**mak_out_bufs;
	CMP_BUFFERS_m13	**spline_bufs;
} DATA_MATRIX_m13;

typedef struct {
	DATA_MATRIX_m13	*dm;
	CHAN_m13	*chan;
	si8		chan_idx;
} DM_CHANNEL_THREAD_INFO_m13;


// Prototypes
pthread_rval_m13	DM_channel_thread_m13(void *ptr);
tern			DM_free_matrix_m13(DATA_MATRIX_m13 **matrix);
DATA_MATRIX_m13 	*DM_get_matrix_m13(DATA_MATRIX_m13 *matrix, SESS_m13 *sess, SLICE_m13 *slice, si4 varargs, ...); // can't use tern to flag varargs (undefined behavior)
// DM_get_matrix_m13() varargs: si8 sample_count, sf8 sampling_frequency, ui8 flags, sf8 scale, sf8 fc1, sf8 fc2
//
// IMPORTANT: pass correct types for varargs - compiler cannot promote / convert to proper type because it doesn't know what they should be
//
// varargs DM_FILT_LOWPASS_m13 set: fc1 == high_cutoff
// varargs DM_FILT_HIGHPASS_m13 set: fc1 == low_cutoff
// varargs DM_FILT_BANDPASS_m13 set: fc1 == low_cutoff, fc2 == high_cutoff
// varargs DM_FILT_BANDSTOP_m13 set: fc1 == low_cutoff, fc2 == high_cutoff
tern			DM_show_flags_m13(ui8 flags);
DATA_MATRIX_m13		*DM_transpose_m13(DATA_MATRIX_m13 **in_matrix, DATA_MATRIX_m13 **out_matrix); // if *in_matrix == *out_matrix, done in place; if *out_matrix == NULL, allocated and returned
tern			DM_transpose_in_place_m13(DATA_MATRIX_m13 *matrix, void *base);
tern			DM_transpose_out_of_place_m13(DATA_MATRIX_m13 *in_matrix, DATA_MATRIX_m13 *out_matrix, void *in_base, void *out_base); // used by DM_transpose_m13(), assumes array allocation is taken care of, so use independently with care



//**********************************************************************************//
//******************************* TRANSMISSION (TR) ******************************//
//**********************************************************************************//

// Transmission Header Types
// • Type numbers 0-63 reserved for generic transmission types
// • Type numbers 64-255 used for application specific transmission types

#define TR_TYPE_GENERIC_MIN_m13					0
#define TR_TYPE_GENERIC_MAX_m13					63
#define TR_TYPE_APPLICATION_MIN_m13				64
#define TR_TYPE_APPLICATION_MAX_m13				255

// Generic Transmission Header (TH) Types
#define TR_TYPE_NO_ENTRY_m13					((ui1) 0)
#define TR_TYPE_KEEP_ALIVE_m13					((ui1) 1) // discarded if received by recv_transmission(), and waits for next transmission
#define TR_TYPE_ACK_OK_m13					((ui1) 2)
#define TR_TYPE_ACK_RETRANSMIT_m13				((ui1) 3)
#define TR_TYPE_MESSAGE_m13					((ui1) 4)
#define TR_TYPE_OPERATION_SUCCEEDED_m13				((ui1) 5)
#define TR_TYPE_OPERATION_SUCCEEDED_WITH_MESSAGE_m13		((ui1) 6)
#define TR_TYPE_OPERATION_FAILED_m13				((ui1) 7)
#define TR_TYPE_OPERATION_FAILED_WITH_WARNING_MESSAGE_m13	((ui1) 8)
#define TR_TYPE_OPERATION_FAILED_WITH_ERROR_MESSAGE_m13		((ui1) 9)

// Header Message Type Aliases (shorter :)
#define TR_ERROR_TYPE_m13	TR_TYPE_OPERATION_FAILED_WITH_ERROR_MESSAGE_m13
#define TR_WARNING_TYPE_m13	TR_TYPE_OPERATION_FAILED_WITH_WARNING_MESSAGE_m13
#define TR_SUCCESS_TYPE_m13	TR_TYPE_OPERATION_SUCCEEDED_WITH_MESSAGE_m13
#define TR_MESSAGE_TYPE_m13	TR_TYPE_MESSAGE_m13

// Transmission Error Codes (not enum so can make si8s)
#define TR_E_NONE_m13			((si8) E_NONE_m13) // zero (0)
#define TR_E_GEN_m13			((si8) FALSE_m13) // unknown or unspecified error
#define TR_E_SOCK_FAILED_m13		((si8) -2)
#define TR_E_SOCK_OPEN_m13		((si8) -3)
#define TR_E_SOCK_CLOSED_m13		((si8) -4)
#define TR_E_SOCK_TIMED_OUT_m13		((si8) -5)
#define TR_E_DATA_m13			((si8) -6)
#define TR_E_ID_m13			((si8) -7)
#define TR_E_TRANS_FAILED_m13		((si8) -8)
#define TR_E_CRC_m13			((si8) -9)
#define TR_E_ACK_m13			((si8) -10)

// Transmission Error Strings
#define	TR_E_NONE_STR_m13		"no error"
#define	TR_E_GEN_STR_m13		"unspecified transmission error"
#define TR_E_SOCK_FAILED_STR_m13	"socket failed"
#define TR_E_SOCK_OPEN_STR_m13		"could not open socket"
#define TR_E_SOCK_CLOSED_STR_m13	"socket closed"
#define TR_E_SOCK_TIMED_OUT_STR_m13	"socket timed out"
#define TR_E_DATA_STR_m13		"no data available"
#define TR_E_ID_STR_m13			"transmission ID mismatch"
#define TR_E_TRANS_FAILED_STR_m13	"transmission failed"
#define TR_E_CRC_STR_m13		"checksum mismatch"
#define TR_E_ACK_STR_m13		"no acknowlegment"

// Transmission Error Tags
#define	TR_E_NONE_TAG_m13		"TR_E_NONE"
#define	TR_E_GEN_TAG_m13		"TR_E_GEN"
#define TR_E_SOCK_FAILED_TAG_m13	"TR_E_SOCK_FAILED"
#define TR_E_SOCK_OPEN_TAG_m13		"TR_E_SOCK_OPEN"
#define TR_E_SOCK_CLOSED_TAG_m13	"TR_E_SOCK_CLOSED"
#define TR_E_SOCK_TIMED_OUT_TAG_m13	"TR_E_SOCK_TIMED_OUT"
#define TR_E_DATA_TAG_m13		"TR_E_DATA"
#define TR_E_ID_TAG_m13			"TR_E_ID"
#define TR_E_TRANS_FAILED_TAG_m13	"TR_E_TRANS_FAILED"
#define TR_E_CRC_TAG_m13		"TR_E_CRC"
#define TR_E_ACK_TAG_m13		"TR_E_ACK"

// Transmission Flags
#define TR_FLAGS_DEFAULT_m13		((ui2) 0)
#define TR_FLAGS_BIG_ENDIAN_m13		((ui2) 1) // Bit 0 (LITTLE_ENDIAN == 0, BIG_ENDIAN == 1)
#define TR_FLAGS_UDP_m13		((ui2) 1 << 1) // Bit 1 (TCP == 0, UDP == 1)
#define TR_FLAGS_ENCRYPT_m13		((ui2) 1 << 2) // Bit 2 (body only - header is not encrypted)
#define TR_FLAGS_INCLUDE_KEY_m13	((ui2) 1 << 3) // Bit 3 (expanded encryption key included in data - less secure than bilateral prescience of key)
#define TR_FLAGS_CLOSE_m13		((ui2) 1 << 4) // Bit 4 (close socket after send/recv)
#define TR_FLAGS_ACKNOWLEDGE_m13	((ui2) 1 << 5) // Bit 5 (acknowledge receipt with OK or retransmit)
#define TR_FLAGS_CRC_m13		((ui2) 1 << 6) // Bit 6 (calculate/check transmission CRC - last 4 bytes of transmission)
#define TR_FLAGS_NO_DESTRUCT_m13	((ui2) 1 << 7) // Bit 7 (set if local memory should not be altered - applies to encrpyted transmissions & transmissions that exceed TR_MTU_BYTES_m13)
#define TR_FLAGS_TO_FILE_m13		((ui2) 1 << 8) // Bit 8 (set if received data should go to a file rather than buffer - pseudo FTP)

// TR Defaults
#define TR_VERSION_DEFAULT_m13		((ui1) 1)
#define TR_TYPE_DEFAULT_m13		TR_TYPE_NO_ENTRY_m13
#define TR_ID_CODE_NO_ENTRY_m13		0 // ui4
#define TR_ID_CODE_DEFAULT		TR_ID_CODE_NO_ENTRY_m13

// Transmission Message
#define TR_MESSAGE_HDR_BYTES_m13			16
#define TR_MESSAGE_HDR_TIME_OFFSET_m13			0 // si8
#define TR_MESSAGE_HDR_NO_ENTRY_m13			UUTC_NO_ENTRY_m13
#define TR_MESSAGE_HDR_MESSAGE_BYTES_OFFSET_m13		8 // si8
#define TR_MESSAGE_HDR_MESSAGE_BYTES_NO_ENTRY_m13	0

// Transmission Header (TH) Format Constants
#define TR_HDR_BYTES_m13				((si8) 32)
#define TR_CRC_OFFSET_m13				0		  // ui4
#define TR_PACKET_BYTES_OFFSET_m13			4		  // ui2
#define TR_FLAGS_OFFSET_m13				6	  // ui2
#define TR_ID_STRING_OFFSET_m13				8	  // ascii[4]
#define TR_ID_STRING_TERMINAL_ZERO_OFFSET_m13		(TR_ID_STRING_OFFSET_m13 + 4) // si1
#define TR_ID_CODE_OFFSET_m13				TR_ID_STRING_OFFSET_m13  // ui4
// TR_ID_CODE_NO_ENTRY_m13 defined above
#define TR_TYPE_OFFSET_m13				13	  // ui1
#define TR_SUBTYPE_OFFSET_m13				14	  // ui1
#define TR_VERSION_OFFSET_m13				15	  // ui1
#define TR_VERSION_NO_ENTRY_m13				0
#define TR_TRANSMISSION_BYTES_OFFSET_m13		16		  // ui8
#define TR_TRANSMISSION_BYTES_NO_ENTRY_m13		0
#define TR_OFFSET_OFFSET_m13				24		  // ui8

// Transmission Info Modes [set by TR_send_transmission_m13() & TR_recv_transmission_m13(), used by TR_close_transmission_m13()]
// indicates whether last transmission was a send or receive
#define TR_MODE_NONE_m13		0
#define TR_MODE_SEND_m13		1
#define TR_MODE_RECV_m13		2
#define TR_MODE_FORCE_CLOSE_m13		3 // set to force close a (TCP) socket

// Miscellaneous
#define TR_INET_MSS_BYTES_m13		1376	// highest multiple of 16, that stays below internet standard frame size (1500) minus [32 (TR header) + 40 (TCP/IP header)
						// + some extra (possible intermediary protocols like GRE, IPsec, PPPoE, or SNAP that may be in the route)]
#define TR_LO_MSS_BYTES_m13		65456 // highest multiple of 16, that stays below backplane (loopback) standard frame size (65535) minus [32 (TR header) + 40 (TCP/IP header)])
#define TR_PORT_STRLEN_m13		8
#define TR_TIMEOUT_NEVER_m13		((sf4) 0.0)
#define TR_PORT_ANY_m13			0 // system assigned port
#define TR_IFACE_ANY_m13		((void *) 0) // all interfaces
#define TR_IFACE_DFLT_m13		"" // default internet interface
#define TR_RETRANSMIT_ATTEMPTS_m13	3


// Typedefs
typedef struct {
	ui4	crc;
	ui2	packet_bytes; // bytes in this packet (including header)
	ui2	flags;
	union {
		struct {
			si1 ID_string[TYPE_BYTES_m13]; // transmission ID is typically application specific
			ui1 type; // transmission type (general [0-63] or transmission ID specific [64-255])
			ui1	subtype; // rarely used
			ui1 version; // transmission header version
		};
		struct {
			ui4 ID_code; // transmission ID is typically application specific
			union {
				ui4	combined_check; // use to to check [zero, type, subtype, version] as a ui4
				struct {
					si1	ID_string_terminal_zero; // here for clarity
					ui1	pad_bytes[3]; // not available for use (type, subtype, & version above)
				};
			};
		};
	};
	si8	transmission_bytes; // full size of tramsmitted data in bytes (*** does not include header ***)
	si8	offset; // offset (in bytes) of packet data into full data (*** does not include header ***)
} TR_HDR_m13;

typedef struct {
	union {
		ui1		*buffer; // used internally, first portion is the transmission header
		TR_HDR_m13	*header;
	};
	si8	buffer_bytes; // bytes available for data (actual allocation also includes room for header)
	ui1	*data; // buffer + TR_HDR_BYTES_m13
	si1	*password; // for encryption (NOT freed by TR_free_transmission_info_m13)
	ui1	*expanded_key; // for encryption
	tern	expanded_key_allocated; // determines whether to free expanded key
	ui1	mode; // TR_MODE_SEND_m13, TR_MODE_RECV_m13, TR_MODE_NONE_m13 (needed to properly close TCP sockets)
	si4	sock_fd;
	si1	dest_addr[INET6_ADDRSTRLEN]; // INET6_ADDRSTRLEN == 46 (this can be an IP address string or or a domain name [< 46 characters])
	ui2	dest_port;
	si1	iface_addr[INET6_ADDRSTRLEN]; // zero-length string for any default internet interface
	ui2	iface_port;
	sf4	timeout; // seconds
	ui2	mss; // maximum segment size (max bytes of data per packet [*** does not include header ***]) (typically multiple of 16, must be at least multiple of 8 for library)
} TR_INFO_m13;

typedef struct {
	si8	time;  // uutc
	si8	message_bytes; // includes text, & pad bytes, NOT header bytes
} TR_MESSAGE_HDR_m13; // text follows structure, padded with zeros to 16 byte alignment


// Prototypes
TR_INFO_m13	*TR_alloc_trans_info_m13(si8 buffer_bytes, ui4 ID_code, ui1 header_flags, sf4 timeout, const si1 *password);
tern		TR_bind_m13(TR_INFO_m13 *trans_info, si1 *iface_addr, ui2 iface_port);
tern		TR_build_message_m13(TR_MESSAGE_HDR_m13 *msg, const si1 *message_text);
tern		TR_check_transmission_header_alignment_m13(ui1 *bytes);
tern		TR_close_transmission_m13(TR_INFO_m13 *trans_info);
tern		TR_connect_m13(TR_INFO_m13 *trans_info, const si1 *dest_addr, ui2 dest_port);
tern		TR_connect_to_server_m13(TR_INFO_m13 *trans_info, const si1 *dest_addr, ui2 dest_port);
tern		TR_create_socket_m13(TR_INFO_m13 *trans_info);
tern		TR_free_transmission_info_m13(TR_INFO_m13 **trans_info_ptr);
tern		TR_realloc_trans_info_m13(TR_INFO_m13 *trans_info, si8 buffer_bytes, TR_HDR_m13 **caller_header);
si8		TR_recv_transmission_m13(TR_INFO_m13 *trans_info, TR_HDR_m13 **caller_header); // receive may reallocate, pass caller header to have function set local variable, otherwise pass NULL, can do manually
tern		TR_send_message_m13(TR_INFO_m13 *trans_info, ui1 type, tern encrypt, const si1 *fmt, ...);
si8		TR_send_transmission_m13(TR_INFO_m13 *trans_info);
tern		TR_set_socket_blocking_m13(TR_INFO_m13 *trans_info, tern set);
tern		TR_set_socket_broadcast_m13(TR_INFO_m13 *trans_info, tern set);
tern		TR_set_socket_reuse_address_m13(TR_INFO_m13 *trans_info, tern set);
tern		TR_set_socket_reuse_port_m13(TR_INFO_m13 *trans_info, tern set);
tern		TR_set_socket_timeout_m13(TR_INFO_m13 *trans_info);
tern		TR_show_message_m13(TR_HDR_m13 *header);
tern		TR_show_transmission_m13(TR_INFO_m13 *trans_info);
si1		*TR_strerror_m13(si4 err_num);



//**********************************************************************************//
//******************************** Time Zone Data ********************************//
//**********************************************************************************//

// Notes:
//
// Western Sahara:
// DST is on most of the year and off during Ramadan, whose dates change annually in a way that is not accomodated by this schema.
// As Ramadan only lasts a month, and can occur at virtually any time of year, this table treats it as if it's Daylight Time
// is it's Standard Time, and it does not observe DST.
//
// If it were to have a proper entry, it would look something like:
// { "WESTERN SAHARA", "EH", "ESH", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 0x3c0002041f00ff01, 0xc40003031300ffff, -1 }
//
// But it is represented here as:
// { "WESTERN SAHARA", "EH", "ESH", "", "", "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 3600, "", "", 0x0, 0x0, -1 }

#define TZ_TABLE_ENTRIES_m13 399
#define TZ_TABLE_m13 { \
	{ "AFGHANISTAN", "AF", "AFG", "", "", "AFGHANISTAN TIME", "AFT", 16200, "", "", 0x0, 0x0, -1 }, \
	{ "AKROTIRI", "", "", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "ALAND ISLANDS", "AX", "ALA", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "ALBANIA", "AL", "ALB", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "ALGERIA", "DZ", "DZA", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "AMERICAN SAMOA", "US", "ASM", "", "", "SAMOA STANDARD TIME", "SST", -39600, "", "", 0x0, 0x0, -1 }, \
	{ "ANDORRA", "AD", "AND", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "ANGOLA", "AO", "AGO", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "ANGUILLA", "AI", "AIA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "ANTIGUA", "AG", "ATG", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "ARGENTINA", "AR", "ARG", "", "", "ARGENTINA TIME", "ART", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "ARMENIA", "AM", "ARM", "", "", "ARMENIA TIME", "AMT", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "ARUBA", "AW", "ABW", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "ASCENSION", "SH", "SHN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "WESTERN AUSTRALIA", "WA", "AUSTRALIAN WESTERN STANDARD TIME", "AWST", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "WESTERN AUSTRALIA", "WA", "AUSTRALIAN CENTRAL WESTERN STANDARD TIME", "ACWST", 31500, "", "", 0x0, 0x0, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "SOUTH AUSTRALIA", "SA", "AUSTRALIAN CENTRAL STANDARD TIME", "ACST", 34200, "AUSTRALIAN CENTRAL DAYLIGHT TIME", "ACDT", 0x3C00020900010001, 0xC4000303000100FF, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "NORTHERN TERRITORY", "NT", "AUSTRALIAN CENTRAL STANDARD TIME", "ACST", 34200, "", "", 0x0, 0x0, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "AUSTRALIAN CAPITAL TERRITORY", "ACT", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "AUSTRALIAN EASTERN DAYLIGHT TIME", "AEDT", 0x3C00020900010001, 0xC4000303000100FF, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "TASMANIA", "TAS", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "AUSTRALIAN EASTERN DAYLIGHT TIME", "AEDT", 0x3C00020900010001, 0xC4000303000100FF, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "VICTORIA", "VIC", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "AUSTRALIAN EASTERN DAYLIGHT TIME", "AEDT", 0x3C00020900010001, 0xC4000303000100FF, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "NEW SOUTH WALES", "NSW", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "AUSTRALIAN EASTERN DAYLIGHT TIME", "AEDT", 0x3C00020900010001, 0xC4000303000100FF, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "QUEENSLAND", "QLD", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "", "", 0x0, 0x0, -1 }, \
	{ "AUSTRALIA", "AU", "AUS", "LORD HOWE ISLAND", "", "LORD HOWE STANDARD TIME", "LHST", 37800, "LORD HOWE DAYLIGHT TIME (+30 MIN)", "LHDT", 0x1E00020900010001, 0xE2000203000100FF, -1 }, \
	{ "AUSTRIA", "AT", "AUT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "AZERBAIJAN", "AZ", "AZE", "", "", "AZERBAIJAN TIME", "AZT", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "BAHAMAS", "BS", "BHS", "", "", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "BAHRAIN", "BH", "BHR", "", "", "ARABIAN STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "BANGLADESH", "BD", "BGD", "", "", "BANGLADESH STANDARD TIME", "BST", 21600, "", "", 0x0, 0x0, -1 }, \
	{ "BARBADOS", "BB", "BRB", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "BARBUDA", "AG", "ATG", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "BELARUS", "BY", "BLR", "", "", "MOSCOW STANDARD TIME", "MSK", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "BELGIUM", "BE", "BEL", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "BELIZE", "BZ", "BLZ", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0, -1 }, \
	{ "BENIN", "BJ", "BEN", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
	{ "BERMUDA", "BM", "BMU", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "BHUTAN", "BT", "BTN", "", "", "BHUTAN TIME", "BTT", 21600, "", "", 0x0, 0x0, -1 }, \
	{ "BOLIVIA", "BO", "BOL", "", "", "BOLIVIA TIME", "BOT", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "BONAIRE", "BQ", "BES", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "BOSNIA", "BA", "BIH", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "BOTSWANA", "BW", "BWA", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "BOUVET ISLAND", "BV", "BVT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "BRAZIL", "BR", "BRA", "", "", "ACRE TIME", "ACT", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "BRAZIL", "BR", "BRA", "", "", "AMAZON TIME", "AMT", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "BRAZIL", "BR", "BRA", "", "", "BRASILIA TIME", "BRT", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "BRAZIL", "BR", "BRA", "", "", "FERNANDO DE NORONHA TIME", "FNT", -7200, "", "", 0x0, 0x0, -1 }, \
	{ "BRITISH VIRGIN ISLANDS", "VG", "VGB", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "BRUNEI", "BN", "BRN", "", "", "BRUNEI TIME", "BNT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "BULGARIA", "BG", "BGR", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "BURKINA FASO", "BF", "BFA", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "BURUNDI", "BI", "BDI", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "CAMBODIA", "KH", "KHM", "", "", "INDOCHINA TIME", "ICT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "CAMEROON", "CM", "CMR", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "CANADA", "CA", "CAN", "NEWFOUNDLAND", "NL", "NEWFOUNDLAND STANDARD TIME", "NST", -12600, "NEWFOUNDLAND DAYLIGHT TIME", "NDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "LABRADOR", "NL", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "NEW BRUNSWICK", "NB", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "NOVA SCOTIA", "NS", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "PRINCE EDWARD ISLAND", "PE", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "ONTARIO", "ON", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "QUEBEC", "QC", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "MANITOBA", "MB", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "SASKATCHEWAN", "SK", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "NUNAVUT", "NU", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "ALBERTA", "AB", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "NORTHWEST TERRITORIES ", "NT", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "BRITISH COLUMBIA", "BC", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CANADA", "CA", "CAN", "YUKON", "YT", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "CAPE VERDE", "CV", "CPV", "", "", "CAPE VERDE TIME", "CVT", -3600, "", "", 0x0, 0x0, -1 }, \
	{ "CAYMAN ISLANDS", "KY", "CYM", "", "", "EASTERN STANDARD TIME", "EST", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "CENTRAL AFRICAN REPUBLIC", "CF", "CAF", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "CHAD", "TD", "TCD", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "CHILE", "CL", "CHL", "", "", "CHILE STANDARD TIME", "CLT", -14400, "CHILE SUMMER TIME", "CLST", 0x3C00000800010001, 0xC4000003000100FF, -1 }, \
	{ "CHILE", "CL", "CHL", "EASTER ISLAND", "", "EASTER ISLAND STANDARD TIME", "EAST", -21600, "EASTER ISLAND SUMMER TIME", "EASST", 0x3C00000800010001, 0xC4000003000100FF, -1 }, \
	{ "CHINA", "CN", "CHN", "", "", "CHINA STANDARD TIME", "CST", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "CHRISTMAS ISLAND (AU)", "CX", "CXR", "", "", "CHRISTMAS ISLAND TIME", "CXT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "COCOS ISLANDS (AU)", "CC", "CCK", "", "", "COCOS ISLAND TIME", "CCT", 23400, "", "", 0x0, 0x0, -1 }, \
	{ "COLOMBIA", "CO", "COL", "", "", "COLOMBIA TIME", "COT", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "COMOROS", "KM", "COM", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "COOK ISLANDS", "CK", "COK", "", "", "COOK ISLAND TIME", "CKT", -36000, "", "", 0x0, 0x0, -1 }, \
	{ "COSTA RICA", "CR", "CRI", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0, -1 }, \
	{ "CROATIA", "HR", "HRV", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600F, -1 }, \
	{ "CUBA", "CU", "CUB", "", "", "CUBA STANDARD TIME", "CST", -18000, "CUBA DAYLIGHT TIME", "CDT", 0x3C00000200020001, 0xC400010A000100FF, -1 }, \
	{ "CURACAO", "CW", "CUW", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "CYPRUS", "CY", "CYP", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "CZECH REPUBLIC", "CZ", "CZE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "DEMOCRATIC REPUBLIC OF THE CONGO", "CD", "COD", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "DEMOCRATIC REPUBLIC OF THE CONGO", "CD", "COD", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "DENMARK", "DK", "DNK", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "DHEKELIA", "", "", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "DJIBOUTI", "DJ", "DJI", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "DOMINICA", "DM", "DMA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "DOMINICAN REPUBLIC", "DO", "DOM", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "EAST TIMOR", "TL", "TLS", "", "", "EAST TIMOR TIME", "TLT", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "ECUADOR", "EC", "ECU", "", "", "ECUADOR TIME", "ECT", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "EGYPT", "EG", "EGY", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "EL SALVADOR", "SV", "SLV", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0, -1 }, \
	{ "EQUATORIAL GUINEA", "GQ", "GNQ", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "ERITREA", "ER", "ERI", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "ESTONIA", "EE", "EST", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "ESWATINI", "SZ", "SWZ", "", "", "SOUTH AFRICA STANDARD TIME", "SAST", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "ETHIOPIA", "ET", "ETH", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "FALKLAND ISLANDS", "FK", "FLK", "", "", "FALKLAND ISLANDS SUMMER TIME", "FKST", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "FAROE ISLANDS", "FO", "FRO", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "FIJI", "FJ", "FJI", "", "", "FIJI TIME", "FJT", 43200, "FIJI DAYLIGHT TIME", "FJDT", 0x3C00020A00010001, 0xC4000300000300FF, -1 }, \
	{ "FINLAND", "FI", "FIN", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "FRANCE", "FR", "FRA", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "FRENCH GUIANA", "GF", "GUF", "", "", "FRENCH GUIANA TIME", "GFT", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "FRENCH POLYNESIA", "PF", "PYF", "TAHITI", "", "TAHITI TIME", "TAHT", -36000, "", "", 0x0, 0x0, -1 }, \
	{ "FRENCH POLYNESIA", "PF", "PYF", "MARQUESAS", "", "MARQUESAS TIME", "MART", -34200, "", "", 0x0, 0x0, -1 }, \
	{ "FRENCH POLYNESIA", "PF", "PYF", "GAMBIER", "", "GAMBIER TIME", "GAMT", -32400, "", "", 0x0, 0x0, -1 }, \
	{ "FRENCH SOUTHERN TERRITORIES", "TF", "ATF", "", "", "FRENCH SOUTHERN AND ANTARCTIC TIME", "TFT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "FUTUNA", "WF", "WLF", "", "", "WALLIS AND FUTUNA TIME", "WFT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "GABON", "GA", "GAB", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "GAMBIA", "GM", "GMB", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "GEORGIA", "GE", "GEO", "", "", "GEORGIA STANDARD TIME", "GET", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "GEORGIA", "GE", "GEO", "", "", "MOSCOW STANDARD TIME", "MSK", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "GERMANY", "DE", "DEU", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "GHANA", "GH", "GHA", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "GIBRALTAR", "GI", "GIB", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "GREECE", "GR", "GRC", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "GREENLAND", "GL", "GRL", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "GREENLAND", "GL", "GRL", "", "", "EASTERN GREENLAND TIME", "EGT", -3600, "EASTERN GREENLAND SUMMER TIME", "EGST", 0x3C00FE0200060001, 0xC400FF09000600FF, -1 }, \
	{ "GREENLAND", "GL", "GRL", "", "", "WESTERN GREENLAND TIME", "WGT", -10800, "WESTERN GREENLAND SUMMER TIME", "WGST", 0x3C00FE0200060001, 0xC400FF09000600FF, -1 }, \
	{ "GREENLAND", "GL", "GRL", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00FE0200060001, 0xC400FF09000600FF, -1 }, \
	{ "GRENADA", "GD", "GRD", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "GRENADINES", "VC", "VCT", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "GUADELOUPE", "GP", "GLP", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "GUAM", "GU", "GUM", "", "", "CHAMORRO STANDARD TIME", "CHST", 36000, "", "", 0x0, 0x0, -1 }, \
	{ "GUATEMALA", "GT", "GTM", "", "", "FRENCH GUIANA TIME", "GFT", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "GUERNSEY", "GG", "GGY", "", "", "GREENWICH MEAN TIME", "GMT", 0, "BRITISH SUMMER TIME", "BST", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "GUINEA", "GN", "GIN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "GUINEA-BISSAU", "GW", "GNB", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "GUYANA", "GY", "GUY", "", "", "GUYANA TIME", "GYT", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "HAITI", "HT", "HTI", "", "", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "HEARD ISLANDS", "HM", "HMD", "", "", "FRENCH SOUTHERN AND ANTARCTIC TIME", "TFT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "HERZEGOVINA", "BA", "BIH", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "HOLY SEE", "VA", "VAT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "HONDURAS", "HN", "HND", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0, -1 }, \
	{ "HONG KONG", "HK", "HKG", "", "", "HONG KONG TIME", "HKT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "HUNGARY", "HU", "HUN", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "ICELAND", "IS", "ISL", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "INDIA", "IN", "IND", "", "", "INDIA TIME ZONE", "IST", 19800, "", "", 0x0, 0x0, -1 }, \
	{ "INDONESIA", "ID", "IDN", "", "", "WESTERN INDONESIAN TIME", "WIB", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "INDONESIA", "ID", "IDN", "", "", "CENTRAL INDONESIAN TIME", "WITA", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "INDONESIA", "ID", "IDN", "", "", "EASTERN INDONESIAN TIME", "WIT", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "IRAN", "IR", "IRN", "", "", "IRAN STANDARD TIME", "IRST", 12600, "IRAN DAYLIGHT TIME", "IRDT", 0x3C0000021600FF01, 0xC40000081600FFFF, -1 }, \
	{ "IRAQ", "IQ", "IRQ", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "IRELAND", "IE", "IRL", "", "", "GREENWICH MEAN TIME", "GMT", 0, "IRISH STANDARD TIME", "IST", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "ISLE OF MAN", "IM", "IMN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "BRITISH SUMMER TIME", "BST", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "ISRAEL", "IL", "ISR", "", "", "ISRAEL STANDARD TIME", "IST", 7200, "ISRAEL DAYLIGHT TIME", "IDT", 0x3C00D20200060001, 0xC4000209000600FF, -1 }, \
	{ "ITALY", "IT", "ITA", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "IVORY COAST", "CI", "CIV", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "JAMAICA", "JM", "JAM", "", "", "EASTERN STANDARD TIME", "EST", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "JAN MAYEN", "SJ", "SJM", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "JAPAN", "JP", "JPN", "", "", "JAPAN STANDARD TIME", "JST", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "JERSEY", "JE", "JEY", "", "", "GREENWICH MEAN TIME", "GMT", 0, "BRITISH SUMMER TIME", "BST", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "JORDAN", "JO", "JOR", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00000200060501, 0xC4000109000605FF, -1 }, \
	{ "KAZAKHSTAN", "KZ", "KAZ", "", "", "ORAL TIME", "ORAT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "KAZAKHSTAN", "KZ", "KAZ", "", "", "ALMA-ATA TIME", "ALMT", 21600, "", "", 0x0, 0x0, -1 }, \
	{ "KEELING ISLANDS", "CC", "CCK", "", "", "COCOS ISLANDS TIME", "CCT", 23400, "", "", 0x0, 0x0, -1 }, \
	{ "KENYA", "KE", "KEN", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "KIRIBATI", "KI", "KIR", "", "", "GILBERT ISLAND TIME", "GILT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "KIRIBATI", "KI", "KIR", "", "", "PHOENIX ISLAND TIME", "PHOT", 46800, "", "", 0x0, 0x0, -1 }, \
	{ "KIRIBATI", "KI", "KIR", "", "", "LINE ISLANDS TIME", "LINT", 50400, "", "", 0x0, 0x0, -1 }, \
	{ "KOSOVO", "XK", "XKX", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "KUWAIT", "KW", "KWT", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "KYRGYZSTAN", "KG", "KGZ", "", "", "KYRGYZSTAN TIME", "KGT", 21600, "", "", 0x0, 0x0, -1 }, \
	{ "LAOS", "LA", "LAO", "", "", "INDOCHINA TIME", "ICT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "LATVIA", "LV", "LVA", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "LEBANON", "LB", "LBN", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00000200060001, 0xC4000009000600FF, -1 }, \
	{ "LESOTHO", "LS", "LSO", "", "", "SOUTH AFRICA STANDARD TIME", "SAST", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "LIBERIA", "LR", "LBR", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "LIBYA", "LY", "LBY", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "LIECHTENSTEIN", "LI", "LIE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "LITHUANIA", "LT", "LTU", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "LUXEMBOURG", "LU", "LUX", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "MACAU", "MO", "MAC", "", "", "CHINA STANDARD TIME", "CST", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "MADAGASCAR", "MG", "MDG", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "MALAWI", "MW", "MWI", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "MALAYSIA", "MY", "MYS", "", "", "MALAYSIA TIME", "MYT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "MALDIVES", "MV", "MDV", "", "", "MALDIVES TIME", "MVT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "MALI", "ML", "MLI", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "MALTA", "MT", "MLT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "MARSHALL ISLANDS", "MH", "MHL", "", "", "MARSHALL ISLANDS TIME", "MHT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "MARTINIQUE", "MQ", "MTQ", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "MAURITANIA", "MR", "MRT", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "MAURITIUS", "MU", "MUS", "", "", "MAURITIUS TIME", "MUT", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "MAYOTTE", "YT", "MYT", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "MCDONALD ISLANDS", "US", "USA", "", "", "ALASKA STANDARD TIME", "AKST", 32400, "ALASKA DAYLIGHT TIME", "AKDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "MCDONALD ISLANDS", "HM", "HMD", "", "", "FRENCH SOUTHERN AND ANTARCTIC TIME", "TFT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "MEXICO", "MX", "MEX", "", "", "EASTERN STANDARD TIME", "EST", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "MEXICO", "MX", "MEX", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020300010001, 0xC4000209000600FF, -1 }, \
	{ "MEXICO", "MX", "MEX", "", "", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020300010001, 0xC4000209000600FF, -1 }, \
	{ "MEXICO", "MX", "MEX", "", "", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020300010001, 0xC4000209000600FF, -1 }, \
	{ "MICRONESIA", "FM", "FSM", "", "", "CHUUK TIME", "CHUT", 36000, "", "", 0x0, 0x0, -1 }, \
	{ "MICRONESIA", "FM", "FSM", "", "", "POHNPEI STANDARD TIME", "PONT", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "MICRONESIA", "FM", "FSM", "", "", "KOSRAE TIME", "KOST", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "MIDWAY", "UM", "UMI", "", "", "SAMOA STANDARD TIME", "SST", -39600, "", "", 0x0, 0x0, -1 }, \
	{ "MIQUELON", "PM", "SPM", "", "", "PIERRE & MIQUELON STANDARD TIME", "PMST", -10800, "PIERRE & MIQUELON DAYLIGHT TIME", "PMDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "MOLDOVA", "MD", "MDA", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "MONACO", "MC", "MCO", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "MONGOLIA", "MN", "MNG", "", "", "HOVD TIME", "HOVT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "MONGOLIA", "MN", "MNG", "", "", "ULAANBAATAR TIME", "ULAT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "MONGOLIA", "MN", "MNG", "", "", "CHOIBALSAN TIME", "CHOT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "MONTENEGRO", "ME", "MNE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "MONTSERRAT", "MS", "MSR", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "MOROCCO", "MA", "MAR", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "", "", 0x0, 0x0, -1 }, \
	{ "MOZAMBIQUE", "MZ", "MOZ", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "MYANMAR", "MM", "MMR", "", "", "MYANMAR TIME", "MMT", 23400, "", "", 0x0, 0x0, -1 }, \
	{ "NAMIBIA", "NA", "NAM", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "NAURU", "NR", "NRU", "", "", "NAURU TIME", "NRT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "NEPAL", "NP", "NPL", "", "", "NEPAL TIME", "NPT", 20700, "", "", 0x0, 0x0, -1 }, \
	{ "NETHERLANDS", "NL", "NLD", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "NETHERLANDS ANTILLES", "AN", "ANT", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "NEVIS", "KN", "KNA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "NEW CALEDONIA", "NC", "NCL", "", "", "NEW CALEDONIA TIME", "NCT", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "NEW ZEALAND", "NZ", "NZL", "", "", "NEW ZEALAND STANDARD TIME", "NZST", 43200, "NEW ZEALAND DAYLIGHT TIME", "NZDT", 0x3C00020800060001, 0xC4000303000100FF, -1 }, \
	{ "NEW ZEALAND", "NZ", "NZL", "CHATHAM ISLAND", "", "CHATHAM ISLAND STANDARD TIME", "CHAST", 45900, "CHATHAM ISLAND DAYLIGHT TIME", "CHADT", 0x3C00020800060001, 0xC4000303000100FF, -1 }, \
	{ "NICARAGUA", "NI", "NIC", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0, -1 }, \
	{ "NIGER", "NE", "NER", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "NIGERIA", "NG", "NGA", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "NIUE", "NU", "NIU", "", "", "NIUE TIME", "NUT", -39600, "", "", 0x0, 0x0, -1 }, \
	{ "NORFOLK ISLAND", "NF", "NFK", "", "", "NORFOLK TIME", "NFT", 39600, "NORFOLK DAYLIGHT TIME", "NFDT", 0x3C00020900010001, 0xC4000303000100FF, -1 }, \
	{ "NORTH KOREA", "KP", "PRK", "", "", "KOREA STANDARD TIME", "KST", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "NORTH MACEDONIA", "MK", "MKD", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "NORTHERN MARIANA ISLANDS", "MP", "MNP", "", "", "CHAMORRO STANDARD TIME", "CHST", 36000, "", "", 0x0, 0x0, -1 }, \
	{ "NORWAY", "NO", "NOR", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "OMAN", "OM", "OMN", "", "", "GULF STANDARD TIME", "GST", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "PAKISTAN", "PK", "PAK", "", "", "PAKISTAN STANDARD TIME", "PKT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "PALAU", "PW", "PLW", "", "", "PALAU TIME", "PWT", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "PALESTINE", "PS", "PSE", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00D20200060001, 0xC4000209000600FF, -1 }, \
	{ "PANAMA", "PA", "PAN", "", "", "EASTERN STANDARD TIME", "EST", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "PAPUA NEW GUINEA", "PG", "PNG", "", "", "PAPUA NEW GUINEA TIME", "PGT", 36000, "", "", 0x0, 0x0, -1 }, \
	{ "PAPUA NEW GUINEA", "PG", "PNG", "BOUGAINVILLE", "", "BOUGAINVILLE STANDARD TIME", "BST", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "PARAGUAY", "PY", "PRY", "", "", "PARAGUAY TIME", "PYT", -14400, "PARAGUAY SUMMER TIME", "PYST", 0x3C00000900010001, 0xC4000002000400FF, -1 }, \
	{ "PERU", "PE", "PER", "", "", "PERU TIME", "PET", -18000, "", "", 0x0, 0x0, -1 }, \
	{ "PHILIPPINES", "PH", "PHL", "", "", "PHILIPPINE TIME", "PHT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "PITCAIRN ISLANDS", "PN", "PCN", "", "", "PITCAIRN STANDARD TIME", "PST", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "POLAND", "PL", "POL", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "PORTUGAL", "PT", "PRT", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "PORTUGAL", "PT", "PRT", "", "", "AZORES TIME", "AZOT", 3600, "AZORES SUMMER TIME", "AZOST", 0x3C00000200060001, 0xC4000109000600FF, -1 }, \
	{ "PRINCIPE", "ST", "STP", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "PUERTO RICO", "PR", "PRI", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "QATAR", "QA", "QAT", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "REPUBLIC OF THE CONGO", "CG", "COG", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "REUNION", "RE", "REU", "", "", "REUNION TIME", "RET", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "ROMANIA", "RO", "ROU", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "RUSSIA", "RU", "RUS", "KALININGRAD", "", "EASTERN EUROPEAN TIME", "EET", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "MOSCOW", "", "MOSCOW STANDARD TIME", "MSK", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "SAMARA", "", "SAMARA TIME", "SAMT", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "YEKATERINBURG", "", "YEKATERINBURG TIME", "YEKT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "OMSK", "", "OMSK STANDARD TIME", "OMST", 21600, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "KRASNOYARSK", "", "KRASNOYARSK TIME", "KRAT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "NOVOSIBIRSK", "", "NOVOSIBIRSK TIME", "NOVT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "IRKUTSK", "", "IRKUTSK TIME", "IRKT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "YAKUTSK", "", "YAKUTSK TIME", "YAKT", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "VLADIVOSTOK", "", "VLADIVOSTOK TIME", "VLAT", 36000, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "MAGADAN", "", "MAGADAN TIME", "MAGT", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "SAKHALIN", "", "SAKHALIN TIME", "SAKT", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "SREDNEKOLYMSK", "", "SREDNEKOLYMSK TIME", "SRED", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "ANADYR", "", "ANADYR TIME", "ANAT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "RUSSIA", "RU", "RUS", "KAMCHATKA", "", "KAMCHATKA TIME", "PETT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "RWANDA", "RW", "RWA", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "SABA", "BQ", "BES", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SAINT BARTHELEMY", "BL", "BLM", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SAINT HELENA", "SH", "SHN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "SAINT KITTS", "KN", "KNA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SAINT LUCIA", "LC", "LCA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SAINT MARTIN", "MF", "MAF", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SAINT PIERRE", "PM", "SPM", "", "", "PIERRE & MIQUELON STANDARD TIME", "PMST", -10800, "PIERRE & MIQUELON DAYLIGHT TIME", "PMDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "SAINT VINCENT", "VC", "VCT", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SAMOA", "WS", "WSM", "", "", "WEST SAMOA TIME", "WST", 46800, "WEST SAMOA TIME", "WST", 0x3C00030800060001, 0xC4000403000100FF, -1 }, \
	{ "SAN MARINO", "SM", "SMR", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SAO TOME", "ST", "STP", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "SAUDI ARABIA", "SA", "SAU", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "SENEGAL", "SN", "SEN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "SERBIA", "RS", "SRB", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SEYCHELLES", "SC", "SYC", "", "", "SEYCHELLES TIME", "SCT", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "SIERRA LEONE", "SL", "SLE", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "SINGAPORE", "SG", "SGP", "", "", "SINGAPORE TIME", "SGT", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "SINT EUSTATIUS", "BQ", "BES", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SINT MAARTEN", "SX", "SXM", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "SLOVAKIA", "SK", "SVK", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SLOVENIA", "SI", "SVN", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SOLOMON ISLANDS", "SB", "SLB", "", "", "SOLOMON ISLANDS TIME", "SBT", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "SOMALIA", "SO", "SOM", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "SOUTH AFRICA", "ZA", "ZAF", "", "", "SOUTH AFRICA STANDARD TIME", "SAST", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "SOUTH AFRICA", "ZA", "ZAF", "MARION ISLAND", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "SOUTH GEORGIA ISLAND", "GS", "SGS", "", "", "SOUTH GEORGIA TIME", "GST", -7200, "", "", 0x0, 0x0, -1 }, \
	{ "SOUTH KOREA", "KR", "KOR", "", "", "KOREA STANDARD TIME", "KST", 32400, "", "", 0x0, 0x0, -1 }, \
	{ "SOUTH SANDWICH ISLANDS", "GS", "SGS", "", "", "SOUTH GEORGIA TIME", "GST", -7200, "", "", 0x0, 0x0, -1 }, \
	{ "SOUTH SUDAN", "SS", "SSD", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "SPAIN", "ES", "ESP", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SPAIN", "ES", "ESP", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SRI LANKA", "LK", "LKA", "", "", "INDIA STANDARD TIME", "IST", 19800, "", "", 0x0, 0x0, -1 }, \
	{ "SUDAN", "SD", "SDN", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "SURINAME", "SR", "SUR", "", "", "SURINAME TIME", "SRT", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "SVALBARD", "SJ", "SJM", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SWAZILAND", "SZ", "SWZ", "", "", "SOUTH AFRICA STANDARD TIME", "SAST", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "SWEDEN", "SE", "SWE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SWITZERLAND", "CH", "CHE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "SYRIA", "SY", "SYR", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00000200060501, 0xC4000009000605FF, -1 }, \
	{ "TAIWAN", "TW", "TWN", "", "", "CHINA STANDARD TIME", "CST", 28800, "", "", 0x0, 0x0, -1 }, \
	{ "TAJIKISTAN", "TJ", "TJK", "", "", "TAJIKISTAN TIME", "TJT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "TANZANIA", "TZ", "TZA", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "THAILAND", "TH", "THA", "", "", "INDOCHINA TIME", "ICT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "TOBAGO", "TT", "TTO", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "TOGO", "TG", "TGO", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "TOKELAU", "TK", "TKL", "", "", "TOKELAU TIME", "TKT", 46800, "", "", 0x0, 0x0, -1 }, \
	{ "TONGA", "TO", "TON", "", "", "TONGA TIME", "TOT", 46800, "", "", 0x0, 0x0, -1 }, \
	{ "TRINIDAD", "TT", "TTO", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "TRISTAN DA CUNHA", "SH", "SHN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0, -1 }, \
	{ "TUNISIA", "TN", "TUN", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "TURKEY", "TR", "TUR", "", "", "TURKEY TIME", "TRT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "TURKMENISTAN", "TM", "TKM", "", "", "TURKMENISTAN TIME", "TMT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "TURKS AND CAICOS", "TC", "TCA", "", "", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "TUVALU", "TV", "TUV", "", "", "TUVALU TIME", "TVT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "UGANDA", "UG", "UGA", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "UKRAINE", "UA", "UKR", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF, -1 }, \
	{ "UKRAINE", "UA", "UKR", "", "", "MOSCOW STANDARD TIME", "MSK", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "UNITED ARAB EMIRATES", "AE", "ARE", "", "", "GULF STANDARD TIME", "GST", 14400, "", "", 0x0, 0x0, -1 }, \
	{ "UNITED KINGDOM", "GB", "GBR", "", "", "GREENWICH MEAN TIME", "GMT", 0, "BRITISH SUMMER TIME", "BST", 0x3C00010200060001, 0xC4000209000600FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "ALABAMA", "AL", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "ALASKA", "AK", "ALASKA STANDARD TIME", "AKST", -32400, "ALASKA DAYLIGHT TIME", "AKDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "ALASKA", "AK", "HAWAII-ALEUTIAN STANDARD TIME", "HST", -36000, "HAWAII-ALEUTIAN DAYLIGHT TIME", "HDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "ARIZONA", "AZ", "MOUNTAIN STANDARD TIME", "MST", -25200, "", "", 0x0, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "ARKANSAS", "AR", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "CALIFORNIA", "CA", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "COLORADO", "CO", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "CONNECTICUT", "CT", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "DELAWARE", "DE", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "DISTRICT OF COLUMBIA", "DC", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "FLORIDA", "FL", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "FLORIDA", "FL", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "GEORGIA", "GA", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "HAWAII", "HI", "HAWAII-ALEUTIAN STANDARD TIME", "HST", -36000, "", "", 0x0, 0x0, -1 }, \
	{ "UNITED STATES", "US", "USA", "IDAHO", "ID", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "IDAHO", "ID", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "ILLINOIS", "IL", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "INDIANA", "IN", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "INDIANA", "IN", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "IOWA", "IA", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "KANSAS", "KS", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "KANSAS", "KS", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "KENTUCKY", "KY", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "KENTUCKY", "KY", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "LOUISIANA", "LA", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MAINE", "ME", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MARYLAND", "MD", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MASSACHUSETTS", "MA", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MICHIGAN", "MI", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MICHIGAN", "MI", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MINNESOTA", "MN", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MISSISSIPPI", "MS", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MISSOURI", "MO", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "MONTANA", "MT", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEBRASKA", "NE", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEBRASKA", "NE", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEVADA", "NV", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEVADA", "NV", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEW HAMPSHIRE", "NH", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEW JERSEY", "NJ", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEW MEXICO", "NM", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NEW YORK", "NY", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NORTH CAROLINA", "NC", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NORTH DAKOTA", "ND", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "NORTH DAKOTA", "ND", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "OHIO", "OH", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "OKLAHOMA", "OK", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "OREGON", "OR", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "OREGON", "OR", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "PENNSYLVANIA", "PA", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "RHODE ISLAND", "RI", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "SOUTH CAROLINA", "SC", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "SOUTH DAKOTA", "SD", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "SOUTH DAKOTA", "SD", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "TENNESSEE", "TN", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "TENNESSEE", "TN", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "TEXAS", "TX", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "TEXAS", "TX", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "UTAH", "UT", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "VERMONT", "VT", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "VIRGINIA", "VA", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "WASHINGTON", "WA", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "WEST VIRGINIA", "WV", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "WISCONSIN", "WI", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES", "US", "USA", "WYOMING", "WY", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF, -1 }, \
	{ "UNITED STATES VIRGIN ISLANDS", "VI", "VIR", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "URUGUAY", "UY", "URY", "", "", "URUGUAY TIME", "UYT", -10800, "", "", 0x0, 0x0, -1 }, \
	{ "UZBEKISTAN", "UZ", "UZB", "", "", "UZBEKISTAN TIME", "UZT", 18000, "", "", 0x0, 0x0, -1 }, \
	{ "VANUATU", "VU", "VUT", "", "", "VANUATU TIME", "VUT", 39600, "", "", 0x0, 0x0, -1 }, \
	{ "VATICAN CITY", "VA", "VAT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF, -1 }, \
	{ "VENEZUELA", "VE", "VEN", "", "", "VENEZUELAN STANDARD TIME", "VET", -14400, "", "", 0x0, 0x0, -1 }, \
	{ "VIETNAM", "VN", "VNM", "", "", "INDOCHINA TIME", "ICT", 25200, "", "", 0x0, 0x0, -1 }, \
	{ "WALLIS", "WF", "WLF", "", "", "WALLIS AND FUTUNA TIME", "WFT", 43200, "", "", 0x0, 0x0, -1 }, \
	{ "WESTERN SAHARA", "EH", "ESH", "", "", "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 3600, "", "", 0x0, 0x0, -1 }, \
	{ "YEMEN", "YE", "YEM", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0, -1 }, \
	{ "ZAMBIA", "ZM", "ZMB", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 }, \
	{ "ZIMBABWE", "ZW", "ZWE", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0, -1 } \
}

#define TZ_COUNTRY_ALIASES_ENTRIES_m13 16
#define TZ_COUNTRY_ALIASES_TABLE_m13 { \
	{ "CHINA", "PEOPLE'S REPUBIC OF CHINA" }, \
	{ "CHINA", "PEOPLES REPUBIC OF CHINA" }, \
	{ "CZECH REPUBLIC", "CZECHIA" }, \
	{ "RUSSIA", "RUSSIAN FEDERATION" }, \
	{ "TURKS AND CAICOS", "TURKS & CAICOS" }, \
	{ "TURKS AND CAICOS", "TURKS" }, \
	{ "TURKS AND CAICOS", "CAICOS" }, \
	{ "UNITED KINGDOM", "UNITED KINGDOM OF GREAT BRITAIN & NORTHERN IRELAND" }, \
	{ "UNITED KINGDOM", "UNITED KINGDOM OF GREAT BRITAIN AND NORTHERN IRELAND" }, \
	{ "UNITED KINGDOM", "BRITAIN" }, \
	{ "UNITED KINGDOM", "ENGLAND" }, \
	{ "UNITED KINGDOM", "GREAT BRITAIN" }, \
	{ "UNITED KINGDOM", "NORTHERN IRELAND" }, \
	{ "UNITED KINGDOM", "SCOTLAND" }, \
	{ "UNITED KINGDOM", "WALES" }, \
	{ "UNITED STATES", "UNITED STATES OF AMERICA" } \
}

#define TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m13 1
#define TZ_COUNTRY_ACRONYM_ALIASES_TABLE_m13 { \
	{ "GB", "UK" } \
}



//**********************************************************************************//
//***************************** DATBASE (DB) FUNCTIONS ***************************//
//**********************************************************************************//

// Currently only PostGres databases are supported.

#ifdef DATABASE_m13

// Defines
#define DB_EXPECTED_ROWS_NO_ENTRY_m13	((si4) -1)

// Prototypes
tern		DB_check_result_m13(PGresult *result);
PGresult	*DB_execute_command_m13(PGconn *conn, const si1 *command, si4 *rows, si4 expected_rows);

#endif // DATABASE_m13



//***********************************************************************//
//***************** MED VERSIONS OF STANDARD C FUNCTIONS ****************//
//***********************************************************************//

// the calling interfaces to these function are generally those defined by posix
// in some cases additional functionality are added via special input values or varargs
// in some cases behavior is more robust than standard functions
// in a few cases return values are modified to be more useful
// see function definition comments for specific changes
// a few non-standard functions that are closely related to standard functions are also included here
// note FILE * types have been changed to void *, where appropriate, to allow use of either FILE * or FILE_m13 * types without casts or compiler warnings

#if defined MACOS_m13 || defined LINUX_m13
	typedef	struct stat		struct_stat_m13;
#endif // MACOS_m13 || LINUX_m13
#ifdef WINDOWS_m13
	typedef	struct _stat64		struct_stat_m13;
#endif // WINDOWS_m13


si4		asprintf_m13(si1 **target, const si1 *fmt, ...);
size_t		calloc_size_m13(void *address, size_t el_size);
tern		cp_m13(const si1 *path, const si1 *new_path);  // copy
si4		errno_m13(void);
void		errno_reset_m13(void); // zero errno before calling functions that may set it
void		exit_exec_m13(const si1 *function, const si4 line, si4 status);
si4		fclose_m13(void *fp);  // pass FILE *, FILE_m13 *, or to null local variable, FILE_m13 **
si4		fileno_m13(void *fp);
tern		fisopen_m13(void *fp);
si8		flen_m13(void *fp);
si4		flock_m13(void *fp, si4 operation, ...); // varargs(FLOCK_TIMEOUT_m13 bit set): const si1 *nap_str (string to pass to nap_m13())
							 // varargs(fp == FILE *): const si1 *file_path, const si1 *nap_str (must pass something for nap_str, but can be NULL)
FILE_m13	*fopen_m13(const si1 *path, const si1 *mode, ...); // varargs(mode == NULL): si1 *mode, si4 flags, ui2 (as si4) permissions
si4		fprintf_m13(void *fp, const si1 *fmt, ...);
si4		fputc_m13(si4 c, void *fp);
size_t		fread_m13(void *ptr, si8 el_size, size_t n_elements, void *fp, ...); // (el_size negative): non_blocking read
tern		freeable_m13(void *address);
void		*freopen_m13(const si1 *path, const si1 *mode, void *fp);
si4		fscanf_m13(void *fp, const si1 *fmt, ...);
si4		fseek_m13(void *fp, si8 offset, si4 whence);
si4		fstat_m13(si4 fd, struct_stat_m13 *sb);
si8		ftell_m13(void *fp);
si4		ftruncate_m13(void *fp, off_t len);
size_t		fwrite_m13(void *ptr, si8 el_size, size_t n_elements, void *fp, ...);   // (el_size negative): non-blocking write; varargs(fp == NULL): FILE *fp (as void *), si1 *path
si1		*getcwd_m13(si1 *buf, size_t size);
pid_t_m13	getpid_m13(void);
pid_t_m13	gettid_m13(void);
void		isem_chown_m13(isem_t_m13 *isem, pid_t_m13 tid);
void		isem_dec_m13(isem_t_m13 *sem);
void		isem_destroy_m13(isem_t_m13 *isem);
ui4		isem_getcnt_m13(isem_t_m13 *isem);
void		isem_inc_m13(isem_t_m13 *sem);
isem_t_m13	*isem_init_m13(isem_t_m13 *isem, ui4 init_val, const si1 *nap_str, const si1 *name, tern free_on_destroy);
void		isem_own_m13(isem_t_m13 *isem, tern own);
void		isem_setcnt_m13(isem_t_m13 *isem, ui4 count);
tern		isem_trydec_m13(isem_t_m13 *sem);
tern		isem_tryinc_m13(isem_t_m13 *sem);
tern		isem_tryown_m13(isem_t_m13 *isem, tern own);
tern		isem_trysetcnt_m13(isem_t_m13 *isem, ui4 count);
tern		isem_trywait_m13(isem_t_m13 *sem);
tern		isem_trywait_noinc_m13(isem_t_m13 *isem);
void		isem_wait_m13(isem_t_m13 *sem);
void		isem_wait_noinc_m13(isem_t_m13 *isem);
size_t		malloc_size_m13(void *address);
tern		md_m13(const si1 *dir);  // synonym for mkdir()
void		*memset_m13(void *ptr, si4 val, size_t n_members, ...); // vargarg(n_members negative): const void *el_val (val == el_size)
tern		mkdir_m13(const si1 *dir);
tern		mlock_m13(void *addr, size_t len, ...); // varargs(addr == NULL): void *addr, size_t len, tern (as si4) zero_data
void		*memcpy_m13(void *target, const void *source, size_t n_bytes);
void		*memmove_m13(void *target, const void *source, size_t n_bytes);
si4		mprotect_m13(void *address, size_t len, si4 protection);
tern		munlock_m13(void *addr, size_t len);
tern		mv_m13(const si1 *path, const si1 *new_path);  // move
void		nanosleep_m13(struct timespec *tv);
void		nap_m13(const si1 *nap_str);
struct timespec	*nap_timespec_m13(const si1 *nap_str, struct timespec *nap);
si4		pthread_equal_m13(pthread_t_m13 thread_1, pthread_t_m13 thread_2);
void		pthread_exit_m13(void *ptr);
si1		*pthread_getname_m13(pthread_t_m13 thread, si1 *thread_name, size_t name_len);
si1		*pthread_getname_id_m13(pid_t_m13 _id, si1 *thread_name, size_t name_len);
si4		pthread_join_m13(pthread_t_m13 thread, void **value_ptr);
si4		pthread_kill_m13(pthread_t_m13 thread, si4 signal);
si4		pthread_mutex_destroy_m13(pthread_mutex_t_m13 *mutex);
si4		pthread_mutex_init_m13(pthread_mutex_t_m13 *mutex, pthread_mutexattr_t_m13 *attr);
si4		pthread_mutex_lock_m13(pthread_mutex_t_m13 *mutex);
si4		pthread_mutex_trylock_m13(pthread_mutex_t_m13 *mutex);
si4		pthread_mutex_unlock_m13(pthread_mutex_t_m13 *mutex);
pthread_t_m13	pthread_self_m13(void);
si4		printf_m13(const si1 *fmt, ...);
si4		putc_m13(si4 c, void *fp);
si4		putch_m13(si4 c);
si4		putchar_m13(si4 c);
si4		random_m13(void); // 31-bit random number using system generator (posix standard)
ui4		rand32_m13(void); // 32-bit random number using system generator
ui4		rand32_med_m13(void); // 32-bit random number using medlib generator (replicable sequences across platforms)
ui4		rand32_med_wz_m13(volatile ui4 *w, volatile ui4 *z); // faster, less convenient, version of rand32_med_m13()
ui8		rand64_m13(void); // 64-bit random number using system random number generator
ui8		rand64_med_m13(void); // 64-bit random number using medlib generator (replicable sequences across platforms)
ui8		rand64_med_wz_m13(volatile ui4 *w, volatile ui4 *z); // faster, less convenient, version of rand64_med_m13()
tern		rm_m13(const si1 *path);  // remove
si4		scanf_m13(const si1 *fmt, ...);
si4		sem_init_m13(sem_t_m13 *sem, si4 shared, ui4 init_val);
sem_t_m13	*sem_open_m13(const si1 *name, si4 o_flags, ...);  // (MacOS only) varargs(o_flags & O_CREAT): mode_t mode (as ui4), ui4 init_val
si4		sem_post_m13(sem_t_m13 *sem);
si4		sem_trywait_m13(sem_t_m13 *sem);
si4		sem_wait_m13(sem_t_m13 *sem);
si4		sprintf_m13(si1 *target, const si1 *fmt, ...);
si4		snprintf_m13(si1 *target, si4 target_field_bytes, const si1 *fmt, ...);
void		srand_med_m13(ui4 seed); // seed medlib random number generator
void		srand_med_wz_m13(ui4 seed, volatile ui4 *w, volatile ui4 *z); // faster, less convenient, version of srand_med_m13()
void		srandom_m13(ui4 seed); // seed system random number generator
si4		sscanf_m13(si1 *target, const si1 *fmt, ...);
si4		stat_m13(const si1 *path, struct_stat_m13 *sb);
si8		strcat_m13(si1 *target, const si1 *source);
size_t		strchar_m13(const si1 *string);
si4		strcmp_m13(const si1 *string_1, const si1 *string_2);
si8		strcpy_m13(si1 *target, const si1 *source);
si8		strncat_m13(si1 *target, const si1 *source, size_t n_chars);
si4		strncmp_m13(const si1 *string_1, const si1 *string_2, size_t n_chars);
si8		strncpy_m13(si1 *target, const si1 *source, size_t n_chars);
si4		system_m13(const si1 *command, ...); // varargs(command = NULL): const si1 *command, tern (as si4) null_std_streams, ui4 behavior;
si4		system_pipe_m13(si1 **buffer_ptr, si8 buf_len, const si1 *command, ui4 flags, ...); // varargs(BEHAVIOR_PASSED_m13 set): ui4 behavior)
												    // varargs(SP_SEPARATE_STREAMS_m13 set): si1 **e_buffer_ptr, si8 e_buf_len
												    // note if both passed, behavior is first argument
void		tempclean_m13(void);
FILE_m13	*tempfile_m13(void);
si1		*tempnam_m13(si1 *path);
tern		touch_m13(const si1 *path);
si4		truncate_m13(const si1 *path, off_t len);
si4		vasprintf_m13(si1 **target, const si1 *fmt, va_list args);
si4		vfprintf_m13(void *fp, const si1 *fmt, va_list args);
si4		vprintf_m13(const si1 *fmt, va_list args);
si4		vsnprintf_m13(si1 *target, si4 target_field_bytes, const si1 *fmt, va_list args);
si4		vsprintf_m13(si1 *target, const si1 *fmt, va_list args);

// standard functions with AT_DEBUG_m13 versions
#ifndef AT_DEBUG_m13 // use these protoypes in all cases, defines will convert if needed
void	*calloc_m13(size_t n_members, si8 el_size); // (el_size negative): level headers flag
void	**calloc_2D_m13(size_t dim1, size_t dim2, si8 el_size); // (el_size negative): level header flag
void	free_m13(void *ptr);
void	free_2D_m13(void **ptr, size_t dim1);
void	*malloc_m13(si8 n_bytes); // (n_bytes negative): level header flag
void	**malloc_2D_m13(size_t dim1, si8 dim2_bytes); // (dim2_bytes negative): level headers flag
void	*realloc_m13(void *ptr, si8 n_bytes); // (n_bytes negative): level header flag
void	**realloc_2D_m13(void **ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2_bytes, si8 new_dim2_bytes); // (new_dim2_bytes negative): level headers flag
void	*recalloc_m13(void *ptr, size_t curr_members, size_t new_members, si8 el_size); // (el_size negative): level header flag
void	**recalloc_2D_m13(void **ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, si8 el_size); // (el_size negative): level header flag
#endif // AT_DEBUG_m13

#define exit_m13(arg)			exit_exec_m13(__FUNCTION__, __LINE__, arg) // call with "G_pop_function_m13(void)" prototype

// "loop"s below deal with terminal semicolon (optimized out on compile)
#ifdef FT_DEBUG_m13
	#define return_m13(arg)		do { G_pop_function_m13(); return(arg); } while(0)
	#define return_void_m13		do { G_pop_function_m13(); return; } while(0)
	#define thread_return_m13(arg)	do { G_pop_function_m13(); G_thread_exit_m13(); return((pthread_rval_m13) arg); } while(0) // clears behavior stack & function stack, if it exists
	#define thread_return_null_m13	do { G_pop_function_m13(); G_thread_exit_m13(); return((pthread_rval_m13) 0); } while(0) // clears behavior stack & function stack, if it exists
#else
	#define return_m13(arg)		return(arg)
	#define return_void_m13		return
	#define thread_return_m13(arg)	do { G_thread_exit_m13(); return((pthread_rval_m13) arg); } while(0) // clears behavior stack & function stack, if it exists
	#define thread_return_null_m13	do { G_thread_exit_m13(); return((pthread_rval_m13) 0); } while(0) // clears behavior stack & function stack, if it exists
#endif // FT_DEBUG_m13



//**********************************************************************************//
//********************************* MED Records **********************************//
//**********************************************************************************//

#include "medrec_m13.h"

#endif // MEDLIB_IN_m13

