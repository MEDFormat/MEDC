
//**********************************************************************************//
//***************************  MED 1.1.3 C Library Records  ************************//
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
// with the suffix "_mFL" (for "MED major format 'F', library 'L'").

// MED_FORMAT_VERSION_MAJOR is restricted to single digits 1 through 9
// MED_FORMAT_VERSION_MINOR is restricted to 0 through 254, minor version resets to zero with new major format version
// MED_LIBRARY_VERSION is restricted to 1 through 254, library version resets to one with new major format version

// MED_FULL_FORMAT_NAME == "<MED_VERSION_MAJOR_m13>.<MED_VERSION_MINOR_m13>"
// MED_FULL_LIBRARY_NAME == "<MED_FULL_FORMAT_NAME_m13>.<MED_LIBRARY_VERSION_m13>"
// MED_LIBRARY_TAG == "<MED_VERSION_MAJOR_m13>.<MED_LIBRARY_VERSION_m13>"

// Examples:
// "_m13" indicates "MED format major version 1, library version 1"
// "_m21" indicates "MED format major version 2, library version 1" (for MED 2)
// "_m213" indicates "MED format major version 2, library version 13" (for MED 2)

// All library versions associated with a particular major format version are guaranteed to work on MED files of that major version.
// Minor format versions may add fields to the format in protected regions, but no preexisting fields will be removed or moved.
// Only library versions released on or afer a minor version will make use of new fields, and only if the minor version of the files contains them.
// Backward compatibility will be maintained between major versions if practical.



#include "medlib_m13.h"


//*************************************************************************************//
//**********************************   show_record()   ********************************//
//*************************************************************************************//

tern	REC_show_record_m13(FPS_m13 *fps, RECORD_HEADER_m13 *record_header, si8 record_number)
{
	ui4	type_code;
	si1	time_str[TIME_STRING_BYTES_m13], hex_str[HEX_STRING_BYTES_m13(CRC_BYTES_m13)];

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// decrypt record body if necesary
	if (record_header->encryption_level > NO_ENCRYPTION_m13)
		G_decrypt_record_data_m13(fps, record_header, 1);
		    
	// display record header fields
	if (record_number != REC_NO_RECORD_NUMBER_m13)
		printf_m13("Record Number: %ld\n", record_number);
	printf_m13("---------------- Record Header - START ----------------\n");
	if (record_header->record_CRC == RECORD_HEADER_CRC_NO_ENTRY_m13) {
		printf_m13("Record CRC: no entry\n");
	} else {
		STR_hex_m13((ui1 *) &record_header->record_CRC, CRC_BYTES_m13, hex_str);
		printf_m13("Record CRC: %s\n", hex_str);
	}
	type_code = record_header->type_code;
	if (type_code) {
		STR_hex_m13((ui1 *) record_header->type_string, CRC_BYTES_m13, hex_str);
		printf_m13("Record Type String: %s (%s)\n", record_header->type_string, hex_str);
	} else {
		printf_m13("Record Type String: no entry\n");
	}
	if (record_header->version_major == RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m13 || record_header->version_minor == RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m13) {
		if (record_header->version_major == RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m13)
			printf_m13("Record Version Major: no entry\n");
		else
			printf_m13("Record Version Major: %u\n", record_header->version_major);
		if (record_header->version_minor == RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m13)
			printf_m13("Record Version Minor: no entry\n");
		else
			printf_m13("Record Version Minor: %u\n", record_header->version_minor);
	} else {
		printf_m13("Record Version: %hu.%hu\n", record_header->version_major, record_header->version_minor);
	}
	printf_m13("Record Encryption Level: %hd ", record_header->encryption_level);
	if (record_header->encryption_level == NO_ENCRYPTION_m13)
		printf_m13("(none)\n");
	else if (record_header->encryption_level == LEVEL_1_ENCRYPTION_m13)
		printf_m13("(level 1, currently encrypted)\n");
	else if (record_header->encryption_level == LEVEL_2_ENCRYPTION_m13)
		printf_m13("(level 2, currently encrypted)\n");
	else if (record_header->encryption_level == LEVEL_1_ENCRYPTION_DECRYPTED_m13)
		printf_m13("(level 1, currently decrypted)\n");
	else if (record_header->encryption_level == LEVEL_2_ENCRYPTION_DECRYPTED_m13)
		printf_m13("(level 2, currently decrypted)\n");
	else
		printf_m13("(unrecognized code)\n");
	if (record_header->total_record_bytes == RECORD_HEADER_TOTAL_RECORD_BYTES_NO_ENTRY_m13)
		printf_m13("Record Total Record Bytes: no entry\n");
	else
		printf_m13("Record Total Record Bytes: %u\n", record_header->total_record_bytes);

	if (record_header->start_time == RECORD_HEADER_START_TIME_NO_ENTRY_m13)
		printf_m13("Record Start Time: no entry\n");
	else {
		STR_time_m13((LEVEL_HEADER_m13 *) fps, record_header->start_time, time_str, TRUE_m13, FALSE_m13, FALSE_m13);
		printf_m13("Record Start Time: %ld (oUTC), %s\n", record_header->start_time, time_str);
	}
	printf_m13("----------------- Record Header - END -----------------\n");

	// record body
	printf_m13("----------------- Record Body - START -----------------\n");
	if (record_header->encryption_level > NO_ENCRYPTION_m13) {
		printf_m13("No access to this record\n");
		printf_m13("------------------ Record Body - END ------------------\n\n");
		return_m13(TRUE_m13);
	}

	// pass the display off to custom functions - new records types should be added here (maintain alphabetical order of record types)
	switch (type_code) {
		case REC_Sgmt_TYPE_CODE_m13:
			REC_show_Sgmt_type_m13(record_header);
			break;
		case REC_Stat_TYPE_CODE_m13:
			REC_show_Stat_type_m13(record_header);
			break;
		case REC_Note_TYPE_CODE_m13:
			REC_show_Note_type_m13(record_header);
			break;
		case REC_EDFA_TYPE_CODE_m13:
			REC_show_EDFA_type_m13(record_header);
			break;
		case REC_Seiz_TYPE_CODE_m13:
			REC_show_Seiz_type_m13(record_header);
			break;
		case REC_SyLg_TYPE_CODE_m13:
			REC_show_SyLg_type_m13(record_header);
			break;
		case REC_NlxP_TYPE_CODE_m13:
			REC_show_NlxP_type_m13(record_header);
			break;
		case REC_Curs_TYPE_CODE_m13:
			REC_show_Curs_type_m13(record_header);
			break;
		case REC_Epoc_TYPE_CODE_m13:
			REC_show_Epoc_type_m13(record_header);
			break;
		case REC_ESti_TYPE_CODE_m13:
			REC_show_ESti_type_m13(record_header);
			break;
		case REC_CSti_TYPE_CODE_m13:
			REC_show_CSti_type_m13(record_header);
			break;
		default:
			G_warning_message_m13("%s(): 0x%x is an unrecognized record type code\n", __FUNCTION__, type_code);
			break;
	}
	printf_m13("------------------ Record Body - END ------------------\n\n");

	return_m13(TRUE_m13);
}


//*************************************************************************************//
//*********************   check_record_structure_alignments()   ***********************//
//*************************************************************************************//

