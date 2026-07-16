
//**********************************************************************************//
//***************************  MED 1.1.3 C Library Records  ************************//
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
// Mnimal UTF-8 manipulation routines are included in the library, for convenience.

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
// with the suffix "_mFL" (for "MED major format 'F', library 'L'").

// MED_FORMAT_VERSION_MAJOR is restricted to single digits 1 through 9
// MED_FORMAT_VERSION_MINOR is restricted to 0 through 254, minor version resets to zero with new major format version
// MED_LIBRARY_VERSION is restricted to 1 through 254, library version resets to one with new major format version

// MED_FULL_FORMAT_NAME == "<MED_VERSION_MAJOR_m13>.<MED_VERSION_MINOR_m13>"
// MED_FULL_LIBRARY_NAME == "<MED_FULL_FORMAT_NAME_m13>.<MED_LIBRARY_VERSION_m13>"
// MED_LIBRARY_TAG == "<MED_VERSION_MAJOR_m13>.<MED_LIBRARY_VERSION_m13>"

// Examples:
// "_m13" indicates "MED format major version 1, library version 3"
// "_m21" indicates "MED format major version 2, library version 1" (for MED 2)
// "_m213" indicates "MED format major version 2, library version 13" (for MED 2)

// All library versions associated with a particular major format version are guaranteed to work on MED files of that major version.
// Minor format versions may add fields to the format in protected regions, but no preexisting fields will be removed or moved.
// Only library versions released on or afer a minor version will make use of new fields, and only if the minor version of the files contains them.
// Backward compatibility will be maintained between major versions if practical.


#ifndef MEDREC_IN_m13
#define MEDREC_IN_m13

#include "medlib_m13.h"

//*************************************************************************************//
//***************************   General Record Constants   ****************************//
//*************************************************************************************//

// Constants
// LARGEST_RECORD_BYTES_m13 needs to be kept up to date, and can fail with arbitrary size records
// (structure layouts are verified at compile time: see the layout assertions at the end of this header)
#define REC_LARGEST_RECORD_BYTES_m13		METADATA_FILE_BYTES_m13  // 16 kB
#define REC_BODY_ALIGNMENT_BYTES_m13		8  // record body size must be multiple of this
#define REC_NO_RECORD_NUMBER_m13		-1

// Prototypes
tern	REC_show_record_m13(FPS_m13 *fps, REC_HDR_m13 *record_header, si8 record_number);  // pass NO_RECORD_NUMBER_m13 (for record_number) to suppress record number display


//*************************************************************************************//
//********************************   Record Types   ***********************************//
//*************************************************************************************//

// Use CamelCase for 4-letter type string (case sensitive types)
// Translate to type code one byte at a time with ascii table
// (note that bytes, and therefore letters, are reversed in little-endian versions)
// #defines, structure names, and function names should follow the format in examples below
// Record versions are specified in defines and structure names as v<version major><version minor>
// e.g. version 1.0 is denoted by v10
// Record structures require 8-byte alignment. Pad structures where necessary.


//*************************************************************************************//
//******************************   Term: Terminal Record   ****************************//
//*************************************************************************************//

// Constants
#define REC_Term_TYPE_STRING_m13        "Term"                  // ascii[4]
#define REC_Term_TYPE_CODE_m13          (ui4) 0x6d726554        // ui4 (little endian)
// #define REC_Term_TYPE_CODE_m13       (ui4) 0x5465726d        // ui4 (big endian)

// If there are any records, there is an additional a terminal record index
// This is for terminal index only, there is no corresponding "Term" record data entry
// File Offset = record data file length
// Start Time = segment end μUTC + 1
// Type String = REC_Term_TYPE_STRING_m13
// Type Code = REC_Term_TYPE_CODE_m13
// Format Version Major = 0xFF
// Format Version Minor = 0xFF
// Encryption Level = 0

//*************************************************************************************//
//*******************************   Sgmt: Segment Record   ****************************//
//*************************************************************************************//

// A segment record is entered at the Session and or Channel Level for each new segment
// The encryption level for these records is typically set to the same as for metadata section 2

// Constants
#define REC_Sgmt_TYPE_STRING_m13        "Sgmt"                  // ascii[4]
#define REC_Sgmt_TYPE_CODE_m13          (ui4) 0x746D6753        // ui4 (little endian)
// #define REC_Sgmt_TYPE_CODE_m13       (ui4) 0x53676D74        // ui4 (big endian)

// Version 1.0 (for compatibility)
#define REC_Sgmt_v10_BYTES_m13				48
#define REC_Sgmt_v10_END_TIME_OFFSET_m13		0						// si8
#define REC_Sgmt_v10_START_IDX_OFFSET_m13		8						// si8
#define REC_Sgmt_v10_START_IDX_NO_ENTRY_m13		SAMPLE_NUMBER_NO_ENTRY_m13
#define REC_Sgmt_v10_END_IDX_OFFSET_m13			16						// si8
#define REC_Sgmt_v10_END_IDX_NO_ENTRY_m13		SAMPLE_NUMBER_NO_ENTRY_m13
#define REC_Sgmt_v10_SEG_UID_OFFSET_m13			24						// ui8
#define REC_Sgmt_v10_SEG_NUM_OFFSET_m13			32						// si4
#define REC_Sgmt_v10_SEG_NUM_NO_ENTRY_m13		SEGMENT_NUMBER_NO_ENTRY_m13
#define REC_Sgmt_v10_ACQ_CHAN_NUM_OFFSET_m13		36						// si4
#define REC_Sgmt_v10_ACQ_CHAN_NUM_NO_ENTRY_m13		CHANNEL_NUMBER_NO_ENTRY_m13
#define REC_Sgmt_v10_ACQ_CHAN_NUM_ALL_CHANNELS_m13	CHANNEL_NUMBER_ALL_CHANNELS_m13
#define REC_Sgmt_v10_RATE_OFFSET_m13			40						// sf8
#define REC_Sgmt_v10_RATE_NO_ENTRY_m13			RATE_NO_ENTRY_m13
#define REC_Sgmt_v10_RATE_VARIABLE_m13			RATE_VARIABLE_m13
#define REC_Sgmt_v10_DESCRIPTION_OFFSET_m13		REC_Sgmt_v10_BYTES_m13				// si1 (arbitrary length, padded to 16-byte alignment for encryption)

// Structure defined in medlib_m13.h due to codependency

