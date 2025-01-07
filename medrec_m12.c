
//**********************************************************************************//
//***************************  MED 1.0.2 C Library Records  ************************//
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
// with the suffix "_mFL" (for "MED major format 'F', library 'L'").

// MED_FORMAT_VERSION_MAJOR is restricted to single digits 1 through 9
// MED_FORMAT_VERSION_MINOR is restricted to 0 through 254, minor version resets to zero with new major format version
// MED_LIBRARY_VERSION is restricted to 1 through 254, library version resets to one with new major format version

// MED_FULL_FORMAT_NAME == "<MED_VERSION_MAJOR_m12>.<MED_VERSION_MINOR_m12>"
// MED_FULL_LIBRARY_NAME == "<MED_FULL_FORMAT_NAME_m12>.<MED_LIBRARY_VERSION_m12>"
// MED_LIBRARY_TAG == "<MED_VERSION_MAJOR_m12>.<MED_LIBRARY_VERSION_m12>"

// Examples:
// "_m12" indicates "MED format major version 1, library version 1"
// "_m21" indicates "MED format major version 2, library version 1" (for MED 2)
// "_m213" indicates "MED format major version 2, library version 13" (for MED 2)

// All library versions associated with a particular major format version are guaranteed to work on MED files of that major version.
// Minor format versions may add fields to the format in protected regions, but no preexisting fields will be removed or moved.
// Only library versions released on or afer a minor version will make use of new fields, and only if the minor version of the files contains them.
// Backward compatibility will be maintained between major versions if practical.



#include "medlib_m12.h"


//*************************************************************************************//
//**********************************   show_record()   ********************************//
//*************************************************************************************//

void	REC_show_record_m12(FILE_PROCESSING_STRUCT_m12 *fps, RECORD_HEADER_m12 *record_header, si8 record_number)
{
	ui4                     type_code;
	si1	                time_str[TIME_STRING_BYTES_m12], hex_str[HEX_STRING_BYTES_m12(CRC_BYTES_m12)];

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// decrypt record body if necesary
	if (record_header->encryption_level > NO_ENCRYPTION_m12)
		G_decrypt_record_data_m12(fps, record_header, 1);
		    
	// display record header fields
	if (record_number != REC_NO_RECORD_NUMBER_m12)
		printf_m12("Record Number: %ld\n", record_number);
	printf_m12("---------------- Record Header - START ----------------\n");
	if (record_header->record_CRC == RECORD_HEADER_RECORD_CRC_NO_ENTRY_m12) {
		printf_m12("Record CRC: no entry\n");
	} else {
		STR_generate_hex_string_m12((ui1 *) &record_header->record_CRC, CRC_BYTES_m12, hex_str);
		printf_m12("Record CRC: %s\n", hex_str);
	}
	type_code = record_header->type_code;
	if (type_code) {
		STR_generate_hex_string_m12((ui1 *) record_header->type_string, CRC_BYTES_m12, hex_str);
		printf_m12("Record Type String: %s (%s)\n", record_header->type_string, hex_str);
	} else {
		printf_m12("Record Type String: no entry\n");
	}
	if (record_header->version_major == RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m12 || record_header->version_minor == RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m12) {
		if (record_header->version_major == RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m12)
			printf_m12("Record Version Major: no entry\n");
		else
			printf_m12("Record Version Major: %u\n", record_header->version_major);
		if (record_header->version_minor == RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m12)
			printf_m12("Record Version Minor: no entry\n");
		else
			printf_m12("Record Version Minor: %u\n", record_header->version_minor);
	} else {
		printf_m12("Record Version: %hu.%hu\n", record_header->version_major, record_header->version_minor);
	}
	printf_m12("Record Encryption Level: %hd ", record_header->encryption_level);
	if (record_header->encryption_level == NO_ENCRYPTION_m12)
		printf_m12("(none)\n");
	else if (record_header->encryption_level == LEVEL_1_ENCRYPTION_m12)
		printf_m12("(level 1, currently encrypted)\n");
	else if (record_header->encryption_level == LEVEL_2_ENCRYPTION_m12)
		printf_m12("(level 2, currently encrypted)\n");
	else if (record_header->encryption_level == LEVEL_1_ENCRYPTION_DECRYPTED_m12)
		printf_m12("(level 1, currently decrypted)\n");
	else if (record_header->encryption_level == LEVEL_2_ENCRYPTION_DECRYPTED_m12)
		printf_m12("(level 2, currently decrypted)\n");
	else
		printf_m12("(unrecognized code)\n");
	if (record_header->total_record_bytes == RECORD_HEADER_TOTAL_RECORD_BYTES_NO_ENTRY_m12)
		printf_m12("Record Total Record Bytes: no entry\n");
	else
		printf_m12("Record Total Record Bytes: %u\n", record_header->total_record_bytes);

	if (record_header->start_time == RECORD_HEADER_START_TIME_NO_ENTRY_m12)
		printf_m12("Record Start Time: no entry\n");
	else {
		STR_time_string_m12(record_header->start_time, time_str, TRUE_m12, FALSE_m12, FALSE_m12);
		printf_m12("Record Start Time: %ld (oUTC), %s\n", record_header->start_time, time_str);
	}
	printf_m12("----------------- Record Header - END -----------------\n");

	// record body
	printf_m12("----------------- Record Body - START -----------------\n");
	if (record_header->encryption_level > NO_ENCRYPTION_m12) {
		printf_m12("No access to this record\n");
		printf_m12("------------------ Record Body - END ------------------\n\n");
		return;
	}

	// pass the display off to custom functions - new records types should be added here (maintain alphabetical order of record types)
	switch (type_code) {
		case REC_Sgmt_TYPE_CODE_m12:
			REC_show_Sgmt_type_m12(record_header);
			break;
		case REC_Stat_TYPE_CODE_m12:
			REC_show_Stat_type_m12(record_header);
			break;
		case REC_Note_TYPE_CODE_m12:
			REC_show_Note_type_m12(record_header);
			break;
		case REC_EDFA_TYPE_CODE_m12:
			REC_show_EDFA_type_m12(record_header);
			break;
		case REC_Seiz_TYPE_CODE_m12:
			REC_show_Seiz_type_m12(record_header);
			break;
		case REC_SyLg_TYPE_CODE_m12:
			REC_show_SyLg_type_m12(record_header);
			break;
		case REC_NlxP_TYPE_CODE_m12:
			REC_show_NlxP_type_m12(record_header);
			break;
		case REC_Curs_TYPE_CODE_m12:
			REC_show_Curs_type_m12(record_header);
			break;
		case REC_Epoc_TYPE_CODE_m12:
			REC_show_Epoc_type_m12(record_header);
			break;
		case REC_ESti_TYPE_CODE_m12:
			REC_show_ESti_type_m12(record_header);
			break;
		case REC_CSti_TYPE_CODE_m12:
			REC_show_CSti_type_m12(record_header);
			break;
		default:
			G_warning_message_m12("%s(): 0x%x is an unrecognized record type code\n", __FUNCTION__, type_code);
			break;
	}
	printf_m12("------------------ Record Body - END ------------------\n\n");

	return;
}


//*************************************************************************************//
//*********************   check_record_structure_alignments()   ***********************//
//*************************************************************************************//

