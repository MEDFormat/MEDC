
// MED2DAT.c
// Written by Matt Stead
// Copyright Dark Horse Neuro Inc, 2020


#include "medlib_m11.h"


#ifndef MAX_CHANNELS
	#define MAX_CHANNELS	512
#endif
#define UV_DAT_CONVERSION_FACTOR	((sf8) 5.128205316306578)  // µV to DAT native amplitude
#define LS_MED2DAT_PROD_VER_MAJOR	((ui1) 1)
#define LS_MED2DAT_PROD_VER_MINOR	((ui1) 0)


int main(int argc, char *argv[])
{
	extern GLOBALS_m11		*globals_m11;
	si1				out_dir[FULL_FILE_NAME_BYTES_m11], out_file[FULL_FILE_NAME_BYTES_m11], *password;
	si2				*out_arr;
	si4				n_channels, seg_idx, list_len;
	ui8				flags;
	si8				i, j, k, m, n_samps, n_segs, progress_loops, progress_loop_ctr, curr_samp;
	sf8				progress, tmp_sf8, f_tot_samps;
	void				*file_list;
	FILE				*out_fp;
	SESSION_m11			*sess;
	CHANNEL_m11			*chan_0;
	SEGMENT_m11			*seg;
	CMP_PROCESSING_STRUCT_m11	*cps;
	TIME_SLICE_m11			slice;
	
	
	// NOTE: in MacOS & Linux change resource limits before calling any functions that use system resources (e.g. printf())
	adjust_open_file_limit_m11(MAX_OPEN_FILES_m11(MAX_CHANNELS, 1), TRUE_m11);

	// initialize library
	initialize_medlib_m11(FALSE_m11, FALSE_m11);
	
	// usage
	if (argc < 2 || argc > 9) {
		extract_path_parts_m11(argv[0], NULL, out_dir, NULL);
		printf_m11("%c\n\t%s version %hhu.%hhu\n\n\tUSAGE: %s MED_directory (multiple w/ regex) [output_directory]  [start_time] [end_time] [start_samp_num] [end_samp_num] [password] [samp_num_ref_chan]\n\n", 7, out_dir, LS_MED2DAT_PROD_VER_MAJOR, LS_MED2DAT_PROD_VER_MINOR, argv[0]);
		exit_m11(0);
	}

	// initialize time slice
	initialize_time_slice_m11(&slice);

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

	// read session
	printf_m11("\nReading MED data ...  "); fflush(stdout);
	flags = LH_SINGLE_READ_DEFAULT_m11;
	sess = read_session_m11(NULL, &slice, file_list, list_len, flags, password);
	if (sess == NULL) {
		error_message_m11("%s(): error reading session\n", __FUNCTION__);
		exit_m11(1);
	}
	if (sess->Sgmt_records[0].sampling_frequency == FREQUENCY_VARIABLE_m11) {
		error_message_m11("%s(): MED2DAT does not currently handle variable sampling frequency sesssions\n", __FUNCTION__);
		exit_m11(1);
	}
	slice = sess->time_slice;
	seg_idx = get_segment_index_m11(slice.start_segment_number);
	n_segs = slice.number_of_segments;

	// set up for write out
	sprintf_m11(out_file, "%s/%s.dat", out_dir, globals_m11->session_name);
	out_fp = fopen_m11(out_file, "w", __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	n_channels = sess->number_of_time_series_channels;
	out_arr = (si2 *) malloc_m11((size_t) n_channels * sizeof(si2), __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);
	chan_0 = sess->time_series_channels[0];
	progress_loop_ctr = progress_loops = 100;
		
	// get total samps for progress
	f_tot_samps = (sf8) TIME_SLICE_SAMPLE_COUNT_m11((&slice)) / (sf8) 100.0;
	
	// interleave channel data
	printf_m11("\n\nProgress: 0.00%%   ");  fflush(stdout);
	curr_samp = 0;
	for (i = 0, j = seg_idx; i < n_segs; ++i, ++j) {
		seg = chan_0->segments[j];
		n_samps = TIME_SLICE_SAMPLE_COUNT_m11((&seg->time_slice));  // assuming same number of samples per segment here
		for (k = 0; k < n_samps; ++k) {
			for (m = 0; m < n_channels; ++m) {
				seg = sess->time_series_channels[m]->segments[j];
				cps = seg->time_series_data_fps->parameters.cps;
				tmp_sf8 = (sf8) cps->decompressed_data[k] * UV_DAT_CONVERSION_FACTOR;
				if (tmp_sf8 > (sf8) POS_INF_SI2_m11)  // 0x7FFF
					tmp_sf8 = (sf8) POS_INF_SI2_m11;
				else if (tmp_sf8 < (sf8) NAN_SI2_m11)  // 0x8000
					tmp_sf8 = (sf8) NAN_SI2_m11;
				out_arr[m] = (si2) round(tmp_sf8);
			}
			++curr_samp;
			fwrite_m11(out_arr, sizeof(si2), n_channels, out_fp, out_file, __FUNCTION__, USE_GLOBAL_BEHAVIOR_m11);

			// progress
			if (--progress_loop_ctr)
				continue;
			progress = (sf8) curr_samp / f_tot_samps;
			printf_m11("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bProgress: %0.2lf%%   ", progress);
			fflush(stdout);
			progress_loop_ctr = progress_loops;
		}
	}
	fclose(out_fp);
	printf_m11("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bProgress: 100.00%%\n\n");
	
	// clean up
	free_session_m11(sess, TRUE_m11);
	free_m11((void *) out_arr, __FUNCTION__);
	free_globals_m11(TRUE_m11);
	
	return(0);
}
