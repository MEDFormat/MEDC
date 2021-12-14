
#ifndef MEDLIB_IN_m10
#define MEDLIB_IN_m10


//**********************************************************************************//
//********************************  MED 1.0 C Library  *****************************//
//**********************************************************************************//


// Multiscale Electrophysiology Data (MED) Format Software Library, Version 1.0
// Written by Matt Stead

// LICENSE & COPYRIGHT:

// MED library source code (medlib) is copyrighted by Dark Horse Neuro Inc, 2021 (Matt Stead & Casey Stengel)

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

// All functions, constants, macros, and data types defined in the library are tagged
// with the suffix "_m10" (for "MED 1.0"). This is to facilitate using multiple versions
// of the library in concert in the future; for example to write a MED 1.0 to MED 2.0 converter.


//**********************************************************************************//
//*********************************  Library Includes  *****************************//
//**********************************************************************************//

#include "targets_m10.h"

#ifdef WINDOWS_m10
// the following is necessary to include <winsock2.h> (or can define WIN32_LEAN_AND_MEAN, but excludes a lot of stuff)
// winsock2.h has to be included before windows.h, but requires WIN32 to be defined, which is usually defined by windows.h
// WIN_NEED_SOCKETS_m10 is defined in "targets.h"
	#ifdef WIN_NEED_SOCKETS_m10
		#ifndef WIN32
			#define WIN32
		#endif
		#include <winsock2.h>
		#pragma comment(lib, "ws2_32.lib") // link with Ws2_32.lib
	#endif
	#include <windows.h>
	#include <io.h>
	#include <direct.h>
	#include <fileapi.h>
	#include <share.h>
	#define _USE_MATH_DEFINES  // Needed for standard math constants. Must be defined before math.h included.
#endif
#if defined MACOS_m10 || defined LINUX_m10
	#include <unistd.h>
	#include <wchar.h>
	#include <dirent.h>
	#include <sys/time.h>
	#include <sys/resource.h>
	#include <stdatomic.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <errno.h>
#ifdef MATLAB_m10
	#include "mex.h"
#endif



//**********************************************************************************//
//******************************  Elemental Typedefs  ******************************//
//**********************************************************************************//

#ifndef SIZE_TYPES_IN_m10
#define SIZE_TYPES_IN_m10

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
typedef long double	sf16;   // NOTE: it often requires an explicit compiler instruction to implement true long floating point math
				// in icc: "-Qoption,cpp,--extended_float_types"

#endif  // SIZE_TYPES_IN_m10

// MED Library Ternary Boolean Schema
typedef si1				TERN_m10;
#define TRUE_m10			1
#define UNKNOWN_m10			0
#define FALSE_m10			-1

// Reserved si4 Sample Values
#define NAN_m10                         ((si4) 0x80000000)
#define NEGATIVE_INFINITY_m10           ((si4) 0x80000001)
#define POSITIVE_INFINITY_m10           ((si4) 0x7FFFFFFF)
#define MAXIMUM_SAMPLE_VALUE_m10        ((si4) 0x7FFFFFFE)
#define MINIMUM_SAMPLE_VALUE_m10        ((si4) 0x80000002)



//**********************************************************************************//
//**********************************  ENCRYPTION  **********************************//
//**********************************************************************************//

// Encryption & Password Constants
#define ENCRYPTION_LEVEL_NO_ENTRY_m10		-128
#define NO_ENCRYPTION_m10			0
#define LEVEL_0_ENCRYPTION_m10			NO_ENCRYPTION_m10
#define LEVEL_1_ENCRYPTION_m10			1
#define LEVEL_2_ENCRYPTION_m10			2
#define LEVEL_0_ACCESS_m10			LEVEL_0_ENCRYPTION_m10
#define LEVEL_1_ACCESS_m10			LEVEL_1_ENCRYPTION_m10
#define LEVEL_2_ACCESS_m10			LEVEL_2_ENCRYPTION_m10
#define LEVEL_1_ENCRYPTION_DECRYPTED_m10        -LEVEL_1_ENCRYPTION_m10
#define LEVEL_2_ENCRYPTION_DECRYPTED_m10        -LEVEL_2_ENCRYPTION_m10
#define ENCRYPTION_BLOCK_BYTES_m10		16      // AES-128
#define ENCRYPTION_KEY_BYTES_m10                176     // AES-128   = ((AES_NR + 1) * AES_NK * AES_NB)
#define PASSWORD_BYTES_m10			ENCRYPTION_BLOCK_BYTES_m10
#define MAX_PASSWORD_CHARACTERS_m10		PASSWORD_BYTES_m10
#define MAX_ASCII_PASSWORD_STRING_BYTES_m10	(MAX_PASSWORD_CHARACTERS_m10 + 1)  // 1 byte per character in ascii plus terminal zero
#define MAX_UTF8_PASSWORD_BYTES_m10		(MAX_PASSWORD_CHARACTERS_m10 * 4)  // up to 4 bytes per character in UTF-8
#define MAX_PASSWORD_STRING_BYTES_m10		(MAX_UTF8_PASSWORD_BYTES_m10 + 1)  // 1 byte for null-termination
#define PASSWORD_VALIDATION_FIELD_BYTES_m10     PASSWORD_BYTES_m10
#define PASSWORD_HINT_BYTES_m10                 256

// Password Data Structure
typedef struct {
	ui1	level_1_encryption_key[ENCRYPTION_KEY_BYTES_m10];
	ui1	level_2_encryption_key[ENCRYPTION_KEY_BYTES_m10];
	si1     level_1_password_hint[PASSWORD_HINT_BYTES_m10];
	si1     level_2_password_hint[PASSWORD_HINT_BYTES_m10];
	ui1	access_level;
	ui1	processed;  // 0 or 1 (not ternary)
} PASSWORD_DATA_m10;



//**********************************************************************************//
//*******************************  CMP (COMPRESSION)  ******************************//
//**********************************************************************************//

// CMP: Miscellaneous Constants
#define CMP_SAMPLE_VALUE_NO_ENTRY_m10				NAN_m10

// CMP: Block Fixed Header Offset Constants
#define CMP_BLOCK_FIXED_HEADER_BYTES_m10                        56                              // fixed region only
#define CMP_BLOCK_START_UID_m10                                 ((ui8) 0x0123456789ABCDEF)      // ui8   (decimal 81,985,529,216,486,895)
#define CMP_BLOCK_START_UID_OFFSET_m10				0
#define CMP_BLOCK_CRC_OFFSET_m10                                8                               // ui4
#define CMP_BLOCK_CRC_NO_ENTRY_m10                              CRC_NO_ENTRY_m10
#define CMP_BLOCK_BLOCK_FLAGS_OFFSET_m10			12                              // ui4
#define CMP_BLOCK_BLOCK_FLAGS_NO_ENTRY_m10			0
#define CMP_BLOCK_CRC_START_OFFSET_m10                          CMP_BLOCK_BLOCK_FLAGS_OFFSET_m10
#define CMP_BLOCK_START_TIME_OFFSET_m10                         16                              // si8
#define CMP_BLOCK_START_TIME_NO_ENTRY_m10                       UUTC_NO_ENTRY_m10
#define CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_OFFSET_m10         24                              // si4
#define CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m10       -1
#define CMP_BLOCK_TOTAL_BLOCK_BYTES_OFFSET_m10			28                              // ui4
#define CMP_BLOCK_TOTAL_BLOCK_BYTES_NO_ENTRY_m10		0
// CMP Block Encryption Start
#define CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m10                  32                              // ui4
#define CMP_BLOCK_ENCRYPTION_START_OFFSET_m10			CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m10
#define CMP_BLOCK_NUMBER_OF_SAMPLES_NO_ENTRY_m10                0xFFFFFFFF
#define CMP_BLOCK_NUMBER_OF_RECORDS_OFFSET_m10                  36                              // ui2
#define CMP_BLOCK_NUMBER_OF_RECORDS_NO_ENTRY_m10                0xFFFF
#define CMP_BLOCK_RECORD_REGION_BYTES_OFFSET_m10		38                              // ui2
#define CMP_BLOCK_RECORD_REGION_BYTES_NO_ENTRY_m10		0xFFFF
#define CMP_BLOCK_PARAMETER_FLAGS_OFFSET_m10			40                              // ui4
#define CMP_BLOCK_PARAMETER_FLAGS_NO_ENTRY_m10			0
#define CMP_BLOCK_PARAMETER_REGION_BYTES_OFFSET_m10		44				// ui2
#define CMP_BLOCK_PARAMETER_REGION_BYTES_NO_ENTRY_m10           0xFFFF
#define CMP_BLOCK_PROTECTED_REGION_BYTES_OFFSET_m10             46                              // ui2
#define CMP_BLOCK_PROTECTED_REGION_BYTES_NO_ENTRY_m10           0xFFFF
#define CMP_BLOCK_DISCRETIONARY_REGION_BYTES_OFFSET_m10         48                              // ui2
#define CMP_BLOCK_DISCRETIONARY_REGION_BYTES_NO_ENTRY_m10       0xFFFF
#define CMP_BLOCK_MODEL_REGION_BYTES_OFFSET_m10                 50                              // ui2
#define CMP_BLOCK_MODEL_REGION_BYTES_NO_ENTRY_m10               0xFFFF
#define CMP_BLOCK_TOTAL_HEADER_BYTES_OFFSET_m10			52                              // ui4
#define CMP_BLOCK_TOTAL_HEADER_BYTES_NO_ENTRY_m10            	0xFFFF
#define CMP_BLOCK_RECORDS_REGION_OFFSET_m10			56

// CMP: Record Header Offset Constants
#define CMP_RECORD_HEADER_BYTES_m10                        	8
#define CMP_RECORD_HEADER_TYPE_CODE_OFFSET_m10                  0				// ui4
#define CMP_RECORD_HEADER_TYPE_CODE_NO_ENTRY_m10		0
#define CMP_RECORD_HEADER_VERSION_MAJOR_OFFSET_m10		4				// ui1
#define CMP_RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m10		0xFF
#define CMP_RECORD_HEADER_VERSION_MINOR_OFFSET_m10		5				// ui1
#define CMP_RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m10		0xFF
#define CMP_RECORD_HEADER_TOTAL_BYTES_OFFSET_m10		6				// ui2
#define CMP_RECORD_HEADER_TOTAL_BYTES_NO_ENTRY_m10		0xFFFF				// Note maximum CMP record size is 65k - smaller than MED record

// CMP: MODEL Offset Constants
#define CMP_RED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10                   0                       // si4
#define CMP_RED_MODEL_DIFFERENCE_BYTES_OFFSET_m10                       4                       // ui4
#define CMP_RED_MODEL_DERIVATIVE_LEVEL_OFFSET_m10                  	8                       // ui1
#define CMP_RED_MODEL_NO_ZERO_COUNTS_FLAG_OFFSET_m10                  	9                       // ui1
#define CMP_RED_MODEL_NUMBER_OF_STATISTICS_BINS_OFFSET_m10              10                      // ui2
#define CMP_RED_MODEL_FIXED_BYTES_m10                                   12
#define CMP_RED_MODEL_BIN_COUNTS_OFFSET_m10                             CMP_RED_MODEL_FIXED_BYTES_m10

#define CMP_PRED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10                  0                       // si4
#define CMP_PRED_MODEL_DIFFERENCE_BYTES_OFFSET_m10                      4                       // ui4
#define CMP_PRED_MODEL_DERIVATIVE_LEVEL_OFFSET_m10                 	8                       // ui1
#define CMP_PRED_MODEL_NO_ZERO_COUNTS_FLAG_OFFSET_m10                  	9                       // ui1
#define CMP_PRED_MODEL_NUMBER_OF_NIL_STATISTICS_BINS_OFFSET_m10         10                      // ui2
#define CMP_PRED_MODEL_NUMBERS_OF_STATISTICS_BINS_OFFSET_m10            CMP_PRED_MODEL_NUMBER_OF_NIL_STATISTICS_BINS_OFFSET_m10
#define CMP_PRED_MODEL_NUMBER_OF_POS_STATISTICS_BINS_OFFSET_m10         12                      // ui2
#define CMP_PRED_MODEL_NUMBER_OF_NEG_STATISTICS_BINS_OFFSET_m10         14                      // ui1
#define CMP_PRED_MODEL_FIXED_BYTES_m10                                  16
#define CMP_PRED_MODEL_BIN_COUNTS_OFFSET_m10                            CMP_PRED_MODEL_FIXED_BYTES_m10

#define CMP_MBE_MODEL_MINIMUM_SAMPLE_VALUE_OFFSET_m10                   0                       // si4
#define CMP_MBE_MODEL_BITS_PER_SAMPLE_OFFSET_m10                        4                       // ui1
#define CMP_MBE_MODEL_FIXED_BYTES_m10                                   5

// CMP Block Flag Masks
#define CMP_BF_BLOCK_FLAG_BITS_m10			32
#define CMP_BF_DISCONTINUITY_MASK_m10			((ui4) 1)       // Bit 0
#define CMP_BF_LEVEL_1_ENCRYPTION_MASK_m10		((ui4) 16)	// Bit 4
#define CMP_BF_LEVEL_2_ENCRYPTION_MASK_m10		((ui4) 32)	// Bit 5
#define CMP_BF_RED_ENCODING_MASK_m10			((ui4) 256)	// Bit 8
#define CMP_BF_PRED_ENCODING_MASK_m10			((ui4) 512)	// Bit 9
#define CMP_BF_MBE_ENCODING_MASK_m10			((ui4) 1024)	// Bit 10

#define CMP_BF_ALGORITHM_MASK_m10			((ui4) (CMP_BF_RED_ENCODING_MASK_m10 | CMP_BF_PRED_ENCODING_MASK_m10 | CMP_BF_MBE_ENCODING_MASK_m10))
#define CMP_BF_ENCRYPTION_MASK_m10			((ui4) (CMP_BF_LEVEL_1_ENCRYPTION_MASK_m10 | CMP_BF_LEVEL_2_ENCRYPTION_MASK_m10))

// CMP Parameter Map Indices
#define CMP_PF_INTERCEPT_IDX_m10			((ui4) 0)       // Bit 0
#define CMP_PF_GRADIENT_IDX_m10				((ui4) 1)	// Bit 1
#define CMP_PF_AMPLITUDE_SCALE_IDX_m10			((ui4) 2)       // Bit 2
#define CMP_PF_FREQUENCY_SCALE_IDX_m10			((ui4) 3)       // Bit 3
#define CMP_PF_NOISE_SCORES_IDX_m10			((ui4) 4)	// Bit 4

// CMP Parameter Flag Masks
#define CMP_PF_PARAMETER_FLAG_BITS_m10			32
#define CMP_PF_INTERCEPT_MASK_m10			((ui4) 1 << CMP_PF_INTERCEPT_IDX_m10)		// Bit 0
#define CMP_PF_GRADIENT_MASK_m10			((ui4) 1 << CMP_PF_GRADIENT_IDX_m10)		// Bit 1
#define CMP_PF_AMPLITUDE_SCALE_MASK_m10			((ui4) 1 << CMP_PF_AMPLITUDE_SCALE_IDX_m10)	// Bit 2
#define CMP_PF_FREQUENCY_SCALE_MASK_m10			((ui4) 1 << CMP_PF_FREQUENCY_SCALE_IDX_m10)	// Bit 3
#define CMP_PF_NOISE_SCORES_MASK_m10			((ui4) 1 << CMP_PF_NOISE_SCORES_IDX_m10)	// Bit 4

// Compression Modes
#define CMP_COMPRESSION_MODE_NO_ENTRY_m10	((ui1) 0)
#define CMP_DECOMPRESSION_MODE_m10              ((ui1) 1)
#define CMP_COMPRESSION_MODE_m10                ((ui1) 2)

// Lossy Compression Modes
#define CMP_AMPLITUDE_SCALE_MODE_m10		((ui1) 1)
#define CMP_FREQUENCY_SCALE_MODE_m10		((ui1) 2)

// Compression Algorithms
#define CMP_RED_COMPRESSION_m10		CMP_BF_RED_ENCODING_MASK_m10
#define CMP_PRED_COMPRESSION_m10	CMP_BF_PRED_ENCODING_MASK_m10
#define CMP_MBE_COMPRESSION_m10		CMP_BF_MBE_ENCODING_MASK_m10

// CMP Directives Defaults
#define CMP_DIRECTIVES_FREE_PASSWORD_DATA_DEFAULT_m10		FALSE_m10
#define CMP_DIRECTIVES_COMPRESSION_MODE_DEFAULT_m10		CMP_COMPRESSION_MODE_NO_ENTRY_m10
#define CMP_DIRECTIVES_ALGORITHM_DEFAULT_m10			CMP_PRED_COMPRESSION_m10
#define CMP_DIRECTIVES_ENCRYPTION_LEVEL_DEFAULT_m10		NO_ENCRYPTION_m10
#define CMP_DIRECTIVES_FALL_THROUGH_TO_MBE_DEFAULT_m10		TRUE_m10
#define CMP_DIRECTIVES_RESET_DISCONTINUITY_DEFAULT_m10		TRUE_m10
#define CMP_DIRECTIVES_INCLUDE_NOISE_SCORES_DEFAULT_m10		FALSE_m10
#define CMP_DIRECTIVES_NO_ZERO_COUNTS_DEFAULT_m10		FALSE_m10
	// lossy compression directives
#define CMP_DIRECTIVES_DETREND_DATA_DEFAULT_m10			FALSE_m10
#define CMP_DIRECTIVES_REQUIRE_NORMALITY_DEFAULT_m10		FALSE_m10
#define CMP_DIRECTIVES_RETURN_LOSSY_DATA_DEFAULT_m10		FALSE_m10
#define CMP_DIRECTIVES_USE_COMPRESSION_RATIO_DEFAULT_m10	FALSE_m10
#define CMP_DIRECTIVES_USE_MEAN_RESIDUAL_RATIO_DEFAULT_m10	TRUE_m10
#define CMP_DIRECTIVES_USE_RELATIVE_RATIO_DEFAULT_m10		TRUE_m10
#define CMP_DIRECTIVES_SET_AMPLITUDE_SCALE_DEFAULT_m10		FALSE_m10
#define CMP_DIRECTIVES_FIND_AMPLITUDE_SCALE_DEFAULT_m10		FALSE_m10
#define CMP_DIRECTIVES_SET_FREQUENCY_SCALE_DEFAULT_m10		FALSE_m10
#define CMP_DIRECTIVES_FIND_FREQUENCY_SCALE_DEFAULT_m10		FALSE_m10
#define CMP_DIRECTIVES_SET_DERIVATIVE_LEVEL_DEFAULT_m10		FALSE_m10
#define CMP_DIRECTIVES_FIND_DERIVATIVE_LEVEL_DEFAULT_m10	FALSE_m10

// CMP Parameters Defaults
#define CMP_PARAMETERS_NUMBER_OF_BLOCK_PARAMETERS_DEFAULT_m10	0
#define CMP_PARAMETERS_MINIMUM_SAMPLE_VALUE_DEFAULT_m10		CMP_SAMPLE_VALUE_NO_ENTRY_m10
#define CMP_PARAMETERS_MAXIMUM_SAMPLE_VALUE_DEFAULT_m10		CMP_SAMPLE_VALUE_NO_ENTRY_m10
#define CMP_PARAMETERS_DISCONTINUITY_DEFAULT_m10		UNKNOWN_m10
#define CMP_PARAMETERS_NO_ZERO_COUNTS_FLAG_DEFAULT_m10          ((ui1) 0)
#define CMP_PARAMETERS_DERIVATIVE_LEVEL_DEFAULT_m10             ((ui1) 1)
	// lossy compression parameters
#define CMP_PARAMETERS_GOAL_RATIO_DEFAULT_m10			((sf8) 0.05)
#define CMP_PARAMETERS_GOAL_TOLERANCE_DEFAULT_m10		((sf8) 0.005)
#define CMP_PARAMETERS_MAXIMUM_GOAL_ATTEMPTS_DEFAULT_m10	20
#define CMP_PARAMETERS_MINIMUM_NORMALITY_DEFAULT_m10		((ui1) 128)  // ronge 0-254 (low to high)
#define CMP_PARAMETERS_AMPLITUDE_SCALE_DEFAULT_m10		((sf4) 1.0)
#define CMP_PARAMETERS_FREQUENCY_SCALE_DEFAULT_m10		((sf4) 1.0)
	// variable region parameters
#define CMP_USER_NUMBER_OF_RECORDS_DEFAULT_m10			((ui2) 0)
#define CMP_USER_RECORD_REGION_BYTES_DEFAULT_m10		((ui2) 0)
#define CMP_USER_PARAMETER_FLAGS_DEFAULT_m10			((ui4) 0)
#define CMP_PROTECTED_REGION_BYTES_DEFAULT_m10			((ui2) 0)
#define CMP_USER_DISCRETIONARY_REGION_BYTES_DEFAULT_m10		((ui2) 0)

// Macros
#define CMP_MAX_DIFFERENCE_BYTES_m10(block_samps)		(block_samps * 5) // full si4 plus 1 keysample flag byte per sample
#define CMP_MAX_COMPRESSED_BYTES_m10(block_samps, n_blocks)	(((block_samps * 4) + CMP_BLOCK_FIXED_HEADER_BYTES_m10 + 7) * n_blocks)	// (no compression + header + maximum pad bytes) for n_blocks blocks
																																		// NOTE: does not take variable region bytes into account and assumes
																	// fallthrough to MBE