// Version 1.1
#define REC_Sgmt_v11_BYTES_m13				32
#define REC_Sgmt_v11_END_TIME_OFFSET_m13		0						// si8
#define REC_Sgmt_v11_START_IDX_OFFSET_m13		8						// si8
#define REC_Sgmt_v11_START_IDX_NO_ENTRY_m13		SAMPLE_NUMBER_NO_ENTRY_m13
#define REC_Sgmt_v11_END_IDX_OFFSET_m13			16						// si8
#define REC_Sgmt_v11_END_IDX_NO_ENTRY_m13		SAMPLE_NUMBER_NO_ENTRY_m13
#define REC_Sgmt_v11_SEG_NUM_OFFSET_m13			24						// si4
#define REC_Sgmt_v11_SEG_NUM_NO_ENTRY_m13		SEGMENT_NUMBER_NO_ENTRY_m13
#define REC_Sgmt_v11_RATE_OFFSET_m13			28						// sf4
#define REC_Sgmt_v11_RATE_NO_ENTRY_m13			((sf4) RATE_NO_ENTRY_m13)
#define REC_Sgmt_v11_RATE_VARIABLE_m13			((sf4) RATE_VARIABLE_m13)
#define REC_Sgmt_v11_DESCRIPTION_OFFSET_m13		REC_Sgmt_v11_BYTES_m13				// si1 (arbitrary length, padded to 8-byte alignment)

// Structure defined in medlib.h due to codependency

// Prototypes
tern	REC_show_Sgmt_type_m13(REC_HDR_m13 *record_header);


//*************************************************************************************//
//*****************************   Stat: Statistics Record   ***************************//
//*************************************************************************************//

// Constants
#define REC_Stat_TYPE_STRING_m13        "Stat"                  // ascii[4]
#define REC_Stat_TYPE_CODE_m13          (ui4) 0x74617453        // ui4 (little endian)
// #define REC_Sgmt_TYPE_CODE_m13       (ui4) 0x53746174        // ui4 (big endian)

// Version 1.0
#define REC_Stat_v10_BYTES_m13			32
#define REC_Stat_v10_MINIMUM_OFFSET_m13		0                       // si4
#define REC_Stat_v10_MINIMUM_NO_ENTRY_m13	NAN_SI4_m13
#define REC_Stat_v10_MAXIMUM_OFFSET_m13		4                       // si4
#define REC_Stat_v10_MAXIMUM_NO_ENTRY_m13	NAN_SI4_m13
#define REC_Stat_v10_MEAN_OFFSET_m13		8                       // si4
#define REC_Stat_v10_MEAN_NO_ENTRY_m13		NAN_SI4_m13
#define REC_Stat_v10_MEDIAN_OFFSET_m13		12                      // si4
#define REC_Stat_v10_MEDIAN_NO_ENTRY_m13	NAN_SI4_m13
#define REC_Stat_v10_MODE_OFFSET_m13		16			// si4
#define REC_Stat_v10_MODE_NO_ENTRY_m13		NAN_SI4_m13
#define REC_Stat_v10_VARIANCE_OFFSET_m13	20                      // sf4
#define REC_Stat_v10_VARIANCE_NO_ENTRY_m13	((sf4) nan(NULL))       // NOTE this value must be tested with isnan(), or an equivalent function, rather than ==
#define REC_Stat_v10_SKEWNESS_OFFSET_m13	24                      // sf4
#define REC_Stat_v10_SKEWNESS_NO_ENTRY_m13	((sf4) nan(NULL))       // NOTE this value must be tested with isnan(), or an equivalent function, rather than ==
#define REC_Stat_v10_KURTOSIS_OFFSET_m13	28                      // sf4
#define REC_Stat_v10_KURTOSIS_NO_ENTRY_m13	((sf4) nan(NULL))       // NOTE this value must be tested with isnan(), or an equivalent function, rather than ==

// Structures
// REC_Stat_v10_m13 defined in medlib_m13.h due to codependency

// Prototypes
tern	REC_show_Stat_type_m13(REC_HDR_m13 *record_header);



//*************************************************************************************//
//******************************   EDFA: EDF Annotation   *****************************//
//*************************************************************************************//
// Constants
#define REC_EDFA_TYPE_STRING_m13        "EDFA"			// ascii[4]
#define REC_EDFA_TYPE_CODE_m13		(ui4) 0x41464445	// ui4 (little endian)
// #define REC_EDFA_TYPE_CODE_m13    	(ui4) 0x45444641	// ui4 (big endian)

// Version 1.0
#define REC_EDFA_v10_BYTES_m13			8
#define REC_EDFA_v10_DURATION_OFFSET_m13        0			// si8
#define REC_EDFA_v10_ANNOTATION_OFFSET_m13      REC_EDFA_v10_BYTES_m13	// si1

// Structures
typedef struct {
	si8	duration;  // microseconds (single element structure not really necessary, but done for consistency).
} REC_EDFA_v10_m13;
// Annotation follows structure - aribitrary length array of si1s padded to 16 byte alignment (for structure + string)

// Prototypes
tern	REC_show_EDFA_type_m13(REC_HDR_m13 *record_header);



//*************************************************************************************//
//********************************   Note: Note Record   ******************************//
//*************************************************************************************//

// Constants
#define REC_Note_TYPE_STRING_m13        "Note"			// ascii[4]
#define REC_Note_TYPE_CODE_m13		(ui4) 0x65746f4e	// ui4 (little endian)
// #define REC_Note_TYPE_CODE_m13       (ui4) 0x4e6f7465	// ui4

// Version 1.0
#define REC_Note_v10_TEXT_OFFSET_m13    0

// Structures
// (none)
// Annotation follows header - aribitrary length array of si1s padded to 8 byte alignment

// Version 1.1
#define REC_Note_v11_BYTES_m13			8
#define REC_Note_v11_END_TIME_OFFSET_m13	0
#define REC_Note_v11_TEXT_OFFSET_m13		REC_Note_v11_BYTES_m13

// Structures
typedef struct {
	si8     end_time;  // time when note entered into record (header start time is time when note initiated)
} REC_Note_v11_m13;
// Annotation follows structure - aribitrary length array of si1s padded to 8 byte alignment (struct plus text)


// Prototypes
tern	REC_show_Note_type_m13(REC_HDR_m13 *record_header);



//*************************************************************************************//
//*******************************   Seiz: Seizure Record   ****************************//
//*************************************************************************************//

// Constants
#define REC_Seiz_TYPE_STRING_m13        "Seiz"                  // ascii[4]
#define REC_Seiz_TYPE_CODE_m13          (ui4) 0x7a696553        // ui4 (little endian)
// #define REC_Seiz_TYPE_CODE_m13       (ui4) 0x5365697a        // ui4 (big endian)

