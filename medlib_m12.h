
#ifndef MEDLIB_IN_m12
#define MEDLIB_IN_m12

//**********************************************************************************//
//*******************************  MED 1.0.2 C Library  ****************************//
//**********************************************************************************//


// Multiscale Electrophysiology Data (MED) Format Software Library, Version 1.0.2
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
// Basic UTF-8 manipulation routines are included in the library, with attribution, for convenience.

// Error detection is implemented with 32-bit cyclic redundancy checksums (CRCs).
// Basic CRC-32 manipulation routines are included in the library, with attribution, for convenience.


// USAGE:

// The library is optimized for 64-bit operating systems on 64-bit processors with 64-bit words and addressing.
// However, it can be used with in 32-bit contexts without modification at a performance cost.

// The library is written with tab width = indent width = 8 spaces and a monospaced font.
// Tabs are tabs characters, not spaces.
// Set your editor preferences to these for intended alignment.

// The library contains some non-standard structures:
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
// "_m12" indicates "MED format major version 1, library version 2"
// "_m21" indicates "MED format major version 2, library version 1" (for MED 2)
// "_m213" indicates "MED format major version 2, library version 13" (for MED 2)

// All library versions associated with a particular major format version are guaranteed to work on MED files of that major version.
// Minor format versions may add fields to the format in protected regions, but no preexisting fields will be removed or moved.
// Only library versions released on or afer a minor version will make use of new fields, and only if the minor version of the files contains them.
// Backward compatibility will be maintained between major versions if practical.



//**********************************************************************************//
//*********************************  Library Includes  *****************************//
//**********************************************************************************//

#include "targets_m12.h"

#ifdef WINDOWS_m12
// the following is necessary to include <winsock2.h> (or can define WIN32_LEAN_AND_MEAN, but excludes a lot of stuff)
// winsock2.h has to be included before windows.h, but requires WIN32 to be defined, which is usually defined by windows.h
// NEED_WIN_SOCKETS_m12 is defined in "targets.h"
	#ifdef NEED_WIN_SOCKETS_m12
		#ifndef WIN32
			#define WIN32
		#endif
		#include <winsock2.h>
		// #pragma comment(lib, "ws2_32.lib")  // link with Ws2_32.lib (required, but repeated below for other libs)
	#endif
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
	#pragma comment(lib, "Ws2_32.lib")  // link with ws2_32.lib
	#pragma comment(lib, "Iphlpapi.lib") // link with Iphlpapi.lib
	#define _USE_MATH_DEFINES  // Needed for standard math constants. Must be defined before math.h included.
#endif
#if defined MACOS_m12 || defined LINUX_m12
	#include <unistd.h>
	#include <dirent.h>
	#include <sys/time.h>
	#include <sys/resource.h>
	#include <stdatomic.h>
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
#endif
#ifdef MACOS_m12
	#include <malloc/malloc.h>
	#include <sys/sysctl.h>
	#include <util.h>
//	#include <netinet/tcp.h>  // for tcp_connection_info
#endif
#ifdef LINUX_m12
	#include <sys/statfs.h>
	#include <sys/sysinfo.h>
	#include <pty.h>
	#include <utmp.h>
//	#include <linux/tcp.h>  // for tcp_info
#endif
#if defined LINUX_m12 || defined WINDOWS_m12
	#include <malloc.h>
#endif
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
#include <errno.h>
#ifdef MATLAB_m12
	#include "mex.h"
	#include "matrix.h"
#endif
#ifdef DATABASE_m12
	#include <libpq-fe.h>  //  postgres header
#endif

//**********************************************************************************//
//******************************  Elemental Typedefs  ******************************//
//**********************************************************************************//

#ifndef SIZE_TYPES_IN_m12
#define SIZE_TYPES_IN_m12

#include <stdint.h>

typedef uint8_t		ui1;
typedef char		si1;  // Note: the "char" type is not guaranteed to be a signed one-byte integer.  If it is not, library will exit during initialization
typedef uint16_t	ui2;
typedef int16_t		si2;
typedef uint32_t	ui4;
typedef int32_t		si4;
typedef uint64_t	ui8;
typedef int64_t		si8;
typedef float		sf4;
typedef double		sf8;
typedef long double	sf16;

#endif  // SIZE_TYPES_IN_m12

// MED Library Ternary Boolean Schema
typedef si1				TERN_m12;
#define TRUE_m12			1
#define UNKNOWN_m12			0
#define FALSE_m12			-1

// Reserved si4 Sample Values
#define NAN_SI4_m12			((si4) 0x80000000)
#define NEG_INF_SI4_m12           	((si4) 0x80000001)
#define POS_INF_SI4_m12			((si4) 0x7FFFFFFF)
#define MAX_SAMP_VAL_SI4_m12        	((si4) 0x7FFFFFFE)
#define MIN_SAMP_VAL_SI4_m12		((si4) 0x80000002)
// Reserved si2 Sample Values
#define NAN_SI2_m12			((si2) 0x8000)
#define NEG_INF_SI2_m12           	((si2) 0x8001)
#define POS_INF_SI2_m12			((si2) 0x7FFF)
#define MAX_SAMP_VAL_SI2_m12        	((si2) 0x7FFE)
#define MIN_SAMP_VAL_SI2_m12		((si2) 0x8002)



//**********************************************************************************//
//****************  Record Structures Integral to the MED Library  *****************//
//**************  (prototypes & constants declared in medrec_m12.h)  ***************//
//**********************************************************************************//


//*************************************************************************************//
//*******************************   Sgmt: Segment Record   ****************************//
//*************************************************************************************//

// A segment record is entered at the Session and or Channel Level for each new segment
// The encryption level for these records is typically set to the same as for metadata section 2

// Structures
typedef struct {
	si8     end_time;
	union {
		si8     start_sample_number;	// session-relative (global indexing)
		si8     start_frame_number;	// session-relative (global indexing)
	};
	union {
		si8     end_sample_number;	// session-relative (global indexing)
		si8     end_frame_number;	// session-relative (global indexing)
	};
	ui8     segment_UID;
	si4     segment_number;
	si4     acquisition_channel_number;  // REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_ALL_CHANNELS_m12 in session level records
	union {
		sf8     sampling_frequency;  // channel sampling frequency (REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m12 in session level records, if sampling frequencies vary across time series channels)
		sf8     frame_rate;  	     // channel frame rate (REC_Sgmt_v10_FRAME_RATE_VARIABLE_m12 in session level records, if frame rates vary across video channels)
	};
} REC_Sgmt_v10_m12;

// Description follows sampling_frequency / frame_rate in structure.
// The description is an aribitrary length array of si1s padded to 16 byte alignment (total of structure + string).

typedef struct {
	si8     	end_time;
	union {
		si8     start_sample_number;	// session-relative (global indexing) (SAMPLE_NUMBER_NO_ENTRY_m12 for variable frequency, session level entries)
		si8     start_frame_number;	// session-relative (global indexing) (FRAME_NUMBER_NO_ENTRY_m12 for variable frequency, session level entries)
	};
	union {
		si8     end_sample_number;	// session-relative (global indexing) (SAMPLE_NUMBER_NO_ENTRY_m12 for variable frequency, session level entries)
		si8     end_frame_number;	// session-relative (global indexing) (FRAME_NUMBER_NO_ENTRY_m12 for variable frequency, session level entries)
	};
	si4     segment_number;
	si4     acquisition_channel_number;  // REC_Sgmt_v11_ACQUISITION_CHANNEL_NUMBER_ALL_CHANNELS_m13 in session level records
} REC_Sgmt_v11_m12;

// Description follows acquisition_channel_number in structure.
// The description is an aribitrary length array of si1s padded to 16 byte alignment (total of structure + string).



//*************************************************************************************//
//*****************************   Stat: Statistics Record   ***************************//
//*************************************************************************************//

typedef struct {
	si4     minimum;
	si4     maximum;
	si4     mean;
	si4     median;
	si4     mode;
	sf4     variance;
	sf4     skewness;
	sf4     kurtosis;
} REC_Stat_v10_m12;



//*************************************************************************************//
//*********************   CMP Structures required in MED section  *********************//
//*************************************************************************************//

typedef struct {
	// CMP block header fixed region start
	ui8     block_start_UID;
	ui4     block_CRC;
	ui4     block_flags;
	si8     start_time;
	si4     acquisition_channel_number;
	ui4     total_block_bytes;
	// CMP block encryption start
	ui4     number_of_samples;
	ui2     number_of_records;
	ui2     record_region_bytes;
	ui4     parameter_flags;
	ui2     parameter_region_bytes;
	ui2     protected_region_bytes;
	ui2     discretionary_region_bytes;
	ui2     model_region_bytes;
	ui4     total_header_bytes;
	// CMP block header variable region start
} CMP_BLOCK_FIXED_HEADER_m12;

typedef struct {
	sf8	user_threshold;
	sf8	algorithm_threshold_LFP;
	sf8	algorithm_threshold_no_filt;
} CMP_VDS_THRESHOLD_MAP_ENTRY_m12;



//**********************************************************************************//
//**********************************  ENCRYPTION  **********************************//
//**********************************************************************************//

// Encryption & Password Constants
#define ENCRYPTION_LEVEL_NO_ENTRY_m12		-128
#define NO_ENCRYPTION_m12			0
#define LEVEL_0_ENCRYPTION_m12			NO_ENCRYPTION_m12
#define LEVEL_1_ENCRYPTION_m12			1
#define LEVEL_2_ENCRYPTION_m12			2
#define LEVEL_0_ACCESS_m12			LEVEL_0_ENCRYPTION_m12
#define LEVEL_1_ACCESS_m12			LEVEL_1_ENCRYPTION_m12
#define LEVEL_2_ACCESS_m12			LEVEL_2_ENCRYPTION_m12
#define LEVEL_1_ENCRYPTION_DECRYPTED_m12        -LEVEL_1_ENCRYPTION_m12
#define LEVEL_2_ENCRYPTION_DECRYPTED_m12        -LEVEL_2_ENCRYPTION_m12
#define ENCRYPTION_BLOCK_BYTES_m12		16      // AES-128
#define ENCRYPTION_KEY_BYTES_m12                176     // AES-128   = ((AES_NR + 1) * AES_NK * AES_NB)
#define PASSWORD_BYTES_m12			ENCRYPTION_BLOCK_BYTES_m12
#define MAX_PASSWORD_CHARACTERS_m12		PASSWORD_BYTES_m12
#define MAX_ASCII_PASSWORD_STRING_BYTES_m12	(MAX_PASSWORD_CHARACTERS_m12 + 1)  // 1 byte per character in ascii plus terminal zero
#define MAX_UTF8_PASSWORD_BYTES_m12		(MAX_PASSWORD_CHARACTERS_m12 * 4)  // up to 4 bytes per character in UTF-8
#define MAX_PASSWORD_STRING_BYTES_m12		(MAX_UTF8_PASSWORD_BYTES_m12 + 1)  // 1 byte for null-termination
#define PASSWORD_VALIDATION_FIELD_BYTES_m12     PASSWORD_BYTES_m12
#define PASSWORD_HINT_BYTES_m12                 256

// Password Data Structure
typedef struct {
	ui1	level_1_encryption_key[ENCRYPTION_KEY_BYTES_m12];
	ui1	level_2_encryption_key[ENCRYPTION_KEY_BYTES_m12];
	si1     level_1_password_hint[PASSWORD_HINT_BYTES_m12];
	si1     level_2_password_hint[PASSWORD_HINT_BYTES_m12];
	ui1	access_level;
	ui1	processed;  // 0 or 1 (boolean, not ternary)
} PASSWORD_DATA_m12;



//**********************************************************************************//
//********************************  MED Constants  *********************************//
//**********************************************************************************//

// Versioning Constants
#define MED_FORMAT_VERSION_MAJOR_m12		1  // restricted to single digits 1 through 9
#define MED_FORMAT_VERSION_MINOR_m12		0  // restricted to 0 through 254, minor version resets to zero with new major format version
#define MED_LIBRARY_VERSION_m12                 1  // restricted to 1 through 254, library version resets to one with new major format version
#define MED_VERSION_NO_ENTRY_m12                0xFF
#define MED_FULL_FORMAT_NAME_m12             	"\"" ## MED_VERSION_MAJOR_m12 ## "." ## MED_VERSION_MINOR_m12 ## "\""
#define MED_FULL_LIBRARY_NAME_m12             	"\"" ## MED_FULL_FORMAT_NAME_m12 ## "." ## MED_LIBRARY_VERSION_m12 ## "\""
#define MED_LIBRARY_TAG_m12			"\"_m" ## MED_VERSION_MAJOR_m12 ## MED_LIBRARY_VERSION_m12 ## "\""

// Miscellaneous Constants
#define BASE_FILE_NAME_BYTES_m12                256        // utf8[63]
#define SEGMENT_BASE_FILE_NAME_BYTES_m12        (BASE_FILE_NAME_BYTES_m12 + 8)
#define VIDEO_DATA_BASE_FILE_NAME_BYTES_m12     (SEGMENT_BASE_FILE_NAME_BYTES_m12 + 8)
#define FULL_FILE_NAME_BYTES_m12                1024        // utf8[255]
#define INDEX_BYTES_m12				24
#define BIG_ENDIAN_m12                          0
#define LITTLE_ENDIAN_m12                       1
#define TYPE_BYTES_m12                          5
#define TYPE_STRLEN_m12                         4
#define UID_BYTES_m12                           8
#define UID_NO_ENTRY_m12                        0
#define PAD_BYTE_VALUE_m12                      0x7e        // ascii tilde ("~") as si1
#define FILE_NUMBERING_DIGITS_m12               4
#define FREQUENCY_NO_ENTRY_m12                  -1.0
#define FRAME_RATE_NO_ENTRY_m12                 FREQUENCY_NO_ENTRY_m12
#define FREQUENCY_VARIABLE_m12			-2.0
#define FRAME_RATE_VARIABLE_m12			FREQUENCY_VARIABLE_m12
#define UNKNOWN_NUMBER_OF_ENTRIES_m12           -1
#define SEGMENT_NUMBER_NO_ENTRY_m12             -1
#define FIRST_OPEN_SEGMENT_m12			-2
#define CHANNEL_NUMBER_NO_ENTRY_m12             -1
#define CHANNEL_NUMBER_ALL_CHANNELS_m12         -2
#define DOES_NOT_EXIST_m12                      FALSE_m12  // -1
#define EXISTS_ERROR_m12                   	UNKNOWN_m12  // 0
#define FILE_EXISTS_m12				TRUE_m12  // 1
#define DIR_EXISTS_m12                          ((si1) 2)
#define SIZE_STRING_BYTES_m12                   32
#define UNKNOWN_SEARCH_m12                      0
#define TIME_SEARCH_m12                         1
#define SAMPLE_SEARCH_m12			2
#define FRAME_SEARCH_m12			SAMPLE_SEARCH_m12
#define NO_OVERFLOWS_m12			4  // e.g. in find_index_m12(), restrict returned index to valid segment values
#define IPV4_ADDRESS_BYTES_m12			4
#define POSTAL_CODE_BYTES_m12			16
#define LOCALITY_BYTES_m12			64  	//  ascii[63]
#define THREAD_NAME_BYTES_m12			64
#define SAMPLE_NUMBER_EPS_m12			((sf8) 0.001)
#define FRAME_NUMBER_EPS_m12			((sf8) 0.01)
#define UNMAPPED_CHANNEL_m12			((si4) -1)
#if defined MACOS_m12 || defined LINUX_m12
	#define NULL_DEVICE_m12					"/dev/null"
	#define DIR_BREAK_m12					'/'
	#define GLOBALS_FILE_CREATION_UMASK_DEFAULT_m12		S_IWOTH  // removes write permission for "other" (defined in <sys/stat.h>)
#endif
#ifdef WINDOWS_m12
	#define PRINTF_BUF_LEN_m12				1024
	#define NULL_DEVICE_m12					"NUL"
	#define DIR_BREAK_m12					'\\'
	#define GLOBALS_FILE_CREATION_UMASK_DEFAULT_m12		0  // full permissions for everyone (Windows does not support "other" category)
#endif

// Pipes
#define READ_END_m12		0
#define WRITE_END_m12		1
#define PIPE_FAILURE_m12	((si4) 255)
#define PIPE_FAILURE_SEND_m12	((si4) -1)  // sent from child, received as (si4) ((ui1) PIPE_FAILURE_m12)

// Error Handling Constants
#define USE_GLOBAL_BEHAVIOR_m12         ((ui4) 0)
#define RESTORE_BEHAVIOR_m12            ((ui4) 1)
#define EXIT_ON_FAIL_m12                ((ui4) 2)  // exit program
#define RETURN_ON_FAIL_m12              ((ui4) 4)  // return from function: if function is main(), behavior is up to programmer
// if neither EXIT_ON_FAIL_m12 nor RETURN_ON_FAIL_m12 are set, function will continue (i.e. could be CONTINUE_ON_FAIL_m12, if this were defined)
#define SUPPRESS_ERROR_OUTPUT_m12       ((ui4) 8)
#define SUPPRESS_WARNING_OUTPUT_m12     ((ui4) 16)
#define SUPPRESS_MESSAGE_OUTPUT_m12     ((ui4) 32)
#define SUPPRESS_OUTPUT_m12         	(SUPPRESS_ERROR_OUTPUT_m12 | SUPPRESS_WARNING_OUTPUT_m12 | SUPPRESS_MESSAGE_OUTPUT_m12)
#define RETRY_ONCE_m12                  ((ui4) 64)

// Target Value Constants (ui4)
#define NO_INDEX_m12			-1  // assigned to signed values (si4 or si8)
#define FIND_DEFAULT_MODE_m12        	0
#define FIND_START_m12          	(1 << 0)
#define FIND_END_m12            	(1 << 1)
#define FIND_CENTER_m12			(1 << 2)
#define FIND_PREVIOUS_m12        	(1 << 3)
#define FIND_CURRENT_m12        	(1 << 4)
#define FIND_NEXT_m12           	(1 << 5)
#define FIND_CLOSEST_m12        	(1 << 6)
#define FIND_LAST_BEFORE_m12		(1 << 7)
#define FIND_FIRST_ON_OR_AFTER_m12	(1 << 8)
#define FIND_LAST_ON_OR_BEFORE_m12	(1 << 9)
#define FIND_FIRST_AFTER_m12		(1 << 10)
#define FIND_ABSOLUTE_m12       	(1 << 30)  // session relative sample numbering
#define FIND_RELATIVE_m12       	(1 << 31)  // segment relative sample numbering

// Text Color Constant Strings
#ifdef MATLAB_m12  // Matlab doesn't do text coloring this way (can be done with CPRINTF())
	#define TC_BLACK_m12            ""
	#define TC_RED_m12              ""
	#define TC_GREEN_m12            ""
	#define TC_YELLOW_m12           ""
	#define TC_BLUE_m12             ""
	#define TC_MAGENTA_m12          ""
	#define TC_CYAN_m12             ""
	#define TC_LIGHT_GRAY_m12	""
	#define TC_DARK_GRAY_m12	""
	#define TC_LIGHT_RED_m12	""
	#define TC_LIGHT_GREEN_m12	""
	#define TC_LIGHT_YELLOW_m12	""
	#define TC_LIGHT_BLUE_m12	""
	#define TC_LIGHT_MAGENTA_m12	""
	#define TC_LIGHT_CYAN_m12	""
	#define TC_WHITE_m12		""
	#define TC_BRIGHT_BLACK_m12	""
	#define TC_BRIGHT_RED_m12	""
	#define TC_BRIGHT_GREEN_m12	""
	#define TC_BRIGHT_YELLOW_m12	""
	#define TC_BRIGHT_BLUE_m12	""
	#define TC_BRIGHT_MAGENTA_m12	""
	#define TC_BRIGHT_CYAN_m12	""
	#define TC_BRIGHT_WHITE_m12	""
	#define TC_RESET_m12		""
	// non-color constants
	#define TC_BOLD_m12		""
	#define TC_BOLD_RESET_m12	""
	#define TC_UNDERLINE_m12	""
	#define TC_UNDERLINE_RESET_m12	""
#else
	#define TC_BLACK_m12		"\033[30m"
	#define TC_RED_m12		"\033[31m"
	#define TC_GREEN_m12		"\033[32m"
	#define TC_YELLOW_m12		"\033[33m"
	#define TC_BLUE_m12		"\033[34m"
	#define TC_MAGENTA_m12		"\033[35m"
	#define TC_CYAN_m12		"\033[36m"
	#define TC_LIGHT_GRAY_m12	"\033[37m"
	#define TC_DARK_GRAY_m12	"\033[90m"
	#define TC_LIGHT_RED_m12	"\033[91m"
	#define TC_LIGHT_GREEN_m12	"\033[92m"
	#define TC_LIGHT_YELLOW_m12	"\033[93m"
	#define TC_LIGHT_BLUE_m12	"\033[94m"
	#define TC_LIGHT_MAGENTA_m12	"\033[95m"
	#define TC_LIGHT_CYAN_m12	"\033[96m"
	#define TC_WHITE_m12		"\033[97m"
	#define TC_BRIGHT_BLACK_m12	"\033[30;1m"
	#define TC_BRIGHT_RED_m12	"\033[31;1m"
	#define TC_BRIGHT_GREEN_m12	"\033[32;1m"
	#define TC_BRIGHT_YELLOW_m12	"\033[33;1m"
	#define TC_BRIGHT_BLUE_m12	"\033[34;1m"
	#define TC_BRIGHT_MAGENTA_m12	"\033[35;1m"
	#define TC_BRIGHT_CYAN_m12	"\033[36;1m"
	#define TC_BRIGHT_WHITE_m12	"\033[37;1m"
	#define TC_RESET_m12		"\033[0m"
	// non-color constants
	#define TC_BOLD_m12		"\033[1m"
	#define TC_BOLD_RESET_m12	"\033[21m"
	#define TC_UNDERLINE_m12	"\033[4m"
	#define TC_UNDERLINE_RESET_m12	"\033[24m"
#endif

// Time Related Constants
#define TIMEZONE_ACRONYM_BYTES_m12                      8       // ascii[7]
#define TIMEZONE_STRING_BYTES_m12                       64      // ascii[63]
#define MAXIMUM_STANDARD_UTC_OFFSET_m12                 ((si4) 86400)
#define MINIMUM_STANDARD_UTC_OFFSET_m12                 ((si4) -86400)
#define STANDARD_UTC_OFFSET_NO_ENTRY_m12                ((si4) 0x7FFFFFFF)
#define MAXIMUM_DST_OFFSET_m12                          7200
#define MINIMUM_DST_OFFSET_m12                          0
#define DST_OFFSET_NO_ENTRY_m12                         -1
#define TIME_STRING_BYTES_m12                           128
#define NUMBER_OF_SAMPLES_NO_ENTRY_m12			-1
#define NUMBER_OF_FRAMES_NO_ENTRY_m12			NUMBER_OF_SAMPLES_NO_ENTRY_m12
#define EMPTY_SLICE_m12					-1
#define SAMPLE_NUMBER_NO_ENTRY_m12                      ((si8) 0x8000000000000000)
#define FRAME_NUMBER_NO_ENTRY_m12                       SAMPLE_NUMBER_NO_ENTRY_m12
#define BEGINNING_OF_SAMPLE_NUMBERS_m12                 ((si8) 0x0000000000000000)
#define END_OF_SAMPLE_NUMBERS_m12                       ((si8) 0x7FFFFFFFFFFFFFFF)
#define UUTC_NO_ENTRY_m12                               ((si8) 0x8000000000000000)
#define UUTC_EARLIEST_TIME_m12                          ((si8) 0x0000000000000000)  // 00:00:00.000000 Thursday, 1 Jan 1970, UTC
#define UUTC_LATEST_TIME_m12                            ((si8) 0x7FFFFFFFFFFFFFFF)  // 04:00:54.775808 Sunday, 10 Jan 29424, UTC
#define BEGINNING_OF_TIME_m12                           UUTC_EARLIEST_TIME_m12
#define END_OF_TIME_m12                                 UUTC_LATEST_TIME_m12
#define CURRENT_TIME_m12				((si8) 0xFFFFFFFFFFFFFFFF)  // used with time_string_m12() & generate_recording_time_offset_m12()
#define TWENTY_FOURS_HOURS_m12				((si8) 86400000000)
#define Y2K_m12                                         ((si8) 0x00035D013B37E000)  // 00:00:00.000000 Saturday, 1 Jan 2000, UTC  (946684800000000 decimal)
#define WIN_TICKS_PER_USEC_m12				((si8) 10)
#define WIN_USECS_TO_EPOCH_m12				((si8) 11644473600000000)

// Time Change Code Constants
#define DTCC_VALUE_NOT_OBSERVED_m12                     0
#define DTCC_VALUE_NO_ENTRY_m12                         -1
#define DTCC_VALUE_DEFAULT_m12                          DTCC_VALUE_NO_ENTRY_m12
#define DTCC_DST_END_CODE                               -1
#define DTCC_DST_START_CODE                             1
#define DTCC_DST_NOT_OBSERVED_CODE                      0
#define DTCC_DAY_OF_WEEK_NO_ENTRY                       0
#define DTCC_FIRST_RELATIVE_WEEKDAY_OF_MONTH            1
#define DTCC_LAST_RELATIVE_WEEKDAY_OF_MONTH             6
#define DTCC_RELATIVE_WEEKDAY_OF_MONTH_NO_ENTRY         0
#define DTCC_DAY_OF_MONTH_NO_ENTRY                      0
#define DTCC_MONTH_NO_ENTRY                             -1
#define DTCC_HOURS_OF_DAY_NO_ENTRY                      -128
#define DTCC_LOCAL_REFERENCE_TIME                       0
#define DTCC_UTC_REFERENCE_TIME                         1
#define DTCC_REFERENCE_TIME_NO_ENTRY                    -1
#define DTCC_SHIFT_MINUTES_TIME_NO_ENTRY                -128
#define DTCC_START_DATE_NO_ENTRY			-1  // NO_ENTRY indicates it is the only historical rule for this timezone in the table

// Global Defaults
#define GLOBALS_VERBOSE_DEFAULT_m12                             FALSE_m12
#define GLOBALS_BEHAVIOR_ON_FAIL_DEFAULT_m12		        EXIT_ON_FAIL_m12
#define GLOBALS_CRC_MODE_DEFAULT_m12			        CRC_CALCULATE_ON_OUTPUT_m12
#define GLOBALS_BEHAVIOR_STACK_SIZE_INCREMENT_m12		256
#define GLOBALS_REFERENCE_CHANNEL_INDEX_NO_ENTRY_m12		-1
#define GLOBALS_MMAP_BLOCK_BYTES_NO_ENTRY_m12			((ui4) 0)
#define GLOBALS_MMAP_BLOCK_BYTES_DEFAULT_m12			4096  // 4 KiB
#define GLOBALS_AT_LIST_SIZE_INCREMENT_m12			8096

// Global Time Defaults
#define GLOBALS_OBSERVE_DST_DEFAULT_m12				FALSE_m12
#define GLOBALS_RTO_KNOWN_DEFAULT_m12				UNKNOWN_m12
#define GLOBALS_SESSION_START_TIME_DEFAULT_m12			UUTC_NO_ENTRY_m12
#define GLOBALS_SESSION_END_TIME_DEFAULT_m12			UUTC_NO_ENTRY_m12
#define GLOBALS_RECORDING_TIME_OFFSET_DEFAULT_m12               0
#define GLOBALS_RECORDING_TIME_OFFSET_NO_ENTRY_m12              0
#define GLOBALS_STANDARD_UTC_OFFSET_DEFAULT_m12                 0
#define GLOBALS_STANDARD_UTC_OFFSET_NO_ENTRY_m12		STANDARD_UTC_OFFSET_NO_ENTRY_m12
#define GLOBALS_STANDARD_TIMEZONE_ACRONYM_DEFAULT_m12	        "oUTC"
#define GLOBALS_STANDARD_TIMEZONE_STRING_DEFAULT_m12            "offset Coordinated Universal Time"
#define GLOBALS_DAYLIGHT_TIMEZONE_ACRONYM_DEFAULT_m12           ""
#define GLOBALS_DAYLIGHT_TIMEZONE_STRING_DEFAULT_m12            ""

// File Type Constants
#define NO_TYPE_CODE_m12                                        (ui4) 0x0
#define UNKNOWN_TYPE_CODE_m12                                   NO_TYPE_CODE_m12
#define NO_FILE_TYPE_STRING_m12				        ""			// ascii[4]
#define NO_FILE_TYPE_CODE_m12				        NO_TYPE_CODE_m12	// ui4 (big & little endian)
#define ALL_TYPES_STRING_m12		        		"allt"			// ascii[4]
#define ALL_TYPES_CODE_m12                                      (ui4) 0x746C6C61	// ui4 (little endian)
// #define ALL_TYPES_CODE_m12					(ui4) 0x616C6C74	// ui4 (big endian)
#define SESSION_DIRECTORY_TYPE_STRING_m12		        "medd"			// ascii[4]
#define SESSION_DIRECTORY_TYPE_CODE_m12                         (ui4) 0x6464656D	// ui4 (little endian)
// #define SESSION_DIRECTORY_TYPE_CODE_m12                      (ui4) 0x6D656464        // ui4 (big endian)
#define TIME_SERIES_CHANNEL_DIRECTORY_TYPE_STRING_m12           "ticd"                  // ascii[4]
#define TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m12             (ui4) 0x64636974        // ui4 (little endian)
// #define TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m12          (ui4) 0x74696364        // ui4 (big endian)
#define TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m12           "tisd"                  // ascii[4]
#define TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m12             (ui4) 0x64736974        // ui4 (little endian)
// #define TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m12          (ui4) 0x74697364        // ui4 (big endian)
#define TIME_SERIES_METADATA_FILE_TYPE_STRING_m12               "tmet"                  // ascii[4]
#define TIME_SERIES_METADATA_FILE_TYPE_CODE_m12                 (ui4) 0x74656D74        // ui4 (little endian)
// #define TIME_SERIES_METADATA_FILE_TYPE_CODE_m12              (ui4) 0x746D6574        // ui4 (big endian)
#define TIME_SERIES_DATA_FILE_TYPE_STRING_m12                   "tdat"                  // ascii[4]
#define TIME_SERIES_DATA_FILE_TYPE_CODE_m12                     (ui4) 0x74616474        // ui4 (little endian)
// #define TIME_SERIES_DATA_FILE_TYPE_CODE_m12                  (ui4) 0x74646174        // ui4 (big endian)
#define TIME_SERIES_INDICES_FILE_TYPE_STRING_m12                "tidx"                  // ascii[4]
#define TIME_SERIES_INDICES_FILE_TYPE_CODE_m12                  (ui4) 0x78646974        // ui4 (little endian)
// #define TIME_SERIES_INDICES_FILE_TYPE_CODE_m12               (ui4) 0x74696478        // ui4 (big endian)
#define VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m12                 "visd"                  // ascii[4]
#define VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m12                   (ui4) 0x64736976        // ui4 (little endian)
// #define VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m12                (ui4) 0x76697364        // ui4 (big endian)
#define VIDEO_CHANNEL_DIRECTORY_TYPE_STRING_m12                 "vicd"                  // ascii[4]
#define VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m12                   (ui4) 0x64636976        // ui4 (little endian)
// #define VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m12                (ui4) 0x76696364        // ui4 (big endian)
#define VIDEO_METADATA_FILE_TYPE_STRING_m12                     "vmet"                  // ascii[4]
#define VIDEO_METADATA_FILE_TYPE_CODE_m12                       (ui4) 0x74656D76        // ui4 (little endian)
// #define VIDEO_METADATA_FILE_TYPE_CODE_m12                    (ui4) 0x766D6574        // ui4 (big endian)
#define VIDEO_DATA_FILE_TYPE_STRING_m12                   	"vdat"                  // ascii[4]			// NOT a file type extension
#define VIDEO_DATA_FILE_TYPE_CODE_m12                     	(ui4) 0x74616476        // ui4 (little endian)		// NOT a file type extension
// #define VIDEO_DATA_FILE_TYPE_CODE_m12                  	(ui4) 0x76646174        // ui4 (big endian)		// NOT a file type extension
#define VIDEO_INDICES_FILE_TYPE_STRING_m12                      "vidx"                  // ascii[4]
#define VIDEO_INDICES_FILE_TYPE_CODE_m12                        (ui4) 0x78646976        // ui4 (little endian)
// #define VIDEO_INDICES_FILE_TYPE_CODE_m12                     (ui4) 0x76696478        // ui4 (big endian)
#define RECORD_DIRECTORY_TYPE_STRING_m12                        "recd"                        // ascii[4]
#define RECORD_DIRECTORY_TYPE_CODE_m12                          (ui4) 0x64636572        // ui4 (little endian)
// #define RECORD_DIRECTORY_TYPE_CODE_m12                       (ui4) 0x72656364        // ui4 (big endian)
#define RECORD_DATA_FILE_TYPE_STRING_m12                        "rdat"			// ascii[4]
#define RECORD_DATA_FILE_TYPE_CODE_m12                          (ui4) 0x74616472	// ui4 (little endian)
// #define RECORD_DATA_FILE_TYPE_CODE_m12                       (ui4) 0x72646174	// ui4 (big endian)
#define RECORD_INDICES_FILE_TYPE_STRING_m12                     "ridx"			// ascii[4]
#define RECORD_INDICES_FILE_TYPE_CODE_m12                       (ui4) 0x78646972	// ui4 (little endian)
// #define RECORD_INDICES_FILE_TYPE_CODE_m12                    (ui4) 0x72696478	// ui4 (big endian)

