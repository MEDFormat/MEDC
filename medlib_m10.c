
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



#include "medlib_m10.h"
#include "medrec_m10.h"

// Globals
GLOBALS_m10	*globals_m10 = NULL;



//***********************************************************************//
//********************  GENERAL MED LIBRARY FUNCTIONS  ******************//
//***********************************************************************//


TERN_m10	adjust_open_file_limit_m10(si4 new_limit)
{
#if defined MACOS_m10 || defined LINUX_m10
	struct rlimit	resource_limit;
#endif


#if defined MACOS_m10 || defined LINUX_m10
	// change resource limits (note: must change before calling any functions that use system resources)
	getrlimit(RLIMIT_NOFILE, &resource_limit);  // get existing limit set
	resource_limit.rlim_cur = (rlim_t) new_limit;  // change open file limit
	if (setrlimit(RLIMIT_NOFILE, &resource_limit) == -1) {  // set limit set
		warning_message_m10("%s(): could not adjust process open file limit\n", __FUNCTION__);
		return(FALSE_m10);
	}
#endif
	
#ifdef WINDOWS_m10
	if (_setmaxstdio((int) new_limit) == -1) {  // change open file limit
		warning_message_m10("%s(): could not adjust process open file limit\n", __FUNCTION__);
		return(FALSE_m10);
	}
#endif
	return(TRUE_m10);
}



TERN_m10	all_zeros_m10(ui1* bytes, si4 field_length)
{
	while (field_length--)
		if (*bytes++)
			return(FALSE_m10);
	
	return(TRUE_m10);
}


CHANNEL_m10	*allocate_channel_m10(CHANNEL_m10 *chan, FILE_PROCESSING_STRUCT_m10 *proto_fps, si1 *enclosing_path, si1 *chan_name, ui4 type_code, si4 n_segs, TERN_m10 chan_recs, TERN_m10 seg_recs)
{
	si1	*type_str;
	si8	i;
	UNIVERSAL_HEADER_m10		*uh;
	FILE_PROCESSING_STRUCT_m10	*gen_fps;
	
	
	// enclosing_path is the path to the enclosing directory
	// chan_name is the base name, with no extension
	// if records are requested, enough memory for 1 record of size LARGEST_RECORD_BYTES_m10 is allocated (reallocate_file_processing_struct_m10() to change this)
	// if records are requested, enough memory for 1 record index is allocated (reallocate_file_processing_struct_m10() to change this)
	
	switch (type_code) {
		case TIME_SERIES_CHANNEL_TYPE_m10:
		case VIDEO_CHANNEL_TYPE_m10:
			break;
		default:
			error_message_m10("Error: unrecognized channel type code \"0x%x\" [function \"%s\", line %d]\n", type_code, __FUNCTION__, __LINE__);
			return(NULL);
	}
	type_str = MED_type_string_from_code_m10(type_code);
	
	if (chan == NULL)
		chan = (CHANNEL_m10 *) calloc_m10((size_t)1, sizeof(CHANNEL_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	if (type_code == TIME_SERIES_CHANNEL_TYPE_m10)
		gen_fps = allocate_file_processing_struct_m10(NULL, NULL, TIME_SERIES_METADATA_FILE_TYPE_CODE_m10, FPS_PROTOTYPE_BYTES_m10, proto_fps, FPS_PROTOTYPE_BYTES_m10);
	else // type_code == TIME_SERIES_CHANNEL_TYPE_m10
		gen_fps = allocate_file_processing_struct_m10(NULL, NULL, VIDEO_METADATA_FILE_TYPE_CODE_m10, FPS_PROTOTYPE_BYTES_m10, proto_fps, FPS_PROTOTYPE_BYTES_m10);
	gen_fps->fd = FPS_FD_EPHEMERAL_m10;
	
	chan->number_of_segments = n_segs;
	
	uh = gen_fps->universal_header;
	if (uh->channel_UID == UID_NO_ENTRY_m10)
		generate_UID_m10(&uh->channel_UID);
	uh->segment_number = UNIVERSAL_HEADER_CHANNEL_LEVEL_CODE_m10;
	strncpy_m10(uh->channel_name, chan_name, BASE_FILE_NAME_BYTES_m10);
	strncpy_m10(chan->name, chan_name, BASE_FILE_NAME_BYTES_m10);
	snprintf_m10(chan->path, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", enclosing_path, chan->name, type_str);
	chan->metadata_fps = gen_fps;
	
	if (chan_recs == TRUE_m10) {
		chan->record_data_fps = allocate_file_processing_struct_m10(NULL, NULL, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, gen_fps, 0);
		snprintf_m10(chan->record_data_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", chan->path, chan->name, RECORD_DATA_FILE_TYPE_STRING_m10);
		chan->record_indices_fps = allocate_file_processing_struct_m10(NULL, NULL, RECORD_INDICES_FILE_TYPE_CODE_m10, RECORD_INDEX_BYTES_m10, gen_fps, 0);
		snprintf_m10(chan->record_indices_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", chan->path, chan->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
	}
	else {
		chan->record_data_fps = chan->record_indices_fps = NULL;
	}
	
	if (n_segs) {
		chan->segments = (SEGMENT_m10 **) calloc_2D_m10((size_t) n_segs, 1, sizeof(SEGMENT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		for (i = 0; i < n_segs; ++i)
			allocate_segment_m10(chan->segments[i], gen_fps, chan->path, chan->name, type_code, (si4)i + 1, seg_recs);
	}
	
	return(chan);
}


FILE_PROCESSING_STRUCT_m10	*allocate_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps, si1 *full_file_name, ui4 type_code, si8 raw_data_bytes, FILE_PROCESSING_STRUCT_m10 *proto_fps, si8 bytes_to_copy)
{
	UNIVERSAL_HEADER_m10	*uh;
	ui1			*data_ptr;
	
	
	// allocate FPS
	if (fps == NULL) {
		fps = (FILE_PROCESSING_STRUCT_m10 *) calloc_m10((size_t)1, sizeof(FILE_PROCESSING_STRUCT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	}
	else {
		if (fps->raw_data != NULL)
			free((void *) fps->raw_data);
	}
	if (full_file_name != NULL)
		strncpy_m10(fps->full_file_name, full_file_name, FULL_FILE_NAME_BYTES_m10);
	if (*fps->full_file_name)
		if (type_code == UNKNOWN_TYPE_CODE_m10)
			type_code = MED_type_code_from_string_m10(fps->full_file_name);
	fps->mutex = FALSE_m10;
	if (proto_fps != NULL)
		if (proto_fps->password_data != NULL)
			fps->password_data = proto_fps->password_data;
	if (fps->password_data == NULL)
		fps->password_data = &globals_m10->password_data;
	
	// allocate raw_data
	fps->raw_data_bytes = (raw_data_bytes += UNIVERSAL_HEADER_BYTES_m10);  // all files start with universal header
	fps->raw_data = (ui1 *) calloc_m10((size_t)raw_data_bytes, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	uh = fps->universal_header = (UNIVERSAL_HEADER_m10 *)fps->raw_data;
	fps->fd = FPS_FD_NO_ENTRY_m10;
	fps->file_length = 0; // nothing read or written yet
	
	// if a prototype FILE_PROCESSING_STRUCT_m10 was passed - copy its directives, password data, universal_header, and raw data
	if (proto_fps != NULL) {
		fps->directives = proto_fps->directives;
		bytes_to_copy += UNIVERSAL_HEADER_BYTES_m10;  // all files start with universal header
		if ((bytes_to_copy > proto_fps->raw_data_bytes) || (bytes_to_copy > fps->raw_data_bytes))
			error_message_m10("%s(): copy request size exceeds avaiable data => no copying done\n", __FUNCTION__);
		else
			memcpy(fps->raw_data, proto_fps->raw_data, bytes_to_copy);
		uh->type_code = type_code;
		uh->header_CRC = uh->body_CRC = CRC_START_VALUE_m10;
		uh->number_of_entries = 0;
		uh->maximum_entry_size = 0;
	}
	else {
		(void)initialize_file_processing_directives_m10(&fps->directives);  // set directives to defaults
		initialize_universal_header_m10(fps, type_code, FALSE_m10, FALSE_m10);
	}
	generate_UID_m10(&uh->file_UID);
	uh->provenance_UID = uh->file_UID;  // if not originating file, caller should change provenance_UID to file_UID of originating file
	
	// set appropriate pointers (also set maximum entry size where it's a fixed value)
	data_ptr = fps->raw_data + UNIVERSAL_HEADER_BYTES_m10;
	switch (type_code) {
		case NO_TYPE_CODE_m10:
			break;
		case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
			fps->time_series_indices = (TIME_SERIES_INDEX_m10 *) data_ptr;
			uh->maximum_entry_size = TIME_SERIES_INDEX_BYTES_m10;
			break;
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
			fps->cps = NULL;  // CMP_PROCESSING_STRUCT allocation must be done seperately with CMP_allocate_processing_struct()
			break;
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
		case VIDEO_METADATA_FILE_TYPE_CODE_m10:
			fps->metadata = (METADATA_m10 *) data_ptr;
			uh->maximum_entry_size = METADATA_BYTES_m10;
			uh->number_of_entries = 1;
			break;
		case VIDEO_INDICES_FILE_TYPE_CODE_m10:
			fps->video_indices = (VIDEO_INDEX_m10 *) data_ptr;
			uh->maximum_entry_size = VIDEO_INDEX_BYTES_m10;
			break;
		case RECORD_DATA_FILE_TYPE_CODE_m10:
			fps->records = (ui1 *) data_ptr;
			break;
		case RECORD_INDICES_FILE_TYPE_CODE_m10:
			fps->record_indices = (RECORD_INDEX_m10 *) data_ptr;
			uh->maximum_entry_size = RECORD_INDEX_BYTES_m10;
			break;
		default:
			free_file_processing_struct_m10(fps, FALSE_m10);
			error_message_m10("%s(): unrecognized type code (code = 0x%08x)\n", type_code, __FUNCTION__);
			return(NULL);
	}
	
	return(fps);
}


SEGMENT_m10	*allocate_segment_m10(SEGMENT_m10 *seg, FILE_PROCESSING_STRUCT_m10 *proto_fps, si1 *enclosing_path, si1 *chan_name, ui4 type_code, si4 seg_num, TERN_m10 seg_recs)
{
	si1                     num_str[FILE_NUMBERING_DIGITS_m10 + 1];
	UNIVERSAL_HEADER_m10	*uh;
	
	// enclosing_path is the path to the enclosing directory
	// chan_name is the base name, with no extension
	// if time series channels are requested, the CMP_PROCESSING_STRUCT_m10 structures must be allocated seperately.
	// if time series segments are requested, enough memory for one time series index is allocated.
	// if records are requested, enough memory for 1 record of size LARGEST_RECORD_BYTES_m10 is allocated (use reallocate_file_processing_struct_m10() to change this)
	// if records are requested, enough memory for 1 record index is allocated (reallocate_file_processing_struct_m10() to change this)
	
	if (seg == NULL)
		seg = (SEGMENT_m10 *) calloc_m10((size_t)1, sizeof(SEGMENT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	numerical_fixed_width_string_m10(num_str, FILE_NUMBERING_DIGITS_m10, seg_num);
	snprintf_m10(seg->name, SEGMENT_BASE_FILE_NAME_BYTES_m10, "%s_s%s", chan_name, num_str);
	switch (type_code) {
		case TIME_SERIES_CHANNEL_TYPE_m10:
			// metadata: used as prototype
			seg->metadata_fps = allocate_file_processing_struct_m10(NULL, NULL, TIME_SERIES_METADATA_FILE_TYPE_CODE_m10, METADATA_BYTES_m10, proto_fps, METADATA_BYTES_m10);
			uh = seg->metadata_fps->universal_header;
			if (uh->segment_UID == UID_NO_ENTRY_m10)
				generate_UID_m10(&uh->segment_UID);
			uh->segment_number = seg_num;
			snprintf_m10(seg->path, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", enclosing_path, seg->name, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m10);
			snprintf_m10(seg->metadata_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
			// time series data
			seg->time_series_data_fps = allocate_file_processing_struct_m10(NULL, NULL, TIME_SERIES_DATA_FILE_TYPE_CODE_m10, 0, seg->metadata_fps, 0);
			snprintf_m10(seg->time_series_data_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, TIME_SERIES_DATA_FILE_TYPE_STRING_m10);
			// time series indices
			seg->time_series_indices_fps = allocate_file_processing_struct_m10(NULL, NULL, TIME_SERIES_INDICES_FILE_TYPE_CODE_m10, TIME_SERIES_INDEX_BYTES_m10, seg->metadata_fps, 0);
			snprintf_m10(seg->time_series_indices_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10);
			break;
		case VIDEO_CHANNEL_TYPE_m10:
			// metadata: used as prototype
			seg->metadata_fps = allocate_file_processing_struct_m10(NULL, NULL, VIDEO_METADATA_FILE_TYPE_CODE_m10, METADATA_BYTES_m10, proto_fps, METADATA_BYTES_m10);
			uh = seg->metadata_fps->universal_header;
			if (uh->segment_UID == UID_NO_ENTRY_m10)
				generate_UID_m10(&uh->segment_UID);
			uh->segment_number = seg_num;
			snprintf_m10(seg->path, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", enclosing_path, seg->name, VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m10);
			snprintf_m10(seg->metadata_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, VIDEO_METADATA_FILE_TYPE_STRING_m10);
			// video indices
			seg->video_indices_fps = allocate_file_processing_struct_m10(NULL, NULL, VIDEO_INDICES_FILE_TYPE_CODE_m10, VIDEO_INDEX_BYTES_m10, seg->metadata_fps, 0);
			snprintf_m10(seg->video_indices_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, VIDEO_INDICES_FILE_TYPE_STRING_m10);
			break;
		default:
			error_message_m10("Error: unrecognized type code \"0x%x\" [function \"%s\", line %d]\n", type_code, __FUNCTION__, __LINE__);
			return(NULL);
	}
	
	if (seg_recs == TRUE_m10) {
		seg->record_data_fps = allocate_file_processing_struct_m10(NULL, NULL, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, seg->metadata_fps, 0);
		snprintf_m10(seg->record_data_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, RECORD_DATA_FILE_TYPE_STRING_m10);
		seg->record_indices_fps = allocate_file_processing_struct_m10(NULL, NULL, RECORD_INDICES_FILE_TYPE_CODE_m10, RECORD_INDEX_BYTES_m10, seg->metadata_fps, 0);
		snprintf_m10(seg->record_indices_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
	}
	else {
		seg->record_data_fps = seg->record_indices_fps = NULL;
	}
	seg->segmented_session_record_data_fps = seg->segmented_session_record_indices_fps = NULL;  // these have to set in session after segment allocated
	
	return(seg);
}


SESSION_m10	*allocate_session_m10(FILE_PROCESSING_STRUCT_m10 *proto_fps, si1 *enclosing_path, si1 *sess_name, si4 n_ts_chans, si4 n_vid_chans, si4 n_segs, si1 **ts_chan_names, si1 **vid_chan_names, TERN_m10 sess_recs, TERN_m10 segmented_sess_recs, TERN_m10 chan_recs, TERN_m10 seg_recs)
{
	si1                             free_names, number_str[FILE_NUMBERING_DIGITS_m10 + 1];
	si8                             i, j;
	SESSION_m10			*sess;
	SEGMENT_m10			*seg;
	UNIVERSAL_HEADER_m10		*uh;
	FILE_PROCESSING_STRUCT_m10	*gen_fps;
	
	
	// enclosing_path is the path to the enclosing directory
	// sess_name is the base name, with no extension
	// if records are requested, enough memory for 1 record of data size LARGEST_RECORD_BYTES_m10 is allocated (reallocate_file_processing_struct_m10() to change this)
	// if records are requested, enough memory for 1 record index is allocated (reallocate_file_processing_struct_m10() to change this)
	
	sess = (SESSION_m10 *) calloc_m10((size_t)1, sizeof(SESSION_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	gen_fps = allocate_file_processing_struct_m10(NULL, NULL, FPS_PROTOTYPE_FILE_TYPE_CODE_m10, FPS_PROTOTYPE_BYTES_m10, proto_fps, FPS_PROTOTYPE_BYTES_m10);
	gen_fps->fd = FPS_FD_EPHEMERAL_m10;
	uh = gen_fps->universal_header;
	if (uh->session_UID == UID_NO_ENTRY_m10)
		generate_UID_m10(&uh->session_UID);
	uh->segment_number = UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m10;;
	strncpy_m10(uh->session_name, sess_name, BASE_FILE_NAME_BYTES_m10);
	strncpy_m10(sess->name, sess_name, BASE_FILE_NAME_BYTES_m10);
	snprintf_m10(sess->path, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", enclosing_path, sess->name, SESSION_DIRECTORY_TYPE_STRING_m10);
	
	sess->number_of_segments = n_segs;
	sess->number_of_time_series_channels = n_ts_chans;
	sess->number_of_video_channels = n_vid_chans;
	
	if (n_ts_chans) {
		sess->time_series_metadata_fps = gen_fps;
		uh->type_code = TIME_SERIES_CHANNEL_TYPE_m10;
		if (n_vid_chans) {
			sess->video_metadata_fps = allocate_file_processing_struct_m10(NULL, NULL, VIDEO_METADATA_FILE_TYPE_CODE_m10, METADATA_BYTES_m10, gen_fps, METADATA_BYTES_m10);
			sess->video_metadata_fps->fd = FPS_FD_EPHEMERAL_m10;
		}
	}
	else if (n_vid_chans) {
		sess->video_metadata_fps = gen_fps;
		uh->type_code = VIDEO_METADATA_FILE_TYPE_CODE_m10;
	}
	
	if (sess_recs == TRUE_m10) {
		sess->record_data_fps = allocate_file_processing_struct_m10(NULL, NULL, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, gen_fps, 0);
		snprintf_m10(sess->record_data_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", sess->path, sess->name, RECORD_DATA_FILE_TYPE_STRING_m10);
		sess->record_indices_fps = allocate_file_processing_struct_m10(NULL, NULL, RECORD_INDICES_FILE_TYPE_CODE_m10, RECORD_INDEX_BYTES_m10, gen_fps, 0);
		snprintf_m10(sess->record_indices_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", sess->path, sess->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
	}
	else {
		sess->record_data_fps = sess->record_indices_fps = NULL;
	}
	
	if (n_ts_chans) {
		free_names = FALSE_m10;
		if (ts_chan_names == NULL) {
			ts_chan_names = generate_numbered_names_m10(NULL, "tch_", n_ts_chans);
			free_names = TRUE_m10;
		}
		sess->time_series_channels = (CHANNEL_m10 **) calloc_2D_m10((size_t) n_ts_chans, 1, sizeof(CHANNEL_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		for (i = 0; i < n_ts_chans; ++i) {
			sess->time_series_metadata_fps->metadata->time_series_section_2.acquisition_channel_number = (si4) i + 1;
			allocate_channel_m10(sess->time_series_channels[i], sess->time_series_metadata_fps, sess->path, ts_chan_names[i], TIME_SERIES_CHANNEL_TYPE_m10, n_segs, chan_recs, seg_recs);
		}
		if (free_names == TRUE_m10)
			free((void *) ts_chan_names);
	}
	
	if (n_vid_chans) {
		free_names = FALSE_m10;
		if (vid_chan_names == NULL) {
			vid_chan_names = generate_numbered_names_m10(NULL, "vch_", n_vid_chans);
			free_names = TRUE_m10;
		}
		sess->video_channels = (CHANNEL_m10 **) calloc_2D_m10((size_t) n_vid_chans, 1, sizeof(CHANNEL_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		for (i = 0; i < n_vid_chans; ++i) {
			sess->video_metadata_fps->metadata->video_section_2.acquisition_channel_number = (si4) i + 1;
			allocate_channel_m10(sess->video_channels[i], sess->video_metadata_fps, sess->path, vid_chan_names[i], VIDEO_CHANNEL_TYPE_m10, n_segs, chan_recs, seg_recs);
		}
		if (free_names == TRUE_m10)
			free((void *) vid_chan_names);
		
		if (segmented_sess_recs == TRUE_m10) {
			sess->segmented_record_data_fps = (FILE_PROCESSING_STRUCT_m10 **) calloc_2D_m10((size_t) n_segs, 1, sizeof(FILE_PROCESSING_STRUCT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
			sess->segmented_record_indices_fps = (FILE_PROCESSING_STRUCT_m10 **) calloc_2D_m10((size_t) n_segs, 1, sizeof(FILE_PROCESSING_STRUCT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
			for (i = 0; i < n_segs; ++i) {
				if (n_ts_chans)
					gen_fps = sess->time_series_channels[0]->segments[i]->metadata_fps;
				else if (n_vid_chans)
					gen_fps = sess->video_channels[0]->segments[i]->metadata_fps;
				numerical_fixed_width_string_m10(number_str, FILE_NUMBERING_DIGITS_m10, (si4)i + 1); // segments numbered from 1
				allocate_file_processing_struct_m10(sess->segmented_record_data_fps[i], NULL, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, gen_fps, 0);
				snprintf_m10(sess->segmented_record_data_fps[i]->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s/%s_s%s.%s", sess->path, sess->name, RECORD_DIRECTORY_TYPE_STRING_m10, sess->name, number_str, RECORD_DATA_FILE_TYPE_STRING_m10);
				allocate_file_processing_struct_m10(sess->segmented_record_indices_fps[i], NULL, RECORD_INDICES_FILE_TYPE_CODE_m10, RECORD_INDEX_BYTES_m10, gen_fps, 0);
				snprintf_m10(sess->segmented_record_indices_fps[i]->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s/%s_s%s.%s", sess->path, sess->name, RECORD_DIRECTORY_TYPE_STRING_m10, sess->name, number_str, RECORD_INDICES_FILE_TYPE_STRING_m10);
				for (j = 0; j < n_ts_chans; ++j) {
					seg = sess->time_series_channels[j]->segments[i];
					seg->segmented_session_record_data_fps = sess->segmented_record_data_fps[i];
					seg->segmented_session_record_indices_fps = sess->segmented_record_indices_fps[i];
				}
				for (j = 0; j < n_vid_chans; ++j) {
					seg = sess->video_channels[j]->segments[i];
					seg->segmented_session_record_data_fps = sess->segmented_record_data_fps[i];
					seg->segmented_session_record_indices_fps = sess->segmented_record_indices_fps[i];
				}
			}
		}
		else {
			sess->segmented_record_data_fps = NULL;
			sess->segmented_record_indices_fps = NULL;
		}
	}
	
	return(sess);
}



inline void	apply_recording_time_offset_m10(si8 *time)
{
	if (*time != UUTC_NO_ENTRY_m10)
		*time -= globals_m10->recording_time_offset;
	
	return;
}


void	calculate_metadata_CRC_m10(FILE_PROCESSING_STRUCT_m10 *fps)
{
	// no real need for this one-liner function - just exists for symmetry with other library functions
	fps->universal_header->body_CRC = CRC_calculate_m10((ui1 *) fps->metadata, METADATA_BYTES_m10);
	
	return;
}


void    calculate_record_data_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_HEADER_m10 *record_header, si8 number_of_items)
{
	ui4     temp_CRC, full_record_CRC;
	si8     i;
	
	
	for (i = 0; i < number_of_items; ++i) {
		// record CRC
		record_header->record_CRC = CRC_calculate_m10((ui1 *)record_header + RECORD_HEADER_RECORD_CRC_START_OFFSET_m10, record_header->total_record_bytes - RECORD_HEADER_RECORD_CRC_START_OFFSET_m10);
		
		// update universal_header->body_CRC
		temp_CRC = CRC_calculate_m10((ui1 *)record_header, RECORD_HEADER_RECORD_CRC_START_OFFSET_m10);
		full_record_CRC = CRC_combine_m10(temp_CRC, record_header->record_CRC, record_header->total_record_bytes - RECORD_HEADER_RECORD_CRC_START_OFFSET_m10);
		fps->universal_header->body_CRC = CRC_combine_m10(fps->universal_header->body_CRC, full_record_CRC, record_header->total_record_bytes);
		
		record_header = (RECORD_HEADER_m10 *) ((ui1 *)record_header + record_header->total_record_bytes);
	}
	
	return;
}


void    calculate_record_indices_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_INDEX_m10 *record_index, si8 number_of_items)
{
	si8     i;
	
	
	for (i = 0; i < number_of_items; ++i, ++record_index)
		fps->universal_header->body_CRC = CRC_update_m10((ui1 *)record_index, RECORD_INDEX_BYTES_m10, fps->universal_header->body_CRC);
	
	return;
}


void    calculate_time_series_data_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, CMP_BLOCK_FIXED_HEADER_m10 *block_header, si8 number_of_items)
{
	ui4     temp_CRC, full_block_CRC;
	si8     i;
	
	
	for (i = 0; i < number_of_items; ++i) {
		// block CRC
		block_header->block_CRC = CRC_calculate_m10((ui1 *)block_header + CMP_BLOCK_CRC_START_OFFSET_m10, block_header->total_block_bytes - CMP_BLOCK_CRC_START_OFFSET_m10);
		
		// update universal_header->file_CRC
		temp_CRC = CRC_calculate_m10((ui1 *)block_header, CMP_BLOCK_CRC_START_OFFSET_m10);
		full_block_CRC = CRC_combine_m10(temp_CRC, block_header->block_CRC, block_header->total_block_bytes - CMP_BLOCK_CRC_START_OFFSET_m10);
		fps->universal_header->body_CRC = CRC_combine_m10(fps->universal_header->body_CRC, full_block_CRC, block_header->total_block_bytes);
		
		block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *)block_header + block_header->total_block_bytes);
	}
	
	return;
}


void    calculate_time_series_indices_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, TIME_SERIES_INDEX_m10 *time_series_index, si8 number_of_items)
{
	si8     i;
	
	
	for (i = 0; i < number_of_items; ++i, ++time_series_index)
		fps->universal_header->body_CRC = CRC_update_m10((ui1 *)time_series_index, TIME_SERIES_INDEX_BYTES_m10, fps->universal_header->body_CRC);
	
	return;
}


void	**calloc_2D_m10(size_t dim1, size_t dim2, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	si8     i;
	ui1	**ptr;
	size_t  dim1_bytes, dim2_bytes, content_bytes, total_bytes;
	
	
	// Returns pointer to 2 dimensional zeroed array of dim1 by dim2 elements of size el_size
	// ptr[0] points to a one dimensional array of size (dim1 * dim2)
	// The whole block can be freed with free(ptr)
	
	if (dim1 == 0 || dim2 == 0 || el_size == 0)
		return((void **) NULL);
	
	dim1_bytes = dim1 * sizeof(void *) ;
	dim2_bytes = dim2 * el_size;
	content_bytes = dim1 * dim2_bytes;
	total_bytes = dim1_bytes + content_bytes;
	ptr = (ui1 **) malloc_m10(total_bytes, function, line, behavior_on_fail);
	ptr[0] = (ui1 *) (ptr + dim1);
	memset((void *) ptr[0], 0, content_bytes);  // remove this line to make e_malloc_2D_m10()
	
	for (i = 1; i < dim1; ++i)
		ptr[i] = ptr[i - 1] + dim2_bytes;
	
	return((void **) ptr);
}


ui4	channel_type_from_path_m10(si1 *path)
{
	si1	*c, temp_path[FULL_FILE_NAME_BYTES_m10], name[SEGMENT_BASE_FILE_NAME_BYTES_m10], extension[TYPE_BYTES_m10];
	
	
	// move pointer to end of string
	c = path + strlen(path) - 1;
	
	// ignore terminal "/" if present
	if (*c == '/')
		c--;
	
	// copy extension
	c -= 4;
	if (*c != '.')
		return(UNKNOWN_CHANNEL_TYPE_m10);
	strncpy_m10(extension, ++c, TYPE_BYTES_m10);
	
	// compare extension: record types => get extension of next level up the hierarchy
	if (!(strcmp(extension, RECORD_DATA_FILE_TYPE_STRING_m10)) || !(strcmp(extension, RECORD_INDICES_FILE_TYPE_STRING_m10))) {
		extract_path_parts_m10(path, temp_path, NULL, NULL);
		extract_path_parts_m10(temp_path, temp_path, name, extension);
	}
	
	// compare extension: TIMES_SERIES_CHANNEL_TYPE
	if (!(strcmp(extension, TIME_SERIES_CHANNEL_DIRECTORY_TYPE_STRING_m10)))
		return(TIME_SERIES_CHANNEL_TYPE_m10);
	else if (!(strcmp(extension, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m10)))
		return(TIME_SERIES_CHANNEL_TYPE_m10);
	else if (!(strcmp(extension, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10)))
		return(TIME_SERIES_CHANNEL_TYPE_m10);
	else if (!(strcmp(extension, TIME_SERIES_DATA_FILE_TYPE_STRING_m10)))
		return(TIME_SERIES_CHANNEL_TYPE_m10);
	else if (!(strcmp(extension, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10)))
		return(TIME_SERIES_CHANNEL_TYPE_m10);
	
	// compare extension: VIDEO_CHANNEL_TYPE
	else if (!(strcmp(extension, VIDEO_CHANNEL_DIRECTORY_TYPE_STRING_m10)))
		return(VIDEO_CHANNEL_TYPE_m10);
	else if (!(strcmp(extension, VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m10)))
		return(VIDEO_CHANNEL_TYPE_m10);
	else if (!(strcmp(extension, VIDEO_METADATA_FILE_TYPE_STRING_m10)))
		return(VIDEO_CHANNEL_TYPE_m10);
	else if (!(strcmp(extension, VIDEO_INDICES_FILE_TYPE_STRING_m10)))
		return(VIDEO_CHANNEL_TYPE_m10);
	
	// unknown channel type
	return(UNKNOWN_CHANNEL_TYPE_m10);
}


TERN_m10        check_all_alignments_m10(const si1 *function, si4 line)
{
	TERN_m10        return_value;
	ui1		*bytes;
	
	
	// see if already checked
	if (globals_m10->all_structures_aligned != UNKNOWN_m10)
		return(globals_m10->all_structures_aligned);
	
	return_value = TRUE_m10;
	bytes = (ui1 *) malloc_m10(METADATA_FILE_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);  // METADATA is largest file structure
	
	// check all structures
	if ((check_universal_header_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_metadata_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_CMP_block_header_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_CMP_record_header_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_time_series_indices_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_video_indices_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_record_indices_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_record_header_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_record_structure_alignments_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;

	free((void *) bytes);
	
	if (return_value == TRUE_m10) {
		globals_m10->all_structures_aligned = TRUE_m10;
		if (globals_m10->verbose == TRUE_m10)
			message_m10("All MED Library structures are aligned\n");
	}
	else {
		error_message_m10("%s(): unaligned MED structures\n\tcalled from function %s(), line %d\n", __FUNCTION__, function, line);
	}
	
	return(return_value);
}


inline wchar_t	*char2wchar_m10(wchar_t *target, si1 *source)
{
	si1	*c, *c2, *tmp_source = NULL;
	si8	len, wsz;
	
	
	// if source == target, done in place
	// if not actually ascii, results may be weird
	// assumes target is big enough
	
	wsz = sizeof(wchar_t);  // 2 or 4 => varies by OS & compiler
	c = (si1 *) target - wsz;
	len = strlen(source);
	if ((void *)  source == (void *) target) {
		tmp_source = (si1 *) malloc_m10((size_t)len + 1, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		memcpy((void *) tmp_source, (void *)  source, (size_t)len + 1);
		c2 = tmp_source;
	}
	else {
		c2 = source;
	}
	memset((void *) target, 0, wsz * (len + 1));
	
	while (len--)
		*(c += wsz) = *c2++;  // little endian version
	
	if (tmp_source != NULL)
		free((void *) tmp_source);
	
	return(target);
}


TERN_m10	check_char_type_m10(void)
{
	char	c;

	
	// check size of "char"
	if (sizeof(char) != 1)
		return(FALSE_m10);

	// check signedness of "char"
	c = -1;
	if ((si4) c != (si4) -1)
		return(FALSE_m10);
	
	return(TRUE_m10);
}


TERN_m10        check_CMP_block_header_alignment_m10(ui1 *bytes)
{
	CMP_BLOCK_FIXED_HEADER_m10	*cbh;
	TERN_m10			free_flag = FALSE_m10;
	
	
	// see if already checked
	if (globals_m10->CMP_block_header_aligned == UNKNOWN_m10)
		globals_m10->CMP_block_header_aligned = FALSE_m10;
	else
		return(globals_m10->CMP_block_header_aligned);
	
	// check overall size
	if (sizeof(CMP_BLOCK_FIXED_HEADER_m10) != CMP_BLOCK_FIXED_HEADER_BYTES_m10)
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_m10(CMP_BLOCK_FIXED_HEADER_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	cbh = (CMP_BLOCK_FIXED_HEADER_m10 *)bytes;
	if (&cbh->block_start_UID != (ui8 *) (bytes + CMP_BLOCK_START_UID_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->block_CRC != (ui4 *) (bytes + CMP_BLOCK_CRC_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->block_flags != (ui4 *) (bytes + CMP_BLOCK_BLOCK_FLAGS_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->start_time != (si8 *) (bytes + CMP_BLOCK_START_TIME_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->acquisition_channel_number != (si4 *) (bytes + CMP_BLOCK_ACQUISITION_CHANNEL_NUMBER_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->total_block_bytes != (ui4 *) (bytes + CMP_BLOCK_TOTAL_BLOCK_BYTES_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->number_of_samples != (ui4 *) (bytes + CMP_BLOCK_NUMBER_OF_SAMPLES_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->number_of_records != (ui2 *) (bytes + CMP_BLOCK_NUMBER_OF_RECORDS_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->record_region_bytes != (ui2 *) (bytes + CMP_BLOCK_RECORD_REGION_BYTES_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->parameter_flags != (ui4 *) (bytes + CMP_BLOCK_PARAMETER_FLAGS_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->parameter_region_bytes != (ui2 *) (bytes + CMP_BLOCK_PARAMETER_REGION_BYTES_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->protected_region_bytes != (ui2 *) (bytes + CMP_BLOCK_PROTECTED_REGION_BYTES_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->discretionary_region_bytes != (ui2 *) (bytes + CMP_BLOCK_DISCRETIONARY_REGION_BYTES_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->model_region_bytes != (ui2 *) (bytes + CMP_BLOCK_MODEL_REGION_BYTES_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	if (&cbh->total_header_bytes != (ui4 *) (bytes + CMP_BLOCK_TOTAL_HEADER_BYTES_OFFSET_m10))
		goto CMP_BLOCK_HEADER_NOT_ALIGNED_m10;
	
	// aligned
	globals_m10->CMP_block_header_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("CMP_BLOCK_FIXED_HEADER_m10 structure is aligned\n", __FUNCTION__);
	
	return(TRUE_m10);
	
	// not aligned
CMP_BLOCK_HEADER_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): CMP_BLOCK_FIXED_HEADER_m10 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m10);
}


TERN_m10        check_CMP_record_header_alignment_m10(ui1 *bytes)
{
	CMP_RECORD_HEADER_m10	*crh;
	TERN_m10		free_flag = FALSE_m10;
	
	
	// see if already checked
	if (globals_m10->CMP_record_header_aligned == UNKNOWN_m10)
		globals_m10->CMP_record_header_aligned = FALSE_m10;
	else
		return(globals_m10->CMP_record_header_aligned);
	
	// check overall size
	if (sizeof(CMP_RECORD_HEADER_m10) != CMP_RECORD_HEADER_BYTES_m10)
		goto CMP_RECORD_HEADER_NOT_ALIGNED_m10;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_m10(CMP_RECORD_HEADER_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	crh = (CMP_RECORD_HEADER_m10 *)bytes;
	if (&crh->type_code != (ui4 *) (bytes + CMP_RECORD_HEADER_TYPE_CODE_OFFSET_m10))
		goto CMP_RECORD_HEADER_NOT_ALIGNED_m10;
	if (&crh->version_major != (ui1 *) (bytes + CMP_RECORD_HEADER_VERSION_MAJOR_OFFSET_m10))
		goto CMP_RECORD_HEADER_NOT_ALIGNED_m10;
	if (&crh->version_minor != (ui1 *) (bytes + CMP_RECORD_HEADER_VERSION_MINOR_OFFSET_m10))
		goto CMP_RECORD_HEADER_NOT_ALIGNED_m10;
	if (&crh->total_bytes != (ui2 *) (bytes + CMP_RECORD_HEADER_TOTAL_BYTES_OFFSET_m10))
		goto CMP_RECORD_HEADER_NOT_ALIGNED_m10;
	
	// aligned
	globals_m10->CMP_record_header_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("CMP_RECORD_HEADER_m10 structure is aligned\n");
	
	return(TRUE_m10);
	
	// not aligned
CMP_RECORD_HEADER_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): CMP_RECORD_HEADER_m10 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m10);
}


TERN_m10	check_file_list_m10(si1 **file_list, si4 n_files)
{
	si4	i;
	
	
	// generate_file_list_m10() does a lot of stuff, but often just need to ensure list contains full paths with no regex
	
	if (file_list == NULL)
		return(FALSE_m10);
	if (file_list[0] == NULL)
		return(FALSE_m10);
	
	for (i = 0; i < n_files; ++i) {
		if (str_contains_regex_m10(file_list[i]) == TRUE_m10)
			return(FALSE_m10);
		if (path_from_root_m10(file_list[i], NULL) == FALSE_m10)
			return(FALSE_m10);
	}

	return(TRUE_m10);
}


TERN_m10        check_metadata_alignment_m10(ui1 *bytes)
{
	TERN_m10	return_value, free_flag = FALSE_m10;
	METADATA_m10	*md;
	
	
	// see if already checked
	if (globals_m10->all_metadata_structures_aligned != UNKNOWN_m10)
		return(globals_m10->all_metadata_structures_aligned);
	
	return_value = TRUE_m10;
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_m10(METADATA_FILE_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	
	// check overall size
	if (sizeof(METADATA_m10) != METADATA_BYTES_m10)
		return_value = FALSE_m10;

	// check substructure offsets
	md = (METADATA_m10 *) bytes;
	if (&md->section_1 != (METADATA_SECTION_1_m10 *) bytes)
		return_value = FALSE_m10;
	if (&md->time_series_section_2 != (TIME_SERIES_METADATA_SECTION_2_m10 *) (bytes + METADATA_SECTION_1_BYTES_m10))
		return_value = FALSE_m10;
	if (&md->video_section_2 != (VIDEO_METADATA_SECTION_2_m10 *) (bytes + METADATA_SECTION_1_BYTES_m10))
		return_value = FALSE_m10;
	if (&md->section_3 != (METADATA_SECTION_3_m10 *) (bytes + METADATA_SECTION_1_BYTES_m10 + METADATA_SECTION_2_BYTES_m10))
		return_value = FALSE_m10;

	// check substructure contents
	if (check_metadata_section_1_alignment_m10(bytes) == FALSE_m10)
		return_value = FALSE_m10;
	if (check_time_series_metadata_section_2_alignment_m10(bytes) == FALSE_m10)
		return_value = FALSE_m10;
	if (check_video_metadata_section_2_alignment_m10(bytes) == FALSE_m10)
		return_value = FALSE_m10;
	if (check_metadata_section_3_alignment_m10(bytes) == FALSE_m10)
			return_value = FALSE_m10;
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (return_value == TRUE_m10)
		globals_m10->all_metadata_structures_aligned = TRUE_m10;

	return(return_value);
}


TERN_m10	check_metadata_section_1_alignment_m10(ui1 *bytes)
{
	METADATA_SECTION_1_m10	*md1;
	TERN_m10		free_flag = FALSE_m10;
	
	
	// see if already checked
	if (globals_m10->metadata_section_1_aligned == UNKNOWN_m10)
		globals_m10->metadata_section_1_aligned = FALSE_m10;
	else
		return(globals_m10->metadata_section_1_aligned);
	
	// check overall size
	if (sizeof(METADATA_SECTION_1_m10) != METADATA_SECTION_1_BYTES_m10)
		goto METADATA_SECTION_1_NOT_ALIGNED_m10;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_m10(METADATA_FILE_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	md1 = (METADATA_SECTION_1_m10 *) (bytes + UNIVERSAL_HEADER_BYTES_m10);
	if (md1->level_1_password_hint != (si1 *) (bytes + METADATA_LEVEL_1_PASSWORD_HINT_OFFSET_m10))
		goto METADATA_SECTION_1_NOT_ALIGNED_m10;
	if (md1->level_2_password_hint != (si1 *) (bytes + METADATA_LEVEL_2_PASSWORD_HINT_OFFSET_m10))
		goto METADATA_SECTION_1_NOT_ALIGNED_m10;
	if (&md1->section_2_encryption_level != (si1 *) (bytes + METADATA_SECTION_2_ENCRYPTION_LEVEL_OFFSET_m10))
		goto METADATA_SECTION_1_NOT_ALIGNED_m10;
	if (&md1->section_3_encryption_level != (si1 *) (bytes + METADATA_SECTION_3_ENCRYPTION_LEVEL_OFFSET_m10))
		goto METADATA_SECTION_1_NOT_ALIGNED_m10;
	if (md1->protected_region != (ui1 *) (bytes + METADATA_SECTION_1_PROTECTED_REGION_OFFSET_m10))
		goto METADATA_SECTION_1_NOT_ALIGNED_m10;
	if (md1->discretionary_region != (ui1 *) (bytes + METADATA_SECTION_1_DISCRETIONARY_REGION_OFFSET_m10))
		goto METADATA_SECTION_1_NOT_ALIGNED_m10;
	
	// aligned
	globals_m10->metadata_section_1_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("METADATA_SECTION_1_m10 structure is aligned\n");
	
	return(TRUE_m10);
	
	// not aligned
METADATA_SECTION_1_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): METADATA_SECTION_1_m10 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m10);
}


TERN_m10	check_metadata_section_3_alignment_m10(ui1 *bytes)
{
	METADATA_SECTION_3_m10	*md3;
	TERN_m10		free_flag = FALSE_m10;
	
	
	// see if already checked
	if (globals_m10->metadata_section_3_aligned == UNKNOWN_m10)
		globals_m10->metadata_section_3_aligned = FALSE_m10;
	else
		return(globals_m10->metadata_section_3_aligned);
	
	// check overall size
	if (sizeof(METADATA_SECTION_3_m10) != METADATA_SECTION_3_BYTES_m10)
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_m10(METADATA_FILE_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	md3 = (METADATA_SECTION_3_m10 *) (bytes + METADATA_SECTION_3_OFFSET_m10);
	if (&md3->recording_time_offset != (si8 *) (bytes + METADATA_RECORDING_TIME_OFFSET_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (&md3->daylight_time_start_code != (DAYLIGHT_TIME_CHANGE_CODE_m10 *) (bytes + METADATA_DAYLIGHT_TIME_START_CODE_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (&md3->daylight_time_end_code != (DAYLIGHT_TIME_CHANGE_CODE_m10 *) (bytes + METADATA_DAYLIGHT_TIME_END_CODE_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->standard_timezone_acronym != (si1 *) (bytes + METADATA_STANDARD_TIMEZONE_ACRONYM_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->standard_timezone_string != (si1 *) (bytes + METADATA_STANDARD_TIMEZONE_STRING_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->daylight_timezone_acronym != (si1 *) (bytes + METADATA_DAYLIGHT_TIMEZONE_ACRONYM_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->daylight_timezone_string != (si1 *) (bytes + METADATA_DAYLIGHT_TIMEZONE_STRING_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->subject_name_1 != (si1 *) (bytes + METADATA_SUBJECT_NAME_1_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->subject_name_2 != (si1 *) (bytes + METADATA_SUBJECT_NAME_2_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->subject_name_3 != (si1 *) (bytes + METADATA_SUBJECT_NAME_3_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->subject_ID != (si1 *) (bytes + METADATA_SUBJECT_ID_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->recording_country != (si1 *) (bytes + METADATA_RECORDING_COUNTRY_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->recording_territory != (si1 *) (bytes + METADATA_RECORDING_TERRITORY_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->recording_locality != (si1 *) (bytes + METADATA_RECORDING_LOCALITY_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->recording_institution != (si1 *) (bytes + METADATA_RECORDING_INSTITUTION_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->geotag_format != (si1 *) (bytes + METADATA_GEOTAG_FORMAT_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->geotag_data != (si1 *) (bytes + METADATA_GEOTAG_DATA_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (&md3->standard_UTC_offset != (si4 *) (bytes + METADATA_STANDARD_UTC_OFFSET_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->protected_region != (ui1 *) (bytes + METADATA_SECTION_3_PROTECTED_REGION_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	if (md3->discretionary_region != (ui1 *) (bytes + METADATA_SECTION_3_DISCRETIONARY_REGION_OFFSET_m10))
		goto METADATA_SECTION_3_NOT_ALIGNED_m10;
	
	// aligned
	globals_m10->metadata_section_3_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("METADATA_SECTION_3_m10 structure is aligned\n");
	
	return(TRUE_m10);
	
	// not aligned
METADATA_SECTION_3_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): METADATA_SECTION_3_m10 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m10);
}


TERN_m10	check_password_m10(si1 *password)
{
	si4	pw_len;
	
	
	// check pointer: return FALSE_m10 for NULL
	if (password == NULL) {
		warning_message_m10("%s(): password is NULL\n", __FUNCTION__);
		return(FALSE_m10);
	}
		
	// check password length:  return +1 for length error
	pw_len = UTF8_strlen_m10(password);
	if (pw_len == 0) {
		warning_message_m10("%s(): password has no characters\n", __FUNCTION__);
		return(FALSE_m10);
	}
	if (pw_len > MAX_PASSWORD_CHARACTERS_m10) {
		warning_message_m10("%s(): password too long (1 to  %d characters)\n", __FUNCTION__, MAX_PASSWORD_CHARACTERS_m10);
		return(FALSE_m10);
	}
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("Password is of valid form\n", __FUNCTION__);
	
	// return TRUE_m10 for valid password
	return(TRUE_m10);
}


TERN_m10	check_record_header_alignment_m10(ui1 *bytes)
{
	RECORD_HEADER_m10	*rh;
	TERN_m10                free_flag = FALSE_m10;
	
	
	// see if already checked
	if (globals_m10->record_header_aligned == UNKNOWN_m10)
		globals_m10->record_header_aligned = FALSE_m10;
	else
		return(globals_m10->record_header_aligned);
	
	// check overall size
	if (sizeof(RECORD_HEADER_m10) != RECORD_HEADER_BYTES_m10)
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_m10(RECORD_HEADER_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	rh = (RECORD_HEADER_m10 *) bytes;
	if (&rh->record_CRC != (ui4 *) (bytes + RECORD_HEADER_RECORD_CRC_OFFSET_m10))
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	if (&rh->total_record_bytes != (ui4 *) (bytes + RECORD_HEADER_TOTAL_RECORD_BYTES_OFFSET_m10))
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	if (&rh->start_time != (si8 *) (bytes + RECORD_HEADER_START_TIME_OFFSET_m10))
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	if (rh->type_string != (si1 *) (bytes + RECORD_HEADER_TYPE_STRING_OFFSET_m10))
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	if (&rh->type_code != (ui4 *) (bytes + RECORD_HEADER_TYPE_CODE_OFFSET_m10))
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	if (&rh->type_string_terminal_zero != (si1 *) (bytes + RECORD_HEADER_TYPE_STRING_TERMINAL_ZERO_OFFSET_m10))
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	if (&rh->version_major != (ui1 *) (bytes + RECORD_HEADER_VERSION_MAJOR_OFFSET_m10))
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	if (&rh->version_minor != (ui1 *) (bytes + RECORD_HEADER_VERSION_MINOR_OFFSET_m10))
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	if (&rh->encryption_level != (si1 *) (bytes + RECORD_HEADER_ENCRYPTION_LEVEL_OFFSET_m10))
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	
	// aligned
	globals_m10->record_header_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("RECORD_HEADER_m10 structure is aligned\n");
	
	return(TRUE_m10);
	
	// not aligned
RECORD_HEADER_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): RECORD_HEADER_m10 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m10);
}


TERN_m10	check_record_indices_alignment_m10(ui1 *bytes)
{
	RECORD_INDEX_m10	*ri;
	TERN_m10		free_flag = FALSE_m10;
	
	
	// see if already checked
	if (globals_m10->record_indices_aligned == UNKNOWN_m10)
		globals_m10->record_indices_aligned = FALSE_m10;
	else
		return(globals_m10->record_indices_aligned);
	
	// check overall size
	if (sizeof(RECORD_INDEX_m10) != RECORD_INDEX_BYTES_m10)
		goto RECORD_INDICES_NOT_ALIGNED_m10;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_m10(RECORD_INDEX_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	ri = (RECORD_INDEX_m10 *) bytes;
	if (&ri->file_offset != (si8 *) (bytes + RECORD_INDEX_FILE_OFFSET_OFFSET_m10))
		goto RECORD_INDICES_NOT_ALIGNED_m10;
	if (&ri->start_time != (si8 *) (bytes + RECORD_INDEX_START_TIME_OFFSET_m10))
		goto RECORD_INDICES_NOT_ALIGNED_m10;
	if (ri->type_string != (si1 *) (bytes + RECORD_INDEX_TYPE_STRING_OFFSET_m10))
		goto RECORD_INDICES_NOT_ALIGNED_m10;
	if (&ri->type_code != (ui4 *) (bytes + RECORD_INDEX_TYPE_CODE_OFFSET_m10))
		goto RECORD_INDICES_NOT_ALIGNED_m10;
	if (&ri->type_string_terminal_zero != (si1 *) (bytes + RECORD_INDEX_TYPE_STRING_TERMINAL_ZERO_OFFSET_m10))
		goto RECORD_INDICES_NOT_ALIGNED_m10;
	if (&ri->version_major != (ui1 *) (bytes + RECORD_INDEX_VERSION_MAJOR_OFFSET_m10))
		goto RECORD_INDICES_NOT_ALIGNED_m10;
	if (&ri->version_minor != (ui1 *) (bytes + RECORD_INDEX_VERSION_MINOR_OFFSET_m10))
		goto RECORD_INDICES_NOT_ALIGNED_m10;
	if (&ri->encryption_level != (si1 *) (bytes + RECORD_INDEX_ENCRYPTION_LEVEL_OFFSET_m10))
		goto RECORD_INDICES_NOT_ALIGNED_m10;
	
	// aligned
	globals_m10->record_indices_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		printf_m10("RECORD_INDEX_m10 structure is aligned\n");
	
	return(TRUE_m10);
	
	// not aligned
RECORD_INDICES_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): RECORD_INDEX_m10 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m10);
}


TERN_m10	check_time_series_indices_alignment_m10(ui1 *bytes)
{
	TIME_SERIES_INDEX_m10	*tsi;
	TERN_m10		free_flag = FALSE_m10;
	
	
	// see if already checked
	if (globals_m10->time_series_indices_aligned == UNKNOWN_m10)
		globals_m10->time_series_indices_aligned = FALSE_m10;
	else
		return(globals_m10->time_series_indices_aligned);
	
	// check overall size
	if (sizeof(TIME_SERIES_INDEX_m10) != TIME_SERIES_INDEX_BYTES_m10)
		goto TIME_SERIES_INDICES_NOT_ALIGNED_m10;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_m10(TIME_SERIES_INDEX_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	tsi = (TIME_SERIES_INDEX_m10 *) bytes;
	if (&tsi->file_offset != (si8 *) (bytes + TIME_SERIES_INDEX_FILE_OFFSET_OFFSET_m10))
		goto TIME_SERIES_INDICES_NOT_ALIGNED_m10;
	if (&tsi->start_time != (si8 *) (bytes + TIME_SERIES_INDEX_START_TIME_OFFSET_m10))
		goto TIME_SERIES_INDICES_NOT_ALIGNED_m10;
	if (&tsi->start_sample_number != (si8 *) (bytes + TIME_SERIES_INDEX_START_SAMPLE_NUMBER_OFFSET_m10))
		goto TIME_SERIES_INDICES_NOT_ALIGNED_m10;
	
	// aligned
	globals_m10->time_series_indices_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("TIME_SERIES_INDEX_m10 structure is aligned\n");
	
	return(TRUE_m10);
	
	// not aligned
TIME_SERIES_INDICES_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): TIME_SERIES_INDEX_m10 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m10);
}


TERN_m10	check_time_series_metadata_section_2_alignment_m10(ui1 *bytes)
{
	TIME_SERIES_METADATA_SECTION_2_m10	*md2;
	TERN_m10				free_flag = FALSE_m10;
	
	
	// see if already checked
	if (globals_m10->time_series_metadata_section_2_aligned == UNKNOWN_m10)
		globals_m10->time_series_metadata_section_2_aligned = FALSE_m10;
	else
		return(globals_m10->time_series_metadata_section_2_aligned);
	
	// check overall size
	if (sizeof(TIME_SERIES_METADATA_SECTION_2_m10) != METADATA_SECTION_2_BYTES_m10)
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_m10(METADATA_FILE_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	md2 = (TIME_SERIES_METADATA_SECTION_2_m10 *) (bytes + METADATA_SECTION_2_OFFSET_m10);
	// channel type independent fields
	if (md2->session_description != (si1 *) (bytes + METADATA_SESSION_DESCRIPTION_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (md2->channel_description != (si1 *) (bytes + METADATA_CHANNEL_DESCRIPTION_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (md2->segment_description != (si1 *) (bytes + METADATA_SEGMENT_DESCRIPTION_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (md2->equipment_description != (si1 *) (bytes + METADATA_EQUIPMENT_DESCRIPTION_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->acquisition_channel_number != (si4 *) (bytes + METADATA_ACQUISITION_CHANNEL_NUMBER_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	// channel type specific fields
	if (md2->reference_description != (si1 *) (bytes + TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->sampling_frequency != (sf8 *) (bytes + TIME_SERIES_METADATA_SAMPLING_FREQUENCY_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->low_frequency_filter_setting != (sf8 *) (bytes + TIME_SERIES_METADATA_LOW_FREQUENCY_FILTER_SETTING_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->high_frequency_filter_setting != (sf8 *) (bytes + TIME_SERIES_METADATA_HIGH_FREQUENCY_FILTER_SETTING_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->notch_filter_frequency_setting != (sf8 *) (bytes + TIME_SERIES_METADATA_NOTCH_FILTER_FREQUENCY_SETTING_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->AC_line_frequency != (sf8 *) (bytes + TIME_SERIES_METADATA_AC_LINE_FREQUENCY_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->amplitude_units_conversion_factor != (sf8 *) (bytes + TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (md2->amplitude_units_description != (si1 *) (bytes + TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->time_base_units_conversion_factor != (sf8 *) (bytes + TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (md2->time_base_units_description != (si1 *) (bytes + TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->absolute_start_sample_number != (si8 *) (bytes + TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->number_of_samples != (si8 *) (bytes + TIME_SERIES_METADATA_NUMBER_OF_SAMPLES_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->number_of_blocks != (si8 *) (bytes + TIME_SERIES_METADATA_NUMBER_OF_BLOCKS_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->maximum_block_bytes != (si8 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_BLOCK_BYTES_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->maximum_block_samples != (ui4 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_BLOCK_SAMPLES_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->maximum_block_difference_bytes != (ui4 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_BLOCK_DIFFERENCE_BYTES_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->maximum_block_duration != (sf8 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_BLOCK_DURATION_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->number_of_discontinuities != (si8 *) (bytes + TIME_SERIES_METADATA_NUMBER_OF_DISCONTINUITIES_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->maximum_contiguous_blocks != (si8 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->maximum_contiguous_block_bytes != (si8 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&md2->maximum_contiguous_samples != (si8 *) (bytes + TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (md2->protected_region != (ui1 *) (bytes + TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (md2->discretionary_region != (ui1 *) (bytes + TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m10))
		goto TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10;
	
	// aligned
	globals_m10->time_series_metadata_section_2_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("TIME_SERIES_METADATA_SECTION_2_m10 structure is aligned\n");
	
	return(TRUE_m10);
	
	// not aligned
TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): TIME_SERIES_METADATA_SECTION_2_m10 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m10);
}


TERN_m10	check_universal_header_alignment_m10(ui1 *bytes)
{
	UNIVERSAL_HEADER_m10	*uh;
	TERN_m10		free_flag = FALSE_m10;
	
	
	// see if already checked
	if (globals_m10->universal_header_aligned == UNKNOWN_m10)
		globals_m10->universal_header_aligned = FALSE_m10;
	else
		return(globals_m10->universal_header_aligned);
	
	// check overall size
	if (sizeof(UNIVERSAL_HEADER_m10) != UNIVERSAL_HEADER_BYTES_m10)
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_m10(UNIVERSAL_HEADER_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	uh = (UNIVERSAL_HEADER_m10 *) bytes;
	if (&uh->header_CRC != (ui4 *) (bytes + UNIVERSAL_HEADER_HEADER_CRC_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->body_CRC != (ui4 *) (bytes + UNIVERSAL_HEADER_BODY_CRC_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->file_end_time != (si8 *) (bytes + UNIVERSAL_HEADER_FILE_END_TIME_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->number_of_entries != (si8 *) (bytes + UNIVERSAL_HEADER_NUMBER_OF_ENTRIES_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->maximum_entry_size != (ui4 *) (bytes + UNIVERSAL_HEADER_MAXIMUM_ENTRY_SIZE_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->segment_number != (si4 *) (bytes + UNIVERSAL_HEADER_SEGMENT_NUMBER_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (uh->type_string != (si1 *) (bytes + UNIVERSAL_HEADER_TYPE_STRING_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->type_code != (ui4 *) (bytes + UNIVERSAL_HEADER_TYPE_CODE_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->type_string_terminal_zero != (si1 *) (bytes + UNIVERSAL_HEADER_TYPE_STRING_TERMINAL_ZERO_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->MED_version_major != (ui1 *) (bytes + UNIVERSAL_HEADER_MED_VERSION_MAJOR_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->MED_version_minor != (ui1 *) (bytes + UNIVERSAL_HEADER_MED_VERSION_MINOR_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->byte_order_code != (ui1 *) (bytes + UNIVERSAL_HEADER_BYTE_ORDER_CODE_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->session_start_time != (si8 *) (bytes + UNIVERSAL_HEADER_SESSION_START_TIME_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->file_start_time != (si8 *) (bytes + UNIVERSAL_HEADER_FILE_START_TIME_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (uh->session_name != (si1 *) (bytes + UNIVERSAL_HEADER_SESSION_NAME_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (uh->channel_name != (si1 *)  (bytes + UNIVERSAL_HEADER_CHANNEL_NAME_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (uh->anonymized_subject_ID != (si1 *) (bytes + UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->session_UID != (ui8 *) (bytes + UNIVERSAL_HEADER_SESSION_UID_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->channel_UID != (ui8 *) (bytes + UNIVERSAL_HEADER_CHANNEL_UID_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->segment_UID != (ui8 *) (bytes + UNIVERSAL_HEADER_SEGMENT_UID_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->file_UID != (ui8 *) (bytes + UNIVERSAL_HEADER_FILE_UID_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (&uh->provenance_UID != (ui8 *) (bytes + UNIVERSAL_HEADER_PROVENANCE_UID_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (uh->level_1_password_validation_field != (ui1 *) (bytes + UNIVERSAL_HEADER_LEVEL_1_PASSWORD_VALIDATION_FIELD_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (uh->level_2_password_validation_field != (ui1 *) (bytes + UNIVERSAL_HEADER_LEVEL_2_PASSWORD_VALIDATION_FIELD_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (uh->level_3_password_validation_field != (ui1 *) (bytes + UNIVERSAL_HEADER_LEVEL_3_PASSWORD_VALIDATION_FIELD_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (uh->protected_region != (ui1 *) (bytes + UNIVERSAL_HEADER_PROTECTED_REGION_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	if (uh->discretionary_region != (ui1 *) (bytes + UNIVERSAL_HEADER_DISCRETIONARY_REGION_OFFSET_m10))
		goto UNIVERSAL_HEADER_NOT_ALIGNED_m10;
	
	// aligned
	globals_m10->universal_header_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("UNIVERSAL_HEADER_m10 structure is aligned\n");
	
	return(TRUE_m10);
	
	// not aligned
UNIVERSAL_HEADER_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): UNIVERSAL_HEADER_m10 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m10);
}


TERN_m10	check_video_indices_alignment_m10(ui1 *bytes)
{
	VIDEO_INDEX_m10		*vi;
	TERN_m10		free_flag = FALSE_m10;
	
	
	// see if already checked
	if (globals_m10->video_indices_aligned == UNKNOWN_m10)
		globals_m10->video_indices_aligned = FALSE_m10;
	else
		return(globals_m10->video_indices_aligned);
	
	// check overall size
	if (sizeof(VIDEO_INDEX_m10) != VIDEO_INDEX_BYTES_m10)
		goto VIDEO_INDICES_NOT_ALIGNED_m10;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_m10(VIDEO_INDEX_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	vi = (VIDEO_INDEX_m10 *) bytes;
	if (&vi->file_offset != (si8 *) (bytes + VIDEO_INDEX_FILE_OFFSET_OFFSET_m10))
		goto VIDEO_INDICES_NOT_ALIGNED_m10;
	if (&vi->start_time != (si8 *) (bytes + VIDEO_INDEX_START_TIME_OFFSET_m10))
		goto VIDEO_INDICES_NOT_ALIGNED_m10;
	if (&vi->start_frame_number != (si4 *) (bytes + VIDEO_INDEX_START_FRAME_OFFSET_m10))
		goto VIDEO_INDICES_NOT_ALIGNED_m10;
	if (&vi->video_file_number != (si4 *) (bytes + VIDEO_INDEX_VIDEO_FILE_NUMBER_OFFSET_m10))
		goto VIDEO_INDICES_NOT_ALIGNED_m10;
	
	// aligned
	globals_m10->video_indices_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("VIDEO_INDEX_m10 structure is aligned\n");
	
	return(TRUE_m10);
	
	// not aligned
VIDEO_INDICES_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): VIDEO_INDEX_m10 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m10);
}


TERN_m10	check_video_metadata_section_2_alignment_m10(ui1 *bytes)
{
	VIDEO_METADATA_SECTION_2_m10	*vmd2;
	TERN_m10			free_flag = FALSE_m10;
	
	
	// see if already checked
	if (globals_m10->video_metadata_section_2_aligned == UNKNOWN_m10)
		globals_m10->video_metadata_section_2_aligned = FALSE_m10;
	else
		return(globals_m10->video_metadata_section_2_aligned);
	
	// check overall size
	if (sizeof(VIDEO_METADATA_SECTION_2_m10) != METADATA_SECTION_2_BYTES_m10)
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_m10(METADATA_FILE_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	vmd2 = (VIDEO_METADATA_SECTION_2_m10 *) (bytes + METADATA_SECTION_2_OFFSET_m10);
	// channel type independent fields
	if (vmd2->session_description != (si1 *) (bytes + METADATA_SESSION_DESCRIPTION_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (vmd2->channel_description != (si1 *) (bytes + METADATA_CHANNEL_DESCRIPTION_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (vmd2->equipment_description != (si1 *) (bytes + METADATA_EQUIPMENT_DESCRIPTION_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&vmd2->acquisition_channel_number != (si4 *) (bytes + METADATA_ACQUISITION_CHANNEL_NUMBER_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	// channel type specific fields
	if (&vmd2->horizontal_resolution != (si8 *) (bytes + VIDEO_METADATA_HORIZONTAL_RESOLUTION_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&vmd2->vertical_resolution != (si8 *) (bytes + VIDEO_METADATA_VERTICAL_RESOLUTION_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&vmd2->frame_rate != (sf8 *) (bytes + VIDEO_METADATA_FRAME_RATE_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&vmd2->number_of_clips != (si8 *) (bytes + VIDEO_METADATA_NUMBER_OF_CLIPS_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&vmd2->maximum_clip_bytes != (si8 *) (bytes + VIDEO_METADATA_MAXIMUM_CLIP_BYTES_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (vmd2->video_format != (si1 *) (bytes + VIDEO_METADATA_VIDEO_FORMAT_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (&vmd2->number_of_video_files != (si4 *) (bytes + VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (vmd2->protected_region != (ui1 *) (bytes + VIDEO_METADATA_SECTION_2_PROTECTED_REGION_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	if (vmd2->discretionary_region != (ui1 *) (bytes + VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_OFFSET_m10))
		goto VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10;
	
	// aligned
	globals_m10->video_metadata_section_2_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("VIDEO_METADATA_SECTION_2_m10 structure is aligned\n");
	
	return(TRUE_m10);
	
	// not aligned
VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		free((void *) bytes);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): VIDEO_METADATA_SECTION_2_m10 structure is NOT aligned\n", __FUNCTION__);
	
	return(FALSE_m10);
}


si4	compare_fps_start_times_m10(const void *a, const void *b)
{
	si8				a_start_time, b_start_time;
	FILE_PROCESSING_STRUCT_m10	*fps;
	
	
	fps = (FILE_PROCESSING_STRUCT_m10 *) *((FILE_PROCESSING_STRUCT_m10 **) a);
	a_start_time = fps->universal_header->file_start_time;

	fps = (FILE_PROCESSING_STRUCT_m10 *)  *((FILE_PROCESSING_STRUCT_m10 **) a);
	b_start_time = fps->universal_header->file_start_time;

	// qsort() requires an si4 return value, so can't just subtract
	if (a_start_time > b_start_time)
		return((si4) 1);
	if (a_start_time < b_start_time)
		return((si4) -1);
	return((si4) 0);
}


void	condition_timezone_info_m10(TIMEZONE_INFO_m10 *tz_info)
{
	si4			i;
	si8			len;
	TIMEZONE_ALIAS_m10	*tz_aliases_table;
	

	if (globals_m10->timezone_table == NULL)
		initialize_timezone_tables_m10();

	// Country: at this time there are no 2 or 3 letter country names => user probably entered acronym
	if (*tz_info->country) {
		len = strlen(tz_info->country);
		if (len == 2) {
			strcpy(tz_info->country_acronym_2_letter, tz_info->country);
			*tz_info->country = 0;
		}
		else if (len == 3) {
			strcpy(tz_info->country_acronym_3_letter, tz_info->country);
			*tz_info->country = 0;
		}
	}
	
	// Territory: at this time there are no 2 letter territory names => user probably entered acronym
	if (*tz_info->territory) {
		len = strlen(tz_info->territory);
		if (len == 2) {
			strcpy(tz_info->territory_acronym, tz_info->territory);
			*tz_info->territory = 0;
		}
	}
	
	// change potential matching strings to caps
	strtoupper_m10(tz_info->country);
	strtoupper_m10(tz_info->country_acronym_2_letter);
	strtoupper_m10(tz_info->country_acronym_3_letter);
	strtoupper_m10(tz_info->territory);
	strtoupper_m10(tz_info->territory_acronym);
	strtoupper_m10(tz_info->standard_timezone);
	strtoupper_m10(tz_info->standard_timezone_acronym);
	strtoupper_m10(tz_info->daylight_timezone);
	strtoupper_m10(tz_info->daylight_timezone_acronym);
	
	// check country aliases
	tz_aliases_table = globals_m10->country_aliases_table;
	
	if (*tz_info->country) {
		for (i = 0; i < TZ_COUNTRY_ALIASES_ENTRIES_m10; ++i) {
			if ((strcmp(tz_info->country, tz_aliases_table[i].alias)) == 0) {
				strcpy(tz_info->country, tz_aliases_table[i].table_name);
				break;
			}
		}
	}
	
	// check country acronyms
	tz_aliases_table = globals_m10->country_acronym_aliases_table;
	
	if (*tz_info->country_acronym_2_letter) {
		for (i = 0; i < TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m10; ++i) {
			if ((strcmp(tz_info->country_acronym_2_letter, tz_aliases_table[i].alias)) == 0) {
				strcpy(tz_info->country_acronym_2_letter, tz_aliases_table[i].table_name);
				break;
			}
		}
	}
	
	if (*tz_info->country_acronym_3_letter) {
		for (i = 0; i < TZ_COUNTRY_ALIASES_ENTRIES_m10; ++i) {
			if ((strcmp(tz_info->country_acronym_3_letter, tz_aliases_table[i].alias)) == 0) {
				strcpy(tz_info->country_acronym_3_letter, tz_aliases_table[i].table_name);
				break;
			}
		}
	}

	return;
}


void	condition_time_slice_m10(TIME_SLICE_m10* slice)
{
	si8	test_time;
	
	
	if (slice == NULL) {
		warning_message_m10("%s(): passed time slice is NULL\n");
		return;
	}
	
	if (globals_m10->RTO_known == FALSE_m10) {
		if (globals_m10->verbose == TRUE_m10)
			warning_message_m10("%s(): recording time offset is not known => assuming no offset\n", __FUNCTION__);
		globals_m10->recording_time_offset = 0;  // this is the default value
	}
	
	if (slice->start_time <= 0) {
		if (slice->start_time == UUTC_NO_ENTRY_m10) {
			if (slice->start_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10)
				slice->start_time = BEGINNING_OF_TIME_m10;
		}
		else {  // relative time
			slice->start_time = globals_m10->session_start_time - slice->start_time;
		}
	}
	else {  // ? unoffset time
		test_time = slice->start_time - globals_m10->recording_time_offset;
		if (test_time > 0)  // start time is not offset
			slice->start_time = test_time;
	}
	
	if (slice->end_time <= 0) {
		if (slice->end_time == UUTC_NO_ENTRY_m10) {
			if (slice->end_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10)
				slice->end_time = END_OF_TIME_m10;
		}
		else {  // relative time
			slice->end_time = globals_m10->session_start_time - slice->end_time;
		}
	}
	else {  // ? unoffset time
		test_time = slice->end_time - globals_m10->recording_time_offset;
		if (test_time > 0 && slice->end_time != END_OF_TIME_m10)  // end time is not offset
			slice->end_time = test_time;
	}
	
	slice->session_start_time = globals_m10->session_start_time;
	slice->conditioned = TRUE_m10;
	
	return;
}


#if defined MACOS_m10 || defined LINUX_m10
inline si8      current_uutc_m10(void)
{
	struct timeval  tv;
	si8             uutc;
	
	
	gettimeofday(&tv, NULL);
	uutc = (si8)tv.tv_sec * (si8)1000000 + (si8)tv.tv_usec;
	
	return(uutc);
}
#endif


#ifdef WINDOWS_m10
inline si8      current_uutc_m10(void)
{
	static const ui8    EPOCH = (ui8) 116444736000000000;
	struct timeval      tv;
	si8                 uutc;
	SYSTEMTIME          system_time;
	FILETIME            file_time;
	ui8                 time;
	
	
	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((ui8)file_time.dwLowDateTime);
	time += ((ui8)file_time.dwHighDateTime) << 32;
	
	tv.tv_sec = (si4)((time - EPOCH) / (ui8) 10000000);
	tv.tv_usec = (si4)(system_time.wMilliseconds * 1000);
	uutc = (si8)tv.tv_sec * (si8) 1000000 + (si8) tv.tv_usec;
	
	return(uutc);
}
#endif


inline si4      days_in_month_m10(si4 month, si4 year)
// Note month is [0 - 11], January == 0, as in struct tm.tm_mon
// Note struct tm.tm_year is (year - 1900), this function expects the full value
{
	static const si4        standard_days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	si4                     days;
	
	days = standard_days[month];
	
	// leap years
	if (month == 1) {  // February
		if ((year % 4) == 0) {  // leap year
			++days;
			if ((year % 100) == 0) {  // centurial exception
				--days;
				if ((year % 400) == 0) {  // centurial exception exception
					++days;
				}
			}
		}
	}
	
	return(days);
}


TERN_m10	decrypt_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps)
{
	ui1			*ui1_p, *decryption_key;
	si4		        i, decryption_blocks;
	PASSWORD_DATA_m10	*pwd;
	METADATA_SECTION_3_m10	*section_3;
	
	
	if (fps == NULL) {
		error_message_m10("%s(): FILE_PROCESSING_STRUCT is NULL\n", __FUNCTION__);
		return(FALSE_m10);
	}
	
	pwd = fps->password_data;
	// section 2 decryption
	if (fps->metadata->section_1.section_2_encryption_level > NO_ENCRYPTION_m10) {  // natively encrypted and currently encrypted
		if (pwd->access_level >= fps->metadata->section_1.section_2_encryption_level) {
			if (fps->metadata->section_1.section_2_encryption_level == LEVEL_1_ENCRYPTION_m10)
				decryption_key = pwd->level_1_encryption_key;
			else
				decryption_key = pwd->level_2_encryption_key;
			decryption_blocks = METADATA_SECTION_2_BYTES_m10 / ENCRYPTION_BLOCK_BYTES_m10;
			ui1_p = fps->raw_data + METADATA_SECTION_2_OFFSET_m10;
			for (i = 0; i < decryption_blocks; ++i) {
				AES_decrypt_m10(ui1_p, ui1_p, NULL, decryption_key);
				ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
			}
			fps->metadata->section_1.section_2_encryption_level = -fps->metadata->section_1.section_2_encryption_level;  // mark as currently decrypted
		}
		else {
			error_message_m10("%s(): Section 2 of the Metadata is encrypted at level %hhd => cannot decrypt\n", __FUNCTION__, fps->metadata->section_1.section_2_encryption_level);
			show_password_data_m10(pwd);
			return(FALSE_m10);  // can't do anything without section 2, so fail
		}
	}
	
	// section 3 decryption
	if (fps->metadata->section_1.section_3_encryption_level > NO_ENCRYPTION_m10) {  // natively encrypted and currently encrypted
		if (pwd->access_level >= fps->metadata->section_1.section_3_encryption_level) {
			if (fps->metadata->section_1.section_3_encryption_level == LEVEL_1_ENCRYPTION_m10)
				decryption_key = pwd->level_1_encryption_key;
			else
				decryption_key = pwd->level_2_encryption_key;
			decryption_blocks = METADATA_SECTION_3_BYTES_m10 / ENCRYPTION_BLOCK_BYTES_m10;
			ui1_p = fps->raw_data + METADATA_SECTION_3_OFFSET_m10;
			for (i = 0; i < decryption_blocks; ++i) {
				AES_decrypt_m10(ui1_p, ui1_p, NULL, decryption_key);
				ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
			}
			fps->metadata->section_1.section_3_encryption_level = -fps->metadata->section_1.section_3_encryption_level;  // mark as currently decrypted
		}
		else {
			if (globals_m10->verbose == TRUE_m10) {
				warning_message_m10("%s(): Metadata section 3 encrypted at level %hhd => cannot decrypt\n", __FUNCTION__, fps->metadata->section_1.section_3_encryption_level);
				show_password_data_m10(pwd);
			}
			globals_m10->RTO_known = FALSE_m10;
			globals_m10->time_constants_set = TRUE_m10;  // set to defaults
			return(TRUE_m10);  // can function without section 3, so return TRUE_m10
		}
	}
	
	// set global time data
	if (globals_m10->RTO_known == FALSE_m10) {
		section_3 = &fps->metadata->section_3;
		globals_m10->recording_time_offset = section_3->recording_time_offset;
		globals_m10->standard_UTC_offset = section_3->standard_UTC_offset;
		strncpy_m10(globals_m10->standard_timezone_acronym, section_3->standard_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m10);
		strncpy_m10(globals_m10->standard_timezone_string, section_3->standard_timezone_string, TIMEZONE_STRING_BYTES_m10);
		strncpy_m10(globals_m10->daylight_timezone_acronym, section_3->daylight_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m10);
		strncpy_m10(globals_m10->daylight_timezone_string, section_3->daylight_timezone_string, TIMEZONE_STRING_BYTES_m10);
		if ((globals_m10->daylight_time_start_code.value = section_3->daylight_time_start_code.value) == DTCC_VALUE_NOT_OBSERVED_m10)
			globals_m10->observe_DST = FALSE_m10;
		else
			globals_m10->observe_DST = TRUE_m10;
		globals_m10->RTO_known = TRUE_m10;
		globals_m10->daylight_time_end_code.value = section_3->daylight_time_end_code.value;
		globals_m10->time_constants_set = TRUE_m10;
	}
	
	return(TRUE_m10);
}


TERN_m10     decrypt_records_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_HEADER_m10 *record_header, si8 number_of_items)
{
	ui1			*ui1_p, *encryption_key;
	ui4			j, encryption_blocks;
	si8			i, failed_decryption_count;
	PASSWORD_DATA_m10	*pwd;
	
	
	pwd = fps->password_data;
	for (i = failed_decryption_count = 0; i < number_of_items; ++i) {
		ui1_p = (ui1 *)record_header;
		if (record_header->encryption_level > NO_ENCRYPTION_m10) {
			if (pwd->access_level >= record_header->encryption_level) {
				if (record_header->encryption_level == LEVEL_1_ENCRYPTION_m10)
					encryption_key = pwd->level_1_encryption_key;
				else
					encryption_key = pwd->level_2_encryption_key;
				encryption_blocks = (record_header->total_record_bytes - RECORD_HEADER_BYTES_m10) / ENCRYPTION_BLOCK_BYTES_m10;
				ui1_p += RECORD_HEADER_BYTES_m10;
				for (j = 0; j < encryption_blocks; ++j) {
					AES_decrypt_m10(ui1_p, ui1_p, NULL, encryption_key);
					ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
				}
				record_header->encryption_level = -record_header->encryption_level;  // mark as currently decrypted
			}
			else {
				++failed_decryption_count;
				if (globals_m10->verbose == TRUE_m10)
					warning_message_m10("%s(): Cannot decrypt record => skipping\n", __FUNCTION__);
			}
		}
		record_header = (RECORD_HEADER_m10 *) ((ui1 *)record_header + record_header->total_record_bytes);
	}
	
	if (failed_decryption_count == number_of_items && number_of_items != 0)  // failure == all records unreadable
		return(FALSE_m10);
	
	return(TRUE_m10);
}


TERN_m10     decrypt_time_series_data_m10(CMP_PROCESSING_STRUCT_m10 *cps, si8 number_of_items)
{
	ui1				*ui1_p, *key = NULL;
	si4                             encryption_blocks, encryptable_blocks;
	si8                             i, encryption_bytes;
	CMP_BLOCK_FIXED_HEADER_m10	*bh;
	PASSWORD_DATA_m10		*pwd;
	
	
	if (number_of_items == 0)
		return(TRUE_m10);
	
	// get decryption key - assume all blocks encrypted at same level
	pwd = cps->password_data;
	bh = cps->block_header;
	for (i = 0; i < number_of_items; ++i) {
		// check if already decrypted
		if ((bh->block_flags & CMP_BF_ENCRYPTION_MASK_m10) == 0) {
			bh = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *)bh + bh->total_block_bytes);
			continue;
		}
		if (bh->block_flags & CMP_BF_LEVEL_1_ENCRYPTION_MASK_m10) {
			if (bh->block_flags & CMP_BF_LEVEL_2_ENCRYPTION_MASK_m10) {
				error_message_m10("%s(): Cannot decrypt data: flags indicate both level 1 & level 2 encryption\n", __FUNCTION__);
				return(FALSE_m10);
			}
			if (pwd->access_level >= LEVEL_1_ENCRYPTION_m10) {
				key = pwd->level_1_encryption_key;
				break;
			}
			else {
				error_message_m10("%s(): Cannot decrypt data: insufficient access\n", __FUNCTION__);
				return(FALSE_m10);
			}
		}
		else {  // level 2 bit is set
			if (pwd->access_level == LEVEL_2_ENCRYPTION_m10) {
				key = pwd->level_2_encryption_key;
				break;
			}
			else {
				error_message_m10("%s(): Cannot decrypt data: insufficient access\n", __FUNCTION__);
				return(FALSE_m10);
			}
		}
		bh = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *)bh + bh->total_block_bytes);
	}
	
	// no blocks encrypted
	if (i == number_of_items)
		return(TRUE_m10);
	
	// decrypt
	for (i = 0; i < number_of_items; ++i) {
		
		// block if already decrypted
		if ((bh->block_flags & CMP_BF_ENCRYPTION_MASK_m10) == 0) {
			bh = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *)bh + bh->total_block_bytes);
			continue;
		}
		
		// calculated encyrption blocks
		encryptable_blocks = (bh->total_block_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10) / ENCRYPTION_BLOCK_BYTES_m10;
		if (bh->block_flags | CMP_BF_MBE_ENCODING_MASK_m10) {
			encryption_blocks = encryptable_blocks;
		}
		else {
			encryption_bytes = bh->total_header_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10 + ENCRYPTION_BLOCK_BYTES_m10;
			encryption_blocks = ((encryption_bytes - 1) / ENCRYPTION_BLOCK_BYTES_m10) + 1;
			if (encryptable_blocks < encryption_blocks)
				encryption_blocks = encryptable_blocks;
		}
		
		// decrypt
		ui1_p = (ui1 *)bh + CMP_BLOCK_ENCRYPTION_START_OFFSET_m10;
		for (i = 0; i < encryption_blocks; ++i) {
			AES_decrypt_m10(ui1_p, ui1_p, NULL, key);
			ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
		}
		
		// set block flags to decrypted
		bh->block_flags &= ~CMP_BF_ENCRYPTION_MASK_m10;
		
		// set pointer to next block
		bh = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *)bh + bh->total_block_bytes);
	}
	
	return(TRUE_m10);
}


si4     DST_offset_m10(si8 uutc)
{
	si4                             i, month, DST_start_month, DST_end_month;
	si4                             first_weekday_of_month, target_day_of_month, last_day_of_month;
	time_t                          utc, local_utc, change_utc;
	struct tm                       time_info = { 0 }, change_time_info = { 0 };
	DAYLIGHT_TIME_CHANGE_CODE_m10	*first_DTCC, *last_DTCC, *change_DTCC;
	
	
	// returns seconds to add to standard time (as UUTC) to adjust for DST on that date, in the globally specified timezone
	
	if (globals_m10->time_constants_set == FALSE_m10) {
		warning_message_m10("%s(): libary time constants not set\n", __FUNCTION__);
		return(0);
	}
	if (globals_m10->observe_DST < TRUE_m10)
		return(0);
	if (globals_m10->daylight_time_start_code.value == DTCC_VALUE_NO_ENTRY_m10) {
		warning_message_m10("%s(): daylight change data not available\n", __FUNCTION__);
		return(0);
	}

	utc = uutc / (si8) 1000000;
	
	// get broken out time info
#if defined MACOS_m10 || defined LINUX_m10
	if (globals_m10->daylight_time_start_code.reference_time == DTCC_LOCAL_REFERENCE_TIME) {
		local_utc = utc + (si8) globals_m10->standard_UTC_offset;
		gmtime_r(&local_utc, &time_info);
	}
	else {
		gmtime_r(&utc, &time_info);
	}
#endif
#ifdef WINDOWS_m10
	if (globals_m10->daylight_time_start_code.reference_time == DTCC_LOCAL_REFERENCE_TIME) {
		local_utc = utc + (si8) globals_m10->standard_UTC_offset;
		time_info = *(gmtime(&local_utc));
	}
	else {
		time_info = *(gmtime(&utc));
	}
#endif
	
	month = time_info.tm_mon;
	DST_start_month = globals_m10->daylight_time_start_code.month;
	DST_end_month = globals_m10->daylight_time_end_code.month;
	if (DST_start_month < DST_end_month) {
		first_DTCC = &globals_m10->daylight_time_start_code;
		last_DTCC = &globals_m10->daylight_time_end_code;
	}
	else {
		first_DTCC = &globals_m10->daylight_time_end_code;
		last_DTCC = &globals_m10->daylight_time_start_code;
	}
	
	// take care of dates not in change months
	if (month != DST_start_month && month != DST_end_month) {
		if (month > first_DTCC->month && month < last_DTCC->month) {
			if (first_DTCC->month == DST_start_month)
				return((si4)first_DTCC->shift_minutes * (si4)60);
			else
				return(0);
		}
		else if (month < first_DTCC->month) {
			if (first_DTCC->month == DST_start_month)
				return(0);
			else
				return((si4)first_DTCC->shift_minutes * (si4)60);
		}
		else {  // month > last_DTCC->month
			if (last_DTCC->month == DST_end_month)
				return(0);
			else
				return((si4)first_DTCC->shift_minutes * (si4)60);
		}
	}
	
	// get change utc
	if (month == first_DTCC->month)
		change_DTCC = first_DTCC;
	else
		change_DTCC = last_DTCC;
	
	change_time_info.tm_hour = change_DTCC->hours_of_day;
	change_time_info.tm_mon = month;
	change_time_info.tm_year = time_info.tm_year;
	
	if (change_DTCC->day_of_month == DTCC_DAY_OF_MONTH_NO_ENTRY) {   // get target day of month
		first_weekday_of_month = time_info.tm_wday - ((time_info.tm_mday - 1) % 7);
		target_day_of_month = (change_DTCC->day_of_week - first_weekday_of_month) + 1;
		if (target_day_of_month < 1)
			target_day_of_month += 7;
		if (change_DTCC->relative_weekday_of_month == DTCC_LAST_RELATIVE_WEEKDAY_OF_MONTH) {
			last_day_of_month = days_in_month_m10(month, time_info.tm_year + 1900);
			while (target_day_of_month <= last_day_of_month)
				target_day_of_month += 7;
			target_day_of_month -= 7;
		}
		else {
			for (i = 1; i < change_DTCC->relative_weekday_of_month; ++i)
				target_day_of_month += 7;
		}
		change_time_info.tm_mday = target_day_of_month;
	}
	else {
		change_time_info.tm_mday = change_DTCC->day_of_month;
	}
	
#if defined MACOS_m10 || defined LINUX_m10
	change_utc = timegm(&change_time_info);
#endif
#ifdef WINDOWS_m10
	// no timegm() in Windows - use mktime() & correct to UTC using local system timezone
	change_utc = _mktime32(&change_time_info);
	change_utc -= _timezone;  // current local offset to UTC in seconds
#endif
	if (globals_m10->daylight_time_start_code.reference_time == DTCC_LOCAL_REFERENCE_TIME)
		change_utc -= globals_m10->standard_UTC_offset;
	
	if (change_DTCC->month == DST_start_month) {
		if (utc >= change_utc)
			return((si4)change_DTCC->shift_minutes * (si4)60);
		else
			return(0);
	}
	else {  // change_DTCC->month == DST_end_month
		if (utc < change_utc)
			return((si4)change_DTCC->shift_minutes * (si4)-60);
		else
			return(0);
	}
	
	return(0);
}


TERN_m10	encrypt_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps)
{
	ui1			*ui1_p, *encryption_key;
	si4		        i, encryption_blocks;
	PASSWORD_DATA_m10	*pwd;
	
	
	// section 2 encrypt
	pwd = fps->password_data;
	if (fps->metadata->section_1.section_2_encryption_level < NO_ENCRYPTION_m10) {  // natively encrypted and currently decrypted
		if (pwd->access_level >= -fps->metadata->section_1.section_2_encryption_level) {
			fps->metadata->section_1.section_2_encryption_level = -fps->metadata->section_1.section_2_encryption_level;  // mark as currently encrypted
			if (fps->metadata->section_1.section_2_encryption_level == LEVEL_1_ENCRYPTION_m10)
				encryption_key = pwd->level_1_encryption_key;
			else
				encryption_key = pwd->level_2_encryption_key;
			encryption_blocks = METADATA_SECTION_2_BYTES_m10 / ENCRYPTION_BLOCK_BYTES_m10;
			ui1_p = fps->raw_data + METADATA_SECTION_2_OFFSET_m10;
			for (i = 0; i < encryption_blocks; ++i) {
				AES_encrypt_m10(ui1_p, ui1_p, NULL, encryption_key);
				ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
			}
		}
	}
	
	// section 3 encrypt
	if (fps->metadata->section_1.section_3_encryption_level < NO_ENCRYPTION_m10) {  // natively encrypted and currently decrypted
		if (pwd->access_level >= -fps->metadata->section_1.section_3_encryption_level) {
			fps->metadata->section_1.section_3_encryption_level = -fps->metadata->section_1.section_3_encryption_level;  // mark as currently encrypted
			if (fps->metadata->section_1.section_3_encryption_level == LEVEL_1_ENCRYPTION_m10)
				encryption_key = pwd->level_1_encryption_key;
			else
				encryption_key = pwd->level_2_encryption_key;
			encryption_blocks = METADATA_SECTION_3_BYTES_m10 / ENCRYPTION_BLOCK_BYTES_m10;
			ui1_p = fps->raw_data + METADATA_SECTION_3_OFFSET_m10;
			for (i = 0; i < encryption_blocks; ++i) {
				AES_encrypt_m10(ui1_p, ui1_p, NULL, encryption_key);
				ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
			}
		}
	}
	
	return(TRUE_m10);
}


TERN_m10	encrypt_records_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_HEADER_m10 *record_header, si8 number_of_items)
{
	ui4		        i, j, encryption_blocks;
	ui1			*ui1_p, *encryption_key;
	PASSWORD_DATA_m10	*pwd;
	
	
	pwd = fps->password_data;
	for (i = 0; i < number_of_items; ++i) {
		ui1_p = (ui1 *) record_header;
		if (record_header->encryption_level < NO_ENCRYPTION_m10) {
			record_header->encryption_level = -record_header->encryption_level;  // mark as currently encrypted
			if (record_header->encryption_level == LEVEL_1_ENCRYPTION_m10)
				encryption_key = pwd->level_1_encryption_key;
			else
				encryption_key = pwd->level_2_encryption_key;
			encryption_blocks = (record_header->total_record_bytes - RECORD_HEADER_BYTES_m10) / ENCRYPTION_BLOCK_BYTES_m10;
			ui1_p += RECORD_HEADER_BYTES_m10;
			for (j = 0; j < encryption_blocks; ++j) {
				AES_encrypt_m10(ui1_p, ui1_p, NULL, encryption_key);
				ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
			}
		}
		record_header = (RECORD_HEADER_m10 *) ((ui1 *) record_header + record_header->total_record_bytes);
	}
	
	return(TRUE_m10);
}
		
		
TERN_m10     encrypt_time_series_data_m10(CMP_PROCESSING_STRUCT_m10 *cps, si8 number_of_items)
{
	ui1				*ui1_p, *key;
	ui4                             encryption_mask;
	si4                             encryption_blocks, encryptable_blocks;
	si8                             i, encryption_bytes;
	PASSWORD_DATA_m10		*pwd;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	// check if already encrypted
	block_header = cps->block_header;
	if (block_header->block_flags & CMP_BF_ENCRYPTION_MASK_m10)
		return(TRUE_m10);
	
	pwd = cps->password_data;
	if (cps->directives.encryption_level > NO_ENCRYPTION_m10) {
		if (pwd->access_level >= cps->directives.encryption_level) {
			if (cps->directives.encryption_level == LEVEL_1_ENCRYPTION_m10) {
				key = pwd->level_1_encryption_key;
				encryption_mask = CMP_BF_LEVEL_1_ENCRYPTION_MASK_m10;
			}
			else {
				key = pwd->level_2_encryption_key;
				encryption_mask = CMP_BF_LEVEL_2_ENCRYPTION_MASK_m10;
			}
		}
		else {
			error_message_m10("%s(): Cannot encrypt data => returning without encrypting\n", __FUNCTION__);
			cps->directives.encryption_level = NO_ENCRYPTION_m10;
			return(FALSE_m10);
		}
	}
	else {
		return(TRUE_m10);
	}
	
	for (i = 0; i < number_of_items; ++i) {
		encryptable_blocks = (block_header->total_block_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10) / ENCRYPTION_BLOCK_BYTES_m10;
		if (block_header->block_flags | CMP_BF_MBE_ENCODING_MASK_m10) {
			encryption_blocks = encryptable_blocks;
		}
		else {
			encryption_bytes = block_header->total_header_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10 + ENCRYPTION_BLOCK_BYTES_m10;
			encryption_blocks = ((encryption_bytes - 1) / ENCRYPTION_BLOCK_BYTES_m10) + 1;
			if (encryptable_blocks < encryption_blocks)
				encryption_blocks = encryptable_blocks;
		}
		ui1_p = (ui1 *)block_header + CMP_BLOCK_ENCRYPTION_START_OFFSET_m10;
		for (i = 0; i < encryption_blocks; ++i) {
			AES_encrypt_m10(ui1_p, ui1_p, NULL, key);
			ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
		}
		block_header->block_flags |= encryption_mask;
		block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *)block_header + block_header->total_block_bytes);
	}
	
	return(TRUE_m10);
}
		
		
void    error_message_m10(si1 *fmt, ...)
{
	va_list		args;
	
	
	// RED suppressible text to stderr with beep, and option to exit program
	if (!(globals_m10->behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
		fprintf_m10(stderr, TC_RED_m10);
		va_start(args, fmt);
		UTF8_vfprintf_m10(stderr, fmt, args);
		va_end(args);
		if (globals_m10->behavior_on_fail & EXIT_ON_FAIL_m10)
			fprintf_m10(stderr, "Exiting.\n\n" TC_RESET_m10);
		else
			fprintf_m10(stderr, TC_RESET_m10);
		fflush(stderr);
	}
	
	if (globals_m10->behavior_on_fail & EXIT_ON_FAIL_m10)
		exit_m10(1);
	
	return;
}
		
		
void    escape_spaces_m10(si1 *string, si8 buffer_len)
{
	si1	*c1, *c2, *tmp_str;
	si8     spaces, len;
	
	
	// count
	for (spaces = 0, c1 = string; *c1++;)
		if (*c1 == 0x20)
			if (*(c1 - 1) != 0x5c)
				++spaces;
	len = (c1 - string) + spaces;
	if (buffer_len != 0) {  // if zero, proceed at caller's peril
		if (buffer_len < len) {
			error_message_m10("%s(): string buffer too small\n", __FUNCTION__);
			return;
		}
	}
	
	tmp_str = (si1 *) malloc_m10(len, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	c1 = string;
	c2 = tmp_str;
	while (*c1) {
		if (*c1 == 0x20) {  // space
			if (*(c1 - 1) != 0x5c)  // backslash
				*c2++ = 0x5c;
		}
		*c2++ = *c1++;
	}
	*c2 = 0;
	strcpy(string, tmp_str);
	
	free((void *) tmp_str);
	
	return;
}
		
		
void	extract_path_parts_m10(si1 *full_file_name, si1 *path, si1 *name, si1 *extension)
{
	si1	*c, *cc, temp_full_file_name[FULL_FILE_NAME_BYTES_m10], dir_break;
	si4	len;
	
	
	// get path from root
	path_from_root_m10(full_file_name, temp_full_file_name);
	
	// move pointer to end of string
	c = temp_full_file_name + strlen(temp_full_file_name) - 1;
	
#ifdef WINDOWS_m10
	dir_break = '\\';
#endif
#if defined MACOS_m10 || defined LINUX_m10
	dir_break = '/';
#endif
	// step back to first extension
	cc = c;
	while (*--c != '.') {
		if (*c == dir_break) {
			c = cc;
			break;
		}
	}
	
	// copy extension if allocated
	if (extension != NULL) {
		if (*c == '.') {
			len = strcpy_m10(extension, c + 1);
			if (len != (TYPE_BYTES_m10 - 1))
				warning_message_m10("%s(): \"%s\" is not a MED extension\n", __FUNCTION__, extension);
		}
		else {
			*extension = 0;
		}
	}
	if (*c == '.')
		*c-- = 0;
	
	// step back to next directory break
	while (*--c != dir_break);
	
	// copy name if allocated
	if (name != NULL)
		strncpy_m10(name, c + 1, BASE_FILE_NAME_BYTES_m10);
	*c = 0;
	
	// copy path if allocated
	if (path != NULL)
		strncpy_m10(path, temp_full_file_name, FULL_FILE_NAME_BYTES_m10);
	
	return;
}
		
		
void	extract_terminal_password_bytes_m10(si1 *password, si1 *password_bytes)
{
	si1	*s;     // terminal (most unique) bytes of UTF-8 password
	si4     i, j;
	ui4     ch;
	
	
	s = password;
	i = j = 0;
	do {
		ch = UTF8_next_char_m10(s, &i);   // "i" modified in UTF8_next_char_m10()
		password_bytes[j++] = (ui1) (ch & 0x000000FF);
	} while (ch);
	
	for (; j < PASSWORD_BYTES_m10; ++j)
		password_bytes[j] = 0;
	
	return;
}
		
		
ui4     file_exists_m10(si1 *path)  // can be used for directories also
{
	si1		tmp_path[FULL_FILE_NAME_BYTES_m10];
	si4             err;
	struct stat     s;
	
	
	if (path == NULL)
		return(DOES_NOT_EXIST_m10);
	
	if (*path == 0)
		return(DOES_NOT_EXIST_m10);
	
	if (path_from_root_m10(path, tmp_path) == FALSE_m10)
		path = tmp_path;
	
	errno = 0;
	err = stat(path, &s);
#if defined MACOS_m10 || defined LINUX_m10
	if (err == -1) {
		if (errno == ENOENT)
			return(DOES_NOT_EXIST_m10);
	}
	else if (S_ISDIR(s.st_mode))
		return(DIR_EXISTS_m10);
#endif
#ifdef WINDOWS_m10
	if (err == -1) {
		if (errno == ENOENT)
			return(DOES_NOT_EXIST_m10);
	}
	else if ((s.st_mode & S_IFMT) == S_IFDIR)
		return(DIR_EXISTS_m10);
#endif
	
	return(FILE_EXISTS_m10);
}


inline si8	file_length_m10(FILE *fp)
{
	si4		fd;
	struct stat	sb;
	
	
#if defined MACOS_m10 || defined LINUX_m10
	fd = fileno(fp);
#endif
#ifdef WINDOWS_m10
	fd = _fileno(fp);
#endif
	fstat(fd, &sb);
	
	return((si8)sb.st_size);
}
		
		
si8	*find_discontinuities_m10(TIME_SERIES_INDEX_m10 *tsi, si8 *num_disconts, si8 number_of_indices, TERN_m10 remove_offsets, TERN_m10 return_sample_numbers)
{
	si8	i, j, * disconts;
	
	
	for (i = *num_disconts = 0; i < number_of_indices; ++i)
		if (tsi[i].file_offset < 0)
			++(*num_disconts);
	
	disconts = (si8 *) malloc_m10(*num_disconts * sizeof(si8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	for (i = j = 0; i < number_of_indices; ++i)
		if (tsi[i].file_offset < 0)
			disconts[j++] = i;
	
	if (remove_offsets == TRUE_m10)
		for (i = 0; i < *num_disconts; ++i)
			tsi[disconts[i]].file_offset = -tsi[disconts[i]].file_offset;
	
	if (return_sample_numbers == TRUE_m10)
		for (i = 0; i < *num_disconts; ++i)
			disconts[i] = tsi[disconts[i]].start_sample_number;
	
	return(disconts);
}
		
		
#if defined MACOS_m10 || defined LINUX_m10
si1	*find_metadata_file_m10(si1 *path, si1 *md_path)
{
	TERN_m10	match;
	si1		*c, *name;
	ui4		code;
	size_t		len;
	DIR		*dir;
	struct dirent	*entry;
	
	
	// large directory trees can take a long time to search with "find" or "ls"
	// cumbersome code => function unrolled for speed

	// caller responsible for freeing, if allocated
	if (md_path == NULL)
		md_path = (si1 *) malloc_m10((size_t) FULL_FILE_NAME_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// find entry level
	strcpy(md_path, path);
	code = MED_type_code_from_string_m10(md_path);
	switch(code) {
		case SESSION_DIRECTORY_TYPE_CODE_m10:
			break;
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m10:
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m10:
			goto FIND_MDF_CHAN_LEVEL_m10;
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10:
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			goto FIND_MDF_SEG_LEVEL_m10;
		default:
			warning_message_m10("%s(): input path must be a MED session, channel, or segment directory\n", __FUNCTION__);
			return(NULL);
	}
	
	// session level
	dir = opendir(md_path);
	if (dir == NULL)
		return(NULL);
	match = FALSE_m10;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_DIR)
			continue;
		name = entry->d_name;
		if (*name == '.')
			continue;
		len = strlen(name);
		if (len < 6)  // min 1 letter channel name + 5 chars for extension
			continue;
		c = name + len - 5;
		if (*c++ != '.')
			continue;
		// check for MED channel ([tv]icd extension)
		if (*c == 't' || *c == 'v') {
			if (*++c == 'i') {
				if (*++c == 'c') {
					if (*++c == 'd') {
						match = TRUE_m10;
						break;
					}
				}
			}
		}
	}
	
	if (match == FALSE_m10)
		return(NULL);
	
	len = strlen(md_path);
	md_path[len++] = '/';
	strcpy(md_path + len, name);
	closedir(dir);
	
FIND_MDF_CHAN_LEVEL_m10:
	dir = opendir(md_path);
	if (dir == NULL)
		return(NULL);
	match = FALSE_m10;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_DIR)
			continue;
		name = entry->d_name;
		if (*name == '.')
			continue;
		len = strlen(name);
		if (len < 12)  // min 1 letter channel name + 6 chars for segment designation + 5 chars for extension
			continue;
		c = name + len - 5;
		if (*c++ != '.')
			continue;
		// check for MED segment ([tv]isd extension)
		if (*c == 't' || *c == 'v') {
			if (*++c == 'i') {
				if (*++c == 's') {
					if (*++c == 'd') {
						match = TRUE_m10;
						break;
					}
				}
			}
		}
	}
	
	if (match == FALSE_m10)
		return(NULL);
	
	len = strlen(md_path);
	md_path[len++] = '/';
	strcpy(md_path + len, name);
	closedir(dir);

FIND_MDF_SEG_LEVEL_m10:
	dir = opendir(md_path); // open the path
	if (dir == NULL)
		return(NULL); // if was not able, return
	match = FALSE_m10;
	while ((entry = readdir(dir)) != NULL) {  // if we were able to read something from the directory
		if (entry->d_type != DT_REG)
			continue;
		name = entry->d_name;
		if (*name == '.')
			continue;
		len = strlen(name);
		if (len < 12)  // min 1 letter channel name + 6 chars for segment desiognation + 5 chars for extension
			continue;
		c = name + len - 5;
		if (*c++ != '.')
			continue;
		// check for MED metadata ([tv]met extension)
		if (*c == 't' || *c == 'v') {
			if (*++c == 'm') {
				if (*++c == 'e') {
					if (*++c == 't') {
						match = TRUE_m10;
						break;
					}
				}
			}
		}
	}
	
	if (match == FALSE_m10)
		return(NULL);
	
	len = strlen(md_path);
	md_path[len++] = '/';
	strcpy(md_path + len, name);
	closedir(dir);

	return(md_path);
}
#endif  // MACOS_m10 || LINUX_m10


#ifdef WINDOWS_m10
si1	*find_metadata_file_m10(si1 *path, si1 *md_path)
{
	si1			*name, *c;
	ui4			code;
	size_t			len;
	WIN32_FIND_DATAA 	ffd;
	HANDLE 		        find_h;

	
	// large directory trees can take a long time to search with "find" or "ls"
	// cumbersome code => function unrolled for speed

	// caller responsible for freeing, if allocated
	if (md_path == NULL)
		md_path = (si1 *) malloc_m10((size_t) FULL_FILE_NAME_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// find entry level
	strcpy(md_path, path);
	code = MED_type_code_from_string_m10(path);
	switch(code) {
		case SESSION_DIRECTORY_TYPE_CODE_m10:
			break;
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m10:
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m10:
			goto WIN_FIND_MDF_CHAN_LEVEL_m10;
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10:
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			goto WIN_FIND_MDF_SEG_LEVEL_m10;
		default:
			warning_message_m10("%s(): input path must be a MED session, channel, or segment directory\n", __FUNCTION__);
			return(NULL);
	}
	
	// session level
	len = strlen(md_path);
	strcpy(md_path + len, "\\*.?icd");
	find_h = FindFirstFileA((LPCSTR) md_path, &ffd);
	if (find_h == INVALID_HANDLE_VALUE)
		return(NULL);
	name = ffd.cFileName;
	strcpy(md_path + len + 1, name);
	FindClose(find_h);
	
	// channel level
WIN_FIND_MDF_CHAN_LEVEL_m10:
	len = strlen(md_path);
	strcpy(md_path + len, "\\*.?isd");
	find_h = FindFirstFileA((LPCSTR) md_path, &ffd);
	if (find_h == INVALID_HANDLE_VALUE)
		return(NULL);
	name = ffd.cFileName;
	strcpy(md_path + len + 1, name);
	FindClose(find_h);

	// segment level
WIN_FIND_MDF_SEG_LEVEL_m10:
	len = strlen(md_path);
	strcpy(md_path + len, "\\*.?met");
	find_h = FindFirstFileA((LPCSTR) md_path, &ffd);
	if (find_h == INVALID_HANDLE_VALUE)
		return(NULL);
	name = ffd.cFileName;
	strcpy(md_path + len + 1, name);
	FindClose(find_h);

	return(md_path);
}
#endif  // WINDOWS_m10


void	force_behavior_m10(ui4 behavior)
{
	//*** THIS ROUTINE IS NOT THREAD SAFE - USE WITH CAUTION IN THREADED APPLICATIONS ***//
	
	static ui4	saved_behavior = GLOBALS_BEHAVIOR_ON_FAIL_DEFAULT_m10;
	
	
	if (behavior == RESTORE_BEHAVIOR_m10) {
		globals_m10->behavior_on_fail = saved_behavior;
		return;
	}
	
	saved_behavior = globals_m10->behavior_on_fail;
	globals_m10->behavior_on_fail = behavior;
	
	return;
}


void    free_2D_m10(void **ptr, size_t dim1, const si1 *function, si4 line)
{
	si8     i;
	
	
	if (ptr == NULL) {
		warning_message_m10("%s(): Attempting to free unallocated object [called from function %s(), line %d]\n", __FUNCTION__, function, line);
		return;
	}
	
	// assume allocated en bloc (dim1 == 0)
	if (dim1 == 0) {
		warning_message_m10("%s(): assuming allocated en bloc\n", __FUNCTION__);
		free((void *) ptr);
		return;
	}
	
	// allocated en bloc
	if ((ui8)ptr[0] == ((ui8)ptr + ((ui8)dim1 * (ui8)sizeof(void *) ))) {
		free((void *) ptr);
		return;
	}
	
	// separately allocated
	for (i = 0; i < dim1; ++i) {
		if (ptr[i] == NULL)
			continue;
		free((void *) ptr[i]);
	}
	free((void *) ptr);
	
	return;
}


void	free_channel_m10(CHANNEL_m10 *channel, TERN_m10 channel_allocated_en_bloc)
{
	si4	        i;
	TERN_m10        segments_allocated_en_bloc;
	
	
	if (channel == NULL) {
		warning_message_m10("%s(): trying to free a NULL CHANNEL_m10 structure => returning with no action\n", __FUNCTION__);
		return;
	}
	
	if (channel->segments != NULL) {
		// allocated with e_calloc_2D
		if ((ui8)channel->segments[0] == ((ui8)channel->segments + (channel->number_of_segments * (ui8)sizeof(void *) )))
			segments_allocated_en_bloc = TRUE_m10;
		else
			segments_allocated_en_bloc = FALSE_m10;
		
		for (i = 0; i < channel->number_of_segments; ++i)
			free_segment_m10(channel->segments[i], segments_allocated_en_bloc);
		
		if (segments_allocated_en_bloc == TRUE_m10)
			free((void *) channel->segments);
	}
	
	if (channel->metadata_fps != NULL)
		free_file_processing_struct_m10(channel->metadata_fps, FALSE_m10);
	if (channel->record_data_fps != NULL)
		free_file_processing_struct_m10(channel->record_data_fps, FALSE_m10);
	if (channel->record_indices_fps != NULL)
		free_file_processing_struct_m10(channel->record_indices_fps, FALSE_m10);
	
	if (channel_allocated_en_bloc == FALSE_m10)
		free((void *) channel);
	
	return;
}


void	free_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps, TERN_m10 allocated_en_bloc)
{
	if (fps == NULL) {
		warning_message_m10("%s(): trying to free a NULL FILE_PROCESSING_STRUCT_m10 => returning with no action\n", __FUNCTION__);
		return;
	}
	
	if (fps->raw_data != NULL && fps->raw_data_bytes > 0)
		free((void *) fps->raw_data);
	
	if (fps->cps != NULL && fps->directives.free_CMP_processing_struct == TRUE_m10)
		CMP_free_processing_struct_m10(fps->cps);
	
	if (fps->directives.free_password_data == TRUE_m10)
		if (fps->password_data != &globals_m10->password_data && fps->password_data != NULL)
			free((void *) fps->password_data);
	
	if (fps->directives.close_file == TRUE_m10)
		FPS_close_m10(fps);  // if already closed, this fails silently
	
	if (allocated_en_bloc == FALSE_m10)
		free((void *) fps);
	
	return;
}


void    free_globals_m10(void)
{
	if (globals_m10 == NULL) {
		warning_message_m10("%s(): trying to free a NULL GLOBALS_m10 structure => returning with no action\n", __FUNCTION__);
		return;
	}
	if (globals_m10->timezone_table != NULL)
		free((void *) globals_m10->timezone_table);
	
	if (globals_m10->country_aliases_table != NULL)
		free((void *) globals_m10->country_aliases_table);
	
	if (globals_m10->country_acronym_aliases_table != NULL)
		free((void* ) globals_m10->country_acronym_aliases_table);
	
	if (globals_m10->CMP_normal_CDF_table != NULL)
		free((void *) globals_m10->CMP_normal_CDF_table);
	
	if (globals_m10->CRC_table != NULL)
		free((void *) globals_m10->CRC_table);
	
	if (globals_m10->AES_sbox_table != NULL)
		free((void *) globals_m10->AES_sbox_table);
	
	if (globals_m10->AES_rsbox_table != NULL)
		free((void *) globals_m10->AES_rsbox_table);
	
	if (globals_m10->AES_rcon_table != NULL)
		free((void *) globals_m10->AES_rcon_table);
	
	if (globals_m10->SHA_h0_table != NULL)
		free((void *) globals_m10->SHA_h0_table);
	
	if (globals_m10->SHA_k_table != NULL)
		free((void *) globals_m10->SHA_k_table);
	
	if (globals_m10->UTF8_offsets_table != NULL)
		free((void *) globals_m10->UTF8_offsets_table);
	
	if (globals_m10->UTF8_trailing_bytes_table != NULL)
		free((void *) globals_m10->UTF8_trailing_bytes_table);
	
	free((void *) globals_m10);
	globals_m10 = NULL;
	
	return;
}


void	free_segment_m10(SEGMENT_m10 *segment, TERN_m10 segment_allocated_en_bloc)
{
	if (segment == NULL) {
		warning_message_m10("$s(): trying to free a NULL SEGMENT_m10 structure => returning with no action\n", __FUNCTION__);
		return;
	}
	if (segment->metadata_fps != NULL)
		free_file_processing_struct_m10(segment->metadata_fps, FALSE_m10);
	if (segment->time_series_data_fps != NULL)
		free_file_processing_struct_m10(segment->time_series_data_fps, FALSE_m10);
	if (segment->time_series_indices_fps != NULL)
		free_file_processing_struct_m10(segment->time_series_indices_fps, FALSE_m10);
	if (segment->video_indices_fps != NULL)
		free_file_processing_struct_m10(segment->video_indices_fps, FALSE_m10);
	if (segment->record_data_fps != NULL)
		free_file_processing_struct_m10(segment->record_data_fps, FALSE_m10);
	if (segment->record_indices_fps != NULL)
		free_file_processing_struct_m10(segment->record_indices_fps, FALSE_m10);
	
	if (segment_allocated_en_bloc == FALSE_m10)
		free((void *)  segment);
	
	return;
}


void	free_session_m10(SESSION_m10 *session)
{
	si4	        i;
	TERN_m10        allocated_en_bloc;
	
	
	if (session == NULL) {
		warning_message_m10("%s(): trying to free a NULL SESSION_m10 structure => returning with no action\n", __FUNCTION__);
		return;
	}
	if (session->time_series_metadata_fps != NULL)
		free_file_processing_struct_m10(session->time_series_metadata_fps, FALSE_m10);
	if (session->video_metadata_fps != NULL)
		free_file_processing_struct_m10(session->video_metadata_fps, FALSE_m10);
	if (session->record_data_fps != NULL)
		free_file_processing_struct_m10(session->record_data_fps, FALSE_m10);
	if (session->record_indices_fps != NULL)
		free_file_processing_struct_m10(session->record_indices_fps, FALSE_m10);
	
	if (session->number_of_time_series_channels) {
		if ((ui8)session->time_series_channels[0] == ((ui8)session->time_series_channels + (session->number_of_time_series_channels * (ui8)sizeof(void *) )))
			allocated_en_bloc = TRUE_m10;
		else
			allocated_en_bloc = FALSE_m10;
		
		for (i = 0; i < session->number_of_time_series_channels; ++i)
			free_channel_m10(session->time_series_channels[i], allocated_en_bloc);
		
		if (allocated_en_bloc == TRUE_m10)
			free((void *)  session->time_series_channels);
	}
	
	if (session->number_of_video_channels) {
		if ((ui8)session->video_channels[0] == ((ui8)session->video_channels + (session->number_of_video_channels * (ui8)sizeof(void *) )))
			allocated_en_bloc = TRUE_m10;
		else
			allocated_en_bloc = FALSE_m10;
		
		for (i = 0; i < session->number_of_video_channels; ++i)
			free_channel_m10(session->video_channels[i], allocated_en_bloc);
		
		if (allocated_en_bloc == TRUE_m10)
			free((void *)  session->video_channels);
	}
	
	if (session->segmented_record_data_fps != NULL) {
		if ((ui8)session->segmented_record_data_fps[0] == ((ui8)session->segmented_record_data_fps + (session->number_of_segments * (ui8)sizeof(void *) )))
			allocated_en_bloc = TRUE_m10;
		else
			allocated_en_bloc = FALSE_m10;
		
		for (i = 0; i < session->number_of_segments; ++i)
			free_file_processing_struct_m10(session->segmented_record_data_fps[i], allocated_en_bloc);
		
		if (allocated_en_bloc == TRUE_m10)
			free((void *)  session->segmented_record_data_fps);
	}
	
	if (session->segmented_record_indices_fps != NULL) {
		if ((ui8)session->segmented_record_indices_fps[0] == ((ui8)session->segmented_record_indices_fps + (session->number_of_segments * (ui8)sizeof(void *) )))
			allocated_en_bloc = TRUE_m10;
		else
			allocated_en_bloc = FALSE_m10;
		
		for (i = 0; i < session->number_of_segments; ++i)
			free_file_processing_struct_m10(session->segmented_record_indices_fps[i], allocated_en_bloc);
		
		if (allocated_en_bloc == TRUE_m10)
			free((void *)  session->segmented_record_indices_fps);
	}
	
	free((void *) session);
	
	return;
}


si1	**generate_file_list_m10(si1 **file_list, si4 *n_files, si1 *enclosing_directory, si1 *name, si1 *extension, ui1 path_parts, TERN_m10 free_input_file_list)
{
	TERN_m10	regex;
	si1		tmp_enclosing_directory[FULL_FILE_NAME_BYTES_m10], tmp_path[FULL_FILE_NAME_BYTES_m10];
	si1		tmp_name[FULL_FILE_NAME_BYTES_m10], tmp_extension[16], tmp_ext[16];
	si1		**tmp_ptr_ptr;
	si4		i, ret_val, n_in_files, *n_out_files;
	FILE		*fp;
	
	
	// can be used to get a directory list also
	// file_list entries, enclosing_directory, name, & extension can contain regexp
	// if file_list is NULL it will be allocated
	
	n_in_files = *n_files;
	n_out_files = n_files;
	
	// quick bailout for nothing to do
	if (path_parts == PP_FULL_PATH_m10) {
		if (check_file_list_m10(file_list, n_in_files) == TRUE_m10) {
			// caller expects a copy to be returned
			if (free_input_file_list == FALSE_m10) {
				tmp_ptr_ptr = (si1 **) calloc_2D_m10((size_t) n_in_files, FULL_FILE_NAME_BYTES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
				for (i = 0; i < n_in_files; ++i)
					strcpy(tmp_ptr_ptr[i], file_list[i]);
				file_list = tmp_ptr_ptr;
			}
			return(file_list);
		}
	}
		
	// copy incoming arguments so as not to modify, and in case they are const type
	if (enclosing_directory == NULL)
		*tmp_enclosing_directory = 0;
	else
		strcpy(tmp_enclosing_directory, enclosing_directory);
	enclosing_directory = tmp_enclosing_directory;
				
	if (name == NULL)
		*tmp_name = 0;
	else
		strcpy(tmp_name, name);
	name = tmp_name;
	
	if (extension == NULL)
		*tmp_extension = 0;
	else
		strcpy(tmp_extension, extension);
	extension = tmp_extension;
	
	// file list passed:
	// File_list components are assumed to contain the file name at a minimum (can be regex)
	// If list components do not have an enclosing directory, and one is passed, it is used.
	// If list components do not have an enclosing directory, and none is passed, path_from_root() is used.
	// If list components do not have an extension, and one is passed, it is used.
	// If list components do not have an extension, and none is passed, none is used.
	regex = FALSE_m10;
	if (file_list != NULL) {
		tmp_ptr_ptr = (si1 **) calloc_2D_m10((size_t) n_in_files, FULL_FILE_NAME_BYTES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		// copy file_list
		for (i = 0; i < n_in_files; ++i) {
			// check for regex
			if (regex == FALSE_m10) {
				if (str_contains_regex_m10(file_list[i]) == TRUE_m10)
					regex = TRUE_m10;
			}
			// fill in list entry path components
			extract_path_parts_m10(file_list[i], tmp_path, NULL, tmp_ext);
			if (*tmp_path == 0) {
				if (*enclosing_directory == 0)
					sprintf_m10(tmp_ptr_ptr[i], "%s/%s", enclosing_directory, file_list[i]);
				else
					path_from_root_m10(file_list[i], file_list[i]);
				
			} else {
				strcpy(tmp_ptr_ptr[i], file_list[i]);
			}
			if (*tmp_ext == 0 && *extension)
				sprintf_m10(tmp_ptr_ptr[i], "%s.%s", tmp_ptr_ptr[i], extension);
		}
		if (free_input_file_list == TRUE_m10)
			free_2D_m10((void **) file_list, n_in_files, __FUNCTION__, __LINE__);
		file_list = tmp_ptr_ptr;
	}

	// no file_list passed (+/- enclosing_directory, +/- name, +/- extension, are passed instead)
	// If no enclosing_directory passed, path_from_root() is used.
	// If no name is passed, "*" is used.
	// If no extension is passed, none is used.
	else {  // file_list == NULL
		file_list = (si1 **) calloc_2D_m10((size_t) 1, FULL_FILE_NAME_BYTES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		path_from_root_m10(enclosing_directory, enclosing_directory);
		if (*name)
			sprintf_m10(file_list[0], "%s/%s", enclosing_directory, name);
		else
			sprintf_m10(file_list[0], "%s/*", enclosing_directory);
		if (*extension)
			sprintf_m10(file_list[0], "%s.%s", file_list[0], extension);
		n_in_files = 1;
		if (str_contains_regex_m10(file_list[0]) == TRUE_m10)
			regex = TRUE_m10;
	}

	// expand regex (use system shell to expand regex)
	if (regex == TRUE_m10) {
		
	#if defined MACOS_m10 || defined LINUX_m10
		si1	*command, *tmp_str;
		
		command = (si1 *) calloc_m10((n_in_files * FULL_FILE_NAME_BYTES_m10) + 32, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		strcpy(command, "ls -1d");
		for (i = 0; i < n_in_files; ++i) {
			escape_spaces_m10(file_list[i], FULL_FILE_NAME_BYTES_m10);
			sprintf_m10(command, "%s %s", command, file_list[i]);
		}
		sprintf_m10(command, "%s > %s 2> %s", command, globals_m10->temp_file, NULL_DEVICE);
		free((void *) file_list);
		
		// count expanded file list
		*n_out_files = 0;
		ret_val = system_m10(command, FALSE_m10, __FUNCTION__, __LINE__, RETURN_ON_FAIL_m10 | SUPPRESS_ERROR_OUTPUT_m10);
		if (ret_val) {
			free((void *) command);
			return(NULL);
		}
		fp = fopen_m10(globals_m10->temp_file, "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		
		tmp_str = command;
		while (fscanf(fp, "%[^\n]", tmp_str) != EOF) {
			fgetc(fp);
			++(*n_out_files);
		}
		free((void *) tmp_str);
		
		if (*n_out_files == 0) {
			fclose(fp);
			return(NULL);
		}
		rewind(fp);
	#endif  // MACOS_m10 || LINUX_m10
		
	#ifdef WINDOWS_m10
		*n_out_files = win_ls_1d_to_tmp_m10(file_list, n_in_files, TRUE_m10);
		free((void *) file_list);
		if (*n_out_files == -1) {  // error
			*n_out_files = 0;
			return(NULL);
		}
		fp = fopen_m10(globals_m10->temp_file, "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	#endif  // WINDOWS_m10

		// re-allocate
		file_list = (si1 **) calloc_2D_m10((size_t) *n_out_files, FULL_FILE_NAME_BYTES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		
		// build file list
		for (i = 0; i < *n_out_files; ++i) {
			fscanf(fp, "%[^\n]", file_list[i]);
			fgetc(fp);
		}

		// clean up
		fclose(fp);
	}
	
	// return requested path parts
	if (path_parts != PP_FULL_PATH_m10) {
		for (i = 0; i < *n_out_files; ++i) {
			extract_path_parts_m10(file_list[i], enclosing_directory, tmp_name, tmp_extension);
			switch (path_parts) {
				case (PP_PATH_m10 | PP_NAME_m10):
					sprintf_m10(file_list[i], "%s/%s", enclosing_directory, tmp_name);
					break;
				case (PP_NAME_m10 | PP_EXTENSION_m10):
					sprintf_m10(file_list[i], "%s.%s", tmp_name, tmp_extension);
					break;
				case PP_NAME_m10:
					strcpy(file_list[i], tmp_name);
					break;
				default:
					error_message_m10("%s(): unrecognized path component combination (path_parts == %hhu)\n", __FUNCTION__, path_parts);
					break;
			}
		}
	}
	
	return(file_list);
}


si1	*generate_hex_string_m10(ui1 *bytes, si4 num_bytes, si1 *string)
{
	si4	i;
	si1	*s;
	
	
	if (string == NULL)  // allocate if NULL is passed
		string = (si1 *) calloc_m10((size_t)((num_bytes + 1) * 3), sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	s = string;
	*s++ = '0';
	*s++ = 'x';
	
	for (i = 0; i < num_bytes; ++i) {
		sprintf_m10(s, " %2x", bytes[i]);
		if (bytes[i] < 0x10)
			*(s + 1) = '0';
		s += 3;
	}
	*s = 0;
	
	return(string);
}


ui4    generate_MED_path_components_m10(si1 *path, si1 *MED_dir, si1 *MED_name)
{
	si1     extension[TYPE_BYTES_m10], local_MED_name[SEGMENT_BASE_FILE_NAME_BYTES_m10];
	si1     unescaped_path[FULL_FILE_NAME_BYTES_m10], temp_path[FULL_FILE_NAME_BYTES_m10];;
	si4     fe;
	ui4     code;
	
	
	strcpy(unescaped_path, path);
	unescape_spaces_m10(unescaped_path);
	fe = file_exists_m10(unescaped_path);
	
	// check input path: if file passed, get enclosing directory
	if (fe == FILE_EXISTS_m10) {
		extract_path_parts_m10(unescaped_path, temp_path, NULL, extension);
		code = MED_type_code_from_string_m10(extension);
		if (code == NO_TYPE_CODE_m10) {
			error_message_m10("%s(): passed file \"%s\" is not a MED file => returning\n", __FUNCTION__, path);
			return(NO_TYPE_CODE_m10);
		}
	}
	else if (fe == DIR_EXISTS_m10) {
		strcpy(temp_path, unescaped_path);
	}
	else {
		error_message_m10("%s(): passed path \"%s\" does not exist => returning\n", __FUNCTION__, path);
		return(NO_TYPE_CODE_m10);
	}
	
	if (MED_name == NULL)
		MED_name = local_MED_name;
	extract_path_parts_m10(temp_path, MED_dir, MED_name, extension);
	
	code = MED_type_code_from_string_m10(extension);
	switch (code) {
		case SESSION_DIRECTORY_TYPE_CODE_m10:
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m10:
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m10:
			snprintf_m10(MED_name, BASE_FILE_NAME_BYTES_m10, "%s", MED_name);
			break;
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10:
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			snprintf_m10(MED_name, SEGMENT_BASE_FILE_NAME_BYTES_m10, "%s", MED_name);
			break;
		default:
			error_message_m10("%s(): passed path \"%s\" is not a MED directory\n", __FUNCTION__, temp_path);
			return(NO_TYPE_CODE_m10);
	}
	
	if (MED_dir != NULL)
		snprintf_m10(MED_dir, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", MED_dir, MED_name, extension);
	
	return(code);
}


si1	**generate_numbered_names_m10(si1 **names, si1 *prefix, si4 number_of_names)
{
	si8     i;
	si1     number_str[FILE_NUMBERING_DIGITS_m10 + 1];
	
	
	if (names == NULL)
		names = (si1 **) calloc_2D_m10((size_t) number_of_names, SEGMENT_BASE_FILE_NAME_BYTES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	for (i = 0; i < number_of_names; ++i) {
		numerical_fixed_width_string_m10(number_str, FILE_NUMBERING_DIGITS_m10, i + 1);
		snprintf_m10(names[i], SEGMENT_BASE_FILE_NAME_BYTES_m10, "%s%s", prefix, number_str);
	}
	
	return(names);
}


si8	generate_recording_time_offset_m10(si8 recording_start_time_uutc)
{
	si4		dst_offset;
	time_t		epoch_utc, recording_start_time_utc, offset_utc_time;
	struct tm	local_time_info, offset_time_info;
	
	
	// receives UNOFFSET recording start time (or CURRENT_TIME_m10); returns OFFSET recording start time
	
	if (recording_start_time_uutc == CURRENT_TIME_m10) // use current system time
		recording_start_time_uutc = current_uutc_m10();
	
	recording_start_time_utc = recording_start_time_uutc / (si8)1000000;
	
	// get epoch & local time
	epoch_utc = 0;
#if defined MACOS_m10 || defined LINUX_m10
	gmtime_r(&epoch_utc, &offset_time_info);
	localtime_r(&recording_start_time_utc, &local_time_info);
#endif
#ifdef WINDOWS_m10
	offset_time_info = *(gmtime(&epoch_utc));
	local_time_info = *(localtime(&recording_start_time_utc));
#endif
	
	// set offset time info
	offset_time_info.tm_sec = local_time_info.tm_sec;
	offset_time_info.tm_min = local_time_info.tm_min;
	offset_time_info.tm_hour = local_time_info.tm_hour;
	
	// get offset UTC time
#if defined MACOS_m10 || defined LINUX_m10
	offset_utc_time = timegm(&offset_time_info);
#endif
#ifdef WINDOWS_m10
	// no timegm() in Windows - use mktime() & correct to UTC using local system timezone
	offset_utc_time = _mktime32(&offset_time_info);
	offset_utc_time -= _timezone;  // current local offset to UTC in seconds
#endif
	dst_offset = DST_offset_m10(recording_start_time_uutc);
	if (dst_offset)  // adjust to standard time if DST in effect
		offset_utc_time -= dst_offset;
	
	// set global offset
	globals_m10->recording_time_offset = (recording_start_time_utc - offset_utc_time) * (si8)1000000;
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("Recording Time Offset = %ld", globals_m10->recording_time_offset);
	
	globals_m10->RTO_known = TRUE_m10;
	
	return(recording_start_time_uutc - globals_m10->recording_time_offset);
}


si1	*generate_segment_name_m10(FILE_PROCESSING_STRUCT_m10 *fps, si1 *segment_name)
{
	si1	segment_number_str[FILE_NUMBERING_DIGITS_m10 + 1];
	
	
	if (segment_name == NULL)  // if NULL is passed, this will be allocated, but the calling function has the responsibility to free it.
		segment_name = (si1 *) malloc_m10((size_t)SEGMENT_BASE_FILE_NAME_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	numerical_fixed_width_string_m10(segment_number_str, FILE_NUMBERING_DIGITS_m10, fps->universal_header->segment_number);
	
	snprintf_m10(segment_name, SEGMENT_BASE_FILE_NAME_BYTES_m10, "%s_s%s", fps->universal_header->channel_name, segment_number_str);
	
	return(segment_name);
}


ui8	generate_UID_m10(ui8 *uid)
{
	si4	        i;
	ui1		*ui1_p;
	static ui8      local_UID;
	
	
	// Note if NULL is passed for uid, this function is not thread-safe
	if (uid == NULL)
		uid = (ui8 *)&local_UID;
	ui1_p = (ui1 *) uid;
	
RESERVED_UID_VALUE_m10:
#if defined MACOS_m10 || defined LINUX_m10
	for (i = 0; i < UID_BYTES_m10; ++i)
		ui1_p[i] = (ui1)(random() % (ui8)0x100);
#endif
#ifdef WINDOWS_m10
	for (i = 0; i < UID_BYTES_m10; ++i)
		ui1_p[i] = (ui1)((ui4)rand() % (ui4)0x100);
#endif
	switch (*uid) {
		case UID_NO_ENTRY_m10:
			goto RESERVED_UID_VALUE_m10;
		case CMP_BLOCK_START_UID_m10:
			goto RESERVED_UID_VALUE_m10;
	}
	
	return(*uid);
}


TERN_m10	get_channel_target_values_m10(CHANNEL_m10 *channel, si8 *target_uutc, si8 *target_sample_number, si4 *target_segment_number, ui1 mode)
{
	ui1                                     search_mode;
	si1                                     tmp_str[FULL_FILE_NAME_BYTES_m10];
	ui4                                     code;
	si8                                     i, target_time, target_samp, ridx_idx, prev_ridx_idx, n_recs;
	TERN_m10                                free_record_indices, free_record_data, Sgmt_records_exist;
	FILE_PROCESSING_STRUCT_m10		*ri_fps, *rd_fps;
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2;
	RECORD_INDEX_m10			*ri;
	RECORD_HEADER_m10			*rh;
	UNIVERSAL_HEADER_m10			*uh;
	REC_Sgmt_v10_m10			*Sgmt;
	SEGMENT_m10				*seg;
	
	
	if (mode != FIND_START_m10 && mode != FIND_END_m10) {
		error_message_m10("%s(): must pass FIND_START_m10 or FIND_END_m10 for mode\n", __FUNCTION__);
		return(FALSE_m10);
	}
	if (*target_uutc == UUTC_NO_ENTRY_m10) {
		search_mode = INDEX_SEARCH_m10;
		if (*target_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10) {
			if (mode == FIND_START_m10)
				target_samp = 0;
			else  // mode == FIND_END_m10
				target_samp = END_OF_TIME_m10;
		}
		else {
			target_samp = *target_sample_number;
		}
	}
	else {  // *target_uutc != UUTC_NO_ENTRY_m10
		search_mode = TIME_SEARCH_m10;
		target_time = *target_uutc;
	}
	
	// read channel records
	ri_fps = channel->record_indices_fps;
	free_record_indices = FALSE_m10;
	if (ri_fps != NULL)
		if (ftell(ri_fps->fp) != EOF)
			ri_fps = NULL;
	if (ri_fps == NULL) {
		sprintf_m10(tmp_str, "%s/%s.%s", channel->path, channel->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
		if (file_exists_m10(tmp_str) == FILE_EXISTS_m10)
			ri_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
		free_record_indices = TRUE_m10;
	}
	rd_fps = channel->record_data_fps;
	free_record_data = FALSE_m10;
	if (rd_fps != NULL)
		if (ftell(rd_fps->fp) != EOF)
			rd_fps = NULL;
	if (rd_fps == NULL) {
		sprintf_m10(tmp_str, "%s/%s.%s", channel->path, channel->name, RECORD_DATA_FILE_TYPE_STRING_m10);
		if (file_exists_m10(tmp_str) == FILE_EXISTS_m10)
			rd_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
		free_record_data = TRUE_m10;
	}
	
	// check that channel records contain Sgmt records
	Sgmt_records_exist = FALSE_m10;
	if (ri_fps != NULL && rd_fps != NULL) {
		uh = ri_fps->universal_header;
		n_recs = uh->number_of_entries;
		ri = ri_fps->record_indices;
		for (i = 0; i < n_recs; ++i) {
			if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10) {
				Sgmt_records_exist = TRUE_m10;
				break;
			}
		}
	}
	
	// use Sgmt records to get segment number
	if (Sgmt_records_exist == TRUE_m10) {
		
		if (search_mode == INDEX_SEARCH_m10) {
			for (; i < n_recs; ++i) {
				if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10) {
					Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[i].file_offset + RECORD_HEADER_BYTES_m10);
					ridx_idx = i;
					if (target_samp <= Sgmt->absolute_end_sample_number)
						break;
				}
			}
		}
		else {  // search_mode == TIME_SEARCH_m10
			ridx_idx = prev_ridx_idx = 0;
			for (; i < n_recs; ++i) {
				if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10) {
					Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[i].file_offset + RECORD_HEADER_BYTES_m10);
					prev_ridx_idx = ridx_idx;
					ridx_idx = i;
					if (target_time <= Sgmt->end_time) {
						rh = (RECORD_HEADER_m10 *) (rd_fps->raw_data + ri[i].file_offset);
						// time fell between segments
						if (target_time < rh->start_time) {
							if (mode == FIND_START_m10) {
								target_time = rh->start_time;
							}
							else {  // mode == FIND_END_m10
								ridx_idx = prev_ridx_idx;
								Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[ridx_idx].file_offset + RECORD_HEADER_BYTES_m10);
								target_time = Sgmt->end_time;
							}
						}
						break;
					}
				}
			}
		}
		if (i == n_recs) {
			if (search_mode == INDEX_SEARCH_m10) {
				if (mode == FIND_START_m10) {
					error_message_m10("%s(): target sample exceeds channel sample range\n", __FUNCTION__);
					if (free_record_indices == TRUE_m10)
						free_file_processing_struct_m10(ri_fps, FALSE_m10);
					if (free_record_data == TRUE_m10)
						free_file_processing_struct_m10(rd_fps, FALSE_m10);
					return(FALSE_m10);
				}
				target_samp = Sgmt->absolute_end_sample_number;
			}
			else { // search_mode == TIME_SEARCH_m10
				if (mode == FIND_START_m10) {
					error_message_m10("%s(): target uutc exceeds channel times\n", __FUNCTION__);
					if (free_record_indices == TRUE_m10)
						free_file_processing_struct_m10(ri_fps, FALSE_m10);
					if (free_record_data == TRUE_m10)
						free_file_processing_struct_m10(rd_fps, FALSE_m10);
					return(FALSE_m10);
				}
				target_time = Sgmt->end_time;
			}
		}
		*target_segment_number = Sgmt->segment_number;
		seg = channel->segments[*target_segment_number];
	}
	
	// Sgmt_records_exist == FALSE_m10 (search segments directly - less efficient)
	else {
		code = MED_type_code_from_string_m10(channel->path);
		if (code != TIME_SERIES_CHANNEL_TYPE_m10) {
			error_message_m10("%s(): cannot search video channels without Sgmt records at this time\n", __FUNCTION__);
			return(FALSE_m10);
		}
		for (i = 0; i < channel->number_of_segments; ++i) {
			seg = channel->segments[i];
			if (search_mode == INDEX_SEARCH_m10) {
				tmd2 = &seg->metadata_fps->metadata->time_series_section_2;
				if (target_samp < (tmd2->absolute_start_sample_number + tmd2->number_of_samples))
					break;
			}
			else {  // search_mode == TIME_SEARCH_m10
				uh = seg->metadata_fps->universal_header;
				if (target_time <= uh->file_end_time) {
					// time fell between segments
					if (target_time < uh->file_start_time) {
						if (mode == FIND_START_m10) {
							target_time = uh->file_start_time;
						}
						else {  // mode == FIND_END_m10
							seg = channel->segments[i - 1];
							uh = seg->metadata_fps->universal_header;
							target_time = uh->file_end_time;
						}
					}
				}
				break;
			}
		}
		if (i == channel->number_of_segments) {
			if (search_mode == INDEX_SEARCH_m10) {
				if (mode == FIND_START_m10) {
					error_message_m10("%s(): target sample exceeds channel sample range\n", __FUNCTION__);
					if (free_record_indices == TRUE_m10)
						free_file_processing_struct_m10(ri_fps, FALSE_m10);
					if (free_record_data == TRUE_m10)
						free_file_processing_struct_m10(rd_fps, FALSE_m10);
					return(FALSE_m10);
				}
				target_samp = tmd2->absolute_start_sample_number + tmd2->number_of_samples - 1;
			}
			else { // search_mode == TIME_SEARCH_m10
				if (mode == FIND_START_m10) {
					error_message_m10("%s(): target uutc exceeds channel times\n", __FUNCTION__);
					if (free_record_indices == TRUE_m10)
						free_file_processing_struct_m10(ri_fps, FALSE_m10);
					if (free_record_data == TRUE_m10)
						free_file_processing_struct_m10(rd_fps, FALSE_m10);
					return(FALSE_m10);
				}
				target_time = seg->metadata_fps->universal_header->file_end_time;
			}
		}
		*target_segment_number = seg->metadata_fps->universal_header->segment_number;
	}
	
	// clean up
	if (free_record_indices == TRUE_m10)
		free_file_processing_struct_m10(ri_fps, FALSE_m10);
	if (free_record_data == TRUE_m10)
		free_file_processing_struct_m10(rd_fps, FALSE_m10);
	
	get_segment_target_values_m10(seg, &target_time, &target_samp, mode);
	if (*target_uutc < 0)
		*target_uutc = -target_time;
	else
		*target_uutc = target_time;
	*target_sample_number = target_samp;
	
	return(TRUE_m10);
}


inline ui1	get_cpu_endianness_m10(void)
{
	ui2	x = 1;
	
	return(*((ui1 *) &x));
}


LOCATION_INFO_m10	*get_location_info_m10(LOCATION_INFO_m10 *loc_info, TERN_m10 set_timezone_globals, TERN_m10 prompt)
{
	TERN_m10	free_loc_info = FALSE_m10;
	si1		*command, temp_str[128], *buffer, *pattern, *c;
	si4		ret_val;
	si8		sz, len;
	FILE		*fp;
	time_t 		curr_time;
	struct tm 	loc_time;
	
	
	if (loc_info == NULL) {
		loc_info = (LOCATION_INFO_m10 *) calloc_m10((size_t)1, sizeof(LOCATION_INFO_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_loc_info = TRUE_m10;
	}
	
	command = "curl -s ipinfo.io";
	sprintf_m10(temp_str, "%s > %s 2> %s", command, globals_m10->temp_file, NULL_DEVICE);
	ret_val = system_m10(temp_str, FALSE_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	if (ret_val)
		return(NULL);
	fp = fopen_m10(globals_m10->temp_file, "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// get file length
	sz = file_length_m10(fp);
	
	// read output
	buffer = (si1 *) calloc_m10((size_t)sz, sizeof(si1), __FUNCTION__, __LINE__, EXIT_ON_FAIL_m10);
	fread_m10(buffer, sizeof(si1), (size_t)sz, fp, globals_m10->temp_file, __FUNCTION__, __LINE__, EXIT_ON_FAIL_m10);
	fclose(fp);
	
	// condition output
	strip_character_m10(buffer, '"');
	strip_character_m10(buffer, '"');
	
	// parse output
	pattern = "ip: ";
	if ((c = str_match_end_m10(pattern, buffer)) == NULL)
		error_message_m10("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%[^,]", loc_info->WAN_IPv4_address);
	
	pattern = "city: ";
	if ((c = str_match_end_m10(pattern, buffer)) == NULL)
		error_message_m10("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%[^,]", loc_info->locality);
	
	pattern = "region: ";
	if ((c = str_match_end_m10(pattern, buffer)) == NULL)
		error_message_m10("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%[^,]", loc_info->timezone_info.territory);
	
	pattern = "country: ";
	if ((c = str_match_end_m10(pattern, buffer)) == NULL)
		error_message_m10("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%[^,]", loc_info->timezone_info.country_acronym_2_letter);
	
	pattern = "loc: ";
	if ((c = str_match_end_m10(pattern, buffer)) == NULL)
		error_message_m10("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%lf,%lf", &loc_info->latitude, &loc_info->longitude);
	
	pattern = "postal: ";
	if ((c = str_match_end_m10(pattern, buffer)) == NULL)
		error_message_m10("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%[^,]", loc_info->postal_code);
	
	pattern = "timezone: ";
	if ((c = str_match_end_m10(pattern, buffer)) == NULL)
		error_message_m10("%s(): Could not match pattern \"%s\" in output of \"%s\"\n", __FUNCTION__, pattern, command);
	else
		sscanf(c, "%[^, ]", loc_info->timezone_description);
	
	free((void  *)buffer);
	
	// get timezone acronym from system
	curr_time = time(NULL);
#if defined MACOS_m10 || defined LINUX_m10
	localtime_r(&curr_time, &loc_time);
	len = strlen(loc_time.tm_zone);
	if (len >= 3) { // the table does not contain 2 letter timezone acronyms (e.g. MT for MST)
		if (loc_time.tm_isdst)
			strcpy(loc_info->timezone_info.daylight_timezone_acronym, loc_time.tm_zone);
		else
			strcpy(loc_info->timezone_info.standard_timezone_acronym, loc_time.tm_zone);
	}
#endif
#ifdef WINDOWS_m10
	loc_time = *(localtime(&curr_time));
	if (*_tzname[0])
		strcpy(loc_info->timezone_info.standard_timezone, _tzname[0]);
	if (*_tzname[1])
		strcpy(loc_info->timezone_info.daylight_timezone, _tzname[1]);
#endif

	if (set_timezone_globals == TRUE_m10) {
		if (set_global_time_constants_m10(&loc_info->timezone_info, 0, prompt) == FALSE_m10) {
			if (free_loc_info == TRUE_m10)
				free((void *) loc_info);
			warning_message_m10("%s(): Could not set timezone globals => returning NULL\n", __FUNCTION__);
			return(NULL);
		}
	}
	
	if (free_loc_info == TRUE_m10) {
		free((void *) loc_info);
		loc_info = NULL;
	}
	
	return(loc_info);
}


si4     get_segment_range_m10(si1 **channel_list, si4 n_channels, TIME_SLICE_m10 *slice)
{
	TERN_m10	search_succeeded;
	si1		*idx_chan_name, tmp_str[BASE_FILE_NAME_BYTES_m10];
	si8		i;
	
	
	// copy inputs to local variables (substitute NO_ENTRY for NULL)
	if (slice == NULL) {
		error_message_m10("%s(): NULL slice pointer\n", __FUNCTION__);
		return((si4)FALSE_m10);
	}
	
	// find index channel
	idx_chan_name = slice->sample_number_reference_channel_name;
	slice->sample_number_reference_channel_index = 0;
	if (*idx_chan_name) {
		for (i = 0; i < n_channels; ++i) {
			extract_path_parts_m10(channel_list[i], NULL, tmp_str, NULL);
			if (strcmp(tmp_str, idx_chan_name) == 0)
				break;
		}
		if (i == n_channels)
			warning_message_m10("%s(): Cannot find reference channel in file list => using first channel\n", __FUNCTION__);
		else
			slice->sample_number_reference_channel_index = i;
	}
	else {
		extract_path_parts_m10(channel_list[0], NULL, tmp_str, NULL);
		strcpy(idx_chan_name, tmp_str);
	}
	
	// search Sgmt records
	search_succeeded = search_Sgmt_records_m10(channel_list[slice->sample_number_reference_channel_index], slice);

	// search segment metadata
	if (search_succeeded == FALSE_m10)
		search_succeeded = search_segment_metadata_m10(channel_list[slice->sample_number_reference_channel_index], slice);
	
	if (search_succeeded == FALSE_m10)
		return((si4)FALSE_m10);
	
	return((slice->end_segment_number - slice->start_segment_number) + 1);
}


void    get_segment_target_values_m10(SEGMENT_m10 *segment, si8 *target_uutc, si8 *target_sample_number, ui1 mode)
{
	ui1					search_mode;
	si8                                     target_samp, target_time;
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2;
	UNIVERSAL_HEADER_m10			*uh;
	
	
	tmd2 = &segment->metadata_fps->metadata->time_series_section_2;
	if (*target_uutc == UUTC_NO_ENTRY_m10) {
		search_mode = INDEX_SEARCH_m10;
		if (*target_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10) {
			uh = segment->metadata_fps->universal_header;
			if (mode == FIND_START_m10) {
				*target_sample_number = tmd2->absolute_start_sample_number;
				*target_uutc = uh->file_start_time;
				return;
			}
			else {  // mode == FIND_END_m10
				*target_sample_number = tmd2->absolute_start_sample_number + tmd2->number_of_samples - 1;
				*target_uutc = uh->file_end_time;
				return;
			}
		}
		else {
			target_samp = *target_sample_number;
		}
	}
	else {  // *target_uutc != UUTC_NO_ENTRY_m10
		search_mode = TIME_SEARCH_m10;
		target_time = *target_uutc;
	}
	
	if (search_mode == INDEX_SEARCH_m10)
		target_time = uutc_for_sample_number_m10(tmd2->absolute_start_sample_number, UUTC_NO_ENTRY_m10, target_samp, tmd2->sampling_frequency, segment->time_series_indices_fps, mode);
	else  // search_mode == TIME_SEARCH_m10
		target_samp = sample_number_for_uutc_m10(tmd2->absolute_start_sample_number, UUTC_NO_ENTRY_m10, target_time, tmd2->sampling_frequency, segment->time_series_indices_fps, FIND_CURRENT_m10);
	
	if (*target_uutc < 0)
		*target_uutc = -target_time;
	else
		*target_uutc = target_time;
	*target_sample_number = target_samp;
	
	return;
}


TERN_m10	get_session_target_values_m10(SESSION_m10 *session, si8 *target_uutc, si8 *target_sample_number, si4 *target_segment_number, ui1 mode, si1 *idx_ref_chan)
{
	ui1                                     search_mode;
	si1                                     chan_name[BASE_FILE_NAME_BYTES_m10], tmp_str[FULL_FILE_NAME_BYTES_m10];
	TERN_m10                                ret_val, same_frequency, free_record_indices, free_record_data, Sgmt_records_exist;
	si4                                     ref_chan_idx, n_channels;
	si8                                     i, n_recs, target_time, target_samp, ridx_idx, prev_ridx_idx;
	CHANNEL_m10				*chan;
	SEGMENT_m10				*seg;
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2;
	VIDEO_METADATA_SECTION_2_m10		*vmd2;
	FILE_PROCESSING_STRUCT_m10		*ri_fps, *rd_fps;
	RECORD_INDEX_m10			*ri;
	RECORD_HEADER_m10			*rh;
	UNIVERSAL_HEADER_m10			*uh;
	REC_Sgmt_v10_m10			*Sgmt;
	
	
	same_frequency = TRUE_m10;
	vmd2 = NULL;
	tmd2 = &session->time_series_metadata_fps->metadata->time_series_section_2;
	if (tmd2 == NULL) {
		vmd2 = &session->time_series_metadata_fps->metadata->video_section_2;
		if (tmd2 == NULL) {
			error_message_m10("%s(): no section 2 metadata in session\n", __FUNCTION__);
			return(FALSE_m10);
		}
		else if (vmd2->frame_rate == VIDEO_METADATA_FRAME_RATE_NO_ENTRY_m10)
			same_frequency = FALSE_m10;
	}
	else {
		if (tmd2->sampling_frequency == FREQUENCY_NO_ENTRY_m10)
			same_frequency = FALSE_m10;
	}
	
	// find reference index channel, if passed
	ref_chan_idx = -1;
	if (idx_ref_chan != NULL) {
		if (*idx_ref_chan != 0) {
			extract_path_parts_m10(idx_ref_chan, NULL, chan_name, NULL);
			if (tmd2 != NULL) {
				n_channels = session->number_of_time_series_channels;
				for (i = 0; i < n_channels; ++i) {
					chan = session->time_series_channels[i];
					if (strcmp(chan->name, chan_name) == 0)
						break;
				}
			}
			else {  // vmd2 != NULL
				n_channels = session->number_of_video_channels;
				for (i = 0; i < n_channels; ++i) {
					chan = session->video_channels[i];
					if (strcmp(chan->name, chan_name) == 0)
						break;
				}
				
			}
			if (i != n_channels)
				ref_chan_idx = i;
		}
	}
	
	// must use channel
	if (same_frequency == FALSE_m10 && *target_sample_number != SAMPLE_NUMBER_NO_ENTRY_m10) {
		if (ref_chan_idx == -1) {
			warning_message_m10("%s(): No reference channel passed => using first channel\n", __FUNCTION__);
			ref_chan_idx = 0;
		}
		if (tmd2 != NULL)
			chan = session->time_series_channels[ref_chan_idx];
		else  // vmd2 != NULL
			chan = session->video_channels[ref_chan_idx];
		ret_val = get_channel_target_values_m10(chan, target_uutc, target_sample_number, target_segment_number, mode);
		
		return(ret_val);
	}
	
	// read session records
	ri_fps = session->record_indices_fps;
	free_record_indices = FALSE_m10;
	if (ri_fps != NULL)
		if (ftell(ri_fps->fp) != EOF)
			ri_fps = NULL;
	if (ri_fps == NULL) {
		sprintf_m10(tmp_str, "%s/%s.%s", session->path, session->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
		if (file_exists_m10(tmp_str) == FILE_EXISTS_m10)
			ri_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
		free_record_indices = TRUE_m10;
	}
	rd_fps = session->record_data_fps;
	free_record_data = FALSE_m10;
	if (rd_fps != NULL)
		if (ftell(rd_fps->fp) != EOF)
			rd_fps = NULL;
	if (rd_fps == NULL) {
		sprintf_m10(tmp_str, "%s/%s.%s", session->path, session->name, RECORD_DATA_FILE_TYPE_STRING_m10);
		if (file_exists_m10(tmp_str) == FILE_EXISTS_m10)
			rd_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
		free_record_data = TRUE_m10;
	}
	
	// check that session records contain Sgmt records
	Sgmt_records_exist = FALSE_m10;
	if (ri_fps != NULL && rd_fps != NULL) {
		uh = ri_fps->universal_header;
		n_recs = uh->number_of_entries;
		ri = ri_fps->record_indices;
		for (i = 0; i < n_recs; ++i) {
			if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10) {
				Sgmt_records_exist = TRUE_m10;
				break;
			}
		}
	}
	
	// must use channel
	if (Sgmt_records_exist == FALSE_m10) {
		if (free_record_indices == TRUE_m10)
			free_file_processing_struct_m10(ri_fps, FALSE_m10);
		if (free_record_data == TRUE_m10)
			free_file_processing_struct_m10(rd_fps, FALSE_m10);
		
		if (ref_chan_idx == -1) {
			warning_message_m10("%s(): No reference channel passed => using first channel\n", __FUNCTION__);
			ref_chan_idx = 0;
		}
		if (tmd2 != NULL)
			chan = session->time_series_channels[ref_chan_idx];
		else  // vmd2 != NULL
			chan = session->video_channels[ref_chan_idx];
		ret_val = get_channel_target_values_m10(chan, target_uutc, target_sample_number, target_segment_number, mode);
		
		return(ret_val);
	}
	
	// use session Sgmt records
	if (same_frequency == TRUE_m10) {
		if (*target_uutc == UUTC_NO_ENTRY_m10) {
			search_mode = INDEX_SEARCH_m10;
			if (*target_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10) {
				if (mode == FIND_START_m10)
					target_samp = 0;
				else  // mode == FIND_END_m10
					target_samp = END_OF_TIME_m10;
			}
			else {
				target_samp = *target_sample_number;
			}
		}
		else {  // *target_uutc != UUTC_NO_ENTRY_m10
			search_mode = TIME_SEARCH_m10;
			target_time = *target_uutc;
		}
	}
	else {  // same_frequency == FALSE_m10
		if (ref_chan_idx != -1) {  // try to use channel
			if (tmd2 != NULL)
				chan = session->time_series_channels[ref_chan_idx];
			else  // vmd2 != NULL
				chan = session->video_channels[ref_chan_idx];
			ret_val = get_channel_target_values_m10(chan, target_uutc, target_sample_number, target_segment_number, mode);
			if (ret_val == 1) {
				if (free_record_indices == TRUE_m10)
					free_file_processing_struct_m10(ri_fps, FALSE_m10);
				if (free_record_data == TRUE_m10)
					free_file_processing_struct_m10(rd_fps, FALSE_m10);
				return(ret_val);
			}
		}
		// must use time - can't get sample number
		search_mode = TIME_SEARCH_m10;
		*target_sample_number = SAMPLE_NUMBER_NO_ENTRY_m10;
		if (*target_uutc == UUTC_NO_ENTRY_m10) {
			if (mode == FIND_START_m10)
				target_time = 0;
			else  // mode == FIND_END_m10
				target_time = END_OF_TIME_m10;
		}
		else {
			target_time = *target_uutc;
		}
	}
	
	if (search_mode == INDEX_SEARCH_m10) {
		for (i = 0; i < n_recs; ++i) {
			if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10) {
				Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[i].file_offset + RECORD_HEADER_BYTES_m10);
				ridx_idx = i;
				if (target_samp <= Sgmt->absolute_end_sample_number)
					break;
			}
		}
	}
	else {  // search_mode == TIME_SEARCH_m10
		ridx_idx = prev_ridx_idx = 0;
		for (i = 0; i < n_recs; ++i) {
			if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10) {
				Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[i].file_offset + RECORD_HEADER_BYTES_m10);
				prev_ridx_idx = ridx_idx;
				ridx_idx = i;
				if (target_time <= Sgmt->end_time) {
					rh = (RECORD_HEADER_m10 *) (rd_fps->raw_data + ri[i].file_offset);
					// time fell between segments
					if (target_time < rh->start_time) {
						if (mode == FIND_START_m10) {
							target_time = rh->start_time;
						}
						else {  // mode == FIND_END_m10
							ridx_idx = prev_ridx_idx;
							Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[ridx_idx].file_offset + RECORD_HEADER_BYTES_m10);
							target_time = Sgmt->end_time;
						}
					}
					break;
				}
			}
		}
	}
	if (i == n_recs) {
		if (search_mode == INDEX_SEARCH_m10) {
			if (mode == FIND_START_m10) {
				error_message_m10("%s(): target sample exceeds channel sample range\n", __FUNCTION__);
				if (free_record_indices == TRUE_m10)
					free_file_processing_struct_m10(ri_fps, FALSE_m10);
				if (free_record_data == TRUE_m10)
					free_file_processing_struct_m10(rd_fps, FALSE_m10);
				return(FALSE_m10);
			}
			target_samp = Sgmt->absolute_end_sample_number;
		}
		else { // search_mode == TIME_SEARCH_m10
			if (mode == FIND_START_m10) {
				error_message_m10("%s(): target uutc exceeds channel times\n", __FUNCTION__);
				if (free_record_indices == TRUE_m10)
					free_file_processing_struct_m10(ri_fps, FALSE_m10);
				if (free_record_data == TRUE_m10)
					free_file_processing_struct_m10(rd_fps, FALSE_m10);
				return(FALSE_m10);
			}
			target_time = Sgmt->end_time;
		}
	}
	*target_segment_number = Sgmt->segment_number;
	
	if (tmd2 != NULL)
		seg = session->time_series_channels[ref_chan_idx]->segments[*target_segment_number];
	else  // vmd2 != NULL
		seg = session->video_channels[ref_chan_idx]->segments[*target_segment_number];
	get_segment_target_values_m10(seg, &target_time, &target_samp, mode);
	
	if (*target_uutc < 0)
		*target_uutc = -target_time;
	else
		*target_uutc = target_time;
	*target_sample_number = target_samp;
	
	// clean up
	if (free_record_indices == TRUE_m10)
		free_file_processing_struct_m10(ri_fps, FALSE_m10);
	if (free_record_data == TRUE_m10)
		free_file_processing_struct_m10(rd_fps, FALSE_m10);
	
	return(TRUE_m10);
}


FILE_PROCESSING_DIRECTIVES_m10	*initialize_file_processing_directives_m10(FILE_PROCESSING_DIRECTIVES_m10 *directives)
{
	if (directives == NULL)
		directives = (FILE_PROCESSING_DIRECTIVES_m10 *) calloc_m10((size_t) 1, sizeof(FILE_PROCESSING_DIRECTIVES_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// set directives to defaults
	directives->close_file = FPS_DIRECTIVES_CLOSE_FILE_DEFAULT_m10;
	directives->flush_after_write = FPS_DIRECTIVES_FLUSH_AFTER_WRITE_DEFAULT_m10;
	directives->update_universal_header = FPS_DIRECTIVES_UPDATE_UNIVERSAL_HEADER_DEFAULT_m10;
	directives->leave_decrypted = FPS_DIRECTIVES_LEAVE_DECRYPTED_DEFAULT_m10;
	directives->free_password_data = FPS_DIRECTIVES_FREE_PASSWORD_DATA_DEFAULT_m10;
	directives->free_CMP_processing_struct = FPS_DIRECTIVES_FREE_CMP_PROCESSING_STRUCT_DEFAULT_m10;
	directives->lock_mode = FPS_DIRECTIVES_LOCK_MODE_DEFAULT_m10;
	directives->open_mode = FPS_DIRECTIVES_OPEN_MODE_DEFAULT_m10;
	
	return(directives);
}


//***********************************************************************//
//****************************  MED GLOBALS  ****************************//
//***********************************************************************//

TERN_m10	initialize_globals_m10(void)
{
	if (globals_m10 == NULL) {
		globals_m10 = (GLOBALS_m10 *) calloc((size_t)1, sizeof(GLOBALS_m10));
		if (globals_m10 == NULL) {
			error_message_m10("%s(): calloc error\n", __FUNCTION__);
			return(FALSE_m10);
		}
	}
	
	// password structure
	memset((void *) &globals_m10->password_data, 0, sizeof(PASSWORD_DATA_m10));
	
	// time constants
	globals_m10->time_constants_set = FALSE_m10;
	globals_m10->RTO_known = GLOBALS_RTO_KNOWN_DEFAULT_m10;
	globals_m10->session_start_time = GLOBALS_SESSION_START_TIME_OFFSET_DEFAULT_m10;
	globals_m10->recording_time_offset = GLOBALS_RECORDING_TIME_OFFSET_DEFAULT_m10;
	globals_m10->standard_UTC_offset = GLOBALS_STANDARD_UTC_OFFSET_DEFAULT_m10;
	globals_m10->daylight_time_start_code.value = DTCC_VALUE_NO_ENTRY_m10;
	globals_m10->daylight_time_end_code.value = DTCC_VALUE_NO_ENTRY_m10;
	strcpy(globals_m10->standard_timezone_acronym, GLOBALS_STANDARD_TIMEZONE_ACRONYM_DEFAULT_m10);
	strcpy(globals_m10->standard_timezone_string, GLOBALS_STANDARD_TIMEZONE_STRING_DEFAULT_m10);
	strcpy(globals_m10->daylight_timezone_acronym, GLOBALS_DAYLIGHT_TIMEZONE_ACRONYM_DEFAULT_m10);
	strcpy(globals_m10->daylight_timezone_string, GLOBALS_DAYLIGHT_TIMEZONE_STRING_DEFAULT_m10);
	globals_m10->observe_DST = GLOBALS_OBSERVE_DST_DEFAULT_m10;
	if (globals_m10->timezone_table != NULL) {
		free((void *) globals_m10->timezone_table);
		globals_m10->timezone_table = NULL;
	}
	
	// alignment fields
	globals_m10->universal_header_aligned = UNKNOWN_m10;
	globals_m10->metadata_section_1_aligned = UNKNOWN_m10;
	globals_m10->time_series_metadata_section_2_aligned = UNKNOWN_m10;
	globals_m10->video_metadata_section_2_aligned = UNKNOWN_m10;
	globals_m10->metadata_section_3_aligned = UNKNOWN_m10;
	globals_m10->all_metadata_structures_aligned = UNKNOWN_m10;
	globals_m10->time_series_indices_aligned = UNKNOWN_m10;
	globals_m10->video_indices_aligned = UNKNOWN_m10;
	globals_m10->CMP_block_header_aligned = UNKNOWN_m10;
	globals_m10->CMP_record_header_aligned = UNKNOWN_m10;
	globals_m10->record_header_aligned = UNKNOWN_m10;
	globals_m10->record_indices_aligned = UNKNOWN_m10;
	globals_m10->all_record_structures_aligned = UNKNOWN_m10;
	globals_m10->all_structures_aligned = UNKNOWN_m10;
	
	// CMP
	if (globals_m10->CMP_normal_CDF_table != NULL) {
		free((void *) globals_m10->CMP_normal_CDF_table);
		globals_m10->CMP_normal_CDF_table = NULL;
	}
	
	// CRC
	if (globals_m10->CRC_table != NULL) {
		free((void *) globals_m10->CRC_table);
		globals_m10->CRC_table = NULL;
	}
	globals_m10->CRC_mode = GLOBALS_CRC_MODE_DEFAULT_m10;
	
	// AES
	if (globals_m10->AES_sbox_table != NULL) {
		free((void *) globals_m10->AES_sbox_table);
		globals_m10->AES_sbox_table = NULL;
	}
	if (globals_m10->AES_rsbox_table != NULL) {
		free((void *)globals_m10->AES_rsbox_table);
		globals_m10->AES_rsbox_table = NULL;
	}
	if (globals_m10->AES_rcon_table != NULL) {
		free((void *) globals_m10->AES_rcon_table);
		globals_m10->AES_rcon_table = NULL;
	}
	
	// SHA
	if (globals_m10->SHA_h0_table != NULL) {
		free((void *) globals_m10->SHA_h0_table);
		globals_m10->SHA_h0_table = NULL;
	}
	if (globals_m10->SHA_k_table != NULL) {
		free((void *) globals_m10->SHA_k_table);
		globals_m10->SHA_k_table = NULL;
	}
	
	// UTF-8
	if (globals_m10->UTF8_offsets_table != NULL) {
		free((void *) globals_m10->UTF8_offsets_table);
		globals_m10->UTF8_offsets_table = NULL;
	}
	if (globals_m10->UTF8_trailing_bytes_table != NULL) {
		free((void *) globals_m10->UTF8_trailing_bytes_table);
		globals_m10->UTF8_trailing_bytes_table = NULL;
	}
	
	// miscellaneous
	globals_m10->verbose = GLOBALS_VERBOSE_DEFAULT_m10;
	globals_m10->behavior_on_fail = GLOBALS_BEHAVIOR_ON_FAIL_DEFAULT_m10;
#if defined MACOS_m10 || defined LINUX_m10
	strcpy(globals_m10->temp_dir, "/tmp");
	strcpy(globals_m10->temp_file, "/tmp/junk");
#endif
#ifdef WINDOWS_m10
	GetTempPathA(FULL_FILE_NAME_BYTES_m10, globals_m10->temp_dir);
	sprintf_m10(globals_m10->temp_file, "%sjunk", globals_m10->temp_dir);
#endif
	
	return(TRUE_m10);
}


//***********************************************************************//
//**************************  END MED GLOBALS  **************************//
//***********************************************************************//


TERN_m10	initialize_medlib_m10(TERN_m10 check_structure_alignments, TERN_m10 initialize_all_tables)
{
	TERN_m10	return_value;
	
	
	// set up globals
	if (globals_m10 == NULL)
		initialize_globals_m10();
	
	// check cpu endianness
	if (get_cpu_endianness_m10() != LITTLE_ENDIAN_m10) {
		error_message_m10("%s(): Library only coded for little-endian machines currently\n", __FUNCTION__);
		exit_m10(1);
	}
	
	// check "char" type
	if (check_char_type_m10() == FALSE_m10) {
		error_message_m10("%s(): Library only coded for 8-bit signed chars currently\n", __FUNCTION__);
		exit_m10(1);
	}

	// check structure alignments
	if (check_structure_alignments == TRUE_m10)
		return_value = check_all_alignments_m10(__FUNCTION__, __LINE__);
	
	// seed random number generator
#if defined MACOS_m10 || defined LINUX_m10
	srandom((ui4) time(NULL));
#endif
#ifdef WINDOWS_m10
	srand((ui4) time(NULL));
#endif
	
#if defined WINDOWS_m10 && defined NEED_WIN_SOCKETS_m10
	// initialize Windows sockets DLL
	if (win_socket_startup_m10() == FALSE_m10)
		return_value = FALSE_m10;
#endif
	
#ifdef WINDOWS_m10
	// initialize Windows terminal
	if (win_initialize_terminal_m10() == FALSE_m10)
		return_value = FALSE_m10;
#endif
	
	if (initialize_all_tables == TRUE_m10) {
		// make normal CDF table global
		if (CMP_initialize_tables_m10() == FALSE_m10)
			return_value = FALSE_m10;
		
		// make CRC table global
		if (CRC_initialize_tables_m10() == FALSE_m10)
			return_value = FALSE_m10;
		
		// make UTF8 tables global
		if (UTF8_initialize_tables_m10() == FALSE_m10)
			return_value = FALSE_m10;
		
		// make AES tables global
		if (AES_initialize_tables_m10() == FALSE_m10)
			return_value = FALSE_m10;
		
		// make SHA tables global
		if (SHA_initialize_tables_m10() == FALSE_m10)
			return_value = FALSE_m10;
		
		// make timezone tables global
		if (initialize_timezone_tables_m10() == FALSE_m10)
			return_value = FALSE_m10;
	}

	return(return_value);
}


void	initialize_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps, TERN_m10 initialize_for_update)
{
	METADATA_SECTION_1_m10			*md1;
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2;
	VIDEO_METADATA_SECTION_2_m10		*vmd2;
	METADATA_SECTION_3_m10			*md3;
	UNIVERSAL_HEADER_m10			*uh;
	
	
	// shortcuts
	md1 = &fps->metadata->section_1;
	tmd2 = &fps->metadata->time_series_section_2;
	vmd2 = &fps->metadata->video_section_2;
	md3 = &fps->metadata->section_3;
	uh = fps->universal_header;
	
	// section 1 fields
	md1->section_2_encryption_level = METADATA_SECTION_2_ENCRYPTION_LEVEL_DEFAULT_m10;
	md1->section_3_encryption_level = METADATA_SECTION_3_ENCRYPTION_LEVEL_DEFAULT_m10;
	
	// section 2 fields
	
	// type independent fields
	memset(tmd2->session_description, 0, METADATA_SESSION_DESCRIPTION_BYTES_m10);
	memset(tmd2->channel_description, 0, METADATA_CHANNEL_DESCRIPTION_BYTES_m10);
	memset(tmd2->segment_description, 0, METADATA_SEGMENT_DESCRIPTION_BYTES_m10);
	memset(tmd2->equipment_description, 0, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m10);
	tmd2->acquisition_channel_number = METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m10;
	
	// type specific fields
	switch (uh->type_code) {
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
			tmd2->sampling_frequency = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10;
			tmd2->low_frequency_filter_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10;
			tmd2->high_frequency_filter_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10;
			tmd2->notch_filter_frequency_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10;
			tmd2->AC_line_frequency = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10;
			tmd2->amplitude_units_conversion_factor = TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10;
			tmd2->time_base_units_conversion_factor = TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10;
			tmd2->absolute_start_sample_number = TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_NO_ENTRY_m10;
			if (initialize_for_update == TRUE_m10) {
				tmd2->number_of_samples = 0;
				tmd2->number_of_blocks = 0;
				tmd2->maximum_block_bytes = 0;
				tmd2->maximum_block_samples = 0;
				tmd2->maximum_block_difference_bytes = 0;
				tmd2->maximum_block_duration = 0.0;
				tmd2->number_of_discontinuities = 0;
				tmd2->maximum_contiguous_blocks = 0;
				tmd2->maximum_contiguous_block_bytes = 0;
				tmd2->maximum_contiguous_samples = 0;
			}
			else {
				tmd2->number_of_samples = TIME_SERIES_METADATA_NUMBER_OF_SAMPLES_NO_ENTRY_m10;
				tmd2->number_of_blocks = TIME_SERIES_METADATA_NUMBER_OF_BLOCKS_NO_ENTRY_m10;
				tmd2->maximum_block_bytes = TIME_SERIES_METADATA_MAXIMUM_BLOCK_BYTES_NO_ENTRY_m10;
				tmd2->maximum_block_samples = TIME_SERIES_METADATA_MAXIMUM_BLOCK_SAMPLES_NO_ENTRY_m10;
				tmd2->maximum_block_difference_bytes = TIME_SERIES_METADATA_MAXIMUM_BLOCK_DIFFERENCE_BYTES_NO_ENTRY_m10;
				tmd2->maximum_block_duration = TIME_SERIES_METADATA_MAXIMUM_BLOCK_DURATION_NO_ENTRY_m10;
				tmd2->number_of_discontinuities = TIME_SERIES_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m10;
				tmd2->maximum_contiguous_blocks = TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_NO_ENTRY_m10;
				tmd2->maximum_contiguous_block_bytes = TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_NO_ENTRY_m10;
				tmd2->maximum_contiguous_samples = TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_NO_ENTRY_m10;
			}
			break;
		case VIDEO_METADATA_FILE_TYPE_CODE_m10:
			vmd2->horizontal_resolution = VIDEO_METADATA_HORIZONTAL_RESOLUTION_NO_ENTRY_m10;
			vmd2->vertical_resolution = VIDEO_METADATA_VERTICAL_RESOLUTION_NO_ENTRY_m10;
			vmd2->frame_rate = VIDEO_METADATA_FRAME_RATE_NO_ENTRY_m10;
			if (initialize_for_update == TRUE_m10) {
				vmd2->number_of_clips = 0;
				vmd2->maximum_clip_bytes = 0;
				vmd2->number_of_video_files = 0;
			}
			else {
				vmd2->number_of_clips = VIDEO_METADATA_NUMBER_OF_CLIPS_NO_ENTRY_m10;
				vmd2->maximum_clip_bytes = VIDEO_METADATA_MAXIMUM_CLIP_BYTES_NO_ENTRY_m10;
				vmd2->number_of_video_files = VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_NO_ENTRY_m10;
			}
			break;
		default:
			error_message_m10("%s(): Unrecognized METADATA SECTION 2 type in file \"%s\"\n", __FUNCTION__, fps->full_file_name);
			break;
	}
	
	// section 3 fields
	md3->recording_time_offset = globals_m10->recording_time_offset;
	md3->daylight_time_start_code = globals_m10->daylight_time_start_code;
	md3->daylight_time_end_code = globals_m10->daylight_time_end_code;
	strncpy_m10(md3->standard_timezone_acronym, globals_m10->standard_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m10);
	strncpy_m10(md3->standard_timezone_string, globals_m10->standard_timezone_string, TIMEZONE_STRING_BYTES_m10);
	strncpy_m10(md3->daylight_timezone_acronym, globals_m10->daylight_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m10);
	strncpy_m10(md3->daylight_timezone_string, globals_m10->daylight_timezone_string, TIMEZONE_STRING_BYTES_m10);
	memset(md3->subject_name_1, 0, METADATA_SUBJECT_NAME_BYTES_m10);
	memset(md3->subject_name_2, 0, METADATA_SUBJECT_NAME_BYTES_m10);
	memset(md3->subject_name_3, 0, METADATA_SUBJECT_NAME_BYTES_m10);
	memset(md3->subject_ID, 0, METADATA_SUBJECT_ID_BYTES_m10);
	memset(md3->recording_country, 0, METADATA_RECORDING_LOCATION_BYTES_m10);
	memset(md3->recording_territory, 0, METADATA_RECORDING_LOCATION_BYTES_m10);
	memset(md3->recording_locality, 0, METADATA_RECORDING_LOCATION_BYTES_m10);
	memset(md3->recording_institution, 0, METADATA_RECORDING_LOCATION_BYTES_m10);
	memset(md3->geotag_format, 0, METADATA_GEOTAG_FORMAT_BYTES_m10);
	memset(md3->geotag_data, 0, METADATA_GEOTAG_DATA_BYTES_m10);
	md3->standard_UTC_offset = globals_m10->standard_UTC_offset;
	
	return;
}


TIME_SLICE_m10	*initialize_time_slice_m10(TIME_SLICE_m10 *slice)
{
	if (slice == NULL)  // caller responsible for freeing
		slice = (TIME_SLICE_m10 *) malloc_m10(sizeof(TIME_SLICE_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	slice->conditioned = FALSE_m10;
	slice->start_time = slice->end_time = UUTC_NO_ENTRY_m10;
	slice->start_sample_number = slice->end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m10;
	slice->local_start_sample_number = slice->local_end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m10;
	slice->number_of_samples = NUMBER_OF_SAMPLES_NO_ENTRY_m10;
	slice->start_segment_number = slice->end_segment_number = SEGMENT_NUMBER_NO_ENTRY_m10;
	slice->session_start_time = slice->session_end_time = UUTC_NO_ENTRY_m10;
	*slice->sample_number_reference_channel_name = 0;
	slice->sample_number_reference_channel_index = 0;  // defaults to first channel
	
	return(slice);
}


TERN_m10	initialize_timezone_tables_m10(void)
{
	// timezone table
	if (globals_m10->timezone_table != NULL)
		free((void *) globals_m10->timezone_table);
	globals_m10->timezone_table = (TIMEZONE_INFO_m10 *) calloc_m10((size_t) TZ_TABLE_ENTRIES_m10, sizeof(TIMEZONE_INFO_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	{
		TIMEZONE_INFO_m10 temp[TZ_TABLE_ENTRIES_m10] = TZ_TABLE_m10;
		memcpy(globals_m10->timezone_table, temp, TZ_TABLE_ENTRIES_m10 * sizeof(TIMEZONE_INFO_m10));
	}

	// country aliases
	if (globals_m10->country_aliases_table != NULL)
		free((void *) globals_m10->country_aliases_table);
	globals_m10->country_aliases_table = (TIMEZONE_ALIAS_m10 *) calloc_m10((size_t)TZ_COUNTRY_ALIASES_ENTRIES_m10, sizeof(TIMEZONE_ALIAS_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	{
		TIMEZONE_ALIAS_m10 temp[TZ_COUNTRY_ALIASES_ENTRIES_m10] = TZ_COUNTRY_ALIASES_TABLE_m10;
		memcpy(globals_m10->country_aliases_table, temp, TZ_COUNTRY_ALIASES_ENTRIES_m10 * sizeof(TIMEZONE_ALIAS_m10));
	}
	
	// country acronym aliases
	if (globals_m10->country_acronym_aliases_table != NULL)
		free((void *) globals_m10->country_acronym_aliases_table);
	globals_m10->country_acronym_aliases_table = (TIMEZONE_ALIAS_m10 *) calloc_m10((size_t)TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m10, sizeof(TIMEZONE_ALIAS_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	{
		TIMEZONE_ALIAS_m10 temp[TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m10] = TZ_COUNTRY_ACRONYM_ALIASES_TABLE_m10;
		memcpy(globals_m10->country_acronym_aliases_table, temp, TZ_COUNTRY_ACRONYM_ALIASES_ENTRIES_m10 * sizeof(TIMEZONE_ALIAS_m10));
	}
	
	return(TRUE_m10);
}


void	initialize_universal_header_m10(FILE_PROCESSING_STRUCT_m10 *fps, ui4 type_code, TERN_m10 generate_file_UID, TERN_m10 originating_file)
{
	UNIVERSAL_HEADER_m10	*uh;
	
	
	uh = fps->universal_header;
	
	uh->header_CRC = uh->body_CRC = CRC_START_VALUE_m10;
	uh->segment_number = UNIVERSAL_HEADER_SEGMENT_NUMBER_NO_ENTRY_m10;
	uh->type_code = type_code;
	uh->MED_version_major = MED_VERSION_MAJOR_m10;
	uh->MED_version_minor = MED_VERSION_MINOR_m10;
	uh->byte_order_code = LITTLE_ENDIAN_m10;
	uh->session_start_time = UUTC_NO_ENTRY_m10;
	uh->file_start_time = UUTC_NO_ENTRY_m10;
	uh->file_end_time = UUTC_NO_ENTRY_m10;
	uh->number_of_entries = 0;
	uh->maximum_entry_size = 0;
	
	if (generate_file_UID == TRUE_m10)
		generate_UID_m10(&uh->file_UID);
	if (originating_file == TRUE_m10)
		uh->provenance_UID = uh->file_UID;
	
	return;
}


si1	*MED_type_string_from_code_m10(ui4 code)
{
	// could have written this differently, since the string bytes are the code bytes, just NULL terminated
	// but would've required accounting for endianness, and handling thread safety
	
	switch (code) {
		case NO_FILE_TYPE_CODE_m10:
			return NO_FILE_TYPE_STRING_m10;
		case SESSION_DIRECTORY_TYPE_CODE_m10:
			return SESSION_DIRECTORY_TYPE_STRING_m10;
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			return TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m10;
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			return VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m10;
		case RECORD_DATA_FILE_TYPE_CODE_m10:
			return RECORD_DATA_FILE_TYPE_STRING_m10;
		case RECORD_INDICES_FILE_TYPE_CODE_m10:
			return RECORD_INDICES_FILE_TYPE_STRING_m10;
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m10:
			return VIDEO_CHANNEL_DIRECTORY_TYPE_STRING_m10;
		case VIDEO_METADATA_FILE_TYPE_CODE_m10:
			return VIDEO_METADATA_FILE_TYPE_STRING_m10;
		case VIDEO_INDICES_FILE_TYPE_CODE_m10:
			return VIDEO_INDICES_FILE_TYPE_STRING_m10;
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m10:
			return TIME_SERIES_CHANNEL_DIRECTORY_TYPE_STRING_m10;
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
			return TIME_SERIES_METADATA_FILE_TYPE_STRING_m10;
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
			return TIME_SERIES_DATA_FILE_TYPE_STRING_m10;
		case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
			return TIME_SERIES_INDICES_FILE_TYPE_STRING_m10;
	}
	
	warning_message_m10("%s(): 0x%x is not a recognized MED file type code\n", __FUNCTION__, code);

	return(NULL);
}


ui4     MED_type_code_from_string_m10(si1 *string)
{
	ui4     code;
	si4     len;
	
	
	if (string == NULL) {
		warning_message_m10("%s(): string is NULL\n", __FUNCTION__);
		return(NO_FILE_TYPE_CODE_m10);
	}
	
	len = strlen(string);
	if (len < 5) {
		if (len != 4)
			return(NO_FILE_TYPE_CODE_m10);
	}
	else {
		string += (len - 5);
		if (*string++ != '.')
			return(NO_FILE_TYPE_CODE_m10);
	}
	
	memcpy((void *) &code, (void *) string, sizeof(ui4));
	
	switch (code) {
		case NO_FILE_TYPE_CODE_m10:
		case SESSION_DIRECTORY_TYPE_CODE_m10:
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10:
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10:
		case RECORD_DATA_FILE_TYPE_CODE_m10:
		case RECORD_INDICES_FILE_TYPE_CODE_m10:
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m10:
		case VIDEO_METADATA_FILE_TYPE_CODE_m10:
		case VIDEO_INDICES_FILE_TYPE_CODE_m10:
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m10:
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
		case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
			return code;
	}
	
	warning_message_m10("%s(): \"%s\" is not a recognized MED file type\n", __FUNCTION__, string);
	
	return(NO_FILE_TYPE_CODE_m10);
}


TERN_m10        merge_metadata_m10(FILE_PROCESSING_STRUCT_m10 *md_fps_1, FILE_PROCESSING_STRUCT_m10 *md_fps_2, FILE_PROCESSING_STRUCT_m10 *merged_md_fps)
{
	// if merged_md_fps == NULL, comparison results will be placed in md_fps_1->metadata
	// returns TRUE_m10 if md_fps_1->metadata == md_fps_2->metadata, FALSE_m10 otherwise
	
	ui4                                     type_code;
	METADATA_SECTION_1_m10			*md1_1, *md1_2, *md1_m;
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2_1, *tmd2_2, *tmd2_m;
	VIDEO_METADATA_SECTION_2_m10		*vmd2_1, *vmd2_2, *vmd2_m;
	METADATA_SECTION_3_m10			*md3_1, *md3_2, *md3_m;
	TERN_m10                                equal;
	
	
	// decrypt if needed
	md1_1 = &md_fps_1->metadata->section_1;
	if (md1_1->section_2_encryption_level > NO_ENCRYPTION_m10 || md1_1->section_3_encryption_level > NO_ENCRYPTION_m10)
		decrypt_metadata_m10(md_fps_1);
	md1_2 = &md_fps_2->metadata->section_1;
	if (md1_2->section_2_encryption_level > NO_ENCRYPTION_m10 || md1_2->section_3_encryption_level > NO_ENCRYPTION_m10)
		decrypt_metadata_m10(md_fps_2);
	
	// setup
	if (merged_md_fps == NULL)
		merged_md_fps = md_fps_1;
	else
		memcpy(merged_md_fps->metadata, md_fps_1->metadata, METADATA_BYTES_m10);
	md1_m = &merged_md_fps->metadata->section_1;
	
	type_code = md_fps_1->universal_header->type_code;
	if (type_code != md_fps_2->universal_header->type_code) {
		error_message_m10("%s(): mismatched type codes\n", __FUNCTION__);
		return(UNKNOWN_m10);
	}
	
	switch (type_code) {
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
		case VIDEO_METADATA_FILE_TYPE_CODE_m10:
			break;
		default:
			error_message_m10("%s(): unrecognized type code 0x%x\n", __FUNCTION__, type_code);
			return(UNKNOWN_m10);
	}
	equal = TRUE_m10;
	
	// section 1
	if (memcmp(md1_1->level_1_password_hint, md1_2->level_1_password_hint, PASSWORD_HINT_BYTES_m10)) {
		memset(md1_m->level_1_password_hint, 0, PASSWORD_HINT_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md1_1->level_2_password_hint, md1_2->level_2_password_hint, PASSWORD_HINT_BYTES_m10)) {
		memset(md1_m->level_2_password_hint, 0, PASSWORD_HINT_BYTES_m10); equal = FALSE_m10;
	}
	if (md1_1->section_2_encryption_level != md1_2->section_2_encryption_level) {
		md1_m->section_2_encryption_level = ENCRYPTION_LEVEL_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (md1_1->section_3_encryption_level != md1_2->section_3_encryption_level) {
		md1_m->section_3_encryption_level = ENCRYPTION_LEVEL_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (memcmp(md1_1->protected_region, md1_2->protected_region, METADATA_SECTION_1_PROTECTED_REGION_BYTES_m10)) {
		memset(md1_m->protected_region, 0, METADATA_SECTION_1_PROTECTED_REGION_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md1_1->discretionary_region, md1_2->discretionary_region, METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m10)) {
		memset(md1_m->discretionary_region, 0, METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m10); equal = FALSE_m10;
	}
	
	// section 2: times series channel
	if (type_code == TIME_SERIES_METADATA_FILE_TYPE_CODE_m10) {
		tmd2_1 = &md_fps_1->metadata->time_series_section_2;
		tmd2_2 = &md_fps_2->metadata->time_series_section_2;
		tmd2_m = &merged_md_fps->metadata->time_series_section_2;
		if (memcmp(tmd2_1->session_description, tmd2_2->session_description, METADATA_SESSION_DESCRIPTION_BYTES_m10)) {
			memset(tmd2_m->session_description, 0, METADATA_SESSION_DESCRIPTION_BYTES_m10); equal = FALSE_m10;
		}
		if (memcmp(tmd2_1->channel_description, tmd2_2->channel_description, METADATA_CHANNEL_DESCRIPTION_BYTES_m10)) {
			memset(tmd2_m->channel_description, 0, METADATA_CHANNEL_DESCRIPTION_BYTES_m10); equal = FALSE_m10;
		}
		if (memcmp(tmd2_1->segment_description, tmd2_2->segment_description, METADATA_SEGMENT_DESCRIPTION_BYTES_m10)) {
			memset(tmd2_m->segment_description, 0, METADATA_SEGMENT_DESCRIPTION_BYTES_m10); equal = FALSE_m10;
		}
		if (memcmp(tmd2_1->equipment_description, tmd2_2->equipment_description, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m10)) {
			memset(tmd2_m->equipment_description, 0, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m10); equal = FALSE_m10;
		}
		if (tmd2_1->acquisition_channel_number != tmd2_2->acquisition_channel_number) {
			tmd2_m->acquisition_channel_number = METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m10; equal = FALSE_m10;
		}
		if (memcmp(tmd2_1->reference_description, tmd2_2->reference_description, TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m10)) {
			memset(tmd2_m->reference_description, 0, TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m10); equal = FALSE_m10;
		}
		if (tmd2_1->sampling_frequency != tmd2_2->sampling_frequency) {
			if (tmd2_1->sampling_frequency == FREQUENCY_NO_ENTRY_m10 || tmd2_2->sampling_frequency == FREQUENCY_NO_ENTRY_m10)
				tmd2_m->sampling_frequency = FREQUENCY_NO_ENTRY_m10; // no entry supercedes variable frequency
			else
				tmd2_m->sampling_frequency = FREQUENCY_VARIABLE_m10;
			equal = FALSE_m10;
		}
		if (tmd2_1->low_frequency_filter_setting != tmd2_2->low_frequency_filter_setting) {
			tmd2_m->low_frequency_filter_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10; equal = FALSE_m10;
		}
		if (tmd2_1->high_frequency_filter_setting != tmd2_2->high_frequency_filter_setting) {
			tmd2_m->high_frequency_filter_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10; equal = FALSE_m10;
		}
		if (tmd2_1->notch_filter_frequency_setting != tmd2_2->notch_filter_frequency_setting) {
			tmd2_m->notch_filter_frequency_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10; equal = FALSE_m10;
		}
		if (tmd2_1->AC_line_frequency != tmd2_2->AC_line_frequency) {
			tmd2_m->AC_line_frequency = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10; equal = FALSE_m10;
		}
		if (tmd2_1->amplitude_units_conversion_factor != tmd2_2->amplitude_units_conversion_factor) {
			tmd2_m->amplitude_units_conversion_factor = TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10; equal = FALSE_m10;
		}
		if (memcmp(tmd2_1->amplitude_units_description, tmd2_2->amplitude_units_description, TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m10)) {
			memset(tmd2_m->amplitude_units_description, 0, TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m10); equal = FALSE_m10;
		}
		if (tmd2_1->time_base_units_conversion_factor != tmd2_2->time_base_units_conversion_factor) {
			tmd2_m->time_base_units_conversion_factor = TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10; equal = FALSE_m10;
		}
		if (memcmp(tmd2_1->time_base_units_description, tmd2_2->time_base_units_description, TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m10)) {
			memset(tmd2_m->time_base_units_description, 0, TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m10); equal = FALSE_m10;
		}
		if (tmd2_1->absolute_start_sample_number > tmd2_2->absolute_start_sample_number) {
			tmd2_m->absolute_start_sample_number = tmd2_2->absolute_start_sample_number; equal = FALSE_m10;
		}
		if (tmd2_1->number_of_samples < tmd2_2->number_of_samples) {
			tmd2_m->number_of_samples = tmd2_2->number_of_samples; equal = FALSE_m10;
		}
		if (tmd2_1->number_of_blocks < tmd2_2->number_of_blocks) {
			tmd2_m->number_of_blocks = tmd2_2->number_of_blocks; equal = FALSE_m10;
		}
		if (tmd2_1->maximum_block_bytes < tmd2_2->maximum_block_bytes) {
			tmd2_m->maximum_block_bytes = tmd2_2->maximum_block_bytes; equal = FALSE_m10;
		}
		if (tmd2_1->maximum_block_samples < tmd2_2->maximum_block_samples) {
			tmd2_m->maximum_block_samples = tmd2_2->maximum_block_samples; equal = FALSE_m10;
		}
		if (tmd2_1->maximum_block_difference_bytes < tmd2_2->maximum_block_difference_bytes) {
			tmd2_m->maximum_block_difference_bytes = tmd2_2->maximum_block_difference_bytes; equal = FALSE_m10;
		}
		if (tmd2_1->maximum_block_duration < tmd2_2->maximum_block_duration) {
			tmd2_m->maximum_block_duration = tmd2_2->maximum_block_duration; equal = FALSE_m10;
		}
		if (tmd2_1->number_of_discontinuities < tmd2_2->number_of_discontinuities) {
			tmd2_m->number_of_discontinuities = tmd2_2->number_of_discontinuities; equal = FALSE_m10;
		}
		if (tmd2_1->maximum_contiguous_blocks < tmd2_2->maximum_contiguous_blocks) {
			tmd2_m->maximum_contiguous_blocks = tmd2_2->maximum_contiguous_blocks; equal = FALSE_m10;
		}
		if (tmd2_1->maximum_contiguous_block_bytes < tmd2_2->maximum_contiguous_block_bytes) {
			tmd2_m->maximum_contiguous_block_bytes = tmd2_2->maximum_contiguous_block_bytes; equal = FALSE_m10;
		}
		if (tmd2_1->maximum_contiguous_samples < tmd2_2->maximum_contiguous_samples) {
			tmd2_m->maximum_contiguous_samples = tmd2_2->maximum_contiguous_samples; equal = FALSE_m10;
		}
		if (memcmp(tmd2_1->protected_region, tmd2_2->protected_region, TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m10)) {
			memset(tmd2_m->protected_region, 0, TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m10); equal = FALSE_m10;
		}
		if (memcmp(tmd2_1->discretionary_region, tmd2_2->discretionary_region, TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m10)) {
			memset(tmd2_m->discretionary_region, 0, TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m10); equal = FALSE_m10;
		}
		// section 2: times series channel
	}
	else if (type_code == VIDEO_METADATA_FILE_TYPE_CODE_m10) {
		vmd2_1 = &md_fps_1->metadata->video_section_2;
		vmd2_2 = &md_fps_2->metadata->video_section_2;
		vmd2_m = &merged_md_fps->metadata->video_section_2;
		if (memcmp(vmd2_1->session_description, vmd2_2->session_description, METADATA_SESSION_DESCRIPTION_BYTES_m10)) {
			memset(vmd2_m->session_description, 0, METADATA_SESSION_DESCRIPTION_BYTES_m10); equal = FALSE_m10;
		}
		if (memcmp(vmd2_1->channel_description, vmd2_2->channel_description, METADATA_CHANNEL_DESCRIPTION_BYTES_m10)) {
			memset(vmd2_m->channel_description, 0, METADATA_CHANNEL_DESCRIPTION_BYTES_m10); equal = FALSE_m10;
		}
		if (memcmp(vmd2_1->equipment_description, vmd2_2->equipment_description, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m10)) {
			memset(vmd2_m->equipment_description, 0, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m10); equal = FALSE_m10;
		}
		if (vmd2_1->acquisition_channel_number != vmd2_2->acquisition_channel_number) {
			vmd2_m->acquisition_channel_number = METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m10; equal = FALSE_m10;
		}
		if (vmd2_1->horizontal_resolution != vmd2_2->horizontal_resolution) {
			vmd2_m->horizontal_resolution = VIDEO_METADATA_HORIZONTAL_RESOLUTION_NO_ENTRY_m10; equal = FALSE_m10;
		}
		if (vmd2_1->vertical_resolution != vmd2_2->vertical_resolution) {
			vmd2_m->vertical_resolution = VIDEO_METADATA_VERTICAL_RESOLUTION_NO_ENTRY_m10; equal = FALSE_m10;
		}
		if (vmd2_1->frame_rate != vmd2_2->frame_rate) {
			vmd2_m->frame_rate = VIDEO_METADATA_FRAME_RATE_NO_ENTRY_m10; equal = FALSE_m10;
		}
		if (vmd2_1->number_of_clips < vmd2_2->number_of_clips) {
			vmd2_m->number_of_clips = vmd2_2->number_of_clips; equal = FALSE_m10;
		}
		if (vmd2_1->maximum_clip_bytes < vmd2_2->maximum_clip_bytes) {
			vmd2_m->maximum_clip_bytes = vmd2_2->maximum_clip_bytes; equal = FALSE_m10;
		}
		if (memcmp(vmd2_1->video_format, vmd2_2->video_format, VIDEO_METADATA_VIDEO_FORMAT_BYTES_m10)) {
			memset(vmd2_1->video_format, 0, VIDEO_METADATA_VIDEO_FORMAT_BYTES_m10); equal = FALSE_m10;
		}
		if (vmd2_1->number_of_video_files < vmd2_2->number_of_video_files) {
			vmd2_m->number_of_video_files = vmd2_2->number_of_video_files; equal = FALSE_m10;
		}
		if (memcmp(vmd2_1->protected_region, vmd2_2->protected_region, VIDEO_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m10)) {
			memset(vmd2_m->protected_region, 0, VIDEO_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m10); equal = FALSE_m10;
		}
		if (memcmp(vmd2_1->discretionary_region, vmd2_2->discretionary_region, VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m10)) {
			memset(vmd2_m->discretionary_region, 0, VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m10); equal = FALSE_m10;
		}
	}
	
	// section 3
	md3_1 = &md_fps_1->metadata->section_3;
	md3_2 = &md_fps_2->metadata->section_3;
	md3_m = &merged_md_fps->metadata->section_3;
	if (md3_1->recording_time_offset != md3_2->recording_time_offset) {
		md3_m->recording_time_offset = UUTC_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (md3_1->daylight_time_start_code.value != md3_2->daylight_time_start_code.value) {
		md3_m->daylight_time_start_code.value = DTCC_VALUE_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (md3_1->daylight_time_end_code.value != md3_2->daylight_time_end_code.value) {
		md3_m->daylight_time_end_code.value = DTCC_VALUE_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (memcmp(md3_1->standard_timezone_acronym, md3_2->standard_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m10)) {
		memset(md3_m->standard_timezone_acronym, 0, TIMEZONE_ACRONYM_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->standard_timezone_string, md3_2->standard_timezone_string, TIMEZONE_STRING_BYTES_m10)) {
		memset(md3_m->standard_timezone_string, 0, TIMEZONE_STRING_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->daylight_timezone_acronym, md3_2->daylight_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m10)) {
		memset(md3_m->daylight_timezone_acronym, 0, TIMEZONE_ACRONYM_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->daylight_timezone_string, md3_2->daylight_timezone_string, TIMEZONE_STRING_BYTES_m10)) {
		memset(md3_m->daylight_timezone_string, 0, TIMEZONE_STRING_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->subject_name_1, md3_2->subject_name_1, METADATA_SUBJECT_NAME_BYTES_m10)) {
		memset(md3_m->subject_name_1, 0, METADATA_SUBJECT_NAME_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->subject_name_2, md3_2->subject_name_2, METADATA_SUBJECT_NAME_BYTES_m10)) {
		memset(md3_m->subject_name_2, 0, METADATA_SUBJECT_NAME_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->subject_name_3, md3_2->subject_name_3, METADATA_SUBJECT_NAME_BYTES_m10)) {
		memset(md3_m->subject_name_3, 0, METADATA_SUBJECT_NAME_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->subject_ID, md3_2->subject_ID, METADATA_SUBJECT_ID_BYTES_m10)) {
		memset(md3_m->subject_ID, 0, METADATA_SUBJECT_ID_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->recording_country, md3_2->recording_country, METADATA_RECORDING_LOCATION_BYTES_m10)) {
		memset(md3_m->recording_country, 0, METADATA_SUBJECT_ID_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->recording_territory, md3_2->recording_territory, METADATA_RECORDING_LOCATION_BYTES_m10)) {
		memset(md3_m->recording_territory, 0, METADATA_SUBJECT_ID_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->recording_locality, md3_2->recording_locality, METADATA_RECORDING_LOCATION_BYTES_m10)) {
		memset(md3_m->recording_locality, 0, METADATA_RECORDING_LOCATION_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->recording_institution, md3_2->recording_institution, METADATA_RECORDING_LOCATION_BYTES_m10)) {
		memset(md3_m->recording_institution, 0, METADATA_RECORDING_LOCATION_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->geotag_format, md3_2->geotag_format, METADATA_GEOTAG_FORMAT_BYTES_m10)) {
		memset(md3_m->geotag_format, 0, METADATA_GEOTAG_FORMAT_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->geotag_data, md3_2->geotag_data, METADATA_GEOTAG_DATA_BYTES_m10)) {
		memset(md3_m->geotag_data, 0, METADATA_GEOTAG_DATA_BYTES_m10); equal = FALSE_m10;
	}
	if (md3_1->standard_UTC_offset != md3_2->standard_UTC_offset) {
		md3_m->standard_UTC_offset = STANDARD_UTC_OFFSET_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (memcmp(md3_1->protected_region, md3_2->protected_region, METADATA_SECTION_3_PROTECTED_REGION_BYTES_m10)) {
		memset(md3_m->protected_region, 0, METADATA_SECTION_3_PROTECTED_REGION_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(md3_1->discretionary_region, md3_2->discretionary_region, METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m10)) {
		memset(md3_m->discretionary_region, 0, METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m10); equal = FALSE_m10;
	}
	
	if (globals_m10->verbose == TRUE_m10) {
		switch (type_code) {
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
				printf_m10("------------ Merged Time Series Metadata --------------\n");
				break;
			case VIDEO_METADATA_FILE_TYPE_CODE_m10:
				printf_m10("--------------- Merged Video Metadata -----------------\n");
				break;
				break;
		}
		show_metadata_m10(NULL, merged_md_fps->metadata);
	}
	
	return(equal);
}


TERN_m10        merge_universal_headers_m10(FILE_PROCESSING_STRUCT_m10 *fps_1, FILE_PROCESSING_STRUCT_m10 * fps_2, FILE_PROCESSING_STRUCT_m10 * merged_fps)
{
	// if merged_fps == NULL, comparison results will be placed in fps_1->universal_header
	// returns TRUE_m10 if fps_1->universal_header == fps_2->universal_header, FALSE_m10 otherwise
	
	UNIVERSAL_HEADER_m10	*uh_1, *uh_2, *merged_uh;
	TERN_m10                equal = TRUE_m10;
	
	
	if (merged_fps == NULL)
		merged_fps = fps_1;
	else
		memcpy(merged_fps->universal_header, fps_1->universal_header, UNIVERSAL_HEADER_BYTES_m10);
	
	equal = TRUE_m10;
	uh_1 = fps_1->universal_header;
	uh_2 = fps_2->universal_header;
	merged_uh = merged_fps->universal_header;
	
	merged_uh->header_CRC = CRC_NO_ENTRY_m10; // CRCs not compared / merged
	merged_uh->body_CRC = CRC_NO_ENTRY_m10; // CRCs not compared / merged
	if (memcmp(uh_1->type_string, uh_2->type_string, TYPE_BYTES_m10)) {
		memset(merged_uh->type_string, 0, TYPE_BYTES_m10); equal = FALSE_m10;
	}
	if (uh_1->MED_version_major != uh_2->MED_version_major) {
		merged_uh->MED_version_major = UNIVERSAL_HEADER_MED_VERSION_MAJOR_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (uh_1->MED_version_minor != uh_2->MED_version_minor) {
		merged_uh->MED_version_minor = UNIVERSAL_HEADER_MED_VERSION_MINOR_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (uh_1->byte_order_code != uh_2->byte_order_code) {
		merged_uh->byte_order_code = UNIVERSAL_HEADER_BYTE_ORDER_CODE_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (uh_1->session_start_time == UUTC_NO_ENTRY_m10) {
		if (uh_2->session_start_time == UUTC_NO_ENTRY_m10)
			merged_uh->session_start_time = UUTC_NO_ENTRY_m10;
		else
			merged_uh->session_start_time = uh_2->session_start_time;
	}
	else if (uh_2->session_start_time == UUTC_NO_ENTRY_m10) {
		merged_uh->session_start_time = uh_1->session_start_time;
	}
	else {
		if (uh_1->session_start_time > uh_2->session_start_time) {
			merged_uh->session_start_time = uh_2->session_start_time;
			equal = FALSE_m10;
		}
	}
	if (uh_1->file_start_time == UUTC_NO_ENTRY_m10) {
		if (uh_2->file_start_time == UUTC_NO_ENTRY_m10)
			merged_uh->file_start_time = UUTC_NO_ENTRY_m10;
		else
			merged_uh->file_start_time = uh_2->file_start_time;
	}
	else if (uh_2->file_start_time == UUTC_NO_ENTRY_m10) {
		merged_uh->file_start_time = uh_1->file_start_time;
	}
	else {
		if (uh_1->file_start_time > uh_2->file_start_time) {
			merged_uh->file_start_time = uh_2->file_start_time;
			equal = FALSE_m10;
		}
	}
	if (uh_1->file_end_time == UUTC_NO_ENTRY_m10) {
		if (uh_2->file_end_time == UUTC_NO_ENTRY_m10)
			merged_uh->file_end_time = UUTC_NO_ENTRY_m10;
		else
			merged_uh->file_end_time = uh_2->file_end_time;
	}
	else if (uh_2->file_end_time == UUTC_NO_ENTRY_m10) {
		merged_uh->file_end_time = uh_1->file_start_time;
	}
	else {
		if (uh_1->file_end_time < uh_2->file_end_time) {
			merged_uh->file_end_time = uh_2->file_end_time;
			equal = FALSE_m10;
		}
	}
	if (uh_1->number_of_entries < uh_2->number_of_entries) {
		merged_uh->number_of_entries = uh_2->number_of_entries; equal = FALSE_m10;
	}
	if (uh_1->maximum_entry_size < uh_2->maximum_entry_size) {
		merged_uh->maximum_entry_size = uh_2->maximum_entry_size; equal = FALSE_m10;
	}
	if (uh_1->segment_number != uh_2->segment_number) {
		merged_uh->segment_number = UNIVERSAL_HEADER_SEGMENT_NUMBER_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (memcmp(uh_1->session_name, uh_2->session_name, BASE_FILE_NAME_BYTES_m10))
		memset(merged_uh->session_name, 0, BASE_FILE_NAME_BYTES_m10);
	if (memcmp(uh_1->channel_name, uh_2->channel_name, BASE_FILE_NAME_BYTES_m10))
		memset(merged_uh->channel_name, 0, BASE_FILE_NAME_BYTES_m10);
	if (memcmp(uh_1->anonymized_subject_ID, uh_2->anonymized_subject_ID, BASE_FILE_NAME_BYTES_m10))
		memset(merged_uh->anonymized_subject_ID, 0, UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_BYTES_m10);
	if (uh_1->session_UID != uh_2->session_UID) {
		merged_uh->session_UID = UID_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (uh_1->channel_UID != uh_2->channel_UID) {
		merged_uh->channel_UID = UID_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (uh_1->segment_UID != uh_2->segment_UID) {
		merged_uh->segment_UID = UID_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (uh_1->file_UID != uh_2->file_UID) {
		merged_uh->file_UID = UID_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (uh_1->provenance_UID != uh_2->provenance_UID) {
		merged_uh->provenance_UID = UID_NO_ENTRY_m10; equal = FALSE_m10;
	}
	if (memcmp(uh_1->level_1_password_validation_field, uh_2->level_1_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10)) {
		memset(merged_uh->level_1_password_validation_field, 0, PASSWORD_VALIDATION_FIELD_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(uh_1->level_2_password_validation_field, uh_2->level_2_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10)) {
		memset(merged_uh->level_2_password_validation_field, 0, PASSWORD_VALIDATION_FIELD_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(uh_1->level_3_password_validation_field, uh_2->level_3_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10)) {
		memset(merged_uh->level_3_password_validation_field, 0, PASSWORD_VALIDATION_FIELD_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(uh_1->protected_region, uh_2->protected_region, UNIVERSAL_HEADER_PROTECTED_REGION_BYTES_m10)) {
		memset(merged_uh->protected_region, 0, UNIVERSAL_HEADER_PROTECTED_REGION_BYTES_m10); equal = FALSE_m10;
	}
	if (memcmp(uh_1->discretionary_region, uh_2->discretionary_region, UNIVERSAL_HEADER_DISCRETIONARY_REGION_BYTES_m10)) {
		memset(merged_uh->discretionary_region, 0, UNIVERSAL_HEADER_DISCRETIONARY_REGION_BYTES_m10); equal = FALSE_m10;
	}
	
	return(equal);
}


void    message_m10(si1 *fmt, ...)
{
	va_list		args;
	
	
	// uncolored suppressible text to stdout
	if (!(globals_m10->behavior_on_fail & SUPPRESS_MESSAGE_OUTPUT_m10)) {
		va_start(args, fmt);
		UTF8_vprintf_m10(fmt, args);
		va_end(args);
		fflush(stdout);
	}
	
	return;
}


si1	*numerical_fixed_width_string_m10(si1 *string, si4 string_bytes, si4 number)
{
	si4	native_numerical_length, temp;
	si1	*c;
	
	
	// string bytes does not include terminal zero
	
	if (string == NULL)
		string = (si1 *) calloc_m10((size_t)(string_bytes + 1), sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	native_numerical_length = 0;
	temp = number;
	while (temp) {
		temp /= 10;
		++native_numerical_length;
	}
	if (number <= 0)
		++native_numerical_length;
	
	c = string;
	temp = string_bytes - native_numerical_length;
	while (temp--)
		*c++ = '0';
	
	(void)sprintf_m10(c, "%d", number);
	
	return(string);
}


si8     pad_m10(ui1 *buffer, si8 content_len, ui4 alignment)
{
	si8        i, pad_bytes;
	
	
	pad_bytes = content_len % (si8) alignment;
	if (pad_bytes) {
		i = pad_bytes = (alignment - pad_bytes);
		buffer += content_len;
		while (i--)
			*buffer++ = PAD_BYTE_VALUE_m10;
	}
	
	return(content_len + pad_bytes);
}


TERN_m10	path_from_root_m10(si1 *path, si1 *root_path)
{
	si1	*c, *c2, base_dir[FULL_FILE_NAME_BYTES_m10];
	si8	len, len2;
	
	
	// assumes root_path has adequate space for new path
	
	// if root_path == NULL : return T/F on path, do not modify path
	// if root_path == path : return T/F on path, do modify path
	// if root_path != path && root_path != NULL : return T/F on path, return path to root in root_path
	
	// if path starts with "/", returns TRUE ("C:" prepended in windows, if "modify_path" is TRUE)
	// Windows: if path starts with "<capital letter>:\" returns TRUE
	// if path starts with ".", "..", or "~", these are resolved as expected.
	// if modify_path == TRUE, assumes adequate space for modified path in path
	
	if (path == NULL)
		return(FALSE_m10);
		
#if defined MACOS_m10 || defined LINUX_m10
	if (*path == '/') {
		if (root_path != NULL && root_path != path)
			strcpy(root_path, path);
		return(TRUE_m10);
	}
	if (root_path == NULL)
		return(FALSE_m10);
	
	if (path != root_path)
		strcpy(root_path, path);
	
	// remove terminal '\' from passed path if present
	len = strlen(root_path);
	if (len) {
		if (root_path[len - 1] == '\\')
			root_path[--len] = 0;
	}
	
	// get base directory
	c = root_path;
	if (*c == '~') {
		strcpy(base_dir, getenv("HOME"));
		++c;
		if (*c == '/')
			++c;
	}
	else {
		getcwd_m10(base_dir, FULL_FILE_NAME_BYTES_m10);
	}
	
	// drop terminal '/' from base_dir, if present
	len2 = strlen(base_dir);
	if (base_dir[len2 - 1] == '/') {
		if (len2 > 1)  // at root
			base_dir[--len2] = 0;
	}
	
	// handle "." & ".."
	while (*c == '.') {
		if (*(c + 1) == '.') {  // backup base_dir to previous directory
			c2 = base_dir + len2;
			while (*--c2 != '/');
			if (c2 == base_dir)  // at root
				*++c2 = 0;
			else
				*c2 = 0;
			len2 = strlen(base_dir);
			++c;
		}
		if (*(c + 1) == '/')
			c += 2;
		else
			break;  // ".filename" (invisible) form
	}

	if (*c)
		sprintf_m10(root_path, "%s/%s", base_dir, c);  // Note c may overlap root_path so use sprintf_m10()
	else
		strcpy(root_path, base_dir);

	return(TRUE_m10);
#endif
	
#ifdef WINDOWS_m10
	if (*path == '\\') {
		// In a Windows shell, "\" refers to the lowest level of the current drive: roughly equivalent to a mount point.
		// If the caller passed a path that begins with "\", it is difficult to know if the intended drive was the
		// system drive, or the current working directory drive. However, if this is code that works across platforms,
		// it should represent the system drive, "C:". Also it is likely that if the intended drive were not the
		// system drive, the caller would have specified it.  It will be left as is, if "modify_path" is FALSE, however
		// as the purpose of this function is to regularize and complete partial paths to the fullest extent possible,
		// "C:\" is substituted for "\" here if "modify_path" is TRUE.
		if (root_path != NULL) {  // add the "C:"
			len = strlen(path);
			memmove(root_path + 2, path, len + 1);
			root_path[0] = 'C';
			root_path[1] = ':';
		}
		return(TRUE_m10);
	}
	
	// awkward but coeorces AND order
	// any "letter" drive can be considered path from root in Windows - no mount directory equivalent.
	if (path[0] >= 'A' && path[0] <= 'Z') {
		if (path[1] == ':') {
			if (path[2] == '\\') {
				if (root_path != NULL && root_path != path)
					strcpy(root_path, path);
				return(TRUE_m10);
			}
		}
	}
	
	if (root_path == NULL)
		return(FALSE_m10);
	
	if (root_path != path)
		strcpy(root_path, path);
	
	// remove terminal '\' from passed path if present
	len = strlen(root_path);
	if (len) {
		if (root_path[len - 1] == '\\')
			root_path[--len] = 0;
	}
	
	// get base directory
	c = root_path;
	if (*c == '~') {
		strcpy(base_dir, getenv("HOMEDRIVE"));
		strcat(base_dir, getenv("HOMEPATH"));
		++c;
		if (*c == '\\')
			++c;
	}
	else {
		getcwd_m10(base_dir, FULL_FILE_NAME_BYTES_m10);
	}
	
	// drop terminal '\' from base_dir if present
	len2 = strlen(base_dir);
	if (base_dir[len2 - 1] == '\\') {
		if (len2 > 3)  // at root: e.g. "C:\"
			base_dir[--len2] = 0;
	}
	
	// handle "." & ".."
	while (*c == '.') {
		if (*(c + 1) == '.') {  // backup base_dir to previous directory
			c2 = base_dir + len2;
			while (*--c2 != '\\');
			if (c2 == (base_dir + 2))  // at root: "C:\"
				*++c2 = 0;
			else
				*c2 = 0;
			len2 = strlen(base_dir);
			++c;
		}
		if (*(c + 1) == '\\')
			c += 2;
		else
			break;  // ".filename" (invisible) form
	}
	
	if (*c)
		sprintf_m10(root_path, "%s/%s", base_dir, c);  // Note c may overlap root_path so use sprintf_m10()
	else
		strcpy(root_path, base_dir);
	
	return(TRUE_m10);
#endif
}


TERN_m10	process_password_data_m10(si1 *unspecified_password, si1 *L1_password, si1 *L2_password, si1 *L3_password, si1 *L1_hint, si1 *L2_hint, FILE_PROCESSING_STRUCT_m10 *fps)
{
	PASSWORD_DATA_m10	*pwd;
	ui1			hash[SHA_HASH_BYTES_m10];
	si1			L1_password_bytes[PASSWORD_BYTES_m10] = {0}, L2_password_bytes[PASSWORD_BYTES_m10] = {0}, L3_password_bytes[PASSWORD_BYTES_m10] = {0};
	si1			unspecified_password_bytes[PASSWORD_BYTES_m10] = {0}, putative_L1_password_bytes[PASSWORD_BYTES_m10] = {0};
	si4			i;
	METADATA_SECTION_1_m10	*md1;
	UNIVERSAL_HEADER_m10	*uh;
	
	
	// returns FALSE_m10 to indicate no encryption/decryption access
	// as long as fps is valid, global password structure is set to processed, regardless of access
	
	// can't process passwords without a universal header
	if (fps == NULL) {
		warning_message_m10("%s(): FILE_PROCESSING_STRUCT_m10 is NULL\n", __FUNCTION__);
		return(FALSE_m10);
	}
	
	// set passed values to NULL if empty
	if (unspecified_password != NULL)
		if (*unspecified_password == 0)
			unspecified_password = NULL;
	if (L1_password != NULL)
		if (*L1_password == 0)
			L1_password = NULL;
	if (L2_password != NULL)
		if (*L2_password == 0)
			L2_password = NULL;
	if (L3_password != NULL)
		if (*L3_password == 0)
			L3_password = NULL;
	if (L1_hint != NULL)
		if (*L1_hint == 0)
			L1_hint = NULL;
	if (L2_hint != NULL)
		if (*L2_hint == 0)
			L2_hint = NULL;

	pwd = fps->password_data;
	memset((void *) pwd, 0, sizeof(PASSWORD_DATA_m10));
	pwd->processed = TRUE_m10;
	
	// return silently if no passwords passed: unencrypted files do not require passwords (passwword data still marked as processed)
	// if calling to get hints, just pass something (e.g. "x") in unspecified password
	if (unspecified_password == NULL && L1_password == NULL && L2_password == NULL)
		return(FALSE_m10);
	
	// copy password hints to metadata & password data
	uh = fps->universal_header;
	if (uh->type_code == TIME_SERIES_METADATA_FILE_TYPE_CODE_m10 || uh->type_code == VIDEO_METADATA_FILE_TYPE_CODE_m10) {
		md1 = &fps->metadata->section_1;
		if (L1_hint != NULL)
			strncpy_m10(md1->level_1_password_hint, L1_hint, PASSWORD_HINT_BYTES_m10);
		if (*md1->level_1_password_hint)
			memcpy(pwd->level_1_password_hint, md1->level_1_password_hint, PASSWORD_HINT_BYTES_m10);
		if (L2_hint != NULL)
			strncpy_m10(md1->level_2_password_hint, L2_hint, PASSWORD_HINT_BYTES_m10);
		if (*md1->level_2_password_hint)
			memcpy(pwd->level_2_password_hint, md1->level_2_password_hint, PASSWORD_HINT_BYTES_m10);
	}
	else {  // copy passed password hints to password data
		if (L1_hint != NULL)
			strncpy_m10(pwd->level_1_password_hint, L1_hint, PASSWORD_HINT_BYTES_m10);
		if (L2_hint != NULL)
			strncpy_m10(pwd->level_2_password_hint, L2_hint, PASSWORD_HINT_BYTES_m10);
	}

	// user passed single password for reading: validate against validation fields and generate encryption keys
	if (unspecified_password != NULL) {
		if (check_password_m10(unspecified_password) == TRUE_m10) {
			
			// get terminal bytes
			extract_terminal_password_bytes_m10(unspecified_password, unspecified_password_bytes);

			// check for level 1 access
			SHA_hash_m10((ui1 *) unspecified_password_bytes, PASSWORD_BYTES_m10, hash);  // generate SHA-256 hash of password bytes			
			for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i)  // compare with stored level 1 hash
				if (hash[i] != uh->level_1_password_validation_field[i])
					break;
			if (i == PASSWORD_BYTES_m10) {  // Level 1 password valid (cannot be level 2 password)
				pwd->access_level = LEVEL_1_ACCESS_m10;
				AES_key_expansion_m10(pwd->level_1_encryption_key, unspecified_password_bytes);  // generate key
				if (globals_m10->verbose == TRUE_m10)
					message_m10("Unspecified password is valid for Level 1 access");
				return(TRUE_m10);
			}
			
			// invalid level 1 => check if level 2 password
			for (i = 0; i < PASSWORD_BYTES_m10; ++i)  // xor with level 2 password validation field
				putative_L1_password_bytes[i] = hash[i] ^ uh->level_2_password_validation_field[i];
			
			SHA_hash_m10((ui1 *)putative_L1_password_bytes, PASSWORD_BYTES_m10, hash); // generate SHA-256 hash of putative level 1 password
			
			for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i)  // compare with stored level 1 hash
				if (hash[i] != uh->level_1_password_validation_field[i])
					break;
			if (i == PASSWORD_VALIDATION_FIELD_BYTES_m10) {  // Level 2 password valid
				pwd->access_level = LEVEL_2_ACCESS_m10;
				AES_key_expansion_m10(pwd->level_1_encryption_key, putative_L1_password_bytes);  // generate key
				AES_key_expansion_m10(pwd->level_2_encryption_key, unspecified_password_bytes);  // generate key
				if (globals_m10->verbose == TRUE_m10)
					message_m10("Unspecified password is valid for Level 1 and Level 2 access\n");
				return(TRUE_m10);
			}
			
			// invalid as level 2 password
			warning_message_m10("%s(): password is not valid for Level 1 or Level 2 access\n", __FUNCTION__);
		}
		// check_password_m10() == FALSE_m10 or unspecified password invalid
		show_password_hints_m10(pwd);
		return(FALSE_m10);
	}
	
	// user passed level 1 password for writing: generate validation field and encryption key
	if (check_password_m10(L1_password) == FALSE_m10)
		return(FALSE_m10);
	
	// passed a level 1 password - at least level 1 access
	pwd->access_level = LEVEL_1_ACCESS_m10;
	
	// get terminal bytes
	extract_terminal_password_bytes_m10(L1_password, L1_password_bytes);
	
	// generate Level 1 password validation field
	SHA_hash_m10((ui1 *) L1_password_bytes, PASSWORD_BYTES_m10, hash);
	memcpy(uh->level_1_password_validation_field, hash, PASSWORD_VALIDATION_FIELD_BYTES_m10);
	if (globals_m10->verbose == TRUE_m10)
		message_m10("Level 1 password validation field generated");
	
	// generate encryption key
	AES_key_expansion_m10(pwd->level_1_encryption_key, L1_password_bytes);
	if (globals_m10->verbose == TRUE_m10)
		message_m10("Level 1 encryption key generated");
	
	// user also passed level 2 password for writing: generate validation field and encryption key
	// Level 2 encryption requires a level 1 password, even if level 1 encryption is not used
	if (L2_password != NULL) {
		if (check_password_m10(L2_password) == TRUE_m10) {
			
			// passed a level 2 password - level 2 access
			pwd->access_level = LEVEL_2_ACCESS_m10;
			
			// get terminal bytes
			extract_terminal_password_bytes_m10(L2_password, L2_password_bytes);
			
			// generate Level 2 password validation field
			SHA_hash_m10((ui1 *)L2_password_bytes, PASSWORD_BYTES_m10, hash);
			memcpy(uh->level_2_password_validation_field, hash, PASSWORD_VALIDATION_FIELD_BYTES_m10);
			
			// exclusive or with level 1 password bytes
			for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i)
				uh->level_2_password_validation_field[i] ^= L1_password_bytes[i];
			if (globals_m10->verbose == TRUE_m10)
				message_m10("Level 2 password validation field generated");
			
			// generate encryption key
			AES_key_expansion_m10(pwd->level_2_encryption_key, L2_password_bytes);
			if (globals_m10->verbose == TRUE_m10)
				message_m10("Level 2 encryption key generated");
		}
		else {
			// check_password_m10() == FALSE_m10
			return(FALSE_m10);
		}
	}
	
	// user also passed level 3 password for recovery: generate validation field
	if (L3_password != NULL) {
		if (check_password_m10(L3_password) == TRUE_m10) {
			
			// get terminal bytes
			extract_terminal_password_bytes_m10(L3_password, L3_password_bytes);
			
			// generate Level 3 password validation field
			SHA_hash_m10((ui1 *)L3_password_bytes, PASSWORD_BYTES_m10, hash);
			memcpy(uh->level_3_password_validation_field, hash, PASSWORD_VALIDATION_FIELD_BYTES_m10);
			if (pwd->access_level == LEVEL_1_ACCESS_m10) {  // only level 1 password passed
				for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i) // exclusive or with level 1 password bytes
					uh->level_3_password_validation_field[i] ^= L1_password_bytes[i];
			}
			else {  // level 1 & level 2 passwords passed
				for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i) // exclusive or with level 2 password bytes
					uh->level_3_password_validation_field[i] ^= L2_password_bytes[i];
			}
			if (globals_m10->verbose == TRUE_m10)
				message_m10("Level 3 password validation field generated");
		}
		else {
			// check_password_m10() == FALSE_m10
			return(FALSE_m10);
		}
	}
	
	return(TRUE_m10);
}


CHANNEL_m10	*read_channel_m10(CHANNEL_m10 *chan, si1 *chan_dir, TIME_SLICE_m10 *slice, si1 *password, TERN_m10 read_time_series_data, TERN_m10 read_record_data)
{
	si1                     full_file_name[FULL_FILE_NAME_BYTES_m10], num_str[FILE_NUMBERING_DIGITS_m10 + 1];
	si1			*chan_list[1];
	ui4                     code;
	si4                     i, n_segs;
	si8                     items_read;
	SEGMENT_m10* seg;
	
	
	// allocate channel
	if (chan == NULL)
		chan = (CHANNEL_m10 *) calloc_m10((size_t)1, sizeof(CHANNEL_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// read whole channel
	if (slice == NULL)
		initialize_time_slice_m10(&chan->time_slice);
	else
		chan->time_slice = *slice;  // passed slice is not modified
	slice = &chan->time_slice;
	
	// get channel path & name
	if (chan_dir != NULL)
		code = generate_MED_path_components_m10(chan_dir, chan->path, chan->name);
	else if (*chan->path)
		code = generate_MED_path_components_m10(chan->path, chan->path, chan->name);
	else {
		error_message_m10("%s(): No channel directory passed\n", __FUNCTION__);
		return(NULL);
	}
	
	switch (code) {
		case TIME_SERIES_CHANNEL_TYPE_m10:
		case VIDEO_CHANNEL_TYPE_m10:
			break;
		default:
			error_message_m10("%s(): input file is not MED channel directory\n", __FUNCTION__);
			return(NULL);
	}
	
	// set up time & generate password data
	if (globals_m10->password_data.processed == 0)
		if (set_time_and_password_data_m10(password, chan->path, NULL, NULL) == FALSE_m10)
			return(NULL);
	
	// process slice (for unoffset & relative times)
	if (slice->conditioned == FALSE_m10)
		condition_time_slice_m10(slice);
	
	// get segment range
	if (slice->start_segment_number == SEGMENT_NUMBER_NO_ENTRY_m10 || slice->end_segment_number == SEGMENT_NUMBER_NO_ENTRY_m10) {
		chan_list[0] = chan->path;
		n_segs = get_segment_range_m10(chan_list, 1, slice);
	} else {
		n_segs = (slice->end_segment_number - slice->start_segment_number) + 1;
	}
	chan->number_of_segments = n_segs;
	
	// allocate segments
	chan->segments = (SEGMENT_m10 **) calloc_2D_m10((size_t) n_segs, 1, sizeof(SEGMENT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// generate segment names
	for (i = 0; i < n_segs; ++i) {
		seg = chan->segments[i];
		numerical_fixed_width_string_m10(num_str, FILE_NUMBERING_DIGITS_m10, i + slice->start_segment_number);
		switch (code) {
			case TIME_SERIES_CHANNEL_TYPE_m10:
				sprintf_m10(full_file_name, "%s/%s_s%s.%s", chan->path, chan->name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m10);
				break;
			case VIDEO_CHANNEL_TYPE_m10:
				sprintf_m10(full_file_name, "%s/%s_s%s.%s", chan->path, chan->name, num_str, VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m10);
				break;
		}
		generate_MED_path_components_m10(full_file_name, seg->path, seg->name);
	}
	
	// read segments
	for (i = 0; i < n_segs; ++i) {
		seg = chan->segments[i];
		if (read_segment_m10(seg, seg->path, slice, NULL, read_time_series_data, read_record_data) == NULL)
			return(NULL);
	}
	
	// update channel slice
	slice->start_time = chan->segments[0]->time_slice.start_time;
	slice->start_sample_number = chan->segments[0]->time_slice.start_sample_number;
	slice->end_time = chan->segments[n_segs - 1]->time_slice.end_time;
	slice->end_sample_number = chan->segments[n_segs - 1]->time_slice.end_sample_number;
	slice->number_of_samples = (slice->end_sample_number - slice->start_sample_number) + 1;
	slice->local_start_sample_number = slice->local_end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m10;
	
	// create channel-level metadata FPS
	seg = chan->segments[0];
	if (code == TIME_SERIES_CHANNEL_TYPE_m10) {
		sprintf_m10(full_file_name, "%s/%s.%s", chan->path, chan->name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
		chan->metadata_fps = allocate_file_processing_struct_m10(NULL, full_file_name, TIME_SERIES_METADATA_FILE_TYPE_CODE_m10, METADATA_BYTES_m10, seg->metadata_fps, METADATA_BYTES_m10);
	}
	else {  // VIDEO_CHANNEL_TYPE_m10
		sprintf_m10(full_file_name, "%s/%s.%s", chan->path, chan->name, VIDEO_METADATA_FILE_TYPE_STRING_m10);
		chan->metadata_fps = allocate_file_processing_struct_m10(NULL, full_file_name, VIDEO_METADATA_FILE_TYPE_CODE_m10, METADATA_BYTES_m10, seg->metadata_fps, METADATA_BYTES_m10);
	}
	chan->metadata_fps->fd = FPS_FD_EPHEMERAL_m10;
	chan->metadata_fps->universal_header->number_of_entries = 0;
	chan->metadata_fps->universal_header->maximum_entry_size = 0;
	for (i = 0; i < n_segs; ++i) {
		seg = chan->segments[i];
		if (seg->record_data_fps != NULL)
			merge_universal_headers_m10(chan->metadata_fps, seg->record_data_fps, NULL);
		if (seg->segmented_session_record_data_fps != NULL)
			merge_universal_headers_m10(chan->metadata_fps, seg->segmented_session_record_data_fps, NULL);
		merge_metadata_m10(chan->metadata_fps, seg->metadata_fps, NULL);
	}
	
	// read channel record indices (if present)
	sprintf_m10(full_file_name, "%s/%s.%s", chan->path, chan->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
	if (file_exists_m10(full_file_name) == FILE_EXISTS_m10) {
		chan->record_indices_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
		// read record data files (if present)
		sprintf_m10(full_file_name, "%s/%s.%s", chan->path, chan->name, RECORD_DATA_FILE_TYPE_STRING_m10);
		if (file_exists_m10(full_file_name) == FILE_EXISTS_m10) {
			if (read_record_data == TRUE_m10) {
				chan->record_data_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
			}
			else {
				chan->record_data_fps = allocate_file_processing_struct_m10(NULL, full_file_name, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, NULL, 0);
				chan->record_data_fps->directives.close_file = FALSE_m10;
				read_file_m10(chan->record_data_fps, full_file_name, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
			}
			merge_universal_headers_m10(chan->metadata_fps, chan->record_data_fps, NULL);
		}
		else {
			UTF8_fprintf_m10(stderr, "%s(): Channel record data file (\"%s\") does not exist, but record indices file does", __FUNCTION__, full_file_name);
		}
	}
	
	// fix channel metadata FPS (from merge functions)
	if (code == TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m10)
		chan->metadata_fps->universal_header->type_code = TIME_SERIES_METADATA_FILE_TYPE_CODE_m10;
	else // code == VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m10
		chan->metadata_fps->universal_header->type_code = VIDEO_METADATA_FILE_TYPE_CODE_m10;
	chan->metadata_fps->universal_header->segment_number = UNIVERSAL_HEADER_CHANNEL_LEVEL_CODE_m10;
	chan->metadata_fps->universal_header->segment_UID = UID_NO_ENTRY_m10;
	
	if (globals_m10->verbose == TRUE_m10) {
		printf_m10("-------------- Channel Universal Header ----------------\n");
		show_universal_header_m10(chan->metadata_fps, NULL);
		printf_m10("------------------ Channel Metadata --------------------\n");
		show_metadata_m10(chan->metadata_fps, NULL);
	}
	
	return(chan);
}


FILE_PROCESSING_STRUCT_m10	*read_file_m10(FILE_PROCESSING_STRUCT_m10 *fps, si1 *full_file_name, si8 number_of_items, ui1 **data_ptr_ptr, si8 *items_read, si1 *password, ui4 behavior_on_fail)
{
	ui1				*data_ptr, *raw_data_end;
	TERN_m10                        full_file_flag, readable, allocated_flag, external_array;
	si8                             i, in_bytes, tmp_in_bytes, bytes_left_in_file, used_bytes, required_bytes;
	UNIVERSAL_HEADER_m10		*uh;
	RECORD_HEADER_m10		*record_header;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if (items_read != NULL)
		*items_read = 0;
	
	allocated_flag = FALSE_m10;
	if (fps == NULL) {
		if (full_file_name == NULL) {
			warning_message_m10("%s(): FILE_PROCESSING_STRUCT_m10 and full_file_name are both NULL\n", __FUNCTION__);
			return(NULL);
		}
		fps = allocate_file_processing_struct_m10(NULL, full_file_name, NO_FILE_TYPE_CODE_m10, 0, NULL, 0);
		allocated_flag = TRUE_m10;
	}
	else if (full_file_name != NULL) {  // passed file name supersedes fps filename, if present
		if (*full_file_name)
			strncpy_m10(fps->full_file_name, full_file_name, FULL_FILE_NAME_BYTES_m10);
	}
	
	uh = fps->universal_header;
	if (fps->fp == NULL) {
		// read universal header
		if (!(fps->directives.open_mode & FPS_GENERIC_READ_OPEN_MODE_m10))
			fps->directives.open_mode = FPS_R_OPEN_MODE_m10;
		FPS_open_m10(fps, __FUNCTION__, __LINE__, behavior_on_fail);
		FPS_read_m10(fps, UNIVERSAL_HEADER_BYTES_m10, (void *)  uh, __FUNCTION__, __LINE__, behavior_on_fail);
		// process password
		if (number_of_items == FPS_UNIVERSAL_HEADER_ONLY_m10) {
			if (fps->password_data->processed == 0) // done below if not returning here
				process_password_data_m10(password, NULL, NULL, NULL, NULL, NULL, fps);
			return(fps);
		}
	}
	
	// get read size
	full_file_flag = FALSE_m10;
	if (number_of_items == FPS_FULL_FILE_m10) {
		in_bytes = fps->file_length - UNIVERSAL_HEADER_BYTES_m10;
		number_of_items = uh->number_of_entries;
		full_file_flag = TRUE_m10;
	}
	else {
		in_bytes = number_of_items * uh->maximum_entry_size;
	}
	bytes_left_in_file = fps->file_length - ftell(fps->fp);
	if (in_bytes > bytes_left_in_file)
		in_bytes = bytes_left_in_file;
	
	// check if data_ptr_ptr in current fps raw_data memory space
	external_array = FALSE_m10;
	if (data_ptr_ptr != NULL) {
		data_ptr = *data_ptr_ptr;
		raw_data_end = fps->raw_data + fps->raw_data_bytes;
		if (data_ptr < fps->raw_data || data_ptr >= raw_data_end)
			external_array = TRUE_m10;
	}
	else {
		data_ptr = fps->raw_data + UNIVERSAL_HEADER_BYTES_m10;
	}
	
	// reallocate raw_data memory space if necessary
	if (external_array == FALSE_m10) {
		used_bytes = (si8)data_ptr - (si8)fps->raw_data;
		required_bytes = used_bytes + in_bytes;
		if (fps->raw_data_bytes < required_bytes) {
			reallocate_file_processing_struct_m10(fps, required_bytes);
			data_ptr = fps->raw_data + used_bytes;
			if (data_ptr_ptr != NULL)
				*data_ptr_ptr = data_ptr;
		}
	}
	
	// read data
	FPS_read_m10(fps, in_bytes, data_ptr, __FUNCTION__, __LINE__, behavior_on_fail);
	switch (uh->type_code) {
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
			// this function is fine, but reading using time series indices is more efficient for reading CMP_blocks
			block_header = (CMP_BLOCK_FIXED_HEADER_m10 *)data_ptr;
			for (tmp_in_bytes = i = 0; i < number_of_items; ++i) {
				tmp_in_bytes += block_header->total_block_bytes;
				if (tmp_in_bytes > bytes_left_in_file) {
					tmp_in_bytes -= block_header->total_block_bytes;
					break;
				}
				block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *)block_header + block_header->total_block_bytes);
			}
			number_of_items = i;
			fseek_m10(fps->fp, tmp_in_bytes - in_bytes, SEEK_CUR, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);
			in_bytes = tmp_in_bytes;
			break;
		case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
			number_of_items = in_bytes / TIME_SERIES_INDEX_BYTES_m10;
			break;
		case RECORD_DATA_FILE_TYPE_CODE_m10:
			// this function is fine, but reading using record indices is more efficient for reading records
			record_header = (RECORD_HEADER_m10 *)data_ptr;
			for (tmp_in_bytes = i = 0; i < number_of_items; ++i) {
				tmp_in_bytes += record_header->total_record_bytes;
				if (tmp_in_bytes > bytes_left_in_file) {
					tmp_in_bytes -= record_header->total_record_bytes;
					break;
				}
				record_header = (RECORD_HEADER_m10 *) ((ui1 *)record_header + record_header->total_record_bytes);
			}
			number_of_items = i;
			fseek_m10(fps->fp, tmp_in_bytes - in_bytes, SEEK_CUR, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);
			in_bytes = tmp_in_bytes;
			break;
		case RECORD_INDICES_FILE_TYPE_CODE_m10:
			number_of_items = in_bytes / RECORD_INDEX_BYTES_m10;
			break;
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
		case VIDEO_METADATA_FILE_TYPE_CODE_m10:
			number_of_items = in_bytes / METADATA_BYTES_m10;
			break;
	}
	if (items_read != NULL)
		*items_read = number_of_items;
	
	// process password
	if (fps->password_data->processed == 0)						 // if metadata file, hints from section 1 will be added to password
		process_password_data_m10(password, NULL, NULL, NULL, NULL, NULL, fps);  // data structure, and displayed if the password is invalid
	
	// Validate CRCs
	if (globals_m10->CRC_mode & (CRC_VALIDATE_m10 | CRC_VALIDATE_ON_INPUT_m10)) {
		CRC_validate_m10(fps->raw_data + UNIVERSAL_HEADER_HEADER_CRC_START_OFFSET_m10, UNIVERSAL_HEADER_BYTES_m10 - UNIVERSAL_HEADER_HEADER_CRC_START_OFFSET_m10, uh->header_CRC);
		switch (fps->universal_header->type_code) {
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
				validate_time_series_data_CRCs_m10((CMP_BLOCK_FIXED_HEADER_m10 *)data_ptr, number_of_items);
				break;
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
				if (full_file_flag == TRUE_m10)
					CRC_validate_m10(fps->raw_data + UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10, fps->file_length - UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10, uh->body_CRC);
				break;
			case RECORD_DATA_FILE_TYPE_CODE_m10:
				validate_record_data_CRCs_m10((RECORD_HEADER_m10 *)data_ptr, number_of_items);
				break;
			case RECORD_INDICES_FILE_TYPE_CODE_m10:
				if (full_file_flag == TRUE_m10)
					CRC_validate_m10(fps->raw_data + UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10, fps->file_length - UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10, uh->body_CRC);
				break;
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
			case VIDEO_METADATA_FILE_TYPE_CODE_m10:
				if (full_file_flag == TRUE_m10)
					CRC_validate_m10(fps->raw_data + UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10, fps->file_length - UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10, uh->body_CRC);
				break;
		}
	}
	
	// decrypt
	switch (fps->universal_header->type_code) {
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
			fps->cps->block_header = (CMP_BLOCK_FIXED_HEADER_m10 *)data_ptr;
			readable = decrypt_time_series_data_m10(fps->cps, number_of_items);
			break;
		case RECORD_DATA_FILE_TYPE_CODE_m10:
			readable = decrypt_records_m10(fps, (RECORD_HEADER_m10 *)data_ptr, number_of_items);
			break;
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
		case VIDEO_METADATA_FILE_TYPE_CODE_m10:
			readable = decrypt_metadata_m10(fps);
			break;
		default:
			readable = TRUE_m10;  // file types without (possible) encryption
			break;
	}
	if (readable == FALSE_m10) {
		warning_message_m10("%s(): Cannot read file \"%s\"\n", __FUNCTION__, fps->full_file_name);
		if (allocated_flag == TRUE_m10)
			free_file_processing_struct_m10(fps, FALSE_m10);
		return(NULL);
	}
	
	// show
	if (globals_m10->verbose == TRUE_m10)
		show_file_processing_struct_m10(fps);
	
	return(fps);
}


SEGMENT_m10	*read_segment_m10(SEGMENT_m10 *seg, si1 *seg_dir, TIME_SLICE_m10 *slice, si1 *password, TERN_m10 read_time_series_data, TERN_m10 read_record_data)
{
	ui1	search_mode;
	si1	full_file_name[FULL_FILE_NAME_BYTES_m10];
	ui4	code;
	si4	seg_abs_start_sample_number, seg_abs_end_sample_number;
	si8	items_read, local_start_idx, local_end_idx;
	sf8	sampling_frequency;
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2;
	
	
	// allocate segment
	if (seg == NULL)
		seg = (SEGMENT_m10 *) calloc_m10((size_t)1, sizeof(SEGMENT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);

	// get segment path & name
	if (seg_dir == NULL) {
		if (*seg->path) {
			seg_dir = seg->path;
		}
		else {
			error_message_m10("%s(): no segment directory passed\n", __FUNCTION__);
			return(NULL);
		}
	}
	code = generate_MED_path_components_m10(seg_dir, seg->path, seg->name);
	
	// read whole segment
	if (slice == NULL)
		initialize_time_slice_m10(&seg->time_slice);
	else
		seg->time_slice = *slice;  // passed slice is not modified
	slice = &seg->time_slice;
	
	// set up time & generate password data
	if (globals_m10->password_data.processed == 0)
		if (set_time_and_password_data_m10(password, seg->path, NULL, NULL) == FALSE_m10)
			return(NULL);
	
	// process slice (for unoffset & relative times)
	if (slice->conditioned == FALSE_m10)
		condition_time_slice_m10(slice);
	
	// check for valid limit pair (time takes priority)
	if (slice->start_time != UUTC_NO_ENTRY_m10 && slice->end_time != UUTC_NO_ENTRY_m10) {
		search_mode = TIME_SEARCH_m10;
	}
	else if (slice->start_sample_number != SAMPLE_NUMBER_NO_ENTRY_m10 && slice->end_sample_number != SAMPLE_NUMBER_NO_ENTRY_m10) {
		search_mode = INDEX_SEARCH_m10;
	}
	else {
		error_message_m10("%s(): no valid limit pair\n", __FUNCTION__);
		return(0);
	}
		
	// read segment metadata
	switch (code) {
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			snprintf_m10(full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
			break;
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			snprintf_m10(full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, VIDEO_METADATA_FILE_TYPE_STRING_m10);
			break;
		default:
			error_message_m10("%s(): unrecognized type code in file \"%s\"\n", __FUNCTION__, seg->path);
			return(NULL);
	}
	seg->metadata_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, password, USE_GLOBAL_BEHAVIOR_m10);
	
	// read segment indices
	switch (code) {
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			snprintf_m10(full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10);
			seg->time_series_indices_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
			break;
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			snprintf_m10(full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, VIDEO_INDICES_FILE_TYPE_STRING_m10);
			seg->video_indices_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
			break;
		default:
			error_message_m10("%s(): unrecognized type code in file \"%s\"\n", __FUNCTION__, full_file_name);
			return(NULL);
	}
	
	// get local indices
	if (code == TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10) {
		
		tmd2 = &seg->metadata_fps->metadata->time_series_section_2;
		sampling_frequency = tmd2->sampling_frequency;
		seg_abs_start_sample_number = tmd2->absolute_start_sample_number;
		seg_abs_end_sample_number = (seg_abs_start_sample_number + tmd2->number_of_samples) - 1;
		
		// get local indices
		if (search_mode == INDEX_SEARCH_m10) {  // convert absolute indices to local indices
			if (slice->start_sample_number == BEGINNING_OF_SAMPLE_NUMBERS_m10)
				slice->start_sample_number = seg_abs_start_sample_number;
			if (slice->end_sample_number == END_OF_SAMPLE_NUMBERS_m10)
				slice->end_sample_number = seg_abs_end_sample_number;
			local_start_idx = slice->start_sample_number;
			local_end_idx = slice->end_sample_number;
			if (slice->start_sample_number > seg_abs_end_sample_number) {
				local_start_idx -= seg_abs_start_sample_number;
				local_end_idx -= seg_abs_start_sample_number;
			}
			if (local_end_idx >= tmd2->number_of_samples)
				local_end_idx = tmd2->number_of_samples - 1;
		}
		else {  // search_mode == TIME_SEARCH_m10, convert input times to local indices
			local_start_idx = sample_number_for_uutc_m10(SAMPLE_NUMBER_NO_ENTRY_m10, UUTC_NO_ENTRY_m10, slice->start_time, sampling_frequency, seg->time_series_indices_fps, FIND_CURRENT_m10);
			local_end_idx = sample_number_for_uutc_m10(SAMPLE_NUMBER_NO_ENTRY_m10, UUTC_NO_ENTRY_m10, slice->end_time, sampling_frequency, seg->time_series_indices_fps, FIND_CURRENT_m10);
		}
		
		slice->start_time = uutc_for_sample_number_m10(SAMPLE_NUMBER_NO_ENTRY_m10, UUTC_NO_ENTRY_m10, local_start_idx, tmd2->sampling_frequency, seg->time_series_indices_fps, FIND_START_m10);
		slice->end_time = uutc_for_sample_number_m10(SAMPLE_NUMBER_NO_ENTRY_m10, UUTC_NO_ENTRY_m10, local_end_idx, tmd2->sampling_frequency, seg->time_series_indices_fps, FIND_END_m10);
		
		// update slice
		slice->start_sample_number = seg_abs_start_sample_number + local_start_idx;
		slice->end_sample_number = seg_abs_start_sample_number + local_end_idx;
		slice->local_start_sample_number = local_start_idx;
		slice->local_end_sample_number = local_end_idx;
		slice->number_of_samples = (local_end_idx - local_start_idx) + 1;
		slice->start_segment_number = slice->end_segment_number = seg->metadata_fps->universal_header->segment_number;
	}
	
	// read segment data
	switch (code) {
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			sprintf_m10(full_file_name, "%s/%s.%s", seg->path, seg->name, TIME_SERIES_DATA_FILE_TYPE_STRING_m10);
			seg->time_series_data_fps = allocate_file_processing_struct_m10(NULL, full_file_name, TIME_SERIES_DATA_FILE_TYPE_CODE_m10, 0, NULL, 0);
			seg->time_series_data_fps->directives.close_file = FALSE_m10;
			read_file_m10(seg->time_series_data_fps, full_file_name, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
			if (read_time_series_data == TRUE_m10) {
				(void)read_time_series_data_m10(seg, local_start_idx, local_end_idx, TRUE_m10);
				FPS_close_m10(seg->time_series_data_fps);
			}
			break;
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			// video segment data are native video files
			break;
		default:
			error_message_m10("%s(): unrecognized type code in file \"%s\"\n", __FUNCTION__, seg->path);
			return(NULL);
	}
	
	// read segment record indices
	snprintf_m10(full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
	if (file_exists_m10(full_file_name) == FILE_EXISTS_m10) {
		seg->record_indices_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
		// read segment record data
		snprintf_m10(full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, RECORD_DATA_FILE_TYPE_STRING_m10);
		if (file_exists_m10(full_file_name) == FILE_EXISTS_m10) {
			if (read_record_data == TRUE_m10) {
				seg->record_data_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
			}
			else {
				seg->record_data_fps = allocate_file_processing_struct_m10(NULL, full_file_name, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, NULL, 0);
				seg->record_data_fps->directives.close_file = FALSE_m10;
				read_file_m10(seg->record_data_fps, full_file_name, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
			}
		}
		else {
			warning_message_m10("%s(): Segment record data file does not exist (\"%s\"), but indices file does\n", __FUNCTION__, full_file_name);
		}
	}
	
	return(seg);
}


SESSION_m10	*read_session_m10(si1 *sess_dir, si1 **chan_list, si4 n_chans, TIME_SLICE_m10 *slice, si1 *password, TERN_m10 read_time_series_data, TERN_m10 read_record_data)
{
	si1                             full_file_name[FULL_FILE_NAME_BYTES_m10], sess_path[FULL_FILE_NAME_BYTES_m10];
	si1                             tmp_str[FULL_FILE_NAME_BYTES_m10];
	si1                             num_str[FILE_NUMBERING_DIGITS_m10 + 1], **seg_rec_file_names;
	si1				**ts_chan_list = NULL, **vid_chan_list = NULL;
	ui4                             type_code;
	si4                             n_ts_chans, n_vid_chans, n_segs;
	si4                             i, j, k, num_seg_rec_files;
	si8                             items_read;
	TIME_SLICE_m10			*chan_slice;
	SESSION_m10			*sess;
	CHANNEL_m10			*chan;
	FILE_PROCESSING_STRUCT_m10	*gen_fps;
	
	
	// allocate session
	sess = (SESSION_m10 *) calloc_m10((size_t)1, sizeof(SESSION_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// read whole session
	if (slice == NULL)
		slice = initialize_time_slice_m10(&sess->time_slice);
	else
		sess->time_slice = *slice;  // passed slice is not modified
	slice = &sess->time_slice;
	
	// expand channel list
	chan_list = generate_file_list_m10(chan_list, &n_chans, sess_dir, NULL, "?icd", PP_FULL_PATH_m10, FALSE_m10);  // extension could be more specific ("[tv]icd") in MacOS & Linux, but not Windows
	if (n_chans == 0) {
		error_message_m10("%s(): no matching MED channels\n", __FUNCTION__);
		return(NULL);
	}
	
	// check that all files are MED channels in the same MED session directory
	// TO DO: check that they have the same session UIDs, & not require they be in the same directory
	extract_path_parts_m10(chan_list[0], sess_path, NULL, NULL);
	type_code = MED_type_code_from_string_m10(sess_path);
	if (type_code != SESSION_DIRECTORY_TYPE_CODE_m10) {
		error_message_m10("%s(): files must be in a MED session directory\n", __FUNCTION__);
		return(NULL);
	}
	
	n_ts_chans = n_vid_chans = 0;
	for (i = 0; i < n_chans; ++i) {
		extract_path_parts_m10(chan_list[i], tmp_str, NULL, NULL);
		if (strcmp(sess_path, tmp_str) != 0) {
			error_message_m10("%s(): channel files must all be in the same directory\n", __FUNCTION__);
			return(NULL);
		}
		type_code = MED_type_code_from_string_m10(chan_list[i]);
		switch (type_code) {
			case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m10:
				++n_ts_chans;
				break;
			case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m10:
				++n_vid_chans;
				break;
			default:
				error_message_m10("%s(): channel files must all be MED channel directories\n", __FUNCTION__);
				return(NULL);
		}
	}
	
	// divide channel lists
	if (n_ts_chans)
		ts_chan_list = (si1 **) calloc_2D_m10((size_t) n_ts_chans, FULL_FILE_NAME_BYTES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	if (n_vid_chans)
		vid_chan_list = (si1 **) calloc_2D_m10((size_t) n_vid_chans, FULL_FILE_NAME_BYTES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	for (i = j = k = 0; i < n_chans; ++i) {
		type_code = MED_type_code_from_string_m10(chan_list[i]);
		switch (type_code) {
			case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m10:
				strcpy(ts_chan_list[j++], chan_list[i]);
				break;
			case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m10:
				strcpy(vid_chan_list[k++], chan_list[i]);
				break;
		}
	}
	free((void *) chan_list);
	
	// password
	generate_MED_path_components_m10(sess_path, sess->path, sess->name);
	if (globals_m10->password_data.processed == 0) {
		if (set_time_and_password_data_m10(password, sess->path, NULL, NULL) == FALSE_m10) {
			error_message_m10("%s(): error setting time or processing password\n", __FUNCTION__);
			return(NULL);
		}
	}
	
	if (slice->conditioned == FALSE_m10)
		condition_time_slice_m10(slice);
	
	// get segment range
	if (n_ts_chans) {
		n_segs = get_segment_range_m10(ts_chan_list, n_ts_chans, slice);
	}
	else {  // n_vid_chans != 0
		if (slice->start_time != UUTC_NO_ENTRY_m10 && slice->end_time != UUTC_NO_ENTRY_m10) {
			n_segs = get_segment_range_m10(vid_chan_list, n_vid_chans, slice);
		}
		else {
			error_message_m10("%s(): cannot search video channels by indices (frames) for segments at this time\n", __FUNCTION__);
			return(NULL);
		}
	}
	
	sess->number_of_time_series_channels = n_ts_chans;
	sess->number_of_video_channels = n_vid_chans;
	sess->number_of_segments = n_segs;
	
	// read time series channels
	if (n_ts_chans) {
		sess->time_series_channels = (CHANNEL_m10 **) calloc_2D_m10((size_t) n_ts_chans, 1, sizeof(CHANNEL_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		for (i = 0; i < n_ts_chans; ++i) {
			chan = sess->time_series_channels[i];
			strcpy(chan->path, ts_chan_list[i]);
			if (read_channel_m10(chan, chan->path, slice, NULL, read_time_series_data, read_record_data) == NULL)
				return(NULL);
		}
		
		// fill in session time slice
		*slice = sess->time_series_channels[0]->time_slice;
		for (i = 1; i < n_ts_chans; ++i) {
			chan_slice = &sess->time_series_channels[i]->time_slice;
			if (chan_slice->start_sample_number != slice->start_sample_number || chan_slice->end_sample_number != slice->end_sample_number) {
				slice->start_sample_number = slice->end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m10;
				slice->number_of_samples = NUMBER_OF_SAMPLES_NO_ENTRY_m10;
				break;
			}
		}
		
		// create session level metadata FPS
		sprintf_m10(full_file_name, "%s/%s.%s", sess->path, sess->name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
		chan = sess->time_series_channels[0];
		sess->time_series_metadata_fps = allocate_file_processing_struct_m10(NULL, full_file_name, TIME_SERIES_METADATA_FILE_TYPE_CODE_m10, METADATA_BYTES_m10, chan->metadata_fps, METADATA_BYTES_m10);
		sess->time_series_metadata_fps->fd = FPS_FD_EPHEMERAL_m10;
		sess->time_series_metadata_fps->universal_header->number_of_entries = 0;
		sess->time_series_metadata_fps->universal_header->maximum_entry_size = 0;
		
		for (i = 1; i < n_ts_chans; ++i) {
			chan = sess->time_series_channels[i];
			merge_universal_headers_m10(sess->time_series_metadata_fps, chan->metadata_fps, NULL);
			merge_metadata_m10(sess->time_series_metadata_fps, chan->metadata_fps, NULL);
		}
		free((void *) ts_chan_list);
	}

	// read video channels
	if (n_vid_chans) {
		sess->video_channels = (CHANNEL_m10 **) calloc_2D_m10((size_t) n_vid_chans, 1, sizeof(CHANNEL_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		sess->video_metadata_fps->fd = FPS_FD_EPHEMERAL_m10;
		for (i = 0; i < n_vid_chans; ++i) {
			chan = sess->video_channels[i];
			strncpy_m10(chan->path, vid_chan_list[i], FULL_FILE_NAME_BYTES_m10);
			if (read_channel_m10(chan, chan->path, slice, NULL, read_time_series_data, read_record_data) == NULL)
				return(NULL);
		}
		
		// create session level metadata FPS
		sprintf_m10(full_file_name, "%s/%s.%s", sess->path, sess->name, VIDEO_METADATA_FILE_TYPE_STRING_m10);
		chan = sess->video_channels[0];
		sess->video_metadata_fps = allocate_file_processing_struct_m10(NULL, full_file_name, VIDEO_METADATA_FILE_TYPE_CODE_m10, METADATA_BYTES_m10, chan->metadata_fps, METADATA_BYTES_m10);
		sess->video_metadata_fps->fd = FPS_FD_EPHEMERAL_m10;
		sess->video_metadata_fps->universal_header->number_of_entries = 0;
		sess->video_metadata_fps->universal_header->maximum_entry_size = 0;
		for (i = 1; i < n_vid_chans; ++i) {
			chan = sess->video_channels[i];
			merge_universal_headers_m10(sess->video_metadata_fps, chan->metadata_fps, NULL);
			merge_metadata_m10(sess->video_metadata_fps, chan->metadata_fps, NULL);
		}
		free((void *) vid_chan_list);
	}
	
	// if session has no channels, it can still have records, but it's probably a bad session
	if (n_ts_chans == 0 && n_vid_chans == 0)
		warning_message_m10("%s(): session contains no channels\n", __FUNCTION__);
	
	// if session has no segments, it can still have records, but it's probably a bad session
	if (n_segs == 0)
		warning_message_m10("%s(): session contains no segments\n", __FUNCTION__);
	
	// read session record indices (if present)
	sprintf_m10(full_file_name, "%s/%s.%s", sess->path, sess->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
	if (file_exists_m10(full_file_name) == FILE_EXISTS_m10) {
		sess->record_indices_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
		
		// read record data files (if present)
		sprintf_m10(full_file_name, "%s/%s.%s", sess->path, sess->name, RECORD_DATA_FILE_TYPE_STRING_m10);
		if (file_exists_m10(full_file_name) == FILE_EXISTS_m10) {
			if (read_record_data == TRUE_m10) {
				sess->record_data_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
			}
			else {
				sess->record_data_fps = allocate_file_processing_struct_m10(NULL, full_file_name, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, NULL, 0);
				sess->record_data_fps->directives.close_file = FALSE_m10;
				read_file_m10(sess->record_data_fps, NULL, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
			}
			if (n_ts_chans) {
				merge_universal_headers_m10(sess->time_series_metadata_fps, sess->record_data_fps, NULL);
			}
			if (n_vid_chans)
				merge_universal_headers_m10(sess->video_metadata_fps, sess->record_data_fps, NULL);
		}
		else {
			warning_message_m10("%s(): Session record data file does not exist (\"%s\"), but indices file does\n", __FUNCTION__, full_file_name);
		}
	}
	
	// check if segmented session records present
	sprintf_m10(full_file_name, "%s/%s.%s", sess->path, sess->name, RECORD_DIRECTORY_TYPE_STRING_m10);
	num_seg_rec_files = 0;
	seg_rec_file_names = generate_file_list_m10(NULL, &num_seg_rec_files, full_file_name, NULL, RECORD_INDICES_FILE_TYPE_STRING_m10, PP_FULL_PATH_m10, FALSE_m10);
	if (seg_rec_file_names != NULL)
		free((void *) *seg_rec_file_names);  // Not using the names, because not every segment must have records,
	// just seeing if there are ANY segmented session records.
	// Use indices of segments instead of looping over names.
	// read segmented session records
	if (num_seg_rec_files) {
		sess->segmented_record_indices_fps = (FILE_PROCESSING_STRUCT_m10 **) calloc_2D_m10((size_t) sess->number_of_segments, 1, sizeof(FILE_PROCESSING_STRUCT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		sess->segmented_record_data_fps = (FILE_PROCESSING_STRUCT_m10 **) calloc_2D_m10((size_t) sess->number_of_segments, 1, sizeof(FILE_PROCESSING_STRUCT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		for (i = 0; i < sess->number_of_segments; ++i) {
			
			// read segmented record indices file (if present)
			gen_fps = sess->segmented_record_indices_fps[i];
			numerical_fixed_width_string_m10(num_str, FILE_NUMBERING_DIGITS_m10, i + slice->start_segment_number);
			sprintf_m10(gen_fps->full_file_name, "%s/%s.%s/%s_s%s.%s", sess->path, sess->name, RECORD_DIRECTORY_TYPE_STRING_m10, sess->name, num_str, RECORD_INDICES_FILE_TYPE_STRING_m10);
			if (file_exists_m10(gen_fps->full_file_name) == FILE_EXISTS_m10) {
				allocate_file_processing_struct_m10(gen_fps, NULL, RECORD_INDICES_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, NULL, 0);
				read_file_m10(gen_fps, NULL, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
				// read segmented record data file (if present)
				gen_fps = sess->segmented_record_data_fps[i];
				sprintf_m10(gen_fps->full_file_name, "%s/%s.%s/%s_s%s.%s", sess->path, sess->name, RECORD_DIRECTORY_TYPE_STRING_m10, sess->name, num_str, RECORD_DATA_FILE_TYPE_STRING_m10);
				if (file_exists_m10(gen_fps->full_file_name) == FILE_EXISTS_m10) {
					allocate_file_processing_struct_m10(gen_fps, NULL, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, NULL, 0);
					if (read_record_data == TRUE_m10) {
						read_file_m10(gen_fps, NULL, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
					}
					else {
						gen_fps->directives.close_file = FALSE_m10;
						read_file_m10(gen_fps, NULL, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
					}
					if (sess->number_of_time_series_channels) {
						merge_universal_headers_m10(sess->time_series_metadata_fps, gen_fps, NULL);
					}
					if (sess->number_of_video_channels)
						merge_universal_headers_m10(sess->video_metadata_fps, gen_fps, NULL);
				}
				else {
					warning_message_m10("%s(): Session segmented record data file does not exist (\"%s\"), but indices file does\n", __FUNCTION__, full_file_name);
				}
			}
		}
		
		// make links to segment FPS's
		for (i = 0; i < sess->number_of_time_series_channels; ++i) {
			chan = sess->time_series_channels[i];
			for (j = 0; j < sess->number_of_segments; ++j) {
				chan->segments[j]->segmented_session_record_indices_fps = sess->segmented_record_indices_fps[j];
				chan->segments[j]->segmented_session_record_data_fps = sess->segmented_record_data_fps[j];
			}
		}
		for (i = 0; i < sess->number_of_video_channels; ++i) {
			chan = sess->video_channels[i];
			for (j = 0; j < sess->number_of_segments; ++j) {
				chan->segments[j]->segmented_session_record_indices_fps = sess->segmented_record_indices_fps[j];
				chan->segments[j]->segmented_session_record_data_fps = sess->segmented_record_data_fps[j];
			}
		}
	}
	
	// fix session metadata FPS (from merge functions)
	if (sess->number_of_time_series_channels > 0) {
		sess->time_series_metadata_fps->universal_header->type_code = TIME_SERIES_METADATA_FILE_TYPE_CODE_m10;
		sess->time_series_metadata_fps->universal_header->segment_number = UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m10;
		sess->time_series_metadata_fps->universal_header->channel_UID = UID_NO_ENTRY_m10;
		sess->time_series_metadata_fps->universal_header->segment_UID = UID_NO_ENTRY_m10;
	}
	if (sess->number_of_video_channels > 0) {
		sess->video_metadata_fps->universal_header->type_code = VIDEO_METADATA_FILE_TYPE_CODE_m10;
		sess->video_metadata_fps->universal_header->segment_number = UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m10;
		sess->video_metadata_fps->universal_header->channel_UID = UID_NO_ENTRY_m10;
		sess->video_metadata_fps->universal_header->segment_UID = UID_NO_ENTRY_m10;
	}
	
	if (globals_m10->verbose == TRUE_m10) {
		if (sess->number_of_time_series_channels > 0) {
			printf_m10("--------- Session Time Series Universal Header ---------\n");
			show_universal_header_m10(sess->time_series_metadata_fps, NULL);
			printf_m10("------------ Session Time Series Metadata --------------\n");
			show_metadata_m10(sess->time_series_metadata_fps, NULL);
		}
		if (sess->number_of_video_channels > 0) {
			printf_m10("------------ Session Video Universal Header ------------\n");
			show_universal_header_m10(sess->video_metadata_fps, NULL);
			printf_m10("--------------- Session Video Metadata -----------------\n");
			show_metadata_m10(sess->video_metadata_fps, NULL);
		}
	}

	return(sess);
}


si8     read_time_series_data_m10(SEGMENT_m10 *seg, si8 local_start_idx, si8 local_end_idx, TERN_m10 alloc_cps)
{
	si8                                     i, n_ts_inds, n_samps, samp_num;
	si8                                     offset_pts, start_block, end_block, compressed_data_bytes;
	FILE_PROCESSING_STRUCT_m10		*tsd_fps, *tsi_fps;
	TIME_SERIES_INDEX_m10			*tsi;
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2;
	CMP_PROCESSING_STRUCT_m10		*cps;
	CMP_BLOCK_FIXED_HEADER_m10		*bh;
	
	
	// local_start_idx and local_end_idx are segment relative
	
	if (seg == NULL) {
		error_message_m10("%s(): SEGMENT_m10 structure is NULL\n", __FUNCTION__);
		return(-1);
	}
	
	tsd_fps = seg->time_series_data_fps;
	tsi_fps = seg->time_series_indices_fps;
	
	if (tsd_fps == NULL) {
		error_message_m10("%s(): time series data FILE_PROCESSING_STRUCT_m10 is NULL\n", __FUNCTION__);
		return(-1);
	}
	if (tsi_fps == NULL) {
		error_message_m10("%s(): time series indices FILE_PROCESSING_STRUCT_m10 is NULL\n", __FUNCTION__);
		return(-1);
	}
	if (tsd_fps->cps == NULL && alloc_cps == FALSE_m10) {
		error_message_m10("%s(): CMP_PROCESSING_STRUCT_m10 is NULL\n", __FUNCTION__);
		return(-1);
	}
	
	n_ts_inds = tsi_fps->universal_header->number_of_entries;
	tsi = tsi_fps->time_series_indices;
	tmd2 = &seg->metadata_fps->metadata->time_series_section_2;
	
	// find start and end blocks
	for (i = 1; i < n_ts_inds; ++i)
		if (tsi[i].start_sample_number > local_start_idx)
			break;
	start_block = i - 1;
	for (; i < n_ts_inds; ++i)
		if (tsi[i].start_sample_number > local_end_idx)
			break;
	if (i == n_ts_inds) {
		end_block = i - 1;
		local_end_idx = tsi[end_block].start_sample_number - 1;
	}
	else {
		end_block = i;
	}
	
	// allocate cps
	samp_num = tsi[start_block].start_sample_number;
	n_samps = tsi[end_block].start_sample_number;
	n_samps -= samp_num;
	compressed_data_bytes = REMOVE_DISCONTINUITY_m10(tsi[end_block].file_offset) - REMOVE_DISCONTINUITY_m10(tsi[start_block].file_offset);
	if (alloc_cps == TRUE_m10) {
		force_behavior_m10(globals_m10->behavior_on_fail | SUPPRESS_WARNING_OUTPUT_m10);
		CMP_free_processing_struct_m10(tsd_fps->cps);
		force_behavior_m10(RESTORE_BEHAVIOR_m10);
		tsd_fps->cps = CMP_allocate_processing_struct_m10(NULL, CMP_DECOMPRESSION_MODE_m10, n_samps, compressed_data_bytes, CMP_MAX_DIFFERENCE_BYTES_m10(tmd2->maximum_block_samples), tmd2->maximum_block_samples, NULL, NULL);
	}
	else if (tsd_fps->cps == NULL) {
		error_message_m10("%s(): no CMP_PROCESSING_STRUCT allocated\n", __FUNCTION__);
		return(-1);
	}
	cps = tsd_fps->cps;
	
	// read in compressed data
	fseek_m10(tsd_fps->fp, REMOVE_DISCONTINUITY_m10(tsi[start_block].file_offset), SEEK_SET, tsd_fps->full_file_name, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	FPS_read_m10(tsd_fps, compressed_data_bytes, cps->compressed_data, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// decompress first block & discard any unrequested initial points
	offset_pts = local_start_idx - tsi[start_block].start_sample_number;
	bh = cps->block_header;
	
	if (offset_pts) {
		CMP_decode_m10(cps);
		memmove(cps->decompressed_ptr, cps->decompressed_ptr + offset_pts, (bh->number_of_samples - offset_pts) * sizeof(si4));
		cps->decompressed_ptr += (bh->number_of_samples - offset_pts);
		bh = CMP_update_CPS_pointers_m10(cps, CMP_UPDATE_BLOCK_HEADER_PTR_m10);
		++start_block;
	}
	
	// loop over rest of blocks
	for (i = start_block; i < end_block; ++i) {
		CMP_decode_m10(cps);
		bh = CMP_update_CPS_pointers_m10(cps, CMP_UPDATE_BLOCK_HEADER_PTR_m10 | CMP_UPDATE_DECOMPRESSED_PTR_m10);
	}
	
	n_samps = (local_end_idx - local_start_idx) + 1;
	
	return(n_samps);
}


void	**realloc_2D_m10(void **curr_ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	si8	i;
	void	**new_ptr;
	size_t	least_dim1, least_dim2;
	
	
	// Returns pointer to a reallocated 2 dimensional array of new_dim1 by new_dim2 elements of size el_size (new unused elements are zeroed)
	// ptr[0] points to a one dimensional array of size (dim1 * dim2)
	// The whole block can be freed with free(ptr)
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if (new_dim1 == 0 || new_dim2 == 0 || el_size == 0) {
		if (curr_ptr != NULL)
			free_2D_m10((void **) curr_ptr, curr_dim1, function, line);
		return((void **) NULL);
	}
	
	if (curr_ptr == NULL) {
		error_message_m10("%s(): attempting to re-allocate NULL pointer (called from function %s(), line %d)\n", __FUNCTION__, function, line);
		return(NULL);
	}
	
	if (new_dim1 < curr_dim1)
		warning_message_m10("%s(): re-allocating first dimension to smaller size (called from function %s(), line %d)\n", __FUNCTION__, function, line);
	if (new_dim2 < curr_dim2)
		warning_message_m10("%s(): re-allocating second dimension to smaller size (called from function %s(), line %d)\n", __FUNCTION__, function, line);
	
	new_ptr = calloc_2D_m10(new_dim1, new_dim2, el_size, function, line, behavior_on_fail);
	
	least_dim1 = (curr_dim1 <= new_dim1) ? curr_dim1 : new_dim1;
	least_dim2 = (curr_dim2 <= new_dim2) ? curr_dim2 : new_dim2;
	for (i = 0; i < least_dim1; ++i)
		memcpy((void *) new_ptr[i], curr_ptr[i], (size_t)(least_dim2 * el_size));
	
	free_2D_m10((void **) curr_ptr, curr_dim1, function, line);
	
	return((void **) new_ptr);
}
		
		
si4	reallocate_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps, si8 new_raw_data_bytes)
{
	void	*data_ptr;
	
	
	// reallocate
	fps->raw_data = (ui1 *) realloc_m10((void *) fps->raw_data, (size_t) new_raw_data_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// zero additional memory (realloc() copies existing memory if necessary, but does not zero additional memory allocated)
	if (new_raw_data_bytes > fps->raw_data_bytes)
		memset(fps->raw_data + fps->raw_data_bytes, 0, new_raw_data_bytes - fps->raw_data_bytes);
	fps->raw_data_bytes = new_raw_data_bytes;
	
	// reset universal header pointer
	fps->universal_header = (UNIVERSAL_HEADER_m10 *)fps->raw_data; // all files start with universal header
	
	// set appropriate pointers
	data_ptr = (void *) (fps->raw_data + UNIVERSAL_HEADER_BYTES_m10);
	switch (fps->universal_header->type_code) {
		case NO_TYPE_CODE_m10:
			break;
		case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
			fps->time_series_indices = (TIME_SERIES_INDEX_m10 *) data_ptr;
			break;
		case VIDEO_INDICES_FILE_TYPE_CODE_m10:
			fps->video_indices = (VIDEO_INDEX_m10 *) data_ptr;
			break;
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
		case VIDEO_METADATA_FILE_TYPE_CODE_m10:
			fps->metadata = (METADATA_m10 *) data_ptr;
			break;
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
			// CMP_PROCESSING_STRUCT allocation done seperately with CMP_allocate_processing_struct()
			break;
		case RECORD_DATA_FILE_TYPE_CODE_m10:
			fps->records = (ui1 *) data_ptr;
			break;
		case RECORD_INDICES_FILE_TYPE_CODE_m10:
			fps->record_indices = (RECORD_INDEX_m10 *)data_ptr;
			break;
		default:
			error_message_m10("%s(): unrecognized file type code (code == 0x%08x)\n", __FUNCTION__, fps->universal_header->type_code);
			return(-1);
	}
	
	return(0);
}


TERN_m10    recover_passwords_m10(si1 *L3_password, UNIVERSAL_HEADER_m10 *universal_header)
{
	ui1     hash[SHA_HASH_BYTES_m10], L3_hash[SHA_HASH_BYTES_m10];
	si1     L3_password_bytes[PASSWORD_BYTES_m10], hex_str[HEX_STRING_BYTES_m10(PASSWORD_BYTES_m10)];
	si1     putative_L1_password_bytes[PASSWORD_BYTES_m10], putative_L2_password_bytes[PASSWORD_BYTES_m10];
	si4     i;
	
	
	if (check_password_m10(L3_password) == FALSE_m10)
		return(FALSE_m10);
	
	// get terminal bytes
	extract_terminal_password_bytes_m10(L3_password, L3_password_bytes);
	
	// get level 3 password hash
	SHA_hash_m10((ui1 *)L3_password_bytes, PASSWORD_BYTES_m10, L3_hash);  // generate SHA-256 hash of level 3 password bytes
	
	// check for level 1 access
	for (i = 0; i < PASSWORD_BYTES_m10; ++i)  // xor with level 3 password validation field
		putative_L1_password_bytes[i] = L3_hash[i] ^ universal_header->level_3_password_validation_field[i];
	
	SHA_hash_m10((ui1 *) putative_L1_password_bytes, PASSWORD_BYTES_m10, hash); // generate SHA-256 hash of putative level 1 password
	
	for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i)  // compare with stored level 1 hash
		if (hash[i] != universal_header->level_1_password_validation_field[i])
			break;
	if (i == PASSWORD_VALIDATION_FIELD_BYTES_m10) {  // Level 1 password recovered
		generate_hex_string_m10((ui1 *)putative_L1_password_bytes, PASSWORD_BYTES_m10, hex_str);
		message_m10("Level 1 password (bytes): '%s' (%s)", putative_L1_password_bytes, hex_str);
		return(TRUE_m10);
	}
	
	// invalid for level 1 (alone) => check if level 2 password
	memcpy(putative_L2_password_bytes, putative_L1_password_bytes, PASSWORD_BYTES_m10);
	for (i = 0; i < PASSWORD_BYTES_m10; ++i)  // xor with level 2 password validation field
		putative_L1_password_bytes[i] = hash[i] ^ universal_header->level_2_password_validation_field[i];
	
	SHA_hash_m10((ui1 *) putative_L1_password_bytes, PASSWORD_BYTES_m10, hash); // generate SHA-256 hash of putative level 1 password
	
	for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i)  // compare with stored level 1 hash
		if (hash[i] != universal_header->level_1_password_validation_field[i])
			break;
	
	if (i == PASSWORD_VALIDATION_FIELD_BYTES_m10) {  // Level 2 password valid
		generate_hex_string_m10((ui1 *)putative_L1_password_bytes, PASSWORD_BYTES_m10, hex_str);
		message_m10("Level 1 password (bytes): '%s' (%s)", putative_L1_password_bytes, hex_str);
		generate_hex_string_m10((ui1 *)putative_L2_password_bytes, PASSWORD_BYTES_m10, hex_str);
		message_m10("Level 2 password (bytes): '%s' (%s)", putative_L2_password_bytes, hex_str);
	}
	else {
		warning_message_m10("%s(): the passed password is not valid for Level 3 access\n", __FUNCTION__, __LINE__);
		return(FALSE_m10);
	}
	
	return(TRUE_m10);
}


inline void	remove_recording_time_offset_m10(si8 *time)
{
	if (*time != UUTC_NO_ENTRY_m10)
		*time += globals_m10->recording_time_offset;
	
	return;
}

void    reset_metadata_for_update_m10(FILE_PROCESSING_STRUCT_m10 *fps)
{
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2;
	VIDEO_METADATA_SECTION_2_m10		*vmd2;
	
	
	// section 2 fields
	switch (fps->universal_header->type_code) {
		case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
			tmd2 = &fps->metadata->time_series_section_2;
			tmd2->number_of_samples = 0;
			tmd2->number_of_blocks = 0;
			tmd2->maximum_block_bytes = 0;
			tmd2->maximum_block_samples = 0;
			tmd2->maximum_block_difference_bytes = 0;
			tmd2->maximum_block_duration = 0;
			tmd2->number_of_discontinuities = 0;
			tmd2->maximum_contiguous_blocks = 0;
			tmd2->maximum_contiguous_block_bytes = 0;
			tmd2->maximum_contiguous_samples = 0;
			break;
		case VIDEO_METADATA_FILE_TYPE_CODE_m10:
			vmd2 = &fps->metadata->video_section_2;
			vmd2->number_of_clips = 0;
			vmd2->maximum_clip_bytes = 0;
			vmd2->number_of_video_files = 0;
			break;
		default:
			error_message_m10("%s(): Unrecognized metadata type in file \"%s\"\n", __FUNCTION__, fps->full_file_name);
			break;
	}
	
	return;
}


si8     sample_number_for_uutc_m10(si8 ref_sample_number, si8 ref_uutc, si8 target_uutc, sf8 sampling_frequency, FILE_PROCESSING_STRUCT_m10 *time_series_indices_fps, ui1 mode)
{
	si8                     samp_num, n_inds, i, absolute_numbering_offset, tmp_si8;
	sf8                     tmp_sf8;
	TIME_SERIES_INDEX_m10	*tsi;
	
	
	// sample_number_for_uutc_m10(ref_sample_number, ref_uutc, target_uutc, sampling_frequency, NULL, mode)
	// returns sample number extrapolated from ref_sample_number
	
	// sample_number_for_uutc_m10(SAMPLE_NUMBER_NO_ENTRY_m10, UUTC_NO_ENTRY_m10, target_uutc, sampling_frequency, tsi_fps, mode)
	// returns sample number extrapolated from closest time series index in local (segment-relative) sample numbering
	
	// sample_number_for_uutc_m10(ref_sample_number, UUTC_NO_ENTRY_m10, target_uutc, sampling_frequency, tsi_fps, mode)
	// in this case ref_sample_number is the segment absolute start sample number
	// returns sample number extrapolated from closest time series index in absolute (channel-relative) sample numbering
	
	// mode FIND_CURRENT_m10 (default): sample period within which the target_uutc falls
	// mode FIND_CLOSEST_m10: sample number closest to the target_uutc
	// mode FIND_NEXT_m10: sample number following the sample period within which the target_uutc falls (== FIND_CURRENT_m10 + 1)
	
	absolute_numbering_offset = 0;
	if (time_series_indices_fps != NULL) {
		tsi = time_series_indices_fps->time_series_indices;
		n_inds = time_series_indices_fps->universal_header->number_of_entries;
		
		// ref_sample_number used to pass absolute_start_sample_number for the segment, if tsi passed
		if (ref_sample_number != SAMPLE_NUMBER_NO_ENTRY_m10)
			absolute_numbering_offset = ref_sample_number;
		
		// use time series indices to get ref_sample_number & ref_uutc instead if just extrapolating (accounts for discontinuities)
		for (i = 0; i < n_inds; ++i)
			if (tsi[i].start_time > target_uutc)
				break;
		
		if (i == 0)  // target time earlier than segment start => return segment start sample
			return(absolute_numbering_offset);
		
		if (i == n_inds)  // target time later than segment end => return segment end sample
			return(tsi[i - 1].start_sample_number + absolute_numbering_offset - 1);
		
		ref_uutc = tsi[i - 1].start_time;
		ref_sample_number = tsi[i - 1].start_sample_number;
		
		// acquisition sampling frequency can vary a little => this is slightly more accurate
		if (tsi[i].file_offset > 0) {  // don't do if discontinuity
			sampling_frequency = (sf8)(tsi[i].start_sample_number - ref_sample_number);
			sampling_frequency /= ((sf8)(tsi[i].start_time - ref_uutc) / (sf8)1e6);
		}
	}
	
	tmp_sf8 = ((sf8)(target_uutc - ref_uutc) / (sf8)1e6) * sampling_frequency;
	switch (mode) {
		case FIND_CLOSEST_m10:
			tmp_si8 = (si8)(tmp_sf8 + (sf8)0.5);
			break;
		case FIND_NEXT_m10:
			tmp_si8 = (si8)tmp_sf8 + 1;
			break;
		case FIND_CURRENT_m10:
		default:
			tmp_si8 = (si8)tmp_sf8;
			break;
	}
	samp_num = ref_sample_number + tmp_si8 + absolute_numbering_offset;
	
	return(samp_num);
}


TERN_m10        search_segment_metadata_m10(si1 *MED_dir, TIME_SLICE_m10 *slice)
{
	ui1                                     search_mode;
	si1					**seg_list, seg_name[BASE_FILE_NAME_BYTES_m10], tmp_str[FULL_FILE_NAME_BYTES_m10];
	si4                                     n_segs, start_seg_idx, end_seg_idx;
	si8                                     i, items_read, start_seg_start_idx, end_seg_start_idx, absolute_end_sample_number;
	sf8                                     sampling_frequency;
	FILE_PROCESSING_STRUCT_m10		*md_fps = NULL, *tsi_fps = NULL;
	UNIVERSAL_HEADER_m10			*uh = NULL;
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2 = NULL;
	
	
	if (slice == NULL)
		error_message_m10("%s(): NULL slice pointer\n", __FUNCTION__);
	else
		slice->start_segment_number = slice->end_segment_number = SEGMENT_NUMBER_NO_ENTRY_m10;
	
	// check for valid limit pair (time takes priority)
	if (slice->start_time != UUTC_NO_ENTRY_m10 && slice->end_time != UUTC_NO_ENTRY_m10) {
		search_mode = TIME_SEARCH_m10;
	}
	else if (slice->start_sample_number != SAMPLE_NUMBER_NO_ENTRY_m10 && slice->end_sample_number != SAMPLE_NUMBER_NO_ENTRY_m10) {
		search_mode = INDEX_SEARCH_m10;
	}
	else {
		error_message_m10("%s(): no valid limit pair\n", __FUNCTION__);
		return(FALSE_m10);
	}
	
	// get segment list
	n_segs = 0;
	seg_list = generate_file_list_m10(NULL, &n_segs, MED_dir, "*", "tisd", PP_FULL_PATH_m10, FALSE_m10);
	if (n_segs == 0) {
		error_message_m10("%s(): Cannot find segment metadata\n", __FUNCTION__);
		return(FALSE_m10);
	}
	
	// find start segment
	slice->start_segment_number = 0;
	for (i = 0; i < n_segs; ++i) {
		extract_path_parts_m10(seg_list[i], NULL, seg_name, NULL);
		sprintf_m10(tmp_str, "%s/%s.%s", seg_list[i], seg_name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
		md_fps = NULL;
		if (file_exists_m10(tmp_str) == FILE_EXISTS_m10)
			md_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
		if (md_fps == NULL) {
			free((void *) *seg_list);
			error_message_m10("%s(): Cannot find segment metadata\n", __FUNCTION__);
			return(FALSE_m10);
		}
		tmd2 = &md_fps->metadata->time_series_section_2;
		uh = md_fps->universal_header;
		
		if (search_mode == TIME_SEARCH_m10) {
			if (slice->start_time <= uh->file_end_time) {
				slice->start_segment_number = uh->segment_number;
				if (slice->start_time < uh->file_start_time)
					slice->start_time = uh->file_start_time;
				break;
			}
		}
		else {  // search_mode == INDEX_SEARCH_m10
			absolute_end_sample_number = tmd2->absolute_start_sample_number + tmd2->number_of_samples - 1;
			if (slice->start_sample_number <= absolute_end_sample_number) {
				slice->start_segment_number = uh->segment_number;
				if (slice->start_sample_number < tmd2->absolute_start_sample_number)
					slice->start_sample_number = tmd2->absolute_start_sample_number;
				break;
			}
		}
		free_file_processing_struct_m10(md_fps, FALSE_m10);
	}
	if (i == n_segs) {
		free((void *) *seg_list);
		error_message_m10("%s(): Start index exceeds session indices\n", __FUNCTION__);
		return(FALSE_m10);
	}
	start_seg_idx = i;
	start_seg_start_idx = tmd2->absolute_start_sample_number;
	free_file_processing_struct_m10(md_fps, FALSE_m10);
	
	// find end segment
	slice->end_segment_number = slice->start_segment_number;
	for (; i < n_segs; ++i) {
		extract_path_parts_m10(seg_list[i], NULL, seg_name, NULL);
		sprintf_m10(tmp_str, "%s/%s.%s", seg_list[i], seg_name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
		md_fps = NULL;
		if (file_exists_m10(tmp_str) == FILE_EXISTS_m10)
			md_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
		if (md_fps == NULL) {
			free((void *) *seg_list);
			error_message_m10("%s(): Cannot find segment metadata\n", __FUNCTION__);
			return(FALSE_m10);
		}
		
		tmd2 = &md_fps->metadata->time_series_section_2;
		end_seg_start_idx = tmd2->absolute_start_sample_number;
		sampling_frequency = tmd2->sampling_frequency;
		uh = md_fps->universal_header;
		if (search_mode == TIME_SEARCH_m10) {
			if (slice->end_time <= uh->file_end_time) {
				slice->end_segment_number = uh->segment_number;
				break;
			}
		}
		else {  // search_mode == INDEX_SEARCH_m10
			absolute_end_sample_number = tmd2->absolute_start_sample_number + tmd2->number_of_samples - 1;
			if (slice->end_sample_number <= absolute_end_sample_number) {
				slice->end_segment_number = uh->segment_number;
				break;
			}
		}
		free_file_processing_struct_m10(md_fps, FALSE_m10);
	}
	if (i == n_segs) {
		slice->end_segment_number = uh->segment_number;
		slice->end_sample_number = absolute_end_sample_number;
		slice->end_time = uh->file_end_time;
		end_seg_idx = i - 1;
		md_fps = NULL;
	}
	else {
		end_seg_idx = i;
		free_file_processing_struct_m10(md_fps, FALSE_m10);
	}
	
	// ********************************************** //
	// ***********  fill in other limits  *********** //
	// ********************************************** //
	
	// fill in slice session start & end times
	extract_path_parts_m10(seg_list[0], NULL, seg_name, NULL);
	sprintf_m10(tmp_str, "%s/%s.%s", seg_list[0], seg_name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
	md_fps = read_file_m10(NULL, tmp_str, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
	slice->session_start_time = md_fps->universal_header->session_start_time;
	if (n_segs > 1) {
		free_file_processing_struct_m10(md_fps, FALSE_m10);
		extract_path_parts_m10(seg_list[n_segs - 1], NULL, seg_name, NULL);
		sprintf_m10(tmp_str, "%s/%s.%s", seg_list[n_segs - 1], seg_name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
		md_fps = read_file_m10(NULL, tmp_str, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
	}
	slice->session_end_time = md_fps->universal_header->file_end_time;
	free_file_processing_struct_m10(md_fps, FALSE_m10);
	
	// get start segment limits
	extract_path_parts_m10(seg_list[start_seg_idx], NULL, seg_name, NULL);
	sprintf_m10(tmp_str, "%s/%s.%s", seg_list[start_seg_idx], seg_name, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10);
	tsi_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
	if (search_mode == TIME_SEARCH_m10)
		slice->start_sample_number = sample_number_for_uutc_m10(start_seg_start_idx, UUTC_NO_ENTRY_m10, slice->start_time, sampling_frequency, tsi_fps, FIND_CURRENT_m10);
	else  // search_mode == INDEX_SEARCH_m10
		slice->start_time = uutc_for_sample_number_m10(start_seg_start_idx, UUTC_NO_ENTRY_m10, slice->start_sample_number, sampling_frequency, tsi_fps, FIND_START_m10);
	
	// get end segment limits
	if (start_seg_idx != end_seg_idx) {
		free_file_processing_struct_m10(tsi_fps, FALSE_m10);
		extract_path_parts_m10(seg_list[end_seg_idx], NULL, seg_name, NULL);
		sprintf_m10(tmp_str, "%s/%s.%s", seg_list[end_seg_idx], seg_name, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10);
		tsi_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
	}
	if (search_mode == TIME_SEARCH_m10)
		slice->end_sample_number = sample_number_for_uutc_m10(end_seg_start_idx, UUTC_NO_ENTRY_m10, slice->end_time, sampling_frequency, tsi_fps, FIND_CURRENT_m10);
	else  // search_mode == INDEX_SEARCH_m10
		slice->end_time = uutc_for_sample_number_m10(end_seg_start_idx, UUTC_NO_ENTRY_m10, slice->end_sample_number, sampling_frequency, tsi_fps, FIND_END_m10);
	free_file_processing_struct_m10(tsi_fps, FALSE_m10);
	free((void *) *seg_list);
	
	slice->number_of_samples = (slice->end_sample_number - slice->start_sample_number) + 1;
	
	return(TRUE_m10);
}


TERN_m10        search_Sgmt_records_m10(si1 *MED_dir, TIME_SLICE_m10 *slice)
{
	ui1                             search_mode;
	si1                             path[FULL_FILE_NAME_BYTES_m10], name[BASE_FILE_NAME_BYTES_m10], tmp_str[FULL_FILE_NAME_BYTES_m10];
	si1				*passed_channel, **chan_list, num_str[FILE_NUMBERING_DIGITS_m10 + 1];
	si4                             n_chans;
	ui4                             type_code;
	si8                             i, n_recs, items_read, start_seg_start_idx, end_seg_start_idx;
	sf8                             sampling_frequency;
	FILE_PROCESSING_STRUCT_m10	*ri_fps, *rd_fps, *md_fps, *tsi_fps;
	UNIVERSAL_HEADER_m10		*uh;
	RECORD_INDEX_m10		*ri;
	REC_Sgmt_v10_m10		*Sgmt;
	
	
	if (slice == NULL)
		error_message_m10("%s(): NULL slice pointer\n", __FUNCTION__);
	else
		slice->start_segment_number = slice->end_segment_number = SEGMENT_NUMBER_NO_ENTRY_m10;
	
	// check for valid limit pair (time takes priority)
	if (slice->start_time != UUTC_NO_ENTRY_m10 && slice->end_time != UUTC_NO_ENTRY_m10) {
		search_mode = TIME_SEARCH_m10;
	}
	else if (slice->start_sample_number != SAMPLE_NUMBER_NO_ENTRY_m10 && slice->end_sample_number != SAMPLE_NUMBER_NO_ENTRY_m10) {
		search_mode = INDEX_SEARCH_m10;
	}
	else {
		error_message_m10("%s(): no valid limit pair\n", __FUNCTION__);
		return(FALSE_m10);
	}
	
	ri_fps = rd_fps = NULL;
	passed_channel = NULL;
	
SEARCH_SEG_TRY_SESSION_m10:

	// open record indices file
	type_code = generate_MED_path_components_m10(MED_dir, path, name);
	if (type_code != TIME_SERIES_CHANNEL_TYPE_m10 && type_code != SESSION_DIRECTORY_TYPE_CODE_m10)
		return(FALSE_m10);
	sprintf_m10(tmp_str, "%s/%s.%s", path, name, RECORD_INDICES_FILE_TYPE_STRING_m10);
	if (file_exists_m10(tmp_str) == FILE_EXISTS_m10)
		ri_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
	if (ri_fps == NULL) {
		if (type_code == TIME_SERIES_CHANNEL_TYPE_m10) {  // see if there are session Sgmt records
			extract_path_parts_m10(MED_dir, path, NULL, NULL);
			passed_channel = MED_dir;
			MED_dir = path;
			goto SEARCH_SEG_TRY_SESSION_m10;
		}
		return(FALSE_m10);
	}
	
	// check for Sgmt records
	uh = ri_fps->universal_header;
	n_recs = uh->number_of_entries;
	ri = ri_fps->record_indices;
	for (i = 0; i < n_recs; ++i) {
		if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10)
			break;
	}
	if (i == n_recs) {
		free_file_processing_struct_m10(ri_fps, FALSE_m10);
		if (type_code == TIME_SERIES_CHANNEL_TYPE_m10) {  // see if there are session Sgmt records
			extract_path_parts_m10(MED_dir, path, NULL, NULL);
			passed_channel = MED_dir;
			MED_dir = path;
			goto SEARCH_SEG_TRY_SESSION_m10;
		}
		return(FALSE_m10);
	}
	
	// fill in slice session start & end times
	slice->session_start_time = uh->session_start_time;
	slice->session_end_time = uh->file_end_time;
	
	// open record data file
	sprintf_m10(tmp_str, "%s/%s.%s", path, name, RECORD_DATA_FILE_TYPE_STRING_m10);
	if (file_exists_m10(tmp_str) == FILE_EXISTS_m10)
		rd_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
	if (rd_fps == NULL) {
		free_file_processing_struct_m10(ri_fps, FALSE_m10);
		return(FALSE_m10);
	}
	
	// check records contain index information if needed
	Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[i].file_offset + RECORD_HEADER_BYTES_m10);
	if (search_mode == INDEX_SEARCH_m10) {
		if (Sgmt->absolute_start_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10 || Sgmt->absolute_end_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10) {
			free_file_processing_struct_m10(ri_fps, FALSE_m10);
			free_file_processing_struct_m10(rd_fps, FALSE_m10);
			return(FALSE_m10);
		}
	}
	sampling_frequency = Sgmt->sampling_frequency;
		
	// find start segment
	slice->start_segment_number = 0;
	if (search_mode == TIME_SEARCH_m10) {
		for (; i < n_recs; ++i) {
			if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10) {
				Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[i].file_offset + RECORD_HEADER_BYTES_m10);
				if (slice->start_time <= Sgmt->end_time) {
					slice->start_segment_number = Sgmt->segment_number;
					if (slice->start_time < ri[i].start_time)
						slice->start_time = ri[i].start_time;
					break;
				}
			}
		}
	}
	else { // search by index
		for (; i < n_recs; ++i) {
			if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10) {
				Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[i].file_offset + RECORD_HEADER_BYTES_m10);
				if (slice->start_sample_number <= Sgmt->absolute_end_sample_number) {
					slice->start_segment_number = Sgmt->segment_number;
					if (slice->start_sample_number < Sgmt->absolute_start_sample_number)
						slice->start_sample_number = Sgmt->absolute_start_sample_number;
					break;
				}
			}
		}
	}
	if (i == n_recs) {
		free_file_processing_struct_m10(ri_fps, FALSE_m10);
		free_file_processing_struct_m10(rd_fps, FALSE_m10);
		error_message_m10("%s(): Start time/index exceeds session limits\n", __FUNCTION__);
		return(FALSE_m10);
	}
	start_seg_start_idx = Sgmt->absolute_start_sample_number;
	
	// find end segment
	slice->end_segment_number = 0;
	if (search_mode == TIME_SEARCH_m10) {
		for (; i < n_recs; ++i) {
			if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10) {
				Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[i].file_offset + RECORD_HEADER_BYTES_m10);
				if (Sgmt->end_time >= slice->end_time) {
					slice->end_segment_number = Sgmt->segment_number;
					break;
				}
			}
		}
	}
	else {  // search by index
		for (; i < n_recs; ++i) {
			if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10) {
				Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[i].file_offset + RECORD_HEADER_BYTES_m10);
				if (Sgmt->absolute_end_sample_number >= slice->end_sample_number) {
					slice->end_segment_number = Sgmt->segment_number;
					break;
				}
			}
		}
	}
	if (i == n_recs) {
		slice->end_segment_number = Sgmt->segment_number;
		slice->end_time = Sgmt->end_time;
		slice->end_sample_number = Sgmt->absolute_end_sample_number;
	}
	end_seg_start_idx = Sgmt->absolute_start_sample_number;
	
	
	// ********************************************** //
	// ***********  fill in other limits  *********** //
	// ********************************************** //
	
	if (type_code == SESSION_DIRECTORY_TYPE_CODE_m10) {
		chan_list = NULL;
		if (passed_channel == NULL) {
			n_chans = 0;
			chan_list = generate_file_list_m10(NULL, &n_chans, path, "*", "ticd", PP_FULL_PATH_m10, FALSE_m10);
			if (n_chans == 0) {
				warning_message_m10("%s(): Cannot fill in all limits\n");
				return(TRUE_m10);
			}
			passed_channel = chan_list[0];
		}
		generate_MED_path_components_m10(passed_channel, path, name);
		if (chan_list != NULL)
			free((void *) *chan_list);
	}
	
	numerical_fixed_width_string_m10(num_str, FILE_NUMBERING_DIGITS_m10, slice->start_segment_number);
	if (sampling_frequency == FREQUENCY_VARIABLE_m10 || sampling_frequency == FREQUENCY_NO_ENTRY_m10) {
		sprintf_m10(tmp_str, "%s/%s_s%s.%s/%s_s%s.%s", path, name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m10, name, num_str, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
		md_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
		sampling_frequency = md_fps->metadata->time_series_section_2.sampling_frequency;
		free_file_processing_struct_m10(md_fps, FALSE_m10);
	}
	
	// get start segment limits
	sprintf_m10(tmp_str, "%s/%s_s%s.%s/%s_s%s.%s", path, name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m10, name, num_str, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10);
	tsi_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
	if (search_mode == TIME_SEARCH_m10)
		slice->start_sample_number = sample_number_for_uutc_m10(start_seg_start_idx, UUTC_NO_ENTRY_m10, slice->start_time, sampling_frequency, tsi_fps, FIND_CURRENT_m10);
	else  // search_mode == INDEX_SEARCH_m10
		slice->start_time = uutc_for_sample_number_m10(start_seg_start_idx, UUTC_NO_ENTRY_m10, slice->start_sample_number, sampling_frequency, tsi_fps, FIND_START_m10);
	
	// get end segment limits
	if (slice->end_segment_number != slice->start_segment_number) {
		free_file_processing_struct_m10(tsi_fps, FALSE_m10);
		numerical_fixed_width_string_m10(num_str, FILE_NUMBERING_DIGITS_m10, slice->end_segment_number);
		sprintf_m10(tmp_str, "%s/%s_s%s.%s/%s_s%s.%s", path, name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m10, name, num_str, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10);
		tsi_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
	}
	if (search_mode == TIME_SEARCH_m10)
		slice->end_sample_number = sample_number_for_uutc_m10(end_seg_start_idx, UUTC_NO_ENTRY_m10, slice->end_time, sampling_frequency, tsi_fps, FIND_CURRENT_m10);
	else  // search_mode == INDEX_SEARCH_m10
		slice->end_time = uutc_for_sample_number_m10(end_seg_start_idx, UUTC_NO_ENTRY_m10, slice->end_sample_number, sampling_frequency, tsi_fps, FIND_END_m10);
	free_file_processing_struct_m10(tsi_fps, FALSE_m10);
	
	slice->number_of_samples = (slice->end_sample_number - slice->start_sample_number) + 1;
	
	return(TRUE_m10);
}


si8     segment_sample_number_to_time_m10(si1 *seg_dir, si8 local_sample_number, si8 absolute_start_sample_number, sf8 sampling_frequency, ui1 mode)
{
	si1	path[FULL_FILE_NAME_BYTES_m10], seg_name[SEGMENT_BASE_FILE_NAME_BYTES_m10];
	si8	uutc;
	FILE_PROCESSING_STRUCT_m10		*fps;
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2;
	
	
	extract_path_parts_m10(seg_dir, NULL, seg_name, NULL);
	
	// get absolute start sample number and sampling frequency from metadata
	if (absolute_start_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10 || sampling_frequency == FREQUENCY_NO_ENTRY_m10) {
		sprintf_m10(path, "%s/%s.%s", seg_dir, seg_name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
		fps = read_file_m10(NULL, path, 1, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
		tmd2 = &fps->metadata->time_series_section_2;
		absolute_start_sample_number = tmd2->absolute_start_sample_number;
		sampling_frequency = tmd2->sampling_frequency;
		free_file_processing_struct_m10(fps, FALSE_m10);
	}
	
	// read in time series indices
	sprintf_m10(path, "%s/%s.%s", seg_dir, seg_name, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10);
	fps = read_file_m10(NULL, path, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
	
	uutc = uutc_for_sample_number_m10(absolute_start_sample_number, UUTC_NO_ENTRY_m10, local_sample_number, sampling_frequency, fps, mode);
	
	free_file_processing_struct_m10(fps, FALSE_m10);
	
	return(uutc);
	
}


TERN_m10    set_global_time_constants_m10(TIMEZONE_INFO_m10 *timezone_info, si8 session_start_time, TERN_m10 prompt)
{
	si4                     n_potential_timezones, potential_timezone_entries[TZ_TABLE_ENTRIES_m10];
	si4                     i, j, response_num, items;
	TIMEZONE_INFO_m10	*tz_table;
	
	
	if (globals_m10->timezone_table == NULL)
		initialize_timezone_tables_m10();
	
	// reset
	globals_m10->time_constants_set = FALSE_m10;
	
	// capitalize & check aliases
	condition_timezone_info_m10(timezone_info);  // modified if alias found
	
	// start search
	n_potential_timezones = TZ_TABLE_ENTRIES_m10;
	tz_table = globals_m10->timezone_table;
	for (i = 0; i < n_potential_timezones; ++i)
		potential_timezone_entries[i] = i;
	
	// match country
	j = 0;
	if (*timezone_info->country) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->country, tz_table[potential_timezone_entries[i]].country) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m10;
		n_potential_timezones = j;
	}
	
	// match country_acronym_2_letter
	j = 0;
	if (*timezone_info->country_acronym_2_letter) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->country_acronym_2_letter, tz_table[potential_timezone_entries[i]].country_acronym_2_letter) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m10;
		n_potential_timezones = j;
	}
	
	// match country_acronym_3_letter
	j = 0;
	if (*timezone_info->country_acronym_3_letter) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->country_acronym_3_letter, tz_table[potential_timezone_entries[i]].country_acronym_3_letter) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m10;
		n_potential_timezones = j;
	}
	
	// match standard_timezone_acronym
	j = 0;
	if (*timezone_info->standard_timezone_acronym) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->standard_timezone_acronym, tz_table[potential_timezone_entries[i]].standard_timezone_acronym) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m10;
		n_potential_timezones = j;
	}

	// match standard_timezone
	j = 0;
	if (*timezone_info->standard_timezone) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->standard_timezone, tz_table[potential_timezone_entries[i]].standard_timezone) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m10;
		n_potential_timezones = j;
	}

	// match daylight_timezone_acronym
	j = 0;
	if (*timezone_info->daylight_timezone_acronym) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->daylight_timezone_acronym, tz_table[potential_timezone_entries[i]].daylight_timezone_acronym) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m10;
		n_potential_timezones = j;
	}

	// match daylight_timezone
	j = 0;
	if (*timezone_info->daylight_timezone) {
		for (i = 0; i < n_potential_timezones; ++i) 
			if (strcmp(timezone_info->daylight_timezone, tz_table[potential_timezone_entries[i]].daylight_timezone) == 0) 
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m10;
		n_potential_timezones = j;
	}

	// match standard_UTC_offset
	j = 0;
	if (timezone_info->standard_UTC_offset) {  // zero is a valid offset, but also could be zero from calloc() - can't use it to exclude
		for (i = 0; i < n_potential_timezones; ++i)
			if (timezone_info->standard_UTC_offset == tz_table[potential_timezone_entries[i]].standard_UTC_offset)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m10;
		n_potential_timezones = j;
	}

	// match territory
	j = 0;
	if (*timezone_info->territory) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->territory, tz_table[potential_timezone_entries[i]].territory) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m10;
		n_potential_timezones = j;
	}
	
	// match territory_acronym
	j = 0;
	if (*timezone_info->territory_acronym) {
		for (i = 0; i < n_potential_timezones; ++i)
			if (strcmp(timezone_info->territory_acronym, tz_table[potential_timezone_entries[i]].territory_acronym) == 0)
				potential_timezone_entries[j++] = potential_timezone_entries[i];
	}
	if (j) {
		if (j == 1)
			goto SET_GTC_TIMEZONE_MATCH_m10;
		n_potential_timezones = j;
	}

	// still multiple: doesn't matter if UTC offset, daylight change info, & timezone names all match
	for (i = 1; i < n_potential_timezones; ++i) {
		if (tz_table[potential_timezone_entries[i]].standard_UTC_offset != tz_table[potential_timezone_entries[0]].standard_UTC_offset)
			break;
		if (tz_table[potential_timezone_entries[i]].daylight_time_start_code != tz_table[potential_timezone_entries[0]].daylight_time_start_code)
			break;
		if (tz_table[potential_timezone_entries[i]].daylight_time_end_code != tz_table[potential_timezone_entries[0]].daylight_time_end_code)
			break;
		if (strcmp(tz_table[potential_timezone_entries[i]].standard_timezone, tz_table[potential_timezone_entries[0]].standard_timezone))
			break;
	}
	if (i == n_potential_timezones)
		goto SET_GTC_TIMEZONE_MATCH_m10;

	// still multiple: ask user
	if (prompt == TRUE_m10) {
		fprintf_m10(stderr, "\nMultiple potential timezone entries:\n\n");
		for (i = 0; i < n_potential_timezones; ++i) {
			fprintf_m10(stderr, "%d)\n", i + 1);
			show_timezone_info_m10(&tz_table[potential_timezone_entries[i]]);
			fputc_m10('\n', stderr);
		}
		fprintf_m10(stderr, "Select one (by number): ");
		items = scanf("%d", &response_num);
		if (items != 1 || response_num < 1 || response_num > n_potential_timezones) {
			error_message_m10("Invalid choice\n");
			exit_m10(1);
		}
		potential_timezone_entries[0] = potential_timezone_entries[--response_num];
	}
	else {
		return(FALSE_m10);
	}
	
SET_GTC_TIMEZONE_MATCH_m10:
	*timezone_info = tz_table[potential_timezone_entries[0]];
	globals_m10->standard_UTC_offset = timezone_info->standard_UTC_offset;
	strncpy_m10(globals_m10->standard_timezone_acronym, timezone_info->standard_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m10);
	strncpy_m10(globals_m10->standard_timezone_string, timezone_info->standard_timezone, TIMEZONE_STRING_BYTES_m10);
	strtotitle_m10(globals_m10->standard_timezone_string);  // beautify
	if (timezone_info->daylight_time_start_code) {
		if (timezone_info->daylight_time_start_code == DTCC_VALUE_NO_ENTRY_m10) {
			globals_m10->observe_DST = UNKNOWN_m10;
		}
		else {
			globals_m10->observe_DST = TRUE_m10;
			strncpy_m10(globals_m10->daylight_timezone_acronym, timezone_info->daylight_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m10);
			strncpy_m10(globals_m10->daylight_timezone_string, timezone_info->daylight_timezone, TIMEZONE_STRING_BYTES_m10);
			strtotitle_m10(globals_m10->daylight_timezone_string);  // beautify
			globals_m10->daylight_time_start_code.value = timezone_info->daylight_time_start_code;
			globals_m10->daylight_time_end_code.value = timezone_info->daylight_time_end_code;
		}
	}
	else {
		globals_m10->observe_DST = FALSE_m10;
	}
	globals_m10->time_constants_set = TRUE_m10;

	if (session_start_time)  // pass CURRENT_TIME_m10 for session starting now; pass zero if just need to get timezone_info for a locale
		generate_recording_time_offset_m10(session_start_time);

	return(TRUE_m10);
}


TERN_m10	set_time_and_password_data_m10(si1 *unspecified_password, si1 *MED_directory, si1 *section_2_encryption_level, si1 *section_3_encryption_level)
{
	si1                             MED_dir_copy[FULL_FILE_NAME_BYTES_m10], metadata_file[FULL_FILE_NAME_BYTES_m10];
	si8                             items_read;
	FILE_PROCESSING_STRUCT_m10	*metadata_fps;
	METADATA_SECTION_1_m10		*md1;
	
	
	// copy directory name to modify if not fromm root
	path_from_root_m10(MED_directory, MED_dir_copy);
		
	// find a MED metadata file
	if (find_metadata_file_m10(MED_dir_copy, metadata_file) == NULL) {
		error_message_m10("%s(): \"%s\" does not contain any metadata files\n", __FUNCTION__, MED_directory);
		return(FALSE_m10);
	}
	
	// read in metadata file (will process password, decrypt metadata and set global time constants)
	globals_m10->password_data.processed = 0;  // not ternary FALSE_m10 (so when structure is zeroed it is marked as not processed)
	metadata_fps = read_file_m10(NULL, metadata_file, 1, NULL, &items_read, unspecified_password, RETURN_ON_FAIL_m10);
	if (metadata_fps == NULL)
		return(FALSE_m10);
	globals_m10->session_start_time = metadata_fps->universal_header->session_start_time;
	
	// return metadata encryption level info
	md1 = &metadata_fps->metadata->section_1;
	if (section_2_encryption_level != NULL)
		*section_2_encryption_level = md1->section_2_encryption_level;
	if (section_3_encryption_level != NULL)
		*section_3_encryption_level = md1->section_3_encryption_level;
	
	// clean up
	free_file_processing_struct_m10(metadata_fps, FALSE_m10);
	
	return(TRUE_m10);
}


void    show_daylight_change_code_m10(DAYLIGHT_TIME_CHANGE_CODE_m10 * code, si1 * prefix)
{
	static si1	*relative_days[7] = { "", "First", "Second", "Third", "Fourth", "Fifth", "Last"};
	static si1	*weekdays[8] = { "", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	static si1	*months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	static si1	*mday_num_sufs[32] = { "", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", \
		"th", "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "st" };

	
	if (prefix == NULL)
		prefix = "";
	printf_m10("%sStructure Content:\n", prefix);
	printf_m10("%sCode Type (DST end / DST Not Observed / DST start) ==  (-1 / 0 / +1): %hhd\n", prefix, code->code_type);
	printf_m10("%sDay of Week (No Entry / [Sunday : Saturday]) ==  (0 / [1 : 7]): %hhd\n", prefix, code->day_of_week);
	printf_m10("%sRelative Weekday of Month (No Entry / [First : Fifth] / Last) ==  (0 / [1 : 5] / 6): %hhd\n", prefix, code->relative_weekday_of_month);
	printf_m10("%sDay of Month (No Entry / [1 : 31]) ==  (0 / [1 : 31]): %hhd\n", prefix, code->day_of_month);
	printf_m10("%sMonth (No Entry / [January : December]) ==  (-1 / [0 : 11]): %hhd\n", prefix, code->month);
	printf_m10("%sHours of Day [-128 : +127] hours relative to 00:00 (midnight): %hhd\n", prefix, code->hours_of_day);
	printf_m10("%sReference Time (Local / UTC) ==  (0 / 1): %hhd\n", prefix, code->reference_time);
	printf_m10("%sShift Minutes [-120 : +120] minutes: %hhd\n", prefix, code->shift_minutes);
	printf_m10("%sValue: 0x%lx\n\n", prefix, code->value);

	// human readable
	printf_m10("%sTranslated Content:\n", prefix);
	switch (code->value) {
		case DTCC_VALUE_NO_ENTRY_m10:
			printf_m10("%s: daylight saving change information not entered\n\n", prefix);
			return;
	case DTCC_VALUE_NOT_OBSERVED_m10:
			printf_m10("%s: daylight saving not observed\n\n", prefix);
			return;
	}
	switch (code->code_type) {
		case -1:
			printf_m10("%s: daylight saving END\n", prefix);
			break;
		case 1:
			printf_m10("%s: daylight saving START\n", prefix);
			break;
	}

	printf_m10("%s: ", prefix);
	if (code->relative_weekday_of_month) {
		printf_m10("%s ", relative_days[(si4) code->relative_weekday_of_month]);
		printf_m10("%s ", weekdays[(si4) code->day_of_week]);
		printf_m10("in %s ", months[(si4) code->month]);
	}
	else if (code->day_of_month) {
		printf_m10("%s ", months[(si4) code->month]);
		printf_m10("%hhd%s ", code->day_of_month, mday_num_sufs[(si4) code->day_of_month]);
	}

	printf_m10("at %hhd:00 ", code->hours_of_day);
	switch (code->reference_time) {
		case 0:
			printf_m10("local ");
			break;
		case 1:
			printf_m10("UTC ");
			break;
	}
	if (code->shift_minutes < 0)
		printf_m10(" (shift back by %hhd minutes)\n\n", ABS_m10(code->shift_minutes));
	else
		printf_m10(" (shift forward by %hhd minutes)\n\n", code->shift_minutes);

	return;
}


void	show_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps)
{
	si1	hex_str[HEX_STRING_BYTES_m10(TYPE_STRLEN_m10)], *s;
	si4	i;
	
	
	printf_m10("----------- File Processing Structure - START ----------\n");
	UTF8_printf_m10("Full File Name: %s\n", fps->full_file_name);
	if (fps->fd >= 3)
		printf_m10("File Descriptor: %d (open)\n", fps->fd);
	else if (fps->fd == -1)
		printf_m10("File Descriptor: %d (closed)\n", fps->fd);
	else if (fps->fd == FPS_FD_NO_ENTRY_m10)
		printf_m10("File Descriptor: %d (not yet opened)\n", fps->fd);
	else if (fps->fd == FPS_FD_EPHEMERAL_m10)
		printf_m10("File Descriptor: %d (ephemeral)\n", fps->fd);
	else    // stdin == 0, stdout == 1, stderr == 2
		printf_m10("File Descriptor: %d (standard stream: invalid)\n", fps->fd);
	printf_m10("File Length: ");
	if (fps->file_length == FPS_FILE_LENGTH_UNKNOWN_m10)
		printf_m10("unknown\n");
	else
		printf_m10("%ld\n", fps->file_length);
	s = (si1 *)&fps->universal_header->type_code;
	generate_hex_string_m10((ui1 *) s, TYPE_STRLEN_m10, hex_str);
	printf_m10("File Type Code: %s    (", hex_str);
	for (i = 0; i < 4; ++i)
		printf_m10(" %c ", *s++);
	printf_m10(")\n");
	printf_m10("Raw Data Bytes: %ld\n", fps->raw_data_bytes);
	show_universal_header_m10(fps, NULL);
	if (fps->raw_data_bytes > UNIVERSAL_HEADER_BYTES_m10) {
		switch (fps->universal_header->type_code) {
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
			case VIDEO_METADATA_FILE_TYPE_CODE_m10:
				show_metadata_m10(fps, NULL);
				break;
			case RECORD_DATA_FILE_TYPE_CODE_m10:
				show_records_m10(fps, ALL_TYPES_CODE_m10);
				break;
			default:
				break;
		}
	}
	printf_m10("------------ File Processing Structure - END -----------\n\n");
	
	return;
}


void    show_globals_m10(void)
{
	si1     hex_str[HEX_STRING_BYTES_m10(8)];
	
	
	printf_m10("MED Globals\n");
	printf_m10("-----------\n\n");
	printf_m10("Time Constants\n");
	printf_m10("--------------\n");
	// time_constants_set, RTO_known
	printf_m10("time_constants_set: %hhd\n", globals_m10->time_constants_set);
	printf_m10("RTO_known: %hhd\n", globals_m10->RTO_known);
	printf_m10("recording_time_offset: %ld\n", globals_m10->recording_time_offset);
	printf_m10("standard_UTC_offset: %d\n", globals_m10->standard_UTC_offset);
	printf_m10("standard_timezone_acronym: %s\n", globals_m10->standard_timezone_acronym);
	printf_m10("standard_timezone_string: %s\n", globals_m10->standard_timezone_string);
	printf_m10("observe_DST: %hhd\n", globals_m10->observe_DST);
	printf_m10("daylight_timezone_acronym: %s\n", globals_m10->daylight_timezone_acronym);
	printf_m10("daylight_timezone_string: %s\n", globals_m10->daylight_timezone_string);
	generate_hex_string_m10((ui1 *)&globals_m10->daylight_time_start_code.value, 8, hex_str);
	printf_m10("daylight_time_start_code: %s\n", hex_str);
	generate_hex_string_m10((ui1 *)&globals_m10->daylight_time_end_code.value, 8, hex_str);
	printf_m10("daylight_time_end_code: %s\n", hex_str);
	printf_m10("Alignment Fields\n");
	printf_m10("----------------\n");
	printf_m10("universal_header_aligned: %hhd\n", globals_m10->universal_header_aligned);
	printf_m10("metadata_section_1_aligned: %hhd\n", globals_m10->metadata_section_1_aligned);
	printf_m10("time_series_metadata_section_2_aligned: %hhd\n", globals_m10->time_series_metadata_section_2_aligned);
	printf_m10("video_metadata_section_2_aligned: %hhd\n", globals_m10->video_metadata_section_2_aligned);
	printf_m10("metadata_section_3_aligned: %hhd\n", globals_m10->metadata_section_3_aligned);
	printf_m10("all_metadata_structures_aligned: %hhd\n", globals_m10->all_metadata_structures_aligned);
	printf_m10("time_series_indices_aligned: %hhd\n", globals_m10->time_series_indices_aligned);
	printf_m10("video_indices_aligned: %hhd\n", globals_m10->video_indices_aligned);
	printf_m10("CMP_block_header_aligned: %hhd\n", globals_m10->CMP_block_header_aligned);
	printf_m10("record_header_aligned: %hhd\n", globals_m10->record_header_aligned);
	printf_m10("record_indices_aligned: %hhd\n", globals_m10->record_indices_aligned);
	printf_m10("all_record_structures_aligned: %hhd\n", globals_m10->all_record_structures_aligned);
	printf_m10("all_structures_aligned: %hhd\n\n", globals_m10->all_structures_aligned);
	printf_m10("Miscellaneous\n");
	printf_m10("-------------\n");
	printf_m10("CRC_mode: %u\n", globals_m10->CRC_mode);
	printf_m10("verbose: %hhd\n", globals_m10->verbose);
	printf_m10("behavior_on_fail: %u\n", globals_m10->behavior_on_fail);
	
	return;
}


void    show_location_info_m10(LOCATION_INFO_m10 *li)
{
	show_timezone_info_m10(&li->timezone_info);
	printf_m10("Locality: %s\n", li->locality);
	printf_m10("Postal Code: %s\n", li->postal_code);
	printf_m10("Timezone Description: %s\n", li->timezone_description);
	printf_m10("Latitude: %lf\n", li->latitude);
	printf_m10("Longitude: %lf\n", li->longitude);
	printf_m10("WAN_IPv4 Address: %s\n", li->WAN_IPv4_address);
	
	return;
}


void	show_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps, METADATA_m10 *md)
{
	si1                                     hex_str[HEX_STRING_BYTES_m10(8)];
	METADATA_SECTION_1_m10			*md1;
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2, *gmd2;
	VIDEO_METADATA_SECTION_2_m10		*vmd2;
	METADATA_SECTION_3_m10			*md3;
	
	
	// assign
	if (fps != NULL)
		md = fps->metadata;
	
	if (md != NULL) {
		md1 = &md->section_1;
		tmd2 = &md->time_series_section_2;
		vmd2 = &md->video_section_2;
		md3 = &md->section_3;
	}
	else {
		error_message_m10("%s(): invalid input\n", __FUNCTION__);
		return;
	}
	
	// decrypt if needed
	if (md1->section_2_encryption_level > NO_ENCRYPTION_m10 || md1->section_3_encryption_level > NO_ENCRYPTION_m10)
		if (fps != NULL)
			decrypt_metadata_m10(fps);
	
	// show
	printf_m10("------------------- Metadata - START -------------------\n");
	printf_m10("------------------ Section 1 - START -------------------\n");
	if (*md1->level_1_password_hint)
		UTF8_printf_m10("Level 1 Password Hint: %s\n", md1->level_1_password_hint);
	if (*md1->level_2_password_hint)
		UTF8_printf_m10("Level 2 Password Hint: %s\n", md1->level_2_password_hint);
	printf_m10("Section 2 Encryption Level: %d ", md1->section_2_encryption_level);
	if (md1->section_2_encryption_level == NO_ENCRYPTION_m10)
		printf_m10("(none)\n");
	else if (md1->section_2_encryption_level == LEVEL_1_ENCRYPTION_m10)
		printf_m10("(level 1, currently encrypted)\n");
	else if (md1->section_2_encryption_level == LEVEL_2_ENCRYPTION_m10)
		printf_m10("(level 2, currently encrypted)\n");
	else if (md1->section_2_encryption_level == -LEVEL_1_ENCRYPTION_m10)
		printf_m10("(level 1, currently decrypted)\n");
	else if (md1->section_2_encryption_level == -LEVEL_2_ENCRYPTION_m10)
		printf_m10("(level 2, currently decrypted)\n");
	else
		printf_m10("(unrecognized code)\n");
	printf_m10("Section 3 Encryption Level: %d ", md1->section_3_encryption_level);
	if (md1->section_3_encryption_level == NO_ENCRYPTION_m10)
		printf_m10("(none)\n");
	else if (md1->section_3_encryption_level == LEVEL_1_ENCRYPTION_m10)
		printf_m10("(level 1, currently encrypted)\n");
	else if (md1->section_3_encryption_level == LEVEL_2_ENCRYPTION_m10)
		printf_m10("(level 2, currently encrypted)\n");
	else if (md1->section_3_encryption_level == -LEVEL_1_ENCRYPTION_m10)
		printf_m10("(level 1, currently decrypted)\n");
	else if (md1->section_3_encryption_level == -LEVEL_2_ENCRYPTION_m10)
		printf_m10("(level 2, currently decrypted)\n");
	else
		printf_m10("(unrecognized code)\n");
	printf_m10("------------------- Section 1 - END --------------------\n");
	printf_m10("------------------ Section 2 - START -------------------\n");
	if (md1->section_2_encryption_level <= NO_ENCRYPTION_m10) {
		
		// type-independent fields
		gmd2 = tmd2;
		if (*gmd2->session_description)
			UTF8_printf_m10("Session Description: %s\n", gmd2->session_description);
		else
			printf_m10("Session Description: no entry\n");
		if (*gmd2->channel_description)
			UTF8_printf_m10("Channel Description: %s\n", gmd2->channel_description);
		else
			printf_m10("Channel Description: no entry\n");
		if (*gmd2->segment_description)
			UTF8_printf_m10("Segment Description: %s\n", gmd2->segment_description);
		else
			printf_m10("Segment Description: no entry\n");
		if (*gmd2->equipment_description)
			UTF8_printf_m10("Equipment Description: %s\n", gmd2->equipment_description);
		else
			printf_m10("Equipment Description: no entry\n");
		if (gmd2->acquisition_channel_number == METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m10)
			printf_m10("Acquisition Channel Number: no entry\n");
		else
			printf_m10("Acquisition Channel Number: %d\n", gmd2->acquisition_channel_number);
		
		// type-specific fields
		if (tmd2 != NULL) {
			if (*tmd2->reference_description)
				UTF8_printf_m10("Reference Description: %s\n", tmd2->reference_description);
			else
				printf_m10("Reference Description: no entry\n");
			if (tmd2->sampling_frequency == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10)
				printf_m10("Sampling Frequency: no entry\n");
			else if (tmd2->sampling_frequency == TIME_SERIES_METADATA_FREQUENCY_VARIABLE_m10)
				printf_m10("Sampling Frequency: variable\n");
			else
				printf_m10("Sampling Frequency: %lf\n", tmd2->sampling_frequency);
			if (tmd2->low_frequency_filter_setting == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10)
				printf_m10("Low Frequency Filter Setting: no entry\n");
			else
				printf_m10("Low Frequency Filter Setting (Hz): %lf\n", tmd2->low_frequency_filter_setting);
			if (tmd2->high_frequency_filter_setting == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10)
				printf_m10("High Frequency Filter Setting: no entry\n");
			else
				printf_m10("High Frequency Filter Setting (Hz): %lf\n", tmd2->high_frequency_filter_setting);
			if (tmd2->notch_filter_frequency_setting == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10)
				printf_m10("Notch Filter Frequency Setting: no entry\n");
			else
				printf_m10("Notch Filter Frequency Setting (Hz): %lf\n", tmd2->notch_filter_frequency_setting);
			if (tmd2->AC_line_frequency == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10)
				printf_m10("AC Line Frequency: no entry\n");
			else
				printf_m10("AC Line Frequency (Hz): %lf\n", tmd2->AC_line_frequency);
			if (tmd2->amplitude_units_conversion_factor == TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10)
				printf_m10("Amplitiude Units Conversion Factor: no entry\n");
			else
				printf_m10("Amplitude Units Conversion Factor: %lf\n", tmd2->amplitude_units_conversion_factor);
			if (*tmd2->amplitude_units_description)
				UTF8_printf_m10("Amplitude Units Description: %s\n", tmd2->amplitude_units_description);
			else
				printf_m10("Amplitude Units Description: no entry\n");
			if (tmd2->time_base_units_conversion_factor == TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10)
				printf_m10("Time Base Units Conversion Factor: no entry\n");
			else
				printf_m10("Time Base Units Conversion Factor: %lf\n", tmd2->time_base_units_conversion_factor);
			if (*tmd2->time_base_units_description)
				UTF8_printf_m10("Time Base Units Description: %s\n", tmd2->time_base_units_description);
			else
				printf_m10("Time Base Units Description: no entry\n");
			if (tmd2->absolute_start_sample_number == TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_NO_ENTRY_m10)
				printf_m10("Absolute Start Sample Number: no entry\n");
			else
				printf_m10("Absolute Start Sample Number: %ld\n", tmd2->absolute_start_sample_number);
			if (tmd2->number_of_samples == TIME_SERIES_METADATA_NUMBER_OF_SAMPLES_NO_ENTRY_m10)
				printf_m10("Number of Samples: no entry\n");
			else
				printf_m10("Number of Samples: %ld\n", tmd2->number_of_samples);
			if (tmd2->number_of_blocks == TIME_SERIES_METADATA_NUMBER_OF_BLOCKS_NO_ENTRY_m10)
				printf_m10("Number of Blocks: no entry\n");
			else
				printf_m10("Number of Blocks: %ld\n", tmd2->number_of_blocks);
			if (tmd2->maximum_block_bytes == TIME_SERIES_METADATA_MAXIMUM_BLOCK_BYTES_NO_ENTRY_m10)
				printf_m10("Maximum Block Bytes: no entry\n");
			else
				printf_m10("Maximum Block Bytes: %ld\n", tmd2->maximum_block_bytes);
			if (tmd2->maximum_block_samples == TIME_SERIES_METADATA_MAXIMUM_BLOCK_SAMPLES_NO_ENTRY_m10)
				printf_m10("Maximum Block Samples: no entry\n");
			else
				printf_m10("Maximum Block Samples: %u\n", tmd2->maximum_block_samples);
			if (tmd2->maximum_block_difference_bytes == TIME_SERIES_METADATA_MAXIMUM_BLOCK_DIFFERENCE_BYTES_NO_ENTRY_m10)
				printf_m10("Maximum Block Difference Bytes: no entry\n");
			else
				printf_m10("Maximum Block Difference Bytes: %u\n", tmd2->maximum_block_difference_bytes);
			if (tmd2->maximum_block_duration == TIME_SERIES_METADATA_MAXIMUM_BLOCK_DURATION_NO_ENTRY_m10)
				printf_m10("Maximum Block Duration: no entry\n");
			else
				UTF8_printf_m10("Maximum Block Duration: %lf %s\n", tmd2->maximum_block_duration, tmd2->time_base_units_description);
			if (tmd2->number_of_discontinuities == TIME_SERIES_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m10)
				printf_m10("Number of Discontinuities: no entry\n");
			else
				printf_m10("Number of Discontinuities: %ld\n", tmd2->number_of_discontinuities);
			if (tmd2->maximum_contiguous_blocks == TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_NO_ENTRY_m10)
				printf_m10("Maximum Contiguous Blocks: no entry\n");
			else
				printf_m10("Maximum Contiguous Blocks: %ld\n", tmd2->maximum_contiguous_blocks);
			if (tmd2->maximum_contiguous_block_bytes == TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_NO_ENTRY_m10)
				printf_m10("Maximum Contiguous Block Bytes: no entry\n");
			else
				printf_m10("Maximum Contiguous Block Bytes: %ld\n", tmd2->maximum_contiguous_block_bytes);
			if (tmd2->maximum_contiguous_samples == TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_NO_ENTRY_m10)
				printf_m10("Maximum Contiguous Samples: no entry\n");
			else
				printf_m10("Maximum Contiguous Samples: %ld\n", tmd2->maximum_contiguous_samples);
		}
		else if (vmd2 != NULL) {
			if (vmd2->horizontal_resolution == VIDEO_METADATA_HORIZONTAL_RESOLUTION_NO_ENTRY_m10)
				printf_m10("Horizontal Resolution: no entry\n");
			else
				printf_m10("Horizontal Resolution: %ld\n", vmd2->horizontal_resolution);
			if (vmd2->vertical_resolution == VIDEO_METADATA_VERTICAL_RESOLUTION_NO_ENTRY_m10)
				printf_m10("Vertical Resolution: no entry\n");
			else
				printf_m10("Vertical Resolution: %ld\n", vmd2->vertical_resolution);
			if (vmd2->frame_rate == VIDEO_METADATA_FRAME_RATE_NO_ENTRY_m10)
				printf_m10("Frame Rate: no entry\n");
			else
				printf_m10("Frame Rate: %lf (frames per second)\n", vmd2->frame_rate);
			if (vmd2->number_of_clips == VIDEO_METADATA_NUMBER_OF_CLIPS_NO_ENTRY_m10)
				printf_m10("Number of Clips: no entry\n");
			else
				printf_m10("Number of Clips: %ld (= number of video indices)\n", vmd2->number_of_clips);
			if (vmd2->maximum_clip_bytes == VIDEO_METADATA_MAXIMUM_CLIP_BYTES_NO_ENTRY_m10)
				printf_m10("Maximum Clip Bytes: no entry\n");
			else
				printf_m10("Maximum Clip Bytes: %ld\n", vmd2->maximum_clip_bytes);
			if (*vmd2->video_format)
				UTF8_printf_m10("Video Format: %s\n", vmd2->video_format);
			else
				printf_m10("Video Format: no entry\n");
			if (vmd2->number_of_video_files == VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_NO_ENTRY_m10)
				printf_m10("Number of Video Files: no entry\n");
			else
				printf_m10("Number of Video Files: %d\n", vmd2->number_of_video_files);
		}
		else {
			printf_m10("(unrecognized metadata section 2 type)\n");
		}
	}
	else {
		printf_m10("No access to section 2\n");
	}
	printf_m10("------------------- Section 2 - END --------------------\n");
	printf_m10("------------------ Section 3 - START -------------------\n");
	if (md1->section_3_encryption_level <= NO_ENCRYPTION_m10) {
		if (md3->recording_time_offset == UUTC_NO_ENTRY_m10)
			printf_m10("Recording Time Offset: no entry\n");
		else
			printf_m10("Recording Time Offset: %ld\n", md3->recording_time_offset);
		if (md3->daylight_time_start_code.value == DTCC_VALUE_NO_ENTRY_m10) {
			printf_m10("Daylight Time Start Code: no entry\n");
		}
		else {
			generate_hex_string_m10((ui1 *) &md3->daylight_time_start_code.value, 8, hex_str);
			printf_m10("Daylight Time Start Code: %s\n", hex_str);
		}
		if (md3->daylight_time_end_code.value == DTCC_VALUE_NO_ENTRY_m10) {
			printf_m10("Daylight Time End Code: no entry\n");
		}
		else {
			generate_hex_string_m10((ui1 *) &md3->daylight_time_end_code.value, 8, hex_str);
			printf_m10("Daylight Time End Code: %s\n", hex_str);
		}
		if (*md3->standard_timezone_acronym)
			printf_m10("Standard Timezone Acronym: %s\n", md3->standard_timezone_acronym);
		else
			printf_m10("Standard Timezone Acronym: no entry\n");
		if (*md3->standard_timezone_string)
			printf_m10("Standard Timezone String: %s\n", md3->standard_timezone_string);
		else
			printf_m10("Standard Timezone String: no entry\n");
		if (*md3->daylight_timezone_acronym)
			printf_m10("Daylight Timezone Acronym: %s\n", md3->daylight_timezone_acronym);
		else
			printf_m10("Daylight Timezone Acronym: no entry\n");
		if (*md3->daylight_timezone_string)
			printf_m10("Daylight Timezone String: %s\n", md3->daylight_timezone_string);
		else
			printf_m10("Daylight Timezone String: no entry\n");
		if (*md3->subject_name_1)
			UTF8_printf_m10("Subject Name 1: %s\n", md3->subject_name_1);
		else
			printf_m10("Subject Name 1: no entry\n");
		if (*md3->subject_name_2)
			UTF8_printf_m10("Subject Name 2: %s\n", md3->subject_name_2);
		else
			printf_m10("Subject Name 2: no entry\n");
		if (*md3->subject_name_3)
			UTF8_printf_m10("Subject Name 3: %s\n", md3->subject_name_3);
		else
			printf_m10("Subject Name 3: no entry\n");
		if (*md3->subject_ID)
			UTF8_printf_m10("Subject ID: %s\n", md3->subject_ID);
		else
			printf_m10("Subject ID: no entry\n");
		if (*md3->recording_country)
			UTF8_printf_m10("Recording Country: %s\n", md3->recording_country);
		else
			printf_m10("Recording Country: no entry\n");
		if (*md3->recording_territory)
			UTF8_printf_m10("Recording Territory: %s\n", md3->recording_territory);
		else
			printf_m10("Recording Territory: no entry\n");
		if (*md3->recording_locality)
			UTF8_printf_m10("Recording Locality: %s\n", md3->recording_locality);
		else
			printf_m10("Recording Locality: no entry\n");
		if (*md3->recording_institution)
			UTF8_printf_m10("Recording Institution: %s\n", md3->recording_institution);
		else
			printf_m10("Recording Institution: no entry\n");
		if (*md3->geotag_format)
			UTF8_printf_m10("GeoTag Format: %s\n", md3->geotag_format);
		else
			printf_m10("GeoTag Format: no entry\n");
		if (*md3->geotag_data)
			UTF8_printf_m10("GeoTag Data: %s\n", md3->geotag_data);
		else
			printf_m10("GeoTag Data: no entry\n");
		if (md3->standard_UTC_offset == STANDARD_UTC_OFFSET_NO_ENTRY_m10)
			printf_m10("Standard UTC Offset: no entry\n");
		else
			printf_m10("Standard UTC Offset: %d\n", md3->standard_UTC_offset);
	}
	else {
		printf_m10("No access to section 3\n");
	}
	printf_m10("------------------- Section 3 - END --------------------\n");
	printf_m10("-------------------- Metadata - END --------------------\n\n");
	
	return;
}


void	show_password_data_m10(PASSWORD_DATA_m10 *pwd)
{
	si1	hex_str[HEX_STRING_BYTES_m10(ENCRYPTION_KEY_BYTES_m10)];
	
	
	// use message_m10() because show_password_data_m10() is used in normal (no programming) functions => so allow output to be suppressed easily
	if (pwd == NULL) {
		message_m10("\n-------------- Global Password Data - START --------------\n");
		pwd = &globals_m10->password_data;
	}
	else {
		message_m10("\n------------------ Password Data - START -----------------\n");
	}
	if (pwd->access_level >= LEVEL_1_ACCESS_m10) {
		generate_hex_string_m10(pwd->level_1_encryption_key, ENCRYPTION_KEY_BYTES_m10, hex_str);
		message_m10("Level 1 Encryption Key: %s\n", hex_str);
	}
	if (pwd->access_level == LEVEL_2_ACCESS_m10) {
		generate_hex_string_m10(pwd->level_2_encryption_key, ENCRYPTION_KEY_BYTES_m10, hex_str);
		message_m10("Level 2 Encryption Key: %s\n", hex_str);
	}
	show_password_hints_m10(pwd);
	message_m10("Access Level: %hhu\n", pwd->access_level);
	message_m10("Processed: %hhu\n", pwd->access_level);
	message_m10("------------------- Password Data - END ------------------\n\n");
	
	return;
}


void	show_password_hints_m10(PASSWORD_DATA_m10 *pwd)
{
	// use message_m10() because show_password_data_m10() is used in normal (not programming) functions => so allow output to be suppressed easily
	
	if (pwd == NULL)
		pwd = &globals_m10->password_data;
	if (*pwd->level_1_password_hint)
		message_m10("Level 1 Password Hint: %s\n", pwd->level_1_password_hint);
	if (*pwd->level_2_password_hint)
		message_m10("Level 2 Password Hint: %s\n", pwd->level_2_password_hint);
	
	return;
}



void	show_records_m10(FILE_PROCESSING_STRUCT_m10 *fps, ui4 type_code)
{
	si8		        number_of_records;
	ui4		        i, r_cnt;
	ui1			*ui1_p, *end_p;
	RECORD_HEADER_m10	*record_header;
	
	
	number_of_records = fps->universal_header->number_of_entries;
	ui1_p = fps->raw_data + UNIVERSAL_HEADER_BYTES_m10;
	
	// number_of_records obtained from universal header
	if (number_of_records == UNKNOWN_NUMBER_OF_ENTRIES_m10) {   // can still process if not passed, but will fail on incomplete final record
		end_p = fps->raw_data + fps->file_length;
		r_cnt = 0;
		while (ui1_p < end_p) {
			record_header = (RECORD_HEADER_m10 *) ui1_p;
			if (record_header->type_code == type_code || type_code == ALL_TYPES_CODE_m10)
				show_record_m10(fps, record_header, r_cnt);
			ui1_p += record_header->total_record_bytes;
			++r_cnt;
		}
		fps->universal_header->number_of_entries = r_cnt;
	}
	else {
		for (i = 0; i < number_of_records; ++i) {
			record_header = (RECORD_HEADER_m10 *) ui1_p;
			if (record_header->type_code == type_code || type_code == ALL_TYPES_CODE_m10)
				show_record_m10(fps, record_header, i);
			ui1_p += record_header->total_record_bytes;
		}
	}
	
	return;
}


void    show_time_slice_m10(TIME_SLICE_m10 *slice)
{
	printf_m10("Conditioned: ");
	if (slice->conditioned == TRUE_m10)
		printf_m10("true\n");
	else if (slice->conditioned == FALSE_m10)
		printf_m10("false\n");
	else if (slice->conditioned == UNKNOWN_m10)
		printf_m10("unknown\n");
	else
		printf_m10("invalid value (%hhd)\n", slice->conditioned);
	
	printf_m10("Start Time: ");
	if (slice->start_time == UUTC_NO_ENTRY_m10)
		printf_m10("no entry\n");
	else if (slice->start_time == BEGINNING_OF_TIME_m10)
		printf_m10("beginning of time\n");
	else if (slice->start_time == END_OF_TIME_m10)
		printf_m10("end of time\n");
	else
		printf_m10("%ld\n", slice->start_time);
	
	printf_m10("End Time: ");
	if (slice->end_time == UUTC_NO_ENTRY_m10)
		printf_m10("no entry\n");
	else if (slice->end_time == BEGINNING_OF_TIME_m10)
		printf_m10("beginning of time\n");
	else if (slice->end_time == END_OF_TIME_m10)
		printf_m10("end of time\n");
	else
		printf_m10("%ld\n", slice->end_time);
	
	printf_m10("Start Sample Number: ");
	if (slice->start_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10)
		printf_m10("no entry\n");
	else if (slice->start_sample_number == END_OF_SAMPLE_NUMBERS_m10)
		printf_m10("end of indices\n");
	else
		printf_m10("%ld\n", slice->start_sample_number);
	
	printf_m10("End Sample Number: ");
	if (slice->end_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10)
		printf_m10("no entry\n");
	else if (slice->end_sample_number == END_OF_SAMPLE_NUMBERS_m10)
		printf_m10("end of indices\n");
	else
		printf_m10("%ld\n", slice->end_sample_number);
	
	printf_m10("Local Start Sample Number: ");
	if (slice->local_start_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10)
		printf_m10("no entry\n");
	else if (slice->local_start_sample_number == END_OF_SAMPLE_NUMBERS_m10)
		printf_m10("end of segment\n");
	else
		printf_m10("%ld\n", slice->local_start_sample_number);
	
	printf_m10("Local End Sample Number: ");
	if (slice->local_end_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10)
		printf_m10("no entry\n");
	else if (slice->local_end_sample_number == END_OF_SAMPLE_NUMBERS_m10)
		printf_m10("end of segment\n");
	else
		printf_m10("%ld\n", slice->local_end_sample_number);
	
	if (slice->number_of_samples == NUMBER_OF_SAMPLES_NO_ENTRY_m10)
		printf_m10("Number of Samples: no entry\n");
	else
		printf_m10("Number of Samples: %ld\n", slice->number_of_samples);
	
	printf_m10("Start Segment Number: ");
	if (slice->start_segment_number == SEGMENT_NUMBER_NO_ENTRY_m10)
		printf_m10("no entry\n");
	else
		printf_m10("%d\n", slice->start_segment_number);
	
	printf_m10("End Segment Number: ");
	if (slice->end_segment_number == SEGMENT_NUMBER_NO_ENTRY_m10)
		printf_m10("no entry\n");
	else
		printf_m10("%d\n", slice->end_segment_number);
	
	printf_m10("Session Start Time: ");
	if (slice->session_start_time == UUTC_NO_ENTRY_m10)
		printf_m10("no entry\n");
	else
		printf_m10("%ld\n", slice->session_start_time);
	
	printf_m10("Session End Time: ");
	if (slice->session_end_time == UUTC_NO_ENTRY_m10)
		printf_m10("no entry\n");
	else
		printf_m10("%ld\n", slice->session_end_time);
	
	printf_m10("Sample Number Reference Channel Name: ");
	if (*slice->sample_number_reference_channel_name)
		printf_m10("%s\n", slice->sample_number_reference_channel_name);
	else
		printf_m10("no entry\n");
	
	printf_m10("Sample Number Reference Channel Index: %d\n", slice->sample_number_reference_channel_index);
	
	printf_m10("\n");
	
	return;
}


void    show_timezone_info_m10(TIMEZONE_INFO_m10 *timezone_entry)
{
	printf_m10("Country: %s\n", timezone_entry->country);
	printf_m10("Country Acronym (2 letter): %s\n", timezone_entry->country_acronym_2_letter);
	printf_m10("Country Acronym (3 letter): %s\n", timezone_entry->country_acronym_3_letter);
	if (*timezone_entry->territory)
		printf_m10("Territory: %s\n", timezone_entry->territory);
	if (*timezone_entry->territory_acronym)
		printf_m10("Territory Acronym: %s\n", timezone_entry->territory_acronym);
	printf_m10("Standard Timezone: %s\n", timezone_entry->standard_timezone);
	printf_m10("Standard Timezone Acronym: %s\n", timezone_entry->standard_timezone_acronym);
	printf_m10("Standard UTC Offset (secs): %d\n", timezone_entry->standard_UTC_offset);
	printf_m10("Daylight Timezone: %s\n", timezone_entry->daylight_timezone);
	printf_m10("Daylight Timezone Acronym: %s\n", timezone_entry->daylight_timezone_acronym);
	
	if (timezone_entry->daylight_time_start_code) {
		if (timezone_entry->daylight_time_start_code == DTCC_VALUE_NO_ENTRY_m10) {
			printf_m10("Daylight Time info not available\n");
		}
		else {
			printf_m10("Daylight Time Start Code: 0x%lX\n", timezone_entry->daylight_time_start_code);
			show_daylight_change_code_m10((DAYLIGHT_TIME_CHANGE_CODE_m10 *)&timezone_entry->daylight_time_start_code, "\t");
			printf_m10("Daylight Time End Code: 0x%lX\n", timezone_entry->daylight_time_end_code);
			show_daylight_change_code_m10((DAYLIGHT_TIME_CHANGE_CODE_m10 *)&timezone_entry->daylight_time_end_code, "\t");
		}
	}
	else {
		printf_m10("Daylight Time not observed\n");
	}
	return;
}


void	show_universal_header_m10(FILE_PROCESSING_STRUCT_m10 *fps, UNIVERSAL_HEADER_m10 *uh)
{
	TERN_m10        ephemeral_flag;
	si1             hex_str[HEX_STRING_BYTES_m10(PASSWORD_VALIDATION_FIELD_BYTES_m10)], time_str[TIME_STRING_BYTES_m10];
	
	
	// assign
	if (fps != NULL) {
		uh = fps->universal_header;
		if (fps->fd == FPS_FD_EPHEMERAL_m10)
			ephemeral_flag = TRUE_m10;
		else
			ephemeral_flag = FALSE_m10;
	}
	else {
		if (uh == NULL) {
			error_message_m10("%s(): invalid input\n", __FUNCTION__);
			return;
		}
		ephemeral_flag = UNKNOWN_m10;
	}
	
	printf_m10("---------------- Universal Header - START ----------------\n");
	if (uh->header_CRC == CRC_NO_ENTRY_m10)
		printf_m10("Header CRC: no entry\n");
	else {
		generate_hex_string_m10((ui1 *)&uh->header_CRC, CRC_BYTES_m10, hex_str);
		printf_m10("Header CRC: %s\n", hex_str);
	}
	if (uh->body_CRC == CRC_NO_ENTRY_m10)
		printf_m10("Body CRC: no entry\n");
	else {
		generate_hex_string_m10((ui1 *)&uh->body_CRC, CRC_BYTES_m10, hex_str);
		printf_m10("Body CRC: %s\n", hex_str);
	}
	if (uh->file_end_time == UUTC_NO_ENTRY_m10)
		printf_m10("File End Time: no entry\n");
	else {
		time_string_m10(uh->file_end_time, time_str, TRUE_m10, FALSE_m10, FALSE_m10);
		printf_m10("File End Time: %ld (oUTC), %s\n", uh->file_end_time, time_str);
	}
	if (uh->number_of_entries == UNIVERSAL_HEADER_NUMBER_OF_ENTRIES_NO_ENTRY_m10)
		printf_m10("Number of Entries: no entry\n");
	else {
		printf_m10("Number of Entries: %ld  ", uh->number_of_entries);
		switch (uh->type_code) {
			case RECORD_DATA_FILE_TYPE_CODE_m10:
				printf_m10("(number of records in the file)\n");
				break;
			case RECORD_INDICES_FILE_TYPE_CODE_m10:
				printf_m10("(number of record indices in the file)\n");
				break;
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
			case VIDEO_METADATA_FILE_TYPE_CODE_m10:
				if (ephemeral_flag == TRUE_m10)
					printf_m10("(maximum number of records in records files at this level and below)\n");
				else if (ephemeral_flag == FALSE_m10)
					printf_m10("(one metadata entry per metadata file)\n");
				else // UNKNOWN
					printf_m10("(one metadata entry, or maximum number of records in a records file at this level and below)\n");
				break;
			case VIDEO_INDICES_FILE_TYPE_CODE_m10:
				printf_m10("(number of video indices in the file)\n");
				break;
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
				printf_m10("(number of CMP blocks in the file)\n");
				break;
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
				printf_m10("(number of time series indices in the file)\n");
				break;
			default:
				printf_m10("\n");
				break;
		}
	}
	if (uh->maximum_entry_size == UNIVERSAL_HEADER_MAXIMUM_ENTRY_SIZE_NO_ENTRY_m10)
		printf_m10("Maximum Entry Size: no entry\n");
	else {
		printf_m10("Maximum Entry Size: %u  ", uh->maximum_entry_size);
		switch (uh->type_code) {
			case RECORD_DATA_FILE_TYPE_CODE_m10:
				printf_m10("(number of bytes in the largest record in the file)\n");
				break;
			case RECORD_INDICES_FILE_TYPE_CODE_m10:
				printf_m10("(number of bytes in a record index)\n");
				break;
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
			case VIDEO_METADATA_FILE_TYPE_CODE_m10:
				if (ephemeral_flag == TRUE_m10)
					printf_m10("(maximum number of bytes in a record at this level and below)\n");
				else if (ephemeral_flag == FALSE_m10)
					printf_m10("(number of bytes in a metadata structure)\n");
				else // UNKNOWN
					printf_m10("(metadata bytes, or maximum number of bytes in a record at this level and below)\n");
				break;
			case VIDEO_INDICES_FILE_TYPE_CODE_m10:
				printf_m10("(number of bytes in a video index)\n");
				break;
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
				printf_m10("(number of bytes in the largest CMP block in the file)\n");
				break;
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
				printf_m10("(number of bytes in a time series index)\n");
				break;
			default:
				printf_m10("\n");
				break;
				
		}
	}
	if (uh->segment_number == UNIVERSAL_HEADER_SEGMENT_NUMBER_NO_ENTRY_m10)
		printf_m10("Segment Number: no entry\n");
	else if (uh->segment_number == UNIVERSAL_HEADER_CHANNEL_LEVEL_CODE_m10)
		printf_m10("Segment Number: channel level\n");
	else if (uh->segment_number == UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m10)
		printf_m10("Segment Number: session level\n");
	else
		printf_m10("Segment Number: %d\n", uh->segment_number);
	if (*uh->type_string)
		printf_m10("File Type String: %s\n", uh->type_string);
	else
		printf_m10("File Type String: no entry\n");
	if (uh->MED_version_major == UNIVERSAL_HEADER_MED_VERSION_MAJOR_NO_ENTRY_m10 || uh->MED_version_minor == UNIVERSAL_HEADER_MED_VERSION_MINOR_NO_ENTRY_m10) {
		if (uh->MED_version_major == UNIVERSAL_HEADER_MED_VERSION_MAJOR_NO_ENTRY_m10)
			printf_m10("MED Version Major: no entry\n");
		else
			printf_m10("MED Version Major: %u\n", uh->MED_version_major);
		if (uh->MED_version_minor == UNIVERSAL_HEADER_MED_VERSION_MINOR_NO_ENTRY_m10)
			printf_m10("MED Version Minor: no entry\n");
		else
			printf_m10("MED Version Minor: %u\n", uh->MED_version_minor);
	}
	else
		printf_m10("MED Version: %u.%u\n", uh->MED_version_major, uh->MED_version_minor);
	if (uh->byte_order_code == UNIVERSAL_HEADER_BYTE_ORDER_CODE_NO_ENTRY_m10)
		printf_m10("Byte Order Code: no entry ");
	else {
		printf_m10("Byte Order Code: %u ", uh->byte_order_code);
		if (uh->byte_order_code == LITTLE_ENDIAN_m10)
			printf_m10("(little endian)\n");
		else if (uh->byte_order_code == BIG_ENDIAN_m10)
			printf_m10("(big endian)\n");
		else
			printf_m10("(unrecognized code)\n");
	}
	if (uh->session_start_time == UUTC_NO_ENTRY_m10)
		printf_m10("Session Start Time: no entry\n");
	else {
		time_string_m10(uh->session_start_time, time_str, TRUE_m10, FALSE_m10, FALSE_m10);
		printf_m10("Session Start Time: %ld (oUTC), %s\n", uh->session_start_time, time_str);
	}
	if (uh->file_start_time == UUTC_NO_ENTRY_m10)
		printf_m10("File Start Time: no entry\n");
	else {
		time_string_m10(uh->file_start_time, time_str, TRUE_m10, FALSE_m10, FALSE_m10);
		printf_m10("File Start Time: %ld (oUTC), %s\n", uh->file_start_time, time_str);
	}
	if (*uh->session_name)
		UTF8_printf_m10("Session Name: %s\n", uh->session_name);
	else
		printf_m10("Session Name: no entry\n");
	if (*uh->channel_name)
		UTF8_printf_m10("Channel Name: %s\n", uh->channel_name);
	else
		printf_m10("Channel Name: no entry\n");
	if (*uh->anonymized_subject_ID)
		UTF8_printf_m10("Anonymized Subject ID: %s\n", uh->anonymized_subject_ID);
	else
		printf_m10("Anonymized Subject ID: no entry\n");
	if (uh->session_UID == UID_NO_ENTRY_m10)
		printf_m10("Session UID: no entry\n");
	else {
		generate_hex_string_m10((ui1 *)&uh->session_UID, UID_BYTES_m10, hex_str);
		printf_m10("Session UID: %s\n", hex_str);
	}
	if (uh->channel_UID == UID_NO_ENTRY_m10)
		printf_m10("Channel UID: no entry\n");
	else {
		generate_hex_string_m10((ui1 *)&uh->channel_UID, UID_BYTES_m10, hex_str);
		printf_m10("Channel UID: %s\n", hex_str);
	}
	if (uh->segment_UID == UID_NO_ENTRY_m10)
		printf_m10("Segment UID: no entry\n");
	else {
		generate_hex_string_m10((ui1 *)&uh->segment_UID, UID_BYTES_m10, hex_str);
		printf_m10("Segment UID: %s\n", hex_str);
	}
	if (uh->file_UID == UID_NO_ENTRY_m10)
		printf_m10("File UID: no entry\n");
	else {
		generate_hex_string_m10((ui1 *)&uh->file_UID, UID_BYTES_m10, hex_str);
		printf_m10("File UID: %s\n", hex_str);
	}
	if (uh->provenance_UID == UID_NO_ENTRY_m10)
		printf_m10("Provenance UID: no entry\n");
	else {
		generate_hex_string_m10((ui1 *)&uh->provenance_UID, UID_BYTES_m10, hex_str);
		printf_m10("Provenance UID: %s  ", hex_str);
		if (uh->provenance_UID == uh->file_UID)
			printf_m10("(original data)\n");
		else
			printf_m10("(derived data)\n");
	}
	if (all_zeros_m10(uh->level_1_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10) == TRUE_m10)
		printf_m10("Level 1 Password Validation_Field: no entry\n");
	else {
		generate_hex_string_m10(uh->level_1_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10, hex_str);
		printf_m10("Level 1 Password Validation_Field: %s\n", hex_str);
	}
	if (all_zeros_m10(uh->level_2_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10) == TRUE_m10)
		printf_m10("Level 2 Password Validation_Field: no entry\n");
	else {
		generate_hex_string_m10(uh->level_2_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10, hex_str);
		printf_m10("Level 2 Password Validation_Field: %s\n", hex_str);
	}
	if (all_zeros_m10(uh->level_3_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10) == TRUE_m10)
		printf_m10("Level 3 Password Validation_Field: no entry\n");
	else {
		generate_hex_string_m10(uh->level_3_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10, hex_str);
		printf_m10("Level 3 Password Validation_Field: %s\n", hex_str);
	}
	printf_m10("---------------- Universal Header - END ----------------\n\n");
	
	return;
}


void	sort_fps_array_m10(FILE_PROCESSING_STRUCT_m10 **fps_array, si4 n_fps)
{
	qsort((void *) fps_array, (size_t) n_fps, sizeof(FILE_PROCESSING_STRUCT_m10 *), compare_fps_start_times_m10);

	return;
}


inline TERN_m10	str_contains_regex_m10(si1 *string)
{
	si1	c;
	
	
	// NOT an exhaustive list of potential regex characters, just enough to know if regex is present
	
	if (string == NULL)
		return(FALSE_m10);
	
	while ((c = *string++)) {
		switch (c) {
			case '*':
			case '?':
			case '+':
			case '|':
			case '^':
			case '$':
			case '[':
			case '(':
			case '{':
				return(TRUE_m10);
		}
	}
	return(FALSE_m10);
}


si1	*str_match_end_m10(si1 *pattern, si1 *buffer)
{
	// returns pointer to the character after the first match in the buffer, NULL if no match (assumes both pattern & buffer are zero-terminated)
	si4	pat_len, buf_len;
	si1	*pat_p, *buf_p;
	
	
	pat_len = strlen(pattern);
	buf_len = strlen(buffer);
	if (pat_len >= buf_len)
		return(NULL);
	
	do {
		pat_p = pattern;
		buf_p = buffer++;
		while (*buf_p++ == *pat_p++) {
			if (!*pat_p) {
				if (*buf_p)
					return(buf_p);
				else
					return(NULL);
			}
		}
	} while (*buf_p);
	
	return(NULL);
}


si1	*str_match_line_end_m10(si1 *pattern, si1 *buffer)
{
	// returns pointer to beginning of the line following the line with first match, NULL if no match (assumes both pattern & buffer are zero-terminated)
	
	buffer = str_match_end_m10(pattern, buffer);
	if (buffer == NULL)
		return(NULL);
	
	while (*buffer != '\n' && *buffer != '\r' && *buffer != 0)
		++buffer;
	if (*buffer == 0)
		return(NULL);
	
	while (*buffer == '\n' || *buffer != '\r')
		++buffer;
	
	if (*buffer == 0)
		return(NULL);
	
	return(buffer);
}


si1	*str_match_line_start_m10(si1 *pattern, si1 *buffer)
{
	si1	*buf_p;
	
	
	// returns pointer to beginning of the line containing the first match, NULL if no match (assumes both pattern & buffer are zero-terminated)

	buf_p = str_match_start_m10(pattern, buffer);
	if (buf_p == NULL)
		return(NULL);
	
	while (*buf_p != '\n' && buf_p != buffer)
		--buf_p;
	
	if (buf_p == buffer)
		return(buffer);
	
	return(++buf_p);
}


si1	*str_match_start_m10(si1 *pattern, si1 *buffer)
{
	si4	pat_len, buf_len;
	si1	*pat_p, *buf_p;
	
	
	// returns pointer to beginning of the first match in the buffer, NULL if no match (assumes both pattern & buffer are zero-terminated)

	pat_len = strlen(pattern);
	buf_len = strlen(buffer);
	if (pat_len > buf_len)
		return(NULL);
	
	do {
		pat_p = pattern;
		buf_p = buffer++;
		while (*buf_p++ == *pat_p++)
			if (!*pat_p)
				return(--buffer);
	} while (*buf_p);
	
	return(NULL);
}


inline void    str_replace_char_m10(si1 c, si1 new_c, si1 *buffer)
{
	// Note: does not handle UTF8 chars
	// Done in place
	
	if (buffer == NULL || c == 0)
		return;
	
	do {
		if (*buffer == c)
			*buffer = new_c;
	} while (*buffer++);
	
	return;
}


si1	*str_replace_pattern_m10(si1 *pattern, si1 *new_pattern, si1 *buffer, TERN_m10 free_input_buffer)
{
	si1	*c, *last_c, *new_c, *c2, *new_buffer = NULL;
	si4	char_diff, extra_chars, matches;
	si8	len, pat_len, new_pat_len;
	
	
	if (pattern == NULL || new_pattern == NULL || buffer == NULL)
		return(buffer);
	if (*pattern == 0 || *buffer == 0)
		return(buffer);
	
	pat_len = strlen(pattern);
	new_pat_len = strlen(new_pattern);
	char_diff = new_pat_len - pat_len;
	
	matches = 0;
	c = buffer;
	while (1) {
		c = str_match_end_m10(pattern, c);
		if (c == NULL)
			break;
		++matches;
	}
	if (!matches)
		return(buffer);
	
	extra_chars = matches * char_diff;
	len = strlen(buffer) + extra_chars + 1;  // extra byte for terminal zero
	new_buffer = (si1 *) calloc_m10((size_t)len, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	strcpy(new_buffer, buffer);
	
	last_c = c = buffer;
	new_c = new_buffer;
	extra_chars = 0;
	while (1) {
		c = str_match_start_m10(pattern, c);
		if (c == NULL)
			break;
		while (last_c < c)
			*new_c++ = *last_c++;
		c2 = new_pattern;
		while (*c2)
			*new_c++ = *c2++;
		last_c = (c += pat_len);
	}
	while (*last_c)
		*new_c++ = *last_c++;
	
	if (free_input_buffer == TRUE_m10)
		free((void *) buffer);
	
	return(new_buffer);
}


void    strip_character_m10(si1 *s, si1 character)
{
	si1	*c1, *c2;
	
	
	c1 = c2 = s;
	while (*c2) {
		if (*c2 == character) {
			++c2;
			continue;
		}
		*c1++ = *c2++;
	}
	*c1 = 0;
	
	return;
}


void	strtolower_m10(si1 *s)
{
	--s;
	while (*++s) {
		if (*s > 40 && *s < 91)
			*s += 32;
	}
	
	return;
}


void	strtotitle_m10(si1 *s)
{
	TERN_m10	cap_mode;
	
	
	// make all lower case
	strtolower_m10(s);
	
	// capitalize first letter regardless of word
	if (*s > 96 && *s < 123)
		*s -= 32;
	
	cap_mode = FALSE_m10;
	while (*++s) {
		if (*s < 97 || *s > 122) {  // not a lower case letter
			if (*s == 32)  // space
				cap_mode = TRUE_m10;
			continue;
		}
		if (cap_mode == TRUE_m10) {
			switch (*s) {  // not exhaustive, but covers most cases
				case 'a':
					if (strncmp(s, "a ", 2) == 0) {
						++s;
						continue;
					}
					if (strncmp(s, "an ", 3) == 0) {
						s += 2;
						continue;
					}
					if (strncmp(s, "and ", 4) == 0) {
						s += 3;
						continue;
					}
					break;
				case 'b':
					if (strncmp(s, "but ", 4) == 0) {
						s += 3;
						continue;
					}
					if (strncmp(s, "by ", 3) == 0) {
						s += 2;
						continue;
					}
					break;
				case 'f':
					if (strncmp(s, "for ", 4) == 0) {
						s += 3;
						continue;
					}
					if (strncmp(s, "from ", 5) == 0) {
						s += 4;
						continue;
					}
					break;
				case 'i':
					if (strncmp(s, "if ", 3) == 0) {
						s += 2;
						continue;
					}
					break;
				case 'o':
					if (strncmp(s, "of ", 3) == 0) {
						s += 2;
						continue;
					}
					break;
				case 't':
					if (strncmp(s, "the ", 4) == 0) {
						s += 3;
						continue;
					}
					if (strncmp(s, "to ", 3) == 0) {
						s += 2;
						continue;
					}
					break;
				case 'w':
					if (strncmp(s, "with ", 5) == 0) {
						s += 4;
						continue;
					}
					if (strncmp(s, "within ", 7) == 0) {
						s += 6;
						continue;
					}
					if (strncmp(s, "without ", 8) == 0) {
						s += 7;
						continue;
					}
					break;
			}
			*s -= 32;
			cap_mode = FALSE_m10;
		}
	}
	
	return;
}


void	strtoupper_m10(si1 *s)
{
	--s;
	while (*++s) {
		if (*s > 96 && *s < 123)
			*s -= 32;
	}
	
	return;
}


si1	*time_string_m10(si8 uutc, si1 *time_str, TERN_m10 fixed_width, TERN_m10 relative_days, si4 colored_text, ...)  // time_str buffer sould be of length TIME_STRING_BYTES_m10
{
	si1			*standard_timezone_acronym, *standard_timezone_string, *date_color, *time_color, *color_reset, *meridian;
	static si1      	private_time_str[TIME_STRING_BYTES_m10];
	static si1      	*mos[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	static si1      	*months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	static si1      	*wdays[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static si1      	*mday_num_sufs[32] = {	"", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", \
							"th", "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "st" };
	static si1      	*weekdays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	TERN_m10		offset;
	si4             	microseconds, DST_offset, day_num;
	time_t             	local_time, test_time;
	sf8             	UTC_offset_hours;
	va_list         	arg_p;
	struct tm       	ti;
	LOCATION_INFO_m10	loc_info = {0};
	
	
	// Note if NULL is passed for time_str, this function is not thread-safe
	if (time_str == NULL)
		time_str = private_time_str;
	
	switch (uutc) {
		case UUTC_NO_ENTRY_m10:
			strcpy(time_str, "no entry");
			return(time_str);
		case BEGINNING_OF_TIME_m10:
			strcpy(time_str, "beginning of time");
			return(time_str);
		case END_OF_TIME_m10:
			strcpy(time_str, "end of time");
			return(time_str);
		case CURRENT_TIME_m10:
			uutc = current_uutc_m10();
			if (globals_m10->time_constants_set == FALSE_m10)  // set global time constants to location of machine
				if (get_location_info_m10(&loc_info, TRUE_m10, FALSE_m10) == NULL)
					warning_message_m10("%s(): daylight change data not available\n", __FUNCTION__);
			break;
	}
	
	if (globals_m10->RTO_known == TRUE_m10) {
		test_time = uutc - globals_m10->recording_time_offset;
		if (test_time < 0)  // time is offset
			uutc += globals_m10->recording_time_offset;
		offset = FALSE_m10;
	}
	else {
		offset = TRUE_m10;
	}
	DST_offset = DST_offset_m10(uutc);
	
	standard_timezone_acronym = globals_m10->standard_timezone_acronym;
	standard_timezone_string = globals_m10->standard_timezone_string;
	local_time = (si4) (uutc / (si8) 1000000) + DST_offset + globals_m10->standard_UTC_offset;
	microseconds = (si4) (uutc % (si8) 1000000);
#if defined MACOS_m10 || defined LINUX_m10
	gmtime_r(&local_time, &ti);
#endif
#ifdef WINDOWS_m10
	ti = *(gmtime(&local_time));
#endif
	ti.tm_year += 1900;
	
	if (colored_text == TRUE_m10) {
		va_start(arg_p, colored_text);
		date_color = va_arg(arg_p, si1 *);
		time_color = va_arg(arg_p, si1 *);
		va_end(arg_p);
		color_reset = TC_RESET_m10;
	}
	else {
		date_color = time_color = color_reset = "";
	}
	if (relative_days == TRUE_m10) {
		uutc -= globals_m10->recording_time_offset;
		day_num = (si4)(uutc / TWENTY_FOURS_HOURS_m10) + 1;
	}
	
	if (fixed_width == TRUE_m10) {
		UTC_offset_hours = (sf8)(DST_offset + globals_m10->standard_UTC_offset) / (sf8)3600.0;
		if (relative_days == TRUE_m10)
			sprintf_m10(time_str, "%sDay %04d  %s%02d:%02d:%02d.%06d", date_color, day_num, time_color, ti.tm_hour, ti.tm_min, ti.tm_sec, microseconds);
		else
			sprintf_m10(time_str, "%s%s %02d %s %d  %s%02d:%02d:%02d.%06d", date_color, wdays[ti.tm_wday], ti.tm_mday, mos[ti.tm_mon], ti.tm_year, time_color, ti.tm_hour, ti.tm_min, ti.tm_sec, microseconds);
		if (DST_offset) {
			if (UTC_offset_hours >= 0.0)
				sprintf_m10(time_str, "%s %s (UTC +%0.2lf)%s", time_str, globals_m10->daylight_timezone_acronym, UTC_offset_hours, color_reset);
			else
				sprintf_m10(time_str, "%s %s (UTC %0.2lf)%s", time_str, globals_m10->daylight_timezone_acronym, UTC_offset_hours, color_reset);
		}
		else {
			if (offset == TRUE_m10)  // no UTC offset displayed
				sprintf_m10(time_str, "%s %s%s", time_str, standard_timezone_acronym, color_reset);
			else if (UTC_offset_hours >= 0.0)
				sprintf_m10(time_str, "%s %s (UTC +%0.2lf)%s", time_str, standard_timezone_acronym, UTC_offset_hours, color_reset);
			else
				sprintf_m10(time_str, "%s %s (UTC %0.2lf)%s", time_str, standard_timezone_acronym, UTC_offset_hours, color_reset);
		}
	}
	else {
		ti.tm_sec += ((microseconds + 5e5) / 1e6);  // round to nearest second
		if (ti.tm_hour < 12) {
			meridian = "AM";
			if (ti.tm_hour == 0)
				ti.tm_hour = 12;
		}
		else {
			meridian = "PM";
			if (ti.tm_hour > 12)
				ti.tm_hour -= 12;
		}
		if (relative_days == TRUE_m10)
			sprintf_m10(time_str, "%sDay %04d  %s%d:%02d:%02d %s,", date_color, day_num, time_color, ti.tm_hour, ti.tm_min, ti.tm_sec, meridian);
		else
			sprintf_m10(time_str, "%s%s, %s %d%s, %d  %s%d:%02d:%02d %s,", date_color, weekdays[ti.tm_wday], months[ti.tm_mon], ti.tm_mday, mday_num_sufs[ti.tm_mday], ti.tm_year, time_color, ti.tm_hour, ti.tm_min, ti.tm_sec, meridian);
		if (DST_offset)
			sprintf_m10(time_str, "%s %s%s", time_str, globals_m10->daylight_timezone_string, color_reset);
		else
			sprintf_m10(time_str, "%s %s%s", time_str, standard_timezone_string, color_reset);
	}
	
	return(time_str);
}


si8     ts_sort_m10(si4 *x, si8 len, NODE_m10 *nodes, NODE_m10 *head, NODE_m10 *tail, si4 return_sorted_ts, ...)
{
	TERN_m10        free_nodes;
	NODE_m10	*last_node, *next_node, *prev_node, *np;
	si8             i, j, n_nodes;
	sf8             new_val;
	si4		*sorted_x;
	va_list         args;
	
	
	// setup
	if (nodes == NULL) {
		nodes = (NODE_m10 *) calloc_m10((size_t)len, sizeof(NODE_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_nodes = TRUE_m10;
	}
	else {
		free_nodes = FALSE_m10;
	}
	
	np = nodes;
	head->next = tail;
	head->val = (si4) NAN_m10;  // This is 0x80000000, but NEGATIVE_INFINITY_m10 (0x80000001) could theoretically fail
	tail->val = (si4) POSITIVE_INFINITY_m10;  // This is 0x7FFFFFFF
	tail->prev = head;
	
	// build linked list
	for (last_node = head, i = len; i--;) {
		new_val = *x++;
		if (new_val == last_node->val) {
			++last_node->count;
			continue;
		}
		else if (new_val > last_node->val) {
			for (next_node = last_node->next; new_val > next_node->val; next_node = next_node->next);
			if (new_val == next_node->val) {
				++next_node->count;
				last_node = next_node;
				continue;
			}
			prev_node = next_node->prev;
		}
		else {  // new_val < last_node->val
			for (prev_node = last_node->prev; new_val < prev_node->val; prev_node = prev_node->prev);
			if (new_val == prev_node->val) {
				++prev_node->count;
				last_node = prev_node;
				continue;
			}
			next_node = prev_node->next;
		}
		np->next = next_node;
		np->prev = prev_node;
		np->val = new_val;
		np->count = 1;
		last_node = prev_node->next = next_node->prev = np++;
	}
	n_nodes = np - nodes;
	
	// expand nodes back to sorted array, if requested
	if (return_sorted_ts == TRUE_m10) {
		va_start(args, return_sorted_ts);
		sorted_x = va_arg(args, si4 *);
		va_end(args);
		if (sorted_x == NULL) {
			warning_message_m10("%s(): passed sorted array pointer is NULL\n", __FUNCTION__);
		}
		else {
			for (i = n_nodes, np = head->next; i--; np = np->next)
				for (j = np->count; j--;)
					*sorted_x++ = np->val;
		}
	}
	
	if (free_nodes == TRUE_m10)
		free((void *) nodes);
	
	return(n_nodes);
}


void    unescape_spaces_m10(si1 *string)
{
	si1	*c1, *c2;
	
	
	c1 = c2 = string;
	while (*c1) {
		if (*c1 == 0x5c) {  // backslash
			if (*(c1 + 1) == 0x20) {
				++c1;
				continue;
			}
		}
		*c2++ = *c1++;
	}
	*c2 = 0;
	
	return;
}


si8     uutc_for_sample_number_m10(si8 ref_sample_number, si8 ref_uutc, si8 target_sample_number, sf8 sampling_frequency, FILE_PROCESSING_STRUCT_m10 *time_series_indices_fps, ui1 mode)
{
	TERN_m10                absolute_numbering_flag = TRUE_m10;
	si8                     uutc, i, tmp_si8, n_inds;
	sf8                     tmp_sf8;
	TIME_SERIES_INDEX_m10	*tsi;
	
	
	// uutc_for_sample_number_m10(ref_sample_number, ref_uutc, target_sample_number, sampling_frequency, NULL, mode)
	// returns uutc extrapolated from ref_uutc with sample numbering in context of ref_sample_number & target_sample_number
	
	// uutc_for_sample_number_m10(SAMPLE_NUMBER_NO_ENTRY_m10, ref_uutc, target_sample_number, sampling_frequency, NULL, mode)
	// returns uutc extrapolated from ref_uutc, assumed to occur at sample number 0, with local (segment-relative) sample numbering
	
	// uutc_for_sample_number_m10(SAMPLE_NUMBER_NO_ENTRY_m10, UUTC_NO_ENTRY_m10, target_sample_number, sampling_frequency, time_series_indices_fps, mode)
	// returns uutc extrapolated from ref_uutc with local (segment-relative) sample numbering
	
	// uutc_for_sample_number_m10(ref_sample_number, UUTC_NO_ENTRY_m10, target_sample_number, sampling_frequency, time_series_indices_fps, mode)
	// returns uutc extrapolated from closest time series index using absolute sample numbering
	
	// sample time is defined as the period from sample onset until the next sample
	// mode FIND_START_m10 (default): first uutc >= start of target_sample_number period
	// mode FIND_END_m10: last uutc < start of next sample period
	// mode FIND_CENTER_m10: uutc closest to the center of the sample period
	
	if (ref_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10) {
		absolute_numbering_flag = FALSE_m10;  // if time series indicies passed, ref_sample_number is the absolute_sample_number relative to the channel/session
		if (time_series_indices_fps != NULL)
			ref_sample_number = 0;
	}
	
	if (time_series_indices_fps != NULL) {  // use time series indices to get ref_sample_number & ref_uutc instead if just extrapolating (accounts for discontinuities)
		tsi = time_series_indices_fps->time_series_indices;
		n_inds = time_series_indices_fps->universal_header->number_of_entries;
		if (absolute_numbering_flag == TRUE_m10)
			target_sample_number -= ref_sample_number;  // ref_sample_number contains the segment absolute_start_sample_number, in this case
		for (i = 0; i < n_inds; ++i)
			if (tsi[i].start_sample_number > target_sample_number)
				break;
		if (i == 0)  // target sample earlier than segment start => return segment start time
			return(tsi[0].start_time);
		else if (i == n_inds) // target sample later than segment end => return segment end time
			return(tsi[i - 1].start_time - 1);  // terminal index time = estimated time of the next sample (or segment end time + 1)
		
		ref_sample_number = tsi[i - 1].start_sample_number;
		ref_uutc = tsi[i - 1].start_time;
		
		// acquisition sampling frequency can vary a little => this is slightly more accurate
		if (tsi[i].file_offset > 0) {  // don't do if discontinuity
			sampling_frequency = (sf8)(tsi[i].start_sample_number - ref_sample_number);
			sampling_frequency /= ((sf8)(tsi[i].start_time - ref_uutc) / (sf8)1e6);
		}
	}
	
	tmp_sf8 = (sf8) (target_sample_number - ref_sample_number) * (sf8) 1e6;
	switch (mode) {
		case FIND_END_m10:
			tmp_sf8 = (tmp_sf8 + (sf8) 1e6) / sampling_frequency;
			tmp_si8 = (si8) tmp_sf8;
			if (tmp_sf8 == (sf8) tmp_si8)
				--tmp_si8;
			break;
		case FIND_CENTER_m10:
			tmp_si8 = (si8)(((tmp_sf8 + (sf8) 5e5) / sampling_frequency) + (sf8) 0.5);
			break;
		case FIND_START_m10:
		default:
			tmp_sf8 = tmp_sf8 / sampling_frequency;
			tmp_si8 = (si8) tmp_sf8;
			if (tmp_sf8 != (sf8)tmp_si8)
				++tmp_si8;
			break;
	}
	
	uutc = ref_uutc + tmp_si8;
	
	return(uutc);
}


TERN_m10        validate_record_data_CRCs_m10(RECORD_HEADER_m10 *record_header, si8 number_of_items)
{
	TERN_m10        valid;
	si8             i;
	
	
	valid = TRUE_m10;
	for (i = 0; i < number_of_items; ++i) {
		
		valid = CRC_validate_m10((ui1 *)record_header + RECORD_HEADER_RECORD_CRC_START_OFFSET_m10, record_header->total_record_bytes - RECORD_HEADER_RECORD_CRC_START_OFFSET_m10, record_header->record_CRC);
		if (valid == FALSE_m10)
			return(valid);
		
		record_header = (RECORD_HEADER_m10 *) ((ui1 *)record_header + record_header->total_record_bytes);
	}
	
	return(valid);
}


TERN_m10        validate_time_series_data_CRCs_m10(CMP_BLOCK_FIXED_HEADER_m10 *block_header, si8 number_of_items)
{
	TERN_m10        valid;
	si8             i;
	
	
	valid = TRUE_m10;
	for (i = 0; i < number_of_items; ++i) {
		
		valid = CRC_validate_m10((ui1 *)block_header + CMP_BLOCK_CRC_START_OFFSET_m10, block_header->total_block_bytes - CMP_BLOCK_CRC_START_OFFSET_m10, block_header->block_CRC);
		if (valid == FALSE_m10)
			return(valid);
		
		block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *)block_header + block_header->total_block_bytes);
	}
	
	return(valid);
}


void    warning_message_m10(si1 *fmt, ...)
{
	va_list		args;
	
	
	// GREEN suppressible text to stderr
	if (!(globals_m10->behavior_on_fail & SUPPRESS_WARNING_OUTPUT_m10)) {
		fprintf_m10(stderr, TC_GREEN_m10);
		va_start(args, fmt);
		UTF8_vfprintf_m10(stderr, fmt, args);
		va_end(args);
		fprintf_m10(stderr, TC_RESET_m10);
		fflush(stderr);
	}
	
	return;
}



inline si1	*wchar2char_m10(si1 *target, wchar_t *source)
{
	si1	*c, *c2;
	si8	len, wsz;
	
	// if source == target, done in place
	// if not actually ascii, results may be weird
	
	wsz = sizeof(wchar_t);  // 2 or 4 => varies by OS & compiler
	c = target;
	c2 = (si1 *) source - wsz;
	len = wcslen(source);
	
	while (len--)
		*c++ = *(c2 += wsz);  // little endian version
	*c = 0;
	
	return(target);
}


void    win_cleanup_m10(void)
{
#ifdef WINDOWS_m10
	#ifdef NEED_WIN_SOCKETS_m10
		WSACleanup();
	#endif
	
	win_reset_terminal_m10();
#endif
	return;
}
	

TERN_m10	win_initialize_terminal_m10(void)
{
#ifdef  WINDOWS_m10
	HANDLE	hOut;
	DWORD	dwOriginalOutMode, dwRequestedOutModes, dwOutMode;
	
	
	// Set output mode to handle virtual terminal sequences
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		return(FALSE_m10);

	dwOriginalOutMode = 0;
	if (!GetConsoleMode(hOut, &dwOriginalOutMode))
		return(FALSE_m10);

	dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;

	dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
	if (!SetConsoleMode(hOut, dwOutMode)) {  // failed to set both modes, try to step down mode gracefully.
	    dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	    dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
	    if (!SetConsoleMode(hOut, dwOutMode))  // Failed to set any VT mode, can't do anything here.
		    return(FALSE_m10);
	}
#endif
	return(TRUE_m10);
}


si4    win_ls_1d_to_tmp_m10(si1 **dir_strs, si4 n_dirs, TERN_m10 full_path)  // replacement for unix "ls -1d > temp_file (on a directory list)"
{
#ifdef WINDOWS_m10
	si1			*file_name, *dir_name, enclosing_directory[FULL_FILE_NAME_BYTES_m10], tmp_dir[FULL_FILE_NAME_BYTES_m10];
	ui4			fe;
	si4			i, n_files;
	WIN32_FIND_DATAA 	ffd;
	HANDLE 		        find_h;
	FILE			*fp;
	
	
	// returns number of files or -1 for error
	// dir_strs can include "*" & "?" regex characters
	
	if (dir_strs == NULL)
		return(-1);
	if (dir_strs[0] == NULL)
		return(-1);
	if (n_dirs < 1)
		return(-1);
	
	// open temp file
	fp = fopen_m10(globals_m10->temp_file, "w", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	find_h = INVALID_HANDLE_VALUE;
	n_files = 0;
	for (i = 0; i < n_dirs; ++i) {
		dir_name = dir_strs[i];
		if (str_contains_regex_m10(dir_name) == FALSE_m10) {
			fe = file_exists_m10(dir_name);
			// a plain directory name will not list it's contents => must append "\*"
			if (fe == DIR_EXISTS_m10) {
				sprintf(tmp_dir, "%s\\*", dir_name);
				dir_name = tmp_dir;
			}
			else if (fe == DOES_NOT_EXIST_m10)
				continue;
			// regular files will list
		}
		find_h = FindFirstFileA((LPCSTR) dir_name, &ffd);
		if (find_h == INVALID_HANDLE_VALUE)
			continue;
		if (full_path == TRUE_m10)
			extract_path_parts_m10(dir_name, enclosing_directory, NULL, NULL);
		do {
			file_name = ffd.cFileName;
			// exclude files/directories starting with "$"
			if (*file_name == '$')
				continue;
			// exclude ".", "..", & files/directories staring with "._"
			// invisible files (".<file_name>") are not excluded
			if (*file_name == '.') {
				if (file_name[1] == 0 || file_name[1] == '.' || file_name[1] == '_')
					continue;
			}
			++n_files;
			if (full_path == TRUE_m10)
				fprintf_m10(fp, "%s\\%s\n", enclosing_directory, file_name);
			else
				fprintf_m10(fp, "%s\n", file_name);
		} while (FindNextFileA(find_h, &ffd));
		
		FindClose(find_h);
	}
	
	fclose(fp);
	
	if (find_h == INVALID_HANDLE_VALUE && n_files == 0)
		return(-1);
	
	return(n_files);
#endif
	
	return(-1);
}


TERN_m10	win_reset_terminal_m10(void)
{
#ifdef  WINDOWS_m10
	HANDLE	hOut;
	DWORD	dwOriginalOutMode;
	
	
	// Set output mode to handle virtual terminal sequences
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		return(FALSE_m10);

	dwOriginalOutMode = 3;
	if (!SetConsoleMode(hOut, dwOriginalOutMode))
		return(FALSE_m10);
#endif
	return(TRUE_m10);
}


TERN_m10	win_socket_startup_m10(void)
{
#ifdef WINDOWS_m10
	WORD		wVersionRequested;
	WSADATA		wsaData;
	si4		err;
	
	
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err) {
		error_message_m10("%s(): WSAStartup failed with error: %d\n", __FUNCTION__, err);
		return(FALSE_m10);
	}
	
	// Confirm that the WinSock DLL supports 2.2.
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		error_message_m10("%s(): Could not find a usable version of Winsock.dll\n", __FUNCTION__);
		WSACleanup();
		return(FALSE_m10);
	}
	
#endif
	return(TRUE_m10);
}


inline si4	win_system_m10(si1 *command)  // Windows has a system() function which works fine, but it opens a command prompt window.
{
#ifdef WINDOWS_m10
	si1			*tmp_command;
	si1			*cmd_exe_path;
	si4			ret_val;
	si8			len;
	PROCESS_INFORMATION	process_info = {0};
	STARTUPINFOA		startup_info = {0};

	
	len = strlen(command);
	tmp_command = malloc(len + 4);
	tmp_command[0] = 0x2F;  // '/'
	tmp_command[1] = 0x63;  // 'c'
	tmp_command[2] = 0x20;  // <space>;
	memcpy(tmp_command + 3, command, len + 1);
	
	startup_info.cb = sizeof(STARTUPINFOA);
	cmd_exe_path = getenv("COMSPEC");
	_flushall();  // required for Windows system() calls, probably a good idea here too
	
	if (CreateProcessA(cmd_exe_path, tmp_command, NULL, NULL, 0, CREATE_NO_WINDOW, NULL, NULL, &startup_info, &process_info)) {
		WaitForSingleObject(process_info.hProcess, INFINITE);
		GetExitCodeProcess(process_info.hProcess, &ret_val);
		CloseHandle(process_info.hProcess);
		CloseHandle(process_info.hThread);
	}
	
	free((void *) tmp_command);
	
	return(ret_val);
#endif
	
#if defined MACOS_m10 || defined LINUX_m10
	return(-1);
#endif
}


inline void windify_file_paths_m10(si1 *target, si1 *source)
{
	TERN_m10	match_made = FALSE_m10;
	si1		*c1, *c2;


	// if target == source, or target == NULL, conversion done in place
	if (source == NULL)
		return;
	if (target == NULL)
		target = source;
	else if (target != source)
		strcpy(target, source);

	// Replace all '/' in string except if preceded by "http or "HTTP", including those not part of a path (not common & not the end of the world)
	// Note: "<white space>\" == "C:\" so don't need to handle that separately

	// try with "http"
	c1 = target;
	while ((c2 = str_match_start_m10("http", c1)) != NULL) {
		*c2 = 0;
		str_replace_char_m10('/', '\\', c1);
		*c2 = 'h';
		while (*c2 && *c2 != ' ')
			++c2;
		c1 = c2;
		match_made = TRUE_m10;
	}
	if (match_made == TRUE_m10) {
		str_replace_char_m10('/', '\\', c1);
		return;
	}
	
	// try with "HTTP"
	while ((c2 = str_match_start_m10("HTTP", c1)) != NULL) {
		*c2 = 0;
		str_replace_char_m10('/', '\\', c1);
		*c2 = 'H';
		while (*c2 && *c2 != ' ')
			++c2;
		c1 = c2;
	}
	str_replace_char_m10('/', '\\', c1);

	return;
}


inline si1	*windify_format_string_m10(si1 *fmt)
{
#ifdef WINDOWS_m10
	// changes ld, li, lo, lu, lx, lX to "ll" versions of the same
	si1	*c, *new_c, *new_fmt;
	si4	matches;
	si8	len;
	
	
	if (fmt == NULL)
		return(NULL);
	
	matches = 0;
	c = fmt;
	while (*c) {
		if (*c++ == '%') {
			// skip over numbers & periods
			while ((*c >= '0' && *c <= '9') || *c == '.')
				++c;
			if (*c == 'l') {
				switch (*++c) {
					case 'd':
					case 'i':
					case 'o':
					case 'u':
					case 'x':
					case 'X':
						++matches;
						break;
				}
			}
		}
	}
	if (!matches)
		return(fmt);
	
	len = (si8) (c - fmt) + matches + 1;  // extra byte for terminal zero
	new_fmt = (si1 *) calloc_m10((size_t) len, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	c = fmt;
	new_c = new_fmt;
	while (*c) {
		if (*c == '%') {
			*new_c++ = *c++;
			// copy numbers & periods
			while ((*c >= '0' && *c <= '9') || *c == '.')
				*new_c++ = *c++;
			if (*c == 'l') {
				*new_c++ = *c++;
				switch (*c) {
					case 'd':
					case 'i':
					case 'o':
					case 'u':
					case 'x':
					case 'X':
						*new_c++ = 'l';
						break;
				}
			}
		}
		*new_c++ = *c++;
	}
	
	return(new_fmt);
#endif
	return(fmt);
}


si8     write_file_m10(FILE_PROCESSING_STRUCT_m10 *fps, ui8 number_of_items, void *data_ptr, ui4 behavior_on_fail)
{
	ui1				*decrypted_data;
	ui4                             entry_size;
	si8                             i, out_bytes;
	RECORD_HEADER_m10		*record_header;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header, *saved_block_header;
	UNIVERSAL_HEADER_m10		*uh;
	
	
	// mutex on
	FPS_mutex_on_m10(fps);
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	// clobber file if exists and is closed, create if non-existent
	if (fps->fp == NULL) {
		if (!(fps->directives.open_mode & FPS_GENERIC_WRITE_OPEN_MODE_m10))
			fps->directives.open_mode = FPS_W_OPEN_MODE_m10;
		FPS_open_m10(fps, __FUNCTION__, __LINE__, behavior_on_fail);
	}
	
	// set pointer, if necessary
	if (data_ptr == NULL)
		data_ptr = (void *) (fps->raw_data + UNIVERSAL_HEADER_BYTES_m10);
	
	out_bytes = 0;
	uh = fps->universal_header;
	if (number_of_items == FPS_UNIVERSAL_HEADER_ONLY_m10) {
		fseek_m10(fps->fp, 0, SEEK_SET, fps->full_file_name, __FUNCTION__, __LINE__, EXIT_ON_FAIL_m10);
		out_bytes = UNIVERSAL_HEADER_BYTES_m10;
		data_ptr = (void *) fps->raw_data;
		number_of_items = 0;
	}
	else if (number_of_items == FPS_FULL_FILE_m10) {
		fseek_m10(fps->fp, 0, SEEK_SET, fps->full_file_name, __FUNCTION__, __LINE__, EXIT_ON_FAIL_m10);
		out_bytes = UNIVERSAL_HEADER_BYTES_m10;
		data_ptr = (void *) fps->raw_data;
		switch (uh->type_code) {
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
				number_of_items = uh->number_of_entries;
				break;
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
				number_of_items = uh->number_of_entries;
				uh->maximum_entry_size = TIME_SERIES_INDEX_BYTES_m10;
				break;
			case VIDEO_INDICES_FILE_TYPE_CODE_m10:
				number_of_items = uh->number_of_entries;
				uh->maximum_entry_size = VIDEO_INDEX_BYTES_m10;
				break;
			case RECORD_INDICES_FILE_TYPE_CODE_m10:
				number_of_items = uh->number_of_entries;
				uh->maximum_entry_size = RECORD_INDEX_BYTES_m10;
				break;
			case RECORD_DATA_FILE_TYPE_CODE_m10:
				number_of_items = uh->number_of_entries;
				break;
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
			case VIDEO_METADATA_FILE_TYPE_CODE_m10:
				number_of_items = 1;
				uh->maximum_entry_size = METADATA_BYTES_m10;
				break;
		}
		uh->number_of_entries = 0; // incremented by number_of_items below
	}
	
	// handle file bodies
	if (number_of_items) {
		
		// update universal header
		uh->number_of_entries += number_of_items;
		
		// calculate out_bytes
		switch (uh->type_code) {
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
				block_header = (CMP_BLOCK_FIXED_HEADER_m10 *)data_ptr;
				for (i = 0; i < number_of_items; ++i) {
					entry_size = block_header->total_block_bytes;
					if (uh->maximum_entry_size < entry_size)
						uh->maximum_entry_size = entry_size;
					out_bytes += entry_size;
					block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *)block_header + block_header->total_block_bytes);
				}
				break;
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
				out_bytes += number_of_items * TIME_SERIES_INDEX_BYTES_m10;
				break;
			case RECORD_DATA_FILE_TYPE_CODE_m10:
				record_header = (RECORD_HEADER_m10 *)data_ptr;
				for (i = 0; i < number_of_items; ++i) {
					entry_size = record_header->total_record_bytes;
					if (uh->maximum_entry_size < entry_size)
						uh->maximum_entry_size = entry_size;
					out_bytes += entry_size;
					record_header = (RECORD_HEADER_m10 *) ((ui1 *)record_header + record_header->total_record_bytes);
				}
				break;
			case RECORD_INDICES_FILE_TYPE_CODE_m10:
				out_bytes += number_of_items * RECORD_INDEX_BYTES_m10;
				break;
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
			case VIDEO_METADATA_FILE_TYPE_CODE_m10:
				out_bytes += METADATA_BYTES_m10;
				break;
		}
		
		// leave decrypted/unoffset directive
		if (fps->directives.leave_decrypted == TRUE_m10) {
			switch (uh->type_code) {
				case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:	// This mechanism assumes copying and copying back is faster than
				case VIDEO_INDICES_FILE_TYPE_CODE_m10:		// decrypting, but it might not be.  Need to check this some time.
				case RECORD_INDICES_FILE_TYPE_CODE_m10:
				case RECORD_DATA_FILE_TYPE_CODE_m10:
					decrypted_data = (ui1 *) malloc_m10(out_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
					memcpy(decrypted_data, data_ptr, out_bytes);
					break;
			}
		}
		
		// encrypt
		switch (uh->type_code) {
			case RECORD_DATA_FILE_TYPE_CODE_m10:
				encrypt_records_m10(fps, (RECORD_HEADER_m10 *)data_ptr, number_of_items);
				break;
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
			case VIDEO_METADATA_FILE_TYPE_CODE_m10:
				encrypt_metadata_m10(fps);
				break;
		}
		
		// Calculate CRCs
		if (globals_m10->CRC_mode & (CRC_CALCULATE_m10 | CRC_CALCULATE_ON_OUTPUT_m10)) {
			switch (uh->type_code) {
				case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
					calculate_time_series_data_CRCs_m10(fps, (CMP_BLOCK_FIXED_HEADER_m10 *)data_ptr, number_of_items);
					break;
				case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
					calculate_time_series_indices_CRCs_m10(fps, (TIME_SERIES_INDEX_m10 *)data_ptr, number_of_items);
					break;
				case RECORD_DATA_FILE_TYPE_CODE_m10:
					calculate_record_data_CRCs_m10(fps, (RECORD_HEADER_m10 *)data_ptr, number_of_items);
					break;
				case RECORD_INDICES_FILE_TYPE_CODE_m10:
					calculate_record_indices_CRCs_m10(fps, (RECORD_INDEX_m10 *)data_ptr, number_of_items);
					break;
				case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
				case VIDEO_METADATA_FILE_TYPE_CODE_m10:
					calculate_metadata_CRC_m10(fps);
					break;
			}
		}
	}
	
	// always update universal header on close
	if (fps->directives.close_file == TRUE_m10)
		fps->directives.update_universal_header = TRUE_m10;
	
	// write
	FPS_write_m10(fps, out_bytes, data_ptr, __FUNCTION__, __LINE__, behavior_on_fail);
	
	// leave decrypted directive
	if (fps->directives.leave_decrypted == TRUE_m10) {
		switch (uh->type_code) {
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
				// in case data_ptr is not cps block_header
				saved_block_header = fps->cps->block_header;
				fps->cps->block_header = (CMP_BLOCK_FIXED_HEADER_m10 *)data_ptr;
				decrypt_time_series_data_m10(fps->cps, number_of_items);
				fps->cps->block_header = saved_block_header;
				break;
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
			case VIDEO_INDICES_FILE_TYPE_CODE_m10:
			case RECORD_INDICES_FILE_TYPE_CODE_m10:
			case RECORD_DATA_FILE_TYPE_CODE_m10:
				memcpy(data_ptr, decrypted_data, out_bytes);
				free((void *) decrypted_data);
				break;
		}
	}
	
	// close
	if (fps->directives.close_file == TRUE_m10)
		FPS_close_m10(fps);
	
	// show
	if (globals_m10->verbose == TRUE_m10)
		show_file_processing_struct_m10(fps);
	
	// mutex off
	FPS_mutex_off_m10(fps);
	
	return(out_bytes);
}


//***********************************************************************//
//**************************  AES-128 FUNCTIONS  ************************//
//***********************************************************************//


// ATTRIBUTION
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
// Minor modifications have been made for compatibility with the MED Library.


// This function adds the round key to state.
// The round key is added to the state by an XOR function.
void	AES_add_round_key_m10(si4 round, ui1 state[][4], ui1 *round_key)
{
	si4	i, j;
	
	
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			state[j][i] ^= round_key[(round * AES_NB_m10 * 4) + (i * AES_NB_m10) + j];
	
	return;
}


// In is encrypted buffer (16 bytes / 128 bits).
// Out is decrypted buffer (16 bytes / 128 bits).
// In can equal out, i.e. can be done in place.
// Pass in expanded key externally - this is more efficient than passing the password
// if encrypting multiple times with the same encryption key
void	AES_decrypt_m10(ui1 *in, ui1 *out, si1 *password, ui1 *expanded_key)
{
	si1	key[16] = {0};
	ui1	state[4][4]; // the array that holds the intermediate results during encryption
	ui1	round_key[240]; // The array that stores the round keys
	
	
	if (globals_m10->AES_sbox_table == NULL)  // all tables initialized together
		AES_initialize_tables_m10();
	
	if (expanded_key != NULL) {
		AES_inv_cipher_m10(in, out, state, expanded_key);
	}
	else if (password != NULL) {
		// password becomes the key (16 bytes, zero-padded if shorter, truncated if longer)
		strncpy_m10(key, password, 16);
		
		//The Key-Expansion routine must be called before the decryption routine.
		AES_key_expansion_m10(round_key, key);
		
		// The next function call decrypts the CipherText with the Key using AES algorithm.
		AES_inv_cipher_m10(in, out, state, round_key);
	}
	else {
		error_message_m10("%s(): No password or expanded key\n", __FUNCTION__);
	}
	
	return;
}


// in is buffer to be encrypted (16 bytes)
// out is encrypted buffer (16 bytes)
// in can equal out, i.e. can be done in place
// Pass in expanded key externally - this is more efficient tahn passing the password
// if encrypting multiple times with the same encryption key
void	AES_encrypt_m10(ui1 *in, ui1 *out, si1 *password, ui1 *expanded_key)
{
	si1	key[16] = {0};
	ui1	state[4][4]; // the array that holds the intermediate results during encryption
	ui1	round_key[240]; // The array that stores the round keys
	
	
	if (globals_m10->AES_sbox_table == NULL)  // all tables initialized together
		AES_initialize_tables_m10();

	if (expanded_key != NULL) {
		AES_cipher_m10(in, out, state, expanded_key);
	}
	else if (password != NULL) {
		// password becomes the key (16 bytes, zero-padded if shorter, truncated if longer)
		strncpy_m10(key, password, 16);
		
		// The KeyExpansion routine must be called before encryption.
		AES_key_expansion_m10(round_key, key);
		
		// The next function call encrypts the PlainText with the Key using AES algorithm.
		AES_cipher_m10(in, out, state, round_key);
	}
	else {
		error_message_m10("%s(): No password or expanded key\n", __FUNCTION__);
	}
	
	return;
}


// This function produces AES_NB * (AES_NR + 1) round keys. The round keys are used in each round to encrypt the states.
// NOTE: make sure any terminal unused bytes in key array (password) are zeroed
void	AES_key_expansion_m10(ui1 *expanded_key, si1 *key)
{
	// The round constant word array, Rcon[i], contains the values given by
	// x to the power (i - 1) being powers of x (x is denoted as {02}) in the field GF(28)
	// Note that i starts at 1, not 0).
	si4	i, j;
	ui1	temp[4], k;
	
	
	if (globals_m10->AES_rcon_table == NULL)
		if (AES_initialize_tables_m10() == FALSE_m10) {
			error_message_m10("%s(): error\n", __FUNCTION__);
			return;
		}
	
	// The first round key is the key itself.
	for (i = j = 0; i < AES_NK_m10; i++, j += 4) {
		expanded_key[j] = key[j];
		expanded_key[j + 1] = key[j + 1];
		expanded_key[j + 2] = key[j + 2];
		expanded_key[j + 3] = key[j + 3];
	}
	
	// All other round keys are found from the previous round keys.
	while (i < (AES_NB_m10 * (AES_NR_m10 + 1))) {
		
		for (j = 0; j < 4; j++) {
			temp[j] = expanded_key[(i - 1) * 4 + j];
		}
		
		if (i % AES_NK_m10 == 0) {
			// This rotates the 4 bytes in a word to the left once.
			// [a0,a1,a2,a3] becomes [a1,a2,a3,a0]
			k = temp[0];
			temp[0] = temp[1];
			temp[1] = temp[2];
			temp[2] = temp[3];
			temp[3] = k;
			
			// This takes a four-byte input word and applies the S-box
			// to each of the four bytes to produce an output word.
			temp[0] = (ui1)AES_get_sbox_value_m10(temp[0]);
			temp[1] = (ui1)AES_get_sbox_value_m10(temp[1]);
			temp[2] = (ui1)AES_get_sbox_value_m10(temp[2]);
			temp[3] = (ui1)AES_get_sbox_value_m10(temp[3]);
			
			temp[0] = temp[0] ^ (ui1)globals_m10->AES_rcon_table[i / AES_NK_m10];
		}
		else if (AES_NK_m10 > 6 && i % AES_NK_m10 == 4) {
			// This takes a four-byte input word and applies the S-box
			// to each of the four bytes to produce an output word.
			temp[0] = (ui1)AES_get_sbox_value_m10(temp[0]);
			temp[1] = (ui1)AES_get_sbox_value_m10(temp[1]);
			temp[2] = (ui1)AES_get_sbox_value_m10(temp[2]);
			temp[3] = (ui1)AES_get_sbox_value_m10(temp[3]);
		}
		
		expanded_key[i * 4] = expanded_key[(i - AES_NK_m10) * 4] ^ temp[0];
		expanded_key[i * 4 + 1] = expanded_key[(i - AES_NK_m10) * 4 + 1] ^ temp[1];
		expanded_key[i * 4 + 2] = expanded_key[(i - AES_NK_m10) * 4 + 2] ^ temp[2];
		expanded_key[i * 4 + 3] = expanded_key[(i - AES_NK_m10) * 4 + 3] ^ temp[3];
		
		i++;
	}
	
	return;
}


// Cipher is the main function that encrypts the PlainText.
void	AES_cipher_m10(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key)
{
	si4	i, j, round = 0;
	
	
	//Copy the input PlainText to state array.
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			state[j][i] = in[i * 4 + j];
		}
	}
	
	// Add the First round key to the state before starting the rounds.
	AES_add_round_key_m10(0, state, round_key);
	
	// There will be AES_NR rounds.
	// The first AES_NR - 1 rounds are identical.
	// These AES_NR - 1 rounds are executed in the loop below.
	for (round = 1; round < AES_NR_m10; round++) {
		AES_sub_bytes_m10(state);
		AES_shift_rows_m10(state);
		AES_mix_columns_m10(state);
		AES_add_round_key_m10(round, state, round_key);
	}
	
	// The last round is given below.
	// The MixColumns function is not here in the last round.
	AES_sub_bytes_m10(state);
	AES_shift_rows_m10(state);
	AES_add_round_key_m10(AES_NR_m10, state, round_key);
	
	// The encryption process is over.
	// Copy the state array to output array.
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			out[i * 4 + j] = state[j][i];
		}
	}
	
	return;
}


inline si4	AES_get_sbox_invert_m10(si4 num)
{
	if (globals_m10->AES_rsbox_table == NULL)
		if (AES_initialize_tables_m10() == FALSE_m10) {
			error_message_m10("%s(): error\n", __FUNCTION__);
			return(-1);
		}
	
	return(globals_m10->AES_rsbox_table[num]);
}


inline si4	AES_get_sbox_value_m10(si4 num)
{
	if (globals_m10->AES_sbox_table == NULL)
		if (AES_initialize_tables_m10() == FALSE_m10) {
			error_message_m10("%s(): error\n", __FUNCTION__);
			return(-1);
		}
	
	return(globals_m10->AES_sbox_table[num]);
}


TERN_m10	AES_initialize_tables_m10(void)
{
	// rcon table
	if (globals_m10->AES_rcon_table != NULL)
		free((void *) globals_m10->AES_rcon_table);
	globals_m10->AES_rcon_table = (si4*) calloc_m10((size_t)AES_RCON_ENTRIES_m10, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	{
		si4 temp[AES_RCON_ENTRIES_m10] = AES_RCON_m10;
		memcpy(globals_m10->AES_rcon_table, temp, AES_RCON_ENTRIES_m10 * sizeof(si4));
	}
	
	// rsbox table
	if (globals_m10->AES_rsbox_table != NULL)
		free((void *) globals_m10->AES_rsbox_table);
	globals_m10->AES_rsbox_table = (si4*) calloc_m10((size_t)AES_RSBOX_ENTRIES_m10, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	{
		si4 temp[AES_RSBOX_ENTRIES_m10] = AES_RSBOX_m10;
		memcpy(globals_m10->AES_rsbox_table, temp, AES_RSBOX_ENTRIES_m10 * sizeof(si4));
	}
	
	// sbox table
	if (globals_m10->AES_sbox_table != NULL)
		free((void *) globals_m10->AES_sbox_table);
	globals_m10->AES_sbox_table = (si4*) calloc_m10((size_t)AES_SBOX_ENTRIES_m10, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	{
		si4 temp[AES_SBOX_ENTRIES_m10] = AES_SBOX_m10;
		memcpy(globals_m10->AES_sbox_table, temp, AES_SBOX_ENTRIES_m10 * sizeof(si4));
	}
	
	return(TRUE_m10);
}


// AES_inv_cipher is the main decryption function
void	AES_inv_cipher_m10(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key)
{
	si4	i, j, round = 0;
	
	
	// Copy the input encrypted text to state array.
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			state[j][i] = in[i * 4 + j];
		}
	}
	
	// Add the First round key to the state before starting the rounds.
	AES_add_round_key_m10(AES_NR_m10, state, round_key);
	
	// There will be AES_NR rounds.
	// The first AES_NR - 1 rounds are identical.
	// These AES_NR - 1 rounds are executed in the loop below.
	for (round = AES_NR_m10 - 1; round > 0; round--) {
		AES_inv_shift_rows_m10(state);
		AES_inv_sub_bytes_m10(state);
		AES_add_round_key_m10(round, state, round_key);
		AES_inv_mix_columns_m10(state);
	}
	
	// The last round is given below.
	// The MixColumns function is not here in the last round.
	AES_inv_shift_rows_m10(state);
	AES_inv_sub_bytes_m10(state);
	AES_add_round_key_m10(0, state, round_key);
	
	// The decryption process is over.
	// Copy the state array to output array.
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			out[i * 4 + j] = state[j][i];
		}
	}
	
	return;
}


// The method used to multiply may be difficult to understand.
// Please use the references to gain more information.
void	AES_inv_mix_columns_m10(ui1 state[][4])
{
	si4	i;
	ui1	a, b, c, d;
	
	
	for (i = 0; i < 4; i++) {
		a = state[0][i];
		b = state[1][i];
		c = state[2][i];
		d = state[3][i];
		state[0][i] = AES_MULTIPLY_m10(a, 0x0e) ^ AES_MULTIPLY_m10(b, 0x0b) ^ AES_MULTIPLY_m10(c, 0x0d) ^ AES_MULTIPLY_m10(d, 0x09);
		state[1][i] = AES_MULTIPLY_m10(a, 0x09) ^ AES_MULTIPLY_m10(b, 0x0e) ^ AES_MULTIPLY_m10(c, 0x0b) ^ AES_MULTIPLY_m10(d, 0x0d);
		state[2][i] = AES_MULTIPLY_m10(a, 0x0d) ^ AES_MULTIPLY_m10(b, 0x09) ^ AES_MULTIPLY_m10(c, 0x0e) ^ AES_MULTIPLY_m10(d, 0x0b);
		state[3][i] = AES_MULTIPLY_m10(a, 0x0b) ^ AES_MULTIPLY_m10(b, 0x0d) ^ AES_MULTIPLY_m10(c, 0x09) ^ AES_MULTIPLY_m10(d, 0x0e);
	}
	
	return;
}


void	AES_inv_shift_rows_m10(ui1 state[][4])
{
	ui1	temp;
	
	
	// Rotate first row 1 columns to right
	temp = state[1][3];
	state[1][3] = state[1][2];
	state[1][2] = state[1][1];
	state[1][1] = state[1][0];
	state[1][0] = temp;
	
	// Rotate second row 2 columns to right
	temp = state[2][0];
	state[2][0] = state[2][2];
	state[2][2] = temp;
	
	temp = state[2][1];
	state[2][1] = state[2][3];
	state[2][3] = temp;
	
	// Rotate third row 3 columns to right
	temp = state[3][0];
	state[3][0] = state[3][1];
	state[3][1] = state[3][2];
	state[3][2] = state[3][3];
	state[3][3] = temp;
	
	return;
}


void	AES_inv_sub_bytes_m10(ui1 state[][4])
{
	si4	i, j;
	
	
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			state[i][j] = (ui1)AES_get_sbox_invert_m10(state[i][j]);
		}
	}
	
	return;
}


// MixColumns function mixes the columns of the state matrix
// The method used may look complicated, but it is easy if you know the underlying theory.
// Refer the documents specified above.
void	AES_mix_columns_m10(ui1 state[][4])
{
	si4	i;
	ui1	Tmp, Tm, t;
	
	
	for (i = 0; i < 4; i++) {
		t = state[0][i];
		Tmp = state[0][i] ^ state[1][i] ^ state[2][i] ^ state[3][i];
		Tm = state[0][i] ^ state[1][i];
		Tm = AES_XTIME_m10(Tm);
		state[0][i] ^= Tm ^ Tmp;
		Tm = state[1][i] ^ state[2][i];
		Tm = AES_XTIME_m10(Tm);
		state[1][i] ^= Tm ^ Tmp;
		Tm = state[2][i] ^ state[3][i];
		Tm = AES_XTIME_m10(Tm);
		state[2][i] ^= Tm ^ Tmp;
		Tm = state[3][i] ^ t;
		Tm = AES_XTIME_m10(Tm);
		state[3][i] ^= Tm ^ Tmp;
	}
	
	return;
}


// The ShiftRows() function shifts the rows in the state to the left.
// Each row is shifted with different offset.
// Offset = Row number. So the first row is not shifted.
void	AES_shift_rows_m10(ui1 state[][4])
{
	ui1	temp;
	
	
	// Rotate first row 1 columns to left
	temp = state[1][0];
	state[1][0] = state[1][1];
	state[1][1] = state[1][2];
	state[1][2] = state[1][3];
	state[1][3] = temp;
	
	// Rotate second row 2 columns to left
	temp = state[2][0];
	state[2][0] = state[2][2];
	state[2][2] = temp;
	
	temp = state[2][1];
	state[2][1] = state[2][3];
	state[2][3] = temp;
	
	// Rotate third row 3 columns to left
	temp = state[3][0];
	state[3][0] = state[3][3];
	state[3][3] = state[3][2];
	state[3][2] = state[3][1];
	state[3][1] = temp;
	
	return;
}


// The SubBytes Function Substitutes the values in the
// state matrix with values in an S-box.
void	AES_sub_bytes_m10(ui1 state[][4])
{
	si4	i, j;
	
	
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			state[i][j] = (ui1)AES_get_sbox_value_m10(state[i][j]);
		}
	}
	
	return;
}



//***********************************************************************//
//****************************  CMP FUNCTIONS  **************************//
//***********************************************************************//


CMP_PROCESSING_STRUCT_m10	*CMP_allocate_processing_struct_m10(CMP_PROCESSING_STRUCT_m10 *cps, ui4 mode, si8 data_samples, si8 compressed_data_bytes, si8 difference_bytes, ui4 block_samples, CMP_DIRECTIVES_m10 *directives, CMP_PARAMETERS_m10 *parameters)
{
	TERN_m10	need_compressed_data = FALSE_m10;
	TERN_m10	need_decompressed_data = FALSE_m10;
	TERN_m10	need_original_data = FALSE_m10;
	TERN_m10	need_difference_buffer = FALSE_m10;
	TERN_m10	need_detrended_buffer = FALSE_m10;
	TERN_m10	need_derivative_buffer = FALSE_m10;
	TERN_m10	need_scaled_amplitude_buffer = FALSE_m10;
	TERN_m10	need_scaled_frequency_buffer = FALSE_m10;
	si8		num_cats, derivative_bytes;
	
	
	if (cps == NULL)
		cps = (CMP_PROCESSING_STRUCT_m10 *) calloc_m10((size_t)1, sizeof(CMP_PROCESSING_STRUCT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	cps->mutex = FALSE_m10;
	if (cps->password_data == NULL)
		cps->password_data = &globals_m10->password_data;
	
	if (mode == CMP_COMPRESSION_MODE_NO_ENTRY_m10) {
		error_message_m10("%s(): No compression mode specified\n", __FUNCTION__);
		exit_m10(1);
	}
	
	// set up directives
	if (directives != NULL)
		cps->directives = *directives;
	else // set defaults
		CMP_initialize_directives_m10(&cps->directives, (ui1)mode);
	
	// set up parameters
	if (parameters != NULL)
		cps->parameters = *parameters;
	else  // set defaults
		CMP_initialize_parameters_m10(&cps->parameters);
	
	// initialize buffers
	cps->input_buffer = cps->original_data = cps->original_ptr = NULL;
	cps->block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) (cps->compressed_data = NULL);
	cps->decompressed_ptr = cps->decompressed_data = NULL;
	cps->difference_buffer = NULL;
	cps->detrended_buffer = NULL;
	cps->derivative_buffer = NULL;
	cps->scaled_amplitude_buffer = NULL;
	cps->scaled_frequency_buffer = NULL;
	cps->count = NULL;
	cps->cumulative_count = NULL;
	cps->sorted_count = NULL;
	cps->minimum_range = NULL;
	cps->symbol_map = NULL;
	
	// allocate RED/PRED buffers
	if (cps->directives.algorithm == CMP_RED_COMPRESSION_m10 || cps->directives.algorithm == CMP_PRED_COMPRESSION_m10) {
		if (cps->directives.algorithm == CMP_PRED_COMPRESSION_m10)
			num_cats = CMP_PRED_CATS_m10;
		else
			num_cats = 1;
		if (cps->directives.mode == CMP_COMPRESSION_MODE_m10) {
			cps->count = (ui4 **) calloc_2D_m10((size_t) num_cats, CMP_RED_MAX_STATS_BINS_m10, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
			cps->sorted_count = (CMP_STATISTICS_BIN_m10 **) calloc_2D_m10((size_t) num_cats, CMP_RED_MAX_STATS_BINS_m10, sizeof(CMP_STATISTICS_BIN_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
			cps->symbol_map = (ui1 **) calloc_2D_m10((size_t) num_cats, CMP_RED_MAX_STATS_BINS_m10, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		}
		cps->cumulative_count = (ui8 **) calloc_2D_m10((size_t) num_cats, CMP_RED_MAX_STATS_BINS_m10 + 1, sizeof(ui8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		cps->minimum_range = (ui8 **) calloc_2D_m10((size_t) num_cats, CMP_RED_MAX_STATS_BINS_m10, sizeof(ui8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	}
	
	// decompression
	if (cps->directives.mode == CMP_DECOMPRESSION_MODE_m10) {
		need_compressed_data = TRUE_m10;
		need_decompressed_data = TRUE_m10;
		need_difference_buffer = TRUE_m10;
	}
	
	// compression
	else {
		need_compressed_data = TRUE_m10;
		need_original_data = TRUE_m10;
		need_difference_buffer = TRUE_m10;
		
		if (cps->directives.detrend_data == TRUE_m10)
			need_detrended_buffer = TRUE_m10;
		if (cps->directives.set_derivative_level == TRUE_m10 || cps->directives.find_derivative_level == TRUE_m10)
			need_derivative_buffer = TRUE_m10;
		if (cps->directives.set_amplitude_scale == TRUE_m10 || cps->directives.find_amplitude_scale == TRUE_m10)
			need_scaled_amplitude_buffer = TRUE_m10;
		if (cps->directives.set_frequency_scale == TRUE_m10 || cps->directives.find_frequency_scale == TRUE_m10)
			need_scaled_frequency_buffer = TRUE_m10;
		if (cps->directives.find_amplitude_scale == TRUE_m10 || cps->directives.find_frequency_scale == TRUE_m10)
			need_decompressed_data = TRUE_m10;
	}
	
	// original_data - caller specified array size
	if (need_original_data == TRUE_m10 && cps->original_data == NULL)
		cps->input_buffer = cps->original_ptr = cps->original_data = (si4*) calloc_m10((size_t)data_samples, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// compressed_data - caller specified array size
	if (need_compressed_data == TRUE_m10 && cps->compressed_data == NULL)
		cps->block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) (cps->compressed_data = (ui1 *) calloc_m10((size_t)compressed_data_bytes, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10));
	
	// difference_buffer - caller specified or maximum bytes required for specified block size
	if (difference_bytes == 0)
		difference_bytes = CMP_MAX_DIFFERENCE_BYTES_m10(block_samples);
	if (need_difference_buffer == TRUE_m10 && cps->difference_buffer == NULL)
		cps->difference_buffer = (si1 *) calloc_m10((size_t)difference_bytes, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// decompressed_data - caller specified array size
	if (need_decompressed_data == TRUE_m10 && cps->decompressed_data == NULL)
		cps->decompressed_data = cps->decompressed_ptr = (si4*) calloc_m10((size_t)data_samples, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// detrended_buffer - maximum bytes required for caller specified block size
	if (need_detrended_buffer == TRUE_m10 && cps->detrended_buffer == NULL)
		cps->detrended_buffer = (si4*) calloc_m10((size_t)block_samples, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// derivative_buffer - maximum bytes required for caller specified block size
	if (need_derivative_buffer == TRUE_m10 && cps->derivative_buffer == NULL) {
		derivative_bytes = (si8)CMP_MAX_DIFFERENCE_BYTES_m10(block_samples);
		cps->derivative_buffer = (si1 *) calloc_m10((size_t)derivative_bytes, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	}
	
	// scaled_amplitude_buffer - maximum bytes required for caller specified block size
	if (need_scaled_amplitude_buffer == TRUE_m10 && cps->scaled_amplitude_buffer == NULL)
		cps->scaled_amplitude_buffer = (si4*) calloc_m10((size_t)block_samples, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// scaled_frequency_buffer - maximum bytes required for caller specified block size
	if (need_scaled_frequency_buffer == TRUE_m10 && cps->scaled_frequency_buffer == NULL)
		cps->scaled_frequency_buffer = (si4*) calloc_m10((size_t)block_samples, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	return(cps);
}


inline sf8      CMP_calculate_mean_residual_ratio_m10(si4 *original_data, si4 *lossy_data, ui4 n_samps)
{
	sf8        sum, mrr, diff, r;
	si8        i;
	
	
	sum = (sf8)0.0;
	for (i = n_samps; i--;) {
		if (*original_data) {
			diff = (sf8)(*original_data - *lossy_data++);
			r = diff / (sf8)*original_data++;
			sum += ABS_m10(r);
		}
		else {
			--n_samps;
			++original_data;
			++lossy_data;
		}
	}
	
	if (sum == (sf8)0.0)
		mrr = (sf8)0.0;
	else
		mrr = sum / (sf8)n_samps;
	
	return(mrr);
}


void    CMP_calculate_statistics_m10(void *stats_ptr, si4 *input_buffer, si8 len, NODE_m10 *nodes)
{
	NODE_m10		*np, head, tail;
	TERN_m10		free_nodes;
	REC_Stat_v10_m10	*stats;
	si4			*x;
	sf16            	sum_x, n, dm, t, sdm2, sdm3, sdm4, m1, m2, m3, m4;
	sf8             	true_median;
	si8             	i, n_nodes, mid_idx, max_cnt, running_cnt;
	ui1             	median_found;
	
	
	// cast
	stats = (REC_Stat_v10_m10 *) stats_ptr;
	
	// allocate
	if (nodes == NULL) {
		nodes = (NODE_m10 *) calloc_m10((size_t)len, sizeof(NODE_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_nodes = TRUE_m10;
	}
	else {
		free_nodes = FALSE_m10;
	}
	
	// sort
	x = input_buffer;
	n_nodes = ts_sort_m10(x, len, nodes, &head, &tail, FALSE_m10);
	
	// min, max, mean, median, & mode
	sum_x = (sf16)0.0;
	running_cnt = max_cnt = 0;
	mid_idx = len >> 1;
	median_found = 0;
	for (i = n_nodes, np = head.next; i--; np = np->next) {
		sum_x += (sf16)np->val * (sf16)np->count;
		if (np->count > max_cnt) {
			max_cnt = np->count;
			stats->mode = np->val;
		}
		if (median_found == 0) {
			running_cnt += np->count;
			if (running_cnt >= mid_idx) {
				if (running_cnt == mid_idx) {
					true_median = (sf8)np->val + (sf8)np->next->val;
					stats->median = CMP_round_m10(true_median);
				}
				else {
					stats->median = np->val;
				}
				median_found = 1;
			}
		}
	}
	n = (sf16)len;
	stats->minimum = head.next->val;
	stats->maximum = head.prev->val;
	m1 = sum_x / n;
	stats->mean = CMP_round_m10((sf8)m1);
	
	// variance
	sdm2 = sdm3 = sdm4 = (sf16)0.0;
	for (i = n_nodes, np = head.next; i--; np = np->next) {
		dm = (sf16)np->val - m1;
		sdm2 += (t = dm * dm * (sf16)np->count);
		sdm3 += (t *= dm);
		sdm4 += (t *= dm);
	}
	stats->variance = (sf4)(m2 = sdm2 / n);
	m3 = sdm3 / n;
	m4 = sdm4 / n;
	
	// skewness
	t = m3 / sqrtl(m2 * m2 * m2);
	if (isnan(t))
		t = (sf16)0.0;  // possible NaN here: set to zero
	else if (len > 2) // correct bias
		t *= sqrtl((n - (sf16)1) / n) * (n / (n - (sf16)2));
	stats->skewness = (sf4)t;
	
	// kurtosis
	t = m4 / (m2 * m2);
	if (len > 3) { // correct bias
		t = ((n + (sf16)1) * t) - ((sf16)3 * (n - (sf16)1));
		t *= (n - (sf16)1) / ((n - (sf16)2) * (n - (sf16)3));
		t += 3;
	}
	stats->kurtosis = (sf4)t;
	
	// clean up
	if (free_nodes == TRUE_m10)
		free((void *) nodes);
	
	return;
}


TERN_m10     CMP_check_CPS_allocation_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	TERN_m10	ret_val = TRUE_m10;
	si1		need_compressed_data = FALSE_m10;
	si1		need_decompressed_data = FALSE_m10;
	si1		need_original_data = FALSE_m10;
	si1		need_detrended_buffer = FALSE_m10;
	si1		need_derivative_buffer = FALSE_m10;
	si1		need_scaled_amplitude_buffer = FALSE_m10;
	si1		need_scaled_frequency_buffer = FALSE_m10;
	si1		need_difference_buffer = FALSE_m10;
	
	
	// decompression
	if (cps->directives.mode == CMP_DECOMPRESSION_MODE_m10) {
		need_compressed_data = TRUE_m10;
		need_decompressed_data = TRUE_m10;
		need_difference_buffer = TRUE_m10;
	}
	
	// compression
	else {
		need_compressed_data = TRUE_m10;
		need_original_data = TRUE_m10;
		need_difference_buffer = TRUE_m10;
		
		if (cps->directives.detrend_data == TRUE_m10)
			need_detrended_buffer = TRUE_m10;
		if (cps->directives.set_derivative_level == TRUE_m10 || cps->directives.find_derivative_level == TRUE_m10)
			need_derivative_buffer = TRUE_m10;
		if (cps->directives.set_amplitude_scale == TRUE_m10 || cps->directives.find_amplitude_scale == TRUE_m10)
			need_scaled_amplitude_buffer = TRUE_m10;
		if (cps->directives.set_frequency_scale == TRUE_m10 || cps->directives.find_frequency_scale == TRUE_m10)
			need_scaled_frequency_buffer = TRUE_m10;
		if (cps->directives.find_amplitude_scale == TRUE_m10 || cps->directives.find_frequency_scale == TRUE_m10)
			need_decompressed_data = TRUE_m10;
	}
	
	// check compressed_data
	if (need_compressed_data == TRUE_m10 && cps->compressed_data == NULL) {
		error_message_m10("%s(): \"compressed_data\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m10;
	}
	
	// check difference_buffer
	if (need_difference_buffer == TRUE_m10 && cps->difference_buffer == NULL) {
		error_message_m10("%s(): \"difference_buffer\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m10;
	}
	
	// check original_data
	if (need_original_data == TRUE_m10 && cps->original_data == NULL) {
		error_message_m10("%s(): \"original_data\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m10;
	}
	if (need_original_data == FALSE_m10 && cps->original_data != NULL) {
		warning_message_m10("%s(): \"original_data\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		free((void *) cps->original_data);
		cps->original_ptr = cps->original_data = NULL;
		ret_val = FALSE_m10;
	}
	
	// check decompressed_data
	if (need_decompressed_data == TRUE_m10 && cps->decompressed_data == NULL) {
		error_message_m10("%s(): \"decompressed_data\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m10;
	}
	if (need_decompressed_data == FALSE_m10 && cps->decompressed_data != NULL) {
		warning_message_m10("%s(): \"decompressed_data\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		free((void *) cps->decompressed_data);
		cps->decompressed_ptr = cps->decompressed_data = NULL;
		ret_val = FALSE_m10;
	}
	
	// check detrended_buffer
	if (need_detrended_buffer == TRUE_m10 && cps->detrended_buffer == NULL) {
		error_message_m10("%s(): \"detrended_buffer\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m10;
	}
	if (need_detrended_buffer == FALSE_m10 && cps->detrended_buffer != NULL) {
		warning_message_m10("%s(): \"detrended_buffer\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		free((void *) cps->detrended_buffer);
		cps->detrended_buffer = NULL;
		ret_val = FALSE_m10;
	}
	
	// check derivative_buffer
	if (need_derivative_buffer == TRUE_m10 && cps->derivative_buffer == NULL) {
		error_message_m10("%s(): \"derivative_buffer\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m10;
	}
	if (need_derivative_buffer == FALSE_m10 && cps->derivative_buffer != NULL) {
		warning_message_m10("%s(): \"derivative_buffer\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		free((void *) cps->derivative_buffer);
		cps->derivative_buffer = NULL;
		ret_val = FALSE_m10;
	}
	
	// check scaled_amplitude_buffer
	if (need_scaled_amplitude_buffer == TRUE_m10 && cps->scaled_amplitude_buffer == NULL) {
		error_message_m10("%s(): \"scaled_amplitude_buffer\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m10;
	}
	if (need_scaled_amplitude_buffer == FALSE_m10 && cps->scaled_amplitude_buffer != NULL) {
		warning_message_m10("%s(): \"scaled_amplitude_buffer\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		free((void *) cps->scaled_amplitude_buffer);
		cps->scaled_amplitude_buffer = NULL;
		ret_val = FALSE_m10;
	}
	
	// check scaled_frequency_buffer
	if (need_scaled_frequency_buffer == TRUE_m10 && cps->scaled_frequency_buffer == NULL) {
		error_message_m10("%s(): \"scaled_frequency_buffer\" is not allocated in the CMP_PROCESSING_STRUCT\n", __FUNCTION__);
		ret_val = FALSE_m10;
	}
	if (need_scaled_frequency_buffer == FALSE_m10 && cps->scaled_frequency_buffer != NULL) {
		warning_message_m10("%s(): \"scaled_frequency_buffer\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing\n", __FUNCTION__);
		free((void *) cps->scaled_frequency_buffer);
		cps->scaled_frequency_buffer = NULL;
		ret_val = FALSE_m10;
	}
	
	return(ret_val);
}


inline void	CMP_cps_mutex_off_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	cps->mutex = FALSE_m10;
	
	return;
}


inline void	CMP_cps_mutex_on_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	while (cps->mutex == TRUE_m10);
	cps->mutex = TRUE_m10;
	
	return;
}


void    CMP_decode_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	ui4				offset;
	si4				*si4_p;
	sf4				*sf4_p, amplitude_scale, frequency_scale;
	sf8				intercept, gradient;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;

	
	block_header = cps->block_header;
	
	// decrypt
	if (block_header->block_flags & CMP_BF_ENCRYPTION_MASK_m10)
		CMP_decrypt_m10(cps);
	
	// discontinuity
	if (block_header->block_flags & CMP_BF_DISCONTINUITY_MASK_m10)
		cps->parameters.discontinuity = TRUE_m10;
	else
		cps->parameters.discontinuity = FALSE_m10;
	
	// get variable region
	CMP_get_variable_region_m10(cps);
	
	// decompress
	switch (block_header->block_flags & CMP_BF_ALGORITHM_MASK_m10) {
		case CMP_BF_RED_ENCODING_MASK_m10:
			cps->directives.algorithm = CMP_RED_COMPRESSION_m10;
			CMP_RED_decode_m10(cps);
			break;
		case CMP_BF_PRED_ENCODING_MASK_m10:
			cps->directives.algorithm = CMP_PRED_COMPRESSION_m10;
			CMP_PRED_decode_m10(cps);
			break;
		case CMP_BF_MBE_ENCODING_MASK_m10:
			cps->directives.algorithm = CMP_MBE_COMPRESSION_m10;
			CMP_MBE_decode_m10(cps);
			break;
		default:
			(void)error_message_m10("%s(): unrecognized compression algorithm (%u)\n", __FUNCTION__, cps->directives.algorithm);
			return;
	}
	
	// unscale frequency-scaled decompressed_data if scaled (in place)
	if (block_header->parameter_flags & CMP_PF_FREQUENCY_SCALE_MASK_m10) {
		sf4_p = (sf4*)cps->parameters.block_parameters;
		offset = cps->parameters.block_parameters[cps->parameters.block_parameter_map[CMP_PF_FREQUENCY_SCALE_IDX_m10]];
		frequency_scale = *(sf4_p + offset);
		CMP_unscale_frequency_m10(cps->decompressed_ptr, cps->decompressed_ptr, (si8)block_header->number_of_samples, frequency_scale);
	}
	
	// unscale amplitude-scaled decompressed_data if scaled (in place)
	if (block_header->parameter_flags & CMP_PF_AMPLITUDE_SCALE_MASK_m10) {
		sf4_p = (sf4*)cps->parameters.block_parameters;
		offset = cps->parameters.block_parameters[cps->parameters.block_parameter_map[CMP_PF_AMPLITUDE_SCALE_IDX_m10]];
		amplitude_scale = *(sf4_p + offset);
		CMP_unscale_amplitude_m10(cps->decompressed_ptr, cps->decompressed_ptr, (si8)block_header->number_of_samples, amplitude_scale);
	}
	
	// add trend to decompressed_data if detrended (in place)
	if (CMP_IS_DETRENDED_m10(block_header)) {
		sf4_p = (sf4*)cps->parameters.block_parameters;
		offset = cps->parameters.block_parameters[cps->parameters.block_parameter_map[CMP_PF_GRADIENT_IDX_m10]];
		gradient = (sf8) * (sf4_p + offset);
		si4_p = (si4*)cps->parameters.block_parameters;
		offset = cps->parameters.block_parameters[cps->parameters.block_parameter_map[CMP_PF_INTERCEPT_IDX_m10]];
		intercept = (sf8) * (si4_p + offset);
		CMP_retrend_m10(cps->decompressed_ptr, cps->decompressed_ptr, block_header->number_of_samples, gradient, intercept);
	}
	
	return;
}


TERN_m10	CMP_decrypt_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	ui1				*ui1_p, *key;
	si4				encryption_blocks, encryptable_blocks;
	si8				i, encryption_bytes;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	PASSWORD_DATA_m10		*pwd;
	
	
	// check if block is encrypted
	block_header = cps->block_header;
	if ((block_header->block_flags & CMP_BF_ENCRYPTION_MASK_m10) == 0)
		return(0);
	
	// get decryption key
	pwd = cps->password_data;
	if (block_header->block_flags & CMP_BF_LEVEL_1_ENCRYPTION_MASK_m10) {
		if (block_header->block_flags & CMP_BF_LEVEL_2_ENCRYPTION_MASK_m10) {
			error_message_m10("%s(): Cannot decrypt data: flags indicate both level 1 & level 2 encryption\n", __FUNCTION__);
			return(FALSE_m10);
		}
		if (pwd->access_level >= LEVEL_1_ENCRYPTION_m10) {
			key = pwd->level_1_encryption_key;
		}
		else {
			(void)error_message_m10("%s(): Cannot decrypt data: insufficient access\n", __FUNCTION__);
			return(FALSE_m10);
		}
	}
	else {  // level 2 bit is set
		if (pwd->access_level == LEVEL_2_ENCRYPTION_m10) {
			key = pwd->level_2_encryption_key;
		}
		else {
			(void)error_message_m10("%s(): Cannot decrypt data: insufficient access\n", __FUNCTION__);
			return(-1);
		}
	}
	
	// calculated encryption blocks
	encryptable_blocks = (block_header->total_block_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10) / ENCRYPTION_BLOCK_BYTES_m10;
	if (block_header->block_flags | CMP_BF_MBE_ENCODING_MASK_m10) {
		encryption_blocks = encryptable_blocks;
	}
	else {
		encryption_bytes = block_header->total_header_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10 + ENCRYPTION_BLOCK_BYTES_m10;
		encryption_blocks = (si4)(((encryption_bytes - 1) / ENCRYPTION_BLOCK_BYTES_m10) + 1);
		if (encryptable_blocks < encryption_blocks)
			encryption_blocks = encryptable_blocks;
	}
	
	// decrypt
	ui1_p = (ui1 *)block_header + CMP_BLOCK_ENCRYPTION_START_OFFSET_m10;
	for (i = 0; i < encryption_blocks; ++i) {
		AES_decrypt_m10(ui1_p, ui1_p, NULL, key);
		ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
	}
	
	// set block flags to decrypted
	block_header->block_flags &= ~CMP_BF_ENCRYPTION_MASK_m10;
	
	return(0);
}


void    CMP_detrend_m10(si4 *input_buffer, si4 *output_buffer, si8 len, CMP_PROCESSING_STRUCT_m10 *cps)
{
	si4	*si4_p1, *si4_p2;
	sf4	sf4_m;
	si4	si4_b;
	sf8	m, b, mx_plus_b;
	
	
	// detrend from input_buffer to output_buffer
	// slope and intercept values entered into block_header
	// if input_buffer == output_buffer detrending will be done in place
	// if cps != NULL store coefficients in block parameters
	
	CMP_lad_reg_m10(input_buffer, len, &m, &b);
	
	// store m & b in block parameter region
	// NOTE: block parameter region must be setup first
	if (cps != NULL) {
		// demote precision
		sf4_m = (sf4)m;
		si4_b = CMP_round_m10(b);
		// store
		*((sf4 *) cps->parameters.block_parameters + cps->parameters.block_parameter_map[CMP_PF_GRADIENT_IDX_m10]) = sf4_m;
		*((si4 *) cps->parameters.block_parameters + cps->parameters.block_parameter_map[CMP_PF_INTERCEPT_IDX_m10]) = si4_b;
		// promote back to sf8 precision
		m = (sf8) sf4_m;
		b = (sf8) si4_b;
	}
	
	// subtract trend from input_buffer to output_buffer
	mx_plus_b = b;
	si4_p1 = input_buffer;
	si4_p2 = output_buffer;
	while (len--)
		*si4_p2++ = CMP_round_m10((sf8) *si4_p1++ - (mx_plus_b += m));
	
	return;
}


void    CMP_encode_m10(CMP_PROCESSING_STRUCT_m10 *cps, si8 start_time, si4 acquisition_channel_number, ui4 number_of_samples)
{
	TERN_m10                        data_is_compressed, allow_lossy_compression;
	ui1				normality;
	void                            (*compression_f)(CMP_PROCESSING_STRUCT_m10 * cps);
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	CMP_cps_mutex_on_m10(cps);
	
	block_header = cps->block_header;
	
	// fill in passed header fields
	block_header->block_start_UID = CMP_BLOCK_START_UID_m10;
	block_header->start_time = start_time;
	block_header->acquisition_channel_number = acquisition_channel_number;
	block_header->number_of_samples = number_of_samples;
	
	// reset block flags
	block_header->block_flags = 0;
	
	// set up variable region
	CMP_set_variable_region_m10(cps);
	
	// discontinuity
	if (cps->parameters.discontinuity == TRUE_m10) {
		block_header->block_flags |= CMP_BF_DISCONTINUITY_MASK_m10;
		if (cps->directives.reset_discontinuity == TRUE_m10)
			cps->parameters.discontinuity = FALSE_m10;
	}
	
	// RED/PRED specific fields
	if (block_header->block_flags & CMP_BF_PRED_ENCODING_MASK_m10) {
		*(cps->model_region + CMP_PRED_MODEL_NO_ZERO_COUNTS_FLAG_OFFSET_m10) = cps->parameters.no_zero_counts_flag;
		*(cps->model_region + CMP_PRED_MODEL_DERIVATIVE_LEVEL_OFFSET_m10) = cps->parameters.derivative_level;
	}
	else if (block_header->block_flags & CMP_BF_RED_ENCODING_MASK_m10) {
		*(cps->model_region + CMP_RED_MODEL_NO_ZERO_COUNTS_FLAG_OFFSET_m10) = cps->parameters.no_zero_counts_flag;
		*(cps->model_region + CMP_RED_MODEL_DERIVATIVE_LEVEL_OFFSET_m10) = cps->parameters.derivative_level;
	}
	
	// select compression
	// (compression algorithms are responsible for filling in: total_header_bytes, total_block_bytes, model_region_bytes, and the model details)
	switch (cps->directives.algorithm) {
		case CMP_RED_COMPRESSION_m10:
			block_header->block_flags |= CMP_BF_RED_ENCODING_MASK_m10;
			compression_f = &CMP_RED_encode_m10;
			break;
		case CMP_PRED_COMPRESSION_m10:
			block_header->block_flags |= CMP_BF_PRED_ENCODING_MASK_m10;
			compression_f = &CMP_PRED_encode_m10;
			break;
		case CMP_MBE_COMPRESSION_m10:
			block_header->block_flags |= CMP_BF_MBE_ENCODING_MASK_m10;
			CMP_find_extrema_m10(NULL, 0, NULL, NULL, cps);
			compression_f = &CMP_MBE_encode_m10;
			break;
		default:
			error_message_m10("%s(): unrecognized compression algorithm (%u)\n", __FUNCTION__, cps->directives.algorithm);
			CMP_cps_mutex_off_m10(cps);
			return;
	}
	
	// detrend
	if (cps->directives.detrend_data == TRUE_m10) {
		CMP_detrend_m10(cps->original_ptr, cps->detrended_buffer, block_header->number_of_samples, cps);
		cps->input_buffer = cps->detrended_buffer;
	}
	
	// lossy compression
	data_is_compressed = FALSE_m10;
	allow_lossy_compression = TRUE_m10;
	if (cps->directives.require_normality == TRUE_m10) {
		normality = CMP_normality_score_m10(cps->input_buffer, block_header->number_of_samples);
		if (normality < cps->parameters.minimum_normality) {
			allow_lossy_compression = FALSE_m10;
			block_header->parameter_flags &= ~(CMP_PF_AMPLITUDE_SCALE_MASK_m10 | CMP_PF_FREQUENCY_SCALE_MASK_m10);
		}
	}
	if (allow_lossy_compression == TRUE_m10) {
		if (cps->directives.set_amplitude_scale == TRUE_m10 || cps->directives.find_amplitude_scale == TRUE_m10) {
			if (cps->directives.set_amplitude_scale == TRUE_m10)
				CMP_scale_amplitude_m10(cps->input_buffer, cps->scaled_amplitude_buffer, block_header->number_of_samples, (sf8)cps->parameters.amplitude_scale, cps);
			if (cps->directives.find_amplitude_scale == TRUE_m10)
				data_is_compressed = CMP_find_amplitude_scale_m10(cps, compression_f);
			cps->input_buffer = cps->scaled_amplitude_buffer;
		}
		if (cps->directives.set_frequency_scale == TRUE_m10 || cps->directives.find_frequency_scale == TRUE_m10) {
			if (cps->directives.set_frequency_scale == TRUE_m10)
				CMP_scale_frequency_m10(cps->input_buffer, cps->scaled_frequency_buffer, block_header->number_of_samples, (sf8)cps->parameters.frequency_scale, cps);
			if (cps->directives.find_frequency_scale == TRUE_m10)
				data_is_compressed = CMP_find_frequency_scale_m10(cps, compression_f);
			cps->input_buffer = cps->scaled_frequency_buffer;
		}
	}
	
	// noise scores
	if (cps->directives.include_noise_scores == TRUE_m10) {
		// code not written yet
	}
	
	// compress
	if (data_is_compressed == FALSE_m10)
		(*compression_f)(cps);
	
	// encrypt
	if (cps->directives.encryption_level > NO_ENCRYPTION_m10)
		CMP_encrypt_m10(cps);
	
	CMP_cps_mutex_off_m10(cps);
	
	return;
}


TERN_m10     CMP_encrypt_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	ui1				*ui1_p, *key;
	si1				encryption_level;
	ui4				encryption_mask, encryption_bits;
	si4				encryption_blocks, encryptable_blocks;
	si8				i, encryption_bytes;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	PASSWORD_DATA_m10		*pwd;
	
	
	block_header = cps->block_header;
	pwd = cps->password_data;
	encryption_level = cps->directives.encryption_level;
	encryption_bits = block_header->block_flags & CMP_BF_ENCRYPTION_MASK_m10;
	
	// block encrypted
	switch (encryption_bits) {
		case 0: // not encrypted
			break;
		case CMP_BF_ENCRYPTION_MASK_m10:
			warning_message_m10("%s(): Level 1 & 2 bits set in block => cannot encrypt\n", __FUNCTION__);
			return(FALSE_m10);
		case CMP_BF_LEVEL_1_ENCRYPTION_MASK_m10:
			if (encryption_level == LEVEL_1_ENCRYPTION_m10)
				return(TRUE_m10); // already encrypted
			CMP_decrypt_m10(cps); // encrypted, but at wrong level
			break;
		case CMP_BF_LEVEL_2_ENCRYPTION_MASK_m10:
			if (encryption_level == LEVEL_2_ENCRYPTION_m10)
				return(TRUE_m10); // already encrypted
			CMP_decrypt_m10(cps); // encrypted, but at wrong level
			break;
	}
	if (encryption_level == NO_ENCRYPTION_m10)
		return(TRUE_m10);
	
	// check access
	if (pwd->access_level >= encryption_level) {
		if (encryption_level == LEVEL_1_ENCRYPTION_m10) {
			key = pwd->level_1_encryption_key;
			encryption_mask = CMP_BF_LEVEL_1_ENCRYPTION_MASK_m10;
		}
		else {
			key = pwd->level_2_encryption_key;
			encryption_mask = CMP_BF_LEVEL_2_ENCRYPTION_MASK_m10;
		}
	}
	else {
		error_message_m10("%s(): Cannot encrypt data => insufficient access\n", __FUNCTION__);
		return(FALSE_m10);
	}
	
	// encrypt
	encryptable_blocks = (block_header->total_block_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10) / ENCRYPTION_BLOCK_BYTES_m10;
	if (block_header->block_flags & CMP_BF_MBE_ENCODING_MASK_m10) {
		encryption_blocks = encryptable_blocks;
	}
	else {
		encryption_bytes = block_header->total_header_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10 + ENCRYPTION_BLOCK_BYTES_m10;
		encryption_blocks = (si4)(((encryption_bytes - 1) / ENCRYPTION_BLOCK_BYTES_m10) + 1);
		if (encryptable_blocks < encryption_blocks)
			encryption_blocks = encryptable_blocks;
	}
	ui1_p = (ui1 *)block_header + CMP_BLOCK_ENCRYPTION_START_OFFSET_m10;
	for (i = 0; i < encryption_blocks; ++i) {
		AES_encrypt_m10(ui1_p, ui1_p, NULL, key);
		ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
	}
	block_header->block_flags |= encryption_mask;
	
	return(TRUE_m10);
}


TERN_m10    CMP_find_amplitude_scale_m10(CMP_PROCESSING_STRUCT_m10 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m10 *cps))
{
	TERN_m10                     	data_is_compressed;
	si8                     	i;
	si4				*input_buffer;
	sf8                     	original_size, goal_compression_ratio;
	sf8                     	low_sf, high_sf, mrr, mrr2, mrr5, sf_per_mrr;
	sf8                     	goal_low_bound, goal_high_bound, goal_mrr, goal_tol;
	sf4                     	new_scale_factor;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	input_buffer = cps->input_buffer;
	block_header = cps->block_header;
	data_is_compressed = FALSE_m10;
	
	if (cps->directives.use_compression_ratio == TRUE_m10) {
		goal_compression_ratio = cps->parameters.goal_ratio;
		goal_low_bound = goal_compression_ratio - cps->parameters.goal_tolerance;
		goal_high_bound = goal_compression_ratio + cps->parameters.goal_tolerance;
		cps->parameters.amplitude_scale = (sf4)1.0;
		(*compression_f)(cps);
		original_size = (sf8)block_header->number_of_samples * (sf8)sizeof(si4);
		cps->parameters.actual_ratio = (sf8)block_header->total_block_bytes / original_size;
		if (cps->parameters.actual_ratio > goal_high_bound) {
			// loop until acceptable scale factor found
			for (i = cps->parameters.maximum_goal_attempts; i--;) {
				new_scale_factor = cps->parameters.amplitude_scale * (sf4)(cps->parameters.actual_ratio / goal_compression_ratio);
				if ((ABS_m10(new_scale_factor - cps->parameters.amplitude_scale) <= (sf4)0.000001) || (new_scale_factor <= (sf4)1.0))
					break;
				cps->parameters.amplitude_scale = new_scale_factor;
				(*compression_f)(cps);  // compress
				cps->parameters.actual_ratio = (sf8)block_header->total_block_bytes / original_size;
				if ((cps->parameters.actual_ratio <= goal_high_bound) && (cps->parameters.actual_ratio >= goal_low_bound))
					break;
			}
		}
		CMP_generate_lossy_data_m10(cps, input_buffer, cps->decompressed_ptr, CMP_AMPLITUDE_SCALE_MODE_m10);
		cps->input_buffer = cps->decompressed_ptr;
	}
	else if (cps->directives.use_mean_residual_ratio == TRUE_m10) {
		// get residual ratio at sf 2 & 5 (roughly linear relationship: reasonable sample points)
		cps->parameters.amplitude_scale = (sf4)2.0;
		CMP_generate_lossy_data_m10(cps, input_buffer, cps->decompressed_ptr, CMP_AMPLITUDE_SCALE_MODE_m10);
		mrr2 = CMP_calculate_mean_residual_ratio_m10(cps->original_ptr, cps->decompressed_ptr, block_header->number_of_samples);
		if (mrr2 == (sf8)0.0) {  // all zeros in block
			cps->parameters.amplitude_scale = (sf4)1.0;
			cps->parameters.actual_ratio = (sf8)0.0;
			(*compression_f)(cps);
			goto CMP_MRR_DONE;
		}
		cps->parameters.amplitude_scale = (sf4)5.0;
		CMP_generate_lossy_data_m10(cps, input_buffer, cps->decompressed_ptr, CMP_AMPLITUDE_SCALE_MODE_m10);
		mrr5 = CMP_calculate_mean_residual_ratio_m10(cps->original_ptr, cps->decompressed_ptr, block_header->number_of_samples);
		sf_per_mrr = (sf8)3.0 / (mrr5 - mrr2);
		// estimate starting points
		goal_mrr = cps->parameters.goal_ratio;
		goal_tol = cps->parameters.goal_tolerance;
		goal_low_bound = goal_mrr - goal_tol;
		goal_high_bound = goal_mrr + goal_tol;
		cps->parameters.amplitude_scale = (sf4)(((goal_mrr - mrr2) * sf_per_mrr) + (sf8)2.0);
		high_sf = ((goal_high_bound - mrr2) * sf_per_mrr) + (sf8)2.0;
		high_sf *= (sf8)2.0;  // empirically reasonable
		low_sf = (sf8)1.0;
		for (i = cps->parameters.maximum_goal_attempts; i--;) {
			CMP_generate_lossy_data_m10(cps, input_buffer, cps->decompressed_ptr, CMP_AMPLITUDE_SCALE_MODE_m10);
			mrr = CMP_calculate_mean_residual_ratio_m10(cps->original_ptr, cps->decompressed_ptr, block_header->number_of_samples);
			if (mrr < goal_low_bound)
				low_sf = (sf8)cps->parameters.amplitude_scale;
			else if (mrr > goal_high_bound)
				high_sf = (sf8)cps->parameters.amplitude_scale;
			else
				break;
			new_scale_factor = (sf4)((low_sf + high_sf) / (sf8)2.0);
			if (new_scale_factor <= (sf4)1.0)
				break;
			if ((high_sf - low_sf) < (sf8)0.005) {
				cps->parameters.amplitude_scale = new_scale_factor;
				break;
			}
			cps->parameters.amplitude_scale = new_scale_factor;
		}
		cps->parameters.actual_ratio = mrr;
		cps->input_buffer = cps->decompressed_ptr;
		(*compression_f)(cps);  // compress
	}
	else {
		error_message_m10("%s(): either use_compression_ratio or use_mean_residual_ratio directive must be set (mode == %d)\n", __FUNCTION__, cps->directives.mode);
		return(data_is_compressed);
	} CMP_MRR_DONE:
	
	return(data_is_compressed);
}


void    CMP_find_extrema_m10(si4 *input_buffer, si8 len, si4 *minimum, si4 *maximum, CMP_PROCESSING_STRUCT_m10 *cps)
{
	si4     min, max;
	si8     i;
	
	
	if (cps != NULL) {
		input_buffer = cps->input_buffer;
		len = cps->block_header->number_of_samples;
	}
	
	min = max = *input_buffer;
	for (i = len; --i;) {
		if (*++input_buffer > max)
			max = *input_buffer;
		else if (*input_buffer < min)
			min = *input_buffer;
	}
	
	if (cps != NULL) {
		cps->parameters.minimum_sample_value = min;
		cps->parameters.maximum_sample_value = max;
	}
	if (minimum != NULL)
		*minimum = min;
	if (maximum != NULL)
		*maximum = max;
	
	return;
}


TERN_m10	CMP_find_frequency_scale_m10(CMP_PROCESSING_STRUCT_m10 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m10 *cps))
{
	
	// code not written yet
	
	return(TRUE_m10);
}


void    CMP_free_processing_struct_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	if (cps == NULL) {
		warning_message_m10("%s(): trying to free a NULL CMP_PROCESSING_STRUCT_m10 => returning with no action\n", __FUNCTION__);
		return;
	}
	if (cps->original_data != NULL)
		free((void *) cps->original_data);
	
	if (cps->decompressed_data != NULL)
		free((void *) cps->decompressed_data);
	
	if (cps->compressed_data != NULL)
		free((void *) cps->compressed_data);
	
	if (cps->difference_buffer != NULL)
		free((void *) cps->difference_buffer);
	
	if (cps->detrended_buffer != NULL)
		free((void *) cps->detrended_buffer);
	
	if (cps->scaled_amplitude_buffer != NULL)
		free((void *) cps->scaled_amplitude_buffer);
	
	if (cps->scaled_frequency_buffer != NULL)
		free((void *) cps->scaled_frequency_buffer);
	
	if (cps->count != NULL)
		free((void *) cps->count);
	
	if (cps->cumulative_count != NULL)
		free((void *) cps->cumulative_count);
	
	if (cps->sorted_count != NULL)
		free((void *) cps->sorted_count);
	
	if (cps->minimum_range != NULL)
		free((void *) cps->minimum_range);
	
	if (cps->symbol_map != NULL)
		free((void *) cps->symbol_map);
	
	if (cps->directives.free_password_data == TRUE_m10)
		if (cps->password_data != &globals_m10->password_data && cps->password_data != NULL)
			free((void *) cps->password_data);
	
	free((void *) cps);
	
	return;
}


void    CMP_generate_lossy_data_m10(CMP_PROCESSING_STRUCT_m10 *cps, si4 *input_buffer, si4 *output_buffer, ui1 mode)
{
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	// generates lossy data from input_buffer to output_buffer
	// if input_buffer == output_buffer lossy data will be made in place
	block_header = cps->block_header;
	
	if (mode == CMP_AMPLITUDE_SCALE_MODE_m10) {
		// amplitude scale from input_buffer to output_buffer (lossy)
		CMP_scale_amplitude_m10(input_buffer, output_buffer, block_header->number_of_samples, (sf8)cps->parameters.amplitude_scale, cps);
		// unscale in place
		CMP_unscale_amplitude_m10(output_buffer, output_buffer, block_header->number_of_samples, (sf8)cps->parameters.amplitude_scale);
	}
	else if (mode == CMP_FREQUENCY_SCALE_MODE_m10) {
		// frequency scale from input_buffer to output_buffer (lossy)
		CMP_scale_frequency_m10(input_buffer, output_buffer, block_header->number_of_samples, (sf8)cps->parameters.frequency_scale, cps);
		// unscale in place
		CMP_unscale_frequency_m10(output_buffer, output_buffer, block_header->number_of_samples, (sf8)cps->parameters.frequency_scale);
	}
	else {
		error_message_m10("%s(): unrecognized lossy compression mode => no data generated\n", __FUNCTION__);
	}
	
	return;
}


void    CMP_get_variable_region_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	ui1				*var_reg_ptr;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	block_header = cps->block_header;
	var_reg_ptr = (ui1 *)block_header + CMP_BLOCK_FIXED_HEADER_BYTES_m10;  // pointer to beginning of variable region
	
	// records region
	cps->records = var_reg_ptr;
	var_reg_ptr += block_header->record_region_bytes;
	
	// parameter region
	cps->parameters.block_parameters = (ui4*)var_reg_ptr;
	CMP_generate_parameter_map_m10(cps);
	var_reg_ptr += block_header->parameter_region_bytes;
	
	// protected region
	cps->protected_region = var_reg_ptr;
	var_reg_ptr += block_header->protected_region_bytes;
	
	// discretionary region
	cps->discretionary_region = var_reg_ptr;
	var_reg_ptr += block_header->discretionary_region_bytes;
	
	// variable region bytes
	cps->parameters.variable_region_bytes = CMP_VARIABLE_REGION_BYTES_v2_m10(block_header);
	
	// model region (not part of variable region, but convenient to do this here)
	cps->model_region = var_reg_ptr;
	
	return;
}


void	CMP_generate_parameter_map_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	ui4				bit, flags, n_params, i, *p_map;
	CMP_BLOCK_FIXED_HEADER_m10	*bh;
	
	
	// add up parameter bytes (4 bytes for each bit set)
	bh = cps->block_header;
	flags = bh->parameter_flags;
	p_map = cps->parameters.block_parameter_map;
	for (bit = 1, n_params = i = 0; i < CMP_PF_PARAMETER_FLAG_BITS_m10; ++i, bit <<= 1)
		if (flags & bit)
			p_map[i] = n_params++;
	
	cps->parameters.number_of_block_parameters = (si4)n_params;
	bh->parameter_region_bytes = (ui2)(n_params * 4);
	
	return;
}


void	CMP_initialize_directives_m10(CMP_DIRECTIVES_m10 *directives, ui1 mode)
{
	directives->mode = mode;
	directives->free_password_data = CMP_DIRECTIVES_FREE_PASSWORD_DATA_DEFAULT_m10;
	directives->algorithm = CMP_DIRECTIVES_ALGORITHM_DEFAULT_m10;
	directives->encryption_level = CMP_DIRECTIVES_ENCRYPTION_LEVEL_DEFAULT_m10;
	directives->fall_through_to_MBE = CMP_DIRECTIVES_FALL_THROUGH_TO_MBE_DEFAULT_m10;
	directives->reset_discontinuity = CMP_DIRECTIVES_RESET_DISCONTINUITY_DEFAULT_m10;
	directives->include_noise_scores = CMP_DIRECTIVES_INCLUDE_NOISE_SCORES_DEFAULT_m10;
	directives->no_zero_counts = CMP_DIRECTIVES_NO_ZERO_COUNTS_DEFAULT_m10;
	directives->set_derivative_level = CMP_DIRECTIVES_SET_DERIVATIVE_LEVEL_DEFAULT_m10;
	directives->find_derivative_level = CMP_DIRECTIVES_FIND_DERIVATIVE_LEVEL_DEFAULT_m10;
	directives->detrend_data = CMP_DIRECTIVES_DETREND_DATA_DEFAULT_m10;
	directives->require_normality = CMP_DIRECTIVES_REQUIRE_NORMALITY_DEFAULT_m10;
	directives->use_compression_ratio = CMP_DIRECTIVES_USE_COMPRESSION_RATIO_DEFAULT_m10;
	directives->use_mean_residual_ratio = CMP_DIRECTIVES_USE_MEAN_RESIDUAL_RATIO_DEFAULT_m10;
	directives->use_relative_ratio = CMP_DIRECTIVES_USE_RELATIVE_RATIO_DEFAULT_m10;
	directives->set_amplitude_scale = CMP_DIRECTIVES_SET_AMPLITUDE_SCALE_DEFAULT_m10;
	directives->find_amplitude_scale = CMP_DIRECTIVES_FIND_AMPLITUDE_SCALE_DEFAULT_m10;
	directives->set_frequency_scale = CMP_DIRECTIVES_SET_FREQUENCY_SCALE_DEFAULT_m10;
	directives->find_frequency_scale = CMP_DIRECTIVES_FIND_FREQUENCY_SCALE_DEFAULT_m10;
	
	return;
}


TERN_m10	CMP_initialize_tables_m10(void)
{
	sf8	*cdf_table;
	
	
	cdf_table = (sf8 *) calloc_m10((size_t)CMP_NORMAL_CDF_TABLE_ENTRIES_m10, sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	{
		sf8 temp[CMP_NORMAL_CDF_TABLE_ENTRIES_m10] = CMP_NORMAL_CDF_TABLE_m10;
		memcpy(cdf_table, temp, CMP_NORMAL_CDF_TABLE_ENTRIES_m10 * sizeof(sf8));
	}
	
	globals_m10->CMP_normal_CDF_table = cdf_table;
	
	return(TRUE_m10);
}


void	CMP_initialize_parameters_m10(CMP_PARAMETERS_m10 *parameters)
{
	parameters->number_of_block_parameters = 0;
	parameters->block_parameters = NULL;
	parameters->minimum_sample_value = CMP_PARAMETERS_MINIMUM_SAMPLE_VALUE_DEFAULT_m10;
	parameters->maximum_sample_value = CMP_PARAMETERS_MAXIMUM_SAMPLE_VALUE_DEFAULT_m10;
	parameters->discontinuity = CMP_PARAMETERS_DISCONTINUITY_DEFAULT_m10;
	parameters->no_zero_counts_flag = CMP_PARAMETERS_NO_ZERO_COUNTS_FLAG_DEFAULT_m10;
	parameters->derivative_level = CMP_PARAMETERS_DERIVATIVE_LEVEL_DEFAULT_m10;
	parameters->goal_ratio = CMP_PARAMETERS_GOAL_RATIO_DEFAULT_m10;
	parameters->goal_tolerance = CMP_PARAMETERS_GOAL_TOLERANCE_DEFAULT_m10;
	parameters->maximum_goal_attempts = CMP_PARAMETERS_MAXIMUM_GOAL_ATTEMPTS_DEFAULT_m10;
	parameters->minimum_normality = CMP_PARAMETERS_MINIMUM_NORMALITY_DEFAULT_m10;
	parameters->amplitude_scale = CMP_PARAMETERS_AMPLITUDE_SCALE_DEFAULT_m10;
	parameters->frequency_scale = CMP_PARAMETERS_FREQUENCY_SCALE_DEFAULT_m10;
	parameters->user_number_of_records = CMP_USER_NUMBER_OF_RECORDS_DEFAULT_m10;
	parameters->user_record_region_bytes = CMP_USER_RECORD_REGION_BYTES_DEFAULT_m10;
	parameters->user_parameter_flags = CMP_USER_PARAMETER_FLAGS_DEFAULT_m10;
	parameters->protected_region_bytes = CMP_PROTECTED_REGION_BYTES_DEFAULT_m10;
	parameters->user_discretionary_region_bytes = CMP_USER_DISCRETIONARY_REGION_BYTES_DEFAULT_m10;
	
	return;
}


void    CMP_lad_reg_m10(si4 *input_buffer, si8 len, sf8 *m, sf8 *b)
{
	sf8		*y, t, *yp, *buff, *bp, min_y, max_y, min_m, max_m, thresh, m_sum;
	sf8             d, ma, ba, m_eps, b_eps, lad_eps, test_m, lad, upper_m, lower_m;
	si8             i;
	const sf8	safe_eps = DBL_EPSILON * (sf8) 1000.0;
	
	
	// linear least absolute deviation_regression (1 array input)
	
	// allocate
	buff = (sf8 *) calloc_m10((size_t) len, sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	y = (sf8 *) calloc_m10((size_t) len, sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// copy & cast
	for (yp = y, i = len; i--;)
		*yp++ = (sf8)*input_buffer++;
	
	// setup
	yp = y;
	min_y = max_y = *yp;
	for (i = len; --i;) {
		if (*++yp > max_y)
			max_y = *yp;
		else if (*yp < min_y)
			min_y = *yp;
	}
	upper_m = max_m = (max_y - min_y) / (sf8)(len - 1);
	lower_m = min_m = -max_m;
	thresh = safe_eps * (sf8) 10.0;
	d = max_m - min_m;
	
	// search
	while (d > thresh) {
		ma = (upper_m + lower_m) / (sf8) 2.0;
		bp = buff; yp = y;
		m_sum = (sf8) 0.0;
		for (i = len; i--;)
			*bp++ = *yp++ - (m_sum += ma);
		ba = CMP_quantile_value_m10(buff, len, 0.5, 0, NULL);  // median
		bp = buff; lad = (sf8) 0.0;
		for (i = len; i--;) {
			t = *bp++ - ba;
			lad += ABS_m10(t);
		}
		m_eps = ma + safe_eps;
		bp = buff; yp = y;
		m_sum = (sf8) 0.0;
		for (i = len; i--;)
			*bp++ = *yp++ - (m_sum += m_eps);
		b_eps = CMP_quantile_value_m10(buff, len, 0.5, 0, NULL);  // median
		bp = buff; lad_eps = (sf8) 0.0;
		for (i = len; i--;) {
			t = *bp++ - b_eps;
			lad_eps += ABS_m10(t);
		}
		test_m = lad_eps - lad;
		if (test_m > (sf8) 0.0)
			upper_m = ma;
		else if (test_m < (sf8) 0.0)
			lower_m = ma;
		else
			break;
		d = upper_m - lower_m;
	}
	
	*b = ba;
	*m = ma;
	
	// clean up
	free((void *) buff);
	free((void *) y);
	
	return;
	
}


void    CMP_lin_reg_m10(si4 *input_buffer, si8 len, sf8 *m, sf8 *b)
{
	sf8	sx, sy, sxx, sxy, n, mx, my, c, val;
	si8	i;
	
	
	// linear least_squares regression (1 array input)
	
	n = (sf8)len;
	sx = (n * (n + (sf8) 1.0)) / (sf8) 2.0;
	sxx = (n * (n + (sf8) 1.0) * ((n * (sf8) 2.0) + (sf8) 1.0)) / (sf8) 6.0;
	
	c = sy = sxy = (sf8) 0.0;
	for (i = len; i--;) {
		val = (sf8)*input_buffer++;
		sy += val;
		sxy += val * (c += (sf8) 1.0);
	}
	
	mx = sx / n;
	my = sy / n;
	*m = (((sx * my) - sxy) / ((sx * mx) - sxx));
	*b = (my - (*m * mx));
	
	return;
}


void    CMP_MBE_decode_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	si4	*si4_p, bits_per_samp;
	ui4	n_samps;
	si8	i, lmin;
	ui8	out_val, *in_word, mask, temp_mask, high_bits, in_word_val;
	ui1	in_bit, *model, *comp_p;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	// read header (generic fields)
	block_header = cps->block_header;
	comp_p = (ui1 *)block_header + block_header->total_header_bytes;
	model = comp_p - CMP_MBE_MODEL_FIXED_BYTES_m10;
	lmin = (si8) * ((si4 *) (model + CMP_MBE_MODEL_MINIMUM_SAMPLE_VALUE_OFFSET_m10));
	bits_per_samp = (si4) * (model + CMP_MBE_MODEL_BITS_PER_SAMPLE_OFFSET_m10);
	n_samps = block_header->number_of_samples;
	
	// MBE decode
	in_word = (ui8 *)block_header + (block_header->total_header_bytes >> 3);
	in_bit = (block_header->total_header_bytes & 7) << 3;
	mask = (ui8)0xFFFFFFFFFFFFFFFF >> (64 - bits_per_samp);
	si4_p = cps->decompressed_ptr;
	in_word_val = *in_word >> in_bit;
	for (i = n_samps; i--;) {
		out_val = in_word_val & mask;
		in_word_val >>= bits_per_samp;
		if ((in_bit += bits_per_samp) > 63) {
			in_word_val = *++in_word;
			if (in_bit &= 63) {
				temp_mask = (ui8)0xFFFFFFFFFFFFFFFF >> (64 - in_bit);
				high_bits = in_word_val & temp_mask;
				out_val |= (high_bits << (bits_per_samp - in_bit));
				in_word_val >>= in_bit;
			}
		}
		*si4_p++ = (si4)((si8)out_val + lmin);
	}
	
	return;
}


void    CMP_MBE_encode_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	si4				*si4_p, bits_per_samp;
	ui4				cmp_data_bytes, variable_region_bytes;
	si8				i, cmp_data_bits, lmin;
	ui8				out_val, *out_word;
	ui1				out_bit, *output, *model;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	// compress (compression algorithms are responsible for filling in: total_block_bytes, total_header_bytes, & model_region_bytes in the header, and the model details)
	
	block_header = cps->block_header;
	
	// find full value bits per sample
	lmin = (si8)cps->parameters.minimum_sample_value;
	for (bits_per_samp = 0, i = (si8) cps->parameters.maximum_sample_value - lmin; i; i >>= 1)
		++bits_per_samp;
	
	// calculate header bytes
	variable_region_bytes = cps->parameters.variable_region_bytes;
	block_header->model_region_bytes = CMP_MBE_MODEL_FIXED_BYTES_m10;
	block_header->total_header_bytes = CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes + block_header->model_region_bytes;
	model = (ui1 *)block_header + CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes;
	output = (ui1 *)block_header + block_header->total_header_bytes;
	
	// calculate total encoding bytes
	cmp_data_bits = (si8)block_header->number_of_samples * (si8)bits_per_samp;
	cmp_data_bytes = cmp_data_bits >> 3;
	if (cmp_data_bits & 7)
		++cmp_data_bytes;
	
	// MBE encode
	memset(output, 0, cmp_data_bytes);
	out_word = (ui8 *)block_header + (block_header->total_header_bytes >> 3);
	out_bit = (block_header->total_header_bytes & 7) << 3;
	si4_p = cps->input_buffer;
	out_val = 0;
	for (i = block_header->number_of_samples; i--;) {
		out_val = (ui8)((si8)*si4_p++ - lmin);
		*out_word |= (out_val << out_bit);
		if ((out_bit += bits_per_samp) > 63) {
			out_bit &= 63;
			*++out_word = (out_val >> (bits_per_samp - out_bit));
		}
	}
	
	// fill in header
	block_header->total_block_bytes = pad_m10((ui1 *)block_header, (si8)(cmp_data_bytes + block_header->total_header_bytes), 8);
	*((si4 *) (model + CMP_MBE_MODEL_MINIMUM_SAMPLE_VALUE_OFFSET_m10)) = cps->parameters.minimum_sample_value;
	*((ui1 *) (model + CMP_MBE_MODEL_BITS_PER_SAMPLE_OFFSET_m10)) = (ui1)bits_per_samp;
	
	return;
}


ui1     CMP_normality_score_m10(si4 *data, ui4 n_samps)
{
	sf8     sx, sx2, sy, sy2, sxy, mx, mx2, sd, val, z, r, n, *norm_cdf;
	sf8     num, den1, den2, cdf[CMP_NORMAL_CDF_TABLE_ENTRIES_m10];
	si8     i, count[CMP_NORMAL_CDF_TABLE_ENTRIES_m10] = {0};
	si4	*si4_p, bin;
	ui1     ks;
	
	
	// Returns the correlation of the distribution in the data to that expected from a normal distribution.
	// Essentially a Kolmogorov-Smirnov test normalized to range [-1 to 0) = 0 & [0 to 1] = [0 to 254]. 255 is reserved for no entry.
	
	if (globals_m10->CMP_normal_CDF_table == NULL)
		CMP_initialize_tables_m10();
	norm_cdf = globals_m10->CMP_normal_CDF_table;

	// calculate mean & standard deviation
	n = (sf8) n_samps;
	si4_p = data;
	sx = sx2 = (sf8) 0.0;
	for (i = n_samps; i--;) {
		val = (sf8)*si4_p++;
		sx += val;
		sx2 += val * val;
	}
	mx = sx / n;
	mx2 = sx2 / n;
	sd = sqrt(mx2 - (mx * mx));
	
	// bin the samples
	si4_p = data;
	for (i = n_samps; i--;) {
		val = (sf8)*si4_p++;
		z = (val - mx) / sd;
		if (isnan(z))
			continue;
		bin = (si4) ((z + (sf8) 3.1) * (sf8) 10.0);
		if (bin < 0)
			bin = 0;
		else if (bin >= CMP_NORMAL_CDF_TABLE_ENTRIES_m10)
			bin = CMP_NORMAL_CDF_TABLE_ENTRIES_m10 - 1;
		++count[bin];
	}
	
	// generate data CDF
	cdf[0] = (sf8) count[0];
	for (i = 1; i < CMP_NORMAL_CDF_TABLE_ENTRIES_m10; ++i)
		cdf[i] = (sf8)count[i] + cdf[i - 1];
	
	// calculate correlation between data CDF and normal CDF
	sx = sx2 = sxy = (sf8) 0.0;
	sy = (sf8 )CMP_SUM_NORMAL_CDF_m10;
	sy2 = (sf8) CMP_SUM_SQ_NORMAL_CDF_m10;
	for (i = 0; i < CMP_NORMAL_CDF_TABLE_ENTRIES_m10; ++i) {
		sx += cdf[i];
		sx2 += cdf[i] * cdf[i];
		sxy += cdf[i] * norm_cdf[i];
	}
	
	num = (n * sxy) - (sx * sy);
	den1 = (n * sx2) - (sx * sx);
	den2 = (n * sy2) - (sy * sy);
	
	//  handle rounding errors
	if ((den1 <= (sf8) 0.0) || (den2 <= (sf8) 0.0))
		r = (sf8) 0.0;
	else
		r = num / (sqrt(den1) * sqrt(den2));
	
	// calculate Kolmogorov Smirnov correlation
	r += CMP_KS_CORRECTION_m10;
	if (r < (sf8) 0.0)
		r = (sf8) 0.0;
	else if (r > (sf8) 1.0)
		r = (sf8) 1.0;
	
	// return KS score
	ks = (ui1) CMP_round_m10(sqrt(1.0 - (r * r)) * 254);
	
	return(ks);
}


void    CMP_PRED_decode_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	ui1	*comp_p, *ui1_p, *low_bound_high_byte_p, *high_bound_high_byte_p;
	ui1	*goal_bound_high_byte_p, prev_cat;
	ui1	*symbol_map[CMP_PRED_CATS_m10], *symbols, *model;
	si1	*si1_p1, *si1_p2, *diff_p;
	ui2	*bin_counts, n_derivs, *stats_entries, *count[CMP_PRED_CATS_m10];
	ui4	n_diffs, total_stats_entries;
	si4	*si4_p, current_val, init_val;
	ui8	**minimum_range, **cumulative_count;
	ui8	low_bound, high_bound, prev_high_bound, goal_bound, range;
	si8	i, j;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	// CMP decompress from block_header to decompressed_ptr
	block_header = cps->block_header;
	model = (ui1 *)block_header + block_header->total_header_bytes - block_header->model_region_bytes;
	init_val = *((si4 *) (model + CMP_PRED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10));
	n_diffs = *((ui4 *) (model + CMP_PRED_MODEL_DIFFERENCE_BYTES_OFFSET_m10));
	n_derivs = *((ui2 *) (model + CMP_PRED_MODEL_DERIVATIVE_LEVEL_OFFSET_m10));
	stats_entries = (ui2 *) (model + CMP_PRED_MODEL_NUMBERS_OF_STATISTICS_BINS_OFFSET_m10);
	for (total_stats_entries = i = 0; i < CMP_PRED_CATS_m10; ++i)
		total_stats_entries += stats_entries[i];
	if (total_stats_entries == 0) {  // zero samples, or only one value
		for (si4_p = cps->decompressed_ptr, i = block_header->number_of_samples; i--;)
			*si4_p++ = init_val;
		return;
	}
	
	// build symbol map, count arrays, & minimum ranges
	bin_counts = (ui2 *) (model + CMP_PRED_MODEL_BIN_COUNTS_OFFSET_m10);
	symbols = (ui1 *) (bin_counts + total_stats_entries);
	cumulative_count = cps->cumulative_count;
	minimum_range = cps->minimum_range;
	for (i = 0; i < CMP_PRED_CATS_m10; ++i) {
		count[i] = bin_counts; bin_counts += stats_entries[i];
		symbol_map[i] = symbols; symbols += stats_entries[i];
		for (cumulative_count[i][0] = j = 0; j < stats_entries[i]; ++j) {
			cumulative_count[i][j + 1] = cumulative_count[i][j] + (ui8)count[i][j];
			minimum_range[i][j] = CMP_RED_TOTAL_COUNTS_m10 / count[i][j];
			if (CMP_RED_TOTAL_COUNTS_m10 > (count[i][j] * minimum_range[i][j]))
				++minimum_range[i][j];
		}
	}
	
	// range decode
	diff_p = cps->difference_buffer;
	prev_high_bound = goal_bound = low_bound = 0;
	range = CMP_RED_MAXIMUM_RANGE_m10;
	comp_p = (ui1 *)block_header + block_header->total_header_bytes;
	low_bound_high_byte_p = ((ui1 *)&low_bound) + 5;
	high_bound_high_byte_p = ((ui1 *)&high_bound) + 5;
	goal_bound_high_byte_p = ((ui1 *)&goal_bound) + 5;
	ui1_p = goal_bound_high_byte_p;
	j = 6; do {
		*ui1_p-- = *comp_p++;
	} while (--j);
	prev_cat = CMP_PRED_NIL_m10;
	
	for (i = n_diffs; i;) {
		for (j = 0; range >= minimum_range[prev_cat][j];) {
			high_bound = low_bound + ((range * cumulative_count[prev_cat][j + 1]) >> 16);
			if (high_bound > goal_bound) {
				range = high_bound - (low_bound = prev_high_bound);
				*diff_p = symbol_map[prev_cat][j];
				if (!--i)
					goto PRED_RANGE_DECODE_DONE;
				prev_cat = CMP_PRED_CAT_m10(*diff_p); ++diff_p;
				j = 0;
			}
			else {
				prev_high_bound = high_bound;
				++j;
			}
		}
		high_bound = low_bound + range;
		if (*low_bound_high_byte_p != *high_bound_high_byte_p) {
			ui1_p = goal_bound_high_byte_p;
			j = 6; do {
				*ui1_p-- = *comp_p++;
			} while (--j);
			low_bound = 0;
			range = CMP_RED_MAXIMUM_RANGE_m10;
		}
		else {
			do {
				low_bound <<= 8;
				high_bound <<= 8;
				goal_bound = (goal_bound << 8) | (ui8)*comp_p++;
				range <<= 8;
			} while (*low_bound_high_byte_p == *high_bound_high_byte_p);
			low_bound &= CMP_RED_RANGE_MASK_m10;
			goal_bound &= CMP_RED_RANGE_MASK_m10;
		}
		prev_high_bound = low_bound;
	} PRED_RANGE_DECODE_DONE:
	
	// generate output from difference data
	si1_p1 = cps->difference_buffer;
	si4_p = cps->decompressed_ptr;
	*si4_p++ = current_val = init_val;
	for (i = block_header->number_of_samples; --i;) {
		if (*si1_p1 == -128) {
			++si1_p1;
			si1_p2 = (si1 *)&current_val;
			*si1_p2++ = *si1_p1++; *si1_p2++ = *si1_p1++; *si1_p2++ = *si1_p1++; *si1_p2 = *si1_p1++;
		}
		else {
			current_val += (si4)*si1_p1++;
		}
		*si4_p++ = current_val;
	}
	
	return;
}


void    CMP_PRED_encode_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	ui1	*low_bound_high_byte_p, *high_bound_high_byte_p, *ui1_p, prev_cat;
	ui1	*diff_p, *comp_p, *symbols, **symbol_map, *model;
	ui2	*bin_counts, *stats_entries;
	ui4	variable_region_bytes, **count, total_diffs, PRED_total_bytes, MBE_total_bytes, total_stats_entries;
	ui4	cat_total_counts[CMP_PRED_CATS_m10], goal_total_counts, bin, MBE_cmp_data_bytes;
	si4	*input_buffer, *curr_si4_val_p, *next_si4_val_p, diff, min, max, MBE_bits_per_samp;
	ui8	**cumulative_count, **minimum_range;
	ui8	range, high_bound, low_bound;
	si8	i, j, k, l, extra_counts, scaled_total_counts, MBE_cmp_data_bits;
	CMP_STATISTICS_BIN_m10		**sorted_count, temp_sorted_count;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	// CMP compress from input_buffer to to block_header -> compressed data array
	count = cps->count;
	cumulative_count = cps->cumulative_count;
	minimum_range = cps->minimum_range;
	sorted_count = cps->sorted_count;
	symbol_map = cps->symbol_map;
	block_header = cps->block_header;
	input_buffer = cps->input_buffer;
	variable_region_bytes = cps->parameters.variable_region_bytes;
	model = cps->model_region;
	
	// zero or one or samples
	if (block_header->number_of_samples <= 1) {
		block_header->total_header_bytes = (ui4) (CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes + CMP_PRED_MODEL_FIXED_BYTES_m10);
		block_header->total_block_bytes = pad_m10((ui1 *) block_header, block_header->total_header_bytes, 8);
		block_header->model_region_bytes = (ui2)CMP_PRED_MODEL_FIXED_BYTES_m10;
		memset(model, 0, CMP_PRED_MODEL_FIXED_BYTES_m10);
		if (block_header->number_of_samples == 1)
			*((si4 *) (model + CMP_PRED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10)) = input_buffer[0];
		else
			warning_message_m10("%s(): No samples in block => returning without encoding\n", __FUNCTION__);
		return;
	}
	
	// generate differences & count
	memset((void *) count[0], 0, CMP_RED_MAX_STATS_BINS_m10 * CMP_PRED_CATS_m10 * sizeof(ui4));
	min = max = *(curr_si4_val_p = input_buffer);
	next_si4_val_p = curr_si4_val_p + 1;
	diff_p = (ui1 *) cps->difference_buffer;
	prev_cat = CMP_PRED_NIL_m10;
	for (i = block_header->number_of_samples; --i;) {
		if (*next_si4_val_p > max)
			max = *next_si4_val_p;
		else if (*next_si4_val_p < min)
			min = *next_si4_val_p;
		diff = *next_si4_val_p++ - *curr_si4_val_p++;
		if (diff > 127 || diff < -127) {
			ui1_p = (ui1 *) curr_si4_val_p;
			++count[prev_cat][*diff_p++ = CMP_RED_KEYSAMPLE_FLAG_m10];
			prev_cat = CMP_PRED_NEG_m10;
			j = 4; do {
				++count[prev_cat][*diff_p = *ui1_p++];
				prev_cat = CMP_PRED_CAT_m10(*diff_p); ++diff_p;  // do not increment within call to CAT
			} while (--j);
		}
		else {
			++count[prev_cat][*diff_p = (ui1)diff];
			prev_cat = CMP_PRED_CAT_m10(*diff_p); ++diff_p;  // do not increment within call to CAT
		}
	}
	total_diffs = (ui4)(diff_p - (ui1 *) cps->difference_buffer);
	cps->parameters.minimum_sample_value = min;
	cps->parameters.maximum_sample_value = max;
	
	// build sorted_count: interleave
	stats_entries = (ui2 *) (model + CMP_PRED_MODEL_NUMBERS_OF_STATISTICS_BINS_OFFSET_m10);
	for (total_stats_entries = i = 0; i < CMP_PRED_CATS_m10; ++i) {
		for (j = stats_entries[i] = 0, k = 255, l = 128; l--; ++j, --k) {
			if (count[i][j]) {
				sorted_count[i][stats_entries[i]].count = count[i][j];
				sorted_count[i][stats_entries[i]++].value = (si1) j;
			}
			if (count[i][k]) {
				sorted_count[i][stats_entries[i]].count = count[i][k];
				sorted_count[i][stats_entries[i]++].value = (si1) k;
			}
		}
		total_stats_entries += stats_entries[i];
	}
	
	// only one unique value (typically all zeros)
	if (total_stats_entries == 1 && sorted_count[CMP_PRED_NIL_m10][0].value == 0) {
		block_header->total_header_bytes = (ui4) (CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes + CMP_PRED_MODEL_FIXED_BYTES_m10);
		block_header->total_block_bytes = pad_m10((ui1 *)  block_header, block_header->total_header_bytes, 8);
		block_header->model_region_bytes = (ui2) CMP_PRED_MODEL_FIXED_BYTES_m10;
		memset(model, 0, CMP_PRED_MODEL_FIXED_BYTES_m10);
		*((si4 *) (model + CMP_PRED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10)) = input_buffer[0];
		return;
	}
	
	// build sorted_count: bubble sort
	for (i = 0; i < CMP_PRED_CATS_m10; ++i) {
		j = stats_entries[i];
		do {
			for (k = 0, l = 1; l < stats_entries[i]; ++l) {
				if (sorted_count[i][l - 1].count < sorted_count[i][l].count) {
					temp_sorted_count = sorted_count[i][l - 1];
					sorted_count[i][l - 1] = sorted_count[i][l];
					sorted_count[i][l] = temp_sorted_count;
					k = l;
				}
			}
		} while ((j = k) > 1);
	}
	
	// get separate count for each category
	for (i = 0; i < CMP_PRED_CATS_m10; ++i)
		for (cat_total_counts[i] = j = 0; j < stats_entries[i]; ++j)
			cat_total_counts[i] += sorted_count[i][j].count;
	
	// scale count so that each category's total_counts equals (RED_TOTAL_COUNTS - 1)
	goal_total_counts = CMP_RED_TOTAL_COUNTS_m10 - 1;
	for (i = 0; i < CMP_PRED_CATS_m10; ++i) {
		if (!stats_entries[i])
			continue;
		for (scaled_total_counts = j = 0; j < stats_entries[i]; ++j) {
			sorted_count[i][j].count = (ui4) (((((ui8) goal_total_counts << 1) * (ui8) sorted_count[i][j].count) + (ui8) cat_total_counts[i]) / ((ui8) cat_total_counts[i] << 1));
			if (sorted_count[i][j].count == 0)
				sorted_count[i][j].count = 1;
			scaled_total_counts += (si8)sorted_count[i][j].count;
		}
		extra_counts = ((si8) goal_total_counts - (si8)scaled_total_counts);
		if (extra_counts > 0) {
			do {
				for (j = 0; (j < stats_entries[i]) && extra_counts; ++j) {
					++sorted_count[i][j].count;
					--extra_counts;
				}
			} while (extra_counts);
		}
		else if (extra_counts < 0) {
			extra_counts = -extra_counts;
			do {
				for (j = stats_entries[i] - 1; (j >= 0) && extra_counts; --j) {
					if (sorted_count[i][j].count > 1) {
						--sorted_count[i][j].count;
						--extra_counts;
					}
				}
			} while (extra_counts);
		}
	}
	
	// build symbol maps, count arrays & minimum ranges
	for (i = 0; i < CMP_PRED_CATS_m10; ++i) {
		for (cumulative_count[i][0] = j = 0; j < stats_entries[i]; ++j) {
			symbol_map[i][(ui1) sorted_count[i][j].value] = (ui1) j;
			cumulative_count[i][j + 1] = cumulative_count[i][j] + (ui8) (count[i][j] = sorted_count[i][j].count);
			minimum_range[i][j] = CMP_RED_TOTAL_COUNTS_m10 / count[i][j];
			if (CMP_RED_TOTAL_COUNTS_m10 > (count[i][j] * minimum_range[i][j]))
				++minimum_range[i][j];
		}
	}
	
	// fill header (compression algorithms are responsible for filling in: total_bytes, header_bytes, & model_region_bytes, and model details)
	block_header->model_region_bytes = (ui2) (total_stats_entries * 3) + CMP_PRED_MODEL_FIXED_BYTES_m10;
	block_header->total_header_bytes = CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes + block_header->model_region_bytes;
	*((si4 *) (model + CMP_PRED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10)) = input_buffer[0];
	*((ui4 *) (model + CMP_PRED_MODEL_DIFFERENCE_BYTES_OFFSET_m10)) = total_diffs;
	
	// write scaled counts & synbols into header
	bin_counts = (ui2 *) (model + CMP_PRED_MODEL_BIN_COUNTS_OFFSET_m10);
	symbols = (ui1 *) (bin_counts + total_stats_entries);
	for (i = 0; i < CMP_PRED_CATS_m10; ++i) {
		for (j = 0; j < stats_entries[i]; ++j) {
			*bin_counts++ = (ui2)sorted_count[i][j].count;
			*symbols++ = sorted_count[i][j].value;
		}
	}
	
	// range encode
	diff_p = (ui1 *) cps->difference_buffer;
	comp_p = (ui1 *) block_header + block_header->total_header_bytes;
	low_bound_high_byte_p = ((ui1 *) &low_bound) + 5;
	high_bound_high_byte_p = ((ui1 *) &high_bound) + 5;
	low_bound = 0;
	range = CMP_RED_MAXIMUM_RANGE_m10;
	prev_cat = CMP_PRED_NIL_m10;
	
	for (i = total_diffs; i;) {
		for (; range >= minimum_range[prev_cat][bin = symbol_map[prev_cat][*diff_p]]; diff_p++) {
			high_bound = low_bound + ((range * cumulative_count[prev_cat][bin + 1]) >> 16);
			if (bin)
				low_bound += (range * cumulative_count[prev_cat][bin]) >> 16;
			range = high_bound - low_bound;
			if (!--i)
				break;
			prev_cat = CMP_PRED_CAT_m10(*diff_p);
		}
		if ((*low_bound_high_byte_p != *high_bound_high_byte_p) || !i) {
			ui1_p = low_bound_high_byte_p;
			j = 6; do {
				*comp_p++ = *ui1_p--;
			} while (--j);
			range = CMP_RED_MAXIMUM_RANGE_m10;
			low_bound = 0;
		}
		else {
			do {
				*comp_p++ = *low_bound_high_byte_p;
				low_bound <<= 8;
				high_bound <<= 8;
				range <<= 8;
			} while (*low_bound_high_byte_p == *high_bound_high_byte_p);
			low_bound &= CMP_RED_RANGE_MASK_m10;
		}
	}
	
	// finish header (compression algorithms are responsible for filling in: total_bytes, header_bytes, & model_region_bytes, and model details)
	PRED_total_bytes = (si8) (comp_p - (ui1 *) block_header);
	block_header->total_block_bytes = (ui4) pad_m10((ui1 *) block_header, PRED_total_bytes, 8);
	
	// calculate MBE encoding total bytes
	if (cps->directives.fall_through_to_MBE == TRUE_m10) {
		for (MBE_bits_per_samp = 0, i = (si8) max - (si8)min; i; i >>= 1)
			++MBE_bits_per_samp;
		MBE_cmp_data_bits = (si8) block_header->number_of_samples * (si8) MBE_bits_per_samp;
		MBE_cmp_data_bytes = MBE_cmp_data_bits >> 3;
		if (MBE_cmp_data_bits & 7)
			++MBE_cmp_data_bytes;
		MBE_total_bytes = MBE_cmp_data_bytes + (model - (ui1 *) block_header) + CMP_MBE_MODEL_FIXED_BYTES_m10;
		
		// MBE encode if smaller (neither number includes pad bytes, so can be off by up to 7 bytes)
		if (MBE_total_bytes < PRED_total_bytes) {
			block_header->block_flags &= ~CMP_BF_PRED_ENCODING_MASK_m10;
			block_header->block_flags |= CMP_BF_MBE_ENCODING_MASK_m10;
			CMP_MBE_encode_m10(cps);
			return;
		}
	}
	
	return;
}


// Algorithm from Niklaus Wirth's book: "Algorithms + data structures = programs".
// Code here is derived from code by Nicolas Devillard. Public domain.
sf8     CMP_quantile_value_m10(sf8 *x, si8 len, sf8 quantile, TERN_m10 preserve_input, sf8 *buff)
{
	TERN_m10        free_buff;
	sf8             q, fk, lo_p, lo_v, *lp, *mp, *last_mp, *lo_kp, *hi_kp;
	si8             lo_k;
	register sf8    v, t, *xip, *xjp;
	
	
	if (len == 1)
		return(*x);
	
	free_buff = FALSE_m10;
	if (preserve_input == TRUE_m10) {
		if (buff == NULL) {
			buff = (sf8 *) malloc_m10((size_t) len * sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
			if (buff == NULL) {
				error_message_m10("%s(): Not enough memory\n", __FUNCTION__);
				exit_m10(1);
			}
			free_buff = TRUE_m10;
		}
		memcpy(buff, x, len * sizeof(sf8));
	}
	else {
		buff = x;
	}
	
	if (quantile == (sf8)1.0) {
		lo_k = len - 2;
		lo_p = (sf8) 0.0;
	}
	else {
		fk = quantile * (sf8)(len - 1);
		lo_k = (si8)fk;
		lo_p = (sf8) 1.0 - (fk - (sf8)lo_k);
	}
	
	if (len == 2) {
		if (x[0] <= x[1])
			return((x[0] * lo_p) + (x[1] * (1.0 - lo_p)));
		return((x[1] * lo_p) + (x[0] * (1.0 - lo_p)));
	}
	
	lp = x;
	last_mp = mp = x + len - 1;
	lo_kp = x + lo_k;
	hi_kp = lo_kp + 1;
	while (lp < mp) {
		v = *lo_kp;
		xip = lp;
		xjp = mp;
		do {
			for (; *xip < v; ++xip);
			for (; v < *xjp; --xjp);
			if (xip <= xjp) {
				t = *xip;
				*xip++ = *xjp;
				*xjp-- = t;
			}
		} while (xip <= xjp);
		
		if (xjp < lo_kp)
			lp = xip;
		if (hi_kp < xip)
			last_mp = mp;
		if (lo_kp < xip)
			mp = xjp;
	}
	lo_v = *lo_kp;
	
	lp = lo_kp; mp = last_mp;
	while (lp < mp) {
		v = *hi_kp;
		xip = lp;
		xjp = mp;
		do {
			for (; *xip < v; ++xip);
			for (; v < *xjp; --xjp);
			if (xip <= xjp) {
				t = *xip;
				*xip++ = *xjp;
				*xjp-- = t;
			}
		} while (xip <= xjp);
		
		if (xjp < hi_kp)
			lp = xip;
		if (hi_kp < xip)
			mp = xjp;
	}
	
	q = (lo_v * lo_p) + (*hi_kp * ((sf8) 1.0 - lo_p));
	
	if (free_buff == TRUE_m10)
		free((void *) buff);
	
	return(q);
}


void    CMP_RED_decode_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	ui1	*comp_p, *ui1_p, *low_bound_high_byte_p, *high_bound_high_byte_p;
	ui1	*goal_bound_high_byte_p, prev_cat;
	ui1	*symbol_map[CMP_PRED_CATS_m10], *symbols, *model;
	si1	*si1_p1, *si1_p2, *diff_p;
	ui2	*bin_counts, n_derivs, *stats_entries, *count[CMP_PRED_CATS_m10];
	ui4	n_diffs, total_stats_entries;
	si4	*si4_p, current_val, init_val;
	ui8	*minimum_range[CMP_PRED_CATS_m10], *cumulative_count[CMP_PRED_CATS_m10], *ccs, *mrs;
	ui8	low_bound, high_bound, prev_high_bound, goal_bound, range;
	si8	i, j;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	// CMP decompress from block_header to decompressed_ptr
	block_header = cps->block_header;
	model = (ui1 *) block_header + block_header->total_header_bytes - block_header->model_region_bytes;
	init_val = *((si4 *) (model + CMP_RED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10));
	n_diffs = *((ui4 *) (model + CMP_RED_MODEL_DIFFERENCE_BYTES_OFFSET_m10));
	n_derivs = *((ui2 *) (model + CMP_RED_MODEL_DERIVATIVE_LEVEL_OFFSET_m10));
	if (block_header->number_of_samples == 0)  // if no samples, just return
		return;
	stats_entries = (ui2 *) (model + CMP_RED_MODEL_NUMBER_OF_STATISTICS_BINS_OFFSET_m10);
	for (total_stats_entries = i = 0; i < CMP_PRED_CATS_m10; ++i)
		total_stats_entries += stats_entries[i];
	if (total_stats_entries == 1) {  // only one value
		for (si4_p = cps->decompressed_ptr, i = block_header->number_of_samples; i--;)
			*si4_p++ = init_val;
		return;
	}
	
	// build symbol map, count arrays, & minimum ranges
	bin_counts = (ui2 *) (model + CMP_PRED_MODEL_BIN_COUNTS_OFFSET_m10);
	symbols = (ui1 *) (bin_counts + total_stats_entries);
	ccs = cps->cumulative_count[0];
	mrs = cps->minimum_range[0];
	for (i = 0; i < CMP_PRED_CATS_m10; ++i) {
		count[i] = bin_counts; bin_counts += stats_entries[i];
		symbol_map[i] = symbols; symbols += stats_entries[i];
		cumulative_count[i] = ccs; ccs += stats_entries[i];
		minimum_range[i] = mrs; mrs += stats_entries[i];
		for (cumulative_count[i][0] = j = 0; j < stats_entries[i]; ++j) {
			cumulative_count[i][j + 1] = cumulative_count[i][j] + (ui8)count[i][j];
			minimum_range[i][j] = CMP_RED_TOTAL_COUNTS_m10 / count[i][j];
			if (CMP_RED_TOTAL_COUNTS_m10 > (count[i][j] * minimum_range[i][j]))
				++minimum_range[i][j];
		}
	}
	
	// range decode
	diff_p = cps->difference_buffer;
	prev_high_bound = goal_bound = low_bound = 0;
	range = CMP_RED_MAXIMUM_RANGE_m10;
	comp_p = (ui1 *) block_header + block_header->total_header_bytes;
	low_bound_high_byte_p = ((ui1 *) &low_bound) + 5;
	high_bound_high_byte_p = ((ui1 *) &high_bound) + 5;
	goal_bound_high_byte_p = ((ui1 *) &goal_bound) + 5;
	ui1_p = goal_bound_high_byte_p;
	j = 6; do {
		*ui1_p-- = *comp_p++;
	} while (--j);
	prev_cat = CMP_PRED_NIL_m10;
	
	for (i = n_diffs; i;) {
		for (j = 0; range >= minimum_range[prev_cat][j];) {
			high_bound = low_bound + ((range * cumulative_count[prev_cat][j + 1]) >> 16);
			if (high_bound > goal_bound) {
				range = high_bound - (low_bound = prev_high_bound);
				*diff_p = symbol_map[prev_cat][j];
				if (!--i)
					goto PRED_RANGE_DECODE_DONE;
				prev_cat = CMP_PRED_CAT_m10(*diff_p); ++diff_p;  // do not increment within call to CAT
				j = 0;
			}
			else {
				prev_high_bound = high_bound;
				++j;
			}
		}
		high_bound = low_bound + range;
		if (*low_bound_high_byte_p != *high_bound_high_byte_p) {
			ui1_p = goal_bound_high_byte_p;
			j = 6; do {
				*ui1_p-- = *comp_p++;
			} while (--j);
			low_bound = 0;
			range = CMP_RED_MAXIMUM_RANGE_m10;
		}
		else {
			do {
				low_bound <<= 8;
				high_bound <<= 8;
				goal_bound = (goal_bound << 8) | (ui8) *comp_p++;
				range <<= 8;
			} while (*low_bound_high_byte_p == *high_bound_high_byte_p);
			low_bound &= CMP_RED_RANGE_MASK_m10;
			goal_bound &= CMP_RED_RANGE_MASK_m10;
		}
		prev_high_bound = low_bound;
	} PRED_RANGE_DECODE_DONE:
	
	// generate output from difference data
	si1_p1 = cps->difference_buffer;
	si4_p = cps->decompressed_ptr;
	*si4_p++ = current_val = init_val;
	for (i = block_header->number_of_samples; --i;) {
		if (*si1_p1 == -128) {
			++si1_p1;
			si1_p2 = (si1 *) &current_val;
			*si1_p2++ = *si1_p1++; *si1_p2++ = *si1_p1++; *si1_p2++ = *si1_p1++; *si1_p2 = *si1_p1++;
		}
		else
			current_val += (si4)*si1_p1++;
		*si4_p++ = current_val;
	}
	
	return;
}


void    CMP_RED_encode_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	ui1	*low_bound_high_byte_p, *high_bound_high_byte_p, *ui1_val_p, prev_cat;
	ui1	*diff_p, *comp_p, *symbols, **symbol_map, *model;
	ui2	*bin_counts, *stats_entries, *num_stats;
	ui4	variable_region_bytes, **count, total_diffs, PRED_total_bytes, MBE_total_bytes, total_stats_entries;
	ui4	cat_total_counts[CMP_PRED_CATS_m10], goal_total_counts, bin, MBE_cmp_data_bytes;
	si4	*input_buffer, *curr_si4_val_p, *next_si4_val_p, diff, min, max, MBE_bits_per_samp;
	ui8	**cumulative_count, **minimum_range;
	ui8	range, high_bound, low_bound;
	si8	i, j, k, l, extra_counts, scaled_total_counts, MBE_cmp_data_bits;
	CMP_STATISTICS_BIN_m10		**sorted_count, temp_sorted_count;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	// CMP compress from input_buffer to to block_header -> compressed data array
	count = cps->count;
	cumulative_count = cps->cumulative_count;
	minimum_range = cps->minimum_range;
	sorted_count = cps->sorted_count;
	symbol_map = cps->symbol_map;
	block_header = cps->block_header;
	input_buffer = cps->input_buffer;
	variable_region_bytes = cps->parameters.variable_region_bytes;
	model = (ui1 *)block_header + CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes;
	
	// if no samples: fill in an empty block header & return;
	if (block_header->number_of_samples <= 0) {
		block_header->total_header_bytes = CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes;
		block_header->model_region_bytes = (ui2) 0;
		block_header->total_block_bytes = pad_m10((ui1 *) block_header, block_header->total_header_bytes, 8);
		error_message_m10("%s(): No samples in block => returning without encoding\n", __FUNCTION__);
		return;
	}
	
	// generate differences & count
	memset((void *) count[0], 0, CMP_RED_MAX_STATS_BINS_m10 * CMP_PRED_CATS_m10 * sizeof(ui4));
	min = max = *(curr_si4_val_p = input_buffer);
	next_si4_val_p = curr_si4_val_p + 1;
	diff_p = (ui1 *) cps->difference_buffer;
	prev_cat = CMP_PRED_NIL_m10;
	for (i = block_header->number_of_samples; --i;) {
		if (*next_si4_val_p > max)
			max = *next_si4_val_p;
		else if (*next_si4_val_p < min)
			min = *next_si4_val_p;
		diff = *next_si4_val_p++ - *curr_si4_val_p++;
		if (diff > 127 || diff < -127) {
			ui1_val_p = (ui1 *)curr_si4_val_p;
			++count[prev_cat][*diff_p++ = CMP_RED_KEYSAMPLE_FLAG_m10];
			prev_cat = CMP_PRED_NEG_m10;
			j = 4; do {
				++count[prev_cat][*diff_p = *ui1_val_p++];
				prev_cat = CMP_PRED_CAT_m10(*diff_p); ++diff_p;  // do not increment within call to CAT
			} while (--j);
		}
		else {
			++count[prev_cat][*diff_p = (ui1) diff];
			prev_cat = CMP_PRED_CAT_m10(*diff_p); ++diff_p;  // do not increment within call to CAT
		}
	}
	total_diffs = (ui4) (diff_p - (ui1 *) cps->difference_buffer);
	cps->parameters.minimum_sample_value = min;
	cps->parameters.maximum_sample_value = max;
	
	// build sorted_count: interleave
	stats_entries = (ui2 *) (model + CMP_PRED_MODEL_NUMBERS_OF_STATISTICS_BINS_OFFSET_m10);
	for (total_stats_entries = i = 0; i < CMP_PRED_CATS_m10; ++i) {
		for (j = stats_entries[i] = 0, k = 255, l = 128; l--; ++j, --k) {
			if (count[i][j]) {
				sorted_count[i][stats_entries[i]].count = count[i][j];
				sorted_count[i][stats_entries[i]++].value = (si1) j;
			}
			if (count[i][k]) {
				sorted_count[i][stats_entries[i]].count = count[i][k];
				sorted_count[i][stats_entries[i]++].value = (si1) k;
			}
		}
		total_stats_entries += stats_entries[i];
	}
	
	// only one value
	if (total_stats_entries == 1) {
		block_header->total_header_bytes = (ui4) (CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes + CMP_PRED_MODEL_FIXED_BYTES_m10);
		block_header->total_block_bytes = pad_m10((ui1 *) block_header, block_header->total_header_bytes, 8);
		block_header->model_region_bytes = (ui2)CMP_PRED_MODEL_FIXED_BYTES_m10;
		memset(model, 0, CMP_PRED_MODEL_FIXED_BYTES_m10);
		*((ui2 *) (model + CMP_PRED_MODEL_NUMBER_OF_NIL_STATISTICS_BINS_OFFSET_m10)) = 1;
		return;
	}
	
	// build sorted_count: bubble sort
	for (i = 0; i < CMP_PRED_CATS_m10; ++i) {
		j = stats_entries[i];
		do {
			for (k = 0, l = 1; l < stats_entries[i]; ++l) {
				if (sorted_count[i][l - 1].count < sorted_count[i][l].count) {
					temp_sorted_count = sorted_count[i][l - 1];
					sorted_count[i][l - 1] = sorted_count[i][l];
					sorted_count[i][l] = temp_sorted_count;
					k = l;
				}
			}
		} while ((j = k) > 1);
	}
	
	// get separate count for each category
	for (i = 0; i < CMP_PRED_CATS_m10; ++i)
		for (cat_total_counts[i] = j = 0; j < stats_entries[i]; ++j)
			cat_total_counts[i] += sorted_count[i][j].count;
	
	// scale count so that cat_total_counts equals (RED_TOTAL_COUNTS - 1)
	goal_total_counts = CMP_RED_TOTAL_COUNTS_m10 - 1;
	for (i = 0; i < CMP_PRED_CATS_m10; ++i) {
		for (scaled_total_counts = j = 0; j < stats_entries[i]; ++j) {
			sorted_count[i][j].count = (ui4) (((((ui8) goal_total_counts << 1) * (ui8) sorted_count[i][j].count) + (ui8) cat_total_counts[i]) / ((ui8) cat_total_counts[i] << 1));
			if (sorted_count[i][j].count == 0)
				sorted_count[i][j].count = 1;
			scaled_total_counts += (si8)sorted_count[i][j].count;
		}
		extra_counts = ((si8) goal_total_counts - (si8) scaled_total_counts);
		if (extra_counts > 0) {
			do {
				for (j = 0; (j < stats_entries[i]) && extra_counts; ++j) {
					++sorted_count[i][j].count;
					--extra_counts;
				}
			} while (extra_counts);
		}
		else if (extra_counts < 0) {
			extra_counts = -extra_counts;
			do {
				for (j = stats_entries[i] - 1; (j >= 0) && extra_counts; --j) {
					if (sorted_count[i][j].count > 1) {
						--sorted_count[i][j].count;
						--extra_counts;
					}
				}
			} while (extra_counts);
		}
	}
	
	// build symbol maps, count arrays & minimum ranges
	for (i = 0; i < CMP_PRED_CATS_m10; ++i) {
		for (cumulative_count[i][0] = j = 0; j < stats_entries[i]; ++j) {
			symbol_map[i][(ui1)sorted_count[i][j].value] = (ui1)j;
			cumulative_count[i][j + 1] = cumulative_count[i][j] + (ui8) (count[i][j] = sorted_count[i][j].count);
			minimum_range[i][j] = CMP_RED_TOTAL_COUNTS_m10 / count[i][j];
			if (CMP_RED_TOTAL_COUNTS_m10 > (count[i][j] * minimum_range[i][j]))
				++minimum_range[i][j];
		}
	}
	
	// range encode
	diff_p = (ui1 *) cps->difference_buffer;
	comp_p = (ui1 *) block_header + CMP_BLOCK_FIXED_HEADER_BYTES_m10 + (CMP_PRED_CATS_m10 * sizeof(ui4)) + (3 * total_stats_entries);
	low_bound_high_byte_p = ((ui1 *) &low_bound) + 5;
	high_bound_high_byte_p = ((ui1 *) &high_bound) + 5;
	low_bound = 0;
	range = CMP_RED_MAXIMUM_RANGE_m10;
	prev_cat = CMP_PRED_CATS_m10;
	
	for (i = total_diffs; i;) {
		for (; range >= minimum_range[prev_cat][bin = symbol_map[prev_cat][*diff_p]]; diff_p++) {
			high_bound = low_bound + ((range * cumulative_count[prev_cat][bin + 1]) >> 16);
			if (bin)
				low_bound += (range * cumulative_count[prev_cat][bin]) >> 16;
			range = high_bound - low_bound;
			if (!--i)
				break;
			prev_cat = CMP_PRED_CAT_m10(*diff_p);  // do not increment within call to CAT
		}
		if ((*low_bound_high_byte_p != *high_bound_high_byte_p) || !i) {
			j = 6; do {
				*comp_p++ = *low_bound_high_byte_p--;
			} while (--j);
			range = CMP_RED_MAXIMUM_RANGE_m10;
			low_bound = 0;
		}
		else {
			do {
				*comp_p++ = *low_bound_high_byte_p;
				low_bound <<= 8;
				high_bound <<= 8;
				range <<= 8;
			} while (*low_bound_high_byte_p == *high_bound_high_byte_p);
			low_bound &= CMP_RED_RANGE_MASK_m10;
		}
	}
	PRED_total_bytes = (si8) (comp_p - (ui1 *) block_header);
	
	// calculate MBE encoding total bytes
	if (cps->directives.fall_through_to_MBE == TRUE_m10) {
		for (MBE_bits_per_samp = 0, i = (si8) max - (si8) min; i; i >>= 1)
			++MBE_bits_per_samp;
		MBE_cmp_data_bits = (si8) block_header->number_of_samples * (si8) MBE_bits_per_samp;
		MBE_cmp_data_bytes = MBE_cmp_data_bits >> 3;
		if (MBE_cmp_data_bits & 7)
			++MBE_cmp_data_bytes;
		
		// MBE encode if smaller
		MBE_total_bytes = CMP_MBE_MODEL_FIXED_BYTES_m10 + variable_region_bytes + CMP_MBE_MODEL_FIXED_BYTES_m10 + MBE_cmp_data_bytes;
		
		if (MBE_total_bytes < PRED_total_bytes) {
			block_header->block_flags &= ~CMP_BF_PRED_ENCODING_MASK_m10;
			block_header->block_flags |= CMP_BF_MBE_ENCODING_MASK_m10;
			CMP_MBE_encode_m10(cps);
			return;
		}
	}
	
	// fill header (compression algorithms are responsible for filling in: total_bytes, header_bytes, & model_region_bytes, and model details)
	block_header->total_block_bytes = pad_m10((ui1 *) block_header, PRED_total_bytes, 8);
	block_header->model_region_bytes = (ui2) (total_stats_entries * 3) + CMP_PRED_MODEL_FIXED_BYTES_m10;
	block_header->total_header_bytes = CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes + block_header->model_region_bytes;
	*((si4 *) (model + CMP_PRED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10)) = input_buffer[0];
	*((ui4 *) (model + CMP_PRED_MODEL_DIFFERENCE_BYTES_OFFSET_m10)) = total_diffs;
	
	// write scaled count & bins into header
	num_stats = (ui2 *) (model + CMP_PRED_MODEL_BIN_COUNTS_OFFSET_m10);
	bin_counts = (ui2 *) (num_stats + CMP_PRED_CATS_m10);
	symbols = (ui1 *) (bin_counts + total_stats_entries);
	for (i = 0; i < CMP_PRED_CATS_m10; ++i) {
		*num_stats++ = stats_entries[i];
		for (j = 0; j < stats_entries[i]; ++j) {
			*bin_counts++ = (ui2) sorted_count[i][j].count;
			*symbols++ = sorted_count[i][j].value;
		}
	}
	
	return;
}


void    CMP_retrend_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 m, sf8 b)
{
	si8     i;
	sf8     mx_plus_b;
	
	
	// retrend data from input_buffer to output_buffer
	// if input_buffer == output_buffer retrending data will be done in place
	
	mx_plus_b = b;
	for (i = len; i--;)
		*output_buffer++ = CMP_round_m10((sf8) *input_buffer++ + (mx_plus_b += m));
	
	return;
}


inline si4      CMP_round_m10(sf8 val)
{
	if (isnan(val))
		return(NAN_m10);
	
	if (val >= (sf8) 0.0) {
		if ((val += (sf8) 0.5) > (sf8) POSITIVE_INFINITY_m10)
			return(POSITIVE_INFINITY_m10);
	}
	else {
		if ((val -= (sf8) 0.5) < (sf8) NEGATIVE_INFINITY_m10)
			return(NEGATIVE_INFINITY_m10);
	}
	
	return((si4)val);
}


void    CMP_scale_amplitude_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor, CMP_PROCESSING_STRUCT_m10 *cps)
{
	si4	*si4_p1, *si4_p2;
	sf4	sf4_scale;
	si8	i;
	
	
	// scale from input_buffer to output_buffer
	// if input_buffer == output_buffer scaling will be done in place
	
	// store scale in block parameter region
	// NOTE: block parameter region must be setup first
	if (cps != NULL) {
		// demote precision
		sf4_scale = (sf4) scale_factor;
		// store
		*((sf4 *) cps->parameters.block_parameters + cps->parameters.block_parameter_map[CMP_PF_AMPLITUDE_SCALE_IDX_m10]) = scale_factor;
		// promote back to sf8 precision
		scale_factor = (sf8)sf4_scale;
	}
	
	si4_p1 = input_buffer;
	si4_p2 = output_buffer;
	for (i = len; i--;)
		*si4_p2++ = CMP_round_m10((sf8) *si4_p1++ / scale_factor);
	
	return;
}


void    CMP_scale_frequency_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor, CMP_PROCESSING_STRUCT_m10 *cps)
{
	sf4	sf4_scale;
	
	
	// scale from input_buffer to output_buffer
	// if input_buffer == output_buffer scaling will be done in place
	
	// store scale in block parameter region
	// NOTE: block parameter region must be setup first
	if (cps != NULL) {
		// demote precision
		sf4_scale = (sf4)scale_factor;
		// store
		*((sf4 *) cps->parameters.block_parameters + cps->parameters.block_parameter_map[CMP_PF_FREQUENCY_SCALE_IDX_m10]) = scale_factor;
		// promote back to sf8 precision
		scale_factor = (sf8) sf4_scale;
	}
	
	// actual frequency scaling not written yet
	
	return;
}


void    CMP_set_variable_region_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
	ui1				*var_reg_ptr;
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	block_header = cps->block_header;
	
	// reset variable region parameters
	block_header->number_of_records = cps->parameters.user_number_of_records;
	block_header->record_region_bytes = cps->parameters.user_record_region_bytes;
	block_header->parameter_flags = cps->parameters.user_parameter_flags;
	block_header->protected_region_bytes = cps->parameters.protected_region_bytes;
	block_header->discretionary_region_bytes = cps->parameters.user_discretionary_region_bytes;
	
	// records region
	var_reg_ptr = (ui1 *)block_header + CMP_BLOCK_FIXED_HEADER_BYTES_m10; // pointer to start of variable region
	cps->records = var_reg_ptr;
	var_reg_ptr += block_header->record_region_bytes;
	
	// parameter region
	cps->parameters.block_parameters = (ui4*)var_reg_ptr;
	
	// set library parameter flags
	if (cps->directives.detrend_data == TRUE_m10)
		block_header->parameter_flags |= (CMP_PF_INTERCEPT_MASK_m10 | CMP_PF_GRADIENT_MASK_m10);
	else
		block_header->parameter_flags &= ~(CMP_PF_INTERCEPT_MASK_m10 | CMP_PF_GRADIENT_MASK_m10);
	
	if (cps->directives.set_amplitude_scale == TRUE_m10 || cps->directives.find_amplitude_scale == TRUE_m10)
		block_header->parameter_flags |= CMP_PF_AMPLITUDE_SCALE_MASK_m10;
	else
		block_header->parameter_flags &= ~CMP_PF_AMPLITUDE_SCALE_MASK_m10;
	
	if (cps->directives.set_frequency_scale == TRUE_m10 || cps->directives.find_frequency_scale == TRUE_m10)
		block_header->parameter_flags |= CMP_PF_FREQUENCY_SCALE_MASK_m10;
	else
		block_header->parameter_flags &= ~CMP_PF_FREQUENCY_SCALE_MASK_m10;
	
	if (cps->directives.include_noise_scores == TRUE_m10)
		block_header->parameter_flags |= CMP_PF_NOISE_SCORES_MASK_m10;
	else
		block_header->parameter_flags &= ~CMP_PF_NOISE_SCORES_MASK_m10;
	
	CMP_generate_parameter_map_m10(cps);
	var_reg_ptr += block_header->parameter_region_bytes;
	
	// protected region
	cps->protected_region = var_reg_ptr;
	var_reg_ptr += block_header->protected_region_bytes;
	
	// discretionary region
	cps->discretionary_region = var_reg_ptr;
	var_reg_ptr += block_header->discretionary_region_bytes;
	
	// variable region bytes
	cps->parameters.variable_region_bytes = CMP_VARIABLE_REGION_BYTES_v1_m10(block_header);
	
	// model region
	// NOTE: model region is not considered part of the variable region, but convenient to set it here
	// NOTE: model region bytes is set by compression function
	cps->model_region = var_reg_ptr;
	
	return;
}


void    CMP_show_block_header_m10(CMP_BLOCK_FIXED_HEADER_m10 *bh)
{
	si1     hex_str[HEX_STRING_BYTES_m10(CRC_BYTES_m10)], time_str[TIME_STRING_BYTES_m10];
	ui4     i, mask;
	
	
	printf_m10("--------------- CMP Fixed Block Header - START ---------------\n");
	printf_m10("Block Start UID: 0x%lx\n", bh->block_start_UID);
	if (bh->block_CRC == CRC_NO_ENTRY_m10)
		printf_m10("Block CRC: no entry\n");
	else {
		generate_hex_string_m10((ui1 *)&bh->block_CRC, CRC_BYTES_m10, hex_str);
		printf_m10("Block CRC: %s\n", hex_str);
	}
	printf_m10("Block Flag Bits: ");
	for (i = 0, mask = 1; i < 32; ++i, mask <<= 1) {
		if (bh->block_flags & mask)
			printf_m10("%d ", i);
	}
	printf_m10(" (value: 0x%08x)\n", bh->block_flags);
	if (bh->start_time == UUTC_NO_ENTRY_m10)
		printf_m10("Start Time: no entry\n");
	else {
		time_string_m10(bh->start_time, time_str, TRUE_m10, FALSE_m10, FALSE_m10);
		printf_m10("Start Time: %ld (µUTC), %s\n", bh->start_time, time_str);
	}
	printf_m10("Acquisition Channel Number: %u\n", bh->acquisition_channel_number);
	printf_m10("Total Block Bytes: %u\n", bh->total_block_bytes);
	printf_m10("Number of Samples: %u\n", bh->number_of_samples);
	printf_m10("Number of Records: %hu\n", bh->number_of_records);
	printf_m10("Parameter Flag Bits: ");
	for (i = 0, mask = 1; i < 32; ++i, mask <<= 1) {
		if (bh->parameter_flags & mask)
			printf_m10("%d ", i);
	}
	printf_m10(" (value: 0x%08x)\n", bh->parameter_flags);
	printf_m10("Parameter Region Bytes: %hu\n", bh->parameter_region_bytes);
	printf_m10("Protected Region Bytes: %hu\n", bh->protected_region_bytes);
	printf_m10("Discretionary Region Bytes: %hu\n", bh->discretionary_region_bytes);
	printf_m10("Model Region Bytes: %hu\n", bh->model_region_bytes);
	printf_m10("Total Header Bytes: %u\n", bh->total_header_bytes);
	printf_m10("---------------- CMP Fixed Block Header - END ----------------\n\n");
	
	return;
}


void    CMP_show_block_model_m10(CMP_BLOCK_FIXED_HEADER_m10 *block_header)
{
	ui1	*model, bits_per_sample, derivative_level, no_zero_counts_flag;
	si1	*symbols;
	ui2     number_of_statistics_bins, number_of_NIL_bins, number_of_POS_bins, number_of_NEG_bins, *counts;
	ui4     difference_bytes, variable_region_bytes;
	si4     minimum_value, initial_sample_value, i;
	
	
	variable_region_bytes = CMP_VARIABLE_REGION_BYTES_v1_m10(block_header);
	model = (ui1 *)block_header + CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes;
	
	printf_m10("------------------- CMP Block Model - START ------------------\n");
	switch (block_header->block_flags & CMP_BF_ALGORITHM_MASK_m10) {
		case CMP_BF_RED_ENCODING_MASK_m10:
			printf_m10("Model: Range Encoded Differences (RED)\n");
			initial_sample_value = *((si4 *) (model + CMP_RED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10));
			printf_m10("Initial Sample Value: %d\n", initial_sample_value);
			difference_bytes = *((ui4 *) (model + CMP_RED_MODEL_DIFFERENCE_BYTES_OFFSET_m10));
			printf_m10("Difference Bytes: %u\n", difference_bytes);
			derivative_level = *((ui1 *) (model + CMP_RED_MODEL_DERIVATIVE_LEVEL_OFFSET_m10));
			printf_m10("Derivative Level: %hhu\n", derivative_level);
			no_zero_counts_flag = *((ui1 *) (model + CMP_RED_MODEL_NO_ZERO_COUNTS_FLAG_OFFSET_m10));
			printf_m10("No Zero Counts Flag: %hhu\n", no_zero_counts_flag);
			number_of_statistics_bins = *((ui2 *) (model + CMP_RED_MODEL_NUMBER_OF_STATISTICS_BINS_OFFSET_m10));
			// end fixed RED model fields
			counts = (ui2 *) (model + CMP_RED_MODEL_NUMBER_OF_STATISTICS_BINS_OFFSET_m10);
			symbols = (si1 *) (counts + number_of_statistics_bins);
			printf_m10("\nNumber of Statistics Bins: %u\n", number_of_statistics_bins);
			for (i = 0; i < number_of_statistics_bins; ++i)
				printf_m10("\tsymbol: %hhd\tcount: %hu\n", *symbols++, *counts++);
			printf_m10("\n");
			break;
			
		case CMP_BF_PRED_ENCODING_MASK_m10:
			printf_m10("Model: Predictive Range Encoded Differences (PRED)\n");
			initial_sample_value = *((si4 *) (model + CMP_PRED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10));
			printf_m10("Initial Sample Value: %d\n", initial_sample_value);
			difference_bytes = *((ui4 *) (model + CMP_PRED_MODEL_DIFFERENCE_BYTES_OFFSET_m10));
			printf_m10("Difference Bytes: %u\n", difference_bytes);
			derivative_level = *((ui1 *) (model + CMP_PRED_MODEL_DERIVATIVE_LEVEL_OFFSET_m10));
			printf_m10("Derivative Level: %hhu\n", derivative_level);
			no_zero_counts_flag = *((ui1 *) (model + CMP_PRED_MODEL_NO_ZERO_COUNTS_FLAG_OFFSET_m10));
			printf_m10("No Zero Counts Flag: %hhu\n", no_zero_counts_flag);
			number_of_NIL_bins = *((ui2 *) (model + CMP_PRED_MODEL_NUMBER_OF_NIL_STATISTICS_BINS_OFFSET_m10));
			number_of_POS_bins = *((ui2 *) (model + CMP_PRED_MODEL_NUMBER_OF_POS_STATISTICS_BINS_OFFSET_m10));
			number_of_NEG_bins = *((ui2 *) (model + CMP_PRED_MODEL_NUMBER_OF_NEG_STATISTICS_BINS_OFFSET_m10));
			// end fixed PRED model fields
			counts = (ui2 *) (model + CMP_PRED_MODEL_BIN_COUNTS_OFFSET_m10);
			symbols = (si1 *) (counts + (number_of_NIL_bins + number_of_POS_bins + number_of_NEG_bins));
			printf_m10("\nNumber of NIL Statistics Bins: %u\n", number_of_NIL_bins);
			for (i = 0; i < number_of_NIL_bins; ++i)
				printf_m10("bin %02d:    symbol: %hhd\tcount: %hu\n", i, *symbols++, *counts++);
			printf_m10("\nNumber of POS Statistics Bins: %u\n", number_of_POS_bins);
			for (i = 0; i < number_of_POS_bins; ++i)
				printf_m10("bin %02d:    symbol: %hhd\tcount: %hu\n", i, *symbols++, *counts++);
			printf_m10("\nNumber of NEG Statistics Bins: %u\n", number_of_NEG_bins);
			for (i = 0; i < number_of_NEG_bins; ++i)
				printf_m10("bin %02d:    symbol: %hhd\tcount: %hu\n", i, *symbols++, *counts++);
			printf_m10("\n");
			break;
			
		case CMP_BF_MBE_ENCODING_MASK_m10:
			printf_m10("Model: Minimal Bit Encoding (MBE)\n");
			minimum_value = *((si4 *) (model + CMP_MBE_MODEL_MINIMUM_SAMPLE_VALUE_OFFSET_m10));
			printf_m10("Minimum Sample Value: %d\n", minimum_value);
			bits_per_sample = *((ui1 *) (model + CMP_MBE_MODEL_BITS_PER_SAMPLE_OFFSET_m10));
			printf_m10("Bits per Sample: %hhu\n", bits_per_sample);
			break;
			
		default:
			error_message_m10("%s(): Unrecognized model (%u)\n", __FUNCTION__, block_header->block_flags & CMP_BF_ALGORITHM_MASK_m10);
			break;
	}
	printf_m10("-------------------- CMP Block Model - END -------------------\n\n");
	
	return;
}


void    CMP_unscale_amplitude_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor)
{
	si4	*si4_p1, *si4_p2;
	si8     i;
	
	
	// unscale from input_buffer to output_buffer
	// if input_buffer == output_buffer unscaling will be done in place
	
	si4_p1 = input_buffer;
	si4_p2 = output_buffer;
	for (i = len; i--;)
		*si4_p2++ = CMP_round_m10((sf8)*si4_p1++ * scale_factor);
	
	return;
}


void    CMP_unscale_frequency_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor)
{
	// not written yet
	return;
}


inline CMP_BLOCK_FIXED_HEADER_m10	*CMP_update_CPS_pointers_m10(CMP_PROCESSING_STRUCT_m10 *cps, ui1 flags)
{
	CMP_BLOCK_FIXED_HEADER_m10	*block_header;
	
	
	block_header = cps->block_header;
	if (flags & CMP_UPDATE_ORIGINAL_PTR_m10)
		cps->input_buffer = (cps->original_ptr += block_header->number_of_samples);
	if (flags & CMP_UPDATE_BLOCK_HEADER_PTR_m10)
		cps->block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *) cps->block_header + block_header->total_block_bytes);
	if (flags & CMP_UPDATE_DECOMPRESSED_PTR_m10)
		cps->decompressed_ptr += block_header->number_of_samples;
	
	return(cps->block_header);
}



//***********************************************************************//
//***************************  CRC FUNCTIONS  ***************************//
//***********************************************************************//

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


inline ui4	CRC_calculate_m10(const ui1 *block_ptr, si8 block_bytes)
{
	ui4	crc;
	
	crc = CRC_update_m10(block_ptr, block_bytes, CRC_START_VALUE_m10);
	
	return(crc);
}


ui4     CRC_combine_m10(ui4 block_1_crc, ui4 block_2_crc, si8 block_2_bytes)
{
	ui4     n, col;
	ui4     even[32];    // even-power-of-two zeros operator
	ui4     odd[32];     // odd-power-of-two zeros operator
	
	
	// degenerate case (also disallow negative lengths)
	if (block_2_bytes <= 0)
		return(block_1_crc);
	
	// put operator for one zero bit in odd
	odd[0] = CRC_POLYNOMIAL_m10;
	col = 1;
	for (n = 1; n < 32; n++) {
		odd[n] = col;
		col <<= 1;
	}
	
	// put operator for two zero bits in even
	CRC_matrix_square_m10(even, odd);
	
	// put operator for four zero bits in odd
	CRC_matrix_square_m10(odd, even);
	
	// apply block_2_bytes zeros to crc1 (first square will put the operator for one zero byte, eight zero bits, in even)
	do {
		// apply zeros operator for this bit of block_2_bytes
		CRC_matrix_square_m10(even, odd);
		if (block_2_bytes & 1)
			block_1_crc = CRC_matrix_times_m10(even, block_1_crc);
		block_2_bytes >>= 1;
		
		// if no more bits set, then done
		if (block_2_bytes == 0)
			break;
		
		// another iteration of the loop with odd and even swapped
		CRC_matrix_square_m10(odd, even);
		if (block_2_bytes & 1)
			block_1_crc = CRC_matrix_times_m10(odd, block_1_crc);
		block_2_bytes >>= 1;
		
		// if no more bits set, then done
	} while (block_2_bytes != 0);
	
	return(block_1_crc ^ block_2_crc);
}


TERN_m10	CRC_initialize_tables_m10(void)
{
	ui4	**crc_table, c, n, k;
	
	
	if (globals_m10->CRC_table != NULL)
		free((void *) globals_m10->CRC_table);
	globals_m10->CRC_table = (ui4 **) calloc_2D_m10((size_t) CRC_TABLES_m10, (size_t) CRC_TABLE_ENTRIES_m10, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	// generate a crc for every 8-bit value
	crc_table = globals_m10->CRC_table;
	for (n = 0; n < CRC_TABLE_ENTRIES_m10; n++) {
		for (c = n, k = 0; k < 8; k++)
			c = c & 1 ? CRC_POLYNOMIAL_m10 ^ (c >> 1) : c >> 1;
		crc_table[0][n] = c;
	}
	
	// generate crc for each value followed by one, two, and three zeros, and then the byte reversal of those as well as the first table
	for (n = 0; n < CRC_TABLE_ENTRIES_m10; n++) {
		c = crc_table[0][n];
		crc_table[4][n] = CRC_SWAP32_m10(c);
		for (k = 1; k < 4; k++) {
			c = crc_table[0][c & 0xff] ^ (c >> 8);
			crc_table[k][n] = c;
			crc_table[k + 4][n] = CRC_SWAP32_m10(c);
		}
	}
	
	return(TRUE_m10);
}


inline void    CRC_matrix_square_m10(ui4 *square, const ui4 *mat)
{
	ui4     n;
	
	for (n = 0; n < 32; n++)
		square[n] = CRC_matrix_times_m10(mat, mat[n]);
	
	return;
}


inline ui4      CRC_matrix_times_m10(const ui4 *mat, ui4 vec)
{
	ui4     sum;
	
	sum = 0;
	while (vec) {
		if (vec & 1)
			sum ^= *mat;
		vec >>= 1;
		mat++;
	}
	
	return(sum);
}


inline ui4	CRC_update_m10(const ui1 *block_ptr, si8 block_bytes, ui4 current_crc)
{
	ui4			**crc_table;
	register ui4            c;
	register const ui4	*ui4_buf;
	
	
	if (globals_m10->CRC_table == NULL)  {
		if (CRC_initialize_tables_m10() == FALSE_m10) {
			error_message_m10("%s(): error\n", __FUNCTION__);
			return(0);
		}
	}
	
	crc_table = globals_m10->CRC_table;
	
	c = ~current_crc;
	
	// bring block_ptr to 4 byte alignment
	while (block_bytes && ((ui8)block_ptr & (ui8)3)) {
		c = crc_table[0][(c ^ *block_ptr++) & 0xFF] ^ (c >> 8);
		block_bytes--;
	}
	
	ui4_buf = (const ui4*)block_ptr;
	while (block_bytes >= 32) {
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		block_bytes -= 32;
	}
	while (block_bytes >= 4) {
		c ^= *ui4_buf++;
		c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24];
		block_bytes -= 4;
	}
	block_ptr = (const ui1 *) ui4_buf;
	
	if (block_bytes) do {
		c = crc_table[0][(c ^ *block_ptr++) & 0xff] ^ (c >> 8);
	} while (--block_bytes);
	
	return(~c);
}


inline TERN_m10	CRC_validate_m10(const ui1 *block_ptr, si8 block_bytes, ui4 crc_to_validate)
{
	ui4	crc;
	
	crc = CRC_calculate_m10(block_ptr, block_bytes);
	
	if (crc == crc_to_validate)
		return(TRUE_m10);
	
	return(FALSE_m10);
}



//***********************************************************************//
//************  FILE PROCESSING STRUCT STANDARD FUNCTIONS  **************//
//***********************************************************************//

		
inline void	FPS_close_m10(FILE_PROCESSING_STRUCT_m10 *fps) {
	
	if (fps->fp != NULL) {
		fclose(fps->fp);
		fps->fp = NULL;
	}
	fps->fd = -1;
	
	return;
}


inline si4	FPS_lock_m10(FILE_PROCESSING_STRUCT_m10 *fps, si4 lock_type, const si1 *function, si4 line, ui4 behavior_on_fail)
{
#if defined MACOS_m10 || defined LINUX_m10
	struct flock	fl;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	fl.l_type = lock_type;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_pid = getpid();
	if (fcntl(fps->fd, F_SETLKW, &fl) == -1) {
		error_message_m10("%s(): fcntl() failed to lock file\n\tsystem error: %s (# %d)\n\tcalled from function %s(), line %d\n", __FUNCTION__, strerror(errno), errno, function, line);
		return(-1);
	}
#endif
	
	return(0);
}


inline void FPS_mutex_off_m10(FILE_PROCESSING_STRUCT_m10 *fps)
{
	fps->mutex = FALSE_m10;
	
	return;
}


inline void FPS_mutex_on_m10(FILE_PROCESSING_STRUCT_m10 *fps)
{
	while (fps->mutex == TRUE_m10);
	fps->mutex = TRUE_m10;
	
	return;
}


inline si4	FPS_open_m10(FILE_PROCESSING_STRUCT_m10 *fps, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	TERN_m10	create_file;
	si1		*mode, path[FULL_FILE_NAME_BYTES_m10], command[FULL_FILE_NAME_BYTES_m10 + 16];
	si1		name[BASE_FILE_NAME_BYTES_m10], extension[TYPE_BYTES_m10];
	si4		lock_type;
	struct stat	sb;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	// open
	mode = NULL;
	create_file = FALSE_m10;
	switch (fps->directives.open_mode) {
		case FPS_R_OPEN_MODE_m10:
			mode = "r";
			break;
		case FPS_R_PLUS_OPEN_MODE_m10:
			mode = "r+";
			break;
		case FPS_W_OPEN_MODE_m10:
			mode = "w";
			create_file = TRUE_m10;
			break;
		case FPS_W_PLUS_OPEN_MODE_m10:
			mode = "w+";
			create_file = TRUE_m10;
			break;
		case FPS_A_OPEN_MODE_m10:
			mode = "a";
			create_file = TRUE_m10;
			break;
		case FPS_A_PLUS_OPEN_MODE_m10:
			mode = "a+";
			create_file = TRUE_m10;
			break;
		case FPS_NO_OPEN_MODE_m10:
		default:
			error_message_m10("%s(): invalid open mode (%u)\n\tcalled from function %s(), line %d\n", __FUNCTION__, fps->directives.open_mode, function, line);
			return(-1);
	}
	
	fps->fp = fopen_m10(fps->full_file_name, mode, function, line, RETURN_ON_FAIL_m10 | SUPPRESS_ERROR_OUTPUT_m10);
	if (fps->fp == NULL && errno == ENOENT && create_file == TRUE_m10) {
		// A component of the required directory tree does not exist - build it & try again
		extract_path_parts_m10(fps->full_file_name, path, name, extension);
#if defined MACOS_m10 || defined LINUX_m10
		sprintf_m10(command, "mkdir -p \"%s\"", path);
#endif
#ifdef WINDOWS_m10
		sprintf_m10(command, "mkdir \"%s\"", path);
#endif
		system_m10(command, TRUE_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		fps->fp = fopen_m10(fps->full_file_name, mode, function, line, behavior_on_fail);
	}
	if (fps->fp == NULL) {
		error_message_m10("%s(): failed to open file \"%s\"\n\tcalled from function %s(), line %d\n", __FUNCTION__, fps->full_file_name, function, line);
		return(-1);
	}
	
	// get file descriptor
#if defined MACOS_m10 || defined LINUX_m10
	fps->fd = fileno(fps->fp);
#endif
#ifdef WINDOWS_m10
	fps->fd = _fileno(fps->fp);
#endif
	
	// lock
#if defined MACOS_m10 || defined LINUX_m10
	if (fps->directives.lock_mode != FPS_NO_LOCK_MODE_m10) {
		lock_type = FPS_NO_LOCK_TYPE_m10;
		if (fps->directives.open_mode == FPS_R_OPEN_MODE_m10) {
			if (fps->directives.lock_mode & FPS_READ_LOCK_ON_READ_OPEN_m10)
				lock_type = F_RDLCK;
			else if (fps->directives.lock_mode & FPS_WRITE_LOCK_ON_READ_OPEN_m10)
				lock_type = F_WRLCK;
		}
		else if (fps->directives.lock_mode & (FPS_WRITE_LOCK_ON_WRITE_OPEN_m10 | FPS_WRITE_LOCK_ON_READ_WRITE_OPEN_m10)) {
			lock_type = F_WRLCK;
		}
		else {
			error_message_m10("%s(): incompatible lock (%u) and open (%u) modes\n\tcalled from function %s(), line %d\n", __FUNCTION__, fps->directives.lock_mode, fps->directives.open_mode, function, line);
			return(-1);
		}
		FPS_lock_m10(fps, lock_type, function, line, behavior_on_fail);
	}
#endif
	
	// get file length
	fstat(fps->fd, &sb);
	fps->file_length = sb.st_size;
	
	return(0);
}


inline si4     FPS_read_m10(FILE_PROCESSING_STRUCT_m10 *fps, si8 in_bytes, void *ptr, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
#if defined MACOS_m10 || defined LINUX_m10
	// lock
	if (fps->directives.lock_mode & FPS_READ_LOCK_ON_READ_m10)
		FPS_lock_m10(fps, F_RDLCK, function, line, behavior_on_fail);
#endif
	
	// read
	fread_m10(ptr, sizeof(ui1), (size_t)in_bytes, fps->fp, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);
	
#if defined MACOS_m10 || defined LINUX_m10
	// unlock
	if (fps->directives.lock_mode & FPS_READ_LOCK_ON_READ_m10)
		FPS_unlock_m10(fps, function, line, behavior_on_fail);
#endif
	
	return(0);
}


inline si4	FPS_unlock_m10(FILE_PROCESSING_STRUCT_m10 *fps, const si1 *function, si4 line, ui4 behavior_on_fail)
{
#if defined MACOS_m10 || defined LINUX_m10
	struct flock	fl;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	fl.l_type = F_UNLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_pid = getpid();
	if (fcntl(fps->fd, F_SETLKW, &fl) == -1) {
		error_message_m10("%s(): fcntl() failed to unlock file\n\tsystem error: %s (# %d)\n\tcalled from function %s(), line %d\n", __FUNCTION__, strerror(errno), errno, function, line);
		return(-1);
	}
#endif
	
	return(0);
}


inline si4	FPS_write_m10(FILE_PROCESSING_STRUCT_m10 *fps, si8 out_bytes, void *ptr, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	UNIVERSAL_HEADER_m10	*uh;
	struct stat             sb;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
#if defined MACOS_m10 || defined LINUX_m10
	// lock
	if (fps->directives.lock_mode & FPS_WRITE_LOCK_ON_WRITE_m10)
		FPS_lock_m10(fps, F_WRLCK, function, line, behavior_on_fail);
#endif
	
	// write
	if (out_bytes == FPS_FULL_FILE_m10)
		out_bytes = fps->raw_data_bytes;
	fwrite_m10(ptr, sizeof(ui1), (size_t)out_bytes, fps->fp, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);
	if (fps->directives.flush_after_write == TRUE_m10)
		fflush(fps->fp);  // fflush updates stat structure
	fstat(fps->fd, &sb);
	fps->file_length = (si8) sb.st_size;
	
#if defined MACOS_m10 || defined LINUX_m10
	// unlock
	if (fps->directives.lock_mode & FPS_WRITE_LOCK_ON_WRITE_m10)
		FPS_unlock_m10(fps, function, line, behavior_on_fail);
#endif
	
	// update universal header, if requested
	if (fps->directives.update_universal_header == TRUE_m10) {
		uh = fps->universal_header;
		
		// update universal_header->file_CRC
		if (uh->body_CRC == CRC_NO_ENTRY_m10)
			uh->body_CRC = CRC_calculate_m10((ui1 *) uh + UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10, fps->file_length - UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10);
		uh->header_CRC = CRC_calculate_m10((ui1 *) uh + UNIVERSAL_HEADER_HEADER_CRC_START_OFFSET_m10, UNIVERSAL_HEADER_BYTES_m10 - UNIVERSAL_HEADER_HEADER_CRC_START_OFFSET_m10);
		
#if defined MACOS_m10 || defined LINUX_m10
		// lock
		if (fps->directives.lock_mode & FPS_WRITE_LOCK_ON_WRITE_m10)
			FPS_lock_m10(fps, F_WRLCK, function, line, behavior_on_fail);
#endif
		
		// write
		fseek_m10(fps->fp, 0, SEEK_SET, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);
		fwrite_m10(fps->raw_data, sizeof(ui1), (size_t)UNIVERSAL_HEADER_BYTES_m10, fps->fp, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);
		if (fps->directives.flush_after_write == TRUE_m10)
			fflush(fps->fp);    // fflush updates stat structure
		fseek_m10(fps->fp, 0, SEEK_END, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);
		
#if defined MACOS_m10 || defined LINUX_m10
		// unlock
		if (fps->directives.lock_mode & FPS_WRITE_LOCK_ON_WRITE_m10)
			FPS_unlock_m10(fps, function, line, behavior_on_fail);
#endif
	}
	
	return(0);
}



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


void	SHA_finalize_m10(SHA_CTX_m10 *ctx, ui1 *hash)
{
	ui4	i;
	

	i = ctx->datalen;

	// Pad whatever data is left in the buffer.
	if (ctx->datalen < 56) {
		ctx->data[i++] = 0x80;
		while (i < 56)
			ctx->data[i++] = 0x00;
	}
	else {
		ctx->data[i++] = 0x80;
		while (i < 64)
			ctx->data[i++] = 0x00;
		SHA_transform_m10(ctx, ctx->data);
		memset(ctx->data, 0, 56);
	}

	// Append to the padding the total message's length in bits and transform.
	ctx->bitlen += ctx->datalen * 8;
	ctx->data[63] = ctx->bitlen;
	ctx->data[62] = ctx->bitlen >> 8;
	ctx->data[61] = ctx->bitlen >> 16;
	ctx->data[60] = ctx->bitlen >> 24;
	ctx->data[59] = ctx->bitlen >> 32;
	ctx->data[58] = ctx->bitlen >> 40;
	ctx->data[57] = ctx->bitlen >> 48;
	ctx->data[56] = ctx->bitlen >> 56;
	SHA_transform_m10(ctx, ctx->data);

	// Since this implementation uses little endian byte ordering and SHA uses big endian,
	// reverse all the bytes when copying the final state to the output hash.
	for (i = 0; i < 4; ++i) {
		hash[i]      = (ctx->state[0] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m10;
		hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m10;
		hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m10;
		hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m10;
		hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m10;
		hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m10;
		hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m10;
		hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & SHA_LOW_BYTE_MASK_m10;
	}
	
	return;
}


ui1    *SHA_hash_m10(const ui1 *data, si8 len, ui1 *hash)
{
	SHA_CTX_m10         ctx;
	
	
	if (globals_m10->SHA_h0_table == NULL)  // all tables initialized together
		SHA_initialize_tables_m10();

	// if hash not passed, up to caller to free it
	if (hash == NULL)
		hash = (ui1 *) calloc_m10((size_t) SHA_HASH_BYTES_m10, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	SHA_initialize_m10(&ctx);
	SHA_update_m10(&ctx, data, len);
	SHA_finalize_m10(&ctx, hash);
	
	return(hash);
}


void	SHA_initialize_m10(SHA_CTX_m10 *ctx)
{
	ui4	*SHA_h0;
	
	
	ctx->datalen = 0;
	ctx->bitlen = 0;
	
	SHA_h0 = globals_m10->SHA_h0_table;
	ctx->state[0] = SHA_h0[0];
	ctx->state[1] = SHA_h0[1];
	ctx->state[2] = SHA_h0[2];
	ctx->state[3] = SHA_h0[3];
	ctx->state[4] = SHA_h0[4];
	ctx->state[5] = SHA_h0[5];
	ctx->state[6] = SHA_h0[6];
	ctx->state[7] = SHA_h0[7];
	
	return;
}


TERN_m10	SHA_initialize_tables_m10(void)
{
	// h0 table
	if (globals_m10->SHA_h0_table != NULL)
		free((void *) globals_m10->SHA_h0_table);
	globals_m10->SHA_h0_table = (ui4 *) calloc_m10((size_t) SHA_H0_ENTRIES_m10, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	{
		ui4 temp[SHA_H0_ENTRIES_m10] = SHA_H0_m10;
		memcpy(globals_m10->SHA_h0_table, temp, SHA_H0_ENTRIES_m10 * sizeof(ui4));
	}
	
	// k table
	if (globals_m10->SHA_k_table != NULL)
		free((void *) globals_m10->SHA_k_table);
	globals_m10->SHA_k_table = (ui4 *) calloc_m10((size_t) SHA_K_ENTRIES_m10, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	{
		ui4 temp[SHA_K_ENTRIES_m10] = SHA_K_m10;
		memcpy(globals_m10->SHA_k_table, temp, SHA_K_ENTRIES_m10 * sizeof(ui4));
	}
	
	return(TRUE_m10);
}


void	SHA_transform_m10(SHA_CTX_m10 *ctx, const ui1 *data)
{
	ui4	a, b, c, d, e, f, g, h, i, j, t1, t2, m[64], *sha_k;

	
	for (i = 0, j = 0; i < 16; ++i, j += 4)
		m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
	for ( ; i < 64; ++i)
		m[i] = SHA_SIG1_m10(m[i - 2]) + m[i - 7] + SHA_SIG0_m10(m[i - 15]) + m[i - 16];

	a = ctx->state[0];
	b = ctx->state[1];
	c = ctx->state[2];
	d = ctx->state[3];
	e = ctx->state[4];
	f = ctx->state[5];
	g = ctx->state[6];
	h = ctx->state[7];
	
	sha_k = globals_m10->SHA_k_table;
	for (i = 0; i < 64; ++i) {
		t1 = h + SHA_EP1_m10(e) + SHA_CH_m10(e,f,g) + sha_k[i] + m[i];
		t2 = SHA_EP0_m10(a) + SHA_MAJ_m10(a,b,c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
	ctx->state[4] += e;
	ctx->state[5] += f;
	ctx->state[6] += g;
	ctx->state[7] += h;
	
	return;
}


void	SHA_update_m10(SHA_CTX_m10 *ctx, const ui1 *data, si8 len)
{
	si8	i;
	

	for (i = 0; i < len; ++i) {
		ctx->data[ctx->datalen] = data[i];
		ctx->datalen++;
		if (ctx->datalen == 64) {
			SHA_transform_m10(ctx, ctx->data);
			ctx->bitlen += 512;
			ctx->datalen = 0;
		}
	}
	
	return;
}



//***********************************************************************//
//******************************  UTF-8 FUNCTIONS  **********************//
//***********************************************************************//

// ATTRIBUTION
//
// Basic UTF-8 manipulation routines
// by Jeff Bezanson
// placed in the public domain Fall 2005

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


// byte offset => char_num
inline si4	UTF8_char_num_m10(si1 *s, si4 offset)
{
	si4	char_num = 0, offs = 0;
	
	
	while (offs < offset && s[offs]) {
		(void) (UTF8_ISUTF_m10(s[++offs]) || UTF8_ISUTF_m10(s[++offs]) || UTF8_ISUTF_m10(s[++offs]) || ++offs);
		char_num++;
	}
	
	return(char_num);
}


inline void	UTF8_dec_m10(si1 *s, si4 *i)
{
	(void) (UTF8_ISUTF_m10(s[--(*i)]) || UTF8_ISUTF_m10(s[--(*i)]) || UTF8_ISUTF_m10(s[--(*i)]) || --(*i));
	
	return;
}


si4	UTF8_escape_m10(si1 *buf, si4 sz, si1 *src, si4 escape_quotes)
{
	si4	c = 0, i = 0, amt;
	
	
	while (src[i] && c < sz) {
		if (escape_quotes && src[i] == '"') {
			amt = snprintf_m10(buf, sz - c, "\\\"");
			i++;
		}
		else {
			amt = UTF8_escape_wchar_m10(buf, sz - c, UTF8_next_char_m10(src, &i));
		}
		c += amt;
		buf += amt;
	}
	if (c < sz)
		*buf = '\0';
	
	return(c);
}


si4	UTF8_escape_wchar_m10(si1 *buf, si4 sz, ui4 ch)
{
	if (ch == L'\n')
		return(snprintf(buf, sz, "\\n"));
	else if (ch == L'\t')
		return(snprintf(buf, sz, "\\t"));
	else if (ch == L'\r')
		return(snprintf(buf, sz, "\\r"));
	else if (ch == L'\b')
		return(snprintf(buf, sz, "\\b"));
	else if (ch == L'\f')
		return(snprintf(buf, sz, "\\f"));
	else if (ch == L'\v')
		return(snprintf(buf, sz, "\\v"));
	else if (ch == L'\a')
		return(snprintf(buf, sz, "\\a"));
	else if (ch == L'\\')
		return(snprintf(buf, sz, "\\\\"));
	else if (ch < 32 || ch == 0x7f)
		return(snprintf(buf, sz, "\\x%hhX", (ui1) ch));
	else if (ch > 0xFFFF)
		return(snprintf(buf, sz, "\\U%.8X", (ui4) ch));
	else if (ch >= 0x80 && ch <= 0xFFFF)
		return(snprintf(buf, sz, "\\u%.4hX", (ui2) ch));
	
	return(snprintf(buf, sz, "%c", (si1) ch));
}


inline si4     UTF8_fprintf_m10(FILE *stream, si1 *fmt, ...)
{
	si4		sz;
	si1		*src;
	ui4		*w_cs;
	va_list		args;
	
	
	va_start(args, fmt);
	sz = vasprintf_m10(&src, fmt, args);
	va_end(args);
	
	w_cs = (ui4 *) calloc_m10(sz + 1, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	UTF8_to_ucs_m10(w_cs, sz + 1, src, sz);
#ifdef MATLAB_m10
	if (stream == stderr || stream == stdout)
		mexPrintf("%s", src);
	else
#endif
	fprintf(stream, "%ls", (wchar_t *) w_cs);
	
	free((void *) src);
	free((void *) w_cs);
	
	return(sz);
}


inline si4	UTF8_hex_digit_m10(si1 c)
{
	return((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}


inline void	UTF8_inc_m10(si1 *s, si4 *i)
{
	(void) (UTF8_ISUTF_m10(s[++(*i)]) || UTF8_ISUTF_m10(s[++(*i)]) || UTF8_ISUTF_m10(s[++(*i)]) || ++(*i));
}


TERN_m10	UTF8_initialize_tables_m10(void)
{
	// offsets table
	if (globals_m10->UTF8_offsets_table != NULL)
		free((void *) globals_m10->UTF8_offsets_table);
	
	globals_m10->UTF8_offsets_table = (ui4 *) calloc_m10((size_t) UTF8_OFFSETS_TABLE_ENTRIES_m10, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	{
		ui4 temp[UTF8_OFFSETS_TABLE_ENTRIES_m10] = UTF8_OFFSETS_TABLE_m10;
		memcpy((void *) globals_m10->UTF8_offsets_table, (void *) temp, (size_t)UTF8_OFFSETS_TABLE_ENTRIES_m10 * sizeof(ui4));
	}
	
	// trailing bytes table
	if (globals_m10->UTF8_trailing_bytes_table != NULL)
		free((void *) globals_m10->UTF8_trailing_bytes_table);
	
	globals_m10->UTF8_trailing_bytes_table = (si1 *) calloc_m10((size_t) UTF8_TRAILING_BYTES_TABLE_ENTRIES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	{
		si1 temp[UTF8_TRAILING_BYTES_TABLE_ENTRIES_m10] = UTF8_TRAILING_BYTES_TABLE_m10;
		memcpy((void *) globals_m10->UTF8_trailing_bytes_table, (void *) temp, (size_t) UTF8_TRAILING_BYTES_TABLE_ENTRIES_m10);
	}
	
	return(TRUE_m10);
}


si4     UTF8_is_locale_utf8_m10(si1 *locale)
{
	// this code based on libutf8
	const si1	*cp = locale;
	
	
	for (; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ','; cp++) {
		if (*cp == '.') {
			const si1* encoding = ++cp;
			for (; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ','; cp++) ;
			if ((cp - encoding == 5 && !strncmp(encoding, "UTF-8", 5)) || (cp - encoding == 4 && !strncmp(encoding, "utf8", 4)))
				return(1); // it's UTF-8
			break;
		}
	}
	
	return(0);
}


si1	*UTF8_memchr_m10(si1 *s, ui4 ch, size_t sz, si4 *char_num)
{
	si4	i = 0, last_i = 0;
	ui4	c;
	si4	csz;
	
	
	if (globals_m10->UTF8_offsets_table == NULL)
		UTF8_initialize_tables_m10();
	
	*char_num = 0;
	while (i < sz) {
		c = csz = 0;
		do {
			c <<= 6;
			c += (ui1) s[i++];
			csz++;
		} while (i < sz && !UTF8_ISUTF_m10(s[i]));
		c -= globals_m10->UTF8_offsets_table[csz - 1];
		
		if (c == ch) {
			return(&s[last_i]);
		}
		last_i = i;
		(*char_num)++;
	}
	
	return(NULL);
}


// reads the next utf-8 sequence out of a string, updating an index
inline ui4     UTF8_next_char_m10(si1 *s, si4 *i)
{
	ui4	ch = 0;
	si4	sz = 0;
	
	
	if (s[*i] == 0)
		return(0);
	
	if (globals_m10->UTF8_offsets_table == NULL)
		UTF8_initialize_tables_m10();
	
	do {
		ch <<= 6;
		ch += (ui1) s[(*i)++];
		sz++;
	} while (s[*i] && !UTF8_ISUTF_m10(s[*i]));
	
	ch -= globals_m10->UTF8_offsets_table[sz - 1];

	return(ch);
}


inline si4	UTF8_octal_digit_m10(si1 c)
{
	return(c >= '0' && c <= '7');
}


// char_num => byte offset
inline si4     UTF8_offset_m10(si1 *str, si4 char_num)
{
	si4	offs = 0;
	
	
	while (char_num > 0 && str[offs]) {
		(void) (UTF8_ISUTF_m10(str[++offs]) || UTF8_ISUTF_m10(str[++offs]) || UTF8_ISUTF_m10(str[++offs]) || ++offs);
		char_num--;
	}
	
	return(offs);
}


inline si4     UTF8_printf_m10(si1 *fmt, ...)
{
	si4		sz;
	si1		*src;
	ui4		*w_cs;
	va_list 	args;
	
	
	va_start(args, fmt);
	sz = vasprintf_m10(&src, fmt, args);
	va_end(args);
	
	w_cs = (ui4 *) calloc_m10(sz + 1, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	UTF8_to_ucs_m10(w_cs, sz + 1, src, sz);
#ifdef MATLAB_m10
	mexPrintf("%s", src);
#else
	printf("%ls", (wchar_t *) w_cs);
#endif
	
	free((void *) src);
	free((void *) w_cs);
	
	return(sz);
}


// str points to a backslash or character after a backslash
// returns number of input characters processed
si4     UTF8_read_escape_sequence_m10(si1 *str, ui4 *dest)
{
	ui4	ch;
	si1	digs[9] = "\0\0\0\0\0\0\0\0";
	si4	dno = 0, i = 1;
	
	
	if (*str == '\\')
		++str;
	
	ch = (ui4)str[0];    // take literal character
	if (str[0] == 'n')
		ch = L'\n';
	else if (str[0] == 't')
		ch = L'\t';
	else if (str[0] == 'r')
		ch = L'\r';
	else if (str[0] == 'b')
		ch = L'\b';
	else if (str[0] == 'f')
		ch = L'\f';
	else if (str[0] == 'v')
		ch = L'\v';
	else if (str[0] == 'a')
		ch = L'\a';
	else if (UTF8_octal_digit_m10(str[0])) {
		i = 0;
		do {
			digs[dno++] = str[i++];
		} while (UTF8_octal_digit_m10(str[i]) && dno < 3);
		ch = strtol(digs, NULL, 8);
	}
	else if (str[0] == 'x') {
		while (UTF8_hex_digit_m10(str[i]) && dno < 2) {
			digs[dno++] = str[i++];
		}
		if (dno > 0)
			ch = strtol(digs, NULL, 16);
	}
	else if (str[0] == 'u') {
		while (UTF8_hex_digit_m10(str[i]) && dno < 4) {
			digs[dno++] = str[i++];
		}
		if (dno > 0)
			ch = strtol(digs, NULL, 16);
	}
	else if (str[0] == 'U') {
		while (UTF8_hex_digit_m10(str[i]) && dno < 8) {
			digs[dno++] = str[i++];
		}
		if (dno > 0)
			ch = strtol(digs, NULL, 16);
	}
	*dest = ch;
	
	return(i);
}


// returns length of next utf-8 sequence
inline si4      UTF8_seqlen_m10(si1 *s)
{
	if (globals_m10->UTF8_trailing_bytes_table == NULL)
		UTF8_initialize_tables_m10();
	
	return(globals_m10->UTF8_trailing_bytes_table[(si4) ((ui1) s[0])] + 1);
}


si1	*UTF8_strchr_m10(si1 *s, ui4 ch, si4 *char_num)
{
	si4	i = 0, last_i = 0;
	ui4	c;
	
	
	*char_num = 0;
	while (s[i]) {
		c = UTF8_next_char_m10(s, &i);
		if (c == ch)
			return(&s[last_i]);
		last_i = i;
		(*char_num)++;
	}
	
	return(NULL);
}


// number of characters
si4     UTF8_strlen_m10(si1 *s)
{
	si4	count = 0;
	si4	i = 0;
	
	
	while (UTF8_next_char_m10(s, &i))
		count++;
	
	return(count);
}


// conversions without error checking
// only works for valid UTF-8, i.e. no 5- or 6-byte sequences
// srcsz == source size in bytes, or -1 if 0-terminated
// sz = dest size in # of wide characters

// returns # characters converted
// dest will always be 0-terminated, even if there isn't enough room
// for all the characters.
// if sz == srcsz+1 (i.e. 4*srcsz+4 bytes), there will always be enough space
si4     UTF8_to_ucs_m10(ui4 *dest, si4 sz, si1 *src, si4 srcsz)
{
	ui4	ch;
	si1	*src_end = src + srcsz;
	si4	nb;
	si4	i = 0;
	
	
	if (globals_m10->UTF8_offsets_table == NULL)
		UTF8_initialize_tables_m10();
	
	while (i < sz - 1) {
		nb = globals_m10->UTF8_trailing_bytes_table[(ui1) *src];
		if (srcsz == -1) {
			if (*src == 0)
				goto UTF8_DONE_TOUCS_m10;
		}
		else {
			if (src + nb >= src_end)
				goto UTF8_DONE_TOUCS_m10;
		}
		ch = 0;
		switch (nb) {
				// these fall through deliberately
			case 3: ch += (ui1) *src++; ch <<= 6;
			case 2: ch += (ui1) *src++; ch <<= 6;
			case 1: ch += (ui1) *src++; ch <<= 6;
			case 0: ch += (ui1) *src++;
		}
		ch -= globals_m10->UTF8_offsets_table[nb];
		dest[i++] = ch;
	}
	
UTF8_DONE_TOUCS_m10:
	
	dest[i] = 0;
	
	return(i);
}


// srcsz == number of source characters, or -1 if 0-terminated
// sz == size of dest buffer in bytes

// returns # characters converted
// dest will only be 0-terminated if there is enough space. this is
// for consistency; imagine there are 2 bytes of space left, but the next
// character requires 3 bytes. in this case we could NUL-terminate, but in
// general we can't when there's insufficient space. therefore this function
// only NULL-terminates if all the characters fit, and there's space for
// the NULL as well.
// the destination string will never be bigger than the source string
si4     UTF8_to_utf8_m10(si1 *dest, si4 sz, ui4 *src, si4 srcsz)
{
	ui4	ch;
	si4	i = 0;
	si1	*dest_end = dest + sz;
	
	
	while (srcsz < 0 ? src[i] != 0 : i < srcsz) {
		ch = src[i];
		if (ch < 0x80) {
			if (dest >= dest_end)
				return(i);
			*dest++ = (si1) ch;
		}
		else if (ch < 0x800) {
			if (dest >= dest_end - 1)
				return(i);
			*dest++ = (ch >> 6) | 0xC0;
			*dest++ = (ch & 0x3F) | 0x80;
		}
		else if (ch < 0x10000) {
			if (dest >= dest_end - 2)
				return(i);
			*dest++ = (ch >> 12) | 0xE0;
			*dest++ = ((ch >> 6) & 0x3F) | 0x80;
			*dest++ = (ch & 0x3F) | 0x80;
		}
		else if (ch < 0x110000) {
			if (dest >= dest_end - 3)
				return(i);
			*dest++ = (ch >> 18) | 0xF0;
			*dest++ = ((ch >> 12) & 0x3F) | 0x80;
			*dest++ = ((ch >> 6) & 0x3F) | 0x80;
			*dest++ = (ch & 0x3F) | 0x80;
		}
		i++;
	}
	if (dest < dest_end)
		*dest = '\0';
	
	return(i);
}


// convert a string with literal \uxxxx or \Uxxxxxxxx characters to UTF-8
// example: UTF8_unescape(mybuf, 256, "hello\\u220e")
// note the double backslash is needed if called on a C string literal
si4     UTF8_unescape_m10(si1 *buf, si4 sz, si1 *src)
{
	si4	c = 0, amt;
	ui4	ch;
	si1	temp[4];
	
	
	while (*src && c < sz) {
		if (*src == '\\') {
			src++;
			amt = UTF8_read_escape_sequence_m10(src, &ch);
		}
		else {
			ch = (si4)*src;
			amt = 1;
		}
		src += amt;
		amt = UTF8_wc_to_utf8_m10(temp, ch);
		if (amt > sz - c)
			break;
		memcpy(&buf[c], temp, amt);
		c += amt;
	}
	if (c < sz)
		buf[c] = '\0';
	
	return(c);
}


si4     UTF8_vfprintf_m10(FILE *stream, si1 *fmt, va_list args)
{
	si4	sz;
	si1	*src;
	ui4	*w_cs;
	
	
	sz = vasprintf_m10(&src, fmt, args);
	w_cs = (ui4 *) calloc_m10(sz + 1, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	UTF8_to_ucs_m10(w_cs, sz + 1, src, sz);
	
#ifdef MATLAB_m10
	if (stream == stderr || stream == stdout)
		mexPrintf("%s", src);
	else
#endif
	fprintf(stream, "%ls", (wchar_t *) w_cs);
	
	free((void *) src);
	free((void *) w_cs);
	
	return(sz);
}


si4     UTF8_vprintf_m10(si1 *fmt, va_list args)
{
	si4	sz;
	si1	*src;
	ui4	*w_cs;
	
	
	sz = vasprintf_m10(&src, fmt, args);
	w_cs = (ui4 *) calloc_m10(sz + 1, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	UTF8_to_ucs_m10(w_cs, sz + 1, src, sz);
	
#ifdef MATLAB_m10
	mexPrintf("%s", src);
#else
	printf("%ls", (wchar_t *) w_cs);
#endif
	
	free((void *) src);
	free((void *) w_cs);
	
	return(sz);
}


si4     UTF8_wc_to_utf8_m10(si1 *dest, ui4 ch)
{
	if (ch < 0x80) {
		dest[0] = (char)ch;
		return(1);
	}
	if (ch < 0x800) {
		dest[0] = (ch >> 6) | 0xC0;
		dest[1] = (ch & 0x3F) | 0x80;
		return(2);
	}
	if (ch < 0x10000) {
		dest[0] = (ch >> 12) | 0xE0;
		dest[1] = ((ch >> 6) & 0x3F) | 0x80;
		dest[2] = (ch & 0x3F) | 0x80;
		return(3);
	}
	if (ch < 0x110000) {
		dest[0] = (ch >> 18) | 0xF0;
		dest[1] = ((ch >> 12) & 0x3F) | 0x80;
		dest[2] = ((ch >> 6) & 0x3F) | 0x80;
		dest[3] = (ch & 0x3F) | 0x80;
		return(4);
	}
	
	return(0);
}



//***********************************************************************//
//*****************  MED VERSIONS OF STANDARD FUNCTIONS  ****************//
//***********************************************************************//


inline si4    asprintf_m10(si1 **target, si1 *fmt, ...)
{
	si4		ret_val;
	va_list		args;


	va_start(args, fmt);
	ret_val = vasprintf_m10(target, fmt, args);
	va_end(args);

	return(ret_val);
}


void	*calloc_m10(size_t n_members, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	void* ptr;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if (n_members == 0 || el_size == 0)
		return((void *) NULL);
	
	
	if ((ptr = calloc(n_members, el_size)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void)fprintf_m10(stderr, "%c\n\t%s() failed to allocate the requested array (%ld members of size %ld)\n", 7, __FUNCTION__, n_members, el_size);
			(void)fprintf_m10(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m10(stderr, "\tcalled from function %s(), line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> returning NULL\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit_m10(1);
	}
	
	return(ptr);
}


void	exit_m10(si4 status)
{
#ifdef WINDOWS_m10
	win_cleanup_m10();
#endif

#ifdef MATLAB_m10
	const si1	tmp_str[32];
	
	sprintf((char *) tmp_str, "Exit status: %d\n", status);
	mexErrMsgTxt(tmp_str);
#else
	exit(status);
#endif
}


FILE	*fopen_m10(si1 *path, si1 *mode, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	FILE	*fp;


	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;

#if defined MACOS_m10 || defined LINUX_m10
	if ((fp = fopen(path, mode)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void)UTF8_fprintf_m10(stderr, "%c\n\t%s() failed to open file \"%s\"\n", 7, __FUNCTION__, path);
			(void)fprintf_m10(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m10(stderr, "\tcalled from function %s(), line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> returning NULL\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit_m10(1);
}
#endif
	
#ifdef WINDOWS_m10  // always binary mode in MED
	si1	tmp_mode[8] = {0};
	
	tmp_mode[0] = mode[0];
	if (mode[1]) {
		tmp_mode[1] = mode[1];
		if (mode[1] != 'b')
			tmp_mode[2] = 'b';
	}
	else {
		tmp_mode[1] = 'b';
	}
	mode = tmp_mode;
	
	if ((fp = _fsopen(path, mode, _SH_DENYNO)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void)UTF8_fprintf_m10(stderr, "%c\n\t%s() failed to open file \"%s\"\n", 7, __FUNCTION__, path);
			(void)fprintf_m10(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m10(stderr, "\tcalled from function %s(), line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> returning NULL\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit_m10(1);
	}
#endif

	return(fp);
}


inline si4     fprintf_m10(FILE *stream, si1 *fmt, ...)
{
	si1		*temp_str;
	si4		ret_val;
	va_list		args;
	
	
	va_start(args, fmt);
	ret_val = vasprintf_m10(&temp_str, fmt, args);  // could just call vfprintf_m10() here & be done, but it's hardly any extra code, so duplicate & skip extra function call
	va_end(args);
	
	if (ret_val >= 0) {
#ifdef MATLAB_m10
		if (stream == stderr || stream == stdout)
			ret_val = mexPrintf("%s", temp_str);
		else
#endif
		ret_val = fprintf(stream, "%s", temp_str);
		free((void *) temp_str);
	}

	return(ret_val);
}


inline si4	fputc_m10(si4 c, FILE *stream)
{
	si4	ret_val;

#ifdef MATLAB_m10
	if (stream == stderr || stream == stdout)
		ret_val = mexPrintf("%c", c);
	else
#endif
	ret_val = fputc(c, stream);
	
	return(ret_val);
}


size_t	fread_m10(void *ptr, size_t el_size, size_t n_members, FILE *stream, si1 *path, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	size_t	nr;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if ((nr = fread(ptr, el_size, n_members, stream)) != n_members) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void)UTF8_fprintf_m10(stderr, "%c\n\t%s() failed to read file \"%s\"\n", 7, __FUNCTION__, path);
			(void)fprintf_m10(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m10(stderr, "\tcalled from function %s(), line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> returning number of items read\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(nr);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit_m10(1);
	}
	
	return(nr);
}


void    free_m10(void *ptr, const si1 *function, si4 line)
{
	if (ptr == NULL) {
		warning_message_m10("%s(): Attempting to free unallocated object [called from function %s(), line %d]\n", __FUNCTION__, function, line);
		return;
	}
	
	free(ptr);
	
	return;
}


inline si4     fscanf_m10(FILE *stream, si1 *fmt, ...)
{
	si4		ret_val;
	va_list		args;
	
	
#ifdef WINDOWS_m10
	si1	*new_fmt = NULL;
	
	// convert format string
	new_fmt = windify_format_string_m10(fmt);
	
	va_start(args, fmt);
	ret_val = vfscanf(stream, new_fmt, args);
	va_end(args);
	
	if (new_fmt != fmt)
		free((void *) new_fmt);
#endif
	
#if defined MACOS_m10 || defined LINUX_m10
	va_start(args, fmt);
	ret_val = vfscanf(stream, fmt, args);
	va_end(args);
#endif
	
	return(ret_val);
}


si4	fseek_m10(FILE *stream, si8 offset, si4 whence, si1 *path, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
#if defined MACOS_m10 || defined LINUX_m10
	if ((fseek(stream, offset, whence)) == -1) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void)fprintf_m10(stderr, "%c\n\t%s() failed to move the file pointer to requested location (offset %ld, whence %d)\n", 7, __FUNCTION__, offset, whence);
			(void)UTF8_fprintf_m10(stderr, "%\tin file \"%s\"\n", path);
			(void)fprintf_m10(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m10(stderr, "\tcalled from function %s(), line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> returning -1\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(-1);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit_m10(1);
	}
#endif

#ifdef WINDOWS_m10
	if ((_fseeki64(stream, offset, whence)) == -1) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void)fprintf_m10(stderr, "%c\n\t%s() failed to move the file pointer to requested location (offset %ld, whence %d)\n", 7, __FUNCTION__, offset, whence);
			(void)UTF8_fprintf_m10(stderr, "%\tin file \"%s\"\n", path);
			(void)fprintf_m10(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m10(stderr, "\tcalled from function %s(), line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> returning -1\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(-1);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit_m10(1);
	}
#endif

	return(0);
}
		
		
si8	ftell_m10(FILE *stream, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	si8	pos;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if ((pos = ftell(stream)) == -1) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void)fprintf_m10(stderr, "%c\n\t%s() failed obtain the current location\n", 7, __FUNCTION__);
			(void)fprintf_m10(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m10(stderr, "\tcalled from function %s(), line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> returning -1\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(-1);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit_m10(1);
	}
	
	return(pos);
}
		
		
size_t	fwrite_m10(void *ptr, size_t el_size, size_t n_members, FILE *stream, si1 *path, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	size_t	nw;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if ((nw = fwrite(ptr, el_size, n_members, stream)) != n_members) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void)UTF8_fprintf_m10(stderr, "%c\n\t%s() failed to write file \"%s\"\n", 7, __FUNCTION__, path);
			(void)fprintf_m10(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m10(stderr, "\tcalled from function %s(), line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> returning number of items written\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(nw);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit_m10(1);
	}
	
	return(nw);
}


inline char	*getcwd_m10(char *buf, size_t size)
{
#if defined MACOS_m10 || defined LINUX_m10
	return(getcwd(buf, size));
#endif
#ifdef WINDOWS_m10
	return(_getcwd(buf, size));
#endif
}


void	*malloc_m10(size_t n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	void	*ptr;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if (n_bytes == 0)
		return((void *) NULL);
	
	if ((ptr = malloc(n_bytes)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void)fprintf_m10(stderr, "%c\n\t%s() failed to allocate the requested array (%ld bytes)\n", 7, __FUNCTION__, n_bytes);
			(void)fprintf_m10(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m10(stderr, "\tcalled from function %s(), line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> returning NULL\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit_m10(1);
	}
	
	return(ptr);
}
		

inline si4     printf_m10(si1 *fmt, ...)
{
	si1		*temp_str;
	si4		ret_val;
	va_list		args;
	
	
	va_start(args, fmt);
	ret_val = vasprintf_m10(&temp_str, fmt, args);  // could just call vprintf_m10() here & be done, but it's hardly any extra code, so duplicate & skip extra function call
	va_end(args);
	
	if (ret_val >= 0) {
#ifdef MATLAB_m10
		ret_val = mexPrintf("%s", temp_str);
#else
		ret_val = printf("%s", temp_str);
#endif
		free((void *) temp_str);
	}
		
	return(ret_val);
}


inline si4	putc_m10(si4 c, FILE *stream)
{
	return(fputc_m10(c, stream));
}


inline si4	putch_m10(si4 c)
{
	si4	ret_val;

#ifdef MATLAB_m10
	ret_val = mexPrintf("%c", c);
#else
	#ifdef WINDOWS_m10
		ret_val = _putch(c);
	#else
		ret_val = fputc_m10(c, stdout);
	#endif
#endif
	
	return(ret_val);
}


inline si4	putchar_m10(si4 c)
{
	return(fputc_m10(c, stdout));
}


void	*realloc_m10(void *orig_ptr, size_t n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	void	*ptr;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if (n_bytes == 0) {
		if (orig_ptr != NULL)
			free((void *) orig_ptr);
		return((void *) NULL);
	}
	
	if ((ptr = realloc(orig_ptr, n_bytes)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void)fprintf_m10(stderr, "%c\n\t%s() failed to reallocate the requested array (%ld bytes)\n", 7, __FUNCTION__, n_bytes);
			(void)fprintf_m10(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void)fprintf_m10(stderr, "\tcalled from function %s(), line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> returning unreallocated pointer\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(orig_ptr);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit_m10(1);
	}
	
	return(ptr);
}


inline si4     scanf_m10(si1 *fmt, ...)
{
	si4         ret_val;
	va_list     args;
	
	
#ifdef WINDOWS_m10
	si1* new_fmt = NULL;
	
	// convert format string
	new_fmt = windify_format_string_m10(fmt);
	
	va_start(args, fmt);
	ret_val = vscanf(new_fmt, args);
	va_end(args);
	
	if (new_fmt != fmt)
		free((void *) new_fmt);
#endif
	
#if defined MACOS_m10 || defined LINUX_m10
	va_start(args, fmt);
	ret_val = vscanf(fmt, args);
	va_end(args);
#endif
	
	return(ret_val);
}


inline si4    snprintf_m10(si1 *target, si4 target_field_bytes, si1 *fmt, ...)
{
	si4		ret_val;
	va_list		args;
	
	
	// as opposed to standard snprintf(), snprintf_m10()allows source & target strings to overlap
	
	va_start(args, fmt);
	ret_val = vsnprintf_m10(target, target_field_bytes, fmt, args);
	va_end(args);
	
	return(ret_val);
}


inline si4    sprintf_m10(si1 *target, si1 *fmt, ...)
{
	si1		*tmp_str;
	si4		ret_val;
	va_list		args;
	
	
	// as opposed to standard sprintf(), sprintf_m10() allows source & target strings to overlap
		
	va_start(args, fmt);
	ret_val = vasprintf_m10(&tmp_str, fmt, args);  	// could just call vsprintf_m10() here & be done, but it's hardly any extra code, so duplicate & skip extra function call
	va_end(args);
	
	memcpy(target, tmp_str, ret_val + 1);
	free((void *) tmp_str);
	
	return(ret_val);
}


inline si4     sscanf_m10(si1 *target, si1 *fmt, ...)
{
	si4		ret_val;
	va_list		args;
	
	
#ifdef WINDOWS_m10
	si1* new_fmt = NULL;
	
	// convert format string
	new_fmt = windify_format_string_m10(fmt);
	
	va_start(args, fmt);
	ret_val = vsscanf(target, new_fmt, args);
	va_end(args);
	
	if (new_fmt != fmt)
		free((void *) new_fmt);
#endif
	
#if defined MACOS_m10 || defined LINUX_m10
	va_start(args, fmt);
	ret_val = vsscanf(target, fmt, args);
	va_end(args);
#endif
	
	return(ret_val);
}


si8     strcat_m10(si1 *target, si1 *source)
{
	si1	*c;
	
	
	// returns final length (not including terminal zero)
	
	if (target == NULL || source == NULL)
		return(-1);
	
	c = target;
	while ((*c++));
	--c;
	while ((*c++ = *source++));
	
	return((si8)((c - target) - 1));
}


si8     strcpy_m10(si1 *target, si1 *source)
{
	si1	*c;
	
	
	// returns length (not including terminal zero)
	
	if (target == NULL || source == NULL)
		return(-1);
	
	c = target;
	while ((*c++ = *source++));
	
	return((si8) ((c - target) - 1));
}


si8    strncat_m10(si1 *target, si1 *source, si4 target_field_bytes)
{
	si1	*c;
	si8	len = 0;
	
	
	// returns final non-zero length
	
	if (target == NULL)
		return(-1);
	if (target_field_bytes < 1) {
		*target = 0;
		return(-1);
	}
	
	c = target;
	if (source == NULL) {
		--target_field_bytes;
	}
	else {
		while (--target_field_bytes)
			if (*c++ == '\0')
				break;
	}
	
	--c;
	++target_field_bytes;
	
	while (--target_field_bytes)
		if ((*c++ = *source++) == '\0')
			break;
	len = (si8)((c - target) - 1);
	if (target_field_bytes) {
		while (--target_field_bytes)
			*c++ = '\0';
	}
	else {
		warning_message_m10("%s(): target string truncated\n", __FUNCTION__);
	}
	
	*c = '\0';
	
	return(len);
}


si8    strncpy_m10(si1 *target, si1 *source, si4 target_field_bytes)
{
	si1	*c;
	si8	len = 0;
	
	
	// returns length, not including terminal zero
	
	if (target == NULL)
		return(-1);
	
	if (target_field_bytes < 1) {
		*target = 0;
		return(-1);
	}
	
	c = target;
	if (source == NULL) {
		--target_field_bytes;
	}
	else {
		while (--target_field_bytes) {
			if ((*c++ = *source++) == 0)
				break;
		}
		len = (si8)((c - target) - 1);
	}
	
	if (target_field_bytes) {
		while (--target_field_bytes)
			*c++ = '\0';
	}
	else {
		warning_message_m10("%s(): target string truncated\n", __FUNCTION__);
	}
	
	*c = '\0';
	
	return(len);
}


si4     system_m10(si1 *command, TERN_m10 null_std_streams, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	si1	*temp_command;
	si4	ret_val, len;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if (null_std_streams == TRUE_m10) {
		len = strlen(command);
		temp_command = malloc_m10(len + 18, function, line, behavior_on_fail);
		sprintf_m10(temp_command, "%s > %s 2>&1", command, NULL_DEVICE);
		command = temp_command;
	}
	
#if defined MACOS_m10 || defined LINUX_m10
	ret_val = system(command);
#endif
#ifdef WINDOWS_m10
	ret_val = win_system_m10(command);
#endif

	if (ret_val) {
		if (behavior_on_fail & RETRY_ONCE_m10) {
#if defined MACOS_m10 || defined LINUX_m10
			usleep((useconds_t) 1000);  // wait 1 ms
			if ((ret_val = system(command)) == 0) {
				if (null_std_streams == TRUE_m10)
					free((void *) command);
				return(0);
			}
#endif
#ifdef WINDOWS_m10
			Sleep(1);  // wait 1 ms
			if ((ret_val = win_system_m10(command)) == 0) {
				if (null_std_streams == TRUE_m10)
					free((void *) command);
				return(0);
			}
#endif
		}
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			fprintf_m10(stderr, "%c\n%s() failed\n", 7, __FUNCTION__);
			fprintf_m10(stderr, "\tcommand: \"%s\"\n", command);
			fprintf_m10(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			fprintf_m10(stderr, "\tshell return value %d\n", ret_val);
			if (function != NULL)
				(void)fprintf_m10(stderr, "\tcalled from function %s(), line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> returning -1\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void)fprintf_m10(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10) {
			if (null_std_streams == TRUE_m10)
				free((void *) command);
			return(-1);
		}
		else if (behavior_on_fail & EXIT_ON_FAIL_m10) {
			exit_m10(1);
		}
	}
	
	if (null_std_streams == TRUE_m10)
		free((void *) command);
	
	return(0);
}


inline si4    vasprintf_m10(si1 **target, si1 *fmt, va_list args)
{
	si4	ret_val;
	
	
#ifdef WINDOWS_m10
	// no vasprintf() in Windows
	*target = (si1 *) calloc_m10((size_t) PRINTF_BUF_LEN_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	force_behavior_m10(SUPPRESS_WARNING_OUTPUT_m10);
	ret_val = vsnprintf_m10(*target, PRINTF_BUF_LEN_m10, fmt, args);
	force_behavior_m10(RESTORE_BEHAVIOR_m10);
	// trim or expand memory to required size
	*target = (si1 *) realloc_m10((void *) *target, (size_t) (ret_val + 1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	if (ret_val >= PRINTF_BUF_LEN_m10)
		ret_val = vsnprintf_m10(*target, ret_val + 1, fmt, args);
#endif
	
#if defined MACOS_m10 || defined LINUX_m10
	ret_val = vasprintf(target, fmt, args);
#endif
	
	return(ret_val);
}


inline si4     vfprintf_m10(FILE *stream, si1 *fmt, va_list args)
{
	si1	*temp_str;
	si4	ret_val;
	
	
	ret_val = vasprintf_m10(&temp_str, fmt, args);
	
	if (ret_val >= 0) {
#ifdef MATLAB_m10
		if (stream == stderr || stream == stdout)
			ret_val = mexPrintf("%s", temp_str);
		else
#endif
		ret_val = fprintf(stream, "%s", temp_str);
		free((void *) temp_str);
	}

	return(ret_val);
}


inline si4     vprintf_m10(si1 *fmt, va_list args)
{
	si1	*temp_str;
	si4	ret_val;
	
	
	ret_val = vasprintf_m10(&temp_str, fmt, args);
	
	if (ret_val >= 0) {
#ifdef MATLAB_m10
		ret_val = mexPrintf("%s", temp_str);
#else
		ret_val = printf("%s", temp_str);
#endif
		free((void *) temp_str);
	}
	
	return(ret_val);
}


inline si4    vsnprintf_m10(si1 *target, si4 target_field_bytes, si1 *fmt, va_list args)
{
	si4	ret_val;
	si1	*temp_str;
	
	
	//******** vsnprintf_m10() CONTAINS THE WINDOWS FORMATTING FOR ALL MED PRINTF FUNCTIONS ********//
	
	// as opposed to standard vsnprintf(), vsnprintf_m10() allows source & target strings to overlap
	
	if (target_field_bytes <= 1) {
		if (target_field_bytes == 1) {
			*target = 0;
			return(0);
		}
		return(-1);
	}
	
#ifdef WINDOWS_m10
	TERN_m10	free_fmt = FALSE_m10;
	si1		*new_fmt;
	
	// convert format string
	new_fmt = windify_format_string_m10(fmt);
	
	if (new_fmt != fmt) {
		fmt = new_fmt;
		free_fmt = TRUE_m10;
	}
#endif
	// Guarantee zeros in unused bytes per MED requirements
	temp_str = (si1 *) calloc_m10((size_t) target_field_bytes, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	ret_val = vsnprintf(temp_str, target_field_bytes, fmt, args);
	
	// Guarantee terminal zero on overflow (not done in Linux & Windows)
	if (ret_val >= target_field_bytes) {
		temp_str[target_field_bytes - 1] = 0;
		warning_message_m10("%s(): target string truncated\n", __FUNCTION__);
	}
	memcpy(target, temp_str, target_field_bytes);
	free((void *) temp_str);
	
#ifdef WINDOWS_m10
	// convert file system paths
	windify_file_paths_m10(NULL, target);

	// clean up
	if (free_fmt == TRUE_m10)
		free((void *) fmt);
#endif
	
	return(ret_val);
}


inline si4    vsprintf_m10(si1 *target, si1 *fmt, va_list args)
{
	si1		*tmp_str;
	si4		ret_val;
	
	
	// as opposed to standard vsprintf(), vsprintf_m10() allows source & target strings to overlap
	
	ret_val = vasprintf_m10(&tmp_str, fmt, args);
	
	memcpy(target, tmp_str, ret_val + 1);
	free((void *) tmp_str);
	
	return(ret_val);
}