TERN_m12	REC_check_structure_alignments_m12(ui1 *bytes)
{
	TERN_m12	return_value, free_flag = FALSE_m12;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// see if already checked
	if (globals_m12->all_record_structures_aligned != UNKNOWN_m12)
		return(globals_m12->all_record_structures_aligned);

	return_value = TRUE_m12;
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_LARGEST_RECORD_BYTES_m12);
		free_flag = TRUE_m12;
	}

	// check all structures - add new functions here
	if ((REC_check_Sgmt_type_alignment_m12(bytes)) == FALSE_m12)
		return_value = FALSE_m12;
	if ((REC_check_Stat_type_alignment_m12(bytes)) == FALSE_m12)
		return_value = FALSE_m12;
	if ((REC_check_Note_type_alignment_m12(bytes)) == FALSE_m12)
		return_value = FALSE_m12;
	if ((REC_check_EDFA_type_alignment_m12(bytes)) == FALSE_m12)
		return_value = FALSE_m12;
	if ((REC_check_Seiz_type_alignment_m12(bytes)) == FALSE_m12)
		return_value = FALSE_m12;
	if ((REC_check_SyLg_type_alignment_m12(bytes)) == FALSE_m12)
		return_value = FALSE_m12;
	if ((REC_check_NlxP_type_alignment_m12(bytes)) == FALSE_m12)
		return_value = FALSE_m12;
	if ((REC_check_Curs_type_alignment_m12(bytes)) == FALSE_m12)
		return_value = FALSE_m12;
	if ((REC_check_Epoc_type_alignment_m12(bytes)) == FALSE_m12)
		return_value = FALSE_m12;
	if ((REC_check_ESti_type_alignment_m12(bytes)) == FALSE_m12)
		return_value = FALSE_m12;
	if ((REC_check_CSti_type_alignment_m12(bytes)) == FALSE_m12)
		return_value = FALSE_m12;

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	if (return_value == TRUE_m12) {
		globals_m12->all_record_structures_aligned = TRUE_m12;
		if (globals_m12->verbose == TRUE_m12)
			printf_m12("%s(): All Record structures are aligned\n", __FUNCTION__);
	}
	else {
		globals_m12->all_record_structures_aligned = FALSE_m12;
		G_error_message_m12("%s(): One or more Record structures are NOT aligned\n", __FUNCTION__);
	}

	return(return_value);
}


//*************************************************************************************//
//*******************************   Sgmt: Segment Record   ****************************//
//*************************************************************************************//

void    REC_show_Sgmt_type_m12(RECORD_HEADER_m12 *record_header)
{
	REC_Sgmt_v10_m12	*Sgmt;
	si1                     time_str[TIME_STRING_BYTES_m12], hex_str[HEX_STRING_BYTES_m12(8)], *segment_description;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		Sgmt = (REC_Sgmt_v10_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);

		STR_time_string_m12(Sgmt->end_time, time_str, TRUE_m12, FALSE_m12, FALSE_m12);
		printf_m12("End Time: %ld (oUTC), %s\n", Sgmt->end_time, time_str);
		if (Sgmt->start_sample_number == REC_Sgmt_v10_START_SAMPLE_NUMBER_NO_ENTRY_m12)
			printf_m12("Start Sample Number: no entry\n");
		else
			printf_m12("Start Sample Number: %ld\n", Sgmt->start_sample_number);
		if (Sgmt->end_sample_number == REC_Sgmt_v10_END_SAMPLE_NUMBER_NO_ENTRY_m12)
			printf_m12("End Sample Number: no entry\n");
		else
			printf_m12("End Sample Number: %ld\n", Sgmt->end_sample_number);
		STR_generate_hex_string_m12((ui1*)&Sgmt->segment_UID, 8, hex_str);
		printf_m12("Segment UID: %s\n", hex_str);
		if (Sgmt->segment_number == REC_Sgmt_v10_SEGMENT_NUMBER_NO_ENTRY_m12)
			printf_m12("Segment Number: no entry\n");
		else
			printf_m12("Segment Number: %d\n", Sgmt->segment_number);

		if (Sgmt->acquisition_channel_number == REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_ALL_CHANNELS_m12)
			printf_m12("Acquisition Channel Number: all channels\n");
		else if (Sgmt->acquisition_channel_number == REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m12)
			printf_m12("Acquisition Channel Number: no entry\n");
		else
			printf_m12("Acquisition Channel Number: %d\n", Sgmt->acquisition_channel_number);

		if (Sgmt->sampling_frequency == REC_Sgmt_v10_SAMPLING_FREQUENCY_NO_ENTRY_m12)
			printf_m12("Sampling Frequency: no entry\n");
		else if (Sgmt->sampling_frequency == REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m12)
			printf_m12("Sampling Frequency: variable\n");
		else
			printf_m12("Sampling Frequency: %lf\n", Sgmt->sampling_frequency);

		if (record_header->total_record_bytes > (RECORD_HEADER_BYTES_m12 + REC_Sgmt_v10_BYTES_m12)) {
			segment_description = (si1 *) Sgmt + REC_Sgmt_v10_SEGMENT_DESCRIPTION_OFFSET_m12;
			if (*segment_description)
				UTF8_printf_m12("Segment Description: %s\n", segment_description);
			else
				printf_m12("Segment Description: no entry\n");
		} else {
			printf_m12("Segment Description: no entry\n");
		}
	}
	// Unrecognized record version
	else {
		G_error_message_m12("%s(): Unrecognized Sgmt Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m12     REC_check_Sgmt_type_alignment_m12(ui1 *bytes)
{
	REC_Sgmt_v10_m12	*Sgmt;
	TERN_m12                free_flag = FALSE_m12;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_Sgmt_v10_m12) != REC_Sgmt_v10_BYTES_m12)
		goto REC_Sgmt_v10_NOT_ALIGNED_m12;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_Sgmt_v10_BYTES_m12);
		free_flag = TRUE_m12;
	}
	Sgmt = (REC_Sgmt_v10_m12 *) bytes;
	if (&Sgmt->end_time != (si8*)(bytes + REC_Sgmt_v10_END_TIME_OFFSET_m12))
		goto REC_Sgmt_v10_NOT_ALIGNED_m12;
	if (&Sgmt->start_sample_number != (si8 *) (bytes + REC_Sgmt_v10_START_SAMPLE_NUMBER_OFFSET_m12))
		goto REC_Sgmt_v10_NOT_ALIGNED_m12;
	if (&Sgmt->start_frame_number != (si8 *) (bytes + REC_Sgmt_v10_START_FRAME_NUMBER_OFFSET_m12))
		goto REC_Sgmt_v10_NOT_ALIGNED_m12;
	if (&Sgmt->end_sample_number != (si8 *) (bytes + REC_Sgmt_v10_END_SAMPLE_NUMBER_OFFSET_m12))
		goto REC_Sgmt_v10_NOT_ALIGNED_m12;
	if (&Sgmt->end_frame_number != (si8 *) (bytes + REC_Sgmt_v10_END_FRAME_NUMBER_OFFSET_m12))
		goto REC_Sgmt_v10_NOT_ALIGNED_m12;
	if (&Sgmt->segment_UID != (ui8 *) (bytes + REC_Sgmt_v10_SEGMENT_UID_OFFSET_m12))
		goto REC_Sgmt_v10_NOT_ALIGNED_m12;
	if (&Sgmt->segment_number != (si4 *) (bytes + REC_Sgmt_v10_SEGMENT_NUMBER_OFFSET_m12))
		goto REC_Sgmt_v10_NOT_ALIGNED_m12;
	if (&Sgmt->acquisition_channel_number != (si4 *) (bytes + REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_OFFSET_m12))
		goto REC_Sgmt_v10_NOT_ALIGNED_m12;
	if (&Sgmt->sampling_frequency != (sf8 *) (bytes + REC_Sgmt_v10_SAMPLING_FREQUENCY_OFFSET_m12))
		goto REC_Sgmt_v10_NOT_ALIGNED_m12;

	// aligned
	if (free_flag == TRUE_m12)
		free((void *) bytes);

	if (globals_m12->verbose == TRUE_m12)
		printf_m12("%s(): REC_Sgmt_v10_m12 structure is aligned\n", __FUNCTION__);

	return(TRUE_m12);

	// not aligned