// Channel Types
#define UNKNOWN_CHANNEL_TYPE_m12	NO_FILE_TYPE_CODE_m12
#define TIME_SERIES_CHANNEL_TYPE_m12	TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m12
#define VIDEO_CHANNEL_TYPE_m12		VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m12

// Reference Channel Types (used in G_change_reference_channel_m12())
#define DEFAULT_CHANNEL_m12			0
#define DEFAULT_TIME_SERIES_CHANNEL_m12		1
#define DEFAULT_VIDEO_CHANNEL_m12		2
#define HIGHEST_RATE_TIME_SERIES_CHANNEL_m12	3
#define LOWEST_RATE_TIME_SERIES_CHANNEL_m12	4
#define HIGHEST_RATE_VIDEO_CHANNEL_m12		5
#define LOWEST_RATE_VIDEO_CHANNEL_m12		6

// Generate File List flags
	// Path Parts
#define GFL_PATH_m12             		((ui4) 1)
#define GFL_NAME_m12             		((ui4) 2)
#define GFL_EXTENSION_m12        		((ui4) 4)
#define GFL_FULL_PATH_m12        		(GFL_PATH_m12 | GFL_NAME_m12 | GFL_EXTENSION_m12)
#define GFL_PATH_PARTS_MASK_m12        		GFL_FULL_PATH_m12
	// Other Options
#define GFL_FREE_INPUT_LIST_m12			((ui4) 16)
#define GFL_INCLUDE_PARITY_m12			((ui4) 32)  // files or directrories
#define GFL_INCLUDE_INVISIBLE_m12		((ui4) 64)  // files or directrories

// System Pipe flags
#define SP_DEFAULT_m12			0  // no flags set (default)
#define SP_TEE_TO_TERMINAL_m12		1  // print buffer(s) to terminal in addition to returning
#define SP_SEPARATE_STREAMS_m12		2  // return seprate "stdout" & "stderr" buffers (buffer = stdout, e_buffer = stderr), otherwise ganged

// Spaces Constants
#define NO_SPACES_m12                           0
#define ESCAPED_SPACES_m12                      1
#define UNESCAPED_SPACES_m12                    2
#define ALL_SPACES_m12                          (ESCAPED_SPACES_m12 | UNESCAPED_SPACES_m12)

// File Processing Constants
#define FPS_FILE_LENGTH_UNKNOWN_m12		-1
#define FPS_UNIVERSAL_HEADER_ONLY_m12		-1
#define FPS_FULL_FILE_m12			-2
#define FPS_APPEND_m12				-3
#define FPS_CLOSE_m12				-4
#define FPS_NO_LOCK_TYPE_m12			~(F_RDLCK | F_WRLCK | F_UNLCK)  // from <fcntl.h>
#define FPS_NO_LOCK_MODE_m12			0
#define FPS_READ_LOCK_ON_READ_OPEN_m12		1
#define FPS_WRITE_LOCK_ON_READ_OPEN_m12		2
#define FPS_WRITE_LOCK_ON_WRITE_OPEN_m12        4
#define FPS_WRITE_LOCK_ON_READ_WRITE_OPEN_m12   8
#define FPS_READ_LOCK_ON_READ_m12               16
#define FPS_WRITE_LOCK_ON_WRITE_m12             32
#define FPS_NO_OPEN_MODE_m12		        0
#define FPS_R_OPEN_MODE_m12			1
#define FPS_R_PLUS_OPEN_MODE_m12                2
#define FPS_W_OPEN_MODE_m12                     4
#define FPS_W_PLUS_OPEN_MODE_m12                8
#define FPS_A_OPEN_MODE_m12                     16
#define FPS_A_PLUS_OPEN_MODE_m12                32
#define FPS_GENERIC_READ_OPEN_MODE_m12		(FPS_R_OPEN_MODE_m12 | FPS_R_PLUS_OPEN_MODE_m12 | FPS_W_PLUS_OPEN_MODE_m12 | FPS_A_PLUS_OPEN_MODE_m12)
#define FPS_GENERIC_WRITE_OPEN_MODE_m12		(FPS_R_PLUS_OPEN_MODE_m12 | FPS_W_OPEN_MODE_m12 | FPS_W_PLUS_OPEN_MODE_m12 | FPS_A_OPEN_MODE_m12 | FPS_A_PLUS_OPEN_MODE_m12)
#define FPS_PROTOTYPE_FILE_TYPE_CODE_m12        TIME_SERIES_METADATA_FILE_TYPE_CODE_m12  // any metadata type would do
#define FPS_FD_CLOSED_m12                     	-1
#define FPS_FD_NO_ENTRY_m12                     -2
#define FPS_FD_EPHEMERAL_m12                    -3

// File Processing Directives Defaults
#define FPS_DIRECTIVES_CLOSE_FILE_DEFAULT_m12		        TRUE_m12
#define FPS_DIRECTIVES_MEMORY_MAP_DEFAULT_m12		        FALSE_m12
#define FPS_DIRECTIVES_FLUSH_AFTER_WRITE_DEFAULT_m12		TRUE_m12
#define FPS_DIRECTIVES_FREE_CMP_PROCESSING_STRUCT_DEFAULT_m12	TRUE_m12
#define FPS_DIRECTIVES_FREE_PASSWORD_DATA_DEFAULT_m12		FALSE_m12
#define FPS_DIRECTIVES_UPDATE_UNIVERSAL_HEADER_DEFAULT_m12	FALSE_m12
#define FPS_DIRECTIVES_LEAVE_DECRYPTED_DEFAULT_m12		FALSE_m12
#define FPS_DIRECTIVES_LOCK_MODE_DEFAULT_m12		        FPS_NO_LOCK_MODE_m12  // Unix file locking may cause problems with networked file systems
// #define FPS_DIRECTIVES_LOCK_MODE_DEFAULT_m12                     (FPS_READ_LOCK_ON_READ_OPEN_m12 | FPS_WRITE_LOCK_ON_WRITE_OPEN_m12 | FPS_WRITE_LOCK_ON_READ_WRITE_OPEN_m12)
#define FPS_DIRECTIVES_OPEN_MODE_DEFAULT_m12		        FPS_NO_OPEN_MODE_m12

// Universal Header: File Format Constants
#define UNIVERSAL_HEADER_OFFSET_m12					0
#define UNIVERSAL_HEADER_BYTES_m12					1024    // 1 KiB
#define UNIVERSAL_HEADER_HEADER_CRC_OFFSET_m12				0       // ui4
#define UNIVERSAL_HEADER_BODY_CRC_OFFSET_m12				4       // ui4
#define UNIVERSAL_HEADER_HEADER_CRC_START_OFFSET_m12			UNIVERSAL_HEADER_BODY_CRC_OFFSET_m12
#define UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m12			UNIVERSAL_HEADER_BYTES_m12
#define UNIVERSAL_HEADER_FILE_END_TIME_OFFSET_m12			8	// si8
#define UNIVERSAL_HEADER_FILE_END_TIME_NO_ENTRY_m12			UUTC_NO_ENTRY_m12
#define UNIVERSAL_HEADER_SEGMENT_END_TIME_ENTRY_m12			UNIVERSAL_HEADER_FILE_END_NO_TIME_ENTRY_m12
#define UNIVERSAL_HEADER_NUMBER_OF_ENTRIES_OFFSET_m12			16      // si8
#define UNIVERSAL_HEADER_NUMBER_OF_ENTRIES_NO_ENTRY_m12			-1
#define UNIVERSAL_HEADER_MAXIMUM_ENTRY_SIZE_OFFSET_m12			24      // ui4
#define UNIVERSAL_HEADER_MAXIMUM_ENTRY_SIZE_NO_ENTRY_m12		0
#define UNIVERSAL_HEADER_SEGMENT_NUMBER_OFFSET_m12                      28      // si4
#define UNIVERSAL_HEADER_SEGMENT_NUMBER_NO_ENTRY_m12                    SEGMENT_NUMBER_NO_ENTRY_m12
#define UNIVERSAL_HEADER_SEGMENT_LEVEL_CODE_m12				-1
#define UNIVERSAL_HEADER_CHANNEL_LEVEL_CODE_m12				-2
#define UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m12				-3
#define UNIVERSAL_HEADER_TYPE_STRING_OFFSET_m12				32       // ascii[4]
#define UNIVERSAL_HEADER_TYPE_STRING_TERMINAL_ZERO_OFFSET_m12		(UNIVERSAL_HEADER_TYPE_STRING_OFFSET_m12 + 4)  // si1
#define UNIVERSAL_HEADER_TYPE_CODE_OFFSET_m12				UNIVERSAL_HEADER_TYPE_STRING_OFFSET_m12  // ui4
#define UNIVERSAL_HEADER_TYPE_NO_ENTRY_m12				0       // zero as ui4 or zero-length string as ascii[4]
#define UNIVERSAL_HEADER_MED_VERSION_MAJOR_OFFSET_m12			37     // ui1
#define UNIVERSAL_HEADER_MED_VERSION_MAJOR_NO_ENTRY_m12			MED_VERSION_NO_ENTRY_m12
#define UNIVERSAL_HEADER_MED_VERSION_MINOR_OFFSET_m12			38      // ui1
#define UNIVERSAL_HEADER_MED_VERSION_MINOR_NO_ENTRY_m12			MED_VERSION_NO_ENTRY_m12
#define UNIVERSAL_HEADER_BYTE_ORDER_CODE_OFFSET_m12			39      // ui1
#define UNIVERSAL_HEADER_BYTE_ORDER_CODE_NO_ENTRY_m12			0xFF
#define UNIVERSAL_HEADER_SESSION_START_TIME_OFFSET_m12			40      // si8
#define UNIVERSAL_HEADER_SESSION_START_TIME_NO_ENTRY_m12		UUTC_NO_ENTRY_m12
#define UNIVERSAL_HEADER_FILE_START_TIME_OFFSET_m12			48      // si8
#define UNIVERSAL_HEADER_FILE_START_TIME_NO_ENTRY_m12			UUTC_NO_ENTRY_m12
#define UNIVERSAL_HEADER_SESSION_NAME_OFFSET_m12                        56      // utf8[63]
#define UNIVERSAL_HEADER_CHANNEL_NAME_OFFSET_m12                        312     // utf8[63]
#define UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_OFFSET_m12             	568     // utf8[63]
#define UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_BYTES_m12              	BASE_FILE_NAME_BYTES_m12
#define UNIVERSAL_HEADER_SESSION_UID_OFFSET_m12                         824     // ui8
#define UNIVERSAL_HEADER_CHANNEL_UID_OFFSET_m12                         832     // ui8
#define UNIVERSAL_HEADER_SEGMENT_UID_OFFSET_m12                         840     // ui8
#define UNIVERSAL_HEADER_FILE_UID_OFFSET_m12				848     // ui8
#define UNIVERSAL_HEADER_PROVENANCE_UID_OFFSET_m12			856     // ui8
#define UNIVERSAL_HEADER_LEVEL_1_PASSWORD_VALIDATION_FIELD_OFFSET_m12	864     // ui1
#define UNIVERSAL_HEADER_LEVEL_2_PASSWORD_VALIDATION_FIELD_OFFSET_m12	880     // ui1
#define UNIVERSAL_HEADER_LEVEL_3_PASSWORD_VALIDATION_FIELD_OFFSET_m12   896     // ui1
#define UNIVERSAL_HEADER_PROTECTED_REGION_OFFSET_m12			912
#define UNIVERSAL_HEADER_PROTECTED_REGION_BYTES_m12			56
#define UNIVERSAL_HEADER_DISCRETIONARY_REGION_OFFSET_m12		968
#define UNIVERSAL_HEADER_DISCRETIONARY_REGION_BYTES_m12			56

// Metadata: File Format Constants
#define METADATA_BYTES_m12			15360   // 15 kB
#define FPS_PROTOTYPE_BYTES_m12			METADATA_BYTES_m12
#define METADATA_FILE_BYTES_m12			(METADATA_BYTES_m12 + UNIVERSAL_HEADER_BYTES_m12)	// 16 kB
#define METADATA_SECTION_1_OFFSET_m12		1024
#define METADATA_SECTION_1_BYTES_m12		1024	// 1 kB
#define METADATA_SECTION_2_OFFSET_m12		2048
#define METADATA_SECTION_2_BYTES_m12		10240   // 10 kB
#define METADATA_SECTION_3_OFFSET_m12		12288
#define METADATA_SECTION_3_BYTES_m12		4096    // 4 kB

// Metadata: File Format Constants - Section 1 Fields
#define METADATA_LEVEL_1_PASSWORD_HINT_OFFSET_m12		1024	// utf8[63]
#define METADATA_LEVEL_2_PASSWORD_HINT_OFFSET_m12		1280    // utf8[63]
#define METADATA_SECTION_2_ENCRYPTION_LEVEL_OFFSET_m12		1536    // si1
#define METADATA_SECTION_2_ENCRYPTION_LEVEL_DEFAULT_m12		LEVEL_1_ENCRYPTION_m12
#define METADATA_SECTION_3_ENCRYPTION_LEVEL_OFFSET_m12		1537    // si1
#define METADATA_SECTION_3_ENCRYPTION_LEVEL_DEFAULT_m12		LEVEL_2_ENCRYPTION_m12
#define METADATA_TIME_SERIES_DATA_ENCRYPTION_LEVEL_OFFSET_m12	1538
#define METADATA_TIME_SERIES_DATA_ENCRYPTION_LEVEL_DEFAULT_m12	NO_ENCRYPTION_m12
#define METADATA_SECTION_1_PROTECTED_REGION_OFFSET_m12		1539
#define METADATA_SECTION_1_PROTECTED_REGION_BYTES_m12		253
#define METADATA_SECTION_1_DISCRETIONARY_REGION_OFFSET_m12	1792
#define METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m12	256

// Metadata: File Format Constants - Section 2 Channel Type Independent Fields
#define METADATA_SESSION_DESCRIPTION_OFFSET_m12                 2048    // utf8[511]
#define METADATA_SESSION_DESCRIPTION_BYTES_m12                  2048
#define METADATA_CHANNEL_DESCRIPTION_OFFSET_m12                 4096    // utf8[255]
#define METADATA_CHANNEL_DESCRIPTION_BYTES_m12                  1024
#define METADATA_SEGMENT_DESCRIPTION_OFFSET_m12                 5120    // utf8[255]
#define METADATA_SEGMENT_DESCRIPTION_BYTES_m12                  1024
#define METADATA_EQUIPMENT_DESCRIPTION_OFFSET_m12               6144    // utf8[510]
#define METADATA_EQUIPMENT_DESCRIPTION_BYTES_m12                2044
#define METADATA_ACQUISITION_CHANNEL_NUMBER_OFFSET_m12          8188    // si4
#define METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m12        CHANNEL_NUMBER_NO_ENTRY_m12

// Metadata: File Format Constants - Time Series Section 2 Fields
#define TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_OFFSET_m12                   8192            // utf8[255]
#define TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m12                    1024
#define TIME_SERIES_METADATA_SAMPLING_FREQUENCY_OFFSET_m12                      9216            // sf8
#define TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m12                             FREQUENCY_NO_ENTRY_m12
#define TIME_SERIES_METADATA_FREQUENCY_VARIABLE_m12				FREQUENCY_VARIABLE_m12
#define TIME_SERIES_METADATA_LOW_FREQUENCY_FILTER_SETTING_OFFSET_m12            9224            // sf8
#define TIME_SERIES_METADATA_HIGH_FREQUENCY_FILTER_SETTING_OFFSET_m12           9232            // sf8
#define TIME_SERIES_METADATA_NOTCH_FILTER_FREQUENCY_SETTING_OFFSET_m12          9240            // sf8
#define TIME_SERIES_METADATA_AC_LINE_FREQUENCY_OFFSET_m12                       9248            // sf8
#define TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_OFFSET_m12       9256            // sf8
#define TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m12     0.0
#define TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_OFFSET_m12             9264            // utf8[31]
#define TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m12              128
#define TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m12       9392            // sf8
#define TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m12     0.0
#define TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_OFFSET_m12             9400            // utf8[31]
#define TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m12              128
#define TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_OFFSET_m12            9528
#define TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_NO_ENTRY_m12          SAMPLE_NUMBER_NO_ENTRY_m12
#define TIME_SERIES_METADATA_NUMBER_OF_SAMPLES_OFFSET_m12                       9536            // si8
#define TIME_SERIES_METADATA_NUMBER_OF_SAMPLES_NO_ENTRY_m12                     NUMBER_OF_SAMPLES_NO_ENTRY_m12
#define TIME_SERIES_METADATA_NUMBER_OF_BLOCKS_OFFSET_m12                        9544            // si8
#define TIME_SERIES_METADATA_NUMBER_OF_BLOCKS_NO_ENTRY_m12                      -1
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_BYTES_OFFSET_m12                     9552            // si8
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_BYTES_NO_ENTRY_m12                   -1
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_SAMPLES_OFFSET_m12                   9560            // ui4
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_SAMPLES_NO_ENTRY_m12                 0xFFFFFFFF
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_KEYSAMPLE_BYTES_OFFSET_m12          	9564            // ui4
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_KEYSAMPLE_BYTES_NO_ENTRY_m12        	0xFFFFFFFF
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_DURATION_OFFSET_m12                  9568            // sf8
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_DURATION_NO_ENTRY_m12                -1.0
#define TIME_SERIES_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m12               9576            // si8
#define TIME_SERIES_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m12             -1
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_OFFSET_m12               9584            // si8
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_NO_ENTRY_m12             -1
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_OFFSET_m12          9592            // si8
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_NO_ENTRY_m12        -1
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_OFFSET_m12              9600            // si8
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_NO_ENTRY_m12            -1
#define TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m12              9608
#define TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m12               1344
#define TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m12          10952
#define TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m12           1336

// Metadata: File Format Constants - Video Section 2 Fields
#define VIDEO_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m12		8192		// sf8
#define VIDEO_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m12		0.0
#define VIDEO_METADATA_TIME_BASE_UNITS_DESCRIPTION_OFFSET_m12			8200		// utf8[31]
#define VIDEO_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m12			128
#define VIDEO_METADATA_ABSOLUTE_START_FRAME_NUMBER_OFFSET_m12			8328
#define VIDEO_METADATA_ABSOLUTE_START_FRAME_NUMBER_NO_ENTRY_m12			FRAME_NUMBER_NO_ENTRY_m12
#define VIDEO_METADATA_NUMBER_OF_FRAMES_OFFSET_m12				8336		// si8
#define VIDEO_METADATA_NUMBER_OF_FRAMES_NO_ENTRY_m12				NUMBER_OF_FRAMES_NO_ENTRY_m12
#define VIDEO_METADATA_FRAME_RATE_OFFSET_m12					8344		// sf8
#define VIDEO_METADATA_FRAME_RATE_NO_ENTRY_m12					FRAME_RATE_NO_ENTRY_m12
#define VIDEO_METADATA_FRAME_RATE_VARIABLE_m12					FRAME_RATE_VARIABLE_m12
#define VIDEO_METADATA_NUMBER_OF_CLIPS_OFFSET_m12                        	8352            // si8
#define VIDEO_METADATA_NUMBER_OF_CLIPS_NO_ENTRY_m12                      	-1
#define VIDEO_METADATA_MAXIMUM_CLIP_BYTES_OFFSET_m12				8360            // si8
#define VIDEO_METADATA_MAXIMUM_CLIP_BYTES_NO_ENTRY_m12				-1
#define VIDEO_METADATA_MAXIMUM_CLIP_FRAMES_OFFSET_m12				8368            // ui4
#define VIDEO_METADATA_MAXIMUM_CLIP_FRAMES_NO_ENTRY_m12				0xFFFFFFFF
#define VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_OFFSET_m12                 	8372    	// si4
#define VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_NO_ENTRY_m12               	-1
#define VIDEO_METADATA_MAXIMUM_CLIP_DURATION_OFFSET_m12                  	8376            // sf8
#define VIDEO_METADATA_MAXIMUM_CLIP_DURATION_NO_ENTRY_m12                	-1.0
#define VIDEO_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m12               	8384            // si8
#define VIDEO_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m12             	-1
#define VIDEO_METADATA_MAXIMUM_CONTIGUOUS_CLIPS_OFFSET_m12			8392            // si8
#define VIDEO_METADATA_MAXIMUM_CONTIGUOUS_CLIPS_NO_ENTRY_m12			-1
#define VIDEO_METADATA_MAXIMUM_CONTIGUOUS_CLIP_BYTES_OFFSET_m12          	8400            // si8
#define VIDEO_METADATA_MAXIMUM_CONTIGUOUS_CLIP_BYTES_NO_ENTRY_m12		-1
#define VIDEO_METADATA_MAXIMUM_CONTIGUOUS_FRAMES_OFFSET_m12			8408            // si8
#define VIDEO_METADATA_MAXIMUM_CONTIGUOUS_FRAMES_NO_ENTRY_m12			-1
#define VIDEO_METADATA_HORIZONTAL_PIXELS_OFFSET_m12				8416		// ui4
#define VIDEO_METADATA_HORIZONTAL_PIXELS_NO_ENTRY_m12				0
#define VIDEO_METADATA_VERTICAL_PIXELS_OFFSET_m12				8420		// ui4
#define VIDEO_METADATA_VERTICAL_PIXELS_NO_ENTRY_m12				0
#define VIDEO_METADATA_VIDEO_FORMAT_OFFSET_m12                          	8424		// utf8[63]
#define VIDEO_METADATA_VIDEO_FORMAT_BYTES_m12                           	256
#define VIDEO_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m12			8680
#define VIDEO_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m12			1808
#define VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m12		10488
#define VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m12			1800

// Metadata: File Format Constants - Section 3 Fields
#define METADATA_RECORDING_TIME_OFFSET_OFFSET_m12               12288           // si8
#define METADATA_RECORDING_TIME_OFFSET_NO_ENTRY_m12             GLOBALS_RECORDING_TIME_OFFSET_NO_ENTRY_m12
#define METADATA_DAYLIGHT_TIME_START_CODE_OFFSET_m12            12296           // DAYLIGHT_TIME_CHANGE_CODE_m12 (si1[8])
#define METADATA_DAYLIGHT_TIME_START_CODE_NO_ENTRY_m12          DTCC_VALUE_NO_ENTRY_m12
#define METADATA_DAYLIGHT_TIME_END_CODE_OFFSET_m12              12304           // DAYLIGHT_TIME_CHANGE_CODE_m12 (si1[8])
#define METADATA_DAYLIGHT_TIME_END_CODE_NO_ENTRY_m12            DTCC_VALUE_NO_ENTRY_m12
#define METADATA_STANDARD_TIMEZONE_ACRONYM_OFFSET_m12           12312           // ascii[7]
#define METADATA_STANDARD_TIMEZONE_ACRONYM_BYTES_m12            TIMEZONE_ACRONYM_BYTES_m12
#define METADATA_STANDARD_TIMEZONE_STRING_OFFSET_m12            12320           // ascii[63]
#define METADATA_STANDARD_TIMEZONE_STRING_BYTES_m12             TIMEZONE_STRING_BYTES_m12
#define METADATA_DAYLIGHT_TIMEZONE_ACRONYM_OFFSET_m12           12384           // ascii[7]
#define METADATA_DAYLIGHT_TIMEZONE_ACRONYM_BYTES_m12            TIMEZONE_ACRONYM_BYTES_m12
#define METADATA_DAYLIGHT_TIMEZONE_STRING_OFFSET_m12            12392           // ascii[63]
#define METADATA_DAYLIGHT_TIMEZONE_STRING_BYTES_m12             TIMEZONE_STRING_BYTES_m12
#define METADATA_SUBJECT_NAME_1_OFFSET_m12                      12456           // utf8[31]
#define METADATA_SUBJECT_NAME_BYTES_m12                         128
#define METADATA_SUBJECT_NAME_2_OFFSET_m12                      12584           // utf8[31]
#define METADATA_SUBJECT_NAME_3_OFFSET_m12                      12712           // utf8[31]
#define METADATA_SUBJECT_ID_OFFSET_m12                          12840           // utf8[31]
#define METADATA_SUBJECT_ID_BYTES_m12                           128
#define METADATA_RECORDING_COUNTRY_OFFSET_m12                   12968           // utf8[63]
#define METADATA_RECORDING_TERRITORY_OFFSET_m12                 13224           // utf8[63]
#define METADATA_RECORDING_LOCALITY_OFFSET_m12                  13480           // utf8[63]
#define METADATA_RECORDING_INSTITUTION_OFFSET_m12               13736           // utf8[63]
#define METADATA_RECORDING_LOCATION_BYTES_m12                   256
#define METADATA_GEOTAG_FORMAT_OFFSET_m12                       13992           // ascii[31]
#define METADATA_GEOTAG_FORMAT_BYTES_m12                        32
#define METADATA_GEOTAG_DATA_OFFSET_m12                         14024           // ascii[1023]
#define METADATA_GEOTAG_DATA_BYTES_m12                          1024
#define METADATA_STANDARD_UTC_OFFSET_OFFSET_m12                 15048           // si4
#define METADATA_STANDARD_UTC_OFFSET_NO_ENTRY_m12               STANDARD_UTC_OFFSET_NO_ENTRY_m12
#define METADATA_SECTION_3_PROTECTED_REGION_OFFSET_m12          15052
#define METADATA_SECTION_3_PROTECTED_REGION_BYTES_m12           668
#define METADATA_SECTION_3_DISCRETIONARY_REGION_OFFSET_m12      15720
#define METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m12       664

// Records: Header Format Constants
#define RECORD_HEADER_BYTES_m12			                        24
#define RECORD_HEADER_RECORD_CRC_OFFSET_m12		                0                       // ui4
#define RECORD_HEADER_RECORD_CRC_NO_ENTRY_m12	                        CRC_NO_ENTRY_m12
#define RECORD_HEADER_TOTAL_RECORD_BYTES_OFFSET_m12                     4                       // ui4
#define RECORD_HEADER_TOTAL_RECORD_BYTES_NO_ENTRY_m12			0
#define RECORD_HEADER_CRC_START_OFFSET_m12				RECORD_HEADER_TOTAL_RECORD_BYTES_OFFSET_m12
#define RECORD_HEADER_START_TIME_OFFSET_m12                             8                       // si8
#define RECORD_HEADER_START_TIME_NO_ENTRY_m12                           UUTC_NO_ENTRY_m12       // si8
#define RECORD_HEADER_TYPE_STRING_OFFSET_m12                            16	                // ascii[4]
#define RECORD_HEADER_TYPE_STRING_TERMINAL_ZERO_OFFSET_m12              (RECORD_HEADER_TYPE_STRING_OFFSET_m12 + 4)	// si1
#define RECORD_HEADER_TYPE_CODE_OFFSET_m12                              RECORD_HEADER_TYPE_STRING_OFFSET_m12		// ui4
#define RECORD_HEADER_TYPE_CODE_NO_ENTRY_m12		                0	                // ui4
#define RECORD_HEADER_VERSION_MAJOR_OFFSET_m12	                        21	                // ui1
#define RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m12	                0xFF
#define RECORD_HEADER_VERSION_MINOR_OFFSET_m12	                        22	                // ui1
#define RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m12	                0xFF
#define RECORD_HEADER_ENCRYPTION_LEVEL_OFFSET_m12                       23	                // si1
#define RECORD_HEADER_ENCRYPTION_LEVEL_NO_ENTRY_m12                     ENCRYPTION_LEVEL_NO_ENTRY_m12

// Record Index: Format Constants
#define RECORD_INDEX_BYTES_m12                                          INDEX_BYTES_m12
#define RECORD_INDEX_FILE_OFFSET_OFFSET_m12                             0                       // si8
#define RECORD_INDEX_FILE_OFFSET_NO_ENTRY_m12                           -1
#define RECORD_INDEX_START_TIME_OFFSET_m12                              8                       // si8
#define RECORD_INDEX_START_TIME_NO_ENTRY_m12                            UUTC_NO_ENTRY_m12
#define RECORD_INDEX_TYPE_STRING_OFFSET_m12                             16                      // ascii[4]
#define RECORD_INDEX_TYPE_STRING_TERMINAL_ZERO_OFFSET_m12               (RECORD_INDEX_TYPE_STRING_OFFSET_m12 + 4)	// si1
#define RECORD_INDEX_TYPE_CODE_OFFSET_m12                               RECORD_INDEX_TYPE_STRING_OFFSET_m12		// as ui4
#define RECORD_INDEX_TYPE_CODE_NO_ENTRY_m12                             0                       // as ui4
#define RECORD_INDEX_VERSION_MAJOR_OFFSET_m12	                        21                      // ui1
#define RECORD_INDEX_VERSION_MAJOR_NO_ENTRY_m12	                        0xFF
#define RECORD_INDEX_VERSION_MINOR_OFFSET_m12	                        22                      // ui1
#define RECORD_INDEX_VERSION_MINOR_NO_ENTRY_m12	                        0xFF
#define RECORD_INDEX_ENCRYPTION_LEVEL_OFFSET_m12                        23                      // si1
#define RECORD_INDEX_ENCRYPTION_LEVEL_NO_ENTRY_m12                      ENCRYPTION_LEVEL_NO_ENTRY_m12

// Time Series Index: Format Constants
#define TIME_SERIES_INDEX_BYTES_m12                                     INDEX_BYTES_m12
#define TIME_SERIES_INDEX_FILE_OFFSET_OFFSET_m12                        0               // si8
#define TIME_SERIES_INDEX_FILE_OFFSET_NO_ENTRY_m12                      -1
#define TIME_SERIES_INDEX_START_TIME_OFFSET_m12			        8               // si8
#define TIME_SERIES_INDEX_START_TIME_NO_ENTRY_m12                       UUTC_NO_ENTRY_m12
#define TIME_SERIES_INDEX_START_SAMPLE_NUMBER_OFFSET_m12                16              // si8
#define TIME_SERIES_INDEX_START_SAMPLE_NUMBER_NO_ENTRY_m12              -1

// Video Index: Format Constants
#define VIDEO_INDEX_BYTES_m12			                INDEX_BYTES_m12
#define VIDEO_INDEX_FILE_OFFSET_OFFSET_m12                      0                       // si8
#define VIDEO_INDEX_FILE_OFFSET_NO_ENTRY_m12                    -1
#define VIDEO_INDEX_START_TIME_OFFSET_m12                       8                       // si8
#define VIDEO_INDEX_START_TIME_NO_ENTRY_m12                     UUTC_NO_ENTRY_m12
#define VIDEO_INDEX_START_FRAME_OFFSET_m12                      16                      // ui4
#define VIDEO_INDEX_START_FRAME_NO_ENTRY_m12                    0xFFFFFFFF
#define VIDEO_INDEX_VIDEO_FILE_NUMBER_OFFSET_m12                20                      // ui4
#define VIDEO_INDEX_VIDEO_FILE_NUMBER_NO_ENTRY_m12              0
#define VIDEO_INDEX_TERMINAL_VIDEO_FILE_NUMBER_m12              0xFFFFFFFF

// Level Header (LH) Type Codes:
#define LH_SESSION_m12			SESSION_DIRECTORY_TYPE_CODE_m12
#define LH_SEGMENTED_SESS_RECS_m12	RECORD_DIRECTORY_TYPE_CODE_m12   // technically a session level element, but handy for distinguishing session records from segmented sess recs
#define LH_TIME_SERIES_CHANNEL_m12	TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m12
#define LH_VIDEO_CHANNEL_m12		VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m12
#define LH_TIME_SERIES_SEGMENT_m12	TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m12
#define LH_VIDEO_SEGMENT_m12		VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m12

// Level Header (LH) Flags Definitions:

// level header flags
// READ == on open: open file & read universal header (applies to data files, index files are always read in full)
//         on read: set FPS pointer to section specified by time slice (decrpyting if necessary)
// READ_FULL == on open: read full file (no memory mapping required, & closing)
// MMAP == allocate memory for full file, but only read on demand, (no re-reading occurs, but potentially memory expensive, good paired with VDS)
// ACTIVE == applies only to channels. Mark a CHANNEL as active to return data. Marking a channel as inactive does not free or close it.
// EPHEMERAL DATA == if GENERATE_EPHEMERAL_DATA_m12 is set, ephemeral data is created if it does not exist.
//	If UPDATE_EPHEMERAL_DATA is set, the data is updated whenever the channel or segment open set changes (opening of new elements, not the active status)
//	The UPDATE_EPHEMERAL_DATA bit is set by the lower levels and reset by the higher level once the data has been updated.
//	i.e  read_channel_m12() checks the segment bits (e.g. read_segment_m12() opened a new segment) & if update required, it does the channel level update & clears the segment bit.
//	It then sets it's bit to trigger update at the session level. After updating, the session will clear the channel level bit.

// all levels
#define LH_NO_FLAGS_m12					((ui8) 0)
#define LH_USE_GLOBAL_FLAGS_m12				LH_NO_FLAGS_m12
#define LH_OPEN_m12					((ui8) 1 << 0)	// level is open
#define LH_GENERATE_EPHEMERAL_DATA_m12			((ui8) 1 << 1)	// implies all level involvement
#define LH_UPDATE_EPHEMERAL_DATA_m12			((ui8) 1 << 2)	// signal to higher level from lower level (reset by higher level after update)

