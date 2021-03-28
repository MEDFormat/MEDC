
// MED2RAW.c
// Written by Matt Stead
// Copyright Dark Horse Neuro Inc, 2020


#include "medlib_m10.h"

#define NLX_MAX_CHANNELS        512


int main(int argc, char *argv[])
{
        extern GLOBALS_m10                      *globals_m10;
        extern si4                              errno;
        TERN_m10                                read_time_series_data, read_record_data, show_records, show_file_processing_structs;
        si1                                     **channel_list, *sess_dir;
        si1                                     out_dir[FULL_FILE_NAME_BYTES_m10], out_file[FULL_FILE_NAME_BYTES_m10], *password;
        ui4                                     type_code;
        si4                                     n_channels;
        si8                                     i, j, n_samps;
        FILE                                    *out_fp;
        SESSION_m10                             *sess;
        CHANNEL_m10                             *chan;
        SEGMENT_m10                             *seg;
        CMP_PROCESSING_STRUCT_m10               *cps;
        TIME_SLICE_m10                          slice;
        struct rlimit                           resource_limit;


        // change resource limits (note: must change before calling any functions that use system resources)
        getrlimit(RLIMIT_NOFILE, &resource_limit);
        resource_limit.rlim_cur = (rlim_t) MAX_OPEN_FILES_m10(NLX_MAX_CHANNELS, 1);
        if (setrlimit(RLIMIT_NOFILE, &resource_limit))
                error_message_m10("could not adjust process open file limit");

        // usage
        if (argc < 2 || argc > 9) {
                fprintf(stderr, "\n\tUSAGE: %s MED_directory [output_directory] [start_time] [end_time] [start_idx] [end_idx] [password] [idx_ref_chan]\n\n", argv[0]);
                return(-1);
        }
        
        // USAGE: %s MED_directory [output_directory] [start_time] [end_time] [password] [start_index] [end_index] [idx_ref_chan]
        // negative times: relative to session start
        // positive times: offset or absolute

        // initialize MED library
        // initialize_globals_m10();
	// globals_m10->verbose = TRUE_m10;
        initialize_medlib_m10();
        initialize_time_slice_m10(&slice);

        // testing
	show_records = FALSE_m10;  // TRUE_m10;
	show_file_processing_structs = FALSE_m10;  // = TRUE_m10;
        
        // input file list
        type_code = MED_type_code_from_string_m10(argv[1]);
        if (type_code == SESSION_DIRECTORY_TYPE_CODE_m10) {
                sess_dir = argv[1];
                channel_list = NULL;
                n_channels = 0;
        } else {
                sess_dir = NULL;
                channel_list = &argv[1];
                n_channels = 1;
        }
        
        // output directory
        if (argc >= 3)
                strcpy(out_dir, argv[2]);
        else
                getcwd(out_dir, FULL_FILE_NAME_BYTES_m10);
        
        // start time
	if (argc >= 4) {
                errno = 0;
                slice.start_time = (si8) strtol(argv[3], NULL, 10);
                if (errno) {
                        if ((strcmp(argv[3], "start")) == 0)
                                slice.start_time = BEGINNING_OF_TIME_m10;
			else
				slice.start_time = UUTC_NO_ENTRY_m10;
                }
        }

        // end time
        if (argc >= 5) {
                errno = 0;
                slice.end_time = (si8) strtol(argv[4], NULL, 10);
                if (errno) {
                        if ((strcmp(argv[4], "end")) == 0)
                                slice.end_time = END_OF_TIME_m10;
			else
				slice.end_time = UUTC_NO_ENTRY_m10;
                }
        }

        // start index
        if (argc >= 6) {
                errno = 0;
                slice.start_index = (si8) strtol(argv[5], NULL, 10);
                if (errno) {
                        if ((strcmp(argv[5], "start")) == 0)
                                slice.start_index = BEGINNING_OF_INDICES_m10;
                        else
				slice.start_index = SAMPLE_NUMBER_NO_ENTRY_m10;
                }
        }
        
        // end index
	if (argc >= 7) {
                errno = 0;
                slice.end_index = (si8) strtol(argv[6], NULL, 10);
                if (errno) {
                        if ((strcmp(argv[6], "end")) == 0)
                                slice.end_index = END_OF_INDICES_m10;
                        else
                                slice.end_index = SAMPLE_NUMBER_NO_ENTRY_m10;
                }
        }
	
        // password
        password = NULL;
        if (argc >= 8)
                password = argv[7];
                
        // index_reference channel
	slice.index_reference_channel_name = NULL;
	if (argc == 9)
		slice.index_reference_channel_name = argv[8];
                
        // read session
        read_time_series_data = read_record_data = TRUE_m10;
	sess = read_session_m10(sess_dir, channel_list, n_channels, &slice, password, read_time_series_data, read_record_data);
	if (sess == NULL)
		exit(-1);

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

                // show channel records
                if (chan->record_data_fps != NULL && show_records == TRUE_m10)
                        show_records_m10(chan->record_data_fps, ALL_TYPES_CODE_m10);

                // open output time series data files
                sprintf(out_file, "%s/%s.raw", out_dir, chan->name);
                out_fp = e_fopen_m10(out_file, "w", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                
                for (j = 0; j < chan->number_of_segments; ++j) {
                        
                        seg = chan->segments[j];
                        
			// show FPSs
			if (show_file_processing_structs == TRUE_m10)
                        	show_file_processing_struct_m10(seg->metadata_fps);

                        // show segment records
                        if (seg->record_data_fps != NULL && show_records == TRUE_m10)
                                show_records_m10(seg->record_data_fps, ALL_TYPES_CODE_m10);
                                                
                        cps = seg->time_series_data_fps->cps;
			n_samps = seg->time_slice.number_of_samples;
                        e_fwrite_m10(cps->decompressed_data, sizeof(si4), n_samps, out_fp, out_file, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                }
                fclose(out_fp);
        }

        // clean up
        free_session_m10(sess);
        
        return(0);
}