// Version 1.0
// REC_Seiz_v10_m13 offsets below apply to base address of record
#define REC_Seiz_v10_BYTES_m13					1296
#define REC_Seiz_v10_LATEST_OFFSET_TIME_OFFSET_m13		0	// si8
#define REC_Seiz_v10_NUMBER_OF_CHANNELS_OFFSET_m13		8	// si4
#define REC_Seiz_v10_ONSET_CODE_OFFSET_m13			12	// si4
#define REC_Seiz_v10_MARKER_NAME_1_OFFSET_m13			16	// utf8[31]
#define REC_Seiz_v10_MARKER_NAME_BYTES_m13			128
#define REC_Seiz_v10_MARKER_NAME_2_OFFSET_m13			144	// utf8[31]
#define REC_Seiz_v10_ANNOTATION_OFFSET_m13			272	// utf8[255]
#define REC_Seiz_v10_ANNOTATION_BYTES_m13			1024
#define REC_Seiz_v10_CHANNELS_OFFSET_m13			REC_Seiz_v10_BYTES_m13
// REC_Seiz_v10_CHANNEL_m13 offsets below apply to base address of channel
#define REC_Seiz_v10_CHANNEL_BYTES_m13				280
#define REC_Seiz_v10_CHANNEL_NAME_OFFSET_m13			0	// utf8[63]
#define REC_Seiz_v10_CHANNEL_ONSET_TIME_OFFSET_m13		256	// si8
#define REC_Seiz_v10_CHANNEL_OFFSET_TIME_OFFSET_m13		264	// si8
#define REC_Seiz_v10_CHANNEL_SEGMENT_NUMBER_OFFSET_m13		272	// si4
#define REC_Seiz_v10_CHANNEL_SEGMENT_NUMBER_NO_ENTRY_m13	SEGMENT_NUMBER_NO_ENTRY_m13
#define REC_Seiz_v10_CHANNEL_PAD_OFFSET_m13			276	// ui1[4]
#define REC_Seiz_v10_CHANNEL_PAD_BYTES_m13			4
// Onset Codes
#define REC_Seiz_v10_ONSET_NO_ENTRY_m13		-1
#define REC_Seiz_v10_ONSET_UNKNOWN_m13		0
#define REC_Seiz_v10_ONSET_FOCAL_m13            1
#define REC_Seiz_v10_ONSET_GENERALIZED_m13      2
#define REC_Seiz_v10_ONSET_PROPAGATED_m13       3
#define REC_Seiz_v10_ONSET_MIXED_m13            4

// Structures
typedef struct {
        // earliest_onset_time is in the record header
        si8	latest_offset_time;                                     // uutc
        si4	number_of_channels;
        si4	onset_code;
        si1	marker_name_1[REC_Seiz_v10_MARKER_NAME_BYTES_m13];	// utf8[31]
        si1	marker_name_2[REC_Seiz_v10_MARKER_NAME_BYTES_m13];	// utf8[31]
        si1	annotation[REC_Seiz_v10_ANNOTATION_BYTES_m13];		// utf8[255]
} REC_Seiz_v10_m13;

typedef struct {
 	si1	name[NAME_BYTES_m13]; // channel name, no extension
       	si8	onset_time;                     // uutc
        si8	offset_time;                    // uutc
        si4     segment_number;
        ui1     pad[REC_Seiz_v10_CHANNEL_PAD_BYTES_m13];	// for 8 byte alignment (couldn't think of a use for them)
} REC_Seiz_v10_CHANNEL_m13;

// Prototypes
tern	REC_show_Seiz_type_m13(REC_HDR_m13 *record_header);



//*************************************************************************************//
//*****************************   SyLg: System Log Record   ***************************//
//*************************************************************************************//

// Constants
#define REC_SyLg_TYPE_STRING_m13        "SyLg"			// ascii[4]
#define REC_SyLg_TYPE_CODE_m13	        (ui4) 0x674c7953	// ui4 (little endian)
// #define REC_SyLg_TYPE_CODE_m13       (ui4) 0x53794c67	// ui4 (big endian)

// Version 1.0
#define REC_SyLg_v10_TEXT_OFFSET_m13    0

// Structures
// (none)
// Log text follows header - aribitrary length array of si1s padded to 16 byte alignment

// Prototype
tern	REC_show_SyLg_type_m13(REC_HDR_m13 *record_header);



//*************************************************************************************//
//***************************   NlxP: NeuraLynx Port Record   *************************//
//*************************************************************************************//

// Constants
#define REC_NlxP_TYPE_STRING_m13             "NlxP"                  // ascii[4]
#define REC_NlxP_TYPE_CODE_m13               (ui4) 0x50786C4E        // ui4 (little endian)
// #define REC_NlxP_TYPE_CODE_m13            (ui4) 0x4E6C7850        // ui4 (big endian)

// Trigger Modes
#define REC_NlxP_v10_NO_TRIGGER_m13		((ui1) 0)
#define REC_NlxP_v10_ANY_BIT_CHANGE_m13		((ui1) 1)
#define REC_NlxP_v10_HIGH_BIT_SET_m13		((ui1) 2)
#define REC_NlxP_v10_UNKNOWN_TRIGGER_MODE_m13	((ui1) 0xFF)

// Version 1.0
#define REC_NlxP_v10_BYTES_m13                          16
#define REC_NlxP_v10_RAW_PORT_VALUE_OFFSET_m13          0	// ui4
#define REC_NlxP_v10_VALUE_OFFSET_m13                   4	// ui4
#define REC_NlxP_v10_SUBPORT_OFFSET_m13                 8	// ui1
#define REC_NlxP_v10_NUMBER_OF_SUBPORTS_OFFSET_m13      9	// ui1
#define REC_NlxP_v10_TRIGGER_MODE_OFFSET_m13            10	// ui1
#define REC_NlxP_v10_PAD_OFFSET_m13               	11	// ui1
#define REC_NlxP_v10_PAD_BYTES_m13                      5

// Structures
typedef struct {
        ui4     raw_port_value;         // full port value (no bits masked out)
        ui4     value;                  // subport value only (high bits masked out in HIGH BIT SET trigger mode)
        ui1     subport;                // [1 to <number_of_subports>]  (subport that triggered this record)
        ui1     number_of_subports;     // [1, 2, or 4]  (one 32-bit, two 16-bit, or four 8-bit)
        ui1     trigger_mode;           // REC_NlxP_v10_NO_TRIGGER_m13, REC_NlxP_v10_ANY_BIT_CHANGE_m13, or REC_NlxP_v10_HIGH_BIT_SET_m13
        ui1     pad[REC_NlxP_v10_PAD_BYTES_m13];
} REC_NlxP_v10_m13;