// session level
#define LH_EXCLUDE_TIME_SERIES_CHANNELS_m12		((ui8) 1 << 10)  // useful when session directory passed, but don't want time series channels
#define LH_EXCLUDE_VIDEO_CHANNELS_m12			((ui8) 1 << 11)  // useful when session directory passed, but don't want video channels
#define LH_MAP_ALL_TIME_SERIES_CHANNELS_m12		((ui8) 1 << 12)  // useful when time series channels may be added to open session
#define LH_MAP_ALL_VIDEO_CHANNELS_m12			((ui8) 1 << 13)  // useful when video channels may be added to open session

#define LH_READ_SLICE_SESSION_RECORDS_m12		((ui8) 1 << 16)	// read full indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_SESSION_RECORDS_m12		((ui8) 1 << 17)	// read full indices file & data files, close all files
#define LH_MEM_MAP_SESSION_RECORDS_m12			((ui8) 1 << 18)	// allocate, but don't read full file
// segmented session records level
#define LH_READ_SLICE_SEGMENTED_SESS_RECS_m12		((ui8) 1 << 19)	// read full indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_SEGMENTED_SESS_RECS_m12		((ui8) 1 << 20)	// read full indices file & data files, close all files
#define LH_MEM_MAP_SEGMENTED_SESS_RECS_m12		((ui8) 1 << 21)	// allocate, but don't read full data file

// channel level
#define LH_CHANNEL_ACTIVE_m12				((ui8) 1 << 32)
#define LH_REFERENCE_INACTIVE_m12			((ui8) 1 << 33)
#define LH_MAP_ALL_SEGMENTS_m12				((ui8) 1 << 34)
// (active channels only)
#define LH_READ_SLICE_CHANNEL_RECORDS_m12		((ui8) 1 << 40)	// read full indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_CHANNEL_RECORDS_m12		((ui8) 1 << 41)	// read full indices file & data files, close all files
#define LH_MEM_MAP_CHANNEL_RECORDS_m12			((ui8) 1 << 42)	// allocate, but don't read full file

// segment level (active channels only)
#define LH_READ_SLICE_SEGMENT_DATA_m12			((ui8) 1 << 48)	// read full metadata & indices files, close files; open data, read universal header, leave open
#define LH_READ_FULL_SEGMENT_DATA_m12			((ui8) 1 << 49)	// read full metadata, indices, & data files, close all files
#define LH_MEM_MAP_SEGMENT_DATA_m12			((ui8) 1 << 50)	// allocate, but don't read full file
#define LH_READ_SLICE_SEGMENT_RECORDS_m12		((ui8) 1 << 51)	// read full indices file (close file); open data, read universal header, leave open
#define LH_READ_FULL_SEGMENT_RECORDS_m12		((ui8) 1 << 52)	// read full indices file & data files, close all files
#define LH_MEM_MAP_SEGMENT_RECORDS_m12			((ui8) 1 << 53)	// allocate, but don't read full file
#define LH_READ_SEGMENT_METADATA_m12			((ui8) 1 << 54)	// read segment metadata
#define LH_NO_CPS_PTR_RESET_m12				((ui8) 1 << 60) // caller will update pointers
#define LH_NO_CPS_CACHING_m12				((ui8) 1 << 61) // set cps_caching parameter to FALSE
#define LH_THREAD_SEGMENT_READS_m12			((ui8) 1 << 63)	// set if likely to cross many segment boundaries in read (e.g. one channel, long read)

// flag groups
#define LH_MAP_ALL_CHANNELS_m12       	      (	LH_MAP_ALL_TIME_SERIES_CHANNELS_m12 | LH_MAP_ALL_VIDEO_CHANNELS_m12 )

// reading masks (not to be used as flags: SLICE/FULL mutually exclusive)
#define LH_READ_SESSION_RECORDS_MASK_m12      (	LH_READ_SLICE_SESSION_RECORDS_m12 | LH_READ_FULL_SESSION_RECORDS_m12 )
#define LH_READ_SEGMENTED_SESS_RECS_MASK_m12  (	LH_READ_SLICE_SEGMENTED_SESS_RECS_m12 | LH_READ_FULL_SEGMENTED_SESS_RECS_m12 )
#define LH_READ_CHANNEL_RECORDS_MASK_m12      (	LH_READ_SLICE_CHANNEL_RECORDS_m12 | LH_READ_FULL_CHANNEL_RECORDS_m12 )
#define LH_READ_SEGMENT_RECORDS_MASK_m12      (	LH_READ_SLICE_SEGMENT_RECORDS_m12 | LH_READ_FULL_SEGMENT_RECORDS_m12 )
#define LH_READ_SEGMENT_DATA_MASK_m12         (	LH_READ_SLICE_SEGMENT_DATA_m12 | LH_READ_FULL_SEGMENT_DATA_m12 )
#define LH_READ_RECORDS_MASK_m12	      (	LH_READ_SESSION_RECORDS_MASK_m12 | LH_READ_SEGMENTED_SESS_RECS_MASK_m12 | \
						LH_READ_CHANNEL_RECORDS_MASK_m12 | LH_READ_SEGMENT_RECORDS_MASK_m12 )
#define LH_ALL_READ_FLAGS_MASK_m12	      ( LH_READ_RECORDS_MASK_m12 | LH_READ_SEGMENT_DATA_MASK_m12 )
// memory map flags & masks
#define LH_MEM_MAP_ALL_RECORDS_m12	      ( LH_MEM_MAP_SESSION_RECORDS_m12 | LH_MEM_MAP_SEGMENTED_SESS_RECS_m12 | LH_MEM_MAP_CHANNEL_RECORDS_m12 | LH_MEM_MAP_SEGMENT_RECORDS_m12 )
#define LH_MEM_MAP_ALL_m12	      	      (	LH_MEM_MAP_ALL_RECORDS_m12 | LH_MEM_MAP_SEGMENT_DATA_m12 )
#define LH_ALL_MEM_MAP_FLAGS_MASK_m12		LH_MEM_MAP_ALL_m12

// record reading groups
#define LH_READ_SLICE_ALL_RECORDS_m12	      (	LH_READ_SLICE_SESSION_RECORDS_m12 | LH_READ_SLICE_SEGMENTED_SESS_RECS_m12 | LH_READ_SLICE_CHANNEL_RECORDS_m12 | LH_READ_SLICE_SEGMENT_RECORDS_m12 )
#define LH_READ_FULL_ALL_RECORDS_m12	      (	LH_READ_FULL_SESSION_RECORDS_m12 | LH_READ_FULL_SEGMENTED_SESS_RECS_m12 | LH_READ_FULL_CHANNEL_RECORDS_m12 | LH_READ_FULL_SEGMENT_RECORDS_m12 )

// channel type groups
#define LH_INCLUDE_ALL_CHAN_TYPES_m12	      (	LH_INCLUDE_TIME_SERIES_CHANNELS_m12 | LH_INCLUDE_VIDEO_CHANNELS_m12 )



//**********************************************************************************//
//*******************************  Processes (PROC)  *******************************//
//**********************************************************************************//

// Thread Management Constants
#define PROC_DEFAULT_PRIORITY_m12    	((si4) 0x7FFFFFFF)
#define PROC_MIN_PRIORITY_m12        	((si4) 0x7FFFFFFE)
#define PROC_LOW_PRIORITY_m12		((si4) 0x7FFFFFFD)
#define PROC_MEDIUM_PRIORITY_m12	((si4) 0x7FFFFFFC)
#define PROC_HIGH_PRIORITY_m12		((si4) 0x7FFFFFFB)
#define PROC_MAX_PRIORITY_m12		((si4) 0x7FFFFFFA)
#define PROC_UNDEFINED_PRIORITY_m12	((si4) 0x7FFFFFF9)

#define PROC_THREAD_WAITING_m12		((si4) 0)
#define PROC_THREAD_RUNNING_m12		((si4) 1)
#define PROC_THREAD_FINISHED_m12	((si4) 2)


typedef ui8		pid_t_m12;	// big enough for all OSs, none use signed values
					// (pid_t_m12 is used for both process and thread IDs throughout the library)

typedef void 	(*sig_handler_t_m12)(si4);  // signal handler function pointer

#if defined MACOS_m12 || defined LINUX_m12
	#ifdef MACOS_m12
		typedef	ui4			cpu_set_t_m12;  // max 32 logical cores
	#else
		typedef	cpu_set_t		cpu_set_t_m12;  // unknown logical cores
	#endif
	typedef	pthread_t		pthread_t_m12;
	typedef pthread_attr_t		pthread_attr_t_m12;
	typedef void *			pthread_rval_m12;
	typedef pthread_rval_m12 	(*pthread_fn_m12)(void *);
	typedef	pthread_mutex_t		pthread_mutex_t_m12;
	typedef	pthread_mutexattr_t	pthread_mutexattr_t_m12;
#endif

#ifdef WINDOWS_m12
	typedef	ui8			cpu_set_t_m12;  // max 64 logical cores (defined as DWORD_PTR in Windows, but not used as a pointer; used as ui8)
	typedef	HANDLE			pthread_t_m12;
	typedef void *			pthread_attr_t_m12;
	typedef ui4 			pthread_rval_m12;
	typedef pthread_rval_m12 	(*pthread_fn_m12)(void *);
	typedef	HANDLE			pthread_mutex_t_m12;
	typedef	SECURITY_ATTRIBUTES	pthread_mutexattr_t_m12;
#endif

typedef struct {
	pthread_fn_m12		thread_f;  // the thread function pointer
	si1			*thread_label;
	void			*arg;  // function-specific info structure, set by calling function
	si4			priority;  // typically PROC_HIGH_PRIORITY_m12
	volatile si4		status;
	pthread_t_m12		thread_id;
} PROC_THREAD_INFO_m12;


TERN_m12	PROC_adjust_open_file_limit_m12(si4 new_limit, TERN_m12 verbose_flag);
TERN_m12	PROC_distribute_jobs_m12(PROC_THREAD_INFO_m12 *thread_infos, si4 n_jobs, si4 n_reserved_cores, TERN_m12 wait_jobs);
cpu_set_t_m12	*PROC_generate_cpu_set_m12(si1 *affinity_str, cpu_set_t_m12 *cpu_set_p);
pid_t_m12	PROC_getpid_m12(void);  // calling process id
pid_t_m12	PROC_gettid_m12(void);  // calling thread id
TERN_m12	PROC_increase_process_priority_m12(TERN_m12 verbose_flag, si4 sudo_prompt_flag, ...);  // varargs (sudo_prompt_flag == TRUE_m12): si1 *exec_name, sf8 timeout_secs
ui4		PROC_launch_thread_m12(pthread_t_m12 *thread_id, pthread_fn_m12 thread_f, void *arg, si4 priority, si1 *affinity_str, cpu_set_t_m12 *cpu_set_p, TERN_m12 detached, si1 *thread_name);
si4		PROC_pthread_join_m12(pthread_t_m12 thread_id, void **value_ptr);
si4		PROC_pthread_mutex_destroy_m12(pthread_mutex_t_m12 *mutex);
si4		PROC_pthread_mutex_init_m12(pthread_mutex_t_m12 *mutex, pthread_mutexattr_t_m12 *attr);
si4		PROC_pthread_mutex_lock_m12(pthread_mutex_t_m12 *mutex);
si4		PROC_pthread_mutex_trylock_m12(pthread_mutex_t_m12 *mutex);
si4		PROC_pthread_mutex_unlock_m12(pthread_mutex_t_m12 *mutex);
pthread_t_m12	PROC_pthread_self_m12(void);
TERN_m12	PROC_set_thread_affinity_m12(pthread_t_m12 *thread_id_p, pthread_attr_t_m12 *attributes, cpu_set_t_m12 *cpu_set_p, TERN_m12 wait_for_lauch);
void		PROC_show_thread_affinity_m12(pthread_t_m12 *thread_id);
TERN_m12	PROC_wait_jobs_m12(PROC_THREAD_INFO_m12 *thread_infos, si4 n_jobs);



//**********************************************************************************//
//**************************  Parallel (PAR) Functions  ****************************//
//**********************************************************************************//

// Constants
#define PAR_LAUNCHING_m12		1
#define PAR_RUNNING_m12			2
#define PAR_FINISHED_m12		3
#define PAR_DEFAULTS_m12		"defaults"
#define PAR_UNTHREADED_m12		0

// PAR function IDs (use PROC function IDs)
#define PAR_OPEN_SESSION_m12		1
#define PAR_READ_SESSION_m12		2
#define PAR_OPEN_CHANNEL_m12		3
#define PAR_READ_CHANNEL_m12		4
#define PAR_OPEN_SEGMENT_m12		5
#define PAR_READ_SEGMENT_m12		6
#define PAR_GET_MATRIX_m12		7

// Structures
typedef struct {
	si1		label[64];
	si1		function[64];
	void		*ret_val;  // pointer to returned data
	pid_t_m12	tid;
	pthread_t_m12	thread_id;
	si4		priority;
	si1		affinity[16];
	si4		detached;
	si4		status;
} PAR_INFO_m12;

typedef struct {
	PAR_INFO_m12	*par_info;
	va_list		args;
} PAR_THREAD_INFO_m12;

// Protoypes
void			PAR_free_m12(PAR_INFO_m12 **par_info_ptr);
PAR_INFO_m12		*PAR_init_m12(PAR_INFO_m12 *par_info, si1 *function, si1 *label, ...); // varagrgs(label == PAR_DEFAULTS_m12): si4 priority, si1 *affinity, si4 detached
PAR_INFO_m12		*PAR_launch_m12(PAR_INFO_m12 *par_info, ...);	// varargs (par_info == NULL): si1 *function, si1 *label, si4 priority, si1 *affinity, si4 detached, <function arguments>
									// varargs (par_info != NULL): <function arguments>
void			PAR_show_info_m12(PAR_INFO_m12 *par_info);
pthread_rval_m12	PAR_thread_m12(void *arg);
void			PAR_wait_m12(PAR_INFO_m12 *par_info, si1 *interval);



//**********************************************************************************//
//***************************  Parity (PRTY) Functions  ****************************//
//**********************************************************************************//

// Flags
#define PRTY_GLB_SESS_REC_DATA_m12	((ui4) 1 << 0)
#define PRTY_GLB_SESS_REC_IDX_m12	((ui4) 1 << 1)
#define PRTY_SEG_SESS_REC_DATA_m12	((ui4) 1 << 2)
#define PRTY_SEG_SESS_REC_IDX_m12	((ui4) 1 << 3)
#define PRTY_TS_CHAN_REC_DATA_m12	((ui4) 1 << 4)
#define PRTY_TS_CHAN_REC_IDX_m12	((ui4) 1 << 5)
#define PRTY_TS_SEG_REC_DATA_m12	((ui4) 1 << 6)		// requires segment number (or PRTY_ALL_SEGS_m12)
#define PRTY_TS_SEG_REC_IDX_m12		((ui4) 1 << 7)		// requires segment number (or PRTY_ALL_SEGS_m12)
#define PRTY_TS_SEG_DAT_DATA_m12	((ui4) 1 << 8)		// requires segment number (or PRTY_ALL_SEGS_m12)
#define PRTY_TS_SEG_DAT_IDX_m12		((ui4) 1 << 9)		// requires segment number (or PRTY_ALL_SEGS_m12)
#define PRTY_TS_SEG_META_m12		((ui4) 1 << 10)		// requires segment number (or PRTY_ALL_SEGS_m12)
#define PRTY_VID_CHAN_REC_DATA_m12	((ui4) 1 << 11)
#define PRTY_VID_CHAN_REC_IDX_m12	((ui4) 1 << 12)
#define PRTY_VID_SEG_REC_DATA_m12	((ui4) 1 << 13)		// requires segment number (or PRTY_ALL_SEGS_m12)
#define PRTY_VID_SEG_REC_IDX_m12	((ui4) 1 << 14)		// requires segment number (or PRTY_ALL_SEGS_m12)
#define PRTY_VID_SEG_DAT_DATA_m12	((ui4) 1 << 15)		// requires segment number (or PRTY_ALL_SEGS_m12)
#define PRTY_VID_SEG_DAT_IDX_m12	((ui4) 1 << 16)		// requires segment number (or PRTY_ALL_SEGS_m12)
#define PRTY_VID_SEG_META_m12		((ui4) 1 << 17)		// requires segment number (or PRTY_ALL_SEGS_m12)

#define PRTY_GLB_SESS_RECS_m12		(PRTY_GLB_SESS_REC_DATA_m12 | PRTY_GLB_SESS_REC_IDX_m12)
#define PRTY_SEG_SESS_RECS_m12		(PRTY_SEG_SESS_REC_DATA_m12 | PRTY_SEG_SESS_REC_IDX_m12)
#define PRTY_TS_CHAN_RECS_m12		(PRTY_TS_CHAN_REC_DATA_m12 | PRTY_TS_CHAN_REC_IDX_m12)
#define PRTY_VID_CHAN_RECS_m12		(PRTY_VID_CHAN_REC_DATA_m12 | PRTY_VID_CHAN_REC_IDX_m12)
#define PRTY_TS_SEG_RECS_m12		(PRTY_TS_SEG_REC_DATA_m12 | PRTY_TS_SEG_REC_IDX_m12)
#define PRTY_VID_SEG_RECS_m12		(PRTY_VID_SEG_REC_DATA_m12 | PRTY_VID_SEG_REC_IDX_m12)
#define PRTY_TS_SEG_DATA_m12		(PRTY_TS_SEG_DAT_DATA_m12 | PRTY_TS_SEG_DAT_IDX_m12)
#define PRTY_VID_SEG_DATA_m12		(PRTY_VID_SEG_DAT_DATA_m12 | PRTY_VID_SEG_DAT_IDX_m12)

#define PRTY_SESS_RECS_m12		(PRTY_GLB_SESS_RECS_m12 | PRTY_SEG_SESS_RECS_m12)
#define PRTY_CHAN_RECS_m12		(PRTY_TS_CHAN_RECS_m12 | PRTY_VID_CHAN_RECS_m12)
#define PRTY_SEG_RECS_m12		(PRTY_TS_SEG_RECS_m12 | PRTY_VID_SEG_RECS_m12)
#define PRTY_SEG_DATA_m12		(PRTY_TS_SEG_DATA_m12 | PRTY_VID_SEG_DATA_m12)

#define PRTY_TS_CHAN_m12		PRTY_TS_CHAN_RECS_m12
#define PRTY_TS_SEG_m12			(PRTY_TS_SEG_RECS_m12 | PRTY_TS_SEG_DATA_m12 | PRTY_TS_SEG_META_m12)
#define PRTY_VID_CHAN_m12		PRTY_VID_CHAN_RECS_m12
#define PRTY_VID_SEG_m12		(PRTY_VID_SEG_RECS_m12 | PRTY_VID_SEG_DATA_m12 | PRTY_VID_SEG_META_m12)

#define PRTY_SESS_m12			(PRTY_SESS_RECS_m12 | PRTY_SEG_SESS_RECS_m12)
#define PRTY_CHAN_m12			(PRTY_TS_CHAN_m12 | PRTY_VID_CHAN_m12)
#define PRTY_SEG_m12			(PRTY_TS_SEG_m12 | PRTY_VID_SEG_m12)

#define PRTY_ALL_TS_m12			(PRTY_SESS_m12 | PRTY_TS_CHAN_m12 | PRTY_TS_SEG_m12)
#define PRTY_ALL_VID_m12		(PRTY_SESS_m12 | PRTY_VID_CHAN_m12 | PRTY_VID_SEG_m12)
#define PRTY_ALL_FILES_m12		(PRTY_ALL_TS_m12 | PRTY_ALL_VID_m12)
#define PRTY_ALL_SEGS_m12		((si4) -1)  // pass as "segment_number" argument to PRTY_write_m12()

// Masks
#define PRTY_TS_MASK_m12		(PRTY_TS_CHAN_m12 | PRTY_TS_SEG_m12)
#define PRTY_VID_MASK_m12		(PRTY_VID_CHAN_m12 | PRTY_VID_SEG_m12)

// Parity file array fixed positions
#define PRTY_FILE_CHECK_IDX_m12		0  				// file to check in first slot
#define PRTY_FILE_DAMAGED_IDX_m12	PRTY_FILE_CHECK_IDX_m12		// damaged file in first slot

// Miscellaneous
#define PRTY_BLOCK_BYTES_DEFAULT_m12	4096  // used in PRTY_CRC_DATA_m12 (must be multiple of 4)
#define PRTY_PCRC_UID_m12		((ui8) 0x0123456789ABCDEF)  // used in PRTY_CRC_DATA_m12


// Structures
typedef struct {
	si1		path[FULL_FILE_NAME_BYTES_m12];
	si8		len;
	FILE		*fp;
	TERN_m12	finished;  // data incorporated into parity
} PRTY_FILE_m12;

typedef struct {
	si8	length;
	si8	offset;
} PRTY_BLOCK_m12;  // bad block location returned from PRTY_validate_m12()

typedef struct {
	ui1		*parity;
	ui1		*data;
	si8		mem_block_bytes;
	si1		path[FULL_FILE_NAME_BYTES_m12];  // path to parity file
	PRTY_FILE_m12	*files;
	si4		n_files;
	si4		n_bad_blocks;
	PRTY_BLOCK_m12	*bad_blocks;
} PRTY_m12;

typedef struct {
	ui8		pcrc_UID;  // == PRTY_UID_m12 (marker to confirm identity of this structure)
	ui8		session_UID;  // present in all parity files
	ui8		segment_UID;  // zero in parity data that is session level
	ui4		number_of_blocks;  // number of data blocks (& crcs) preceding this structure
	ui4		block_bytes;  // bytes per block (except probably the last), multiple of 4 bytes (defaults to 4096)
} PRTY_CRC_DATA_m12;

// Parity File Structure:
// 1) parity data
// 2) crc of parity data in blocks  // used to confirm that parity data is not itself damaged, & if so, to localize the damage, so that it can hopefully still be used & then rebuilt
// 3) PRTY_CRC_DATA_m12 structure

// Prototypes
TERN_m12	PRTY_build_m12(PRTY_m12 *parity_ps);
si4		PRTY_file_compare_m12(const void *a, const void *b);
si1		**PRTY_file_list_m12(si1 *MED_path, si4 *n_files);
ui4		PRTY_flag_for_path_m12(si1 *path);
si8		PRTY_pcrc_length_m12(FILE *fp, si1 *file_path);
TERN_m12	PRTY_recover_segment_header_fields_m12(si1 *MED_file, ui8 *segment_uid, si4 *segment_number);
TERN_m12	PRTY_repair_file_m12(PRTY_m12 *parity_ps);
TERN_m12	PRTY_restore_m12(si1 *MED_path);
TERN_m12	PRTY_set_pcrc_uids_m12(PRTY_CRC_DATA_m12 *pcrc, si1 *MED_path);
TERN_m12	PRTY_show_pcrc_m12(si1 *file_path);
TERN_m12        PRTY_validate_m12(si1 *file_path, ...);  // varargs(file_path == NULL): si1 *file_path, PRTY_BLOCK_m12 **bad_blocks, si4 *n_bad_blocks, ui4 *n_blocks
TERN_m12	PRTY_validate_pcrc_m12(si1 *file_path, ...);  // varargs(file_path == NULL): si1 *file_path, PRTY_BLOCK_m12 **bad_blocks, si4 *n_bad_blocks, ui4 *n_blocks
TERN_m12	PRTY_write_m12(si1 *sess_path, ui4 flags, si4 segment_number);
TERN_m12	PRTY_write_pcrc_m12(si1 *file_path, ui4 block_bytes);



//**********************************************************************************//
//********************  Runtime Configuration (RC) Functions  **********************//
//**********************************************************************************//

// Constants
#define RC_NO_ENTRY_m12    		-1
#define RC_NO_OPTION_m12   		0
#define RC_STRING_TYPE_m12      	1
#define RC_FLOAT_TYPE_m12       	2
#define RC_INTEGER_TYPE_m12     	3
#define RC_TERNARY_TYPE_m12     	4
#define RC_UNKNOWN_TYPE_m12     	5

#define RC_STRING_BYTES_m12		256


// Prototypes
si4	RC_read_field_m12(si1 *field_name, si1 **buffer, TERN_m12 update_buffer_ptr, si1 *field_value_str, sf8 *float_val, si8 *int_val, TERN_m12 *TERN_val);
si4     RC_read_field_2_m12(si1 *field_name, si1 **buffer, TERN_m12 update_buffer_ptr, void *val, si4 val_type, ...);  // vararg (val_type == RC_UNKNOWN_m12): *returned_val_type


//**********************************************************************************//
//*************************  Networking (NET) Functions  ***************************//
//**********************************************************************************//

// Constants
#define NET_MAC_ADDRESS_BYTES_m12		6
#define NET_MAC_ADDRESS_STR_BYTES_m12		(NET_MAC_ADDRESS_BYTES_m12 * 3)  // 6 hex bytes plus colons & terminal zero
#define NET_IPV4_ADDRESS_BYTES_m12		4
#define NET_IPV4_ADDRESS_STR_BYTES_m12		(NET_IPV4_ADDRESS_BYTES_m12 * 4)  // 4 dec bytes plus periods & terminal zero


// Structures
typedef struct {
	si1		interface_name[64];
	si1		host_name[256];  // max 253 ascii characters
	union {
		ui1	MAC_address_bytes[8];  // network byte order
		ui8	MAC_address_num;  // network byte order
	};
	si1             MAC_address_string[NET_MAC_ADDRESS_STR_BYTES_m12]; // upper case hex with colons
	union {
		ui1	LAN_IPv4_address_bytes[NET_IPV4_ADDRESS_BYTES_m12];  // network byte order
		ui4	LAN_IPv4_address_num;  // network byte order
	};
	si1             LAN_IPv4_address_string[NET_IPV4_ADDRESS_STR_BYTES_m12];  // dec with periods
	union {
		ui1	LAN_IPv4_subnet_mask_bytes[NET_IPV4_ADDRESS_BYTES_m12];  // network byte order
		ui4	LAN_IPv4_subnet_mask_num;  // network byte order
	};
	si1             LAN_IPv4_subnet_mask_string[NET_IPV4_ADDRESS_STR_BYTES_m12];  // dec with periods
	union {
		ui1	WAN_IPv4_address_bytes[NET_IPV4_ADDRESS_BYTES_m12];  // network byte order
		ui4	WAN_IPv4_address_num;  // network byte order
	};
	si1             WAN_IPv4_address_string[NET_IPV4_ADDRESS_STR_BYTES_m12];  // dec with periods
	si4             MTU;  // maximum transmission unit
	si1             link_speed[16];
	si1             duplex[16];
	TERN_m12        active;  // interface status
	TERN_m12        plugged_in;
} NET_PARAMS_m12;


// Prototypes
TERN_m12	NET_check_internet_connection_m12(void);
TERN_m12	NET_domain_to_ip_m12(si1 *domain_name, si1 *ip);
NET_PARAMS_m12	*NET_get_active_m12(si1 *iface, NET_PARAMS_m12 *np);
TERN_m12	NET_get_adapter_m12(NET_PARAMS_m12 *np, TERN_m12 copy_global);
TERN_m12	NET_get_config_m12(NET_PARAMS_m12 *np, TERN_m12 copy_global);
NET_PARAMS_m12	*NET_get_default_interface_m12(NET_PARAMS_m12 *np);
NET_PARAMS_m12	*NET_get_duplex_m12(si1 *iface, NET_PARAMS_m12 *np);
TERN_m12	NET_get_ethtool_m12(NET_PARAMS_m12 *np, TERN_m12 copy_global);
NET_PARAMS_m12	*NET_get_host_name_m12(NET_PARAMS_m12 *np);
void		*NET_get_in_addr_m12(struct sockaddr *sa);
NET_PARAMS_m12	*NET_get_lan_ipv4_address_m12(si1 *iface, NET_PARAMS_m12 *np);
NET_PARAMS_m12	*NET_get_link_speed_m12(si1 *iface, NET_PARAMS_m12 *np);
NET_PARAMS_m12	*NET_get_mac_address_m12(si1 *iface, NET_PARAMS_m12 *np);
NET_PARAMS_m12	*NET_get_mtu_m12(si1 *iface, NET_PARAMS_m12 *np);
NET_PARAMS_m12	*NET_get_parameters_m12(si1 *iface, NET_PARAMS_m12 *np);
NET_PARAMS_m12	*NET_get_plugged_in_m12(si1 *iface, NET_PARAMS_m12 *np);
NET_PARAMS_m12	*NET_get_wan_ipv4_address_m12(NET_PARAMS_m12 *np);
si1		*NET_iface_name_for_addr_m12(si1 *iface_name, si1 *iface_addr);
TERN_m12	NET_initialize_tables_m12(void);  // set global NET_PARAMS for default internet interface
void		NET_reset_parameters_m12(NET_PARAMS_m12 *np);
TERN_m12	NET_resolve_arguments_m12(si1 *iface, NET_PARAMS_m12 **params_ptr, TERN_m12 *free_params);
void            NET_show_parameters_m12(NET_PARAMS_m12 *np);
void		NET_trim_address_m12(si1 *addr_str);



//**********************************************************************************//
//**********************************  MED Errors  **********************************//
//**********************************************************************************//

// error codes
#define	E_NO_ERR_m12			0
#define E_NO_FILE_m12			1
#define E_READ_ERR_m12			2
#define E_WRITE_ERR_m12			3
#define E_NOT_MED_m12			4
#define E_BAD_PASSWORD_m12		5
#define E_NO_METADATA_m12		6
#define	E_NO_INET_m12			7


// error strings
#define	E_NO_ERR_STR_m12		"no errors"
#define	E_NO_FILE_STR_m12		"file not found"
#define	E_READ_ERR_STR_m12		"file read error"
#define	E_WRITE_ERR_STR_m12		"file write error"
#define E_NOT_MED_STR_m12		"not a MED file or directory"
#define E_BAD_PASSWORD_STR_m12		"invalid password"
#define E_NO_METADATA_STR_m12		"metadata file not found"
#define	E_NO_INET_STR_m12		"no internet connection found"



//**********************************************************************************//
//**********************************  MED Macros  **********************************//
//**********************************************************************************//

#define PLURAL_m12(x) 			( ((x) == 1) ? "" : "s" )
#define ABS_m12(x)			( ((x) >= 0) ? (x) : -(x) )	// do not increment/decrement in call to ABS (as x occurs thrice)
#define HEX_STRING_BYTES_m12(x)         ( ((x) + 1) * 3 )
#define REMOVE_DISCONTINUITY_m12(x)     ( ((x) >= 0) ? (x) : -(x) )	// do not increment/decrement in call to REMOVE_DISCONTINUITY (as x occurs thrice)
#define APPLY_DISCONTINUITY_m12(x)      ( ((x) <= 0) ? (x) : -(x) )	// do not increment/decrement in call to APPLY_DISCONTINUITY (as x occurs thrice)
#define MAX_OPEN_FILES_m12(number_of_channels, number_of_segments)      ((5 * number_of_channels * number_of_segments) + (2 * number_of_segments) + (2 * number_of_channels) + 5)
									// Note: final +5 == 2 for session level records plus 3 for standard streams (stdin, stdout, & stderr)
// "S" versions are for slice structures (not pointers)
#define TIME_SLICE_SAMPLE_COUNT_m12(slice)	(((slice)->end_sample_number - (slice)->start_sample_number) + 1)
#define TIME_SLICE_SAMPLE_COUNT_S_m12(slice)	(((slice).end_sample_number - (slice).start_sample_number) + 1)
#define TIME_SLICE_SEGMENT_COUNT_m12(slice)	(((slice)->end_segment_number - (slice)->start_segment_number) + 1)
#define TIME_SLICE_SEGMENT_COUNT_S_m12(slice)	(((slice).end_segment_number - (slice).start_segment_number) + 1)
#define TIME_SLICE_DURATION_m12(slice)		(((slice)->end_time - (slice)->start_time) + 1)  // time in usecs
#define TIME_SLICE_DURATION_S_m12(slice)	(((slice).end_time - (slice).start_time) + 1)  // time in usecs



//**********************************************************************************//
//***********************************  Hardware  ***********************************//
//**********************************************************************************//

// Structures
typedef struct {
	si8		integer_multiplications_per_sec;  // test mimics RED/PRED in operand length, other tests may yield somewhat different results
	si8		integer_divisions_per_sec;  // test mimics RED/PRED in operand length, other tests may yield somewhat different results
	sf8		nsecs_per_integer_multiplication;  // test mimics RED/PRED in operand length, other tests may yield somewhat different results
	sf8		nsecs_per_integer_division;  // test mimics RED/PRED in operand length, other tests may yield somewhat different results
} HW_PERFORMANCE_SPECS_m12;

typedef struct {
	ui1				endianness;
	si4				physical_cores;
	si4				logical_cores;
	TERN_m12			hyperthreading;
	sf8				minimum_speed;  // GHz
	sf8				maximum_speed;  // GHz
	sf8				current_speed;  // GHz
	HW_PERFORMANCE_SPECS_m12	performance_specs;
	ui8				system_memory_size;  // system physical RAM (in bytes)
	ui4				system_page_size;  // memory page (in bytes)
	ui8				heap_base_address;
	ui8				heap_max_address;
	si1				cpu_manufacturer[64];
	si1				cpu_model[64];
	si1				serial_number[56];  // maximum serial number length is 50 characters
	ui4				machine_code;  // code based on serial number
} HW_PARAMS_m12;

