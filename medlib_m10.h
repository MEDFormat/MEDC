
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

// The library is written with tab width = indent width = 8 spaces and a monospaced font.
// Set your editor preferences to these for intended alignment.

// All functions, constants, macros, and data types defined in the library are tagged
// with the suffix "_m10" (for "MED 1.0"). This is to facilitate using multiple versions
// of the library in concert in the future; for example to write a MED 1.0 to MED 2.0 converter.


//**********************************************************************************//
//*****************************  Unix Library Includes  ****************************//
//**********************************************************************************//

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <stdatomic.h>



//**********************************************************************************//
//******************************  Elemental Typedefs  ******************************//
//**********************************************************************************//

#ifndef SIZE_TYPES_IN_m10
#define SIZE_TYPES_IN_m10

typedef unsigned char           ui1;
typedef char                    si1;
typedef unsigned short          ui2;
typedef short                   si2;
typedef unsigned int            ui4;
typedef int                     si4;
typedef unsigned long           ui8;
typedef long		        si8;
typedef float			sf4;
typedef double			sf8;
typedef long double		sf16;   // NOTE: it often requires an explicit compiler instruction to implement true long floating point math
					// in icc: "-Qoption,cpp,--extended_float_types"
#endif  // SIZE_TYPES_IN_m10

// MED Library Ternary Boolean Schema
typedef si1                                     TERN_m10;
#define TRUE_m10                                1
#define UNKNOWN_m10                             0
#define FALSE_m10                               -1

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
#define CMP_NORMAL_CDF_TABLE_m10              { 0.00134989803163010, 0.00186581330038404, 0.00255513033042794, 0.00346697380304067, \
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
        ui4                     type_code;  // note this is not null terminated and so cannot be treated as a string as in RECORD_HEADER_m10 structure
        ui1                     version_major;
	ui1                     version_minor;
        ui2                     total_bytes;  // note maximum record size is 65535 - smaller than in RECORD_HEADER_m10 structure
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
        TERN_m10 _Atomic		mutex;
        ui4                             **count;  // used by RED/PRED encode & decode
        CMP_STATISTICS_BIN_m10          **sorted_count;  // used by RED/PRED encode & decode
        ui8                             **cumulative_count;  // used by RED/PRED encode & decode
        ui8                             **minimum_range;  // used by RED/PRED encode & decode
        ui1                             **symbol_map;  // used by RED/PRED encode & decode
        si4                             *input_buffer;
        ui1                             *compressed_data;  // passed in decompression, returned in compression, should not be updated
        CMP_BLOCK_FIXED_HEADER_m10      *block_header; // points to beginning of current block within compressed_data array, updatable
        si4                             *decompressed_data;  // returned in decompression or if lossy data requested, used in some compression modes, should not be updated
        si4                             *decompressed_ptr;  // points to beginning of current block within decompressed_data array, updatable
        si4                             *original_data;  // passed in compression, should not be updated
        si4                             *original_ptr;  // points to beginning of current block within original_data array, updatable
        si1                             *difference_buffer;  // passed in both compression & decompression
	si1                             *derivative_buffer;  // used if needed in compression & decompression, size of maximum block differences
        si4                             *detrended_buffer;  // used if needed in compression, size of decompressed block
        si4                             *scaled_amplitude_buffer;  // used if needed in compression, size of decompressed block
        si4                             *scaled_frequency_buffer;  // used if needed in compression, size of decompressed block
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
	struct NODE_STRUCT_m10  *prev, *next;
} NODE_m10;


// Function Prototypes
CMP_PROCESSING_STRUCT_m10	*CMP_allocate_processing_struct_m10(CMP_PROCESSING_STRUCT_m10 *cps, ui4 mode, si8 data_samples, si8 compressed_data_bytes, si8 difference_bytes, ui4 block_samples, CMP_DIRECTIVES_m10 *directives, CMP_PARAMETERS_m10 *parameters);
sf8		CMP_calculate_mean_residual_ratio_m10(si4 *data, si4 *lossy_data, ui4 n_samps);
void    	CMP_calculate_statistics_m10(void *stats_ptr, si4 *data, si8 len, NODE_m10 *nodes);  // "stats_ptr" is a pointer to a "REC_Stat_v10_m10" structure ffrom medrec_m10.h
TERN_m10	CMP_check_CPS_allocation_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void		CMP_cps_mutex_off_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void		CMP_cps_mutex_on_m10(CMP_PROCESSING_STRUCT_m10 *cps);
TERN_m10     	CMP_decrypt_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_detrend_m10(si4 *input_buffer, si4 *output_buffer, si8 len, CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_encode_m10(CMP_PROCESSING_STRUCT_m10 *cps, si8 start_time, si4 acquisition_channel_number, ui4 number_of_samples);
TERN_m10     	CMP_encrypt_m10(CMP_PROCESSING_STRUCT_m10 *cps);
TERN_m10	CMP_find_amplitude_scale_m10(CMP_PROCESSING_STRUCT_m10 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m10 *cps));
void    	CMP_find_extrema_m10(si4 *input_buffer, si8 len, si4 *min, si4 *max, CMP_PROCESSING_STRUCT_m10 *cps);
TERN_m10	CMP_find_frequency_scale_m10(CMP_PROCESSING_STRUCT_m10 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m10 *cps));
void    	CMP_free_processing_struct_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_generate_lossy_data_m10(CMP_PROCESSING_STRUCT_m10 *cps, si4 *input_buffer, si4 *output_buffer, ui1 mode);
void		CMP_generate_parameter_map_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void    	CMP_get_variable_region_m10(CMP_PROCESSING_STRUCT_m10 *cps);
void		CMP_initialize_normal_CDF_table_m10(void);
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
void    	CMP_unscale_frequency_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf4 scale_factor);
CMP_BLOCK_FIXED_HEADER_m10	*CMP_update_CPS_pointers_m10(CMP_PROCESSING_STRUCT_m10 *cps, ui1 flags);


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

// Target Value Constants
#define DEFAULT_MODE_m10        0
#define FIND_START_m10          1
#define FIND_END_m10            2
#define FIND_CENTER_m10         3
#define FIND_CURRENT_m10        4
#define FIND_NEXT_m10           5
#define FIND_CLOSEST_m10        6

// Text Color Constant Strings
#define TC_BLACK_m10            "\033[30m"
#define TC_RED_m10              "\033[31m"
#define TC_GREEN_m10            "\033[32m"
#define TC_YELLOW_m10           "\033[33m"
#define TC_BLUE_m10             "\033[34m"
#define TC_MAGENTA_m10          "\033[35m"
#define TC_CYAN_m10             "\033[36m"
#define TC_WHITE_m10            "\033[37m"
#define TC_BRIGHT_BLACK_m10     "\033[30;1m"
#define TC_BRIGHT_RED_m10       "\033[31;1m"
#define TC_BRIGHT_GREEN_m10     "\033[32;1m"
#define TC_BRIGHT_YELLOW_m10    "\033[33;1m"
#define TC_BRIGHT_BLUE_m10      "\033[34;1m"
#define TC_BRIGHT_MAGENTA_m10   "\033[35;1m"
#define TC_BRIGHT_CYAN_m10      "\033[36;1m"
#define TC_BRIGHT_WHITE_m10     "\033[37;1m"
#define TC_RESET_m10            "\033[0m"

// Other Time Related Constants
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
#define BEGINNING_OF_INDICES_m10                        ((si8) 0x0000000000000000)
#define END_OF_INDICES_m10                              ((si8) 0x7FFFFFFFFFFFFFFF)
#define UUTC_NO_ENTRY_m10                               ((si8) 0x8000000000000000)
#define UUTC_EARLIEST_TIME_m10                          ((si8) 0x0000000000000000)  // 00:00:00.000000 Thursday, 1 Jan 1970, UTC
#define UUTC_LATEST_TIME_m10                            ((si8) 0x7FFFFFFFFFFFFFFF)  // 04:00:54.775808 Sunday, 10 Jan 29424, UTC
#define BEGINNING_OF_TIME_m10                           UUTC_EARLIEST_TIME_m10
#define END_OF_TIME_m10                                 UUTC_LATEST_TIME_m10
#define CURRENT_TIME_m10				((si8) 0xFFFFFFFFFFFFFFFF)  // used with time_string_m10() & generate_recording_time_offset_m10()
#define TWENTY_FOURS_HOURS_m10				((si8) 86400000000)
#define Y2K_m10                                         ((si8) 0x00035D013B37E000)  // 00:00:00.000000 Saturday, 1 Jan 2000, UTC  (946684800000000 decimal)

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
#define NO_TYPE_CODE_m10                                        (ui4) 0x00000000
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
#define METADATA_RECORDING_CITY_OFFSET_m10                      13480           // utf8[63]
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
#define VIDEO_INDEX_START_FRAME_OFFSET_m10                      16                      // si4
#define VIDEO_INDEX_START_FRAME_NO_ENTRY_m10                    0x80000000
#define VIDEO_INDEX_VIDEO_FILE_NUMBER_OFFSET_m10                20                      // si4
#define VIDEO_INDEX_VIDEO_FILE_NUMBER_NO_ENTRY_m10              -1


//**********************************************************************************//
//**********************************  MED Macros  **********************************//
//**********************************************************************************//

#define ABS_m10(x)			( ((x) >= 0) ? (x) : -(x) )  // do not increment/decrement in call to ABS (as x occurs thrice)
#define HEX_STRING_BYTES_m10(x)         ( ((x) + 1) * 3 )
#define REMOVE_DISCONTINUITY_m10(x)     ( ((x) >= 0) ? (x) : -(x) )  // do not increment/decrement in call to REMOVE_DISCONTINUITY (as x occurs thrice)
#define APPLY_DISCONTINUITY_m10(x)      ( ((x) < 0) ? (x) : -(x) )  // do not increment/decrement in call to APPLY_DISCONTINUITY (as x occurs thrice)
#define MAX_OPEN_FILES_m10(number_of_channels, number_of_segments)      ((5 * number_of_channels * number_of_segments) + (2 * number_of_segments) + (2 * number_of_channels) + 5)
// Note: final +5 == 2 for session level records plus 3 for stdin, stdout & stderr


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
        si4        observe_DST;
        si1        daylight_timezone[TIMEZONE_STRING_BYTES_m10];
        si1        daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m10];
        si1        daylight_time_start_description[METADATA_RECORDING_LOCATION_BYTES_m10];
        si8        daylight_time_start_code;  // DAYLIGHT_TIME_CHANGE_CODE_m10 - cast to use other fields
        si1        daylight_time_end_description[METADATA_RECORDING_LOCATION_BYTES_m10];
        si8        daylight_time_end_code;  // DAYLIGHT_TIME_CHANGE_CODE_m10 - cast to use other fields
} TIMEZONE_INFO_m10;