#define CMP_PRED_CAT_m10(x)					((x) ? (((x) & 0x80) ? CMP_PRED_NEG_m10 : CMP_PRED_POS_m10) : CMP_PRED_NIL_m10) // do not increment/decrement within call to CMP_PRED_CAT_m10
																		// as "x" is used twice
#define CMP_IS_DETRENDED_m10(block_header_ptr)			((block_header_ptr->parameter_flags & CMP_PF_INTERCEPT_MASK_m10) && (block_header_ptr->parameter_flags & CMP_PF_GRADIENT_MASK_m10))
#define CMP_VARIABLE_REGION_BYTES_v1_m10(block_header_ptr)	((ui4) (block_header_ptr)->record_region_bytes + (ui4) (block_header_ptr)->parameter_region_bytes + \
								(ui4) (block_header_ptr)->protected_region_bytes + (ui4) (block_header_ptr)->discretionary_region_bytes)
#define CMP_VARIABLE_REGION_BYTES_v2_m10(block_header_ptr)	((ui4) (block_header_ptr)->total_header_bytes - ((ui4) CMP_BLOCK_FIXED_HEADER_BYTES_m10 + \
								(ui4) (block_header_ptr)->model_region_bytes))

// Update CPS Pointer Flags
#define CMP_UPDATE_ORIGINAL_PTR_m10             ((ui1) 1)
#define CMP_UPDATE_BLOCK_HEADER_PTR_m10         ((ui1) 2)
#define CMP_UPDATE_DECOMPRESSED_PTR_m10         ((ui1) 4)

// RED/PRED Codec Constants
#define CMP_RED_KEYSAMPLE_FLAG_m10      ((ui1) 0x80)            // -128 as si1, +128 as ui1
#define CMP_RED_TOTAL_COUNTS_m10        ((ui4) 0x10000)         // 2^16
#define CMP_RED_MAXIMUM_RANGE_m10       ((ui8) 0x1000000000000) // 2^48
#define CMP_RED_RANGE_MASK_m10          ((ui8) 0xFFFFFFFFFFFF)  // 2^48 - 1
#define CMP_RED_MAX_STATS_BINS_m10      256
#define CMP_PRED_CATS_m10               3
#define CMP_PRED_NIL_m10                0
#define CMP_PRED_POS_m10                1
#define CMP_PRED_NEG_m10                2

// Normal cumulative distribution fucntion values from -3 to +3 standard deviations in 0.1 sigma steps
#define CMP_NORMAL_CDF_TABLE_ENTRIES_m10        61
#define CMP_NORMAL_CDF_TABLE_m10	      { 0.00134989803163010, 0.00186581330038404, 0.00255513033042794, 0.00346697380304067, \
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

#define CMP_SUM_NORMAL_CDF_m10                  30.5
#define CMP_SUM_SQ_NORMAL_CDF_m10               24.864467406647070
#define CMP_KS_CORRECTION_m10                   ((sf8) 0.0001526091333688973)

// Typedefs & Structures
typedef struct {
	ui4	type_code;  // note this is not null terminated and so cannot be treated as a string as in RECORD_HEADER_m10 structure
	ui1	version_major;
	ui1	version_minor;
	ui2	total_bytes;  // note maximum record size is 65535 - smaller than in RECORD_HEADER_m10 structure
} CMP_RECORD_HEADER_m10;

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
} CMP_BLOCK_FIXED_HEADER_m10;

typedef struct {
	ui4             mode;  // CMP_COMPRESSION_MODE_m10, CMP_DECOMPRESSION_MODE_m10
	ui4             algorithm;  // RED, PRED, or MBE
	si1             encryption_level;  // encryption level for data blocks: passed in compression
	TERN_m10        fall_through_to_MBE;  // if MBE would be smaller than RED/PRED, use MBE for that block
	TERN_m10        reset_discontinuity;  // if discontinuity directive == TRUE_m10, reset to FALSE_m10 after compressing the block
	TERN_m10        include_noise_scores;
	TERN_m10        no_zero_counts;  // in RED & PRED codecs (when blocks must be encoded with non-block statistics. This is a special case.)
	TERN_m10        free_password_data;  // when freeing CPS
	TERN_m10        set_derivative_level;  // value passed in "derivative_level" parameter
	TERN_m10        find_derivative_level;  // mutually exclusive with "set_derivative_level"
	// lossy compression directives
	TERN_m10        detrend_data;  // Lossless operation, but most useful for lossy compression.
	TERN_m10        require_normality;  // For lossy compression - use lossless if data amplitudes are too oddly distributed.  Pairs with "minimum_normality" parameter.
	TERN_m10        use_compression_ratio;  // Used in "find" directives. Mutually exclusive with "use_mean_residual_ratio".
	TERN_m10        use_mean_residual_ratio;  // Used in "find" directives. Mutually exclusive with "use_compression_ratio".
	TERN_m10        use_relative_ratio;  // divide goal ratio by the block coefficient of variation in lossy compression routines (more precision in blocks with higher variance)
	TERN_m10        set_amplitude_scale;  // value passed in "amplitude_scale" parameter
	TERN_m10        find_amplitude_scale;  // mutually exclusive with "set_amplitude_scale"
	TERN_m10        set_frequency_scale;  // value passed in "frequency_scale" parameter
	TERN_m10        find_frequency_scale;  // mutually exclusive with "set_frequency_scale"
} CMP_DIRECTIVES_m10;

typedef struct {
	si4		number_of_block_parameters;
	ui4		*block_parameters;  // pointer beginning of parameter region of block header
	ui4		block_parameter_map[CMP_PF_PARAMETER_FLAG_BITS_m10];
	si4             minimum_sample_value;  // found on compression, stored for use in METADATA (and MBE, if used)
	si4             maximum_sample_value;  // stored for use in METADATA (and MBE, if used)
	TERN_m10        discontinuity;  // set if block is first after a discontinuity, passed in compression, returned in decompression
	ui1		no_zero_counts_flag;
	ui1             derivative_level;  // used by with set_derivative_level directive, also returned in decode
	// lossy compression parameters
	sf8             goal_ratio;  // either compression ratio or mean residual ratio
	sf8             actual_ratio;  // either compression ratio or mean residual ratio
	sf8             goal_tolerance;  // tolerance for lossy compression mode goal, value of <= 0.0 uses default values, which are returned
	si4             maximum_goal_attempts;  // maximum loops to attain goal compression
	ui1		minimum_normality;
	sf4             amplitude_scale;  // used with set_amplitude_scale directive
	sf4             frequency_scale;  // used with set_frequency_scale directive
	ui2		user_number_of_records;
	ui2		user_record_region_bytes;  // set by user to reserve bytes for records in header
	ui4		user_parameter_flags;  // user bits to be set in parameter flags of block header (library flags will be set automatically)
	ui2		protected_region_bytes;  // not currently used
	ui2		user_discretionary_region_bytes;  // set by user to reserve bytes for discretionary region in header
	ui4		variable_region_bytes;
} CMP_PARAMETERS_m10;

typedef struct {
	ui4     count;
	si1     value;
} CMP_STATISTICS_BIN_m10;

typedef struct {
#if defined MACOS_m10 || defined LINUX_m10
	TERN_m10 _Atomic		mutex;
#else
	TERN_m10			mutex;
#endif
	ui4				**count;  // used by RED/PRED encode & decode
	CMP_STATISTICS_BIN_m10		**sorted_count;  // used by RED/PRED encode & decode
	ui8				**cumulative_count;  // used by RED/PRED encode & decode
	ui8				**minimum_range;  // used by RED/PRED encode & decode
	ui1				**symbol_map;  // used by RED/PRED encode & decode
	si4				*input_buffer;
	ui1				*compressed_data;  // passed in decompression, returned in compression, should not be updated
	CMP_BLOCK_FIXED_HEADER_m10	*block_header; // points to beginning of current block within compressed_data array, updatable
	si4				*decompressed_data;  // returned in decompression or if lossy data requested, used in some compression modes, should not be updated
	si4				*decompressed_ptr;  // points to beginning of current block within decompressed_data array, updatable
	si4				*original_data;  // passed in compression, should not be updated
	si4				*original_ptr;  // points to beginning of current block within original_data array, updatable
	si1				*difference_buffer;  // passed in both compression & decompression
	si1				*derivative_buffer;  // used if needed in compression & decompression, size of maximum block differences
	si4				*detrended_buffer;  // used if needed in compression, size of decompressed block
	si4				*scaled_amplitude_buffer;  // used if needed in compression, size of decompressed block
	si4				*scaled_frequency_buffer;  // used if needed in compression, size of decompressed block
	PASSWORD_DATA_m10		*password_data;
	CMP_DIRECTIVES_m10   		directives;
	ui1				*records;
	CMP_PARAMETERS_m10  		parameters;
	ui1				*protected_region;
	ui1				*discretionary_region;
	ui1				*model_region;
} CMP_PROCESSING_STRUCT_m10;

// defined here because it is used by CMP functions also
typedef struct NODE_STRUCT_m10 {
	si4                     val;
	ui4                     count;
	struct NODE_STRUCT_m10	*prev, *next;
} NODE_m10;


// Function Prototypes
CMP_PROCESSING_STRUCT_m10	*CMP_allocate_processing_struct_m10(CMP_PROCESSING_STRUCT_m10 *cps, ui4 mode, si8 data_samples, si8 compressed_data_bytes, si8 difference_bytes, ui4 block_samples, CMP_DIRECTIVES_m10 *directives, CMP_PARAMETERS_m10 *parameters);
sf8		CMP_calculate_mean_residual_ratio_m10(si4 *data, si4 *lossy_data, ui4 n_samps);
void    	CMP_calculate_statistics_m10(void *stats_ptr, si4 *data, si8 len, NODE_m10 *nodes);  // "stats_ptr" is a pointer to a "REC_Stat_v10_m10" structure from medrec_m10.h
TERN_m10	CMP_check_CPS_allocation_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void		CMP_cps_mutex_off_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void		CMP_cps_mutex_on_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_decode_m10(CMP_PROCESSING_STRUCT_m10 *cps);
TERN_m10     	CMP_decrypt_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_detrend_m10(si4 *input_buffer, si4 *output_buffer, si8 len, CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_encode_m10(CMP_PROCESSING_STRUCT_m10 *cps, si8 start_time, si4 acquisition_channel_number, ui4 number_of_samples);
TERN_m10     	CMP_encrypt_m10(CMP_PROCESSING_STRUCT_m10 *cps);
TERN_m10	CMP_find_amplitude_scale_m10(CMP_PROCESSING_STRUCT_m10 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m10 *cps));
void    	CMP_find_extrema_m10(si4 *input_buffer, si8 len, si4 *min, si4 *max, CMP_PROCESSING_STRUCT_m10 *cps);
TERN_m10	CMP_find_frequency_scale_m10(CMP_PROCESSING_STRUCT_m10 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m10 *cps));
void    	CMP_free_processing_struct_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_generate_lossy_data_m10(CMP_PROCESSING_STRUCT_m10 *cps, si4* input_buffer, si4 *output_buffer, ui1 mode);
void		CMP_generate_parameter_map_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_get_variable_region_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void		CMP_initialize_directives_m10(CMP_DIRECTIVES_m10 *directives, ui1 mode);
TERN_m10	CMP_initialize_tables_m10(void);
void		CMP_initialize_parameters_m10(CMP_PARAMETERS_m10 *parameters);
void    	CMP_lad_reg_m10(si4 *data, si8 len, sf8 *m, sf8 *b);
void    	CMP_lin_reg_m10(si4 *data, si8 len, sf8 *m, sf8 *b);
void    	CMP_MBE_decode_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_MBE_encode_m10(CMP_PROCESSING_STRUCT_m10 *cps);
ui1     	CMP_normality_score_m10(si4 *data, ui4 n_samps);
void    	CMP_offset_time_m10(CMP_BLOCK_FIXED_HEADER_m10 *block_header, si4 action);
void    	CMP_PRED_decode_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_PRED_encode_m10(CMP_PROCESSING_STRUCT_m10 *cps);
sf8     	CMP_quantile_value_m10(sf8 *x, si8 len, sf8 quantile, TERN_m10 preserve_input, sf8 *buff);
void    	CMP_RED_decode_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_RED_encode_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_retrend_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 m, sf8 b);
si4     	CMP_round_m10(sf8 val);
void    	CMP_scale_amplitude_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor, CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_scale_frequency_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor, CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_set_variable_region_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_show_block_header_m10(CMP_BLOCK_FIXED_HEADER_m10 *block_header);
void    	CMP_show_block_model_m10(CMP_BLOCK_FIXED_HEADER_m10 *block_header);
void    	CMP_unscale_amplitude_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor);
void    	CMP_unscale_frequency_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor);
CMP_BLOCK_FIXED_HEADER_m10 *CMP_update_CPS_pointers_m10(CMP_PROCESSING_STRUCT_m10* cps, ui1 flags);


//**********************************************************************************//
//********************************  MED Constants  *********************************//
//**********************************************************************************//

// Miscellaneous Constants
#define MED_VERSION_MAJOR_m10                   1
#define MED_VERSION_MINOR_m10                   0
#define MED_VERSION_NO_ENTRY_m10                0xFF
#define BIG_ENDIAN_m10                          0
#define LITTLE_ENDIAN_m10                       1
#define TYPE_BYTES_m10                          5
#define TYPE_STRLEN_m10                         4
#define UID_BYTES_m10                           8
#define UID_NO_ENTRY_m10                        0
#define BASE_FILE_NAME_BYTES_m10                256        // utf8[63]
#define SEGMENT_BASE_FILE_NAME_BYTES_m10        (BASE_FILE_NAME_BYTES_m10 + 8)
#define VIDEO_DATA_BASE_FILE_NAME_BYTES_m10     (BASE_FILE_NAME_BYTES_m10 + 8)
#define FULL_FILE_NAME_BYTES_m10                1024        // utf8[255]
#define PAD_BYTE_VALUE_m10                      0x7e        // ascii tilde ("~") as si1
#define FILE_NUMBERING_DIGITS_m10               4
#define FREQUENCY_NO_ENTRY_m10                  -1.0
#define FREQUENCY_VARIABLE_m10			-2.0
#define FRAME_RATE_NO_ENTRY_m10                 -1.0
#define UNKNOWN_NUMBER_OF_ENTRIES_m10           -1
#define SEGMENT_NUMBER_NO_ENTRY_m10             -1
#define CHANNEL_NUMBER_NO_ENTRY_m10             -1
#define CHANNEL_NUMBER_ALL_CHANNELS_m10         -2
#define DOES_NOT_EXIST_m10                      0
#define FILE_EXISTS_m10                         1
#define DIR_EXISTS_m10                          2
#define SIZE_STRING_BYTES_m10                   32
#define UNKNOWN_SEARCH_m10                      ((ui1) 0)
#define TIME_SEARCH_m10                         ((ui1) 1)
#define INDEX_SEARCH_m10                        ((ui1) 2)
#define IPV4_ADDRESS_BYTES_m10			4
#define POSTAL_CODE_BYTES_m10			16
#define LOCALITY_BYTES_m10			64  	//  ascii[63]
#if defined MACOS_m10 || defined LINUX_m10
	#define NULL_DEVICE			"/dev/null"
#endif
#ifdef WINDOWS_m10
	#define PRINTF_BUF_LEN_m10		1024
	#define NULL_DEVICE			"NUL"
#endif

// for clarity use of read_session_m10, read_channel_m10(), & read_segment_m10()
// in the "read_time_series_data" & "read_record_data arguments"
					// TRUE_m10 	// read in the data
#define OPEN_ONLY_m10			UNKNOWN_m10	// create the fps's, open the files, read the universal headers, but leave file pointers at the start of the data
					// FALSE_m10	// ignore the data

// Error Handling Constants
#define USE_GLOBAL_BEHAVIOR_m10         0
#define RESTORE_BEHAVIOR_m10            1
#define EXIT_ON_FAIL_m10                2
#define RETURN_ON_FAIL_m10              4
#define SUPPRESS_ERROR_OUTPUT_m10       8
#define SUPPRESS_WARNING_OUTPUT_m10     16
#define SUPPRESS_MESSAGE_OUTPUT_m10     32
#define SUPPRESS_ALL_OUTPUT_m10         (SUPPRESS_ERROR_OUTPUT_m10 | SUPPRESS_WARNING_OUTPUT_m10 | SUPPRESS_MESSAGE_OUTPUT_m10)
#define RETRY_ONCE_m10                  64

// Target Value Constants
#define DEFAULT_MODE_m10        0
#define FIND_START_m10          1
#define FIND_END_m10            2
#define FIND_CENTER_m10         3
#define FIND_CURRENT_m10        4
#define FIND_NEXT_m10           5
#define FIND_CLOSEST_m10        6

// Text Color Constant Strings
#ifdef MATLAB_m10  // Matlab doesn't do text coloring this way (can be done with CPRINTF())
	#define TC_BLACK_m10            ""
	#define TC_RED_m10              ""
	#define TC_GREEN_m10            ""
	#define TC_YELLOW_m10           ""
	#define TC_BLUE_m10             ""
	#define TC_MAGENTA_m10          ""
	#define TC_CYAN_m10             ""
	#define TC_LIGHT_GRAY_m10	""
	#define TC_DARK_GRAY_m10	""
	#define TC_LIGHT_RED_m10	""
	#define TC_LIGHT_GREEN_m10	""
	#define TC_LIGHT_YELLOW_m10	""
	#define TC_LIGHT_BLUE_m10	""
	#define TC_LIGHT_MAGENTA_m10	""
	#define TC_LIGHT_CYAN_m10	""
	#define TC_WHITE_m10		""
	#define TC_BRIGHT_BLACK_m10	""
	#define TC_BRIGHT_RED_m10	""
	#define TC_BRIGHT_GREEN_m10	""
	#define TC_BRIGHT_YELLOW_m10	""
	#define TC_BRIGHT_BLUE_m10	""
	#define TC_BRIGHT_MAGENTA_m10	""
	#define TC_BRIGHT_CYAN_m10	""
	#define TC_BRIGHT_WHITE_m10	""
	#define TC_RESET_m10		""
	// non-color constants
	#define TC_BOLD_m10		""
	#define TC_BOLD_RESET_m10	""
	#define TC_UNDERLINE_m10	""
	#define TC_UNDERLINE_RESET_m10	""
#else
	#define TC_BLACK_m10		"\033[30m"
	#define TC_RED_m10		"\033[31m"
	#define TC_GREEN_m10		"\033[32m"
	#define TC_YELLOW_m10		"\033[33m"
	#define TC_BLUE_m10		"\033[34m"
	#define TC_MAGENTA_m10		"\033[35m"
	#define TC_CYAN_m10		"\033[36m"
	#define TC_LIGHT_GRAY_m10	"\033[37m"
	#define TC_DARK_GRAY_m10	"\033[90m"
	#define TC_LIGHT_RED_m10	"\033[91m"
	#define TC_LIGHT_GREEN_m10	"\033[92m"
	#define TC_LIGHT_YELLOW_m10	"\033[93m"
	#define TC_LIGHT_BLUE_m10	"\033[94m"
	#define TC_LIGHT_MAGENTA_m10	"\033[95m"
	#define TC_LIGHT_CYAN_m10	"\033[96m"
	#define TC_WHITE_m10		"\033[97m"
	#define TC_BRIGHT_BLACK_m10	"\033[30;1m"
	#define TC_BRIGHT_RED_m10	"\033[31;1m"
	#define TC_BRIGHT_GREEN_m10	"\033[32;1m"
	#define TC_BRIGHT_YELLOW_m10	"\033[33;1m"
	#define TC_BRIGHT_BLUE_m10	"\033[34;1m"
	#define TC_BRIGHT_MAGENTA_m10	"\033[35;1m"
	#define TC_BRIGHT_CYAN_m10	"\033[36;1m"
	#define TC_BRIGHT_WHITE_m10	"\033[37;1m"
	#define TC_RESET_m10		"\033[0m"
	// non-color constants
	#define TC_BOLD_m10		"\033[1m"
	#define TC_BOLD_RESET_m10	"\033[21m"
	#define TC_UNDERLINE_m10	"\033[4m"
	#define TC_UNDERLINE_RESET_m10	"\033[24m"
#endif

// Time Related Constants
#define TIMEZONE_ACRONYM_BYTES_m10                      8       // ascii[7]
#define TIMEZONE_STRING_BYTES_m10                       64      // ascii[63]
#define MAXIMUM_STANDARD_UTC_OFFSET_m10                 ((si4) 86400)
#define MINIMUM_STANDARD_UTC_OFFSET_m10                 ((si4) -86400)
#define STANDARD_UTC_OFFSET_NO_ENTRY_m10                ((si4) -86401)
#define MAXIMUM_DST_OFFSET_m10                          7200
#define MINIMUM_DST_OFFSET_m10                          0
#define DST_OFFSET_NO_ENTRY_m10                         -1
#define TIME_STRING_BYTES_m10                           128
#define NUMBER_OF_SAMPLES_NO_ENTRY_m10			-1
#define SAMPLE_NUMBER_NO_ENTRY_m10                      ((si8) 0x8000000000000000)
#define FRAME_NUMBER_NO_ENTRY_m10                       SAMPLE_NUMBER_NO_ENTRY_m10
#define BEGINNING_OF_SAMPLE_NUMBERS_m10                 ((si8) 0x000000000)
#define END_OF_SAMPLE_NUMBERS_m10                       ((si8) 0x7FFFFFFFFFFFFFFF)
#define UUTC_NO_ENTRY_m10                               ((si8) 0x8000000000000000)
#define UUTC_EARLIEST_TIME_m10                          ((si8) 0x000000000)  // 00:00:00.000000 Thursday, 1 Jan 1970, UTC
#define UUTC_LATEST_TIME_m10                            ((si8) 0x7FFFFFFFFFFFFFFF)  // 04:00:54.775808 Sunday, 10 Jan 29424, UTC
#define BEGINNING_OF_TIME_m10                           UUTC_EARLIEST_TIME_m10
#define END_OF_TIME_m10                                 UUTC_LATEST_TIME_m10
#define CURRENT_TIME_m10				((si8) 0xFFFFFFFFFFFFFFFF)  // used with time_string_m10() & generate_recording_time_offset_m10()
#define TWENTY_FOURS_HOURS_m10				((si8) 86400000000)
#define Y2K_m10                                         ((si8) 0x00035D013B37E000)  // 00:00:00.000000 Saturday, 1 Jan 2000, UTC  (946684800000000 decimal)
#define WIN_TICKS_PER_USEC_m10				((si8) 10)
#define WIN_USECS_TO_EPOCH_m10				((si8) 11644473600000000)