// Prototypes
void		HW_get_core_info_m12(void);
void		HW_get_endianness_m12(void);
void		HW_get_info_m12(void);  // fill whole HW_PARAMS_m12 structure
void		HW_get_machine_code_m12(void);
void		HW_get_machine_serial_m12(void);
void		HW_get_performance_specs_m12(TERN_m12 get_current);
si1		*HW_get_performance_specs_file_m12(si1 *file);
TERN_m12	HW_get_performance_specs_from_file_m12(void);
void		HW_get_memory_info_m12(void);
TERN_m12	HW_initialize_tables_m12(void);
void		HW_show_info_m12(void);



//**********************************************************************************//
//**********************************  General MED  *********************************//
//**********************************************************************************//

// Daylight Change code
typedef union {
	struct {
		si1     code_type;                      // (DST end / DST Not Observed / DST start) ==  (-1 / 0 / +1)
		si1	day_of_week;                    // (No Entry / [Sunday : Saturday]) ==  (-1 / [0 : 6])
		si1     relative_weekday_of_month;      // (No Entry / [First : Fifth] / Last) ==  (0 / [1 : 5] / 6)
		si1     day_of_month;                   // (No Entry / [1 : 31]) ==  (0 / [1 : 31])
		si1     month;                          // (No Entry / [January : December]) ==  (-1 / [0 : 11])
		si1     hours_of_day;                   // [-128 : +127] hours relative to 0:00 (midnight)
		si1     reference_time;                 // (Local / UTC) ==  (0 / +1)
		si1     shift_minutes;                  // [-120 : +120] minutes
	};
	si8     value;                                  // 0 indicates DST is not observed, -1 indicates no entry
} DAYLIGHT_TIME_CHANGE_CODE_m12;

typedef struct {
	si1	country[METADATA_RECORDING_LOCATION_BYTES_m12];
	si1	country_acronym_2_letter[3]; // two-letter acronym; (ISO 3166 ALPHA-2)
	si1	country_acronym_3_letter[4]; // three-letter acronym (ISO-3166 ALPHA-3)
	si1	territory[METADATA_RECORDING_LOCATION_BYTES_m12];
	si1	territory_acronym[TIMEZONE_STRING_BYTES_m12];
	si1	standard_timezone[TIMEZONE_STRING_BYTES_m12];
	si1	standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m12];
	si4	standard_UTC_offset; // seconds
	si1	daylight_timezone[TIMEZONE_STRING_BYTES_m12];
	si1	daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m12];
	si8	daylight_time_start_code;  // DAYLIGHT_TIME_CHANGE_CODE_m12 - cast to use other fields
	si8	daylight_time_end_code;  // DAYLIGHT_TIME_CHANGE_CODE_m12 - cast to use other fields
	si8	daylight_codes_start_date;  // onset of rules encoded in daylight codes (in uutc). DTCC_START_DATE_NO_ENTRY (-1) indicates it is the only historical rule for this timezone in the table.
} TIMEZONE_INFO_m12;

typedef struct {
	si1	table_name[METADATA_RECORDING_LOCATION_BYTES_m12];
	si1	alias[METADATA_RECORDING_LOCATION_BYTES_m12];
} TIMEZONE_ALIAS_m12;

typedef struct {
	TERN_m12	conditioned;
	si4		number_of_segments;  // == UNKNOWN_m12 if segment range is unknown, otherwise == number of segments in slice
	si8     	start_time;
	si8     	end_time;
	union {  // session-relative (global indexing)
		si8	start_sample_number;
		si8	start_frame_number;
	};
	union {  // session-relative (global indexing)
		si8	end_sample_number;
		si8	end_frame_number;
	};
	si4     	start_segment_number;
	si4     	end_segment_number;
} TIME_SLICE_m12;

typedef struct {
	si8     	start_time;
	si8     	end_time;
	union {  // session-relative (global indexing)
		si8	start_sample_number;
		si8	start_frame_number;
	};
	union {  // session-relative (global indexing)
		si8	end_sample_number;
		si8	end_frame_number;
	};
	si4     	start_segment_number;
	si4     	end_segment_number;
} CONTIGUON_m12;

typedef struct {  // times in uutc
	si8	creation;
	si8	access;
	si8	modification;
} FILE_TIMES_m12;

typedef struct {  // fields from ipinfo.io
	TIMEZONE_INFO_m12	timezone_info;
	si1			WAN_IPv4_address[IPV4_ADDRESS_BYTES_m12 * 4];
	si1			locality[LOCALITY_BYTES_m12];
	si1			postal_code[POSTAL_CODE_BYTES_m12];
	si1			timezone_description[METADATA_RECORDING_LOCATION_BYTES_m12];
	sf8			latitude;
	sf8			longitude;
} LOCATION_INFO_m12;

typedef struct {
	void 		*address;
	ui8		requested_bytes;
	ui8		actual_bytes;  // actual bytes allocated => may be more than were requested
	const si1	*alloc_function;
	const si1	*free_function;
} AT_NODE;

typedef struct {
	// Identifier
	pid_t_m12			_id;  // thread or process id
	// Password
	PASSWORD_DATA_m12               password_data;
	// Record Filters
	si4 				*record_filters;	// signed, "NULL terminated" array version of MED record type codes to include or exclude when reading records.
								// The terminal entry is NO_TYPE_CODE_m12 (== zero). NULL or no filter codes includes all records (== no filters).
								// filter modes: match positive: include
								//		 match negative: exclude
								//		 no match:
								//			all filters positive: exclude
								//			else: include
								// Note: as type codes are composed of ascii bytes values (< 0x80), it is always possible to make them negative without promotion.
	// Current Session
	ui8				session_UID;
	si1				session_directory[FULL_FILE_NAME_BYTES_m12];	// path including file system session directory name
	si1				*session_name;  				// points to: uh_session_name if known, else fs_session_name if known, else NULL
	si1				uh_session_name[BASE_FILE_NAME_BYTES_m12];	// from MED universal header - original name
	si1				fs_session_name[BASE_FILE_NAME_BYTES_m12];	// from file system - may be renamed by user (e.g. channel subset)
	si8				session_start_time;
	si8				session_end_time;
	union {
		si8			number_of_session_samples;
		si8			number_of_session_frames;
	};
	si4				number_of_session_segments;	// number of segments in the session, regardless of whether they are mapped
	si4				number_of_mapped_segments;	// may be less than number_of_session_segments
	si4				first_mapped_segment_number;
	si1				reference_channel_name[BASE_FILE_NAME_BYTES_m12];	// contains user specified value if needed, open_session_m12() matches to session channel
	struct CHANNEL_m12		*reference_channel;		// note "reference" here refers to reference channel for sample/frame numbers, not the time series recording reference electrode
	// Active Channels (applies to active channel set)
	TERN_m12 			time_series_frequencies_vary;
	sf8				minimum_time_series_frequency;
	sf8				maximum_time_series_frequency;
	struct CHANNEL_m12		*minimum_time_series_frequency_channel;
	struct CHANNEL_m12		*maximum_time_series_frequency_channel;
	TERN_m12 			video_frame_rates_vary;
	sf8				minimum_video_frame_rate;
	sf8				maximum_video_frame_rate;
	struct CHANNEL_m12		*minimum_video_frame_rate_channel;
	struct CHANNEL_m12		*maximum_video_frame_rate_channel;
	// Time Constants
	TERN_m12			time_constants_set;
	TERN_m12			RTO_known;
	TERN_m12                        observe_DST;
	si8                             recording_time_offset;
	si4                             standard_UTC_offset;
	si1                             standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m12];
	si1                             standard_timezone_string[TIMEZONE_STRING_BYTES_m12];
	si1                             daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m12];
	si1                             daylight_timezone_string[TIMEZONE_STRING_BYTES_m12];
	DAYLIGHT_TIME_CHANGE_CODE_m12   daylight_time_start_code;  // si1[8] / si8
	DAYLIGHT_TIME_CHANGE_CODE_m12   daylight_time_end_code;  // si1[8] / si8
	// Alignment Fields
	TERN_m12                        universal_header_aligned;
	TERN_m12                        metadata_section_1_aligned;
	TERN_m12                        time_series_metadata_section_2_aligned;
	TERN_m12                        video_metadata_section_2_aligned;
	TERN_m12                        metadata_section_3_aligned;
	TERN_m12                        all_metadata_structures_aligned;
	TERN_m12                        time_series_indices_aligned;
	TERN_m12                        video_indices_aligned;
	TERN_m12                        CMP_block_header_aligned;
	TERN_m12			CMP_record_header_aligned;
	TERN_m12                        record_header_aligned;
	TERN_m12                        record_indices_aligned;
	TERN_m12                        all_record_structures_aligned;
	TERN_m12                        all_structures_aligned;
	TERN_m12			transmission_header_aligned;
	// CRC
	ui4                             CRC_mode;

	// allocation tracking (AT)
	AT_NODE				*AT_nodes;
	si8				AT_node_count;  // total allocated nodes
	si8				AT_used_node_count;  // nodes in use
	pthread_mutex_t_m12		AT_mutex;

	// Errors
	si4				err_code;
	const si1			*err_func;
	si4				err_line;
	// Miscellaneous
	ui4				file_creation_umask;
	TERN_m12			time_series_data_encryption_level;
	TERN_m12                        verbose;
	ui4                             behavior_on_fail;
	si1				temp_dir[FULL_FILE_NAME_BYTES_m12];  // system temp directory (periodically auto-cleared)
	si1				temp_file[FULL_FILE_NAME_BYTES_m12];  // full path to temp file (i.e. incudes temp_dir), not thread safe => use G_unique_temp_file_m12() in threaded applications
	ui4				*behavior_stack;
	volatile ui4			behavior_stack_entries;
	volatile ui4			behavior_stack_size;
	pthread_mutex_t_m12		behavior_mutex;
	ui8				level_header_flags;
	ui4				mmap_block_bytes;  // read size for memory mapped files
} GLOBALS_m12;

typedef struct {
	TIMEZONE_INFO_m12		*timezone_table;
	TIMEZONE_ALIAS_m12		*country_aliases_table;
	TIMEZONE_ALIAS_m12		*country_acronym_aliases_table;
	ui4				**CRC_table;
	si4				*AES_sbox_table;
	si4				*AES_rsbox_table;
	si4				*AES_rcon_table;
	ui4				*SHA_h0_table;
	ui4				*SHA_k_table;
	ui4				*UTF8_offsets_table;
	si1				*UTF8_trailing_bytes_table;
	sf8				*CMP_normal_CDF_table;
	CMP_VDS_THRESHOLD_MAP_ENTRY_m12	*CMP_VDS_threshold_map;
	NET_PARAMS_m12			NET_params;  // parameters for default internet interface
	HW_PARAMS_m12			HW_params;

	pthread_mutex_t_m12		TZ_mutex;
	pthread_mutex_t_m12		SHA_mutex;
	pthread_mutex_t_m12		AES_mutex;
	pthread_mutex_t_m12		CRC_mutex;
	pthread_mutex_t_m12		UTF8_mutex;
	pthread_mutex_t_m12		CMP_mutex;
	pthread_mutex_t_m12		NET_mutex;
	pthread_mutex_t_m12		HW_mutex;
	
	#ifdef WINDOWS_m12
	HINSTANCE			hNTdll;  // handle to ntdll dylib (used by WN_nap(), only loaded if used)
	#endif
} GLOBAL_TABLES_m12;

// Globals List (thread local storage)
#define globals_m12	(G_globals_pointer_m12())  // thread local globals accessed with process id => use "globals_m12" like a pointer



//**********************************************************************************//
//********************************  MED Structures  ********************************//
//**********************************************************************************//


// Generally Useful Structures
typedef union {
	si1	ext[8];
	ui4	code;
} EXT_CODE_m12;

// Universal Header Structure
typedef struct {
	// start robust mode region
	ui4	header_CRC;     // CRC of the universal header after this field
	ui4     body_CRC;       // CRC of the entire file after the universal header
	union {  // "segment_end_time" used in code when that is it's meaning, for clarity
		si8	file_end_time;
		si8	segment_end_time;
	};
	si8	number_of_entries;
	ui4	maximum_entry_size;
	// end robust mode region
	si4     segment_number;
	union {
		struct {
			si1     type_string[TYPE_BYTES_m12];
			ui1     MED_version_major;
			ui1     MED_version_minor;
			ui1     byte_order_code;
		};
		struct {
			ui4     type_code;
			si1	type_string_terminal_zero;  // not used - here for clarity
		};
	};
	si8	session_start_time;
	union {  // "segment_start_time" used in code when that is it's meaning, for clarity
		si8	file_start_time;
		si8	segment_start_time;
	};
	si1	session_name[BASE_FILE_NAME_BYTES_m12]; // utf8[63], base name only, no extension
	si1     channel_name[BASE_FILE_NAME_BYTES_m12]; // utf8[63], base name only, no extension
	si1	anonymized_subject_ID[UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_BYTES_m12]; // utf8[63]
	ui8	session_UID;
	ui8     channel_UID;
	ui8     segment_UID;
	ui8	file_UID;
	ui8	provenance_UID;
	ui1	level_1_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m12];
	ui1     level_2_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m12];
	ui1	level_3_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m12];
	ui1	protected_region[UNIVERSAL_HEADER_PROTECTED_REGION_BYTES_m12];
	ui1	discretionary_region[UNIVERSAL_HEADER_DISCRETIONARY_REGION_BYTES_m12];
} UNIVERSAL_HEADER_m12;

// Metadata Structures
typedef struct {
	si1     level_1_password_hint[PASSWORD_HINT_BYTES_m12];
	si1     level_2_password_hint[PASSWORD_HINT_BYTES_m12];
	si1     section_2_encryption_level;
	si1     section_3_encryption_level;
	si1     time_series_data_encryption_level;
	ui1     protected_region[METADATA_SECTION_1_PROTECTED_REGION_BYTES_m12];
	ui1     discretionary_region[METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m12];
} METADATA_SECTION_1_m12;

typedef struct {
	// channel type independent fields
	si1     session_description[METADATA_SESSION_DESCRIPTION_BYTES_m12];            // utf8[511]
	si1     channel_description[METADATA_CHANNEL_DESCRIPTION_BYTES_m12];            // utf8[255]
	si1     segment_description[METADATA_SEGMENT_DESCRIPTION_BYTES_m12];            // utf8[255]
	si1     equipment_description[METADATA_EQUIPMENT_DESCRIPTION_BYTES_m12];        // utf8[510]
	si4     acquisition_channel_number;
	// channel type specific fields
	si1     reference_description[TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m12];        // utf8[255]
	sf8     sampling_frequency;
	sf8     low_frequency_filter_setting;
	sf8     high_frequency_filter_setting;
	sf8     notch_filter_frequency_setting;
	sf8     AC_line_frequency;
	sf8     amplitude_units_conversion_factor;
	si1     amplitude_units_description[TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m12];  // utf8[31]
	sf8     time_base_units_conversion_factor;
	si1     time_base_units_description[TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m12];  // utf8[31]
	si8     absolute_start_sample_number;
	si8     number_of_samples;
	si8	number_of_blocks;
	si8     maximum_block_bytes;
	ui4     maximum_block_samples;
	ui4     maximum_block_keysample_bytes;
	sf8     maximum_block_duration;
	si8     number_of_discontinuities;
	si8     maximum_contiguous_blocks;
	si8     maximum_contiguous_block_bytes;
	si8     maximum_contiguous_samples;
	ui1     protected_region[TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m12];
	ui1     discretionary_region[TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m12];
} TIME_SERIES_METADATA_SECTION_2_m12;

typedef struct {
	// type-independent fields
	si1     session_description[METADATA_SESSION_DESCRIPTION_BYTES_m12];			// utf8[511]
	si1     channel_description[METADATA_CHANNEL_DESCRIPTION_BYTES_m12];			// utf8[511]
	si1     segment_description[METADATA_SEGMENT_DESCRIPTION_BYTES_m12];			// utf8[511]
	si1     equipment_description[METADATA_EQUIPMENT_DESCRIPTION_BYTES_m12];        	// utf8[510]
	si4     acquisition_channel_number;
	// type-specific fields
	sf8     time_base_units_conversion_factor;
	si1     time_base_units_description[VIDEO_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m12];	// utf8[31]
	si8     absolute_start_frame_number;
	si8     number_of_frames;
	sf8     frame_rate;
	si8     number_of_clips;
	si8     maximum_clip_bytes;
	ui4     maximum_clip_frames;
	si4	number_of_video_files;
	sf8     maximum_clip_duration;
	si8     number_of_discontinuities;
	si8	maximum_contiguous_clips;
	si8	maximum_contiguous_clip_bytes;
	si8	maximum_contiguous_frames;
	ui4     horizontal_pixels;
	ui4     vertical_pixels;
	si1     video_format[VIDEO_METADATA_VIDEO_FORMAT_BYTES_m12];                		// utf8[31]
	ui1     protected_region[VIDEO_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m12];
	ui1     discretionary_region[VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m12];
} VIDEO_METADATA_SECTION_2_m12;

// All metadata section substructures are the same sizes
typedef union {
		ui1					section_2[METADATA_SECTION_2_BYTES_m12];
		TIME_SERIES_METADATA_SECTION_2_m12	time_series_section_2;
		VIDEO_METADATA_SECTION_2_m12		video_section_2;
} METADATA_SECTION_2_m12;

typedef struct {
	si8     recording_time_offset;
	DAYLIGHT_TIME_CHANGE_CODE_m12   daylight_time_start_code;                       // si1[8] / si8
	DAYLIGHT_TIME_CHANGE_CODE_m12   daylight_time_end_code;                         // si1[8] / si8
	si1     standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m12];                  // ascii[8]
	si1     standard_timezone_string[TIMEZONE_STRING_BYTES_m12];                    // ascii[31]
	si1     daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m12];                  // ascii[8]
	si1     daylight_timezone_string[TIMEZONE_STRING_BYTES_m12];                    // ascii[31]
	si1     subject_name_1[METADATA_SUBJECT_NAME_BYTES_m12];                        // utf8[31]
	si1     subject_name_2[METADATA_SUBJECT_NAME_BYTES_m12];                        // utf8[31]
	si1     subject_name_3[METADATA_SUBJECT_NAME_BYTES_m12];                        // utf8[31]
	si1     subject_ID[METADATA_SUBJECT_ID_BYTES_m12];                              // utf8[31]
	si1     recording_country[METADATA_RECORDING_LOCATION_BYTES_m12];               // utf8[63]
	si1     recording_territory[METADATA_RECORDING_LOCATION_BYTES_m12];             // utf8[63]
	si1     recording_locality[METADATA_RECORDING_LOCATION_BYTES_m12];              // utf8[63]
	si1     recording_institution[METADATA_RECORDING_LOCATION_BYTES_m12];           // utf8[63]
	si1     geotag_format[METADATA_GEOTAG_FORMAT_BYTES_m12];                        // ascii[31]
	si1     geotag_data[METADATA_GEOTAG_DATA_BYTES_m12];                            // ascii[1023]
	si4     standard_UTC_offset;
	ui1     protected_region[METADATA_SECTION_3_PROTECTED_REGION_BYTES_m12];
	ui1     discretionary_region[METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m12];
} METADATA_SECTION_3_m12;

#ifdef __cplusplus  // c++ does not accept anonymous structures
typedef struct {
	METADATA_SECTION_1_m12		section_1;
	union {
		ui1					section_2[METADATA_SECTION_2_BYTES_m12];
		TIME_SERIES_METADATA_SECTION_2_m12	time_series_section_2;
		VIDEO_METADATA_SECTION_2_m12		video_section_2;
	};
	METADATA_SECTION_3_m12		section_3;
} METADATA_m12;
#else
typedef struct {
	METADATA_SECTION_1_m12		section_1;
	METADATA_SECTION_2_m12;
	METADATA_SECTION_3_m12		section_3;
} METADATA_m12;
#endif

// Record Structures
typedef struct RECORD_HEADER_m12 {  // struct name for medrec_m12.h interdependency
	ui4	record_CRC;
	ui4     total_record_bytes;  // header + body bytes
	si8     start_time;  // for record types with a start_time (records written when all info known)
	union {  // anonymous union
		struct {
			si1     type_string[TYPE_BYTES_m12];
			ui1     version_major;
			ui1     version_minor;
			si1     encryption_level;
		};
		struct {
			ui4     type_code;
			si1	type_string_terminal_zero;  // not used - here for clarity
		};
	};
} RECORD_HEADER_m12;

typedef struct {
	si8	file_offset;  // never negative: the record indices are not used to indicate discontinuities
	si8     start_time;  // for record types with a start_time (records written when all info known)
	union {  // anonymous union
		struct {
			si1     type_string[TYPE_BYTES_m12];
			ui1     version_major;
			ui1     version_minor;
			si1     encryption_level;
		};
		struct {
			ui4     type_code;
			si1	type_string_terminal_zero;  // not used - there for clarity
		};
	};
} RECORD_INDEX_m12;

// Time Series Indices Structures
typedef struct {
	si8	file_offset;  // negative values indicate discontinuity
	si8	start_time;
	si8     start_sample_number;
} TIME_SERIES_INDEX_m12;

// Video Indices Structures
typedef struct {
	si8     file_offset;  // negative values indicate discontinuity
	si8	start_time;
	ui4     start_frame_number;
	ui4     video_file_number;
} VIDEO_INDEX_m12;

typedef struct {
	si8     file_offset;  // negative values indicate discontinuity (in time series & video indices)
	si8	start_time;
	ui1     pad[8];
} GENERIC_INDEX_m12;

// All index structures are the same size, and have the same first two fields (hence GENERIC_INDEX_m12)
typedef struct {
	union {
		RECORD_INDEX_m12	record_index;
		TIME_SERIES_INDEX_m12	time_series_index;
		VIDEO_INDEX_m12		video_index;
		GENERIC_INDEX_m12	generic_index;
	};
} INDEX_m12;

// File Processing Structures
typedef struct {
	TERN_m12        close_file;
	TERN_m12        flush_after_write;
	TERN_m12        update_universal_header;	// when writing
	TERN_m12        leave_decrypted;		// if encrypted during write, return from write function decrypted
	TERN_m12        free_password_data;		// when freeing FPS
	TERN_m12        free_CMP_processing_struct;	// when freeing FPS
	ui4             lock_mode;
	ui4             open_mode;
	TERN_m12	memory_map;  // full file allocated; read regions stored in bitmap; no re-reads; efficient, but memory expensive
} FPS_DIRECTIVES_m12;

// Parameters contain "mechanics" of FPS (mostly used internally by library functions)
typedef struct {
	pthread_mutex_t_m12			mutex;
	si8					last_access_time;	// uutc of last read into or write from this structure to the file system (update by read_file_m12 & write_file_m12)
	TERN_m12				full_file_read;		// full file has been read in / decrypted
	si8					raw_data_bytes;		// bytes in raw data array,
	ui1					*raw_data;		// universal header followed by data (in standard read - just region requested, in full file & mem map - matches media)
	PASSWORD_DATA_m12			*password_data;
	struct CMP_PROCESSING_STRUCT_m12	*cps;			// for time series data FPSs
	// file status
	si4			fd;	// file descriptor
	FILE			*fp;	// file pointer
	si8			fpos;	// current file pointer position (from file start)
	si8			flen;	// file length
	// memory mapping
	ui4			mmap_block_bytes;  // bytes per bit in block bitmap
	ui4			mmap_number_of_blocks;  // file system block in file == number of bits in bitmap
	ui8			*mmap_block_bitmap;  // each bit represents block_bytes bytes;  NULL if not memory mapping
} FPS_PARAMETERS_m12;

typedef struct {
	void					*parent;  // parent structure, NULL if created alone
	si1					full_file_name[FULL_FILE_NAME_BYTES_m12];  // full path from root including extension
	UNIVERSAL_HEADER_m12			*universal_header;  // points to base of raw_data array
	FPS_DIRECTIVES_m12	        	directives;
	FPS_PARAMETERS_m12	        	parameters;
	union {					// the MED file types
						// these are set to point to current data (just read, or to write)
		METADATA_m12			*metadata;
		RECORD_INDEX_m12		*record_indices;
		ui1				*record_data;
		TIME_SERIES_INDEX_m12		*time_series_indices;
		ui1				*time_series_data;  // compressed data (not modified), CPS block header is modifiable pointer within this array
		VIDEO_INDEX_m12			*video_indices;
		void				*video_data;  // place holder - not yet implemented  (universal_header is NULL as video data is stored in native video format)
		ui1				*data_pointers;  // generic name for all of the above (dissociable from raw data array / universal header, if needed)
	};
	si8					number_of_items;  // items in current read/write, not necessarily the whole file
} FILE_PROCESSING_STRUCT_m12;

// Session, Segmented Session Records, Channel, & Segment Structures
typedef struct {
	union {  // anonymous union
		struct {
			si1     	type_string[TYPE_BYTES_m12];
			TERN_m12	en_bloc_allocation;
			ui1     	pad[2];  // force to 8-byte alignment
		};
		struct {
			ui4    		type_code;
			si1		type_string_terminal_zero;  // not used - there for clarity
		};
	};
	void	*parent;  // parent structure, NULL for session or if created alone
	ui8	flags;
	si8	last_access_time;  // uutc of last use of this structure by the calling program (updated by read & open functions)
} LEVEL_HEADER_m12;


//**********************************************************************************//
//****************************  Non-standard Structures  ***************************//
//**********************************************************************************//

// required compiler option (gcc, clang):  -fms-extensions
// suppress warnings:  -Wno-microsoft-anon-tag
#ifdef __cplusplus
typedef struct {
	struct {  // this replaces RECORD_HEADER_m12 for C++
		ui4	record_CRC;
		ui4     total_record_bytes;  // header + body bytes
		si8     start_time;
		union {  // anonymous union
			struct {
				si1     type_string[TYPE_BYTES_m12];
				ui1     version_major;
				ui1     version_minor;
				si1     encryption_level;
			};
			struct {
				ui4     type_code;
				si1	type_string_terminal_zero;  // not used - here for clarity
			};
		};
	};
	struct {  // this replaces REC_Sgmt_v10_m12 for C++
		si8     end_time;
		union {
			si8     start_sample_number;	// session-relative (global indexing)
			si8     start_frame_number;	// session-relative (global indexing)
		};
		union {
			si8     end_sample_number;	// session-relative (global indexing)
			si8     end_frame_number;	// session-relative (global indexing)
		};
		ui8     segment_UID;
		si4     segment_number;
		si4     acquisition_channel_number;  // REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_ALL_CHANNELS_m12 in session level records
		union {
			sf8     sampling_frequency;  // channel sampling frequency (REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m12 in session level records, if sampling frequencies vary across time series channels)
			sf8     frame_rate;  	     // channel frame rate (REC_Sgmt_v10_FRAME_RATE_VARIABLE_m12 in session level records, if frame rates vary across video channels)
		};
	};
} Sgmt_RECORD_m12;
#else
typedef struct {
	union {
		RECORD_HEADER_m12	header;  // in case just want the record header
		RECORD_HEADER_m12;	// anonoymous RECORD_HEADER_m12
	};
	union {
		REC_Sgmt_v10_m12	body;  // in case just want the record body
		REC_Sgmt_v10_m12;	// anonoymous REC_Sgmt_v10_m12
	};
} Sgmt_RECORD_m12;
#endif
// NOTE: construction of Sgmt_RECORD_m12 in this way allows direct reading of Sgmt record headers
// & bodies into this structure (excluding the segment description).

#ifdef __cplusplus
typedef struct {
	struct {  // this struct replaces LEVEL_HEADER_m12 for C++
		union {  // anonymous union
			struct {
				si1     	type_string[TYPE_BYTES_m12];
				TERN_m12	en_bloc_allocation;
				ui1     	pad[2];  // force to 8-byte alignment
			};
			struct {
				ui4     	type_code;
				si1		type_string_terminal_zero;  // not used - there for clarity
			};
		};
		void	*parent;  // parent structure, NULL for session or if created alone
		ui8	flags;
		si8	last_access_time;  // uutc of last use of this structure by the calling program (updated by read & open functions)
	};
	FILE_PROCESSING_STRUCT_m12	*metadata_fps;  // also used as prototype
	union {
		FILE_PROCESSING_STRUCT_m12	*time_series_data_fps;
		FILE_PROCESSING_STRUCT_m12	*video_data_fps;
	};
	union {
		FILE_PROCESSING_STRUCT_m12	*time_series_indices_fps;
		FILE_PROCESSING_STRUCT_m12	*video_indices_fps;
	};
	FILE_PROCESSING_STRUCT_m12	*record_data_fps;
	FILE_PROCESSING_STRUCT_m12	*record_indices_fps;
	si1                             path[FULL_FILE_NAME_BYTES_m12]; // full path to segment directory (including segment directory itself)
	si1                             name[SEGMENT_BASE_FILE_NAME_BYTES_m12];  // stored here, no segment_name field in universal header
	TIME_SLICE_m12			time_slice;
	si8				number_of_contigua;
	CONTIGUON_m12			*contigua;
} SEGMENT_m12;
#else
typedef struct {
	union {
		LEVEL_HEADER_m12	header;  // in case just want the level header
		LEVEL_HEADER_m12;	// anonoymous LEVEL_HEADER_m12
	};
	FILE_PROCESSING_STRUCT_m12	*metadata_fps;  // also used as prototype
	union {
		FILE_PROCESSING_STRUCT_m12	*time_series_data_fps;
		FILE_PROCESSING_STRUCT_m12	*video_data_fps;
	};
	union  {
		FILE_PROCESSING_STRUCT_m12	*time_series_indices_fps;
		FILE_PROCESSING_STRUCT_m12	*video_indices_fps;
	};
	FILE_PROCESSING_STRUCT_m12	*record_data_fps;
	FILE_PROCESSING_STRUCT_m12	*record_indices_fps;
	si1                             path[FULL_FILE_NAME_BYTES_m12]; // full path to segment directory (including segment directory itself)
	si1                             name[SEGMENT_BASE_FILE_NAME_BYTES_m12];  // stored here, no segment_name field in universal header
	TIME_SLICE_m12			time_slice;
	si8				number_of_contigua;
	CONTIGUON_m12			*contigua;
} SEGMENT_m12;
#endif

#ifdef __cplusplus
typedef struct CHANNEL_m12 {
	struct {  // this struct replaces LEVEL_HEADER_m12 for C++
		union {  // anonymous union
			struct {
				si1     	type_string[TYPE_BYTES_m12];
				TERN_m12	en_bloc_allocation;
				ui1     	pad[2];  // force to 8-byte alignment
			};
			struct {
				ui4     	type_code;
				si1		type_string_terminal_zero;  // not used - there for clarity
			};
		};
		void	*parent;  // parent structure, NULL for session or if created alone
		ui8	flags;
		si8	last_access_time;  // uutc of last use of this structure by the calling program (updated by read & open functions)
	};
	FILE_PROCESSING_STRUCT_m12	*metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	FILE_PROCESSING_STRUCT_m12	*record_data_fps;
	FILE_PROCESSING_STRUCT_m12	*record_indices_fps;
	SEGMENT_m12			**segments;
	Sgmt_RECORD_m12			*Sgmt_records;
	si1			        path[FULL_FILE_NAME_BYTES_m12]; // full path to channel directory (including channel directory itself)
	si1                             name[BASE_FILE_NAME_BYTES_m12];
	TIME_SLICE_m12			time_slice;
	si8				number_of_contigua;
	CONTIGUON_m12			*contigua;
} CHANNEL_m12;
#else
typedef struct CHANNEL_m12 {
	union {
		LEVEL_HEADER_m12	header;  // in case just want the level header
		LEVEL_HEADER_m12;	// anonoymous LEVEL_HEADER_m12
	};
	FILE_PROCESSING_STRUCT_m12	*metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	FILE_PROCESSING_STRUCT_m12	*record_data_fps;
	FILE_PROCESSING_STRUCT_m12	*record_indices_fps;
	SEGMENT_m12			**segments;
	Sgmt_RECORD_m12			*Sgmt_records;
	si1			        path[FULL_FILE_NAME_BYTES_m12]; // full path to channel directory (including channel directory itself)
	si1                             name[BASE_FILE_NAME_BYTES_m12];
	TIME_SLICE_m12			time_slice;
	si8				number_of_contigua;
	CONTIGUON_m12			*contigua;
} CHANNEL_m12;
#endif

#ifdef __cplusplus
typedef struct {
	struct LEVEL_HEADER_m12 {  // this struct replaces LEVEL_HEADER_m12 in C++
		union {  // anonymous union
			struct {
				si1     	type_string[TYPE_BYTES_m12];
				TERN_m12	en_bloc_allocation;
				ui1     	pad[3];  // force to 8-byte alignment
			};
			struct {
				ui4     	type_code;
				si1		type_string_terminal_zero;  // not used - there for clarity
			};
		};
		void	*parent;  // parent structure, NULL for session or if created alone
		ui8	flags;
		si8	last_access_time;  // uutc of last use of this structure by the calling program (updated by read & open functions)
	};
	FILE_PROCESSING_STRUCT_m12	**record_data_fps;
	FILE_PROCESSING_STRUCT_m12	**record_indices_fps;
	si1			        path[FULL_FILE_NAME_BYTES_m12];		// full path to segmented session records directory (including directory itself)
	si1                             name[BASE_FILE_NAME_BYTES_m12];		// session name, duplcated in globals
	TIME_SLICE_m12			time_slice;
} SEGMENTED_SESS_RECS_m12;
#else
typedef struct {
	union {
		LEVEL_HEADER_m12	header;  // in case just want the level header
		LEVEL_HEADER_m12;	// anonoymous LEVEL_HEADER_m12
	};
	FILE_PROCESSING_STRUCT_m12	**record_data_fps;
	FILE_PROCESSING_STRUCT_m12	**record_indices_fps;
	si1			        path[FULL_FILE_NAME_BYTES_m12];		// full path to segmented session records directory (including directory itself)
	si1                             name[BASE_FILE_NAME_BYTES_m12];		// session name, duplicated in globals
	TIME_SLICE_m12			time_slice;
} SEGMENTED_SESS_RECS_m12;
#endif