REC_Sgmt_v10_NOT_ALIGNED_m12:

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	G_error_message_m12("%s(): REC_Sgmt_v10_m12 structure is NOT aligned", __FUNCTION__);

	return(FALSE_m12);

}


//*************************************************************************************//
//*******************************   Stat: Segment Record   ****************************//
//*************************************************************************************//

void    REC_show_Stat_type_m12(RECORD_HEADER_m12 *record_header)
{
	REC_Stat_v10_m12	*Stat;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		Stat = (REC_Stat_v10_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);
		if (Stat->minimum == REC_Stat_v10_MINIMUM_NO_ENTRY_m12)
			printf_m12("Minimum: no entry\n");
		else
			printf_m12("Minimum: %d\n", Stat->minimum);
		if (Stat->maximum == REC_Stat_v10_MAXIMUM_NO_ENTRY_m12)
			printf_m12("Maximum: no entry\n");
		else
			printf_m12("Maximum: %d\n", Stat->maximum);
		if (Stat->mean == REC_Stat_v10_MEAN_NO_ENTRY_m12)
			printf_m12("Mean: no entry\n");
		else
			printf_m12("Mean: %d\n", Stat->mean);
		if (Stat->median == REC_Stat_v10_MEDIAN_NO_ENTRY_m12)
			printf_m12("Median: no entry\n");
		else
			printf_m12("Median: %d\n", Stat->median);
		if (Stat->mode == REC_Stat_v10_MODE_NO_ENTRY_m12)
			printf_m12("Mode: no entry\n");
		else
			printf_m12("Mode: %d\n", Stat->mode);
		if (isnan(Stat->variance))
			printf_m12("Variance: no entry\n");
		else
			printf_m12("Variance: %f\n", Stat->variance);
		if (isnan(Stat->skewness))
			printf_m12("Skewness: no entry\n");
		else
			printf_m12("Skewness: %f\n", Stat->skewness);
		if (isnan(Stat->kurtosis))
			printf_m12("Kurtosis: no entry\n");
		else
			printf_m12("Kurtosis: %f\n", Stat->kurtosis);
	}
	// Unrecognized record version
	else {
		G_error_message_m12("%s(): Unrecognized Stat Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;

}


TERN_m12     REC_check_Stat_type_alignment_m12(ui1 *bytes)
{
	REC_Stat_v10_m12	*Stat;
	TERN_m12                free_flag = FALSE_m12;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_Stat_v10_m12) != REC_Stat_v10_BYTES_m12)
		goto REC_Stat_v10_NOT_ALIGNED_m12;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_Stat_v10_BYTES_m12);
		free_flag = TRUE_m12;
	}
	Stat = (REC_Stat_v10_m12 *) bytes;
	if (&Stat->minimum != (si4 *) (bytes + REC_Stat_v10_MINIMUM_OFFSET_m12))
		goto REC_Stat_v10_NOT_ALIGNED_m12;
	if (&Stat->maximum != (si4 *) (bytes + REC_Stat_v10_MAXIMUM_OFFSET_m12))
		goto REC_Stat_v10_NOT_ALIGNED_m12;
	if (&Stat->mean != (si4 *) (bytes + REC_Stat_v10_MEAN_OFFSET_m12))
		goto REC_Stat_v10_NOT_ALIGNED_m12;
	if (&Stat->median != (si4 *) (bytes + REC_Stat_v10_MEDIAN_OFFSET_m12))
		goto REC_Stat_v10_NOT_ALIGNED_m12;
	if (&Stat->mode != (si4 *) (bytes + REC_Stat_v10_MODE_OFFSET_m12))
		goto REC_Stat_v10_NOT_ALIGNED_m12;
	if (&Stat->variance != (sf4 *) (bytes + REC_Stat_v10_VARIANCE_OFFSET_m12))
		goto REC_Stat_v10_NOT_ALIGNED_m12;
	if (&Stat->skewness != (sf4 *) (bytes + REC_Stat_v10_SKEWNESS_OFFSET_m12))
		goto REC_Stat_v10_NOT_ALIGNED_m12;
	if (&Stat->kurtosis != (sf4 *) (bytes + REC_Stat_v10_KURTOSIS_OFFSET_m12))
		goto REC_Stat_v10_NOT_ALIGNED_m12;

	// aligned
	if (free_flag == TRUE_m12)
		free((void *) bytes);

	if (globals_m12->verbose == TRUE_m12)
		printf_m12("%s(): REC_Stat_v10_m12 structure is aligned\n", __FUNCTION__);

	return(TRUE_m12);

	// not aligned
REC_Stat_v10_NOT_ALIGNED_m12:

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	G_error_message_m12("%s(): REC_Stat_v10_m12 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m12);
}


//*************************************************************************************//
//********************************   Note: Note Record   ******************************//
//*************************************************************************************//