// Time Change Code Constants
#define DTCC_VALUE_NOT_OBSERVED_m10                     0
#define DTCC_VALUE_NO_ENTRY_m10                         -1
#define DTCC_VALUE_DEFAULT_m10                          DTCC_VALUE_NO_ENTRY_m10
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

// Global Defaults
#define GLOBALS_VERBOSE_DEFAULT_m10                             FALSE_m10
#define GLOBALS_BEHAVIOR_ON_FAIL_DEFAULT_m10		        EXIT_ON_FAIL_m10
#define GLOBALS_CRC_MODE_DEFAULT_m10			        CRC_CALCULATE_ON_OUTPUT_m10

// Global Time Defaults
#define GLOBALS_SESSION_START_TIME_OFFSET_DEFAULT_m10		0
#define GLOBALS_RECORDING_TIME_OFFSET_DEFAULT_m10               0
#define GLOBALS_RECORDING_TIME_OFFSET_NO_ENTRY_m10              0
#define GLOBALS_RECORDING_TIME_OFFSET_MODE_DEFAULT_m10	        (RTO_APPLY_ON_OUTPUT_m10 | RTO_REMOVE_ON_INPUT_m10)
#define GLOBALS_STANDARD_UTC_OFFSET_DEFAULT_m10                 0
#define GLOBALS_OBSERVE_DST_DEFAULT_m10				FALSE_m10
#define GLOBALS_RTO_KNOWN_DEFAULT_m10				FALSE_m10
#define GLOBALS_STANDARD_TIMEZONE_ACRONYM_DEFAULT_m10	        "oUTC"
#define GLOBALS_STANDARD_TIMEZONE_STRING_DEFAULT_m10            "offset Coordinated Universal Time"
#define GLOBALS_DAYLIGHT_TIMEZONE_ACRONYM_DEFAULT_m10           ""
#define GLOBALS_DAYLIGHT_TIMEZONE_STRING_DEFAULT_m10            ""

// File Type Constants
#define NO_TYPE_CODE_m10                                        (ui4) 0x0
#define ALL_TYPES_CODE_m10                                      (ui4) 0xFFFFFFFF
#define UNKNOWN_TYPE_CODE_m10                                   NO_TYPE_CODE_m10
#define NO_FILE_TYPE_STRING_m10				        ""			// ascii[4]
#define NO_FILE_TYPE_CODE_m10				        NO_TYPE_CODE_m10	// ui4 (big & little endian)
#define SESSION_DIRECTORY_TYPE_STRING_m10		        "medd"			// ascii[4]
#define SESSION_DIRECTORY_TYPE_CODE_m10                         (ui4) 0x6464656D	// ui4 (little endian)
// #define SESSION_DIRECTORY_TYPE_CODE_m10                      (ui4) 0x6D656464        // ui4 (big endian)
#define TIME_SERIES_CHANNEL_DIRECTORY_TYPE_STRING_m10           "ticd"                  // ascii[4]
#define TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m10             (ui4) 0x64636974        // ui4 (little endian)
// #define TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m10          (ui4) 0x74696364        // ui4 (big endian)
#define TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m10           "tisd"                  // ascii[4]
#define TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10             (ui4) 0x64736974        // ui4 (little endian)
// #define TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10          (ui4) 0x74697364        // ui4 (big endian)
#define TIME_SERIES_METADATA_FILE_TYPE_STRING_m10               "tmet"                  // ascii[4]
#define TIME_SERIES_METADATA_FILE_TYPE_CODE_m10                 (ui4) 0x74656D74        // ui4 (little endian)
// #define TIME_SERIES_METADATA_FILE_TYPE_CODE_m10              (ui4) 0x746D6574        // ui4 (big endian)
#define TIME_SERIES_DATA_FILE_TYPE_STRING_m10                   "tdat"                  // ascii[4]
#define TIME_SERIES_DATA_FILE_TYPE_CODE_m10                     (ui4) 0x74616474        // ui4 (little endian)
// #define TIME_SERIES_DATA_FILE_TYPE_CODE_m10                  (ui4) 0x74646174        // ui4 (big endian)
#define TIME_SERIES_INDICES_FILE_TYPE_STRING_m10                "tidx"                  // ascii[4]
#define TIME_SERIES_INDICES_FILE_TYPE_CODE_m10                  (ui4) 0x78646974        // ui4 (little endian)
// #define TIME_SERIES_INDICES_FILE_TYPE_CODE_m10               (ui4) 0x74696478        // ui4 (big endian)
#define VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m10                 "visd"                  // ascii[4]
#define VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10                   (ui4) 0x64736976        // ui4 (little endian)
// #define VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10                (ui4) 0x76697364        // ui4 (big endian)
#define VIDEO_CHANNEL_DIRECTORY_TYPE_STRING_m10                 "vicd"                  // ascii[4]
#define VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m10                   (ui4) 0x64636976        // ui4 (little endian)
// #define VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m10                (ui4) 0x76696364        // ui4 (big endian)
#define VIDEO_METADATA_FILE_TYPE_STRING_m10                     "vmet"                  // ascii[4]
#define VIDEO_METADATA_FILE_TYPE_CODE_m10                       (ui4) 0x74656D76        // ui4 (little endian)
// #define VIDEO_METADATA_FILE_TYPE_CODE_m10                    (ui4) 0x766D6574        // ui4 (big endian)
#define VIDEO_INDICES_FILE_TYPE_STRING_m10                      "vidx"                  // ascii[4]
#define VIDEO_INDICES_FILE_TYPE_CODE_m10                        (ui4) 0x78646976        // ui4 (little endian)
// #define VIDEO_INDICES_FILE_TYPE_CODE_m10                     (ui4) 0x76696478        // ui4 (big endian)
#define RECORD_DIRECTORY_TYPE_STRING_m10                        "recd"                        // ascii[4]
#define RECORD_DIRECTORY_TYPE_CODE_m10                          (ui4) 0x64636572        // ui4 (little endian)
// #define RECORD_DIRECTORY_TYPE_CODE_m10                       (ui4) 0x72656364        // ui4 (big endian)
#define RECORD_DATA_FILE_TYPE_STRING_m10                        "rdat"			// ascii[4]
#define RECORD_DATA_FILE_TYPE_CODE_m10                          (ui4) 0x74616472	// ui4 (little endian)
// #define RECORD_DATA_FILE_TYPE_CODE_m10                       (ui4) 0x72646174	// ui4 (big endian)
#define RECORD_INDICES_FILE_TYPE_STRING_m10                     "ridx"			// ascii[4]
#define RECORD_INDICES_FILE_TYPE_CODE_m10                       (ui4) 0x78646972	// ui4 (little endian)
// #define RECORD_INDICES_FILE_TYPE_CODE_m10                    (ui4) 0x72696478	// ui4 (big endian)

// Channel Types
#define UNKNOWN_CHANNEL_TYPE_m10	NO_FILE_TYPE_CODE_m10
#define TIME_SERIES_CHANNEL_TYPE_m10	TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m10
#define VIDEO_CHANNEL_TYPE_m10		VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m10

// Path Parts
#define PP_PATH_m10             1
#define PP_NAME_m10             2
#define PP_EXTENSION_m10        4
#define PP_FULL_PATH_m10        (PP_PATH_m10 | PP_NAME_m10 | PP_EXTENSION_m10)

#define NO_SPACES_m10                           0
#define ESCAPED_SPACES_m10                      1
#define UNESCAPED_SPACES_m10                    2
#define ALL_SPACES_m10                          (ESCAPED_SPACES_m10 | UNESCAPED_SPACES_m10)

// File Processing Constants
#define FPS_FILE_LENGTH_UNKNOWN_m10		-1
#define FPS_FULL_FILE_m10			-1
#define FPS_UNIVERSAL_HEADER_ONLY_m10		0
#define FPS_NO_LOCK_TYPE_m10			~(F_RDLCK | F_WRLCK | F_UNLCK)  // from <fcntl.h>
#define FPS_NO_LOCK_MODE_m10			0
#define FPS_READ_LOCK_ON_READ_OPEN_m10		1
#define FPS_WRITE_LOCK_ON_READ_OPEN_m10		2
#define FPS_WRITE_LOCK_ON_WRITE_OPEN_m10        4
#define FPS_WRITE_LOCK_ON_READ_WRITE_OPEN_m10   8
#define FPS_READ_LOCK_ON_READ_m10               16
#define FPS_WRITE_LOCK_ON_WRITE_m10             32
#define FPS_NO_OPEN_MODE_m10		        0
#define FPS_R_OPEN_MODE_m10			1
#define FPS_R_PLUS_OPEN_MODE_m10                2
#define FPS_W_OPEN_MODE_m10                     4
#define FPS_W_PLUS_OPEN_MODE_m10                8
#define FPS_A_OPEN_MODE_m10                     16
#define FPS_A_PLUS_OPEN_MODE_m10                32
#define FPS_GENERIC_READ_OPEN_MODE_m10		(FPS_R_OPEN_MODE_m10 | FPS_R_PLUS_OPEN_MODE_m10 | FPS_W_PLUS_OPEN_MODE_m10 | FPS_A_PLUS_OPEN_MODE_m10)
#define FPS_GENERIC_WRITE_OPEN_MODE_m10		(FPS_R_PLUS_OPEN_MODE_m10 | FPS_W_OPEN_MODE_m10 | FPS_W_PLUS_OPEN_MODE_m10 | FPS_A_OPEN_MODE_m10 | FPS_A_PLUS_OPEN_MODE_m10)
#define FPS_PROTOTYPE_FILE_TYPE_CODE_m10        TIME_SERIES_METADATA_FILE_TYPE_CODE_m10  // any metadata type would do
#define FPS_FD_NO_ENTRY_m10                     -2
#define FPS_FD_EPHEMERAL_m10                    -3

// File Processing Directives Defaults
#define FPS_DIRECTIVES_CLOSE_FILE_DEFAULT_m10		        TRUE_m10
#define FPS_DIRECTIVES_FLUSH_AFTER_WRITE_DEFAULT_m10		TRUE_m10
#define FPS_DIRECTIVES_FREE_CMP_PROCESSING_STRUCT_DEFAULT_m10	TRUE_m10
#define FPS_DIRECTIVES_FREE_PASSWORD_DATA_DEFAULT_m10		FALSE_m10
#define FPS_DIRECTIVES_UPDATE_UNIVERSAL_HEADER_DEFAULT_m10	FALSE_m10
#define FPS_DIRECTIVES_LEAVE_DECRYPTED_DEFAULT_m10		FALSE_m10
#define FPS_DIRECTIVES_LOCK_MODE_DEFAULT_m10		        FPS_NO_LOCK_MODE_m10  // Unix file locking may cause problems with networked file systems
// #define FPS_DIRECTIVES_LOCK_MODE_DEFAULT_m10                     (FPS_READ_LOCK_ON_READ_OPEN_m10 | FPS_WRITE_LOCK_ON_WRITE_OPEN_m10 | FPS_WRITE_LOCK_ON_READ_WRITE_OPEN_m10)
#define FPS_DIRECTIVES_OPEN_MODE_DEFAULT_m10		        FPS_NO_OPEN_MODE_m10

// Universal Header: File Format Constants
#define UNIVERSAL_HEADER_OFFSET_m10					0
#define UNIVERSAL_HEADER_BYTES_m10					1024    // 1 kB
#define UNIVERSAL_HEADER_HEADER_CRC_OFFSET_m10				0       // ui4
#define UNIVERSAL_HEADER_BODY_CRC_OFFSET_m10				4       // ui4
#define UNIVERSAL_HEADER_HEADER_CRC_START_OFFSET_m10			UNIVERSAL_HEADER_BODY_CRC_OFFSET_m10
#define UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10			UNIVERSAL_HEADER_BYTES_m10
#define UNIVERSAL_HEADER_FILE_END_TIME_OFFSET_m10			8	// si8
#define UNIVERSAL_HEADER_NUMBER_OF_ENTRIES_OFFSET_m10			16      // si8
#define UNIVERSAL_HEADER_NUMBER_OF_ENTRIES_NO_ENTRY_m10			-1
#define UNIVERSAL_HEADER_MAXIMUM_ENTRY_SIZE_OFFSET_m10			24      // ui4
#define UNIVERSAL_HEADER_MAXIMUM_ENTRY_SIZE_NO_ENTRY_m10		0
#define UNIVERSAL_HEADER_SEGMENT_NUMBER_OFFSET_m10                      28      // si4
#define UNIVERSAL_HEADER_SEGMENT_NUMBER_NO_ENTRY_m10                    SEGMENT_NUMBER_NO_ENTRY_m10
// #define UNIVERSAL_HEADER_SEGMENT_LEVEL_CODE_m10			-1      // unused at this time
#define UNIVERSAL_HEADER_CHANNEL_LEVEL_CODE_m10				-2
#define UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m10				-3
#define UNIVERSAL_HEADER_TYPE_STRING_OFFSET_m10				32       // ascii[4]
#define UNIVERSAL_HEADER_TYPE_STRING_TERMINAL_ZERO_OFFSET_m10		(UNIVERSAL_HEADER_TYPE_STRING_OFFSET_m10 + 4)  // si1
#define UNIVERSAL_HEADER_TYPE_CODE_OFFSET_m10				UNIVERSAL_HEADER_TYPE_STRING_OFFSET_m10  // ui4
#define UNIVERSAL_HEADER_TYPE_NO_ENTRY_m10				0       // zero as ui4 or zero-length string as ascii[4]
#define UNIVERSAL_HEADER_MED_VERSION_MAJOR_OFFSET_m10			37     // ui1
#define UNIVERSAL_HEADER_MED_VERSION_MAJOR_NO_ENTRY_m10			MED_VERSION_NO_ENTRY_m10
#define UNIVERSAL_HEADER_MED_VERSION_MINOR_OFFSET_m10			38      // ui1
#define UNIVERSAL_HEADER_MED_VERSION_MINOR_NO_ENTRY_m10			MED_VERSION_NO_ENTRY_m10
#define UNIVERSAL_HEADER_BYTE_ORDER_CODE_OFFSET_m10			39      // ui1
#define UNIVERSAL_HEADER_BYTE_ORDER_CODE_NO_ENTRY_m10			0xFF
#define UNIVERSAL_HEADER_SESSION_START_TIME_OFFSET_m10			40      // si8
#define UNIVERSAL_HEADER_FILE_START_TIME_OFFSET_m10			48      // si8
#define UNIVERSAL_HEADER_SESSION_NAME_OFFSET_m10                        56      // utf8[63]
#define UNIVERSAL_HEADER_CHANNEL_NAME_OFFSET_m10                        312     // utf8[63]
#define UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_OFFSET_m10             	568     // utf8[63]
#define UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_BYTES_m10              	BASE_FILE_NAME_BYTES_m10
#define UNIVERSAL_HEADER_SESSION_UID_OFFSET_m10                         824     // ui8
#define UNIVERSAL_HEADER_CHANNEL_UID_OFFSET_m10                         832     // ui8
#define UNIVERSAL_HEADER_SEGMENT_UID_OFFSET_m10                         840     // ui8
#define UNIVERSAL_HEADER_FILE_UID_OFFSET_m10				848     // ui8
#define UNIVERSAL_HEADER_PROVENANCE_UID_OFFSET_m10			856     // ui8
#define UNIVERSAL_HEADER_LEVEL_1_PASSWORD_VALIDATION_FIELD_OFFSET_m10	864     // ui1
#define UNIVERSAL_HEADER_LEVEL_2_PASSWORD_VALIDATION_FIELD_OFFSET_m10	880     // ui1
#define UNIVERSAL_HEADER_LEVEL_3_PASSWORD_VALIDATION_FIELD_OFFSET_m10   896     // ui1
#define UNIVERSAL_HEADER_PROTECTED_REGION_OFFSET_m10			912
#define UNIVERSAL_HEADER_PROTECTED_REGION_BYTES_m10			56
#define UNIVERSAL_HEADER_DISCRETIONARY_REGION_OFFSET_m10		968
#define UNIVERSAL_HEADER_DISCRETIONARY_REGION_BYTES_m10			56

// Metadata: File Format Constants
#define METADATA_BYTES_m10			15360   // 15 kB
#define FPS_PROTOTYPE_BYTES_m10			METADATA_BYTES_m10
#define METADATA_FILE_BYTES_m10			(METADATA_BYTES_m10 + UNIVERSAL_HEADER_BYTES_m10)	// 16 kB
#define METADATA_SECTION_1_OFFSET_m10		1024
#define METADATA_SECTION_1_BYTES_m10		1024	// 1 kB
#define METADATA_SECTION_2_OFFSET_m10		2048
#define METADATA_SECTION_2_BYTES_m10		10240   // 10 kB
#define METADATA_SECTION_3_OFFSET_m10		12288
#define METADATA_SECTION_3_BYTES_m10		4096    // 4 kB

// Metadata: File Format Constants - Section 1 Fields
#define METADATA_LEVEL_1_PASSWORD_HINT_OFFSET_m10		1024	// utf8[63]
#define METADATA_LEVEL_2_PASSWORD_HINT_OFFSET_m10		1280    // utf8[63]
#define METADATA_SECTION_2_ENCRYPTION_LEVEL_OFFSET_m10		1536    // si1
#define METADATA_SECTION_2_ENCRYPTION_LEVEL_DEFAULT_m10		LEVEL_1_ENCRYPTION_m10
#define METADATA_SECTION_3_ENCRYPTION_LEVEL_OFFSET_m10		1537    // si1
#define METADATA_SECTION_3_ENCRYPTION_LEVEL_DEFAULT_m10		LEVEL_2_ENCRYPTION_m10
#define METADATA_SECTION_1_PROTECTED_REGION_OFFSET_m10		1538
#define METADATA_SECTION_1_PROTECTED_REGION_BYTES_m10		254
#define METADATA_SECTION_1_DISCRETIONARY_REGION_OFFSET_m10	1792
#define METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m10	256

// Metadata: File Format Constants - Section 2 Channel Type Independent Fields
#define METADATA_SESSION_DESCRIPTION_OFFSET_m10                 2048    // utf8[511]
#define METADATA_SESSION_DESCRIPTION_BYTES_m10                  2048
#define METADATA_CHANNEL_DESCRIPTION_OFFSET_m10                 4096    // utf8[255]
#define METADATA_CHANNEL_DESCRIPTION_BYTES_m10                  1024
#define METADATA_SEGMENT_DESCRIPTION_OFFSET_m10                 5120    // utf8[255]
#define METADATA_SEGMENT_DESCRIPTION_BYTES_m10                  1024
#define METADATA_EQUIPMENT_DESCRIPTION_OFFSET_m10               6144    // utf8[510]
#define METADATA_EQUIPMENT_DESCRIPTION_BYTES_m10                2044
#define METADATA_ACQUISITION_CHANNEL_NUMBER_OFFSET_m10          8188    // si4
#define METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m10        CHANNEL_NUMBER_NO_ENTRY_m10