typedef struct {
	TERN_m10	conditioned;
        si8     	start_time;
        si8     	end_time;
        si8     	start_index;  // session-relative (global indexing)
        si8     	end_index;  // session-relative (global indexing)
	si8		local_start_index;  // segment-relative (local indexing)
	si8		local_end_index;  // segment-relative (local indexing)
	si8		number_of_samples;
        si4     	start_segment_number;
        si4     	end_segment_number;
        si8     	session_start_time;
        si8     	session_end_time;
	si1		*index_reference_channel_name;  // string containing channel base name (or NULL, if unnecessary)
	si4		index_reference_channel_index;  // index of the index reference channel in the session channel array
} TIME_SLICE_m10;

typedef struct {
        // Password
	PASSWORD_DATA_m10               password_data;
        // Time Constants
	si8				session_start_time;
	si8                             recording_time_offset;
        si4                             standard_UTC_offset;
        si1                             standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m10];
        si1                             standard_timezone_string[TIMEZONE_STRING_BYTES_m10];
        TERN_m10                        observe_DST;
	TERN_m10			RTO_known;
        si1                             daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m10];
        si1                             daylight_timezone_string[TIMEZONE_STRING_BYTES_m10];
        DAYLIGHT_TIME_CHANGE_CODE_m10   daylight_time_start_code;  // si1[8] / si8
        DAYLIGHT_TIME_CHANGE_CODE_m10   daylight_time_end_code;  // si1[8] / si8
        TIMEZONE_INFO_m10               *timezone_table;
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
        sf8                             *CMP_normal_CDF_table;
        // CRC
        ui4                             **CRC_table;
        ui4                             CRC_mode;
        // AES tables
        si4                             *AES_sbox_table;
        si4                             *AES_rcon_table;
        si4                             *AES_rsbox_table;
        // SHA256 tables
        ui4                             *SHA_h0_table;
        ui4                             *SHA_k_table;
        // UTF8 tables
        ui4                             *UTF8_offsets_table;
        si1                             *UTF8_trailing_bytes_table;
	// Miscellaneous
        TERN_m10                        verbose;
        ui4                             behavior_on_fail;
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
        union {  // anonymous union
                struct {
                        si1     type_string[TYPE_BYTES_m10];
                        ui1     MED_version_major;
                        ui1     MED_version_minor;
                        ui1     byte_order_code;
                };
		struct {
			ui4     type_code;
			si1	type_string_terminal_zero;
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
        si1     recording_city[METADATA_RECORDING_LOCATION_BYTES_m10];                  // utf8[63]
        si1     recording_institution[METADATA_RECORDING_LOCATION_BYTES_m10];           // utf8[63]
        si1     geotag_format[METADATA_GEOTAG_FORMAT_BYTES_m10];                        // ascii[31]
        si1     geotag_data[METADATA_GEOTAG_DATA_BYTES_m10];                            // ascii[1023]
        si4     standard_UTC_offset;
        ui1     protected_region[METADATA_SECTION_3_PROTECTED_REGION_BYTES_m10];
        ui1     discretionary_region[METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m10];
} METADATA_SECTION_3_m10;

typedef struct {
        ui1                                     *metadata;      // same as section_1 pointer (exists for clarity in functions that operate on whole metadata)
        METADATA_SECTION_1_m10		        *section_1;
        TIME_SERIES_METADATA_SECTION_2_m10	*time_series_section_2;
	VIDEO_METADATA_SECTION_2_m10	        *video_section_2;
        METADATA_SECTION_3_m10		        *section_3;
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
			si1	type_string_terminal_zero;
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
			si1	type_string_terminal_zero;
		};
        };
} RECORD_INDEX_m10;

// Time Series Indices Structures
typedef struct {
	si8	file_offset;
	si8	start_time;
        si8     start_sample_number;  // relative to segment sample numbering, negative values indicate disconrtinuity
} TIME_SERIES_INDEX_m10;

// Video Indices Structures
typedef struct {
        si8     file_offset;
	si8	start_time;
        si4     start_frame_number;  // relative to video file, negative values indicate disconrtinuity
        si4     video_file_number;
} VIDEO_INDEX_m10;

// File Processing Structures
typedef struct {
	TERN_m10        close_file;
        TERN_m10        flush_after_write;
        TERN_m10        update_universal_header;	// when writing
        TERN_m10        leave_decrypted;		// if encrypted during write, return from write function decryptedF
	TERN_m10        free_password_data;		// when freeing FPS
        TERN_m10        free_CMP_processing_struct;	// when freeing FPS
        ui4             lock_mode;
	ui4             open_mode;
} FILE_PROCESSING_DIRECTIVES_m10;

typedef struct {
        TERN_m10 _Atomic                        mutex;
	PASSWORD_DATA_m10			*password_data;
	si1                                     full_file_name[FULL_FILE_NAME_BYTES_m10];  // full path including extension
	FILE                                    *fp;    // file pointer
        si4                                     fd;     // file descriptor
	si8                                     file_length;
	UNIVERSAL_HEADER_m10		        *universal_header;
        FILE_PROCESSING_DIRECTIVES_m10	        directives;
	METADATA_m10			        metadata;       // structure containing pointers to each of the three metadata sections
        TIME_SERIES_INDEX_m10                   *time_series_indices;
	VIDEO_INDEX_m10			        *video_indices;
	ui1                                     *records;
        RECORD_INDEX_m10                        *record_indices;
	si8                                     raw_data_bytes;
	ui1                                     *raw_data;
        CMP_PROCESSING_STRUCT_m10               *cps;  // associated with time series data FPS, NULL in others
} FILE_PROCESSING_STRUCT_m10;

// Session, Channel, Segment Processing Structures
typedef struct {
	FILE_PROCESSING_STRUCT_m10      *metadata_fps;  // also used as prototype
	FILE_PROCESSING_STRUCT_m10      *time_series_data_fps;
	FILE_PROCESSING_STRUCT_m10      *time_series_indices_fps;
        FILE_PROCESSING_STRUCT_m10      *video_indices_fps;
	FILE_PROCESSING_STRUCT_m10      *record_data_fps;
	FILE_PROCESSING_STRUCT_m10      *record_indices_fps;
        FILE_PROCESSING_STRUCT_m10      *segmented_session_record_data_fps;
        FILE_PROCESSING_STRUCT_m10      *segmented_session_record_indices_fps;
	si1                             path[FULL_FILE_NAME_BYTES_m10]; // full path to segment directory (including segment directory itself)
        si1                             name[SEGMENT_BASE_FILE_NAME_BYTES_m10];  // stored here, no segment_name field in universal header (for programming convenience)
	TIME_SLICE_m10			time_slice;
} SEGMENT_m10;

typedef struct {
        FILE_PROCESSING_STRUCT_m10      *metadata_fps;
        FILE_PROCESSING_STRUCT_m10	*record_data_fps;
	FILE_PROCESSING_STRUCT_m10	*record_indices_fps;
        si4			        number_of_segments;
	SEGMENT_m10			**segments;
	si1			        path[FULL_FILE_NAME_BYTES_m10]; // full path to channel directory (including channel directory itself)
        si1                             name[BASE_FILE_NAME_BYTES_m10];
	TIME_SLICE_m10			time_slice;
} CHANNEL_m10;

typedef struct {
        FILE_PROCESSING_STRUCT_m10      *time_series_metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
        FILE_PROCESSING_STRUCT_m10      *video_metadata_fps;  // used as prototype or ephemeral file, does not correspond to stored data
        si4                             number_of_segments;
        si4			        number_of_time_series_channels;
	CHANNEL_m10			**time_series_channels;
        si4			        number_of_video_channels;
        CHANNEL_m10			**video_channels;
        FILE_PROCESSING_STRUCT_m10	*record_data_fps;
        FILE_PROCESSING_STRUCT_m10	*record_indices_fps;
        FILE_PROCESSING_STRUCT_m10      **segmented_record_data_fps;
        FILE_PROCESSING_STRUCT_m10      **segmented_record_indices_fps;
	si1			        path[FULL_FILE_NAME_BYTES_m10];     // full path to session directory (including session directory itself)
        si1                             name[BASE_FILE_NAME_BYTES_m10];
	TIME_SLICE_m10			time_slice;
} SESSION_m10;


//**********************************************************************************//
//********************************  MED Prototypes  ********************************//
//**********************************************************************************//

// Alignment Function Prototypes
TERN_m10	check_all_alignments_m10(const si1 *function, si4 line);
TERN_m10	check_metadata_alignment_m10(ui1 *bytes);
TERN_m10	check_metadata_section_1_alignment_m10(ui1 *bytes);
TERN_m10	check_metadata_section_3_alignment_m10(ui1 *bytes);
TERN_m10	check_record_header_alignment_m10(ui1 *bytes);
TERN_m10	check_record_indices_alignment_m10(ui1 *bytes);
TERN_m10	check_CMP_block_header_alignment_m10(ui1 *bytes);
TERN_m10        check_CMP_record_header_alignment_m10(ui1 *bytes);
TERN_m10	check_time_series_indices_alignment_m10(ui1 *bytes);
TERN_m10	check_time_series_metadata_section_2_alignment_m10(ui1 *bytes);
TERN_m10	check_universal_header_alignment_m10(ui1 *bytes);
TERN_m10	check_video_indices_alignment_m10(ui1 *bytes);
TERN_m10	check_video_metadata_section_2_alignment_m10(ui1 *bytes);