// Prototypes
tern	REC_show_NlxP_type_m13(REC_HDR_m13 *record_header);


//*************************************************************************************//
//***********************   Curs: Cadwell EMG Cursor Annotation   *********************//
//*************************************************************************************//

// Constants
#define REC_Curs_TYPE_STRING_m13	"Curs"                  // ascii[4]
#define REC_Curs_TYPE_CODE_m13		(ui4) 0x73727543        // ui4 (little endian)
// #define REC_Curs_TYPE_CODE_m13	(ui4) 0x43757273        // ui4 (big endian)

// Version 1.0
#define REC_Curs_v10_BYTES_m13			160
#define REC_Curs_v10_ID_NUMBER_OFFSET_m13	0	// si8
#define REC_Curs_v10_LATENCY_OFFSET_m13		8	// si8
#define REC_Curs_v10_VALUE_OFFSET_m13		16	// sf8
#define REC_Curs_v10_NAME_OFFSET_m13		24	// si1[136]
#define REC_Curs_v10_NAME_BYTES_m13		136

// Structures
typedef struct {
    si8 id_number;
    si8 latency;
    sf8 value;
    si1 name[REC_Curs_v10_NAME_BYTES_m13];
} REC_Curs_v10_m13;

// Prototypes
tern	REC_show_Curs_type_m13(REC_HDR_m13 *record_header);



//*************************************************************************************//
//****************************   Epoc: Sleep Stage Record   ***************************//
//*************************************************************************************//

// Constants
#define REC_Epoc_TYPE_STRING_m13	"Epoc"                  // ascii[4]
#define REC_Epoc_TYPE_CODE_m13		(ui4) 0x636F7045        // ui4 (little endian)
// #define REC_Epoc_TYPE_CODE_m13	(ui4) 0x45706F63        // ui4 (big endian)

// Version 1.0 Defines
#define REC_Epoc_v10_BYTES_m13			176
#define REC_Epoc_v10_ID_NUMBER_OFFSET_m13	0	// si8
#define REC_Epoc_v10_END_TIME_OFFSET_m13	8	// si8
#define REC_Epoc_v10_EPOCH_TYPE_OFFSET_m13	16	// si1[32]
#define REC_Epoc_v10_EPOCH_TYPE_BYTES_m13	32
#define REC_Epoc_v10_TEXT_OFFSET_m13		48	// si1[128]
#define REC_Epoc_v10_TEXT_BYTES_m13		128

// Version 1.0 Structures
typedef struct {
	si8 id_number;
	si8 end_time;
	si1 epoch_type[REC_Epoc_v10_EPOCH_TYPE_BYTES_m13];
	si1 text[REC_Epoc_v10_TEXT_BYTES_m13];
} REC_Epoc_v10_m13;

// Version 2.0 Defines
#define REC_Epoc_v20_BYTES_m13			48
#define REC_Epoc_v20_END_TIME_OFFSET_m13	0	// si8
#define REC_Epoc_v20_STAGE_CODE_OFFSET_m13	8	// ui1
#define REC_Epoc_v20_SCORER_ID_OFFSET_m13	9	// si1[39]
#define REC_Epoc_v20_SCORER_ID_BYTES_m13	39

#define REC_Epoc_v20_STAGE_AWAKE_m13		0
#define REC_Epoc_v20_STAGE_NREM_1_m13		1
#define REC_Epoc_v20_STAGE_NREM_2_m13		2
#define REC_Epoc_v20_STAGE_NREM_3_m13		3
#define REC_Epoc_v20_STAGE_NREM_4_m13		4
#define REC_Epoc_v20_STAGE_REM_m13		5
#define REC_Epoc_v20_STAGE_UNKNOWN_m13		255

// Structures
typedef struct {
	si8 end_time;
	ui1 stage_code;
	si1 scorer_id[REC_Epoc_v20_SCORER_ID_BYTES_m13];  // person or algorithm
} REC_Epoc_v20_m13;

// Prototypes
tern	REC_show_Epoc_type_m13(REC_HDR_m13 *record_header);



//*************************************************************************************//
//**************************   ESti: Electrical Stimulation   *************************//
//*************************************************************************************//

// Constants
#define REC_ESti_TYPE_STRING_m13	"ESti"                  // ascii[4]
#define REC_ESti_TYPE_CODE_m13		(ui4) 0x69745345        // ui4 (little endian)
// #define REC_ESti_TYPE_CODE_m13	(ui4) 0x45537469        // ui4 (big endian)

// Version 1.0
#define REC_ESti_v10_BYTES_m13          		416
#define REC_ESti_v10_AMPLITUDE_OFFSET_m13          	0	// sf8
#define REC_ESti_v10_FREQUENCY_OFFSET_m13  		8	// sf8
#define REC_ESti_v10_PULSE_WIDTH_OFFSET_m13		16	// si8
#define REC_ESti_v10_AMP_UNIT_CODE_OFFSET_m13		24	// si4
#define REC_ESti_v10_MODE_CODE_OFFSET_m13		28	// si4
#define REC_ESti_v10_WAVEFORM_OFFSET_m13		32	// utf8[31]
#define REC_ESti_v10_WAVEFORM_BYTES_m13			128
#define REC_ESti_v10_ANODE_OFFSET_m13			160	// utf8[31]
#define REC_ESti_v10_ANODE_BYTES_m13			128
#define REC_ESti_v10_CATHODE_OFFSET_m13			288	// utf8[31]
#define REC_ESti_v10_CATHODE_BYTES_m13			128

// Unit codes
#define REC_ESti_v10_AMP_UNIT_NO_ENTRY_m13	-1
#define REC_ESti_v10_AMP_UNIT_UNKNOWN_m13	0
#define REC_ESti_v10_AMP_UNIT_MA_m13		1
#define REC_ESti_v10_AMP_UNIT_V_m13		2

// Mode codes
#define REC_ESti_v10_MODE_NO_ENTRY_m13		-1
#define REC_ESti_v10_MODE_UNKNOWN_m13		0
#define REC_ESti_v10_MODE_CURRENT_m13		1
#define REC_ESti_v10_MODE_VOLTAGE_m13		2