// Metadata: File Format Constants - Time Series Section 2 Fields
#define TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_OFFSET_m10                   8192            // utf8[255]
#define TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m10                    1024
#define TIME_SERIES_METADATA_SAMPLING_FREQUENCY_OFFSET_m10                      9216            // sf8
#define TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10                             FREQUENCY_NO_ENTRY_m10
#define TIME_SERIES_METADATA_FREQUENCY_VARIABLE_m10				FREQUENCY_VARIABLE_m10
#define TIME_SERIES_METADATA_LOW_FREQUENCY_FILTER_SETTING_OFFSET_m10            9224            // sf8
#define TIME_SERIES_METADATA_HIGH_FREQUENCY_FILTER_SETTING_OFFSET_m10           9232            // sf8
#define TIME_SERIES_METADATA_NOTCH_FILTER_FREQUENCY_SETTING_OFFSET_m10          9240            // sf8
#define TIME_SERIES_METADATA_AC_LINE_FREQUENCY_OFFSET_m10                       9248            // sf8
#define TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_OFFSET_m10       9256            // sf8
#define TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10     0.0
#define TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_OFFSET_m10             9264            // utf8[31]
#define TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m10              128
#define TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m10       9392            // sf8
#define TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10     0.0
#define TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_OFFSET_m10             9400            // utf8[31]
#define TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m10              128
#define TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_OFFSET_m10            9528
#define TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_NO_ENTRY_m10          SAMPLE_NUMBER_NO_ENTRY_m10
#define TIME_SERIES_METADATA_NUMBER_OF_SAMPLES_OFFSET_m10                       9536            // si8
#define TIME_SERIES_METADATA_NUMBER_OF_SAMPLES_NO_ENTRY_m10                     NUMBER_OF_SAMPLES_NO_ENTRY_m10
#define TIME_SERIES_METADATA_NUMBER_OF_BLOCKS_OFFSET_m10                        9544            // si8
#define TIME_SERIES_METADATA_NUMBER_OF_BLOCKS_NO_ENTRY_m10                      -1
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_BYTES_OFFSET_m10                     9552            // si8
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_BYTES_NO_ENTRY_m10                   -1
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_SAMPLES_OFFSET_m10                   9560            // ui4
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_SAMPLES_NO_ENTRY_m10                 0xFFFFFFFF
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_DIFFERENCE_BYTES_OFFSET_m10          9564            // ui4
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_DIFFERENCE_BYTES_NO_ENTRY_m10        0xFFFFFFFF
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_DURATION_OFFSET_m10                  9568            // sf8
#define TIME_SERIES_METADATA_MAXIMUM_BLOCK_DURATION_NO_ENTRY_m10                -1.0
#define TIME_SERIES_METADATA_BLOCK_DURATION_VARIABLE_m10                        -2.0
#define TIME_SERIES_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m10               9576            // si8
#define TIME_SERIES_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m10             -1
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_OFFSET_m10               9584            // si8
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_NO_ENTRY_m10             -1
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_OFFSET_m10          9592            // si8
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_NO_ENTRY_m10        -1
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_OFFSET_m10              9600            // si8
#define TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_NO_ENTRY_m10            -1
#define TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m10              9608
#define TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m10               1344
#define TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m10          10952
#define TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m10           1336

// Metadata: File Format Constants - Video Section 2 Fields
#define VIDEO_METADATA_HORIZONTAL_RESOLUTION_OFFSET_m10                 8192    // si8
#define VIDEO_METADATA_HORIZONTAL_RESOLUTION_NO_ENTRY_m10               -1
#define VIDEO_METADATA_VERTICAL_RESOLUTION_OFFSET_m10                   8200    // si8
#define VIDEO_METADATA_VERTICAL_RESOLUTION_NO_ENTRY_m10                 -1
#define VIDEO_METADATA_FRAME_RATE_OFFSET_m10                            8208    // sf8
#define VIDEO_METADATA_FRAME_RATE_NO_ENTRY_m10                          FRAME_RATE_NO_ENTRY_m10
#define VIDEO_METADATA_NUMBER_OF_CLIPS_OFFSET_m10                       8216
#define VIDEO_METADATA_NUMBER_OF_CLIPS_NO_ENTRY_m10                     -1
#define VIDEO_METADATA_MAXIMUM_CLIP_BYTES_OFFSET_m10                    8224
#define VIDEO_METADATA_MAXIMUM_CLIP_BYTES_NO_ENTRY_m10                  -1
#define VIDEO_METADATA_VIDEO_FORMAT_OFFSET_m10                          8232    // utf8[63]
#define VIDEO_METADATA_VIDEO_FORMAT_BYTES_m10                           256
#define VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_OFFSET_m10                 8488    // si4
#define VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_NO_ENTRY_m10               -1
#define VIDEO_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m10            8492
#define VIDEO_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m10             1900
#define VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m10        10392
#define VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m10         1896

// Metadata: File Format Constants - Section 3 Fields
#define METADATA_RECORDING_TIME_OFFSET_OFFSET_m10               12288           // si8
#define METADATA_RECORDING_TIME_OFFSET_NO_ENTRY_m10             GLOBALS_RECORDING_TIME_OFFSET_NO_ENTRY_m10
#define METADATA_DAYLIGHT_TIME_START_CODE_OFFSET_m10            12296           // DAYLIGHT_TIME_CHANGE_CODE_m10 (si1[8])
#define METADATA_DAYLIGHT_TIME_START_CODE_NO_ENTRY_m10          DTCC_VALUE_NO_ENTRY_m10
#define METADATA_DAYLIGHT_TIME_END_CODE_OFFSET_m10              12304           // DAYLIGHT_TIME_CHANGE_CODE_m10 (si1[8])
#define METADATA_DAYLIGHT_TIME_END_CODE_NO_ENTRY_m10            DTCC_VALUE_NO_ENTRY_m10
#define METADATA_STANDARD_TIMEZONE_ACRONYM_OFFSET_m10           12312           // ascii[7]
#define METADATA_STANDARD_TIMEZONE_ACRONYM_BYTES_m10            TIMEZONE_ACRONYM_BYTES_m10
#define METADATA_STANDARD_TIMEZONE_STRING_OFFSET_m10            12320           // ascii[63]
#define METADATA_STANDARD_TIMEZONE_STRING_BYTES_m10             TIMEZONE_STRING_BYTES_m10
#define METADATA_DAYLIGHT_TIMEZONE_ACRONYM_OFFSET_m10           12384           // ascii[7]
#define METADATA_DAYLIGHT_TIMEZONE_ACRONYM_BYTES_m10            TIMEZONE_ACRONYM_BYTES_m10
#define METADATA_DAYLIGHT_TIMEZONE_STRING_OFFSET_m10            12392           // ascii[63]
#define METADATA_DAYLIGHT_TIMEZONE_STRING_BYTES_m10             TIMEZONE_STRING_BYTES_m10
#define METADATA_SUBJECT_NAME_1_OFFSET_m10                      12456           // utf8[31]
#define METADATA_SUBJECT_NAME_BYTES_m10                         128
#define METADATA_SUBJECT_NAME_2_OFFSET_m10                      12584           // utf8[31]
#define METADATA_SUBJECT_NAME_3_OFFSET_m10                      12712           // utf8[31]
#define METADATA_SUBJECT_ID_OFFSET_m10                          12840           // utf8[31]
#define METADATA_SUBJECT_ID_BYTES_m10                           128
#define METADATA_RECORDING_COUNTRY_OFFSET_m10                   12968           // utf8[63]
#define METADATA_RECORDING_TERRITORY_OFFSET_m10                 13224           // utf8[63]
#define METADATA_RECORDING_LOCALITY_OFFSET_m10                  13480           // utf8[63]
#define METADATA_RECORDING_INSTITUTION_OFFSET_m10               13736           // utf8[63]
#define METADATA_RECORDING_LOCATION_BYTES_m10                   256
#define METADATA_GEOTAG_FORMAT_OFFSET_m10                       13992           // ascii[31]
#define METADATA_GEOTAG_FORMAT_BYTES_m10                        32
#define METADATA_GEOTAG_DATA_OFFSET_m10                         14024           // ascii[1023]
#define METADATA_GEOTAG_DATA_BYTES_m10                          1024
#define METADATA_STANDARD_UTC_OFFSET_OFFSET_m10                 15048           // si4
#define METADATA_STANDARD_UTC_OFFSET_NO_ENTRY_m10               GLOBALS_STANDARD_UTC_OFFSET_NO_ENTRY_m10
#define METADATA_SECTION_3_PROTECTED_REGION_OFFSET_m10          15052
#define METADATA_SECTION_3_PROTECTED_REGION_BYTES_m10           668
#define METADATA_SECTION_3_DISCRETIONARY_REGION_OFFSET_m10      15720
#define METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m10       664

// Records: Header Format Constants
#define RECORD_HEADER_BYTES_m10			                        24
#define RECORD_HEADER_RECORD_CRC_OFFSET_m10		                0                       // ui4
#define RECORD_HEADER_RECORD_CRC_NO_ENTRY_m10	                        CRC_NO_ENTRY_m10
#define RECORD_HEADER_TOTAL_RECORD_BYTES_OFFSET_m10                     4                       // ui4
#define RECORD_HEADER_TOTAL_RECORD_BYTES_NO_ENTRY_m10			0
#define RECORD_HEADER_RECORD_CRC_START_OFFSET_m10			RECORD_HEADER_TOTAL_RECORD_BYTES_OFFSET_m10
#define RECORD_HEADER_START_TIME_OFFSET_m10                             8                       // si8
#define RECORD_HEADER_START_TIME_NO_ENTRY_m10                           UUTC_NO_ENTRY_m10       // si8
#define RECORD_HEADER_TYPE_STRING_OFFSET_m10                            16	                // ascii[4]
#define RECORD_HEADER_TYPE_STRING_TERMINAL_ZERO_OFFSET_m10              (RECORD_HEADER_TYPE_STRING_OFFSET_m10 + 4)	// si1
#define RECORD_HEADER_TYPE_CODE_OFFSET_m10                              RECORD_HEADER_TYPE_STRING_OFFSET_m10		// ui4
#define RECORD_HEADER_TYPE_CODE_NO_ENTRY_m10		                0	                // ui4
#define RECORD_HEADER_VERSION_MAJOR_OFFSET_m10	                        21	                // ui1
#define RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m10	                0xFF
#define RECORD_HEADER_VERSION_MINOR_OFFSET_m10	                        22	                // ui1
#define RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m10	                0xFF
#define RECORD_HEADER_ENCRYPTION_LEVEL_OFFSET_m10                       23	                // si1
#define RECORD_HEADER_ENCRYPTION_LEVEL_NO_ENTRY_m10                     ENCRYPTION_LEVEL_NO_ENTRY_m10

// Record Index: Format Constants
#define RECORD_INDEX_BYTES_m10                                          24
#define RECORD_INDEX_FILE_OFFSET_OFFSET_m10                             0                       // si8
#define RECORD_INDEX_FILE_OFFSET_NO_ENTRY_m10                           -1
#define RECORD_INDEX_START_TIME_OFFSET_m10                              8                       // si8
#define RECORD_INDEX_START_TIME_NO_ENTRY_m10                            UUTC_NO_ENTRY_m10
#define RECORD_INDEX_TYPE_STRING_OFFSET_m10                             16                      // ascii[4]
#define RECORD_INDEX_TYPE_STRING_TERMINAL_ZERO_OFFSET_m10               (RECORD_INDEX_TYPE_STRING_OFFSET_m10 + 4)	// si1
#define RECORD_INDEX_TYPE_CODE_OFFSET_m10                               RECORD_INDEX_TYPE_STRING_OFFSET_m10		// as ui4
#define RECORD_INDEX_TYPE_CODE_NO_ENTRY_m10                             0                       // as ui4
#define RECORD_INDEX_VERSION_MAJOR_OFFSET_m10	                        21                      // ui1
#define RECORD_INDEX_VERSION_MAJOR_NO_ENTRY_m10	                        0xFF
#define RECORD_INDEX_VERSION_MINOR_OFFSET_m10	                        22                      // ui1
#define RECORD_INDEX_VERSION_MINOR_NO_ENTRY_m10	                        0xFF
#define RECORD_INDEX_ENCRYPTION_LEVEL_OFFSET_m10                        23                      // si1
#define RECORD_INDEX_ENCRYPTION_LEVEL_NO_ENTRY_m10                      ENCRYPTION_LEVEL_NO_ENTRY_m10

// Time Series Index: Format Constants
#define TIME_SERIES_INDEX_BYTES_m10                                     24
#define TIME_SERIES_INDEX_FILE_OFFSET_OFFSET_m10                        0               // si8
#define TIME_SERIES_INDEX_FILE_OFFSET_NO_ENTRY_m10                      -1
#define TIME_SERIES_INDEX_START_TIME_OFFSET_m10			        8               // si8
#define TIME_SERIES_INDEX_START_TIME_NO_ENTRY_m10                       UUTC_NO_ENTRY_m10
#define TIME_SERIES_INDEX_START_SAMPLE_NUMBER_OFFSET_m10                16              // si8
#define TIME_SERIES_INDEX_START_SAMPLE_NUMBER_NO_ENTRY_m10              -1

// Video Index: Format Constants
#define VIDEO_INDEX_BYTES_m10			                24
#define VIDEO_INDEX_FILE_OFFSET_OFFSET_m10                      0                       // si8
#define VIDEO_INDEX_FILE_OFFSET_NO_ENTRY_m10                    -1
#define VIDEO_INDEX_START_TIME_OFFSET_m10                       8                       // si8
#define VIDEO_INDEX_START_TIME_NO_ENTRY_m10                     UUTC_NO_ENTRY_m10
#define VIDEO_INDEX_START_FRAME_OFFSET_m10                      16                      // ui4
#define VIDEO_INDEX_START_FRAME_NO_ENTRY_m10                    0xFFFFFFFF
#define VIDEO_INDEX_VIDEO_FILE_NUMBER_OFFSET_m10                20                      // ui4
#define VIDEO_INDEX_VIDEO_FILE_NUMBER_NO_ENTRY_m10              0
#define VIDEO_INDEX_TERMINAL_VIDEO_FILE_NUMBER_m10              0xFFFFFFFF


//**********************************************************************************//
//**********************************  MED Macros  **********************************//
//**********************************************************************************//

#define ABS_m10(x)			( ((x) >= 0) ? (x) : -(x) )  // do not increment/decrement in call to ABS (as x occurs thrice)
#define HEX_STRING_BYTES_m10(x)         ( ((x) + 1) * 3 )
#define REMOVE_DISCONTINUITY_m10(x)     ( ((x) >= 0) ? (x) : -(x) )  // do not increment/decrement in call to REMOVE_DISCONTINUITY (as x occurs thrice)
#define APPLY_DISCONTINUITY_m10(x)      ( ((x) < 0) ? (x) : -(x) )  // do not increment/decrement in call to APPLY_DISCONTINUITY (as x occurs thrice)
#define MAX_OPEN_FILES_m10(number_of_channels, number_of_segments)      ((5 * number_of_channels * number_of_segments) + (2 * number_of_segments) + (2 * number_of_channels) + 5)
// Note: final +5 == 2 for session level records plus 3 for standard streams (stdin, stdout, & stderr)


//**********************************************************************************//
//**********************************  MED Globals  *********************************//
//**********************************************************************************//

// Daylight Change code
typedef union {
	struct {
		si1     code_type;                      // (DST end / DST Not Observed / DST start) ==  (-1 / 0 / +1)
		si1     day_of_week;                    // (No Entry / [Sunday : Saturday]) ==  (-1 / [0 : 6])
		si1     relative_weekday_of_month;      // (No Entry / [First : Fifth] / Last) ==  (0 / [1 : 5] / 6)
		si1     day_of_month;                   // (No Entry / [1 : 31]) ==  (0 / [1 : 31])
		si1     month;                          // (No Entry / [January : December]) ==  (-1 / [0 : 11])
		si1     hours_of_day;                   // [-128 : +127] hours relative to 0:00 (midnight)
		si1     reference_time;                 // (Local / UTC) ==  (0 / +1)
		si1     shift_minutes;                  // [-120 : +120] minutes
	};
	si8     value;                                  // 0 indicates DST is not observed, -1 indicates no entry
} DAYLIGHT_TIME_CHANGE_CODE_m10;

typedef struct {
	si1        country[METADATA_RECORDING_LOCATION_BYTES_m10];
	si1        country_acronym_2_letter[3]; // two-letter acronym; (ISO 3166 ALPHA-2)
	si1        country_acronym_3_letter[4]; // three-letter acronym (ISO-3166 ALPHA-3)
	si1        territory[METADATA_RECORDING_LOCATION_BYTES_m10];
	si1        territory_acronym[TIMEZONE_STRING_BYTES_m10];
	si1        standard_timezone[TIMEZONE_STRING_BYTES_m10];
	si1        standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m10];
	si4        standard_UTC_offset; // seconds
	si1        daylight_timezone[TIMEZONE_STRING_BYTES_m10];
	si1        daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m10];
	si8        daylight_time_start_code;  // DAYLIGHT_TIME_CHANGE_CODE_m10 - cast to use other fields
	si8        daylight_time_end_code;  // DAYLIGHT_TIME_CHANGE_CODE_m10 - cast to use other fields
} TIMEZONE_INFO_m10;

typedef struct {
	si1	table_name[METADATA_RECORDING_LOCATION_BYTES_m10];
	si1	alias[METADATA_RECORDING_LOCATION_BYTES_m10];
} TIMEZONE_ALIAS_m10;

typedef struct {
	TERN_m10	conditioned;
	si8     	start_time;
	si8     	end_time;
	si8     	start_sample_number;  // session-relative (global indexing)
	si8     	end_sample_number;  // session-relative (global indexing)
	si8		local_start_sample_number;  // segment-relative (local indexing)
	si8		local_end_sample_number;  // segment-relative (local indexing)
	si8		number_of_samples;
	si4     	start_segment_number;
	si4     	end_segment_number;
	si8     	session_start_time;
	si8     	session_end_time;
	si1		sample_number_reference_channel_name[BASE_FILE_NAME_BYTES_m10];  // string containing channel base name (or NULL, if unnecessary)
	si4		sample_number_reference_channel_index;  // index of the "sample_number reference channel" in the session channel array
} TIME_SLICE_m10;

typedef struct {  // times in uutc
	si8	creation;
	si8	access;
	si8	modification;
} FILE_TIMES_m10;

typedef struct {  // fields from ipinfo.io
	TIMEZONE_INFO_m10	timezone_info;
	si1			WAN_IPv4_address[IPV4_ADDRESS_BYTES_m10 * 4];
	si1			locality[LOCALITY_BYTES_m10];
	si1			postal_code[POSTAL_CODE_BYTES_m10];
	si1			timezone_description[METADATA_RECORDING_LOCATION_BYTES_m10];
	sf8			latitude;
	sf8			longitude;
} LOCATION_INFO_m10;

typedef struct {
	// Password
	PASSWORD_DATA_m10               password_data;
	// Current Session
	si8				session_UID;
	si1				session_directory[FULL_FILE_NAME_BYTES_m10];  // path including file system session directory name
	si1				*session_name;  // points to: uh_session_name if known, else fs_session_name if known, else NULL
	si1				uh_session_name[BASE_FILE_NAME_BYTES_m10];    // from MED universal header - original name
	si1				fs_session_name[BASE_FILE_NAME_BYTES_m10];  // from file system - may be renamed
	// Time Constants
	TERN_m10			time_constants_set;
	TERN_m10			RTO_known;
	si8				session_start_time;
	si8                             recording_time_offset;
	si4                             standard_UTC_offset;
	si1                             standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m10];
	si1                             standard_timezone_string[TIMEZONE_STRING_BYTES_m10];
	TERN_m10                        observe_DST;
	si1                             daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m10];
	si1                             daylight_timezone_string[TIMEZONE_STRING_BYTES_m10];
	DAYLIGHT_TIME_CHANGE_CODE_m10   daylight_time_start_code;  // si1[8] / si8
	DAYLIGHT_TIME_CHANGE_CODE_m10   daylight_time_end_code;  // si1[8] / si8
	TIMEZONE_INFO_m10		*timezone_table;
	TIMEZONE_ALIAS_m10		*country_aliases_table;
	TIMEZONE_ALIAS_m10		*country_acronym_aliases_table;
	ui4                             recording_time_offset_mode;
	// Alignment Fields
	TERN_m10                        universal_header_aligned;
	TERN_m10                        metadata_section_1_aligned;
	TERN_m10                        time_series_metadata_section_2_aligned;
	TERN_m10                        video_metadata_section_2_aligned;
	TERN_m10                        metadata_section_3_aligned;
	TERN_m10                        all_metadata_structures_aligned;
	TERN_m10                        time_series_indices_aligned;
	TERN_m10                        video_indices_aligned;
	TERN_m10                        CMP_block_header_aligned;
	TERN_m10			CMP_record_header_aligned;
	TERN_m10                        record_header_aligned;
	TERN_m10                        record_indices_aligned;
	TERN_m10                        all_record_structures_aligned;
	TERN_m10                        all_structures_aligned;
	// CMP
	sf8				*CMP_normal_CDF_table;
	// CRC
	ui4				**CRC_table;
	ui4                             CRC_mode;
	// AES tables
	si4				*AES_sbox_table;
	si4				*AES_rcon_table;
	si4				*AES_rsbox_table;
	// SHA256 tables
	ui4				*SHA_h0_table;
	ui4				*SHA_k_table;
	// UTF8 tables
	ui4				*UTF8_offsets_table;
	si1				*UTF8_trailing_bytes_table;
	// Miscellaneous
	TERN_m10                        verbose;
	ui4                             behavior_on_fail;
	si1				temp_dir[FULL_FILE_NAME_BYTES_m10];  // system temp directory (periodically auto-cleared)
	si1				temp_file[FULL_FILE_NAME_BYTES_m10];  // full path to temp file (i.e. incudes temp_dir)
} GLOBALS_m10;


//**********************************************************************************//
//********************************  MED Structures  ********************************//
//**********************************************************************************//


// Universal Header Structure
typedef struct {
	// start robust mode region
	ui4	header_CRC;     // CRC of the universal header after this field
	ui4     body_CRC;       // CRC of the entire file after the universal header
	si8	file_end_time;
	si8	number_of_entries;
	ui4	maximum_entry_size;
	// end robust mode region
	si4     segment_number;
	union {
		struct {
			si1     type_string[TYPE_BYTES_m10];
			ui1     MED_version_major;
			ui1     MED_version_minor;
			ui1     byte_order_code;
		};
		struct {
			ui4     type_code;
			si1	type_string_terminal_zero;  // not used - there for clarity
		};
	};
	si8	session_start_time;
	si8	file_start_time;
	si1	session_name[BASE_FILE_NAME_BYTES_m10]; // utf8[63], base name only, no extension
	si1     channel_name[BASE_FILE_NAME_BYTES_m10]; // utf8[63], base name only, no extension
	si1	anonymized_subject_ID[UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_BYTES_m10]; // utf8[63]
	ui8	session_UID;
	ui8     channel_UID;
	ui8     segment_UID;
	ui8	file_UID;
	ui8	provenance_UID;
	ui1	level_1_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m10];
	ui1     level_2_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m10];
	ui1	level_3_password_validation_field[PASSWORD_VALIDATION_FIELD_BYTES_m10];
	ui1	protected_region[UNIVERSAL_HEADER_PROTECTED_REGION_BYTES_m10];
	ui1	discretionary_region[UNIVERSAL_HEADER_DISCRETIONARY_REGION_BYTES_m10];
} UNIVERSAL_HEADER_m10;