// MED Function Prototypes
si8             absolute_index_to_time_m10(si1 *seg_dir, si8 index, si8 absolute_start_sample_number, sf8 sampling_frequency, ui1 mode);
si1             all_zeros_m10(ui1 *bytes, si4 field_length);
CHANNEL_m10	*allocate_channel_m10(CHANNEL_m10 *chan, FILE_PROCESSING_STRUCT_m10 *proto_fps, si1 *enclosing_path, si1 *chan_name, ui4 type_code, si4 n_segs, TERN_m10 chan_recs, TERN_m10 seg_recs);
FILE_PROCESSING_STRUCT_m10	*allocate_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps, si1 *full_file_name, ui4 type_code, si8 raw_data_bytes, FILE_PROCESSING_STRUCT_m10 *proto_fps, si8 bytes_to_copy);
METADATA_m10	*allocate_metadata_m10(METADATA_m10 *metadata, ui1 *data_ptr);
SEGMENT_m10	*allocate_segment_m10(SEGMENT_m10 *seg, FILE_PROCESSING_STRUCT_m10 *proto_fps, si1 *enclosing_path, si1 *chan_name, ui4 type_code, si4 seg_num, TERN_m10 seg_recs);
SESSION_m10	*allocate_session_m10(FILE_PROCESSING_STRUCT_m10 *proto_fps, si1 *enclosing_path, si1 *sess_name, si4 n_ts_chans, si4 n_vid_chans, si4 n_segs, si1 **chan_names, si1 **vid_chan_names, TERN_m10 sess_recs, TERN_m10 segmented_sess_recs, TERN_m10 chan_recs, TERN_m10 seg_recs);
void     	apply_recording_time_offset_m10(si8 *time);
void            calculate_metadata_CRC_m10(FILE_PROCESSING_STRUCT_m10 *fps);
void            calculate_record_data_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_HEADER_m10 *record_header, si8 number_of_items);
void            calculate_record_indices_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_INDEX_m10 *record_index, si8 number_of_items);
void            calculate_time_series_data_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, CMP_BLOCK_FIXED_HEADER_m10 *block_header, si8 number_of_items);
void            calculate_time_series_indices_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, TIME_SERIES_INDEX_m10 *time_series_index, si8 number_of_items);
ui4             channel_type_from_path_m10(si1 *path);
TERN_m10        check_password_m10(si1 *password);
void		condition_time_slice_m10(TIME_SLICE_m10 *slice);
si8		current_uutc_m10(void);
si4		days_in_month_m10(si4 month, si4 year);
TERN_m10        decrypt_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps);
TERN_m10        decrypt_records_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_HEADER_m10 *record_header, si8 number_of_items);
TERN_m10        decrypt_time_series_data_m10(CMP_PROCESSING_STRUCT_m10 *cps, si8 number_of_items);
si4             DST_offset_m10(si8 uutc);
TERN_m10        encrypt_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps);
TERN_m10	encrypt_records_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_HEADER_m10 *record_header, si8 number_of_items);
TERN_m10        encrypt_time_series_data_m10(CMP_PROCESSING_STRUCT_m10 *cps, si8 number_of_items);
void            error_message_m10(si1 *fmt, ...);
void            escape_spaces_m10(si1 *string, si8 buffer_len);
void            extract_path_parts_m10(si1 *full_file_name, si1 *path, si1 *name, si1 *extension);
void            extract_terminal_password_bytes_m10(si1 *password, si1 *password_bytes);
si8 *           find_discontinuities_m10(TIME_SERIES_INDEX_m10 *tsi, si8 *num_disconts, si8 number_of_indices, TERN_m10 remove_offsets, TERN_m10 return_sample_numbers);
ui4             file_exists_m10(si1 *path);
void            force_behavior_m10(ui4 behavior);
void            fps_close_m10(FILE_PROCESSING_STRUCT_m10 *fps);
si4             fps_lock_m10(FILE_PROCESSING_STRUCT_m10 *fps, si4 lock_type, const si1 *function, si4 line, ui4 behavior_on_fail);
void		fps_mutex_off_m10(FILE_PROCESSING_STRUCT_m10 *fps);
void		fps_mutex_on_m10(FILE_PROCESSING_STRUCT_m10 *fps);
si4             fps_open_m10(FILE_PROCESSING_STRUCT_m10 *fps, const si1 *function, si4 line, ui4 behavior_on_fail);
si4             fps_read_m10(FILE_PROCESSING_STRUCT_m10 *fps, si8 in_bytes, void *ptr, const si1 *function, si4 line, ui4 behavior_on_fail);
si4             fps_unlock_m10(FILE_PROCESSING_STRUCT_m10 *fps, const si1 *function, si4 line, ui4 behavior_on_fail);
si4             fps_write_m10(FILE_PROCESSING_STRUCT_m10 *fps, si8 out_bytes, void *ptr, const si1 *function, si4 line, ui4 behavior_on_fail);
si1		*find_timezone_acronym_m10(si1 *timezone_acronym, si4 standard_UTC_offset, si4 DST_offset);
void            free_channel_m10(CHANNEL_m10 *channel, TERN_m10 allocated_en_bloc);
void            free_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps, TERN_m10 allocated_en_bloc);
void            free_globals_m10(void);
void    	free_metadata_m10(METADATA_m10 *metadata);
void            free_segment_m10(SEGMENT_m10 *segment, TERN_m10 allocated_en_bloc);
void            free_session_m10(SESSION_m10 *session);
si1		**generate_file_list_m10(si1 **file_list, si4 n_in_files, si4 *n_out_files, si1 *enclosing_directory, si1 *name, si1 *extension, ui1 path_parts, TERN_m10 free_input_file_list);
si1		*generate_hex_string_m10(ui1 *bytes, si4 num_bytes, si1 *string);
ui4             generate_MED_path_components_m10(si1 *path, si1 *MED_dir, si1 *MED_name);
si1		**generate_numbered_names_m10(si1 **names, si1 *prefix, si4 number_of_names);
si8             generate_recording_time_offset_m10(si8 recording_start_time_uutc);
si1		*generate_segment_name_m10(FILE_PROCESSING_STRUCT_m10 *fps, si1 *segment_name);
ui8             generate_UID_m10(ui8 *uid);
TERN_m10	get_channel_target_values_m10(CHANNEL_m10 *channel, si8 *target_uutc, si8 *target_sample_number, si4 *target_segment_number, ui1 mode);
ui1		get_cpu_endianness_m10(void);
si4             get_segment_range_m10(si1 **channel_list, si4 n_channels, TIME_SLICE_m10 *slice);
void		get_segment_target_values_m10(SEGMENT_m10 *segment, si8 *target_uutc, si8 *target_sample_number, ui1 mode);
TERN_m10	get_session_target_values_m10(SESSION_m10 *session, si8 *target_uutc, si8 *target_sample_number, si4 *target_segment_number, ui1 mode, si1 *idx_ref_chan);
FILE_PROCESSING_DIRECTIVES_m10	*initialize_file_processing_directives_m10(FILE_PROCESSING_DIRECTIVES_m10 *directives);
void            initialize_globals_m10(void);
TERN_m10	initialize_medlib_m10(void);
void            initialize_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps, TERN_m10 initialize_for_update);
TIME_SLICE_m10	*initialize_time_slice_m10(TIME_SLICE_m10 *slice);
void		initialize_timezone_table_m10(void);
void		initialize_universal_header_m10(FILE_PROCESSING_STRUCT_m10 *fps, ui4 type_code, TERN_m10 generate_file_UID, TERN_m10 originating_file);
si1		*MED_type_string_from_code_m10(ui4 code);
ui4             MED_type_code_from_string_m10(si1 *string);
TERN_m10        merge_metadata_m10(FILE_PROCESSING_STRUCT_m10 *md_fps_1, FILE_PROCESSING_STRUCT_m10 *md_fps_2, FILE_PROCESSING_STRUCT_m10 *merged_md_fps);
TERN_m10        merge_universal_headers_m10(FILE_PROCESSING_STRUCT_m10 *fps_1, FILE_PROCESSING_STRUCT_m10 *fps_2, FILE_PROCESSING_STRUCT_m10 *merged_fps);
void    	message_m10(si1 *fmt, ...);
si1		*numerical_fixed_width_string_m10(si1 *string, si4 string_bytes, si4 number);
si8             pad_m10(ui1 *buffer, si8 content_len, ui4 alignment);
TERN_m10	process_password_data_m10(si1 *unspecified_password, si1 *L1_password, si1 *L2_password, si1 *L3_password, si1 *L1_hint, si1 *L2_hint, FILE_PROCESSING_STRUCT_m10 *fps);
CHANNEL_m10	*read_channel_m10(CHANNEL_m10 *chan, si1 *chan_dir, TIME_SLICE_m10 *slice, si1 *password, TERN_m10 read_time_series_data, TERN_m10 read_record_data);
FILE_PROCESSING_STRUCT_m10	*read_file_m10(FILE_PROCESSING_STRUCT_m10 *fps, si1 *full_file_name, si8 number_of_items, ui1 **data_ptr_ptr, si8 *items_read, si1 *password, ui4 behavior_on_fail);
SEGMENT_m10	*read_segment_m10(SEGMENT_m10 *seg, si1 *seg_dir, TIME_SLICE_m10 *slice, si1 *password, TERN_m10 read_time_series_data, TERN_m10 read_record_data);
SESSION_m10	*read_session_m10(si1 *sess_dir, si1 **chan_list, si4 n_chans, TIME_SLICE_m10 *slice, si1 *password, TERN_m10 read_time_series_data, TERN_m10 read_record_data);
si8             read_time_series_data_m10(SEGMENT_m10 *seg, si8 local_start_idx, si8 local_end_idx, TERN_m10 alloc_cps);
si4             reallocate_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps, si8 raw_data_bytes);
TERN_m10	recover_passwords_m10(si1 *L3_password, UNIVERSAL_HEADER_m10 *universal_header);
void     	remove_recording_time_offset_m10(si8 *time);
void            reset_metadata_for_update_m10(FILE_PROCESSING_STRUCT_m10 *fps);
si8             sample_number_for_uutc_m10(si8 ref_sample_number, si8 ref_uutc, si8 target_uutc, sf8 sampling_frequency, FILE_PROCESSING_STRUCT_m10 *time_series_indices_fps, ui1 mode);
TERN_m10        search_segment_metadata_m10(si1 *MED_dir, TIME_SLICE_m10 *slice);
TERN_m10        search_Sgmt_records_m10(si1 *MED_dir, TIME_SLICE_m10 *slice);
void            set_global_time_constants_m10(TIMEZONE_INFO_m10 *timezone_info, si8 session_start_time);
TERN_m10	set_time_and_password_data_m10(si1 *unspecified_password, si1 *MED_directory, si1 *section_2_encryption_level, si1 *section_3_encryption_level);
void            show_daylight_time_change_code_m10(DAYLIGHT_TIME_CHANGE_CODE_m10 *code, si1 *prefix);
void            show_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps);
void            show_globals_m10(void);
void            show_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps, METADATA_m10 *md);
void            show_password_data_m10(PASSWORD_DATA_m10 *pwd);
void            show_records_m10(FILE_PROCESSING_STRUCT_m10 *fps, ui4 type_code);
void    	show_time_slice_m10(TIME_SLICE_m10 *slice);
void            show_timezone_info_m10(TIMEZONE_INFO_m10 *timezone_entry);
void            show_universal_header_m10(FILE_PROCESSING_STRUCT_m10 *fps, UNIVERSAL_HEADER_m10 *uh);
si1		*time_string_m10(si8 uutc_time, si1 *time_str, TERN_m10 fixed_width, TERN_m10 relative_days, si4 colored_text, ...);
si8             ts_sort_m10(si4 *x, si8 len, NODE_m10 *nodes, NODE_m10 *head, NODE_m10 *tail, si4 return_sorted_ts, ...);
void            unescape_spaces_m10(si1 *string);
si8             uutc_for_sample_number_m10(si8 ref_sample_number, si8 ref_uutc, si8 target_sample_number, sf8 sampling_frequency, FILE_PROCESSING_STRUCT_m10 *time_series_indices_fps, ui1 mode);
TERN_m10        validate_record_data_CRCs_m10(RECORD_HEADER_m10 *record_header, si8 number_of_items);
TERN_m10        validate_time_series_data_CRCs_m10(CMP_BLOCK_FIXED_HEADER_m10 *block_header, si8 number_of_items);
void            warning_message_m10(si1 *fmt, ...);
si8             write_file_m10(FILE_PROCESSING_STRUCT_m10 *fps, ui8 number_of_items, void *data_ptr, ui4 behavior_on_fail);


