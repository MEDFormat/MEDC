
// MED2DAT.c
// Written by Matt Stead
// Copyright Dark Horse Neuro Inc, 2020


#include "medlib_m12.h"


#ifndef MAX_CHANNELS
	#define MAX_CHANNELS	512
#endif
#define UV_DAT_CONVERSION_FACTOR	((sf8) 5.128205316306578)  // µV to DAT native amplitude
#define LS_MED2DAT_VER_MAJOR		((ui1) 1)
#define LS_MED2DAT_VER_MINOR		((ui1) 1)


int main(int argc, char *argv[])
{
	extern GLOBALS_m12			*globals_m12;
	TERN_m12				use_matrix;
	si1					out_dir[FULL_FILE_NAME_BYTES_m12], out_file[FULL_FILE_NAME_BYTES_m12], *password, *end_ptr;
	si2					*out_arr;
	si4					n_channels, seg_idx, list_len, prog_loop_ctr;
	ui8					flags, dm_flags;
	si8					i, j, k, m, samps_per_min, n_samps, n_segs, slice_end_samp, start_samp, end_samp, total_samps, span;
	sf8					tmp_sf8, out_fs, goal_samps, progress, highpass_cutoff, *tx, *qx, *sf8_p1, *sf8_p2, *last_avgs, diff, inc;
	void					*file_list;
	FILE					*out_fp;
	SESSION_m12				*sess;
	CHANNEL_m12				*chan;
	SEGMENT_m12				*seg;
	CMP_PROCESSING_STRUCT_m12		*cps;
	TIME_SLICE_m12				*slice, local_slice;
	DATA_MATRIX_m12				*dm;
	
	
	// NOTE: in MacOS & Linux change resource limits before calling any functions that use system resources (e.g. printf())
	PROC_adjust_open_file_limit_m12(MAX_OPEN_FILES_m12(MAX_CHANNELS, 1), TRUE_m12);
	
	// command format output
	if (argc == 2) {
		if (strcmp(argv[1], "-command_format") == 0) {
			fprintf(stderr, "arg: MED_directory (dir) (it)\narg: output_directory (dir) (opt)\narg: start_time (int) (opt)\narg: end_time (int) (opt)\narg: start_samp_num (int) (opt)\narg: end_samp_num (int) (opt)\narg: password (str) (opt)\narg: samp_num_ref_chan (str) (opt)\narg: highpass_cutoff (float) (opt)\nver: %hhu.%hhu\n", LS_MED2DAT_VER_MAJOR, LS_MED2DAT_VER_MINOR);
			return(0);
		}
	}
	
	// initialize library
	G_initialize_medlib_m12(FALSE_m12, FALSE_m12);
	
	// usage
	if (argc < 2 || argc > 10) {
		G_extract_path_parts_m12(argv[0], NULL, out_dir, NULL);
		printf_m12("%c\n\t%s version %hhu.%hhu\n\n\tUSAGE: %s MED_directory (multiple w/ regex) [output_directory]  [start_time] [end_time] [start_samp_num] [end_samp_num] [password] [samp_num_ref_chan] [highpass_cutoff (Hz)]\n\n", 7, out_dir, LS_MED2DAT_VER_MAJOR, LS_MED2DAT_VER_MINOR, argv[0]);
		exit_m12(0);
	}

	// increase process priority
	PROC_increase_process_priority_m12(TRUE_m12, TRUE_m12, argv[0], 10.0);  // 10 second timeout on password entry
	
	// initialize time slice
	slice = &local_slice;
	G_initialize_time_slice_m12(slice);
	
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
				slice->start_time = BEGINNING_OF_TIME_m12;
			} else {
				slice->start_time = (si8) strtol(argv[3], &end_ptr, 10);
				if (end_ptr == argv[3])
					slice->start_time = UUTC_NO_ENTRY_m12;
			}
		}
	}
	
	// end time
	if (argc >= 5) {
		if (*argv[4]) {
			if ((strcmp(argv[4], "end")) == 0) {
				slice->end_time = END_OF_TIME_m12;
			} else {
				slice->end_time = (si8) strtol(argv[4], &end_ptr, 10);
				if (end_ptr == argv[4])
					slice->end_time = UUTC_NO_ENTRY_m12;
			}
		}
	}
	
	// start sample number
	if (argc >= 6) {
		if (*argv[5]) {
			if ((strcmp(argv[5], "start")) == 0) {
				slice->start_sample_number = BEGINNING_OF_SAMPLE_NUMBERS_m12;
			} else {
				slice->start_sample_number = (si8) strtol(argv[5], &end_ptr, 10);
				if (end_ptr == argv[5])
					slice->start_sample_number = SAMPLE_NUMBER_NO_ENTRY_m12;
			}
		}
	}
	
	// end sample number
	if (argc >= 7) {
		if (*argv[6]) {  // LINUX strtol() returns zero for a zero-length string and does not set errno for EINVAL
			if ((strcmp(argv[6], "end")) == 0)
				slice->end_sample_number = END_OF_SAMPLE_NUMBERS_m12;
			else {
				slice->end_sample_number = (si8) strtol(argv[6], &end_ptr, 10);
				if (end_ptr == argv[6])
					slice->end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m12;
			}
		}
	}
	
	// password
	password = NULL;
	if (argc >= 8)
		password = argv[7];
	
	// sample number reference channel name (base name only)
	if (argc >= 9)
		strcpy(globals_m12->reference_channel_name, argv[8]);
	
	// highpass cutoff
	highpass_cutoff = (sf8) 0.0;
	if (argc == 10) {
		highpass_cutoff = strtod(argv[9], &end_ptr);
		if (end_ptr == argv[9] || highpass_cutoff < (sf8) 0.0)
			highpass_cutoff = (sf8) 0.0;
	}
	
	// open session
	flags = LH_INCLUDE_TIME_SERIES_CHANNELS_m12 | LH_READ_SLICE_SEGMENT_DATA_m12 | LH_MAP_ALL_SEGMENTS_m12;
	sess = G_open_session_m12(NULL, slice, file_list, list_len, flags, password);
	if (sess == NULL)
		G_error_message_m12("%s(): error opening session\n", __FUNCTION__);
	slice = &sess->time_slice;
	n_channels = sess->number_of_time_series_channels;
	out_fs = globals_m12->maximum_time_series_frequency;
	
	// use matrix for variable sampling frequency
	out_arr = NULL;
	dm = NULL;
	if (globals_m12->time_series_frequencies_vary == TRUE_m12) {
		use_matrix = TRUE_m12;
		if (slice->start_time == UUTC_NO_ENTRY_m12) {
			slice->start_time = G_uutc_for_sample_number_m12((LEVEL_HEADER_m12 *) sess, slice->start_sample_number, FIND_START_m12);
			slice->start_sample_number = SAMPLE_NUMBER_NO_ENTRY_m12;
			slice->end_time = G_uutc_for_sample_number_m12((LEVEL_HEADER_m12 *) sess, slice->end_sample_number, FIND_END_m12);
			slice->end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m12;
		}
		G_change_reference_channel_m12(sess, NULL, NULL, HIGHEST_RATE_TIME_SERIES_CHANNEL_m12);
		dm_flags = DM_SCALE_m12 | DM_EXTMD_SAMP_COUNT_m12 | DM_EXTMD_ABSOLUTE_LIMITS_m12 | DM_INTRP_UP_MAKIMA_DN_LINEAR_m12;
		if (highpass_cutoff == (sf8) 0.0) {
			dm_flags |= (DM_TYPE_SI2_m12 | DM_FMT_SAMPLE_MAJOR_m12);
		} else {
			dm_flags |= (DM_TYPE_SF8_m12 | DM_FMT_CHANNEL_MAJOR_m12);
			out_arr = (si2 *) malloc_m12((size_t) n_channels * sizeof(si2), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m12);
		}
		G_warning_message_m12("Variable sampling frequencies => progress updates may be slow\n");
	} else {
		use_matrix = FALSE_m12;
		out_arr = (si2 *) malloc_m12((size_t) n_channels * sizeof(si2), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m12);
	}
	
	// set up for write out
	sprintf_m12(out_file, "%s/%s.dat", out_dir, globals_m12->fs_session_name);  // these can be subsets (e.g. just the micros), so use FS name
	out_fp = fopen_m12(out_file, "w", __FUNCTION__, USE_GLOBAL_BEHAVIOR_m12);
	
	// read session in one minute segments
	if (slice->start_sample_number == SAMPLE_NUMBER_NO_ENTRY_m12) {
		slice->start_sample_number = G_sample_number_for_uutc_m12((LEVEL_HEADER_m12 *) sess, slice->start_time, FIND_CURRENT_m12);
		slice->end_sample_number = G_sample_number_for_uutc_m12((LEVEL_HEADER_m12 *) sess, slice->end_time, FIND_CURRENT_m12);
	}
	goal_samps = (sf8) TIME_SLICE_SAMPLE_COUNT_m12(slice);;
	slice_end_samp = slice->end_sample_number;
	samps_per_min = (si8) (out_fs * (sf8) 60.0);
	qx = tx = NULL;
	if (highpass_cutoff > (sf8) 0.0) {
		qx = (sf8 *) malloc_m12((size_t) samps_per_min * sizeof(sf8), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m12);
		if (use_matrix == FALSE_m12)
			tx = (sf8 *) malloc_m12((size_t) samps_per_min * sizeof(sf8), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m12);
		span = (si8) round(out_fs / highpass_cutoff);
		if (span > samps_per_min) {
			span = samps_per_min;
			G_warning_message_m12("Highpass cutoff too low => changed to %lf Hz\n", out_fs / (sf8) span);
		}
		last_avgs = (sf8 *) malloc_m12((size_t) n_channels * sizeof(sf8), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m12);
	}
	start_samp = slice->start_sample_number;
	end_samp = start_samp + samps_per_min - 1;
	progress = 0.0;
	total_samps = 0;
	printf_m12("\nReading MED data ...\n\nProgress: 0.00%%   ");  fflush(stdout);
	while (start_samp <= slice_end_samp) {
		slice->start_time = slice->end_time = UUTC_NO_ENTRY_m12;  // use indices
		slice->start_sample_number = start_samp;
		slice->end_sample_number = end_samp;
		slice->number_of_segments = UNKNOWN_m12;
		if (use_matrix == TRUE_m12) {
			if (dm == NULL)
				dm = DM_get_matrix_m12(dm, sess, NULL, TRUE_m12, samps_per_min, (sf8) 0.0, dm_flags, UV_DAT_CONVERSION_FACTOR, (sf8) 0.0, (sf8) 0.0);
			else
				dm = DM_get_matrix_m12(dm, sess, NULL, FALSE_m12);
			if (dm == NULL)
				G_error_message_m12("%s(): error generating matrix\n", __FUNCTION__);
			if (highpass_cutoff > (sf8) 0.0) {
				sf8_p1 = (sf8 *) dm->data;
				for (i = 0; i < n_channels; ++i) {  // filter sf8s, & subtract from samples in matrix
					FILT_moving_average_m12(sf8_p1, qx, dm->sample_count, span, FILT_EXTRAPOLATE_m12);
					// matrix read will update session sample numbers to SAMPLE_NUMBER_NO_ENTRY_m12 in variable sampling frequency sessions
					if (start_samp > globals_m12->reference_channel->time_slice.start_sample_number) {  // smooth tail from last window to this one
						diff = qx[0] - last_avgs[i];
						last_avgs[i] = qx[dm->sample_count - 1];
						inc = diff / (sf8) (span / 2);
						for (j = 0; j < (span / 2); ++j)
							qx[j] -= (diff -= inc);
					}
					sf8_p2 = qx;
					for (j = dm->sample_count; j--;)
						*sf8_p1++ -= *sf8_p2++;
				}
				for (i = 0; i < dm->sample_count; ++i) {  // convert to si2s & interleave
					sf8_p1 = (sf8 *) dm->data + i;
					for (j = 0; j < n_channels; ++j) {
						tmp_sf8 = *sf8_p1;
						if (tmp_sf8 >= (sf8) POS_INF_SI2_m12)  // 0x7FFF
							out_arr[j] = POS_INF_SI2_m12;
						else if (tmp_sf8 <= (sf8) NEG_INF_SI2_m12)  // 0x8001
							out_arr[j] = NEG_INF_SI2_m12;
						else
							out_arr[j] = (si2) round(tmp_sf8);
						sf8_p1 += dm->sample_count;
					}
					fwrite_m12(out_arr, sizeof(si2), n_channels, out_fp, out_file, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m12);
				}
			} else {
				fwrite_m12(dm->data, sizeof(si2), n_channels * dm->sample_count, out_fp, out_file, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m12);
			}
			total_samps += dm->sample_count;
			progress = (sf8) (total_samps * 100) / goal_samps;
			printf_m12("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bProgress: %0.2lf%%   ", progress);
			fflush(stdout);
		} else {  // use_matrix == FALSE_m12
			sess = G_read_session_m12(sess, slice);
			if (sess == NULL)
				G_error_message_m12("%s(): error reading session\n", __FUNCTION__);
			
			// interleave & write out
			seg_idx = G_get_segment_index_m12(slice->start_segment_number);
			n_segs = slice->number_of_segments;
			chan = sess->time_series_channels[0];
			for (i = 0, j = seg_idx; i < n_segs; ++i, ++j) {
				seg = chan->segments[j];  // first channel
				n_samps = TIME_SLICE_SAMPLE_COUNT_S_m12(seg->time_slice);  // assuming same number of samples per segment here
				if (highpass_cutoff > (sf8) 0.0) {  // highpass filter
					for (k = 0; k < n_channels; ++k) {
						seg = sess->time_series_channels[k]->segments[j];
						cps = seg->time_series_data_fps->parameters.cps;
						CMP_si4_to_sf8_m12(cps->decompressed_data, tx, n_samps);
						FILT_moving_average_m12(tx, qx, n_samps, span, FILT_EXTRAPOLATE_m12);
						if (start_samp > slice->start_sample_number) {  // smooth tail from last window to this one
							diff = qx[0] - last_avgs[i];
							last_avgs[i] = qx[dm->sample_count - 1];
							inc = diff / (sf8) (span / 2);
							for (j = 0; j < (span / 2); ++j)
								qx[j] -= (diff -= inc);
						}
						sf8_p1 = tx;
						sf8_p2 = qx;
						for (m = n_samps; m--;)
							*sf8_p1++ -=  *sf8_p2++;
						CMP_sf8_to_si4_m12(tx, cps->decompressed_data, n_samps);
					}
				}
				for (prog_loop_ctr = k = 0; k < n_samps; ++k) {
					for (m = 0; m < n_channels; ++m) {
						seg = sess->time_series_channels[m]->segments[j];
						cps = seg->time_series_data_fps->parameters.cps;
						tmp_sf8 = (sf8) cps->decompressed_data[k] * UV_DAT_CONVERSION_FACTOR;
						if (tmp_sf8 >= (sf8) POS_INF_SI2_m12)  // 0x7FFF
							out_arr[m] = POS_INF_SI2_m12;
						else if (tmp_sf8 <= (sf8) NEG_INF_SI2_m12)  // 0x8001
							out_arr[m] = NEG_INF_SI2_m12;
						else
							out_arr[m] = (si2) round(tmp_sf8);
					}
					fwrite_m12(out_arr, sizeof(si2), n_channels, out_fp, out_file, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m12);
					if (++prog_loop_ctr == 100) {
						progress = (sf8) ((total_samps + k) * 100) / goal_samps;
						printf_m12("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bProgress: %0.2lf%%   ", progress);
						fflush(stdout);
						prog_loop_ctr = 0;
					}
				}
				total_samps += n_samps;
			}
		}
		start_samp += samps_per_min;
		end_samp += samps_per_min;
	}
	fclose(out_fp);
	printf_m12("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bProgress: 100.00%%\n\n");
	
	printf_m12("number of channels: %ld\nsampling frequency: %d\noutput samples: %ld\n\n", n_channels, (si4) out_fs, total_samps);
	
	// clean up
	G_free_session_m12(sess, TRUE_m12);
	if (dm != NULL)
		DM_free_matrix_m12(dm, TRUE_m12);
	if (out_arr != NULL)
		free_m12((void *) out_arr, __FUNCTION__);
	if (qx != NULL)
		free_m12((void *) qx, __FUNCTION__);
	if (tx != NULL)
		free_m12((void *) tx, __FUNCTION__);
	if (last_avgs != NULL)
		free_m12((void *) last_avgs, __FUNCTION__);
	G_free_globals_m12(TRUE_m12);
	
	return(0);
}