// Metadata Structures
typedef struct {
	si1     level_1_password_hint[PASSWORD_HINT_BYTES_m10];
	si1     level_2_password_hint[PASSWORD_HINT_BYTES_m10];
	si1     section_2_encryption_level;
	si1     section_3_encryption_level;
	ui1     protected_region[METADATA_SECTION_1_PROTECTED_REGION_BYTES_m10];
	ui1     discretionary_region[METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m10];
} METADATA_SECTION_1_m10;

typedef struct {
	// channel type independent fields
	si1     session_description[METADATA_SESSION_DESCRIPTION_BYTES_m10];            // utf8[511]
	si1     channel_description[METADATA_CHANNEL_DESCRIPTION_BYTES_m10];            // utf8[255]
	si1     segment_description[METADATA_SEGMENT_DESCRIPTION_BYTES_m10];            // utf8[255]
	si1     equipment_description[METADATA_EQUIPMENT_DESCRIPTION_BYTES_m10];        // utf8[510]
	si4     acquisition_channel_number;
	// channel type specific fields
	si1     reference_description[TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m10];        // utf8[255]
	sf8     sampling_frequency;
	sf8     low_frequency_filter_setting;
	sf8     high_frequency_filter_setting;
	sf8     notch_filter_frequency_setting;
	sf8     AC_line_frequency;
	sf8     amplitude_units_conversion_factor;
	si1     amplitude_units_description[TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m10];  // utf8[31]
	sf8     time_base_units_conversion_factor;
	si1     time_base_units_description[TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m10];  // utf8[31]
	si8     absolute_start_sample_number;
	si8     number_of_samples;
	si8	number_of_blocks;
	si8     maximum_block_bytes;
	ui4     maximum_block_samples;
	ui4     maximum_block_difference_bytes;
	sf8     maximum_block_duration;
	si8     number_of_discontinuities;
	si8     maximum_contiguous_blocks;
	si8     maximum_contiguous_block_bytes;
	si8     maximum_contiguous_samples;
	ui1     protected_region[TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m10];
	ui1     discretionary_region[TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m10];
} TIME_SERIES_METADATA_SECTION_2_m10;

typedef struct {
	// type-independent fields
	si1     session_description[METADATA_SESSION_DESCRIPTION_BYTES_m10];            // utf8[511]
	si1     channel_description[METADATA_CHANNEL_DESCRIPTION_BYTES_m10];            // utf8[511]
	si1     segment_description[METADATA_SEGMENT_DESCRIPTION_BYTES_m10];            // utf8[511]
	si1     equipment_description[METADATA_EQUIPMENT_DESCRIPTION_BYTES_m10];        // utf8[510]
	si4     acquisition_channel_number;
	// type-specific fields
	si8     horizontal_resolution;
	si8     vertical_resolution;
	sf8     frame_rate;
	si8     number_of_clips;
	si8     maximum_clip_bytes;
	si1     video_format[VIDEO_METADATA_VIDEO_FORMAT_BYTES_m10];                // utf8[31]
	si4     number_of_video_files;
	ui1     protected_region[VIDEO_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m10];
	ui1     discretionary_region[VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m10];
} VIDEO_METADATA_SECTION_2_m10;

// All metadata section 2 structures are the same size, so this structure may be convenient, but it is not currently used in the library.
typedef struct {
	union {
		TIME_SERIES_METADATA_SECTION_2_m10	time_series_section_2;
		VIDEO_METADATA_SECTION_2_m10		video_section_2;
	};
} METADATA_SECTION_2_m10;

typedef struct {
	si8     recording_time_offset;
	DAYLIGHT_TIME_CHANGE_CODE_m10   daylight_time_start_code;                       // si1[8] / si8
	DAYLIGHT_TIME_CHANGE_CODE_m10   daylight_time_end_code;                         // si1[8] / si8
	si1     standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m10];                  // ascii[8]
	si1     standard_timezone_string[TIMEZONE_STRING_BYTES_m10];                    // ascii[31]
	si1     daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m10];                  // ascii[8]
	si1     daylight_timezone_string[TIMEZONE_STRING_BYTES_m10];                    // ascii[31]
	si1     subject_name_1[METADATA_SUBJECT_NAME_BYTES_m10];                        // utf8[31]
	si1     subject_name_2[METADATA_SUBJECT_NAME_BYTES_m10];                        // utf8[31]
	si1     subject_name_3[METADATA_SUBJECT_NAME_BYTES_m10];                        // utf8[31]
	si1     subject_ID[METADATA_SUBJECT_ID_BYTES_m10];                              // utf8[31]
	si1     recording_country[METADATA_RECORDING_LOCATION_BYTES_m10];               // utf8[63]
	si1     recording_territory[METADATA_RECORDING_LOCATION_BYTES_m10];             // utf8[63]
	si1     recording_locality[METADATA_RECORDING_LOCATION_BYTES_m10];              // utf8[63]
	si1     recording_institution[METADATA_RECORDING_LOCATION_BYTES_m10];           // utf8[63]
	si1     geotag_format[METADATA_GEOTAG_FORMAT_BYTES_m10];                        // ascii[31]
	si1     geotag_data[METADATA_GEOTAG_DATA_BYTES_m10];                            // ascii[1023]
	si4     standard_UTC_offset;
	ui1     protected_region[METADATA_SECTION_3_PROTECTED_REGION_BYTES_m10];
	ui1     discretionary_region[METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m10];
} METADATA_SECTION_3_m10;

typedef struct {
	METADATA_SECTION_1_m10				section_1;
	union {  // == 	METADATA_SECTION_2_m10
		TIME_SERIES_METADATA_SECTION_2_m10	time_series_section_2;
		VIDEO_METADATA_SECTION_2_m10		video_section_2;
	};
	METADATA_SECTION_3_m10				section_3;
} METADATA_m10;

// Record Structures
typedef struct {
	ui4	record_CRC;
	ui4     total_record_bytes;  // header + body bytes
	si8     start_time;
	union {  // anonymous union
		struct {
			si1     type_string[TYPE_BYTES_m10];
			ui1     version_major;
			ui1     version_minor;
			si1     encryption_level;
		};
		struct {
			ui4     type_code;
			si1	type_string_terminal_zero;  // not used - there for clarity
		};
	};
} RECORD_HEADER_m10;

typedef struct {
	si8	file_offset;
	si8	start_time;
	union {  // anonymous union
		struct {
			si1     type_string[TYPE_BYTES_m10];
			ui1     version_major;
			ui1     version_minor;
			si1     encryption_level;
		};
		struct {
			ui4     type_code;
			si1	type_string_terminal_zero;  // not used - there for clarity
		};
	};
} RECORD_INDEX_m10;

// Time Series Indices Structures
typedef struct {
	si8	file_offset;  // negative values indicate discontinuity
	si8	start_time;
	si8     start_sample_number;
} TIME_SERIES_INDEX_m10;

// Video Indices Structures
typedef struct {
	si8     file_offset;  // negative values indicate discontinuity
	si8	start_time;
	ui4     start_frame_number;
	ui4     video_file_number;
} VIDEO_INDEX_m10;

// All index structures are the same size, so this structure may be convenient, but it is not currently used in the library.
typedef struct {
	union {
		RECORD_INDEX_m10	record_index;
		TIME_SERIES_INDEX_m10	time_series_index;
		VIDEO_INDEX_m10		video_index;
	};
} INDEX_m10;

// File Processing Structures
typedef struct {
	TERN_m10        close_file;
	TERN_m10        flush_after_write;
	TERN_m10        update_universal_header;	// when writing
	TERN_m10        leave_decrypted;		// if encrypted during write, return from write function decrypted
	TERN_m10        free_password_data;		// when freeing FPS
	TERN_m10        free_CMP_processing_struct;	// when freeing FPS
	ui4             lock_mode;
	ui4             open_mode;
} FILE_PROCESSING_DIRECTIVES_m10;

typedef struct {
#if defined MACOS_m10 || defined LINUX_m10
	TERN_m10 _Atomic                        mutex;
#else
	TERN_m10                        	mutex;
#endif
	PASSWORD_DATA_m10			*password_data;
	si1                                     full_file_name[FULL_FILE_NAME_BYTES_m10];  // full path including extension
	FILE					*fp;  // file pointer
	si4                                     fd;  // file descriptor
	si8                                     file_length;
	UNIVERSAL_HEADER_m10			*universal_header;
	FILE_PROCESSING_DIRECTIVES_m10	        directives;
	union {					// the MED file types
		METADATA_m10			*metadata;
		ui1				*records;
		RECORD_INDEX_m10		*record_indices;
		TIME_SERIES_INDEX_m10		*time_series_indices;
		VIDEO_INDEX_m10			*video_indices;
	};
	si8                                     raw_data_bytes;
	ui1					*raw_data;
	CMP_PROCESSING_STRUCT_m10		*cps;  // associated with time series data FPS, NULL in others
} FILE_PROCESSING_STRUCT_m10;

// Session, Channel, Segment Processing Structures
typedef struct {
	FILE_PROCESSING_STRUCT_m10	*metadata_fps;  // also used as prototype
	FILE_PROCESSING_STRUCT_m10	*time_series_data_fps;
	FILE_PROCESSING_STRUCT_m10	*time_series_indices_fps;
	FILE_PROCESSING_STRUCT_m10	*video_indices_fps;
	FILE_PROCESSING_STRUCT_m10	*record_data_fps;
	FILE_PROCESSING_STRUCT_m10	*record_indices_fps;
	FILE_PROCESSING_STRUCT_m10	*segmented_session_record_data_fps;
	FILE_PROCESSING_STRUCT_m10	*segmented_session_record_indices_fps;
	si1                             path[FULL_FILE_NAME_BYTES_m10]; // full path to segment directory (including segment directory itself)
	si1                             name[SEGMENT_BASE_FILE_NAME_BYTES_m10];  // stored here, no segment_name field in universal header (for programming convenience)
	TIME_SLICE_m10			time_slice;
} SEGMENT_m10;

typedef struct {
	FILE_PROCESSING_STRUCT_m10	*metadata_fps;
	FILE_PROCESSING_STRUCT_m10	*record_data_fps;
	FILE_PROCESSING_STRUCT_m10	*record_indices_fps;
	si4			        number_of_segments;
	SEGMENT_m10			**segments;
	si1			        path[FULL_FILE_NAME_BYTES_m10]; // full path to channel directory (including channel directory itself)
	si1                             name[BASE_FILE_NAME_BYTES_m10];
	TIME_SLICE_m10			time_slice;
} CHANNEL_m10;

typedef struct {
	FILE_PROCESSING_STRUCT_m10	*time_series_metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	FILE_PROCESSING_STRUCT_m10	*video_metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
	si4                             number_of_segments;
	si4			        number_of_time_series_channels;
	CHANNEL_m10			**time_series_channels;
	si4			        number_of_video_channels;
	CHANNEL_m10			**video_channels;
	FILE_PROCESSING_STRUCT_m10	*record_data_fps;
	FILE_PROCESSING_STRUCT_m10	*record_indices_fps;
	FILE_PROCESSING_STRUCT_m10	**segmented_record_data_fps;
	FILE_PROCESSING_STRUCT_m10	**segmented_record_indices_fps;
	si1			        path[FULL_FILE_NAME_BYTES_m10];     // full path to session directory (including session directory itself)
	si1                             name[BASE_FILE_NAME_BYTES_m10];
	TIME_SLICE_m10			time_slice;
} SESSION_m10;



//**********************************************************************************//
//****************************  GENERAL MED Functions  *****************************//
//**********************************************************************************//


// Prototypes
TERN_m10	adjust_open_file_limit_m10(si4 new_limit, TERN_m10 verbose_flag);
TERN_m10	all_zeros_m10(ui1 *bytes, si4 field_length);
CHANNEL_m10	*allocate_channel_m10(CHANNEL_m10 *chan, FILE_PROCESSING_STRUCT_m10 *proto_fps, si1 *enclosing_path, si1 *chan_name, ui4 type_code, si4 n_segs, TERN_m10 chan_recs, TERN_m10 seg_recs);
FILE_PROCESSING_STRUCT_m10	*allocate_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10* fps, si1* full_file_name, ui4 type_code, si8 raw_data_bytes, FILE_PROCESSING_STRUCT_m10* proto_fps, si8 bytes_to_copy);
SEGMENT_m10	*allocate_segment_m10(SEGMENT_m10 *seg, FILE_PROCESSING_STRUCT_m10 *proto_fps, si1* enclosing_path, si1 *chan_name, ui4 type_code, si4 seg_num, TERN_m10 seg_recs);
SESSION_m10	*allocate_session_m10(FILE_PROCESSING_STRUCT_m10 *proto_fps, si1 *enclosing_path, si1 *sess_name, si4 n_ts_chans, si4 n_vid_chans, si4 n_segs, si1 **chan_names, si1 **vid_chan_names, TERN_m10 sess_recs, TERN_m10 segmented_sess_recs, TERN_m10 chan_recs, TERN_m10 seg_recs);
void     	apply_recording_time_offset_m10(si8 *time);
void            calculate_metadata_CRC_m10(FILE_PROCESSING_STRUCT_m10 *fps);
void            calculate_record_data_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_HEADER_m10 *record_header, si8 number_of_items);
void            calculate_record_indices_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_INDEX_m10 *record_index, si8 number_of_items);
void            calculate_time_series_data_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, CMP_BLOCK_FIXED_HEADER_m10 *block_header, si8 number_of_items);
void            calculate_time_series_indices_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, TIME_SERIES_INDEX_m10 *time_series_index, si8 number_of_items);
void		**calloc_2D_m10(size_t dim1, size_t dim2, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail);
ui4             channel_type_from_path_m10(si1 *path);
wchar_t		*char2wchar_m10(wchar_t *target, si1 *source);
TERN_m10	check_all_alignments_m10(const si1 *function, si4 line);
TERN_m10	check_char_type_m10(void);
TERN_m10	check_file_list_m10(si1 **file_list, si4 n_files);
TERN_m10	check_metadata_alignment_m10(ui1 *bytes);
TERN_m10	check_metadata_section_1_alignment_m10(ui1 *bytes);
TERN_m10	check_metadata_section_3_alignment_m10(ui1 *bytes);
TERN_m10        check_password_m10(si1 *password);
TERN_m10	check_record_header_alignment_m10(ui1 *bytes);
TERN_m10	check_record_indices_alignment_m10(ui1 *bytes);
TERN_m10	check_CMP_block_header_alignment_m10(ui1 *bytes);
TERN_m10        check_CMP_record_header_alignment_m10(ui1 *bytes);
TERN_m10	check_time_series_indices_alignment_m10(ui1 *bytes);
TERN_m10	check_time_series_metadata_section_2_alignment_m10(ui1 *bytes);
TERN_m10	check_universal_header_alignment_m10(ui1 *bytes);
TERN_m10	check_video_indices_alignment_m10(ui1 *bytes);
TERN_m10	check_video_metadata_section_2_alignment_m10(ui1 *bytes);
void		condition_timezone_info_m10(TIMEZONE_INFO_m10 *tz_info);
void		condition_time_slice_m10(TIME_SLICE_m10 *slice);
si8		current_uutc_m10(void);
si4		days_in_month_m10(si4 month, si4 year);
TERN_m10        decrypt_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps);
TERN_m10        decrypt_records_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_HEADER_m10* record_header, si8 number_of_items);
TERN_m10        decrypt_time_series_data_m10(CMP_PROCESSING_STRUCT_m10 *cps, si8 number_of_items);
si4             DST_offset_m10(si8 uutc);
TERN_m10        encrypt_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps);
TERN_m10	encrypt_records_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_HEADER_m10 *record_header, si8 number_of_items);
TERN_m10        encrypt_time_series_data_m10(CMP_PROCESSING_STRUCT_m10 *cps, si8 number_of_items);
void            error_message_m10(si1 *fmt, ...);
void            escape_spaces_m10(si1 *string, si8 buffer_len);
void            extract_path_parts_m10(si1 *full_file_name, si1 *path, si1 *name, si1 *extension);
void            extract_terminal_password_bytes_m10(si1 *password, si1 *password_bytes);
si8		*find_discontinuities_m10(TIME_SERIES_INDEX_m10 *tsi, si8 *num_disconts, si8 number_of_indices, TERN_m10 remove_offsets, TERN_m10 return_sample_numbers);
ui4             file_exists_m10(si1 *path);
si8		file_length_m10(FILE *fp, si1 *path);
FILE_TIMES_m10	*file_times_d10(si1 *path, FILE_TIMES_m10 *ft, TERN_m10 set_time);
si1		*find_timezone_acronym_m10(si1 *timezone_acronym, si4 standard_UTC_offset, si4 DST_offset);
si1		*find_metadata_file_m10(si1 *path, si1 *md_path);
void            force_behavior_m10(ui4 behavior);
void            free_2D_m10(void **ptr, size_t dim1, const si1 *function, si4 line);
void            free_channel_m10(CHANNEL_m10* channel, TERN_m10 allocated_en_bloc);
void            free_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps, TERN_m10 allocated_en_bloc);
void            free_globals_m10(void);
void            free_segment_m10(SEGMENT_m10 *segment, TERN_m10 allocated_en_bloc);
void            free_session_m10(SESSION_m10 *session);
si1		**generate_file_list_m10(si1 **file_list, si4 *n_files, si1 *enclosing_directory, si1 *name, si1 *extension, ui1 path_parts, TERN_m10 free_input_file_list);
si1		*generate_hex_string_m10(ui1 *bytes, si4 num_bytes, si1 *string);
ui4             generate_MED_path_components_m10(si1 *path, si1 *MED_dir, si1* MED_name);
si1		**generate_numbered_names_m10(si1 **names, si1 *prefix, si4 number_of_names);
si8             generate_recording_time_offset_m10(si8 recording_start_time_uutc);
si1		*generate_segment_name_m10(FILE_PROCESSING_STRUCT_m10 *fps, si1 *segment_name);
ui8             generate_UID_m10(ui8 *uid);
TERN_m10	get_channel_target_values_m10(CHANNEL_m10 *channel, si8 *target_uutc, si8 *target_sample_number, si4 *target_segment_number, ui1 mode);
ui1		get_cpu_endianness_m10(void);
LOCATION_INFO_m10	*get_location_info_m10(LOCATION_INFO_m10 *loc_info, TERN_m10 set_timezone_globals, TERN_m10 prompt);
si4             get_segment_range_m10(si1 **channel_list, si4 n_channels, TIME_SLICE_m10 *slice);
void		get_segment_target_values_m10(SEGMENT_m10 *segment, si8 *target_uutc, si8 *target_sample_number, ui1 mode);
si1		*get_session_directory_m10(si1 *session_directory, si1 *MED_file_name, FILE_PROCESSING_STRUCT_m10 *MED_fps);
TERN_m10	get_session_target_values_m10(SESSION_m10 *session, si8 *target_uutc, si8 *target_sample_number, si4 *target_segment_number, ui1 mode, si1 *idx_ref_chan);
FILE_PROCESSING_DIRECTIVES_m10	*initialize_file_processing_directives_m10(FILE_PROCESSING_DIRECTIVES_m10 *directives);
TERN_m10	initialize_globals_m10(void);
TERN_m10	initialize_medlib_m10(TERN_m10 check_structure_alignments, TERN_m10 initialize_all_tables);
void            initialize_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps, TERN_m10 initialize_for_update);
TIME_SLICE_m10	*initialize_time_slice_m10(TIME_SLICE_m10 *slice);
TERN_m10	initialize_timezone_tables_m10(void);
void		initialize_universal_header_m10(FILE_PROCESSING_STRUCT_m10 *fps, ui4 type_code, TERN_m10 generate_file_UID, TERN_m10 originating_file);
si1		*MED_type_string_from_code_m10(ui4 code);
ui4             MED_type_code_from_string_m10(si1 *string);
TERN_m10        merge_metadata_m10(FILE_PROCESSING_STRUCT_m10 *md_fps_1, FILE_PROCESSING_STRUCT_m10 *md_fps_2, FILE_PROCESSING_STRUCT_m10 *merged_md_fps);
TERN_m10        merge_universal_headers_m10(FILE_PROCESSING_STRUCT_m10 *fps_1, FILE_PROCESSING_STRUCT_m10 *fps_2, FILE_PROCESSING_STRUCT_m10 *merged_fps);
void    	message_m10(si1 *fmt, ...);
si1		*numerical_fixed_width_string_m10(si1 *string, si4 string_bytes, si4 number);
si8             pad_m10(ui1 *buffer, si8 content_len, ui4 alignment);
TERN_m10	path_from_root_m10(si1 *path, si1 *root_path);
TERN_m10	process_password_data_m10(si1 *unspecified_password, si1 *L1_password, si1 *L2_password, si1 *L3_password, si1 *L1_hint, si1 *L2_hint, FILE_PROCESSING_STRUCT_m10 *fps);
CHANNEL_m10	*read_channel_m10(CHANNEL_m10 *chan, si1 *chan_dir, TIME_SLICE_m10 *slice, si1 *password, TERN_m10 read_time_series_data, TERN_m10 read_record_data);
FILE_PROCESSING_STRUCT_m10	*read_file_m10(FILE_PROCESSING_STRUCT_m10 *fps, si1 *full_file_name, si8 number_of_items, ui1 **data_ptr_ptr, si8 *items_read, si1 *password, ui4 behavior_on_fail);
SEGMENT_m10	*read_segment_m10(SEGMENT_m10 *seg, si1 *seg_dir, TIME_SLICE_m10 *slice, si1 *password, TERN_m10 read_time_series_data, TERN_m10 read_record_data);
SESSION_m10	*read_session_m10(si1 *sess_dir, si1 **chan_list, si4 n_chans, TIME_SLICE_m10 *slice, si1 *password, TERN_m10 read_time_series_data, TERN_m10 read_record_data);
si8             read_time_series_data_m10(SEGMENT_m10 *seg, si8 local_start_idx, si8 local_end_idx, TERN_m10 alloc_cps);
void		**realloc_2D_m10(void **curr_ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail);
si4             reallocate_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps, si8 raw_data_bytes);
TERN_m10	recover_passwords_m10(si1 *L3_password, UNIVERSAL_HEADER_m10* universal_header);
void     	remove_recording_time_offset_m10(si8 *time);
void            reset_metadata_for_update_m10(FILE_PROCESSING_STRUCT_m10 *fps);
si8             sample_number_for_uutc_m10(si8 ref_sample_number, si8 ref_uutc, si8 target_uutc, sf8 sampling_frequency, FILE_PROCESSING_STRUCT_m10 *time_series_indices_fps, ui1 mode);
TERN_m10        search_segment_metadata_m10(si1 *MED_dir, TIME_SLICE_m10 *slice);
TERN_m10        search_Sgmt_records_m10(si1 *MED_dir, TIME_SLICE_m10* slice);
si8     	segment_sample_number_to_time_m10(si1 *seg_dir, si8 local_sample_number, si8 absolute_start_sample_number, sf8 sampling_frequency, ui1 mode);
TERN_m10	set_global_time_constants_m10(TIMEZONE_INFO_m10 *timezone_info, si8 session_start_time, TERN_m10 prompt);
TERN_m10	set_time_and_password_data_m10(si1 *unspecified_password, si1 *MED_directory, si1 *metadata_section_2_encryption_level, si1 *metadata_section_3_encryption_level);
void            show_daylight_change_code_m10(DAYLIGHT_TIME_CHANGE_CODE_m10 *code, si1 *prefix);
void            show_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps);
void		show_file_times_m10(FILE_TIMES_m10 *ft);
void            show_globals_m10(void);
void    	show_location_info_m10(LOCATION_INFO_m10 *li);
void            show_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps, METADATA_m10 *md);
void            show_password_data_m10(PASSWORD_DATA_m10 *pwd);
void		show_password_hints_m10(PASSWORD_DATA_m10 *pwd);
void            show_records_m10(FILE_PROCESSING_STRUCT_m10 *fps, ui4 type_code);
void    	show_time_slice_m10(TIME_SLICE_m10 *slice);
void            show_timezone_info_m10(TIMEZONE_INFO_m10 *timezone_entry);
void            show_universal_header_m10(FILE_PROCESSING_STRUCT_m10 *fps, UNIVERSAL_HEADER_m10 *uh);
si4		str_compare_m10(const void *a, const void *b);
TERN_m10	str_contains_regex_m10(si1 *string);
si1		*str_match_end_m10(si1 *pattern, si1 *buffer);
si1		*str_match_line_end_m10(si1 *pattern, si1 *buffer);
si1		*str_match_line_start_m10(si1 *pattern, si1 *buffer);
si1		*str_match_start_m10(si1 *pattern, si1 *buffer);
void    	str_replace_char_m10(si1 c, si1 new_c, si1 *buffer);
si1		*str_replace_pattern_m10(si1 *pattern, si1 *new_pattern, si1 *buffer, TERN_m10 free_input_buffer);
void		str_sort_m10(si1 **string_array, si8 n_strings);
void		strip_character_m10(si1 *s, si1 character);
void		strtolower_m10(si1 *s);
void		strtotitle_m10(si1 *s);
void		strtoupper_m10(si1 *s);
si1		*time_string_m10(si8 uutc_time, si1 *time_str, TERN_m10 fixed_width, TERN_m10 relative_days, si4 colored_text, ...);
si8             ts_sort_m10(si4 *x, si8 len, NODE_m10 *nodes, NODE_m10 *head, NODE_m10 *tail, si4 return_sorted_ts, ...);
void            unescape_spaces_m10(si1 *string);
#ifdef WINDOWS_m10
FILETIME	uutc_to_win_time_m10(si8 uutc);
#endif
si8             uutc_for_sample_number_m10(si8 ref_sample_number, si8 ref_uutc, si8 target_sample_number, sf8 sampling_frequency, FILE_PROCESSING_STRUCT_m10 *time_series_indices_fps, ui1 mode);
TERN_m10        validate_record_data_CRCs_m10(RECORD_HEADER_m10* record_header, si8 number_of_items);
TERN_m10        validate_time_series_data_CRCs_m10(CMP_BLOCK_FIXED_HEADER_m10* block_header, si8 number_of_items);
void            warning_message_m10(si1 *fmt, ...);
si1*		wchar2char_m10(si1 *target, wchar_t *source);
void		win_cleanup_m10(void);
si4		win_ls_1d_to_tmp_m10(si1 **dir_strs, si4 n_dirs, TERN_m10 full_path);
TERN_m10	win_initialize_terminal_m10(void);
TERN_m10	win_reset_terminal_m10(void);
TERN_m10	win_socket_startup_m10(void);
inline si4	win_system_m10(si1 *command);
#ifdef WINDOWS_m10
si8		win_time_to_uutc_m10(FILETIME win_time);
#endif
void		windify_file_paths_m10(si1 *target, si1 *source);
si1		*windify_format_string_m10(si1 *fmt);
si8             write_file_m10(FILE_PROCESSING_STRUCT_m10 *fps, ui8 number_of_items, void *data_ptr, ui4 behavior_on_fail);