tern	REC_check_structure_alignments_m13(ui1 *bytes)
{
	tern	return_value, free_flag = FALSE_m13;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	return_value = TRUE_m13;
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_LARGEST_RECORD_BYTES_m13);
		free_flag = TRUE_m13;
	}

	// check all structures - add new functions here
	if ((REC_check_Sgmt_type_alignment_m13(bytes)) == FALSE_m13)
		return_value = FALSE_m13;
	if ((REC_check_Stat_type_alignment_m13(bytes)) == FALSE_m13)
		return_value = FALSE_m13;
	if ((REC_check_Note_type_alignment_m13(bytes)) == FALSE_m13)
		return_value = FALSE_m13;
	if ((REC_check_EDFA_type_alignment_m13(bytes)) == FALSE_m13)
		return_value = FALSE_m13;
	if ((REC_check_Seiz_type_alignment_m13(bytes)) == FALSE_m13)
		return_value = FALSE_m13;
	if ((REC_check_SyLg_type_alignment_m13(bytes)) == FALSE_m13)
		return_value = FALSE_m13;
	if ((REC_check_NlxP_type_alignment_m13(bytes)) == FALSE_m13)
		return_value = FALSE_m13;
	if ((REC_check_Curs_type_alignment_m13(bytes)) == FALSE_m13)
		return_value = FALSE_m13;
	if ((REC_check_Epoc_type_alignment_m13(bytes)) == FALSE_m13)
		return_value = FALSE_m13;
	if ((REC_check_ESti_type_alignment_m13(bytes)) == FALSE_m13)
		return_value = FALSE_m13;
	if ((REC_check_CSti_type_alignment_m13(bytes)) == FALSE_m13)
		return_value = FALSE_m13;

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	if (return_value == FALSE_m13)
		G_set_error_m13(E_REC_m13, "one or more record structures are NOT aligned");

	return_m13(return_value);
}


//*************************************************************************************//
//*******************************   Sgmt: Segment Record   ****************************//
//*************************************************************************************//

tern	REC_show_Sgmt_type_m13(RECORD_HEADER_m13 *record_header)
{
	REC_Sgmt_v10_m13	*Sgmt_v10;
	REC_Sgmt_v11_m13	*Sgmt_v11;
	si1                     time_str[TIME_STRING_BYTES_m13], hex_str[HEX_STRING_BYTES_m13(8)], *segment_description;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		Sgmt_v10 = (REC_Sgmt_v10_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);

		STR_time_m13(NULL, Sgmt_v10->end_time, time_str, TRUE_m13, FALSE_m13, FALSE_m13);
		printf_m13("End Time: %ld (oUTC), %s\n", Sgmt_v10->end_time, time_str);
		if (Sgmt_v10->start_sample_number == REC_Sgmt_v10_START_SAMPLE_NUMBER_NO_ENTRY_m13)
			printf_m13("Start Sample Number: no entry\n");
		else
			printf_m13("Start Sample Number: %ld\n", Sgmt_v10->start_sample_number);
		if (Sgmt_v10->end_sample_number == REC_Sgmt_v10_END_SAMPLE_NUMBER_NO_ENTRY_m13)
			printf_m13("End Sample Number: no entry\n");
		else
			printf_m13("End Sample Number: %ld\n", Sgmt_v10->end_sample_number);
		STR_hex_m13((ui1 *) &Sgmt_v10->segment_UID, 8, hex_str);
		printf_m13("Segment UID: %s\n", hex_str);
		if (Sgmt_v10->segment_number == REC_Sgmt_v10_SEGMENT_NUMBER_NO_ENTRY_m13)
			printf_m13("Segment Number: no entry\n");
		else
			printf_m13("Segment Number: %d\n", Sgmt_v10->segment_number);

		if (Sgmt_v10->acquisition_channel_number == REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_ALL_CHANNELS_m13)
			printf_m13("Acquisition Channel Number: all channels\n");
		else if (Sgmt_v10->acquisition_channel_number == REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m13)
			printf_m13("Acquisition Channel Number: no entry\n");
		else
			printf_m13("Acquisition Channel Number: %d\n", Sgmt_v10->acquisition_channel_number);

		if (Sgmt_v10->sampling_frequency == REC_Sgmt_v10_SAMPLING_FREQUENCY_NO_ENTRY_m13)
			printf_m13("Sampling Frequency: no entry\n");
		else if (Sgmt_v10->sampling_frequency == REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m13)
			printf_m13("Sampling Frequency: variable\n");
		else
			printf_m13("Sampling Frequency: %lf\n", Sgmt_v10->sampling_frequency);
		if (record_header->total_record_bytes > (RECORD_HEADER_BYTES_m13 + REC_Sgmt_v10_BYTES_m13)) {
			segment_description = (si1 *) Sgmt_v10 + REC_Sgmt_v10_SEGMENT_DESCRIPTION_OFFSET_m13;
			if (*segment_description)
				UTF8_printf_m13("Segment Description: %s\n", segment_description);
			else
				printf_m13("Segment Description: no entry\n");
		} else {
			printf_m13("Segment Description: no entry\n");
		}
	}
	// Version 1.1
	if (record_header->version_major == 1 && record_header->version_minor == 1) {
		Sgmt_v11 = (REC_Sgmt_v11_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);

		STR_time_m13(NULL, Sgmt_v11->end_time, time_str, TRUE_m13, FALSE_m13, FALSE_m13);
		printf_m13("End Time: %ld (oUTC), %s\n", Sgmt_v11->end_time, time_str);
		if (Sgmt_v11->start_sample_number == REC_Sgmt_v11_START_SAMPLE_NUMBER_NO_ENTRY_m13)
			printf_m13("Start Sample Number: no entry\n");
		else
			printf_m13("Start Sample Number: %ld\n", Sgmt_v11->start_sample_number);
		if (Sgmt_v11->end_sample_number == REC_Sgmt_v11_END_SAMPLE_NUMBER_NO_ENTRY_m13)
			printf_m13("End Sample Number: no entry\n");
		else
			printf_m13("End Sample Number: %ld\n", Sgmt_v11->end_sample_number);
		if (Sgmt_v11->segment_number == REC_Sgmt_v11_SEGMENT_NUMBER_NO_ENTRY_m13)
			printf_m13("Segment Number: no entry\n");
		else
			printf_m13("Segment Number: %d\n", Sgmt_v11->segment_number);
		if (record_header->total_record_bytes > (RECORD_HEADER_BYTES_m13 + REC_Sgmt_v11_BYTES_m13)) {
			segment_description = (si1 *) Sgmt_v11 + REC_Sgmt_v11_SEGMENT_DESCRIPTION_OFFSET_m13;
			if (*segment_description)
				UTF8_printf_m13("Segment Description: %s\n", segment_description);
			else
				printf_m13("Segment Description: no entry\n");
		} else {
			printf_m13("Segment Description: no entry\n");
		}
	}
	// Unrecognized record version
	else {
		G_set_error_m13(E_REC_m13, "unrecognized Sgmt Record version (%hhd.%hhd)", record_header->version_major, record_header->version_minor);
	}

	return_m13(TRUE_m13);
}


