

//**********************************************************************************//
//****************************  MED 1.0 C Library Records **************************//
//**********************************************************************************//


// Multiscale Electrophysiology Data (MED) Format version 1.0
// Written by Matt Stead
// Copyright Dark Horse Neuro Inc, 2020

// MED derives from the Multiscale Electrophysiology Format (MEF), versions 1-3 which are in the public domain.
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


#include "medrec_m10.h"


//*************************************************************************************//
//**********************************   show_record()   ********************************//
//*************************************************************************************//

void	show_record_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_HEADER_m10 *record_header, si8 record_number)
{
        extern GLOBALS_m10      *globals_m10;
        si4	                i, decryption_blocks;
        ui4                     type_code;
        ui1	                *ui1_p, *decryption_key;
	si1	                time_str[TIME_STRING_BYTES_m10], hex_str[HEX_STRING_BYTES_m10(CRC_BYTES_m10)];
	PASSWORD_DATA_m10	*pwd;
        
        
	pwd = fps->password_data;
        
        type_code = record_header->type_code;
        decrypt_records_m10(fps, record_header, 1);
        
	// display record header fields
        if (record_number != NO_RECORD_NUMBER_m10)
                printf("Record Number: %ld\n", record_number);
	printf("---------------- Record Header - START ----------------\n");
	if (record_header->record_CRC == RECORD_HEADER_RECORD_CRC_NO_ENTRY_m10) {
		printf("Record CRC: no entry\n");
	} else {
                generate_hex_string_m10((ui1 *) &record_header->record_CRC, CRC_BYTES_m10, hex_str);
		printf("Record CRC: %s\n", hex_str);
        }
	if (type_code) {
                generate_hex_string_m10((ui1 *) record_header->type_string, CRC_BYTES_m10, hex_str);
		printf("Record Type String: %s\n", record_header->type_string);
	} else {
		printf("Record Type String: no entry\n");
	}
	if (record_header->version_major == RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m10 || record_header->version_minor == RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m10) {
		if (record_header->version_major == RECORD_HEADER_VERSION_MAJOR_NO_ENTRY_m10)
			printf("Record Version Major: no entry\n");
		else
			printf("Record Version Major: %u\n", record_header->version_major);
		if (record_header->version_minor == RECORD_HEADER_VERSION_MINOR_NO_ENTRY_m10)
			printf("Record Version Minor: no entry\n");
		else
			printf("Record Version Minor: %u\n", record_header->version_minor);
        } else {
		printf("Record Version: %hu.%hu\n", record_header->version_major, record_header->version_minor);
        }
        printf("Record Encryption Level: %hd ", record_header->encryption_level);
	if (record_header->encryption_level == NO_ENCRYPTION_m10)
		printf("(none)\n");
	else if (record_header->encryption_level == LEVEL_1_ENCRYPTION_m10)
		printf("(level 1, currently encrypted)\n");
	else if (record_header->encryption_level == LEVEL_2_ENCRYPTION_m10)
		printf("(level 2, currently encrypted)\n");
	else if (record_header->encryption_level == LEVEL_1_ENCRYPTION_DECRYPTED_m10)
		printf("(level 1, currently decrypted)\n");
	else if (record_header->encryption_level == LEVEL_2_ENCRYPTION_DECRYPTED_m10)
		printf("(level 2, currently decrypted)\n");
	else
		printf("(unrecognized code)\n");
	if (record_header->total_record_bytes == RECORD_HEADER_TOTAL_RECORD_BYTES_NO_ENTRY_m10)
		printf("Record Total Record Bytes: no entry\n");
	else
		printf("Record Total Record Bytes: %u\n", record_header->total_record_bytes);

	if (record_header->start_time == RECORD_HEADER_START_TIME_NO_ENTRY_m10)
		printf("Record Start Time: no entry\n");
	else {
		time_string_m10(record_header->start_time, time_str, TRUE_m10, FALSE_m10);
		printf("Record Start Time: %ld (µUTC), %s\n", ABS_m10(record_header->start_time), time_str);
	}
	printf("----------------- Record Header - END -----------------\n");

        
	// decrypt record body if necesary & access level is sufficient
	printf("----------------- Record Body - START -----------------\n");
	if (record_header->encryption_level > NO_ENCRYPTION_m10) {
                if (pwd != NULL) {
                        if (pwd->access_level >= record_header->encryption_level) {
                                if (record_header->encryption_level == LEVEL_1_ENCRYPTION_m10)
                                        decryption_key = pwd->level_1_encryption_key;
                                else
                                        decryption_key = pwd->level_2_encryption_key;
                                decryption_blocks = (record_header->total_record_bytes - RECORD_HEADER_BYTES_m10) / ENCRYPTION_BLOCK_BYTES_m10;
                                ui1_p = (ui1 *) record_header + RECORD_HEADER_BYTES_m10;
                                for (i = decryption_blocks; i--;) {
                                        AES_decrypt_m10(ui1_p, ui1_p, NULL, decryption_key);
                                        ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
                                }
                                record_header->encryption_level = -record_header->encryption_level;  // mark as currently decrypted
                                printf("                (record now decrypted)\n");
			} else {
				printf("No access to this record\n");
				printf("------------------ Record Body - END ------------------\n\n");
				return;
			}
                } else {
                        printf("No access to this record\n");
                        printf("------------------ Record Body - END ------------------\n\n");
                        return;
                }
        }
	
	// pass the display off to custom functions - new records types should be added here (maintain alphabetical order of record types)
	switch (type_code) {
                case REC_Sgmt_TYPE_CODE_m10:
                        show_rec_Sgmt_type_m10(record_header);
                        break;
		case REC_Stat_TYPE_CODE_m10:
			show_rec_Stat_type_m10(record_header);
			break;
		case REC_Note_TYPE_CODE_m10:
			show_rec_Note_type_m10(record_header);
			break;
		case REC_EDFA_TYPE_CODE_m10:
			show_rec_EDFA_type_m10(record_header);
			break;
		case REC_Seiz_TYPE_CODE_m10:
			show_rec_Seiz_type_m10(record_header);
			break;
		case REC_SyLg_TYPE_CODE_m10:
			show_rec_SyLg_type_m10(record_header);
			break;
                case REC_NlxP_TYPE_CODE_m10:
                        show_rec_NlxP_type_m10(record_header);
                        break;
		default:
			warning_message_m10("%s(): 0x%x is an unrecognized record type code", __FUNCTION__, type_code);
			break;
	}
	printf("------------------ Record Body - END ------------------\n\n");
        
        return;
}


