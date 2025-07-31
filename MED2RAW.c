
// MED2RAW.c
// Written by Matt Stead
// Copyright Dark Horse Neuro Inc, 2020


#include "medlib_m13.h"


#ifndef MAX_CHANNELS
	#define MAX_CHANNELS	512
#endif
#define MED2RAW_VER_MAJOR	((ui1) 1)
#define MED2RAW_VER_MINOR	((ui1) 3)



si4	main(si4 argc, si1 **argv)
{
	extern GLOBALS_m13	*globals_m13;
	tern			show_recs, show_fps, show_slices;
	const si1		*password;
	si1			out_dir[PATH_BYTES_m13], out_file[PATH_BYTES_m13], *app_name, idx_chan_name[NAME_BYTES_m13], *end_ptr;
	si4			list_len, seg_idx, n_segs;
	ui8			lh_flags;
	si8			i, j, k, nw, n_samps;
	void			*file_list;
	FILE_m13		*out_fp;
	SESS_m13		*sess;
	SSR_m13			*ssr;  // segmented session records
	CHAN_m13		*chan;
	SEG_m13			*seg;
	CPS_m13			*cps;
	SLICE_m13		slice;
	
	
	// NOTE: in MacOS & Linux change resource limits before calling any functions that use system resources (e.g. printf())
	PROC_adjust_open_file_limit_m13(MAX_OPEN_FILES_m13(MAX_CHANNELS, 1), TRUE_m13);

	// command format output
	if (argc == 2) {
		if (strcmp(argv[1], "-command_format") == 0) {
			fprintf(stderr, "arg: MED_directory (dir) (it)\narg: output_directory (dir) (opt)\narg: start_time (int) (opt)\narg: end_time (int) (opt)\narg: start_index (int) (opt)\narg: end_index (int) (opt)\narg: password (str) (opt)\narg: index_channel (str) (opt)\nver: %hhu.%hhu\n", MED2RAW_VER_MAJOR, MED2RAW_VER_MINOR);
			return(0);  // standard return required because medlib is not initialized at this point
		}
	}

	// initialize medlib
	// prototype: tern init_all_tables, si1 *app_path, ... )  // varargs(app_path not empty): ui4 version_major, ui4 version_minor
	// set initialize_all_tables to FALSE_m13 to reduce initialization time (any required tables will be loaded on demand)
	G_init_medlib_m13(FALSE_m13, argv[0], MED2RAW_VER_MAJOR, MED2RAW_VER_MINOR);
	
	// usage
	if (argc < 2 || argc > 9) {
		app_name = out_dir;  // using out_dir string for app_name - just aliasing for code clarity
		G_path_parts_m13(argv[0], NULL, app_name, NULL);
		printf("\n\t%s version %hhu.%hhu\n\n\tUSAGE: %s MED_directory (multiple w/ regex) [output_directory] [start_time] [end_time] [start_index] [end_index] [password] [index_channel]\n\n", app_name, MED2RAW_VER_MAJOR, MED2RAW_VER_MINOR, argv[0]);
		G_free_globals_m13(TRUE_m13);  // do this mainly to clean up for Windows; OS will free globals if omitted, but will not Windows reset terminal
		return(0);  // standard return required because globals freed
	}

	// customize globals (just examples, not exhaustive list)
	// globals_m13->default_behavior.code = E_BEHAVIOR_m13;  // empty behavior stacks will default to this (E_BEHAVIOR_m13 == exit on fail, register errors, do not retry, show all output)
	// globals_m13->miscellaneous.file_lock_mode = FLOCK_MODE_MED_m13;  // use file locking on MED files (only)
	// globals_m13->miscellaneous.threading = FALSE_m13;  // disable threading (typically for debugging)
	
	// Times Note:
	// negative times: relative to session start
	// positive times: offset or absolute
	
	// increase process priority
	PROC_increase_process_priority_m13(TRUE_m13, TRUE_m13, argv[0], 10.0);
			
	// initialize slice
	G_init_slice_m13(&slice);

	// testing variables: adjust to your needs
	show_recs = FALSE_m13;  // records
	show_fps = FALSE_m13;  // file processing structures
	show_slices = FALSE_m13;  // data slice extents

	// input file list
	file_list = (void *) argv[1];
	list_len = 0;

	// output directory
	*out_dir = 0;
	if (argc >= 3)
		if (*argv[2])
			strcpy(out_dir, argv[2]);
	if (*out_dir)
		G_full_path_m13(out_dir, out_dir);
	else
		getcwd_m13(out_dir, PATH_BYTES_m13);

	// start time
	if (argc >= 4) {
		if (*argv[3]) {
			if ((strcmp(argv[3], "start")) == 0) {
				slice.start_time = BEGINNING_OF_TIME_m13;
			} else {
				slice.start_time = (si8) strtol(argv[3], &end_ptr, 10);
				if (end_ptr == argv[3])
					slice.start_time = UUTC_NO_ENTRY_m13;
			}
		}
	}

	// end time
	if (argc >= 5) {
		if (*argv[4]) {
			if ((strcmp(argv[4], "end")) == 0) {
				slice.end_time = END_OF_TIME_m13;
			} else {
				slice.end_time = (si8) strtol(argv[4], &end_ptr, 10);
				if (end_ptr == argv[4])
					slice.end_time = UUTC_NO_ENTRY_m13;
			}
		}
	}

	// start sample number
	if (argc >= 6) {
		if (*argv[5]) {
			if ((strcmp(argv[5], "start")) == 0) {
				slice.start_samp_num = BEGINNING_OF_SAMPLE_NUMBERS_m13;
			} else {
				slice.start_samp_num = (si8) strtol(argv[5], &end_ptr, 10);
				if (end_ptr == argv[5])
					slice.start_samp_num = SAMPLE_NUMBER_NO_ENTRY_m13;
			}
		}
	}

	// end sample number
	if (argc >= 7) {
		if (*argv[6]) {  // LINUX strtol() returns zero for a zero-length string and does not set errno for EINVAL
			if ((strcmp(argv[6], "end")) == 0) {
				slice.end_samp_num = END_OF_SAMPLE_NUMBERS_m13;
			} else {
				slice.end_samp_num = (si8) strtol(argv[6], &end_ptr, 10);
				if (end_ptr == argv[6])
					slice.end_samp_num = SAMPLE_NUMBER_NO_ENTRY_m13;
			}
		}
	}

	// password
	password = NULL;
	if (argc >= 8)
		password = argv[7];

	// index channel name (base name only)
	*idx_chan_name = 0;
	if (argc == 9)
		if (*argv[8])
			G_path_parts_m13(argv[8], NULL, idx_chan_name, NULL);

	// show slices
	if (show_slices == TRUE_m13) {
		printf_m13("%sInput Slice:%s\n", TC_RED_m13, TC_RESET_m13);
		G_show_slice_m13(&slice);
	}

	// set basic level header flags
	lh_flags = LH_EXCLUDE_VID_CHANS_m13 | LH_READ_SLICE_SEG_DATA_m13 | LH_READ_SLICE_ALL_RECS_m13;
	
	// set additional flags
	lh_flags |= LH_NO_CPS_CACHING_m13;  // no caching is more efficient for single reads in channels with with VDS encoding; for other encodings it makes no difference

	// show flags
	// G_show_level_header_flags_m13(lh_flags);

	// read session
	sess = G_read_session_m13(NULL, &slice, (void *) file_list, list_len, lh_flags, password, idx_chan_name);
	if (sess == NULL)
		return_m13(-1);  // m13 return shows error (& function stack if FT_DEBUG_m13 defined in targets.h)
	slice = sess->slice;  // copy returned session slice

	// show slices
	if (show_slices == TRUE_m13) {
		printf_m13("%sSession Slice:%s\n", TC_RED_m13, TC_RESET_m13);
		G_show_slice_m13(&slice);  // since copied above, or just G_show_slice_m13(&sess->slice)
	}

	seg_idx = G_segment_index_m13(sess, slice.start_seg_num);  // seg_idx != 0 if all segments are mapped & slice does not start at first segment (multi-read usage)
	n_segs = slice.n_segs;
	
	// set record filters
	si4	my_rec_filters[] = { REC_Sgmt_TYPE_CODE_m13, REC_Note_TYPE_CODE_m13, NO_TYPE_CODE_m13 };
	globals_m13->record_filters = my_rec_filters;  // make global
	
	// show session records
	if (show_recs == TRUE_m13) {
		if (sess->rec_data_fps)
			G_show_records_m13(sess->rec_data_fps, my_rec_filters);  // pass NULL for filters to use global filters (if globals also NULL, it shows all record types)
		ssr = sess->ssr;
		if (ssr) {  // segmented session records
			for (i = 0, j = seg_idx; i < n_segs; ++i, ++j)
				if (ssr->rec_data_fps[j])
					G_show_records_m13(ssr->rec_data_fps[j], my_rec_filters);
		}
	}

	// write out raw data / show records
	for (i = 0; i < sess->n_ts_chans; ++i) {

		chan = sess->ts_chans[i];
		if (!(chan->flags & LH_CHAN_ACTIVE_m13))
			continue;  // multi-read usage

		// show slice
		if (show_slices == TRUE_m13) {
			printf_m13("%sChannel %s Slice:%s\n", TC_RED_m13, chan->name, TC_RESET_m13);
			G_show_slice_m13(&chan->slice);
		}

		// show channel records
		if (chan->rec_data_fps && show_recs == TRUE_m13)
			G_show_records_m13(chan->rec_data_fps, NULL);

		// open output time series data files
		sprintf_m13(out_file, "%s/%s.raw", out_dir, chan->name);
		out_fp = fopen_m13(out_file, "w");

		for (j = 0, k = seg_idx; j < n_segs; ++j, ++k) {

			seg = chan->segs[k];

			// show FPSs
			if (show_fps == TRUE_m13)
				FPS_show_m13(seg->metadata_fps);

			// show segment records, if they exist
			if (seg->rec_data_fps && show_recs == TRUE_m13)
				G_show_records_m13(seg->rec_data_fps, my_rec_filters);

			// show time slices
			if (show_slices == TRUE_m13) {
				printf_m13("%sChannel %s, Segment %ld Slice:%s\n", TC_RED_m13, chan->name, k + 1, TC_RESET_m13);
				G_show_slice_m13(&seg->slice);
			}

			cps = seg->ts_data_fps->params.cps;
			n_samps = SLICE_IDX_COUNT_S_m13(seg->slice);
			nw = fwrite_m13(cps->decompressed_data, sizeof(si4), (size_t) n_samps, out_fp);
			if (nw != n_samps) {
				fclose_m13(out_fp);
				return_m13(-1);  // m13 return shows error (& function stack if FT_DEBUG_m13 defined in targets.h)
			}
		}
		fclose_m13(out_fp);
	}

	// clean up
	G_free_session_m13(sess);  // (optional: OS will do this anyway)
	G_free_globals_m13(TRUE_m13);  // frees globals, cleans up (Windows), & displays any unfreed memory (if AT_DEBUG_m13 defined); also optional, but see below

	/* ****************************************************************************** */
	/*  Note: If not calling G_free_globals_m13(TRUE_m13), do this:                   */
	/*  (prepressor directives may be omitted: function just returns if not Windows)  */
	/*                                                                                */
	/*  #ifdef WINDOWS_m13                                                            */
	/*  WN_cleanup_m13();                                                             */
	/*  #endif                                                                        */
	/*                                                                                */
	/* ****************************************************************************** */


	return(0);  // standard return required because globals freed
}