#ifdef __cplusplus
typedef struct {
	struct {  // this struct replaces LEVEL_HEADER_m12 in C++
		union {  // anonymous union
			struct {
				si1		type_string[TYPE_BYTES_m12];
				TERN_m12	en_bloc_allocation;
				ui1     	pad[2];  // force to 8-byte alignment
			};
			struct {
				ui4     	type_code;
				si1		type_string_terminal_zero;  // not used - there for clarity
			};
		};
		void	*parent;  // parent structure, NULL for session or if created alone
		ui8	flags;
		si8	last_access_time;  // uutc of last use of this structure by the calling program (updated by read & open functions)
	};
	FILE_PROCESSING_STRUCT_m12	*time_series_metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	FILE_PROCESSING_STRUCT_m12	*video_metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	Sgmt_RECORD_m12			*Sgmt_records;
	si4			        number_of_time_series_channels;
	CHANNEL_m12			**time_series_channels;
	si4			        number_of_video_channels;
	CHANNEL_m12			**video_channels;
	FILE_PROCESSING_STRUCT_m12	*record_data_fps;
	FILE_PROCESSING_STRUCT_m12	*record_indices_fps;
	SEGMENTED_SESS_RECS_m12		*segmented_sess_recs;
	si1			        path[FULL_FILE_NAME_BYTES_m12];		// full path to session directory (including directory itself)
	si1                             *name;					// points to global uh_name (universal header), if known otherwise to fs_name (from file system)
	TIME_SLICE_m12			time_slice;
	si8				number_of_contigua;
	CONTIGUON_m12			*contigua;
} SESSION_m12;
#else
typedef struct {
	union {
		LEVEL_HEADER_m12	header;  // in case just want the level header
		LEVEL_HEADER_m12;	// anonoymous LEVEL_HEADER_m12
	};
	FILE_PROCESSING_STRUCT_m12	*time_series_metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	FILE_PROCESSING_STRUCT_m12	*video_metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	Sgmt_RECORD_m12			*Sgmt_records;
	si4			        number_of_time_series_channels;
	CHANNEL_m12			**time_series_channels;
	si4			        number_of_video_channels;
	CHANNEL_m12			**video_channels;
	FILE_PROCESSING_STRUCT_m12	*record_data_fps;
	FILE_PROCESSING_STRUCT_m12	*record_indices_fps;
	SEGMENTED_SESS_RECS_m12		*segmented_sess_recs;
	si1			        path[FULL_FILE_NAME_BYTES_m12];		// full path to session directory (including session directory itself)
	si1                             *name;					// points to global uh_name (universal header), if known otherwise to fs_name (from file system)
	TIME_SLICE_m12			time_slice;
	si8				number_of_contigua;
	CONTIGUON_m12			*contigua;
} SESSION_m12;
#endif
// NOTE: placement of LEVEL_HEADER_m12 in SESSION_m12, SEGMENTED_SESS_RECS_m12, CHANNEL_m12, & SEGMENT_m12 structures allows passing of LEVEL_HEADER_m12 pointer to functions,
// and based on its content, functions can cast pointer to specific level structures.
// e.g:	if (level_header->type_code == LH_SESSION_m12)
//		sess = (SESSION_m12 *) level_header;


// Miscellaneous structures that depend on above
typedef struct {
	si1			MED_dir[FULL_FILE_NAME_BYTES_m12];
	ui8			flags;
	LEVEL_HEADER_m12	*MED_struct;  // CHANNEL_m12 or SEGMENT_m12 pointer (used to pass & return)
	TIME_SLICE_m12		*slice;
	si1			*password;
} READ_MED_THREAD_INFO_m12;

typedef struct {
	si4		acq_num;
	CHANNEL_m12	*chan;
} ACQ_NUM_SORT_m12;



//**********************************************************************************//
//****************************  GENERAL (G) MED Functions  *****************************//
//**********************************************************************************//


// Prototypes
ui4 		G_add_level_extension_m12(si1 *directory_name);
TERN_m12	G_all_zeros_m12(ui1 *bytes, si4 field_length);
CHANNEL_m12	*G_allocate_channel_m12(CHANNEL_m12 *chan, FILE_PROCESSING_STRUCT_m12 *proto_fps, si1 *enclosing_path, si1 *chan_name, ui4 type_code, si4 n_segs, TERN_m12 chan_recs, TERN_m12 seg_recs);
SEGMENT_m12	*G_allocate_segment_m12(SEGMENT_m12 *seg, FILE_PROCESSING_STRUCT_m12 *proto_fps, si1* enclosing_path, si1 *chan_name, ui4 type_code, si4 seg_num, TERN_m12 seg_recs);
SESSION_m12	*G_allocate_session_m12(FILE_PROCESSING_STRUCT_m12 *proto_fps, si1 *enclosing_path, si1 *sess_name, si4 n_ts_chans, si4 n_vid_chans, si4 n_segs, si1 **chan_names, si1 **vid_chan_names, TERN_m12 sess_recs, TERN_m12 segmented_sess_recs, TERN_m12 chan_recs, TERN_m12 seg_recs);
TERN_m12	G_allocated_en_bloc_m12(LEVEL_HEADER_m12 *level_header);
void     	G_apply_recording_time_offset_m12(si8 *time);
si1		*G_behavior_string_m12(ui4 behavior, si1 *behavior_string);
si8		G_build_contigua_m12(LEVEL_HEADER_m12 *level_header);
Sgmt_RECORD_m12	*G_build_Sgmt_records_array_m12(FILE_PROCESSING_STRUCT_m12 *ri_fps, FILE_PROCESSING_STRUCT_m12 *rd_fps, CHANNEL_m12 *chan);
si8		G_bytes_for_items_m12(FILE_PROCESSING_STRUCT_m12 *fps, si8 *number_of_items, si8 read_file_offset);
void    	G_calculate_indices_CRCs_m12(FILE_PROCESSING_STRUCT_m12 *fps);
void            G_calculate_metadata_CRC_m12(FILE_PROCESSING_STRUCT_m12 *fps);
void            G_calculate_record_data_CRCs_m12(FILE_PROCESSING_STRUCT_m12 *fps);
void            G_calculate_time_series_data_CRCs_m12(FILE_PROCESSING_STRUCT_m12 *fps);
CHANNEL_m12	*G_change_reference_channel_m12(SESSION_m12 *sess, CHANNEL_m12 *channel, si1 *channel_name, si1 channel_type);
ui4             G_channel_type_from_path_m12(si1 *path);
TERN_m12	G_check_char_type_m12(void);
TERN_m12	G_check_file_list_m12(si1 **file_list, si4 n_files);
TERN_m12	G_check_file_system_m12(si1 *file_system_path, si4 is_cloud, ...);  // varargs: si1 *cloud_directory, si1 *cloud_service_name, si1 *cloud_utilities_directory
TERN_m12        G_check_password_m12(si1 *password);
si4		G_check_segment_map_m12(TIME_SLICE_m12 *m12, SESSION_m12 *sess);
void		G_clear_terminal_m12(void);
si4		G_compare_acq_nums_m12(const void *a, const void *b);
si4    		G_compare_record_index_times(const void *a, const void *b);
void		G_condition_timezone_info_m12(TIMEZONE_INFO_m12 *tz_info);
void		G_condition_time_slice_m12(TIME_SLICE_m12 *slice);
TERN_m12	G_correct_universal_header_m12(FILE_PROCESSING_STRUCT_m12 *fps);
si8		G_current_uutc_m12(void);
si4		G_days_in_month_m12(si4 month, si4 year);
TERN_m12        G_decrypt_metadata_m12(FILE_PROCESSING_STRUCT_m12 *fps);
TERN_m12        G_decrypt_record_data_m12(FILE_PROCESSING_STRUCT_m12 *fps, ...);  // varargs (fps == NULL): RECORD_HEADER_m12 *rh, si8 number_of_records  (used to decrypt Sgmt_records arrays)
TERN_m12        G_decrypt_time_series_data_m12(FILE_PROCESSING_STRUCT_m12 *fps);
si4             G_DST_offset_m12(si8 uutc);
TERN_m12	G_empty_string_m12(si1 *string);
TERN_m12	G_en_bloc_allocation_m12(LEVEL_HEADER_m12 *level_header);
TERN_m12        G_encrypt_metadata_m12(FILE_PROCESSING_STRUCT_m12 *fps);
TERN_m12	G_encrypt_record_data_m12(FILE_PROCESSING_STRUCT_m12 *fps);
TERN_m12        G_encrypt_time_series_data_m12(FILE_PROCESSING_STRUCT_m12 *fps);
TERN_m12	G_enter_ascii_password_m12(si1 *password, si1 *prompt, TERN_m12 confirm_no_entry, sf8 timeout_secs, TERN_m12 create_password);
void            G_error_message_m12(si1 *fmt, ...);
void		G_error_string_m12(void);
si1             G_exists_m12(si1 *path);
void            G_extract_path_parts_m12(si1 *full_file_name, si1 *path, si1 *name, si1 *extension);
void            G_extract_terminal_password_bytes_m12(si1 *password, si1 *password_bytes);
si8		G_file_length_m12(FILE *fp, si1 *path);
FILE_TIMES_m12	*G_file_times_m12(FILE *fp, si1 *path, FILE_TIMES_m12 *ft, TERN_m12 set_time);
void            G_fill_empty_password_bytes_m12(si1 *password_bytes);
CONTIGUON_m12	*G_find_discontinuities_m12(LEVEL_HEADER_m12 *level_header, si8 *num_contigua);
si8		G_find_index_m12(SEGMENT_m12 *seg, si8 target, ui4 mode);
si1		*G_find_timezone_acronym_m12(si1 *timezone_acronym, si4 standard_UTC_offset, si4 DST_offset);
si1		*G_find_metadata_file_m12(si1 *path, si1 *md_path);
si8		G_find_record_index_m12(FILE_PROCESSING_STRUCT_m12 *record_indices_fps, si8 target_time, ui4 mode, si8 low_idx);
si8     	G_frame_number_for_uutc_m12(LEVEL_HEADER_m12 *level_header, si8 target_uutc, ui4 mode, ...);  // varargs: si8 ref_frame_number, si8 ref_uutc, sf8 frame_rate
TERN_m12	G_free_channel_m12(CHANNEL_m12* channel, TERN_m12 free_channel_structure);
void		G_free_global_tables_m12(void);
void            G_free_globals_m12(TERN_m12 cleanup_for_exit);
TERN_m12	G_free_segment_m12(SEGMENT_m12 *segment, TERN_m12 free_segment_structure);
void		G_free_segmented_sess_recs_m12(SEGMENTED_SESS_RECS_m12 *ssr, TERN_m12 free_segmented_sess_rec_structure);
void            G_free_session_m12(SESSION_m12 *session, TERN_m12 free_session_structure);
TERN_m12	G_frequencies_vary_m12(SESSION_m12 *sess);
si1		**G_generate_file_list_m12(si1 **file_list, si4 *n_files, si1 *enclosing_directory, si1 *name, si1 *extension, ui4 flags);
si1		**G_generate_numbered_names_m12(si1 **names, si1 *prefix, si4 number_of_names);
TERN_m12	G_generate_password_data_m12(FILE_PROCESSING_STRUCT_m12* fps, si1* L1_pw, si1* L2_pw, si1* L3_pw, si1* L1_pw_hint, si1* L2_pw_hint);
si8             G_generate_recording_time_offset_m12(si8 recording_start_time_uutc);
si1		*G_generate_segment_name_m12(FILE_PROCESSING_STRUCT_m12 *fps, si1 *segment_name);
ui8             G_generate_UID_m12(ui8 *uid);
CHANNEL_m12	*G_get_active_channel_m12(SESSION_m12 *sess, si1 channel_type);
ui4		G_get_level_m12(si1 *full_file_name, ui4 *input_type_code);
LOCATION_INFO_m12	*G_get_location_info_m12(LOCATION_INFO_m12 *loc_info, TERN_m12 set_timezone_globals, TERN_m12 prompt);
si4		G_get_search_mode_m12(TIME_SLICE_m12 *slice);
si4		G_get_segment_index_m12(si4 segment_number);
si4             G_get_segment_range_m12(LEVEL_HEADER_m12 *level_header, TIME_SLICE_m12 *slice);
ui4		*G_get_segment_video_start_frames_m12(FILE_PROCESSING_STRUCT_m12 *video_indices_fps, ui4 *number_of_video_files);
si1		*G_get_session_directory_m12(si1 *session_directory, si1 *MED_file_name, FILE_PROCESSING_STRUCT_m12 *MED_fps);
TERN_m12	G_get_terminal_entry_m12(si1 *prompt, si1 type, void *buffer, void *default_input, TERN_m12 required, TERN_m12 validate);
pid_t_m12	G_globals_ID_m12(pid_t_m12 old_id);
GLOBALS_m12	*G_globals_pointer_m12(void);
TERN_m12	G_include_record_m12(ui4 type_code, si4 *record_filters);
TERN_m12	G_initialize_global_tables_m12(TERN_m12 initialize_all_tables);
TERN_m12	G_initialize_globals_m12(TERN_m12 initialize_all_tables);
TERN_m12	G_initialize_medlib_m12(TERN_m12 check_structure_alignments, TERN_m12 initialize_all_tables);
void            G_initialize_metadata_m12(FILE_PROCESSING_STRUCT_m12 *fps, TERN_m12 initialize_for_update);
TIME_SLICE_m12	*G_initialize_time_slice_m12(TIME_SLICE_m12 *slice);
TERN_m12	G_initialize_timezone_tables_m12(void);
void		G_initialize_universal_header_m12(FILE_PROCESSING_STRUCT_m12 *fps, ui4 type_code, TERN_m12 generate_file_UID, TERN_m12 originating_file);
si8		G_items_for_bytes_m12(FILE_PROCESSING_STRUCT_m12 *fps, si8 *number_of_bytes);
ui4		G_level_from_base_name_m12(si1 *path, si1 *level_path);
void		G_lh_set_directives_m12(si1 *full_file_name, ui8 lh_flags, TERN_m12 *mmap_flag, TERN_m12 *close_flag, si8 *number_of_items);
ui4             G_MED_path_components_m12(si1 *path, si1 *MED_dir, si1* MED_name);
si1		*G_MED_type_string_from_code_m12(ui4 code);
ui4             G_MED_type_code_from_string_m12(si1 *string);
TERN_m12        G_merge_metadata_m12(FILE_PROCESSING_STRUCT_m12 *md_fps_1, FILE_PROCESSING_STRUCT_m12 *md_fps_2, FILE_PROCESSING_STRUCT_m12 *merged_md_fps);
TERN_m12        G_merge_universal_headers_m12(FILE_PROCESSING_STRUCT_m12 *fps_1, FILE_PROCESSING_STRUCT_m12 *fps_2, FILE_PROCESSING_STRUCT_m12 *merged_fps);
void    	G_message_m12(si1 *fmt, ...);
void     	G_nap_m12(si1 *nap_str);
si1		*G_numerical_fixed_width_string_m12(si1 *string, si4 string_bytes, si4 number);
CHANNEL_m12	*G_open_channel_m12(CHANNEL_m12 *chan, TIME_SLICE_m12 *slice, si1 *chan_path, ui8 flags, si1 *password);
CHANNEL_m12	*G_open_channel_nt_m12(CHANNEL_m12 *chan, TIME_SLICE_m12 *slice, si1 *channel_path, ui8 flags, si1 *password);  // "nt" == not threaded
pthread_rval_m12	G_open_channel_thread_m12(void *ptr);
SEGMENT_m12	*G_open_segment_m12(SEGMENT_m12 *seg, TIME_SLICE_m12 *slice, si1 *segment_path, ui8 flags, si1 *password);
pthread_rval_m12	G_open_segment_thread_m12(void *ptr);
SESSION_m12	*G_open_session_m12(SESSION_m12 *sess, TIME_SLICE_m12 *slice, void *file_list, si4 list_len, ui8 flags, si1 *password);
SESSION_m12	*G_open_session_nt_m12(SESSION_m12 *sess, TIME_SLICE_m12 *slice, void *file_list, si4 list_len, ui8 flags, si1 *password);  // "nt" == not threaded
si8             G_pad_m12(ui1 *buffer, si8 content_len, ui4 alignment);
TERN_m12	G_path_from_root_m12(si1 *path, si1 *root_path);
void            G_pop_behavior_m12(void);
TERN_m12	G_process_password_data_m12(FILE_PROCESSING_STRUCT_m12 *fps, si1 *unspecified_pw);
void		G_propogate_flags_m12(LEVEL_HEADER_m12 *level_header, ui8 new_flags);
void            G_push_behavior_m12(ui4 behavior);
CHANNEL_m12	*G_read_channel_m12(CHANNEL_m12 *chan, TIME_SLICE_m12 *slice, ...);  // varargs: si1 *chan_path, ui4 flags, si1 *password
CHANNEL_m12	*G_read_channel_nt_m12(CHANNEL_m12 *chan, TIME_SLICE_m12 *slice, ...);  // varargs: si1 *chan_path, ui8 flags, si1 *password  ("nt" == not threaded)
pthread_rval_m12	G_read_channel_thread_m12(void *ptr);
LEVEL_HEADER_m12	*G_read_data_m12(LEVEL_HEADER_m12 *level_header, TIME_SLICE_m12 *slice, ...);  // varargs (level_header == NULL): si1 *file_list, si4 list_len, ui8 flags, si1 *password
FILE_PROCESSING_STRUCT_m12	*G_read_file_m12(FILE_PROCESSING_STRUCT_m12 *fps, si1 *full_file_name, si8 file_offset, si8 bytes_to_read, si8 number_of_items, LEVEL_HEADER_m12 *lh, si1 *password, ui4 behavior_on_fail);
si8     	G_read_record_data_m12(LEVEL_HEADER_m12 *level_header, TIME_SLICE_m12 *slice, ...);  // varargs: si4 seg_num
SEGMENT_m12	*G_read_segment_m12(SEGMENT_m12 *seg, TIME_SLICE_m12 *slice, ...);  // varargs: si1 *seg_path, ui8 flags, si1 *password
pthread_rval_m12	G_read_segment_thread_m12(void *ptr);
SESSION_m12	*G_read_session_m12(SESSION_m12 *sess, TIME_SLICE_m12 *slice, ...);  // varargs: void *file_list, si4 list_len, ui4 flags, si1 *password
SESSION_m12	*G_read_session_nt_m12(SESSION_m12 *sess, TIME_SLICE_m12 *slice, ...);  // varargs: void *file_list, si4 list_len, ui8 flags, si1 *password  ("nt" == not threaded)
si8     	G_read_time_series_data_m12(SEGMENT_m12 *seg, TIME_SLICE_m12 *slice);
TERN_m12	G_recover_passwords_m12(si1 *L3_password, UNIVERSAL_HEADER_m12* universal_header);
TERN_m12	G_remove_path_m12(si1 *path);
void     	G_remove_recording_time_offset_m12(si8 *time);
void            G_reset_metadata_for_update_m12(FILE_PROCESSING_STRUCT_m12 *fps);
si8		G_sample_number_for_uutc_m12(LEVEL_HEADER_m12 *level_header, si8 target_uutc, ui4 mode, ...);  // varargs: si8 ref_sample_number, si8 ref_uutc, sf8 sampling_frequency
si4		G_search_Sgmt_records_m12(Sgmt_RECORD_m12 *Sgmt_records, TIME_SLICE_m12 *slice, ui4 search_mode);
si4		G_segment_for_frame_number_m12(LEVEL_HEADER_m12 *level_header, si8 target_sample);
si4		G_segment_for_sample_number_m12(LEVEL_HEADER_m12 *level_header, si8 target_sample);
si4		G_segment_for_uutc_m12(LEVEL_HEADER_m12 *level_header, si8 target_time);
si4		G_segment_number_for_path_m12(si1 *path);
void		G_sendgrid_email_m12(si1 *sendgrid_key, si1 *to_email, si1 *cc_email, si1 *to_name, si1 *subject, si1 *content, si1 *from_email, si1 *from_name, si1 *reply_to_email, si1 *reply_to_name);
si1		*G_session_path_for_path_m12(si1 *path, si1 *sess_path);
void		G_set_error_m12(const si4 err_code, const si1 *function, const si4 line);
TERN_m12	G_set_global_time_constants_m12(TIMEZONE_INFO_m12 *timezone_info, si8 session_start_time, TERN_m12 prompt);
void		G_set_globals_pointer_m12(GLOBALS_m12 *new_globals);
TERN_m12	G_set_time_and_password_data_m12(si1 *unspecified_password, si1 *MED_directory, si1 *metadata_section_2_encryption_level, si1 *metadata_section_3_encryption_level);
void		G_show_behavior_m12(void);
void            G_show_daylight_change_code_m12(DAYLIGHT_TIME_CHANGE_CODE_m12 *code, si1 *prefix);
void		G_show_file_times_m12(FILE_TIMES_m12 *ft);
void            G_show_globals_m12(void);
void		G_show_level_header_flags_m12(ui8 flags);
void    	G_show_location_info_m12(LOCATION_INFO_m12 *li);
void            G_show_metadata_m12(FILE_PROCESSING_STRUCT_m12 *fps, METADATA_m12 *md, ui4 type_code);
void            G_show_password_data_m12(PASSWORD_DATA_m12 *pwd);
void		G_show_password_hints_m12(PASSWORD_DATA_m12 *pwd);
void		G_show_records_m12(FILE_PROCESSING_STRUCT_m12 *record_data_fps, si4 *record_filters);
void		G_show_Sgmt_records_array_m12(LEVEL_HEADER_m12 *level_header, Sgmt_RECORD_m12 *Sgmt);
void    	G_show_time_slice_m12(TIME_SLICE_m12 *slice);
void            G_show_timezone_info_m12(TIMEZONE_INFO_m12 *timezone_entry, TERN_m12 show_DST_detail);
void            G_show_universal_header_m12(FILE_PROCESSING_STRUCT_m12 *fps, UNIVERSAL_HEADER_m12 *uh);
TERN_m12	G_sort_channels_by_acq_num_m12(SESSION_m12 *sess);
void		G_sort_records_m12(LEVEL_HEADER_m12 *level_header, si4 segment_number);
TERN_m12	G_ternary_entry_m12(si1 *entry);
void		G_textbelt_text_m12(si1 *phone_number, si1 *content, si1 *textbelt_key);
si1		*G_unique_temp_file_m12(si1 *temp_file);
void		G_update_maximum_entry_size_m12(FILE_PROCESSING_STRUCT_m12 *fps, si8 number_of_items, si8 bytes_to_write, si8 file_offset);
si8		G_uutc_for_frame_number_m12(LEVEL_HEADER_m12 *level_header, si8 target_frame_number, ui4 mode, ...);  // varargs: si8 ref_frame_number, si8 ref_uutc, sf8 frame_rate
si8		G_uutc_for_sample_number_m12(LEVEL_HEADER_m12 *level_header, si8 target_sample_number, ui4 mode, ...);  // varargs: si8 ref_smple_number, si8 ref_uutc, sf8 sampling_frequency
TERN_m12        G_validate_record_data_CRCs_m12(FILE_PROCESSING_STRUCT_m12 *fps);
TERN_m12        G_validate_time_series_data_CRCs_m12(FILE_PROCESSING_STRUCT_m12 *fps);
void            G_warning_message_m12(si1 *fmt, ...);
si8     	G_write_file_m12(FILE_PROCESSING_STRUCT_m12 *fps, si8 file_offset, si8 bytes_to_write, si8 number_of_items, void *external_data, ui4 behavior_on_fail);



//**********************************************************************************//
//************************  Windows-specific (WN) Functions  ***********************//
//**********************************************************************************//

#ifdef WINDOWS_m12

// function typedefs for WN_sleep_m12()
typedef HRESULT (CALLBACK* ZWSETTIMERRESTYPE)(ULONG, BOOLEAN, ULONG *);
typedef HRESULT (CALLBACK* NTDELAYEXECTYPE)(BOOLEAN, LARGE_INTEGER *);


FILETIME	WN_uutc_to_win_time_m12(si8 uutc);
void		WN_cleanup_m12(void);
void		WN_clear_m12(void);
si8		WN_date_to_uutc_m12(sf8 date);
TERN_m12	WN_initialize_terminal_m12(void);
si4    		WN_ls_1d_to_buf_m12(si1 **dir_strs, si4 n_dirs, TERN_m12 full_path, si1 **buffer);
si4		WN_ls_1d_to_tmp_m12(si1 **dir_strs, si4 n_dirs, TERN_m12 full_path, si1 *temp_file);
void		WN_nap_m12(struct timespec *nap);
TERN_m12	WN_reset_terminal_m12(void);
TERN_m12	WN_socket_startup_m12(void);
inline si4	WN_system_m12(si1 *command);
si8		WN_time_to_uutc_m12(FILETIME win_time);
sf8		WN_uutc_to_date_m12(si8 uutc);
void		WN_windify_file_paths_m12(si1 *target, si1 *source);
si1		*WN_windify_format_string_m12(si1 *fmt);
#endif

si8		WN_filetime_to_uutc_m12(ui1 *win_filetime);  // for conversion of windows file time to uutc on any platform



//**********************************************************************************//
//********************  MED Alignmment Checking (ALCK) Functions  ******************//
//**********************************************************************************//

TERN_m12	ALCK_all_m12(void);
TERN_m12	ALCK_metadata_m12(ui1 *bytes);
TERN_m12	ALCK_metadata_section_1_m12(ui1 *bytes);
TERN_m12	ALCK_metadata_section_3_m12(ui1 *bytes);
TERN_m12	ALCK_record_header_m12(ui1 *bytes);
TERN_m12	ALCK_record_indices_m12(ui1 *bytes);
TERN_m12	ALCK_time_series_indices_m12(ui1 *bytes);
TERN_m12	ALCK_time_series_metadata_section_2_m12(ui1 *bytes);
TERN_m12	ALCK_universal_header_m12(ui1 *bytes);
TERN_m12	ALCK_video_indices_m12(ui1 *bytes);
TERN_m12	ALCK_video_metadata_section_2_m12(ui1 *bytes);



//**********************************************************************************//
//*********************  MED Allocation Tracking (AT) Functions  *******************//
//**********************************************************************************//

// NOTE: The AT system keeps track all allocated & freed blocks of memory, so the list can grow continuously & may appear like a a very slow memory leak
// Previously freed memory blocks are not replaced in the list so in the case of an attempted double free, it can inform where the block was previously freed.

#define AT_CURRENTLY_ALLOCATED_m12	((ui4) 1)
#define AT_PREVIOUSLY_FREED_m12		((ui4) 2)
#define AT_ALL_m12			(AT_CURRENTLY_ALLOCATED_m12 | AT_PREVIOUSLY_FREED_m12)

void		AT_add_entry_m12(void *address, size_t requested_bytes, const si1 *function);
ui8		AT_alloc_size_m12(void *address);
void		AT_free_all_m12(void);
TERN_m12	AT_freeable_m12(void *address);
void		AT_mutex_off(void);
void		AT_mutex_on(void);
TERN_m12 	AT_remove_entry_m12(void *address, const si1 *function);
void		AT_show_entries_m12(ui4 entry_type);
void		AT_show_entry_m12(void *address);
TERN_m12 	AT_update_entry_m12(void *orig_address, void *new_address, size_t requested_bytes, const si1 *function);



//**********************************************************************************//
//*******************************  MED FPS Functions  ******************************//
//**********************************************************************************//

// Prototypes
FILE_PROCESSING_STRUCT_m12	*FPS_allocate_processing_struct_m12(FILE_PROCESSING_STRUCT_m12 *fps, si1 *full_file_name, ui4 type_code, si8 raw_data_bytes, LEVEL_HEADER_m12 *parent, FILE_PROCESSING_STRUCT_m12 *proto_fps, si8 bytes_to_copy);
void		FPS_close_m12(FILE_PROCESSING_STRUCT_m12 *fps);
si4		FPS_compare_start_times_m12(const void *a, const void *b);
void            FPS_free_processing_struct_m12(FILE_PROCESSING_STRUCT_m12 *fps, TERN_m12 free_fps_structure);
FPS_DIRECTIVES_m12	*FPS_initialize_directives_m12(FPS_DIRECTIVES_m12 *directives);
FPS_PARAMETERS_m12	*FPS_initialize_parameters_m12(FPS_PARAMETERS_m12 *parameters);
TERN_m12	FPS_lock_m12(FILE_PROCESSING_STRUCT_m12 *fps, si4 lock_type, const si1 *function, ui4 behavior_on_fail);
si8		FPS_memory_map_read_m12(FILE_PROCESSING_STRUCT_m12 *fps, si8 file_offset, si8 bytes_to_read, const si1 *function, ui4 behavior_on_fail);
void		FPS_mutex_off_m12(FILE_PROCESSING_STRUCT_m12 *fps);
void		FPS_mutex_on_m12(FILE_PROCESSING_STRUCT_m12 *fps);
TERN_m12	FPS_open_m12(FILE_PROCESSING_STRUCT_m12 *fps, const si1 *function, ui4 behavior_on_fail);
si8		FPS_read_m12(FILE_PROCESSING_STRUCT_m12 *fps, si8 file_offset, si8 bytes_to_read, const si1 *function, ui4 behavior_on_fail);
TERN_m12	FPS_reallocate_processing_struct_m12(FILE_PROCESSING_STRUCT_m12 *fps, si8 raw_data_bytes);
void		FPS_seek_m12(FILE_PROCESSING_STRUCT_m12 *fps, si8 file_offset);
void		FPS_set_pointers_m12(FILE_PROCESSING_STRUCT_m12 *fps, si8 file_offset);
void		FPS_show_processing_struct_m12(FILE_PROCESSING_STRUCT_m12 *fps);
void		FPS_sort_m12(FILE_PROCESSING_STRUCT_m12 **fps_array, si4 n_fps);
si4		FPS_unlock_m12(FILE_PROCESSING_STRUCT_m12 *fps, const si1 *function, ui4 behavior_on_fail);
si8		FPS_write_m12(FILE_PROCESSING_STRUCT_m12 *fps, si8 file_offset, si8 bytes_to_write, const si1 *function, ui4 behavior_on_fail);



//**********************************************************************************//
//**************************  MED String (STR) Functions  **************************//
//**********************************************************************************//

// Prototypes
wchar_t		*STR_char2wchar_m12(wchar_t *target, si1 *source);
ui4             STR_check_spaces_m12(si1 *string);
si4		STR_compare_m12(const void *a, const void *b);
TERN_m12    	STR_contains_formatting_m12(si1 *string, si1 *plain_string);
TERN_m12	STR_contains_regex_m12(si1 *string);
si1		*STR_duration_string_m12(si1 *dur_str, si8 int_usecs, TERN_m12 abbreviated, TERN_m12 two_level);
void            STR_escape_chars_m12(si1 *string, si1 target_char, si8 buffer_len);
si1		*STR_generate_hex_string_m12(ui1 *bytes, si4 num_bytes, si1 *string);
si1		*STR_match_end_m12(si1 *pattern, si1 *buffer);
si1		*STR_match_end_bin_m12(si1 *pattern, si1 *buffer, si8 buf_len);
si1		*STR_match_line_end_m12(si1 *pattern, si1 *buffer);
si1		*STR_match_line_start_m12(si1 *pattern, si1 *buffer);
si1		*STR_match_start_m12(si1 *pattern, si1 *buffer);
si1     	*STR_re_escape_m12(si1 *str, si1 *esc_str);
void    	STR_replace_char_m12(si1 c, si1 new_c, si1 *buffer);
si1		*STR_replace_pattern_m12(si1 *pattern, si1 *new_pattern, si1 *buffer, si1 *new_buffer);
si1		*STR_size_string_m12(si1 *size_str, si8 n_bytes, TERN_m12 base_2);
void		STR_sort_m12(si1 **string_array, si8 n_strings);
void		STR_strip_character_m12(si1 *s, si1 character);
const si1	*STR_tern_m12(TERN_m12 val);
si1		*STR_time_string_m12(si8 uutc_time, si1 *time_str, TERN_m12 fixed_width, TERN_m12 relative_days, si4 colored_text, ...);
void		STR_to_lower_m12(si1 *s);
void		STR_to_title_m12(si1 *s);
void		STR_to_upper_m12(si1 *s);
void            STR_unescape_chars_m12(si1 *string, si1 target_char);
si1		*STR_wchar2char_m12(si1 *target, wchar_t *source);



//**********************************************************************************//
//************************  CMP (COMPRESSION / COMPUTATION)  ***********************//
//**********************************************************************************//