// Structures
typedef struct {
	sf8	amplitude;
	sf8	frequency;
	si8	pulse_width;
	si4	amp_unit_code;
	si4	mode_code;
	si1	waveform[REC_ESti_v10_WAVEFORM_BYTES_m13];
	si1	anode[REC_ESti_v10_ANODE_BYTES_m13];
	si1	cathode[REC_ESti_v10_CATHODE_BYTES_m13];
} REC_ESti_v10_m13;

// Prototypes
tern	REC_show_ESti_type_m13(REC_HDR_m13 *record_header);



//*************************************************************************************//
//****************   CSig: Cryptographic Signature (digest / signature)   *************//
//*************************************************************************************//

// Constants
#define REC_CSig_TYPE_STRING_m13	"CSig"			// ascii[4]
#define REC_CSig_TYPE_CODE_m13		(ui4) 0x67695343	// ui4 (little endian)
// #define REC_CSig_TYPE_CODE_m13	(ui4) 0x43536967	// ui4 (big endian)

// Digest algorithm codes
#define REC_CSig_DIGEST_NONE_m13	((ui4) 0)
#define REC_CSig_DIGEST_SHA256_m13	((ui4) 1) // SHA-256 triplet (see DGST module, medlib_m13.h): body digest (data region alone;
						  // survives legitimate UH transformations), full digest (data then UH - canonical),
						  // & pre-UH resume state (re-derive full after a UH change without re-reading the body)
#define REC_CSig_DIGEST_SHA256_BODY_OFFSET_m13		0	// ui1[32] (into the digest element of the variable region)
#define REC_CSig_DIGEST_SHA256_FULL_OFFSET_m13		32	// ui1[32]
#define REC_CSig_DIGEST_SHA256_RESUME_OFFSET_m13	64	// ui1[DGST_RESUME_BYTES_m13 == 108] (layout: DGST_RESUME_*_OFFSET_m13)
#define REC_CSig_DIGEST_SHA256_BYTES_m13		( 64 + DGST_RESUME_BYTES_m13 )	// 172 == digest_bytes for this algorithm

// Signature algorithm codes
#define REC_CSig_SIG_NONE_m13		((ui4) 0) // digest only (tamper-evidence, no attribution)
#define REC_CSig_SIG_ED25519_m13	((ui4) 1) // reserved: Ed25519 signature over the digest (not yet implemented)

// Occasion codes (why the record was generated; when == record time - chain-of-custody value)
#define REC_CSig_OCCASION_UNSPECIFIED_m13	((ui1) 0)
#define REC_CSig_OCCASION_CLOSE_m13		((ui1) 1) // file finalized by its writer (acquisition segment close)
#define REC_CSig_OCCASION_CONVERSION_m13	((ui1) 2) // format conversion completed
#define REC_CSig_OCCASION_TRANSFER_m13		((ui1) 3) // transfer hand-off
#define REC_CSig_OCCASION_ARCHIVE_m13		((ui1) 4) // archive commit
#define REC_CSig_OCCASION_REPAIR_m13		((ui1) 5) // post-repair
#define REC_CSig_OCCASION_REKEY_m13		((ui1) 6) // post re-encryption
#define REC_CSig_OCCASION_DEMAND_m13		((ui1) 7) // on demand

// Version 1.0
#define REC_CSig_v10_BYTES_m13				24	// fixed region only; digest/signature/public-key follow
#define REC_CSig_v10_TARGET_FILE_UID_OFFSET_m13		0	// ui8 (File UID of the file this record vouches for; named by UID, not path)
#define REC_CSig_v10_DIGEST_ALGORITHM_OFFSET_m13	8	// ui4
#define REC_CSig_v10_SIGNATURE_ALGORITHM_OFFSET_m13	12	// ui4
#define REC_CSig_v10_DIGEST_BYTES_OFFSET_m13		16	// ui2 (d)
#define REC_CSig_v10_SIGNATURE_BYTES_OFFSET_m13		18	// ui2 (s)
#define REC_CSig_v10_PUBLIC_KEY_BYTES_OFFSET_m13	20	// ui2 (k)
#define REC_CSig_v10_OCCASION_OFFSET_m13		22	// ui1 (REC_CSig_OCCASION_*)
#define REC_CSig_v10_PAD_OFFSET_m13			23	// ui1[1]
#define REC_CSig_v10_PAD_BYTES_m13			1
#define REC_CSig_v10_VARIABLE_REGION_OFFSET_m13		REC_CSig_v10_BYTES_m13	// digest (d bytes), then signature (s bytes), then public key (k bytes)

// Structures
typedef struct {
	ui8	target_file_UID;	// File UID of the file this record vouches for
	ui4	digest_algorithm;	// REC_CSig_DIGEST_*
	ui4	signature_algorithm;	// REC_CSig_SIG_*
	ui2	digest_bytes;		// d
	ui2	signature_bytes;	// s (0 when signature_algorithm == none)
	ui2	public_key_bytes;	// k (0 when signature_algorithm == none)
	ui1	occasion;		// REC_CSig_OCCASION_*
	ui1	pad[REC_CSig_v10_PAD_BYTES_m13];
} REC_CSig_v10_m13;
// Variable region follows structure: digest[d] || signature[s] || public_key[k], padded to 16-byte alignment (struct + arrays)

// Prototypes
tern	REC_show_CSig_type_m13(REC_HDR_m13 *record_header);
si8	REC_build_CSig_body_m13(const si1 *target_file_path, ui8 target_file_UID, ui1 occasion, const DGST_RESULT_m13 *dgst, ui1 *body); // fills a v1.0 CSig record body (digest-only); dgst == NULL => compute by streamed read (DGST_file_m13()); returns body bytes (pre-pad) or FALSE_m13
tern	REC_write_CSig_m13(const si1 *level_path, si8 record_time, const si1 *target_file_path, ui8 target_file_UID, ui1 occasion, const DGST_RESULT_m13 *dgst); // appends a CSig record + index entry to the passed level's records files - CONVENTION: the target's CHANNEL directory (channel subsets carry their attestations); see REC_build_CSig_body_m13() for dgst semantics; record_time is oUTC



//*************************************************************************************//
//***************************   CSti: Cognitive Stimulation   *************************//
//*************************************************************************************//

// Constants
#define REC_CSti_TYPE_STRING_m13	"CSti"                  // ascii[4]
#define REC_CSti_TYPE_CODE_m13		(ui4) 0x69745343        // ui4 (little endian)
// #define REC_CSti_TYPE_CODE_m13	(ui4) 0x43537469        // ui4 (big endian)