tern     REC_check_Sgmt_type_alignment_m13(ui1 *bytes)
{
	si1			*version_string;
	REC_Sgmt_v10_m13	*Sgmt_v10;
	REC_Sgmt_v11_m13	*Sgmt_v11;
	tern			free_flag = FALSE_m13;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// check overall size
	if (sizeof(REC_Sgmt_v10_m13) != REC_Sgmt_v10_BYTES_m13)
		goto REC_Sgmt_NOT_ALIGNED_m13;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_Sgmt_v10_BYTES_m13);  // REC_Sgmt_v10_BYTES_m13 larger than REC_Sgmt_v11_BYTES_m13
		free_flag = TRUE_m13;
	}
	
	// Version 1.0
	Sgmt_v10 = (REC_Sgmt_v10_m13 *) bytes;
	version_string = "version 1.0";
	if (&Sgmt_v10->end_time != (si8 *) (bytes + REC_Sgmt_v10_END_TIME_OFFSET_m13))
		goto REC_Sgmt_NOT_ALIGNED_m13;
	if (&Sgmt_v10->start_sample_number != (si8 *) (bytes + REC_Sgmt_v10_START_SAMPLE_NUMBER_OFFSET_m13))
		goto REC_Sgmt_NOT_ALIGNED_m13;
	if (&Sgmt_v10->end_sample_number != (si8 *) (bytes + REC_Sgmt_v10_END_SAMPLE_NUMBER_OFFSET_m13))
		goto REC_Sgmt_NOT_ALIGNED_m13;
	if (&Sgmt_v10->segment_UID != (ui8 *) (bytes + REC_Sgmt_v10_SEGMENT_UID_OFFSET_m13))
		goto REC_Sgmt_NOT_ALIGNED_m13;
	if (&Sgmt_v10->segment_number != (si4 *) (bytes + REC_Sgmt_v10_SEGMENT_NUMBER_OFFSET_m13))
		goto REC_Sgmt_NOT_ALIGNED_m13;
	if (&Sgmt_v10->acquisition_channel_number != (si4 *) (bytes + REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_OFFSET_m13))
		goto REC_Sgmt_NOT_ALIGNED_m13;
	if (&Sgmt_v10->sampling_frequency != (sf8 *) (bytes + REC_Sgmt_v10_SAMPLING_FREQUENCY_OFFSET_m13))
		goto REC_Sgmt_NOT_ALIGNED_m13;

	// Version 1.1
	Sgmt_v11 = (REC_Sgmt_v11_m13 *) bytes;
	version_string = "version 1.1";
	if (&Sgmt_v11->end_time != (si8 *) (bytes + REC_Sgmt_v11_END_TIME_OFFSET_m13))
		goto REC_Sgmt_NOT_ALIGNED_m13;
	if (&Sgmt_v11->start_sample_number != (si8 *) (bytes + REC_Sgmt_v11_START_SAMPLE_NUMBER_OFFSET_m13))
		goto REC_Sgmt_NOT_ALIGNED_m13;
	if (&Sgmt_v11->end_sample_number != (si8 *) (bytes + REC_Sgmt_v11_END_SAMPLE_NUMBER_OFFSET_m13))
		goto REC_Sgmt_NOT_ALIGNED_m13;
	if (&Sgmt_v11->segment_number != (si4 *) (bytes + REC_Sgmt_v11_SEGMENT_NUMBER_OFFSET_m13))
		goto REC_Sgmt_NOT_ALIGNED_m13;
	if (Sgmt_v11->pad != bytes + REC_Sgmt_v11_PAD_OFFSET_m13)
		goto REC_Sgmt_NOT_ALIGNED_m13;
	
	// aligned
	if (free_flag == TRUE_m13)
		free((void *) bytes);

	return_m13(TRUE_m13);

	// not aligned
REC_Sgmt_NOT_ALIGNED_m13:

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	G_set_error_m13(E_REC_m13, "Sgmt %s structure is NOT aligned", version_string);

	return_m13(FALSE_m13);

}


//*************************************************************************************//
//*******************************   Stat: Segment Record   ****************************//
//*************************************************************************************//

tern    REC_show_Stat_type_m13(RECORD_HEADER_m13 *record_header)
{
	REC_Stat_v10_m13	*Stat;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		Stat = (REC_Stat_v10_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
		if (Stat->minimum == REC_Stat_v10_MINIMUM_NO_ENTRY_m13)
			printf_m13("Minimum: no entry\n");
		else
			printf_m13("Minimum: %d\n", Stat->minimum);
		if (Stat->maximum == REC_Stat_v10_MAXIMUM_NO_ENTRY_m13)
			printf_m13("Maximum: no entry\n");
		else
			printf_m13("Maximum: %d\n", Stat->maximum);
		if (Stat->mean == REC_Stat_v10_MEAN_NO_ENTRY_m13)
			printf_m13("Mean: no entry\n");
		else
			printf_m13("Mean: %d\n", Stat->mean);
		if (Stat->median == REC_Stat_v10_MEDIAN_NO_ENTRY_m13)
			printf_m13("Median: no entry\n");
		else
			printf_m13("Median: %d\n", Stat->median);
		if (Stat->mode == REC_Stat_v10_MODE_NO_ENTRY_m13)
			printf_m13("Mode: no entry\n");
		else
			printf_m13("Mode: %d\n", Stat->mode);
		if (isnan(Stat->variance))
			printf_m13("Variance: no entry\n");
		else
			printf_m13("Variance: %f\n", Stat->variance);
		if (isnan(Stat->skewness))
			printf_m13("Skewness: no entry\n");
		else
			printf_m13("Skewness: %f\n", Stat->skewness);
		if (isnan(Stat->kurtosis))
			printf_m13("Kurtosis: no entry\n");
		else
			printf_m13("Kurtosis: %f\n", Stat->kurtosis);
	}
	// Unrecognized record version
	else {
		G_set_error_m13(E_REC_m13, "unrecognized Stat record version (%hhd.%hhd)", record_header->version_major, record_header->version_minor);
	}

	return_m13(TRUE_m13);

}


tern     REC_check_Stat_type_alignment_m13(ui1 *bytes)
{
	REC_Stat_v10_m13	*Stat;
	tern			free_flag = FALSE_m13;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// check overall size
	if (sizeof(REC_Stat_v10_m13) != REC_Stat_v10_BYTES_m13)
		goto REC_Stat_NOT_ALIGNED_m13;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_Stat_v10_BYTES_m13);
		free_flag = TRUE_m13;
	}
	Stat = (REC_Stat_v10_m13 *) bytes;
	if (&Stat->minimum != (si4 *) (bytes + REC_Stat_v10_MINIMUM_OFFSET_m13))
		goto REC_Stat_NOT_ALIGNED_m13;
	if (&Stat->maximum != (si4 *) (bytes + REC_Stat_v10_MAXIMUM_OFFSET_m13))
		goto REC_Stat_NOT_ALIGNED_m13;
	if (&Stat->mean != (si4 *) (bytes + REC_Stat_v10_MEAN_OFFSET_m13))
		goto REC_Stat_NOT_ALIGNED_m13;
	if (&Stat->median != (si4 *) (bytes + REC_Stat_v10_MEDIAN_OFFSET_m13))
		goto REC_Stat_NOT_ALIGNED_m13;
	if (&Stat->mode != (si4 *) (bytes + REC_Stat_v10_MODE_OFFSET_m13))
		goto REC_Stat_NOT_ALIGNED_m13;
	if (&Stat->variance != (sf4 *) (bytes + REC_Stat_v10_VARIANCE_OFFSET_m13))
		goto REC_Stat_NOT_ALIGNED_m13;
	if (&Stat->skewness != (sf4 *) (bytes + REC_Stat_v10_SKEWNESS_OFFSET_m13))
		goto REC_Stat_NOT_ALIGNED_m13;
	if (&Stat->kurtosis != (sf4 *) (bytes + REC_Stat_v10_KURTOSIS_OFFSET_m13))
		goto REC_Stat_NOT_ALIGNED_m13;

	// aligned
	if (free_flag == TRUE_m13)
		free((void *) bytes);

	return_m13(TRUE_m13);

	// not aligned
REC_Stat_NOT_ALIGNED_m13:

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	G_set_error_m13(E_REC_m13, "Stat structure is NOT aligned");

	return_m13(FALSE_m13);
}


//*************************************************************************************//
//********************************   Note: Note Record   ******************************//
//*************************************************************************************//

tern	REC_show_Note_type_m13(RECORD_HEADER_m13 *record_header)
{
	si1			*note_text;
	REC_Note_v11_m13	*note;
	
#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		if (record_header->total_record_bytes > RECORD_HEADER_BYTES_m13) {
			note_text = (si1 *) record_header + RECORD_HEADER_BYTES_m13;
			if (*note_text)
				UTF8_printf_m13("Note Text: %s\n", note_text);
			else
				printf_m13("Note Text: no entry\n");
		} else {
			printf_m13("Note Text: no entry\n");
		}
	}
	// Version 1.1
	else if (record_header->version_major == 1 && record_header->version_minor == 1) {
		note = (REC_Note_v11_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
		if (note->end_time <= 0)  // covers zero & UUTC_NO_ENTRY_m13
			printf_m13("End Time: no entry\n");
		else
			printf_m13("End Time: %ld\n", note->end_time);
		note_text = (si1 *) ((ui1 *) note + REC_Note_v11_TEXT_OFFSET_m13);
		if (*note_text)
			UTF8_printf_m13("Text: %s\n", note_text);
		else
			printf_m13("Text: no entry\n");
	}
	// Unrecognized record version
	else {
		G_set_error_m13(E_REC_m13, "unrecognized Note record version (%hhd.%hhd)", record_header->version_major, record_header->version_minor);
	}

	return_m13(TRUE_m13);
}


