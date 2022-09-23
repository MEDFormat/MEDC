
// MED2RAW.c
// Written by Matt Stead
// Copyright Dark Horse Neuro Inc, 2020


#include "medlib_m11.h"

#ifndef MAX_CHANNELS
	#define MAX_CHANNELS	512
#endif
#define LS_MED2RAW_PROD_VER_MAJOR	((ui1) 1)
#define LS_MED2RAW_PROD_VER_MINOR	((ui1) 0)



int main(int argc, char *argv[])
{
	extern GLOBALS_m11		*globals_m11;
	TERN_m11			show_records, show_file_processing_structs, show_time_slices;
	si1				out_dir[FULL_FILE_NAME_BYTES_m11], out_file[FULL_FILE_NAME_BYTES_m11], *password;
	si4				list_len, seg_idx, n_segs;
	ui8				flags;
	si8				i, j, k, n_samps;
	void				*file_list;
	FILE				*out_fp;
	SESSION_m11			*sess;
	SEGMENTED_SESS_RECS_m11		*ssr;
	CHANNEL_m11			*chan;
	SEGMENT_m11			*seg;
	CMP_PROCESSING_STRUCT_m11	*cps;
	TIME_SLICE_m11			slice;
	
	
	// NOTE: in MacOS & Linux change resource limits before calling any functions that use system resources (e.g. printf())
	adjust_open_file_limit_m11(MAX_OPEN_FILES_m11(MAX_CHANNELS, 1), TRUE_m11);

	// initialize medlib
	// prototype: TERN_m11 initialize_medlib_m11(TERN_m11 check_structure_alignments, TERN_m11 initialize_all_tables)
	// change check_structure_alignments to FALSE_m11 to reduce initialization time (all structures align on all systems tested so far)
	// change initialize_all_tables to FALSE_m11 to reduce initialization time (any required table will be loaded if/when it is first accessed)
	initialize_medlib_m11(TRUE_m11, TRUE_m11);
	
	// usage
	if (argc < 2 || argc > 9) {
		extract_path_parts_m11(argv[0], NULL, out_dir, NULL);
		printf_m11("%c\n\t%s version %hhu.%hhu\n\n\tUSAGE: %s MED_directory (multiple w/ regex) [output_directory] [start_time] [end_time] [start_samp_num] [end_samp_num] [password] [samp_num_ref_chan]\n\n", 7, out_dir, LS_MED2RAW_PROD_VER_MAJOR, LS_MED2RAW_PROD_VER_MINOR, argv[0]);
		exit_m11(0);
	}
	// USAGE:
	// negative times: relative to session start
	// positive times: offset or absolute

	// initialize time slice
	initialize_time_slice_m11(&slice);

	// testing variables: adjust to your needs
	show_records = FALSE_m11;
	show_file_processing_structs = FALSE_m11;
	show_time_slices = FALSE_m11;

	// input file list
	file_list = (void *) argv[1];
	list_len = 0;

	// output directory
	if (argc >= 3)
		strcpy(out_dir, argv[2]);
	else
		getcwd_m11(out_dir, FULL_FILE_NAME_BYTES_m11);

	// start time
	if (argc >= 4) {
		if (*argv[3]) {  // LINUX strtol() returns zero for a zero-length string and does not set errno for EINVAL
			if ((strcmp(argv[3], "start")) == 0)
				slice.start_time = BEGINNING_OF_TIME_m11;
			else
				slice.start_time = (si8) strtol(argv[3], NULL, 10);
		}
	}

	// end time
	if (argc >= 5) {
		if (*argv[4]) {  // LINUX strtol() returns zero for a zero-length string and does not set errno for EINVAL
			if ((strcmp(argv[4], "end")) == 0)
				slice.end_time = END_OF_TIME_m11;
			else {
				slice.end_time = (si8) strtol(argv[4], NULL, 10);
				if (slice.end_time == 0)
					slice.end_time = UUTC_NO_ENTRY_m11;
			}
		}
	}

	// start sample number
	if (argc >= 6) {
		if (*argv[5]) {  // LINUX strtol() returns zero for a zero-length string and does not set errno for EINVAL
			if ((strcmp(argv[5], "start")) == 0)
				slice.start_sample_number = BEGINNING_OF_SAMPLE_NUMBERS_m11;
			else
				slice.start_sample_number = (si8) strtol(argv[5], NULL, 10);
		}
	}

	// end sample number
	if (argc >= 7) {
		if (*argv[6]) {  // LINUX strtol() returns zero for a zero-length string and does not set errno for EINVAL
			if ((strcmp(argv[6], "end")) == 0)
				slice.end_sample_number = END_OF_SAMPLE_NUMBERS_m11;
			else {
				slice.end_sample_number = (si8) strtol(argv[6], NULL, 10);
				if (slice.end_sample_number == 0)
					slice.end_time = SAMPLE_NUMBER_NO_ENTRY_m11;
			}
		}
	}

	// password
	password = NULL;
	if (argc >= 8)
		password = argv[7];

	// sample number reference channel name (base name only)
	if (argc == 9)
		extract_path_parts_m11(argv[8], NULL, globals_m11->reference_channel_name, NULL);

	// show time slices
	if (show_time_slices == TRUE_m11) {
		printf_m11("%sInput Time Slice:%s\n", TC_RED_m11, TC_RESET_m11);
		show_time_slice_m11(&slice);
	}

	// read session
	flags = LH_INCLUDE_TIME_SERIES_CHANNELS_m11 | LH_READ_SLICE_SEGMENT_DATA_m11 | LH_READ_SLICE_ALL_RECORDS_m11;
	// show_level_header_flags_m11(flags);
	sess = read_session_m11(NULL, &slice, (void *) file_list, list_len, flags, password);
	if (sess == NULL) {
		error_message_m11("%s(): error reading session\n", __FUNCTION__);
		exit_m11(1);
	}
	slice = sess->time_slice;

	// show time slices
	if (show_time_slices == TRUE_m11) {
		printf_m11("%sSession Time Slice:%s\n", TC_RED_m11, TC_RESET_m11);
		show_time_slice_m11(&slice);
	}

	seg_idx = get_segment_index_m11(slice.start_segment_number);  // seg_idx != 0 if all segments are mapped & slice does not start at first segment (multi-read usage)
	n_segs = slice.number_of_segments;

	// show session records
	si4	my_rec_filters[] = { REC_Sgmt_TYPE_CODE_m11, REC_Note_TYPE_CODE_m11, NO_TYPE_CODE_m11 };
	// globals_m11->record_filters = my_rec_filters;  // make global
	if (show_records == TRUE_m11) {
		if (sess->record_data_fps != NULL)
			show_records_m11(sess->record_data_fps, my_rec_filters);  // pass NULL for filters to use global filters (if globals also NULL, it shows all record types)
		ssr = sess->segmented_sess_recs;
		if (ssr != NULL) {
			for (i = 0, j = seg_idx; i < n_segs; ++i, ++j) {
				if (ssr->record_data_fps[j] != NULL)
					show_records_m11(ssr->record_data_fps[j], my_rec_filters);
			}
		}
	}

	// write out raw data / show records
	for (i = 0; i < sess->number_of_time_series_channels; ++i) {

		chan = sess->time_series_channels[i];
		if (!(chan->flags & LH_CHANNEL_ACTIVE_m11))
			continue;  // multi-read usage

		// show time slices
		if (show_time_slices == TRUE_m11) {
			printf_m11("%sChannel %s Time Slice:%s\n", TC_RED_m11, chan->name, TC_RESET_m11);
			show_time_slice_m11(&chan->time_slice);
		}

		// show channel records
		if (chan->record_data_fps != NULL && show_records == TRUE_m11)
			show_records_m11(chan->record_data_fps, NULL);

		// open output time series data files
		sprintf_m11(out_file, "%s/%s.raw", out_dir, chan->name);
		out_fp = fopen_m11(out_file, "w", __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);

		for (j = 0, k = seg_idx; j < n_segs; ++j, ++k) {

			seg = chan->segments[k];

			// show FPSs
			if (show_file_processing_structs == TRUE_m11)
				FPS_show_processing_struct_m11(seg->metadata_fps);

			// show segment records
			if (seg->record_data_fps != NULL && show_records == TRUE_m11)
				show_records_m11(seg->record_data_fps, my_rec_filters);

			// show time slices
			if (show_time_slices == TRUE_m11) {
				printf_m11("%sChannel %s, Segment %ld Time Slice:%s\n", TC_RED_m11, chan->name, k + 1, TC_RESET_m11);
				show_time_slice_m11(&seg->time_slice);
			}

			cps = seg->time_series_data_fps->parameters.cps;
			n_samps = TIME_SLICE_SAMPLE_COUNT_S_m11(seg->time_slice);
			fwrite_m11(cps->decompressed_data, sizeof(si4), n_samps, out_fp, out_file, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
		}
		fclose(out_fp);
	}

	// clean up
	free_session_m11(sess, TRUE_m11);
	free_globals_m11(TRUE_m11);
	
	return(0);
}