void	REC_show_Note_type_m12(RECORD_HEADER_m12 *record_header)
{
	si1			*note_text;
	REC_Note_v11_m12	*Note_v11;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		if (record_header->total_record_bytes > RECORD_HEADER_BYTES_m12) {
			note_text = (si1 *) record_header + RECORD_HEADER_BYTES_m12;
			if (*note_text)
				UTF8_printf_m12("Note Text: %s\n", note_text);
			else
				printf_m12("Note Text: no entry\n");
		} else {
			printf_m12("Note Text: no entry\n");
		}
	}
	
	// Version 1.1
	else if (record_header->version_major == 1 && record_header->version_minor == 1) {
		Note_v11 = (REC_Note_v11_m12 *) (record_header + 1);
		printf_m12("End Time: %ld\n", Note_v11->end_time);
		note_text = (si1 *) &Note_v11->text;
		if (*note_text)
			UTF8_printf_m12("Note Text: %s\n", note_text);
		else
			printf_m12("Note Text: no entry\n");
	}

	// Unrecognized record version
	else {
		G_error_message_m12("%s(): Unrecognized Note Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m12        REC_check_Note_type_alignment_m12(ui1 *bytes)
{
#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// no structures to check
	return(TRUE_m12);
}


//*************************************************************************************//
//******************   EDFA: European Data Format Annotation Record   *****************//
//*************************************************************************************//

void	REC_show_EDFA_type_m12(RECORD_HEADER_m12 *record_header)
{
	REC_EDFA_v10_m12	*edfa;
	si1			*annotation;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		edfa = (REC_EDFA_v10_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);
		printf_m12("Duration %ld microseconds\n", edfa->duration);
		annotation = (si1 *) edfa + REC_EDFA_v10_ANNOTATION_OFFSET_m12;
		if (*annotation)
			UTF8_printf_m12("Annotation: %s\n", annotation);
		else
			printf_m12("Annotation: no entry\n");
	}
	// Unrecognized record version
	else {
		G_error_message_m12("%s(): Unrecognized EDFA Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m12	REC_check_EDFA_type_alignment_m12(ui1 *bytes)
{
	REC_EDFA_v10_m12	*edfa;
	TERN_m12		free_flag = FALSE_m12;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_EDFA_v10_m12) != REC_EDFA_v10_BYTES_m12)
		goto REC_EDFA_v10_NOT_ALIGNED_m12;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_EDFA_v10_BYTES_m12);
		free_flag = TRUE_m12;
	}
	edfa = (REC_EDFA_v10_m12 *) bytes;
	if (&edfa->duration != (si8 *) (bytes + REC_EDFA_v10_DURATION_OFFSET_m12))
		goto REC_EDFA_v10_NOT_ALIGNED_m12;

	// aligned
	if (free_flag == TRUE_m12)
		free((void *) bytes);

	if (globals_m12->verbose == TRUE_m12)
		printf_m12("%s(): REC_EDFA_v10_m12 structure is aligned\n", __FUNCTION__);

	return(TRUE_m12);

	// not aligned
REC_EDFA_v10_NOT_ALIGNED_m12:

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	G_error_message_m12("%s(): REC_EDFA_v10_m12 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m12);
}



//*************************************************************************************//
//*******************************   Seiz: Seizure Record   ****************************//
//*************************************************************************************//

void	REC_show_Seiz_type_m12(RECORD_HEADER_m12 *record_header)
{
	si1				*description_text;
	si4			        i;
	TERN_m12                        mn1 = FALSE_m12, mn2 = FALSE_m12;
	REC_Seiz_v10_m12		*Seiz_v10;
	REC_Seiz_v20_m12		*Seiz_v20;
	REC_Seiz_v20_CHANNEL_m12	*chans;
	si1			        time_str[TIME_STRING_BYTES_m12];

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		Seiz_v10 = (REC_Seiz_v10_m12 *) (record_header + 1);
		printf_m12("End Time: %ld\n", Seiz_v10->end_time);
		description_text = (si1 *) &Seiz_v10->description;
		if (*description_text)
			UTF8_printf_m12("Description: %s\n", description_text);
		else
			printf_m12("Description: no entry\n");
	}

	// Version 2.0
	if (record_header->version_major == 2 && record_header->version_minor == 0) {
		Seiz_v20 = (REC_Seiz_v20_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);
		STR_time_string_m12(Seiz_v20->latest_offset_time, time_str, TRUE_m12, FALSE_m12, FALSE_m12);
		printf_m12("Latest Offset Time: %ld (oUTC), %ld (µUTC), %s\n", Seiz_v20->latest_offset_time, Seiz_v20->latest_offset_time + globals_m12->recording_time_offset, time_str);
		printf_m12("Number of Channels: %d\n", Seiz_v20->number_of_channels);
		printf_m12("Onset Code: %d ", Seiz_v20->onset_code);
		switch (Seiz_v20->onset_code) {
		case REC_Seiz_v20_ONSET_NO_ENTRY_m12:
			printf_m12("(no entry)\n");
			break;
		case REC_Seiz_v20_ONSET_UNKNOWN_m12:
			printf_m12("(unknown)\n");
			break;
		case REC_Seiz_v20_ONSET_FOCAL_m12:
			printf_m12("(focal)\n");
			break;
		case REC_Seiz_v20_ONSET_GENERALIZED_m12:
			printf_m12("(generalized)\n");
			break;
		case REC_Seiz_v20_ONSET_PROPAGATED_m12:
			printf_m12("(propagated)\n");
			break;
		case REC_Seiz_v20_ONSET_MIXED_m12:
			printf_m12("(mixed)\n");
			break;
		default:
			G_warning_message_m12("%s(): %d is an unrecognized Seiz onset code", __FUNCTION__, Seiz_v20->onset_code);
			break;
		}
		if (strlen(Seiz_v20->marker_name_1))
			mn1 = TRUE_m12;
		if (strlen(Seiz_v20->marker_name_2))
			mn2 = TRUE_m12;
		if (mn1 == TRUE_m12 && mn2 == TRUE_m12)
			UTF8_printf_m12("Marker Names: %s %s\n", Seiz_v20->marker_name_1, Seiz_v20->marker_name_2);
		else if (mn1 == TRUE_m12)
			UTF8_printf_m12("Marker Name 1: %s\nMarker Name 2: no entry\n", Seiz_v20->marker_name_1);
		else if (mn2 == TRUE_m12)
			UTF8_printf_m12("Marker Name 1: no_entry\nMarker Name 2: %s\n", Seiz_v20->marker_name_2);
		else
			printf_m12("Marker Names: no_entry\n");
		if (strlen(Seiz_v20->annotation))
			UTF8_printf_m12("Annotation: %s\n", Seiz_v20->annotation);
		else
			printf_m12("Annotation: no entry\n");
		chans = (REC_Seiz_v20_CHANNEL_m12 *) ((ui1 *) Seiz_v20 + REC_Seiz_v20_CHANNELS_OFFSET_m12);
		for (i = 0; i < Seiz_v20->number_of_channels; ++i) {
			if (strlen(chans[i].name))
				UTF8_printf_m12("Channel Name: %s\n", chans[i].name);
			else
				printf_m12("Channel Name: no entry\n");
			STR_time_string_m12(chans[i].onset_time, time_str, TRUE_m12, FALSE_m12, FALSE_m12);
			printf_m12("\tOnset Time: %ld (oUTC), %ld (µUTC), %s\n", chans[i].onset_time, chans[i].onset_time + globals_m12->recording_time_offset, time_str);
			STR_time_string_m12(chans[i].offset_time, time_str, TRUE_m12, FALSE_m12, FALSE_m12);
			printf_m12("\tOffset Time: %ld (oUTC), %ld (µUTC), %s\n", chans[i].offset_time, chans[i].offset_time + globals_m12->recording_time_offset, time_str);
			if (chans[i].segment_number == REC_Seiz_v20_CHANNEL_SEGMENT_NUMBER_NO_ENTRY_m12)
				printf_m12("Segment Number: no entry\n");
			else
				printf_m12("Segment Number: %d\n", chans[i].segment_number);
		}
	}
	
	// Unrecognized record version
	else {
		G_error_message_m12("%s(): Unrecognized Seiz Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m12	REC_check_Seiz_type_alignment_m12(ui1 *bytes)
{
	REC_Seiz_v10_m12		*Seiz_v10;
	REC_Seiz_v20_m12		*Seiz_v20;
	REC_Seiz_v20_CHANNEL_m12	*chan;
	TERN_m12			free_flag = FALSE_m12;
	ui1				*chan_bytes;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// check overall sizes
	if (sizeof(REC_Seiz_v10_m12) != REC_Seiz_v10_BYTES_m12)
		goto REC_Seiz_v10_NOT_ALIGNED_m12;
	if (sizeof(REC_Seiz_v20_m12) != REC_Seiz_v20_BYTES_m12)
		goto REC_Seiz_v20_NOT_ALIGNED_m12;
	if (sizeof(REC_Seiz_v20_CHANNEL_m12) != REC_Seiz_v20_CHANNEL_BYTES_m12)
		goto REC_Seiz_v20_NOT_ALIGNED_m12;

	// check fields - base structure
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_Seiz_v20_BYTES_m12 + REC_Seiz_v20_CHANNEL_BYTES_m12);
		free_flag = TRUE_m12;
	}
	
	// Version 1.0 fields
	Seiz_v10 = (REC_Seiz_v10_m12 *) bytes;
	if (&Seiz_v10->end_time != (si8 *) (bytes + REC_Seiz_v10_END_TIME_OFFSET_m12))
		goto REC_Seiz_v10_NOT_ALIGNED_m12;
	if (Seiz_v10->description != (si1 *) (bytes + REC_Seiz_v10_DESCRIPTION_OFFSET_m12))
		goto REC_Seiz_v10_NOT_ALIGNED_m12;

	// Version 2.0 fields
	Seiz_v20 = (REC_Seiz_v20_m12 *) bytes;
	if (&Seiz_v20->latest_offset_time != (si8 *) (bytes + REC_Seiz_v20_LATEST_OFFSET_TIME_OFFSET_m12))
		goto REC_Seiz_v20_NOT_ALIGNED_m12;
	if (&Seiz_v20->number_of_channels != (si4 *) (bytes + REC_Seiz_v20_NUMBER_OF_CHANNELS_OFFSET_m12))
		goto REC_Seiz_v20_NOT_ALIGNED_m12;
	if (&Seiz_v20->onset_code != (si4 *) (bytes + REC_Seiz_v20_ONSET_CODE_OFFSET_m12))
		goto REC_Seiz_v20_NOT_ALIGNED_m12;
	if (Seiz_v20->marker_name_1 != (si1 *) (bytes + REC_Seiz_v20_MARKER_NAME_1_OFFSET_m12))
		goto REC_Seiz_v20_NOT_ALIGNED_m12;
	if (Seiz_v20->marker_name_2 != (si1 *) (bytes + REC_Seiz_v20_MARKER_NAME_2_OFFSET_m12))
		goto REC_Seiz_v20_NOT_ALIGNED_m12;
	if (Seiz_v20->annotation != (si1 *) (bytes + REC_Seiz_v20_ANNOTATION_OFFSET_m12))
		goto REC_Seiz_v20_NOT_ALIGNED_m12;
	
	// check fields - channel structures
	chan_bytes = bytes + REC_Seiz_v20_CHANNELS_OFFSET_m12;
	chan = (REC_Seiz_v20_CHANNEL_m12 *) chan_bytes;
	if (chan->name != (si1 *) (chan_bytes + REC_Seiz_v20_CHANNEL_NAME_OFFSET_m12))
		goto REC_Seiz_v20_NOT_ALIGNED_m12;
	if (&chan->onset_time != (si8 *) (chan_bytes + REC_Seiz_v20_CHANNEL_ONSET_TIME_OFFSET_m12))
		goto REC_Seiz_v20_NOT_ALIGNED_m12;
	if (&chan->offset_time != (si8 *) (chan_bytes + REC_Seiz_v20_CHANNEL_OFFSET_TIME_OFFSET_m12))
		goto REC_Seiz_v20_NOT_ALIGNED_m12;
	if (&chan->segment_number != (si4 *) (chan_bytes + REC_Seiz_v20_CHANNEL_SEGMENT_NUMBER_OFFSET_m12))
		goto REC_Seiz_v20_NOT_ALIGNED_m12;
	if (chan->pad != (ui1 *) (chan_bytes + REC_Seiz_v20_CHANNEL_PAD_OFFSET_m12))
		goto REC_Seiz_v20_NOT_ALIGNED_m12;

	// aligned
	if (free_flag == TRUE_m12)
		free((void *) bytes);

	if (globals_m12->verbose == TRUE_m12)
		printf_m12("%s(): REC_Seiz_vxx_m12 structures are aligned\n", __FUNCTION__);

	return(TRUE_m12);

	// not aligned
REC_Seiz_v10_NOT_ALIGNED_m12:

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	G_error_message_m12("%s(): REC_Seiz_v10_m12 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m12);
	
REC_Seiz_v20_NOT_ALIGNED_m12:

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	G_error_message_m12("%s(): REC_Seiz_v20_m12 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m12);
}


//*************************************************************************************//
//*****************************   SyLg: System Log Record   ***************************//
//*************************************************************************************//

void	REC_show_SyLg_type_m12(RECORD_HEADER_m12 *record_header)
{
	si1	*log_entry;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		log_entry = (si1 *) record_header + RECORD_HEADER_BYTES_m12;
		if (*log_entry)
			UTF8_printf_m12("System Log entry:\n%s\n", log_entry);
		else
			printf_m12("System Log entry: no entry\n");
	}
	// Unrecognized record version
	else {
		G_error_message_m12("%s(): Unrecognized SyLg Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m12	REC_check_SyLg_type_alignment_m12(ui1 *bytes)
{
#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// no structures to check
	return(TRUE_m12);
}



//*************************************************************************************//
//*********************   NlxP: NeuraLynx Parallel Port Record   **********************//
//*************************************************************************************//

void    REC_show_NlxP_type_m12(RECORD_HEADER_m12 *record_header)
{
	si1                     hex_str[HEX_STRING_BYTES_m12(4)];
	REC_NlxP_v10_m12	*nlxp;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		nlxp = (REC_NlxP_v10_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);
		printf_m12("Value: %u\n", nlxp->value);
		printf_m12("Subport: %hhu\n", nlxp->subport);
		printf_m12("Number of Subports: %hhu\n", nlxp->number_of_subports);
		printf_m12("Trigger Mode: ");
		switch (nlxp->trigger_mode) {
		case REC_NlxP_v10_NO_TRIGGER_m12:
			printf_m12("NO TRIGGER\n");
			break;
		case REC_NlxP_v10_ANY_BIT_CHANGE_m12:
			printf_m12("ANY BIT CHANGE\n");
			break;
		case REC_NlxP_v10_HIGH_BIT_SET_m12:
			printf_m12("HIGH BIT SET\n");
			break;
		default:
			G_warning_message_m12("%s(): Unrecognized trigger mode (%hhu)", __FUNCTION__, nlxp->trigger_mode);
			break;
		}
		printf_m12("Raw Port Value: %u  (unsigned dec)\n", nlxp->raw_port_value);
		STR_generate_hex_string_m12((ui1 *) &nlxp->raw_port_value, 4, hex_str);
		printf_m12("Raw Port Bytes: %s  (hex)\n", hex_str);
	}
	// Unrecognized record version
	else {
		G_error_message_m12("%s(): Unrecognized NlxP Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m12     REC_check_NlxP_type_alignment_m12(ui1 *bytes)
{
	REC_NlxP_v10_m12	*nlxp;
	TERN_m12                free_flag = FALSE_m12;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_NlxP_v10_m12) != REC_NlxP_v10_BYTES_m12)
		goto REC_NlxP_v10_NOT_ALIGNED_m12;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_NlxP_v10_BYTES_m12);
		free_flag = TRUE_m12;
	}

	nlxp = (REC_NlxP_v10_m12 *) bytes;
	if (&nlxp->raw_port_value != (ui4 *) (bytes + REC_NlxP_v10_RAW_PORT_VALUE_OFFSET_m12))
		goto REC_NlxP_v10_NOT_ALIGNED_m12;
	if (&nlxp->value != (ui4 *) (bytes + REC_NlxP_v10_VALUE_OFFSET_m12))
		goto REC_NlxP_v10_NOT_ALIGNED_m12;
	if (&nlxp->subport != (ui1 *) (bytes + REC_NlxP_v10_SUBPORT_OFFSET_m12))
		goto REC_NlxP_v10_NOT_ALIGNED_m12;
	if (&nlxp->number_of_subports != (ui1 *) (bytes + REC_NlxP_v10_NUMBER_OF_SUBPORTS_OFFSET_m12))
		goto REC_NlxP_v10_NOT_ALIGNED_m12;
	if (&nlxp->trigger_mode != (ui1 *) (bytes + REC_NlxP_v10_TRIGGER_MODE_OFFSET_m12))
		goto REC_NlxP_v10_NOT_ALIGNED_m12;
	if (nlxp->pad != (ui1 *) (bytes + REC_NlxP_v10_PAD_OFFSET_m12))
		goto REC_NlxP_v10_NOT_ALIGNED_m12;

	// aligned
	if (free_flag == TRUE_m12)
		free((void *) bytes);

	if (globals_m12->verbose == TRUE_m12)
		printf_m12("%s(): REC_NlxP_v10_m12 structure is aligned\n", __FUNCTION__);

	return(TRUE_m12);

	// not aligned
REC_NlxP_v10_NOT_ALIGNED_m12:

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	G_error_message_m12("%s(): REC_NlxP_v10_m12 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m12);

}


//*************************************************************************************//
//***********************   Curs: Cadwell EMG Cursor Annotation   *********************//
//*************************************************************************************//

void    REC_show_Curs_type_m12(RECORD_HEADER_m12 *record_header)
{
	REC_Curs_v10_m12	*curs;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		curs = (REC_Curs_v10_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);
		printf_m12("ID Number: %ld\n", curs->id_number);
		printf_m12("Latency: %ld\n", curs->latency);
		printf_m12("Value: %lf\n", curs->value);
		UTF8_printf_m12("Name: %s\n", curs->name);
	}
	// Unrecognized record version
	else {
		G_error_message_m12("%s(): Unrecognized Curs Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m12     REC_check_Curs_type_alignment_m12(ui1 *bytes)
{
	REC_Curs_v10_m12	*curs;
	TERN_m12                free_flag = FALSE_m12;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_Curs_v10_m12) != REC_Curs_v10_BYTES_m12)
		goto REC_Curs_v10_NOT_ALIGNED_m12;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1*) malloc(REC_Curs_v10_BYTES_m12);
		free_flag = TRUE_m12;
	}

	curs = (REC_Curs_v10_m12 *) bytes;
	if (&curs->id_number != (si8 *) (bytes + REC_Curs_v10_ID_NUMBER_OFFSET_m12))
		goto REC_Curs_v10_NOT_ALIGNED_m12;
	if (&curs->latency != (si8 *) (bytes + REC_Curs_v10_LATENCY_OFFSET_m12))
		goto REC_Curs_v10_NOT_ALIGNED_m12;
	if (&curs->value != (sf8 *) (bytes + REC_Curs_v10_VALUE_OFFSET_m12))
		goto REC_Curs_v10_NOT_ALIGNED_m12;
	if (curs->name != (si1 *) (bytes + REC_Curs_v10_NAME_OFFSET_m12))
		goto REC_Curs_v10_NOT_ALIGNED_m12;

	// aligned
	if (free_flag == TRUE_m12)
		free((void *) bytes);

	if (globals_m12->verbose == TRUE_m12)
		printf_m12("%s(): REC_Curs_v10_m12 structure is aligned\n", __FUNCTION__);

	return(TRUE_m12);

	// not aligned
REC_Curs_v10_NOT_ALIGNED_m12:

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	G_error_message_m12("%s(): REC_Curs_v10_m12 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m12);

}


//*************************************************************************************//
//****************************   Epoc: Sleep Stage Record   ***************************//
//*************************************************************************************//

void    REC_show_Epoc_type_m12(RECORD_HEADER_m12 *record_header)
{
	REC_Epoc_v10_m12	*epoc1;
	REC_Epoc_v20_m12	*epoc2;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		epoc1 = (REC_Epoc_v10_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);
		printf_m12("ID Number: %ld\n", epoc1->id_number);
		printf_m12("End Time: %ld\n", epoc1->end_time);
		UTF8_printf_m12("Epoch Type: %s\n", epoc1->epoch_type);
		UTF8_printf_m12("Text: %s\n", epoc1->text);
	}
	// Version 2.0
	else if (record_header->version_major == 2 && record_header->version_minor == 0) {
		epoc2 = (REC_Epoc_v20_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);
		printf_m12("End Time: %ld\n", epoc2->end_time);
		printf_m12("Stage: ");
		switch (epoc2->stage_code) {
			case REC_Epoc_v20_STAGE_AWAKE_m12:
				printf_m12("awake\n");
				break;
			case REC_Epoc_v20_STAGE_NREM_1_m12:
				printf_m12("1\n");
				break;
			case REC_Epoc_v20_STAGE_NREM_2_m12:
				printf_m12("2\n");
				break;
			case REC_Epoc_v20_STAGE_NREM_3_m12:
				printf_m12("3\n");
				break;
			case REC_Epoc_v20_STAGE_NREM_4_m12:
				printf_m12("4\n");
				break;
			case REC_Epoc_v20_STAGE_REM_m12:
				printf_m12("REM\n");
				break;
			case REC_Epoc_v20_STAGE_UNKNOWN_m12:
				printf_m12("unknown\n");
				break;
			default:
				G_warning_message_m12("%s(): Unrecognized Epoc v2.0 stage code (%hhu)\n", __FUNCTION__, epoc2->stage_code);
				break;
		}
		UTF8_printf_m12("Scorer ID: %s\n", epoc2->scorer_id);
	}
	// Unrecognized record version
	else {
		G_warning_message_m12("%s(): Unrecognized Epoc Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m12     REC_check_Epoc_type_alignment_m12(ui1 *bytes)
{
	si1			*version_string;
	REC_Epoc_v10_m12	*epoc1;
	REC_Epoc_v20_m12	*epoc2;
	TERN_m12                free_flag = FALSE_m12;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	version_string = "REC_Epoc_v10_m12";
	
	// check overall size
	if (sizeof(REC_Epoc_v10_m12) != REC_Epoc_v10_BYTES_m12)
		goto REC_Epoc_NOT_ALIGNED_m12;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_Epoc_v10_BYTES_m12);
		free_flag = TRUE_m12;
	}

	epoc1 = (REC_Epoc_v10_m12 *) bytes;
	if (&epoc1->id_number != (si8 *) (bytes + REC_Epoc_v10_ID_NUMBER_OFFSET_m12))
		goto REC_Epoc_NOT_ALIGNED_m12;
	if (&epoc1->end_time != (si8 *) (bytes + REC_Epoc_v10_END_TIME_OFFSET_m12))
		goto REC_Epoc_NOT_ALIGNED_m12;
	if (epoc1->epoch_type != (si1 *) (bytes + REC_Epoc_v10_EPOCH_TYPE_OFFSET_m12))
		goto REC_Epoc_NOT_ALIGNED_m12;
	if (epoc1->text != (si1 *) (bytes + REC_Epoc_v10_TEXT_OFFSET_m12))
		goto REC_Epoc_NOT_ALIGNED_m12;

	// aligned
	if (globals_m12->verbose == TRUE_m12)
		printf_m12("%s(): %s structure is aligned\n", __FUNCTION__, version_string);
	
	// Version 2.0
	version_string = "REC_Epoc_v20_m12";
	
	// check overall size
	if (sizeof(REC_Epoc_v20_m12) != REC_Epoc_v20_BYTES_m12)
		goto REC_Epoc_NOT_ALIGNED_m12;

	epoc2 = (REC_Epoc_v20_m12 *) bytes;
	if (&epoc2->end_time != (si8 *) (bytes + REC_Epoc_v20_END_TIME_OFFSET_m12))
		goto REC_Epoc_NOT_ALIGNED_m12;
	if (&epoc2->stage_code != (ui1 *) (bytes + REC_Epoc_v20_STAGE_CODE_OFFSET_m12))
		goto REC_Epoc_NOT_ALIGNED_m12;
	if (epoc2->scorer_id != (si1 *) (bytes + REC_Epoc_v20_SCORER_ID_OFFSET_m12))
		goto REC_Epoc_NOT_ALIGNED_m12;

	// aligned
	if (globals_m12->verbose == TRUE_m12)
		printf_m12("%s(): %s structure is aligned\n", __FUNCTION__, version_string);

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	return(TRUE_m12);

	// not aligned
REC_Epoc_NOT_ALIGNED_m12:

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	G_error_message_m12("%s(): %s structure is NOT aligned\n", __FUNCTION__, version_string);

	return(FALSE_m12);
}


//*************************************************************************************//
//**************************   ESti: Electrical Stimulation   *************************//
//*************************************************************************************//

void    REC_show_ESti_type_m12(RECORD_HEADER_m12 *record_header)
{
	REC_ESti_v10_m12	*esti;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		esti = (REC_ESti_v10_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);
		printf_m12("Amplitude: %lf ", esti->amplitude);
		switch (esti->amp_unit_code) {
			case REC_ESti_v10_AMP_UNIT_MA_m12:
				printf_m12("mA\n");
				break;
			case REC_ESti_v10_AMP_UNIT_V_m12:
				printf_m12("V\n");
				break;
			case REC_ESti_v10_AMP_UNIT_NO_ENTRY_m12:
				printf_m12("(units no entry)\n");
				break;
			case REC_ESti_v10_AMP_UNIT_UNKNOWN_m12:
				printf_m12("(units unknown)\n");
				break;
			default:
				printf_m12("(unrecognized units code: %d)\n", esti->amp_unit_code);
				break;
		}
		printf_m12("Frequency: %lf (Hz)\n", esti->frequency);
		printf_m12("Pulse Width: %ld (µS)\n", esti->pulse_width);
		printf_m12("Mode: ");
		switch (esti->mode_code) {
			case REC_ESti_v10_MODE_CURRENT_m12:
				printf_m12("constant current\n");
				break;
			case REC_ESti_v10_MODE_VOLTAGE_m12:
				printf_m12("constant voltage\n");
				break;
			case REC_ESti_v10_MODE_NO_ENTRY_m12:
				printf_m12("no entry\n");
				break;
			case REC_ESti_v10_MODE_UNKNOWN_m12:
				printf_m12("unknown\n");
				break;
			default:
				G_warning_message_m12("unrecognized mode code (%d)\n", esti->mode_code);
				break;
		}

		UTF8_printf_m12("Waveform: %s\n", esti->waveform);
		UTF8_printf_m12("Anode: %s\n", esti->anode);
		UTF8_printf_m12("Cathode: %s\n", esti->cathode);
	}
	// Unrecognized record version
	else {
		G_error_message_m12("%s(): Unrecognized ESti Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m12     REC_check_ESti_type_alignment_m12(ui1 *bytes)
{
	REC_ESti_v10_m12	*esti;
	TERN_m12                free_flag = FALSE_m12;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_ESti_v10_m12) != REC_ESti_v10_BYTES_m12)
		goto REC_ESti_v10_NOT_ALIGNED_m12;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_ESti_v10_BYTES_m12);
		free_flag = TRUE_m12;
	}

	esti = (REC_ESti_v10_m12 *) bytes;
	if (&esti->amplitude != (sf8 *) (bytes + REC_ESti_v10_AMPLITUDE_OFFSET_m12))
		goto REC_ESti_v10_NOT_ALIGNED_m12;
	if (&esti->frequency != (sf8 *) (bytes + REC_ESti_v10_FREQUENCY_OFFSET_m12))
		goto REC_ESti_v10_NOT_ALIGNED_m12;
	if (&esti->pulse_width != (si8 *) (bytes + REC_ESti_v10_PULSE_WIDTH_OFFSET_m12))
		goto REC_ESti_v10_NOT_ALIGNED_m12;
	if (&esti->amp_unit_code != (si4 *) (bytes + REC_ESti_v10_AMP_UNIT_CODE_OFFSET_m12))
		goto REC_ESti_v10_NOT_ALIGNED_m12;
	if (&esti->mode_code != (si4 *) (bytes + REC_ESti_v10_MODE_CODE_OFFSET_m12))
		goto REC_ESti_v10_NOT_ALIGNED_m12;
	if (esti->waveform != (si1 *) (bytes + REC_ESti_v10_WAVEFORM_OFFSET_m12))
		goto REC_ESti_v10_NOT_ALIGNED_m12;
	if (esti->anode != (si1 *) (bytes + REC_ESti_v10_ANODE_OFFSET_m12))
		goto REC_ESti_v10_NOT_ALIGNED_m12;
	if (esti->cathode != (si1 *) (bytes + REC_ESti_v10_CATHODE_OFFSET_m12))
		goto REC_ESti_v10_NOT_ALIGNED_m12;

	// aligned
	if (free_flag == TRUE_m12)
		free((void *) bytes);

	if (globals_m12->verbose == TRUE_m12)
		printf_m12("%s(): REC_ESti_v10_m12 structure is aligned\n", __FUNCTION__);

	return(TRUE_m12);

	// not aligned
REC_ESti_v10_NOT_ALIGNED_m12:

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	G_error_message_m12("%s(): REC_ESti_v10_m12 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m12);

}


//*************************************************************************************//
//**************************   CSti: Cognitive Stimulation   **************************//
//*************************************************************************************//

void    REC_show_CSti_type_m12(RECORD_HEADER_m12 *record_header)
{
	REC_CSti_v10_m12	*csti;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		csti = (REC_CSti_v10_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);
		printf_m12("Stimulus Duration: %ld (usecs)\n", csti->stimulus_duration);
		UTF8_printf_m12("Task Type: %s\n", csti->task_type);
		UTF8_printf_m12("Stimulus Type: %s\n", csti->stimulus_type);
		UTF8_printf_m12("Patient Response: %s\n", csti->patient_response);
	}
	// Unrecognized record version
	else {
		G_error_message_m12("%s(): Unrecognized CSti Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m12     REC_check_CSti_type_alignment_m12(ui1 *bytes)
{
	REC_CSti_v10_m12	*csti;
	TERN_m12                free_flag = FALSE_m12;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_CSti_v10_m12) != REC_CSti_v10_BYTES_m12)
		goto REC_CSti_v10_NOT_ALIGNED_m12;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_CSti_v10_BYTES_m12);
		free_flag = TRUE_m12;
	}

	csti = (REC_CSti_v10_m12 *) bytes;
	if (&csti->stimulus_duration != (si8 *) (bytes + REC_CSti_v10_STIMULUS_DURATION_OFFSET_m12))
		goto REC_CSti_v10_NOT_ALIGNED_m12;
	if (csti->task_type != (si1 *) (bytes + REC_CSti_v10_TASK_TYPE_OFFSET_m12))
		goto REC_CSti_v10_NOT_ALIGNED_m12;
	if (csti->stimulus_type != (si1 *) (bytes + REC_CSti_v10_STIMULUS_TYPE_OFFSET_m12))
		goto REC_CSti_v10_NOT_ALIGNED_m12;
	if (csti->patient_response != (si1 *) (bytes + REC_CSti_v10_PATIENT_RESPONSE_OFFSET_m12))
		goto REC_CSti_v10_NOT_ALIGNED_m12;

	// aligned
	if (free_flag == TRUE_m12)
		free((void *) bytes);

	if (globals_m12->verbose == TRUE_m12)
		printf_m12("%s(): REC_CSti_v10_m12 structure is aligned\n", __FUNCTION__);

	return(TRUE_m12);

	// not aligned
REC_CSti_v10_NOT_ALIGNED_m12:

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	G_error_message_m12("%s(): REC_CSti_v10_m12 structure is NOT aligned\n", __FUNCTION__);

	return(FALSE_m12);

}


