
// MED2RAW.c
// Written by Matt Stead
// Copyright Dark Horse Neuro Inc, 2020


#include "medlib_m12.h"


#ifndef MAX_CHANNELS
	#define MAX_CHANNELS	512
#endif
#define MED2RAW_PROD_VER_MAJOR	((ui1) 1)
#define MED2RAW_PROD_VER_MINOR	((ui1) 1)



si4	main(si4 argc, si1 **argv)
{
	extern GLOBALS_m12		*globals_m12;
	TERN_m12			show_records, show_file_processing_structs, show_time_slices;
	si1				out_dir[FULL_FILE_NAME_BYTES_m12], out_file[FULL_FILE_NAME_BYTES_m12], *password, *end_ptr;
	si4				list_len, seg_idx, n_segs;
	ui8				flags;
	si8				i, j, k, n_samps;
	void				*file_list;
	FILE				*out_fp;
	SESSION_m12			*sess;
	SEGMENTED_SESS_RECS_m12		*ssr;
	CHANNEL_m12			*chan;
	SEGMENT_m12			*seg;
	CMP_PROCESSING_STRUCT_m12	*cps;
	TIME_SLICE_m12			slice;
	
	
	// NOTE: in MacOS & Linux change resource limits before calling any functions that use system resources (e.g. printf())
	PROC_adjust_open_file_limit_m12(MAX_OPEN_FILES_m12(MAX_CHANNELS, 1), TRUE_m12);

	// command format output
	if (argc == 2) {
		if (strcmp(argv[1], "-command_format") == 0) {
			fprintf(stderr, "arg: MED_directory (dir) (it)\narg: output_directory (dir) (opt)\narg: start_time (int) (opt)\narg: end_time (int) (opt)\narg: start_samp_num (int) (opt)\narg: end_samp_num (int) (opt)\narg: password (str) (opt)\narg: samp_num_ref_chan (str) (opt)\nver: %hhu.%hhu\n", MED2RAW_PROD_VER_MAJOR, MED2RAW_PROD_VER_MINOR);
			return(0);
		}
	}
	
	// initialize medlib
	// prototype: TERN_m12 initialize_medlib_m12(TERN_m12 check_structure_alignments, TERN_m12 initialize_all_tables)
	// change check_structure_alignments to FALSE_m12 to reduce initialization time (all structures align on all systems tested so far)
	// change initialize_all_tables to FALSE_m12 to reduce initialization time (any required table will be loaded if/when it is first accessed)
	G_initialize_medlib_m12(FALSE_m12, FALSE_m12);

	// usage
	if (argc < 2 || argc > 9) {
		G_extract_path_parts_m12(argv[0], NULL, out_dir, NULL);
		printf_m12("%c\n\t%s version %hhu.%hhu\n\n\tUSAGE: %s MED_directory (multiple w/ regex) [output_directory] [start_time] [end_time] [start_samp_num] [end_samp_num] [password] [samp_num_ref_chan]\n\n", 7, out_dir, MED2RAW_PROD_VER_MAJOR, MED2RAW_PROD_VER_MINOR, argv[0]);
		exit_m12(0);
	}
	// USAGE:
	// negative times: relative to session start
	// positive times: offset or absolute
	
	// increase process priority
	PROC_increase_process_priority_m12(TRUE_m12, TRUE_m12, argv[0], 10.0);

	// initialize time slice
	G_initialize_time_slice_m12(&slice);

	// testing variables: adjust to your needs
	show_records = FALSE_m12;
	show_file_processing_structs = TRUE_m12;
	show_time_slices = TRUE_m12;

	// input file list
	file_list = (void *) argv[1];
	list_len = 0;

	// output directory
	if (argc >= 3)
		strcpy(out_dir, argv[2]);
	else
		getcwd_m12(out_dir, FULL_FILE_NAME_BYTES_m12);

	// start time
	if (argc >= 4) {
		if (*argv[3]) {
			if ((strcmp(argv[3], "start")) == 0) {
				slice.start_time = BEGINNING_OF_TIME_m12;
			} else {
				slice.start_time = (si8) strtol(argv[3], &end_ptr, 10);
				if (end_ptr == argv[3])
					slice.start_time = UUTC_NO_ENTRY_m12;
			}
		}
	}

	// end time
	if (argc >= 5) {
		if (*argv[4]) {
			if ((strcmp(argv[4], "end")) == 0) {
				slice.end_time = END_OF_TIME_m12;
			} else {
				slice.end_time = (si8) strtol(argv[4], &end_ptr, 10);
				if (end_ptr == argv[4])
					slice.end_time = UUTC_NO_ENTRY_m12;
			}
		}
	}

	// start sample number
	if (argc >= 6) {
		if (*argv[5]) {
			if ((strcmp(argv[5], "start")) == 0) {
				slice.start_sample_number = BEGINNING_OF_SAMPLE_NUMBERS_m12;
			} else {
				slice.start_sample_number = (si8) strtol(argv[5], &end_ptr, 10);
				if (end_ptr == argv[5])
					slice.start_sample_number = SAMPLE_NUMBER_NO_ENTRY_m12;
			}
		}
	}

	// end sample number
	if (argc >= 7) {
		if (*argv[6]) {  // LINUX strtol() returns zero for a zero-length string and does not set errno for EINVAL
			if ((strcmp(argv[6], "end")) == 0)
				slice.end_sample_number = END_OF_SAMPLE_NUMBERS_m12;
			else {
				slice.end_sample_number = (si8) strtol(argv[6], &end_ptr, 10);
				if (end_ptr == argv[6])
					slice.end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m12;
			}
		}
	}

	// password
	password = NULL;
	if (argc >= 8)
		password = argv[7];

	// sample number reference channel name (base name only)
	if (argc == 9)
		G_extract_path_parts_m12(argv[8], NULL, globals_m12->reference_channel_name, NULL);

	// show time slices
	if (show_time_slices == TRUE_m12) {
		printf_m12("%sInput Time Slice:%s\n", TC_RED_m12, TC_RESET_m12);
		G_show_time_slice_m12(&slice);
	}

	// read session
	flags = LH_INCLUDE_TIME_SERIES_CHANNELS_m12 | LH_READ_SLICE_SEGMENT_DATA_m12 | LH_READ_SLICE_ALL_RECORDS_m12 | LH_NO_CPS_CACHING_m12;  // no caching more efficient for single reads with VDS;
	// show_level_header_flags_m12(flags);
	sess = G_read_session_m12(NULL, &slice, (void *) file_list, list_len, flags, password);
	if (sess == NULL) {
		G_error_message_m12("%s(): error reading session\n", __FUNCTION__);
		exit_m12(1);
	}
	slice = sess->time_slice;
		
	// show time slices
	if (show_time_slices == TRUE_m12) {
		printf_m12("%sSession Time Slice:%s\n", TC_RED_m12, TC_RESET_m12);
		G_show_time_slice_m12(&slice);
	}

	seg_idx = G_get_segment_index_m12(slice.start_segment_number);  // seg_idx != 0 if all segments are mapped & slice does not start at first segment (multi-read usage)
	n_segs = slice.number_of_segments;

	// show session records
	si4	my_rec_filters[] = { REC_Sgmt_TYPE_CODE_m12, REC_Note_TYPE_CODE_m12, NO_TYPE_CODE_m12 };
	// globals_m12->record_filters = my_rec_filters;  // make global
	if (show_records == TRUE_m12) {
		if (sess->record_data_fps != NULL)
			G_show_records_m12(sess->record_data_fps, my_rec_filters);  // pass NULL for filters to use global filters (if globals also NULL, it shows all record types)
		ssr = sess->segmented_sess_recs;
		if (ssr != NULL) {
			for (i = 0, j = seg_idx; i < n_segs; ++i, ++j) {
				if (ssr->record_data_fps[j] != NULL)
					G_show_records_m12(ssr->record_data_fps[j], my_rec_filters);
			}
		}
	}

	// write out raw data / show records
	for (i = 0; i < sess->number_of_time_series_channels; ++i) {

		chan = sess->time_series_channels[i];
		if (!(chan->flags & LH_CHANNEL_ACTIVE_m12))
			continue;  // multi-read usage

		// show time slices
		if (show_time_slices == TRUE_m12) {
			printf_m12("%sChannel %s Time Slice:%s\n", TC_RED_m12, chan->name, TC_RESET_m12);
			G_show_time_slice_m12(&chan->time_slice);
		}

		// show channel records
		if (chan->record_data_fps != NULL && show_records == TRUE_m12)
			G_show_records_m12(chan->record_data_fps, NULL);

		// open output time series data files
		sprintf_m12(out_file, "%s/%s.raw", out_dir, chan->name);
		out_fp = fopen_m12(out_file, "w", __FUNCTION__, USE_GLOBAL_BEHAVIOR_m12);

		for (j = 0, k = seg_idx; j < n_segs; ++j, ++k) {

			seg = chan->segments[k];

			// show FPSs
			if (show_file_processing_structs == TRUE_m12)
				FPS_show_processing_struct_m12(seg->metadata_fps);

			// show segment records
			if (seg->record_data_fps != NULL && show_records == TRUE_m12)
				G_show_records_m12(seg->record_data_fps, my_rec_filters);

			// show time slices
			if (show_time_slices == TRUE_m12) {
				printf_m12("%sChannel %s, Segment %ld Time Slice:%s\n", TC_RED_m12, chan->name, k + 1, TC_RESET_m12);
				G_show_time_slice_m12(&seg->time_slice);
			}

			cps = seg->time_series_data_fps->parameters.cps;
			n_samps = TIME_SLICE_SAMPLE_COUNT_S_m12(seg->time_slice);
			fwrite_m12(cps->decompressed_data, sizeof(si4), n_samps, out_fp, out_file, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m12);
		}
		fclose(out_fp);
	}

	// clean up
	G_free_session_m12(sess, TRUE_m12);
	G_free_globals_m12(TRUE_m12);

	return(0);
}