//**********************************************************************************//
//**********************  Error Checking Standard Functions  ***********************//
//**********************************************************************************//

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

// Function Prototypes
void		*e_calloc_m10(ui8 n_members, ui8 el_size, const si1 *function, si4 line, ui4 behavior_on_fail);
void		**e_calloc_2D_m10(ui8 dim1, ui8 dim2, ui8 el_size, const si1 *function, si4 line, ui4 behavior_on_fail);
FILE		*e_fopen_m10(si1 *path, si1 *mode, const si1 *function, si4 line, ui4 behavior_on_fail);
size_t          e_fread_m10(void *ptr, ui8 size, ui8 n_members, FILE *stream, si1 *path, const si1 *function, si4 line, ui4 behavior_on_fail);
void            e_free_m10(void *ptr, const si1 *function, si4 line);
void            e_free_2D_m10(void **ptr, si8 dim1, const si1 *function, si4 line);
si4             e_fseek_m10(FILE *stream, ui8 offset, si4 whence, si1 *path, const si1 *function, si4 line, ui4 behavior_on_fail);
si8             e_ftell_m10(FILE *stream, const si1 *function, si4 line, ui4 behavior_on_fail);
ui8             e_fwrite_m10(void *ptr, ui8 size, ui8 n_members, FILE *stream, si1 *path, const si1 *function, si4 line, ui4 behavior_on_fail);
void		*e_malloc_m10(ui8 n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail);
void		*e_realloc_m10(void *ptr, ui8 n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail);
void		**e_realloc_2D_m10(void **curr_ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail);
si4             e_system_m10(si1 *command, TERN_m10 null_std_streams, const si1 *function, si4 line, ui4 behavior_on_fail);


//**********************************************************************************//
//*****************************  MED String Functions  *****************************//
//**********************************************************************************//

// Standard Function Prototypes
si4     sprintf_m10(si1 *target, si1 *format, ...);
void    snprintf_m10(si1 *target, si4 target_field_bytes, si1 *format, ...);
si4     strcat_m10(si1 *target_string, si1 *source_string);
si4     strcpy_m10(si1 *target_string, si1 *source_string);
void    strncat_m10(si1 *target_string, si1 *source_string, si4 target_field_bytes);
void    strncpy_m10(si1 *target_string, si1 *source_string, si4 target_field_bytes);


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
#define CRC_START_VALUE_m10     ((ui4) 0x00000000)

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
ui4	CRC_calculate_m10(const ui1 *block_ptr, si8 block_bytes);
ui4		CRC_combine_m10(ui4 block_1_crc, ui4 block_2_crc, si8 block_2_bytes);
void		CRC_initialize_table_m10(void);
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
#define UTF8_isutf_m10(c)       (((c) & 0xC0) != 0x80) // true if c is the start of a UTF-8 sequence

#define UTF8_OFFSETS_TABLE_ENTRIES_m10	6
#define UTF8_OFFSETS_TABLE_m10        { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL }

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
si4	UTF8_charnum_m10(si1 *s, si4 offset);  // byte offset to character number
void	UTF8_dec_m10(si1 *s, si4 *i);  // move to previous character
si4	UTF8_escape_m10(si1 *buf, si4 sz, si1 *src, si4 escape_quotes);  // convert UTF-8 "src" to ASCII with escape sequences.
si4	UTF8_escape_wchar_m10(si1 *buf, si4 sz, ui4 ch);  // given a wide character, convert it to an ASCII escape sequence stored in buf, where buf is "sz" bytes. returns the number of characters output
si4	UTF8_fprintf_m10(FILE *stream, si1 *fmt, ...);  // fprintf() where the format string and arguments may be in UTF-8. You can avoid this function and just use ordinary printf() if the current locale is UTF-8.
si4	UTF8_hex_digit_m10(si1 c);  // utility predicates used by the above
void	UTF8_inc_m10(si1 *s, si4 *i);  // move to next character
void	UTF8_initialize_offsets_table_m10(void);
void	UTF8_initialize_trailing_bytes_table_m10(void);
si4	UTF8_is_locale_utf8_m10(si1 *locale);  // boolean function returns if locale is UTF-8, 0 otherwise
si1	*UTF8_memchr_m10(si1 *s, ui4 ch, size_t sz, si4 *charn);  // same as the above, but searches a buffer of a given size instead of a NUL-terminated string.
ui4	UTF8_nextchar_m10(si1 *s, si4 *i);  // return next character, updating an index variable
si4	UTF8_octal_digit_m10(si1 c);  // utility predicates used by the above
si4	UTF8_offset_m10(si1 *str, si4 charnum);  // character number to byte offset
si4	UTF8_printf_m10(si1 *fmt, ...);  // printf() where the format string and arguments may be in UTF-8. You can avoid this function and just use ordinary printf() if the current locale is UTF-8.
si4	UTF8_read_escape_sequence_m10(si1 *str, ui4 *dest);  // assuming str points to the character after a backslash, read an escape sequence, storing the result in dest and returning the number of input characters processed
si4	UTF8_seqlen_m10(si1 *s);  // returns length of next UTF-8 sequence
si1	*UTF8_strchr_m10(si1 *s, ui4 ch, si4 *charn);  // return a pointer to the first occurrence of ch in s, or NULL if not found. character index of found character returned in *charn.
si4	UTF8_strlen_m10(si1 *s);  // count the number of characters in a UTF-8 string
si4	UTF8_toucs_m10(ui4 *dest, si4 sz, si1 *src, si4 srcsz);  // convert UTF-8 data to wide character
si4	UTF8_toutf8_m10(si1 *dest, si4 sz, ui4 *src, si4 srcsz);  // convert wide character to UTF-8 data
si4	UTF8_unescape_m10(si1 *buf, si4 sz, si1 *src);  // convert a string "src" containing escape sequences to UTF-8 if escape_quotes is nonzero, quote characters will be preceded by  backslashes as well.
si4	UTF8_vfprintf_m10(FILE *stream, si1 *fmt, va_list ap);    // called by UTF8_fprintf()
si4	UTF8_vprintf_m10(si1 *fmt, va_list ap);  // called by UTF8_printf()
si4	UTF8_wc_toutf8_m10(si1 *dest, ui4 ch);  // single character to UTF-8



//**********************************************************************************//
//*************************************  AES-128  **********************************//
//**********************************************************************************//


// ATRIBUTION
//
// Advanced Encryption Standard implementation in C.
// By Niyaz PK
// E-mail: niyazlife@gmail.com
// Downloaded from Website: www.hoozi.com
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
#define AES_MULTIPLY_m10(x,y)   (((y & 1) * x) ^ ((y>>1 & 1) * AES_XTIME_m10(x)) ^ ((y>>2 & 1) * AES_XTIME_m10(AES_XTIME_m10(x))) ^ ((y>>3 & 1) * AES_XTIME_m10(AES_XTIME_m10(AES_XTIME_m10(x)))) ^ ((y>>4 & 1) * AES_XTIME_m10(AES_XTIME_m10(AES_XTIME_m10(AES_XTIME_m10(x)))))) // Multiplty is a macro used to multiply numbers in the field GF(2^8)

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
void		AES_initialize_rcon_table_m10(void);
void		AES_initialize_rsbox_table_m10(void);
void		AES_initialize_sbox_table_m10(void);
void		AES_inv_cipher_m10(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key);
void		AES_inv_mix_columns_m10(ui1 state[][4]);
void		AES_inv_shift_rows_m10(ui1 state[][4]);
void		AES_inv_sub_bytes_m10(ui1 state[][4]);
void		AES_mix_columns_m10(ui1 state[][4]);
void		AES_shift_rows_m10(ui1 state[][4]);
void		AES_sub_bytes_m10(ui1 state[][4]);


//**********************************************************************************//
//***********************************  SHA-256  ************************************//
//**********************************************************************************//