//*************************************************************************************//
//*********************   check_record_structure_alignments()   ***********************//
//*************************************************************************************//

TERN_m10	check_record_structure_alignments_m10(ui1 *bytes)
{
 	TERN_m10		return_value, free_flag = FALSE_m10;
        extern GLOBALS_m10	*globals_m10;

        
	// see if already checked
	if (globals_m10->all_record_structures_aligned != UNKNOWN_m10)
		return(globals_m10->all_record_structures_aligned);
        
	return_value = TRUE_m10;
	if (bytes == NULL) {
		bytes = (ui1 *) e_malloc_m10(LARGEST_RECORD_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	
	// check all structures - add new functions here
        if ((check_rec_Sgmt_type_alignment_m10(bytes)) == FALSE_m10)
                return_value = FALSE_m10;
	if ((check_rec_Stat_type_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_rec_Note_type_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_rec_EDFA_type_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_rec_Seiz_type_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_rec_SyLg_type_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
        if ((check_rec_NlxP_type_alignment_m10(bytes)) == FALSE_m10)
                return_value = FALSE_m10;

        if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (return_value == TRUE_m10) {
		globals_m10->all_record_structures_aligned = TRUE_m10;
		if (globals_m10->verbose == TRUE_m10)
			printf("%s(): All Record structures are aligned\n", __FUNCTION__);
	} else {
		globals_m10->all_record_structures_aligned = FALSE_m10;
		error_message_m10("%s(): One or more Record structures are NOT aligned", __FUNCTION__);
	}
	
	return(return_value);
}


//*************************************************************************************//
//*******************************   Sgmt: Segment Record   ****************************//
//*************************************************************************************//

void    show_rec_Sgmt_type_m10(RECORD_HEADER_m10 *record_header)
{
        REC_Sgmt_v10_m10        *Sgmt;
        si1                     time_str[TIME_STRING_BYTES_m10], hex_str[HEX_STRING_BYTES_m10(8)], *segment_description;
        
        
        // Version 1.0
        if (record_header->version_major == 1 && record_header->version_minor == 0) {
                Sgmt = (REC_Sgmt_v10_m10 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m10);
                
                time_string_m10(Sgmt->end_time, time_str, TRUE_m10, FALSE_m10);
                printf("End Time: %ld (µUTC), %s\n", ABS_m10(Sgmt->end_time), time_str);
                if (Sgmt->absolute_start_sample_number == REC_Sgmt_v10_ABSOLUTE_START_SAMPLE_NUMBER_NO_ENTRY_m10)
                        printf("Absolute Start Sample Number: no entry\n");
                else
                        printf("Absolute Start Sample Number: %ld\n", Sgmt->absolute_start_sample_number);
                if (Sgmt->absolute_end_sample_number == REC_Sgmt_v10_ABSOLUTE_END_SAMPLE_NUMBER_NO_ENTRY_m10)
                        printf("Absolute End Sample Number: no entry\n");
                else
                        printf("Absolute End Sample Number: %ld\n", Sgmt->absolute_end_sample_number);
                generate_hex_string_m10((ui1 *) &Sgmt->segment_UID, 8, hex_str);
                printf("Segment UID: %s\n", hex_str);
                if (Sgmt->segment_number == REC_Sgmt_v10_SEGMENT_NUMBER_NO_ENTRY_m10)
                        printf("Segment Number: no entry\n");
                else
                        printf("Segment Number: %d\n", Sgmt->segment_number);
                
                if (Sgmt->acquisition_channel_number == REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_ALL_CHANNELS_m10)
                        printf("Acquisition Channel Number: all channels\n");
                else if (Sgmt->acquisition_channel_number == REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m10)
                        printf("Acquisition Channel Number: no entry\n");
                else
                        printf("Acquisition Channel Number: %d\n", Sgmt->acquisition_channel_number);
                
                if (Sgmt->sampling_frequency == REC_Sgmt_v10_SAMPLING_FREQUENCY_NO_ENTRY_m10)
                        printf("Sampling Frequency: no entry\n");
                else if (Sgmt->sampling_frequency == REC_Sgmt_v10_SAMPLING_FREQUENCY_VARIABLE_m10)
                        printf("Sampling Frequency: variable\n");
                else
                        printf("Sampling Frequency: %lf\n", Sgmt->sampling_frequency);

                segment_description = (si1 *) Sgmt + REC_Sgmt_v10_SEGMENT_DESCRIPTION_OFFSET_m10;
                if (*segment_description)
                        UTF8_printf_m10("Segment Description: %s\n", segment_description);
                else
                        printf("Segment Description: no entry\n");
        }
        // Unrecognized record version
        else {
                error_message_m10("%s(): Unrecognized Sgmt Record version (%hhd.%hhd)", __FUNCTION__, record_header->version_major, record_header->version_minor);
        }
        
        return;
}


TERN_m10     check_rec_Sgmt_type_alignment_m10(ui1 *bytes)
{
        REC_Sgmt_v10_m10        *Sgmt;
        TERN_m10                free_flag = FALSE_m10;
        extern GLOBALS_m10      *globals_m10;


        // check overall size
        if (sizeof(REC_Sgmt_v10_m10) != REC_Sgmt_v10_BYTES_m10)
                goto REC_Sgmt_v10_NOT_ALIGNED_m10;

        // check fields
        if (bytes == NULL) {
                bytes = (ui1 *) e_malloc_m10(LARGEST_RECORD_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                free_flag = TRUE_m10;
        }
        Sgmt = (REC_Sgmt_v10_m10 *) bytes;
        if (&Sgmt->end_time != (si8 *) (bytes + REC_Sgmt_v10_END_TIME_OFFSET_m10))
                goto REC_Sgmt_v10_NOT_ALIGNED_m10;
        if (&Sgmt->absolute_start_sample_number != (si8 *) (bytes + REC_Sgmt_v10_ABSOLUTE_START_SAMPLE_NUMBER_OFFSET_m10))
                goto REC_Sgmt_v10_NOT_ALIGNED_m10;
        if (&Sgmt->absolute_start_frame_number != (si8 *) (bytes + REC_Sgmt_v10_ABSOLUTE_START_FRAME_NUMBER_OFFSET_m10))
                goto REC_Sgmt_v10_NOT_ALIGNED_m10;
        if (&Sgmt->absolute_end_sample_number != (si8 *) (bytes + REC_Sgmt_v10_ABSOLUTE_END_SAMPLE_NUMBER_OFFSET_m10))
                goto REC_Sgmt_v10_NOT_ALIGNED_m10;
        if (&Sgmt->absolute_end_frame_number != (si8 *) (bytes + REC_Sgmt_v10_ABSOLUTE_END_FRAME_NUMBER_OFFSET_m10))
                goto REC_Sgmt_v10_NOT_ALIGNED_m10;
        if (&Sgmt->segment_UID != (ui8 *) (bytes + REC_Sgmt_v10_SEGMENT_UID_OFFSET_m10))
                goto REC_Sgmt_v10_NOT_ALIGNED_m10;
        if (&Sgmt->segment_number != (si4 *) (bytes + REC_Sgmt_v10_SEGMENT_NUMBER_OFFSET_m10))
                goto REC_Sgmt_v10_NOT_ALIGNED_m10;
        if (&Sgmt->acquisition_channel_number != (si4 *) (bytes + REC_Sgmt_v10_ACQUISITION_CHANNEL_NUMBER_OFFSET_m10))
                goto REC_Sgmt_v10_NOT_ALIGNED_m10;
        if (&Sgmt->sampling_frequency != (sf8 *) (bytes + REC_Sgmt_v10_SAMPLING_FREQUENCY_OFFSET_m10))
                goto REC_Sgmt_v10_NOT_ALIGNED_m10;

        // aligned
        if (free_flag == TRUE_m10)
                e_free_m10(bytes, __FUNCTION__, __LINE__);
        
        if (globals_m10->verbose == TRUE_m10)
		printf("%s(): REC_Sgmt_v10_m10 structure is aligned\n", __FUNCTION__);
        
        return(TRUE_m10);
        
        // not aligned
        REC_Sgmt_v10_NOT_ALIGNED_m10:
        
        if (free_flag == TRUE_m10)
                e_free_m10(bytes, __FUNCTION__, __LINE__);
        
        error_message_m10("%s(): REC_Sgmt_v10_m10 structure is NOT aligned", __FUNCTION__);
        
        return(FALSE_m10);

}


//*************************************************************************************//
//*******************************   Stat: Segment Record   ****************************//
//*************************************************************************************//

void    show_rec_Stat_type_m10(RECORD_HEADER_m10 *record_header)
{
	REC_Stat_v10_m10        *Stat;
	
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		Stat = (REC_Stat_v10_m10 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m10);
		if (Stat->minimum == REC_Stat_v10_MINIMUM_NO_ENTRY_m10)
			printf("Minimum: no entry\n");
		else
			printf("Minimum: %d\n", Stat->minimum);
		if (Stat->maximum == REC_Stat_v10_MAXIMUM_NO_ENTRY_m10)
			printf("Maximum: no entry\n");
		else
			printf("Maximum: %d\n", Stat->maximum);
		if (Stat->mean == REC_Stat_v10_MEAN_NO_ENTRY_m10)
			printf("Mean: no entry\n");
		else
			printf("Mean: %d\n", Stat->mean);
		if (Stat->median == REC_Stat_v10_MEDIAN_NO_ENTRY_m10)
			printf("Median: no entry\n");
		else
			printf("Median: %d\n", Stat->median);
		if (Stat->mode == REC_Stat_v10_MODE_NO_ENTRY_m10)
			printf("Mode: no entry\n");
		else
			printf("Mode: %d\n", Stat->mode);
		if (isnan(Stat->variance))
			printf("Variance: no entry\n");
		else
			printf("Variance: %f\n", Stat->variance);
		if (isnan(Stat->skewness))
			printf("Skewness: no entry\n");
		else
			printf("Skewness: %f\n", Stat->skewness);
		if (isnan(Stat->kurtosis))
			printf("Kurtosis: no entry\n");
		else
			printf("Kurtosis: %f\n", Stat->kurtosis);
	}
	// Unrecognized record version
	else {
		error_message_m10("%s(): Unrecognized Stat Record version (%hhd.%hhd)", __FUNCTION__, record_header->version_major, record_header->version_minor);
	}

	return;

}


TERN_m10     check_rec_Stat_type_alignment_m10(ui1 *bytes)
{
	REC_Stat_v10_m10        *Stat;
	TERN_m10                free_flag = FALSE_m10;
	extern GLOBALS_m10      *globals_m10;


	// check overall size
	if (sizeof(REC_Stat_v10_m10) != REC_Stat_v10_BYTES_m10)
		goto REC_Stat_v10_NOT_ALIGNED_m10;

	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) e_malloc_m10(LARGEST_RECORD_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	Stat = (REC_Stat_v10_m10 *) bytes;
	if (&Stat->minimum != (si4 *) (bytes + REC_Stat_v10_MINIMUM_OFFSET_m10))
		goto REC_Stat_v10_NOT_ALIGNED_m10;
	if (&Stat->maximum != (si4 *) (bytes + REC_Stat_v10_MAXIMUM_OFFSET_m10))
		goto REC_Stat_v10_NOT_ALIGNED_m10;
	if (&Stat->mean != (si4 *) (bytes + REC_Stat_v10_MEAN_OFFSET_m10))
		goto REC_Stat_v10_NOT_ALIGNED_m10;
	if (&Stat->median != (si4 *) (bytes + REC_Stat_v10_MEDIAN_OFFSET_m10))
		goto REC_Stat_v10_NOT_ALIGNED_m10;
	if (&Stat->mode != (si4 *) (bytes + REC_Stat_v10_MODE_OFFSET_m10))
		goto REC_Stat_v10_NOT_ALIGNED_m10;
	if (&Stat->variance != (sf4 *) (bytes + REC_Stat_v10_VARIANCE_OFFSET_m10))
		goto REC_Stat_v10_NOT_ALIGNED_m10;
	if (&Stat->skewness != (sf4 *) (bytes + REC_Stat_v10_SKEWNESS_OFFSET_m10))
		goto REC_Stat_v10_NOT_ALIGNED_m10;
	if (&Stat->kurtosis != (sf4 *) (bytes + REC_Stat_v10_KURTOSIS_OFFSET_m10))
		goto REC_Stat_v10_NOT_ALIGNED_m10;

	// aligned
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		printf("%s(): REC_Stat_v10_m10 structure is aligned\n", __FUNCTION__);
	
	return(TRUE_m10);
	
	// not aligned
	REC_Stat_v10_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	error_message_m10("%s(): REC_Stat_v10_m10 structure is NOT aligned", __FUNCTION__);
	
	return(FALSE_m10);
}


//*************************************************************************************//
//********************************   Note: Note Record   ******************************//
//*************************************************************************************//

void	show_rec_Note_type_m10(RECORD_HEADER_m10 *record_header)
{
        si1	*note_text;
  
        
        // Version 1.0
        if (record_header->version_major == 1 && record_header->version_minor == 0) {
                note_text = (si1 *) record_header + RECORD_HEADER_BYTES_m10;
                if (*note_text)
                        UTF8_printf_m10("Note Text: %s\n", note_text);
                else
                        printf("Note Text: no entry\n");
        }
        // Unrecognized record version
        else {
                error_message_m10("%s(): Unrecognized Note Record version (%hhd.%hhd)", __FUNCTION__, record_header->version_major, record_header->version_minor);
        }
        
        return;
}


TERN_m10        check_rec_Note_type_alignment_m10(ui1 *bytes)
{
        // no structures to check
        return(TRUE_m10);
}


//*************************************************************************************//
//******************   EDFA: European Data Format Annotation Record   *****************//
//*************************************************************************************//

void	show_rec_EDFA_type_m10(RECORD_HEADER_m10 *record_header)
{
	REC_EDFA_v10_m10	*edfa;
	si1		        *annotation;
	
	
	// Version 1.0
	if (record_header->version_major == 1 && record_header->version_minor == 0) {
		edfa = (REC_EDFA_v10_m10 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m10);
                printf("Duration %ld microseconds\n", edfa->duration);
                annotation = (si1 *) edfa + REC_EDFA_v10_ANNOTATION_OFFSET_m10;
                if (*annotation)
                        UTF8_printf_m10("Annotation: %s\n", annotation);
                else
                        printf("Annotation: no entry\n");
	}
	// Unrecognized record version
	else {
		error_message_m10("%s(): Unrecognized EDFA Record version (%hhd.%hhd)", __FUNCTION__, record_header->version_major, record_header->version_minor);	}
	
	return;
}


TERN_m10	check_rec_EDFA_type_alignment_m10(ui1 *bytes)
{
	REC_EDFA_v10_m10	*edfa;
	TERN_m10		free_flag = FALSE_m10;
	extern GLOBALS_m10	*globals_m10;


	// check overall size
	if (sizeof(REC_EDFA_v10_m10) != REC_EDFA_v10_BYTES_m10)
		goto REC_EDFA_v10_NOT_ALIGNED_m10;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) e_malloc_m10(LARGEST_RECORD_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	edfa = (REC_EDFA_v10_m10 *) bytes;
	if (&edfa->duration != (si8 *) (bytes + REC_EDFA_v10_DURATION_OFFSET_m10))
		goto REC_EDFA_v10_NOT_ALIGNED_m10;
	
	// aligned
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		printf("%s(): REC_EDFA_v10_m10 structure is aligned\n", __FUNCTION__);
	
	return(TRUE_m10);
	
	// not aligned
	REC_EDFA_v10_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	error_message_m10("%s(): REC_EDFA_v10_m10 structure is NOT aligned", __FUNCTION__);
	
	return(FALSE_m10);
}



//*************************************************************************************//
//*******************************   Seiz: Seizure Record   ****************************//
//*************************************************************************************//

void	show_rec_Seiz_type_m10(RECORD_HEADER_m10 *record_header)
{
        si4			        i;
        TERN_m10                        mn1 = FALSE_m10, mn2 = FALSE_m10;
        REC_Seiz_v10_m10		*Seiz;
        REC_Seiz_v10_CHANNEL_m10	*chans;
	si1			        time_str[TIME_STRING_BYTES_m10];
        
        
        // Version 1.0
        if (record_header->version_major == 1 && record_header->version_minor == 0) {
                Seiz = (REC_Seiz_v10_m10 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m10);
                time_string_m10(Seiz->latest_offset_time, time_str, TRUE_m10, FALSE_m10);
		printf("Latest Offset Time: %ld (µUTC), %s\n", ABS_m10(Seiz->latest_offset_time), time_str);
		printf("Number of Channels: %d\n", Seiz->number_of_channels);
		printf("Onset Code: %d ", Seiz->onset_code);
                switch (Seiz->onset_code) {
                        case REC_Seiz_v10_ONSET_NO_ENTRY_m10:
                                printf("(no entry)\n");
                                break;
                        case REC_Seiz_v10_ONSET_UNKNOWN_m10:
                                printf("(unknown)\n");
                                break;
                        case REC_Seiz_v10_ONSET_FOCAL_m10:
                                printf("(focal)\n");
                                break;
                        case REC_Seiz_v10_ONSET_GENERALIZED_m10:
                                printf("(generalized)\n");
                                break;
                        case REC_Seiz_v10_ONSET_PROPAGATED_m10:
                                printf("(propagated)\n");
                                break;
                        case REC_Seiz_v10_ONSET_MIXED_m10:
                                printf("(mixed)\n");
                                break;
			default:
				warning_message_m10("%s(): %d is an unrecognized Seiz onset code", __FUNCTION__, Seiz->onset_code);
				break;
                }
                if (strlen(Seiz->marker_name_1))
                        mn1 = TRUE_m10;
                if (strlen(Seiz->marker_name_2))
                        mn2 = TRUE_m10;
                if (mn1 == TRUE_m10 && mn2 == TRUE_m10)
                	UTF8_printf_m10("Marker Names: %s %s\n", Seiz->marker_name_1, Seiz->marker_name_2);
                else if (mn1 == TRUE_m10)
			UTF8_printf_m10("Marker Name 1: %s\nMarker Name 2: no entry\n", Seiz->marker_name_1);
                else if (mn2 == TRUE_m10)
			UTF8_printf_m10("Marker Name 1: no_entry\nMarker Name 2: %s\n", Seiz->marker_name_2);
                else
                        printf("Marker Names: no_entry\n");
                if (strlen(Seiz->annotation))
                        UTF8_printf_m10("Annotation: %s\n", Seiz->annotation);
                else
                        printf("Annotation: no entry\n");
                chans = (REC_Seiz_v10_CHANNEL_m10 *) ((ui1 *) Seiz + REC_Seiz_v10_CHANNELS_OFFSET_m10);
                for (i = 0; i < Seiz->number_of_channels; ++i) {
                        if (strlen(chans[i].name))
                                UTF8_printf_m10("Channel Name: %s\n", chans[i].name);
                        else
                                printf("Channel Name: no entry\n");
                        time_string_m10(chans[i].onset_time, time_str, TRUE_m10, FALSE_m10);
                        printf("\tOnset Time: %ld (µUTC), %s\n", ABS_m10(chans[i].onset_time), time_str);
                        time_string_m10(chans[i].offset_time, time_str, TRUE_m10, FALSE_m10);
                        printf("\tOffset Time: %ld (µUTC), %s\n", ABS_m10(chans[i].offset_time), time_str);
                        if (chans[i].segment_number == REC_Seiz_v10_CHANNEL_SEGMENT_NUMBER_NO_ENTRY_m10)
                                printf("Segment Number: no entry\n");
                        else
                                printf("Segment Number: %d\n", chans[i].segment_number);
                }
        }
        // Unrecognized record version
        else {
		error_message_m10("%s(): Unrecognized Seiz Record version (%hhd.%hhd)", __FUNCTION__, record_header->version_major, record_header->version_minor);        }
         
        return;
}


TERN_m10	check_rec_Seiz_type_alignment_m10(ui1 *bytes)
{
	REC_Seiz_v10_m10		*Seiz;
        REC_Seiz_v10_CHANNEL_m10	*chan;
	TERN_m10			free_flag = FALSE_m10;
        ui1			        *chan_bytes;
        extern GLOBALS_m10	        *globals_m10;
	
	
	// check overall sizes
	if (sizeof(REC_Seiz_v10_m10) != REC_Seiz_v10_BYTES_m10)
		goto REC_Seiz_v10_NOT_ALIGNED_m10;
	if (sizeof(REC_Seiz_v10_CHANNEL_m10) != REC_Seiz_v10_CHANNEL_BYTES_m10)
		goto REC_Seiz_v10_NOT_ALIGNED_m10;
	
	// check fields - base structure
	if (bytes == NULL) {
		bytes = (ui1 *) e_malloc_m10(LARGEST_RECORD_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	Seiz = (REC_Seiz_v10_m10 *) bytes;
	if (&Seiz->latest_offset_time != (si8 *) (bytes + REC_Seiz_v10_LATEST_OFFSET_TIME_OFFSET_m10))
		goto REC_Seiz_v10_NOT_ALIGNED_m10;
	if (&Seiz->number_of_channels != (si4 *) (bytes + REC_Seiz_v10_NUMBER_OF_CHANNELS_OFFSET_m10))
		goto REC_Seiz_v10_NOT_ALIGNED_m10;
	if (&Seiz->onset_code != (si4 *) (bytes + REC_Seiz_v10_ONSET_CODE_OFFSET_m10))
		goto REC_Seiz_v10_NOT_ALIGNED_m10;
	if (Seiz->marker_name_1 != (si1 *) (bytes + REC_Seiz_v10_MARKER_NAME_1_OFFSET_m10))
		goto REC_Seiz_v10_NOT_ALIGNED_m10;
	if (Seiz->marker_name_2 != (si1 *) (bytes + REC_Seiz_v10_MARKER_NAME_2_OFFSET_m10))
		goto REC_Seiz_v10_NOT_ALIGNED_m10;
	if (Seiz->annotation != (si1 *) (bytes + REC_Seiz_v10_ANNOTATION_OFFSET_m10))
		goto REC_Seiz_v10_NOT_ALIGNED_m10;
	// check fields - channel structures
        chan_bytes = bytes + REC_Seiz_v10_CHANNELS_OFFSET_m10;
	chan = (REC_Seiz_v10_CHANNEL_m10 *) chan_bytes;
	if (chan->name != (si1 *) (chan_bytes + REC_Seiz_v10_CHANNEL_NAME_OFFSET_m10))
		goto REC_Seiz_v10_NOT_ALIGNED_m10;
	if (&chan->onset_time != (si8 *) (chan_bytes + REC_Seiz_v10_CHANNEL_ONSET_TIME_OFFSET_m10))
		goto REC_Seiz_v10_NOT_ALIGNED_m10;
	if (&chan->offset_time != (si8 *) (chan_bytes + REC_Seiz_v10_CHANNEL_OFFSET_TIME_OFFSET_m10))
		goto REC_Seiz_v10_NOT_ALIGNED_m10;
        if (&chan->segment_number != (si4 *) (chan_bytes + REC_Seiz_v10_CHANNEL_SEGMENT_NUMBER_OFFSET_m10))
                goto REC_Seiz_v10_NOT_ALIGNED_m10;
        if (chan->pad_bytes != (ui1 *) (chan_bytes + REC_Seiz_v10_CHANNEL_PAD_BYTES_OFFSET_m10))
                goto REC_Seiz_v10_NOT_ALIGNED_m10;

	// aligned
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		printf("%s(): REC_Seiz_v10_m10 structure is aligned\n", __FUNCTION__);
	
        return(TRUE_m10);
	
	// not aligned
	REC_Seiz_v10_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	(void) error_message_m10("%s(): REC_Seiz_v10_m10 structure is NOT aligned", __FUNCTION__);
        
        return(FALSE_m10);
}


//*************************************************************************************//
//*****************************   SyLg: System Log Record   ***************************//
//*************************************************************************************//

void	show_rec_SyLg_type_m10(RECORD_HEADER_m10 *record_header)
{
        si1	*log_entry;
        
        
        // Version 1.0
        if (record_header->version_major == 1 && record_header->version_minor == 0) {
                log_entry = (si1 *) record_header + RECORD_HEADER_BYTES_m10;
                if (*log_entry)
                        UTF8_printf_m10("System Log entry:\n%s\n", log_entry);
                else
                        printf("System Log entry: no entry\n");
        }
        // Unrecognized record version
        else {
		error_message_m10("%s(): Unrecognized SyLg Record version (%hhd.%hhd)", __FUNCTION__, record_header->version_major, record_header->version_minor);
        }
        
        return;
}

TERN_m10	check_rec_SyLg_type_alignment_m10(ui1 *bytes)
{
        // no structures to check	
        return(TRUE_m10);
}



//*************************************************************************************//
//*********************   NlxP: NeuraLynx Parallel Port Record   **********************//
//*************************************************************************************//

void    show_rec_NlxP_type_m10(RECORD_HEADER_m10 *record_header)
{
        si1                     hex_str[HEX_STRING_BYTES_m10(4)];
        REC_NlxP_v10_m10        *nlxp;
        
        
        // Version 1.0
        if (record_header->version_major == 1 && record_header->version_minor == 0) {
                nlxp = (REC_NlxP_v10_m10 *) ((ui1 *) record_header + RECORD_HEADER_BYTES_m10);
                printf("Value: %u\n", nlxp->value);
                printf("Subport: %hhu\n", nlxp->subport);
                printf("Number of Subports: %hhu\n", nlxp->number_of_subports);
                printf("Trigger Mode: ");
                switch (nlxp->trigger_mode) {
                        case REC_NlxP_v10_NO_TRIGGER_m10:
                                printf("NO TRIGGER\n");
                                break;
                        case REC_NlxP_v10_ANY_BIT_CHANGE_m10:
                                printf("ANY BIT CHANGE\n");
                                break;
                        case REC_NlxP_v10_HIGH_BIT_SET_m10:
                                printf("HIGH BIT SET\n");
                                break;
                        default:
				warning_message_m10("%s(): Unrecognized trigger mode (%hhu)", __FUNCTION__, nlxp->trigger_mode);
                                break;
                }
                printf("Raw Port Value: %u  (unsigned dec)\n", nlxp->raw_port_value);
                generate_hex_string_m10((ui1 *) &nlxp->raw_port_value, 4, hex_str);
                printf("Raw Port Bytes: %s  (hex)\n", hex_str);
        }
        // Unrecognized record version
        else {
		error_message_m10("%s(): Unrecognized NlxP Record version (%hhd.%hhd)", __FUNCTION__, record_header->version_major, record_header->version_minor);
        }
        
        return;
}

TERN_m10     check_rec_NlxP_type_alignment_m10(ui1 *bytes)
{
        REC_NlxP_v10_m10        *nlxp;
        TERN_m10                free_flag = FALSE_m10;
        extern GLOBALS_m10      *globals_m10;


        // check overall size
        if (sizeof(REC_NlxP_v10_m10) != REC_NlxP_v10_BYTES_m10)
                goto REC_NlxP_v10_NOT_ALIGNED_m10;
        
        // check fields
        if (bytes == NULL) {
                bytes = (ui1 *) e_malloc_m10(LARGEST_RECORD_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                free_flag = TRUE_m10;
        }
        
        nlxp = (REC_NlxP_v10_m10 *) bytes;
        if (&nlxp->raw_port_value != (ui4 *) (bytes + REC_NlxP_v10_RAW_PORT_VALUE_OFFSET_m10))
                goto REC_NlxP_v10_NOT_ALIGNED_m10;
        if (&nlxp->value != (ui4 *) (bytes + REC_NlxP_v10_VALUE_OFFSET_m10))
                goto REC_NlxP_v10_NOT_ALIGNED_m10;
        if (&nlxp->subport != (ui1 *) (bytes + REC_NlxP_v10_SUBPORT_OFFSET_m10))
                goto REC_NlxP_v10_NOT_ALIGNED_m10;
        if (&nlxp->number_of_subports != (ui1 *) (bytes + REC_NlxP_v10_NUMBER_OF_SUBPORTS_OFFSET_m10))
                goto REC_NlxP_v10_NOT_ALIGNED_m10;
        if (&nlxp->trigger_mode != (ui1 *) (bytes + REC_NlxP_v10_TRIGGER_MODE_OFFSET_m10))
                goto REC_NlxP_v10_NOT_ALIGNED_m10;
        if (nlxp->pad_bytes != (ui1 *) (bytes + REC_NlxP_v10_PAD_BYTES_OFFSET_m10))
                goto REC_NlxP_v10_NOT_ALIGNED_m10;

        // aligned
        if (free_flag == TRUE_m10)
                e_free_m10(bytes, __FUNCTION__, __LINE__);
        
        if (globals_m10->verbose == TRUE_m10)
                printf("%s(): REC_NlxP_v10_m10 structure is aligned\n", __FUNCTION__);
        
        return(TRUE_m10);
        
        // not aligned
        REC_NlxP_v10_NOT_ALIGNED_m10:
        
        if (free_flag == TRUE_m10)
                e_free_m10(bytes, __FUNCTION__, __LINE__);
        
        error_message_m10("%s(): REC_NlxP_v10_m10 structure is NOT aligned", __FUNCTION__);
        
        return(FALSE_m10);

}



//*************************************************************************************//
//********************************   New Record Type   ********************************//
//*************************************************************************************//