tern        REC_check_Note_type_alignment_m13(ui1 *bytes)
{
	REC_Note_v11_m13	*note;
	const si1		*vers_str;
	tern			free_flag = FALSE_m13;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// Version 1.0
	// no structure for REC_Note_v10_m13
	
	// Version 1.1
	vers_str = "REC_Note_v11_m13";

	// check overall size
	if (sizeof(REC_Note_v11_m13) != REC_Note_v11_BYTES_m13)
		goto REC_NOTE_NOT_ALIGNED_m13;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc((size_t) REC_Note_v11_BYTES_m13);
		free_flag = TRUE_m13;
	}
	note = (REC_Note_v11_m13 *) bytes;
	if (&note->end_time != (si8 *) (bytes + REC_Note_v11_END_TIME_OFFSET_m13))
		goto REC_NOTE_NOT_ALIGNED_m13;
	if (note->text != (si1 *) (bytes + REC_Note_v11_TEXT_OFFSET_m13))
		goto REC_NOTE_NOT_ALIGNED_m13;

	// aligned
	if (free_flag == TRUE_m13)
		free((void *) bytes);

	return_m13(TRUE_m13);

	// not aligned
REC_NOTE_NOT_ALIGNED_m13:

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	G_set_error_m13(E_REC_m13, "Note %s structure is NOT aligned\n", vers_str);

	return_m13(FALSE_m13);
}


//*************************************************************************************//
//******************   EDFA: European Data Format Annotation Record   *****************//
//*************************************************************************************//

tern	REC_show_EDFA_type_m13(RECORD_HEADER_m13 *record_header)
{
	REC_EDFA_v10_m13	*edfa;
	si1			*annotation;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		edfa = (REC_EDFA_v10_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
		printf_m13("Duration %ld microseconds\n", edfa->duration);
		annotation = (si1 *) edfa + REC_EDFA_v10_ANNOTATION_OFFSET_m13;
		if (*annotation)
			UTF8_printf_m13("Annotation: %s\n", annotation);
		else
			printf_m13("Annotation: no entry\n");
	}
	// Unrecognized record version
	else {
		G_set_error_m13(E_REC_m13, "unrecognized EDFA record version (%hhd.%hhd)", record_header->version_major, record_header->version_minor);
	}

	return_m13(TRUE_m13);
}


tern	REC_check_EDFA_type_alignment_m13(ui1 *bytes)
{
	REC_EDFA_v10_m13	*edfa;
	tern			free_flag = FALSE_m13;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// check overall size
	if (sizeof(REC_EDFA_v10_m13) != REC_EDFA_v10_BYTES_m13)
		goto REC_EDFA_NOT_ALIGNED_m13;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_EDFA_v10_BYTES_m13);
		free_flag = TRUE_m13;
	}
	edfa = (REC_EDFA_v10_m13 *) bytes;
	if (&edfa->duration != (si8 *) (bytes + REC_EDFA_v10_DURATION_OFFSET_m13))
		goto REC_EDFA_NOT_ALIGNED_m13;

	// aligned
	if (free_flag == TRUE_m13)
		free((void *) bytes);

	return_m13(TRUE_m13);

	// not aligned
REC_EDFA_NOT_ALIGNED_m13:

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	G_set_error_m13(E_REC_m13, "EDFA structure is NOT aligned");

	return_m13(FALSE_m13);
}



//*************************************************************************************//
//*******************************   Seiz: Seizure Record   ****************************//
//*************************************************************************************//

tern	REC_show_Seiz_type_m13(RECORD_HEADER_m13 *record_header)
{
	tern				mn1 = FALSE_m13, mn2 = FALSE_m13;
	si4			        i;
	REC_Seiz_v10_m13		*Seiz;
	REC_Seiz_v10_CHANNEL_m13	*chans;
	si1			        time_str[TIME_STRING_BYTES_m13];
	PROC_GLOBALS_m13		*proc_globals;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		proc_globals = G_proc_globals_m13(NULL);
		Seiz = (REC_Seiz_v10_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
		STR_time_m13(NULL, Seiz->latest_offset_time, time_str, TRUE_m13, FALSE_m13, FALSE_m13);
		printf_m13("Latest Offset Time: %ld (oUTC), %ld (µUTC), %s\n", Seiz->latest_offset_time, Seiz->latest_offset_time + proc_globals->time_constants.recording_time_offset, time_str);
		printf_m13("Number of Channels: %d\n", Seiz->number_of_channels);
		printf_m13("Onset Code: %d ", Seiz->onset_code);
		switch (Seiz->onset_code) {
		case REC_Seiz_v10_ONSET_NO_ENTRY_m13:
			printf_m13("(no entry)\n");
			break;
		case REC_Seiz_v10_ONSET_UNKNOWN_m13:
			printf_m13("(unknown)\n");
			break;
		case REC_Seiz_v10_ONSET_FOCAL_m13:
			printf_m13("(focal)\n");
			break;
		case REC_Seiz_v10_ONSET_GENERALIZED_m13:
			printf_m13("(generalized)\n");
			break;
		case REC_Seiz_v10_ONSET_PROPAGATED_m13:
			printf_m13("(propagated)\n");
			break;
		case REC_Seiz_v10_ONSET_MIXED_m13:
			printf_m13("(mixed)\n");
			break;
		default:
			G_warning_message_m13("%s(): %d is an unrecognized Seiz onset code", __FUNCTION__, Seiz->onset_code);
			break;
		}
		if (strlen(Seiz->marker_name_1))
			mn1 = TRUE_m13;
		if (strlen(Seiz->marker_name_2))
			mn2 = TRUE_m13;
		if (mn1 == TRUE_m13 && mn2 == TRUE_m13)
			UTF8_printf_m13("Marker Names: %s %s\n", Seiz->marker_name_1, Seiz->marker_name_2);
		else if (mn1 == TRUE_m13)
			UTF8_printf_m13("Marker Name 1: %s\nMarker Name 2: no entry\n", Seiz->marker_name_1);
		else if (mn2 == TRUE_m13)
			UTF8_printf_m13("Marker Name 1: no_entry\nMarker Name 2: %s\n", Seiz->marker_name_2);
		else
			printf_m13("Marker Names: no_entry\n");
		if (strlen(Seiz->annotation))
			UTF8_printf_m13("Annotation: %s\n", Seiz->annotation);
		else
			printf_m13("Annotation: no entry\n");
		chans = (REC_Seiz_v10_CHANNEL_m13 *) ((ui1 *) Seiz + REC_Seiz_v10_CHANNELS_OFFSET_m13);
		for (i = 0; i < Seiz->number_of_channels; ++i) {
			if (strlen(chans[i].name))
				UTF8_printf_m13("Channel Name: %s\n", chans[i].name);
			else
				printf_m13("Channel Name: no entry\n");
			STR_time_m13(NULL, chans[i].onset_time, time_str, TRUE_m13, FALSE_m13, FALSE_m13);
			printf_m13("\tOnset Time: %ld (oUTC), %ld (µUTC), %s\n", chans[i].onset_time, chans[i].onset_time + proc_globals->time_constants.recording_time_offset, time_str);
			STR_time_m13(NULL, chans[i].offset_time, time_str, TRUE_m13, FALSE_m13, FALSE_m13);
			printf_m13("\tOffset Time: %ld (oUTC), %ld (µUTC), %s\n", chans[i].offset_time, chans[i].offset_time + proc_globals->time_constants.recording_time_offset, time_str);
			if (chans[i].segment_number == REC_Seiz_v10_CHANNEL_SEGMENT_NUMBER_NO_ENTRY_m13)
				printf_m13("Segment Number: no entry\n");
			else
				printf_m13("Segment Number: %d\n", chans[i].segment_number);
		}
	}
	// Unrecognized record version
	else {
		G_set_error_m13(E_REC_m13, "unrecognized Seiz record version (%hhd.%hhd)", record_header->version_major, record_header->version_minor);
	}

	return_m13(TRUE_m13);
}