//**********************************************************************************//
//*******************************  MED FPS Functions  ******************************//
//**********************************************************************************//

// Prototypes
void            FPS_close_m10(FILE_PROCESSING_STRUCT_m10 *fps);
si4		FPS_compare_start_times_m10(const void *a, const void *b);
si4             FPS_lock_m10(FILE_PROCESSING_STRUCT_m10 *fps, si4 lock_type, const si1 *function, si4 line, ui4 behavior_on_fail);
void		FPS_mutex_off_m10(FILE_PROCESSING_STRUCT_m10 *fps);
void		FPS_mutex_on_m10(FILE_PROCESSING_STRUCT_m10 *fps);
si4             FPS_open_m10(FILE_PROCESSING_STRUCT_m10 *fps, const si1 *function, si4 line, ui4 behavior_on_fail);
si4             FPS_read_m10(FILE_PROCESSING_STRUCT_m10 *fps, si8 in_bytes, void *ptr, const si1 *function, si4 line, ui4 behavior_on_fail);
void		FPS_sort_m10(FILE_PROCESSING_STRUCT_m10 **fps_array, si4 n_fps);
si4             FPS_unlock_m10(FILE_PROCESSING_STRUCT_m10 *fps, const si1 *function, si4 line, ui4 behavior_on_fail);
si4             FPS_write_m10(FILE_PROCESSING_STRUCT_m10 *fps, si8 out_bytes, void *ptr, const si1 *function, si4 line, ui4 behavior_on_fail);



//***********************************************************************//
//*****************  MED VERSIONS OF STANDARD FUNCTIONS  ****************//
//***********************************************************************//


si4		asprintf_m10(si1 **target, si1 *fmt, ...);
void		*calloc_m10(size_t n_members, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail);
void		exit_m10(si4 status);
FILE		*fopen_m10(si1 *path, si1 *mode, const si1 *function, si4 line, ui4 behavior_on_fail);
si4     	fprintf_m10(FILE *stream, si1 *fmt, ...);
si4		fputc_m10(si4 c, FILE *stream);
size_t          fread_m10(void *ptr, size_t el_size, size_t n_members, FILE *stream, si1 *path, const si1 *function, si4 line, ui4 behavior_on_fail);
void            free_m10(void *ptr, const si1 *function, si4 line);
si4     	fscanf_m10(FILE *stream, si1 *fmt, ...);
si4             fseek_m10(FILE *stream, si8 offset, si4 whence, si1 *path, const si1 *function, si4 line, ui4 behavior_on_fail);
si8            	ftell_m10(FILE *stream, const si1 *function, si4 line, ui4 behavior_on_fail);
size_t		fwrite_m10(void *ptr, size_t el_size, size_t n_members, FILE *stream, si1 *path, const si1 *function, si4 line, ui4 behavior_on_fail);
char		*getcwd_m10(char *buf, size_t size);
void		*malloc_m10(size_t n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail);
si4     	printf_m10(si1 *fmt, ...);
si4		putc_m10(si4 c, FILE *stream);
si4		putch_m10(si4 c);  // Windows "_putch()"
si4		putchar_m10(si4 c);
void		*realloc_m10(void *ptr, size_t n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail);
si4     	scanf_m10(si1 *fmt, ...);
si4     	sprintf_m10(si1 *target, si1 *fmt, ...);
si4		snprintf_m10(si1 *target, si4 target_field_bytes, si1 *fmt, ...);
si4     	sscanf_m10(si1 *target, si1 *fmt, ...);
si8		strcat_m10(si1 *target, si1 *source);
si8		strcpy_m10(si1 *target, si1 *source);
si8		strncat_m10(si1 *target, si1 *source, si4 target_field_bytes);
si8		strncpy_m10(si1 *target, si1 *source, si4 target_field_bytes);
si4             system_m10(si1 *command, TERN_m10 null_std_streams, const si1 *function, si4 line, ui4 behavior_on_fail);
si4		vasprintf_m10(si1 **target, si1 *fmt, va_list args);
si4		vfprintf_m10(FILE *stream, si1 *fmt, va_list args);
si4		vprintf_m10(si1 *fmt, va_list args);
si4		vsnprintf_m10(si1 *target, si4 target_field_bytes, si1 *fmt, va_list args);
si4    		vsprintf_m10(si1 *target, si1 *fmt, va_list args);



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
#define CRC_BYTES_m10           4
#define CRC_TABLES_m10          8
#define CRC_TABLE_ENTRIES_m10   256
#define CRC_POLYNOMIAL_m10      ((ui4) 0xEDB88320)    // note library CRC routines are customized to this polynomial, it cannot be changed arbitrarily
#define CRC_START_VALUE_m10     ((ui4) 0x0)

// CRC Modes
#define CRC_NO_ENTRY_m10                CRC_START_VALUE_m10
#define CRC_IGNORE_m10                  0
#define CRC_VALIDATE_m10                1
#define CRC_VALIDATE_ON_INPUT_m10       2
#define CRC_VALIDATE_ON_OUTPUT_m10      4
#define CRC_CALCULATE_m10               8
#define CRC_CALCULATE_ON_INPUT_m10      16
#define CRC_CALCULATE_ON_OUTPUT_m10     32

// Macros
#define CRC_SWAP32_m10(q)       ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

// Function Prototypes
ui4		CRC_calculate_m10(const ui1 *block_ptr, si8 block_bytes);
ui4		CRC_combine_m10(ui4 block_1_crc, ui4 block_2_crc, si8 block_2_bytes);
TERN_m10	CRC_initialize_tables_m10(void);
void		CRC_matrix_square_m10(ui4 *square, const ui4 *mat);
ui4		CRC_matrix_times_m10(const ui4 *mat, ui4 vec);
ui4		CRC_update_m10(const ui1 *block_ptr, si8 block_bytes, ui4 current_crc);
TERN_m10	CRC_validate_m10(const ui1 *block_ptr, si8 block_bytes, ui4 crc_to_validate);



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
#define UTF8_ISUTF_m10(c)       (((c) & 0xC0) != 0x80) // true if c is the start of a UTF-8 sequence

#define UTF8_OFFSETS_TABLE_ENTRIES_m10	6
#define UTF8_OFFSETS_TABLE_m10        { 0x0UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL }

#define UTF8_TRAILING_BYTES_TABLE_ENTRIES_m10	256
#define UTF8_TRAILING_BYTES_TABLE_m10	      {	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
							1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, \
							2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5 }

// Function Prototypes
si4	UTF8_char_num_m10(si1 *s, si4 offset);  // byte offset to character number
void	UTF8_dec_m10(si1 *s, si4 *i);  // move to previous character
si4	UTF8_escape_m10(si1 *buf, si4 sz, si1 *src, si4 escape_quotes);  // convert UTF-8 "src" to ASCII with escape sequences.
si4	UTF8_escape_wchar_m10(si1 *buf, si4 sz, ui4 ch);  // given a wide character, convert it to an ASCII escape sequence stored in buf, where buf is "sz" bytes. returns the number of characters output
si4	UTF8_fprintf_m10(FILE *stream, si1 *fmt, ...);  // fprintf() where the format string and arguments may be in UTF-8. You can avoid this function and just use ordinary fprintf() if the current locale is UTF-8.
si4	UTF8_hex_digit_m10(si1 c);  // utility predicates used by the above
void	UTF8_inc_m10(si1 *s, si4 *i);  // move to next character
TERN_m10	UTF8_initialize_tables_m10(void);
si4	UTF8_is_locale_utf8_m10(si1 *locale);  // boolean function returns if locale is UTF-8, 0 otherwise
si1	*UTF8_memchr_m10(si1 *s, ui4 ch, size_t sz, si4 *char_num);  // same as the above, but searches a buffer of a given size instead of a NUL-terminated string.
ui4	UTF8_next_char_m10(si1 *s, si4* i);  // return next character, updating an index variable
si4	UTF8_octal_digit_m10(si1 c);  // utility predicates used by the above
si4	UTF8_offset_m10(si1 *str, si4 char_num);  // character number to byte offset
si4     UTF8_printf_m10(si1 *fmt, ...);  // printf() where the format string and arguments may be in UTF-8. You can avoid this function and just use ordinary printf() if the current locale is UTF-8.
si4	UTF8_read_escape_sequence_m10(si1 *str, ui4 *dest);  // assuming str points to the character after a backslash, read an escape sequence, storing the result in dest and returning the number of input characters processed
si4	UTF8_seqlen_m10(si1 *s);  // returns length of next UTF-8 sequence
si1	*UTF8_strchr_m10(si1 *s, ui4 ch, si4 *char_num);  // return a pointer to the first occurrence of ch in s, or NULL if not found. character index of found character returned in *char_num.
si4	UTF8_strlen_m10(si1 *s);  // count the number of characters in a UTF-8 string
si4	UTF8_to_ucs_m10(ui4 *dest, si4 sz, si1 *src, si4 srcsz);  // convert UTF-8 data to wide character
si4	UTF8_to_utf8_m10(si1 *dest, si4 sz, ui4 *src, si4 srcsz);  // convert wide character to UTF-8 data
si4	UTF8_unescape_m10(si1 *buf, si4 sz, si1 *src);  // convert a string "src" containing escape sequences to UTF-8 if escape_quotes is nonzero, quote characters will be preceded by  backslashes as well.
si4	UTF8_vfprintf_m10(FILE *stream, si1 *fmt, va_list ap);    // called by UTF8_fprintf()
si4	UTF8_vprintf_m10(si1 *fmt, va_list ap);  // called by UTF8_printf()
si4	UTF8_wc_to_utf8_m10(si1 *dest, ui4 ch);  // single character to UTF-8



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
// THE CODE IN THIS FILE IS SET FOR AES 128-BIT ENCRYPTION / DECRYPTION ONLY
//
// Minor modifications for compatibility with the MED Library.

#define AES_NR_m10	        10	// The number of rounds in AES Cipher
#define AES_NK_m10	        4	// The number of 32 bit words in the key
#define AES_NB_m10	        4	// The number of columns comprising a state in AES. This is a constant in AES.
#define AES_XTIME_m10(x)        ((x<<1) ^ (((x>>7) & 1) * 0x1b)) // AES_XTIME is a macro that finds the product of {02} and the argument to AES_XTIME modulo {1b}
#define AES_MULTIPLY_m10(x, y)  (((y & 1) * x) ^ ((y>>1 & 1) * AES_XTIME_m10(x)) ^ ((y>>2 & 1) * AES_XTIME_m10(AES_XTIME_m10(x))) ^ ((y>>3 & 1) * AES_XTIME_m10(AES_XTIME_m10(AES_XTIME_m10(x)))) ^ ((y>>4 & 1) * AES_XTIME_m10(AES_XTIME_m10(AES_XTIME_m10(AES_XTIME_m10(x)))))) // Multiplty is a macro used to multiply numbers in the field GF(2^8)

#define AES_SBOX_ENTRIES_m10	256
#define AES_SBOX_m10          {	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, \
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

#define AES_RSBOX_ENTRIES_m10	256
#define AES_RSBOX_m10         {	0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, \
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

#define AES_RCON_ENTRIES_m10	255
#define AES_RCON_m10          {	0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, \
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
void		AES_add_round_key_m10(si4 round, ui1 state[][4], ui1 *round_key);
void		AES_decrypt_m10(ui1 *in, ui1 *out, si1 *password, ui1 *expanded_key);
void		AES_encrypt_m10(ui1 *in, ui1 *out, si1 *password, ui1 *expanded_key);
void		AES_key_expansion_m10(ui1 *round_key, si1 *key);
void		AES_cipher_m10(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key);
si4		AES_get_sbox_invert_m10(si4 num);
si4		AES_get_sbox_value_m10(si4 num);
TERN_m10	AES_initialize_tables_m10(void);
void		AES_inv_cipher_m10(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key);
void		AES_inv_mix_columns_m10(ui1 state[][4]);
void		AES_inv_shift_rows_m10(ui1 state[][4]);
void		AES_inv_sub_bytes_m10(ui1 state[][4]);
void		AES_mix_columns_m10(ui1 state[][4]);
void		AES_shift_rows_m10(ui1 state[][4]);
void		AES_sub_bytes_m10(ui1 state[][4]);



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
// Only SHA-256 functions are included in the MED library
// The version below contains minor modifications for compatibility with the MED Library.


// Constants
#define SHA_HASH_BYTES_m10	32  // 256 bit
#define SHA_LOW_BYTE_MASK_m10	(ui4) 0x000000FF

#define	SHA_H0_ENTRIES_m10	8
#define	SHA_H0_m10            {	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 }

#define	SHA_K_ENTRIES_m10	64
#define	SHA_K_m10	      {	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, \
       				0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, \
       				0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, \
       				0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, \
       				0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, \
       				0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, \
       				0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, \
				0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 }

// Macros
#define SHA_ROTLEFT_m10(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define SHA_ROTRIGHT_m10(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define SHA_CH_m10(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define SHA_MAJ_m10(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA_EP0_m10(x) (SHA_ROTRIGHT_m10(x,2) ^ SHA_ROTRIGHT_m10(x,13) ^ SHA_ROTRIGHT_m10(x,22))
#define SHA_EP1_m10(x) (SHA_ROTRIGHT_m10(x,6) ^ SHA_ROTRIGHT_m10(x,11) ^ SHA_ROTRIGHT_m10(x,25))
#define SHA_SIG0_m10(x) (SHA_ROTRIGHT_m10(x,7) ^ SHA_ROTRIGHT_m10(x,18) ^ ((x) >> 3))
#define SHA_SIG1_m10(x) (SHA_ROTRIGHT_m10(x,17) ^ SHA_ROTRIGHT_m10(x,19) ^ ((x) >> 10))

// Typedefs & Structures
typedef struct {
	ui1	data[64];
	ui4	state[8];
	ui8	bitlen;
	ui4	datalen;
} SHA_CTX_m10;

// Function Prototypes
void		SHA_finalize_m10(SHA_CTX_m10 *ctx, ui1 *hash);
ui1    		*SHA_hash_m10(const ui1 *data, si8 len, ui1 *hash);
void		SHA_initialize_m10(SHA_CTX_m10 *ctx);
TERN_m10	SHA_initialize_tables_m10(void);
void		SHA_transform_m10(SHA_CTX_m10 *ctx, const ui1 *data);
void		SHA_update_m10(SHA_CTX_m10 *ctx, const ui1 *data, si8 len);


//**********************************************************************************//
//********************************  Time Zone Data  ********************************//
//**********************************************************************************//

// Notes:
//
// Western Sahara:
// DST is on most of the year and off during Ramadan, whose dates change annually in a way that is not accomodated by this schema.
// As Ramadan only lasts a month, and can occur at vitually any time of year, this table treats it as if it's Daylight Time
// is it's Standard Time, and it does not observe DST.
//
// If it were to have a proper entry, it would look something like:
// { "Western Sahara", "EH", "ESH", "", "", "Western European Time", "WET", 0, 1, "Western European Daylight Time", "WEDT", "May 31 at 02:00 Local", 0x3c0002041f00ff01, "Apr 19 at 03:00 Local", 0xc40003031300ffff }
//
// But it is represented here as:
// { "Western Sahara", "EH", "ESH", "", "", "Western European Daylight Time", "WEDT", +3600, 0, "", "", "", 0x0, "", 0x0 }