// CMP: Miscellaneous Constants
#define CMP_SAMPLE_VALUE_NO_ENTRY_m12				NAN_SI4_m12
#define CMP_SPLINE_TAIL_LEN_m12                			6
#define CMP_SPLINE_UPSAMPLE_SF_RATIO_m12			((sf8) 3.0)
#define CMP_MAK_PAD_SAMPLES_m12					3
#define CMP_MAK_INPUT_BUFFERS_m12				8
#define CMP_MAK_IN_Y_BUF					0
#define CMP_MAK_IN_X_BUF					1
#define CMP_MAK_OUTPUT_BUFFERS_m12				4
#define CMP_MAK_OUT_Y_BUF					0
#define CMP_MAK_OUT_X_BUF					1
#define CMP_VDS_INPUT_BUFFERS_m12				(CMP_MAK_INPUT_BUFFERS_m12 + 1)
#define CMP_VDS_OUTPUT_BUFFERS_m12				CMP_MAK_OUTPUT_BUFFERS_m12
#define CMP_VDS_LOWPASS_ORDER_m12				6
#define CMP_VDS_MINIMUM_SAMPLES_m12				10
#define CMP_SELF_MANAGED_MEMORY_m12				-1  // pass CMP_SELF_MANAGED_MEMORY_m12 to CMP_allocate_processing_struct to prevent automatic re-allocation

// CMP: Block Fixed Header Offset Constants
#define CMP_BLOCK_FIXED_HEADER_BYTES_m12                        56                              // fixed region only
#define CMP_BLOCK_START_UID_m12                                 ((ui8) 0x0123456789ABCDEF)      // ui8   (decimal 81,985,529,216,486,895)
#define CMP_BLOCK_START_UID_OFFSET_m12				0
#define CMP_BLOCK_CRC_OFFSET_m12                                8                               // ui4
#define CMP_BLOCK_CRC_NO_ENTRY_m12                              CRC_NO_ENTRY_m12
#define CMP_BLOCK_BLOCK_FLAGS_OFFSET_m12			12                              // ui4
#define CMP_BLOCK_BLOCK_FLAGS_NO_ENTRY_m12			0
#define CMP_BLOCK_CRC_START_OFFSET_m12                          CMP_BLOCK_BLOCK_FLAGS_OFFSET_m12
#define CMP_BLOCK_START_TIME_OFFSET_m12                         16                              // si8
#define CMP_BLOCK_START_TIME_NO_ENTRY_m12                       UUTC_NO_ENTRY_m12
#define CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_OFFSET_m12         24                              // si4
#define CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m12       -1
#define CMP_BLOCK_TOTAL_BLOCK_BYTES_OFFSET_m12			28                              // ui4
#define CMP_BLOCK_TOTAL_BLOCK_BYTES_NO_ENTRY_m12		0
// CMP Block Encryption Start
#define CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m12                  32                              // ui4
#define CMP_BLOCK_ENCRYPTION_START_OFFSET_m12			CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m12
#define CMP_BLOCK_NUMBER_OF_SAMPLES_NO_ENTRY_m12                0xFFFFFFFF
#define CMP_BLOCK_NUMBER_OF_RECORDS_OFFSET_m12                  36                              // ui2
#define CMP_BLOCK_NUMBER_OF_RECORDS_NO_ENTRY_m12                0xFFFF
#define CMP_BLOCK_RECORD_REGION_BYTES_OFFSET_m12		38                              // ui2
#define CMP_BLOCK_RECORD_REGION_BYTES_NO_ENTRY_m12		0xFFFF
#define CMP_BLOCK_PARAMETER_FLAGS_OFFSET_m12			40                              // ui4
#define CMP_BLOCK_PARAMETER_FLAGS_NO_ENTRY_m12			0
#define CMP_BLOCK_PARAMETER_REGION_BYTES_OFFSET_m12		44				// ui2
#define CMP_BLOCK_PARAMETER_REGION_BYTES_NO_ENTRY_m12           0xFFFF
#define CMP_BLOCK_PROTECTED_REGION_BYTES_OFFSET_m12             46                              // ui2
#define CMP_BLOCK_PROTECTED_REGION_BYTES_NO_ENTRY_m12           0xFFFF
#define CMP_BLOCK_DISCRETIONARY_REGION_BYTES_OFFSET_m12         48                              // ui2
#define CMP_BLOCK_DISCRETIONARY_REGION_BYTES_NO_ENTRY_m12       0xFFFF
#define CMP_BLOCK_MODEL_REGION_BYTES_OFFSET_m12                 50                              // ui2
#define CMP_BLOCK_MODEL_REGION_BYTES_NO_ENTRY_m12               0xFFFF
#define CMP_BLOCK_TOTAL_HEADER_BYTES_OFFSET_m12			52                              // ui4
#define CMP_BLOCK_TOTAL_HEADER_BYTES_NO_ENTRY_m12            	0xFFFF
#define CMP_BLOCK_RECORDS_REGION_OFFSET_m12			56

// CMP: Record Header Offset Constants
#define CMP_RECORD_HEADER_BYTES_m12                        	8
#define CMP_RECORD_HEADER_TYPE_CODE_OFFSET_m12                  0				// ui4
#define CMP_RECORD_HEADER_TYPE_CODE_NO_ENTRY_m12		0
#define CMP_RECORD_HEADER_VERSION_MAJOR_OFFSET_m12		4				// ui1
#define CMP_RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m12		0xFF
#define CMP_RECORD_HEADER_VERSION_MINOR_OFFSET_m12		5				// ui1
#define CMP_RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m12		0xFF
#define CMP_RECORD_HEADER_TOTAL_BYTES_OFFSET_m12		6				// ui2
#define CMP_RECORD_HEADER_TOTAL_BYTES_NO_ENTRY_m12		0xFFFF				// Note maximum CMP record size is 65k - smaller than MED record

// CMP: RED (Range Encoded Derivatives) Model Offset Constants
#define CMP_RED_MODEL_NUMBER_OF_KEYSAMPLE_BYTES_OFFSET_m12             	0                       // ui4
#define CMP_RED_MODEL_DERIVATIVE_LEVEL_OFFSET_m12                  	4                       // ui1
#define CMP_RED_MODEL_PAD_BYTES_OFFSET_m12                  		5                       // ui1[3]
#define CMP_RED_MODEL_NUMBER_OF_STATISTICS_BINS_OFFSET_m12              8                      	// ui2
#define CMP_RED_MODEL_FLAGS_OFFSET_m12                  		10                      // ui2
#define CMP_RED_MODEL_FIXED_HEADER_BYTES_m12                            12
// RED Model Flags
#define CMP_RED_FLAGS_NO_ZERO_COUNTS_MASK_m12				((ui2) 1)       	// bit 0
#define CMP_RED_FLAGS_POSITIVE_DERIVATIVES_MASK_m12			((ui2) 1 << 1)       	// bit 1
#define CMP_RED_2_BYTE_OVERFLOWS_MASK_m12				((ui2) 1 << 2)		// bit 2
#define CMP_RED_3_BYTE_OVERFLOWS_MASK_m12				((ui2) 1 << 3)		// bit 3
#define CMP_RED_OVERFLOW_BYTES_MASK_m12					(CMP_RED_2_BYTE_OVERFLOWS_MASK_m12 | CMP_RED_3_BYTE_OVERFLOWS_MASK_m12)

// CMP: PRED (Predictive RED) Model Offset Constants
#define CMP_PRED_MODEL_NUMBER_OF_KEYSAMPLE_BYTES_OFFSET_m12            	0                       // ui4
#define CMP_PRED_MODEL_DERIVATIVE_LEVEL_OFFSET_m12                 	4                       // ui1
#define CMP_PRED_MODEL_PAD_BYTES_OFFSET_m12                  		5                       // ui1[3]
#define CMP_PRED_MODEL_NUMBERS_OF_STATISTICS_BINS_OFFSET_m12            8			// ui2[3]
#define CMP_PRED_MODEL_NUMBER_OF_NIL_STATISTICS_BINS_OFFSET_m12         CMP_PRED_MODEL_NUMBERS_OF_STATISTICS_BINS_OFFSET_m12  // ui2
#define CMP_PRED_MODEL_NUMBER_OF_POS_STATISTICS_BINS_OFFSET_m12         10                      // ui2
#define CMP_PRED_MODEL_NUMBER_OF_NEG_STATISTICS_BINS_OFFSET_m12         12                      // ui2
#define CMP_PRED_MODEL_FLAGS_OFFSET_m12                  		14                      // ui2
#define CMP_PRED_MODEL_FIXED_HEADER_BYTES_m12                           16
// PRED Model Flags
#define CMP_PRED_FLAGS_NO_ZERO_COUNTS_MASK_m12				((ui2) 1)       	// bit 0
#define CMP_PRED_FLAGS_BIT_1_m12					((ui2) 1 << 1)       	// bit 1  Note: this is used for positive derivatives in RED, left empty here to keep bits same
#define CMP_PRED_2_BYTE_OVERFLOWS_MASK_m12				((ui2) 1 << 2)		// bit 2
#define CMP_PRED_3_BYTE_OVERFLOWS_MASK_m12				((ui2) 1 << 3)		// bit 3
#define CMP_PRED_OVERFLOW_BYTES_MASK_m12				(CMP_PRED_2_BYTE_OVERFLOWS_MASK_m12 | CMP_PRED_3_BYTE_OVERFLOWS_MASK_m12)

// CMP: MBE (Minimal Bit Encoding) Model Offset Constants
#define CMP_MBE_MODEL_MINIMUM_VALUE_OFFSET_m12                   	0			// si4
#define CMP_MBE_MODEL_BITS_PER_SAMPLE_OFFSET_m12                        4                       // ui1
#define CMP_MBE_MODEL_DERIVATIVE_LEVEL_OFFSET_m12                 	5                       // ui1
#define CMP_MBE_MODEL_FLAGS_OFFSET_m12                           	6			// ui2
#define CMP_MBE_MODEL_FIXED_HEADER_BYTES_m12                            8
// MBE Model Flags
#define CMP_MBE_FLAGS_PREPROCESSED_MASK_m12				((ui2) 1)       	// bit 0 - message to MBE_encode()) it will clear it

// CMP: VDS (Vectorized Data Stream) Model Offset Constants
#define CMP_VDS_MODEL_NUMBER_OF_VDS_SAMPLES_OFFSET_m12            	0                       // ui4
#define CMP_VDS_MODEL_AMPLITUDE_BLOCK_TOTAL_BYTES_OFFSET_m12            4                       // ui4
#define CMP_VDS_MODEL_AMPLITUDE_BLOCK_MODEL_BYTES_OFFSET_m12            8                       // ui2
#define CMP_VDS_MODEL_TIME_BLOCK_MODEL_BYTES_OFFSET_m12            	10                      // ui2
#define CMP_VDS_MODEL_FLAGS_OFFSET_m12                  		12                      // ui4  (more options for VDS)
#define CMP_VDS_MODEL_FIXED_HEADER_BYTES_m12                            16
// VDS Model Flags
#define CMP_VDS_FLAGS_AMPLITUDE_RED1_MASK_m12				((ui4) 1)       	// bit 0
#define CMP_VDS_FLAGS_AMPLITUDE_PRED1_MASK_m12				((ui4) 1 << 1)       	// bit 1
#define CMP_VDS_FLAGS_AMPLITUDE_MBE_MASK_m12				((ui4) 1 << 2)		// bit 2
#define CMP_VDS_FLAGS_AMPLITUDE_RED2_MASK_m12				((ui4) 1 << 3)       	// bit 3
#define CMP_VDS_FLAGS_AMPLITUDE_PRED2_MASK_m12				((ui4) 1 << 4)       	// bit 4
#define CMP_VDS_FLAGS_TIME_RED1_MASK_m12				((ui4) 1 << 5)		// bit 5
#define CMP_VDS_FLAGS_TIME_PRED1_MASK_m12				((ui4) 1 << 6)		// bit 6
#define CMP_VDS_FLAGS_TIME_MBE_MASK_m12					((ui4) 1 << 7)		// bit 7
#define CMP_VDS_FLAGS_TIME_RED2_MASK_m12				((ui4) 1 << 8)		// bit 8
#define CMP_VDS_FLAGS_TIME_PRED2_MASK_m12				((ui4) 1 << 9)		// bit 9
#define CMP_VDS_AMPLITUDE_ALGORITHMS_MASK_m12				((ui4) (CMP_VDS_FLAGS_AMPLITUDE_RED1_MASK_m12 | CMP_VDS_FLAGS_AMPLITUDE_PRED1_MASK_m12 |   CMP_VDS_FLAGS_AMPLITUDE_MBE_MASK_m12 | CMP_VDS_FLAGS_AMPLITUDE_RED2_MASK_m12 | CMP_VDS_FLAGS_AMPLITUDE_PRED2_MASK_m12))
#define CMP_VDS_TIME_ALGORITHMS_MASK_m12				((ui4) (CMP_VDS_FLAGS_TIME_RED1_MASK_m12 | CMP_VDS_FLAGS_TIME_PRED1_MASK_m12 | CMP_VDS_FLAGS_TIME_MBE_MASK_m12 | CMP_VDS_FLAGS_TIME_RED2_MASK_m12 | CMP_VDS_FLAGS_TIME_PRED2_MASK_m12))
#define CMP_VDS_ALGORITHMS_MASK_m12					((ui4) (CMP_VDS_AMPLITUDE_ALGORITHMS_MASK_m12 | CMP_VDS_TIME_ALGORITHMS_MASK_m12))

// CMP Block Flag Masks
#define CMP_BF_BLOCK_FLAG_BITS_m12			32
#define CMP_BF_DISCONTINUITY_MASK_m12			((ui4) 1)       	// bit 0
#define CMP_BF_LEVEL_1_ENCRYPTION_MASK_m12		((ui4) 1 << 4)		// bit 4
#define CMP_BF_LEVEL_2_ENCRYPTION_MASK_m12		((ui4) 1 << 5)		// bit 5
#define CMP_BF_RED1_ENCODING_MASK_m12			((ui4) 1 << 8)		// bit 8
#define CMP_BF_PRED1_ENCODING_MASK_m12			((ui4) 1 << 9)		// bit 9
#define CMP_BF_MBE_ENCODING_MASK_m12			((ui4) 1 << 10)		// bit 10
#define CMP_BF_VDS_ENCODING_MASK_m12			((ui4) 1 << 11)		// bit 11
#define CMP_BF_RED2_ENCODING_MASK_m12			((ui4) 1 << 12)		// bit 12 - faster, use as default RED version
#define CMP_BF_PRED2_ENCODING_MASK_m12			((ui4) 1 << 13)		// bit 13 - faster, use as default PRED version

#define CMP_BF_ALGORITHMS_MASK_m12			((ui4) (CMP_BF_RED1_ENCODING_MASK_m12 | CMP_BF_PRED1_ENCODING_MASK_m12 | \
							CMP_BF_MBE_ENCODING_MASK_m12 | CMP_BF_VDS_ENCODING_MASK_m12 | \
							CMP_BF_RED2_ENCODING_MASK_m12 | CMP_BF_PRED2_ENCODING_MASK_m12 ))
#define CMP_BF_ENCRYPTION_MASK_m12			((ui4) (CMP_BF_LEVEL_1_ENCRYPTION_MASK_m12 | CMP_BF_LEVEL_2_ENCRYPTION_MASK_m12))

// CMP Parameter Map Indices
#define CMP_PF_INTERCEPT_IDX_m12			((ui4) 0)       // bit 0
#define CMP_PF_GRADIENT_IDX_m12				((ui4) 1)	// bit 1
#define CMP_PF_AMPLITUDE_SCALE_IDX_m12			((ui4) 2)       // bit 2
#define CMP_PF_FREQUENCY_SCALE_IDX_m12			((ui4) 3)       // bit 3
#define CMP_PF_NOISE_SCORES_IDX_m12			((ui4) 4)	// bit 4

// CMP Parameter Flag Masks
#define CMP_PF_PARAMETER_FLAG_BITS_m12			32
#define CMP_PF_INTERCEPT_MASK_m12			((ui4) 1 << CMP_PF_INTERCEPT_IDX_m12)		// bit 0
#define CMP_PF_GRADIENT_MASK_m12			((ui4) 1 << CMP_PF_GRADIENT_IDX_m12)		// bit 1
#define CMP_PF_AMPLITUDE_SCALE_MASK_m12			((ui4) 1 << CMP_PF_AMPLITUDE_SCALE_IDX_m12)	// bit 2
#define CMP_PF_FREQUENCY_SCALE_MASK_m12			((ui4) 1 << CMP_PF_FREQUENCY_SCALE_IDX_m12)	// bit 3
#define CMP_PF_NOISE_SCORES_MASK_m12			((ui4) 1 << CMP_PF_NOISE_SCORES_IDX_m12)	// bit 4

// Compression Modes
#define CMP_COMPRESSION_MODE_NO_ENTRY_m12	((ui1) 0)
#define CMP_DECOMPRESSION_MODE_m12              ((ui1) 1)
#define CMP_COMPRESSION_MODE_m12                ((ui1) 2)

// Lossy Compression Modes
#define CMP_AMPLITUDE_SCALE_MODE_m12		((ui1) 1)
#define CMP_FREQUENCY_SCALE_MODE_m12		((ui1) 2)

// Compression Algorithms
#define CMP_RED1_COMPRESSION_m12	CMP_BF_RED1_ENCODING_MASK_m12
#define CMP_PRED1_COMPRESSION_m12	CMP_BF_PRED1_ENCODING_MASK_m12
#define CMP_MBE_COMPRESSION_m12		CMP_BF_MBE_ENCODING_MASK_m12
#define CMP_VDS_COMPRESSION_m12		CMP_BF_VDS_ENCODING_MASK_m12
#define CMP_RED2_COMPRESSION_m12	CMP_BF_RED2_ENCODING_MASK_m12
#define CMP_PRED2_COMPRESSION_m12	CMP_BF_PRED2_ENCODING_MASK_m12

#define CMP_RED_COMPRESSION_m12		CMP_RED2_COMPRESSION_m12	// use RED v2 as default
#define CMP_PRED_COMPRESSION_m12	CMP_PRED2_COMPRESSION_m12	// use PRED v2 as default

// CMP Directives Defaults
#define CMP_DIRECTIVES_COMPRESSION_MODE_DEFAULT_m12			CMP_COMPRESSION_MODE_NO_ENTRY_m12
#define CMP_DIRECTIVES_ALGORITHM_DEFAULT_m12				CMP_PRED2_COMPRESSION_m12
#define CMP_DIRECTIVES_ENCRYPTION_LEVEL_DEFAULT_m12			NO_ENCRYPTION_m12
#define CMP_DIRECTIVES_CPS_POINTER_RESET_DEFAULT_m12			TRUE_m12
#define CMP_DIRECTIVES_CPS_CACHING_DEFAULT_m12				TRUE_m12
#define CMP_DIRECTIVES_FALL_THROUGH_TO_BEST_ENCODING_DEFAULT_m12	TRUE_m12
#define CMP_DIRECTIVES_RESET_DISCONTINUITY_DEFAULT_m12			TRUE_m12
#define CMP_DIRECTIVES_INCLUDE_NOISE_SCORES_DEFAULT_m12			FALSE_m12
#define CMP_DIRECTIVES_NO_ZERO_COUNTS_DEFAULT_m12			FALSE_m12
#define CMP_DIRECTIVES_SET_OVERFLOW_BYTES_DEFAULT_m12			FALSE_m12  	// user sets value in parameters
#define CMP_DIRECTIVES_FIND_OVERFLOW_BYTES_DEFAULT_m12			TRUE_m12  	// determine overflow bytes on a block by block basis
#define CMP_DIRECTIVES_NO_ZERO_COUNTS_DEFAULT_m12          		FALSE_m12
#define CMP_DIRECTIVES_POSITIVE_DERIVATIVES_DEFAULT_m12    		FALSE_m12
#define CMP_DIRECTIVES_SET_DERIVATIVE_LEVEL_DEFAULT_m12			FALSE_m12	// user sets value in parameters
#define CMP_DIRECTIVES_FIND_DERIVATIVE_LEVEL_DEFAULT_m12		FALSE_m12
#define CMP_DIRECTIVES_CONVERT_TO_NATIVE_UNITS_DEFAULT_m12		TRUE_m12
// lossy compression directives
#define CMP_DIRECTIVES_DETREND_DATA_DEFAULT_m12				FALSE_m12
#define CMP_DIRECTIVES_REQUIRE_NORMALITY_DEFAULT_m12			FALSE_m12
#define CMP_DIRECTIVES_RETURN_LOSSY_DATA_DEFAULT_m12			FALSE_m12
#define CMP_DIRECTIVES_USE_COMPRESSION_RATIO_DEFAULT_m12		FALSE_m12
#define CMP_DIRECTIVES_USE_MEAN_RESIDUAL_RATIO_DEFAULT_m12		TRUE_m12
#define CMP_DIRECTIVES_USE_RELATIVE_RATIO_DEFAULT_m12			FALSE_m12
#define CMP_DIRECTIVES_SET_AMPLITUDE_SCALE_DEFAULT_m12			FALSE_m12	// user sets value in parameters
#define CMP_DIRECTIVES_FIND_AMPLITUDE_SCALE_DEFAULT_m12			FALSE_m12
#define CMP_DIRECTIVES_SET_FREQUENCY_SCALE_DEFAULT_m12			FALSE_m12	// user sets value in parameters
#define CMP_DIRECTIVES_FIND_FREQUENCY_SCALE_DEFAULT_m12			FALSE_m12
#define CMP_DIRECTIVES_VDS_SCALE_BY_BASELINE_DEFAULT_m12		FALSE_m12	// increases compression by ~15%

// CMP Parameters Defaults
#define CMP_PARAMETERS_NUMBER_OF_BLOCK_PARAMETERS_DEFAULT_m12	0
#define CMP_PARAMETERS_MINIMUM_SAMPLE_VALUE_DEFAULT_m12		CMP_SAMPLE_VALUE_NO_ENTRY_m12
#define CMP_PARAMETERS_MAXIMUM_SAMPLE_VALUE_DEFAULT_m12		CMP_SAMPLE_VALUE_NO_ENTRY_m12
#define CMP_PARAMETERS_DISCONTINUITY_DEFAULT_m12		UNKNOWN_m12
#define CMP_PARAMETERS_DERIVATIVE_LEVEL_DEFAULT_m12		((ui1) 1)
#define CMP_PARAMETERS_OVERFLOW_BYTES_DEFAULT_m12          	4
	// lossy compression parameters
#define CMP_PARAMETERS_GOAL_RATIO_DEFAULT_m12			((sf8) 0.05)
#define CMP_PARAMETERS_GOAL_TOLERANCE_DEFAULT_m12		((sf8) 0.005)
#define CMP_PARAMETERS_MAXIMUM_GOAL_ATTEMPTS_DEFAULT_m12	20
#define CMP_PARAMETERS_MINIMUM_NORMALITY_DEFAULT_m12		((ui1) 128)	// range 0-254 (low to high)
#define CMP_PARAMETERS_AMPLITUDE_SCALE_DEFAULT_m12		((sf4) 1.0)
#define CMP_PARAMETERS_FREQUENCY_SCALE_DEFAULT_m12		((sf4) 1.0)
#define CMP_PARAMETERS_VDS_THRESHOLD_DEFAULT_m12		((sf8) 5.0)	// generally an integer, but any float value is fine. Range 0.0 to 10.0; default == 5.0  (0.0 == lossless compression)
	// variable region parameters
#define CMP_USER_NUMBER_OF_RECORDS_DEFAULT_m12			((ui2) 0)
#define CMP_USER_RECORD_REGION_BYTES_DEFAULT_m12		((ui2) 0)
#define CMP_USER_PARAMETER_FLAGS_DEFAULT_m12			((ui4) 0)
#define CMP_PROTECTED_REGION_BYTES_DEFAULT_m12			((ui2) 0)
#define CMP_USER_DISCRETIONARY_REGION_BYTES_DEFAULT_m12		((ui2) 0)

// RED/PRED Codec Constants
#define CMP_SI1_KEYSAMPLE_FLAG_m12      	((si1) 0x80)		// -128 as si1
#define CMP_UI1_KEYSAMPLE_FLAG_m12      	((ui1) 0x80)		// +128 as ui1
#define CMP_POS_DERIV_KEYSAMPLE_FLAG_m12	((ui1) 0x00)		// no zero differences expected in positive derivative model
#define CMP_RED_TOTAL_COUNTS_m12        	((ui4) 0x10000)         // 2^16
#define CMP_RED_MAXIMUM_RANGE_m12       	((ui8) 0x1000000000000) // 2^48
#define CMP_RED_RANGE_MASK_m12          	((ui8) 0xFFFFFFFFFFFF)  // 2^48 - 1
#define CMP_RED_MAX_STATS_BINS_m12      	256
#define CMP_PRED_CATS_m12               	3
#define CMP_PRED_NIL_m12                	0
#define CMP_PRED_POS_m12                	1
#define CMP_PRED_NEG_m12			2

// Macros
#define CMP_MAX_KEYSAMPLE_BYTES_m12(block_samps)		(block_samps * 5) // full si4 plus 1 keysample flag byte per sample
#define CMP_MAX_COMPRESSED_BYTES_m12(block_samps, n_blocks)	(((block_samps * 4) + CMP_BLOCK_FIXED_HEADER_BYTES_m12 + 7) * n_blocks)	// (no compression + header + maximum pad bytes) for n_blocks blocks
																	// NOTE: does not take variable region bytes into account and assumes
																	// fallthrough to MBE
#define CMP_PRED_CAT_m12(x)					((x) ? (((x) & 0x80) ? CMP_PRED_NEG_m12 : CMP_PRED_POS_m12) : CMP_PRED_NIL_m12) // do not increment/decrement within call to CMP_PRED_CAT_m12
																		// as "x" is used twice
#define CMP_IS_DETRENDED_m12(block_header_ptr)			((block_header_ptr->parameter_flags & CMP_PF_INTERCEPT_MASK_m12) && (block_header_ptr->parameter_flags & CMP_PF_GRADIENT_MASK_m12))
#define CMP_VARIABLE_REGION_BYTES_v1_m12(block_header_ptr)	((ui4) (block_header_ptr)->record_region_bytes + (ui4) (block_header_ptr)->parameter_region_bytes + \
								(ui4) (block_header_ptr)->protected_region_bytes + (ui4) (block_header_ptr)->discretionary_region_bytes)
#define CMP_VARIABLE_REGION_BYTES_v2_m12(block_header_ptr)	((ui4) (block_header_ptr)->total_header_bytes - ((ui4) CMP_BLOCK_FIXED_HEADER_BYTES_m12 + \
								(ui4) (block_header_ptr)->model_region_bytes))

// Update CPS Pointer Flags
#define CMP_UPDATE_ORIGINAL_PTR_m12             ((ui1) 1)
#define CMP_RESET_ORIGINAL_PTR_m12             	((ui1) 2)
#define CMP_UPDATE_BLOCK_HEADER_PTR_m12         ((ui1) 4)
#define CMP_RESET_BLOCK_HEADER_PTR_m12         	((ui1) 8)
#define CMP_UPDATE_DECOMPRESSED_PTR_m12         ((ui1) 16)
#define CMP_RESET_DECOMPRESSED_PTR_m12         	((ui1) 32)

// Binterpolate() center mode codes
#define CMP_CENT_MODE_NONE_m12			0  // extrema only
#define CMP_CENT_MODE_MIDPOINT_m12		1  // best performance if extrema needed: (min + max) / 2
#define CMP_CENT_MODE_MEAN_m12			2  // best performance if extrema not needed
#define CMP_CENT_MODE_MEDIAN_m12		3  // best measure of central tendency
#define CMP_CENT_MODE_FASTEST_m12		4  // CMP_CENT_MODE_MIDPOINT_m12 if extrema requested, CMP_CENT_MODE_MEAN_m12 if not
#define CMP_CENT_MODE_DEFAULT_m12		CMP_CENT_MODE_MEAN_m12

// Normal cumulative distribution function values from -3 to +3 standard deviations in 0.1 sigma steps
#define CMP_NORMAL_CDF_TABLE_ENTRIES_m12        61
#define CMP_NORMAL_CDF_TABLE_m12	      { 0.00134989803163010, 0.00186581330038404, 0.00255513033042794, 0.00346697380304067, \
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

#define CMP_SUM_NORMAL_CDF_m12                  30.5
#define CMP_SUM_SQ_NORMAL_CDF_m12               24.864467406647070
#define CMP_KS_CORRECTION_m12                   ((sf8) 0.0001526091333688973)

#define CMP_VDS_THRESHOLD_MAP_TABLE_ENTRIES_m12	101
#define CMP_VDS_THRESHOLD_MAP_TABLE_m12 { \
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
typedef struct {  // requires 4-byte alignment
	ui4	number_of_keysample_bytes;
	ui1	derivative_level;
	ui1	pad[3];
	ui2	number_of_statistics_bins;
	ui2	flags;
} CMP_RED_MODEL_FIXED_HEADER_m12;

typedef struct {  // requires 4-byte alignment
	ui4	number_of_keysample_bytes;
	ui1	derivative_level;
	ui1	pad[3];
	union {
		ui2	numbers_of_statistics_bins[3];
		struct {
			ui2	number_of_nil_statistics_bins;
			ui2	number_of_pos_statistics_bins;
			ui2	number_of_neg_statistics_bins;
		};
	};
	ui2	flags;
} CMP_PRED_MODEL_FIXED_HEADER_m12;

typedef struct {  // requires 4-byte alignment
	si4	minimum_value;  // of highest derivative
	ui1	bits_per_sample;
	ui1	derivative_level;
	ui2	flags;
} CMP_MBE_MODEL_FIXED_HEADER_m12;

typedef struct {  // requires 4-byte alignment
	ui4	number_of_VDS_samples;
	ui4	amplitude_block_total_bytes;
	ui2	amplitude_block_model_bytes;
	ui2	time_block_model_bytes;
	ui4	flags;  // potentially more options for VDS, so 32 bits
} CMP_VDS_MODEL_FIXED_HEADER_m12;

typedef struct {
	ui4	type_code;  // note this is not null terminated and so cannot be treated as a string as in RECORD_HEADER_m12 structure
	ui1	version_major;
	ui1	version_minor;
	ui2	total_bytes;  // note maximum record size is 65535 - smaller than in RECORD_HEADER_m12 structure
} CMP_RECORD_HEADER_m12;

// CMP_BLOCK_FIXED_HEADER_m12 declared above

typedef struct {
	ui4     count;
	union {
		si1     value;
		ui1	pos_value;
	};
} CMP_STATISTICS_BIN_m12;

typedef struct {
	si8	n_buffers;
	si8 	n_elements;
	si8	element_size;
	void	**buffer;
	// used internally
	ui8		total_allocated_bytes;
	TERN_m12	locked;
} CMP_BUFFERS_m12;

typedef struct {
	si8		cache_offset;
	ui4		block_samples;
	si4		block_number;
	TERN_m12	data_read;
} CMP_CACHE_BLOCK_INFO_m12;

typedef struct NODE_STRUCT_m12 {
	si4                     val;
	ui4                     count;
	struct NODE_STRUCT_m12	*prev, *next;
} CMP_NODE_m12;

// Directives contain "behavior" of CPS
typedef struct {
	ui4             mode;  // CMP_COMPRESSION_MODE_m12, CMP_DECOMPRESSION_MODE_m12
	ui4             algorithm;  // RED, PRED, MBE, or VDS
	si1             encryption_level;  // encryption level for data blocks: passed in compression
	TERN_m12	cps_pointer_reset;  // FALSE to maunually control cps pointers
	TERN_m12        cps_caching;  // use for largely sequential reads, not random reads or open/close behavior
	TERN_m12        fall_through_to_best_encoding;  // if another encoding would be smaller than RED/PRED, use it for the block
	TERN_m12        reset_discontinuity;  // if discontinuity directive == TRUE_m12, reset to FALSE_m12 after compressing the block
	TERN_m12        include_noise_scores;  // a set of 4 metrics that measure different types of noise (range 0-254: 0 no noise, 254 max noise, 0xFF no entry)
	TERN_m12        no_zero_counts;  // in RED & PRED codecs (when blocks must be encoded with non-block statistics. This is a special case.)
	TERN_m12        set_derivative_level;  // value passed in "goal_derivative_level" parameter
	TERN_m12        find_derivative_level;  // mutually exclusive with "set_derivative_level"
	TERN_m12	set_overflow_bytes;  // value passed in "goal_overflow_bytes" parameter (range 2-4 bytes)
	TERN_m12	find_overflow_bytes;  // mutually exclusive with "set_overflow_bytes"
	TERN_m12	convert_to_native_units;  // use amplitude_units_conversion_factor to to convert to units of amplitude_units_description on read
	// lossy compression directives
	TERN_m12        detrend_data;  // lossless operation, but most useful for lossy compression.
	TERN_m12        require_normality;  // for lossy compression - use lossless if data amplitudes are too oddly distributed.  Pairs with "minimum_normality" parameter.
	TERN_m12        use_compression_ratio;  // used in "find" directives. Mutually exclusive with "use_mean_residual_ratio".
	TERN_m12        use_mean_residual_ratio;  // used in "find" directives. Mutually exclusive with "use_compression_ratio".
	TERN_m12        use_relative_ratio;  // divide goal ratio by the block coefficient of variation in lossy compression routines (more precision in blocks with higher variance)
	TERN_m12        set_amplitude_scale;  // value passed in "amplitude_scale" parameter
	TERN_m12        find_amplitude_scale;  // mutually exclusive with "set_amplitude_scale"
	TERN_m12        set_frequency_scale;  // value passed in "frequency_scale" parameter
	TERN_m12        find_frequency_scale;  // mutually exclusive with "set_frequency_scale"
	TERN_m12	VDS_scale_by_baseline;  // in VDS compression: scale data by baseline amplitude
} CMP_DIRECTIVES_m12;