tern	REC_check_Seiz_type_alignment_m13(ui1 *bytes)
{
	REC_Seiz_v10_m13		*Seiz;
	REC_Seiz_v10_CHANNEL_m13	*chan;
	tern				free_flag = FALSE_m13;
	ui1				*chan_bytes;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// check overall sizes
	if (sizeof(REC_Seiz_v10_m13) != REC_Seiz_v10_BYTES_m13)
		goto REC_Seiz_NOT_ALIGNED_m13;
	if (sizeof(REC_Seiz_v10_CHANNEL_m13) != REC_Seiz_v10_CHANNEL_BYTES_m13)
		goto REC_Seiz_NOT_ALIGNED_m13;

	// check fields - base structure
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_Seiz_v10_BYTES_m13 + REC_Seiz_v10_CHANNEL_BYTES_m13);
		free_flag = TRUE_m13;
	}
	Seiz = (REC_Seiz_v10_m13 *) bytes;
	if (&Seiz->latest_offset_time != (si8 *) (bytes + REC_Seiz_v10_LATEST_OFFSET_TIME_OFFSET_m13))
		goto REC_Seiz_NOT_ALIGNED_m13;
	if (&Seiz->number_of_channels != (si4 *) (bytes + REC_Seiz_v10_NUMBER_OF_CHANNELS_OFFSET_m13))
		goto REC_Seiz_NOT_ALIGNED_m13;
	if (&Seiz->onset_code != (si4 *) (bytes + REC_Seiz_v10_ONSET_CODE_OFFSET_m13))
		goto REC_Seiz_NOT_ALIGNED_m13;
	if (Seiz->marker_name_1 != (si1 *) (bytes + REC_Seiz_v10_MARKER_NAME_1_OFFSET_m13))
		goto REC_Seiz_NOT_ALIGNED_m13;
	if (Seiz->marker_name_2 != (si1 *) (bytes + REC_Seiz_v10_MARKER_NAME_2_OFFSET_m13))
		goto REC_Seiz_NOT_ALIGNED_m13;
	if (Seiz->annotation != (si1 *) (bytes + REC_Seiz_v10_ANNOTATION_OFFSET_m13))
		goto REC_Seiz_NOT_ALIGNED_m13;
	// check fields - channel structures
	chan_bytes = bytes + REC_Seiz_v10_CHANNELS_OFFSET_m13;
	chan = (REC_Seiz_v10_CHANNEL_m13 *) chan_bytes;
	if (chan->name != (si1 *) (chan_bytes + REC_Seiz_v10_CHANNEL_NAME_OFFSET_m13))
		goto REC_Seiz_NOT_ALIGNED_m13;
	if (&chan->onset_time != (si8 *) (chan_bytes + REC_Seiz_v10_CHANNEL_ONSET_TIME_OFFSET_m13))
		goto REC_Seiz_NOT_ALIGNED_m13;
	if (&chan->offset_time != (si8 *) (chan_bytes + REC_Seiz_v10_CHANNEL_OFFSET_TIME_OFFSET_m13))
		goto REC_Seiz_NOT_ALIGNED_m13;
	if (&chan->segment_number != (si4 *) (chan_bytes + REC_Seiz_v10_CHANNEL_SEGMENT_NUMBER_OFFSET_m13))
		goto REC_Seiz_NOT_ALIGNED_m13;
	if (chan->pad != (ui1 *) (chan_bytes + REC_Seiz_v10_CHANNEL_PAD_OFFSET_m13))
		goto REC_Seiz_NOT_ALIGNED_m13;

	// aligned
	if (free_flag == TRUE_m13)
		free((void *) bytes);

	return_m13(TRUE_m13);

	// not aligned
REC_Seiz_NOT_ALIGNED_m13:

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	G_set_error_m13(E_REC_m13, "Seiz structure is NOT aligned");

	return_m13(FALSE_m13);
}


//*************************************************************************************//
//*****************************   SyLg: System Log Record   ***************************//
//*************************************************************************************//

tern	REC_show_SyLg_type_m13(RECORD_HEADER_m13 *record_header)
{
	si1	*log_entry;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		log_entry = (si1 *) record_header + RECORD_HEADER_BYTES_m13;
		if (*log_entry)
			UTF8_printf_m13("System Log entry:\n%s\n", log_entry);
		else
			printf_m13("System Log entry: no entry\n");
	}
	// Unrecognized record version
	else {
		G_set_error_m13(E_REC_m13, "unrecognized SyLg Record version (%hhd.%hhd)", record_header->version_major, record_header->version_minor);
	}

	return_m13(TRUE_m13);
}


tern	REC_check_SyLg_type_alignment_m13(ui1 *bytes)
{
#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// no structures to check
	return_m13(TRUE_m13);
}



//*************************************************************************************//
//*********************   NlxP: NeuraLynx Parallel Port Record   **********************//
//*************************************************************************************//

tern	REC_show_NlxP_type_m13(RECORD_HEADER_m13 *record_header)
{
	si1                     hex_str[HEX_STRING_BYTES_m13(4)];
	REC_NlxP_v10_m13	*nlxp;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		nlxp = (REC_NlxP_v10_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
		printf_m13("Value: %u\n", nlxp->value);
		printf_m13("Subport: %hhu\n", nlxp->subport);
		printf_m13("Number of Subports: %hhu\n", nlxp->number_of_subports);
		printf_m13("Trigger Mode: ");
		switch (nlxp->trigger_mode) {
		case REC_NlxP_v10_NO_TRIGGER_m13:
			printf_m13("NO TRIGGER\n");
			break;
		case REC_NlxP_v10_ANY_BIT_CHANGE_m13:
			printf_m13("ANY BIT CHANGE\n");
			break;
		case REC_NlxP_v10_HIGH_BIT_SET_m13:
			printf_m13("HIGH BIT SET\n");
			break;
		default:
			G_warning_message_m13("%s(): Unrecognized trigger mode (%hhu)", __FUNCTION__, nlxp->trigger_mode);
			break;
		}
		printf_m13("Raw Port Value: %u  (unsigned dec)\n", nlxp->raw_port_value);
		STR_hex_m13((ui1 *) &nlxp->raw_port_value, 4, hex_str);
		printf_m13("Raw Port Bytes: %s  (hex)\n", hex_str);
	}
	// Unrecognized record version
	else {
		G_set_error_m13(E_REC_m13, "unrecognized NlxP record version (%hhd.%hhd)", record_header->version_major, record_header->version_minor);
	}

	return_m13(TRUE_m13);
}


tern     REC_check_NlxP_type_alignment_m13(ui1 *bytes)
{
	REC_NlxP_v10_m13	*nlxp;
	tern			free_flag = FALSE_m13;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// check overall size
	if (sizeof(REC_NlxP_v10_m13) != REC_NlxP_v10_BYTES_m13)
		goto REC_NlxP_NOT_ALIGNED_m13;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_NlxP_v10_BYTES_m13);
		free_flag = TRUE_m13;
	}

	nlxp = (REC_NlxP_v10_m13 *) bytes;
	if (&nlxp->raw_port_value != (ui4 *) (bytes + REC_NlxP_v10_RAW_PORT_VALUE_OFFSET_m13))
		goto REC_NlxP_NOT_ALIGNED_m13;
	if (&nlxp->value != (ui4 *) (bytes + REC_NlxP_v10_VALUE_OFFSET_m13))
		goto REC_NlxP_NOT_ALIGNED_m13;
	if (&nlxp->subport != (ui1 *) (bytes + REC_NlxP_v10_SUBPORT_OFFSET_m13))
		goto REC_NlxP_NOT_ALIGNED_m13;
	if (&nlxp->number_of_subports != (ui1 *) (bytes + REC_NlxP_v10_NUMBER_OF_SUBPORTS_OFFSET_m13))
		goto REC_NlxP_NOT_ALIGNED_m13;
	if (&nlxp->trigger_mode != (ui1 *) (bytes + REC_NlxP_v10_TRIGGER_MODE_OFFSET_m13))
		goto REC_NlxP_NOT_ALIGNED_m13;
	if (nlxp->pad != (ui1 *) (bytes + REC_NlxP_v10_PAD_OFFSET_m13))
		goto REC_NlxP_NOT_ALIGNED_m13;

	// aligned
	if (free_flag == TRUE_m13)
		free((void *) bytes);

	return_m13(TRUE_m13);

	// not aligned