#define TZ_TABLE_ENTRIES_m10      399
#define TZ_TABLE_m10 { \
	{ "AFGHANISTAN", "AF", "AFG", "", "", "AFGHANISTAN TIME", "AFT", 16200, "", "", 0x0, 0x0 }, \
	{ "AKROTIRI", "", "", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00010200060001, 0xC4000209000600FF }, \
	{ "ALAND ISLANDS", "AX", "ALA", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF }, \
	{ "ALBANIA", "AL", "ALB", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "ALGERIA", "DZ", "DZA", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "", "", 0x0, 0x0 }, \
	{ "AMERICAN SAMOA", "US", "ASM", "", "", "SAMOA STANDARD TIME", "SST", -39600, "", "", 0x0, 0x0 }, \
	{ "ANDORRA", "AD", "AND", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "ANGOLA", "AO", "AGO", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
	{ "ANGUILLA", "AI", "AIA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "ANTIGUA", "AG", "ATG", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "ARGENTINA", "AR", "ARG", "", "", "ARGENTINA TIME", "ART", -10800, "", "", 0x0, 0x0 }, \
	{ "ARMENIA", "AM", "ARM", "", "", "ARMENIA TIME", "AMT", 14400, "", "", 0x0, 0x0 }, \
	{ "ARUBA", "AW", "ABW", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "ASCENSION", "SH", "SHN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "AUSTRALIA", "AU", "AUS", "WESTERN AUSTRALIA", "WA", "AUSTRALIAN WESTERN STANDARD TIME", "AWST", 28800, "", "", 0x0, 0x0 }, \
	{ "AUSTRALIA", "AU", "AUS", "WESTERN AUSTRALIA", "WA", "AUSTRALIAN CENTRAL WESTERN STANDARD TIME", "ACWST", 31500, "", "", 0x0, 0x0 }, \
	{ "AUSTRALIA", "AU", "AUS", "SOUTH AUSTRALIA", "SA", "AUSTRALIAN CENTRAL STANDARD TIME", "ACST", 34200, "AUSTRALIAN CENTRAL DAYLIGHT TIME", "ACDT", 0x3C00020900010001, 0xC4000303000100FF }, \
	{ "AUSTRALIA", "AU", "AUS", "NORTHERN TERRITORY", "NT", "AUSTRALIAN CENTRAL STANDARD TIME", "ACST", 34200, "", "", 0x0, 0x0 }, \
	{ "AUSTRALIA", "AU", "AUS", "AUSTRALIAN CAPITAL TERRITORY", "ACT", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "AUSTRALIAN EASTERN DAYLIGHT TIME", "AEDT", 0x3C00020900010001, 0xC4000303000100FF }, \
	{ "AUSTRALIA", "AU", "AUS", "TASMANIA", "TAS", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "AUSTRALIAN EASTERN DAYLIGHT TIME", "AEDT", 0x3C00020900010001, 0xC4000303000100FF }, \
	{ "AUSTRALIA", "AU", "AUS", "VICTORIA", "VIC", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "AUSTRALIAN EASTERN DAYLIGHT TIME", "AEDT", 0x3C00020900010001, 0xC4000303000100FF }, \
	{ "AUSTRALIA", "AU", "AUS", "NEW SOUTH WALES", "NSW", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "AUSTRALIAN EASTERN DAYLIGHT TIME", "AEDT", 0x3C00020900010001, 0xC4000303000100FF }, \
	{ "AUSTRALIA", "AU", "AUS", "QUEENSLAND", "QLD", "AUSTRALIAN EASTERN STANDARD TIME", "AEST", 36000, "", "", 0x0, 0x0 }, \
	{ "AUSTRALIA", "AU", "AUS", "LORD HOWE ISLAND", "", "LORD HOWE STANDARD TIME", "LHST", 37800, "LORD HOWE DAYLIGHT TIME (+30 MIN)", "LHDT", 0x1E00020900010001, 0xE2000203000100FF }, \
	{ "AUSTRIA", "AT", "AUT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "AZERBAIJAN", "AZ", "AZE", "", "", "AZERBAIJAN TIME", "AZT", 14400, "", "", 0x0, 0x0 }, \
	{ "BAHAMAS", "BS", "BHS", "", "", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "BAHRAIN", "BH", "BHR", "", "", "ARABIAN STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0 }, \
	{ "BANGLADESH", "BD", "BGD", "", "", "BANGLADESH STANDARD TIME", "BST", 21600, "", "", 0x0, 0x0 }, \
	{ "BARBADOS", "BB", "BRB", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "BARBUDA", "AG", "ATG", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "BELARUS", "BY", "BLR", "", "", "MOSCOW STANDARD TIME", "MSK", 10800, "", "", 0x0, 0x0 }, \
	{ "BELGIUM", "BE", "BEL", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "BELIZE", "BZ", "BLZ", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0 }, \
	{ "BENIN", "BJ", "BEN", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
	{ "BERMUDA", "BM", "BMU", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "BHUTAN", "BT", "BTN", "", "", "BHUTAN TIME", "BTT", 21600, "", "", 0x0, 0x0 }, \
	{ "BOLIVIA", "BO", "BOL", "", "", "BOLIVIA TIME", "BOT", -14400, "", "", 0x0, 0x0 }, \
	{ "BONAIRE", "BQ", "BES", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "BOSNIA", "BA", "BIH", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "BOTSWANA", "BW", "BWA", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0 }, \
	{ "BOUVET ISLAND", "BV", "BVT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "BRAZIL", "BR", "BRA", "", "", "ACRE TIME", "ACT", -18000, "", "", 0x0, 0x0 }, \
	{ "BRAZIL", "BR", "BRA", "", "", "AMAZON TIME", "AMT", -14400, "", "", 0x0, 0x0 }, \
	{ "BRAZIL", "BR", "BRA", "", "", "BRASILIA TIME", "BRT", -10800, "", "", 0x0, 0x0 }, \
	{ "BRAZIL", "BR", "BRA", "", "", "FERNANDO DE NORONHA TIME", "FNT", -7200, "", "", 0x0, 0x0 }, \
	{ "BRITISH VIRGIN ISLANDS", "VG", "VGB", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "BRUNEI", "BN", "BRN", "", "", "BRUNEI TIME", "BNT", 28800, "", "", 0x0, 0x0 }, \
	{ "BULGARIA", "BG", "BGR", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF }, \
	{ "BURKINA FASO", "BF", "BFA", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "BURUNDI", "BI", "BDI", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0 }, \
	{ "CAMBODIA", "KH", "KHM", "", "", "INDOCHINA TIME", "ICT", 25200, "", "", 0x0, 0x0 }, \
	{ "CAMEROON", "CM", "CMR", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
	{ "CANADA", "CA", "CAN", "NEWFOUNDLAND", "NL", "NEWFOUNDLAND STANDARD TIME", "NST", -12600, "NEWFOUNDLAND DAYLIGHT TIME", "NDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "LABRADOR", "NL", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "NEW BRUNSWICK", "NB", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "NOVA SCOTIA", "NS", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "PRINCE EDWARD ISLAND", "PE", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "ONTARIO", "ON", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "QUEBEC", "QC", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "MANITOBA", "MB", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "SASKATCHEWAN", "SK", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "NUNAVUT", "NU", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "ALBERTA", "AB", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "NORTHWEST TERRITORIES ", "NT", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "BRITISH COLUMBIA", "BC", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CANADA", "CA", "CAN", "YUKON", "YT", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "CAPE VERDE", "CV", "CPV", "", "", "CAPE VERDE TIME", "CVT", -3600, "", "", 0x0, 0x0 }, \
	{ "CAYMAN ISLANDS", "KY", "CYM", "", "", "EASTERN STANDARD TIME", "EST", -18000, "", "", 0x0, 0x0 }, \
	{ "CENTRAL AFRICAN REPUBLIC", "CF", "CAF", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
	{ "CHAD", "TD", "TCD", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
	{ "CHILE", "CL", "CHL", "", "", "CHILE STANDARD TIME", "CLT", -14400, "CHILE SUMMER TIME", "CLST", 0x3C00000800010001, 0xC4000003000100FF }, \
	{ "CHILE", "CL", "CHL", "EASTER ISLAND", "", "EASTER ISLAND STANDARD TIME", "EAST", -21600, "EASTER ISLAND SUMMER TIME", "EASST", 0x3C00000800010001, 0xC4000003000100FF }, \
	{ "CHINA", "CN", "CHN", "", "", "CHINA STANDARD TIME", "CST", 28800, "", "", 0x0, 0x0 }, \
	{ "CHRISTMAS ISLAND (AU)", "CX", "CXR", "", "", "CHRISTMAS ISLAND TIME", "CXT", 25200, "", "", 0x0, 0x0 }, \
	{ "COCOS ISLANDS (AU)", "CC", "CCK", "", "", "COCOS ISLAND TIME", "CCT", 23400, "", "", 0x0, 0x0 }, \
	{ "COLOMBIA", "CO", "COL", "", "", "COLOMBIA TIME", "COT", -18000, "", "", 0x0, 0x0 }, \
	{ "COMOROS", "KM", "COM", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0 }, \
	{ "COOK ISLANDS", "CK", "COK", "", "", "COOK ISLAND TIME", "CKT", -36000, "", "", 0x0, 0x0 }, \
	{ "COSTA RICA", "CR", "CRI", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0 }, \
	{ "CROATIA", "HR", "HRV", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "CUBA", "CU", "CUB", "", "", "CUBA STANDARD TIME", "CST", -18000, "CUBA DAYLIGHT TIME", "CDT", 0x3C00000200020001, 0xC400010A000100FF }, \
	{ "CURACAO", "CW", "CUW", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "CYPRUS", "CY", "CYP", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF }, \
	{ "CZECH REPUBLIC", "CZ", "CZE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "DEMOCRATIC REPUBLIC OF THE CONGO", "CD", "COD", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
	{ "DEMOCRATIC REPUBLIC OF THE CONGO", "CD", "COD", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0 }, \
	{ "DENMARK", "DK", "DNK", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "DHEKELIA", "", "", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00010200060001, 0xC4000209000600FF }, \
	{ "DJIBOUTI", "DJ", "DJI", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0 }, \
	{ "DOMINICA", "DM", "DMA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "DOMINICAN REPUBLIC", "DO", "DOM", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "EAST TIMOR", "TL", "TLS", "", "", "EAST TIMOR TIME", "TLT", 32400, "", "", 0x0, 0x0 }, \
	{ "ECUADOR", "EC", "ECU", "", "", "ECUADOR TIME", "ECT", -18000, "", "", 0x0, 0x0 }, \
	{ "EGYPT", "EG", "EGY", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "", "", 0x0, 0x0 }, \
	{ "EL SALVADOR", "SV", "SLV", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0 }, \
	{ "EQUATORIAL GUINEA", "GQ", "GNQ", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
	{ "ERITREA", "ER", "ERI", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0 }, \
	{ "ESTONIA", "EE", "EST", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF }, \
	{ "ESWATINI", "SZ", "SWZ", "", "", "SOUTH AFRICA STANDARD TIME", "SAST", 7200, "", "", 0x0, 0x0 }, \
	{ "ETHIOPIA", "ET", "ETH", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0 }, \
	{ "FALKLAND ISLANDS", "FK", "FLK", "", "", "FALKLAND ISLANDS SUMMER TIME", "FKST", -10800, "", "", 0x0, 0x0 }, \
	{ "FAROE ISLANDS", "FO", "FRO", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 0x3C00010200060001, 0xC4000209000600FF }, \
	{ "FIJI", "FJ", "FJI", "", "", "FIJI TIME", "FJT", 43200, "FIJI DAYLIGHT TIME", "FJDT", 0x3C00020A00010001, 0xC4000300000300FF }, \
	{ "FINLAND", "FI", "FIN", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF }, \
	{ "FRANCE", "FR", "FRA", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "FRENCH GUIANA", "GF", "GUF", "", "", "FRENCH GUIANA TIME", "GFT", -10800, "", "", 0x0, 0x0 }, \
	{ "FRENCH POLYNESIA", "PF", "PYF", "TAHITI", "", "TAHITI TIME", "TAHT", -36000, "", "", 0x0, 0x0 }, \
	{ "FRENCH POLYNESIA", "PF", "PYF", "MARQUESAS", "", "MARQUESAS TIME", "MART", -34200, "", "", 0x0, 0x0 }, \
	{ "FRENCH POLYNESIA", "PF", "PYF", "GAMBIER", "", "GAMBIER TIME", "GAMT", -32400, "", "", 0x0, 0x0 }, \
	{ "FRENCH SOUTHERN TERRITORIES", "TF", "ATF", "", "", "FRENCH SOUTHERN AND ANTARCTIC TIME", "TFT", 18000, "", "", 0x0, 0x0 }, \
	{ "FUTUNA", "WF", "WLF", "", "", "WALLIS AND FUTUNA TIME", "WFT", 43200, "", "", 0x0, 0x0 }, \
	{ "GABON", "GA", "GAB", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
	{ "GAMBIA", "GM", "GMB", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "GEORGIA", "GE", "GEO", "", "", "GEORGIA STANDARD TIME", "GET", 14400, "", "", 0x0, 0x0 }, \
	{ "GEORGIA", "GE", "GEO", "", "", "MOSCOW STANDARD TIME", "MSK", 10800, "", "", 0x0, 0x0 }, \
	{ "GERMANY", "DE", "DEU", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "GHANA", "GH", "GHA", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "GIBRALTAR", "GI", "GIB", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "GREECE", "GR", "GRC", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF }, \
	{ "GREENLAND", "GL", "GRL", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "GREENLAND", "GL", "GRL", "", "", "EASTERN GREENLAND TIME", "EGT", -3600, "EASTERN GREENLAND SUMMER TIME", "EGST", 0x3C00FE0200060001, 0xC400FF09000600FF }, \
	{ "GREENLAND", "GL", "GRL", "", "", "WESTERN GREENLAND TIME", "WGT", -10800, "WESTERN GREENLAND SUMMER TIME", "WGST", 0x3C00FE0200060001, 0xC400FF09000600FF }, \
	{ "GREENLAND", "GL", "GRL", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "ATLANTIC DAYLIGHT TIME", "ADT", 0x3C00FE0200060001, 0xC400FF09000600FF }, \
	{ "GRENADA", "GD", "GRD", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "GRENADINES", "VC", "VCT", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "GUADELOUPE", "GP", "GLP", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "GUAM", "GU", "GUM", "", "", "CHAMORRO STANDARD TIME", "CHST", 36000, "", "", 0x0, 0x0 }, \
	{ "GUATEMALA", "GT", "GTM", "", "", "FRENCH GUIANA TIME", "GFT", -10800, "", "", 0x0, 0x0 }, \
	{ "GUERNSEY", "GG", "GGY", "", "", "GREENWICH MEAN TIME", "GMT", 0, "BRITISH SUMMER TIME", "BST", 0x3C00010200060001, 0xC4000209000600FF }, \
	{ "GUINEA", "GN", "GIN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "GUINEA-BISSAU", "GW", "GNB", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "GUYANA", "GY", "GUY", "", "", "GUYANA TIME", "GYT", -14400, "", "", 0x0, 0x0 }, \
	{ "HAITI", "HT", "HTI", "", "", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "HEARD ISLANDS", "HM", "HMD", "", "", "FRENCH SOUTHERN AND ANTARCTIC TIME", "TFT", 18000, "", "", 0x0, 0x0 }, \
	{ "HERZEGOVINA", "BA", "BIH", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "HOLY SEE", "VA", "VAT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "HONDURAS", "HN", "HND", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0 }, \
	{ "HONG KONG", "HK", "HKG", "", "", "HONG KONG TIME", "HKT", 28800, "", "", 0x0, 0x0 }, \
	{ "HUNGARY", "HU", "HUN", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "ICELAND", "IS", "ISL", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "INDIA", "IN", "IND", "", "", "INDIA TIME ZONE", "IST", 19800, "", "", 0x0, 0x0 }, \
	{ "INDONESIA", "ID", "IDN", "", "", "WESTERN INDONESIAN TIME", "WIB", 25200, "", "", 0x0, 0x0 }, \
	{ "INDONESIA", "ID", "IDN", "", "", "CENTRAL INDONESIAN TIME", "WITA", 28800, "", "", 0x0, 0x0 }, \
	{ "INDONESIA", "ID", "IDN", "", "", "EASTERN INDONESIAN TIME", "WIT", 32400, "", "", 0x0, 0x0 }, \
	{ "IRAN", "IR", "IRN", "", "", "IRAN STANDARD TIME", "IRST", 12600, "IRAN DAYLIGHT TIME", "IRDT", 0x3C0000021600FF01, 0xC40000081600FFFF }, \
	{ "IRAQ", "IQ", "IRQ", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0 }, \
	{ "IRELAND", "IE", "IRL", "", "", "GREENWICH MEAN TIME", "GMT", 0, "IRISH STANDARD TIME", "IST", 0x3C00010200060001, 0xC4000209000600FF }, \
	{ "ISLE OF MAN", "IM", "IMN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "BRITISH SUMMER TIME", "BST", 0x3C00010200060001, 0xC4000209000600FF }, \
	{ "ISRAEL", "IL", "ISR", "", "", "ISRAEL STANDARD TIME", "IST", 7200, "ISRAEL DAYLIGHT TIME", "IDT", 0x3C00D20200060001, 0xC4000209000600FF }, \
	{ "ITALY", "IT", "ITA", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "IVORY COAST", "CI", "CIV", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "JAMAICA", "JM", "JAM", "", "", "EASTERN STANDARD TIME", "EST", -18000, "", "", 0x0, 0x0 }, \
	{ "JAN MAYEN", "SJ", "SJM", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "JAPAN", "JP", "JPN", "", "", "JAPAN STANDARD TIME", "JST", 32400, "", "", 0x0, 0x0 }, \
	{ "JERSEY", "JE", "JEY", "", "", "GREENWICH MEAN TIME", "GMT", 0, "BRITISH SUMMER TIME", "BST", 0x3C00010200060001, 0xC4000209000600FF }, \
	{ "JORDAN", "JO", "JOR", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00000200060501, 0xC4000109000605FF }, \
	{ "KAZAKHSTAN", "KZ", "KAZ", "", "", "ORAL TIME", "ORAT", 18000, "", "", 0x0, 0x0 }, \
	{ "KAZAKHSTAN", "KZ", "KAZ", "", "", "ALMA-ATA TIME", "ALMT", 21600, "", "", 0x0, 0x0 }, \
	{ "KEELING ISLANDS", "CC", "CCK", "", "", "COCOS ISLANDS TIME", "CCT", 23400, "", "", 0x0, 0x0 }, \
	{ "KENYA", "KE", "KEN", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0 }, \
	{ "KIRIBATI", "KI", "KIR", "", "", "GILBERT ISLAND TIME", "GILT", 43200, "", "", 0x0, 0x0 }, \
	{ "KIRIBATI", "KI", "KIR", "", "", "PHOENIX ISLAND TIME", "PHOT", 46800, "", "", 0x0, 0x0 }, \
	{ "KIRIBATI", "KI", "KIR", "", "", "LINE ISLANDS TIME", "LINT", 50400, "", "", 0x0, 0x0 }, \
	{ "KOSOVO", "XK", "XKX", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "KUWAIT", "KW", "KWT", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0 }, \
	{ "KYRGYZSTAN", "KG", "KGZ", "", "", "KYRGYZSTAN TIME", "KGT", 21600, "", "", 0x0, 0x0 }, \
	{ "LAOS", "LA", "LAO", "", "", "INDOCHINA TIME", "ICT", 25200, "", "", 0x0, 0x0 }, \
	{ "LATVIA", "LV", "LVA", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF }, \
	{ "LEBANON", "LB", "LBN", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00000200060001, 0xC4000009000600FF }, \
	{ "LESOTHO", "LS", "LSO", "", "", "SOUTH AFRICA STANDARD TIME", "SAST", 7200, "", "", 0x0, 0x0 }, \
	{ "LIBERIA", "LR", "LBR", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "LIBYA", "LY", "LBY", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "", "", 0x0, 0x0 }, \
	{ "LIECHTENSTEIN", "LI", "LIE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "LITHUANIA", "LT", "LTU", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF }, \
	{ "LUXEMBOURG", "LU", "LUX", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "MACAU", "MO", "MAC", "", "", "CHINA STANDARD TIME", "CST", 28800, "", "", 0x0, 0x0 }, \
	{ "MADAGASCAR", "MG", "MDG", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0 }, \
	{ "MALAWI", "MW", "MWI", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0 }, \
	{ "MALAYSIA", "MY", "MYS", "", "", "MALAYSIA TIME", "MYT", 28800, "", "", 0x0, 0x0 }, \
	{ "MALDIVES", "MV", "MDV", "", "", "MALDIVES TIME", "MVT", 18000, "", "", 0x0, 0x0 }, \
	{ "MALI", "ML", "MLI", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "MALTA", "MT", "MLT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "MARSHALL ISLANDS", "MH", "MHL", "", "", "MARSHALL ISLANDS TIME", "MHT", 43200, "", "", 0x0, 0x0 }, \
	{ "MARTINIQUE", "MQ", "MTQ", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "MAURITANIA", "MR", "MRT", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "MAURITIUS", "MU", "MUS", "", "", "MAURITIUS TIME", "MUT", 14400, "", "", 0x0, 0x0 }, \
	{ "MAYOTTE", "YT", "MYT", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0 }, \
	{ "MCDONALD ISLANDS", "US", "USA", "", "", "ALASKA STANDARD TIME", "AKST", 32400, "ALASKA DAYLIGHT TIME", "AKDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "MCDONALD ISLANDS", "HM", "HMD", "", "", "FRENCH SOUTHERN AND ANTARCTIC TIME", "TFT", 18000, "", "", 0x0, 0x0 }, \
	{ "MEXICO", "MX", "MEX", "", "", "EASTERN STANDARD TIME", "EST", -18000, "", "", 0x0, 0x0 }, \
	{ "MEXICO", "MX", "MEX", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020300010001, 0xC4000209000600FF }, \
	{ "MEXICO", "MX", "MEX", "", "", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020300010001, 0xC4000209000600FF }, \
	{ "MEXICO", "MX", "MEX", "", "", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020300010001, 0xC4000209000600FF }, \
	{ "MICRONESIA", "FM", "FSM", "", "", "CHUUK TIME", "CHUT", 36000, "", "", 0x0, 0x0 }, \
	{ "MICRONESIA", "FM", "FSM", "", "", "POHNPEI STANDARD TIME", "PONT", 39600, "", "", 0x0, 0x0 }, \
	{ "MICRONESIA", "FM", "FSM", "", "", "KOSRAE TIME", "KOST", 39600, "", "", 0x0, 0x0 }, \
	{ "MIDWAY", "UM", "UMI", "", "", "SAMOA STANDARD TIME", "SST", -39600, "", "", 0x0, 0x0 }, \
	{ "MIQUELON", "PM", "SPM", "", "", "PIERRE & MIQUELON STANDARD TIME", "PMST", -10800, "PIERRE & MIQUELON DAYLIGHT TIME", "PMDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "MOLDOVA", "MD", "MDA", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "MONACO", "MC", "MCO", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "MONGOLIA", "MN", "MNG", "", "", "HOVD TIME", "HOVT", 25200, "", "", 0x0, 0x0 }, \
	{ "MONGOLIA", "MN", "MNG", "", "", "ULAANBAATAR TIME", "ULAT", 28800, "", "", 0x0, 0x0 }, \
	{ "MONGOLIA", "MN", "MNG", "", "", "CHOIBALSAN TIME", "CHOT", 28800, "", "", 0x0, 0x0 }, \
	{ "MONTENEGRO", "ME", "MNE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "MONTSERRAT", "MS", "MSR", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "MOROCCO", "MA", "MAR", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "", "", 0x0, 0x0 }, \
	{ "MOZAMBIQUE", "MZ", "MOZ", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0 }, \
	{ "MYANMAR", "MM", "MMR", "", "", "MYANMAR TIME", "MMT", 23400, "", "", 0x0, 0x0 }, \
	{ "NAMIBIA", "NA", "NAM", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0 }, \
	{ "NAURU", "NR", "NRU", "", "", "NAURU TIME", "NRT", 43200, "", "", 0x0, 0x0 }, \
	{ "NEPAL", "NP", "NPL", "", "", "NEPAL TIME", "NPT", 20700, "", "", 0x0, 0x0 }, \
	{ "NETHERLANDS", "NL", "NLD", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "NETHERLANDS ANTILLES", "AN", "ANT", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "NEVIS", "KN", "KNA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "NEW CALEDONIA", "NC", "NCL", "", "", "NEW CALEDONIA TIME", "NCT", 39600, "", "", 0x0, 0x0 }, \
	{ "NEW ZEALAND", "NZ", "NZL", "", "", "NEW ZEALAND STANDARD TIME", "NZST", 43200, "NEW ZEALAND DAYLIGHT TIME", "NZDT", 0x3C00020800060001, 0xC4000303000100FF }, \
	{ "NEW ZEALAND", "NZ", "NZL", "CHATHAM ISLAND", "", "CHATHAM ISLAND STANDARD TIME", "CHAST", 45900, "CHATHAM ISLAND DAYLIGHT TIME", "CHADT", 0x3C00020800060001, 0xC4000303000100FF }, \
	{ "NICARAGUA", "NI", "NIC", "", "", "CENTRAL STANDARD TIME", "CST", -21600, "", "", 0x0, 0x0 }, \
	{ "NIGER", "NE", "NER", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
	{ "NIGERIA", "NG", "NGA", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
	{ "NIUE", "NU", "NIU", "", "", "NIUE TIME", "NUT", -39600, "", "", 0x0, 0x0 }, \
	{ "NORFOLK ISLAND", "NF", "NFK", "", "", "NORFOLK TIME", "NFT", 39600, "NORFOLK DAYLIGHT TIME", "NFDT", 0x3C00020900010001, 0xC4000303000100FF }, \
	{ "NORTH KOREA", "KP", "PRK", "", "", "KOREA STANDARD TIME", "KST", 32400, "", "", 0x0, 0x0 }, \
	{ "NORTH MACEDONIA", "MK", "MKD", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "NORTHERN MARIANA ISLANDS", "MP", "MNP", "", "", "CHAMORRO STANDARD TIME", "CHST", 36000, "", "", 0x0, 0x0 }, \
	{ "NORWAY", "NO", "NOR", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "OMAN", "OM", "OMN", "", "", "GULF STANDARD TIME", "GST", 14400, "", "", 0x0, 0x0 }, \
	{ "PAKISTAN", "PK", "PAK", "", "", "PAKISTAN STANDARD TIME", "PKT", 18000, "", "", 0x0, 0x0 }, \
	{ "PALAU", "PW", "PLW", "", "", "PALAU TIME", "PWT", 32400, "", "", 0x0, 0x0 }, \
	{ "PALESTINE", "PS", "PSE", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00D20200060001, 0xC4000209000600FF }, \
	{ "PANAMA", "PA", "PAN", "", "", "EASTERN STANDARD TIME", "EST", -18000, "", "", 0x0, 0x0 }, \
	{ "PAPUA NEW GUINEA", "PG", "PNG", "", "", "PAPUA NEW GUINEA TIME", "PGT", 36000, "", "", 0x0, 0x0 }, \
	{ "PAPUA NEW GUINEA", "PG", "PNG", "BOUGAINVILLE", "", "BOUGAINVILLE STANDARD TIME", "BST", 39600, "", "", 0x0, 0x0 }, \
	{ "PARAGUAY", "PY", "PRY", "", "", "PARAGUAY TIME", "PYT", -14400, "PARAGUAY SUMMER TIME", "PYST", 0x3C00000900010001, 0xC4000002000400FF }, \
	{ "PERU", "PE", "PER", "", "", "PERU TIME", "PET", -18000, "", "", 0x0, 0x0 }, \
	{ "PHILIPPINES", "PH", "PHL", "", "", "PHILIPPINE TIME", "PHT", 28800, "", "", 0x0, 0x0 }, \
	{ "PITCAIRN ISLANDS", "PN", "PCN", "", "", "PITCAIRN STANDARD TIME", "PST", 28800, "", "", 0x0, 0x0 }, \
	{ "POLAND", "PL", "POL", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "PORTUGAL", "PT", "PRT", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 0x3C00010200060001, 0xC4000209000600FF }, \
	{ "PORTUGAL", "PT", "PRT", "", "", "AZORES TIME", "AZOT", 3600, "AZORES SUMMER TIME", "AZOST", 0x3C00000200060001, 0xC4000109000600FF }, \
	{ "PRINCIPE", "ST", "STP", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "PUERTO RICO", "PR", "PRI", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "QATAR", "QA", "QAT", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0 }, \
	{ "REPUBLIC OF THE CONGO", "CG", "COG", "", "", "WEST AFRICA TIME", "WAT", 3600, "", "", 0x0, 0x0 }, \
	{ "REUNION", "RE", "REU", "", "", "REUNION TIME", "RET", 14400, "", "", 0x0, 0x0 }, \
	{ "ROMANIA", "RO", "ROU", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF }, \
	{ "RUSSIA", "RU", "RUS", "KALININGRAD", "", "EASTERN EUROPEAN TIME", "EET", 7200, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "MOSCOW", "", "MOSCOW STANDARD TIME", "MSK", 10800, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "SAMARA", "", "SAMARA TIME", "SAMT", 14400, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "YEKATERINBURG", "", "YEKATERINBURG TIME", "YEKT", 18000, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "OMSK", "", "OMSK STANDARD TIME", "OMST", 21600, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "KRASNOYARSK", "", "KRASNOYARSK TIME", "KRAT", 25200, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "NOVOSIBIRSK", "", "NOVOSIBIRSK TIME", "NOVT", 25200, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "IRKUTSK", "", "IRKUTSK TIME", "IRKT", 28800, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "YAKUTSK", "", "YAKUTSK TIME", "YAKT", 32400, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "VLADIVOSTOK", "", "VLADIVOSTOK TIME", "VLAT", 36000, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "MAGADAN", "", "MAGADAN TIME", "MAGT", 39600, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "SAKHALIN", "", "SAKHALIN TIME", "SAKT", 39600, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "SREDNEKOLYMSK", "", "SREDNEKOLYMSK TIME", "SRED", 39600, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "ANADYR", "", "ANADYR TIME", "ANAT", 43200, "", "", 0x0, 0x0 }, \
	{ "RUSSIA", "RU", "RUS", "KAMCHATKA", "", "KAMCHATKA TIME", "PETT", 43200, "", "", 0x0, 0x0 }, \
	{ "RWANDA", "RW", "RWA", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0 }, \
	{ "SABA", "BQ", "BES", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "SAINT BARTHELEMY", "BL", "BLM", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "SAINT HELENA", "SH", "SHN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "SAINT KITTS", "KN", "KNA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "SAINT LUCIA", "LC", "LCA", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "SAINT MARTIN", "MF", "MAF", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "SAINT PIERRE", "PM", "SPM", "", "", "PIERRE & MIQUELON STANDARD TIME", "PMST", -10800, "PIERRE & MIQUELON DAYLIGHT TIME", "PMDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "SAINT VINCENT", "VC", "VCT", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "SAMOA", "WS", "WSM", "", "", "WEST SAMOA TIME", "WST", 46800, "WEST SAMOA TIME", "WST", 0x3C00030800060001, 0xC4000403000100FF }, \
	{ "SAN MARINO", "SM", "SMR", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "SAO TOME", "ST", "STP", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "SAUDI ARABIA", "SA", "SAU", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0 }, \
	{ "SENEGAL", "SN", "SEN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "SERBIA", "RS", "SRB", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "SEYCHELLES", "SC", "SYC", "", "", "SEYCHELLES TIME", "SCT", 14400, "", "", 0x0, 0x0 }, \
	{ "SIERRA LEONE", "SL", "SLE", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "SINGAPORE", "SG", "SGP", "", "", "SINGAPORE TIME", "SGT", 28800, "", "", 0x0, 0x0 }, \
	{ "SINT EUSTATIUS", "BQ", "BES", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "SINT MAARTEN", "SX", "SXM", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "SLOVAKIA", "SK", "SVK", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "SLOVENIA", "SI", "SVN", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "SOLOMON ISLANDS", "SB", "SLB", "", "", "SOLOMON ISLANDS TIME", "SBT", 39600, "", "", 0x0, 0x0 }, \
	{ "SOMALIA", "SO", "SOM", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0 }, \
	{ "SOUTH AFRICA", "ZA", "ZAF", "", "", "SOUTH AFRICA STANDARD TIME", "SAST", 7200, "", "", 0x0, 0x0 }, \
	{ "SOUTH AFRICA", "ZA", "ZAF", "MARION ISLAND", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0 }, \
	{ "SOUTH GEORGIA ISLAND", "GS", "SGS", "", "", "SOUTH GEORGIA TIME", "GST", -7200, "", "", 0x0, 0x0 }, \
	{ "SOUTH KOREA", "KR", "KOR", "", "", "KOREA STANDARD TIME", "KST", 32400, "", "", 0x0, 0x0 }, \
	{ "SOUTH SANDWICH ISLANDS", "GS", "SGS", "", "", "SOUTH GEORGIA TIME", "GST", -7200, "", "", 0x0, 0x0 }, \
	{ "SOUTH SUDAN", "SS", "SSD", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0 }, \
	{ "SPAIN", "ES", "ESP", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "SPAIN", "ES", "ESP", "", "", "WESTERN EUROPEAN TIME", "WET", 0, "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "SRI LANKA", "LK", "LKA", "", "", "INDIA STANDARD TIME", "IST", 19800, "", "", 0x0, 0x0 }, \
	{ "SUDAN", "SD", "SDN", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0 }, \
	{ "SURINAME", "SR", "SUR", "", "", "SURINAME TIME", "SRT", -10800, "", "", 0x0, 0x0 }, \
	{ "SVALBARD", "SJ", "SJM", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "SWAZILAND", "SZ", "SWZ", "", "", "SOUTH AFRICA STANDARD TIME", "SAST", 7200, "", "", 0x0, 0x0 }, \
	{ "SWEDEN", "SE", "SWE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "SWITZERLAND", "CH", "CHE", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "SYRIA", "SY", "SYR", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00000200060501, 0xC4000009000605FF }, \
	{ "TAIWAN", "TW", "TWN", "", "", "CHINA STANDARD TIME", "CST", 28800, "", "", 0x0, 0x0 }, \
	{ "TAJIKISTAN", "TJ", "TJK", "", "", "TAJIKISTAN TIME", "TJT", 18000, "", "", 0x0, 0x0 }, \
	{ "TANZANIA", "TZ", "TZA", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0 }, \
	{ "THAILAND", "TH", "THA", "", "", "INDOCHINA TIME", "ICT", 25200, "", "", 0x0, 0x0 }, \
	{ "TOBAGO", "TT", "TTO", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "TOGO", "TG", "TGO", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "TOKELAU", "TK", "TKL", "", "", "TOKELAU TIME", "TKT", 46800, "", "", 0x0, 0x0 }, \
	{ "TONGA", "TO", "TON", "", "", "TONGA TIME", "TOT", 46800, "", "", 0x0, 0x0 }, \
	{ "TRINIDAD", "TT", "TTO", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "TRISTAN DA CUNHA", "SH", "SHN", "", "", "GREENWICH MEAN TIME", "GMT", 0, "", "", 0x0, 0x0 }, \
	{ "TUNISIA", "TN", "TUN", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "", "", 0x0, 0x0 }, \
	{ "TURKEY", "TR", "TUR", "", "", "TURKEY TIME", "TRT", 10800, "", "", 0x0, 0x0 }, \
	{ "TURKMENISTAN", "TM", "TKM", "", "", "TURKMENISTAN TIME", "TMT", 18000, "", "", 0x0, 0x0 }, \
	{ "TURKS AND CAICOS", "TC", "TCA", "", "", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "TUVALU", "TV", "TUV", "", "", "TUVALU TIME", "TVT", 43200, "", "", 0x0, 0x0 }, \
	{ "UGANDA", "UG", "UGA", "", "", "EASTERN AFRICA TIME", "EAT", 10800, "", "", 0x0, 0x0 }, \
	{ "UKRAINE", "UA", "UKR", "", "", "EASTERN EUROPEAN TIME", "EET", 7200, "EASTERN EUROPEAN DAYLIGHT TIME", "EEDT", 0x3C00030200060001, 0xC4000409000600FF }, \
	{ "UKRAINE", "UA", "UKR", "", "", "MOSCOW STANDARD TIME", "MSK", 10800, "", "", 0x0, 0x0 }, \
	{ "UNITED ARAB EMIRATES", "AE", "ARE", "", "", "GULF STANDARD TIME", "GST", 14400, "", "", 0x0, 0x0 }, \
	{ "UNITED KINGDOM", "GB", "GBR", "", "", "GREENWICH MEAN TIME", "GMT", 0, "BRITISH SUMMER TIME", "BST", 0x3C00010200060001, 0xC4000209000600FF }, \
	{ "UNITED STATES", "US", "USA", "ALABAMA", "AL", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "ALASKA", "AK", "ALASKA STANDARD TIME", "AKST", -32400, "ALASKA DAYLIGHT TIME", "AKDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "ALASKA", "AK", "HAWAII-ALEUTIAN STANDARD TIME", "HST", -36000, "HAWAII-ALEUTIAN DAYLIGHT TIME", "HDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "ARIZONA", "AZ", "MOUNTAIN STANDARD TIME", "MST", -25200, "", "", 0x0, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "ARKANSAS", "AR", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "CALIFORNIA", "CA", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "COLORADO", "CO", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "CONNECTICUT", "CT", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "DELAWARE", "DE", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "DISTRICT OF COLUMBIA", "DC", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "FLORIDA", "FL", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "FLORIDA", "FL", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "GEORGIA", "GA", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "HAWAII", "HI", "HAWAII-ALEUTIAN STANDARD TIME", "HST", -36000, "", "", 0x0, 0x0 }, \
	{ "UNITED STATES", "US", "USA", "IDAHO", "ID", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "IDAHO", "ID", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "ILLINOIS", "IL", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "INDIANA", "IN", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "INDIANA", "IN", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "IOWA", "IA", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "KANSAS", "KS", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "KANSAS", "KS", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "KENTUCKY", "KY", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "KENTUCKY", "KY", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "LOUISIANA", "LA", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "MAINE", "ME", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "MARYLAND", "MD", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "MASSACHUSETTS", "MA", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "MICHIGAN", "MI", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "MICHIGAN", "MI", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "MINNESOTA", "MN", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "MISSISSIPPI", "MS", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "MISSOURI", "MO", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "MONTANA", "MT", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "NEBRASKA", "NE", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "NEBRASKA", "NE", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "NEVADA", "NV", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "NEVADA", "NV", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "NEW HAMPSHIRE", "NH", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "NEW JERSEY", "NJ", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "NEW MEXICO", "NM", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "NEW YORK", "NY", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "NORTH CAROLINA", "NC", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "NORTH DAKOTA", "ND", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "NORTH DAKOTA", "ND", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "OHIO", "OH", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "OKLAHOMA", "OK", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "OREGON", "OR", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "OREGON", "OR", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "PENNSYLVANIA", "PA", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "RHODE ISLAND", "RI", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "SOUTH CAROLINA", "SC", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "SOUTH DAKOTA", "SD", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "SOUTH DAKOTA", "SD", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "TENNESSEE", "TN", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "TENNESSEE", "TN", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "TEXAS", "TX", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "TEXAS", "TX", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "UTAH", "UT", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "VERMONT", "VT", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "VIRGINIA", "VA", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "WASHINGTON", "WA", "PACIFIC STANDARD TIME", "PST", -28800, "PACIFIC DAYLIGHT TIME", "PDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "WEST VIRGINIA", "WV", "EASTERN STANDARD TIME", "EST", -18000, "EASTERN DAYLIGHT TIME", "EDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "WISCONSIN", "WI", "CENTRAL STANDARD TIME", "CST", -21600, "CENTRAL DAYLIGHT TIME", "CDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES", "US", "USA", "WYOMING", "WY", "MOUNTAIN STANDARD TIME", "MST", -25200, "MOUNTAIN DAYLIGHT TIME", "MDT", 0x3C00020200020001, 0xC400020A000100FF }, \
	{ "UNITED STATES VIRGIN ISLANDS", "VI", "VIR", "", "", "ATLANTIC STANDARD TIME", "AST", -14400, "", "", 0x0, 0x0 }, \
	{ "URUGUAY", "UY", "URY", "", "", "URUGUAY TIME", "UYT", -10800, "", "", 0x0, 0x0 }, \
	{ "UZBEKISTAN", "UZ", "UZB", "", "", "UZBEKISTAN TIME", "UZT", 18000, "", "", 0x0, 0x0 }, \
	{ "VANUATU", "VU", "VUT", "", "", "VANUATU TIME", "VUT", 39600, "", "", 0x0, 0x0 }, \
	{ "VATICAN CITY", "VA", "VAT", "", "", "CENTRAL EUROPEAN TIME", "CET", 3600, "CENTRAL EUROPEAN DAYLIGHT TIME", "CEDT", 0x3C00020200060001, 0xC4000309000600FF }, \
	{ "VENEZUELA", "VE", "VEN", "", "", "VENEZUELAN STANDARD TIME", "VET", -14400, "", "", 0x0, 0x0 }, \
	{ "VIETNAM", "VN", "VNM", "", "", "INDOCHINA TIME", "ICT", 25200, "", "", 0x0, 0x0 }, \
	{ "WALLIS", "WF", "WLF", "", "", "WALLIS AND FUTUNA TIME", "WFT", 43200, "", "", 0x0, 0x0 }, \
	{ "WESTERN SAHARA", "EH", "ESH", "", "", "WESTERN EUROPEAN DAYLIGHT TIME", "WEDT", 3600, "", "", 0x0, 0x0 }, \
	{ "YEMEN", "YE", "YEM", "", "", "ARABIA STANDARD TIME", "AST", 10800, "", "", 0x0, 0x0 }, \
	{ "ZAMBIA", "ZM", "ZMB", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0 }, \
	{ "ZIMBABWE", "ZW", "ZWE", "", "", "CENTRAL AFRICA TIME", "CAT", 7200, "", "", 0x0, 0x0 } \
}

#define TZ_COUNTRY_ALIASES_ENTRIES_m10      14
#define TZ_COUNTRY_ALIASES_TABLE_m10 { \
	{ "CHINA", "PEOPLE'S REPUBIC OF CHINA" }, \
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

#define TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m10      1
#define TZ_COUNTRY_ACRONYM_ALIASES_TABLE_m10 { \
	{ "GB", "UK" } \
}



//**********************************************************************************//
//***********************  Headers that Require medlib_m10.h  **********************//
//**********************************************************************************//

#include "medrec_m10.h"


#endif  // MEDLIB_IN_m10