// Parameters contain "mechanics" of CPS
typedef struct {
	pthread_mutex_t_m12	mutex;
	// cache parameters
	CMP_CACHE_BLOCK_INFO_m12	*cached_blocks;
	si4		cached_block_list_len;
	si4		cached_block_cnt;
	si4		*cache;
	// memory parameters
	si8		allocated_block_samples;
	si8		allocated_keysample_bytes;
	si8		allocated_compressed_bytes;  // == time series data fps: (raw_data_bytes - UNIVERSAL_HEADER_BYTES_m12)
	si8		allocated_decompressed_samples;
	// compression parameters
	ui1		goal_derivative_level;  // used with set_derivative_level directive
	ui1		derivative_level;  // goal/actual pairs because not always possible
	ui1		goal_overflow_bytes;  // used with set_overflow_bytes directive
	ui1		overflow_bytes;  // goal/actual pairs because not always possible
	// block parameters
	TERN_m12	discontinuity;  // set if block is first after a discontinuity, passed in compression, returned in decompression
	ui4		block_start_index;  // block relative
	ui4		block_end_index;  // block relative
	si4		number_of_block_parameters;
	ui4		block_parameter_map[CMP_PF_PARAMETER_FLAG_BITS_m12];
	si4		minimum_sample_value;  // found on compression, stored for general use (and MBE, if used)
	si4		maximum_sample_value;  // found on compression, stored for general use (and MBE, if used)
	si4		minimum_difference_value;
	si4		maximum_difference_value;
	ui2		user_number_of_records;  // set by user
	ui2		user_record_region_bytes;  // set by user to reserve bytes for records in block header
	ui4		user_parameter_flags;  // user bits to be set in parameter flags of block header (library flags will be set automatically)
	ui2		protected_region_bytes;  // not currently used, set to zero (allows for future expansion)
	ui2		user_discretionary_region_bytes;  // set by user to reserve bytes for discretionary region in header
	ui4		variable_region_bytes;  // value calculated and set by library based on parameters & directives
	ui4		number_of_derivative_bytes;  // values in derivative or difference buffer
	// lossy compression parameters
	sf8		goal_ratio;  // either compression ratio or mean residual ratio
	sf8		actual_ratio;  // either compression ratio or mean residual ratio
	sf8		goal_tolerance;  // tolerance for lossy compression mode goal, value of <= 0.0 uses default values, which are returned
	si4		maximum_goal_attempts;  // maximum loops to attain goal compression
	ui1		minimum_normality;  // range 0-254: 0 not normal, 254 perfectly normal, 0xFF no entry
	sf4		amplitude_scale;  // used with set_amplitude_scale directive
	sf4		frequency_scale;  // used with set_frequency_scale directive
	sf8		VDS_LFP_high_fc;  // lowpass filter cutoff for VDS encoding (0.0 for no filter)
	sf8		VDS_threshold;  // generally an integer, but float values work fine. Range 0.0 to 10.0; default == 5.0  (0.0 indicates lossless compression)
	sf8		VDS_sampling_frequency;  // used to preserve units during LFP filtering, if filtering is not specified, this is not used
	// compression arrays
	si1			*keysample_buffer;  // passed in both compression & decompression
	si4			*derivative_buffer;  // used if needed in compression & decompression, size of maximum block differences
	si4			*detrended_buffer;  // used if needed in compression, size of decompressed block
	si4			*scaled_amplitude_buffer;  // used if needed in compression, size of decompressed block
	si4			*scaled_frequency_buffer;  // used if needed in compression, size of decompressed block
	CMP_BUFFERS_m12		*scrap_buffers;  // generic use
	CMP_BUFFERS_m12		*VDS_input_buffers;
	CMP_BUFFERS_m12		*VDS_output_buffers;
	void			**filtps;
	si4			n_filtps;
	ui1			*model_region;
	void			*count;  // used by RED/PRED encode & decode (ui4 * or ui4 **)
	void			*sorted_count;  // used by RED/PRED encode & decode (CMP_STATISTICS_BIN_m12 * or CMP_STATISTICS_BIN_m12 **)
	void			*cumulative_count;  // used by RED/PRED encode & decode (ui8 * or ui8 **)
	void			*minimum_range;  // used by RED/PRED encode & decode (ui8 * or ui8 **)
	void			*symbol_map;  // used by RED/PRED encode & decode (ui1 * or ui1 **)
} CMP_PARAMETERS_m12;

typedef struct CMP_PROCESSING_STRUCT_m12 {
	CMP_DIRECTIVES_m12   		directives;
	CMP_PARAMETERS_m12  		parameters;
	si4				*input_buffer;  // pointer that is updated depending on processing options (e.g. points to detrended data, scaled daata, etc.)
	ui1				*compressed_data;  // points to base of FPS time_series_data array, NOT an allocated pointer => do not free; should not be updated
	CMP_BLOCK_FIXED_HEADER_m12	*block_header; // == FPS time_series_data; points to beginning of current block within compressed_data array, updatable
	si4				*decompressed_data;  // returned in decompression or if lossy data requested, used in some compression modes, should not be updated
	si4				*decompressed_ptr;  // points to beginning of current block within decompressed_data array, updatable
	si4				*original_data;  // passed in compression, should not be updated
	si4				*original_ptr;  // points to beginning of current block within original_data array, updatable
	ui1				*block_records;  // pointer beginning of records region of block header
	ui4				*block_parameters;  // pointer beginning of parameter region of block header
	ui1				*discretionary_region;
} CMP_PROCESSING_STRUCT_m12;