// ATTRIBUTION
//
// FIPS 180-2 SHA-224/256/384/512 implementation
// Last update: 02/02/2007
// Issue date:  04/30/2005
//
// Copyright (C) 2005, 2007 Olivier Gay <olivier.gay@a3.epfl.ch>
// All rights reserved.
//
// "Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE."
//
// ONLY SHA-256 FUNCTIONS ARE INCLUDED IN THE MED LIBRARY
//
// Minor modifications for compatibility with the MED Library.


// Constants
#define  SHA_OUTPUT_SIZE_m10	256
#define  SHA_DIGEST_SIZE_m10	(256 / 8)
#define  SHA_BLOCK_SIZE_m10	(512 / 8)

#define  SHA_H0_ENTRIES_m10	8
#define  SHA_H0_m10         {	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, \
                                0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 }

#define  SHA_K_ENTRIES_m10	64
#define  SHA_K_m10          {	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, \
                                0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, \
                                0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, \
                                0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, \
                                0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, \
                                0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, \
                                0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, \
                                0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, \
                                0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, \
                                0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, \
                                0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, \
                                0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, \
                                0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, \
                                0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, \
                                0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, \
                                0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 }

// Macros
#define  SHA_SHFR_m10(x, n)	(x >> n)
#define  SHA_ROTR_m10(x, n)	((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define  SHA_ROTL_m10(x, n)	((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define  SHA_CH_m10(x, y, z)	((x & y) ^ (~x & z))
#define  SHA_MAJ_m10(x, y, z)	((x & y) ^ (x & z) ^ (y & z))

#define  SHA_F1_m10(x)	( SHA_ROTR_m10(x,  2) ^  SHA_ROTR_m10(x, 13) ^  SHA_ROTR_m10(x, 22))
#define  SHA_F2_m10(x)	( SHA_ROTR_m10(x,  6) ^  SHA_ROTR_m10(x, 11) ^  SHA_ROTR_m10(x, 25))
#define  SHA_F3_m10(x)	( SHA_ROTR_m10(x,  7) ^  SHA_ROTR_m10(x, 18) ^  SHA_SHFR_m10(x,  3))
#define  SHA_F4_m10(x)	( SHA_ROTR_m10(x, 17) ^  SHA_ROTR_m10(x, 19) ^  SHA_SHFR_m10(x, 10))

#define  SHA_UNPACK32_m10(x, str)       { *((str) + 3) = (ui1) (x); *((str) + 2) = (ui1) ((x) >>  8); *((str) + 1) = (ui1) ((x) >> 16); *((str) + 0) = (ui1) ((x) >> 24); }

#define  SHA_PACK32_m10(str, x)         { *(x) = ((ui4) *((str) + 3)) | ((ui4) *((str) + 2) <<  8) | ((ui4) *((str) + 1) << 16) | ((ui4) *((str) + 0) << 24); }

#define  SHA_SCR_m10(i)                 {  w[i] =   SHA_F4_m10(w[i -  2]) + w[i -  7] +  SHA_F3_m10(w[i - 15]) + w[i - 16]; }

#define  SHA_EXP_m10(a, b, c, d, e, f, g, h, j)         { t1 = wv[h] +  SHA_F2_m10(wv[e]) + SHA_CH_m10(wv[e], wv[f], wv[g]) + globals_m10->SHA_k_table[j] + w[j]; t2 =  SHA_F1_m10(wv[a]) + SHA_MAJ_m10(wv[a], wv[b], wv[c]); wv[d] += t1; wv[h] = t1 + t2; }

// Typedefs & Structures
typedef struct {
	ui4	tot_len;
	ui4	len;
	ui1	block[2 *  SHA_BLOCK_SIZE_m10];
	ui4	h[8];
}  SHA_CTX_m10;

// Function Prototypes
void    SHA_sha_m10(const ui1 *message, ui4 len, ui1 *digest);
void    SHA_final_m10(SHA_CTX_m10 *ctx, ui1 *digest);
void    SHA_init_m10(SHA_CTX_m10 *ctx);
void	SHA_initialize_h0_table_m10(void);
void	SHA_initialize_k_table_m10(void);
void    SHA_transf_m10(SHA_CTX_m10 *ctx, const ui1 *message, ui4 block_nb);
void    SHA_update_m10(SHA_CTX_m10 *ctx, const ui1 *message, ui4 len);


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