//*************************************************************************************//
//*****************************   HFOc: CS HFO Detection   ****************************//
//*************************************************************************************//

void    REC_show_HFOc_type_m12(RECORD_HEADER_m12 *record_header)
{
	REC_HFOc_v11_m12	*hfoc_1;
	REC_HFOc_v12_m12	*hfoc_2;
	REC_HFOc_v13_m12	*hfoc_3;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// Versions 1.0-3
	if (record_header->version_major == 1 && record_header->version_minor <= 3) {
		switch (record_header->version_minor) {
			case 0:
				break;
			case 1:
				hfoc_1 = (REC_HFOc_v11_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);
				printf_m12("End Time: %ld\n", hfoc_1->end_time);
				printf_m12("Start Frequency: %f\n", hfoc_1->start_frequency);
				printf_m12("End Frequency: %f\n", hfoc_1->end_frequency);
				break;
			case 2:
				hfoc_2 = (REC_HFOc_v12_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);
				printf_m12("End Time: %ld\n", hfoc_2->end_time);
				printf_m12("Start Frequency: %f\n", hfoc_2->start_frequency);
				printf_m12("End Frequency: %f\n", hfoc_2->end_frequency);
				printf_m12("Start Times: %ld, %ld, %ld, %ld\n", hfoc_2->start_times[0], hfoc_2->start_times[1], hfoc_2->start_times[2], hfoc_2->start_times[3]);
				printf_m12("End Times: %ld, %ld, %ld, %ld\n", hfoc_2->end_times[0], hfoc_2->end_times[1], hfoc_2->end_times[2], hfoc_2->end_times[3]);
				printf_m12("Combinations: %f, %f, %f, %f\n", hfoc_2->combinations[0], hfoc_2->combinations[1], hfoc_2->combinations[2], hfoc_2->combinations[3]);
				break;
			case 3:
				hfoc_3 = (REC_HFOc_v13_m12 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m12);
				printf_m12("End Time: %ld\n", hfoc_3->end_time);
				printf_m12("Start Frequency: %f\n", hfoc_3->start_frequency);
				printf_m12("End Frequency: %f\n", hfoc_3->end_frequency);
				printf_m12("Start Times: %ld, %ld, %ld, %ld\n", hfoc_3->start_times[0], hfoc_3->start_times[1], hfoc_3->start_times[2], hfoc_3->start_times[3]);
				printf_m12("End Times: %ld, %ld, %ld, %ld\n", hfoc_3->end_times[0], hfoc_3->end_times[1], hfoc_3->end_times[2], hfoc_3->end_times[3]);
				printf_m12("Combinations: %f, %f, %f, %f\n", hfoc_3->combinations[0], hfoc_3->combinations[1], hfoc_3->combinations[2], hfoc_3->combinations[3]);
				printf_m12("Amplitudes: %f, %f, %f, %f\n", hfoc_3->amplitudes[0], hfoc_3->amplitudes[1], hfoc_3->amplitudes[2], hfoc_3->amplitudes[3]);
				printf_m12("Frequency Dominances: %f, %f, %f, %f\n", hfoc_3->frequency_dominances[0], hfoc_3->frequency_dominances[1], hfoc_3->frequency_dominances[2], hfoc_3->frequency_dominances[3]);
				printf_m12("Products: %f, %f, %f, %f\n", hfoc_3->products[0], hfoc_3->products[1], hfoc_3->products[2], hfoc_3->products[3]);
				printf_m12("Cycles: %f, %f, %f, %f\n", hfoc_3->cycles[0], hfoc_3->cycles[1], hfoc_3->cycles[2], hfoc_3->cycles[3]);
				break;
		}

	}
	// Unrecognized record version
	else {
		G_error_message_m12("%s(): Unrecognized HFOc Record version (%hhd.%hhd)\n", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;
}


TERN_m12     REC_check_HFOc_type_alignment_m12(ui1 *bytes)
{
	TERN_m12                free_flag = FALSE_m12;
	ui1			version;
	REC_HFOc_v11_m12	*hfoc_1;
	REC_HFOc_v12_m12	*hfoc_2;
	REC_HFOc_v13_m12	*hfoc_3;

#ifdef FN_DEBUG_m12
	G_message_m12("%s()\n", __FUNCTION__);
#endif
	
	// check overall size
	if (sizeof(REC_HFOc_v11_m12) != REC_HFOc_v11_BYTES_m12) {
		version = 1;
		goto REC_HFOc_NOT_ALIGNED_m12;
	}
	if (sizeof(REC_HFOc_v12_m12) != REC_HFOc_v12_BYTES_m12) {
		version = 2;
		goto REC_HFOc_NOT_ALIGNED_m12;
	}
	if (sizeof(REC_HFOc_v13_m12) != REC_HFOc_v13_BYTES_m12) {
		version = 3;
		goto REC_HFOc_NOT_ALIGNED_m12;
	}

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(REC_HFOc_v13_BYTES_m12);
		free_flag = TRUE_m12;
	}

	// version 1.1
	version = 1;
	hfoc_1 = (REC_HFOc_v11_m12 *) bytes;
	if (&hfoc_1->end_time != (si8 *) (bytes + REC_HFOc_v11_END_TIME_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_1->start_frequency != (sf4 *) (bytes + REC_HFOc_v11_START_FREQUENCY_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_1->end_frequency != (sf4 *) (bytes + REC_HFOc_v11_END_FREQUENCY_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;

	// version 1.2
	version = 2;
	hfoc_2 = (REC_HFOc_v12_m12 *) bytes;
	if (&hfoc_2->end_time != (si8 *) (bytes + REC_HFOc_v12_END_TIME_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_2->start_frequency != (sf4 *) (bytes + REC_HFOc_v12_START_FREQUENCY_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_2->end_frequency != (sf4 *) (bytes + REC_HFOc_v12_END_FREQUENCY_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_2->start_times != (si8 (*)[REC_HFOc_NUMBER_OF_BANDS_m12]) (bytes + REC_HFOc_v12_START_TIMES_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_2->end_times != (si8 (*)[REC_HFOc_NUMBER_OF_BANDS_m12]) (bytes + REC_HFOc_v12_END_TIMES_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_2->combinations != (sf4 (*)[REC_HFOc_NUMBER_OF_BANDS_m12]) (bytes + REC_HFOc_v12_COMBINATIONS_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;

	// version 1.3
	version = 3;
	hfoc_3 = (REC_HFOc_v13_m12 *) bytes;
	if (&hfoc_3->end_time != (si8 *) (bytes + REC_HFOc_v13_END_TIME_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_3->start_frequency != (sf4 *) (bytes + REC_HFOc_v13_START_FREQUENCY_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_3->end_frequency != (sf4 *) (bytes + REC_HFOc_v13_END_FREQUENCY_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_3->start_times != (si8 (*)[REC_HFOc_NUMBER_OF_BANDS_m12]) (bytes + REC_HFOc_v13_START_TIMES_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_3->end_times != (si8 (*)[REC_HFOc_NUMBER_OF_BANDS_m12]) (bytes + REC_HFOc_v13_END_TIMES_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_3->combinations != (sf4 (*)[REC_HFOc_NUMBER_OF_BANDS_m12]) (bytes + REC_HFOc_v13_COMBINATIONS_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_3->amplitudes != (sf4 (*)[REC_HFOc_NUMBER_OF_BANDS_m12]) (bytes + REC_HFOc_v13_AMPLITUDES_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_3->frequency_dominances != (sf4 (*)[REC_HFOc_NUMBER_OF_BANDS_m12]) (bytes + REC_HFOc_v13_FREQUENCY_DOMINANCES_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_3->products != (sf4 (*)[REC_HFOc_NUMBER_OF_BANDS_m12]) (bytes + REC_HFOc_v13_PRODUCTS_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;
	if (&hfoc_3->cycles != (sf4 (*)[REC_HFOc_NUMBER_OF_BANDS_m12]) (bytes + REC_HFOc_v13_CYCLES_OFFSET_m12))
		goto REC_HFOc_NOT_ALIGNED_m12;

	// aligned
	if (free_flag == TRUE_m12)
		free((void *) bytes);

	if (globals_m12->verbose == TRUE_m12)
		printf_m12("%s(): all REC_HFOc_v1x_m12 structures are aligned\n", __FUNCTION__);

	return(TRUE_m12);

	// not aligned
REC_HFOc_NOT_ALIGNED_m12:

	if (free_flag == TRUE_m12)
		free((void *) bytes);

	G_error_message_m12("%s(): REC_HFOc_v1%hhu_m12 structure is NOT aligned\n", __FUNCTION__, version);

	return(FALSE_m12);

}


//*************************************************************************************//
//********************************   New Record Type   ********************************//
//*************************************************************************************//