REC_NlxP_NOT_ALIGNED_m13:

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	G_set_error_m13(E_REC_m13, "NlxP structure is NOT aligned");

	return_m13(FALSE_m13);

}


//*************************************************************************************//
//***********************   Curs: Cadwell EMG Cursor Annotation   *********************//
//*************************************************************************************//

tern    REC_show_Curs_type_m13(RECORD_HEADER_m13 *record_header)
{
	REC_Curs_v10_m13	*curs;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		curs = (REC_Curs_v10_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
		printf_m13("ID Number: %ld\n", curs->id_number);
		printf_m13("Latency: %ld\n", curs->latency);
		printf_m13("Value: %lf\n", curs->value);
		UTF8_printf_m13("Name: %s\n", curs->name);
	}
	// Unrecognized record version
	else {
		G_set_error_m13(E_REC_m13, "unrecognized Curs record version (%hhd.%hhd)", record_header->version_major, record_header->version_minor);
	}

	return_m13(TRUE_m13);
}


tern     REC_check_Curs_type_alignment_m13(ui1 *bytes)
{
	REC_Curs_v10_m13	*curs;
	tern			free_flag = FALSE_m13;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// check overall size
	if (sizeof(REC_Curs_v10_m13) != REC_Curs_v10_BYTES_m13)
		goto REC_Curs_NOT_ALIGNED_m13;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1*) malloc(REC_Curs_v10_BYTES_m13);
		free_flag = TRUE_m13;
	}

	curs = (REC_Curs_v10_m13 *) bytes;
	if (&curs->id_number != (si8 *) (bytes + REC_Curs_v10_ID_NUMBER_OFFSET_m13))
		goto REC_Curs_NOT_ALIGNED_m13;
	if (&curs->latency != (si8 *) (bytes + REC_Curs_v10_LATENCY_OFFSET_m13))
		goto REC_Curs_NOT_ALIGNED_m13;
	if (&curs->value != (sf8 *) (bytes + REC_Curs_v10_VALUE_OFFSET_m13))
		goto REC_Curs_NOT_ALIGNED_m13;
	if (curs->name != (si1 *) (bytes + REC_Curs_v10_NAME_OFFSET_m13))
		goto REC_Curs_NOT_ALIGNED_m13;

	// aligned
	if (free_flag == TRUE_m13)
		free((void *) bytes);

	return_m13(TRUE_m13);

	// not aligned
REC_Curs_NOT_ALIGNED_m13:

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	G_set_error_m13(E_REC_m13, "Curs structure is NOT aligned");

	return_m13(FALSE_m13);
}


//*************************************************************************************//
//****************************   Epoc: Sleep Stage Record   ***************************//
//*************************************************************************************//

tern    REC_show_Epoc_type_m13(RECORD_HEADER_m13 *record_header)
{
	REC_Epoc_v10_m13	*epoc1;
	REC_Epoc_v20_m13	*epoc2;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		epoc1 = (REC_Epoc_v10_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
		printf_m13("ID Number: %ld\n", epoc1->id_number);
		printf_m13("End Time: %ld\n", epoc1->end_time);
		UTF8_printf_m13("Epoch Type: %s\n", epoc1->epoch_type);
		UTF8_printf_m13("Text: %s\n", epoc1->text);
	}
	// Version 2.0
	else if (record_header->version_major == 2 && record_header->version_minor == 0) {
		epoc2 = (REC_Epoc_v20_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
		printf_m13("ID Number: %ld\n", epoc2->end_time);
		printf_m13("Stage: ");
		switch (epoc2->stage_code) {
			case REC_Epoc_v20_STAGE_AWAKE_m13:
				printf_m13("awake\n");
				break;
			case REC_Epoc_v20_STAGE_NREM_1_m13:
				printf_m13("non-REM 1\n");
				break;
			case REC_Epoc_v20_STAGE_NREM_2_m13:
				printf_m13("non-REM 2\n");
				break;
			case REC_Epoc_v20_STAGE_NREM_3_m13:
				printf_m13("non-REM 3\n");
				break;
			case REC_Epoc_v20_STAGE_NREM_4_m13:
				printf_m13("non-REM 4\n");
				break;
			case REC_Epoc_v20_STAGE_REM_m13:
				printf_m13("REM\n");
				break;
			case REC_Epoc_v20_STAGE_UNKNOWN_m13:
				printf_m13("unknown\n");
				break;
			default:
				G_warning_message_m13("%s(): Unrecognized Epoc v2.0 stage code (%hhu)\n", __FUNCTION__, epoc2->stage_code);
				break;
		}
		UTF8_printf_m13("Scorer ID: %s\n", epoc2->scorer_id);
	}
	// Unrecognized record version
	else {
		G_warning_message_m13("%s(): Unrecognized Epoc Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return_m13(TRUE_m13);
}


tern     REC_check_Epoc_type_alignment_m13(ui1 *bytes)
{
	si1			*version_string;
	REC_Epoc_v10_m13	*epoc1;
	REC_Epoc_v20_m13	*epoc2;
	tern			free_flag = FALSE_m13;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// Version 1.0
	version_string = "version 1.0";
	
	// check overall size
	if (sizeof(REC_Epoc_v10_m13) != REC_Epoc_v10_BYTES_m13)
		goto REC_Epoc_NOT_ALIGNED_m13;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_Epoc_v10_BYTES_m13);
		free_flag = TRUE_m13;
	}

	epoc1 = (REC_Epoc_v10_m13 *) bytes;
	if (&epoc1->id_number != (si8 *) (bytes + REC_Epoc_v10_ID_NUMBER_OFFSET_m13))
		goto REC_Epoc_NOT_ALIGNED_m13;
	if (&epoc1->end_time != (si8 *) (bytes + REC_Epoc_v10_END_TIME_OFFSET_m13))
		goto REC_Epoc_NOT_ALIGNED_m13;
	if (epoc1->epoch_type != (si1 *) (bytes + REC_Epoc_v10_EPOCH_TYPE_OFFSET_m13))
		goto REC_Epoc_NOT_ALIGNED_m13;
	if (epoc1->text != (si1 *) (bytes + REC_Epoc_v10_TEXT_OFFSET_m13))
		goto REC_Epoc_NOT_ALIGNED_m13;

	// Version 2.0
	version_string = "version 2.0";
	
	// check overall size
	if (sizeof(REC_Epoc_v20_m13) != REC_Epoc_v20_BYTES_m13)
		goto REC_Epoc_NOT_ALIGNED_m13;

	epoc2 = (REC_Epoc_v20_m13 *) bytes;
	if (&epoc2->end_time != (si8 *) (bytes + REC_Epoc_v20_END_TIME_OFFSET_m13))
		goto REC_Epoc_NOT_ALIGNED_m13;
	if (&epoc2->stage_code != (ui1 *) (bytes + REC_Epoc_v20_STAGE_CODE_OFFSET_m13))
		goto REC_Epoc_NOT_ALIGNED_m13;
	if (epoc2->scorer_id != (si1 *) (bytes + REC_Epoc_v20_SCORER_ID_OFFSET_m13))
		goto REC_Epoc_NOT_ALIGNED_m13;

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	return_m13(TRUE_m13);

	// not aligned
REC_Epoc_NOT_ALIGNED_m13:

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	G_set_error_m13(E_REC_m13, "Epoc %s structure is NOT aligned", version_string);

	return_m13(FALSE_m13);
}


//*************************************************************************************//
//**************************   ESti: Electrical Stimulation   *************************//
//*************************************************************************************//

tern	REC_show_ESti_type_m13(RECORD_HEADER_m13 *record_header)
{
	REC_ESti_v10_m13	*esti;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		esti = (REC_ESti_v10_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
		printf_m13("Amplitude: %lf ", esti->amplitude);
		switch (esti->amp_unit_code) {
			case REC_ESti_v10_AMP_UNIT_MA_m13:
				printf_m13("mA\n");
				break;
			case REC_ESti_v10_AMP_UNIT_V_m13:
				printf_m13("V\n");
				break;
			case REC_ESti_v10_AMP_UNIT_NO_ENTRY_m13:
				printf_m13("(units no entry)\n");
				break;
			case REC_ESti_v10_AMP_UNIT_UNKNOWN_m13:
				printf_m13("(units unknown)\n");
				break;
			default:
				printf_m13("(unrecognized units code: %d)\n", esti->amp_unit_code);
				break;
		}
		printf_m13("Frequency: %lf (Hz)\n", esti->frequency);
		printf_m13("Pulse Width: %ld (µS)\n", esti->pulse_width);
		printf_m13("Mode: ");
		switch (esti->mode_code) {
			case REC_ESti_v10_MODE_CURRENT_m13:
				printf_m13("constant current\n");
				break;
			case REC_ESti_v10_MODE_VOLTAGE_m13:
				printf_m13("constant voltage\n");
				break;
			case REC_ESti_v10_MODE_NO_ENTRY_m13:
				printf_m13("no entry\n");
				break;
			case REC_ESti_v10_MODE_UNKNOWN_m13:
				printf_m13("unknown\n");
				break;
			default:
				G_warning_message_m13("unrecognized mode code (%d)\n", esti->mode_code);
				break;
		}

		UTF8_printf_m13("Waveform: %s\n", esti->waveform);
		UTF8_printf_m13("Anode: %s\n", esti->anode);
		UTF8_printf_m13("Cathode: %s\n", esti->cathode);
	}
	// Unrecognized record version
	else {
		G_set_error_m13(E_REC_m13, "unrecognized ESti Record version (%hhd.%hhd)", record_header->version_major, record_header->version_minor);
	}

	return_m13(TRUE_m13);
}


tern     REC_check_ESti_type_alignment_m13(ui1 *bytes)
{
	REC_ESti_v10_m13	*esti;
	tern			free_flag = FALSE_m13;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// check overall size
	if (sizeof(REC_ESti_v10_m13) != REC_ESti_v10_BYTES_m13)
		goto REC_ESti_NOT_ALIGNED_m13;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_ESti_v10_BYTES_m13);
		free_flag = TRUE_m13;
	}

	esti = (REC_ESti_v10_m13 *) bytes;
	if (&esti->amplitude != (sf8 *) (bytes + REC_ESti_v10_AMPLITUDE_OFFSET_m13))
		goto REC_ESti_NOT_ALIGNED_m13;
	if (&esti->frequency != (sf8 *) (bytes + REC_ESti_v10_FREQUENCY_OFFSET_m13))
		goto REC_ESti_NOT_ALIGNED_m13;
	if (&esti->pulse_width != (si8 *) (bytes + REC_ESti_v10_PULSE_WIDTH_OFFSET_m13))
		goto REC_ESti_NOT_ALIGNED_m13;
	if (&esti->amp_unit_code != (si4 *) (bytes + REC_ESti_v10_AMP_UNIT_CODE_OFFSET_m13))
		goto REC_ESti_NOT_ALIGNED_m13;
	if (&esti->mode_code != (si4 *) (bytes + REC_ESti_v10_MODE_CODE_OFFSET_m13))
		goto REC_ESti_NOT_ALIGNED_m13;
	if (esti->waveform != (si1 *) (bytes + REC_ESti_v10_WAVEFORM_OFFSET_m13))
		goto REC_ESti_NOT_ALIGNED_m13;
	if (esti->anode != (si1 *) (bytes + REC_ESti_v10_ANODE_OFFSET_m13))
		goto REC_ESti_NOT_ALIGNED_m13;
	if (esti->cathode != (si1 *) (bytes + REC_ESti_v10_CATHODE_OFFSET_m13))
		goto REC_ESti_NOT_ALIGNED_m13;

	// aligned
	if (free_flag == TRUE_m13)
		free((void *) bytes);

	return_m13(TRUE_m13);

	// not aligned