// Version 1.0
#define REC_CSti_v10_BYTES_m13          		208
#define REC_CSti_v10_STIMULUS_DURATION_OFFSET_m13	0	// si8
#define REC_CSti_v10_TASK_TYPE_OFFSET_m13  		8	// utf8[15]
#define REC_CSti_v10_TASK_TYPE_BYTES_m13		64
#define REC_CSti_v10_STIMULUS_TYPE_OFFSET_m13		72	// utf8[15]
#define REC_CSti_v10_STIMULUS_TYPE_BYTES_m13		64
#define REC_CSti_v10_PATIENT_RESPONSE_OFFSET_m13	136	// utf8[15]
#define REC_CSti_v10_PATIENT_RESPONSE_BYTES_m13		64
#define REC_CSti_v10_PAD_OFFSET_m13			200	// ui1[8]
#define REC_CSti_v10_PAD_BYTES_m13			8

// Structures
typedef struct {
	si8	stimulus_duration;
	si1	task_type[REC_CSti_v10_TASK_TYPE_BYTES_m13];
	si1	stimulus_type[REC_CSti_v10_STIMULUS_TYPE_BYTES_m13];
	si1	patient_response[REC_CSti_v10_PATIENT_RESPONSE_BYTES_m13];
	ui1	pad[REC_CSti_v10_PAD_BYTES_m13];
} REC_CSti_v10_m13;

// Prototypes
tern	REC_show_CSti_type_m13(REC_HDR_m13 *record_header);



//*************************************************************************************//
//******************************   HFOc: CS HFO Detection   ***************************//
//*************************************************************************************//

// Constants
#define REC_HFOc_TYPE_STRING_m13			"HFOc"                  // ascii[4]
#define REC_HFOc_TYPE_CODE_m13				(ui4) 0x634F4648        // ui4 (little endian)
// #define REC_HFOc_TYPE_CODE_m13			(ui4) 0x48464F63        // ui4 (big endian)

// CS HFO bands
#define REC_HFOc_NUMBER_OF_BANDS_m13			4
#define REC_HFOc_BAND_STARTS_m13			{44, 73, 120, 197}
#define REC_HFOc_BAND_CENTERS_m13			{73, 120, 197, 326}
#define REC_HFOc_BAND_STOPS_m13				{120, 197, 326, 537}

// Version 1.0 (onset time only)
#define REC_HFOc_v10_BYTES_m13          		0

// Version 1.1 (minimal info)
#define REC_HFOc_v11_BYTES_m13          		16
#define REC_HFOc_v11_END_TIME_OFFSET_m13		0	// si8
#define REC_HFOc_v11_START_FREQUENCY_OFFSET_m13		8	// sf4
#define REC_HFOc_v11_END_FREQUENCY_OFFSET_m13		12	// sf4

// Version 1.2 (standard info)
#define REC_HFOc_v12_BYTES_m13          		96
#define REC_HFOc_v12_END_TIME_OFFSET_m13		REC_HFOc_v11_END_TIME_OFFSET_m13
#define REC_HFOc_v12_START_FREQUENCY_OFFSET_m13		REC_HFOc_v11_START_FREQUENCY_OFFSET_m13
#define REC_HFOc_v12_END_FREQUENCY_OFFSET_m13		REC_HFOc_v11_END_FREQUENCY_OFFSET_m13
#define REC_HFOc_v12_START_TIMES_OFFSET_m13		16	// si8[4]
#define REC_HFOc_v12_END_TIMES_OFFSET_m13		48	// si8[4]
#define REC_HFOc_v12_COMBINATIONS_OFFSET_m13		80	// sf4[4]

// Version 1.3 (maximal info)
#define REC_HFOc_v13_BYTES_m13          		160
#define REC_HFOc_v13_END_TIME_OFFSET_m13		REC_HFOc_v12_END_TIME_OFFSET_m13
#define REC_HFOc_v13_START_FREQUENCY_OFFSET_m13		REC_HFOc_v12_START_FREQUENCY_OFFSET_m13
#define REC_HFOc_v13_END_FREQUENCY_OFFSET_m13		REC_HFOc_v12_END_FREQUENCY_OFFSET_m13
#define REC_HFOc_v13_START_TIMES_OFFSET_m13		REC_HFOc_v12_START_TIMES_OFFSET_m13
#define REC_HFOc_v13_END_TIMES_OFFSET_m13		REC_HFOc_v12_END_TIMES_OFFSET_m13
#define REC_HFOc_v13_COMBINATIONS_OFFSET_m13		REC_HFOc_v12_COMBINATIONS_OFFSET_m13
#define REC_HFOc_v13_AMPLITUDES_OFFSET_m13		96	// sf4[4]
#define REC_HFOc_v13_FREQUENCY_DOMINANCES_OFFSET_m13	112	// sf4[4]
#define REC_HFOc_v13_PRODUCTS_OFFSET_m13		128	// sf4[4]
#define REC_HFOc_v13_CYCLES_OFFSET_m13			144	// sf4[4]

// Structures

// version 1.1
typedef struct {
	si8	end_time;  // conglomerate end time
	sf4	start_frequency;  // lowest frequency  (in detected bands)
	sf4	end_frequency;  // highest frequency  (in detected bands)
} REC_HFOc_v11_m13;

// version 1.2
typedef struct {
	si8	end_time;  // conglomerate end time
	sf4	start_frequency;  // lowest frequency  (in detected bands)
	sf4	end_frequency;  // highest frequency  (in detected bands)
	si8	start_times[REC_HFOc_NUMBER_OF_BANDS_m13];
	si8	end_times[REC_HFOc_NUMBER_OF_BANDS_m13];
	sf4	combinations[REC_HFOc_NUMBER_OF_BANDS_m13];  // normalized combination scores (0 - 1)
} REC_HFOc_v12_m13;

// version 1.3
typedef struct {
	si8	end_time;  // conglomerate end time
	sf4	start_frequency;  // lowest frequency  (in detected bands)
	sf4	end_frequency;  // highest frequency  (in detected bands)
	si8	start_times[REC_HFOc_NUMBER_OF_BANDS_m13];
	si8	end_times[REC_HFOc_NUMBER_OF_BANDS_m13];
	sf4	combinations[REC_HFOc_NUMBER_OF_BANDS_m13];  // normalized combination scores (0 - 1)
	sf4	amplitudes[REC_HFOc_NUMBER_OF_BANDS_m13];  // normalized amplitude scores (0 - 1)
	sf4	frequency_dominances[REC_HFOc_NUMBER_OF_BANDS_m13];  // normalized frequency dominance scores (0 - 1)
	sf4	products[REC_HFOc_NUMBER_OF_BANDS_m13];  // normalized product scores (0 - 1)
	sf4	cycles[REC_HFOc_NUMBER_OF_BANDS_m13];  // normalized cycles scores (0 - 1)
} REC_HFOc_v13_m13;

