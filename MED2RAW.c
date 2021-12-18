
// MED2RAW.c
// Written by Matt Stead
// Copyright Dark Horse Neuro Inc, 2020


#include "medlib_m10.h"

#ifndef MAX_CHANNELS
#define MAX_CHANNELS	512
#endif


int main(int argc, char *argv[])
{
	extern GLOBALS_m10		*globals_m10;
	TERN_m10			read_time_series_data, read_record_data, show_records, show_file_processing_structs, show_time_slices;
	si1				**channel_list, *sess_dir;
	si1				*in_dir, out_dir[FULL_FILE_NAME_BYTES_m10], out_file[FULL_FILE_NAME_BYTES_m10], *password;
	ui4				type_code;
	si4				n_channels;
	si8				i, j, n_samps;
	FILE				*out_fp;
	SESSION_m10			*sess;
	CHANNEL_m10			*chan;
	SEGMENT_m10			*seg;
	CMP_PROCESSING_STRUCT_m10	*cps;
	TIME_SLICE_m10			slice;
	
	
	// NOTE: in MacOS & Linux change resource limits before calling any functions that use system resources (e.g. printf())
	adjust_open_file_limit_m10(MAX_OPEN_FILES_m10(MAX_CHANNELS, 1), TRUE_m10);

	// usage
	if (argc < 2 || argc > 9) {
		fprintf(stderr, "\n\tUSAGE: %s MED_directory (multiple w/ regex) [output_directory] [start_time] [end_time] [start_samp_num] [end_samp_num] [password] [samp_num_ref_chan]\n\n", argv[0]);
		return(-1);
	}

	// USAGE: %s MED_directory [output_directory] [start_time] [end_time] [start_samp_num] [end_samp_num] [password] [samp_num_ref_chan]
	// negative times: relative to session start
	// positive times: offset or absolute

	// initialize medlib:
	// prototype: TERN_m10 initialize_medlib_m10(TERN_m10 check_structure_alignments, TERN_m10 initialize_all_tables)
	// change check_structure_alignments to FALSE_m10 to reduce initialization time (all structures align on all systems tested so far)
	// change initialize_all_tables to FALSE_m10 to reduce initialization time (any required table will be loaded when it is first accessed)
	initialize_medlib_m10(TRUE_m10, TRUE_m10);
	
	// initialize time slice
	initialize_time_slice_m10(&slice);

	// testing: adjust to your needs
	show_records = FALSE_m10;
	show_file_processing_structs = FALSE_m10;
	show_time_slices = FALSE_m10;

	// input file list
	in_dir = (si1 *) calloc_m10((size_t) FULL_FILE_NAME_BYTES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	strcpy(in_dir, argv[1]);
	escape_spaces_m10(in_dir, FULL_FILE_NAME_BYTES_m10);
	type_code = MED_type_code_from_string_m10(in_dir);
	if (type_code == SESSION_DIRECTORY_TYPE_CODE_m10) {
		sess_dir = in_dir;
		channel_list = NULL;
		n_channels = 0;
	}
	else {
		sess_dir = NULL;
		channel_list = &in_dir;
		n_channels = 1;
	}

	// output directory
	if (argc >= 3)
		strcpy(out_dir, argv[2]);
	else
		getcwd_m10(out_dir, FULL_FILE_NAME_BYTES_m10);

	// start time
	if (argc >= 4) {
		if (*argv[3]) {  // LINUX strtol() returns zero for a zero-length string and does not set errno for EINVAL
			if ((strcmp(argv[3], "start")) == 0)
				slice.start_time = BEGINNING_OF_TIME_m10;
			else
				slice.start_time = (si8) strtol(argv[3], NULL, 10);
		}
	}

	// end time
	if (argc >= 5) {
		if (*argv[4]) {  // LINUX strtol() returns zero for a zero-length string and does not set errno for EINVAL
			if ((strcmp(argv[4], "end")) == 0)
				slice.end_time = END_OF_TIME_m10;
			else {
				slice.end_time = (si8) strtol(argv[4], NULL, 10);
				if (slice.end_time == 0)
					slice.end_time = UUTC_NO_ENTRY_m10;
			}
		}
	}

	// start sample number
	if (argc >= 6) {
		if (*argv[5]) {  // LINUX strtol() returns zero for a zero-length string and does not set errno for EINVAL
			if ((strcmp(argv[5], "start")) == 0)
				slice.start_sample_number = BEGINNING_OF_SAMPLE_NUMBERS_m10;
			else
				slice.start_sample_number = (si8) strtol(argv[5], NULL, 10);
		}
	}

	// end sample number
	if (argc >= 7) {
		if (*argv[6]) {  // LINUX strtol() returns zero for a zero-length string and does not set errno for EINVAL
			if ((strcmp(argv[6], "end")) == 0)
				slice.end_sample_number = END_OF_SAMPLE_NUMBERS_m10;
			else {
				slice.end_sample_number = (si8) strtol(argv[6], NULL, 10);
				if (slice.end_sample_number == 0)
					slice.end_time = SAMPLE_NUMBER_NO_ENTRY_m10;
			}
		}
	}

	// password
	password = NULL;
	if (argc >= 8)
		password = argv[7];

	// sample number reference channel name (base name only)
	if (argc == 9)
		extract_path_parts_m10(argv[8], NULL, slice.sample_number_reference_channel_name, NULL);

	// show time slices
	if (show_time_slices == TRUE_m10) {
		printf_m10("%sInput Time Slice:%s\n", TC_RED_m10, TC_RESET_m10);
		show_time_slice_m10(&slice);
	}

	// read session
	read_time_series_data = TRUE_m10;
	read_record_data = TRUE_m10;
	sess = read_session_m10(sess_dir, channel_list, n_channels, &slice, password, read_time_series_data, read_record_data);
	if (sess == NULL) {
		error_message_m10("%s(): error reading session\n", __FUNCTION__);
		exit_m10(1);
	}

	// show time slices
	if (show_time_slices == TRUE_m10) {
		printf_m10("%sSession Time Slice:%s\n", TC_RED_m10, TC_RESET_m10);
		show_time_slice_m10(&sess->time_slice);
	}

	// show session records
	if (show_records == TRUE_m10) {
		if (sess->record_data_fps != NULL)
			show_records_m10(sess->record_data_fps, ALL_TYPES_CODE_m10);
		if (sess->segmented_record_data_fps != NULL) {
			for (i = 0; i < sess->number_of_segments; ++i) {
				if (sess->segmented_record_data_fps[i] != NULL) {
					show_records_m10(sess->segmented_record_data_fps[i], ALL_TYPES_CODE_m10);
					// show_records_m10(sess->segmented_record_data_fps[i], REC_Note_TYPE_CODE_m10);
				}
			}
		}
	}

	// write out raw data / show records
	for (i = 0; i < sess->number_of_time_series_channels; ++i) {

		chan = sess->time_series_channels[i];

		// show time slices
		if (show_time_slices == TRUE_m10) {
			printf_m10("%sChannel %ld Time Slice:%s\n", TC_RED_m10, i, TC_RESET_m10);
			show_time_slice_m10(&chan->time_slice);
		}

		// show channel records
		if (chan->record_data_fps != NULL && show_records == TRUE_m10)
			show_records_m10(chan->record_data_fps, ALL_TYPES_CODE_m10);

		// open output time series data files
		sprintf_m10(out_file, "%s/%s.raw", out_dir, chan->name);
		out_fp = fopen_m10(out_file, "w", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);

		for (j = 0; j < chan->number_of_segments; ++j) {

			seg = chan->segments[j];

			// show FPSs
			if (show_file_processing_structs == TRUE_m10)
				show_file_processing_struct_m10(seg->metadata_fps);

			// show segment records
			if (seg->record_data_fps != NULL && show_records == TRUE_m10)
				show_records_m10(seg->record_data_fps, ALL_TYPES_CODE_m10);

			// show time slices
			if (show_time_slices == TRUE_m10) {
				printf_m10("%sChannel %ld, Segment %ld Time Slice:%s\n", TC_RED_m10, i, j, TC_RESET_m10);
				show_time_slice_m10(&seg->time_slice);
			}

			cps = seg->time_series_data_fps->cps;
			n_samps = seg->time_slice.number_of_samples;
			fwrite_m10(cps->decompressed_data, sizeof(si4), n_samps, out_fp, out_file, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		}
		fclose(out_fp);
	}

	// clean up
	free_session_m10(sess);
	free_m10((void *) in_dir, __FUNCTION__, __LINE__);

	#ifdef WINDOWS_m10
	win_cleanup_m10();
	#endif
	return(0);
}