REC_ESti_NOT_ALIGNED_m13:

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	G_set_error_m13(E_REC_m13, "ESti structure is NOT aligned");

	return_m13(FALSE_m13);

}


//*************************************************************************************//
//**************************   CSti: Cognitive Stimulation   **************************//
//*************************************************************************************//

tern    REC_show_CSti_type_m13(RECORD_HEADER_m13 *record_header)
{
	REC_CSti_v10_m13	*csti;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		csti = (REC_CSti_v10_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
		printf_m13("Stimulus Duration: %ld (usecs)\n", csti->stimulus_duration);
		UTF8_printf_m13("Task Type: %s\n", csti->task_type);
		UTF8_printf_m13("Stimulus Type: %s\n", csti->stimulus_type);
		UTF8_printf_m13("Patient Response: %s\n", csti->patient_response);
	}
	// Unrecognized record version
	else {
		G_set_error_m13(E_REC_m13, "unrecognized CSti record version (%hhd.%hhd)", record_header->version_major, record_header->version_minor);
	}

	return_m13(TRUE_m13);
}


tern     REC_check_CSti_type_alignment_m13(ui1 *bytes)
{
	tern			free_flag = FALSE_m13;
	REC_CSti_v10_m13	*csti;

#ifdef FN_DEBUG_m13
	G_push_function_m13();
#endif

	// check overall size
	if (sizeof(REC_CSti_v10_m13) != REC_CSti_v10_BYTES_m13)
		goto REC_CSti_NOT_ALIGNED_m13;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_CSti_v10_BYTES_m13);
		free_flag = TRUE_m13;
	}

	csti = (REC_CSti_v10_m13 *) bytes;
	if (&csti->stimulus_duration != (si8 *) (bytes + REC_CSti_v10_STIMULUS_DURATION_OFFSET_m13))
		goto REC_CSti_NOT_ALIGNED_m13;
	if (csti->task_type != (si1 *) (bytes + REC_CSti_v10_TASK_TYPE_OFFSET_m13))
		goto REC_CSti_NOT_ALIGNED_m13;
	if (csti->stimulus_type != (si1 *) (bytes + REC_CSti_v10_STIMULUS_TYPE_OFFSET_m13))
		goto REC_CSti_NOT_ALIGNED_m13;
	if (csti->patient_response != (si1 *) (bytes + REC_CSti_v10_PATIENT_RESPONSE_OFFSET_m13))
		goto REC_CSti_NOT_ALIGNED_m13;

	// aligned
	if (free_flag == TRUE_m13)
		free((void *) bytes);

	return_m13(TRUE_m13);

	// not aligned
REC_CSti_NOT_ALIGNED_m13:

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	G_set_error_m13(E_REC_m13, "CSti structure is NOT aligned");

	return_m13(FALSE_m13);

}


//*************************************************************************************//
//*****************************   HFOc: CS HFO Detection   ****************************//
//*************************************************************************************//