#define TIMEZONE_TABLE_ENTRIES_m10      400
#define TIMEZONE_TABLE_m10 { \
        { "Afghanistan", "AF", "AFG", "", "", "Afghanistan Time", "AFT", +16200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Akrotiri", "", "", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 01:00 Local", 0x3c00010200060001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Aland Islands", "AX", "ALA", "", "", "Eastern European Time", "EET", +7200,1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 03:00 Local", 0x3c00030200060001,"Last Sunday of October at 04:00 Local", 0xc4000409000600ff }, \
        { "Albania", "AL", "ALB", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Algeria", "DZ", "DZA", "", "", "Central European Time", "CET", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "American Samoa", "US", "ASM", "", "", "Samoa Standard Time", "SST", -39600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Andorra", "AD", "AND", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Angola", "AO", "AGO", "", "", "West Africa Time", "WAT", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Anguilla", "AI", "AIA", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Antigua", "AG", "ATG", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Argentina", "AR", "ARG", "", "", "Argentina Time", "ART", -10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Armenia", "AM", "ARM", "", "", "Armenia Time", "AMT", +14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Aruba", "AW", "ABW", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Ascension", "SH", "SHN", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Australia", "AU", "AUS", "Western Australia", "WA", "Australian Western Standard Time", "AWST", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Australia", "AU", "AUS", "Western Australia", "WA", "Australian Central Western Standard Time", "ACWST", +31500, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Australia", "AU", "AUS", "South Australia", "SA", "Australian Central Standard Time", "ACST", +34200, 1, "Australian Central Daylight Time", "ACDT", "First Sunday of October at 02:00 Local", 0x3c00020900010001, "First Sunday of April at 03:00 Local", 0xc4000303000100ff }, \
        { "Australia", "AU", "AUS", "Northern Territory", "NT", "Australian Central Standard Time", "ACST", +34200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Australia", "AU", "AUS", "Australian Capital Territory", "ACT", "Australian Eastern Standard Time", "AEST", +36000, 1, "Australian Eastern Daylight Time", "AEDT", "First Sunday of October at 02:00 Local", 0x3c00020900010001, "First Sunday of April at 03:00 Local", 0xc4000303000100ff }, \
        { "Australia", "AU", "AUS", "Tasmania", "Tas", "Australian Eastern Standard Time", "AEST", +36000, 1, "Australian Eastern Daylight Time", "AEDT", "First Sunday of October at 02:00 Local", 0x3c00020900010001, "First Sunday of April at 03:00 Local", 0xc4000303000100ff }, \
        { "Australia", "AU", "AUS", "Victoria", "Vic", "Australian Eastern Standard Time", "AEST", +36000, 1, "Australian Eastern Daylight Time", "AEDT", "First Sunday of October at 02:00 Local", 0x3c00020900010001, "First Sunday of April at 03:00 Local", 0xc4000303000100ff }, \
        { "Australia", "AU", "AUS", "New South Wales", "NSW", "Australian Eastern Standard Time", "AEST", +36000, 1, "Australian Eastern Daylight Time", "AEDT", "First Sunday of October at 02:00 Local", 0x3c00020900010001, "First Sunday of April at 03:00 Local", 0xc4000303000100ff }, \
        { "Australia", "AU", "AUS", "Queensland", "Qld", "Australian Eastern Standard Time", "AEST", +36000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Australia", "AU", "AUS", "Lord Howe Island", "", "Lord Howe Standard Time", "LHST", +37800, 1, "Lord Howe Daylight Time (+30 min)", "LHDT", "First Sunday of October at 02:00 Local", 0x1e00020900010001, "First Sunday of April at 02:00 Local", 0xe2000203000100ff }, \
        { "Austria", "AT", "AUT", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Azerbaijan", "AZ", "AZE", "", "", "Azerbaijan Time", "AZT", +14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Bahamas", "BS", "BHS", "", "", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Bahrain", "BH", "BHR", "", "", "Arabian Standard Time", "AST", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Bangladesh", "BD", "BGD", "", "", "Bangladesh Standard Time", "BST", +21600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Barbados", "BB", "BRB", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Barbuda", "AG", "ATG", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Belarus", "BY", "BLR", "", "", "Moscow Standard Time", "MSK", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Belgium", "BE", "BEL", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Belize", "BZ", "BLZ", "", "", "Central Standard Time", "CST", -21600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Benin", "BJ", "BEN", "", "", "West Africa Time", "WAT", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Bermuda", "BM", "BMU", "", "", "Atlantic Standard Time", "AST", -14400, 1, "Atlantic Daylight Time", "ADT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Bhutan", "BT", "BTN", "", "", "Bhutan Time", "BTT", +21600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Bolivia", "BO", "BOL", "", "", "Bolivia Time", "BOT", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Bonaire", "BQ", "BES", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Bosnia", "BA", "BIH", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Botswana", "BW", "BWA", "", "", "Central Africa Time", "CAT", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Bouvet Island", "BV", "BVT", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Brazil", "BR", "BRA", "", "", "Acre Time", "ACT", -18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Brazil", "BR", "BRA", "", "", "Amazon Time", "AMT", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Brazil", "BR", "BRA", "", "", "Brasilia Time", "BRT", -10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Brazil", "BR", "BRA", "", "", "Fernando de Noronha Time", "FNT", -7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "British Virgin Islands", "VG", "VGB", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Brunei", "BN", "BRN", "", "", "Brunei Time", "BNT", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Bulgaria", "BG", "BGR", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 03:00 Local", 0x3c00030200060001, "Last Sunday of October at 04:00 Local", 0xc4000409000600ff }, \
        { "Burkina Faso", "BF", "BFA", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Burundi", "BI", "BDI", "", "", "Central Africa Time", "CAT", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Caicos", "TC", "TCA", "", "", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Cambodia", "KH", "KHM", "", "", "Indochina Time", "ICT", +25200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Cameroon", "CM", "CMR", "", "", "West Africa Time", "WAT", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Canada", "CA", "CAN", "Newfoundland", "NL", "Newfoundland Standard Time", "NST", -12600, 1, "Newfoundland Daylight Time", "NDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "Labrador", "NL", "Atlantic Standard Time", "AST", -14400, 1, "Atlantic Daylight Time", "ADT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "New Brunswick", "NB", "Atlantic Standard Time", "AST", -14400, 1, "Atlantic Daylight Time", "ADT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "Nova Scotia", "NS", "Atlantic Standard Time", "AST", -14400, 1, "Atlantic Daylight Time", "ADT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "Prince Edward Island", "PE", "Atlantic Standard Time", "AST", -14400, 1, "Atlantic Daylight Time", "ADT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "Ontario", "ON", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "Quebec", "QC", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "Manitoba", "MB", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "Saskatchewan", "SK", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "Nunavut", "NU", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "Alberta", "AB", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "Northwest Territories ", "NT", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "British Columbia", "BC", "Pacific Standard Time", "PST", -28800, 1, "Pacific Daylight Time", "PDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Canada", "CA", "CAN", "Yukon", "YT", "Pacific Standard Time", "PST", -28800, 1, "Pacific Daylight Time", "PDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Cape Verde", "CV", "CPV", "", "", "Cape Verde Time", "CVT", -3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Cayman Islands", "KY", "CYM", "", "", "Eastern Standard Time", "EST", -18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Central African Republic", "CF", "CAF", "", "", "West Africa Time", "WAT", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Chad", "TD", "TCD", "", "", "West Africa Time", "WAT", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Chile", "CL", "CHL", "", "", "Chile Standard Time", "CLT", -14400, 1, "Chile Summer Time", "CLST", "First Sunday of September at 00:00 Local", 0x3c00000800010001, "First Sunday of April at 00:00 Local", 0xc4000003000100ff }, \
        { "Chile", "CL", "CHL", "Easter Island", "", "Easter Island Standard Time", "EAST", -21600, 1, "Easter Island Summer Time", "EASST", "First Sunday of September at 00:00 Local", 0x3c00000800010001, "First Sunday of April at 00:00 Local", 0xc4000003000100ff }, \
        { "China", "CN", "CHN", "", "", "China Standard Time", "CST", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Christmas Island (AU)", "CX", "CXR", "", "", "Christmas Island Time", "CXT", +25200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Cocos Islands (AU)", "CC", "CCK", "", "", "Cocos Island Time", "CCT", +23400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Colombia", "CO", "COL", "", "", "Colombia Time", "COT", -18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Comoros", "KM", "COM", "", "", "Eastern Africa Time", "EAT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Cook Islands", "CK", "COK", "", "", "Cook Island time", "CKT", -36000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Costa Rica", "CR", "CRI", "", "", "Central Standard Time", "CST", -21600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Croatia", "HR", "HRV", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Cuba", "CU", "CUB", "", "", "Cuba Standard Time", "CST", -18000, 1, "Cuba Daylight Time", "CDT", "Second Sunday of March at 00:00 Local", 0x3c00000200020001, "First Sunday of November at 01:00 Local", 0xc400010a000100ff }, \
        { "Curacao", "CW", "CUW", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Cyprus", "CY", "CYP", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 03:00 Local", 0x3c00030200060001, "Last Sunday of October at 04:00 Local", 0xc4000409000600ff }, \
        { "Czech Republic", "CZ", "CZE", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Democratic Republic of the Congo", "CD", "COD", "", "", "West Africa Time", "WAT", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Democratic Republic of the Congo", "CD", "COD", "", "", "Central Africa Time", "CAT", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Denmark", "DK", "DNK", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Dhekelia", "", "", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 01:00 Local", 0x3c00010200060001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Djibouti", "DJ", "DJI", "", "", "Eastern Africa Time", "EAT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Dominica", "DM", "DMA", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Dominican Republic", "DO", "DOM", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "East Timor", "TL", "TLS", "", "", "East Timor Time", "TLT", +32400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Ecuador", "EC", "ECU", "", "", "Ecuador Time", "ECT", -18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Egypt", "EG", "EGY", "", "", "Eastern European Time", "EET", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "El Salvador", "SV", "SLV", "", "", "Central Standard Time", "CST", -21600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Equatorial Guinea", "GQ", "GNQ", "", "", "West Africa Time", "WAT", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Eritrea", "ER", "ERI", "", "", "Eastern Africa Time", "EAT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Estonia", "EE", "EST", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 03:00 Local", 0x3c00030200060001, "Last Sunday of October at 04:00 Local", 0xc4000409000600ff }, \
        { "Eswatini", "SZ", "SWZ", "", "", "South Africa Standard Time", "SAST", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Ethiopia", "ET", "ETH", "", "", "Eastern Africa Time", "EAT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Falkland Islands", "FK", "FLK", "", "", "Falkland Islands Summer Time", "FKST", -10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Faroe Islands", "FO", "FRO", "", "", "Western European Time", "WET", 0, 1, "Western European Daylight Time", "WEDT", "Last Sunday of March at 01:00 Local", 0x3c00010200060001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Fiji", "FJ", "FJI", "", "", "Fiji Time", "FJT", +43200, 1, "Fiji Daylight Time", "FJDT", "First Sunday of November at 02:00 Local", 0x3c00020a00010001, "Third Sunday of January at 03:00 Local", 0xc4000300000300ff }, \
        { "Finland", "FI", "FIN", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 03:00 Local", 0x3c00030200060001, "Last Sunday of October at 04:00 Local", 0xc4000409000600ff }, \
        { "France", "FR", "FRA", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "French Guiana", "GF", "GUF", "", "", "French Guiana Time", "GFT", -10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "French Polynesia", "PF", "PYF", "Tahiti", "", "Tahiti Time", "TAHT", -36000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "French Polynesia", "PF", "PYF", "Marquesas", "", "Marquesas Time", "MART", -34200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "French Polynesia", "PF", "PYF", "Gambier", "", "Gambier Time", "GAMT", -32400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "French Southern Territories", "TF", "ATF", "", "", "French Southern and Antarctic Time", "TFT", +18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Futuna", "WF", "WLF", "", "", "Wallis and Futuna Time", "WFT", +43200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Gabon", "GA", "GAB", "", "", "West Africa Time", "WAT", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Gambia", "GM", "GMB", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Georgia", "GE", "GEO", "", "", "Georgia Standard Time", "GET", +14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Georgia", "GE", "GEO", "", "", "Moscow Standard Time", "MSK", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Germany", "DE", "DEU", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Ghana", "GH", "GHA", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Gibraltar", "GI", "GIB", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Greece", "GR", "GRC", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 03:00 Local", 0x3c00030200060001, "Last Sunday of October at 04:00 Local", 0xc4000409000600ff }, \
        { "Greenland", "GL", "GRL", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Greenland", "GL", "GRL", "", "", "Eastern Greenland Time", "EGT", -3600, 1, "Eastern Greenland Summer Time", "EGST", "Saturday before last Sunday of March at 22:00 Local", 0x3c00fe0200060001, "Saturday before last Sunday of October at 23:00 Local", 0xc400ff09000600ff }, \
        { "Greenland", "GL", "GRL", "", "", "Western Greenland Time", "WGT", -10800, 1, "Western Greenland Summer Time", "WGST", "Saturday before last Sunday of March at 22:00 Local", 0x3c00fe0200060001, "Saturday before last Sunday of October at 23:00 Local", 0xc400ff09000600ff }, \
        { "Greenland", "GL", "GRL", "", "", "Atlantic Standard Time", "AST", -14400, 1, "Atlantic Daylight Time", "ADT", "Saturday before last Sunday of March at 22:00 Local", 0x3c00fe0200060001, "Saturday before last Sunday of October at 23:00 Local", 0xc400ff09000600ff }, \
        { "Grenada", "GD", "GRD", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Grenadines", "VC", "VCT", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Guadeloupe", "GP", "GLP", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Guam", "GU", "GUM", "", "", "Chamorro Standard Time", "ChST", +36000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Guatemala", "GT", "GTM", "", "", "French Guiana Time", "GFT", -10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Guernsey", "GG", "GGY", "", "", "Greenwich Mean Time", "GMT", 0, 1, "British Summer Time", "BST", "Last Sunday of March at 01:00 Local", 0x3c00010200060001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Guinea", "GN", "GIN", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Guinea-Bissau", "GW", "GNB", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Guyana", "GY", "GUY", "", "", "Guyana Time", "GYT", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Haiti", "HT", "HTI", "", "", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Heard Islands", "HM", "HMD", "", "", "French Southern and Antarctic Time", "TFT", +18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Herzegovina", "BA", "BIH", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Holy See", "VA", "VAT", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Honduras", "HN", "HND", "", "", "Central Standard Time", "CST", -21600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Hong Kong", "HK", "HKG", "", "", "Hong Kong Time", "HKT", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Hungary", "HU", "HUN", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Iceland", "IS", "ISL", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "India", "IN", "IND", "", "", "India Time Zone", "IST", +19800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Indonesia", "ID", "IDN", "", "", "Western Indonesian Time", "WIB", +25200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Indonesia", "ID", "IDN", "", "", "Central Indonesian Time", "WITA", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Indonesia", "ID", "IDN", "", "", "Eastern Indonesian Time", "WIT", +32400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Iran", "IR", "IRN", "", "", "Iran Standard Time", "IRST", +12600, 1, "Iran Daylight Time", "IRDT", "March 22 at 00:00 Local", 0x3c0000021600ff01, "September 22 at 00:00 Local", 0xc40000081600ffff }, \
        { "Iraq", "IQ", "IRQ", "", "", "Arabia Standard Time", "AST", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Ireland", "IE", "IRL", "", "", "Greenwich Mean Time", "GMT", 0, 1, "Irish Standard Time", "IST", "Last Sunday of March at 01:00 Local", 0x3c00010200060001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Isle of Man", "IM", "IMN", "", "", "Greenwich Mean Time", "GMT", 0, 1, "British Summer Time", "BST", "Last Sunday of March at 01:00 Local", 0x3c00010200060001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Israel", "IL", "ISR", "", "", "Israel Standard Time", "IST", +7200, 1, "Israel Daylight Time", "IDT", "Friday before last Sunday of March at 02:00 Local", 0x3c00d20200060001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Italy", "IT", "ITA", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Ivory Coast", "CI", "CIV", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Jamaica", "JM", "JAM", "", "", "Eastern Standard Time", "EST", -18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Jan Mayen", "SJ", "SJM", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Japan", "JP", "JPN", "", "", "Japan Standard Time", "JST", +32400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Jersey", "JE", "JEY", "", "", "Greenwich Mean Time", "GMT", 0, 1, "British Summer Time", "BST", "Last Sunday of March at 01:00 Local", 0x3c00010200060001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Jordan", "JO", "JOR", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Friday of March at 00:00 Local", 0x3c00000200060501, "Last Friday of October at 01:00 Local", 0xc4000109000605ff }, \
        { "Kazakhstan", "KZ", "KAZ", "", "", "Oral Time", "ORAT", +18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Kazakhstan", "KZ", "KAZ", "", "", "Alma-Ata Time", "ALMT", +21600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Keeling Islands", "CC", "CCK", "", "", "Cocos Islands Time", "CCT", +23400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Kenya", "KE", "KEN", "", "", "Eastern Africa Time", "EAT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Kiribati", "KI", "KIR", "", "", "Gilbert Island Time", "GILT", +43200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Kiribati", "KI", "KIR", "", "", "Phoenix Island Time", "PHOT", +46800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Kiribati", "KI", "KIR", "", "", "Line Islands Time", "LINT", +50400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Kosovo", "XK", "XKX", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Kuwait", "KW", "KWT", "", "", "Arabia Standard Time", "AST", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Kyrgyzstan", "KG", "KGZ", "", "", "Kyrgyzstan Time", "KGT", +21600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Laos", "LA", "LAO", "", "", "Indochina Time", "ICT", +25200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Latvia", "LV", "LVA", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 03:00 Local", 0x3c00030200060001, "Last Sunday of October at 04:00 Local", 0xc4000409000600ff }, \
        { "Lebanon", "LB", "LBN", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 00:00 Local", 0x3c00000200060001, "Last Sunday of October at 00:00 Local", 0xc4000009000600ff }, \
        { "Lesotho", "LS", "LSO", "", "", "South Africa Standard Time", "SAST", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Liberia", "LR", "LBR", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Libya", "LY", "LBY", "", "", "Eastern European Time", "EET", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Liechtenstein", "LI", "LIE", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Lithuania", "LT", "LTU", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 03:00 Local", 0x3c00030200060001, "Last Sunday of October at 04:00 Local", 0xc4000409000600ff }, \
        { "Luxembourg", "LU", "LUX", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Macau", "MO", "MAC", "", "", "China Standard Time", "CST", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Madagascar", "MG", "MDG", "", "", "Eastern Africa Time", "EAT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Malawi", "MW", "MWI", "", "", "Central Africa Time", "CAT", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Malaysia", "MY", "MYS", "", "", "Malaysia Time", "MYT", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Maldives", "MV", "MDV", "", "", "Maldives Time", "MVT", +18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Mali", "ML", "MLI", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Malta", "MT", "MLT", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Marshall Islands", "MH", "MHL", "", "", "Marshall Islands Time", "MHT", +43200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Martinique", "MQ", "MTQ", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Mauritania", "MR", "MRT", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Mauritius", "MU", "MUS", "", "", "Mauritius Time", "MUT", +14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Mayotte", "YT", "MYT", "", "", "Eastern Africa Time", "EAT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "McDonald Islands", "US", "USA", "", "", "Alaska Standard Time", "AKST", +32400, 1, "Alaska Daylight Time", "AKDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "McDonald Islands", "HM", "HMD", "", "", "French Southern and Antarctic Time", "TFT", +18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Mexico", "MX", "MEX", "", "", "Eastern Standard Time", "EST", -18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Mexico", "MX", "MEX", "", "", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "First Sunday of April at 02:00 Local", 0x3c00020300010001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Mexico", "MX", "MEX", "", "", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "First Sunday of April at 02:00 Local", 0x3c00020300010001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Mexico", "MX", "MEX", "", "", "Pacific Standard Time", "PST", -28800, 1, "Pacific Daylight Time", "PDT", "First Sunday of April at 02:00 Local", 0x3c00020300010001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Micronesia", "FM", "FSM", "", "", "Chuuk Time", "CHUT", +36000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Micronesia", "FM", "FSM", "", "", "Pohnpei Standard Time", "PONT", +39600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Micronesia", "FM", "FSM", "", "", "Kosrae Time", "KOST", +39600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Midway", "UM", "UMI", "", "", "Samoa Standard Time", "SST", -39600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Miquelon", "PM", "SPM", "", "", "Pierre & Miquelon Standard Time", "PMST", -10800, 1, "Pierre & Miquelon Daylight Time", "PMDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Moldova", "MD", "MDA", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Monaco", "MC", "MCO", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Mongolia", "MN", "MNG", "", "", "Hovd Time", "HOVT", +25200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Mongolia", "MN", "MNG", "", "", "Ulaanbaatar Time", "ULAT", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Mongolia", "MN", "MNG", "", "", "Choibalsan Time", "CHOT", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Montenegro", "ME", "MNE", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Montserrat", "MS", "MSR", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Morocco", "MA", "MAR", "", "", "Western European Time", "WET", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Mozambique", "MZ", "MOZ", "", "", "Central Africa Time", "CAT", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Myanmar", "MM", "MMR", "", "", "Myanmar Time", "MMT", +23400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Namibia", "NA", "NAM", "", "", "Central Africa Time", "CAT", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Nauru", "NR", "NRU", "", "", "Nauru Time", "NRT", +43200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Nepal", "NP", "NPL", "", "", "Nepal Time", "NPT", +20700, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Netherlands", "NL", "NLD", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Netherlands Antilles", "AN", "ANT", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Nevis", "KN", "KNA", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "New Caledonia", "NC", "NCL", "", "", "New Caledonia Time", "NCT", +39600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "New Zealand", "NZ", "NZL", "", "", "New Zealand Standard Time", "NZST", +43200, 1, "New Zealand Daylight Time", "NZDT", "Last Sunday of September at 02:00 Local", 0x3c00020800060001, "First Sunday of April at 03:00 Local", 0xc4000303000100ff }, \
        { "New Zealand", "NZ", "NZL", "Chatham Island", "", "Chatham Island Standard Time", "CHAST", +45900, 1, "Chatham Island Daylight Time", "CHADT", "Last Sunday of September at 02:00 Local", 0x3c00020800060001, "First Sunday of April at 03:00 Local", 0xc4000303000100ff }, \
        { "Nicaragua", "NI", "NIC", "", "", "Central Standard Time", "CST", -21600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Niger", "NE", "NER", "", "", "West Africa Time", "WAT", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Nigeria", "NG", "NGA", "", "", "West Africa Time", "WAT", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Niue", "NU", "NIU", "", "", "Niue Time", "NUT", -39600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Norfolk Island", "NF", "NFK", "", "", "Norfolk Time", "NFT", +39600, 1, "Norfolk Daylight Time", "NFDT", "First Sunday of October at 02:00 Local", 0x3c00020900010001, "First Sunday of April at 03:00 Local", 0xc4000303000100ff }, \
        { "North Korea", "KP", "PRK", "", "", "Korea Standard Time", "KST", +32400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "North Macedonia", "MK", "MKD", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Northern Mariana Islands", "MP", "MNP", "", "", "Chamorro Standard Time", "ChST", +36000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Norway", "NO", "NOR", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Oman", "OM", "OMN", "", "", "Gulf Standard Time", "GST", +14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Pakistan", "PK", "PAK", "", "", "Pakistan Standard Time", "PKT", +18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Palau", "PW", "PLW", "", "", "Palau Time", "PWT", +32400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Palestine", "PS", "PSE", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Friday before last Sunday of March at 02:00 Local", 0x3c00d20200060001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Panama", "PA", "PAN", "", "", "Eastern Standard Time", "EST", -18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Papua New Guinea", "PG", "PNG", "", "", "Papua New Guinea Time", "PGT", +36000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Papua New Guinea", "PG", "PNG", "Bougainville", "", "Bougainville Standard Time", "BST", +39600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Paraguay", "PY", "PRY", "", "", "Paraguay Time", "PYT", -14400, 1, "Paraguay Summer Time", "PYST", "First Sunday of October at 00:00 Local", 0x3c00000900010001, "Fourth Sunday of March at 00:00 Local", 0xc4000002000400ff }, \
        { "Peru", "PE", "PER", "", "", "Peru Time", "PET", -18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Philippines", "PH", "PHL", "", "", "Philippine Time", "PHT", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Pitcairn Islands", "PN", "PCN", "", "", "Pitcairn Standard Time", "PST", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Poland", "PL", "POL", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Portugal", "PT", "PRT", "", "", "Western European Time", "WET", 0, 1, "Western European Daylight Time", "WEDT", "Last Sunday of March at 01:00 Local", 0x3c00010200060001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "Portugal", "PT", "PRT", "", "", "Azores Time", "AZOT", +3600, 1, "Azores Summer Time", "AZOST", "Last Sunday of March at 00:00 Local", 0x3c00000200060001, "Last Sunday of October at 01:00 Local", 0xc4000109000600ff }, \
        { "Principe", "ST", "STP", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Puerto Rico", "PR", "PRI", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Qatar", "QA", "QAT", "", "", "Arabia Standard Time", "AST", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Republic of the Congo", "CG", "COG", "", "", "West Africa Time", "WAT", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Reunion", "RE", "REU", "", "", "Reunion Time", "RET", +14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Romania", "RO", "ROU", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 03:00 Local", 0x3c00030200060001, "Last Sunday of October at 04:00 Local", 0xc4000409000600ff }, \
        { "Russia", "RU", "RUS", "Kaliningrad", "", "Eastern European Time", "EET", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Moscow", "", "Moscow Standard Time", "MSK", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Samara", "", "Samara Time", "SAMT", +14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Yekaterinburg", "", "Yekaterinburg Time", "YEKT", +18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Omsk", "", "Omsk Standard Time", "OMST", +21600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Krasnoyarsk", "", "Krasnoyarsk Time", "KRAT", +25200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Novosibirsk", "", "Novosibirsk Time", "NOVT", +25200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Irkutsk", "", "Irkutsk Time", "IRKT", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Yakutsk", "", "Yakutsk Time", "YAKT", +32400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Vladivostok", "", "Vladivostok Time", "VLAT", +36000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Magadan", "", "Magadan Time", "MAGT", +39600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Sakhalin", "", "Sakhalin Time", "SAKT", +39600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Srednekolymsk", "", "Srednekolymsk Time", "SRED", +39600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Anadyr", "", "Anadyr Time", "ANAT", +43200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Russia", "RU", "RUS", "Kamchatka", "", "Kamchatka Time", "PETT", +43200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Rwanda", "RW", "RWA", "", "", "Central Africa Time", "CAT", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Saba", "BQ", "BES", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Saint Barthelemy", "BL", "BLM", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Saint Helena", "SH", "SHN", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Saint Kitts", "KN", "KNA", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Saint Lucia", "LC", "LCA", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Saint Martin", "MF", "MAF", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Saint Pierre", "PM", "SPM", "", "", "Pierre & Miquelon Standard Time", "PMST", -10800, 1, "Pierre & Miquelon Daylight Time", "PMDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Saint Vincent", "VC", "VCT", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Samoa", "WS", "WSM", "", "", "West Samoa Time", "WST", +46800, 1, "West Samoa Time", "WST", "Last Sunday of September at 03:00 Local", 0x3c00030800060001, "First Sunday of April at 04:00 Local", 0xc4000403000100ff }, \
        { "San Marino", "SM", "SMR", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Sao Tome", "ST", "STP", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Saudi Arabia", "SA", "SAU", "", "", "Arabia Standard Time", "AST", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Senegal", "SN", "SEN", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Serbia", "RS", "SRB", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Seychelles", "SC", "SYC", "", "", "Seychelles Time", "SCT", +14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Sierra Leone", "SL", "SLE", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Singapore", "SG", "SGP", "", "", "Singapore Time", "SGT", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Sint Eustatius", "BQ", "BES", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Sint Maarten", "SX", "SXM", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Slovakia", "SK", "SVK", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Slovenia", "SI", "SVN", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Solomon Islands", "SB", "SLB", "", "", "Solomon Islands Time", "SBT", +39600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Somalia", "SO", "SOM", "", "", "Eastern Africa Time", "EAT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "South Africa", "ZA", "ZAF", "", "", "South Africa Standard Time", "SAST", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "South Africa", "ZA", "ZAF", "Marion Island", "", "Eastern Africa Time", "EAT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "South Georgia Island", "GS", "SGS", "", "", "South Georgia Time", "GST", -7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "South Korea", "KR", "KOR", "", "", "Korea Standard Time", "KST", +32400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "South Sandwich Islands", "GS", "SGS", "", "", "South Georgia Time", "GST", -7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "South Sudan", "SS", "SSD", "", "", "Eastern Africa Time", "EAT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Spain", "ES", "ESP", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Spain", "ES", "ESP", "", "", "Western European Time", "WET", 0, 1, "Western European Daylight Time", "WEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Sri Lanka", "LK", "LKA", "", "", "India Standard Time", "IST", +19800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Sudan", "SD", "SDN", "", "", "Central Africa Time", "CAT", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Suriname", "SR", "SUR", "", "", "Suriname Time", "SRT", -10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Svalbard", "SJ", "SJM", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Swaziland", "SZ", "SWZ", "", "", "South Africa Standard Time", "SAST", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Sweden", "SE", "SWE", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Switzerland", "CH", "CHE", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Syria", "SY", "SYR", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Friday of March at 00:00 Local", 0x3c00000200060501, "Last Friday of October at 00:00 Local", 0xc4000009000605ff }, \
        { "Taiwan", "TW", "TWN", "", "", "China Standard Time", "CST", +28800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Tajikistan", "TJ", "TJK", "", "", "Tajikistan Time", "TJT", +18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Tanzania", "TZ", "TZA", "", "", "Eastern Africa Time", "EAT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Thailand", "TH", "THA", "", "", "Indochina Time", "ICT", +25200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Tobago", "TT", "TTO", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Togo", "TG", "TGO", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Tokelau", "TK", "TKL", "", "", "Tokelau Time", "TKT", +46800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Tonga", "TO", "TON", "", "", "Tonga Time", "TOT", +46800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Trinidad", "TT", "TTO", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Tristan da Cunha", "SH", "SHN", "", "", "Greenwich Mean Time", "GMT", 0, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Tunisia", "TN", "TUN", "", "", "Central European Time", "CET", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Turkey", "TR", "TUR", "", "", "Turkey Time", "TRT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Turkmenistan", "TM", "TKM", "", "", "Turkmenistan Time", "TMT", +18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Turks", "TC", "TCA", "", "", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "Tuvalu", "TV", "TUV", "", "", "Tuvalu Time", "TVT", +43200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Uganda", "UG", "UGA", "", "", "Eastern Africa Time", "EAT", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Ukraine", "UA", "UKR", "", "", "Eastern European Time", "EET", +7200, 1, "Eastern European Daylight Time", "EEDT", "Last Sunday of March at 03:00 Local", 0x3c00030200060001, "Last Sunday of October at 04:00 Local", 0xc4000409000600ff }, \
        { "Ukraine", "UA", "UKR", "", "", "Moscow Standard Time", "MSK", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "United Arab Emirates", "AE", "ARE", "", "", "Gulf Standard Time", "GST", +14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "United Kingdom", "GB", "GBR", "", "", "Greenwich Mean Time", "GMT", 0, 1, "British Summer Time", "BST", "Last Sunday of March at 01:00 Local", 0x3c00010200060001, "Last Sunday of October at 02:00 Local", 0xc4000209000600ff }, \
        { "United States", "US", "USA", "Alabama", "AL", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Alaska", "AK", "Alaska Standard Time", "AKST", -32400, 1, "Alaska Daylight Time", "AKDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Alaska", "AK", "Hawaii-Aleutian Standard Time", "HST", -36000, 1, "Hawaii-Aleutian Daylight Time", "HDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Arizona", "AZ", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Arkansas", "AR", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "California", "CA", "Pacific Standard Time", "PST", -28800, 1, "Pacific Daylight Time", "PDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Colorado", "CO", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Connecticut", "CT", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Delaware", "DE", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "District of Columbia", "DC", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Florida", "FL", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Florida", "FL", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Georgia", "GA", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Hawaii", "HI", "Hawaii-Aleutian Standard Time", "HST", -36000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "United States", "US", "USA", "Idaho", "ID", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Idaho", "ID", "Pacific Standard Time", "PST", -28800, 1, "Pacific Daylight Time", "PDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Illinois", "IL", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Indiana", "IN", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Indiana", "IN", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Iowa", "IA", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Kansas", "KS", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Kansas", "KS", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Kentucky", "KY", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Kentucky", "KY", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Louisiana", "LA", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Maine", "ME", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Maryland", "MD", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Massachusetts", "MA", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Michigan", "MI", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Michigan", "MI", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Minnesota", "MN", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Mississippi", "MS", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Missouri", "MO", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Montana", "MT", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Nebraska", "NE", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Nebraska", "NE", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Nevada", "NV", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Nevada", "NV", "Pacific Standard Time", "PST", -28800, 1, "Pacific Daylight Time", "PDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "New Hampshire", "NH", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "New Jersey", "NJ", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "New Mexico", "NM", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "New York", "NY", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "North Carolina", "NC", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "North Dakota", "ND", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "North Dakota", "ND", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Ohio", "OH", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Oklahoma", "OK", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Oregon", "OR", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Oregon", "OR", "Pacific Standard Time", "PST", -28800, 1, "Pacific Daylight Time", "PDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Pennsylvania", "PA", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Rhode Island", "RI", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "South Carolina", "SC", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "South Dakota", "SD", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "South Dakota", "SD", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Tennessee", "TN", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Tennessee", "TN", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Texas", "TX", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Texas", "TX", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Utah", "UT", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Vermont", "VT", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Virginia", "VA", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Washington", "WA", "Pacific Standard Time", "PST", -28800, 1, "Pacific Daylight Time", "PDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "West Virginia", "WV", "Eastern Standard Time", "EST", -18000, 1, "Eastern Daylight Time", "EDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Wisconsin", "WI", "Central Standard Time", "CST", -21600, 1, "Central Daylight Time", "CDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States", "US", "USA", "Wyoming", "WY", "Mountain Standard Time", "MST", -25200, 1, "Mountain Daylight Time", "MDT", "Second Sunday of March at 02:00 Local", 0x3c00020200020001, "First Sunday of November at 02:00 Local", 0xc400020a000100ff }, \
        { "United States Virgin Islands", "VI", "VIR", "", "", "Atlantic Standard Time", "AST", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Uruguay", "UY", "URY", "", "", "Uruguay Time", "UYT", -10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Uzbekistan", "UZ", "UZB", "", "", "Uzbekistan Time", "UZT", +18000, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Vanuatu", "VU", "VUT", "", "", "Vanuatu Time", "VUT", +39600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Vatican City", "VA", "VAT", "", "", "Central European Time", "CET", +3600, 1, "Central European Daylight Time", "CEDT", "Last Sunday of March at 02:00 Local", 0x3c00020200060001, "Last Sunday of October at 03:00 Local", 0xc4000309000600ff }, \
        { "Venezuela", "VE", "VEN", "", "", "Venezuelan Standard Time", "VET", -14400, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Vietnam", "VN", "VNM", "", "", "Indochina Time", "ICT", +25200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Wallis", "WF", "WLF", "", "", "Wallis and Futuna Time", "WFT", +43200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Western Sahara", "EH", "ESH", "", "", "Western European Daylight Time", "WEDT", +3600, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Yemen", "YE", "YEM", "", "", "Arabia Standard Time", "AST", +10800, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Zambia", "ZM", "ZMB", "", "", "Central Africa Time", "CAT", +7200, 0, "", "", "", 0x0, "", 0x0 }, \
        { "Zimbabwe", "ZW", "ZWE", "", "", "Central Africa Time", "CAT", +7200, 0, "", "", "", 0x0, "", 0x0 } \
}


//**********************************************************************************//
//****************  Library Includes (that depend on medlib_m10.h)   ***************//
//**********************************************************************************//

#include "medrec_m10.h"


#endif  // MEDLIB_IN_m10