// Prototypes
void	REC_show_HFOc_type_m13(REC_HDR_m13 *record_header);




//*************************************************************************************//
//****************   Record structure layouts (compile-time verified)   **************//
//*************************************************************************************//
// these assertions replace the former runtime alignment check functions (removed 2026-07-16):
// a construction error - or a compiler with different packing - fails the BUILD at the exact
// field, at zero runtime cost; one assertion per check the runtime functions performed

// Sgmt
LAYOUT_SIZE_m13(REC_Sgmt_v10_m13, REC_Sgmt_v10_BYTES_m13);
LAYOUT_SIZE_m13(REC_Sgmt_v11_m13, REC_Sgmt_v11_BYTES_m13);
LAYOUT_FIELD_m13(REC_Sgmt_v10_m13, end_time, REC_Sgmt_v10_END_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Sgmt_v10_m13, start_idx, REC_Sgmt_v10_START_IDX_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Sgmt_v10_m13, end_idx, REC_Sgmt_v10_END_IDX_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Sgmt_v10_m13, seg_UID, REC_Sgmt_v10_SEG_UID_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Sgmt_v10_m13, seg_num, REC_Sgmt_v10_SEG_NUM_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Sgmt_v10_m13, acq_chan_num, REC_Sgmt_v10_ACQ_CHAN_NUM_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Sgmt_v10_m13, rate, REC_Sgmt_v10_RATE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Sgmt_v11_m13, end_time, REC_Sgmt_v11_END_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Sgmt_v11_m13, start_idx, REC_Sgmt_v11_START_IDX_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Sgmt_v11_m13, end_idx, REC_Sgmt_v11_END_IDX_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Sgmt_v11_m13, seg_num, REC_Sgmt_v11_SEG_NUM_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Sgmt_v11_m13, rate, REC_Sgmt_v11_RATE_OFFSET_m13);

// Stat
LAYOUT_SIZE_m13(REC_Stat_v10_m13, REC_Stat_v10_BYTES_m13);
LAYOUT_FIELD_m13(REC_Stat_v10_m13, minimum, REC_Stat_v10_MINIMUM_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Stat_v10_m13, maximum, REC_Stat_v10_MAXIMUM_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Stat_v10_m13, mean, REC_Stat_v10_MEAN_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Stat_v10_m13, median, REC_Stat_v10_MEDIAN_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Stat_v10_m13, mode, REC_Stat_v10_MODE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Stat_v10_m13, variance, REC_Stat_v10_VARIANCE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Stat_v10_m13, skewness, REC_Stat_v10_SKEWNESS_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Stat_v10_m13, kurtosis, REC_Stat_v10_KURTOSIS_OFFSET_m13);

// Note
LAYOUT_SIZE_m13(REC_Note_v11_m13, REC_Note_v11_BYTES_m13);
LAYOUT_FIELD_m13(REC_Note_v11_m13, end_time, REC_Note_v11_END_TIME_OFFSET_m13);

// EDFA
LAYOUT_SIZE_m13(REC_EDFA_v10_m13, REC_EDFA_v10_BYTES_m13);
LAYOUT_FIELD_m13(REC_EDFA_v10_m13, duration, REC_EDFA_v10_DURATION_OFFSET_m13);

// Seiz
LAYOUT_SIZE_m13(REC_Seiz_v10_m13, REC_Seiz_v10_BYTES_m13);
LAYOUT_SIZE_m13(REC_Seiz_v10_CHANNEL_m13, REC_Seiz_v10_CHANNEL_BYTES_m13);
LAYOUT_FIELD_m13(REC_Seiz_v10_m13, latest_offset_time, REC_Seiz_v10_LATEST_OFFSET_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Seiz_v10_m13, number_of_channels, REC_Seiz_v10_NUMBER_OF_CHANNELS_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Seiz_v10_m13, onset_code, REC_Seiz_v10_ONSET_CODE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Seiz_v10_m13, marker_name_1, REC_Seiz_v10_MARKER_NAME_1_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Seiz_v10_m13, marker_name_2, REC_Seiz_v10_MARKER_NAME_2_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Seiz_v10_m13, annotation, REC_Seiz_v10_ANNOTATION_OFFSET_m13);

// SyLg

// NlxP
LAYOUT_SIZE_m13(REC_NlxP_v10_m13, REC_NlxP_v10_BYTES_m13);
LAYOUT_FIELD_m13(REC_NlxP_v10_m13, raw_port_value, REC_NlxP_v10_RAW_PORT_VALUE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_NlxP_v10_m13, value, REC_NlxP_v10_VALUE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_NlxP_v10_m13, subport, REC_NlxP_v10_SUBPORT_OFFSET_m13);
LAYOUT_FIELD_m13(REC_NlxP_v10_m13, number_of_subports, REC_NlxP_v10_NUMBER_OF_SUBPORTS_OFFSET_m13);
LAYOUT_FIELD_m13(REC_NlxP_v10_m13, trigger_mode, REC_NlxP_v10_TRIGGER_MODE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_NlxP_v10_m13, pad, REC_NlxP_v10_PAD_OFFSET_m13);

// Curs
LAYOUT_SIZE_m13(REC_Curs_v10_m13, REC_Curs_v10_BYTES_m13);
LAYOUT_FIELD_m13(REC_Curs_v10_m13, id_number, REC_Curs_v10_ID_NUMBER_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Curs_v10_m13, latency, REC_Curs_v10_LATENCY_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Curs_v10_m13, value, REC_Curs_v10_VALUE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Curs_v10_m13, name, REC_Curs_v10_NAME_OFFSET_m13);

// Epoc
LAYOUT_SIZE_m13(REC_Epoc_v10_m13, REC_Epoc_v10_BYTES_m13);
LAYOUT_SIZE_m13(REC_Epoc_v20_m13, REC_Epoc_v20_BYTES_m13);
LAYOUT_FIELD_m13(REC_Epoc_v10_m13, id_number, REC_Epoc_v10_ID_NUMBER_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Epoc_v10_m13, end_time, REC_Epoc_v10_END_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Epoc_v10_m13, epoch_type, REC_Epoc_v10_EPOCH_TYPE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Epoc_v10_m13, text, REC_Epoc_v10_TEXT_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Epoc_v20_m13, end_time, REC_Epoc_v20_END_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Epoc_v20_m13, stage_code, REC_Epoc_v20_STAGE_CODE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Epoc_v20_m13, scorer_id, REC_Epoc_v20_SCORER_ID_OFFSET_m13);