void    REC_show_HFOc_type_m13(RECORD_HEADER_m13 *record_header)
{
	REC_HFOc_v11_m13	*hfoc_1;
	REC_HFOc_v12_m13	*hfoc_2;
	REC_HFOc_v13_m13	*hfoc_3;

#ifdef FN_DEBUG_m13
	G_message_m13("%s()\n", __FUNCTION__);
#endif
	
	// Versions 1.0-3
	if (record_header->version_major == 1 && record_header->version_minor <= 3) {
		switch (record_header->version_minor) {
			case 0:
				break;
			case 1:
				hfoc_1 = (REC_HFOc_v11_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
				printf_m13("End Time: %ld\n", hfoc_1->end_time);
				printf_m13("Start Frequency: %f\n", hfoc_1->start_frequency);
				printf_m13("End Frequency: %f\n", hfoc_1->end_frequency);
				break;
			case 2:
				hfoc_2 = (REC_HFOc_v12_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
				printf_m13("End Time: %ld\n", hfoc_2->end_time);
				printf_m13("Start Frequency: %f\n", hfoc_2->start_frequency);
				printf_m13("End Frequency: %f\n", hfoc_2->end_frequency);
				printf_m13("Start Times: %ld, %ld, %ld, %ld\n", hfoc_2->start_times[0], hfoc_2->start_times[1], hfoc_2->start_times[2], hfoc_2->start_times[3]);
				printf_m13("End Times: %ld, %ld, %ld, %ld\n", hfoc_2->end_times[0], hfoc_2->end_times[1], hfoc_2->end_times[2], hfoc_2->end_times[3]);
				printf_m13("Combinations: %f, %f, %f, %f\n", hfoc_2->combinations[0], hfoc_2->combinations[1], hfoc_2->combinations[2], hfoc_2->combinations[3]);
				break;
			case 3:
				hfoc_3 = (REC_HFOc_v13_m13 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m13);
				printf_m13("End Time: %ld\n", hfoc_3->end_time);
				printf_m13("Start Frequency: %f\n", hfoc_3->start_frequency);
				printf_m13("End Frequency: %f\n", hfoc_3->end_frequency);
				printf_m13("Start Times: %ld, %ld, %ld, %ld\n", hfoc_3->start_times[0], hfoc_3->start_times[1], hfoc_3->start_times[2], hfoc_3->start_times[3]);
				printf_m13("End Times: %ld, %ld, %ld, %ld\n", hfoc_3->end_times[0], hfoc_3->end_times[1], hfoc_3->end_times[2], hfoc_3->end_times[3]);
				printf_m13("Combinations: %f, %f, %f, %f\n", hfoc_3->combinations[0], hfoc_3->combinations[1], hfoc_3->combinations[2], hfoc_3->combinations[3]);
				printf_m13("Amplitudes: %f, %f, %f, %f\n", hfoc_3->amplitudes[0], hfoc_3->amplitudes[1], hfoc_3->amplitudes[2], hfoc_3->amplitudes[3]);
				printf_m13("Frequency Dominances: %f, %f, %f, %f\n", hfoc_3->frequency_dominances[0], hfoc_3->frequency_dominances[1], hfoc_3->frequency_dominances[2], hfoc_3->frequency_dominances[3]);
				printf_m13("Products: %f, %f, %f, %f\n", hfoc_3->products[0], hfoc_3->products[1], hfoc_3->products[2], hfoc_3->products[3]);
				printf_m13("Cycles: %f, %f, %f, %f\n", hfoc_3->cycles[0], hfoc_3->cycles[1], hfoc_3->cycles[2], hfoc_3->cycles[3]);
				break;
		}

	}
	// Unrecognized record version
	else {
		G_set_error_m13(E_REC_m13, "unrecognized HFOc Record version (%hhd.%hhd)", record_header->version_major, record_header->version_minor);
	}

	return;
}


tern	REC_check_HFOc_type_alignment_m13(ui1 *bytes)
{
	tern			free_flag = FALSE_m13;
	ui1			version;
	REC_HFOc_v11_m13	*hfoc_1;
	REC_HFOc_v12_m13	*hfoc_2;
	REC_HFOc_v13_m13	*hfoc_3;

#ifdef FN_DEBUG_m13
	G_message_m13("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_HFOc_v11_m13) != REC_HFOc_v11_BYTES_m13) {
		version = 1;
		goto REC_HFOc_NOT_ALIGNED_m13;
	}
	if (sizeof(REC_HFOc_v12_m13) != REC_HFOc_v12_BYTES_m13) {
		version = 2;
		goto REC_HFOc_NOT_ALIGNED_m13;
	}
	if (sizeof(REC_HFOc_v13_m13) != REC_HFOc_v13_BYTES_m13) {
		version = 3;
		goto REC_HFOc_NOT_ALIGNED_m13;
	}

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_HFOc_v13_BYTES_m13);
		free_flag = TRUE_m13;
	}

	// version 1.1
	version = 1;
	hfoc_1 = (REC_HFOc_v11_m13 *) bytes;
	if (&hfoc_1->end_time != (si8 *) (bytes + REC_HFOc_v11_END_TIME_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_1->start_frequency != (sf4 *) (bytes + REC_HFOc_v11_START_FREQUENCY_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_1->end_frequency != (sf4 *) (bytes + REC_HFOc_v11_END_FREQUENCY_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;

	// version 1.2
	version = 2;
	hfoc_2 = (REC_HFOc_v12_m13 *) bytes;
	if (&hfoc_2->end_time != (si8 *) (bytes + REC_HFOc_v12_END_TIME_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_2->start_frequency != (sf4 *) (bytes + REC_HFOc_v12_START_FREQUENCY_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_2->end_frequency != (sf4 *) (bytes + REC_HFOc_v12_END_FREQUENCY_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_2->start_times != (si8 (*)[REC_HFOc_NUMBER_OF_BANDS_m13]) (bytes + REC_HFOc_v12_START_TIMES_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_2->end_times != (si8 (*)[REC_HFOc_NUMBER_OF_BANDS_m13]) (bytes + REC_HFOc_v12_END_TIMES_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_2->combinations != (sf4 (*)[REC_HFOc_NUMBER_OF_BANDS_m13]) (bytes + REC_HFOc_v12_COMBINATIONS_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;

	// version 1.3
	version = 3;
	hfoc_3 = (REC_HFOc_v13_m13 *) bytes;
	if (&hfoc_3->end_time != (si8 *) (bytes + REC_HFOc_v13_END_TIME_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_3->start_frequency != (sf4 *) (bytes + REC_HFOc_v13_START_FREQUENCY_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_3->end_frequency != (sf4 *) (bytes + REC_HFOc_v13_END_FREQUENCY_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_3->start_times != (si8 (*)[REC_HFOc_NUMBER_OF_BANDS_m13]) (bytes + REC_HFOc_v13_START_TIMES_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_3->end_times != (si8 (*)[REC_HFOc_NUMBER_OF_BANDS_m13]) (bytes + REC_HFOc_v13_END_TIMES_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_3->combinations != (sf4 (*)[REC_HFOc_NUMBER_OF_BANDS_m13]) (bytes + REC_HFOc_v13_COMBINATIONS_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_3->amplitudes != (sf4 (*)[REC_HFOc_NUMBER_OF_BANDS_m13]) (bytes + REC_HFOc_v13_AMPLITUDES_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_3->frequency_dominances != (sf4 (*)[REC_HFOc_NUMBER_OF_BANDS_m13]) (bytes + REC_HFOc_v13_FREQUENCY_DOMINANCES_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_3->products != (sf4 (*)[REC_HFOc_NUMBER_OF_BANDS_m13]) (bytes + REC_HFOc_v13_PRODUCTS_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;
	if (&hfoc_3->cycles != (sf4 (*)[REC_HFOc_NUMBER_OF_BANDS_m13]) (bytes + REC_HFOc_v13_CYCLES_OFFSET_m13))
		goto REC_HFOc_NOT_ALIGNED_m13;

	// aligned
	if (free_flag == TRUE_m13)
		free((void *) bytes);

	return(TRUE_m13);

	// not aligned
REC_HFOc_NOT_ALIGNED_m13:

	if (free_flag == TRUE_m13)
		free((void *) bytes);

	G_set_error_m13(E_REC_m13, "REC_HFOc_v1%hhu_m13 structure is NOT aligned", version);

	return(FALSE_m13);

}


//*************************************************************************************//
//********************************   New Record Type   ********************************//
//*************************************************************************************//