// Function Prototypes
CMP_BUFFERS_m12	*CMP_allocate_buffers_m12(CMP_BUFFERS_m12 *buffers, si8 n_buffers, si8 n_elements, si8 element_size, TERN_m12 zero_data, TERN_m12 lock_memory);
CMP_PROCESSING_STRUCT_m12	*CMP_allocate_processing_struct_m12(FILE_PROCESSING_STRUCT_m12 *fps, ui4 mode, si8 data_samples, si8 compressed_data_bytes, si8 keysample_bytes, ui4 block_samples, CMP_DIRECTIVES_m12 *directives, CMP_PARAMETERS_m12 *parameters);
void		CMP_binterpolate_sf8_m12(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len, ui4 center_mode, TERN_m12 extrema, sf8 *minima, sf8 *maxima);
void		CMP_byte_to_hex_m12(ui1 byte, si1 *hex);
sf8      	CMP_calculate_mean_residual_ratio_m12(si4 *original_data, si4 *lossy_data, ui4 n_samps);
void		CMP_calculate_statistics_m12(REC_Stat_v10_m12 *stats_ptr, si4 *data, si8 len, CMP_NODE_m12 *nodes);
TERN_m12	CMP_check_block_header_alignment_m12(ui1 *bytes);
TERN_m12	CMP_check_CPS_allocation_m12(FILE_PROCESSING_STRUCT_m12 *fps);
TERN_m12        CMP_check_record_header_alignment_m12(ui1 *bytes);
si4		CMP_compare_sf8_m12(const void *a, const void * b);
si4		CMP_compare_si4_m12(const void *a, const void * b);
si4     	CMP_compare_si8_m12(const void *a, const void * b);
si4		CMP_count_bins_m12(CMP_PROCESSING_STRUCT_m12 *cps, si4 *deriv_buffer, ui1 n_derivs);
void		CMP_cps_mutex_off_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void		CMP_cps_mutex_on_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_decode_m12(FILE_PROCESSING_STRUCT_m12 *fps);
TERN_m12     	CMP_decrypt_m12(FILE_PROCESSING_STRUCT_m12 *fps);  // single block decrypt (see also decrypt_time_series_data_m12)
void    	CMP_detrend_m12(si4 *input_buffer, si4 *output_buffer, si8 len, CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_detrend_sf8_m12(sf8 *input_buffer, sf8 *output_buffer, si8 len);
ui1		CMP_differentiate_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void		CMP_encode_m12(FILE_PROCESSING_STRUCT_m12 *fps, si8 start_time, si4 acquisition_channel_number, ui4 number_of_samples);
TERN_m12	CMP_encrypt_m12(FILE_PROCESSING_STRUCT_m12 *fps);  // single block encrypt (see also encrypt_time_series_data_m12)
TERN_m12	CMP_find_amplitude_scale_m12(CMP_PROCESSING_STRUCT_m12 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m12 *cps));
si8    		*CMP_find_crits_m12(sf8 *data, si8 data_len, si8 *n_crits, si8 *crit_xs);
void    	CMP_find_crits_2_m12(sf8 *data, si8 data_len, si8 *n_peaks, si8 *peak_xs, si8 *n_troughs, si8 *trough_xs);
void    	CMP_find_extrema_m12(si4 *input_buffer, si8 len, si4 *min, si4 *max, CMP_PROCESSING_STRUCT_m12 *cps);
TERN_m12	CMP_find_frequency_scale_m12(CMP_PROCESSING_STRUCT_m12 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m12 *cps));
void    	CMP_free_buffers_m12(CMP_BUFFERS_m12 *buffers, TERN_m12 free_structure);
TERN_m12    	CMP_free_cache_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_free_processing_struct_m12(CMP_PROCESSING_STRUCT_m12 *cps, TERN_m12 free_cps_structure);
sf8		CMP_gamma_cdf_m12(sf8 x, sf8 k, sf8 theta, sf8 offset);
sf8		CMP_gamma_cf_m12(sf8 a, sf8 x, sf8 *g_ln);
sf8		CMP_gamma_inv_cdf_m12(sf8 p, sf8 k, sf8 theta, sf8 offset);
sf8		CMP_gamma_inv_p_m12(sf8 p, sf8 a);
sf8		CMP_gamma_ln_m12(sf8 xx);
sf8		CMP_gamma_p_m12(sf8 a, sf8 x);
sf8		CMP_gamma_ser_m12(sf8 a, sf8 x, sf8 *g_ln);
void    	CMP_generate_lossy_data_m12(CMP_PROCESSING_STRUCT_m12 *cps, si4* input_buffer, si4 *output_buffer, ui1 mode);
void		CMP_generate_parameter_map_m12(CMP_PROCESSING_STRUCT_m12 *cps);
ui1    		CMP_get_overflow_bytes_m12(CMP_PROCESSING_STRUCT_m12 *cps, ui4 mode, ui4 algorithm);
void    	CMP_get_variable_region_m12(CMP_PROCESSING_STRUCT_m12 *cps);
TERN_m12	CMP_hex_to_int_m12(ui1 *in, ui1 *out, si4 len);
void		CMP_initialize_directives_m12(CMP_DIRECTIVES_m12 *directives, ui1 mode);
void		CMP_initialize_parameters_m12(CMP_PARAMETERS_m12 *parameters);
TERN_m12	CMP_initialize_tables_m12(void);
void		CMP_integrate_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_lad_reg_2_sf8_m12(sf8 *x_input_buffer, sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lad_reg_2_si4_m12(si4 *x_input_buffer, si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lad_reg_sf8_m12(sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lad_reg_si4_m12(si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
sf8		*CMP_lin_interp_2_sf8_m12(si8 *in_x, sf8 *in_y, si8 in_len, sf8 *out_y, si8 *out_len);
sf8		*CMP_lin_interp_sf8_m12(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len);
si4		*CMP_lin_interp_si4_m12(si4 *in_data, si8 in_len, si4 *out_data, si8 out_len);
void    	CMP_lin_reg_2_sf8_m12(sf8 *x_input_buffer, sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lin_reg_2_si4_m12(si4 *x_input_buffer, si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lin_reg_sf8_m12(sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lin_reg_si4_m12(si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void		CMP_lock_buffers_m12(CMP_BUFFERS_m12 *buffers);
void    	CMP_MBE_decode_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_MBE_encode_m12(CMP_PROCESSING_STRUCT_m12 *cps);
sf8     	*CMP_mak_interp_sf8_m12(CMP_BUFFERS_m12 *in_bufs, si8 in_len, CMP_BUFFERS_m12 *out_bufs, si8 out_len);
ui1     	CMP_normality_score_m12(si4 *data, ui4 n_samps);
sf8		CMP_p2z_m12(sf8 p);
void    	CMP_PRED1_decode_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_PRED2_decode_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_PRED1_encode_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_PRED2_encode_m12(CMP_PROCESSING_STRUCT_m12 *cps);
sf8     	CMP_quantval_m12(sf8 *data, si8 len, sf8 quantile, TERN_m12 preserve_input, sf8 *buff);
CMP_PROCESSING_STRUCT_m12	*CMP_reallocate_processing_struct_m12(FILE_PROCESSING_STRUCT_m12 *fps, ui4 mode, si8 data_samples, ui4 block_samples);
ui1             CMP_random_byte_m12(ui4 *m_w, ui4 *m_z);
void    	CMP_rectify_m12(si4 *input_buffer, si4 *output_buffer, si8 len);
void    	CMP_RED1_decode_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_RED2_decode_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_RED1_encode_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_RED2_encode_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_retrend_si4_m12(si4 *in_y, si4 *out_y, si8 len, sf8 m, sf8 b);
void    	CMP_retrend_2_sf8_m12(sf8 *in_x, sf8 *in_y, sf8 *out_y, si8 len, sf8 m, sf8 b);
si2      	CMP_round_si2_m12(sf8 val);
si4     	CMP_round_si4_m12(sf8 val);
void    	CMP_scale_amplitude_si4_m12(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor, CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_scale_frequency_si4_m12(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor, CMP_PROCESSING_STRUCT_m12 *cps);
void    	CMP_set_variable_region_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void      	CMP_sf8_to_si4_m12(sf8 *sf8_arr, si4 *si4_arr, si8 len, TERN_m12 round);
void      	CMP_sf8_to_si4_and_scale_m12(sf8 *sf8_arr, si4 *si4_arr, si8 len, sf8 scale);
void    	CMP_show_block_header_m12(CMP_BLOCK_FIXED_HEADER_m12 *block_header);
void    	CMP_show_block_model_m12(CMP_PROCESSING_STRUCT_m12 *cps, TERN_m12 recursed_call);
void      	CMP_si4_to_sf8_m12(si4 *si4_arr, sf8 *sf8_arr, si8 len);
sf8		*CMP_spline_interp_sf8_m12(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len, CMP_BUFFERS_m12 *spline_bufs);
si4		*CMP_spline_interp_si4_m12(si4 *in_data, si8 in_len, si4 *out_data, si8 out_len, CMP_BUFFERS_m12 *spline_bufs);
sf8		CMP_splope_m12(sf8 *xa, sf8 *ya, sf8 *d2y, sf8 x, si8 lo_pt, si8 hi_pt);
sf8		CMP_trace_amplitude_m12(sf8 *y, sf8 *buffer, si8 len, TERN_m12 detrend);
si8             CMP_ts_sort_m12(si4 *x, si8 len, CMP_NODE_m12 *nodes, CMP_NODE_m12 *head, CMP_NODE_m12 *tail, si4 return_sorted_ts, ...);
void		CMP_unlock_buffers_m12(CMP_BUFFERS_m12 *buffers);
void    	CMP_unscale_amplitude_si4_m12(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor);
void    	CMP_unscale_amplitude_sf8_m12(sf8 *input_buffer, sf8 *output_buffer, si8 len, sf8 scale_factor);
void    	CMP_unscale_frequency_si4_m12(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor);
CMP_BLOCK_FIXED_HEADER_m12 *CMP_update_CPS_pointers_m12(FILE_PROCESSING_STRUCT_m12 *fps, ui1 flags);
void		CMP_VDS_decode_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void		CMP_VDS_encode_m12(CMP_PROCESSING_STRUCT_m12 *cps);
void		CMP_VDS_generate_template_m12(CMP_PROCESSING_STRUCT_m12 *cps, si8 data_len);
sf8		CMP_VDS_get_theshold_m12(CMP_PROCESSING_STRUCT_m12 *cps);
sf8		CMP_z2p_m12(sf8 z);
void    	CMP_zero_buffers_m12(CMP_BUFFERS_m12 *buffers);



//**********************************************************************************//
//**************************************  CRC  *************************************//
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
#define CRC_BYTES_m12           4
#define CRC_TABLES_m12          8
#define CRC_TABLE_ENTRIES_m12   256
#define CRC_POLYNOMIAL_m12      ((ui4) 0xEDB88320)    // note library CRC routines are customized to this polynomial, it cannot be changed arbitrarily
#define CRC_START_VALUE_m12     ((ui4) 0x0)

// CRC Modes
#define CRC_NO_ENTRY_m12                CRC_START_VALUE_m12
#define CRC_IGNORE_m12                  0
#define CRC_VALIDATE_m12                1
#define CRC_VALIDATE_ON_INPUT_m12       2
#define CRC_VALIDATE_ON_OUTPUT_m12      4
#define CRC_CALCULATE_m12               8
#define CRC_CALCULATE_ON_INPUT_m12      16
#define CRC_CALCULATE_ON_OUTPUT_m12     32

// Macros
#define CRC_SWAP32_m12(q)       ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

// Function Prototypes
ui4		CRC_calculate_m12(const ui1 *block_ptr, si8 block_bytes);
ui4		CRC_combine_m12(ui4 block_1_crc, ui4 block_2_crc, si8 block_2_bytes);
TERN_m12	CRC_initialize_tables_m12(void);
void		CRC_matrix_square_m12(ui4 *square, const ui4 *mat);
ui4		CRC_matrix_times_m12(const ui4 *mat, ui4 vec);
ui4		CRC_update_m12(const ui1 *block_ptr, si8 block_bytes, ui4 current_crc);
TERN_m12	CRC_validate_m12(const ui1 *block_ptr, si8 block_bytes, ui4 crc_to_validate);



//**********************************************************************************//
//************************************  UTF-8  *************************************//
//**********************************************************************************//

// ATTRIBUTION
//
// Basic UTF-8 manipulation routines
// by Jeff Bezanson
// placed in the public domain Fall 2005
//
// "This code is designed to provide the utilities you need to manipulate
// UTF-8 as an internal string encoding. These functions do not perform the
// error checking normally needed when handling UTF-8 data, so if you happen
// to be from the Unicode Consortium you will want to flay me alive.
// I do this because error checking can be performed at the boundaries (I/O),
// with these routines reserved for higher performance on data known to be
// valid."
//
// downloaded from http://www.cprogramming.com
//
// Minor modifications for compatibility with the MED Library.

// Defines
#define UTF8_BUFFER_SIZE	2048

// Macros
#define UTF8_ISUTF_m12(c)       (((c) & 0xC0) != 0x80) // true if c is the start of a UTF-8 sequence

#define UTF8_OFFSETS_TABLE_ENTRIES_m12	6
#define UTF8_OFFSETS_TABLE_m12        { 0x0UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL }

#define UTF8_TRAILING_BYTES_TABLE_ENTRIES_m12	256
#define UTF8_TRAILING_BYTES_TABLE_m12	      {	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
						1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, \
						2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5 }

// Function Prototypes
si4		UTF8_char_num_m12(si1 *s, si4 offset);  // byte offset to character number
void		UTF8_dec_m12(si1 *s, si4 *i);  // move to previous character
si4		UTF8_escape_m12(si1 *buf, si4 sz, si1 *src, si4 escape_quotes);  // convert UTF-8 "src" to ASCII with escape sequences.
si4		UTF8_escape_wchar_m12(si1 *buf, si4 sz, ui4 ch);  // given a wide character, convert it to an ASCII escape sequence stored in buf, where buf is "sz" bytes. returns the number of characters output
si4		UTF8_fprintf_m12(FILE *stream, si1 *fmt, ...);  // fprintf() where the format string and arguments may be in UTF-8. You can avoid this function and just use ordinary fprintf() if the current locale is UTF-8.
si4		UTF8_hex_digit_m12(si1 c);  // utility predicates used by the above
void		UTF8_inc_m12(si1 *s, si4 *i);  // move to next character
TERN_m12	UTF8_initialize_tables_m12(void);
si4		UTF8_is_locale_utf8_m12(si1 *locale);  // boolean function returns if locale is UTF-8, 0 otherwise
TERN_m12	UTF8_is_valid_m12(si1 *string, TERN_m12 zero_invalid, si1 *field_name);
si1		*UTF8_memchr_m12(si1 *s, ui4 ch, si4 sz, si4 *char_num);  // same as the above, but searches a buffer of a given size instead of a NUL-terminated string.
ui4		UTF8_next_char_m12(si1 *s, si4* i);  // return next character, updating an index variable
si4		UTF8_octal_digit_m12(si1 c);  // utility predicates used by the above
si4		UTF8_offset_m12(si1 *str, si4 char_num);  // character number to byte offset
si4    		UTF8_printf_m12(si1 *fmt, ...);  // printf() where the format string and arguments may be in UTF-8. You can avoid this function and just use ordinary printf() if the current locale is UTF-8.
si4		UTF8_read_escape_sequence_m12(si1 *str, ui4 *dest);  // assuming str points to the character after a backslash, read an escape sequence, storing the result in dest and returning the number of input characters processed
si4		UTF8_seqlen_m12(si1 *s);  // returns length of next UTF-8 sequence
si1		*UTF8_strchr_m12(si1 *s, ui4 ch, si4 *char_num);  // return a pointer to the first occurrence of ch in s, or NULL if not found. character index of found character returned in *char_num.
si4		UTF8_strlen_m12(si1 *s);  // count the number of characters in a UTF-8 string
si4		UTF8_to_ucs_m12(ui4 *dest, si4 sz, si1 *src, si4 srcsz);  // convert UTF-8 data to wide character
si4		UTF8_to_utf8_m12(si1 *dest, si4 sz, ui4 *src, si4 srcsz);  // convert wide character to UTF-8 data
si4		UTF8_unescape_m12(si1 *buf, si4 sz, si1 *src);  // convert a string "src" containing escape sequences to UTF-8 if escape_quotes is nonzero, quote characters will be preceded by  backslashes as well.
si4		UTF8_vfprintf_m12(FILE *stream, si1 *fmt, va_list ap);    // called by UTF8_fprintf()
si4		UTF8_vprintf_m12(si1 *fmt, va_list ap);  // called by UTF8_printf()
si4		UTF8_wc_to_utf8_m12(si1 *dest, ui4 ch);  // single character to UTF-8



//**********************************************************************************//
//*************************************  AES-128  **********************************//
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

#define AES_KEY_BYTES_m12		PASSWORD_BYTES_m12
#define AES_EXPANDED_KEY_BYTES_m12	ENCRYPTION_KEY_BYTES_m12  // AES-128 == ((AES_NR + 1) * AES_NK * AES_NB)

#define AES_NR_m12	        10	// The number of rounds in AES Cipher
#define AES_NK_m12	        4	// The number of 32 bit words in the key
#define AES_NB_m12	        4	// The number of columns comprising a state in AES. This is a constant in AES.
#define AES_XTIME_m12(x)        ((x << 1) ^ (((x >> 7) & 1) * 0x1b)) // AES_XTIME is a macro that finds the product of {02} and the argument to AES_XTIME modulo {1b}
#define AES_MULTIPLY_m12(x, y)  (((y & 1) * x) ^ ((y >> 1 & 1) * AES_XTIME_m12(x)) ^ ((y >> 2 & 1) * AES_XTIME_m12(AES_XTIME_m12(x))) ^ \
				((y >> 3 & 1) * AES_XTIME_m12(AES_XTIME_m12(AES_XTIME_m12(x)))) ^ ((y >> 4 & 1) * \
				AES_XTIME_m12(AES_XTIME_m12(AES_XTIME_m12(AES_XTIME_m12(x)))))) // Multiply is a macro used to multiply numbers in the field GF(2^8)

#define AES_SBOX_ENTRIES_m12	256
#define AES_SBOX_m12          {	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, \
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

#define AES_RSBOX_ENTRIES_m12	256
#define AES_RSBOX_m12         {	0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, \
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

#define AES_RCON_ENTRIES_m12	255
#define AES_RCON_m12          {	0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, \
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
void		AES_add_round_key_m12(si4 round, ui1 state[][4], ui1 *round_key);
void		AES_cipher_m12(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key);
void		AES_decrypt_m12(ui1 *data, si8 len, si1 *password, ui1 *expanded_key);
void		AES_encrypt_m12(ui1 *data, si8 len, si1 *password, ui1 *expanded_keyy);
void		AES_key_expansion_m12(ui1 *round_key, si1 *key);
si4		AES_get_sbox_invert_m12(si4 num);
si4		AES_get_sbox_value_m12(si4 num);
TERN_m12	AES_initialize_tables_m12(void);
void		AES_inv_cipher_m12(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key);
void		AES_inv_mix_columns_m12(ui1 state[][4]);
void		AES_inv_shift_rows_m12(ui1 state[][4]);
void		AES_inv_sub_bytes_m12(ui1 state[][4]);
void		AES_leftover_decrypt_m12(si4 n_leftovers, ui1 *data);
void		AES_leftover_encrypt_m12(si4 n_leftovers, ui1 *data);
void		AES_mix_columns_m12(ui1 state[][4]);
void		AES_shift_rows_m12(ui1 state[][4]);
void		AES_sub_bytes_m12(ui1 state[][4]);


//***********************************************************************//
//**************************  SHA-256 FUNCTIONS  ************************//
//***********************************************************************//

// ATTRIBUTION:
//
// Author:	Brad Conte (brad@bradconte.com)
// Disclaimer:	This code is presented "as is" without any guarantees.
// Details:	Implementation of the SHA-256 hashing algorithm.
//		Algorithm specification can be found here:
//	      	http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
//		This implementation uses little endian byte order.
//
// Code:	https://github.com/B-Con/crypto-algorithms/blob/master/sha256.c
//
// Only SHA-256 functions are included in the MED library.
// The version below contains minor modifications for compatibility with the MED Library.


// Constants
#define SHA_HASH_BYTES_m12	32  // 256 bit
#define SHA_LOW_BYTE_MASK_m12	(ui4) 0x000000FF

#define	SHA_H0_ENTRIES_m12	8
#define	SHA_H0_m12            {	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 }

#define	SHA_K_ENTRIES_m12	64
#define	SHA_K_m12	      {	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, \
       				0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, \
       				0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, \
       				0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, \
       				0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, \
       				0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, \
       				0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, \
				0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 }

// Macros
#define SHA_ROTLEFT_m12(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define SHA_ROTRIGHT_m12(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define SHA_CH_m12(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define SHA_MAJ_m12(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA_EP0_m12(x) (SHA_ROTRIGHT_m12(x,2) ^ SHA_ROTRIGHT_m12(x,13) ^ SHA_ROTRIGHT_m12(x,22))
#define SHA_EP1_m12(x) (SHA_ROTRIGHT_m12(x,6) ^ SHA_ROTRIGHT_m12(x,11) ^ SHA_ROTRIGHT_m12(x,25))
#define SHA_SIG0_m12(x) (SHA_ROTRIGHT_m12(x,7) ^ SHA_ROTRIGHT_m12(x,18) ^ ((x) >> 3))
#define SHA_SIG1_m12(x) (SHA_ROTRIGHT_m12(x,17) ^ SHA_ROTRIGHT_m12(x,19) ^ ((x) >> 10))

// Typedefs & Structures
typedef struct {
	ui1	data[64];
	ui4	state[8];
	ui8	bitlen;
	ui4	datalen;
} SHA_CTX_m12;

// Function Prototypes
void		SHA_finalize_m12(SHA_CTX_m12 *ctx, ui1 *hash);
ui1    		*SHA_hash_m12(const ui1 *data, si8 len, ui1 *hash);
void		SHA_initialize_m12(SHA_CTX_m12 *ctx);
TERN_m12	SHA_initialize_tables_m12(void);
void		SHA_transform_m12(SHA_CTX_m12 *ctx, const ui1 *data);
void		SHA_update_m12(SHA_CTX_m12 *ctx, const ui1 *data, si8 len);



//**********************************************************************************//
//************************************  FILTER  ************************************//
//**********************************************************************************//

// ATTRIBUTION
//
// Some of the filter code was adapted from Matlab functions (MathWorks, Inc).
// www.mathworks.com
//
// The c code herein was written entirely from scratch.


// Constants
#define FILT_LOWPASS_TYPE_m12                   	1
#define FILT_BANDPASS_TYPE_m12                  	2
#define FILT_HIGHPASS_TYPE_m12                  	3
#define FILT_BANDSTOP_TYPE_m12                  	4
#define FILT_TYPE_DEFAULT_m12                   	FILT_LOWPASS_TYPE_m12
#define FILT_ORDER_DEFAULT_m12                  	5
#define FILT_PAD_SAMPLES_PER_POLE_m12			3  // minimum == 3
#define FILT_MAX_ORDER_m12                      	10
#define FILT_BAD_FILTER_m12                     	-1
#define FILT_BAD_DATA_m12                       	-2
#define FILT_EPS_SF8_m12                        	((sf8) 2.22045e-16)
#define FILT_RADIX_m12                          	((sf8) 2.0)
#define FILT_LINE_NOISE_HARMONICS_DEFAULT_m12   	4
#define FILT_ANTIALIAS_FREQ_DIVISOR_DEFAULT_m12 	((sf8) 3.5);
#define FILT_UNIT_THRESHOLD_DEFAULT_m12			CMP_PARAMETERS_VDS_UNIT_THRESHOLD_DEFAULT_m12
#define FILT_NFF_BUFFERS_m12				4
#define FILT_VDS_TEMPLATE_MIN_PS_m12			0  // index of CPS filtps
#define FILT_VDS_TEMPLATE_LFP_PS_m12			1  // index of CPS filtps
#define	FILT_VDS_MIN_SAMPS_PER_CYCLE_m12		((sf8) 4.5)  // rolloff starts at ~5 samples per cycle

// Quantfilt Tail Options
#define FILT_TRUNCATE_m12                        1
#define FILT_EXTRAPOLATE_m12                     2
#define FILT_ZEROPAD_m12                         3
#define FILT_DEFAULT_TAIL_OPTION_CODE_m12        FILT_TRUNCATE_m12

// Macros
#define FILT_ABS_m12(x)             		((x) >= ((sf8) 0.0) ? (x) : (-x))
#define FILT_SIGN_m12(x, y)         		((y) >= ((sf8) 0.0) ? FILT_ABS_m12(x) : -FILT_ABS_m12(x))
// filtps->n_poles = poles = n_fcs * order;
#define FILT_POLES_m12(order, cutoffs)		(order * cutoffs)
#define FILT_FILT_PAD_SAMPLES_m12(poles)	(poles * FILT_PAD_SAMPLES_PER_POLE_m12 * 2)
#define FILT_OFFSET_ORIG_DATA_m12(filtps)	(filtps->filt_data + (filtps->n_poles * FILT_PAD_SAMPLES_PER_POLE_m12))

// Typedefs & Structs
typedef struct {
	ui4	behavior_on_fail;
	si4	order;
	si4	n_poles;  // n_poles == order * n_cutoffs
	si4	type;
	sf8	sampling_frequency;
	si8	data_length;
	sf8	cutoffs[2];
	sf8	*numerators;  // entries == n_poles + 1
	sf8	*denominators;  // entries == n_poles + 1
	sf8	*initial_conditions;  // entries == n_poles
	sf8	*orig_data;
	sf8	*filt_data;
	sf8	*buffer;
} FILT_PROCESSING_STRUCT_m12;

typedef struct {
	sf8	real;
	sf8	imag;
} FILT_COMPLEX_m12;

typedef struct FILT_NODE_STRUCT {
	sf8			val;
	struct FILT_NODE_STRUCT *prev, *next;
} FILT_NODE_m12;

typedef struct {
	si1		tail_option_code;
	si8 		len, span, in_idx, out_idx, oldest_idx;
	sf8 		*x, *qx, quantile, low_val_q, high_val_q;
	FILT_NODE_m12	*nodes, head, tail, *oldest_node, *curr_node;
} QUANTFILT_DATA_m12;


// Prototypes
QUANTFILT_DATA_m12	*FILT_alloc_quantfilt_data_m12(si8 len, si8 span);
void    FILT_balance_m12(sf8 **a, si4 poles);
si4     FILT_butter_m12(FILT_PROCESSING_STRUCT_m12 *filtps);
void    FILT_complex_div_m12(FILT_COMPLEX_m12 *a, FILT_COMPLEX_m12 *b, FILT_COMPLEX_m12 *quotient);
void    FILT_complex_exp_m12(FILT_COMPLEX_m12 *exponent, FILT_COMPLEX_m12 *ans);
void    FILT_complex_mult_m12(FILT_COMPLEX_m12 *a, FILT_COMPLEX_m12 *b, FILT_COMPLEX_m12 *product);
void    FILT_elmhes_m12(sf8 **a, si4 poles);
void	FILT_excise_transients_m12(CMP_PROCESSING_STRUCT_m12 *cps, si8 data_len, si8 *n_extrema);
si4     FILT_filtfilt_m12(FILT_PROCESSING_STRUCT_m12 *filtps);
void	FILT_free_CPS_filtps_m12(CMP_PROCESSING_STRUCT_m12 *cps, TERN_m12 free_orig_data, TERN_m12 free_filt_data, TERN_m12 free_buffer);
void    FILT_free_processing_struct_m12(FILT_PROCESSING_STRUCT_m12 *filtps, TERN_m12 free_orig_data, TERN_m12 free_filt_data, TERN_m12 free_buffer, TERN_m12 free_structure);
void	FILT_free_quantfilt_data_m12(QUANTFILT_DATA_m12 *qd, TERN_m12 free_structure);
FILT_PROCESSING_STRUCT_m12  *FILT_initialize_processing_struct_m12(si4 order, si4 type, sf8 samp_freq, si8 data_len, TERN_m12 alloc_orig_data, TERN_m12 alloc_filt_data, TERN_m12 alloc_buffer, ui4 behavior_on_fail, sf8 cutoff_1, ...);
void    FILT_generate_initial_conditions_m12(FILT_PROCESSING_STRUCT_m12 *filtps);
void    FILT_hqr_m12(sf8 **a, si4 poles, FILT_COMPLEX_m12 *eigs);
void    FILT_invert_matrix_m12(sf8 **a, sf8 **inv_a, si4 order);
sf8	FILT_line_noise_filter_m12(sf8 *y, sf8 *fy, si8 len, sf8 samp_freq, sf8 line_freq, si8 cycles_per_template, TERN_m12 calculate_score, TERN_m12 fast_mode, CMP_BUFFERS_m12 *lnf_buffers);
void    FILT_mat_mult_m12(void *a, void *b, void *product, si4 outer_dim1, si4 inner_dim, si4 outer_dim2);
sf8	*FILT_moving_average_m12(sf8 *x, sf8 *ax, si8 len, si8 span, si1 tail_option_code);
sf8    	*FILT_noise_floor_filter_m12(sf8 *data, sf8 *filt_data, si8 data_len, sf8 rel_thresh, sf8 abs_thresh, CMP_BUFFERS_m12 *nff_buffers);
sf8	*FILT_quantfilt_m12(sf8 *x, sf8 *qx, si8 len, sf8 quantile, si8 span, si1 tail_option_code);
QUANTFILT_DATA_m12	*FILT_quantfilt_head_m12(QUANTFILT_DATA_m12 *qd, ...);  // varargs: sf8 *x, sf8 *qx, si8 len, sf8 quantile, si8 span, si4 tail_option_code
void	FILT_quantfilt_mid_m12(QUANTFILT_DATA_m12 *qd);
void	FILT_quantfilt_tail_m12(QUANTFILT_DATA_m12 *qd);
si4     FILT_sf8_sort_m12(const void *n1, const void *n2);
void	FILT_show_processing_struct_m12(FILT_PROCESSING_STRUCT_m12 *filt_ps);
void    FILT_unsymmeig_m12(sf8 **a, si4 poles, FILT_COMPLEX_m12 *eigs);



//**********************************************************************************//
//*******************************  DATA MATRIX (DM) ********************************//
//**********************************************************************************//

//	Extent Mode (EXTMD) Flags:
//
//	DM_EXTMD_ABSOLUTE_LIMITS_m12: all samples that exist between slice is start & end (specified by either time or sample number) are returned
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
//	DM_EXTMD_RELATIVE_LIMITS_m12: slice start is treated as absolute, slice end is treated as indication of desired slice extents as follows:
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
//	DM_EXTMD_SAMP_COUNT_m12: output sample_count specified when passing matrix
//	DM_EXTMD_SAMP_FREQ_m12: output sampling frequency specified when passing matrix ( == valid_samples / valid_time in slice - not affected by discontinuities)
//
//	These two modes are driven by to general use cases:
//	1) want matrix of this specific size (e.g. viewer window where sample count == pixel columns)
//	2) want all channels at the specified sampling frequency (e.g algorithm looking across channels)


// Flag Definitions:
#define DM_NO_FLAGS_m12				((ui8) 0)
#define DM_TYPE_SI2_m12				((ui8) 1 << 1)
#define DM_TYPE_SI4_m12				((ui8) 1 << 2)
#define DM_TYPE_SF4_m12				((ui8) 1 << 3)
#define DM_TYPE_SF8_m12				((ui8) 1 << 4)
#define DM_TYPE_MASK_m12	              (	DM_TYPE_SI2_m12 | DM_TYPE_SI4_m12 | DM_TYPE_SF4_m12 | DM_TYPE_SF8_m12 )
#define DM_2D_INDEXING_m12			((ui8) 1 << 7)		// include array of pointers so that matrix[x][y] indexing is possible (expensive with large major dinensions)
#define DM_FMT_SAMPLE_MAJOR_m12			((ui8) 1 << 8)
#define DM_FMT_CHANNEL_MAJOR_m12		((ui8) 1 << 9)
#define DM_FMT_MASK_m12	   	              (	DM_FMT_SAMPLE_MAJOR_m12 | DM_FMT_CHANNEL_MAJOR_m12 )
#define DM_EXTMD_SAMP_COUNT_m12			((ui8) 1 << 12)
#define DM_EXTMD_SAMP_FREQ_m12			((ui8) 1 << 13)
#define DM_EXTMD_MASK_m12	              (	DM_EXTMD_SAMP_COUNT_m12 | DM_EXTMD_SAMP_FREQ_m12 )
#define DM_EXTMD_ABSOLUTE_LIMITS_m12		((ui8) 1 << 14)
#define DM_EXTMD_RELATIVE_LIMITS_m12		((ui8) 1 << 15)
#define DM_EXTMD_LIMIT_MASK_m12	              (	DM_EXTMD_ABSOLUTE_LIMITS_m12 | DM_EXTMD_RELATIVE_LIMITS_m12 )
#define DM_SCALE_m12				((ui8) 1 << 18)		// output multiplied by this number (unless 0 or 1)
#define DM_FILT_LOWPASS_m12			((ui8) 1 << 20)		// low cutoff == vararg 1
#define DM_FILT_HIGHPASS_m12			((ui8) 1 << 21)		// high cutoff == vararg 1
#define DM_FILT_BANDPASS_m12			((ui8) 1 << 22)		// low cutoff == vararg 1, high cutoff == vararg 2
#define DM_FILT_BANDSTOP_m12			((ui8) 1 << 23)		// low cutoff == vararg 1, high cutoff == vararg 2
#define DM_FILT_ANTIALIAS_m12			((ui8) 1 << 24)		// lowpass with high cutoff computed, no varargs
#define DM_FILT_CUTOFFS_MASK_m12      	      ( DM_FILT_LOWPASS_m12 | DM_FILT_HIGHPASS_m12 | DM_FILT_BANDPASS_m12 | DM_FILT_BANDSTOP_m12 )
#define DM_FILT_MASK_m12	      	      ( DM_FILT_CUTOFFS_MASK_m12 | DM_FILT_ANTIALIAS_m12 )
#define DM_INTRP_LINEAR_m12			((ui8) 1 << 28)
#define DM_INTRP_MAKIMA_m12			((ui8) 1 << 29)
#define DM_INTRP_SPLINE_m12			((ui8) 1 << 30)
#define DM_INTRP_UP_MAKIMA_DN_LINEAR_m12	((ui8) 1 << 31)
#define DM_INTRP_MAKIMA_UPSAMPLE_SF_RATIO_m12	((sf8) 1.5)
#define DM_INTRP_UP_SPLINE_DN_LINEAR_m12	((ui8) 1 << 32)		// if sampling frequency ratio >= DM_INTRP_SPLINE_UPSAMPLE_SF_RATIO_m12
#define DM_INTRP_SPLINE_UPSAMPLE_SF_RATIO_m12	CMP_SPLINE_UPSAMPLE_SF_RATIO_m12	// require (out_sf / in_sf) be >= this before spline upsampling, if lower use linear (prevents unnatural spline turns)
#define DM_INTRP_BINTRP_MDPT_m12		((ui8) 1 << 33)		// binterpolate with midpoint center mode
#define DM_INTRP_BINTRP_MEAN_m12		((ui8) 1 << 34)		// binterpolate with mean center mode
#define DM_INTRP_BINTRP_MEDN_m12		((ui8) 1 << 35)		// binterpolate with median center mode
#define DM_INTRP_BINTRP_FAST_m12		((ui8) 1 << 36)		// binterpolate with fast center mode
#define DM_INTRP_BINTRP_MASK_d1		      ( DM_INTRP_BINTRP_MDPT_m12 | DM_INTRP_BINTRP_MEAN_m12 | DM_INTRP_BINTRP_MEDN_m12 | DM_INTRP_BINTRP_FAST_m12 )
#define DM_INTRP_MASK_m12	              (	DM_INTRP_LINEAR_m12 | DM_INTRP_MAKIMA_m12 | DM_INTRP_SPLINE_m12 | DM_INTRP_UP_MAKIMA_DN_LINEAR_m12 | DM_INTRP_UP_SPLINE_DN_LINEAR_m12 | DM_INTRP_BINTRP_MASK_d1 )
#define DM_TRACE_RANGES_m12			((ui8) 1 << 40)		// return bin minima & maxima (equal in size, type, & format to data matrix)
#define DM_TRACE_EXTREMA_m12			((ui8) 1 << 41)		// return minima & maxima values in put traces as two arrays (minimum & maximum per channel, same type as data matrix)
#define DM_DETREND_m12				((ui8) 1 << 42)		// detrend traces (and trace range matrices if DM_TRACE_RANGES_m12 is set)
#define DM_DSCNT_CONTIG_m12			((ui8) 1 << 48)		// return contiguons
#define DM_DSCNT_NAN_m12			((ui8) 1 << 49)		// fill absent samples with NaNs (locations specified in returned arrays)
									// si2: NAN_SI2_m12 (0x8000)
									// si4: NAN_SI4_m12 (0x80000000)
									// sf4: NAN == nanf("")
									// sf8: NAN == nan("")
#define DM_DSCNT_ZERO_m12			((ui8) 1 << 50)		// fill absent samples with zeros (locations specified in returned arrays)
#define DM_PAD_MASK_m12	              	      (	DM_DSCNT_NAN_m12 | DM_DSCNT_ZERO_m12 )
#define DM_DSCNT_MASK_m12	              (	DM_DSCNT_CONTIG_m12 | DM_PAD_MASK_m12 )

// Non-flag defines
#define DM_MAXIMUM_INPUT_FREQUENCY_m12		((sf8) -3.0)	// value chosen to distinguish from FREQUENCY_NO_ENTRY_m12 (-1.0) & FREQUENCY_VARIABLE_m12 (-2.0)
#define DM_MAXIMUM_INPUT_COUNT_m12		((si8) -3)	// value chosen to parallel DM_MAXIMUM_INPUT_FREQUENCY_m12 & not conflict with NUMBER_OF_SAMPLES_NO_ENTRY_m12 (-1)


// Note: if arrays are allocted as 2D arrays, array[0] is beginning of one dimensional array containing (channel_count * sample_count) values of specfified type
typedef struct {
	si8		channel_count;		// defines dimension of allocated matrix: updated based on active channels
	si8		sample_count;		// defines dimension of allocated matrix: if extent mode (EXTMD) == DM_EXTMD_SAMP_FREQ_m12, resultant sample count filled in
	sf8		sampling_frequency;	// defines dimension of allocated matrix: if extent mode (EXTMD) == DM_EXTMD_SAMP_COUNT_m12, resultant sampling frequenc filled in
	sf8		scale_factor;		// optionally passed (can be passed in varargs), always returned
	sf8		filter_low_fc;		// optionally passed (can be passed in varargs), always returned
	sf8		filter_high_fc;		// optionally passed (can be passed in varargs), always returned
	void		*data;			// alloced / realloced as needed   (cast to type * or type **, if DM_2D_INDEXING_m12 is set)
	void		*range_minima;		// alloced / realloced as needed, present if DM_TRACE_RANGES_m12 bit is set, otherwise NULL   (cast to type * or type **, if DM_2D_INDEXING_m12 is set)
	void		*range_maxima;		// alloced / realloced as needed, present if DM_TRACE_RANGES_m12 bit is set, otherwise NULL   (cast to type * or type **, if DM_2D_INDEXING_m12 is set)
	void		*trace_minima;  	// alloced / realloced as needed, present if DM_TRACE_EXTREMA_m12 bit is set, otherwise NULL   (cast to type *)
	void		*trace_maxima;  	// alloced / realloced as needed, present if DM_TRACE_EXTREMA_m12 bit is set, otherwise NULL   (cast to type *)
	si4		number_of_contigua;
	CONTIGUON_m12	*contigua;		// sample indexes in matrix frame
	// internal processing elements //
	si8			valid_sample_count;		// used with padding options
	ui8			flags;
	si8			maj_dim;
	si8			min_dim;
	si8			el_size;
	si8			data_bytes;
	si8			n_proc_bufs;
	CMP_BUFFERS_m12		**in_bufs;
	CMP_BUFFERS_m12		**out_bufs;
	CMP_BUFFERS_m12		**mak_in_bufs;
	CMP_BUFFERS_m12		**mak_out_bufs;
	CMP_BUFFERS_m12		**spline_bufs;
} DATA_MATRIX_m12;

typedef struct {
	DATA_MATRIX_m12	*dm;
	CHANNEL_m12	*chan;
	si8		chan_idx;
} DM_CHANNEL_THREAD_INFO_m12;


// Prototypes
pthread_rval_m12	DM_channel_thread_m12(void *ptr);
void			DM_free_matrix_m12(DATA_MATRIX_m12 *matrix, TERN_m12 free_structure);
DATA_MATRIX_m12 	*DM_get_matrix_m12(DATA_MATRIX_m12 *matrix, SESSION_m12 *sess, TIME_SLICE_m12 *slice, si4 varargs, ...);  // can't use TERN_m12 to flag varargs (undefined behavior)
// DM_get_matrix_m12() varargs: si8 sample_count, sf8 sampling_frequency, ui8 flags, sf8 scale, sf8 fc1, sf8 fc2
//
// IMPORTANT: pass correct types for varargs - compiler cannot promote / convert to proper type because it doesn't know what they should be
//
// varargs DM_FILT_LOWPASS_m12 set: fc1 == high_cutoff
// varargs DM_FILT_HIGHPASS_m12 set: fc1 == low_cutoff
// varargs DM_FILT_BANDPASS_m12 set: fc1 == low_cutoff, fc2 == high_cutoff
// varargs DM_FILT_BANDSTOP_m12 set: fc1 == low_cutoff, fc2 == high_cutoff
void			DM_show_flags_m12(ui8 flags);
DATA_MATRIX_m12		*DM_transpose_m12(DATA_MATRIX_m12 **in_matrix, DATA_MATRIX_m12 **out_matrix);  // if *in_matrix == *out_matrix, done in place; if *out_matrix == NULL, allocated and returned
void			DM_transpose_in_place_m12(DATA_MATRIX_m12 *matrix, void *base);
void			DM_transpose_out_of_place_m12(DATA_MATRIX_m12 *in_matrix, DATA_MATRIX_m12 *out_matrix, void *in_base, void *out_base);  // used by DM_transpose_m12(), assumes array allocation is taken care of, so use independently with care



//**********************************************************************************//
//*******************************  TRANSMISSION (TR)  ******************************//
//**********************************************************************************//

// Transmission Header Types
// • Type numbers 0-63 reserved for generic transmission types
// • Type numbers 64-255 used for application specific transmission types

#define TR_TYPE_GENERIC_MIN_m12					0
#define TR_TYPE_GENERIC_MAX_m12					63
#define TR_TYPE_APPLICATION_MIN_m12				64
#define TR_TYPE_APPLICATION_MAX_m12				255

// Generic Transmission Header (TH) Types
#define TR_TYPE_NO_ENTRY_m12					((ui1) 0)
#define TR_TYPE_KEEP_ALIVE_m12					((ui1) 1)  // discarded if received by recv_transmission(), and waits for next transmission
#define TR_TYPE_ACK_OK_m12					((ui1) 2)
#define TR_TYPE_ACK_RETRANSMIT_m12				((ui1) 3)
#define TR_TYPE_MESSAGE_m12					((ui1) 4)
#define TR_TYPE_OPERATION_SUCCEEDED_m12				((ui1) 5)
#define TR_TYPE_OPERATION_SUCCEEDED_WITH_MESSAGE_m12		((ui1) 6)
#define TR_TYPE_OPERATION_FAILED_m12				((ui1) 7)
#define TR_TYPE_OPERATION_FAILED_WITH_WARNING_MESSAGE_m12	((ui1) 8)
#define TR_TYPE_OPERATION_FAILED_WITH_ERROR_MESSAGE_m12		((ui1) 9)

// Header Message Type Aliases (shorter :)
#define TR_ERROR_TYPE_m12	TR_TYPE_OPERATION_FAILED_WITH_ERROR_MESSAGE_m12
#define TR_WARNING_TYPE_m12	TR_TYPE_OPERATION_FAILED_WITH_WARNING_MESSAGE_m12
#define TR_SUCCESS_TYPE_m12	TR_TYPE_OPERATION_SUCCEEDED_WITH_MESSAGE_m12
#define TR_MESSAGE_TYPE_m12	TR_TYPE_MESSAGE_m12

// Transmission Error Codes
#define TR_E_NO_ERR_m12			(si8) E_NO_ERR_m12  // 0
#define TR_E_UNSPEC_m12			(si8) FALSE_m12
#define TR_E_SOCK_FAILED_m12		(si8) -2
#define TR_E_SOCK_NO_OPEN_m12		(si8) -3
#define TR_E_SOCK_CLOSED_m12		(si8) -4
#define TR_E_SOCK_TIMED_OUT_m12		(si8) -5
#define TR_E_NO_DATA_m12		(si8) -6
#define TR_E_ID_MISMATCH_m12		(si8) -7
#define TR_E_TRANS_FAILED_m12		(si8) -8
#define TR_E_CRC_MISMATCH_m12		(si8) -9
#define TR_E_NO_ACK_m12			(si8) -10

// Transmission Error Strings
#define	TR_E_NO_ERR_STR_m12		"no error"
#define	TR_E_UNSPEC_STR_m12		"unspecified transmission error"
#define TR_E_SOCK_FAILED_STR_m12	"socket failed"
#define TR_E_SOCK_NO_OPEN_STR_m12	"could not open socket"
#define TR_E_SOCK_CLOSED_STR_m12	"socket closed"
#define TR_E_SOCK_TIMED_OUT_STR_m12	"socket timed out"
#define TR_E_NO_DATA_STR_m12		"no data available"
#define TR_E_ID_MISMATCH_STR_m12	"transmission ID mismatch"
#define TR_E_TRANS_FAILED_STR_m12	"transmission failed"
#define TR_E_CRC_MISMATCH_STR_m12	"checksum mismatch"
#define TR_E_NO_ACK_STR_m12		"no acknowlegment"

// Transmission Flags
#define TR_FLAGS_DEFAULT_m12			((ui2) 0)
#define TR_FLAGS_BIG_ENDIAN_m12			((ui2) 1)       // Bit 0  (LITTLE_ENDIAN == 0, BIG_ENDIAN == 1)
#define TR_FLAGS_UDP_m12			((ui2) 1 << 1)	// Bit 1  (TCP == 0, UDP == 1)
#define TR_FLAGS_ENCRYPT_m12			((ui2) 1 << 2)	// Bit 2  (body only - header is not encrypted)
#define TR_FLAGS_INCLUDE_KEY_m12		((ui2) 1 << 3)  // Bit 3  (expanded encryption key included in data - less secure than bilateral prescience of key)
#define TR_FLAGS_CLOSE_m12			((ui2) 1 << 4)	// Bit 4  (close socket after send/recv)
#define TR_FLAGS_ACKNOWLEDGE_m12		((ui2) 1 << 5)	// Bit 5  (acknowledge receipt with OK or retransmit)
#define TR_FLAGS_CRC_m12			((ui2) 1 << 6)	// Bit 6  (calculate/check transmission CRC - last 4 bytes of transmission)
#define TR_FLAGS_NO_DESTRUCT_m12		((ui2) 1 << 7)	// Bit 7  (set if local memory should not be altered - applies to encrpyted transmissions & transmissions that exceed TR_MTU_BYTES_m12)
#define TR_FLAGS_TO_FILE_m12			((ui2) 1 << 8)	// Bit 8  (set if received data should go to a file rather than buffer - pseudo FTP)

// TR Defaults
#define TR_VERSION_DEFAULT_m12		((ui1) 1)
#define TR_TYPE_DEFAULT_m12		TR_TYPE_NO_ENTRY_m12
#define TR_ID_CODE_NO_ENTRY_m12		0  // ui4
#define TR_ID_CODE_DEFAULT		TR_ID_CODE_NO_ENTRY_m12

// Transmission Message
#define TR_MESSAGE_HEADER_BYTES_m12			16
#define TR_MESSAGE_HEADER_TIME_OFFSET_m12		0	// si8
#define TR_MESSAGE_HEADER_NO_ENTRY_m12			UUTC_NO_ENTRY_m12
#define TR_MESSAGE_HEADER_MESSAGE_BYTES_OFFSET_m12	8	// si8
#define TR_MESSAGE_HEADER_MESSAGE_BYTES_NO_ENTRY_m12	0

// Transmission Header (TH) Format Constants
#define TR_HEADER_BYTES_m12				((si8) 32)
#define TR_CRC_OFFSET_m12				0				// ui4
#define TR_PACKET_BYTES_OFFSET_m12			4				// ui2
#define TR_FLAGS_OFFSET_m12				6	                	// ui2
#define TR_ID_STRING_OFFSET_m12				8	                	// ascii[4]
#define TR_ID_STRING_TERMINAL_ZERO_OFFSET_m12		(TR_ID_STRING_OFFSET_m12 + 4)	// si1
#define TR_ID_CODE_OFFSET_m12				TR_ID_STRING_OFFSET_m12		// ui4
// TR_ID_CODE_NO_ENTRY_m12 defined above
#define TR_TYPE_OFFSET_m12				13	                	// ui1
#define TR_SUBTYPE_OFFSET_m12				14	                	// ui1
#define TR_VERSION_OFFSET_m12				15	                	// ui1
#define TR_VERSION_NO_ENTRY_m12				0
#define TR_TRANSMISSION_BYTES_OFFSET_m12		16				// ui8
#define TR_TRANSMISSION_BYTES_NO_ENTRY_m12		0
#define TR_OFFSET_OFFSET_m12				24				// ui8

// Transmission Info Modes  [set by TR_send_transmission_m12() & TR_recv_transmission_m12(), used by TR_close_transmission_m12()]
// indicates whether last transmission was a send or receive
#define TR_MODE_NONE_m12		0
#define TR_MODE_SEND_m12		1
#define TR_MODE_RECV_m12		2
#define TR_MODE_FORCE_CLOSE_m12		3  // set to force close a (TCP) socket

// Miscellaneous
#define TR_INET_MSS_BYTES_m12				1376  // highest multiple of 16, that stays below internet standard frame size (1500) minus [32 (TR header) + 40 (TCP/IP header)
							      // + some extra (possible intermediary protocols like GRE, IPsec, PPPoE, or SNAP that may be in the route)]
#define TR_LO_MSS_BYTES_m12				65456  // highest multiple of 16, that stays below backplane (loopback) standard frame size (65535) minus [32 (TR header) + 40 (TCP/IP header)])
#define TR_PORT_STRLEN_m12				8
#define TR_TIMEOUT_NEVER_m12				((sf4) 0.0)
#define TR_PORT_ANY_m12					0  // system assigned port
#define TR_IFACE_ANY_m12				((void *) 0)  // all interfaces
#define TR_IFACE_DFLT_m12				""  // default internet interface
#define TR_RETRANSMIT_ATTEMPTS_m12			3


// Typedefs
typedef struct {
	ui4	crc;
	ui2	packet_bytes;  // bytes in this packet (including header)
	ui2	flags;
	union {
		struct {
			si1     ID_string[TYPE_BYTES_m12];  // transmission ID is typically application specific
			ui1     type;  // transmission type (general [0-63] or transmission ID specific [64-255])
			ui1	subtype;  // rarely used
			ui1     version;  // transmission header version
		};
		struct {
			ui4     ID_code;  // transmission ID is typically application specific
			union {
				ui4	combined_check;  // use to to check [zero, type, subtype, version] as a ui4
				struct {
					si1	ID_string_terminal_zero;  // here for clarity
					ui1	pad_bytes[3];  // not available for use (type, type_2, & version above)
				};
			};
		};
	};
	si8	transmission_bytes;  // full size of tramsmitted data in bytes (*** does not include header ***)
	si8	offset;  // offset (in bytes) of packet data into full data (*** does not include header ***)
} TR_HEADER_m12;

typedef struct {
	union {
		ui1		*buffer;  // used internally, first portion is the transmission header
		TR_HEADER_m12	*header;
	};
	si8			buffer_bytes;  // bytes available for data (actual allocation also includes room for header)
	ui1			*data;  // buffer + TR_HEADER_BYTES_m12
	si1			*password;   // for encryption (NOT freed by TR_free_transmission_info_m12)
	ui1			*expanded_key;   // for encryption
	TERN_m12		expanded_key_allocated;  // determines whether to free expanded key
	ui1			mode;  // TR_MODE_SEND_m12, TR_MODE_RECV_m12, TR_MODE_NONE_m12 (needed to properly close TCP sockets)
	si4			sock_fd;
	si1			dest_addr[INET6_ADDRSTRLEN];  // INET6_ADDRSTRLEN == 46 (this can be an IP address string or or a domain name [< 46 characters])
	ui2			dest_port;
	si1			iface_addr[INET6_ADDRSTRLEN];  // zero-length string for any default internet interface
	ui2			iface_port;
	sf4			timeout;  // seconds
	ui2			mss;  // maximum segment size (max bytes of data per packet [*** does not include header ***]) (typically multiple of 16, must be at least multiple of 8 for library)
} TR_INFO_m12;

typedef struct {
	si8	time;		// uutc
	si8	message_bytes;	// includes text, & pad bytes, NOT header bytes
} TR_MESSAGE_HEADER_m12;	// text follows structure, padded with zeros to 16 byte alignment


// Prototypes
TR_INFO_m12	*TR_alloc_trans_info_m12(si8 buffer_bytes, ui4 ID_code, ui1 header_flags, sf4 timeout, si1 *password);
TERN_m12	TR_bind_m12(TR_INFO_m12 *trans_info, si1 *iface_addr, ui2 iface_port);
void		TR_build_message_m12(TR_MESSAGE_HEADER_m12 *msg, si1 *message_text);
TERN_m12	TR_check_transmission_header_alignment_m12(ui1 *bytes);
void		TR_close_transmission_m12(TR_INFO_m12 *trans_info);
TERN_m12	TR_connect_m12(TR_INFO_m12 *trans_info, si1 *dest_addr, ui2 dest_port);
TERN_m12	TR_connect_to_server_m12(TR_INFO_m12 *trans_info, si1 *dest_addr, ui2 dest_port);
TERN_m12	TR_create_socket_m12(TR_INFO_m12 *trans_info);
void		TR_free_transmission_info_m12(TR_INFO_m12 **trans_info_ptr, TERN_m12 free_structure);
void		TR_realloc_trans_info_m12(TR_INFO_m12 *trans_info, si8 buffer_bytes, TR_HEADER_m12 **caller_header);
si8		TR_recv_transmission_m12(TR_INFO_m12 *trans_info, TR_HEADER_m12 **caller_header);  // receive may reallocate, pass caller header to have function set local variable, otherwise pass NULL, can do manually
TERN_m12	TR_send_message_m12(TR_INFO_m12 *trans_info, ui1 type, TERN_m12 encrypt, si1 *fmt, ...);
si8		TR_send_transmission_m12(TR_INFO_m12 *trans_info);
TERN_m12	TR_set_socket_blocking_m12(TR_INFO_m12 *trans_info, TERN_m12 blocking);
void		TR_set_socket_timeout_m12(TR_INFO_m12 *trans_info);
TERN_m12	TR_show_message_m12(TR_HEADER_m12 *header);
void		TR_show_transmission_m12(TR_INFO_m12 *trans_info);
si1		*TR_strerror(si4 err_num);



//**********************************************************************************//
//********************************  Time Zone Data  ********************************//
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

#define TZ_TABLE_ENTRIES_m12      399
#define TZ_TABLE_m12 { \
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

#define TZ_COUNTRY_ALIASES_ENTRIES_m12      16
#define TZ_COUNTRY_ALIASES_TABLE_m12 { \
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

#define TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m12      1
#define TZ_COUNTRY_ACRONYM_ALIASES_TABLE_m12 { \
	{ "GB", "UK" } \
}



//**********************************************************************************//
//*****************************  DATBASE (DB) FUNCTIONS  ***************************//
//**********************************************************************************//

// Currently only postgres databases are supported.

#ifdef DATABASE_m12

	// Defines
	#define DB_EXPECTED_ROWS_NO_ENTRY_m12	((si4) -1)

	// Prototypes
	TERN_m12	DB_check_result_m12(PGresult *result);
	PGresult	*DB_execute_command_m12(PGconn *conn, si1 *command, si4 *rows, si4 expected_rows, const si1 *function, ui4 behavior_on_fail);

#endif



//***********************************************************************//
//*****************  MED VERSIONS OF STANDARD FUNCTIONS  ****************//
//***********************************************************************//


si4		asprintf_m12(si1 **target, si1 *fmt, ...);
void		*calloc_m12(size_t n_members, size_t el_size, const si1 *function, ui4 behavior_on_fail);
void		**calloc_2D_m12(size_t dim1, size_t dim2, size_t el_size, const si1 *function, ui4 behavior_on_fail);
size_t		calloc_size_m12(void *address, size_t element_size);
si4		errno_m12(void);
void		errno_reset_m12(void);  // zero errno before calling functions that may set it
void		exit_m12(si4 status);
FILE		*fopen_m12(si1 *path, si1 *mode, const si1 *function, ui4 behavior_on_fail);
si4     	fprintf_m12(FILE *stream, si1 *fmt, ...);
si4		fputc_m12(si4 c, FILE *stream);
size_t          fread_m12(void *ptr, size_t el_size, size_t n_members, FILE *stream, si1 *path, const si1 *function, ui4 behavior_on_fail);
void            free_m12(void *ptr, const si1 *function);
void            free_2D_m12(void **ptr, size_t dim1, const si1 *function);
TERN_m12	freeable_m12(void *address);
si4     	fscanf_m12(FILE *stream, si1 *fmt, ...);
si4             fseek_m12(FILE *stream, si8 offset, si4 whence, si1 *path, const si1 *function, ui4 behavior_on_fail);
si8            	ftell_m12(FILE *stream, const si1 *function, ui4 behavior_on_fail);
size_t		fwrite_m12(void *ptr, size_t el_size, size_t n_members, FILE *stream, si1 *path, const si1 *function, ui4 behavior_on_fail);
char		*getcwd_m12(char *buf, size_t size);
void		*malloc_m12(size_t n_bytes, const si1 *function, ui4 behavior_on_fail);
void		**malloc_2D_m12(size_t dim1, size_t dim2, size_t el_size, const si1 *function, ui4 behavior_on_fail);
size_t		malloc_size_m12(void *address);
void		memset_m12(void *ptr, const void *pattern, size_t pat_len, size_t n_members);
TERN_m12	mlock_m12(void *addr, size_t len, TERN_m12 zero_data, const si1 *function, ui4 behavior_on_fail);
TERN_m12	munlock_m12(void *addr, size_t len, const si1 *function, ui4 behavior_on_fail);
si4     	printf_m12(si1 *fmt, ...);
si4		putc_m12(si4 c, FILE *stream);
si4		putch_m12(si4 c);
si4		putchar_m12(si4 c);
void		*realloc_m12(void *ptr, size_t n_bytes, const si1 *function, ui4 behavior_on_fail);
void		**realloc_2D_m12(void **curr_ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, size_t el_size, const si1 *function, ui4 behavior_on_fail);
void		*recalloc_m12(void *orig_ptr, size_t curr_bytes, size_t new_bytes, const si1 *function, ui4 behavior_on_fail);
si4     	scanf_m12(si1 *fmt, ...);
si4     	sprintf_m12(si1 *target, si1 *fmt, ...);
si4		snprintf_m12(si1 *target, si4 target_field_bytes, si1 *fmt, ...);
si4     	sscanf_m12(si1 *target, si1 *fmt, ...);
si8		strcat_m12(si1 *target, si1 *source);
si8		strcpy_m12(si1 *target, si1 *source);
si8		strncat_m12(si1 *target, si1 *source, si4 target_field_bytes);
si8		strncpy_m12(si1 *target, si1 *source, si4 target_field_bytes);
si4             system_m12(si1 *command, TERN_m12 null_std_streams, const si1 *function, ui4 behavior_on_fail);
si4		system_pipe_m12(si1 **buffer_ptr, si8 buf_len, si1 *command, ui4 flags, const si1 *function, ui4 behavior, ...);  // varargs(SP_SEPARATE_STREAMS_m12 set): si1 **e_buffer_ptr, si8 e_buf_len
si4		vasprintf_m12(si1 **target, si1 *fmt, va_list args);
si4		vfprintf_m12(FILE *stream, si1 *fmt, va_list args);
si4		vprintf_m12(si1 *fmt, va_list args);
si4		vsnprintf_m12(si1 *target, si4 target_field_bytes, si1 *fmt, va_list args);
si4    		vsprintf_m12(si1 *target, si1 *fmt, va_list args);



//**********************************************************************************//
//*********************************  MED Records  **********************************//
//**********************************************************************************//

#include "medrec_m12.h"

#endif  // MEDLIB_IN_m12