// ESti
LAYOUT_SIZE_m13(REC_ESti_v10_m13, REC_ESti_v10_BYTES_m13);
LAYOUT_FIELD_m13(REC_ESti_v10_m13, amplitude, REC_ESti_v10_AMPLITUDE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_ESti_v10_m13, frequency, REC_ESti_v10_FREQUENCY_OFFSET_m13);
LAYOUT_FIELD_m13(REC_ESti_v10_m13, pulse_width, REC_ESti_v10_PULSE_WIDTH_OFFSET_m13);
LAYOUT_FIELD_m13(REC_ESti_v10_m13, amp_unit_code, REC_ESti_v10_AMP_UNIT_CODE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_ESti_v10_m13, mode_code, REC_ESti_v10_MODE_CODE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_ESti_v10_m13, waveform, REC_ESti_v10_WAVEFORM_OFFSET_m13);
LAYOUT_FIELD_m13(REC_ESti_v10_m13, anode, REC_ESti_v10_ANODE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_ESti_v10_m13, cathode, REC_ESti_v10_CATHODE_OFFSET_m13);

// CSig
LAYOUT_SIZE_m13(REC_CSig_v10_m13, REC_CSig_v10_BYTES_m13);
LAYOUT_FIELD_m13(REC_CSig_v10_m13, target_file_UID, REC_CSig_v10_TARGET_FILE_UID_OFFSET_m13);
LAYOUT_FIELD_m13(REC_CSig_v10_m13, digest_algorithm, REC_CSig_v10_DIGEST_ALGORITHM_OFFSET_m13);
LAYOUT_FIELD_m13(REC_CSig_v10_m13, signature_algorithm, REC_CSig_v10_SIGNATURE_ALGORITHM_OFFSET_m13);
LAYOUT_FIELD_m13(REC_CSig_v10_m13, digest_bytes, REC_CSig_v10_DIGEST_BYTES_OFFSET_m13);
LAYOUT_FIELD_m13(REC_CSig_v10_m13, signature_bytes, REC_CSig_v10_SIGNATURE_BYTES_OFFSET_m13);
LAYOUT_FIELD_m13(REC_CSig_v10_m13, public_key_bytes, REC_CSig_v10_PUBLIC_KEY_BYTES_OFFSET_m13);
LAYOUT_FIELD_m13(REC_CSig_v10_m13, occasion, REC_CSig_v10_OCCASION_OFFSET_m13);
LAYOUT_FIELD_m13(REC_CSig_v10_m13, pad, REC_CSig_v10_PAD_OFFSET_m13);

// CSti
LAYOUT_SIZE_m13(REC_CSti_v10_m13, REC_CSti_v10_BYTES_m13);
LAYOUT_FIELD_m13(REC_CSti_v10_m13, stimulus_duration, REC_CSti_v10_STIMULUS_DURATION_OFFSET_m13);
LAYOUT_FIELD_m13(REC_CSti_v10_m13, task_type, REC_CSti_v10_TASK_TYPE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_CSti_v10_m13, stimulus_type, REC_CSti_v10_STIMULUS_TYPE_OFFSET_m13);
LAYOUT_FIELD_m13(REC_CSti_v10_m13, patient_response, REC_CSti_v10_PATIENT_RESPONSE_OFFSET_m13);

// HFOc
LAYOUT_SIZE_m13(REC_HFOc_v11_m13, REC_HFOc_v11_BYTES_m13);
LAYOUT_SIZE_m13(REC_HFOc_v12_m13, REC_HFOc_v12_BYTES_m13);
LAYOUT_SIZE_m13(REC_HFOc_v13_m13, REC_HFOc_v13_BYTES_m13);
LAYOUT_FIELD_m13(REC_HFOc_v11_m13, end_time, REC_HFOc_v11_END_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v11_m13, start_frequency, REC_HFOc_v11_START_FREQUENCY_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v11_m13, end_frequency, REC_HFOc_v11_END_FREQUENCY_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v12_m13, end_time, REC_HFOc_v12_END_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v12_m13, start_frequency, REC_HFOc_v12_START_FREQUENCY_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v12_m13, end_frequency, REC_HFOc_v12_END_FREQUENCY_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v13_m13, end_time, REC_HFOc_v13_END_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v13_m13, start_frequency, REC_HFOc_v13_START_FREQUENCY_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v13_m13, end_frequency, REC_HFOc_v13_END_FREQUENCY_OFFSET_m13);
// Seiz channel substructure (offsets are substructure-relative)
LAYOUT_FIELD_m13(REC_Seiz_v10_CHANNEL_m13, name, REC_Seiz_v10_CHANNEL_NAME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Seiz_v10_CHANNEL_m13, onset_time, REC_Seiz_v10_CHANNEL_ONSET_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Seiz_v10_CHANNEL_m13, offset_time, REC_Seiz_v10_CHANNEL_OFFSET_TIME_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Seiz_v10_CHANNEL_m13, segment_number, REC_Seiz_v10_CHANNEL_SEGMENT_NUMBER_OFFSET_m13);
LAYOUT_FIELD_m13(REC_Seiz_v10_CHANNEL_m13, pad, REC_Seiz_v10_CHANNEL_PAD_OFFSET_m13);

// HFOc v1.2 / v1.3 band arrays
LAYOUT_FIELD_m13(REC_HFOc_v12_m13, start_times, REC_HFOc_v12_START_TIMES_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v12_m13, end_times, REC_HFOc_v12_END_TIMES_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v12_m13, combinations, REC_HFOc_v12_COMBINATIONS_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v13_m13, start_times, REC_HFOc_v13_START_TIMES_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v13_m13, end_times, REC_HFOc_v13_END_TIMES_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v13_m13, combinations, REC_HFOc_v13_COMBINATIONS_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v13_m13, amplitudes, REC_HFOc_v13_AMPLITUDES_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v13_m13, frequency_dominances, REC_HFOc_v13_FREQUENCY_DOMINANCES_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v13_m13, products, REC_HFOc_v13_PRODUCTS_OFFSET_m13);
LAYOUT_FIELD_m13(REC_HFOc_v13_m13, cycles, REC_HFOc_v13_CYCLES_OFFSET_m13);

#endif // MEDREC_IN_m13

