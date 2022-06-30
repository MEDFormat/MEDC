
//***********************************************************************//
//******************  DARK HORSE NEURO C Library 1.0.1  *****************//
//***********************************************************************//

// Written by Matt Stead
// Copyright Dark Horse Neuro Inc, 2020

#include "dhnlib_d11.h"

// Globals
GLOBALS_d11     *globals_d11 = NULL;


//***********************************************************************//
//************************  GENERAL DHN FUNCTIONS  **********************//
//***********************************************************************//


void    add_alloc_entity_d11(void *ptr, ui8 n_bytes, const si1 *function, si4 line)
{
	ALLOC_ENTITY_d11        *ae;
	
	
	if (globals_d11->alloc_tracking == FALSE_m11) {
		warning_message_m11("%s(): Memory allocation tracking is not enabled", __FUNCTION__);
		return;
	}
	
	if (globals_d11->ae_array_len == globals_d11->ae_n_entities) {
		globals_d11->ae_array_len += GLOBALS_INIT_ALLOC_TRACKING_ARRAY_LEN_d11;
		globals_d11->alloc_entities = (ALLOC_ENTITY_d11 *) realloc((void *) globals_d11->alloc_entities, (size_t) globals_d11->ae_array_len * sizeof(ALLOC_ENTITY_d11));
		if (globals_d11->alloc_entities == NULL) {
			error_message_m11("%s(): realloc error\n", __FUNCTION__);
			return;
		}
		ae = globals_d11->alloc_entities + globals_d11->ae_n_entities;
		memset(ae, 0, GLOBALS_INIT_ALLOC_TRACKING_ARRAY_LEN_d11 * sizeof(ALLOC_ENTITY_d11));
	}
	
	ae = globals_d11->alloc_entities + globals_d11->ae_n_entities;
	ae->ptr = ptr;
	ae->n_bytes = n_bytes;
	ae->line = line;
	ae->freed = FALSE_m11;
	strncpy(ae->function, function, GLOBALS_ALLOC_TRACKING_FUNCTION_STRING_LEN_d11);
	++globals_d11->ae_n_entities;
	++globals_d11->ae_curr_allocated_entities;
	globals_d11->ae_curr_allocated_bytes += n_bytes;
	if (globals_d11->ae_curr_allocated_bytes > globals_d11->ae_max_allocated_bytes)
		globals_d11->ae_max_allocated_bytes = globals_d11->ae_curr_allocated_bytes;
	
	if (globals_d11->verbose == TRUE_m11)
		show_alloc_entity_d11(ae);
	
	return;
}


TRANSMISSION_INFO_d11	*alloc_trans_info_d11(si8 buffer_bytes, TRANSMISSION_INFO_d11 *trans_info, si1 *addr_str, si1 *port_str, ui4 ID_code, si4 timeout_seconds)
{
	
	// to allocate a fresh a TRANSMISSION_INFO_d11 structure: trans_info = alloc_trans_info_d11(buffer_bytes, NULL, addr_str, port_str, ID_code, timeout_seconds);
	// to refresh an existing TRANSMISSION_INFO_d11 structure: (void) alloc_trans_info_d11(buffer_bytes, trans_info, NULL, NULL, 0, -1);
	// (-1 == TH_SOCK_TIMEOUT_USE_EXISTING_d11)
	
	if (buffer_bytes < TH_HEADER_BYTES_d11)
		buffer_bytes = TH_HEADER_BYTES_d11;
	
	if (trans_info == NULL) {
		trans_info = (TRANSMISSION_INFO_d11 *) calloc_d11(sizeof(TRANSMISSION_INFO_d11), sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		trans_info->buffer = (ui1 *) calloc_d11((size_t) buffer_bytes, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		trans_info->buffer_bytes = buffer_bytes;
		trans_info->sock_fd = -1;
		trans_info->header = (TRANSMISSION_HEADER_d11 *) trans_info->buffer;
		
		// set defaults
		trans_info->header->version = TH_VERSION_DEFAULT_d11;
		trans_info->header->type = TH_TYPE_DEFAULT_d11;
		trans_info->header->flags = TH_FLAGS_DEFAULT;
	} else {
		if (buffer_bytes > trans_info->buffer_bytes) {
			trans_info->buffer = (ui1 *) realloc_d11((void *) trans_info->buffer, (size_t) buffer_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			trans_info->buffer_bytes = buffer_bytes;
			trans_info->header =  (TRANSMISSION_HEADER_d11 *) trans_info->buffer;
		}
	}

	// passed values supersede existing
	if (addr_str != NULL)
		strcpy(trans_info->addr_str, addr_str);
	if (port_str  != NULL)
		strcpy(trans_info->port_str, port_str);
	if (ID_code != TH_ID_CODE_NO_ENTRY_d11)
		trans_info->header->ID_code = ID_code;

	// timeout seconds		// if value is 0, socket never times out [TH_SOCK_TIMEOUT_NEVER_d11 == 0]
	if (timeout_seconds >= 0)	// negative uses existing value [TH_SOCK_TIMEOUT_USE_EXISTING_d11 == -1]
		trans_info->timeout_seconds = timeout_seconds;

	return(trans_info);
}


void	build_message_d11(MESSAGE_HEADER_d11 *msg, si1 *message_text)
{
	msg->time = (si8) time(NULL) * (si8) 1000000;
	msg->message_bytes = ((strlen(message_text) / ENCRYPTION_BLOCK_BYTES_m11) + 1) * ENCRYPTION_BLOCK_BYTES_m11;
	strncpy((si1 *) (msg + 1), message_text, msg->message_bytes);
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void	byte_to_hex_d11(ui1 byte, si1 *hex)
{
	ui1	hi_val, lo_val;
	
	
	hi_val = byte >> 4;
	if (hi_val > 9)
		hi_val += ((ui1) 'a' - 10);
	else
		hi_val += (ui1) '0';
	*hex++ = (si1) hi_val;
	
	lo_val = byte & 0x0F;
	if (lo_val > 9)
		lo_val += ((ui1) 'a' - 10);
	else
		lo_val += (ui1) '0';
	*hex = (si1) lo_val;

	return;
}


void	*calloc_d11(size_t n_members, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	extern GLOBALS_m11	*globals_m11;
	void			*ptr;
	ui8     		n_bytes;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if ((ptr = calloc(n_members, el_size)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			fprintf_m11(stderr, "%c\n\t%s() failed to allocate the requested array (%ld members of size %ld)\n", 7, __FUNCTION__, n_members, el_size);
			fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL) {
				fprintf_m11(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			}
			if (behavior_on_fail & RETURN_ON_FAIL_m11) {
				fprintf_m11(stderr, "\t=> returning NULL\n\n");
			} else if (behavior_on_fail & EXIT_ON_FAIL_m11) {
				fprintf_m11(stderr, "\t=> exiting program\n\n");
			}
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(1);
	}
	
	if (globals_d11->alloc_tracking == TRUE_m11) {
		n_bytes = n_members * el_size;
		add_alloc_entity_d11(ptr, n_bytes, function, line);
	}

	return(ptr);
}


void    **calloc_2D_d11(size_t dim1, size_t dim2, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	si8     i;
	ui1     **ptr;
	size_t  dim1_bytes, dim2_bytes, content_bytes, total_bytes;
	
	// Returns pointer to 2 dimensional zeroed array of dim1 by dim2 elements of size el_size
	// ptr[0] points to a one dimensional array of size (dim1 * dim2)
	// The whole block can be freed with free(ptr)

	dim1_bytes = dim1 * sizeof(void *);
	dim2_bytes = dim2 * el_size;
	content_bytes = dim1 * dim2_bytes;
	total_bytes = dim1_bytes + content_bytes;
	ptr = (ui1 **) malloc_d11(total_bytes, function, line, behavior_on_fail);
	ptr[0] = (ui1 *) (ptr + dim1);
	memset((void *) ptr[0], 0, content_bytes);  // remove this line to make e_malloc_2D_m11()
	
	for (i = 1; i < dim1; ++i)
		ptr[i] = ptr[i - 1] + dim2_bytes;

	return((void **) ptr);
}


TERN_m11        check_all_alignments_d11(const si1 *function, si4 line)
{
	TERN_m11        return_value;
	ui1		*bytes;
	
	
	// see if already checked
	if (globals_d11->all_structures_aligned != UNKNOWN_m11)
		return(globals_d11->all_structures_aligned);
	
	return_value = TRUE_m11;
	bytes = (ui1 *) malloc_d11(METADATA_FILE_BYTES_m11, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);  // METADATA is largest file structure
	
	// check all structures
	if ((LSc_check_license_file_entry_alignment_d11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;
	if ((check_transmission_header_alignment_d11(bytes)) == FALSE_m11)
		return_value = FALSE_m11;

	free((void *) bytes);

	if (return_value == TRUE_m11) {
		globals_d11->all_structures_aligned = TRUE_m11;
		if (globals_d11->verbose == TRUE_m11)
			message_m11("All MED Library structures are aligned");
	} else {
		error_message_m11("%s(): unaligned MED structures\n\tcalled from function %s(), line %d", __FUNCTION__, function, line);
	}
	
	return(return_value);
}


TERN_m11	check_file_system_d11(si1 *file_system_path, si4 is_cloud, ...)  // varargs: si1 *cloud_directory, si1 *cloud_service_name, si1 *cloud_utilities_directory
{
	si1		command[FULL_FILE_NAME_BYTES_m11 + 64], cloud_prefix[FULL_FILE_NAME_BYTES_m11];
	si1		full_path[FULL_FILE_NAME_BYTES_m11];
	si1		*cloud_directory, *cloud_service_name, *cloud_utilities_directory;
	si4		ret_val;
	va_list		args;
	
	
	if (file_system_path == NULL)
		file_system_path = ".";
	else if (*file_system_path == 0)
		file_system_path = ".";
	path_from_root_m11(file_system_path, full_path);
	
	// make directory if it oesn't exist
	#if defined MACOS_m11 || defined LINUX_m11
	sprintf_m11(command, "mkdir -p \"%s\"", full_path);
	#endif
	#ifdef WINDOWS_m11
	sprintf_m11(command, "mkdir \"%s\"", full_path);
	#endif
	ret_val = system_m11(command, TRUE_m11, __FUNCTION__, __LINE__, RETURN_ON_FAIL_m11 | SUPPRESS_ERROR_OUTPUT_m11);
	if (ret_val) {
		error_message_m11("%s(): Cannot create files on \"%s\"", __FUNCTION__, full_path);
		return(FALSE_m11);
	}

	// check write ability on file system
	sprintf_m11(command, "echo x > \"%s/test_file-remove_me\"", full_path);  // create non-empty file in case file system is cloud
	ret_val = system_m11(command, TRUE_m11, __FUNCTION__, __LINE__, RETURN_ON_FAIL_m11 | SUPPRESS_ERROR_OUTPUT_m11);
	if (ret_val) {
		error_message_m11("%s(): Cannot create files on \"%s\"", __FUNCTION__, full_path);
		return(FALSE_m11);
	}

	// check write ability on cloud
	if (is_cloud == TRUE_m11) {
		va_start(args, is_cloud);
		cloud_directory = va_arg(args, si1 *);
		cloud_service_name = va_arg(args, si1 *);
		cloud_utilities_directory = va_arg(args, si1 *);
		va_end(args);
		
		if (strcmp(cloud_service_name, "amazon") == 0)
			sprintf_m11(cloud_prefix, "%s/gustil ", cloud_utilities_directory);
		else if (strcmp(cloud_service_name, "google") == 0)
			sprintf_m11(cloud_prefix, "%s/aws s3 ", cloud_utilities_directory);
		
		// copy file system test file to cloud
		sprintf(command, "%scp %s/test_file-remove_me %s/test_file-remove_me", cloud_prefix, file_system_path, cloud_directory);
		system_m11(command, TRUE_m11, __FUNCTION__, __LINE__, RETURN_ON_FAIL_m11 | SUPPRESS_ERROR_OUTPUT_m11);
		if (ret_val) {
			error_message_m11("%s(): Cannot create files on \"%s\"", __FUNCTION__, cloud_directory);
			return(FALSE_m11);
		} else {
			// clean up
			sprintf(command, "%srm %s/test_file-remove_me", cloud_prefix, cloud_directory);
			system_m11(command, TRUE_m11, __FUNCTION__, __LINE__, RETURN_ON_FAIL_m11 | SUPPRESS_ERROR_OUTPUT_m11);
		}
	}
	
	// clean up
	#if defined MACOS_m11 || defined LINUX_m11
	sprintf(command, "rm \"%s/test_file-remove_me\"", full_path);
	#endif
	#ifdef WINDOWS_m11
	sprintf(command, "del \"%s/test_file-remove_me\"", full_path);
	#endif
	system_m11(command, TRUE_m11, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);

	return(TRUE_m11);
}


TERN_m11	check_internet_connection_d11(void)
{
	// no default route => no internet, so use get_lan_ipv4_address_d11().
	NETWORK_PARAMETERS_d11	np = { 0 };
	

	if (get_lan_ipv4_address_d11(&np) == NULL)
		return(FALSE_m11);

	if (*np.LAN_IPv4_address_string == 0)
		return(FALSE_m11);

	return(TRUE_m11);
}


ui4     check_spaces_d11(si1 *string)
{
	ui4     spaces;
	si1     *c;
	
	
	if (string == NULL)
		return(NO_SPACES_m11);
	if (*string == 0)
		return(NO_SPACES_m11);
	
	c = string;
	spaces = NO_SPACES_m11;
	while (*++c) {
		if (*c == 0x20) {  // space
			if (*(c - 1) == 0x5c)  // backslash
				spaces |= ESCAPED_SPACES_m11;
			else
				spaces |= UNESCAPED_SPACES_m11;
		}
	}
			    
	return(spaces);
}


TERN_m11	check_transmission_header_alignment_d11(ui1 *bytes)
{
	TRANSMISSION_HEADER_d11		*th;
	TERN_m11                	free_flag = FALSE_m11;
	
	
	// see if already checked
	if (globals_d11->transmission_header_aligned == UNKNOWN_m11)
		globals_d11->transmission_header_aligned = FALSE_m11;
	else
		return(globals_d11->transmission_header_aligned);
	
	// check overall size
	if (sizeof(TRANSMISSION_HEADER_d11) != TH_BYTES_d11)
		goto TRANSMISSION_HEADER_NOT_ALIGNED_d11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc_d11(TH_BYTES_d11, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		free_flag = TRUE_m11;
	}
	th = (TRANSMISSION_HEADER_d11 *) bytes;
	if (th->ID_string != (si1 *) (bytes + TH_ID_STRING_OFFSET_d11))
		goto TRANSMISSION_HEADER_NOT_ALIGNED_d11;
	if (&th->ID_string_terminal_zero != (si1 *) (bytes + LS_LFE_PRODUCT_STRING_TERMINAL_ZERO_OFFSET_d11))
		goto TRANSMISSION_HEADER_NOT_ALIGNED_d11;
	if (&th->ID_code != (ui4 *) (bytes + TH_ID_CODE_OFFSET_d11))
		goto TRANSMISSION_HEADER_NOT_ALIGNED_d11;
	if (&th->type != (ui1 *) (bytes + TH_TYPE_OFFSET_d11))
		goto TRANSMISSION_HEADER_NOT_ALIGNED_d11;
	if (&th->version != (ui1 *) (bytes + TH_VERSION_OFFSET_d11))
		goto TRANSMISSION_HEADER_NOT_ALIGNED_d11;
	if (&th->flags != (ui1 *) (bytes + TH_FLAGS_OFFSET_d11))
		goto TRANSMISSION_HEADER_NOT_ALIGNED_d11;
	if (&th->transmission_bytes != (si8 *) (bytes + TH_TRANSMISSION_BYTES_OFFSET_d11))
		goto TRANSMISSION_HEADER_NOT_ALIGNED_d11;

	// aligned
	globals_d11->transmission_header_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_d11->verbose == TRUE_m11)
		message_m11("TRANSMISSION_HEADER_d11 structure is aligned");
	
	return(TRUE_m11);
	
	// not aligned
TRANSMISSION_HEADER_NOT_ALIGNED_d11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_d11->verbose == TRUE_m11)
		error_message_m11("%s(): TRANSMISSION_HEADER_d11 structure is NOT aligned", __FUNCTION__);
	
	return(FALSE_m11);
}


void	close_transmission_d11(TRANSMISSION_INFO_d11 *trans_info)
{
#if defined MACOS_m11 || defined LINUX_m11
	shutdown(trans_info->sock_fd, SHUT_RDWR);
	close(trans_info->sock_fd);
#endif
#ifdef WINDOWS_m11
	shutdown(trans_info->sock_fd, SD_BOTH);
	closesocket(trans_info->sock_fd);
#endif
	trans_info->sock_fd = -1;
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     compare_sf8_d11(const void *a, const void * b)
{
	if (*((sf8 *) a) > *((sf8 *) b))
		return(1);
	else if (*((sf8 *) a) < *((sf8 *) b))
		return(-1);
	return(0);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     compare_si4_d11(const void *a, const void * b)
{
	if (*((si4 *) a) > *((si4 *) b))
		return(1);
	else if (*((si4 *) a) < *((si4 *) b))
		return(-1);
	return(0);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4     compare_si8_d11(const void *a, const void * b)
{
	if (*((si8 *) a) > *((si8 *) b))
		return(1);
	else if (*((si8 *) a) < *((si8 *) b))
		return(-1);
	return(0);
}


TERN_m11	connect_to_server_d11(TRANSMISSION_INFO_d11 *trans_info, si1 *addr_str, si1 *port_str, ui4 ID_code)
{
	si4				rv, sock_fd;
	struct addrinfo			hints, *server_info, *sa_p;
	TRANSMISSION_HEADER_d11		*header;


	if (trans_info == NULL) {
		warning_message_m11("%s(): trans_info is NULL\n", __FUNCTION__);
		return(FALSE_m11);
	}
	
	// don't leave existing socket open
	close_transmission_d11(trans_info);
			
	// passed values take precedence
	if (addr_str != NULL)
		strcpy(trans_info->addr_str, addr_str);
	if (port_str != NULL)
		strcpy(trans_info->port_str, port_str);
	header = trans_info->header;
	if (ID_code != TH_ID_CODE_NO_ENTRY_d11)
		header->ID_code = ID_code;
	
	// check we have info to connect
	if (*trans_info->addr_str == 0) {
		warning_message_m11("%s(): no address\n", __FUNCTION__);
		return(FALSE_m11);
	}
	if (*trans_info->port_str == 0) {
		warning_message_m11("%s(): no port\n", __FUNCTION__);
		return(FALSE_m11);
	}

	// connect
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(trans_info->addr_str, trans_info->port_str, &hints, &server_info)) != 0) {
#if defined MACOS_m11 || defined LINUX_m11
		warning_message_m11("%s(): getaddrinfo() error \"%s\"\n", __FUNCTION__, gai_strerror(rv));
#endif
#ifdef WINDOWS_m11
		warning_message_m11("%s(): getaddrinfo() error \"%s\"\n", __FUNCTION__, gai_strerrorA(rv));
#endif
		return(FALSE_m11);
	}
	
	// connect to the first available interface
	for (sa_p = server_info; sa_p != NULL; sa_p = sa_p->ai_next) {
		if ((sock_fd = socket(sa_p->ai_family, sa_p->ai_socktype, sa_p->ai_protocol)) == -1)
			continue;
		if (connect(sock_fd, sa_p->ai_addr, sa_p->ai_addrlen) == -1) {
#if defined MACOS_m11 || defined LINUX_m11
			close(sock_fd);
#endif
#ifdef WINDOWS_m11
			closesocket(sock_fd);
#endif
			continue;
		}
		break;
	}
	if (sa_p == NULL) {
		warning_message_m11("%s(): failed to connect\n", __FUNCTION__);
		freeaddrinfo(server_info); // finished with this structure
		return(FALSE_m11);
	}
	
	// set socket timeout
	if (trans_info->timeout_seconds) {
#if defined MACOS_m11 || defined LINUX_m11
		struct timeval          	tv;

		tv.tv_sec = trans_info->timeout_seconds; tv.tv_usec = 0;
		setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
#endif
#ifdef WINDOWS_m11
		si1	timeout_in_ms[16];
		si8 	len;

		len = sprintf_m11(timeout_in_ms, "%d", trans_info->timeout_seconds * 1000);
		setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, timeout_in_ms, len + 1);

#endif
	}

	trans_info->sock_fd = sock_fd;
	inet_ntop(sa_p->ai_family, get_in_addr_d11((struct sockaddr*)sa_p->ai_addr), trans_info->addr_str, INET6_ADDRSTRLEN);
	freeaddrinfo(server_info);  // finished with this structure

	return(TRUE_m11);
}


TERN_m11 domain_to_ip_d11(si1 *domain_name, si1 *ip)
{
	si4			rv;
	struct addrinfo		hints, *servinfo;
	struct sockaddr_in	*h;


	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(domain_name , "http" , &hints , &servinfo)) != 0) {
		error_message_m11("%s(): getaddrinfo: %s\n", gai_strerror(rv));
		return(FALSE_m11);
	}

	// connect to the first one
	if ((h = (struct sockaddr_in *) servinfo->ai_addr) == NULL) {
		*ip = 0;
		freeaddrinfo(servinfo);
		return(FALSE_m11);
	}

	strcpy(ip, inet_ntoa(h->sin_addr));
	freeaddrinfo(servinfo);  // done with this structure
	
	return(TRUE_m11);
}


si1     *duration_string_d11(si1 *dur_str, si8 i_usecs)
{
	static si1      private_dur_str[TIME_STRING_BYTES_m11];
	sf8             years, months, weeks, days, hours, mins, secs, msecs, usecs;

	
	// Note: if dur_str == NULL, this function is not thread safe
	if (dur_str == NULL)
		dur_str = private_dur_str;
	
	usecs = (sf8) i_usecs;
	
	years = usecs / (sf8) 31556926000000.0;
	if (years >= (sf8) 1.0) {
		sprintf_m11(dur_str, "%0.2lf years", years);
	} else {
		months = usecs / (sf8) 2629744000000.0;
		if (months >= (sf8) 1.0) {
			sprintf_m11(dur_str, "%0.2lf months", months);
		} else {
			weeks = usecs / (sf8) 604800000000.0;
			if (weeks >= (sf8) 1.0) {
				sprintf_m11(dur_str, "%0.2lf weeks", weeks);
			} else {
				days = usecs / (sf8) 86400000000.0;
				if (days >= (sf8) 1.0) {
					sprintf_m11(dur_str, "%0.2lf days", days);
				} else {
					hours = usecs / (sf8) 3600000000.0;
					if (hours >= (sf8) 1.0) {
						sprintf_m11(dur_str, "%0.2lf hours", hours);
					} else {
						mins = usecs / (sf8) 60000000.0;
						if (mins >= (sf8) 1.0) {
							sprintf_m11(dur_str, "%0.2lf minutes", mins);
						} else {
							secs = usecs / (sf8) 1000000.0;
							if (secs >= (sf8) 1.0) {
								sprintf_m11(dur_str, "%0.2lf seconds", secs);
							} else {
							       msecs = usecs / (sf8) 1000.0;
							       if (msecs >= (sf8) 1.0) {
								       sprintf_m11(dur_str, "%0.2lf milliseconds", msecs);
							       } else {
								       sprintf_m11(dur_str, "%0.2lf microseconds", usecs);
							       }
							}
						}
					}
				}
			}
		}
	}

	return(dur_str);
}


void    free_d11(void *ptr, const si1 *function, si4 line)
{
	si4     ret_val;
	
	
	if (ptr == NULL) {
		warning_message_m11("%s(): Attempting to free unallocated object [called from function %s(), line %d]", __FUNCTION__, function, line);
		return;
	}

	if (globals_d11->alloc_tracking == TRUE_m11) {
		ret_val = remove_alloc_entity_d11(ptr, function, line);
		if (ret_val)
			return;
	}
	
	free(ptr);
	
	return;
}


void    free_2D_d11(void **ptr, si8 dim1, const si1 *function, si4 line)
{
	si8     i;
  
	
	if (ptr == NULL)
		return;
	
	// assume allocated en bloc
	if (dim1 == 0) {
		warning_message_m11("%s(): assuming allocated en bloc", __FUNCTION__);
		free_d11(ptr, function, line);
		return;
	}
	
	// allocated en bloc
	if ((ui8) ptr[0] == ((ui8) ptr + ((ui8) dim1 * (ui8) sizeof(void *)))) {
		free_d11(ptr, function, line);
		return;
	}

	// separately allocated
	for (i = 0; i < dim1; ++i) {
		if (ptr[i] == NULL)
			continue;
		free_d11(ptr[i], function, line);
	}
	free_d11(ptr, function, line);
		
	return;
}


void	*malloc_d11(size_t n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	extern GLOBALS_m11	*globals_m11;
	void			*ptr;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if ((ptr = malloc(n_bytes)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			fprintf_m11(stderr, "%c\n\t%s() failed to allocate the requested array (%ld bytes)\n", 7, __FUNCTION__, n_bytes);
			fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				fprintf_m11(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> returning NULL\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(1);
	}

	if (globals_d11->alloc_tracking == TRUE_m11)
		add_alloc_entity_d11(ptr, n_bytes, function, line);
	
	return(ptr);
}


si4     mlock_d11(void *ptr, size_t n_bytes, TERN_m11 zero_data, const si1 *function, si4 line, ui4 behavior_on_fail)
{
#if defined MACOS_m11 || defined LINUX_m11
	extern GLOBALS_m11	*globals_m11;

	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if ((mlock(ptr, n_bytes)) == -1) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			fprintf_m11(stderr, "%c\n\t%s() failed to lock the requested array (%ld bytes)\n", 7, __FUNCTION__, n_bytes);
			fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				fprintf_m11(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> returning FALSE\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(-1);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(1);
	}
	if (zero_data == TRUE_m11)
		memset(ptr, 0, n_bytes);  // forces OS to give real memory before return (otherwise there may be a lag)
#endif

	return(0);
}


si4     munlock_d11(void *ptr, size_t n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail)
{
#if defined MACOS_m11 || defined LINUX_m11
	extern GLOBALS_m11	*globals_m11;

	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if ((munlock(ptr, n_bytes)) == -1) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			fprintf_m11(stderr, "%c\n\t%s() failed to unlock the requested array (%ld bytes)\n", 7, __FUNCTION__, n_bytes);
			fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				fprintf_m11(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> returning FALSE\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(-1);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(1);
	}
#endif

	return(0);
}


void	*realloc_d11(void *orig_ptr, size_t n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	extern GLOBALS_m11	*globals_m11;
	void    		*ptr;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	if ((ptr = realloc(orig_ptr, n_bytes)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11)) {
			fprintf_m11(stderr, "%c\n\t%s() failed to reallocate the requested array (%ld bytes)\n", 7, __FUNCTION__, n_bytes);
			fprintf_m11(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				fprintf_m11(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> returning unrealloacvted pointer\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m11)
				fprintf_m11(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m11)
			return(orig_ptr);
		else if (behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(1);
	}
	
	if (globals_d11->alloc_tracking == TRUE_m11) {
		remove_alloc_entity_d11(orig_ptr, function, line);
		add_alloc_entity_d11(ptr, n_bytes, function, line);
	}
	
	return(ptr);
}


void    **realloc_2D_d11(void **curr_ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	extern GLOBALS_m11	*globals_m11;
	si8             	i;
	void            	**new_ptr;
	size_t          	least_dim1, least_dim2;
	

	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)
		behavior_on_fail = globals_m11->behavior_on_fail;
	
	// Returns pointer to a reallocated 2 dimensional array of new_dim1 by new_dim2 elements of size el_size (new unused elements are zeroed)
	// ptr[0] points to a one dimensional array of size (dim1 * dim2)
	// The whole block can be freed with free(ptr)
	if (curr_ptr == NULL) {
		error_message_m11("%s(): attempting to re-allocate NULL pointer (called from function %s(), line %d)\n", __FUNCTION__, function, line);
		return(NULL);
	}

	if (new_dim1 < curr_dim1)
		warning_message_m11("%s(): re-allocating first dimension to smaller size (called from function %s(), line %d)\n", __FUNCTION__, function, line);
	if (new_dim2 < curr_dim2)
		warning_message_m11("%s(): re-allocating second dimension to smaller size (called from function %s(), line %d)\n", __FUNCTION__, function, line);
	
	new_ptr = calloc_2D_d11(new_dim1, new_dim2, el_size, function, line, behavior_on_fail);
	
	least_dim1 = (curr_dim1 <= new_dim1) ? curr_dim1 : new_dim1;
	least_dim2 = (curr_dim2 <= new_dim2) ? curr_dim2 : new_dim2;
	for (i = 0; i < least_dim1; ++i)
		memcpy((void *) new_ptr[i], curr_ptr[i], (size_t) (least_dim2 * el_size));
	
	free_2D_d11(curr_ptr, curr_dim1, function, line);

	return((void **) new_ptr);
}



//***********************************************************************//
//**************  END ERROR CHECKING STANDARD FUNCTIONS  ****************//
//***********************************************************************//


#if defined MACOS_m11 || defined LINUX_m11
TERN_m11	enter_ascii_password_d11(si1 *password, si1 *prompt, TERN_m11 create_password)
{
	si1		pw_copy[MAX_ASCII_PASSWORD_STRING_BYTES_m11], dc[8];
	const si4	MAX_ATTEMPTS = 3;
	si4		i, c, attempts;
	struct termios	term, saved_term;
	
	
	if (password == NULL) {
		warning_message_m11("password is NULL\n");
		return(FALSE_m11);
	}
	
	if (prompt == NULL)
		prompt = "Enter Password";
	else if (*prompt == 0)
		prompt = "Enter Password";
	
	// get settings of STDIN_FILENO and copy for resetting
	tcgetattr(STDIN_FILENO, &term);
	saved_term = term;
	
	// set the approriate bit in the termios struct (displays "key" character)
	term.c_lflag &= ~(ECHO);
	
	// set the new bits
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
	
ENTER_ASCII_PASSWORD_RETRY_1_d11:
	
	printf_m11("%s: ", prompt);
	
	// read password from the console
	i = 0;
	while ((c = getchar())!= '\n' && c != EOF && i < MAX_PASSWORD_CHARACTERS_m11){
		// check that it is acceptable character
		if (c < 33 || c > 126 || c == '\"' || c == '\'') {
			putchar_m11(7);  // beep
			continue;
		}
		password[i++] = c;
	}
	password[i] = 0;
	putchar_m11('\n');
	if (*password == 0) {
		printf_m11("\tIs %s<no entry>%s correct (y/n): ", TC_RED_m11, TC_RESET_m11);
		fflush(stdout);
		*dc = 0;
		scanf("%[^\n]", dc);
		getchar();  // clear '\n' from stdin
		putchar_m11(*dc);
		if (*dc == 'y' || *dc == 'Y') {
			putchar_m11('\n');
			tcsetattr(STDIN_FILENO, TCSANOW, &saved_term);
			return(TRUE_m11);  // user intends no entry so return TRUE. Calling function should decide what to do with no password.
		} else {
			putchar_m11('\n');
			goto ENTER_ASCII_PASSWORD_RETRY_1_d11;
		}
	}
	
	// confirm
	attempts = 0;
	if (create_password == TRUE_m11) {
		
	ENTER_ASCII_PASSWORD_RETRY_2_d11:
		
		printf_m11("Re-enter password: ");
		i = 0;
		while ((c = getchar())!= '\n' && c != EOF && i < MAX_PASSWORD_CHARACTERS_m11){
			// check that it is acceptable character
			if (c < 33 || c > 126 || c == '\"' || c == '\'') {
				putchar_m11(7);  // beep
				continue;
			}
			pw_copy[i++] = c;
		}
		pw_copy[i] = 0;
		putchar_m11('\n');
		if (strcmp(password, pw_copy)) {
			if (++attempts == MAX_ATTEMPTS) {
				printf_m11("%sPasswords do not match. Maximum attempts made.\n%s", TC_RED_m11, TC_RESET_m11);
				tcsetattr(STDIN_FILENO, TCSANOW, &saved_term);
				return(FALSE_m11);
			}
			printf_m11("%sPasswords do not match. Try again.\n%s", TC_RED_m11, TC_RESET_m11);
			goto ENTER_ASCII_PASSWORD_RETRY_2_d11;
		}
	}
	
	// reset terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &saved_term);
	
	return(TRUE_m11);
}
#endif  // MACOS_m11 || LINUX_m11


#ifdef WINDOWS_m11
TERN_m11	enter_ascii_password_d11(si1* password, si1* prompt, TERN_m11 create_password)
{
	si1		pw_copy[MAX_ASCII_PASSWORD_STRING_BYTES_m11];
	const si4	MAX_ATTEMPTS = 3;
	si4		i, c, dc, attempts;


	if (password == NULL) {
		warning_message_m11("password is NULL\n");
		return(FALSE_m11);
	}

	if (prompt == NULL)
		prompt = "Enter Password";
	else if (*prompt == 0)
		prompt = "Enter Password";

ENTER_ASCII_PASSWORD_RETRY_1_d11:

	printf_m11("%s: ", prompt);

	// read password from the console
	*password = 0;
	i = 0;
	while (i < MAX_PASSWORD_CHARACTERS_m11) {
		c = _getch();
		// carriage return or CTRL-C (finished)
		if (c == '\r' || c == 3) {
			putch_m11('\r'); putch_m11('\n');
			password[i] = 0;
			break;
		}
		// backspace
		if (c == '\b') {
			if (i) {
				putch_m11('\b'); putch_m11(' '); putch_m11('\b');
				password[--i] = 0;
				continue;
			}
			// else fall through to bad character
		}
		// check that it is acceptable character
		if (c < 33 || c > 126 || c == '\"' || c == '\'') {
			putch_m11(7);  // beep
			continue;
		}
		putch_m11(c);
		Sleep(300);
		putch_m11('\b'); putch_m11('*');
		password[i++] = c;
	}
	
	if (*password == 0) {
		printf_m11("\tIs %s<no entry>%s correct (y/n): ", TC_RED_m11, TC_RESET_m11);
		fflush(stdout);
		dc = _getch();
		putch_m11(dc); putch_m11('\r'); putch_m11('\n');
		if (dc == 'y' || dc == 'Y')
			return(TRUE_m11);  // user intends no entry so return TRUE. Calling function should decide what to do with no password.
		else
			goto ENTER_ASCII_PASSWORD_RETRY_1_d11;
	}

	// confirm
	attempts = 0;
	if (create_password == TRUE_m11) {

	ENTER_ASCII_PASSWORD_RETRY_2_d11:

		printf_m11("Re-enter password: ");
		i = 0;

		pw_copy[0] = 0;
		i = 0;
		while (i < MAX_PASSWORD_CHARACTERS_m11) {
			c = _getch();
			// carriage return or CTRL-C (finished)
			if (c == '\r' || c == 3) {
				putch_m11('\r'); putch_m11('\n');
				pw_copy[i] = 0;
				break;
			}
			// backspace
			if (c == '\b') {
				if (i) {
					putch_m11('\b'); putch_m11(' '); putch_m11('\b');
					password[--i] = 0;
					continue;
				}  
				// else fall through to bad character
			}
			// check that it is acceptable character
			if (c < 33 || c > 126 || c == '\"' || c == '\'') {
				putch_m11(7);  // beep
				continue;
			}
			putch_m11(c);
			Sleep(300);
			putch_m11('\b'); putch_m11('*');
			pw_copy[i++] = c;
		}

		if (strcmp(password, pw_copy)) {
			if (++attempts == MAX_ATTEMPTS) {
				printf_m11("%sPasswords do not match. Maximum attempts made.\n%s", TC_RED_m11, TC_RESET_m11);
				return(FALSE_m11);
			}
			printf_m11("%sPasswords do not match. Try again.\n%s", TC_RED_m11, TC_RESET_m11);
			goto ENTER_ASCII_PASSWORD_RETRY_2_d11;
		}
	}

	return(TRUE_m11);
}
#endif  // WINDOWS_m11


void	fill_empty_password_bytes_d11(si1 *password_bytes)
{
	ui4	m_w, m_z;
	si4	i;
	
	
	// initialize random number generator
	m_w = m_z = 0;
	for (i = 0; i < PASSWORD_BYTES_m11; ++i) {
		if (password_bytes[i] == 0)
			break;
		m_w += password_bytes[i];
		m_z -= password_bytes[i];
	}
	if (m_w == 0 || m_w == 0x464FFFFF)
		m_w = 0x01020304;
	if (m_z == 0 || m_z == 0x9068FFFF)
		m_z = 0x05060708;
	
	// fill in bytes
	for (; i < PASSWORD_BYTES_m11; ++i)
		password_bytes[i] = random_byte_d11(&m_w, &m_z);
  
	return;
}



//***********************************************************************//
//*************************  LOSSY COMPRESSION  *************************//
//***********************************************************************//


void	CMP_binterpolate_sf8_d11(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len, ui4 center_mode, TERN_m11 extrema, sf8 *minima, sf8 *maxima)
{
	si8	i, j, max_bin_width;
	si8	i_bin_width, i_bin_start, i_bin_end;
	sf8	*in_val, *bin_start_p, f_bin_width, f_bin_end, bin_min, bin_max, bin_sum, *quantile_buf, us_ratio;
	
	
	if (in_len <= 1) {
		if (in_len == 0)
			return;
		for (i = 0; i < out_len; ++i)
			out_data[i] = in_data[0];
		if (extrema == TRUE_m11)
			minima[i] = maxima[i] = in_data[0];
		return;
	}
	if (in_len == out_len) {
		memcpy(out_data, in_data, in_len << 3);
		if (extrema == TRUE_m11) {
			memcpy(minima, in_data, in_len << 3);
			memcpy(maxima, in_data, in_len << 3);
		}
		return;
	}
	
	switch (center_mode) {
		case	CMP_CENT_MODE_NONE_d11:
			if (extrema == FALSE_m11)  // no binterpolation requested: interpolate
				return;
		case	CMP_CENT_MODE_MIDPOINT_d11:
		case	CMP_CENT_MODE_MEAN_d11:
		case	CMP_CENT_MODE_MEDIAN_d11:
			break;
		case	CMP_CENT_MODE_FASTEST_d11:
			if (extrema == TRUE_m11)
				center_mode = CMP_CENT_MODE_MIDPOINT_d11;
			else
				center_mode = CMP_CENT_MODE_MEAN_d11;
			break;
		default:
			warning_message_m11("%s(): invalid center mode\n", __FUNCTION__);
			return;
	}
	
	// upsample
	if (in_len <= out_len) {
		us_ratio = (sf8) out_len / (sf8) in_len;
		if (center_mode == CMP_CENT_MODE_NONE_d11)
			out_data = (sf8 *) malloc((size_t) (in_len << 3));
		if (us_ratio >= CMP_SPLINE_UPSAMPLE_SF_RATIO_m11)
			CMP_spline_interp_sf8_m11(in_data, in_len, out_data, out_len, NULL);
		else
			CMP_lin_interp_sf8_m11(in_data, in_len, out_data, out_len);
		if (extrema == TRUE_m11) {
			--out_len;
			f_bin_width = (sf8) out_len / (sf8) in_len;
			f_bin_end = f_bin_width / (sf8) 2.0;
			i_bin_end = (si8) (f_bin_end + 0.5);
			in_val = out_data;
			// initial half-bin
			bin_min = bin_max = *in_val++;
			for (j = i_bin_end; --j; ++in_val) {
				if (*in_val < bin_min)
					bin_min = *in_val;
				else if (*in_val > bin_max)
					bin_max = *in_val;
			}
			for (j = 0; j < i_bin_end; ++j) {
				minima[j] = bin_min;
				maxima[j] = bin_max;
			}
			// central bins
			for (i = 1; i < out_len; ++i) {
				i_bin_start = i_bin_end;
				f_bin_end += f_bin_width;
				i_bin_end = (si8) (f_bin_end + 0.5);
				i_bin_width = i_bin_end - i_bin_start;
				bin_min = bin_max = *in_val++;
				for (j = i_bin_width; --j; ++in_val) {
					if (*in_val < bin_min)
						bin_min = *in_val;
					else if (*in_val > bin_max)
						bin_max = *in_val;
				}
				for (j = i_bin_start; j < i_bin_end; ++j) {
					minima[j] = bin_min;
					maxima[j] = bin_max;
				}
			}
			// terminal half-bin
			i_bin_start = i_bin_end;
			i_bin_end = out_len;
			i_bin_width = i_bin_end - i_bin_start;
			bin_min = bin_max = *in_val++;
			for (j = i_bin_width; --j; ++in_val) {
				if (*in_val < bin_min)
					bin_min = *in_val;
				else if (*in_val > bin_max)
					bin_max = *in_val;
			}
			for (j = i_bin_start; j <= i_bin_end; ++j) {
				minima[j] = bin_min;
				maxima[j] = bin_max;
			}
		}
		if (center_mode == CMP_CENT_MODE_NONE_d11)
			free((void *) out_data);
		return;
	}

	// downsample
	--out_len;
	f_bin_width = (sf8) in_len / (sf8) out_len;
	f_bin_end = f_bin_width / (sf8) 2.0;
	i_bin_end = (si8) (f_bin_end + 0.5);
	in_val = in_data;
	if (center_mode == CMP_CENT_MODE_MEDIAN_d11) {
		max_bin_width = (si8) ceil(f_bin_width);
		quantile_buf = (sf8 *) malloc((size_t) (max_bin_width << 3));
	}
	
	// initial half-bin (no central tendency measure - could skew)
	if (center_mode != CMP_CENT_MODE_NONE_d11)
		out_data[0] = in_data[0];
	if (extrema == TRUE_m11) {
		bin_min = bin_max = *in_val++;
		for (j = i_bin_end; --j; ++in_val) {
			if (*in_val < bin_min)
				bin_min = *in_val;
			else if (*in_val > bin_max)
				bin_max = *in_val;
		}
		minima[0] = bin_min;
		maxima[0] = bin_max;
	} else {
		in_val += i_bin_end;
	}
	
	// central bins
	// Note: every combination is done to avoid unnessary computation where possible
	if (extrema == TRUE_m11) {
		switch (center_mode) {
			case CMP_CENT_MODE_NONE_d11:
				for (i = 1; i < out_len; ++i) {
					i_bin_start = i_bin_end;
					f_bin_end += f_bin_width;
					i_bin_end = (si8) (f_bin_end + 0.5);
					i_bin_width = i_bin_end - i_bin_start;
					bin_min = bin_max = *in_val++;
					for (j = i_bin_width; --j; ++in_val) {
						if (*in_val < bin_min)
							bin_min = *in_val;
						else if (*in_val > bin_max)
							bin_max = *in_val;
					}
					minima[i] = bin_min;
					maxima[i] = bin_max;
				}
				break;
			case CMP_CENT_MODE_MIDPOINT_d11:
				for (i = 1; i < out_len; ++i) {
					i_bin_start = i_bin_end;
					f_bin_end += f_bin_width;
					i_bin_end = (si8) (f_bin_end + 0.5);
					i_bin_width = i_bin_end - i_bin_start;
					bin_min = bin_max = *in_val++;
					for (j = i_bin_width; --j; ++in_val) {
						if (*in_val < bin_min)
							bin_min = *in_val;
						else if (*in_val > bin_max)
							bin_max = *in_val;
					}
					minima[i] = bin_min;
					maxima[i] = bin_max;
					out_data[i] = (bin_min + bin_max) / (sf8) 2.0;
				}
				break;
			case	CMP_CENT_MODE_MEAN_d11:
				for (i = 1; i < out_len; ++i) {
					i_bin_start = i_bin_end;
					f_bin_end += f_bin_width;
					i_bin_end = (si8) (f_bin_end + 0.5);
					i_bin_width = i_bin_end - i_bin_start;
					bin_sum = bin_min = bin_max = *in_val++;
					for (j = i_bin_width; --j; ++in_val) {
						bin_sum += *in_val;
						if (*in_val < bin_min)
							bin_min = *in_val;
						else if (*in_val > bin_max)
							bin_max = *in_val;
					}
					minima[i] = bin_min;
					maxima[i] = bin_max;
					out_data[i] = bin_sum / (sf8) i_bin_width;
				}
				break;
			case	CMP_CENT_MODE_MEDIAN_d11:
				for (i = 1; i < out_len; ++i) {
					i_bin_start = i_bin_end;
					f_bin_end += f_bin_width;
					i_bin_end = (si8) (f_bin_end + 0.5);
					i_bin_width = i_bin_end - i_bin_start;
					bin_start_p = in_val;
					bin_sum = bin_min = bin_max = *in_val++;
					for (j = i_bin_width; --j; ++in_val) {
						bin_sum += *in_val;
						if (*in_val < bin_min)
							bin_min = *in_val;
						else if (*in_val > bin_max)
							bin_max = *in_val;
					}
					minima[i] = bin_min;
					maxima[i] = bin_max;
					out_data[i] = CMP_quantval_d11(bin_start_p, i_bin_width, (sf8) 0.5, TRUE_m11, quantile_buf);
				}
				break;
		}
	} else {  // extrema == FALSE_m11
		switch (center_mode) {
			case CMP_CENT_MODE_MIDPOINT_d11:
				for (i = 1; i < out_len; ++i) {
					i_bin_start = i_bin_end;
					f_bin_end += f_bin_width;
					i_bin_end = (si8) (f_bin_end + 0.5);
					i_bin_width = i_bin_end - i_bin_start;
					bin_min = bin_max = *in_val++;
					for (j = i_bin_width; --j; ++in_val) {
						if (*in_val < bin_min)
							bin_min = *in_val;
						else if (*in_val > bin_max)
							bin_max = *in_val;
					}
					out_data[i] = (bin_min + bin_max) / (sf8) 2.0;
				}
				break;
			case	CMP_CENT_MODE_MEAN_d11:
				for (i = 1; i < out_len; ++i) {
					i_bin_start = i_bin_end;
					f_bin_end += f_bin_width;
					i_bin_end = (si8) (f_bin_end + 0.5);
					i_bin_width = i_bin_end - i_bin_start;
					bin_sum = *in_val++;
					for (j = i_bin_width; --j; ++in_val)
						bin_sum += *in_val;
					out_data[i] = bin_sum / (sf8) i_bin_width;
				}
				break;
			case	CMP_CENT_MODE_MEDIAN_d11:
				for (i = 1; i < out_len; ++i) {
					i_bin_start = i_bin_end;
					f_bin_end += f_bin_width;
					i_bin_end = (si8) (f_bin_end + 0.5);
					i_bin_width = i_bin_end - i_bin_start;
					out_data[i] = CMP_quantval_d11(in_val, i_bin_width, (sf8) 0.5, TRUE_m11, quantile_buf);
					in_val += i_bin_width;
				}
				break;
		}
	}
		
	// terminal half-bin (no central tendency measure - could skew)
	if (center_mode != CMP_CENT_MODE_NONE_d11)
		out_data[out_len] = in_data[in_len - 1];
	if (extrema == TRUE_m11) {
		i_bin_start = i_bin_end;
		i_bin_end = in_len;
		i_bin_width = i_bin_end - i_bin_start;
		bin_min = bin_max = *in_val++;
		for (j = i_bin_width; --j; ++in_val) {
			if (*in_val < bin_min)
				bin_min = *in_val;
			else if (*in_val > bin_max)
				bin_max = *in_val;
		}
		minima[out_len] = bin_min;
		maxima[out_len] = bin_max;
	}
	
	if (center_mode == CMP_CENT_MODE_MEDIAN_d11)
		free((void *) quantile_buf);

	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
sf8      CMP_calculate_mean_residual_ratio_d11(si4 *original_data, si4 *lossy_data, ui4 n_samps)
{
	sf8        sum, mrr, diff, r;
	si8        i;
	
	
	sum = (sf8) 0.0;
	for (i = n_samps; i--;) {
		if (*original_data) {
			diff = (sf8) (*original_data - *lossy_data++);
			r = diff / (sf8) *original_data++;
			sum += ABS_m11(r);
		} else {
			--n_samps;
			++original_data;
			++lossy_data;
		}
	}
	
	if (sum == (sf8) 0.0)
		mrr = (sf8) 0.0;
	else
		mrr = sum / (sf8) n_samps;
	
	return(mrr);
}


void    CMP_detrend_d11(si4 *input_buffer, si4 *output_buffer, si8 len, CMP_PROCESSING_STRUCT_m11 *cps)
{
	si4	*si4_p1, *si4_p2;
	sf4	sf4_m;
	si4	si4_b;
	sf8	m, b, mx_plus_b;
	
	
	// detrend from input_buffer to output_buffer
	// slope and intercept values entered into block_header
	// if input_buffer == output_buffer detrending will be done in place
	// if cps != NULL store coefficients in block parameters
	
	CMP_lad_reg_si4_d11(input_buffer, len, &m, &b);
	
	// store m & b in block parameter region
	// NOTE: block parameter region must be setup first
	if (cps != NULL) {
		// demote precision
		sf4_m = (sf4) m;
		si4_b = CMP_round_si4_m11(b);  // this is an integer because sf4 can only precisely encode offsets up to 24 bits, but MED guarantees 32-bit lossless detrending
		// store the values
		*((sf4 *) cps->block_parameters + cps->parameters.block_parameter_map[CMP_PF_GRADIENT_IDX_m11]) = sf4_m;
		*((si4 *) cps->block_parameters + cps->parameters.block_parameter_map[CMP_PF_INTERCEPT_IDX_m11]) = si4_b;
		// promote back to sf8, maintaining demoted precision
		m = (sf8) sf4_m;
		b = (sf8) si4_b;
	}

	// subtract trend from input_buffer to output_buffer
	mx_plus_b = b;
	si4_p1 = input_buffer;
	si4_p2 = output_buffer;
	while (len--)
		*si4_p2++ = CMP_round_si4_m11((sf8) *si4_p1++ - (mx_plus_b += m));

	return;
}

					   
void    CMP_encode_d11(FILE_PROCESSING_STRUCT_m11 *fps, si8 start_time, si4 acquisition_channel_number, ui4 number_of_samples)
{
	TERN_m11                        data_is_compressed, allow_lossy_compression;
	ui1				normality;
	void                            (*compression_f)(CMP_PROCESSING_STRUCT_m11 * cps);
	CMP_PROCESSING_STRUCT_m11	*cps;
	CMP_BLOCK_FIXED_HEADER_m11	*block_header;
	
	
	if (fps->universal_header->type_code != TIME_SERIES_DATA_FILE_TYPE_CODE_m11) {
		error_message_m11("%s(): FPS must be time series data\n", __FUNCTION__);
		return;
	}
	cps = fps->parameters.cps;
	CMP_cps_mutex_on_m11(cps);
	block_header = cps->block_header;
	if (cps->parameters.allocated_block_samples < block_header->number_of_samples) {
		if (CMP_reallocate_processing_struct_m11(fps, CMP_COMPRESSION_MODE_m11, (si8) number_of_samples, number_of_samples) == NULL) {
			error_message_m11("%s(): reallocation error\n", __FUNCTION__);
			CMP_cps_mutex_off_m11(cps);
			return;
		}
		block_header = cps->block_header;
	}

	// calling function must set cps->input_buffer to use anything other than cps->original_ptr
	if (cps->input_buffer == NULL) {
		if (cps->original_ptr == NULL)
			error_message_m11("%s(): input buffer is NULL\n", __FUNCTION__);
		else
			cps->input_buffer = cps->original_ptr;
	}
	
	// fill in passed header fields
	block_header->block_start_UID = CMP_BLOCK_START_UID_m11;
	block_header->start_time = start_time;
	block_header->acquisition_channel_number = acquisition_channel_number;
	block_header->number_of_samples = number_of_samples;
	
	// reset block flags
	block_header->block_flags = 0;
	
	// set up variable region
	CMP_set_variable_region_m11(cps);
	
	// discontinuity
	if (cps->parameters.discontinuity == TRUE_m11) {
		block_header->block_flags |= CMP_BF_DISCONTINUITY_MASK_m11;
		if (cps->directives.reset_discontinuity == TRUE_m11)
			cps->parameters.discontinuity = FALSE_m11;
	}
		
	// select compression
	// (compression algorithms are responsible for filling in: total_header_bytes, total_block_bytes, model_region_bytes, and the model details)
	block_header->block_flags &= ~CMP_BF_ALGORITHMS_MASK_m11;
	switch (cps->directives.algorithm) {
		case CMP_RED_COMPRESSION_m11:
			block_header->block_flags |= CMP_BF_RED_ENCODING_MASK_m11;
			compression_f = CMP_RED_encode_m11;
			break;
		case CMP_PRED_COMPRESSION_m11:
			block_header->block_flags |= CMP_BF_PRED_ENCODING_MASK_m11;
			compression_f = CMP_PRED_encode_m11;
			break;
		case CMP_MBE_COMPRESSION_m11:
			block_header->block_flags |= CMP_BF_MBE_ENCODING_MASK_m11;
			CMP_find_extrema_m11(NULL, 0, NULL, NULL, cps);
			compression_f = CMP_MBE_encode_m11;
			break;
		case CMP_VDS_COMPRESSION_m11:
			block_header->block_flags |= CMP_BF_VDS_ENCODING_MASK_m11;
			compression_f = CMP_VDS_encode_d11;
			break;
		default:
			error_message_m11("%s(): unrecognized compression algorithm (%u)\n", __FUNCTION__, cps->directives.algorithm);
			CMP_cps_mutex_off_m11(cps);
			return;
	}
	
	// detrend
	if (cps->directives.detrend_data == TRUE_m11) {
		CMP_detrend_d11(cps->input_buffer, cps->parameters.detrended_buffer, block_header->number_of_samples, cps);
		cps->input_buffer = cps->parameters.detrended_buffer;
	}
	
	// lossy compression
	data_is_compressed = FALSE_m11;
	if (compression_f != CMP_VDS_encode_d11) {
		allow_lossy_compression = TRUE_m11;
		if (cps->directives.require_normality == TRUE_m11) {
			normality = CMP_normality_score_d11(cps->input_buffer, block_header->number_of_samples);
			if (normality < cps->parameters.minimum_normality) {
				allow_lossy_compression = FALSE_m11;
				block_header->parameter_flags &= ~(CMP_PF_AMPLITUDE_SCALE_MASK_m11 | CMP_PF_FREQUENCY_SCALE_MASK_m11);
			}
		}
		if (allow_lossy_compression == TRUE_m11) {
			if (cps->directives.set_amplitude_scale == TRUE_m11 || cps->directives.find_amplitude_scale == TRUE_m11) {
				if (cps->directives.find_amplitude_scale == TRUE_m11)
					data_is_compressed = CMP_find_amplitude_scale_d11(cps, compression_f);
				else if (cps->directives.set_amplitude_scale == TRUE_m11)
					CMP_scale_amplitude_si4_m11(cps->input_buffer, cps->parameters.scaled_amplitude_buffer, block_header->number_of_samples, (sf8) cps->parameters.amplitude_scale, cps);
				cps->input_buffer = cps->parameters.scaled_amplitude_buffer;
			}
			if (cps->directives.set_frequency_scale == TRUE_m11 || cps->directives.find_frequency_scale == TRUE_m11) {
				if (cps->directives.find_frequency_scale == TRUE_m11)
					data_is_compressed = CMP_find_frequency_scale_d11(cps, compression_f);
				else if (cps->directives.set_frequency_scale == TRUE_m11)
					CMP_scale_frequency_si4_m11(cps->input_buffer, cps->parameters.scaled_frequency_buffer, block_header->number_of_samples, (sf8)cps->parameters.frequency_scale, cps);
				cps->input_buffer = cps->parameters.scaled_frequency_buffer;
			}
		}
	}
	
	// noise scores
	if (cps->directives.include_noise_scores == TRUE_m11) {
		// code not written yet
	}
	
	// compress
	if (data_is_compressed == FALSE_m11)
		(*compression_f)(cps);

	// encryption done in write_file_m11()
	// if done here, leave_decrypted FPS directive won't work
	// call would be: CMP_encrypt_m11(cps);

	// reset input_buffer (because this can be changed by internal library functions)
	cps->input_buffer = NULL;
	
	CMP_cps_mutex_off_m11(cps);
	
	return;
}


TERN_m11    CMP_find_amplitude_scale_d11(CMP_PROCESSING_STRUCT_m11 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m11 *cps))
{
	TERN_m11                     	data_is_compressed;
	si8                     	i;
	si4				*input_buffer;
	sf8                     	original_size, goal_compression_ratio;
	sf8                     	low_sf, high_sf, mrr, mrr2, mrr5, sf_per_mrr;
	sf8                     	goal_low_bound, goal_high_bound, goal_mrr, goal_tol;
	sf4                     	new_scale_factor;
	CMP_BLOCK_FIXED_HEADER_m11	*block_header;
	
	
	input_buffer = cps->input_buffer;
	block_header = cps->block_header;
	data_is_compressed = FALSE_m11;

	if (cps->directives.use_compression_ratio == TRUE_m11) {
		goal_compression_ratio = cps->parameters.goal_ratio;
		goal_low_bound = goal_compression_ratio - cps->parameters.goal_tolerance;
		goal_high_bound = goal_compression_ratio + cps->parameters.goal_tolerance;
		cps->parameters.amplitude_scale = (sf4) 1.0;
		(*compression_f)(cps);
		data_is_compressed = TRUE_m11;
		original_size = (sf8) block_header->number_of_samples * (sf8) sizeof(si4);
		cps->parameters.actual_ratio = (sf8) block_header->total_block_bytes / original_size;
		if (cps->parameters.actual_ratio > goal_high_bound) {
			// loop until acceptable scale factor found
			for (i = cps->parameters.maximum_goal_attempts; i--;) {
				new_scale_factor = cps->parameters.amplitude_scale * (sf4) (cps->parameters.actual_ratio / goal_compression_ratio);
				if ((ABS_m11(new_scale_factor - cps->parameters.amplitude_scale) <= (sf4) 0.000001) || (new_scale_factor <= (sf4) 1.0))
					break;
				cps->parameters.amplitude_scale = new_scale_factor;
				(*compression_f)(cps);  // compress
				cps->parameters.actual_ratio = (sf8) block_header->total_block_bytes / original_size;
				if ((cps->parameters.actual_ratio <= goal_high_bound) && (cps->parameters.actual_ratio >= goal_low_bound))
					break;
			}
		}
	}
	else if (cps->directives.use_mean_residual_ratio == TRUE_m11) {
		// get residual ratio at sf 2 & 5 (roughly linear relationship: reasonable sample points)
		cps->parameters.amplitude_scale = (sf4) 2.0;
		CMP_generate_lossy_data_d11(cps, input_buffer, cps->decompressed_ptr, CMP_AMPLITUDE_SCALE_MODE_m11);
		mrr2 = CMP_calculate_mean_residual_ratio_d11(input_buffer, cps->decompressed_ptr, block_header->number_of_samples);
		if (mrr2 == (sf8) 0.0) {  // all zeros in block
			cps->parameters.amplitude_scale = (sf4) 1.0;
			cps->parameters.actual_ratio = (sf8) 0.0;
			(*compression_f)(cps);
			goto CMP_MRR_DONE_d11;
		}
		cps->parameters.amplitude_scale = (sf4) 5.0;
		CMP_generate_lossy_data_d11(cps, input_buffer, cps->decompressed_ptr, CMP_AMPLITUDE_SCALE_MODE_m11);
		mrr5 = CMP_calculate_mean_residual_ratio_d11(input_buffer, cps->decompressed_ptr, block_header->number_of_samples);
		sf_per_mrr = (sf8) 3.0 / (mrr5 - mrr2);
		// estimate starting points
		goal_mrr = cps->parameters.goal_ratio;
		goal_tol = cps->parameters.goal_tolerance;
		goal_low_bound = goal_mrr - goal_tol;
		goal_high_bound = goal_mrr + goal_tol;
		cps->parameters.amplitude_scale = (sf4)(((goal_mrr - mrr2) * sf_per_mrr) + (sf8)2.0);
		high_sf = ((goal_high_bound - mrr2) * sf_per_mrr) + (sf8) 2.0;
		high_sf *= (sf8) 2.0;  // empirically reasonable
		low_sf = (sf8) 1.0;
		for (i = cps->parameters.maximum_goal_attempts; i--;) {
			CMP_generate_lossy_data_d11(cps, input_buffer, cps->decompressed_ptr, CMP_AMPLITUDE_SCALE_MODE_m11);
			mrr = CMP_calculate_mean_residual_ratio_d11(input_buffer, cps->decompressed_ptr, block_header->number_of_samples);
			if (mrr < goal_low_bound)
				low_sf = (sf8) cps->parameters.amplitude_scale;
			else if (mrr > goal_high_bound)
				high_sf = (sf8) cps->parameters.amplitude_scale;
			else
				break;
			new_scale_factor = (sf4) ((low_sf + high_sf) / (sf8) 2.0);
			if (new_scale_factor <= (sf4) 1.0)
				break;
			cps->parameters.amplitude_scale = new_scale_factor;
			if ((high_sf - low_sf) < (sf8) 0.005)
				break;
		}
		cps->parameters.actual_ratio = mrr;
	} else {
		error_message_m11("%s(): either use_compression_ratio or use_mean_residual_ratio directive must be set (mode == %d)\n", __FUNCTION__, cps->directives.mode);
		return(data_is_compressed);
	} CMP_MRR_DONE_d11:
	
	return(data_is_compressed);
}


si8    *CMP_find_crits_d11(sf8 *data, si8 data_len, si8 *n_crits, si8 *crit_xs)
{
	const si1	PEAK = 1, TROUGH = -1;
	si1     	mode;
	si8     	nc, i, j, crit;
	
	
	// find peaks & troughs (also see CMP_find_crits_2_m11)
	// if crit_xs == NULL, array is allocated & returned;
	
	if (data == NULL) {
		error_message_m11("%s(): NULL pointer passed", __FUNCTION__);
		return(NULL);
	}
	
	if (crit_xs == NULL)
		crit_xs = (si8 *) malloc((size_t) (data_len << 3));
	
	for (j = 1; j < data_len; ++j)
		if (data[j] != data[0])
			break;
	
	crit_xs[0] = 0;
	if (j == data_len) {
		*n_crits = 2;
		crit_xs[1] = data_len - 1;
		return(crit_xs);
	}
	nc = 1;
	
	if (data[0] < data[j])
		mode = PEAK;
	else
		mode = TROUGH;
	
	i = j - 1;
	while (j < data_len) {
		if (mode == PEAK) {
			while (j < data_len) {
				if (data[j] > data[i])
					i = j++;
				else if (data[j] == data[i])
					++j;
				else
					break;
			}
			mode = TROUGH;
		} else {  // mode == TROUGH
			while (j < data_len) {
				if (data[j] < data[i])
					i = j++;
				else if (data[j] == data[i])
					++j;
				else
					break;
			}
			mode = PEAK;
		}
		if (i == (j - 1)) {
			crit = i;
		} else {
			crit = (i + j + 1) / 2;
			i = j - 1;
		}
		crit_xs[nc++] = crit;
	}
	
	if (crit_xs[nc - 1] != (data_len - 1))
		crit_xs[nc++] = (data_len - 1);
	
	*n_crits = nc;
	
	return(crit_xs);
}


void    CMP_find_crits_2_d11(sf8 *data, si8 data_len, si8 *n_peaks, si8 *peak_xs, si8 *n_troughs, si8 *trough_xs)
{
	const si1	PEAK = 1, TROUGH = 2;
	si1     	mode;
	si8     	np, nt, i, j, crit;
	
	
	// CMP_find_crits_2(): find peaks & troughs separately (see CMP_find_crits_m11)
	
	if (data == NULL || peak_xs == NULL || trough_xs == NULL) {
		error_message_m11("%s(): NULL pointer passed", __FUNCTION__);
		return;
	}
	
	for (j = 1; j < data_len; ++j)
		if (data[j] != data[0])
			break;
	
	peak_xs[0] = trough_xs[0] = 0;
	if (j == data_len) {
		peak_xs[1] = trough_xs[1] = data_len - 1;
		*n_peaks = *n_troughs = 2;
		return;
	}
	np = nt = 1;
	
	if (data[0] < data[j])
		mode = PEAK;
	else
		mode = TROUGH;
	
	i = j - 1;
	while (j < data_len) {
		if (mode == PEAK) {
			while (j < data_len) {
				if (data[j] > data[i])
					i = j++;
				else if (data[j] == data[i])
					++j;
				else
					break;
			}
			mode = TROUGH;
		} else {  // mode == TROUGH
			while (j < data_len) {
				if (data[j] < data[i])
					i = j++;
				else if (data[j] == data[i])
					++j;
				else
					break;
			}
			mode = PEAK;
		}
		if (i == (j - 1)) {
			crit = i;
		} else {
			crit = (i + j + 1) / 2;
			i = j - 1;
		}
		if (mode == TROUGH)
			peak_xs[np++] = crit;
		else
			trough_xs[nt++] = crit;
	}
	
	if (peak_xs[np - 1] != (data_len - 1))
		peak_xs[np++] = (data_len - 1);
	if (trough_xs[nt - 1] != (data_len - 1))
		trough_xs[nt++] = (data_len - 1);
	
	*n_peaks = np;
	*n_troughs = nt;
	
	return;
}


TERN_m11	CMP_find_frequency_scale_d11(CMP_PROCESSING_STRUCT_m11 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m11 *cps))
{
	
	// code not written yet
	
	return(TRUE_m11);
}


void    CMP_generate_lossy_data_d11(CMP_PROCESSING_STRUCT_m11 *cps, si4 *input_buffer, si4 *output_buffer, ui1 mode)
{
	CMP_BLOCK_FIXED_HEADER_m11	*block_header;
	
	
	// generates lossy data from input_buffer to output_buffer
	// if input_buffer == output_buffer lossy data will be made in place
	block_header = cps->block_header;
	
	if (mode == CMP_AMPLITUDE_SCALE_MODE_m11) {
		// amplitude scale from input_buffer to scaled_amplitude_buffer (lossy)
		CMP_scale_amplitude_si4_m11(input_buffer, cps->parameters.scaled_amplitude_buffer, block_header->number_of_samples, (sf8) cps->parameters.amplitude_scale, cps);
		// unscale from scaled_amplitude_buffer to output_buffer
		CMP_unscale_amplitude_si4_m11(cps->parameters.scaled_amplitude_buffer, output_buffer, block_header->number_of_samples, (sf8) cps->parameters.amplitude_scale);
	} else if (mode == CMP_FREQUENCY_SCALE_MODE_m11) {
		// frequency scale from input_buffer to scaled_frequency_buffer (lossy)
		CMP_scale_frequency_si4_m11(input_buffer, cps->parameters.scaled_frequency_buffer, block_header->number_of_samples, (sf8) cps->parameters.frequency_scale, cps);
		// unscale from scaled_frequency_buffer to output_buffer
		CMP_unscale_frequency_si4_m11(cps->parameters.scaled_frequency_buffer, output_buffer, block_header->number_of_samples, (sf8) cps->parameters.frequency_scale);
	} else {
		error_message_m11("%s(): unrecognized lossy compression mode => no data generated\n", __FUNCTION__);
	}
	
	return;
}


TERN_m11	CMP_initialize_tables_d11(void)
{
	sf8				*cdf_table;
	VDS_THRESHOLD_MAP_ENTRY_d11	*threshold_map;
	
	
	if (globals_d11->CMP_normal_CDF_table == NULL) {
		cdf_table = (sf8 *) calloc_m11((size_t) CMP_NORMAL_CDF_TABLE_ENTRIES_d11, sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		{
			sf8 temp[CMP_NORMAL_CDF_TABLE_ENTRIES_d11] = CMP_NORMAL_CDF_TABLE_d11;
			memcpy(cdf_table, temp, CMP_NORMAL_CDF_TABLE_ENTRIES_d11 * sizeof(sf8));
		}
		globals_d11->CMP_normal_CDF_table = cdf_table;
	}
	
	if (globals_d11->CMP_VDS_threshold_map == NULL) {
		threshold_map = (VDS_THRESHOLD_MAP_ENTRY_d11 *) calloc_m11((size_t) VDS_THRESHOLD_MAP_TABLE_ENTRIES_d11, sizeof(VDS_THRESHOLD_MAP_ENTRY_d11), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		{
			VDS_THRESHOLD_MAP_ENTRY_d11 temp[VDS_THRESHOLD_MAP_TABLE_ENTRIES_d11] = VDS_THRESHOLD_MAP_TABLE_d11;
			memcpy(threshold_map, temp, VDS_THRESHOLD_MAP_TABLE_ENTRIES_d11 * sizeof(VDS_THRESHOLD_MAP_ENTRY_d11));
		}
		globals_d11->CMP_VDS_threshold_map = threshold_map;
	}
	
	return(TRUE_m11);
}


void    CMP_lad_reg_2_sf8_d11(sf8 *x_input_buffer, sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b)
{
	sf8		t, *xp, *yp, *buff, *bp, min_x, max_x, min_y, max_y, min_m, max_m;
	sf8             d, ma, ba, m_eps, b_eps, lad_eps, test_m, lad, upper_m, lower_m;
	si8             i;
	const sf8	safe_eps = DBL_EPSILON * (sf8) 1000.0;
	const sf8	thresh = safe_eps * (sf8) 10.0;

	
	// linear least absolute deviation_regression (2 array input)

	// allocate
	buff = (sf8 *) malloc((size_t) len * sizeof(sf8));

	// setup
	xp = x_input_buffer;
	min_x = max_x = *xp;
	yp = y_input_buffer;
	min_y = max_y = *yp;
	for (i = len; --i;) {
		if (*++xp > max_x)
			max_x = *xp;
		else if (*xp < min_x)
			min_x = *xp;
		if (*++yp > max_y)
			max_y = *yp;
		else if (*yp < min_y)
			min_y = *yp;
	}
	upper_m = max_m = (max_y - min_y) / (max_x - min_x);
	lower_m = min_m = -max_m;
	d = max_m - min_m;

	// search
	while (d > thresh) {
		ma = (upper_m + lower_m) / (sf8) 2.0;
		bp = buff; xp = x_input_buffer; yp = y_input_buffer;
		for (i = len; i--;)
			*bp++ = *yp++ - (*xp++ * ma);
		ba = CMP_quantval_d11(buff, len, 0.5, 0, NULL);  // median
		bp = buff; lad = (sf8)0.0;
		for (i = len; i--;) {
			t = *bp++ - ba;
			lad += ABS_m11(t);
		}
		m_eps = ma + safe_eps;
		bp = buff; xp = x_input_buffer; yp = y_input_buffer;
		for (i = len; i--;)
			*bp++ = *yp++ - (*xp++ * m_eps);
		b_eps = CMP_quantval_d11(buff, len, 0.5, 0, NULL);  // median
		bp = buff; lad_eps = (sf8)0.0;
		for (i = len; i--;) {
			t = *bp++ - b_eps;
			lad_eps += ABS_m11(t);
		}
		test_m = lad_eps - lad;
		if (test_m > (sf8)0.0)
			upper_m = ma;
		else if (test_m < (sf8)0.0)
			lower_m = ma;
		else
			break;
		d = upper_m - lower_m;
	}

	*b = ba;
	*m = ma;

	// clean up
	free((void *) buff);

	return;
}


void    CMP_lad_reg_2_si4_d11(si4 *x_input_buffer, si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b)
{
	sf8		*x, *y, t, *xp, *yp, *buff, *bp, min_x, max_x, min_y, max_y, min_m, max_m;
	sf8             d, ma, ba, m_eps, b_eps, lad_eps, test_m, lad, upper_m, lower_m;
	si8             i;
	const sf8	safe_eps = DBL_EPSILON * (sf8) 1000.0;
	const sf8	thresh = safe_eps * (sf8) 10.0;

	// linear least absolute deviation_regression (2 array input)

	// allocate
	buff = (sf8 *) malloc((size_t) len * sizeof(sf8));
	x = (sf8 *) malloc((size_t) len * sizeof(sf8));
	y = (sf8 *) malloc((size_t) len * sizeof(sf8));

	// copy & cast
	for (xp = x, yp = y, i = len; i--;) {
		*xp++ = (sf8) *x_input_buffer++;
		*yp++ = (sf8) *y_input_buffer++;
	}

	// setup
	xp = x;
	min_x = max_x = *xp;
	yp = y;
	min_y = max_y = *yp;
	for (i = len; --i;) {
		if (*++xp > max_x)
			max_x = *xp;
		else if (*xp < min_x)
			min_x = *xp;
		if (*++yp > max_y)
			max_y = *yp;
		else if (*yp < min_y)
			min_y = *yp;
	}
	upper_m = max_m = (max_y - min_y) / (max_x - min_x);
	lower_m = min_m = -max_m;
	d = max_m - min_m;

	// search
	while (d > thresh) {
		ma = (upper_m + lower_m) / (sf8) 2.0;
		bp = buff; xp = x; yp = y;
		for (i = len; i--;)
			*bp++ = *yp++ - (*xp++ * ma);
		ba = CMP_quantval_d11(buff, len, 0.5, 0, NULL);  // median
		bp = buff; lad = (sf8)0.0;
		for (i = len; i--;) {
			t = *bp++ - ba;
			lad += ABS_m11(t);
		}
		m_eps = ma + safe_eps;
		bp = buff; xp = x; yp = y;
		for (i = len; i--;)
			*bp++ = *yp++ - (*xp++ * m_eps);
		b_eps = CMP_quantval_d11(buff, len, 0.5, 0, NULL);  // median
		bp = buff; lad_eps = (sf8)0.0;
		for (i = len; i--;) {
			t = *bp++ - b_eps;
			lad_eps += ABS_m11(t);
		}
		test_m = lad_eps - lad;
		if (test_m > (sf8)0.0)
			upper_m = ma;
		else if (test_m < (sf8)0.0)
			lower_m = ma;
		else
			break;
		d = upper_m - lower_m;
	}

	*b = ba;
	*m = ma;

	// clean up
	free((void *) buff);
	free((void *) x);
	free((void *) y);

	return;
}


void    CMP_lad_reg_sf8_d11(sf8 *y, si8 len, sf8 *m, sf8 *b)
{
	sf8     	lb, lm, t, *yp, *buff, *bp, min_y, max_y, min_m, max_m, m_sum;
	sf8     	d, m_eps, b_eps, lad_eps, test_m, lad, upper_m, lower_m;
	si8     	i;
	const sf8	safe_eps = DBL_EPSILON * (sf8) 1000.0;
	const sf8	thresh = safe_eps * (sf8) 10.0;

	
	// least absolute differences linear regression
	// assumes x to be 1:length(x)
	// fit: y = mx + b
	
	// allocate
	buff = (sf8 *) malloc((size_t) len * sizeof(sf8));
	if (buff == NULL)
		error_message_m11("%s(): could not allocate enough memory\n", __FUNCTION__);
	
	// setup
	yp = y;
	min_y = max_y = *yp;
	for (i = len; --i;) {
		if (*++yp > max_y)
			max_y = *yp;
		else if (*yp < min_y)
			min_y = *yp;
	}
	lower_m = min_m = (min_y - max_y) / (sf8) (len - 1);
	upper_m = max_m  = -min_m;
	d = max_m - min_m;
	
	// search
	while (d > thresh) {
		lm = (upper_m + lower_m) / (sf8) 2.0;
		bp = buff; yp = y;
		m_sum = (sf8) 0.0;
		for (i = len; i--;)
			*bp++ = *yp++ - (m_sum += lm);
		lb = CMP_quantval_d11(buff, len, 0.5, FALSE_m11, NULL);  // median
		bp = buff; lad = (sf8) 0.0;
		for (i = len; i--;) {
			t = *bp++ - lb;
			lad += ABS_m11(t);
		}
		m_eps = lm + safe_eps;
		bp = buff; yp = y;
		m_sum = (sf8) 0.0;
		for (i = len; i--;)
			*bp++ = *yp++ - (m_sum += m_eps);
		b_eps = CMP_quantval_d11(buff, len, 0.5, FALSE_m11, NULL);  // median
		bp = buff; lad_eps = (sf8) 0.0;
		for (i = len; i--;) {
			t = *bp++ - b_eps;
			lad_eps += ABS_m11(t);
		}
		test_m = lad_eps - lad;
		if (test_m > (sf8) 0.0)
			upper_m = lm;
		else if (test_m < (sf8) 0.0)
			lower_m = lm;
		else
			break;
		d = upper_m - lower_m;
	}
	*b = lb;
	*m = lm;
	
	// clean up
	free(buff);
	
	return;
}


void    CMP_lad_reg_si4_d11(si4 *input_buffer, si8 len, sf8 *m, sf8 *b)
{
	sf8		*y, t, *yp, *buff, *bp, min_y, max_y, min_m, max_m, m_sum;
	sf8             d, ma, ba, m_eps, b_eps, lad_eps, test_m, lad, upper_m, lower_m;
	si8             i;
	const sf8	safe_eps = DBL_EPSILON * (sf8) 1000.0;
	const sf8	thresh = safe_eps * (sf8) 10.0;
	
	
	// linear least absolute deviation_regression (1 array input)
	
	// allocate
	buff = (sf8 *) calloc_m11((size_t) len, sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	y = (sf8 *) calloc_m11((size_t) len, sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	
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
	d = max_m - min_m;
	
	// search
	while (d > thresh) {
		ma = (upper_m + lower_m) / (sf8) 2.0;
		bp = buff; yp = y;
		m_sum = (sf8) 0.0;
		for (i = len; i--;)
			*bp++ = *yp++ - (m_sum += ma);
		ba = CMP_quantval_d11(buff, len, 0.5, 0, NULL);  // median
		bp = buff; lad = (sf8) 0.0;
		for (i = len; i--;) {
			t = *bp++ - ba;
			lad += ABS_m11(t);
		}
		m_eps = ma + safe_eps;
		bp = buff; yp = y;
		m_sum = (sf8) 0.0;
		for (i = len; i--;)
			*bp++ = *yp++ - (m_sum += m_eps);
		b_eps = CMP_quantval_d11(buff, len, 0.5, 0, NULL);  // median
		bp = buff; lad_eps = (sf8) 0.0;
		for (i = len; i--;) {
			t = *bp++ - b_eps;
			lad_eps += ABS_m11(t);
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


void    CMP_lin_reg_2_si4_d11(si4 *x_input_buffer, si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b)
{
	sf8                     sx, sy, sxx, sxy, n, mx, my, x_val, y_val;
	si8                     i;
	
	
	// linear least_squares regression (2 array input)
	n = (sf8) len;
	sx = sy = sxx = sxy = (sf8) 0.0;
	for (i = len; i--;) {
		x_val = (sf8) *x_input_buffer++;
		y_val = (sf8) *y_input_buffer++;
		sx += x_val;
		sxx += x_val * x_val;
		sy += y_val;
		sxy += x_val * y_val;
	}
	
	mx = sx / n;
	my = sy / n;
	*m = (((sx * my) - sxy) / ((sx * mx) - sxx));
	*b = (my - (*m * mx));
		
	return;
}


void    CMP_lin_reg_si4_d11(si4 *input_buffer, si8 len, sf8 *m, sf8 *b)
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


ui1     CMP_normality_score_d11(si4 *data, ui4 n_samps)
{
	sf8     sx, sx2, sy, sy2, sxy, mx, mx2, sd, val, z, r, n, *norm_cdf;
	sf8     num, den1, den2, cdf[CMP_NORMAL_CDF_TABLE_ENTRIES_d11];
	si8     i, count[CMP_NORMAL_CDF_TABLE_ENTRIES_d11] = {0};
	si4	*si4_p, bin;
	ui1     ks;
	
	
	// Returns the correlation of the distribution in the data to that expected from a normal distribution.
	// Essentially a Kolmogorov-Smirnov test normalized to range [-1 to 0) = 0 & [0 to 1] = [0 to 254]. 255 is reserved for no entry.
	
	if (globals_d11->CMP_normal_CDF_table == NULL)
		CMP_initialize_tables_d11();
	norm_cdf = globals_d11->CMP_normal_CDF_table;

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
		else if (bin >= CMP_NORMAL_CDF_TABLE_ENTRIES_d11)
			bin = CMP_NORMAL_CDF_TABLE_ENTRIES_d11 - 1;
		++count[bin];
	}
	
	// generate data CDF
	cdf[0] = (sf8) count[0];
	for (i = 1; i < CMP_NORMAL_CDF_TABLE_ENTRIES_d11; ++i)
		cdf[i] = (sf8)count[i] + cdf[i - 1];
	
	// calculate correlation between data CDF and normal CDF
	sx = sx2 = sxy = (sf8) 0.0;
	sy = (sf8 )CMP_SUM_NORMAL_CDF_d11;
	sy2 = (sf8) CMP_SUM_SQ_NORMAL_CDF_d11;
	for (i = 0; i < CMP_NORMAL_CDF_TABLE_ENTRIES_d11; ++i) {
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
	r += CMP_KS_CORRECTION_d11;
	if (r < (sf8) 0.0)
		r = (sf8) 0.0;
	else if (r > (sf8) 1.0)
		r = (sf8) 1.0;
	
	// return KS score (negative values set to zero, positive scaled to 0-254, 255 reserved for no entry)
	ks = (ui1) CMP_round_si4_m11(r * 254);     // ks = (ui1) CMP_round_si4_m11(sqrt(1.0 - (r * r)) * 254);  => I had this before, but I don't know why
	
	return(ks);
}


// Algorithm from Niklaus Wirth's book: "Algorithms + data structures = programs".
// Code here is adapted from code by Nicolas Devillard. Public domain.
sf8     CMP_quantval_d11(sf8 *x, si8 len, sf8 quantile, TERN_m11 preserve_input, sf8 *buff)
{
	TERN_m11        free_buff;
	sf8             q, fk, lo_p, lo_v, *lp, *mp, *last_mp, *lo_kp, *hi_kp;
	si8             lo_k;
	register sf8    v, t, *xip, *xjp;
	
	
	if (len == 1)
		return(*x);
	
	free_buff = FALSE_m11;
	if (preserve_input == TRUE_m11) {
		if (buff == NULL) {
			buff = (sf8 *) malloc_m11((size_t) len * sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			free_buff = TRUE_m11;
		}
		memcpy((void *) buff, (void *) x, (size_t) (len << 3));
		x = buff;
	}
	
	if (quantile == (sf8)1.0) {
		lo_k = len - 2;
		lo_p = (sf8) 0.0;
	} else {
		fk = quantile * (sf8) (len - 1);
		lo_k = (si8) fk;
		lo_p = (sf8) 1.0 - (fk - (sf8) lo_k);
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
	
	if (free_buff == TRUE_m11)
		free((void *) buff);
	
	return(q);
}


sf8	CMP_splope_d11(sf8 *xa, sf8 *ya, sf8 *d2y, sf8 x, si8 lo_pt, si8 hi_pt)
{
	sf8	a, b, c, d, e, f, g, h;
	
	
	// slope of splined array (characterized by d2y), at point x
	// useful for many things, but in particular finding precise locations of critical points in splined data

	a = xa[lo_pt];
	b = xa[hi_pt];
	c = ya[lo_pt];
	d = ya[hi_pt];
	e = d2y[lo_pt];
	f = d2y[hi_pt];
	g = b - a;

	h = (x * (e * b - f * a) + d - c) / g;
	h += (x * x * (f - e) + f * a * a - e * b * b) / (2.0 * g);
	h += g * (e - f) / 6.0;

	return(h);
}


void	CMP_VDS_encode_d11(CMP_PROCESSING_STRUCT_m11 *cps)
{
	TERN_m11			change_made;
	ui1				*VDS_model_region, *VDS_amplitude_model_region, *VDS_time_model_region;
	ui4				VDS_number_of_samples, VDS_total_header_bytes;
	ui4				VDS_total_block_bytes, algorithm;
	si4				*si4_p, spacing;
	sf4				*sf4_p;
	si8				i, j, k, new_in_len, block_samps, poles, pad_samps, in_len, offset;
	si8				*in_x, *new_in_x, *si8_p1, *si8_p2, scale;
	sf8				VDS_alg_thresh, *out_x, *template, *sf8_p1, *sf8_p2, *sf8_p3, *in_y, *out_y;
	sf8				*abs_diffs, *quantval_buf, diff, thresh, baseline, *resids, max_dx, max_dy;
	CMP_BUFFERS_m11			*VDS_in_bufs, *VDS_out_bufs;
	CMP_BLOCK_FIXED_HEADER_m11	*block_header;
	CMP_VDS_MODEL_FIXED_HEADER_m11	*VDS_header;


	// VDS Buffer Map:
	// 	VDS_in_bufs[0]:	mak() in_y
	// 	VDS_in_bufs[1]:	mak() in_x
	// 	VDS_in_bufs[8]:	template

	// redirect to PRED for lossless encoding
	if (cps->parameters.VDS_threshold == (sf8) 0.0)  {
		// change block header & directive (so don't do this for every block)
		cps->block_header->block_flags &= ~CMP_BF_ALGORITHMS_MASK_m11;
		cps->block_header->block_flags |= CMP_BF_PRED_ENCODING_MASK_m11;
		cps->directives.algorithm = CMP_PRED_COMPRESSION_m11;
		// change RED arrays to PRED (2D arrays)
		free(cps->parameters.count);
		free(cps->parameters.sorted_count);
		free(cps->parameters.symbol_map);
		free(cps->parameters.cumulative_count);
		free(cps->parameters.minimum_range);
		cps->parameters.count = (void *) calloc_2D_m11((size_t) CMP_PRED_CATS_m11, CMP_RED_MAX_STATS_BINS_m11, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		cps->parameters.sorted_count = (void *) calloc_2D_m11((size_t) CMP_PRED_CATS_m11, CMP_RED_MAX_STATS_BINS_m11, sizeof(CMP_STATISTICS_BIN_m11), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		cps->parameters.symbol_map = (void *) calloc_2D_m11((size_t) CMP_PRED_CATS_m11, CMP_RED_MAX_STATS_BINS_m11, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		cps->parameters.cumulative_count = (void *) calloc_2D_m11((size_t) CMP_PRED_CATS_m11, CMP_RED_MAX_STATS_BINS_m11 + 1, sizeof(ui8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		cps->parameters.minimum_range = (void *) calloc_2D_m11((size_t) CMP_PRED_CATS_m11, CMP_RED_MAX_STATS_BINS_m11, sizeof(ui8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		// PRED encode
		CMP_PRED_encode_m11(cps);
		return;
	}
	// convert user to algorithm threshold
	VDS_alg_thresh = CMP_VDS_get_theshold_d11(cps);

	// redirect to MBE for tiny blocks
	block_samps = (si8) cps->block_header->number_of_samples;
	if (block_samps < CMP_VDS_MINIMUM_SAMPLES_m11) {
		cps->block_header->block_flags &= ~CMP_BF_ALGORITHMS_MASK_m11;
		cps->block_header->block_flags |= CMP_BF_MBE_ENCODING_MASK_m11;
		CMP_MBE_encode_m11(cps);
		return;
	}
	
	// allocate
	poles = FILT_POLES_d11(CMP_VDS_LOWPASS_ORDER_m11, 1);
	pad_samps = FILT_FILT_PAD_SAMPLES_d11(poles);
	cps->parameters.VDS_input_buffers = CMP_allocate_buffers_m11(cps->parameters.VDS_input_buffers, CMP_VDS_INPUT_BUFFERS_m11, block_samps + pad_samps, sizeof(sf8), FALSE_m11, FALSE_m11);
	cps->parameters.VDS_output_buffers = CMP_allocate_buffers_m11(cps->parameters.VDS_output_buffers, CMP_VDS_OUTPUT_BUFFERS_m11, block_samps, sizeof(sf8), FALSE_m11, FALSE_m11);
	VDS_in_bufs = cps->parameters.VDS_input_buffers;
	VDS_out_bufs = cps->parameters.VDS_output_buffers;

	// convert block samples to sf8 array
	in_y = (sf8 *) VDS_in_bufs->buffer[0];
	CMP_si4_to_sf8_m11(cps->input_buffer, in_y, block_samps);

	// generate template
	CMP_VDS_generate_template_d11(cps, block_samps);
	
	// get baseline
	template = (sf8 *) VDS_in_bufs->buffer[8];
	abs_diffs = (sf8 *) VDS_in_bufs->buffer[2];
	sf8_p1 = in_y;
	sf8_p2 = template;
	sf8_p3 = abs_diffs;
	for (i = block_samps; i--;) {
		diff = *sf8_p1++ - *sf8_p2++;
		*sf8_p3++ = (diff >= (sf8) 0.0) ? diff : -diff;
	}
	quantval_buf = (sf8 *) VDS_in_bufs->buffer[3];
	baseline = CMP_quantval_d11(abs_diffs, block_samps, 0.5, TRUE_m11, quantval_buf);

	// find critical points
	in_x = (si8 *) VDS_in_bufs->buffer[1];
	CMP_find_crits_d11(template, block_samps, &in_len, in_x);

	// put critical points into input mak arrays
	sf8_p1 = in_y;
	si8_p1 = in_x;
	for (i = in_len; i--;)
		*sf8_p1++ = template[*si8_p1++];

	// get absolute differences of critical point y values
	sf8_p1 = in_y;
	sf8_p2 = in_y + 1;
	sf8_p3 = abs_diffs;
	for (i = in_len; --i;) {
		diff = *sf8_p2++ - *sf8_p1++;
		*sf8_p3++ = (diff >= (sf8) 0.0) ? diff : -diff;
	}

	// eliminate spurious critical points
	sf8_p1 = abs_diffs;
	si8_p1 = si8_p2 = in_x + 1;
	thresh = baseline * VDS_alg_thresh;
	for (i = in_len; --i; ++si8_p1)
		if (*sf8_p1++ > thresh)
			*si8_p2++ = *si8_p1;
	in_len = si8_p2 - in_x;
	if (in_x[in_len - 1] != (block_samps - 1))
		in_x[in_len++] = block_samps - 1;

	// create out_x array
	out_x = VDS_out_bufs->buffer[1];
	sf8_p1 = out_x + block_samps;
	for (i = block_samps; i--;)
		*--sf8_p1 = (sf8) i;

	// refine fit
	new_in_x = VDS_in_bufs->buffer[2];
	out_y = VDS_out_bufs->buffer[0];
	resids = VDS_out_bufs->buffer[2];
	do {
		// copy anchors into mak input array
		sf8_p1 = in_y;
		si8_p1 = in_x;
		for (i = in_len; i--;)
			*sf8_p1++ = template[*si8_p1++];

		// fit anchor points
		CMP_mak_interp_sf8_m11(VDS_in_bufs, in_len, VDS_out_bufs, block_samps);

		// calculate residuals
		sf8_p1 = template;
		sf8_p2 = out_y;
		sf8_p3 = resids;
		for (i = block_samps; --i;) {
			diff = *sf8_p1++ - *sf8_p2++;
			*sf8_p3++ = (diff >= (sf8) 0.0) ? diff : -diff;
		}

		// add anchors
		change_made = FALSE_m11;
		new_in_x[0] = in_x[0];
		for (new_in_len = i = 1; i < in_len; ++i) {
			// add point that deviates maximally in segment, if it exceeds threshold
			for (max_dy = 0, k = in_x[i], j = in_x[i - 1] + 1; j < k; ++j) {
				if (resids[j] > max_dy) {
					max_dx = j;
					max_dy = resids[j];
				}
			}
			spacing = (si4) (in_x[i] - in_x[i - 1]);
			if (max_dy > thresh || spacing > 255) {  // find max diff even if didn't exceed thresh (for RED)
				new_in_x[new_in_len++] = max_dx;
				change_made = TRUE_m11;
			}
			new_in_x[new_in_len++] = in_x[i];
		}

		// copy new anchor set into in_x
		if (change_made == TRUE_m11) {
			memcpy((void *) in_x, (void *) new_in_x, (size_t) (new_in_len << 3));
			in_len = new_in_len;
		}
	} while (change_made == TRUE_m11);
	
	// scale data (if requested)
	if (cps->directives.VDS_scale_by_baseline == TRUE_m11) {
		if (baseline > (sf8) 1.0) {
			cps->directives.set_amplitude_scale = TRUE_m11;
			cps->parameters.amplitude_scale = (sf4) baseline;
			CMP_set_variable_region_m11(cps);
		} else {
			cps->directives.set_amplitude_scale = FALSE_m11;
			cps->parameters.amplitude_scale = (sf4) 1.0;
		}
	}
	if (cps->directives.set_amplitude_scale == TRUE_m11) {
		sf4_p = (sf4 *) cps->block_parameters;
		offset = (si8) cps->parameters.block_parameter_map[CMP_PF_AMPLITUDE_SCALE_IDX_m11];
		*(sf4_p + offset) = cps->parameters.amplitude_scale;
		scale = (sf8) cps->parameters.amplitude_scale;
		sf8_p1 = in_y;
		for (i = in_len; i--;)
			*sf8_p1++ /= scale;
	}
	
	// copy data to input buffer
	cps->input_buffer = (si4 *) VDS_in_bufs->buffer[8];  // use VDS template buffer (finished with template for this round)
	CMP_sf8_to_si4_m11(in_y, cps->input_buffer, in_len);

	// get VDS model info
	VDS_number_of_samples = (ui4) in_len;
	block_header = cps->block_header;
	VDS_model_region = cps->parameters.model_region;
	VDS_total_header_bytes = (ui4) (VDS_model_region - (ui1 *) block_header) + (ui4) CMP_VDS_MODEL_FIXED_HEADER_BYTES_m11;
	VDS_total_block_bytes = VDS_total_header_bytes;
	VDS_header = (CMP_VDS_MODEL_FIXED_HEADER_m11 *) VDS_model_region;
	VDS_header->number_of_VDS_samples = VDS_number_of_samples;
	VDS_header->flags = 0;

	// encode amplitudes
	block_header = cps->block_header;
	block_header->number_of_samples = VDS_number_of_samples;
	VDS_amplitude_model_region = (ui1 *) block_header + VDS_total_block_bytes;
	cps->parameters.model_region = VDS_amplitude_model_region;
	block_header->block_flags &= ~CMP_BF_ALGORITHMS_MASK_m11;
	block_header->block_flags |= CMP_BF_RED_ENCODING_MASK_m11;  // start with RED - may fall through
	CMP_RED_encode_m11(cps);
	VDS_header->amplitude_block_total_bytes = block_header->total_block_bytes - VDS_total_block_bytes;
	VDS_header->amplitude_block_model_bytes = block_header->model_region_bytes;
	VDS_total_block_bytes = block_header->total_block_bytes;
	algorithm = block_header->block_flags & CMP_BF_ALGORITHMS_MASK_m11;
	block_header->block_flags &= ~CMP_BF_ALGORITHMS_MASK_m11;
	switch (algorithm) {
		case CMP_BF_RED_ENCODING_MASK_m11:
			VDS_header->flags |= CMP_VDS_FLAGS_AMPLITUDE_RED_MASK_m11;
			break;
		case CMP_BF_PRED_ENCODING_MASK_m11:
			VDS_header->flags |= CMP_VDS_FLAGS_AMPLITUDE_PRED_MASK_m11;
			break;
		case CMP_BF_MBE_ENCODING_MASK_m11:
			VDS_header->flags |= CMP_VDS_FLAGS_AMPLITUDE_MBE_MASK_m11;
			break;
	}
	
	// encode times
	si4_p = cps->input_buffer;
	si8_p1 = in_x;
	for (i = VDS_number_of_samples; i--;)
		*si4_p++ = (si4) *si8_p1++;
	
	VDS_time_model_region = (ui1 *) block_header + VDS_total_block_bytes;
	cps->parameters.model_region = VDS_time_model_region;
	block_header->block_flags &= ~CMP_BF_ALGORITHMS_MASK_m11;
	block_header->block_flags |= CMP_BF_RED_ENCODING_MASK_m11;  // start with RED - may fall through
	CMP_RED_encode_m11(cps);
	VDS_header->time_block_model_bytes = block_header->model_region_bytes;
	algorithm = block_header->block_flags & CMP_BF_ALGORITHMS_MASK_m11;
	switch (algorithm) {
		case CMP_BF_RED_ENCODING_MASK_m11:
			VDS_header->flags |= CMP_VDS_FLAGS_TIME_RED_MASK_m11;
			break;
		case CMP_BF_PRED_ENCODING_MASK_m11:
			VDS_header->flags |= CMP_VDS_FLAGS_TIME_PRED_MASK_m11;
			break;
		case CMP_BF_MBE_ENCODING_MASK_m11:
			VDS_header->flags |= CMP_VDS_FLAGS_TIME_MBE_MASK_m11;
			break;
	}

	// set block back to VDS values (in case re-read from memory)
	block_header->block_flags &= ~CMP_BF_ALGORITHMS_MASK_m11;
	block_header->block_flags |= CMP_BF_VDS_ENCODING_MASK_m11;
	block_header->number_of_samples = (ui4) block_samps;
	block_header->total_header_bytes = VDS_total_header_bytes;
	block_header->model_region_bytes = (ui2) CMP_VDS_MODEL_FIXED_HEADER_BYTES_m11;
	cps->parameters.model_region = VDS_model_region;
	
	return;
}


void	CMP_VDS_generate_template_d11(CMP_PROCESSING_STRUCT_m11 *cps, si8 data_len)
{
	TERN_m11			LFP_filter, realloc_flag;
	si8				i, j, block_samps, *extrema, n_extrema, min_cutoff;
	sf8				*y, *smooth, *transients, *template, samp_freq, LFP_high_fc;
	sf8				*sf8_p1, *sf8_p2, *sf8_p3;
	FILT_PROCESSING_STRUCT_d11	*min_filtps, *lfp_filtps;
	
	
	// Function assumes VDS buffers are allocated
	// Buffer Map:
	// 	VDS_in_bufs[0]:	y
	// 	VDS_in_bufs[1]: x (not used, but don't touch)
	// 	VDS_in_bufs[2]:	excise_transients() smooth_data & LFP filt_data
	// 	VDS_in_bufs[3]:	excise_transients() transients
	// 	VDS_in_bufs[4]:	excise_transients() extrema
	//	VDS_in_bufs[7]:	filter() buffer
	//	VDS_in_bufs[8]:	template buffer
	
	samp_freq = cps->parameters.VDS_sampling_frequency;
	LFP_high_fc = cps->parameters.VDS_LFP_high_fc;
	block_samps = (si8) cps->block_header->number_of_samples;
	y = (sf8 *) cps->parameters.VDS_input_buffers->buffer[0];

	LFP_filter = FALSE_m11;
	if (LFP_high_fc != (sf8) 0.0)
		LFP_filter = TRUE_m11;
	
	// get filter processing struct: minimal filter (5 samples / cycle)
	min_cutoff = samp_freq / (sf8) 5.0;
	realloc_flag = FALSE_m11;
	if (cps->parameters.n_filtps < (FILT_VDS_TEMPLATE_MIN_PS_d11 + 1) || cps->parameters.filtps == NULL) {
		realloc_flag = TRUE_m11;
		cps->parameters.n_filtps = FILT_VDS_TEMPLATE_MIN_PS_d11 + 1;
		cps->parameters.filtps = (void **) realloc((void *) cps->parameters.filtps, sizeof(void *) * cps->parameters.n_filtps);
		min_filtps = (FILT_PROCESSING_STRUCT_d11 *) (cps->parameters.filtps[FILT_VDS_TEMPLATE_MIN_PS_d11] = NULL);
	} else {
		min_filtps = (FILT_PROCESSING_STRUCT_d11 *) cps->parameters.filtps[FILT_VDS_TEMPLATE_MIN_PS_d11];
		if (min_filtps == NULL)
			realloc_flag = TRUE_m11;
		else if (min_filtps->order != CMP_VDS_LOWPASS_ORDER_m11)
			realloc_flag = TRUE_m11;
		else if (min_filtps->type != FILT_LOWPASS_TYPE_d11)
			realloc_flag = TRUE_m11;
		else if (min_filtps->sampling_frequency != samp_freq)
			realloc_flag = TRUE_m11;
		else if (min_filtps->cutoffs[0] != min_cutoff)
			realloc_flag = TRUE_m11;
	}
	if (realloc_flag == TRUE_m11) {
		if (min_filtps != NULL)
			FILT_free_processing_struct_d11(min_filtps, FALSE_m11, FALSE_m11, FALSE_m11, TRUE_m11);
		cps->parameters.filtps[FILT_VDS_TEMPLATE_MIN_PS_d11] = (void *) FILT_initialize_processing_struct_d11(CMP_VDS_LOWPASS_ORDER_m11, FILT_LOWPASS_TYPE_d11, samp_freq, block_samps, FALSE_m11, FALSE_m11, FALSE_m11, (RETURN_ON_FAIL_m11 | SUPPRESS_WARNING_OUTPUT_m11), min_cutoff);
		min_filtps = (FILT_PROCESSING_STRUCT_d11 *) cps->parameters.filtps[FILT_VDS_TEMPLATE_MIN_PS_d11];
	}
	
	// get filter processing struct: LFP filter (user specified cutoff)
	if (LFP_filter == TRUE_m11) {
		realloc_flag = FALSE_m11;
		if (cps->parameters.n_filtps < (FILT_VDS_TEMPLATE_LFP_PS_d11 + 1)) {
			realloc_flag = TRUE_m11;
			cps->parameters.n_filtps = FILT_VDS_TEMPLATE_LFP_PS_d11 + 1;
			cps->parameters.filtps = (void **) realloc((void *) cps->parameters.filtps, sizeof(void *) * cps->parameters.n_filtps);
			lfp_filtps = (FILT_PROCESSING_STRUCT_d11 *) (cps->parameters.filtps[FILT_VDS_TEMPLATE_LFP_PS_d11] = NULL);
		} else {
			lfp_filtps = (FILT_PROCESSING_STRUCT_d11 *) cps->parameters.filtps[FILT_VDS_TEMPLATE_LFP_PS_d11];
			if (lfp_filtps == NULL)
				realloc_flag = TRUE_m11;
			else if (lfp_filtps->order != CMP_VDS_LOWPASS_ORDER_m11)
				realloc_flag = TRUE_m11;
			else if (lfp_filtps->type != FILT_LOWPASS_TYPE_d11)
				realloc_flag = TRUE_m11;
			else if (lfp_filtps->sampling_frequency != samp_freq)
				realloc_flag = TRUE_m11;
			else if (lfp_filtps->cutoffs[0] != LFP_high_fc)
				realloc_flag = TRUE_m11;
		}
		if (realloc_flag == TRUE_m11) {
			if (lfp_filtps != NULL)
				FILT_free_processing_struct_d11(lfp_filtps, FALSE_m11, FALSE_m11, FALSE_m11, TRUE_m11);
			cps->parameters.filtps[FILT_VDS_TEMPLATE_LFP_PS_d11] = (void *) FILT_initialize_processing_struct_d11(CMP_VDS_LOWPASS_ORDER_m11, FILT_LOWPASS_TYPE_d11, samp_freq, block_samps, FALSE_m11, FALSE_m11, FALSE_m11, (RETURN_ON_FAIL_m11 | SUPPRESS_WARNING_OUTPUT_m11), LFP_high_fc);
			lfp_filtps = (FILT_PROCESSING_STRUCT_d11 *) cps->parameters.filtps[FILT_VDS_TEMPLATE_LFP_PS_d11];
		}
	}
	
	// excise transients
	if (LFP_filter == TRUE_m11) {  // if LFP filtering have smooth data put into offset position
		smooth = (sf8 *) cps->parameters.VDS_input_buffers->buffer[2];
		lfp_filtps->filt_data = smooth;  // smooth data will end up in buffer 2
		lfp_filtps->orig_data = FILT_OFFSET_ORIG_DATA_d11(lfp_filtps);  // offset smooth data for filtering
		cps->parameters.VDS_input_buffers->buffer[2] = (void *) lfp_filtps->orig_data;  // offset smooth data (buffer 2) pointer
	}
	FILT_excise_transients_d11(cps, data_len, &n_extrema);
	extrema = (si8 *) cps->parameters.VDS_input_buffers->buffer[4];
	transients = (sf8 *) cps->parameters.VDS_input_buffers->buffer[3];

	// set up template minimal filter
	template = (sf8 *) cps->parameters.VDS_input_buffers->buffer[8];
	min_filtps->filt_data = template;
	min_filtps->orig_data = FILT_OFFSET_ORIG_DATA_d11(min_filtps);
	
	if (LFP_filter == TRUE_m11) {
		// lowpass filter smooth data
		cps->parameters.VDS_input_buffers->buffer[2] = smooth;  // reset buffer 2 pointer (smooth data)
		lfp_filtps->data_length = data_len;
		lfp_filtps->buffer = (sf8 *) cps->parameters.VDS_input_buffers->buffer[7];
		FILT_filtfilt_d11(lfp_filtps);
	
		// add transients and smooth data into template
		sf8_p1 = smooth;
		sf8_p2 = transients;
		sf8_p3 = min_filtps->orig_data;  // offset template
		for (i = data_len; i--;)
			*sf8_p3++ = *sf8_p1++ + *sf8_p2++;
	} else {
		// copy input data into offset template
		memcpy((void *) min_filtps->orig_data, (void *) y, (size_t) (data_len << 3));  // offset template
	}
	
	// minimal filter template
	min_filtps->data_length = data_len;
	min_filtps->buffer = (sf8 *) cps->parameters.VDS_input_buffers->buffer[7];
	FILT_filtfilt_d11(min_filtps);
	
	// fix extrema in template
	for (i = 0; i < n_extrema; ++i) {
		j = extrema[i];
		template[j] = y[j];
	}
	
	// Exit Buffer Map:
	// 	VDS_in_bufs[0]:	y (not touched)
	// 	VDS_in_bufs[1]: x (not touched)
	// 	VDS_in_bufs[2 - 7]: available
	//	VDS_in_bufs[8]:	template
	
	return;
}


sf8	CMP_VDS_get_theshold_d11(CMP_PROCESSING_STRUCT_m11 *cps)
{
	TERN_m11			no_filt;
	si4				i, low_i, high_i;
	sf8				prop, user_thresh, alg_thresh;
	VDS_THRESHOLD_MAP_ENTRY_d11	*thresh_map;
	
	
	if (globals_d11->CMP_VDS_threshold_map == NULL)
		CMP_initialize_tables_d11();
	thresh_map = globals_d11->CMP_VDS_threshold_map;
	
	user_thresh = cps->parameters.VDS_threshold;
	if (cps->parameters.VDS_LFP_high_fc == 0.0)
		no_filt = TRUE_m11;
	else
		no_filt = FALSE_m11;

	if (user_thresh > (sf8) 10.0) {
		if (user_thresh == (sf8) 11.0)
			message_m11("%s: This threshold goes to 11.\n", __FUNCTION__);
		else
			warning_message_m11("%s: the VDS threshold range is 0 to 10 => setting to 10\n", __FUNCTION__);
		if (no_filt == TRUE_m11)
			return(thresh_map[VDS_THRESHOLD_MAP_TABLE_ENTRIES_d11 - 1].algorithm_threshold_no_filt);
		else
			return(thresh_map[VDS_THRESHOLD_MAP_TABLE_ENTRIES_d11 - 1].algorithm_threshold_LFP);
	}

	if (user_thresh < (sf8) 0.0) {
		warning_message_m11("%s: the VDS threshold range is 0 to 10 => setting to 0\n", __FUNCTION__);
		if (no_filt == TRUE_m11)
			return(thresh_map[0].algorithm_threshold_no_filt);
		else
			return(thresh_map[0].algorithm_threshold_LFP);
	}

	for (i = 1; i < VDS_THRESHOLD_MAP_TABLE_ENTRIES_d11; ++i)
		if (user_thresh < thresh_map[i].user_threshold)
			break;
	
	high_i = i;
	low_i = i - 1;
	if (user_thresh == thresh_map[low_i].user_threshold) {
		if (no_filt == TRUE_m11)
			return(thresh_map[low_i].algorithm_threshold_no_filt);
		else
			return(thresh_map[low_i].algorithm_threshold_LFP);
	}
	
	// interpolate
	prop = (user_thresh - thresh_map[low_i].user_threshold) / (thresh_map[high_i].user_threshold - thresh_map[low_i].user_threshold);
	if (no_filt == TRUE_m11) {
		alg_thresh = (1.0 - prop) * thresh_map[low_i].algorithm_threshold_no_filt;
		alg_thresh += prop * thresh_map[high_i].algorithm_threshold_no_filt;
	} else {
		alg_thresh = (1.0 - prop) * thresh_map[low_i].algorithm_threshold_LFP;
		alg_thresh += prop * thresh_map[high_i].algorithm_threshold_LFP;
	}
	
	return(alg_thresh);
}


//***********************************************************************//
//*********************  DATA MATRIX (DM) FUNCTIONS  ********************//
//***********************************************************************//


void	DM_free_matrix_d11(DATA_MATRIX_d11 *matrix, TERN_m11 free_structure)
{
	si8	i;
	
	
	if (matrix == NULL) {
		warning_message_m11("%s(): attempting to free NULL structure\n", __FUNCTION__);
		return;
	}
	
	if (matrix->data != NULL)
		free(matrix->data);
	
	if (matrix->range_minima != NULL)
		free(matrix->range_minima);
	
	if (matrix->range_maxima != NULL)
		free(matrix->range_maxima);
	
	if (matrix->range_maxima != NULL)
		free(matrix->range_maxima);

	if (matrix->contigua != NULL)
		free((void *) matrix->contigua);

	if (matrix->in_bufs != NULL) {
		for (i = 0; i < matrix->n_proc_bufs; ++i) {
			CMP_free_buffers_m11(matrix->in_bufs[i], TRUE_m11);
			CMP_free_buffers_m11(matrix->out_bufs[i], TRUE_m11);
			CMP_free_buffers_m11(matrix->spline_bufs[i], TRUE_m11);
		}
		free((void *) matrix->in_bufs);
		free((void *) matrix->out_bufs);
		free((void *) matrix->spline_bufs);
	}
	
	if (free_structure == TRUE_m11) {
		free((void *) matrix);
	} else {
		matrix->data = NULL;
		matrix->range_minima = NULL;
		matrix->range_maxima = NULL;
		matrix->range_maxima = NULL;
		matrix->contigua = NULL;
		matrix->number_of_contigua = 0;
		matrix->in_bufs = NULL;
		matrix->out_bufs = NULL;
		matrix->spline_bufs = NULL;
		matrix->n_proc_bufs = 0;
	}
	
	return;
}


DATA_MATRIX_d11 *DM_get_matrix_d11(DATA_MATRIX_d11 *matrix, SESSION_m11 *sess, TIME_SLICE_m11 *slice, ...)
{
	extern GLOBALS_m11		*globals_m11;
	TERN_m11			args_read;
	ui1				*data_base, *rng_min_base, *rng_max_base;
	ui1				*new_data_base, *new_rng_min_base, *new_rng_max_base;
	si2				si2_nan;
	si4				search_mode, si4_nan;
	sf4				sf4_nan;
	si8				i, j, req_ref_samps, req_samp_count, old_maj_dim, old_min_dim, old_el_size, old_offset, new_offset;
	si8				ref_num_samps, width, new_start_sample_number, new_end_sample_number;
	sf8 				fc1, fc2, ref_time, ref_samp_secs, ref_samp_freq, proportion, sf8_nan;
	void				*new_data, *new_range_minima, *new_range_maxima;
	size_t				new_data_bytes, curr_bytes, new_bytes, trace_extrema_bytes, num_elements, bytes_per_sample;
	va_list				args;
	CHANNEL_m11			*chan, *ref_chan;
	TIME_SLICE_m11			passed_slice_copy, *req_slice, *sess_slice;
	DM_GET_MATRIX_THREAD_INFO_d11	*gm_thread_infos, *ti;
	
	
	// USAGE:
	// Session must have been opened by caller (so mapping mechanism, active channels, etc. are set), but session is not read.
	// The desired extents are specified in the session TIME_SLICE by the caller.
	// DM_get_matrix_d11() may need to change the time extents of the slice depending on the DM flags and dicontinuities.
	// DM_get_matrix_d11() will call read_session_d11() once the required times have been determined.

	// NOTE:
	// DM_EXTMD_COUNT_AND_FREQ_d11: If the caller wants a fixed number of valid output samples, at a specific output frequency, they should set this flag,
	// and fill in both of these values. DM_get_matrix_d11() will use the slice start time, or start sample number, but adjust the end time if there are
	// discontinuities. The session time slice will reflect what actually occured upon return.
	// DM_EXTMD_COUNT_AND_FREQ_d11 is not compatible with DM_DSCNT_NAN_d11 or DM_DSCNT_ZERO_d11. If these are set the function will return.
	// If discontinuity information is desired with DM_EXTMD_COUNT_AND_FREQ_d11, set DM_DSCNT_CONTIG_d11. This is because DM_EXTMD_COUNT_AND_FREQ_d11
	// implies the caller wants only valid sample values & also, if padding were requested, the number of output samples could be enormous.
	
	// NOTE: This function handles a lot of options. I don't like creating subfunctions with pieces that won't be used anywhere else.
	// As a result, it is behemoth, and somewhat difficult to follow.  Apologies to my fellow coders that need to dig into it.
	
	// DM_get_matrix_d11() varargs: sf8 fc1, sf8 fc2, ui8 flags, si8 out_samp_count, sf8 out_sf
	// varargs DM_FILT_LOWPASS_d11 set: fc1 == high_cutoff
	// varargs DM_FILT_HIGHPASS_d11 set: fc1 == low_cutoff
	// varargs DM_FILT_BANDPASS_d11 set: fc1 == low_cutoff, fc2 == high_cutoff
	// varargs DM_FILT_BANDSTOP_d11 set: fc1 == low_cutoff, fc2 == high_cutoff
	// varargs matrix == NULL: flags, out_samp_count, out_samp_freq are passed (fc1 & fc2 must be filled in, but if not used, 0.0 should be passed as place holders)

	if (sess == NULL) {
		warning_message_m11("%s(): invalid session => returning\n");
		return(NULL);
	}
	if (!(sess->flags & LH_OPEN_m11)) {
		warning_message_m11("%s(): session closed => returning\n");
		return(NULL);
	}
	if (!(sess->flags & LH_RESET_CPS_POINTERS_m11)) {
		warning_message_m11("%s(): reset CPS pointers flag is not set => returning\n");
		return(NULL);
	}

	// allocate matrix structure
	args_read = FALSE_m11;
	if (matrix == NULL) {
		matrix = (DATA_MATRIX_d11 *) calloc_m11((size_t) 1, sizeof(DATA_MATRIX_d11), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		va_start(args, slice);
		fc1 = va_arg(args, sf8);
		fc2 = va_arg(args, sf8);
		matrix->flags = va_arg(args, ui8);
		matrix->sample_count = va_arg(args, si8);
		matrix->sampling_frequency = va_arg(args, sf8);
		va_end(args);
		args_read = TRUE_m11;
	}
	old_maj_dim = matrix->maj_dim;
	old_min_dim = matrix->min_dim;
	old_el_size = matrix->el_size;
		
	// check session slice extents (set by caller)
	passed_slice_copy = *slice;  // passed slice not modified
	req_slice = &passed_slice_copy;
	ref_chan = globals_m11->reference_channel;
	ref_samp_freq = ref_chan->metadata_fps->metadata->time_series_section_2.sampling_frequency;
	switch (matrix->flags & DM_EXTMD_MASK_d11) {
		case DM_EXTMD_SAMP_COUNT_d11:
			if ((search_mode = get_search_mode_m11(slice)) == FALSE_m11)  // ensure there's a valid limit pair
				return(NULL);
			if (matrix->sample_count <= 0) {
				warning_message_m11("%s(): must specify sample count => returning\n");
				return(NULL);
			}
			break;
		case DM_EXTMD_SAMP_FREQ_d11:
			if ((search_mode = get_search_mode_m11(slice)) == FALSE_m11)  // ensure there's a valid limit pair
				return(NULL);
			if (matrix->sample_count <= 0) {
				warning_message_m11("%s(): must specify sampling frequency => returning\n");
				return(NULL);
			}
			break;
		case DM_EXTMD_COUNT_AND_FREQ_d11:
			if (matrix->sample_count <= 0 || matrix->sampling_frequency <= 0.0) {
				warning_message_m11("%s(): must specify both sample count and sampling frequency => returning\n");
				return(NULL);
			}
			if (matrix->flags & (DM_DSCNT_NAN_d11 | DM_DSCNT_ZERO_d11)) {
				warning_message_m11("%s(): DM_EXTMD_SAMP_COUNT_d11 is not compatible with DM_DSCNT_NAN_d11 or DM_DSCNT_ZERO_d11 => returning\n");
				return(NULL);
			}
			if (req_slice->start_sample_number == SAMPLE_NUMBER_NO_ENTRY_m11) {
				if (req_slice->start_time == UUTC_NO_ENTRY_m11) {  // need either start time or sample number
					warning_message_m11("%s(): specify either start time or start sample number => returning\n");
					return(NULL);
				}
				req_slice->start_sample_number = sample_number_for_uutc_m11((LEVEL_HEADER_m11 *) sess, req_slice->start_time, FIND_CURRENT_m11);
			}
			req_ref_samps = (si8) ceil((sf8) matrix->sample_count * (ref_samp_freq / matrix->sampling_frequency));
			req_slice->end_sample_number = req_slice->start_sample_number + (req_ref_samps - 1);
			req_slice->start_time = req_slice->end_time = UUTC_NO_ENTRY_m11;  // force a sample search;
			search_mode = SAMPLE_SEARCH_m11;
			break;
		default:
			warning_message_m11("%s(): invalid extents mode => returning\n");
			return(NULL);
	}
	
	// get requested number of input samples
	if (search_mode == TIME_SEARCH_m11) {
		req_slice->start_sample_number = sample_number_for_uutc_m11((LEVEL_HEADER_m11 *) sess, req_slice->start_time, FIND_CURRENT_m11);
		req_slice->end_sample_number = sample_number_for_uutc_m11((LEVEL_HEADER_m11 *) sess, req_slice->end_time, FIND_CURRENT_m11);
	}
	
	// read session
	// NOTE: session flags should include LH_RESET_CPS_POINTERS_m11 on open for DM functions (flag will be inherited by segments)
	sess_slice = &sess->time_slice;
	*sess_slice = *req_slice;  // read_session_d11() will modify it's slice to available data
	sess_slice->number_of_segments = UNKNOWN_m11;
	read_session_d11(sess, NULL);
	
	// get output sample count & sampling frequency
	ref_num_samps = TIME_SLICE_SAMPLE_COUNT_m11(sess_slice);
	ref_samp_secs = (sf8) ref_num_samps / ref_samp_freq;  // elapsed sample time, ignoring discontinuities
	req_samp_count = matrix->sample_count;  // save this for padded output options
	switch (matrix->flags & DM_EXTMD_MASK_d11) {
		case DM_EXTMD_SAMP_COUNT_d11:
			matrix->sample_count = (si8) round((sf8) matrix->sample_count * ((sf8) ref_num_samps / (sf8) req_ref_samps));  // adjust for not getting as many samples as requested
			matrix->sampling_frequency = (sf8) matrix->sample_count / ref_samp_secs;
			break;
		case DM_EXTMD_SAMP_FREQ_d11:
		case DM_EXTMD_COUNT_AND_FREQ_d11:
			matrix->sample_count = (si8) round(matrix->sampling_frequency * ref_samp_secs);
			break;
	}

	// read filter cutoffs
	if (matrix->flags & DM_FILT_MASK_d11) {
		if (args_read == FALSE_m11 && matrix->filter_low_fc == 0.0 && matrix->filter_high_fc == 0.0) {
			va_start(args, slice);
			fc1 = va_arg(args, sf8);
			if (matrix->flags & (DM_FILT_BANDPASS_d11 | DM_FILT_BANDSTOP_d11))  // 2 cutoffs
				fc2 = va_arg(args, sf8);
			va_end(args);
			args_read = TRUE_m11;
		}
		if (args_read == TRUE_m11) {  // caller passed cutoffs as arguments
			switch (matrix->flags & DM_FILT_MASK_d11) {
				case DM_FILT_LOWPASS_d11:
					matrix->filter_high_fc = fc1;
					break;
				case DM_FILT_HIGHPASS_d11:
					matrix->filter_low_fc = fc1;
					break;
				case DM_FILT_BANDPASS_d11:
				case DM_FILT_BANDSTOP_d11:
					matrix->filter_low_fc = fc1;
					matrix->filter_high_fc = fc2;
					break;
			}
		}
		switch (matrix->flags & DM_FILT_MASK_d11) {
			case DM_FILT_ANTIALIAS_d11:
				matrix->filter_high_fc = matrix->sampling_frequency / FILT_ANTIALIAS_FREQ_DIVISOR_DEFAULT_d11;
			case DM_FILT_LOWPASS_d11:
				matrix->filter_low_fc = NAN;  // nan("");
				break;
			case DM_FILT_HIGHPASS_d11:
				matrix->filter_high_fc = NAN;  // nan("");
				break;
			case DM_FILT_BANDPASS_d11:
			case DM_FILT_BANDSTOP_d11:
				break;
		}
	}
	
	// get active channel count
	for (matrix->channel_count = i = 0; i < sess->number_of_time_series_channels; ++i) {
		chan = sess->time_series_channels[i];
		if (chan->flags & LH_CHANNEL_ACTIVE_m11)
			++matrix->channel_count;
	}
	if (matrix->channel_count == 0) {
		warning_message_m11("%s(): invalid channel count => returning\n");
		return(NULL);
	}

	// get matrix dimensions
	if ((matrix->flags & DM_FMT_MASK_d11) == 0) {  // does not check if multiple flags set
		warning_message_m11("%s(): invalid format => returning\n");
		return(NULL);
	}
	if (matrix->flags & DM_FMT_SAMPLE_MAJOR_d11) {
		matrix->maj_dim = matrix->sample_count;
		matrix->min_dim = matrix->channel_count;
	} else {  // DM_FMT_CHANNEL_MAJOR_d11
		matrix->maj_dim = matrix->channel_count;
		matrix->min_dim = matrix->sample_count;
	}
	
	// get element size
	switch (matrix->flags & DM_TYPE_MASK_d11) {
		case DM_TYPE_SI2_d11:
			matrix->el_size = 2;
			break;
		case DM_TYPE_SI4_d11:
		case DM_TYPE_SF4_d11:
			matrix->el_size = 4;
			break;
		case DM_TYPE_SF8_d11:
			matrix->el_size = 8;
			break;
		default:
			warning_message_m11("%s(): invalid element size => returning\n");
			return(NULL);
	}
	
	// allocate channel processing buffer pointers
	if (matrix->n_proc_bufs < matrix->channel_count) {
		if (matrix->in_bufs == NULL) {
			matrix->in_bufs = (CMP_BUFFERS_m11 **) calloc_m11((size_t) matrix->channel_count, sizeof(CMP_BUFFERS_m11 *), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			matrix->out_bufs = (CMP_BUFFERS_m11 **) calloc_m11((size_t) matrix->channel_count, sizeof(CMP_BUFFERS_m11 *), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			matrix->spline_bufs = (CMP_BUFFERS_m11 **) calloc_m11((size_t) matrix->channel_count, sizeof(CMP_BUFFERS_m11 *), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		} else {
			curr_bytes = matrix->n_proc_bufs * sizeof(CMP_BUFFERS_m11 *);
			new_bytes = matrix->channel_count * sizeof(CMP_BUFFERS_m11 *);
			matrix->in_bufs = (CMP_BUFFERS_m11 **) recalloc_m11(matrix->in_bufs, curr_bytes, new_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			matrix->out_bufs = (CMP_BUFFERS_m11 **) recalloc_m11(matrix->out_bufs, curr_bytes, new_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			matrix->spline_bufs = (CMP_BUFFERS_m11 **) recalloc_m11(matrix->spline_bufs, curr_bytes, new_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		}
		matrix->n_proc_bufs = matrix->channel_count;
	}
	if (matrix->flags & DM_TRACE_EXTREMA_d11) {
		if (matrix->n_proc_bufs < matrix->channel_count || matrix->el_size > old_el_size) {
			if (matrix->trace_minima != NULL) {
				free(matrix->trace_minima);
				free(matrix->trace_maxima);
				matrix->trace_minima = NULL;
			}
		}
		if (matrix->trace_minima == NULL) {
			trace_extrema_bytes = matrix->channel_count * matrix->el_size;
			matrix->trace_minima = malloc_m11(trace_extrema_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			matrix->trace_maxima = malloc_m11(trace_extrema_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		}
	}

	// allocate matrix return contents
	new_data_bytes = matrix->maj_dim * matrix->min_dim * matrix->el_size;
	if (matrix->flags & DM_2D_INDEXING_d11) {
		new_data_bytes += matrix->maj_dim * sizeof(void *);
		if (matrix->maj_dim != old_maj_dim || matrix->min_dim != old_min_dim || matrix->el_size != old_el_size)  // everthing must match
			matrix->data_bytes = 0;  // force failure below
	}
	if (matrix->data_bytes < new_data_bytes) {
		matrix->data_bytes = new_data_bytes;
		if (matrix->data != NULL) {
			free((void *) matrix->data);
			if (matrix->range_minima != NULL) {  // always paired
				free((void *) matrix->range_minima);
				free((void *) matrix->range_maxima);
				matrix->range_minima = matrix->range_maxima = NULL;
			}
		}
		if (matrix->flags & DM_2D_INDEXING_d11)
			matrix->data = (void *) malloc_2D_m11(matrix->maj_dim, matrix->min_dim, matrix->el_size, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		else
			matrix->data = malloc_m11(new_data_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		if (matrix->flags & DM_TRACE_RANGES_d11) {
			if (matrix->flags & DM_2D_INDEXING_d11) {
				matrix->range_minima = (void *) malloc_2D_m11(matrix->maj_dim, matrix->min_dim, matrix->el_size, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
				matrix->range_maxima = (void *) malloc_2D_m11(matrix->maj_dim, matrix->min_dim, matrix->el_size, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			} else {
				matrix->range_minima = malloc_m11(new_data_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
				matrix->range_maxima = malloc_m11(new_data_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			}
		}
	}
		
	// launch channel threads
	ti = gm_thread_infos = (DM_GET_MATRIX_THREAD_INFO_d11 *) calloc_m11((size_t) matrix->channel_count, sizeof(DM_GET_MATRIX_THREAD_INFO_d11), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	for (i = j = 0; i < matrix->channel_count; ++j) {
		chan = sess->time_series_channels[j];
		if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
			ti->dm = matrix;
			ti->chan = chan;
			ti->chan_idx = i++;
			launch_thread_d11(&ti->thread_id, DM_gm_thread_f_d11, (void *) ti, HIGH_PRIORITY_d11, "~0", NULL, FALSE_m11, "DM_gm_thread_f_d11");
			++ti;
		}
	}
	
	// get contigua
	if (matrix->contigua != NULL)
		free((void *) matrix->contigua);
	if (matrix->flags & DM_DSCNT_MASK_d11) {
		build_contigua_m11((LEVEL_HEADER_m11 *) sess);
		matrix->number_of_contigua = sess->number_of_contigua;
		if (matrix->number_of_contigua) {
			matrix->contigua = (CONTIGUON_m11 *) calloc_m11((size_t) matrix->number_of_contigua, sizeof(CONTIGUON_m11), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			memcpy(matrix->contigua, sess->contigua, matrix->number_of_contigua * sizeof(CONTIGUON_m11));
		}
	} else {
		matrix->number_of_contigua = 0;
		matrix->contigua = NULL;
	}
	
	// wait for threads
	ti = gm_thread_infos;
	for (i = matrix->channel_count; i--; ++ti)
		pthread_join_d11(ti->thread_id, NULL);
	free((void *) gm_thread_infos);
	
	// adjust matrices to discontinuity specifications
	if (matrix->flags & DM_DSCNT_MASK_d11) {
		// adjust contigua indices
		if (matrix->number_of_contigua) {
			for (i = 0; i < matrix->number_of_contigua; ++i) {
				proportion = (sf8) (matrix->contigua[i].start_sample_number - sess_slice->start_sample_number) / (sf8) ref_num_samps;
				matrix->contigua[i].start_sample_number = (si8) round((sf8) matrix->sample_count * proportion);
				proportion = (sf8) (matrix->contigua[i].end_sample_number - sess_slice->start_sample_number) / (sf8) ref_num_samps;
				matrix->contigua[i].end_sample_number = (si8) round((sf8) matrix->sample_count * proportion);
			}
			if (matrix->contigua[matrix->number_of_contigua - 1].end_sample_number > (matrix->sample_count - 1))  // correct any rounding error
				matrix->contigua[matrix->number_of_contigua - 1].end_sample_number = matrix->sample_count - 1;
		} else {  // empty slice
			sess_slice->start_time = req_slice->start_time;
			sess_slice->end_time = req_slice->end_time;
			sess_slice->start_sample_number = sess_slice->end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m11;
		}
		// padding options (big pain in the ass)
		if (matrix->flags & (DM_DSCNT_NAN_d11 | DM_DSCNT_ZERO_d11)) {
			// allocate new arrays
			if (matrix->flags & DM_FMT_SAMPLE_MAJOR_d11) {
				matrix->maj_dim = req_samp_count;
				matrix->min_dim = matrix->channel_count;
			} else {  // DM_FMT_CHANNEL_MAJOR_d11
				matrix->maj_dim = matrix->channel_count;
				matrix->min_dim = req_samp_count;
			}
			matrix->data_bytes = matrix->maj_dim * matrix->min_dim * matrix->el_size;
			if (matrix->flags & DM_2D_INDEXING_d11) {
				matrix->data_bytes += matrix->maj_dim * sizeof(void *);
				new_data = (void *) calloc_2D_m11(matrix->maj_dim, matrix->min_dim, matrix->el_size, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			} else {
				new_data = calloc_m11((size_t) matrix->data_bytes, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			}
			if (matrix->flags & DM_TRACE_RANGES_d11) {
				if (matrix->flags & DM_2D_INDEXING_d11) {
					new_range_minima = (void *) calloc_2D_m11(matrix->maj_dim, matrix->min_dim, matrix->el_size, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
					new_range_minima = (void *) calloc_2D_m11(matrix->maj_dim, matrix->min_dim, matrix->el_size, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
				} else {
					new_range_minima = calloc_m11(matrix->data_bytes, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
					new_range_maxima = calloc_m11(matrix->data_bytes, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
				}
			}
			// fill NaN-option arrays with NaNs
			if (matrix->flags & DM_DSCNT_NAN_d11) {
				num_elements = matrix->maj_dim * matrix->min_dim;
				if (matrix->flags & DM_2D_INDEXING_d11) {
					new_data_base = *((ui1 **) new_data);
					if (matrix->flags & DM_TRACE_RANGES_d11) {
						new_rng_min_base = *((ui1 **) new_range_minima);
						new_rng_max_base = *((ui1 **) new_range_maxima);
					}
				} else {
					new_data_base = (ui1 *) new_data;
					if (matrix->flags & DM_TRACE_RANGES_d11) {
						new_rng_min_base = (ui1 *) new_range_minima;
						new_rng_max_base = (ui1 *) new_range_maxima;
					}
				}
				switch (matrix->flags & DM_TYPE_MASK_d11) {
					case DM_TYPE_SI2_d11:
						si2_nan = NAN_SI2_m11;
						memset_m11(new_data_base, (void *) &si2_nan, 2, num_elements << 1);
						if (matrix->flags & DM_TRACE_RANGES_d11) {
							memset_m11(new_rng_min_base, (void *) &si2_nan, 2, num_elements << 1);
							memset_m11(new_rng_max_base, (void *) &si2_nan, 2, num_elements << 1);
						}
						break;
					case DM_TYPE_SI4_d11:
						si4_nan = NAN_SI4_m11;
						memset_m11(new_data_base, (void *) &si4_nan, 4, num_elements << 2);
						if (matrix->flags & DM_TRACE_RANGES_d11) {
							memset_m11(new_rng_min_base, (void *) &si4_nan, 4, num_elements << 2);
							memset_m11(new_rng_max_base, (void *) &si4_nan, 4, num_elements << 2);
						}
						break;
					case DM_TYPE_SF4_d11:
						sf4_nan = NAN;
						memset_m11(new_data_base, (void *) &sf4_nan, 4, num_elements << 2);
						if (matrix->flags & DM_TRACE_RANGES_d11) {
							memset_m11(new_rng_min_base, (void *) &sf4_nan, 4, num_elements << 2);
							memset_m11(new_rng_max_base, (void *) &sf4_nan, 4, num_elements << 2);
						}
						break;
					case DM_TYPE_SF8_d11:
						sf8_nan = NAN;
						memset_m11(new_data_base, (void *) &sf8_nan, 8, num_elements << 3);
						if (matrix->flags & DM_TRACE_RANGES_d11) {
							memset_m11(new_rng_min_base, (void *) &sf8_nan, 8, num_elements << 3);
							memset_m11(new_rng_max_base, (void *) &sf8_nan, 8, num_elements << 3);
						}
						break;
				}
			}  // END fill NaN-option arrays with NaNs
			if (matrix->flags & DM_2D_INDEXING_d11) {
				data_base = *((ui1 **) matrix->data);
				new_data_base = *((ui1 **) new_data);
				if (matrix->flags & DM_TRACE_RANGES_d11) {
					rng_min_base = *((ui1 **) matrix->range_minima);
					new_rng_min_base = *((ui1 **) new_range_minima);
					rng_max_base = *((ui1 **) matrix->range_maxima);
					new_rng_max_base = *((ui1 **) new_range_maxima);
				}
			} else {
				data_base = (ui1 *) matrix->data;
				new_data_base = (ui1 *) new_data;
				if (matrix->flags & DM_TRACE_RANGES_d11) {
					rng_min_base = (ui1 *) matrix->range_minima;
					new_rng_min_base = (ui1 *) new_range_minima;
					rng_max_base = (ui1 *) matrix->range_maxima;
					new_rng_max_base = (ui1 *) new_range_maxima;
				}
			}
			// copy data & adjust contigua indicess
			ref_time = (sf8) TIME_SLICE_DURATION_m11(req_slice);
			for (i = 0; i < matrix->number_of_contigua; ++i) {
				proportion = (sf8) (matrix->contigua[i].start_time - req_slice->start_time) / (sf8) ref_time;
				width = matrix->contigua[i].end_sample_number - matrix->contigua[i].start_sample_number;
				new_start_sample_number = (si8) round((sf8) req_samp_count * proportion);
				new_end_sample_number = new_start_sample_number + width;
				if (new_end_sample_number >= matrix->sample_count)  // correct any rounding error
					new_end_sample_number = matrix->sample_count - 1;
				// copy
				if (matrix->flags & DM_FMT_SAMPLE_MAJOR_d11) {
					bytes_per_sample = matrix->channel_count * matrix->el_size;
					old_offset = bytes_per_sample * matrix->contigua[i].start_sample_number;
					new_offset = bytes_per_sample * new_start_sample_number;
					memcpy((void *) (new_data_base + new_offset), (void *) (data_base + old_offset), (size_t)  (bytes_per_sample * width));
					if (matrix->flags & DM_TRACE_RANGES_d11) {
						memcpy((void *) (new_rng_min_base + new_offset), (void *) (rng_min_base + old_offset), (size_t)  (bytes_per_sample * width));
						memcpy((void *) (new_rng_max_base + new_offset), (void *) (rng_max_base + old_offset), (size_t)  (bytes_per_sample * width));
					}
				} else {  // DM_FMT_CHANNEL_MAJOR_d11
					bytes_per_sample = matrix->el_size;
					for (j = 0; j < matrix->channel_count; ++j) {
						old_offset = bytes_per_sample * ((j * matrix->sample_count) + matrix->contigua[i].start_sample_number);
						new_offset = bytes_per_sample * ((j * req_samp_count) + new_start_sample_number);
						memcpy((void *) (new_data_base + new_offset), (void *) (data_base + old_offset), (size_t)  (bytes_per_sample * width));
						if (matrix->flags & DM_TRACE_RANGES_d11) {
							memcpy((void *) (new_rng_min_base + new_offset), (void *) (rng_min_base + old_offset), (size_t)  (bytes_per_sample * width));
							memcpy((void *) (new_rng_max_base + new_offset), (void *) (rng_max_base + old_offset), (size_t)  (bytes_per_sample * width));
						}
					}
				}
				matrix->contigua[i].start_sample_number = new_start_sample_number;
				matrix->contigua[i].end_sample_number = new_end_sample_number;
			}
			matrix->sample_count = req_samp_count;
			// clean up
			free((void *) matrix->data);
			matrix->data = new_data;
			if (matrix->flags & DM_TRACE_RANGES_d11) {
				free((void *) matrix->range_minima);
				matrix->range_minima = new_range_minima;
				free((void *) matrix->range_maxima);
				matrix->range_maxima = new_range_maxima;
			}
		}  // END padding options
	}  // END matrix->flags & DM_DSCNT_MASK_d11

	return(matrix);
}


pthread_rval_d11	DM_gm_thread_f_d11(void *ptr)
{
	TERN_m11			trace_ranges;
	ui1				*data_base, *min_base, *max_base;
	si2				*si2_p1, *si2_p2, *si2_p3;
	si4				i, filt_type, *seg_samps, n_cutoffs, order, filt_poles, pad_samps, seg_offset, bint_mode;
	si4				*si4_p1, *si4_p2, *si4_p3;
	sf4				*sf4_p1, *sf4_p2, *sf4_p3;
	si8				j, k, chan_idx, n_raw_samps, n_seg_samps, required_in_buf_len, chan_offset, samp_offset;
	size_t				n_out_bufs, maj_ptr_bytes;
	sf8				*raw_samps, *rsp, raw_samp_freq, cutoff_ratio, sf_ratio, b, m, q;
	sf8				fc1, fc2, *sf8_p1, *sf8_p2, *sf8_p3, *sf8_p4, *sf8_p5, *sf8_p6;
	sf8				*out_buf, *out_mins, *out_maxs, trace_min, trace_max;
	CHANNEL_m11			*chan;
	SEGMENT_m11			*seg;
	TIME_SLICE_m11			*slice;
	CMP_PROCESSING_STRUCT_m11	*cps;
	FILT_PROCESSING_STRUCT_d11      *filtps;
	DATA_MATRIX_d11			*dm;
	DM_GET_MATRIX_THREAD_INFO_d11	*ti;

		
	ti = (DM_GET_MATRIX_THREAD_INFO_d11 *) ptr;
	dm = ti->dm;
	chan = ti->chan;
	chan_idx = ti->chan_idx;
	slice = &chan->time_slice;
	raw_samp_freq = chan->metadata_fps->metadata->time_series_section_2.sampling_frequency;
	n_raw_samps = TIME_SLICE_SAMPLE_COUNT_m11(slice);
	
	// Note: trace ranges are rarely used for long: so don't leave memory allocated if not needed
	if (dm->flags & DM_TRACE_RANGES_d11) {
		trace_ranges = TRUE_m11;
	} else {
		trace_ranges = FALSE_m11;
		if (dm->out_bufs[chan_idx] != NULL)
			if (dm->out_bufs[chan_idx]->n_buffers != 1)
				CMP_free_buffers_m11(dm->out_bufs[chan_idx], FALSE_m11);
	}

	// set up for filtering
	required_in_buf_len = n_raw_samps;
	if (dm->flags & DM_FILT_MASK_d11) {
		switch (dm->flags & DM_FILT_MASK_d11) {
			case DM_FILT_ANTIALIAS_d11:
			case DM_FILT_LOWPASS_d11:
				filt_type = FILT_LOWPASS_TYPE_d11;
				fc1 = dm->filter_high_fc;
				fc2 = 0.0;
				n_cutoffs = 1;
				break;
			case DM_FILT_HIGHPASS_d11:
				filt_type = FILT_HIGHPASS_TYPE_d11;
				fc1 = dm->filter_low_fc;
				fc2 = 0.0;
				n_cutoffs = 1;
				break;
			case DM_FILT_BANDPASS_d11:
				filt_type = FILT_BANDPASS_TYPE_d11;
				fc1 = dm->filter_low_fc;
				fc2 = dm->filter_high_fc;
				n_cutoffs = 2;
				break;
			case DM_FILT_BANDSTOP_d11:
				filt_type = FILT_BANDSTOP_TYPE_d11;
				fc1 = dm->filter_low_fc;
				fc2 = dm->filter_high_fc;
				n_cutoffs = 2;
				break;
		}
		cutoff_ratio = fc1 / raw_samp_freq;
		if (cutoff_ratio >= (sf8) 3.14e-05)  // empirically determined
			order = 4;
		else
			order = 3;
		filt_poles = FILT_POLES_d11(order, n_cutoffs);
		pad_samps = FILT_FILT_PAD_SAMPLES_d11(filt_poles);
		required_in_buf_len += pad_samps;
	}
	
	// allocate processing buffers
	dm->in_bufs[chan_idx] = CMP_allocate_buffers_m11(dm->in_bufs[chan_idx], 3, required_in_buf_len, sizeof(sf8), FALSE_m11, FALSE_m11);
	n_out_bufs = 1;
	if (trace_ranges == TRUE_m11)
		n_out_bufs = 3;
	dm->out_bufs[chan_idx] = CMP_allocate_buffers_m11(dm->out_bufs[chan_idx], n_out_bufs, dm->sample_count, sizeof(sf8), FALSE_m11, FALSE_m11);

	// initialize filter
	if (dm->flags & DM_FILT_MASK_d11) {
		filtps = FILT_initialize_processing_struct_d11(order, filt_type, raw_samp_freq, n_raw_samps, FALSE_m11, FALSE_m11, FALSE_m11, (SUPPRESS_ALL_OUTPUT_m11 | RETURN_ON_FAIL_m11), fc1, fc2);
		filtps->filt_data = dm->in_bufs[chan_idx]->buffer[1];
		filtps->buffer = dm->in_bufs[chan_idx]->buffer[2];
		if (trace_ranges == TRUE_m11)  // need a copy of raw data for trace ranges
			filtps->orig_data = dm->in_bufs[chan_idx]->buffer[0];
		else  // put data directly into filt_data array to skip initial copy in FILT_filtfilt_d10()
			filtps->orig_data = FILT_OFFSET_ORIG_DATA_d11(filtps);
		raw_samps = filtps->orig_data;
	} else {  // no filtering
		raw_samps = dm->in_bufs[chan_idx]->buffer[0];
	}
	
	// put segmented channel si4 data into a single sf8 array
	rsp = raw_samps;
	seg_offset = get_segment_offset_m11((LEVEL_HEADER_m11 *) chan);
	for (i = 0, j = seg_offset; i < slice->number_of_segments; ++i, ++j) {
		seg = chan->segments[j];
		cps = seg->time_series_data_fps->parameters.cps;
		seg_samps = cps->decompressed_data;
		n_seg_samps = TIME_SLICE_SAMPLE_COUNT_m11((&seg->time_slice));
		for (k = n_seg_samps; k--;)
			*rsp++ = (sf8) *seg_samps++;
	}

	// filter
	if (dm->flags & DM_FILT_MASK_d11) {
		FILT_filtfilt_d11(filtps);
		FILT_free_processing_struct_d11(filtps, FALSE_m11, FALSE_m11, FALSE_m11, TRUE_m11);
	}
	
	// set up output buffers
	if ((dm->flags & DM_FMT_CHANNEL_MAJOR_d11) && (dm->flags & DM_FMT_CHANNEL_MAJOR_d11)) {
		// special case - put results directly in output array
		chan_offset = chan_idx * dm->sample_count;
		if (dm->flags & DM_2D_INDEXING_d11) {
			out_buf = *((sf8 **) dm->data) + chan_offset;
			if (trace_ranges == TRUE_m11) {
				out_mins =  *((sf8 **) dm->range_minima) + chan_offset;
				out_maxs =  *((sf8 **) dm->range_maxima) + chan_offset;
			}
		} else {
			out_buf = (sf8 *) dm->data + chan_offset;
			if (trace_ranges == TRUE_m11) {
				out_mins = (sf8 *) dm->range_minima + chan_offset;
				out_maxs = (sf8 *) dm->range_maxima + chan_offset;
			}
		}
	} else {
		out_buf = dm->out_bufs[chan_idx]->buffer[0];
		if (trace_ranges == TRUE_m11) {
			out_mins = dm->out_bufs[chan_idx]->buffer[1];
			out_maxs = dm->out_bufs[chan_idx]->buffer[2];
		}
	}
	
	// interpolate
	switch (dm->flags & DM_INTRP_BINTRP_MASK_d1) {
		case DM_INTRP_BINTRP_MDPT_d11:
			bint_mode = CMP_CENT_MODE_MIDPOINT_d11;
			break;
		case DM_INTRP_BINTRP_MEAN_d11:
			bint_mode = CMP_CENT_MODE_MEAN_d11;
			break;
		case DM_INTRP_BINTRP_MEDN_d11:
			bint_mode = CMP_CENT_MODE_MEDIAN_d11;
			break;
		case DM_INTRP_BINTRP_FAST_d11:
			bint_mode = CMP_CENT_MODE_FASTEST_d11;
			break;
		default:  // linear or spline modes selected: binterpolate() just used for trace_ranges
			bint_mode = CMP_CENT_MODE_NONE_d11;
			break;
	}
	switch (dm->flags & DM_INTRP_MASK_d11) {
		case DM_INTRP_SPLINE_d11:
			dm->spline_bufs[chan_idx] = CMP_allocate_buffers_m11(dm->spline_bufs[chan_idx], 3, n_raw_samps + CMP_SPLINE_TAIL_LEN_m11, sizeof(sf8), FALSE_m11, FALSE_m11);
			CMP_spline_interp_sf8_m11(raw_samps, n_raw_samps, out_buf, dm->sample_count, dm->spline_bufs[chan_idx]);
			if (trace_ranges == TRUE_m11)
				CMP_binterpolate_sf8_d11(raw_samps, n_raw_samps, NULL, dm->sample_count, bint_mode, trace_ranges, out_mins, out_maxs);
			break;
		case DM_INTRP_LINEAR_d11:
			CMP_lin_interp_sf8_m11(raw_samps, n_raw_samps, out_buf, dm->sample_count);
			if (trace_ranges == TRUE_m11)
				CMP_binterpolate_sf8_d11(raw_samps, n_raw_samps, NULL, dm->sample_count, bint_mode, trace_ranges, out_mins, out_maxs);
			break;
		case DM_INTRP_BINTRP_MDPT_d11:
		case DM_INTRP_BINTRP_MEAN_d11:
		case DM_INTRP_BINTRP_MEDN_d11:
		case DM_INTRP_BINTRP_FAST_d11:
			CMP_binterpolate_sf8_d11(raw_samps, n_raw_samps, out_buf, dm->sample_count, bint_mode, trace_ranges, out_mins, out_maxs);
			break;
		case DM_INTRP_UP_SPLINE_DN_LINEAR_d11:  // default
		default:
			sf_ratio = dm->sampling_frequency / raw_samp_freq;
			if (sf_ratio >= DM_INTRP_SPLINE_UPSAMPLE_SF_RATIO_d11) {
				dm->spline_bufs[chan_idx] = CMP_allocate_buffers_m11(dm->spline_bufs[chan_idx], 3, n_raw_samps + CMP_SPLINE_TAIL_LEN_m11, sizeof(sf8), FALSE_m11, FALSE_m11);
				CMP_spline_interp_sf8_m11(raw_samps, n_raw_samps, out_buf, dm->sample_count, dm->spline_bufs[chan_idx]);
			} else {
				CMP_lin_interp_sf8_m11(raw_samps, n_raw_samps, out_buf, dm->sample_count);
			}
			if (trace_ranges == TRUE_m11)
				CMP_binterpolate_sf8_d11(raw_samps, n_raw_samps, NULL, dm->sample_count, bint_mode, trace_ranges, out_mins, out_maxs);
			break;
	}
			
	// detrend
	if (dm->flags & DM_DETREND_d11) {
		CMP_lad_reg_sf8_d11(out_buf, dm->sample_count, &m, &b);
		sf8_p1 = out_buf;
		i = dm->sample_count;
		if (trace_ranges == TRUE_m11) {
			sf8_p2 = out_mins;
			sf8_p3 = out_maxs;
			while (i--) {
				*sf8_p1++ -= (q = (b += m));
				*sf8_p2++ -= q;
				*sf8_p3++ -= q;
			}
		} else {
			while (i--)
				*sf8_p1++ -= (b += m);
		}
	}
	
	// trace extrema
	if (dm->flags & DM_TRACE_EXTREMA_d11) {
		sf8_p1 = out_buf;
		i = dm->sample_count - 1;
		trace_min = trace_max = *sf8_p1++;
		for (i = dm->sample_count - 1; i--; ++sf8_p1) {
			if (trace_min > *sf8_p1)
				trace_min = *sf8_p1;
			else if (trace_max < *sf8_p1)
				trace_max = *sf8_p1;
		}
		switch (dm->flags & DM_TYPE_MASK_d11) {
			case DM_TYPE_SI2_d11:
				((si2 *) dm->trace_minima)[chan_idx] = CMP_round_si2_m11(trace_min);
				((si2 *) dm->trace_maxima)[chan_idx] = CMP_round_si2_m11(trace_max);
				break;
			case DM_TYPE_SI4_d11:
				((si4 *) dm->trace_minima)[chan_idx] = CMP_round_si4_m11(trace_min);
				((si4 *) dm->trace_maxima)[chan_idx] = CMP_round_si4_m11(trace_max);
				break;
			case DM_TYPE_SF4_d11:
				((sf4 *) dm->trace_minima)[chan_idx] = (sf4) trace_min;
				((sf4 *) dm->trace_maxima)[chan_idx] = (sf4) trace_max;
				break;
			case DM_TYPE_SF8_d11:
				((sf8 *) dm->trace_minima)[chan_idx] = trace_min;
				((sf8 *) dm->trace_maxima)[chan_idx] = trace_max;
				break;
			default:
				warning_message_m11("%s(): invalid element size => returning\n");
				return((pthread_rval_d11) 0);
		}
	}

	// put data in target arrays
	data_base = (ui1 *) dm->data;
	min_base = (ui1 *) dm->range_minima;
	max_base = (ui1 *) dm->range_maxima;
	if (dm->flags & DM_2D_INDEXING_d11) {
		maj_ptr_bytes = dm->maj_dim * sizeof(void *);
		data_base += maj_ptr_bytes;
		min_base += maj_ptr_bytes;
		max_base += maj_ptr_bytes;
	}
	if (dm->flags & DM_FMT_CHANNEL_MAJOR_d11)
		chan_offset = chan_idx * dm->sample_count;
	else  // DM_FMT_SAMPLE_MAJOR_d11
		samp_offset = dm->channel_count;
		
	switch (dm->flags & DM_TYPE_MASK_d11) {
		case DM_TYPE_SI2_d11:
			if (dm->flags & DM_FMT_CHANNEL_MAJOR_d11) {
				sf8_p1 = out_buf;
				si2_p1 = (si2 *) data_base + chan_offset;
				i = dm->sample_count;
				if (trace_ranges == TRUE_m11) {
					sf8_p2 = out_mins;
					si2_p2 = (si2 *) min_base + chan_offset;
					sf8_p3 = out_maxs;
					si2_p3 = (si2 *) max_base + chan_offset;
					while (i--) {
						*si2_p1++ = CMP_round_si2_m11(*sf8_p1++);
						*si2_p2++ = CMP_round_si2_m11(*sf8_p2++);
						*si2_p3++ = CMP_round_si2_m11(*sf8_p3++);
					}
				} else {
					while (i--)
						*si2_p1++ = CMP_round_si2_m11(*sf8_p1++);
				}
			} else {  // DM_FMT_SAMPLE_MAJOR_d11
				sf8_p1 = out_buf;
				si2_p1 = ((si2 *) data_base + chan_idx) - samp_offset;
				i = dm->sample_count;
				if (trace_ranges == TRUE_m11) {
					sf8_p2 = out_mins;
					si2_p2 = (si2 *) min_base + chan_idx - samp_offset;
					sf8_p3 = out_maxs;
					si2_p3 = (si2 *) max_base + chan_idx - samp_offset;
					while (i--) {
						*(si2_p1 += samp_offset) = CMP_round_si2_m11(*sf8_p1++);
						*(si2_p2 += samp_offset) = CMP_round_si2_m11(*sf8_p2++);
						*(si2_p3 += samp_offset) = CMP_round_si2_m11(*sf8_p3++);
					}
				} else {
					while (i--)
						*(si2_p1 += samp_offset) = CMP_round_si2_m11(*sf8_p1++);
				}

			}
			break;
		case DM_TYPE_SI4_d11:
			if (dm->flags & DM_FMT_CHANNEL_MAJOR_d11) {
				sf8_p1 = out_buf;
				si4_p1 = (si4 *) data_base + chan_offset;
				i = dm->sample_count;
				if (trace_ranges == TRUE_m11) {
					sf8_p2 = out_mins;
					si4_p2 = (si4 *) min_base + chan_offset;
					sf8_p3 = out_maxs;
					si4_p3 = (si4 *) max_base + chan_offset;
					while (i--) {
						*si4_p1++ = CMP_round_si4_m11(*sf8_p1++);
						*si4_p2++ = CMP_round_si4_m11(*sf8_p2++);
						*si4_p3++ = CMP_round_si4_m11(*sf8_p3++);
					}
				} else {
					while (i--)
						*si4_p1++ = CMP_round_si2_m11(*sf8_p1++);
				}
			} else {  // DM_FMT_SAMPLE_MAJOR_d11
				sf8_p1 = out_buf;
				si4_p1 = ((si4 *) data_base + chan_idx) - samp_offset;
				i = dm->sample_count;
				if (trace_ranges == TRUE_m11) {
					sf8_p2 = out_mins;
					si4_p2 = (si4 *) min_base + chan_idx - samp_offset;
					sf8_p3 = out_maxs;
					si4_p3 = (si4 *) max_base + chan_idx - samp_offset;
					while (i--) {
						*(si4_p1 += samp_offset) = CMP_round_si4_m11(*sf8_p1++);
						*(si4_p2 += samp_offset) = CMP_round_si4_m11(*sf8_p2++);
						*(si4_p3 += samp_offset) = CMP_round_si4_m11(*sf8_p3++);
					}
				} else {
					while (i--)
						*(si4_p1 += samp_offset) = CMP_round_si4_m11(*sf8_p1++);
				}

			}
			break;
		case DM_TYPE_SF4_d11:
			if (dm->flags & DM_FMT_CHANNEL_MAJOR_d11) {
				sf8_p1 = out_buf;
				sf4_p1 = (sf4 *) data_base + chan_offset;
				i = dm->sample_count;
				if (trace_ranges == TRUE_m11) {
					sf8_p2 = out_mins;
					sf4_p2 = (sf4 *) min_base + chan_offset;
					sf8_p3 = out_maxs;
					sf4_p3 = (sf4 *) max_base + chan_offset;
					while (i--) {
						*sf4_p1++ = (sf4) *sf8_p1++;
						*sf4_p2++ = (sf4) *sf8_p2++;
						*sf4_p3++ = (sf4) *sf8_p3++;
					}
				} else {
					while (i--)
						*sf4_p1++ = (sf4) *sf8_p1++;
				}
			} else {  // DM_FMT_SAMPLE_MAJOR_d11
				sf8_p1 = out_buf;
				sf4_p1 = ((sf4 *) data_base + chan_idx) - samp_offset;
				i = dm->sample_count;
				if (trace_ranges == TRUE_m11) {
					sf8_p2 = out_mins;
					sf4_p2 = (sf4 *) min_base + chan_idx - samp_offset;
					sf8_p3 = out_maxs;
					sf4_p3 = (sf4 *) max_base + chan_idx - samp_offset;
					while (i--) {
						*(sf4_p1 += samp_offset) = (sf4) *sf8_p1++;
						*(sf4_p2 += samp_offset) = (sf4) *sf8_p2++;
						*(sf4_p3 += samp_offset) = (sf4) *sf8_p3++;
					}
				} else {
					while (i--)
						*(sf4_p1 += samp_offset) = (sf4) *sf8_p1++;
				}

			}
			break;
		case DM_TYPE_SF8_d11:
			// DM_FMT_CHANNEL_MAJOR_d11 written directly into output buffers
			if (dm->flags & DM_FMT_SAMPLE_MAJOR_d11) {
				sf8_p1 = out_buf;
				sf8_p4 = ((sf8 *) data_base + chan_idx) - samp_offset;
				i = dm->sample_count;
				if (trace_ranges == TRUE_m11) {
					sf8_p2 = out_mins;
					sf8_p5 = (sf8 *) min_base + chan_idx - samp_offset;
					sf8_p3 = out_maxs;
					sf8_p6 = (sf8 *) max_base + chan_idx - samp_offset;
					while (i--) {
						*(sf8_p4 += samp_offset) = *sf8_p1++;
						*(sf8_p5 += samp_offset) = *sf8_p2++;
						*(sf8_p6 += samp_offset) = *sf8_p3++;
					}
				} else {
					while (i--)
						*(sf8_p4 += samp_offset) = *sf8_p1++;
				}
			}
			break;
	}

	return((pthread_rval_d11) 0);
}


DATA_MATRIX_d11 *DM_transpose_d11(DATA_MATRIX_d11 *in_matrix, DATA_MATRIX_d11 *out_matrix)
{
	DATA_MATRIX_d11 	*tmp_matrix;
	
	
	// if in_matrix == out_matrix, done in place; if out_matrix == NULL, allocated and returned)

	tmp_matrix = (DATA_MATRIX_d11 *) calloc_m11((size_t) 1, sizeof(DATA_MATRIX_d11), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	tmp_matrix->channel_count = in_matrix->channel_count;
	tmp_matrix->sample_count = in_matrix->sample_count;
	tmp_matrix->filter_low_fc = in_matrix->filter_low_fc;
	tmp_matrix->filter_low_fc = in_matrix->filter_low_fc;
	tmp_matrix->flags = in_matrix->flags;
	tmp_matrix->maj_dim = in_matrix->min_dim;
	tmp_matrix->min_dim = in_matrix->maj_dim;
	tmp_matrix->el_size = in_matrix->el_size;
	if (tmp_matrix->flags & DM_2D_INDEXING_d11) {
		tmp_matrix->data_bytes = tmp_matrix->maj_dim * tmp_matrix->min_dim * tmp_matrix->el_size;
		tmp_matrix->data_bytes += tmp_matrix->maj_dim * sizeof(void *);
		tmp_matrix->data = (void *) malloc_2D_m11(tmp_matrix->maj_dim, tmp_matrix->min_dim, tmp_matrix->el_size, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	} else {
		tmp_matrix->data_bytes = in_matrix->data_bytes;
		tmp_matrix->data = malloc(tmp_matrix->data_bytes);
	}
	if (tmp_matrix->flags & DM_TRACE_RANGES_d11) {
		if (tmp_matrix->flags & DM_2D_INDEXING_d11) {
			tmp_matrix->range_minima = (void *) malloc_2D_m11(tmp_matrix->maj_dim, tmp_matrix->min_dim, tmp_matrix->el_size, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
			tmp_matrix->range_maxima = (void *) malloc_2D_m11(tmp_matrix->maj_dim, tmp_matrix->min_dim, tmp_matrix->el_size, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		} else {
			tmp_matrix->range_minima = malloc(tmp_matrix->data_bytes);
			tmp_matrix->range_maxima = malloc(tmp_matrix->data_bytes);
		}
	}
	if (tmp_matrix->flags & DM_TRACE_EXTREMA_d11) {
		tmp_matrix->trace_minima = malloc((size_t) (tmp_matrix->channel_count * in_matrix->el_size));
		memcpy((void *) tmp_matrix->trace_minima, (void *) in_matrix->trace_minima, (size_t) (tmp_matrix->channel_count * tmp_matrix->el_size));
		tmp_matrix->trace_maxima = malloc((size_t) (in_matrix->channel_count * in_matrix->el_size));
		memcpy((void *) tmp_matrix->trace_maxima, (void *) in_matrix->trace_maxima, (size_t) (tmp_matrix->channel_count * tmp_matrix->el_size));
	}
	tmp_matrix->number_of_contigua = in_matrix->number_of_contigua;
	if (tmp_matrix->number_of_contigua)
		memcpy((void *) tmp_matrix->contigua, (void *) in_matrix->contigua, (size_t) (tmp_matrix->number_of_contigua * sizeof(CONTIGUON_m11)));
	tmp_matrix->n_proc_bufs = 0;  // don't reallocate processing buffers: DM_get_matrix_d11() will do it if it's called
	
	// transpose
	DM_transpose_arrays_d11(in_matrix, tmp_matrix, in_matrix->data, tmp_matrix->data);
	if (tmp_matrix->flags & DM_TRACE_RANGES_d11) {
		DM_transpose_arrays_d11(in_matrix, tmp_matrix, in_matrix->range_minima, tmp_matrix->range_minima);
		DM_transpose_arrays_d11(in_matrix, tmp_matrix, in_matrix->range_maxima, tmp_matrix->range_maxima);
	}
	
	// return
	if (out_matrix == NULL) {
		out_matrix = tmp_matrix;
	} else {
		DM_free_matrix_d11(out_matrix, FALSE_m11);
		*out_matrix = *tmp_matrix;
	}
	return(out_matrix);
}


void	DM_transpose_arrays_d11(DATA_MATRIX_d11 *in_matrix, DATA_MATRIX_d11 *out_matrix, void *in_base, void *out_base)
{
	si2	*si2_p1, *si2_p2;
	si4	*si4_p1, *si4_p2;
	sf4	*sf4_p1, *sf4_p2;
	sf8	*sf8_p1, *sf8_p2;
	si8	i, j, out_min_dim, out_maj_dim;
	
	
	// this function is used internally in DM_tranpose_d11()
	
	out_min_dim = out_matrix->min_dim;
	out_maj_dim = out_matrix->maj_dim;
	if (out_matrix->flags & DM_2D_INDEXING_d11) {
		in_base = (void *) ((ui1 *) in_base + (in_matrix->maj_dim * sizeof(void *)));
		out_base = (void *) ((ui1 *) out_base + (out_matrix->maj_dim * sizeof(void *)));
	}
	
	switch (out_matrix->flags & DM_TYPE_MASK_d11) {
		case DM_TYPE_SI2_d11:
			si2_p1 = (si2 *) in_base;
			for (i = 0; i < out_min_dim; ++i) {
				si2_p2 = (si2 *) out_base + i;
				for (j = out_maj_dim; j--; si2_p2 += out_min_dim)
					*si2_p2 = *si2_p1++;
			}
			break;
		case DM_TYPE_SI4_d11:
			si4_p1 = (si4 *) in_base;
			for (i = 0; i < out_min_dim; ++i) {
				si4_p2 = (si4 *) out_base + i;
				for (j = out_maj_dim; j--; si4_p2 += out_min_dim)
					*si4_p2 = *si4_p1++;
			}
			break;
		case DM_TYPE_SF4_d11:
			sf4_p1 = (sf4 *) in_base;
			for (i = 0; i < out_min_dim; ++i) {
				sf4_p2 = (sf4 *) out_base + i;
				for (j = out_maj_dim; j--; sf4_p2 += out_min_dim)
					*sf4_p2 = *sf4_p1++;
			}
			break;
		case DM_TYPE_SF8_d11:
			sf8_p1 = (sf8 *) in_base;
			for (i = 0; i < out_min_dim; ++i) {
				sf8_p2 = (sf8 *) out_base + i;
				for (j = out_maj_dim; j--; sf8_p2 += out_min_dim)
					*sf8_p2 = *sf8_p1++;
			}
			break;
	}
	
	return;
}

//***********************************************************************//
//**************************  FILTER FUNCTIONS  *************************//
//***********************************************************************//


// ATTRIBUTION
//
// Some of the filter code was adapted from Matlab functions.
// MathWorks Inc. (www.mathworks.com)
//
// The c code in this library was written entirely from scratch.
//
// NOTE: This code requres long double (sf16) math.
// It may require an explicit compiler instruction to implement true long floating point math.
// in icc: "-Qoption,cpp,--extended_float_types"


void	FILT_balance_d11(sf16 **a, si4 poles)
{
	sf16    radix, sqrdx, c, r, g, f, s;
	si4     i, j, done;
	
	
	radix = (sf16) FILT_RADIX_d11;
	sqrdx = radix * radix;
	done = 0;
	while (!done) {
		done = 1;
		for (i = 0; i < poles; i++) {
			r = c = FILT_ZERO_d11;
			for (j = 0; j < poles; j++)
				if (j != i) {
					c += FILT_ABS_d11(a[j][i]);
					r += FILT_ABS_d11(a[i][j]);
				}
			if (c != FILT_ZERO_d11 && r != FILT_ZERO_d11) {
				g = r / radix;
				f = FILT_ONE_d11;
				s = c + r;
				while (c < g) {
					f *= radix;
					c *= sqrdx;
				}
				g = r * radix;
				while (c > g) {
					f /= radix;
					c /= sqrdx;
				}
				if (((c + r) / f) < ((sf16) 0.95 * s)) {
					done = 0;
					g = 1.0 / f;
					for (j = 0; j < poles; j++)
						a[i][j] *= g;
					for (j = 0; j < poles; j++)
						a[j][i] *= f;
				}
			}
		}
	}
      
	
	return;
}


si4	FILT_butter_d11(FILT_PROCESSING_STRUCT_d11 *filtps)
{
	si4			i, j, n_fcs, offset, idx, order, poles, is_odd;
	sf8 			*d_num, *d_den;
	sf16			samp_freq, fcs[2], *den, sum_num, sum_den;
	sf16			u[2], pi, half_pi, bw, wn, w, *r, *num, ratio;
	sf16            	**a, **inv_a, **ta1, **ta2, *b, *bt, *c, t;
	FILT_LONG_COMPLEX_d11	csum_num, csum_den, cratio, *ckern;
	FILT_LONG_COMPLEX_d11	*p, tc, *eigs, *cden, *rc, *cnum;

	
	// check input
	switch(filtps->type) {
		case FILT_LOWPASS_TYPE_d11:
		case FILT_BANDPASS_TYPE_d11:
		case FILT_HIGHPASS_TYPE_d11:
		case FILT_BANDSTOP_TYPE_d11:
			break;
		default:
			if (!(filtps->behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11))
				error_message_m11("Unrecognized filter type: %d [function %s, line %d]\n", filtps->type, __FUNCTION__, __LINE__);
			if (filtps->behavior_on_fail & EXIT_ON_FAIL_m11)
				exit_m11(1);
			return(-1);
	}
	samp_freq = (sf16) filtps->sampling_frequency;
	fcs[0] = (sf16) filtps->cutoffs[0];
	n_fcs = ((filtps->type == FILT_LOWPASS_TYPE_d11) || (filtps->type == FILT_HIGHPASS_TYPE_d11)) ? 1 : 2;
	if (n_fcs == 2)
		fcs[1] = (sf16) filtps->cutoffs[1];
	order = filtps->order;
	filtps->n_poles = poles = n_fcs * order;
	is_odd = order % 2;
	
	// step 1: get analog, pre-warped frequencies
	pi = (sf16) M_PI;
	half_pi = pi / (sf16) 2.0;
	for (i = 0; i < n_fcs; ++i)
		u[i] = (sf16) 4.0 * tanl((pi * fcs[i]) / samp_freq);
	
	// step 2: convert to low-pass prototype estimate
	switch (filtps->type) {
		case FILT_LOWPASS_TYPE_d11:
			wn = u[0];
			break;
		case FILT_BANDPASS_TYPE_d11:
			bw = u[1] - u[0];
			wn = sqrtl(u[0] * u[1]);
			break;
		case FILT_HIGHPASS_TYPE_d11:
			wn = u[0];
			break;
		case FILT_BANDSTOP_TYPE_d11:
			bw = u[1] - u[0];
			wn = sqrtl(u[0] * u[1]);
			break;
	}
	
	// step 3: Get N-th order Butterworth analog lowpass prototype
	p = (FILT_LONG_COMPLEX_d11 *) calloc((size_t) order, sizeof(FILT_LONG_COMPLEX_d11));
	for (i = 1; i < order; i += 2) {
		p[i - 1].imag = ((pi * (sf16) i) / (sf16) (2 * order)) + half_pi;
		FILT_complex_expl_d11(p + i - 1, p + i - 1);
	}
	for (i = 1; i < order; i += 2) {
		p[i].real = p[i - 1].real;
		p[i].imag = -p[i - 1].imag;
	}
	if (is_odd)
		p[order - 1].real = (sf16) -1.0;

	j = order - 1;  // sort into ascending order, by real values
	if (is_odd) --j;
	for (i = 0; j > i; ++i, --j) {
		tc = p[i];
		p[i] = p[j];
		p[j] = tc;
	}
	
	// Transform to state-space
	a = (sf16 **) calloc_2D_m11((size_t) poles, (size_t) poles, sizeof(sf16), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	inv_a = (sf16 **) calloc_2D_m11((size_t) poles, (size_t) poles, sizeof(sf16), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	ta1 = (sf16 **) calloc_2D_m11((size_t) poles, (size_t) poles, sizeof(sf16), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	ta2 = (sf16 **) calloc_2D_m11((size_t) poles, (size_t) poles, sizeof(sf16), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	b = (sf16 *) calloc((size_t) poles, sizeof(sf16));
	bt = (sf16 *) calloc((size_t) poles, sizeof(sf16));
	c = (sf16 *) calloc((size_t) poles, sizeof(sf16));
	
	if ((offset = is_odd))
		a[0][0] = (sf16) -1.0;
	for (i = 0; i < order - 1; i += 2) {
		if ((idx = i + offset))
			a[i + offset][idx - 1] = FILT_ONE_d11;
		a[i + offset][i + offset] = p[i].real + p[i + 1].real;
		a[i + offset][i + offset + 1] = (sf16) -1.0;
		a[i + offset + 1][i + offset] = FILT_ONE_d11;
	}
	b[order - 1] = FILT_ONE_d11;
	c[order - 1] = FILT_ONE_d11;
	
	// step 4: Transform to lowpass, bandpass, highpass, or bandstop of desired Wn
	switch (filtps->type) {
		case FILT_LOWPASS_TYPE_d11:
			for (i = 0; i < order; ++i) {
				for (j = 0; j < order; ++j)
					a[i][j] *= wn;
				b[i] *= wn;
			}
			break;
		case FILT_BANDPASS_TYPE_d11:
			for (i = 0; i < order; ++i) {
				for (j = 0; j < order; ++j) {
					a[i][j] *= bw;
				}
				a[i][i + order] = wn;
				a[i + order][i] = -wn;
			}
			b[0] = bw;
			break;
		case FILT_HIGHPASS_TYPE_d11:
			for (i = 0; i < order; ++i) {
				c[i] = (sf16) -1.0;
				b[i] = wn;
			}
			for (i = is_odd; i < order; i += 2) {
				c[i + 1] = a[i][i];
				b[i] = FILT_ZERO_d11;
			}
			// d = FILT_ONE_d11;
			FILT_invert_matrix_d11(a, inv_a, order);
			
			for (i = 0; i < order; ++i)
				for (j = 0; j < order; ++j)
					a[i][j] = wn * inv_a[i][j];
			break;
		case FILT_BANDSTOP_TYPE_d11:
			for (i = 0; i < order; ++i) {
				c[i] = (sf16) -1.0;
				b[i] = bw;
			}
			for (i = is_odd; i < order; i += 2) {
				c[i + 1] = a[i][i];
				b[i] = FILT_ZERO_d11;
			}
			FILT_invert_matrix_d11(a, inv_a, order);
			for (i = 0; i < order; ++i) {
				for (j = 0; j < order; ++j) {
					a[i][j] = bw * inv_a[i][j];
				}
				a[i][i + order] = wn;
				a[i + order][i] = -wn;
			}
			break;
	}

	// step 5: Use bilinear transformation to find discrete equivalent
	t = (sf16) 0.25;
	for (i = 0; i < poles; ++i) {
		for (j = 0; j < poles; ++j) {
			ta1[i][j] = t * a[i][j];
			ta2[i][j] = -ta1[i][j];
		}
		ta1[i][i] += FILT_ONE_d11;
		ta2[i][i] += FILT_ONE_d11;
	}
	
	FILT_invert_matrix_d11(ta2, inv_a, poles);
	
	FILT_mat_multl_d11((void *) inv_a, (void *) ta1, (void *) a, poles, poles, poles);
	
	FILT_mat_multl_d11((void *) c, (void *) inv_a, (void *) bt, 1, poles, poles);
	t = sqrtl((sf16) 0.5);
	for (i = 0; i < poles; ++i)
		c[i] = bt[i] * t;
	
	FILT_mat_multl_d11((void *) bt, (void *) b, (void *) &t, 1, poles, 1);
	
	FILT_mat_multl_d11((void *) inv_a, (void *) b, (void *) bt, poles, poles, 1);
	t = FILT_ONE_d11 / sqrtl((sf16) 2.0);
	for (i = 0; i < poles; ++i)
		b[i] = bt[i] * t;
	
	// Transform to zero-pole-gain and polynomial forms
	eigs = (FILT_LONG_COMPLEX_d11 *) calloc((size_t) poles, sizeof(FILT_LONG_COMPLEX_d11));
	FILT_unsymmeig_d11(a, poles, eigs);
	
	den = (sf16 *) calloc((size_t) (poles + 1), sizeof(sf16));
	cden = (FILT_LONG_COMPLEX_d11 *) calloc((size_t) (poles + 1), sizeof(FILT_LONG_COMPLEX_d11));
	cden[0].real = FILT_ONE_d11;
	for (i = 0; i < poles; ++i) {
		for (j = i + 1; j--;) {
			FILT_complex_multl_d11(eigs + i, cden + j, &tc);
			cden[j + 1].real -= tc.real;
			cden[j + 1].imag -= tc.imag;
		}
	}
	for (i = 0; i <= poles; ++i)
		den[i] = cden[i].real;
	
	// generate numerator
	r = (sf16 *) calloc((size_t) (poles + 1), sizeof(sf16));
	rc = (FILT_LONG_COMPLEX_d11 *) calloc((size_t) (poles + 1), sizeof(FILT_LONG_COMPLEX_d11));
	wn = (sf16) 2.0 * atan2l(wn, 4.0);
	
	switch (filtps->type) {
		case FILT_LOWPASS_TYPE_d11:
			for (i = 0; i < poles; ++i)
				r[i] = (sf16) -1.0;
			break;
		case FILT_BANDPASS_TYPE_d11:
			for (i = 0; i < order; ++i) {
				r[i] = FILT_ONE_d11;
				r[i + order] = (sf16) -1.0;
			}
			w = -wn;
			break;
		case FILT_HIGHPASS_TYPE_d11:
			for (i = 0; i < poles; ++i)
				r[i] = FILT_ONE_d11;
			w = -pi;
			break;
		case FILT_BANDSTOP_TYPE_d11:
			tc.real = FILT_ZERO_d11;
			tc.imag = wn;
			FILT_complex_expl_d11(&tc, &tc);
			for (i = 0; i < poles; i += 2) {
				rc[i].real = tc.real;
				rc[i].imag = tc.imag;
				rc[i + 1].real = tc.real;
				rc[i + 1].imag = -tc.imag;
			}
			break;
	}
	
	num = (sf16 *) calloc((size_t) (poles + 1), sizeof(sf16));
	cnum = (FILT_LONG_COMPLEX_d11 *) calloc((size_t) (poles + 1), sizeof(FILT_LONG_COMPLEX_d11));
	if (filtps->type == FILT_BANDSTOP_TYPE_d11) {
		cnum[0].real = FILT_ONE_d11;
		for (i = 0; i < poles; ++i) {
			for (j = i + 1; j--;) {
				FILT_complex_multl_d11(rc + i, cnum + j, &tc);
				cnum[j + 1].real -= tc.real;
				cnum[j + 1].imag -= tc.imag;
			}
		}
		for (i = 0; i <= poles; ++i)
			num[i] = cnum[i].real;
	} else {
		num[0] = FILT_ONE_d11;
		for (i = 0; i < poles; ++i)
			for (j = i + 1; j--;)
				num[j + 1] -= r[i] * num[j];
	}
						
	// normalize
	ckern = (FILT_LONG_COMPLEX_d11 *) calloc((size_t) (poles + 1), sizeof(FILT_LONG_COMPLEX_d11));
	if ((filtps->type == FILT_LOWPASS_TYPE_d11) || (filtps->type == FILT_BANDSTOP_TYPE_d11)) {
		sum_num = sum_den = FILT_ZERO_d11;
		for (i = 0; i <= poles; ++i) {
			sum_num += num[i];
			sum_den += den[i];
		}
		ratio = sum_den / sum_num;
		for (i = 0; i <= poles; ++i)
			num[i] *= ratio;
	} else {
		tc.real = FILT_ZERO_d11;
		for (i = 0; i <= poles; ++i) {
			tc.imag = w * (sf16) i;
			FILT_complex_expl_d11(&tc, ckern + i);
			cnum[i].real = num[i];
			cnum[i].imag = FILT_ZERO_d11;
			cden[i].real = den[i];
			cden[i].imag = FILT_ZERO_d11;
		}
		csum_num.real = csum_den.real = csum_num.imag = csum_den.imag = FILT_ZERO_d11;
		for (i = 0; i <= poles; ++i) {
			FILT_complex_multl_d11(ckern + i, cnum + i, &tc);
			csum_num.real += tc.real;
			csum_num.imag += tc.imag;
			FILT_complex_multl_d11(ckern + i, cden + i, &tc);
			csum_den.real += tc.real;
			csum_den.imag += tc.imag;
		}
		FILT_complex_divl_d11(&csum_den, &csum_num, &cratio);
		for (i = 0; i <= poles; ++i) {
			FILT_complex_multl_d11(cnum + i, &cratio, &tc);
			num[i] = tc.real;
		}
	}
	
	// set & check output
	d_num = (sf8 *) calloc((size_t) (poles + 1), sizeof(sf8));
	d_den = (sf8 *) calloc((size_t) (poles + 1), sizeof(sf8));
	for (i = 0; i <= poles; ++i) {
		d_den[i] = (sf8) den[i];
		d_num[i] = (sf8) num[i];
		if (isnan(d_num[i]) || isinf(d_num[i]) || isnan(d_den[i]) || isinf(d_den[i])) {
			if (!(filtps->behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m11))
				error_message_m11("Bad filter: [function %s, line %d]\n", __FUNCTION__, __LINE__);
			if (filtps->behavior_on_fail & EXIT_ON_FAIL_m11)
				exit_m11(FILT_BAD_FILTER_d11);
			return(FILT_BAD_FILTER_d11);
		}
	}
	filtps->numerators = d_num;
	filtps->denominators = d_den;
						
	// clean up
	free((void *) a);
	free((void *) inv_a);
	free((void *) ta1);
	free((void *) ta2);
	free((void *) p);
	free((void *) b);
	free((void *) bt);
	free((void *) c);
	free((void *) den);
	free((void *) cden);
	free((void *) num);
	free((void *) cnum);
	free((void *) eigs);
	free((void *) r);
	free((void *) rc);
	free((void *) ckern);
	
	
	return(0);
}


void	FILT_complex_divl_d11(FILT_LONG_COMPLEX_d11 *a, FILT_LONG_COMPLEX_d11 *b, FILT_LONG_COMPLEX_d11 *quotient)  //  returns a / b
{
	FILT_LONG_COMPLEX_d11	ta, tb;
	sf16			den;
	
	
	ta = *a;  // copy in case in place
	tb = *b;
	den = (tb.real * tb.real) + (tb.imag * tb.imag);
	quotient->real = ((ta.real * tb.real) + (ta.imag * tb.imag)) / den;
	quotient->imag = ((ta.imag * tb.real) - (ta.real * tb.imag)) / den;
	
	
	return;
}


void	FILT_complex_expl_d11(FILT_LONG_COMPLEX_d11 *exponent, FILT_LONG_COMPLEX_d11 *ans)
{
	FILT_LONG_COMPLEX_d11   t;
	sf16            	c;
	
	
	t = *exponent;  // copy in case in place
	c = expl(t.real);
	ans->real = c * cosl(t.imag);
	ans->imag = c * sinl(t.imag);
	
	
	return;
}


void	FILT_complex_multl_d11(FILT_LONG_COMPLEX_d11 *a, FILT_LONG_COMPLEX_d11 *b, FILT_LONG_COMPLEX_d11 *product)
{
	FILT_LONG_COMPLEX_d11    ta, tb;
	
	
	ta = *a;  // copy in case in place
	tb = *b;
	product->real = (ta.real * tb.real) - (ta.imag * tb.imag);
	product->imag = (ta.real * tb.imag) + (ta.imag * tb.real);
	
	
	return;
}


void	FILT_elmhes_d11(sf16 **a, si4 poles)
{
	si4     i, j, m;
	sf16    x, y, t1;
	
	
	for (m = 1; m < (poles - 1); m++) {
		x = FILT_ZERO_d11;
		i = m;
		for (j = m; j < poles; j++) {
			if (FILT_ABS_d11(a[j][m-1]) > FILT_ABS_d11(x)) {
				x = a[j][m - 1];
				i = j;
			}
		}
		if (i != m) {
			for (j = m - 1; j < poles; j++) {
				t1 = a[i][j];
				a[i][j] = a[m][j];
				a[m][j] = t1;
			}
			for (j = 0; j < poles; j++) {
				t1 = a[j][i];
				a[j][i] = a[j][m];
				a[j][m] = t1;
			}
		}
		if (x != FILT_ZERO_d11) {
			for (i = m + 1; i < poles; i++) {
				y = a[i][m - 1];
				if (y != FILT_ZERO_d11) {
					y /= x;
					a[i][m - 1] = y;
					for (j = m; j < poles; j++)
						a[i][j] -= (y * a[m][j]);
					for (j = 0; j < poles; j++)
						a[j][m] += (y * a[j][i]);
				}
			}
		}
	}
	
	
	return;
}


void	FILT_excise_transients_d11(CMP_PROCESSING_STRUCT_m11 *cps, si8 len, si8 *n_extrema)
{
	si8	i, j, span, ext_x, *ex, wind_start, wind_end, n_ext, wind_len;
	sf8	samp_freq, LFP_high_fc, *y, *qy, *sy, *ty, *jy, *try, ext_y;
	sf8	baseline, VDS_alg_thresh, thresh, *sf8_p1, *sf8_p2, *sf8_p3;
	
	
	// Function assumes VDS buffers are allocated
	// VDS Buffer Map:
	// 	VDS_in_bufs[0]:	in_y
	// 	VDS_in_bufs[1]: in_x (not used here, but don't touch)
	// 	VDS_in_bufs[2]:	excise_transients() smooth_data
	// 	VDS_in_bufs[3]:	excise_transients() transients
	// 	VDS_in_bufs[4]:	excise_transients() extrema
	// 	VDS_in_bufs[5-8]: scrap buffers (at this point in VDS)

	samp_freq = cps->parameters.VDS_sampling_frequency;
	LFP_high_fc = cps->parameters.VDS_LFP_high_fc;
	y = (sf8 *) cps->parameters.VDS_input_buffers->buffer[0];
	
	// median filter (to buffer 5)
	if (LFP_high_fc > (sf8) 0.0)
		span = (si8) round(samp_freq / LFP_high_fc);
	else
		span = (si8) round(samp_freq / (sf8) 500.0);  // assume 500 Hz is enough frequency resolution
	if (span < 12)
		span = 12;  // minimum of 3 cycles at 4 samples/cycle
	if (len < span) {  // not tested
		memcpy(cps->parameters.VDS_input_buffers->buffer[2], cps->parameters.VDS_input_buffers->buffer[0], (size_t) (len << 3));  // copy original data to smooth_data
		memset(cps->parameters.VDS_input_buffers->buffer[3], 0, (size_t) (len << 3));  // zero transients
		memset(cps->parameters.VDS_input_buffers->buffer[4], 0, (size_t) (len << 3));  // zero extrema
		*n_extrema = 0;
		return;
	}
	qy = (sf8 *) cps->parameters.VDS_input_buffers->buffer[5];
	FILT_quantfilt_d11(y, qy, len, (sf8) 0.5, span, FILT_TRUNCATE_d11);
	
	// generate smooth trace (to buffer 2)
	sy = (sf8 *) cps->parameters.VDS_input_buffers->buffer[2];
	sf8_p1 = y;
	sf8_p2 = qy;
	sf8_p3 = sy;
	for (i = len; i--;)
		*sf8_p3++ = *sf8_p1++ - *sf8_p2++;
	
	// generate threshold trace (to buffer 6)
	ty = (sf8 *) cps->parameters.VDS_input_buffers->buffer[6];
	sf8_p1 = sy;
	sf8_p2 = ty;
	for (i = len; i--; ++sf8_p1)
		*sf8_p2++ = (*sf8_p1 >= 0.0) ? *sf8_p1 : -*sf8_p1;

	// get baseline & threshold (buffer 7 used as scrap)
	jy = (sf8 *) cps->parameters.VDS_input_buffers->buffer[7];
	baseline = CMP_quantval_d11(ty, len, (sf8) 0.5, TRUE_m11, jy);
	VDS_alg_thresh = CMP_VDS_get_theshold_d11(cps);
	thresh = ((VDS_alg_thresh + (sf8) 6.0) / (sf8) 2.0) * baseline;
	
	// zero transients array (buffer 3)
	try = (sf8 *) cps->parameters.VDS_input_buffers->buffer[3];
	memset((void *) try, 0, (size_t) (len << 3));

	// set up extrema array (buffer 4)
	n_ext = 0;
	ex = (si8 *) cps->parameters.VDS_input_buffers->buffer[4];
	
	// excision loop
	for (i = 0; i < len;) {
		if (ty[i] > thresh) {
		    	wind_start = wind_end = i;
			if (sy[i] >= 0) {  // find transient window (upgoing)
				for (; wind_start >= 0; --wind_start)
					if (sy[wind_start] < 0)
						break;
				for (; wind_start >= 0; --wind_start)
					if (sy[wind_start] > 0)
						break;
				for (; wind_end < len; ++wind_end)
					if (sy[wind_end] < 0)
						break;
				for (; wind_end < len; ++wind_end)
					if (sy[wind_end] > 0)
						break;
			} else {  // find transient window (downgoing)
				for (; wind_start; --wind_start)
					if (sy[wind_start] > 0)
						break;
				for (; wind_start; --wind_start)
					if (sy[wind_start] < 0)
						break;
				for (; wind_end < len; ++wind_end)
					if (sy[wind_end] > 0)
						break;
				for (; wind_end < len; ++wind_end)
					if (sy[wind_end] < 0)
						break;
			}
			// correct for window overshoot
			++wind_start;
			--wind_end;
			
			// copy smooth contents into transients array & get extrema
			wind_len = (wind_end - wind_start) - 1;
			if (wind_len > 1) {
				ext_x = wind_start;
				ext_y = sy[wind_start];
				if (sy[i] >= 0) {  // upgoing
					for (j = wind_start; j <= wind_end; ++j) {
						try[j] = sy[j];
						if (ext_y < sy[j]) {
							ext_y = sy[j];
							ext_x = j;
						}
					}
				} else {  // downgoing
					for (j = wind_start; j <= wind_end; ++j) {
						try[j] = sy[j];
						if (ext_y > sy[j]) {
							ext_y = sy[j];
							ext_x = j;
						}
					}
				}
				// add extreme values
				ex[n_ext++] = ext_x;
			}
						
			// update for next loop
			i = wind_end;
			
		} else {  // ty[i] <= thresh
			++i;
		}
	}
	
	// finish smooth trace: add quantfilt trace back, or replace with quantfilt trace in transient windows
	sf8_p1 = try;
	sf8_p2 = qy;
	sf8_p3 = sy;
	for (i = len; i--;) {
		if (*sf8_p1++ == (sf8) 0.0)
			*sf8_p3++ += *sf8_p2++;
		else
			*sf8_p3++ = *sf8_p2++;
	}
	*n_extrema = n_ext;

	return;
}

				
si4	FILT_filtfilt_d11(FILT_PROCESSING_STRUCT_d11 *filtps)
{
	TERN_m11        	free_z_flag, free_buf_flag;
	si4	        	padded_data_len, pad_len, pad_lenx2, poles;
	si8	        	i, j, k, m, data_len;
	sf8	        	dx2, zc[FILT_MAX_ORDER_d11 * 2], t1, t2;
	sf8 	        	*num, *den, *data, *filt_data, *z, *buf, *dp, *fdp;
	
	
	// filter data from filtps->orig_data into filtps->filt_data
	// if filtps->orig_data == filtps->filt_data, filtering done in place, but the target array must have room for the pads
	// if orig_data == (filt_data + pad_len) caller put data directly into the filt_data array with room for pad - skip initial copy
	// pad_len == (order * n_cutoffs * 3) => see FILT_OFFSET_ORIG_DATA_d11() macro.
	
	// error check
	if (filtps->orig_data == NULL) {
		if (!(filtps->behavior_on_fail & SUPPRESS_WARNING_OUTPUT_m11))
			warning_message_m11("%s(): No data passed", __FUNCTION__);
		if (filtps->behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(1);
		return(FILT_BAD_DATA_d11);
	}
	poles = filtps->n_poles;
	pad_len = poles * FILT_PAD_SAMPLES_PER_POLE_d11;
	pad_lenx2 = pad_len << 1;
	data_len = filtps->data_length;
	if (data_len < pad_len) {
		if (!(filtps->behavior_on_fail & SUPPRESS_WARNING_OUTPUT_m11)) {
			if (filtps->type == FILT_LOWPASS_TYPE_d11 || filtps->type == FILT_HIGHPASS_TYPE_d11)
				warning_message_m11("%s(): At least %d data points required for a filter of order %d\n", __FUNCTION__, pad_len, filtps->order);
			else
				warning_message_m11("%s(): At least %d data points required for a filter of order %d with two cutoffs\n", __FUNCTION__, pad_len, filtps->order);
		}
		if (filtps->behavior_on_fail & EXIT_ON_FAIL_m11)
			exit_m11(1);
		memmove(filtps->filt_data, filtps->orig_data, data_len * sizeof(sf8));
		return(FILT_BAD_DATA_d11);
	}
	
	num = filtps->numerators;
	den = filtps->denominators;
	filt_data = filtps->filt_data;
	free_z_flag = FALSE_m11;
	if (filtps->initial_conditions == NULL) {
		FILT_generate_initial_conditions_d11(filtps);
		free_z_flag = TRUE_m11;
	}
	z = filtps->initial_conditions;
	free_buf_flag = FALSE_m11;
	if (filtps->buffer == NULL) {
		filtps->buffer = (sf8 *) calloc((size_t) data_len + pad_lenx2, sizeof(sf8));
		free_buf_flag = TRUE_m11;
	}
	buf = filtps->buffer;
	data = filtps->orig_data;
	
	// copy data to filt_data with romm for pads
	if (data == filt_data) {  // just shift data by pad_len  [ equivalent: memmove((void *) (filt_data + pad_len), (void *) data, data_len * sizeof(sf8)); ]
		dp = data + data_len;
		fdp = dp + pad_len;
		for (i = data_len; i--;)
			*--fdp = *--dp;
	} else if (data != FILT_OFFSET_ORIG_DATA_d11(filtps)) {
		memcpy((void *) (filt_data + pad_len), (void *) data, data_len * sizeof(sf8));  // memcpy typically faster, but cannot overlap
	}
	// else: caller put data directly into the filt_data array with room for pad - skip copy
	
	// front pad
	dx2 = data[0] * (sf8) 2.0;
	for (i = 0, j = pad_len; j; ++i, --j)
	 	filt_data[i] = dx2 - data[j];
	// back pad
	padded_data_len = data_len + pad_lenx2;
	dx2 = data[data_len - 1] * (sf8) 2.0;
	for (i = data_len + pad_len, j = data_len - 2; i < padded_data_len; ++i, --j)
		filt_data[i] = dx2 - data[j];
	
	// copy and initialize initial conditions
	for (i = 0; i < poles; ++i)
		zc[i] = z[i] * filt_data[0];
	
	// forward filter from filt_data to buffer
	for (i = 0; i < padded_data_len; ++i) {
		t1 = filt_data[i];
		t2 = (num[0] * t1) + zc[0];
		for (j = 1; j < poles; ++j)
			zc[j - 1] = (num[j] * t1) - (den[j] * t2) + zc[j];
		zc[poles - 1] = (num[poles] * t1) - (den[poles] * t2);
		buf[i] = t2;
	}
	
	// copy and initialize initial conditions
	for (i = 0; i < poles; ++i)
		zc[i] = z[i] * buf[padded_data_len - 1];
	
	// reverse filter from buffer to filt_data
	for (i = padded_data_len - 1, k = pad_len; k--;) {
		t1 = buf[i--];
		t2 = (num[0] * t1) + zc[0];
		for (j = 1; j < poles; ++j)
			zc[j - 1] = (num[j] * t1) - (den[j] * t2) + zc[j];
		zc[poles - 1] = (num[poles] * t1) - (den[poles] * t2);
	}
	for (m = i - pad_len, k = data_len; k--;) {
		t1 = buf[i--];
		t2 = (num[0] * t1) + zc[0];
		for (j = 1; j < poles; ++j)
			zc[j - 1] = (num[j] * t1) - (den[j] * t2) + zc[j];
		zc[poles - 1] = (num[poles] * t1) - (den[poles] * t2);
		filt_data[m--] = t2;
	}
	
	// free as required
	if (free_z_flag == TRUE_m11) {
		free_d11(z, __FUNCTION__, __LINE__);
		filtps->initial_conditions = NULL;
	}
	if (free_buf_flag == TRUE_m11) {
		free_d11(buf, __FUNCTION__, __LINE__);
		filtps->buffer = NULL;
	}
	
	return(0);
}


void	FILT_free_CPS_filtps_d11(CMP_PROCESSING_STRUCT_m11 *cps, TERN_m11 free_orig_data, TERN_m11 free_filt_data, TERN_m11 free_buffer)
{
	si4				i;
	FILT_PROCESSING_STRUCT_d11	*filtps;
	
	
	// filtps in CMP_PROCESSING_STRUCT_m11 are not freed by CMP_free_processing_struct_m11() because FILT_PROCESSING_STRUCT_d11 is define in the dhn library
	// Call this funtion before freeing a cps to prevent memory leaks.
	
	if (cps->parameters.filtps == NULL)
		return;
	for (i = 0; i < cps->parameters.n_filtps; ++i) {
		filtps = (FILT_PROCESSING_STRUCT_d11 *) cps->parameters.filtps[i];
		if (filtps != NULL)
			FILT_free_processing_struct_d11(filtps, free_orig_data, free_filt_data, free_buffer, TRUE_m11);
	}
	free((void *) cps->parameters.filtps);
	cps->parameters.filtps = NULL;
	cps->parameters.n_filtps = 0;
	
	return;
}


void	FILT_free_processing_struct_d11(FILT_PROCESSING_STRUCT_d11 *filtps, TERN_m11 free_orig_data, TERN_m11 free_filt_data, TERN_m11 free_buffer, TERN_m11 free_structure)
{
	if (filtps == NULL) {
		warning_message_m11("%s(): trying to free a NULL FILT_PROCESSING_STRUCT_d11", __FUNCTION__);
		return;
	}
	if (filtps->numerators != NULL)
		free((void *) filtps->numerators);
	if (filtps->denominators != NULL)
		free((void *) filtps->denominators);
	if (filtps->initial_conditions != NULL)
		free((void *) filtps->initial_conditions);
	if (filtps->orig_data != NULL)
		if (free_orig_data == TRUE_m11)  // IMPORTANT: if keeping orig_data, caller should have a copy of address to free when they're done to avoid a memory leak
			if (filtps->orig_data != filtps->filt_data)  // simple filter-in-place arrangement
				if (filtps->orig_data != FILT_OFFSET_ORIG_DATA_d11(filtps))  // efficient filter-in-place arrangement
					free((void *) filtps->orig_data);
	if (filtps->filt_data != NULL)
		if (free_filt_data == TRUE_m11)  // IMPORTANT: if keeping filt_data, caller should have a copy of address to free when they're done to avoid a memory leak
			free((void *) filtps->filt_data);
	if (filtps->buffer != NULL)
		if (free_buffer == TRUE_m11)
			free((void *) filtps->buffer);
	
	if (free_structure == TRUE_m11) {
		free((void *) filtps);
	} else {
		filtps->data_length = 0;
		filtps->numerators = NULL;
		filtps->denominators = NULL;
		filtps->initial_conditions = NULL;
		filtps->orig_data = NULL;
		filtps->filt_data = NULL;
		filtps->buffer = NULL;
	}
	
	return;
}


void	FILT_generate_initial_conditions_d11(FILT_PROCESSING_STRUCT_d11 *filtps)
{
	si4     i, j, poles;
	sf16    **q, *rhs, *z;
	sf8	*num, *den;


	poles = filtps->n_poles;
	num = filtps->numerators;
	den = filtps->denominators;
	q = (sf16 **) calloc_2D_m11((size_t) poles, (size_t) poles, sizeof(sf16), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	
	rhs = (sf16 *) calloc((size_t) poles, sizeof(sf16));
	z = (sf16 *) calloc((size_t) poles, sizeof(sf16));
	filtps->initial_conditions = (sf8 *) calloc((size_t) poles, sizeof(sf8));
	
	q[0][0] = (sf16) 1.0 + (sf16) den[1];
	for (i = 1, j = 2; i < poles; ++i, ++j)
		q[i][0] = (sf16) den[j];
	for (i = 1; i < poles; ++i) {
		q[i - 1][i] = (sf16) -1.0;
		q[i][i] = (sf16) 1.0;
	}
	for (i = 0, j = 1; i < poles; ++i, ++j)
		rhs[i] = (sf16) num[j] - ((sf16) num[0] * (sf16) den[j]);
	
	FILT_invert_matrix_d11(q, q, poles);
	FILT_mat_multl_d11(q, rhs, z, poles, poles, 1);
	
	for (i = 0; i < poles; ++i)
		filtps->initial_conditions[i] = (sf8) z[i];
	
	free((void *) q);
	free((void *) rhs);
	free((void *) z);
	
	
	return;
}


void	FILT_hqr_d11(sf16 **a, si4 poles, FILT_LONG_COMPLEX_d11 *eigs)
{
	si4     nn, m, l, k, j, its, i, mmin, max;
	sf16    z, y, x, w, v, u, t, s, r, q, p, anorm, eps, t1, t2, t3, t4;
	
	
	anorm = FILT_ZERO_d11;
	eps = (sf16) FILT_EPS_SF16_d11;
	
	for (i = 0; i < poles; i++) {
		max = ((i - 1) > 0) ? (i - 1) : 0;
		for (j = max; j < poles; j++) {
			anorm += FILT_ABS_d11(a[i][j]);
		}
	}
	
	nn = poles - 1;
	t = FILT_ZERO_d11;
	while (nn >= 0) {
		its = 0;
		do {
			for (l = nn; l > 0; l--) {
				t1 = FILT_ABS_d11(a[l - 1][l - 1]);
				t2 = FILT_ABS_d11(a[l][l]);
				s = t1 + t2;
				if (s == FILT_ZERO_d11)
					s = anorm;
				t1 = FILT_ABS_d11(a[l][l - 1]);
				if (t1 <= (eps * s)) {
					a[l][l - 1] = FILT_ZERO_d11;
					break;
				}
			}
			x = a[nn][nn];
			if (l == nn) {
				eigs[nn].real = x + t;
				eigs[nn--].imag = FILT_ZERO_d11;
			} else {
				y = a[nn - 1][nn - 1];
				w = a[nn][nn - 1] * a[nn - 1][nn];
				if (l == (nn - 1)) {
					p = (sf16) 0.5 * (y - x);
					q = (p * p) + w;
					z = sqrtl(FILT_ABS_d11(q));
					x += t;
					if (q >= FILT_ZERO_d11) {
						t1 = FILT_SIGN_d11(z, p);
						z = p + t1;
						eigs[nn - 1].real = eigs[nn].real = x + z;
						if (z != FILT_ZERO_d11)
							eigs[nn].real = x - w / z;
					} else {
						eigs[nn].real = x + p;
						eigs[nn].imag = -z;
						eigs[nn - 1].real = eigs[nn].real;
						eigs[nn - 1].imag = -eigs[nn].imag;
					}
					nn -= 2;
				} else {
					if (its == 30) {
						error_message_m11("Too many iterations in hqr\n");
						exit_m11(1);
					}
					if (its == 10 || its == 20) {
						t += x;
						for (i = 0; i < nn + 1; i++)
							a[i][i] -= x;
						t1 = FILT_ABS_d11(a[nn][nn - 1]);
						t2 = FILT_ABS_d11(a[nn - 1][nn - 2]);
						s = t1 + t2;
						y = x = (sf16) 0.75 * s;
						w = (sf16) -0.4375 * s * s;
					}
					++its;
					for (m = nn - 2; m >= l; m--) {
						z = a[m][m];
						r = x - z;
						s = y - z;
						p = ((r * s - w) / a[m + 1][m]) + a[m][m + 1];
						q = a[m + 1][m + 1] - z - r - s;
						r = a[m + 2][m + 1];
						t1 = FILT_ABS_d11(p);
						t2 = FILT_ABS_d11(q);
						t3 = FILT_ABS_d11(r);
						s = t1 + t2 + t3;
						p /= s;
						q /= s;
						r /= s;
						if (m == l)
							break;
						t1 = FILT_ABS_d11(a[m][m - 1]);
						t2 = FILT_ABS_d11(q);
						t3 = FILT_ABS_d11(r);
						u = t1 * (t2 + t3);
						t1 = FILT_ABS_d11(p);
						t2 = FILT_ABS_d11(a[m - 1][m - 1]);
						t3 = FILT_ABS_d11(z);
						t4 = FILT_ABS_d11(a[m + 1][m + 1]);
						v = t1 * (t2 + t3 + t4);
						if (u <= (eps * v))
							break;
					}
					for (i = m; i < (nn - 1); i++) {
						a[i + 2][i] = FILT_ZERO_d11;
						if (i != m)
							a[i + 2][i - 1] = FILT_ZERO_d11;
					}
					for (k = m; k < nn; k++) {
						if (k != m) {
							p = a[k][k - 1];
							q = a[k + 1][k - 1];
							r = FILT_ZERO_d11;
							if (k + 1 != nn)
								r = a[k + 2][k - 1];
							t1 = FILT_ABS_d11(p);
							t2 = FILT_ABS_d11(q);
							t3 = FILT_ABS_d11(r);
							if ((x = t1 + t2 + t3) != FILT_ZERO_d11) {
								p /= x;
								q /= x;
								r /= x;
							}
						}
						t1 = sqrtl((p * p) + (q * q) + (r * r));
						s = FILT_SIGN_d11(t1, p);
						if (s != 0.0) {
							if (k == m) {
								if (l != m)
									a[k][k - 1] = -a[k][k - 1];
							} else
								a[k][k - 1] = -s * x;
							p += s;
							x = p / s;
							y = q / s;
							z = r / s;
							q /= p;
							r /= p;
							for (j = k; j < (nn + 1); j++) {
								p = a[k][j] + (q * a[k + 1][j]);
								if ((k + 1) != nn) {
									p += r * a[k + 2][j];
									a[k + 2][j] -= p * z;
								}
								a[k + 1][j] -= p * y;
								a[k][j] -= p * x;
							}
							mmin = nn < (k + 3) ? nn : k + 3;
							for (i = l; i < (mmin + 1); i++) {
								p = (x * a[i][k]) + (y * a[i][k + 1]);
								if ((k + 1) != nn) {
									p += z * a[i][k + 2];
									a[i][k + 2] -= p * r;
								}
								a[i][k + 1] -= p * q;
								a[i][k] -= p;
							}
						}
					}
				}
			}
		} while ((l + 1) < nn);
	}
	
	return;
}


FILT_PROCESSING_STRUCT_d11      *FILT_initialize_processing_struct_d11(si4 order, si4 type, sf8 samp_freq, si8 data_len, TERN_m11 alloc_orig_data, TERN_m11 alloc_filt_data, TERN_m11 alloc_buffer, ui4 behavior_on_fail, sf8 cutoff_1, ...)
{
	extern GLOBALS_m11		*globals_m11;
	si8			        pad_samples, buf_len;
	FILT_PROCESSING_STRUCT_d11	*filtps;
	va_list			        arg_p;
	
		
	// allocate
	filtps = (FILT_PROCESSING_STRUCT_d11 *) calloc((size_t) 1, sizeof(FILT_PROCESSING_STRUCT_d11));  // calloc b/c need zeroes in all fields
	
	// populate
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m11)  // filtps has it's own behavior_on_fail so filter functions can be thread-safe
		filtps->behavior_on_fail = globals_m11->behavior_on_fail;
	else
		filtps->behavior_on_fail = behavior_on_fail;
	filtps->order = filtps->n_poles = order;
	filtps->type = type;
	filtps->sampling_frequency = samp_freq;
	filtps->data_length = data_len;
	filtps->cutoffs[0] = cutoff_1;
	if (type == FILT_BANDPASS_TYPE_d11 || type == FILT_BANDSTOP_TYPE_d11) {
		va_start(arg_p, cutoff_1);
		filtps->cutoffs[1] = va_arg(arg_p, sf8);
		va_end(arg_p);
		filtps->n_poles *= 2;  // (poles == order * cutoffs)
	}
	
	// build filter
	FILT_butter_d11(filtps);
	FILT_generate_initial_conditions_d11(filtps);

	// allocate
	filtps->orig_data = filtps->filt_data = filtps->buffer = NULL;
	if (alloc_orig_data == TRUE_m11)
		filtps->orig_data = (sf8 *) malloc((size_t) data_len * sizeof(sf8));
	pad_samples = 2 * (FILT_PAD_SAMPLES_PER_POLE_d11 * filtps->n_poles);  // (FILT_PAD_SAMPLES_PER_POLE_d11 for each pole at front & back)
	buf_len = data_len + pad_samples;  // filt_data & buffer must have room for pads
	if (alloc_filt_data == TRUE_m11)
		filtps->filt_data = (sf8 *) malloc((size_t) buf_len * sizeof(sf8));
	if (alloc_buffer == TRUE_m11)
		filtps->buffer = (sf8 *) malloc((size_t) buf_len * sizeof(sf8));

	return(filtps);
}


void	FILT_invert_matrix_d11(sf16 **a, sf16 **inv_a, si4 order)  // done in place if a == inv_a
{
	si4	*indxc, *indxr, *ipiv;
	si4	i, icol, irow, j, k, l, ll;
	sf16	big, dum, pivinv, temp;
	
	
	indxc = (si4 *) calloc((size_t) order, sizeof(si4));
	indxr = (si4 *) calloc((size_t) order, sizeof(si4));
	ipiv = (si4 *) calloc((size_t) order, sizeof(si4));
	
	if (inv_a != a) {
		for (i = 0; i < order; i++)
			for (j = 0; j < order; j++)
				inv_a[i][j] = a[i][j];
	}
	
	for (i = 0; i < order; i++) {
		big = FILT_ZERO_d11;
		for (j = 0; j < order; j++)
			if (ipiv[j] != 1)
				for (k = 0; k < order; k++) {
					if (ipiv[k] == 0) {
						if (FILT_ABS_d11(inv_a[j][k]) >= big) {
							big = FILT_ABS_d11(inv_a[j][k]);
							irow = j;
							icol = k;
						}
					}
				}
		++ipiv[icol];
		if (irow != icol) {
			for (l = 0; l < order; l++) {
				temp = inv_a[irow][l];
				inv_a[irow][l] = inv_a[icol][l];
				inv_a[icol][l] = temp;
			}
		}
		indxr[i] = irow;
		indxc[i] = icol;
		if (inv_a[icol][icol] == FILT_ZERO_d11) {
			error_message_m11("invert_matrix: Singular Matrix\n");
			exit_m11(1);
		}
		pivinv = FILT_ONE_d11 / inv_a[icol][icol];
		inv_a[icol][icol] = FILT_ONE_d11;
		for (l = 0; l < order; l++)
			inv_a[icol][l] *= pivinv;
		for (ll = 0; ll < order; ll++) {
			if (ll != icol) {
				dum = inv_a[ll][icol];
				inv_a[ll][icol] = FILT_ZERO_d11;
				for (l = 0; l < order;l++)
					inv_a[ll][l] -= inv_a[icol][l] * dum;
			}
		}
	}
	
	for (l = order - 1; l >= 0; l--) {
		if (indxr[l] != indxc[l]) {
			for (k = 0; k < order; k++) {
				temp = inv_a[k][indxr[l]];
				inv_a[k][indxr[l]] = inv_a[k][indxc[l]];
				inv_a[k][indxc[l]] = temp;
			}
		}
	}
	
	free((void *) ipiv);
	free((void *) indxr);
	free((void *) indxc);
	
	return;
}


void	FILT_mat_multl_d11(void *a, void *b, void *product, si4 outer_dim1, si4 inner_dim, si4 outer_dim2)
{
	si4	i, j, k, v1, v2, vp;
	sf16	sum, t1, t2, *av, **am, *bv, **bm, *pv, **pm;
	
	
	if ((outer_dim1 == 1) || (inner_dim == 1)) {
		av = (sf16 *) a;
		v1 = 1;
	} else {
		am = (sf16 **) a;
		v1 = 0;
	}
	if ((outer_dim2 == 1) || (inner_dim == 1)) {
		bv = (sf16 *) b;
		v2 = 1;
	} else {
		bm = (sf16 **) b;
		v2 = 0;
	}
	if ((outer_dim1 == 1) || (outer_dim2 == 1)) {
		pv = (sf16 *) product;
		vp = 1;
	} else {
		pm = (sf16 **) product;
		vp = 0;
	}
	
	for (i = 0; i < outer_dim1; ++i) {
		for (j = 0; j < outer_dim2; ++j) {
			sum = 0.0;
			for (k = 0; k < inner_dim; ++k) {
				t1 = (v1) ? av[k] : am[i][k];
				t2 = (v2) ? bv[k] : bm[k][j];
				sum += t1 * t2;
			}
			if (vp) {
				if (outer_dim1 == 1)
					pv[j] = sum;
				else
					pv[i] = sum;
			} else {
				pm[i][j] = sum;
			}
		}
	}
	
	
	return;
}


sf8    *FILT_noise_floor_filter_d11(sf8 *data, sf8 *filt_data, si8 data_len, sf8 rel_thresh, sf8 abs_thresh, CMP_BUFFERS_m11 *nff_buffers)
{
	register si8	i, j, k;
	TERN_m11	free_buffers;
	si8		n_peaks, n_troughs, *peak_xs, *trough_xs, x, xm1, dlm1;
	sf8		*peak_env, *trough_env, *mean_env, *env_diffs, base, baseline, dy, step, *quantval_buf, thresh;
	
	
	if (filt_data == NULL)
		filt_data = (sf8 *) malloc((size_t) (data_len << 3));

	free_buffers = FALSE_m11;
	if (nff_buffers == NULL)
		free_buffers = TRUE_m11;
	nff_buffers = CMP_allocate_buffers_m11(nff_buffers, 4, data_len, sizeof(sf8), FALSE_m11, FALSE_m11);  // also reallocates
	peak_xs = (si8 *) nff_buffers->buffer[0];
	trough_xs = (si8 *) nff_buffers->buffer[1];
	peak_env = (sf8 *) nff_buffers->buffer[2];
	trough_env = (sf8 *) nff_buffers->buffer[3];
	
	// find critical points
	CMP_find_crits_2_d11(data, data_len, &n_peaks, peak_xs, &n_troughs, trough_xs);
	if (n_peaks == 0)
		return(NULL);
	
	// create peak envelope
	peak_env[0] = data[0];
	x = peak_xs[0];
	for (i = 1; i < n_peaks; ++i) {
		xm1 = x;
		x = peak_xs[i];
		base = data[xm1];
		dy = data[x] - base;
		step = dy / (sf8) (x - xm1);
		for (j = xm1 + 1; j <= x; ++j)
			peak_env[j] = (base += step);
	}
	
	// create trough envelope
	trough_env[0] = data[0];
	x = trough_xs[0];
	for (i = 1; i < n_troughs; ++i) {
		xm1 = x;
		x = trough_xs[i];
		base = data[xm1];
		dy = data[x] - base;
		step = dy / (sf8) (x - xm1);
		for (j = xm1 + 1; j <= x; ++j)
			trough_env[j] = (base += step);
	}
	
	// transform envelopes to means & differences
	env_diffs = peak_env;
	mean_env = trough_env;
	for (i = 0; i < data_len; ++i) {
		env_diffs[i] -= trough_env[i];
		mean_env[i] += env_diffs[i] / (sf8) 2.0;
	}

	// get median of envelope difference time series
	if (rel_thresh > 0.0) {
		quantval_buf = (sf8 *) trough_xs;
		baseline = CMP_quantval_d11(env_diffs, data_len, 0.5, TRUE_m11, quantval_buf);
		thresh = (baseline / (sf8) 2.0) * rel_thresh;
	} else {
		thresh = abs_thresh;
	}

	// replace large dy with original data
	dlm1 = data_len - 1;;
	for (i = 1, j = 0, k = 2; i < dlm1; ++i, ++j, ++k) {
		if (env_diffs[i] > thresh) {
			if (env_diffs[j] <= thresh && env_diffs[k] <= thresh)   // skip points that exceed thresh for just 1 sample
				continue;
			mean_env[i] = data[i];
		}
	}
	
	// copy to output
	memcpy((void *) filt_data, (void *) mean_env, (size_t) (data_len << 3));
	
	// release resources
	if (free_buffers == TRUE_m11)
		CMP_free_buffers_m11(nff_buffers, TRUE_m11);
	
	return(filt_data);
}


sf8	*FILT_quantfilt_d11(sf8 *x, sf8 *qx, si8 len, sf8 quantile, si8 span, si1 tail_option_code)
{
	FILT_NODE_d11	*nodes, head, tail, *new_node, *prev_new_node, *curr_node, *next_node, *prev_node, *low_q_node, *oldest_node;
	si8     	i, new_span, out_idx, in_idx, low_q_idx, oldest_idx, last_sliding_out_idx, odd_span;
	sf8     	new_val, prev_new_val, temp_idx, low_val_q, high_val_q, low_q_val, high_q_val, true_q_val, q_shift, oldest_val;
	
	
	// setup
	if (qx == NULL) // caller responsible for freeing qx
		qx = (sf8 *) calloc_m11((size_t) len, sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	nodes = (FILT_NODE_d11 *) calloc_m11((size_t) (span + 1), sizeof(FILT_NODE_d11), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	new_node = nodes;
	head.val = -DBL_MAX;
	head.next = new_node;
	tail.val = DBL_MAX;
	tail.prev = new_node;
	new_node->val = x[0];
	new_node->next = &tail;
	new_node->prev = &head;
	in_idx = 1;
	out_idx = 0;
	odd_span = span & 1;

	// build initial window (for "truncate" tail option)
	if (tail_option_code == FILT_TRUNCATE_d11) {
		if (odd_span) {
			qx[out_idx++] = prev_new_val = new_node->val;
			prev_new_node = new_node++;
		} else {  // even span
			prev_new_node = new_node++;
			prev_new_val = prev_new_node->val;
			new_val = new_node->val = x[in_idx++];
			if (new_val < prev_new_val) {
				new_node->next = prev_new_node;
				new_node->prev = prev_new_node->prev;
				prev_new_node->prev = new_node;
				head.next = new_node;
			} else {
				new_node->next = prev_new_node->next;
				new_node->prev = prev_new_node;
				prev_new_node->next = new_node;
				tail.prev = new_node;
			}
			low_q_node = head.next;
			low_q_val = low_q_node->val;
			high_q_val = (low_q_node->next)->val;
			qx[out_idx++] = (((sf8) 1.0 - quantile) * low_q_val) + (quantile * high_q_val);
			prev_new_node = new_node++;
			prev_new_val = new_val;
		}
	}

	while (in_idx < span) {
		 
		// insert a new node
		new_val = new_node->val = x[in_idx++];
		curr_node = prev_new_node;
		if (new_val >= prev_new_val) {
			// search forward
			while (1) {
				next_node = curr_node->next;
				if (new_val < next_node->val)
					break;
				curr_node = next_node;
			}
			// insert new_node after curr_node
			new_node->prev = curr_node;
			new_node->next = next_node;
			curr_node->next = new_node;
			next_node->prev = new_node;
		} else {  // new_val < prev_new_val
			// search backward
			while (1) {
				prev_node = curr_node->prev;
				if (new_val >= prev_node->val)
					break;
				curr_node = prev_node;
			}
			// insert new_node before curr_node
			new_node->next = curr_node;
			new_node->prev = prev_node;
			curr_node->prev = new_node;
			prev_node->next = new_node;
		}
		
		// insert another new node
		prev_new_node = new_node++;
		prev_new_val = new_val;
		new_val = new_node->val = x[in_idx++];
		curr_node = prev_new_node;
		if (new_val >= prev_new_val) {
			// search forward
			while (1) {
				next_node = curr_node->next;
				if (new_val < next_node->val)
					break;
				curr_node = next_node;
			}
			// insert new_node after curr_node
			new_node->prev = curr_node;
			new_node->next = next_node;
			curr_node->next = new_node;
			next_node->prev = new_node;
		} else {  // new_val < prev_new_val
			// search backward
			while (1) {
				prev_node = curr_node->prev;
				if (new_val >= prev_node->val)
					break;
				curr_node = prev_node;
			}
			// insert new_node before curr_node
			new_node->next = curr_node;
			new_node->prev = prev_node;
			curr_node->prev = new_node;
			prev_node->next = new_node;
		}

		// calculate output
		if (quantile != (sf8) 1.0) {
			temp_idx = quantile * (sf8) (in_idx - 1);
			low_q_idx = (ui8) temp_idx;
			high_val_q = temp_idx - (sf8) low_q_idx;
			low_val_q = (sf8) 1.0 - high_val_q;
			curr_node = head.next;
			for (i = low_q_idx; i--;)
				curr_node = curr_node->next;
			low_q_val = (low_q_node = curr_node)->val;
			high_q_val = (low_q_node->next)->val;
			qx[out_idx++] = (low_q_val * low_val_q) + (high_q_val * high_val_q);
		} else {  // quantile == 1.0
			qx[out_idx++] = (curr_node = tail.prev)->val;
		}
		
		// update loop variables
		prev_new_node = new_node++;
		prev_new_val = new_val;
	}
	
	// handle other tail options (for initial window)
	if (tail_option_code == FILT_EXTRAPOLATE_d11) {
		true_q_val = qx[out_idx - 1];
		for (i = out_idx - 1; i--;)
			qx[i] = true_q_val;
	} else if (tail_option_code == FILT_ZEROPAD_d11) {
		for (i = out_idx - 1; i--;)
			qx[i] = (sf8) 0.0;
	}
		
	// slide window (main loop)
	oldest_idx = 0;
	oldest_node = nodes;
	oldest_val = oldest_node->val;
	low_q_node = curr_node;
	low_q_val = low_q_node->val;
	while (in_idx < len) {
		
		// insert new value into empty node
		new_node->val = new_val = x[in_idx];
		curr_node = prev_new_node;
		if (new_val >= prev_new_val) {
			// search forward
			while (1) {
				next_node = curr_node->next;
				if (new_val < next_node->val)
					break;
				curr_node = next_node;
			}
			// insert new_node after curr_node
			new_node->prev = curr_node;
			new_node->next = next_node;
			curr_node->next = new_node;
			next_node->prev = new_node;
		} else {  // new_val < prev_new_val
			// search backward
			while (1) {
				prev_node = curr_node->prev;
				if (new_val >= prev_node->val)
					break;
				curr_node = prev_node;
			}
			// insert new_node before curr_node
			new_node->next = curr_node;
			new_node->prev = prev_node;
			curr_node->prev = new_node;
			prev_node->next = new_node;
		}
		
		// update q node
		if (new_val >= low_q_val) {
			q_shift = (sf8) 0.5;
		} else {  // new_val < low_q_val
			q_shift = (sf8) -0.5;
		}
		if (oldest_val > low_q_val) {
			q_shift -= (sf8) 0.5;
		} else if (oldest_val < low_q_val) {
			q_shift += (sf8) 0.5;
		} else {  // oldest_val == low_q_val
			if (oldest_node == low_q_node) {
				q_shift *= (sf8) 2.0;
			} else {  // oldest_node != low_q_node
				q_shift += (sf8) 0.5;
			}
		}
		
		// remove oldest node
		(oldest_node->prev)->next = oldest_node->next;
		(oldest_node->next)->prev = oldest_node->prev;
		
		// update q node
		if (q_shift == (sf8) 1.0) {
			low_q_node = low_q_node->next;
		} else if (q_shift == (sf8) -1.0) {
			low_q_node = low_q_node->prev;
		}
		
		// output new q value
		if (quantile != (sf8) 1.0) {
			low_q_val = low_q_node->val;
			high_q_val = (low_q_node->next)->val;
			qx[out_idx] = (low_q_val * low_val_q) + (high_q_val * high_val_q);
		} else {  // quantile == 1.0
			qx[out_idx] = low_q_val = (low_q_node = tail.prev)->val;
		}
		
		// update rotating indices
		prev_new_node = new_node;
		prev_new_val = new_val;
		new_node = oldest_node;
		if (++oldest_idx > span)
		    oldest_idx = 0;
		oldest_node = nodes + oldest_idx;
		oldest_val = oldest_node->val;
		
		// update non-rotating indices
		++in_idx;
		++out_idx;
	}
	
	// build terminal window (for "truncate" tail option)
	last_sliding_out_idx = out_idx;
	if (tail_option_code == FILT_TRUNCATE_d11) {
		for (new_span = span - 3; new_span > 0; new_span -= 2) {
			
			// remove oldest node
			(oldest_node->prev)->next = oldest_node->next;
			(oldest_node->next)->prev = oldest_node->prev;
			if (++oldest_idx > span)
			    oldest_idx = 0;
			oldest_node = nodes + oldest_idx;

			// remove next oldest node
			(oldest_node->prev)->next = oldest_node->next;
			(oldest_node->next)->prev = oldest_node->prev;
			if (++oldest_idx > span)
			    oldest_idx = 0;
			oldest_node = nodes + oldest_idx;

			// calculate output
			if (quantile != (sf8) 1.0) {
				temp_idx = quantile * (sf8) new_span;
				low_q_idx = (ui8) temp_idx;
				high_val_q = temp_idx - (sf8) low_q_idx;
				low_val_q = (sf8) 1.0 - high_val_q;
				low_q_node = head.next;
				for (i = low_q_idx; i--;)
					low_q_node = low_q_node->next;
				low_q_val = low_q_node->val;
				high_q_val = (low_q_node->next)->val;
				qx[out_idx++] = (low_q_val * low_val_q) + (high_q_val * high_val_q);
			} else {  // quantile == 1.0
				qx[out_idx++] = (tail.prev)->val;
			}
		}
		qx[len - 1] = x[len - 1];
	}
	
	// handle other tail options (for terminal window)
	else if (tail_option_code == FILT_EXTRAPOLATE_d11) {
		true_q_val = qx[last_sliding_out_idx - 1];
		for (i = last_sliding_out_idx; i < len; ++i)
			qx[i] = true_q_val;
	} else if (tail_option_code == FILT_ZEROPAD_d11) {
		for (i = last_sliding_out_idx; i < len; ++i)
			qx[i] = (sf8) 0.0;
	}
	
	// clean up
	free(nodes);
	
	return(qx);
}


ui1     FILT_remove_line_noise_d11(si4 *data, si8 n_samps, sf8 samp_freq, sf8 line_freq, si4 cycles_in_template, ui1 thresh_score, si1 calc_score_flag, si1 remove_noise_flag, si4 n_harmonics)
{
	FILT_PROCESSING_STRUCT_d11      *filtps;
	si8     i, j, extra_points, resampled_samps, resampled_samps_per_cycle, cycles_in_data;
	sf8     raw_amplitude, *sf8_buf_1, *sf8_buf_2, *sf8_buf_3, *sf8_p1, *sf8_p2, *sf8_p3, samps_per_cycle, sf8_cycles_in_data, sf8_temp_amplitude;
	sf8     temp_sf8, template_sum, template_mean, total_template_sum, resampled_samp_freq;
	si4     *si4_p;
	ui1     score;
	
	
	// setup
	samps_per_cycle = samp_freq / line_freq;
	resampled_samps_per_cycle = (si8) ceil(samps_per_cycle);
	resampled_samp_freq = (samp_freq * resampled_samps_per_cycle) / samps_per_cycle;
	sf8_cycles_in_data = (sf8) n_samps / samps_per_cycle;
	cycles_in_data = (si8) ceil(sf8_cycles_in_data);
	resampled_samps = (si8) ceil(sf8_cycles_in_data * (sf8) resampled_samps_per_cycle);
	// step_size = samps_per_cycle / (sf8) resampled_samps_per_cycle;
	extra_points = resampled_samps - ((cycles_in_data - 1) * resampled_samps_per_cycle);
	if (!cycles_in_template) {
		cycles_in_template = cycles_in_data; // non-adaptive mode
		if (extra_points < resampled_samps_per_cycle)
			--cycles_in_template;
	}

	sf8_buf_1 = (sf8 *) calloc((size_t) (resampled_samps_per_cycle * cycles_in_data), sizeof(sf8));
	sf8_buf_2 = (sf8 *) calloc((size_t) (resampled_samps_per_cycle * cycles_in_data), sizeof(sf8));
	sf8_buf_3 = (sf8 *) calloc((size_t) (resampled_samps_per_cycle * cycles_in_data), sizeof(sf8));
       
	// resample original data
	sf8_p1 = sf8_buf_1;
	si4_p = data;
	for (i = n_samps; i--;)
		*sf8_p1++ = (sf8) *si4_p++;
	CMP_lin_interp_sf8_m11(sf8_buf_1, n_samps, sf8_buf_2, resampled_samps);
	// orig data in buf1, resampled orig data in buf2]
	// bandpass filter resampled data
	if (!n_harmonics)
		n_harmonics = FILT_LINE_NOISE_HARMONICS_DEFAULT_d11;
	filtps = FILT_initialize_processing_struct_d11(5, FILT_BANDPASS_TYPE_d11, resampled_samp_freq, resampled_samps, FALSE_m11, FALSE_m11, TRUE_m11, (RETURN_ON_FAIL_m11 | SUPPRESS_WARNING_OUTPUT_m11), line_freq - 10.0, (line_freq * (sf8) n_harmonics) + 10.0);
	filtps->orig_data = sf8_buf_2;
	FILT_filtfilt_d11(filtps);
	memcpy(sf8_buf_1, filtps->filt_data, resampled_samps * sizeof(sf8));
	FILT_free_processing_struct_d11(filtps, FALSE_m11, FALSE_m11, TRUE_m11, TRUE_m11);
	// resampled filtered data in buf1, resampled orig data in buf2

	// calculate unfiltered signal amplitude without DC offset
	if (calc_score_flag == TRUE_m11 || thresh_score) {
//                printf_m11("Line %d\n", __LINE__);
		sf8_p1 = sf8_buf_1;
		for  (total_template_sum = 0.0, i = cycles_in_data; --i;) {
//                        printf_m11("Line %d\n", __LINE__);
			for (template_sum = 0.0, j = resampled_samps_per_cycle; j--;){
//                                printf_m11("%ld %ld %ld one\n", sf8_p1 - sf8_buf_1, cycles_in_data * resampled_samps_per_cycle, i * resampled_samps_per_cycle + j);
				template_sum += *sf8_p1++;
//                                printf_m11("Line %d\n", __LINE__);
				
			}
//                        printf_m11("Line %d\n", __LINE__);
			template_mean = template_sum / (sf8) resampled_samps_per_cycle;
//                        printf_m11("Line %d\n", __LINE__);
			for (sf8_p1 -= resampled_samps_per_cycle, j = resampled_samps_per_cycle; j--;){
				sf8_temp_amplitude = (*sf8_p1++ - template_mean); //sf8_temp_amplitude used to avoid incrementing twice with ABS macro
				total_template_sum += ABS_m11(sf8_temp_amplitude);
			}
//                        printf_m11("%ld %ld %ld two\n", sf8_p1 - sf8_buf_1, cycles_in_data * resampled_samps_per_cycle, i * resampled_samps_per_cycle + j);
//                        printf_m11("Line %d\n", __LINE__);
		}
		for (template_sum = 0.0, j = extra_points; j--;)
			template_sum += *sf8_p1++;
		template_mean = template_sum / (sf8) extra_points;
		for (sf8_p1 -= extra_points, j = extra_points; j--;){
			sf8_temp_amplitude = (*sf8_p1++ - template_mean);
			total_template_sum += ABS_m11(sf8_temp_amplitude);
		}
		raw_amplitude = log((total_template_sum / (sf8) resampled_samps) + 1.0);
	}
	// resampled filtered data in buf1, resampled orig data in buf2
//                printf_m11("Line %d\n", __LINE__);
	// reorder points by position in cycle
	sf8_p3 = sf8_buf_3;
//        sf8_p1 = sf8_buf_1;        for (i=0;i<resampled_samps_per_cycle * cycles_in_data;i++)
//                printf_m11("%lf\n", *sf8_p1++);
//        exit_m11(1);
	for  (i = 0; i < extra_points; ++i) {
		sf8_p1 = sf8_buf_1 - resampled_samps_per_cycle + i;
		for (j = cycles_in_data; j--;)
			*sf8_p3++ = *(sf8_p1 += resampled_samps_per_cycle);
	}
	for  (; i < resampled_samps_per_cycle; ++i, sf8_p3++) {
		sf8_p1 = sf8_buf_1 - resampled_samps_per_cycle + i;
		for (j = cycles_in_data; --j;)
			*sf8_p3++ = *(sf8_p1 += resampled_samps_per_cycle);
	}
//                sf8_p3 = sf8_buf_3;
//                for (i=0;i<resampled_samps_per_cycle * cycles_in_data;i++)
//                        printf_m11("%lf\n", *sf8_p3++);
//                exit_m11(1);
	// resampled filtered data in buf1, resampled orig data in buf2, reordered data in buf3
//                printf_m11("Line %d\n", __LINE__);
	// median filter
	sf8_p1 = sf8_buf_1 - cycles_in_data;
	sf8_p3 = sf8_buf_3 - cycles_in_data;
	for (i = 0; i < extra_points; ++i)
		FILT_quantfilt_d11((sf8_p3 += cycles_in_data), (sf8_p1 += cycles_in_data), cycles_in_data, 0.5, cycles_in_template, FILT_TRUNCATE_d11);
	for (; i < resampled_samps_per_cycle; ++i)
		FILT_quantfilt_d11((sf8_p3 += cycles_in_data), (sf8_p1 += cycles_in_data), cycles_in_data - 1, 0.5, cycles_in_template, FILT_TRUNCATE_d11);
	// reordered median filtered data in buf1, resampled orig data in buf2, reordered data in buf3

//        printf_m11("Line %d\n", __LINE__);
	sf8_p1 = sf8_buf_1; // ptr to reordered mf data
	for(i = 0; i < extra_points; i++) {
		sf8_p3 = sf8_buf_3 + i - cycles_in_data; // ptr to temporal data
		for(j = cycles_in_data; j--;)
			*(sf8_p3 += resampled_samps_per_cycle) = *sf8_p1++;
	}
	for(; i < resampled_samps_per_cycle; i++, sf8_p1++) {
		sf8_p3 = sf8_buf_3 + i - cycles_in_data; // ptr to temporal data
		for(j = cycles_in_data; --j;)
			*(sf8_p3 += resampled_samps_per_cycle) = *sf8_p1++;
	}
	// reordered median filtered data in buf1, resampled orig data in buf2, temporally ordered templates in buf3
//        printf_m11("Line %d\n", __LINE__);
	// generate scores
	
	if (calc_score_flag == TRUE_m11 || thresh_score) {
		sf8_p1 = sf8_buf_1;
		sf8_p3 = sf8_buf_3;
		for (i = cycles_in_data; --i;) {
			for (template_sum = 0.0, j = resampled_samps_per_cycle; j--;){
				sf8_temp_amplitude = *sf8_p3++;
				template_sum += ABS_m11(sf8_temp_amplitude);
			}
			*sf8_p1++ = template_sum / (sf8) resampled_samps_per_cycle;
		}
		for (template_sum = 0.0, j = extra_points; j--;){
			sf8_temp_amplitude = *sf8_p3++;
			template_sum += ABS_m11(sf8_temp_amplitude);
		}
		*sf8_p1 = template_sum / (sf8) extra_points;
		qsort(sf8_buf_1, cycles_in_data, sizeof(sf8), FILT_sf8_sort_d11);
		if (cycles_in_data & 1)
			temp_sf8 = sf8_buf_1[cycles_in_data >> 1];
		else
			temp_sf8 = (sf8_buf_1[cycles_in_data >> 1] + sf8_buf_1[(cycles_in_data >> 1) - 1]) / 2.0;
		temp_sf8 = log(temp_sf8 + 1.0) / raw_amplitude;
		temp_sf8 = (temp_sf8 * sqrt(temp_sf8) * 254.0) + 1.5;
		score = (ui1) (temp_sf8 >= 255.0) ? 255 : (ui1) temp_sf8;
	} else {
		score = 0;
	}
	// buf1 free, resampled orig data in buf2, temporally ordered templates in buf3
	
	// remove noise
	if (remove_noise_flag == TRUE_m11 && (calc_score_flag == FALSE_m11 || (score > thresh_score))) {
		sf8_p3 = sf8_buf_3;
		sf8_p2 = sf8_buf_2;
		for (i = resampled_samps; i--;) {
			printf_m11("%lf\n", *sf8_p3);
			*sf8_p2++ -= *sf8_p3++;
		}
		sf8_p2 = sf8_buf_2;
		CMP_lin_interp_sf8_m11(sf8_buf_2, resampled_samps, sf8_buf_1, n_samps);
		si4_p = data;
		sf8_p1 = sf8_buf_1;
		for (i = n_samps; i--;)
			*si4_p++ = CMP_round_si4_m11(*sf8_p1++);
	}
	// denoised data in buf1 and data, resampled denoised data in buf2, temporally ordered templates in buf3

	// clean up
	free((void *) sf8_buf_1);
	free((void *) sf8_buf_2);
	free((void *) sf8_buf_3);
	
	return(score);
}


si4     FILT_sf8_sort_d11(const void *n1, const void *n2)
{
	
	if (*((sf8 *) n1) > *((sf8 *) n2))
		return(1);
	else if (*((sf8 *) n1) < *((sf8 *) n2))
		return(-1);
	
	
	return(0);
}


si4     FILT_sort_by_idx_d11(const void *n1, const void *n2)
{
	return(((FILT_NODE_d11 *) n1)->idx - ((FILT_NODE_d11 *) n2)->idx);
}


si4     FILT_sort_by_val_d11(const void *n1, const void *n2)
{
	sf8     v1, v2;
	
	
	v1 = ((FILT_NODE_d11 *) n1)->val;
	v2 = ((FILT_NODE_d11 *) n2)->val;
	
	if (v1 > v2)
		return(1);
	else if (v1 < v2)
		return(-1);
	return(0);
}


void	FILT_unsymmeig_d11(sf16 **a, si4 poles, FILT_LONG_COMPLEX_d11 *eigs)
{
	FILT_balance_d11(a, poles);
	FILT_elmhes_d11(a, poles);
	FILT_hqr_d11(a, poles, eigs);
	
	
	return;
}



//***********************************************************************//
//************************  END FILTER FUNCTIONS  ***********************//
//***********************************************************************//


void    free_globals_d11(TERN_m11 cleanup_for_exit)
{
	
	if (globals_d11 == NULL) {
		warning_message_m11("%s(): trying to free a NULL GLOBALS_d11 structure => returning with no action", __FUNCTION__);
		return;
	}
	
	// alloc tracking
	if (globals_d11->alloc_tracking == TRUE_m11 && globals_d11->ae_curr_allocated_bytes)
		warning_message_m11("%s(): [alloc tracking] %lu bytes currently allocated\n", __FUNCTION__, globals_d11->ae_curr_allocated_bytes);
	if (globals_d11->alloc_entities != NULL)
		free(globals_d11->alloc_entities);

	// SK matrix
	if (globals_d11->sk_matrix != NULL)
		free((void *) globals_d11->sk_matrix);
	
	// CMP tables
	if (globals_d11->CMP_normal_CDF_table != NULL)
		free((void *) globals_d11->CMP_normal_CDF_table);
	if (globals_d11->CMP_VDS_threshold_map != NULL)
		free((void *) globals_d11->CMP_VDS_threshold_map);
	
	free(globals_d11);
	
	return;
}


void	free_transmission_info_d11(TRANSMISSION_INFO_d11 **trans_info_ptr, TERN_m11 free_structure)
{
	TRANSMISSION_INFO_d11 	*trans_info;
	
	
	trans_info = *trans_info_ptr;
	if (trans_info == NULL) {
		warning_message_m11("%s(): attempting to free NULL pointer", __FUNCTION__);
		return;
	}
	
	close_transmission_d11(trans_info);

	if (trans_info->buffer != NULL)
		free((void *) trans_info->buffer);
		
	if (free_structure == TRUE_m11) {
		free((void *) trans_info);
		*trans_info_ptr = NULL;
	} else {
		trans_info->buffer = NULL;
		trans_info->buffer_bytes = 0;
	}

	return;
}

	    
cpu_set_t_d11	*generate_cpu_set_d11(si1 *affinity_str, cpu_set_t_d11 *passed_cpu_set_p)
{
	TERN_m11        	not_flag, lessthan_flag, greaterthan_flag;
	si1             	*aff_str;
	si4             	i, n_cpus, start_num, end_num, cpus_set;
	cpu_set_t_d11 		*cpu_set_p;

	
	/* affinity string examples
	"a"/"all" set to all logical cpus  // same as not specifying a cpu set on most OS's
	"0" set to cpu 0  (read: "0")
	"~0" set to any cpu except 0  (read: "not 0")  // this is the default setting if no cpus are specified
	"<2" set to any cpu less than 2  (read: "less than 2")
	"~<2" set to any cpu greater than or equal to 2  (read: "not less than 2")
	">2" set to any cpu greater than 2  (read: "greater than 2")
	"~>2" set to any cpu less than or equal to 2  (read: "not greater than 2")
	"2-5" set to cpus 2 through 5  (read: "2 through 5")
	"~2-5" set to any cpu except 2 through 5  (read: "not 2 through 5")
	*/
	
	if (affinity_str == NULL)
		return(NULL);
	if (!*affinity_str)
		return(NULL);

	if (passed_cpu_set_p == NULL)  // up to caller to receive & free
		cpu_set_p = (cpu_set_t_d11 *) malloc(sizeof(cpu_set_t_d11));
	else
		cpu_set_p = passed_cpu_set_p;

	// get logical cpus
	if (globals_d11->cpu_info.logical_cores == 0)
		get_cpu_info_d11();
	n_cpus = globals_d11->cpu_info.logical_cores;
	if (n_cpus == 1) {
	#ifdef LINUX_m11
		CPU_ZERO(cpu_set_p);
		CPU_SET(0, cpu_set_p);
	#endif
	#if defined MACOS_m11 || defined WINDOWS_m11
		*cpu_set_p = 1;
	#endif
		return(cpu_set_p);
	}

	// parse affinity string
	aff_str = affinity_str;
	not_flag = FALSE_m11;
	if (*aff_str == '~') {
		not_flag = TRUE_m11;
		++aff_str;
	}
	if (*aff_str == 'a')
		end_num = n_cpus - 1;
	lessthan_flag = greaterthan_flag = FALSE_m11;
	if (*aff_str == '<') {
		lessthan_flag = TRUE_m11;
		++aff_str;
	} else if (*aff_str == '>') {
		greaterthan_flag = TRUE_m11;
		++aff_str;
	}
	start_num = 0;
	while (*aff_str >= '0' && *aff_str <= '9') {
		start_num *= 10;
		start_num += *aff_str - '0';
		++aff_str;
	}
	if (*aff_str == '-') {
		++aff_str;
		end_num = 0;
		while (*aff_str >= '0' && *aff_str <= '9') {
			end_num *= 10;
			end_num += *aff_str - '0';
			++aff_str;
		}
	} else {
		if (lessthan_flag == TRUE_m11) {
			end_num = start_num;
			start_num = 0;
		} else if (greaterthan_flag == TRUE_m11) {
			++start_num;
			end_num = n_cpus - 1;
		} else {
			end_num = start_num;
		}
	}
	if (start_num > (n_cpus - 1))
		start_num = (n_cpus - 1);
	if (end_num > (n_cpus - 1))
		end_num = (n_cpus - 1);

	// build affinity set
#ifdef LINUX_m11
	if (not_flag == TRUE_m11) {
		for (i = 0; i < n_cpus; ++i)  // set all bits
			CPU_SET(i, cpu_set_p);
		for (cpus_set = n_cpus, i = start_num; i <= end_num; ++i, --cpus_set)
			CPU_CLR(i, cpu_set_p);
	} else {
		CPU_ZERO(cpu_set_p);
		for (cpus_set = 0, i = start_num; i <= end_num; ++i, ++cpus_set)
			CPU_SET(i, cpu_set_p);
	}
#endif

#if defined MACOS_m11 || defined WINDOWS_m11
	cpu_set_t_d11	mask;

	mask = 1 << start_num;
	*cpu_set_p = 0;
	for (cpus_set = 0, i = start_num; i <= end_num; ++i, mask <<= 1, ++cpus_set)
		*cpu_set_p |= mask;
	if (not_flag == TRUE_m11) {
		*cpu_set_p = ~*cpu_set_p;
		cpus_set = n_cpus - cpus_set;
	}
#endif
	
	if (cpus_set == 0) {
		warning_message_m11("%s(): no cpus specified => setting to ~0\n");
#ifdef LINUX_m11
		CPU_CLR(0, cpu_set_p);
		for (i = 1; i < n_cpus; ++i)  // set all remaining bits
			CPU_SET(i, cpu_set_p);
#endif
#if defined MACOS_m11 || defined WINDOWS_m11
		*cpu_set_p = ~((cpu_set_t_d11) 1);
#endif
	}

	return(cpu_set_p);
}


void	get_cpu_info_d11(void)
{
	CPU_INFO_d11	*cpu_info;
	
	
	cpu_info = &globals_d11->cpu_info;
	cpu_info->endianness = get_cpu_endianness_m11();

#ifdef LINUX_m11
	cpu_info->logical_cores = (si4) get_nprocs_conf();
#endif  // LINUX_m11
	
#ifdef MACOS_m11
	size_t			len;
	
	len = sizeof(si4);
	
	sysctlbyname("machdep.cpu.core_count", &cpu_info->physical_cores, &len, NULL, 0);
	sysctlbyname("machdep.cpu.thread_count", &cpu_info->logical_cores, &len, NULL, 0);
	if (cpu_info->physical_cores < cpu_info->logical_cores)
		cpu_info->hyperthreading = TRUE_m11;
#endif  // MACOS_m11
	
#ifdef WINDOWS_m11
	SYSTEM_INFO	sys_info;
	
	GetSystemInfo(&sys_info);  // I think this returns logical, not physical, cores
	cpu_info->logical_cores = (si4) sys_info.dwNumberOfProcessors;
#endif  // WINDOWS_m11
	
	return;
	
}


si1	*get_DHN_license_path_d11(si1 *path)
{
	si1	*env_var;
	
	
	if (path == NULL)
		path = (si1 *) calloc((size_t) FULL_FILE_NAME_BYTES_m11, sizeof(si1));
	
	env_var = getenv("DHN_SOFTWARE_PATH");

#if defined MACOS_m11 || defined LINUX_m11
	if (env_var == NULL) {
		env_var = getenv("HOME");
		if (env_var == NULL) {
			warning_message_m11("%s(): neither \"DHN_SOFTWARE_PATH\" nor \"HOME\" are defined in the environment => returning NULL\n", __FUNCTION__);
			return(NULL);
		}
		sprintf_m11(path, "%s/%s/DHN_Licenses.txt", env_var, DHN_SOFTWARE_PATH_DEFAULT_d11);
	} else {
		sprintf_m11(path, "%s/DHN_Licenses.txt", env_var);
	}
#endif
#ifdef WINDOWS_m11
	si1	*home_drive, *home_path;
	
	if (env_var == NULL) {
		home_drive = getenv("HOMEDRIVE");
		home_path = getenv("HOMEPATH");
		if (home_path == NULL || home_drive == NULL) {
			warning_message_m11("%s(): neither \"DHN_SOFTWARE_PATH\" nor \"HOMEPATH\" are defined in the environment => returning NULL\n", __FUNCTION__);
			return(NULL);
		}
		sprintf_m11(path, "%s%s/%s/DHN_Licenses.txt", home_drive, home_path, DHN_SOFTWARE_PATH_DEFAULT_d11);
	}
	else {
		sprintf_m11(path, "%s/DHN_Licenses.txt", env_var);
	}
#endif

	return(path);
}


void	*get_in_addr_d11(struct sockaddr *sa)	// get sockaddr, IPv4 or IPv6
{
	if (sa->sa_family == AF_INET)
		return(&(((struct sockaddr_in *) sa)->sin_addr));

	return(&(((struct sockaddr_in6 *) sa)->sin6_addr));
}


NETWORK_PARAMETERS_d11	*get_lan_ipv4_address_d11(NETWORK_PARAMETERS_d11 *np)
{
	extern GLOBALS_m11	*globals_m11;
	si1			command[1024], *buffer, *c;
	si4			ret_val;
	si8			sz;
	FILE			*fp;
	

	if (np == NULL)
		np = (NETWORK_PARAMETERS_d11 *) calloc((size_t) 1, sizeof(NETWORK_PARAMETERS_d11));
	
	*np->host_name = 0;
	if (gethostname(np->host_name, sizeof(np->host_name)) == -1) {
		warning_message_m11("%s(): cannot get host name\n", __FUNCTION__);
		return(NULL);
	}
	
#ifdef MACOS_m11
	sprintf_m11(command, "route -n get default | grep interface > %s 2> %s", globals_m11->temp_file, NULL_DEVICE_m11);
#endif
#ifdef LINUX_m11
	sprintf_m11(command, "ip route get 8.8.8.8 > %s 2> %s", globals_m11->temp_file, NULL_DEVICE_m11);
#endif
#ifdef WINDOWS_m11
	sprintf_m11(command, "route PRINT -4 0.0.0.0 > %s 2> %s", globals_m11->temp_file, NULL_DEVICE_m11);
#endif
	force_behavior_m11(RETURN_ON_FAIL_m11 | SUPPRESS_ALL_OUTPUT_m11);
	ret_val = system_m11(command, FALSE_m11, __FUNCTION__, __LINE__,  USE_GLOBAL_BEHAVIOR_m11);
	force_behavior_m11(RESTORE_BEHAVIOR_m11);
	if (ret_val) // probably no internet connection, otherwise route() error
		return(NULL);
	fp = fopen_m11(globals_m11->temp_file, "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
       
	// get file length
	sz = file_length_m11(fp, NULL);

	// read route() output
	buffer = (si1 *) calloc((size_t) sz, sizeof(si1));
	fread_m11(buffer, sizeof(si1), (size_t) sz, fp, globals_m11->temp_file, __FUNCTION__, __LINE__, EXIT_ON_FAIL_m11);
	fclose(fp);

	force_behavior_m11(RETURN_ON_FAIL_m11);
#ifdef MACOS_m11
	si1	interface_name[32];
	
	// parse route() output to get internet interface name
	if ((c = STR_match_end_m11("interface: ", buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"interface: \" in output of route()", __FUNCTION__);
		np->LAN_IPv4_address_string[0] = 0;
		memset(np->LAN_IPv4_address_bytes, 0, IPV4_ADDRESS_BYTES_d11);
	}
	if (c != NULL) {
		sscanf(c, "%s", interface_name);
		
		// send ifconfig() output to temp file
		sprintf_m11(command, "ifconfig %s > %s 2> %s", interface_name, globals_m11->temp_file, NULL_DEVICE_m11);
		ret_val = system_m11(command, FALSE_m11, __FUNCTION__, __LINE__,  USE_GLOBAL_BEHAVIOR_m11);
		if (ret_val) {
			force_behavior_m11(RESTORE_BEHAVIOR_m11);
			return(NULL);
		}
		fp = fopen_m11(globals_m11->temp_file, "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	
		// get file length
		sz = file_length_m11(fp, NULL);
	
		// read ifconfig() output
		buffer = (si1 *) realloc((void *) buffer, (size_t) sz);
		fread_m11(buffer, sizeof(si1), (size_t) sz, fp, globals_m11->temp_file, __FUNCTION__, __LINE__, EXIT_ON_FAIL_m11);
		fclose(fp);
		if ((c = STR_match_end_m11("inet ", buffer)) == NULL) {
			error_message_m11("%s(): Could not match pattern \"inet \" in output of ifconfig()", __FUNCTION__);
			np->LAN_IPv4_address_string[0] = 0;
			memset(np->LAN_IPv4_address_bytes, 0, IPV4_ADDRESS_BYTES_d11);
		}
	}
#endif  // MACOS_m11
#ifdef LINUX_m11
	// parse route() output to get internet ip address
	if ((c = STR_match_end_m11("src ", buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"src \" in output of ip route()", __FUNCTION__);
		np->LAN_IPv4_address_string[0] = 0;
		memset(np->LAN_IPv4_address_bytes, 0, IPV4_ADDRESS_BYTES_d11);
	}
#endif  // LINUX_m11
#ifdef WINDOWS_m11
	// parse route() output to get internet ip address
	if ((c = STR_match_end_m11("0.0.0.0", buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"0.0.0.0\" in output of route()", __FUNCTION__);
		np->LAN_IPv4_address_string[0] = 0;
		memset(np->LAN_IPv4_address_bytes, 0, IPV4_ADDRESS_BYTES_d11);
	}

#endif
#if defined MACOS_m11 || defined LINUX_m11
	// get internet ip address
	if (c != NULL) {
		sscanf(c, "%s", np->LAN_IPv4_address_string);
		sscanf(c, "%hhu.%hhu.%hhu.%hhu", np->LAN_IPv4_address_bytes, np->LAN_IPv4_address_bytes + 1, np->LAN_IPv4_address_bytes + 2, np->LAN_IPv4_address_bytes + 3);
	}
#endif
#ifdef WINDOWS_m11
	si1	*junk_str = command;

	// get internet ip address
	if (c != NULL) {
		sscanf(c, "%s%s%s", junk_str, junk_str, np->LAN_IPv4_address_string);
		sscanf(np->LAN_IPv4_address_string, "%hhu.%hhu.%hhu.%hhu", np->LAN_IPv4_address_bytes, np->LAN_IPv4_address_bytes + 1, np->LAN_IPv4_address_bytes + 2, np->LAN_IPv4_address_bytes + 3);
	}
#endif
	
	force_behavior_m11(RESTORE_BEHAVIOR_m11);
	free((void *) buffer);
	
	return(np);
}


ui4	get_machine_code_d11(void)
{
	extern GLOBALS_m11	*globals_m11;
	si1			command[1024], *buf, *machine_sn;
	si8			file_length, block_bytes;
	FILE			*fp;
	
	
	// get machine serial number
#ifdef LINUX_m11
	if (file_exists_m11(LS_MC_FILE_NAME_d11) == FILE_EXISTS_m11) {
		LS_MC_FILE_STRUCT_d11	mc_file_struct;
		
		fp = fopen_m11(LS_MC_FILE_NAME_d11, "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		fread_m11((void *) &mc_file_struct, sizeof(ui1), (size_t) LS_MC_FILE_STRUCT_BYTES_d11, fp, LS_MC_FILE_NAME_d11, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		fclose(fp);
		
		globals_d11->LS_machine_code = mc_file_struct.machine_code;
		strcpy(globals_d11->machine_serial, mc_file_struct.serial_number);

		return(globals_d11->LS_machine_code);
	} else {
		// out example: 2UA75217NB
		sprintf_m11(command, "sudo cat /sys/devices/virtual/dmi/id/product_serial > %s", globals_m11->temp_file); // system prompts for password, unless run as root
		// other way:
		// out example: Serial Number: 2UA75217NB
		// command = "echo <root_password> | sudo -S -u root dmidecode -t system | grep Serial";  // don't want to need root password
	}
#endif  // LINUX_m11
#ifdef MACOS_m11
	// out example: "IOPlatformSerialNumber" = "C02XK4D2JGH6"  // quotes are part of output
	sprintf_m11(command, "ioreg -l | grep IOPlatformSerialNumber > %s", globals_m11->temp_file);
#endif  // MACOS_m11
#ifdef WINDOWS_m11
	// out example: SerialNumber\nC02RP18FG8WM
	sprintf_m11(command, "wmic bios get serialnumber > %s", globals_m11->temp_file);
#endif  // WINDOWS_m11

	system_m11(command, FALSE_m11, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	fp = fopen_m11(globals_m11->temp_file, "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	file_length = file_length_m11(fp, NULL);
	buf = calloc((size_t) file_length, sizeof(si1));
	fread_m11((void *) buf, sizeof(si1), (size_t) file_length, fp, globals_m11->temp_file, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	fclose(fp);

#ifdef LINUX_m11
	machine_sn = buf;
	buf[file_length - 1] = 0;
#endif  // LINUX_m11
#ifdef MACOS_m11
	machine_sn = STR_match_end_m11("IOPlatformSerialNumber\" = \"", buf);
	buf[file_length - 2] = 0;
#endif  // MACOS_m11
#ifdef WINDOWS_m11
	buf[file_length - 7] = buf[file_length - 8] = 0;
	wchar2char_m11(buf, (wchar_t *) buf);
	machine_sn = STR_match_end_m11("SerialNumber  \r\n", buf);  // <sp><sp><cr><lf>
#endif  // WINDOWS_m11

	// get CRC of machine serial number
	block_bytes = strcpy_m11(globals_d11->machine_serial, machine_sn);
	globals_d11->LS_machine_code = CRC_calculate_m11((ui1 *) machine_sn, block_bytes);
	
	free(buf);

	return(globals_d11->LS_machine_code);
}


#ifdef LINUX_m11
NETWORK_PARAMETERS_d11	*get_network_parameters_d11(si1 *interface_name, NETWORK_PARAMETERS_d11 *np)
{
	extern GLOBALS_m11	*globals_m11;
	si1             	temp_str[256], *buffer, *c, *pattern;
	si4             	i, ret_val;
	si8             	sz, len;
	FILE            	*fp;

	
	if (np == NULL)
		np = (NETWORK_PARAMETERS_d11 *) calloc((size_t) 1, sizeof(NETWORK_PARAMETERS_d11));
	
	if (gethostname(np->host_name, sizeof(np->host_name)) == -1)
		warning_message_m11("%s(): cannot get host_name\n", __FUNCTION__);

	// send ifconfig() output to temp file
	sprintf_m11(temp_str, "ifconfig %s > %s 2> %s", interface_name, globals_m11->temp_file, NULL_DEVICE_m11);
	ret_val = system_m11(temp_str, FALSE_m11, __FUNCTION__, __LINE__,  USE_GLOBAL_BEHAVIOR_m11);
	if (ret_val)
		return(NULL);
	fp = fopen_m11(globals_m11->temp_file, "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
       
	// get file length
	sz = file_length_m11(fp, NULL);

	// read ifconfig() output
	buffer = calloc((size_t) sz, sizeof(si1));
	fread_m11(buffer, sizeof(si1), (size_t) sz, fp, globals_m11->temp_file, __FUNCTION__, __LINE__, EXIT_ON_FAIL_m11);
	fclose(fp);

	// parse ifconfig() output
	force_behavior_m11(RETURN_ON_FAIL_m11);
	
	pattern = "mtu ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"%s\" in output of ifconfig() for interface \"%s\"", __FUNCTION__, pattern, interface_name);
		np->MTU = 0;
	} else {
		sscanf(c, "%d", &np->MTU);
	}
	
	pattern = "ether ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"%s\" in output of ifconfig() for interface \"%s\"", __FUNCTION__, pattern, interface_name);
		np->MAC_address_string_UC[0] = np->MAC_address_string_LC[0] = 0;
		memset(np->MAC_address_bytes, 0, MAC_ADDRESS_BYTES_d11);
	} else {
		sscanf(c, "%s", np->MAC_address_string_UC);
		len = strcpy_m11(np->MAC_address_string_LC, np->MAC_address_string_UC);
		for (i = 0; i < len; ++i) {
			if (np->MAC_address_string_UC[i] >= 'a' && np->MAC_address_string_UC[i] <= 'f')
				np->MAC_address_string_UC[i] -= 32;
			if (np->MAC_address_string_LC[i] >= 'A' && np->MAC_address_string_LC[i] <= 'F')
				np->MAC_address_string_LC[i] += 32;
		}
		sscanf(np->MAC_address_string_LC, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", np->MAC_address_bytes, np->MAC_address_bytes + 1, np->MAC_address_bytes + 2, np->MAC_address_bytes + 3, np->MAC_address_bytes + 4, np->MAC_address_bytes + 5);
	}

	pattern = "inet ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"%s\" in output of ifconfig() for interface \"%s\"", __FUNCTION__, pattern, interface_name);
		np->LAN_IPv4_address_string[0] = 0;
		memset(np->LAN_IPv4_address_bytes, 0, IPV4_ADDRESS_BYTES_d11);
	} else {
		sscanf(c, "%s", np->LAN_IPv4_address_string);
		sscanf(c, "%hhu.%hhu.%hhu.%hhu", np->LAN_IPv4_address_bytes, np->LAN_IPv4_address_bytes + 1, np->LAN_IPv4_address_bytes + 2, np->LAN_IPv4_address_bytes + 3);
	}

	pattern = "netmask ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"%s\" in output of ifconfig() for interface \"%s\"", __FUNCTION__, pattern, interface_name);
		np->LAN_IPv4_subnet_mask_string[0] = 0;
		memset(np->LAN_IPv4_subnet_mask_bytes, 0, IPV4_ADDRESS_BYTES_d11);
	} else {
		sscanf(c, "%s", np->LAN_IPv4_subnet_mask_string);
		sscanf(c, "%hhu.%hhu.%hhu.%hhu", np->LAN_IPv4_subnet_mask_bytes, np->LAN_IPv4_subnet_mask_bytes + 1, np->LAN_IPv4_subnet_mask_bytes + 2, np->LAN_IPv4_subnet_mask_bytes + 3);
	}

	pattern = "UP";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL)
		np->active = FALSE_m11;
	else
		np->active = TRUE_m11;
 
	pattern = "RUNNING";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL)
		np->plugged_in = FALSE_m11;
	else
		np->plugged_in = TRUE_m11;

	// send ethtool() output to temp file
	sprintf_m11(temp_str, "ethtool %s > %s 2> %s", interface_name, globals_m11->temp_file, NULL_DEVICE_m11);
	ret_val = system_m11(temp_str, FALSE_m11, __FUNCTION__, __LINE__,  USE_GLOBAL_BEHAVIOR_m11);
	if (ret_val)
		return(NULL);
	fp = fopen_m11(globals_m11->temp_file, "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
       
	// get file length
	sz = file_length_m11(fp, NULL);

	// read ethtool() output
	buffer = realloc((void *) buffer, (size_t) sz);
	fread_m11(buffer, sizeof(si1), (size_t) sz, fp, globals_m11->temp_file, __FUNCTION__, __LINE__, EXIT_ON_FAIL_m11);
	fclose(fp);

	pattern = "Speed: ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"%s\" in output of ethtool() for interface \"%s\"", __FUNCTION__, pattern, interface_name);
		np->link_speed[0] = 0;
	} else {
		sscanf(c, "%s", np->link_speed);
	}
	pattern = "Duplex: ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"%s\" in output of ethtool() for interface \"%s\"", __FUNCTION__, pattern, interface_name);
		np->duplex[0] = 0;
	} else {
		sscanf(c, "%s", np->duplex);
	}
	
	// get WAN IPV4 address
	get_wan_ipv4_address_d11(np);

	// clean up
	force_behavior_m11(RESTORE_BEHAVIOR_m11);
	free(buffer);

	return(np);
}
#endif  // LINUX_m11


#ifdef MACOS_m11
NETWORK_PARAMETERS_d11	*get_network_parameters_d11(si1 *interface_name, NETWORK_PARAMETERS_d11 *np)
{
	extern GLOBALS_m11	*globals_m11;
	si1             	temp_str[256], *buffer, *c, *pattern;
	si4             	i, ret_val;
	si8             	sz, len;
	FILE            	*fp;

	
	if (np == NULL)
		np = (NETWORK_PARAMETERS_d11 *) calloc((size_t) 1, sizeof(NETWORK_PARAMETERS_d11));
	
	if (gethostname(np->host_name, sizeof(np->host_name)) == -1)
		warning_message_m11("%s(): cannot get host_name\n", __FUNCTION__);

	// send ifconfig() output to temp file
	sprintf_m11(temp_str, "ifconfig %s > %s 2> %s", interface_name, globals_m11->temp_file, NULL_DEVICE_m11);
	ret_val = system_m11(temp_str, FALSE_m11, __FUNCTION__, __LINE__,  USE_GLOBAL_BEHAVIOR_m11);
	if (ret_val)
		return(NULL);
	fp = fopen_m11(globals_m11->temp_file, "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
       
	// get file length
	sz = file_length_m11(fp, NULL);

	// read ifconfig() output
	buffer = calloc((size_t) sz, sizeof(si1));
	fread_m11(buffer, sizeof(si1), (size_t) sz, fp, globals_m11->temp_file, __FUNCTION__, __LINE__, EXIT_ON_FAIL_m11);
	fclose(fp);

	// parse ifconfig() output
	force_behavior_m11(RETURN_ON_FAIL_m11);
	
	pattern = "mtu ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"%s\" in output of ifconfig() for interface \"%s\"\n", __FUNCTION__, pattern, interface_name);
		np->MTU = 0;
	} else {
		sscanf(c, "%d", &np->MTU);
	}
	
	pattern = "ether ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"%s\" in output of ifconfig() for interface \"%s\"\n", __FUNCTION__, pattern, interface_name);
		np->MAC_address_string_UC[0] = np->MAC_address_string_LC[0] = 0;
		memset(np->MAC_address_bytes, 0, MAC_ADDRESS_BYTES_d11);
	} else {
		sscanf(c, "%s", np->MAC_address_string_UC);
		len = strcpy_m11(np->MAC_address_string_LC, np->MAC_address_string_UC);
		for (i = 0; i < len; ++i) {
			if (np->MAC_address_string_UC[i] >= 'a' && np->MAC_address_string_UC[i] <= 'f')
				np->MAC_address_string_UC[i] -= 32;
			if (np->MAC_address_string_LC[i] >= 'A' && np->MAC_address_string_LC[i] <= 'F')
				np->MAC_address_string_LC[i] += 32;
		}
		sscanf(np->MAC_address_string_LC, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", np->MAC_address_bytes, np->MAC_address_bytes + 1, np->MAC_address_bytes + 2, np->MAC_address_bytes + 3, np->MAC_address_bytes + 4, np->MAC_address_bytes + 5);
	}

	pattern = "inet ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"%s\" in output of ifconfig() for interface \"%s\".\nCheck that cable is plugged in\n", __FUNCTION__, pattern, interface_name);
		np->LAN_IPv4_address_string[0] = 0;
		memset(np->LAN_IPv4_address_bytes, 0, IPV4_ADDRESS_BYTES_d11);
	} else {
		sscanf(c, "%s", np->LAN_IPv4_address_string);
		sscanf(c, "%hhu.%hhu.%hhu.%hhu", np->LAN_IPv4_address_bytes, np->LAN_IPv4_address_bytes + 1, np->LAN_IPv4_address_bytes + 2, np->LAN_IPv4_address_bytes + 3);
	}

	pattern = "netmask 0x";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"%s\" in output of ifconfig() for interface \"%s\"\n", __FUNCTION__, pattern, interface_name);
		np->LAN_IPv4_subnet_mask_string[0] = 0;
		memset(np->LAN_IPv4_subnet_mask_bytes, 0, IPV4_ADDRESS_BYTES_d11);
	} else {
		sscanf(c, "%02hhx%02hhx%02hhx%02hhx", np->LAN_IPv4_subnet_mask_bytes, np->LAN_IPv4_subnet_mask_bytes + 1, np->LAN_IPv4_subnet_mask_bytes + 2, np->LAN_IPv4_subnet_mask_bytes + 3);
		sprintf_m11(np->LAN_IPv4_subnet_mask_string, "%hhu.%hhu.%hhu.%hhu", np->LAN_IPv4_subnet_mask_bytes[0], np->LAN_IPv4_subnet_mask_bytes[1], np->LAN_IPv4_subnet_mask_bytes[2], np->LAN_IPv4_subnet_mask_bytes[3]);
	}

	pattern = "media: ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"%s\" in output of ifconfig() for interface \"%s\"\n", __FUNCTION__, pattern, interface_name);
		np->link_speed[0] = np->duplex[0] = 0;
	} else {
		sscanf(c, "%s %s", np->link_speed, np->duplex);
	}

	pattern = "status: ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		error_message_m11("%s(): Could not match pattern \"%s\" in output of ifconfig() for interface \"%s\"\n", __FUNCTION__, pattern, interface_name);
		np->active = UNKNOWN_m11;
	} else {
		sscanf(c, "%s", temp_str);
		if (strcmp(temp_str, "active") == 0)
			np->active = TRUE_m11;
		else if (strcmp(temp_str, "inactive") == 0)
			np->active = FALSE_m11;
		else {
			error_message_m11("%s(): Unrecognized value (\"%s\") for field \"%s\" in output of ifconfig() for interface \"%s\"\n", __FUNCTION__, temp_str, pattern, interface_name);
			np->active = UNKNOWN_m11;
		}
	}
	
	// get WAN IPV4 address
	get_wan_ipv4_address_d11(np);
	
	// clean up
	force_behavior_m11(RESTORE_BEHAVIOR_m11);
	free(buffer);

	return(np);
}
#endif  // MACOS_m11


TERN_m11	get_terminal_entry_d11(si1 *prompt, si1 type, void *buffer, void *default_input, TERN_m11 required, TERN_m11 validate)
{
	si8	items;
	si1	local_buffer[128], local_prompt[128];
	
	
	if (default_input != NULL) {
		if (type == RC_STRING_TYPE_d11)
			if (*((si1 *) default_input) == 0)
				default_input = NULL;
	}

GET_TERMINAL_ENTRY_RETRY_d11:
	
	if (required == TRUE_m11)
		sprintf_m11(local_prompt, "* %s", prompt);
	else
		strcpy(local_prompt, prompt);

	// clear
	switch (type) {
		case RC_STRING_TYPE_d11:
			*((si1 *) buffer) = 0;
			break;
		case RC_FLOAT_TYPE_d11:
			*((sf8 *) buffer) = (sf8) 0;
			break;
		case RC_INTEGER_TYPE_d11:
			*((si8 *) buffer) = (si8) 0;
			break;
		case RC_TERNARY_TYPE_d11:
			*((si1 *) buffer) = (si1) 0;
			break;
		default:
			warning_message_m11("%s(): unrecognized data type => returning FALSE\n", __FUNCTION__);
			return(FALSE_m11);
	}

	if (default_input != NULL) {
		switch (type) {
			case RC_STRING_TYPE_d11:
				sprintf_m11(local_prompt, "%s [%s]", local_prompt, (si1 *) default_input);
				break;
			case RC_FLOAT_TYPE_d11:
				sprintf_m11(local_prompt, "%s [%lf]", local_prompt, *((sf8 *) default_input));
				break;
			case RC_INTEGER_TYPE_d11:
				sprintf_m11(local_prompt, "%s [%ld]", local_prompt, *((si8 *) default_input));
				break;
			case RC_TERNARY_TYPE_d11:
				sprintf_m11(local_prompt, "%s [%hhd]", local_prompt, *((si1 *) default_input));
				break;
		}
	}
		
	printf_m11("%s: ", local_prompt);
	fflush(stdout);
	items = scanf("%[^\n]", (si1 *) local_buffer);
	if (items) {
		switch (type) {
			case RC_STRING_TYPE_d11:
				strcpy((si1 *) buffer, local_buffer);
				break;
			case RC_FLOAT_TYPE_d11:
				*((sf8 *) buffer) = strtod(local_buffer, NULL);
				break;
			case RC_INTEGER_TYPE_d11:
				*((si8 *) buffer) = (si8) strtol(local_buffer, NULL, 10);
				break;
			case RC_TERNARY_TYPE_d11:
				*((si1 *) buffer) = (si1) strtol(local_buffer, NULL, 10);
				break;
		}
	}
	getchar();  // clear '\n' from stdin
	
	if (items == 0 && default_input != NULL) {
		switch (type) {
			case RC_STRING_TYPE_d11:
				strcpy((si1 *) buffer, (si1 *) default_input);
				break;
			case RC_FLOAT_TYPE_d11:
				*((sf8 *) buffer) = *((sf8 *) default_input);
				break;
			case RC_INTEGER_TYPE_d11:
				*((si8 *) buffer) = *((si8 *) default_input);
				break;
			case RC_TERNARY_TYPE_d11:
				*((si1 *) buffer) = *((si1 *) default_input);
				break;
		}
	}

	if (validate == TRUE_m11) {
		if (items == 1 || default_input != NULL) {
			switch (type) {
				case RC_STRING_TYPE_d11:
					printf_m11("\tIs %s\"%s\"%s correct (y/n): ", TC_RED_m11, (si1 *) buffer, TC_RESET_m11);
					break;
				case RC_FLOAT_TYPE_d11:
					printf_m11("\tIs %s%lf%s correct (y/n): ", TC_RED_m11, *((sf8 *) buffer), TC_RESET_m11);
					break;
				case RC_INTEGER_TYPE_d11:
					printf_m11("\tIs %s%ld%s correct (y/n): ", TC_RED_m11, *((si8 *) buffer), TC_RESET_m11);
					break;
				case RC_TERNARY_TYPE_d11:
					printf_m11("\tIs %s%hhd%s correct (y/n): ", TC_RED_m11, *((si1 *) buffer), TC_RESET_m11);
					break;
			}
		} else {
			printf_m11("\tIs %s<no entry>%s correct (y/n): ", TC_RED_m11, TC_RESET_m11);
		}
		fflush(stdout);
		*local_buffer = 0;
		scanf("%[^\n]", local_buffer);
		getchar();  // clear '\n' from stdin
		if (*local_buffer == 'y' || *local_buffer == 'Y') {
			required = FALSE_m11;
		} else {
			default_input = NULL;  // in case user wanted no entry instead of default
			goto GET_TERMINAL_ENTRY_RETRY_d11;
		}
	}

	if (items == 0 && required == TRUE_m11) {
		*local_buffer = 0;
		warning_message_m11("No entry in required field\n");
		goto GET_TERMINAL_ENTRY_RETRY_d11;
	}
	
	putchar_m11('\n');

	return(TRUE_m11);
}


NETWORK_PARAMETERS_d11 *get_wan_ipv4_address_d11(NETWORK_PARAMETERS_d11 *np)
{
	extern GLOBALS_m11	*globals_m11;
	si1		temp_str[1024], *buffer, *pattern, *c, retry_count;
	si4		ret_val;
	si8		sz;
	FILE		*fp;
	
	
	if (np == NULL)
		np = (NETWORK_PARAMETERS_d11 *) calloc((size_t) 1, sizeof(NETWORK_PARAMETERS_d11));

	// get WAN IPV4 address
	retry_count = 0;
	
#if defined MACOS_m11 || defined LINUX_m11
	sprintf_m11(temp_str, "curl -s checkip.dyndns.org > %s 2> %s", globals_m11->temp_file, NULL_DEVICE_m11);
#endif
#ifdef WINDOWS_m11
	sprintf_m11(temp_str, "curl.exe -s checkip.dyndns.org > %s 2> %s", globals_m11->temp_file, NULL_DEVICE_m11);
#endif
	
GET_WAN_IPV4_ADDR_RETRY_d11:

	ret_val = system_m11(temp_str, FALSE_m11, __FUNCTION__, __LINE__, RETURN_ON_FAIL_m11 | SUPPRESS_ALL_OUTPUT_m11 | RETRY_ONCE_m11);
	if (ret_val) {
		if (check_internet_connection_d11() == FALSE_m11)
			warning_message_m11("%s(): no internet connection\n", __FUNCTION__);
		else
			warning_message_m11("%s(): cannot connect to checkip.dyndns.org\n", __FUNCTION__);
		return(NULL);
	}
	fp = fopen_m11(globals_m11->temp_file, "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
       
	// get file length
	sz = file_length_m11(fp, NULL);

	// read output
	buffer = calloc((size_t) sz, sizeof(si1));
	fread_m11(buffer, sizeof(si1), (size_t) sz, fp, globals_m11->temp_file, __FUNCTION__, __LINE__, EXIT_ON_FAIL_m11);
	fclose(fp);

	// parse output
	pattern = "Current IP Address: ";
	if ((c = STR_match_end_m11(pattern, buffer)) == NULL) {
		if (retry_count) {
			warning_message_m11("%s(): Could not match pattern \"%s\" in output of \"curl -s checkip.dyndns.org\"\n", __FUNCTION__, pattern);
			np->WAN_IPv4_address_string[0] = 0;
			memset(np->WAN_IPv4_address_bytes, 0, IPV4_ADDRESS_BYTES_d11);
		} else {
			retry_count = 1;
			
			nap_d11("1 sec");
			goto GET_WAN_IPV4_ADDR_RETRY_d11;
		}
	} else {
		sscanf(c, "%[^< ]s", np->WAN_IPv4_address_string);
		sscanf(c, "%hhu.%hhu.%hhu.%hhu", np->WAN_IPv4_address_bytes, np->WAN_IPv4_address_bytes + 1, np->WAN_IPv4_address_bytes + 2, np->WAN_IPv4_address_bytes + 3);
	}
	
	free((void *) buffer);

	return(np);
}


TERN_m11	hex_to_int_d11(ui1 *in, ui1 *out, si4 len)
{
	ui1	hi_val, lo_val;
	si4	i;
	
	
	// if "in" is null-terminated string, can pass zero for len
	// can be done in place i.e. "in" can == "out"
	
	if (*in == (ui1) '0' && ((*(in + 1) == (ui1) 'x') || (*(in + 1) == (ui1) 'X')))
		in += 2;
	
	if (len == 0)
		len = strlen((const char *) in) / 2;
	
	for (i = 0; i < len; ++i) {
		if (*in >= (ui1) 'a') {
			if(*in > (ui1) 'f')
				return(FALSE_m11);
			hi_val = (*in - (ui1) 'a') + 10;
		} else if (*in >= (ui1) 'A') {
			if(*in > (ui1) 'F')
				return(FALSE_m11);
			hi_val = (*in - (ui1) 'A') + 10;
		} else if (*in >= (ui1) '0') {
			if(*in > (ui1) '9')
				return(FALSE_m11);
			hi_val = *in - (ui1) '0';
		} else {
			return(FALSE_m11);
		}
		++in;
		if (*in >= (ui1) 'a') {
			if(*in > (ui1) 'f')
				return(FALSE_m11);
			lo_val = (*in - (ui1) 'a') + 10;
		} else if (*in >= (ui1) 'A') {
			if(*in > (ui1) 'F')
				return(FALSE_m11);
			lo_val = (*in - (ui1) 'A') + 10;
		} else if (*in >= (ui1) '0') {
			if(*in > (ui1) '9')
				return(FALSE_m11);
			lo_val = *in - (ui1) '0';
		} else {
			return(FALSE_m11);
		}
		++in;
		*out++ = (hi_val << 4) | lo_val;
	}
	
	return(TRUE_m11);
}


TERN_m11	increase_process_priority_d11(TERN_m11 verbose_flag)
{
	TERN_m11	ret_val = TRUE_m11;
	
	
	// verbose_flag passed because this function is usually called before the MED libraries are initialized
	// no printf_m11() calls for the same reason
	
	#if defined MACOS_m11 || defined LINUX_m11
	system("sudo -l 1> /dev/null");  // update sudo timeout if necessary
	if (setpriority(PRIO_PROCESS, 0, PRIO_MIN))
		ret_val = FALSE_m11;
	#endif
	
	#ifdef WINDOWS_m11
	if ((ret_val = SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)) == 0)  // zero indicates failure in Windows
		ret_val = FALSE_m11;
	#endif
	
	if (ret_val == FALSE_m11) {
		if (verbose_flag == TRUE_m11) {
		#if defined MACOS_m11 || defined LINUX_m11
			#ifdef MATLAB_m11
			mexPrintf("%s(): Could not increase process priority.\nTo run with high priority, in a terminal type: \"sudo chown root <mex file>; sudo chmod ug+s <mex file>\"\n", __FUNCTION__);
			#else
			fprintf(stderr, "%s(): Could not increase process priority.\nTo run with high priority, run with sudo or as root.\nOr: \"sudo chown root <program name>; sudo chmod ug+s <program name>\"\n", __FUNCTION__);
			#endif
		#endif
			
		#ifdef WINDOWS_m11
			#ifdef MATLAB_m11
			mexPrintf("%s(): could not increase process priority\n", __FUNCTION__);
			#else
			fprintf(stderr, "%s(): could not increase process priority\n", __FUNCTION__);
			#endif
		#endif
		}
	}
	
	return(ret_val);
}


void    initialize_alloc_tracking_d11(void)
{
	globals_d11->alloc_tracking = TRUE_m11;
	if (globals_d11->alloc_entities != NULL)
		free(globals_d11->alloc_entities);
	
	globals_d11->ae_array_len = GLOBALS_INIT_ALLOC_TRACKING_ARRAY_LEN_d11;
	globals_d11->alloc_entities = (ALLOC_ENTITY_d11 *) calloc((size_t) globals_d11->ae_array_len, sizeof(ALLOC_ENTITY_d11));
	if (globals_d11->alloc_entities == NULL) {
		warning_message_m11("%s(): calloc error => alloc tracking not initiated\n", __FUNCTION__);
		return;
	}
	globals_d11->ae_curr_allocated_bytes = globals_d11->ae_max_allocated_bytes = 0;
	globals_d11->ae_n_entities = globals_d11->ae_curr_allocated_entities = 0;
	
	return;
}


TERN_m11	initialize_dhnlib_d11(TERN_m11 check_structure_alignments, TERN_m11 initialize_all_tables)
{
#if defined MACOS_m11 || defined LINUX_m11
	static TERN_m11 _Atomic		mutex = FALSE_m11;
#else
	static volatile TERN_m11	mutex = FALSE_m11;
#endif
	TERN_m11	return_value = TRUE_m11;

	while (mutex == TRUE_m11);
	mutex = TRUE_m11;
	
	// set up globals
	if (globals_d11 == NULL)
		initialize_globals_d11();

	// check structure alignments
	if (check_structure_alignments == TRUE_m11)
		if (check_all_alignments_d11(__FUNCTION__, __LINE__) == FALSE_m11)
			return_value = FALSE_m11;
	
	// initialize
	if (globals_d11->alloc_tracking == TRUE_m11)  // if alloc_tracking, must be done first
		initialize_alloc_tracking_d11();

	// umask
#if defined MACOS_m11 || defined LINUX_m11
	umask(globals_d11->file_creation_umask);
#endif
#ifdef WINDOWS_m11
	_umask(globals_d11->file_creation_umask);
#endif
	
	// sk matrix
	if (initialize_all_tables == TRUE_m11) {
		initialize_sk_matrix_d11();
		CMP_initialize_tables_d11();
	}
	
	mutex = FALSE_m11;
	return(return_value);
}


void	initialize_globals_d11(void)
{
#if defined MACOS_m11 || defined LINUX_m11
	static TERN_m11 _Atomic		mutex = FALSE_m11;
#else
	static volatile TERN_m11	mutex = FALSE_m11;
#endif

	
	while (mutex == TRUE_m11);
	mutex = TRUE_m11;
	
	if (globals_d11 == NULL) {
		globals_d11 = (GLOBALS_d11 *) calloc((size_t) 1, sizeof(GLOBALS_d11));
		if (globals_d11 == NULL) {
			error_message_m11("%s(): calloc error", __FUNCTION__);
			return;
		}
	}
	globals_d11->LS_machine_code = GLOBALS_LS_MACHINE_CODE_DEFAULT_d11;
	globals_d11->LS_customer_code = GLOBALS_LS_CUSTOMER_CODE_DEFAULT_d11;
	globals_d11->license_file_entry_aligned = UNKNOWN_m11;
	globals_d11->transmission_header_aligned = UNKNOWN_m11;
	globals_d11->file_creation_umask = GLOBALS_FILE_CREATION_UMASK_DEFAULT_d11;
	globals_d11->alloc_tracking = GLOBALS_ALLOC_TRACKING_DEFAULT_d11;
	globals_d11->verbose = GLOBALS_VERBOSE_DEFAULT_d11;
	globals_d11->sk_matrix = NULL;
	globals_d11->CMP_normal_CDF_table = NULL;
	globals_d11->CMP_VDS_threshold_map = NULL;

	mutex = FALSE_m11;
	return;
}


void	initialize_sk_matrix_d11(void)
{
	if (globals_d11->sk_matrix == NULL) {
		globals_d11->sk_matrix = (ui1 *) calloc_d11((size_t) SK_MATRIX_ENTRIES_d11, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		{
			ui1	temp[SK_MATRIX_ENTRIES_d11] = SK_MATRIX_d11;
			memcpy((void *) globals_d11->sk_matrix, (void *) temp, (size_t) SK_MATRIX_ENTRIES_d11);
		}
	}
	
	return;
}


#if defined MACOS_m11 || defined LINUX_m11
ui4    launch_thread_d11(pthread_t_d11 *thread_id_ptr, pthread_fn_d11 thread_f, void *arg, si4 priority, si1 *affinity_str, cpu_set_t_d11 *cpu_set_p, TERN_m11 detached, si1 *thread_name)
{
	sf8              	f_min_priority, f_max_priority;
	pthread_t		*thread_id_p, local_thread_id_p;
	pthread_attr_t          thread_attributes;
	struct sched_param      scheduling_parameters;
	static si4		min_priority = UNDEFINED_PRIORITY_d11, low_priority, medium_priority, high_priority, max_priority;

	
	if (thread_id_ptr == NULL)  // caller doesn't need thread_id - detached thread
		thread_id_p = &local_thread_id_p;
	else
		thread_id_p = (pthread_t *) thread_id_ptr;
	
	pthread_attr_init(&thread_attributes);
	
	if (priority != DEFAULT_PRIORITY_d11) {
		pthread_attr_getschedparam(&thread_attributes, &scheduling_parameters);
		if (min_priority == UNDEFINED_PRIORITY_d11) {  // only do this once
			f_max_priority = (sf8) sched_get_priority_max(SCHED_OTHER);
			f_min_priority = (sf8) sched_get_priority_min(SCHED_OTHER);
			low_priority = (si4) round((0.75 * f_min_priority) + (0.25 * f_max_priority));
			medium_priority = (si4) round((0.5 * f_min_priority) + (0.5 * f_max_priority));
			high_priority = (si4) round((0.25 * f_min_priority) + (0.75 * f_max_priority));
			max_priority = (si4) f_max_priority;
			min_priority = (si4) f_min_priority;
		}
		switch (priority) {
			case MIN_PRIORITY_d11:
				scheduling_parameters.sched_priority = min_priority;
				break;
			case LOW_PRIORITY_d11:
				scheduling_parameters.sched_priority = low_priority;
				break;
			case MEDIUM_PRIORITY_d11:
				scheduling_parameters.sched_priority = medium_priority;
				break;
			case HIGH_PRIORITY_d11:
				scheduling_parameters.sched_priority = high_priority;
				break;
			case MAX_PRIORITY_d11:
				scheduling_parameters.sched_priority = max_priority;
				break;
			default: // caller passed priority value
				scheduling_parameters.sched_priority = priority;
				break;
		}
		pthread_attr_setschedparam(&thread_attributes, &scheduling_parameters);
	}

	if (detached == TRUE_m11)
		pthread_attr_setdetachstate(&thread_attributes, PTHREAD_CREATE_DETACHED);

# ifdef LINUX_m11
	cpu_set_t_d11 	local_cpu_set;
	
	// generate affinity
	if (affinity_str != NULL) {
		if (*affinity_str) {
			if (cpu_set_p == NULL)  // caller did't supply cpu set
				cpu_set_p = &local_cpu_set;
			generate_cpu_set_d11(affinity_str, cpu_set_p);
		}
	}
	// set affinity
	if (cpu_set_p != NULL)
		set_thread_affinity_d11(NULL, &thread_attributes, cpu_set_p, TRUE_m11);
# endif  // LINUX_m11

	// start thread
	pthread_create(thread_id_p, &thread_attributes, thread_f, arg);

	// finished with attributes (destroy, or get memory leak)
	pthread_attr_destroy(&thread_attributes);

	// set thread name
# ifdef LINUX_m11
	if (thread_name != NULL)
		if (*thread_name)
			pthread_setname_np(*thread_id_p, thread_name);  // _np is for "not portable"
# endif  // LINUX_m11
	
	return(1);  // zero indicates failure (for compatibility with Windows version)
}
#endif  // MACOS_m11 || LINUX_m11


#ifdef WINDOWS_m11
ui4    launch_thread_d11(pthread_t_d11 *thread_handle_p, pthread_fn_d11 thread_f, void *arg, si4 priority, si1 *affinity_str, cpu_set_t_d11 *cpu_set_p, TERN_m11 detached, si1 *thread_name)
{
	HANDLE		*thread_hp, local_thread_h;
	ui4		thread_id;
	wchar_t		w_thread_name[THREAD_NAME_BYTES_d11];
	cpu_set_t_d11 	local_cpu_set_p;

	
	// returns thread id
	if (thread_handle_p == NULL)  // caller doesn't need thread_id - detached thread
		thread_hp = &local_thread_h;
	else
		thread_hp = thread_handle_p;

	// Create thread in suspended state so can set attributes
	thread_id = 0;  // zero indicates failure
	*thread_hp = (HANDLE) _beginthreadex(NULL, 0, (LPTHREAD_START_ROUTINE) thread_f, arg, CREATE_SUSPENDED, &thread_id);
	// _beginthreadex() is supposed to be better than CreateThread() for memory leaks & cleanup - called identically

	// Set Priority
	if (priority != DEFAULT_PRIORITY_d11) {
		switch (priority) {
			case MIN_PRIORITY_d11:
				priority = THREAD_PRIORITY_LOWEST;
				break;
			case LOW_PRIORITY_d11:
				priority = THREAD_PRIORITY_BELOW_NORMAL;
				break;
			case MEDIUM_PRIORITY_d11:
				priority = THREAD_PRIORITY_NORMAL;
				break;
			case HIGH_PRIORITY_d11:
				priority = THREAD_PRIORITY_ABOVE_NORMAL;
				break;
			case MAX_PRIORITY_d11:
				priority = THREAD_PRIORITY_HIGHEST;
				break;
			default: // caller passed priority value
				break;
		}
		SetThreadPriority(*thread_hp, priority);
	}
	
	// Set Affinity
	if (affinity_str != NULL) {
		if (*affinity_str) {
			if (cpu_set_p == NULL)
				cpu_set_p = &local_cpu_set_p;
			generate_cpu_set_d11(affinity_str, cpu_set_p);
		}
	}
	if (cpu_set_p != NULL)
		set_thread_affinity_d11(thread_hp, NULL, cpu_set_p, TRUE_m11);
	
	// set thread name
	if (thread_name != NULL) {
		if (*thread_name) {
			char2wchar_m11(w_thread_name, thread_name);
			SetThreadDescription(*thread_hp, w_thread_name);
		}
	}
	
	// start thread
	ResumeThread(*thread_hp);
	
	// detach thread
	if (detached == TRUE_m11)
		CloseHandle(*thread_hp);

	return(thread_id);  // zero indicates failure
}
#endif  // WINDOWS_m11


TERN_m11	LSc_add_customer_d11(LS_PASSWORD_HASH_d11 *returned_master_pw_hash)
{
	extern GLOBALS_m11		*globals_m11;
	extern GLOBALS_d11		*globals_d11;
	const ui1			EMAIL = 1, TEXT = 2;
	ui1				mode;
	si1				*c, response[128], initial_contact[128], content[256];
	si1				password[MAX_PASSWORD_STRING_BYTES_m11], license_path[FULL_FILE_NAME_BYTES_m11];
	ui4				authentication_code;
	si8				authentication_code_reply, buffer_bytes, bytes_sent, bytes_received;
	TRANSMISSION_INFO_d11		*trans_info;
	TRANSMISSION_HEADER_d11		*header;
	LS_AUXILIARY_LICENSE_DATA_d11	*aux_lic_data;
	LS_MACHINE_INFO_d11		*mach_info;
	LS_CUSTOMER_INFO_d11		*cust_info;
	LS_PASSWORD_HASH_d11		*standard_pw_hash, *master_pw_hash;
	LS_PASSWORD_HINT_d11		*standard_pw_hint, *master_pw_hint;
	LOCATION_INFO_m11		loc_info;
	TIMEZONE_INFO_m11		tz_info = {0};


	if (globals_d11->LS_machine_code == GLOBALS_LS_MACHINE_CODE_DEFAULT_d11)
		get_machine_code_d11();

	// get customer info
	printf_m11("\nPlease provide the following basic information.\n%sRequired responses are marked with an asterisk (*)%s\n\n", TC_RED_m11, TC_RESET_m11);

	// get contact
	if (get_terminal_entry_d11("Enter a valid email address or text message number", RC_STRING_TYPE_d11, (void *) initial_contact, NULL, TRUE_m11, TRUE_m11) == FALSE_m11)
		return(FALSE_m11);
	
	// email or phone number
	c = initial_contact;
	mode = TEXT;
	while (*c) {
		if (*c++ == '@') {
			mode = EMAIL;
			break;
		}
	}
	
	// send code
#if defined MACOS_m11 || defined LINUX_m11
	authentication_code = (ui4) ((random() % (si8) 89999) + 10001);
#endif
#ifdef WINDOWS_m11
	authentication_code = (ui4)(((si8)rand() % (si8) 89999) + 10001);
#endif
	sprintf_m11(content, "DHN Authentication Code: %u", authentication_code);
	if (mode == TEXT) {
		strcpy(response, initial_contact);
		STR_strip_character_m11(response, '-');
		STR_strip_character_m11(response, ' ');
		STR_strip_character_m11(response, '(');
		STR_strip_character_m11(response, ')');
		// check only numerals, "+" or ","
		c = response;
		mode = TEXT;
		while (*c) {
			if (*c < '0' || *c > '9') {
				if (*c != '+' && *c != ',') {
					warning_message_m11("%s(): invalid text number entered\n", __FUNCTION__);
					return(FALSE_m11);
				}
			}
			++c;
		}
		textbelt_text_d11(response, content, TEXTBELT_KEY_d11);
	} else {  // mode = EMAIL
		sendgrid_email_d11(SENDGRID_KEY_d11, initial_contact, "", "New DHN Customer", "DHN Customer Authentication Code", content, "contact@darkhorseneuro.com", "DHN License Manager", "contact@darkhorseneuro.com", "DHN License Manager");
	}
	
	// verify code
	if (mode == TEXT)
		sprintf_m11(content, "Please enter the authentication code sent to %s", initial_contact);
	else  // mode = EMAIL
		sprintf_m11(content, "Please enter the authentication code sent to %s  (if not received, check junk/spam folder)", initial_contact);
	if (get_terminal_entry_d11(content, RC_INTEGER_TYPE_d11, (void *) &authentication_code_reply, NULL, TRUE_m11, TRUE_m11) == FALSE_m11)
		return(FALSE_m11);
	if ((ui4) authentication_code_reply != authentication_code) {
		warning_message_m11("%s(): invalid code\n", __FUNCTION__);
		return(FALSE_m11);
	}
	
	// get defaults from current location
	get_location_info_m11(&loc_info, TRUE_m11, TRUE_m11);
	
	// header => aux_lic_data => customer_info => machine_info => stand_hash => stand_hint => mast_hash => mast_hint (encryp) )
	buffer_bytes = TH_HEADER_BYTES_d11 + LS_AUXILIARY_LICENSE_DATA_BYTES_d11 + LS_CUSTOMER_INFO_BYTES_d11 + LS_MACHINE_INFO_BYTES_d11 + (2 * LS_PASSWORD_HASH_BYTES_d11)+ (2 * LS_PASSWORD_HINT_BYTES_d11);
	trans_info = alloc_trans_info_d11(buffer_bytes, NULL, LS_SERVER_IP_ADDRESS_d11, LS_PORT_d11, LS_TH_ID_CODE_d11, LS_SOCK_TIMEOUT_SECS_d11);
	header = trans_info->header;
	aux_lic_data = (LS_AUXILIARY_LICENSE_DATA_d11 *) (header + 1);
	cust_info = (LS_CUSTOMER_INFO_d11 *) (aux_lic_data + 1);
	mach_info = (LS_MACHINE_INFO_d11 *) (cust_info + 1);
	standard_pw_hash = (LS_PASSWORD_HASH_d11 *) (mach_info + 1);
	standard_pw_hint = (LS_PASSWORD_HINT_d11 *) (standard_pw_hash + 1);
	master_pw_hash = (LS_PASSWORD_HASH_d11 *) (standard_pw_hint + 1);
	master_pw_hint = (LS_PASSWORD_HINT_d11 *) (master_pw_hash + 1);

	// fill in
	header->type = LS_TH_TYPE_ADD_CUSTOMER_d11;
	header->flags |= TH_FLAGS_ENCRYPTED_MASK_d11;  // send password hashes encrypted
	header->transmission_bytes = buffer_bytes;
	aux_lic_data->customer_code = globals_d11->LS_customer_code;
	aux_lic_data->machine_code = globals_d11->LS_machine_code;
#if defined MACOS_m11 || defined LINUX_m11
	strncpy(aux_lic_data->user, getenv("USER"), LS_USER_NAME_BYTES_d11 - 1);
#endif
#ifdef WINDOWS_m11
	strncpy(aux_lic_data->user, getenv("USERNAME"), LS_USER_NAME_BYTES_d11 - 1);
#endif
	if (LSc_get_machine_info_d11(mach_info, loc_info.WAN_IPv4_address) == FALSE_m11)
		warning_message_m11("%s(): could not get machine info => continuing\n", __FUNCTION__);

	// customer type
	(void) get_terminal_entry_d11("Single User or Group Administrator (u/g)", RC_STRING_TYPE_d11, (void *) response, "u", TRUE_m11, TRUE_m11);
	if (*response == 'u' || *response == 'U') {
		strcpy(cust_info->customer_type, "user");
	} else if (*response == 'g' || *response == 'G') {
		strcpy(cust_info->customer_type, "group");
	} else {
		warning_message_m11("%s(): invalid customer type entered\n", __FUNCTION__);
		free_transmission_info_d11(&trans_info, TRUE_m11);
		return(FALSE_m11);
	}

	printf_m11("\nUser or Group Administrator Information:\n");
	printf_m11("----------------------------------------\n\n");
	(void) get_terminal_entry_d11("Salutation (Dr, Mr, Ms, etc)", RC_STRING_TYPE_d11, (void *) cust_info->salutation, NULL, TRUE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Forename (first name)", RC_STRING_TYPE_d11, (void *) cust_info->forename, NULL, TRUE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Surname (last name)", RC_STRING_TYPE_d11, (void *) cust_info->surname, NULL, TRUE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Degrees (BS, MD, PhD, etc)", RC_STRING_TYPE_d11, (void *) cust_info->degrees, NULL, FALSE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Occupation (neuroscientist, neurologist, programmer, etc)", RC_STRING_TYPE_d11, (void *) cust_info->occupation, NULL, FALSE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Job Title (director, student, CTO, etc)", RC_STRING_TYPE_d11, (void *) cust_info->job_title, NULL, FALSE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Gender ((m)ale / (f)emale / (o)ther)", RC_STRING_TYPE_d11, (void *) response, NULL, FALSE_m11, TRUE_m11);
	if (*response == 'm' || *response == 'M')
		strcpy(cust_info->gender, "male");
	else if (*response == 'f' || *response == 'F')
		strcpy(cust_info->gender, "female");
	else if (*response == 'o' || *response == 'O')
		strcpy(cust_info->gender, "other");
	else
		strcpy(cust_info->gender, "unknown");
	printf_m11("\nOrganization Information:\n");
	printf_m11("-------------------------\n\n");
	(void) get_terminal_entry_d11("Organization (institution, company, etc)", RC_STRING_TYPE_d11, (void *) cust_info->organization, NULL, TRUE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Department", RC_STRING_TYPE_d11, (void *) cust_info->department, NULL, FALSE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Division", RC_STRING_TYPE_d11, (void *) cust_info->division, NULL, FALSE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Country", RC_STRING_TYPE_d11, (void *) tz_info.country, loc_info.timezone_info.country, TRUE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Standard Timezone Acronym (GMT, EST, etc; not daylight acronym)", RC_STRING_TYPE_d11, (void *) tz_info.standard_timezone_acronym, loc_info.timezone_info.standard_timezone_acronym, TRUE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Territory (state, province, etc)", RC_STRING_TYPE_d11, (void *) tz_info.territory, loc_info.timezone_info.territory, FALSE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Locality (city, town, hamlet, etc)", RC_STRING_TYPE_d11, (void *) cust_info->locality, loc_info.locality, FALSE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Street Address", RC_STRING_TYPE_d11, (void *) cust_info->street_address, NULL, FALSE_m11, TRUE_m11);
	(void) get_terminal_entry_d11("Postal Code", RC_STRING_TYPE_d11, (void *) cust_info->postal_code, loc_info.postal_code, FALSE_m11, TRUE_m11);
	if (mode == EMAIL)
		get_terminal_entry_d11("Primary Email", RC_STRING_TYPE_d11, (void *) cust_info->email_primary, initial_contact, TRUE_m11, TRUE_m11);
	else
		get_terminal_entry_d11("Primary Email", RC_STRING_TYPE_d11, (void *) cust_info->email_primary, NULL, TRUE_m11, TRUE_m11);
	if (*cust_info->email_primary == 0) {
		warning_message_m11("No entry in required field\n");
		free_transmission_info_d11(&trans_info, TRUE_m11);
		return(FALSE_m11);
	}
	(void) get_terminal_entry_d11("Secondary Email", RC_STRING_TYPE_d11, (void *) cust_info->email_secondary, NULL, FALSE_m11, TRUE_m11);
	if (mode == TEXT)
		get_terminal_entry_d11("Primary Phone", RC_STRING_TYPE_d11, (void *) cust_info->phone_primary, initial_contact, TRUE_m11, TRUE_m11);
	else
		get_terminal_entry_d11("Primary Phone", RC_STRING_TYPE_d11, (void *) cust_info->phone_primary, NULL, TRUE_m11, TRUE_m11);
	if (*cust_info->phone_primary == 0) {
			warning_message_m11("No entry in required field\n");
			free_transmission_info_d11(&trans_info, TRUE_m11);
			return(FALSE_m11);
	}
	(void) get_terminal_entry_d11("Secondary Phone", RC_STRING_TYPE_d11, (void *) cust_info->phone_secondary, NULL, FALSE_m11, TRUE_m11);
	printf_m11("\nUsage Information:\n");
	printf_m11("------------------\n\n");
	(void) get_terminal_entry_d11("Research Use (y/n)", RC_STRING_TYPE_d11, (void *) response, NULL, FALSE_m11, FALSE_m11);
	if (*response == 'y' || *response == 'Y')
		strcpy(cust_info->research, "yes");
	else if (*response == 'n' || *response == 'N')
		strcpy(cust_info->research, "no");
	else
		strcpy(cust_info->research, "unknown");
	(void) get_terminal_entry_d11("Clinical Use (y/n)", RC_STRING_TYPE_d11, (void *) response, NULL, FALSE_m11, FALSE_m11);
	if (*response == 'y' || *response == 'Y')
		strcpy(cust_info->clinical, "yes");
	else if (*response == 'n' || *response == 'N')
		strcpy(cust_info->clinical, "no");
	else
		strcpy(cust_info->clinical, "unknown");
	(void) get_terminal_entry_d11("Industry Use (y/n)", RC_STRING_TYPE_d11, (void *) response, NULL, FALSE_m11, FALSE_m11);
	if (*response == 'y' || *response == 'Y')
		strcpy(cust_info->industry, "yes");
	else if (*response == 'n' || *response == 'N')
		strcpy(cust_info->industry, "no");
	else
		strcpy(cust_info->industry, "unknown");

	// get passwords
	printf_m11("\nPasswords:\n");
	printf_m11("----------\n");
	printf_m11("%s(a master and a standard password is each created here)%s\n\n", TC_YELLOW_m11, TC_RESET_m11);
	
	// master
	if (enter_ascii_password_d11(password, "Master Password", TRUE_m11) == FALSE_m11) {
		free_transmission_info_d11(&trans_info, TRUE_m11);
		warning_message_m11("%s(): error entering password\n", __FUNCTION__);
		return(FALSE_m11);
	}
	if (LSc_process_password_d11(password, master_pw_hash) == FALSE_m11) {
		free_transmission_info_d11(&trans_info, TRUE_m11);
		warning_message_m11("%s(): error processing password\n", __FUNCTION__);
		return(FALSE_m11);
	} else {
		message_m11("\n%sMaster password accepted%s\n\n", TC_GREEN_m11, TC_RESET_m11);
	}
	(void) get_terminal_entry_d11("Master Password Hint", RC_STRING_TYPE_d11, (void *) master_pw_hint->hint, NULL, FALSE_m11, TRUE_m11);
	
	// standard
	if (enter_ascii_password_d11(password, "Standard Password", TRUE_m11) == FALSE_m11) {
		free_transmission_info_d11(&trans_info, TRUE_m11);
		warning_message_m11("%s(): error entering password\n", __FUNCTION__);
		return(FALSE_m11);
	}
	if (LSc_process_password_d11(password, standard_pw_hash) == FALSE_m11) {
		free_transmission_info_d11(&trans_info, TRUE_m11);
		warning_message_m11("%s(): error processing password\n", __FUNCTION__);
		return(FALSE_m11);
	} else {
		message_m11("\n%sStandard password accepted%s\n\n", TC_GREEN_m11, TC_RESET_m11);
	}
	(void) get_terminal_entry_d11("Standard Password Hint", RC_STRING_TYPE_d11, (void *) standard_pw_hint->hint, NULL, FALSE_m11, TRUE_m11);
	
	// return the master password hash
	if (returned_master_pw_hash != NULL)
		*returned_master_pw_hash = *master_pw_hash;  //  "master_pw_hash" is part of the the transmission buffer which will be freed
	
	// check / correct timezone info
	set_global_time_constants_m11(&tz_info, 0, TRUE_m11);
	strcpy(cust_info->country, tz_info.country);
	strcpy(cust_info->territory, tz_info.territory);
	strcpy(cust_info->standard_timezone_acronym, tz_info.standard_timezone_acronym);
	
	// send transmission
	bytes_sent = send_transmission_d11(trans_info);
	if (bytes_sent != header->transmission_bytes) {
		free_transmission_info_d11(&trans_info, TRUE_m11);
		warning_message_m11("%s(): error sending transmission\n", __FUNCTION__);
		return(FALSE_m11);
	}
	
	// receive transmission
	bytes_received = recv_transmission_d11(trans_info);
	header = trans_info->header;  // reset in case reallocated
	if (bytes_received != header->transmission_bytes) {
		free_transmission_info_d11(&trans_info, TRUE_m11);
		warning_message_m11("%s(): error receiving transmission\n", __FUNCTION__);
		return(FALSE_m11);
	}

	// check for old license file
	if (get_DHN_license_path_d11(license_path) != NULL) {
		if (file_exists_m11(license_path) == FILE_EXISTS_m11) {
			extract_path_parts_m11(license_path, license_path, NULL, NULL);
			warning_message_m11("An older DHN license file exists: \"%s/DHN_licenses.txt\"\nIt will be renamed to: \"%s/old_DHN_licenses.txt\"\n", license_path, license_path);
#if defined MACOS_m11 || defined LINUX_m11
			sprintf_m11(content, "mv \"%s/DHN_licenses.txt\" \"%s/old_DHN_licenses.txt\"", license_path, license_path);
#endif
#ifdef WINDOWS_m11
			sprintf_m11(content, "move \"%s/DHN_licenses.txt\" \"%s/old_DHN_licenses.txt\"", license_path, license_path);
#endif

			system_m11(content, FALSE_m11, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		}
	}

	// header => aux_lic_data
	if (header->type == LS_TH_TYPE_CUSTOMER_ADDED_d11) {
		aux_lic_data = (LS_AUXILIARY_LICENSE_DATA_d11 *) (header + 1);
		globals_d11->LS_customer_code = aux_lic_data->customer_code;
		free_transmission_info_d11(&trans_info, TRUE_m11);
		message_m11("%sRegistration complete.%s\n", TC_BLUE_m11, TC_RESET_m11);
		return(TRUE_m11);
	} else {
		if (show_message_d11(header) == FALSE_m11)
			warning_message_m11("%s(): unexpected transmission type\n", __FUNCTION__);
		free_transmission_info_d11(&trans_info, TRUE_m11);
		return(FALSE_m11);
	}
}


TERN_m11	LSc_check_license_d11(ui4 product_code, ui1 version_major, ui1 version_minor)
{
	TERN_m11			license_ok, read_ok, connect_to_server_flag, write_flag, response[16];
	si1				password[MAX_ASCII_PASSWORD_STRING_BYTES_m11] = {0};
	si4				i, j, k, number_of_licenses;
	ui4				curr_time;
	si8				temp_si8;
	LS_LICENSE_FILE_ENTRY_d11	*license_entries;
	LS_PASSWORD_HASH_d11		*pw_hash, local_pw_hash = {0};


	if (globals_d11->LS_machine_code == GLOBALS_LS_MACHINE_CODE_DEFAULT_d11)
		get_machine_code_d11();

	license_ok = TRUE_m11;
	connect_to_server_flag = FALSE_m11;  // TRUE_m11 sets write_flag = TRUE_m11;
	write_flag = FALSE_m11;
	pw_hash = NULL;

	// read license file ("DHN_Licenses.txt")
	read_ok = LSc_read_license_file_d11(&license_entries, &number_of_licenses);
	if (read_ok == FALSE_m11) {
		#ifdef MATLAB_m11
		return(FALSE_m11);
		#else
		connect_to_server_flag = TRUE_m11;
		#endif
	}

	// new customer
	if (globals_d11->LS_customer_code == LS_UNKNOWN_CUSTOMER_CODE_d11) {
		pw_hash = &local_pw_hash;  // need the returned hash
		if (get_terminal_entry_d11("\nAre you an existing DHN customer (y/n)", RC_STRING_TYPE_d11, (void *) response, NULL, FALSE_m11, FALSE_m11) == FALSE_m11)
			return(FALSE_m11);
		if (*response == 'n' || *response == 'N') {
			if (LSc_add_customer_d11(pw_hash) == FALSE_m11)
				return(FALSE_m11);
		} else if (*response == 'y' || *response == 'Y') {
			if (get_terminal_entry_d11("Enter your DHN customer code", RC_INTEGER_TYPE_d11, (void *) &temp_si8, NULL, FALSE_m11, TRUE_m11) == FALSE_m11) {
				message_m11("%s(): If you don't know your customer code, contact DHN at contact@darkhorsenero.com.\n", __FUNCTION__);
				return(FALSE_m11);
			}
			globals_d11->LS_customer_code = (ui4) temp_si8;
			get_terminal_entry_d11("Have you previously entered your DHN customer information (y/n)", RC_STRING_TYPE_d11, (void *) response, NULL, FALSE_m11, FALSE_m11);
			if (*response == 'n' || *response == 'N') {
				if (LSc_add_customer_d11(pw_hash) == FALSE_m11) {
					warning_message_m11("%s(): error adding customer\n", __FUNCTION__);
					return(FALSE_m11);
				}
			} else if (*response == 'y' || *response == 'Y') {
				if (enter_ascii_password_d11(password, "Enter your DHN license password to create or renew this license", FALSE_m11) == FALSE_m11) {
					warning_message_m11("%s(): error entering password\n", __FUNCTION__);
					return(FALSE_m11);
				}
				// make hash
				if (LSc_process_password_d11(password, pw_hash) == FALSE_m11) {
					warning_message_m11("%s(): error processing password\n", __FUNCTION__);
					return(FALSE_m11);
				}
			} else {
				warning_message_m11("%s(): invalid response\n", __FUNCTION__);
				return(FALSE_m11);
			}
		} else {
			warning_message_m11("%s(): invalid response\n", __FUNCTION__);
			return(FALSE_m11);
		}
	}

	// find product license
	for (i = 0; i < number_of_licenses; ++i)
		if (license_entries[i].product_code == product_code)
			break;
	
	// no license for product
	if (i == number_of_licenses) {
		++number_of_licenses;
		license_entries = (LS_LICENSE_FILE_ENTRY_d11 *) realloc((void *) license_entries, (size_t) (number_of_licenses * sizeof(LS_LICENSE_FILE_ENTRY_d11)));
		license_entries[i].product_code = product_code;
		license_entries[i].product_version_major = version_major;
		license_entries[i].product_version_minor = version_minor;
		license_entries[i].license_type = LS_UNKNOWN_LICENSE_TYPE_d11;
		license_entries[i].machine_code = globals_d11->LS_machine_code;
		license_entries[i].timeout = 0;
		connect_to_server_flag = TRUE_m11;
	}
	
	// remove duplicate product licenses
	else {
		for (j = i + 1; j < number_of_licenses; ++j) {
			if (license_entries[j].product_code == product_code) {
				for (k = j + 1; k < number_of_licenses; ++k)
					license_entries[k - 1] = license_entries[k];
				--number_of_licenses;
				write_flag = TRUE_m11;
			}
		}
	}

	// check version
	if (license_entries[i].product_version_major == version_major) {
		if (license_entries[i].product_version_minor < version_minor)
			connect_to_server_flag = TRUE_m11;
	} else if (license_entries[i].product_version_major < version_major) {
		connect_to_server_flag = TRUE_m11;
	}
	
	// check machine
	if (license_entries[i].machine_code != globals_d11->LS_machine_code) {
		license_entries[i].machine_code = globals_d11->LS_machine_code;
		connect_to_server_flag = TRUE_m11;
	}
	
	// check timeout
	curr_time = (ui4) ((si8) time(NULL) - (si8) LS_DHN_INCEPTION_TIME_d11);  // subtracting LS_DHN_INCEPTION_TIME_d11 adds 50 years to range of ui4 in license entry
	if (curr_time > license_entries[i].timeout)
		connect_to_server_flag = TRUE_m11;
	
	// server transaction required
	if (connect_to_server_flag == TRUE_m11) {
		write_flag = TRUE_m11;
		if ((license_ok = LSc_issue_license_d11(NULL, license_entries + i, pw_hash)) == FALSE_m11) {
			warning_message_m11("%s(): error issuing license\n", __FUNCTION__);
			if (license_entries[i].license_type & LS_EMERGENCY_LICENSE_TYPE_d11)  // don't require internet connection
				license_ok = TRUE_m11;
		}
	}
	
	if (write_flag == TRUE_m11 && license_ok == TRUE_m11)
		LSc_write_license_file_d11(license_entries, number_of_licenses);
	
	if (license_ok == TRUE_m11) {
		if (globals_d11->verbose == TRUE_m11)
			message_m11("%s(): Valid DHN License", __FUNCTION__);
	}
		
	free((void *) license_entries);

	return(license_ok);
}


TERN_m11	LSc_check_license_file_entry_alignment_d11(ui1 *bytes)
{
	LS_LICENSE_FILE_ENTRY_d11	*le;
	TERN_m11                	free_flag = FALSE_m11;
	
	
	// see if already checked
	if (globals_d11->license_file_entry_aligned == UNKNOWN_m11)
		globals_d11->license_file_entry_aligned = FALSE_m11;
	else
		return(globals_d11->license_file_entry_aligned);
	
	// check overall size
	if (sizeof(LS_LICENSE_FILE_ENTRY_d11) != LS_LICENSE_FILE_ENTRY_BYTES_d11)
		goto LS_LICENSE_FILE_ENTRY_NOT_ALIGNED_d11;
	
	// check fields
	if (bytes == NULL) {
		bytes = (ui1 *) malloc(LS_LICENSE_FILE_ENTRY_BYTES_d11);
		free_flag = TRUE_m11;
	}
	le = (LS_LICENSE_FILE_ENTRY_d11 *) bytes;
	if (le->product_string != (si1 *) (bytes + LS_LFE_PRODUCT_STRING_OFFSET_d11))
		goto LS_LICENSE_FILE_ENTRY_NOT_ALIGNED_d11;
	if (&le->product_string_terminal_zero != (si1 *) (bytes + LS_LFE_PRODUCT_STRING_TERMINAL_ZERO_OFFSET_d11))
		goto LS_LICENSE_FILE_ENTRY_NOT_ALIGNED_d11;
	if (&le->product_code != (ui4 *) (bytes + LS_LFE_PRODUCT_CODE_OFFSET_d11))
		goto LS_LICENSE_FILE_ENTRY_NOT_ALIGNED_d11;
	if (&le->product_version_major != (ui1 *) (bytes + LS_LFE_PRODUCT_VERSION_MAJOR_OFFSET_d11))
		goto LS_LICENSE_FILE_ENTRY_NOT_ALIGNED_d11;
	if (&le->product_version_minor != (ui1 *) (bytes + LS_LFE_PRODUCT_VERSION_MINOR_OFFSET_d11))
		goto LS_LICENSE_FILE_ENTRY_NOT_ALIGNED_d11;
	if (&le->license_type != (ui1 *) (bytes + LS_LFE_LICENSE_TYPE_OFFSET_d11))
		goto LS_LICENSE_FILE_ENTRY_NOT_ALIGNED_d11;
	if (&le->timeout != (ui4 *) (bytes + LS_LFE_TIMEOUT_OFFSET_d11))
		goto LS_LICENSE_FILE_ENTRY_NOT_ALIGNED_d11;
	if (&le->machine_code != (ui4 *) (bytes + LS_LFE_MACHINE_CODE_OFFSET_d11))
		goto LS_LICENSE_FILE_ENTRY_NOT_ALIGNED_d11;

	// aligned
	globals_d11->license_file_entry_aligned = TRUE_m11;
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_d11->verbose == TRUE_m11)
		message_m11("LS_LICENSE_FILE_ENTRY_d11 structure is aligned\n");
	
	return(TRUE_m11);
	
	// not aligned
LS_LICENSE_FILE_ENTRY_NOT_ALIGNED_d11:
	
	if (free_flag == TRUE_m11)
		free((void *) bytes);
	
	if (globals_d11->verbose == TRUE_m11)
		error_message_m11("%s(): LS_LICENSE_FILE_ENTRY_d11 structure is NOT aligned", __FUNCTION__);
	
	return(FALSE_m11);
}


si1	LSc_check_password_d11(TRANSMISSION_INFO_d11 *trans_info, si1 *password, si1 *prompt, LS_PASSWORD_HASH_d11 *returned_pw_hash)
{
	extern GLOBALS_d11		*globals_d11;
	si1				local_pw[MAX_ASCII_PASSWORD_STRING_BYTES_m11] = {0};
	TERN_m11			free_trans_info = FALSE_m11;
	si8				buffer_bytes, bytes_sent, bytes_received;
	TRANSMISSION_HEADER_d11		*header;
	LS_AUXILIARY_LICENSE_DATA_d11	*aux_lic_data;
	LS_MISC_SERVER_DATA_d11		*misc_serv_data;
	LS_PASSWORD_HASH_d11		*buf_pw_hash, local_pw_hash = {0};
	

	// return of FALSE indicates error, and implies NO_ACCESS

	// header => aux_lic_data => unspecified_hash
	if (returned_pw_hash == NULL)
		returned_pw_hash = &local_pw_hash;
	if (password == NULL)
		password = local_pw;
	buffer_bytes = TH_HEADER_BYTES_d11 + LS_AUXILIARY_LICENSE_DATA_BYTES_d11 + LS_PASSWORD_HASH_BYTES_d11;
	if (trans_info == NULL)
		free_trans_info = TRUE_m11;
	trans_info = alloc_trans_info_d11(buffer_bytes, trans_info, LS_SERVER_IP_ADDRESS_d11, LS_PORT_d11, LS_TH_ID_CODE_d11, LS_SOCK_TIMEOUT_SECS_d11);

	// get password
	if (*password == 0) {  // user entry
		if (enter_ascii_password_d11(password, prompt, FALSE_m11) == FALSE_m11) {
			warning_message_m11("%s(): error entering password\n", __FUNCTION__);
			if (free_trans_info == TRUE_m11) free_transmission_info_d11(&trans_info, TRUE_m11);
			return(FALSE_m11);
		}
	}
	if (*password) {  // password exists
		if (LSc_process_password_d11(password, returned_pw_hash) == FALSE_m11) {
			warning_message_m11("%s(): error processing password\n", __FUNCTION__);
			if (free_trans_info == TRUE_m11) free_transmission_info_d11(&trans_info, TRUE_m11);
			return(FALSE_m11);
		}
	} else {  // no password => not an error
		if (free_trans_info == TRUE_m11) free_transmission_info_d11(&trans_info, TRUE_m11);
		return(LS_NO_ACCESS_d11);
	}

	// check password (header => aux_lic_data => unspecified_hash (encryp))
	header = trans_info->header;
	header->type = LS_TH_TYPE_CHECK_PASSWORD_d11;
	header->transmission_bytes = buffer_bytes;
	header->flags |= TH_FLAGS_ENCRYPTED_MASK_d11;
	aux_lic_data = (LS_AUXILIARY_LICENSE_DATA_d11 *) (header + 1);
	aux_lic_data->customer_code = globals_d11->LS_customer_code;
#if defined MACOS_m11 || defined LINUX_m11
	strncpy(aux_lic_data->user, getenv("USER"), LS_USER_NAME_BYTES_d11 - 1);
#endif
#ifdef WINDOWS_m11
	strncpy(aux_lic_data->user, getenv("USERNAME"), LS_USER_NAME_BYTES_d11 - 1);
#endif
	buf_pw_hash = (LS_PASSWORD_HASH_d11 *) (aux_lic_data + 1);
	*buf_pw_hash = *returned_pw_hash;

	// transmit
	if ((bytes_sent = send_transmission_d11(trans_info)) != header->transmission_bytes) {
		warning_message_m11("%s(): error sending\n", __FUNCTION__);
		if (free_trans_info == TRUE_m11) free_transmission_info_d11(&trans_info, TRUE_m11);
		return(FALSE_m11);
	}
	
	// receive (header => misc_serv_data)
	bytes_received = recv_transmission_d11(trans_info);
	header = trans_info->header;  // reset in case reallocated
	if (bytes_received != header->transmission_bytes) {
		warning_message_m11("%s(): error receiving\n", __FUNCTION__);
		if (free_trans_info == TRUE_m11) free_transmission_info_d11(&trans_info, TRUE_m11);
		return(FALSE_m11);

	}
	if (header->type != LS_TH_TYPE_PASSWORD_CHECKED_d11) {
		show_message_d11(header);
		if (free_trans_info == TRUE_m11) free_transmission_info_d11(&trans_info, TRUE_m11);
		return(FALSE_m11);
	}
	
	// clean up
	if (free_trans_info == TRUE_m11)
		free_transmission_info_d11(&trans_info, TRUE_m11);

	// return access level
	misc_serv_data = (LS_MISC_SERVER_DATA_d11 *) (header + 1);

	return(misc_serv_data->access_level);
}


TERN_m11	LSc_get_machine_info_d11(LS_MACHINE_INFO_d11 *machine_info, si1 *passed_WAN_IPv4_addr_str)
{
	NETWORK_PARAMETERS_d11		np;
	
	
	if (machine_info == NULL) {
		warning_message_m11("%s(): machine_info is NULL without assignment\n", __FUNCTION__);
		return(FALSE_m11);
	}
	
	// host name
	if (gethostname(machine_info->host_name, sizeof(machine_info->host_name)) == -1) {
		warning_message_m11("%s(): cannot get host_name\n", __FUNCTION__);
		return(FALSE_m11);
	}

	// serial_number
	strcpy(machine_info->serial_number, globals_d11->machine_serial);
	
	// LAN IPv4 address
	if (get_lan_ipv4_address_d11(&np) == NULL) {
		warning_message_m11("%s(): cannot get LAN IPv4 address\n", __FUNCTION__);
		return(FALSE_m11);
	}
	strcpy(machine_info->LAN_IPv4_address, np.LAN_IPv4_address_string);
	
	// WAN IPv4 address
	if (passed_WAN_IPv4_addr_str != NULL) {
		strcpy(machine_info->WAN_IPv4_address, passed_WAN_IPv4_addr_str);
	} else {
		if (get_wan_ipv4_address_d11(&np) == NULL) {
			warning_message_m11("%s(): cannot get WAN IPv4 address\n", __FUNCTION__);
			return(FALSE_m11);
		}
		strcpy(machine_info->WAN_IPv4_address, np.WAN_IPv4_address_string);
	}
		
	return(TRUE_m11);
}


TERN_m11	LSc_issue_license_d11(TRANSMISSION_INFO_d11 *trans_info, LS_LICENSE_FILE_ENTRY_d11 *lfe, LS_PASSWORD_HASH_d11 *pw_hash)
{
	TERN_m11			ok, free_trans_info = FALSE_m11;
	si1				*msg, prompt[256];
	si8				buffer_bytes, bytes_sent, bytes_received;
	TRANSMISSION_HEADER_d11		*header;
	LS_AUXILIARY_LICENSE_DATA_d11	*aux_lic_data;
	LS_LICENSE_FILE_ENTRY_d11	*buffer_lfe;
	LS_PASSWORD_HASH_d11 		*buf_pw_hash;
	MESSAGE_HEADER_d11		*msg_header;
	
	
	if (globals_d11->LS_machine_code == GLOBALS_LS_MACHINE_CODE_DEFAULT_d11)
		get_machine_code_d11();

	buffer_bytes = TH_HEADER_BYTES_d11 + LS_AUXILIARY_LICENSE_DATA_BYTES_d11 + LS_LICENSE_FILE_ENTRY_BYTES_d11;
	if (pw_hash != NULL) {
		if (all_zeros_m11((ui1 *) pw_hash, sizeof(LS_PASSWORD_HASH_d11)) == TRUE_m11)
		    	pw_hash = NULL;
		else
			buffer_bytes += LS_PASSWORD_HASH_BYTES_d11;
	}
	if (trans_info == NULL)
		free_trans_info = TRUE_m11;
	trans_info = alloc_trans_info_d11(buffer_bytes + 500, trans_info, LS_SERVER_IP_ADDRESS_d11, LS_PORT_d11, LS_TH_ID_CODE_d11, LS_SOCK_TIMEOUT_SECS_d11);

	// header => auxilary_db_data => license_file_entry [+/- unspecified_hash (encrypted if present) ]
	header = trans_info->header;
	header->type = LS_TH_TYPE_ISSUE_LICENSE_d11;
	if (pw_hash != NULL)
		header->flags |= TH_FLAGS_ENCRYPTED_MASK_d11;
	header->transmission_bytes = buffer_bytes;
	aux_lic_data = (LS_AUXILIARY_LICENSE_DATA_d11 *) (header + 1);
	aux_lic_data->customer_code = globals_d11->LS_customer_code;
	aux_lic_data->machine_code = globals_d11->LS_machine_code;
	aux_lic_data->product_code = lfe->product_code;  // duplicate field
#if defined MACOS_m11 || defined LINUX_m11
	strncpy(aux_lic_data->user, getenv("USER"), LS_USER_NAME_BYTES_d11 - 1);
#endif
#ifdef WINDOWS_m11
	strncpy(aux_lic_data->user, getenv("USERNAME"), LS_USER_NAME_BYTES_d11 - 1);
#endif
	buffer_lfe = (LS_LICENSE_FILE_ENTRY_d11 *) (aux_lic_data + 1);
	*buffer_lfe = *lfe;
	if (pw_hash != NULL) {
		buf_pw_hash = (LS_PASSWORD_HASH_d11 *) (buffer_lfe + 1);
		*buf_pw_hash = *pw_hash;
		header->flags |= TH_FLAGS_ENCRYPTED_MASK_d11;
	}

	bytes_sent = send_transmission_d11(trans_info);
	if (bytes_sent != header->transmission_bytes) {
		if (free_trans_info == TRUE_m11)
			free_transmission_info_d11(&trans_info, TRUE_m11);
		else
			close_transmission_d11(trans_info);

		// if send transmission failed, DHN License server may be down (maintenance, failure, etc.)
		// so check internet - if that's working, extend license by an hour without contacting server
		if (check_internet_connection_d11() == TRUE_m11) {
			lfe->timeout = (time(NULL) - LS_DHN_INCEPTION_TIME_d11) + 3600;
			return(TRUE_m11);
		} else {
			warning_message_m11("%s(): transmission error. Check internet connection.\n", __FUNCTION__);
			return(FALSE_m11);
		}
	}

LS_RENEW_LIC_RECV_d11:

	// header => license_file_entry
	bytes_received = recv_transmission_d11(trans_info);
	header = trans_info->header;  // reset in case reallocated
	if (bytes_received != header->transmission_bytes) {
		if (free_trans_info == TRUE_m11)
			free_transmission_info_d11(&trans_info, TRUE_m11);
		else
			close_transmission_d11(trans_info);
		warning_message_m11("%s(): error receiving\n", __FUNCTION__);
		return(FALSE_m11);
	}

	switch (header->type) {
		case LS_TH_TYPE_LICENSE_ISSUED_d11:  // header => license_file_entry
			buffer_lfe = (LS_LICENSE_FILE_ENTRY_d11 *) (header + 1);
			*lfe = *buffer_lfe;
			ok = TRUE_m11;
			break;
		case LS_TH_TYPE_SEND_MACHINE_INFO_d11:  // header
			if (LSc_send_machine_info_d11(trans_info, pw_hash) == FALSE_m11) {
				warning_message_m11("%s(): error sending machine info\n", __FUNCTION__);
				ok = FALSE_m11;
				break;
			}
			goto LS_RENEW_LIC_RECV_d11;
		case LS_TH_TYPE_SEND_PASSWORD_d11:  // header => message (password prompt)
			msg_header = (MESSAGE_HEADER_d11 *) (header + 1);
			msg = (si1 *) (msg_header + 1);
			strncpy(prompt, msg, 256);
			if (LSc_send_password_d11(trans_info, prompt, pw_hash) == FALSE_m11) {
				warning_message_m11("%s(): error sending password\n", __FUNCTION__);
				ok = FALSE_m11;
				break;
			}
			putchar_m11('\n');
			goto LS_RENEW_LIC_RECV_d11;
		case TH_MESSAGE_TYPE_d11:  // new product message
			show_message_d11(header);
			goto LS_RENEW_LIC_RECV_d11;
		default:
			if (show_message_d11(header) == FALSE_m11)
				warning_message_m11("\n%s(): unexpected transmission type\n", __FUNCTION__);
			ok = FALSE_m11;
			break;
	}
	
	if (free_trans_info == TRUE_m11)
		free_transmission_info_d11(&trans_info, TRUE_m11);

	return(ok);
}


TERN_m11	LSc_process_password_d11(si1 *pw, LS_PASSWORD_HASH_d11 *pw_hash)
{
	ui1	*c, hash[SHA_HASH_BYTES_m11];
	si1	*hex;
	si1	i;


	// check pw
	if (check_password_m11(pw) == FALSE_m11)
		return(FALSE_m11);

	// check pw hash
	if (pw_hash == NULL)
		return(FALSE_m11);

	// extract terminal bytes
	extract_terminal_password_bytes_m11(pw, pw);
	
	// hash pw
	SHA_hash_m11((ui1 *) pw, PASSWORD_BYTES_m11, hash);

	// hex hash
	c = hash;
	hex = pw_hash->hash;
	for (i = 0; i < PASSWORD_BYTES_m11; ++i) {
		byte_to_hex_d11(*c++, hex);
		hex += 2;
	}
	*(--hex) = 0;

	return(TRUE_m11);
}


TERN_m11	LSc_read_license_file_d11(LS_LICENSE_FILE_ENTRY_d11 **license_entries, si4 *number_of_license_entries)
{
	si1				license_path[FULL_FILE_NAME_BYTES_m11], *buf, *c;
	si4				i, file_characters, n_licenses;
	si8				file_length;
	FILE				*fp;
	LS_LICENSE_FILE_ENTRY_d11	*lic;
	
	
	if (globals_d11->sk_matrix == NULL)
		initialize_sk_matrix_d11();

	*number_of_license_entries = 0;
	*license_entries = NULL;

	// read license file
	if (get_DHN_license_path_d11(license_path) == NULL) {
		warning_message_m11("%s(): getting path to license file\n", __FUNCTION__);
		return(FALSE_m11);
	}
	fp = fopen_m11(license_path, "r", __FUNCTION__, __LINE__, SUPPRESS_ALL_OUTPUT_m11);
	if (fp == NULL) {
		if (file_exists_m11(license_path) == FILE_EXISTS_m11)
			warning_message_m11("%s(): error reading license file\n", __FUNCTION__);
		else
			warning_message_m11("%s(): license file does not exist\n", __FUNCTION__);
		return(FALSE_m11);
	}
	file_length = file_length_m11(fp, NULL);
	buf = calloc((size_t) (file_length + 1), sizeof(si1));
	fread_m11((void *) buf, sizeof(si1), (size_t) file_length, fp, license_path, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	fclose(fp);
	
	c = STR_match_end_m11("Customer Code: ", buf);
	sscanf(c, "%u", &globals_d11->LS_customer_code);
	while (*c++ != '\n');
	
	// condition text
	STR_strip_character_m11(c, '\r');	// carriage return (PCs)
	STR_strip_character_m11(c, '\n');	// newline (line feed)
	STR_strip_character_m11(c, '-');	// hyphen
	STR_strip_character_m11(c, ' ');	// space

	// allocate output
	file_characters = (si4) strlen(c);
	if (file_characters % (si4) LS_LICENSE_FILE_ENTRY_BASE_CHARACTERS_d11)
		warning_message_m11("%s(): Stray characters in license file => continuing\n", __FUNCTION__);
	n_licenses = file_characters / (si4) LS_LICENSE_FILE_ENTRY_BASE_CHARACTERS_d11;
	*license_entries = (LS_LICENSE_FILE_ENTRY_d11 *) calloc(n_licenses, sizeof(LS_LICENSE_FILE_ENTRY_d11));

	// read entries
	lic = *license_entries;
	for (i = 0; i < n_licenses; ++i, ++lic) {
		if ((hex_to_int_d11((ui1 *) c, (ui1 *) lic, (si4) LS_LICENSE_FILE_ENTRY_BYTES_d11)) == FALSE_m11) {
			warning_message_m11("%s(): improper characters in license file\n", __FUNCTION__);
			*number_of_license_entries = 0;
			return(FALSE_m11);
		}
		AES_decrypt_m11((ui1 *) lic, (ui1 *) lic, NULL, globals_d11->sk_matrix);
		c += LS_LICENSE_FILE_ENTRY_BASE_CHARACTERS_d11;
	}
	free((void *) buf);
	
	// verbose
	if (globals_d11->verbose == TRUE_m11)
		LSc_show_license_entries_d11(*license_entries, n_licenses);

	*number_of_license_entries = n_licenses;
	
	return(TRUE_m11);
}


TERN_m11	LSc_send_machine_info_d11(TRANSMISSION_INFO_d11 *trans_info, LS_PASSWORD_HASH_d11 *pw_hash)
{
	TERN_m11			free_trans_info = FALSE_m11;
	si1				password[MAX_ASCII_PASSWORD_STRING_BYTES_m11] = {0};
	si8				buffer_bytes, bytes_sent;
	LS_MACHINE_INFO_d11		*machine_info;
	LS_PASSWORD_HASH_d11		*buf_pw_hash, local_pw_hash = {0};
	TRANSMISSION_HEADER_d11		*header;
	
	
	// setup [ header => machine_info => unspec_hash (encrypted) ]
	buffer_bytes = TH_HEADER_BYTES_d11 + LS_MACHINE_INFO_BYTES_d11 + LS_PASSWORD_HASH_BYTES_d11;
	if (trans_info == NULL)
		free_trans_info = TRUE_m11;
	trans_info = alloc_trans_info_d11(buffer_bytes, trans_info, LS_SERVER_IP_ADDRESS_d11, LS_PORT_d11, LS_TH_ID_CODE_d11, LS_SOCK_TIMEOUT_SECS_d11);
	header = trans_info->header;
	machine_info = (LS_MACHINE_INFO_d11 *) (header + 1);
	buf_pw_hash = (LS_PASSWORD_HASH_d11 *) (machine_info + 1);
	
	// get machine info
	if (LSc_get_machine_info_d11(machine_info, NULL) == FALSE_m11) {
		warning_message_m11("%s(): error machine info\n", __FUNCTION__);
		return(FALSE_m11);
	}
	
	// get password
	if (pw_hash == NULL) {
		pw_hash = &local_pw_hash;
		if (enter_ascii_password_d11(password, "Enter your DHN license password to authorize this machine", FALSE_m11) == FALSE_m11) {
			warning_message_m11("%s(): error entering password\n", __FUNCTION__);
			return(FALSE_m11);
		}
		// make hash
		if (LSc_process_password_d11(password, pw_hash) == FALSE_m11) {
			warning_message_m11("%s(): error processing password\n", __FUNCTION__);
			return(FALSE_m11);
		}
	}
	*buf_pw_hash = *pw_hash;
	
	// send transmission
	header->type = LS_TH_TYPE_MACHINE_INFO_SENT_d11;
	header->flags |= TH_FLAGS_ENCRYPTED_MASK_d11;  // don't close, not done (server only asks for machine info as part of other transactions)
	header->transmission_bytes = buffer_bytes;
	bytes_sent = send_transmission_d11(trans_info);
	if (bytes_sent != header->transmission_bytes) {
		warning_message_m11("%s(): error sending machine info\n", __FUNCTION__);
		if (free_trans_info == TRUE_m11)
			free_transmission_info_d11(&trans_info, TRUE_m11);
		else
			close_transmission_d11(trans_info);
		return(FALSE_m11);
	}
	
	return(TRUE_m11);
}


TERN_m11	LSc_send_password_d11(TRANSMISSION_INFO_d11 *trans_info, si1 *prompt, LS_PASSWORD_HASH_d11 *pw_hash)
{
	TERN_m11			free_trans_info = FALSE_m11;
	si1				password[MAX_ASCII_PASSWORD_STRING_BYTES_m11] = {0};
	si8				buffer_bytes, bytes_sent;
	TRANSMISSION_HEADER_d11		*header;
	LS_PASSWORD_HASH_d11		*buf_pw_hash, local_pw_hash = {0};
	
	
	// setup [ header => unspecified_hash (encrypted) ]
	buffer_bytes = TH_HEADER_BYTES_d11 + LS_PASSWORD_HASH_BYTES_d11;
	if (trans_info == NULL)
		free_trans_info = TRUE_m11;
	trans_info = alloc_trans_info_d11(buffer_bytes, trans_info, LS_SERVER_IP_ADDRESS_d11, LS_PORT_d11, LS_TH_ID_CODE_d11, LS_SOCK_TIMEOUT_SECS_d11);
	header = trans_info->header;
	buf_pw_hash = (LS_PASSWORD_HASH_d11 *) (header + 1);

	// get password
	if (pw_hash == NULL) {
		pw_hash = &local_pw_hash;
		if (enter_ascii_password_d11(password, prompt, FALSE_m11) == FALSE_m11) {
			warning_message_m11("%s(): error entering password\n", __FUNCTION__);
			return(FALSE_m11);
		}
		// make hash
		if (LSc_process_password_d11(password, pw_hash) == FALSE_m11) {
			warning_message_m11("%s(): error processing password\n", __FUNCTION__);
			return(FALSE_m11);
		}
	}
	*buf_pw_hash = *pw_hash;

	// send password hash
	header->type = LS_TH_TYPE_PASSWORD_SENT_d11;
	header->flags |= TH_FLAGS_ENCRYPTED_MASK_d11;  // transaction not finished - leave socket open
	header->transmission_bytes = buffer_bytes;
	bytes_sent = send_transmission_d11(trans_info);
	if (bytes_sent != header->transmission_bytes) {
		warning_message_m11("%s(): error sending password\n", __FUNCTION__);
		if (free_trans_info == TRUE_m11)
			free_transmission_info_d11(&trans_info, TRUE_m11);
		else
			close_transmission_d11(trans_info);
		return(FALSE_m11);
	}

	return(TRUE_m11);
}


void    LSc_show_license_entries_d11(LS_LICENSE_FILE_ENTRY_d11 *license_entries, si4 number_of_licenses)
{
	si1	time_str[TIME_STRING_BYTES_m11];
	si4	i;
	si8	timeout_utc;
	
	
	printf_m11("Customer Code: %u\n\n", globals_d11->LS_customer_code);
	for (i = 0; i < number_of_licenses; ++i) {
		printf_m11("License #%d:\n", i + 1);
		printf_m11("Product String: %s\n", license_entries[i].product_string);
		printf_m11("Product Code: %04x\n", license_entries[i].product_code);
		printf_m11("Product Version Major: %hhu\n", license_entries[i].product_version_major);
		printf_m11("Product Version Minor: %hhu\n", license_entries[i].product_version_minor);
		printf_m11("License Type(s): ");
		if (license_entries[i].license_type == 0)
			printf_m11("None\n");
		else {
			if (license_entries[i].license_type & LS_USER_LICENSE_TYPE_d11)
				printf_m11("User ");
			if (license_entries[i].license_type & LS_GROUP_LICENSE_TYPE_d11)
				printf_m11("Group ");
			if (license_entries[i].license_type & LS_PERPETUAL_LICENSE_TYPE_d11)
				printf_m11("Perpetual ");
			if (license_entries[i].license_type & LS_SUBSCRIPTION_LICENSE_TYPE_d11)
				printf_m11("Subcription ");
			if (license_entries[i].license_type & LS_TRIAL_LICENSE_TYPE_d11)
				printf_m11("Trial ");
			if (license_entries[i].license_type & LS_EMERGENCY_LICENSE_TYPE_d11)
				printf_m11("Emergency ");
//			if (license_entries[i].license_type & LS_MASTER_LICENSE_TYPE_d11)
//				printf_m11("Master ");
//			if (license_entries[i].license_type & LS_UNREGISTERED_LICENSE_TYPE_d11)
//				printf_m11("Unregistered ");
			putchar_m11('\n');
		}

		if (license_entries[i].machine_code == LS_LFE_MACHINE_CODE_NO_ENTRY_d11)
			printf_m11("Machine Code: no entry\n");
		else
			printf_m11("Machine Code: %04x\n", license_entries[i].machine_code);
		timeout_utc = (si8) license_entries[i].timeout * (si8) 1000000;
		time_string_m11(timeout_utc, time_str, FALSE_m11, FALSE_m11, FALSE_m11);
		printf_m11("Usage Timeout: %s\n", time_str);
	}

	return;
}


TERN_m11	LSc_write_license_file_d11(LS_LICENSE_FILE_ENTRY_d11 *licence_entries, si4 number_of_licenses)
{
	ui1				*c, encryption_buffer[LS_LICENSE_FILE_ENTRY_BYTES_d11];
	si1				*buf, *hex, license_path[FULL_FILE_NAME_BYTES_m11];
	si4				i, j, out_bytes;
	FILE				*fp;
	LS_LICENSE_FILE_ENTRY_d11	*lic;
	
	
	if (globals_d11->sk_matrix == NULL)
		initialize_sk_matrix_d11();

	out_bytes = number_of_licenses * (LS_LICENSE_FILE_ENTRY_STRLEN_d11 + 1);  // account for terminal zero)
	buf = calloc((size_t) out_bytes, sizeof(si1));
	
	hex = buf;
	lic = licence_entries;
	for (i = 0; i < number_of_licenses; ++i, ++lic) {
		AES_encrypt_m11((ui1 *) lic, encryption_buffer, NULL, globals_d11->sk_matrix);
		c = encryption_buffer;
		for (j = 0; j < (LS_LICENSE_FILE_ENTRY_BYTES_d11 / 2); ++j) {
			byte_to_hex_d11(*c++, hex);
			hex += 2;
			byte_to_hex_d11(*c++, hex);
			hex += 2;
			*hex++ = '-';
		}
		--hex; *hex++ = '\n';
	}
	
	// write license file
	if (get_DHN_license_path_d11(license_path) == NULL) {
		warning_message_m11("%s(): error getting path to license file\n", __FUNCTION__);
		return(FALSE_m11);
	}
	if (file_exists_m11(license_path) == DOES_NOT_EXIST_m11) {
		si1	DHN_dir[FULL_FILE_NAME_BYTES_m11], command[FULL_FILE_NAME_BYTES_m11 + 16];

		extract_path_parts_m11(license_path, DHN_dir, NULL, NULL);
		if (file_exists_m11(DHN_dir) == DOES_NOT_EXIST_m11) {
#if defined MACOS_m11 || defined LINUX_m11
			sprintf_m11(command, "mkdir -p %s", DHN_dir);
#endif
#ifdef WINDOWS_m11
			sprintf_m11(command, "mkdir %s", DHN_dir);
#endif
			system_m11(command, TRUE_m11, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
		}
	}
	fp = fopen_m11(license_path, "w", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	fprintf_m11(fp, "Customer Code: %u\n", globals_d11->LS_customer_code);
	fwrite_m11((void *) buf, sizeof(si1), (size_t) out_bytes, fp, license_path, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m11);
	fclose(fp);
		
	// NOTE (MacOS/Linux): If program is running as root, the license file owner will be root.
	// Both user & group have write privileges, so this hasn't been a problem so far.
	
	free((void *) buf);
	
	return(TRUE_m11);
}


void	MSc_initialize_data_request_d11(MS_DATA_REQUEST_d11 *data_request)
{
	strcpy(data_request->server_IPv4_address, MS_SERVER_LOCAL_IP_ADDRESS_d11);
	data_request->sess_uid = UID_NO_ENTRY_m11;
	*data_request->sess_name = 0;
	*data_request->sess_path = 0;
	*data_request->user_name = 0;
	*data_request->password = 0;
	data_request->start_time = MS_START_TIME_DEFAULT_d11;
	data_request->end_time = MS_END_TIME_DEFAULT_d11;
	data_request->start_index = MS_START_INDEX_DEFAULT_d11;
	data_request->end_index = MS_END_INDEX_DEFAULT_d11;
	data_request->idx_ref_chan_uid = UID_NO_ENTRY_m11;
	*data_request->idx_ref_chan_name = 0;
	data_request->pass_filt_low_fc = FREQUENCY_NO_ENTRY_m11;
	data_request->pass_filt_high_fc = FREQUENCY_NO_ENTRY_m11;
	data_request->stop_filt_low_fc = FREQUENCY_NO_ENTRY_m11;
	data_request->stop_filt_high_fc = FREQUENCY_NO_ENTRY_m11;
	data_request->output_block_samples = MS_OUTPUT_BLOCK_SAMPLES_DEFAULT_d11;
	data_request->timeout_seconds = MS_SOCK_DEFAULT_TIMEOUT_SECS_d11;
	data_request->filter_order = MS_DEFAULT_FILTER_ORDER_d11;
	data_request->CMP_directives_attached = MS_CMP_DIRECTIVES_ATTACHED_DEFAULT_d11;
	data_request->CMP_parameters_attached = MS_CMP_PARAMETERS_ATTACHED_DEFAULT_d11;
	data_request->interpolation_type = MS_DEFAULT_INTERPOLATION_d11;
	data_request->data_format = MS_DATA_FORMAT_DEFAULT_d11;
	data_request->output_target = MS_OUTPUT_TARGET_DEFAULT_d11;
	data_request->bypass_database = MS_BYPASS_DATABASE_DEFAULT_d11;
	data_request->keep_connection = MS_KEEP_CONNECTION_DEFAULT_d11;
	
	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
void     nap_d11(si1 *nap_str)
{
	si1             *c;
	struct timespec nap;
	si8             num;
	
	// string format: <number>[<space>]<unit letter>
	// e.g. to sleep for 1 millisecond:
	// "1 millisecond" == "1millisecond" == "1 ms" == "1ms" == "1 m" == "1m"

	if (nap_str == NULL) {
		error_message_m11("%s(): NULL input string => not napping", __FUNCTION__, nap_str);
		return;
	}
	
	if (*nap_str == 0) {
		error_message_m11("%s(): no input string => not napping", __FUNCTION__, nap_str);
		return;
	}

	c = nap_str;
	num = *c++ - '0';
	while (*c >= '0' && *c <= '9' && *c) {
		num *= 10;
		num += *c++ - '0';
	}
	
	// optional space
	if (*c == 32)
		++c;

	// units: ns, us, ms, sec
	switch(*c) {
		case 'n':  // nanoseconds
			nap.tv_sec = 0;
			nap.tv_nsec = num;
			break;
		case 'u':  // microseconds
			nap.tv_sec = 0;
			nap.tv_nsec = num * (ui8) 1e3;
			break;
		case 'm':  // milliseconds
			nap.tv_sec = 0;
			nap.tv_nsec = num * (ui8) 1e6;
			break;
		case 's':  // seconds
			nap.tv_sec = num;
			nap.tv_nsec = 0;
			break;
		default:
			error_message_m11("%s(): \"%s\" is not a valid input string => not napping", __FUNCTION__, nap_str);
			return;
	}
	
	// overflow
	if (nap.tv_nsec >= (ui8) 1e9) {
		nap.tv_sec = nap.tv_nsec / (ui8) 1e9;
		nap.tv_nsec -= (nap.tv_sec * (ui8) 1e9);
	}
	
	// sleep
#if defined MACOS_m11 || defined LINUX_m11
	nanosleep(&nap, NULL);
#endif
#ifdef WINDOWS_m11  // limited to millisecond resolution (can do better, but requires much more code)
	ui8	ms;

	ms = (ui8) nap.tv_sec * (ui8) 1000;
	ms += (ui8) round((sf8)nap.tv_nsec / (sf8) 1e6);
	if (ms > 0x7FFFFFFF) {
		warning_message_m11("%s(): millisecond overflow\n", __FUNCTION__);
		ms = 0x7FFFFFFF;
	}
	Sleep((si4) ms);
#endif

	return;
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	pthread_join_d11(pthread_t_d11 thread_id, void **value_ptr)
{
	si4	ret_val;
	
	
#if defined MACOS_m11 || defined LINUX_m11
	ret_val = pthread_join(thread_id, value_ptr);
#endif
#ifdef WINDOWS_m11
	if (WaitForSingleObject(thread_id, INFINITE) == WAIT_OBJECT_0)
		ret_val = 0;
	else
		ret_val = -1;
#endif

	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	pthread_mutex_destroy_d11(pthread_mutex_t_d11 *mutex)
{
	si4	ret_val;
	
#if defined MACOS_m11 || defined LINUX_m11
	ret_val = pthread_mutex_destroy(mutex);
#endif
#ifdef WINDOWS_m11
	ret_val = 1 - (si4) CloseHandle(*mutex);  // CloseHandle returns zero on fail
#endif
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	pthread_mutex_init_d11(pthread_mutex_t_d11 *mutex, pthread_mutexattr_t_d11 *attr)
{
	si4	ret_val;
	
	
#if defined MACOS_m11 || defined LINUX_m11
	ret_val = pthread_mutex_init(mutex, attr);
#endif
#ifdef WINDOWS_m11
	if ((*mutex = CreateMutex(attr, 0, NULL)) == NULL)
		ret_val = -1;
	else
		ret_val = 0;
#endif
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	pthread_mutex_lock_d11(pthread_mutex_t_d11 *mutex)
{
	si4	ret_val;
	
#if defined MACOS_m11 || defined LINUX_m11
	ret_val = pthread_mutex_lock(mutex);
#endif
#ifdef WINDOWS_m11
	if (WaitForSingleObject(*mutex, INFINITE) == WAIT_OBJECT_0)
		ret_val = 0;
	else
		ret_val = -1;
#endif
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
si4	pthread_mutex_unlock_d11(pthread_mutex_t_d11 *mutex)
{
	si4	ret_val;
	
	
#if defined MACOS_m11 || defined LINUX_m11
	ret_val = pthread_mutex_unlock(mutex);
#endif
#ifdef WINDOWS_m11
	if (ReleaseMutex(*mutex) == 0)
		ret_val = -1;
	else
		ret_val = 0;
#endif
	
	return(ret_val);
}


#ifndef WINDOWS_m11  // inline causes linking problem in Windows
inline
#endif
ui1	random_byte_d11(ui4 *m_w, ui4 *m_z)
{
	ui1	rb;
	
	// see fill_empty_password_bytes_d11() for initialization & usage
	
	*m_z = 0x00009069 * (*m_z & 0x0000FFFF) + (*m_z >> 0x10);
	*m_w = 0x00004650 * (*m_w & 0x0000FFFF) + (*m_w >> 0x10);
	rb = (ui1) (((*m_z << 0x10) + *m_w) % 0x00000100);
	
	return(rb);
}


si1     *re_escape_d11(si1 *str, si1 *esc_str)
{
	si8     len;
	si1     *c1, *c2;
	
	
	c1 = str;
	while (*c1++);
	len = c1 - str;
	if (esc_str == NULL)
		esc_str = (si1 *) calloc((size_t) (len * 2), sizeof(si1));
	strcpy(esc_str, str);
	c1 = esc_str;
	c2 = str - 1;
	while (*++c2) {
		switch (*c2) {
			case '\n':
				*c1++ = '\\'; *c1++ = 'n'; break;
			case '\r':
				*c1++ = '\\'; *c1++ = 'r'; break;
			case '\t':
				*c1++ = '\\'; *c1++ = 't'; break;
			case '\7':
				*c1++ = '\\'; *c1++ = '7'; break;
			case '\\':
				*c1++ = '\\'; *c1++ = '\\'; break;
			default:
				*c1++ = *c2; break;
		}
	}
	*c1 = 0;
	
	return(esc_str);
}


CHANNEL_m11	*read_channel_d11(CHANNEL_m11 *chan, TIME_SLICE_m11 *slice, ...)  // varargs: si1 *chan_path, ui4 flags, si1 *password
{
	extern GLOBALS_m11		*globals_m11;
	TERN_m11			open_channel, free_channel;
	si1                             tmp_str[FULL_FILE_NAME_BYTES_m11], *chan_path, *password;
	si1                             num_str[FILE_NUMBERING_DIGITS_m11 + 1];
	ui4                             flags;
	si4                             i, j, k, n_segs, seg_offset;
	va_list				args;
	SEGMENT_m11			*seg;
	READ_MED_THREAD_INFO_d11	*seg_thread_infos;
	

	// open channel
	open_channel = free_channel = FALSE_m11;
	if (chan == NULL)
		open_channel = free_channel = TRUE_m11;
	else if (!(chan->flags & LH_OPEN_m11))
		open_channel = TRUE_m11;
	if (open_channel == TRUE_m11) {
		// get varargs
		va_start(args, slice);
		chan_path = va_arg(args, si1 *);
		flags = va_arg(args, ui4);
		password = va_arg(args, si1 *);
		va_end(args);
		// open channel
		chan = open_channel_m11(chan, slice, chan_path, flags, password);
		if (chan == NULL) {
			error_message_m11("%s(): error opening channel\n", __FUNCTION__);
			return(NULL);
		}
	}
	
	// process time slice (passed slice is not modified)
	if (slice == NULL) {
		if (all_zeros_m11((ui1 *) &chan->time_slice, (si4) sizeof(TIME_SLICE_m11)) == TRUE_m11)
			initialize_time_slice_m11(&chan->time_slice);  // read whole channel
	} else {  // passed slice supersedes structure slice
		chan->time_slice = *slice;  // passed slice is not modified
	}
	slice = &chan->time_slice;
	if (slice->conditioned == FALSE_m11)
		condition_time_slice_m11(slice);
		
	// get segment range
	if (slice->number_of_segments == UNKNOWN_m11) {
		n_segs = get_segment_range_m11((LEVEL_HEADER_m11 *) chan, slice);
		if (n_segs == 0) {
			if (free_channel == TRUE_m11)
				free_channel_m11(chan, TRUE_m11);
			return(NULL);
		}
	} else {
		n_segs = slice->number_of_segments;
	}
	
	// read segments
	seg_offset = 0;
	if (chan->flags & LH_MAP_ALL_SEGMENTS_m11)
		seg_offset = slice->start_segment_number - 1;
	
	if (n_segs == 1) {  // this is most common scenario - no need for thread overhead
		i = seg_offset;
		if (chan->segments[i] == NULL) {
			numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, i + 1);
			if (chan->type_code == LH_TIME_SERIES_CHANNEL_m11)
				sprintf_m11(tmp_str, "%s/%s_s%s.%s", chan->path, chan->name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m11);
			else  // LH_VIDEO_CHANNEL_m11
				sprintf_m11(tmp_str, "%s/%s_s%s.%s", chan->path, chan->name, num_str, VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m11);
			chan->segments[i] = read_segment_m11(NULL, slice, tmp_str, chan->flags, NULL);
		} else {
			read_segment_m11(chan->segments[i], slice);
		}
	} else {  // thread  out multiple segments
		#ifdef MATLAB_m11
		force_behavior_m11(SUPPRESS_ALL_OUTPUT_m11 | RETURN_ON_FAIL_m11);  // no printf output from threads in mex functions
		#endif
		// start read_segment threads
		seg_thread_infos = (READ_MED_THREAD_INFO_d11 *) calloc((size_t) n_segs, sizeof(READ_MED_THREAD_INFO_d11));
		for (i = slice->start_segment_number, j = seg_offset, k = 0; i <= slice->end_segment_number; ++i, ++j, ++k) {
			seg = chan->segments[j];
			if (seg == NULL) {
				numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, i);
				if (chan->type_code == LH_TIME_SERIES_CHANNEL_m11)
					sprintf_m11(seg_thread_infos[k].MED_dir, "%s/%s_s%s.%s", chan->path, chan->name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m11);
				else  // LH_VIDEO_CHANNEL_m11
					sprintf_m11(seg_thread_infos[k].MED_dir, "%s/%s_s%s.%s", chan->path, chan->name, num_str, VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m11);
				seg_thread_infos[k].flags  = chan->flags;
			} else {
				seg_thread_infos[k].MED_struct = (LEVEL_HEADER_m11 *) seg;
			}
			seg_thread_infos[k].slice = slice;
			seg_thread_infos[k].password = NULL;
			launch_thread_d11(&seg_thread_infos[k].thread_id, read_segment_thread_d11, (void *) (seg_thread_infos + k), HIGH_PRIORITY_d11, "~0", NULL, FALSE_m11, "read_segment_thread");
		}
		
		// wait for threads
		for (i = 0, j = seg_offset; i < n_segs; ++i, ++j) {
			pthread_join_d11(seg_thread_infos[i].thread_id, NULL);
			if (seg_thread_infos[i].returned_MED_struct == NULL) {
				if (free_channel == TRUE_m11)
					free_channel_m11(chan, TRUE_m11);
				return(NULL);
			}
			if (chan->segments[j] == NULL)
				chan->segments[j] = (SEGMENT_m11 *) seg_thread_infos[i].returned_MED_struct;
		}
		free((void *) seg_thread_infos);
		#ifdef MATLAB_m11
		force_behavior_m11(RESTORE_BEHAVIOR_m11);
		#endif
	}
	
	// update slice
	seg = chan->segments[slice->start_segment_number - 1];
	slice->start_time = seg->time_slice.start_time;
	slice->start_sample_number = seg->time_slice.start_sample_number;
	seg = chan->segments[slice->end_segment_number - 1];
	slice->end_time = seg->time_slice.end_time;
	slice->end_sample_number = seg->time_slice.end_sample_number;
	
	// records
	if (chan->flags & LH_READ_CHANNEL_RECORDS_m11)
		read_record_data_m11((LEVEL_HEADER_m11 *) chan, slice, 0);
	
	// update ephemeral data
	if (chan->flags & LH_GENERATE_EPHEMERAL_DATA_m11) {
		for (i = slice->start_segment_number, j = seg_offset; i <= slice->end_segment_number; ++i, ++j) {
			seg = chan->segments[j];
			if (seg->flags & LH_UPDATE_EPHEMERAL_DATA_m11) {
				merge_universal_headers_m11(chan->metadata_fps, seg->metadata_fps, NULL);
				merge_metadata_m11(chan->metadata_fps, seg->metadata_fps, NULL);
				if (seg->record_data_fps != NULL)
					merge_universal_headers_m11(chan->metadata_fps, seg->record_data_fps, NULL);
				seg->flags &= ~LH_UPDATE_EPHEMERAL_DATA_m11;  // clear segment flag
				chan->flags |= LH_UPDATE_EPHEMERAL_DATA_m11;  // set channel flag (for session)
			}
		}
	
		// fix session ephemeral FPS (from merge functions)
		if (chan->type_code == LH_TIME_SERIES_CHANNEL_m11)
			chan->metadata_fps->universal_header->type_code = TIME_SERIES_METADATA_FILE_TYPE_CODE_m11;
		else if (chan->type_code == LH_VIDEO_CHANNEL_m11)
			chan->metadata_fps->universal_header->type_code = VIDEO_METADATA_FILE_TYPE_CODE_m11;
		chan->metadata_fps->universal_header->segment_number = UNIVERSAL_HEADER_CHANNEL_LEVEL_CODE_m11;
		chan->metadata_fps->universal_header->segment_UID = UID_NO_ENTRY_m11;
	}

	// verbose
	if (globals_m11->verbose == TRUE_m11) {
		printf_m11("--------- Channel Universal Header ---------\n");
		show_universal_header_m11(chan->metadata_fps, NULL);
		printf_m11("------------ Channel Metadata --------------\n");
		show_metadata_m11(chan->metadata_fps, NULL, 0);
	}

	chan->last_access_time = current_uutc_m11();
	
	return(chan);
}


pthread_rval_d11	read_channel_thread_d11(void *ptr)
{
	READ_MED_THREAD_INFO_d11 *thread_info;

	
	thread_info = (READ_MED_THREAD_INFO_d11 *) ptr;
	if (thread_info->MED_struct == NULL)
		thread_info->returned_MED_struct = (LEVEL_HEADER_m11 *) read_channel_d11(NULL, thread_info->slice, thread_info->MED_dir, thread_info->flags, thread_info->password);
	else
		read_channel_d11((CHANNEL_m11 *) thread_info->MED_struct, thread_info->slice);
	
	return((pthread_rval_d11) 0);
}


si4     read_rc_field_d11(si1 *field_name, si1 **buffer, TERN_m11 update_buffer_ptr, si1 *field_value_str, sf8 *float_val, si8 *int_val, TERN_m11 *TERN_val)
{
	TERN_m11        option_selected, free_field_value_str, local_TERN_val, options_only;
	si1             *c, temp_str[256], *temp_si1_ptr, *field_title_ptr;
	si1             *type_ptr, type_str[256];
	si1             *options_ptr, options_str[256];
	si1             *default_value_ptr, default_value_str[256];
	si1             *field_value_ptr;
	si4             type, option_number;
	si8             item, default_item, local_int_val;
	sf8             local_float_val;
	
	
	// If update_buffer_ptr == TRUE_m11, caller can use it to progress serially through the RC file instead of starting at beginning each time.
	// Requires that caller knows that order of entries will stay the same. It is more efficient, but less flexible.
	// IMPORTANT: Caller responsible for saving a copy of *buffer for freeing, if it will be modified.
	
	// setup
	free_field_value_str = FALSE_m11;
	if (field_value_str == NULL) {  // need this string regardless of types
		field_value_str = (si1 *) malloc((size_t) 256);
		free_field_value_str = TRUE_m11;
	}
	// prevent error if user passes NULL to expected type (value will still be in field_value_str)
	if (float_val == NULL)
		float_val = &local_float_val;
	if (int_val == NULL)
		int_val = &local_int_val;
	if (TERN_val == NULL)
		TERN_val = &local_TERN_val;
	// zero strings
	*type_str = *options_str = *default_value_str = *field_value_str = 0;
	
	// find requested field entry
	c = *buffer;
	sprintf_m11(temp_str, "%%%% FIELD: %s", field_name);
	if ((field_title_ptr = STR_match_end_m11(temp_str, c)) == NULL)
		error_message_m11("%s(): Could not match field label \"%s\" in rc file\n", __FUNCTION__, temp_str);
	 
	// get type
	c = field_title_ptr;
	if ((type_ptr = STR_match_end_m11("%% TYPE:", c)) == NULL)
		error_message_m11("%s(): Could not match TYPE subfield in field \"%s\" of rc file\n", __FUNCTION__, field_name);
	while (*type_ptr == (si1) 32)  // space
		++type_ptr;
	item = sscanf(type_ptr, "%[^\r\n]", type_str);
	if (item) {
		temp_si1_ptr = type_str + strlen(type_str);
		while (*--temp_si1_ptr == (si1) 32);
		*++temp_si1_ptr = 0;
	} else
		error_message_m11("%s(): No TYPE subfield specified in field \"%s\" of rc file\n", __FUNCTION__, field_name);

	type = 0;
	if (strcmp(type_str, "string") == 0)
		type = RC_STRING_TYPE_d11;
	else if (strcmp(type_str, "float") == 0)
		type = RC_FLOAT_TYPE_d11;
	else if (strcmp(type_str, "integer") == 0)
		type = RC_INTEGER_TYPE_d11;
	else if (strcmp(type_str, "ternary") == 0)
		type = RC_TERNARY_TYPE_d11;
	else
	       error_message_m11("%s(): Could not match TYPE subfield in field \"%s\" of rc file\n", __FUNCTION__, field_name);

	// get options pointer
	c = type_ptr;
	options_only = FALSE_m11;
	if ((options_ptr = STR_match_end_m11("%% OPTIONS", c)) == NULL)
	      error_message_m11("%s(): Could not match OPTIONS subfield in field \"%s\" of rc file\n", __FUNCTION__, field_name);
	if (*options_ptr == ':') {
		++options_ptr;
	} else if (strncmp(options_ptr, " ONLY:", 6) == 0) {
		options_ptr += 6;
		options_only = TRUE_m11;
	} else {
	      error_message_m11("%s(): Could not match OPTIONS subfield in field \"%s\" of rc file\n", __FUNCTION__, field_name);
	}
	while (*options_ptr == (si1) 32)  // space
		++options_ptr;
	item = sscanf(options_ptr, "%[^\r\n]", options_str);
	if (item) {
		temp_si1_ptr = options_str + strlen(options_str);
		while (*--temp_si1_ptr == (si1) 32);
		*++temp_si1_ptr = 0;
	}

	// get default value pointer
	c = options_ptr;
	if ((default_value_ptr = STR_match_end_m11("%% DEFAULT:", c)) == NULL)
	     error_message_m11("%s(): Could not match DEFAULT subfield in field \"%s\" of rc file\n", __FUNCTION__, field_name);
	while (*default_value_ptr == (si1) 32)  // space
		++default_value_ptr;
	default_item = sscanf(default_value_ptr, "%[^\r\n]", default_value_str);
	if (default_item) {
		temp_si1_ptr = default_value_str + strlen(default_value_str);
		while (*--temp_si1_ptr == (si1) 32);  // space
		*++temp_si1_ptr = 0;
	}

	// get field value as string
	c = default_value_ptr;
	if ((field_value_ptr = STR_match_end_m11("%% VALUE:", c)) == NULL)
		error_message_m11("%s(): Could not match value field label \"%s\" in rc file\n", __FUNCTION__, temp_str);
	while (*field_value_ptr == (si1) 32)  // space
		++field_value_ptr;
	item = sscanf(field_value_ptr, "%[^\r\n]", field_value_str);
	temp_si1_ptr = field_value_str + strlen(field_value_str);
	if (update_buffer_ptr == TRUE_m11)
		*buffer = temp_si1_ptr;
	if (item) {
		while (*--temp_si1_ptr == (si1) 32);  // space
		*++temp_si1_ptr = 0;
	} else {
		strcpy(field_value_str, "DEFAULT");
	}
	
READ_RC_HANDLE_DEFAULT_d11:
	
	// VALUE field is "DEFAULT", and default may be "PROMPT"
	if (strcmp(field_value_str, "DEFAULT") == 0) {
		if (default_item)
			strcpy(field_value_str, default_value_str);
		else
			error_message_m11("%s(): No DEFAULT value to enter in field \"%s\" of rc file\n", __FUNCTION__, field_name);
	}

	// PROMPT (Note: user can enter "DEFAULT", "NO ENTRY", or any of the recognized OPTIONS here if desired)
	if (strcmp(field_value_str, "PROMPT") == 0) {
		if (options_only == TRUE_m11)
			printf_m11("RC FIELD: \033[31m%s\033[0m\nOPTIONS: \033[31m%s\033[0m\nDEFAULT: \033[31m%s\033[0m\nEnter an option: ", field_name, options_str, default_value_str);
		else
			printf_m11("RC FIELD: \033[31m%s\033[0m\nOPTIONS: \033[31m%s\033[0m\nDEFAULT: \033[31m%s\033[0m\nEnter a value: ", field_name, options_str, default_value_str);
		item = scanf("%[^\r\n]", field_value_str); fgetc(stdin); putchar_m11('\n');
		if (item) {
			temp_si1_ptr = field_value_str + strlen(field_value_str);
			while (*--temp_si1_ptr == (si1) 32);  // space
			*++temp_si1_ptr = 0;
		}
	}

	// no entry
	option_selected = RC_NO_OPTION_d11;
	if ((strcmp(field_value_str, "NO ENTRY") == 0)) {
		if (options_only == TRUE_m11) {
			error_message_m11("%s(): \"NO ENTRY\" is not an option in field \"%s\" of rc file => using default\n", __FUNCTION__, field_name);
			strcpy(field_value_str, "DEFAULT");
			goto READ_RC_HANDLE_DEFAULT_d11;
		} else
			option_selected = RC_NO_ENTRY_d11;
	}

	// options
	if (option_selected == RC_NO_OPTION_d11) {
		option_number = 0;
		options_ptr = options_str;
		while (1) {
			while (*options_ptr == 32 || *options_ptr == ',')  // space or comma
				++options_ptr;
			if (*options_ptr == 0)
				break;
			++option_number;
			item = sscanf(options_ptr, "%[^,\r\n]", temp_str);
			if (item) {
				if (strcmp(temp_str, field_value_str) == 0) {
					option_selected = option_number;
					strcpy(field_value_str, temp_str);
					break;
				}
				options_ptr += strlen(temp_str);
			} else {
				break;
			}
		}
		if (option_selected == RC_NO_OPTION_d11) {
			if (options_only == TRUE_m11) {
				error_message_m11("%s(): String \"%s\" is not an option in field \"%s\" of rc file => using default\n", __FUNCTION__, field_value_str, field_name);
				strcpy(field_value_str, "DEFAULT");
				goto READ_RC_HANDLE_DEFAULT_d11;
			}
		}
	}
			
	// user entered value
	switch (type) {
		case RC_STRING_TYPE_d11:
			if (option_selected == RC_NO_ENTRY_d11)
				field_value_str[0] = 0;  // function default
			break;
		case RC_FLOAT_TYPE_d11:
			if (option_selected == RC_NO_ENTRY_d11)
				*float_val = 0.0;  // function default
			else {
				item = sscanf(field_value_str, "%lf", float_val);
				if (item != 1 && option_selected == RC_NO_OPTION_d11)
					error_message_m11("%s(): Could not convert string \"%s\" to type \"%s\" in field \"%s\" of rc file\n", __FUNCTION__, field_value_str, type_str, field_name);
			}
			break;
		case RC_INTEGER_TYPE_d11:
			if (option_selected == RC_NO_ENTRY_d11)
				*int_val = 0;  // function default
			else {
				// I have no idea why, but the Visual Studio linker can't find sscanf_m11() - just this function
				item = sscanf_m11(field_value_str, "%ld", int_val);
				if (item != 1 && option_selected == RC_NO_OPTION_d11)
					error_message_m11("%s(): Could not convert string \"%s\" to type \"%s\" in field \"%s\" of rc file\n", __FUNCTION__, field_value_str, type_str, field_name);
			}
			break;
		case RC_TERNARY_TYPE_d11:
			if (option_selected == RC_NO_ENTRY_d11) {
				*TERN_val = UNKNOWN_m11;  // function default
				break;
			}
			if (option_selected == RC_NO_OPTION_d11) {  // user entered value
				item = sscanf(field_value_str, "%hhd", TERN_val);
				if (item != 1) {
					error_message_m11("%s(): Could not convert string \"%s\" to type \"%s\" in field \"%s\" of rc file\n", __FUNCTION__, field_value_str, type_str, field_name);
					break;
				}
				if (*TERN_val < FALSE_m11 || *TERN_val > TRUE_m11)
					error_message_m11("%s(): Invalid value for type \"%s\" in field \"%s\" of rc file\n", __FUNCTION__, type_str, field_name);
			} else {  // user entered option
				if (strcmp(field_value_str, "YES") == 0 || strcmp(field_value_str, "TRUE") == 0) {
					*TERN_val = TRUE_m11;
				} else if (strcmp(field_value_str, "NO") == 0 || strcmp(field_value_str, "FALSE") == 0) {
					*TERN_val = FALSE_m11;
				} else if (strcmp(field_value_str, "UNKNOWN") == 0) {
					*TERN_val = UNKNOWN_m11;
				}
			}
			break;
		default:
			break;
	}

	if (free_field_value_str == TRUE_m11)
		free((void *) field_value_str);
	
	return(option_selected);
}


pthread_rval_d11	read_segment_thread_d11(void *ptr)
{
	READ_MED_THREAD_INFO_d11 *thread_info;
	

	thread_info = (READ_MED_THREAD_INFO_d11 *) ptr;
	if (thread_info->MED_struct == NULL)
		thread_info->returned_MED_struct = (LEVEL_HEADER_m11 *) read_segment_m11(NULL, thread_info->slice, thread_info->MED_dir, thread_info->flags, thread_info->password);
	else
		read_segment_m11((SEGMENT_m11 *) thread_info->MED_struct, thread_info->slice);
	
	return((pthread_rval_d11) 0);
}


SESSION_m11	*read_session_d11(SESSION_m11 *sess, TIME_SLICE_m11 *slice, ...)  // varargs: void *file_list, si4 list_len, ui4 flags, si1 *password
{
	extern GLOBALS_m11		*globals_m11;
	TERN_m11			open_session, free_session;
	si1                             *password, num_str[FILE_NUMBERING_DIGITS_m11 + 1];
	si1				tmp_str[FULL_FILE_NAME_BYTES_m11];
	ui4                             flags;
	si4                             i, j, list_len, seg_offset, active_ts_chans, active_vid_chans;
	sf8				ref_chan_sf;
	void				*file_list;
	va_list				args;
	CHANNEL_m11			*chan;
	UNIVERSAL_HEADER_m11		*uh;
	READ_MED_THREAD_INFO_d11	*ts_chan_thread_infos, *vid_chan_thread_infos;
	SEGMENTED_SESS_RECS_m11		*ssr;
	

	// open session
	open_session = free_session = FALSE_m11;
	if (sess == NULL)
		open_session = free_session = TRUE_m11;
	else if (!(sess->flags & LH_OPEN_m11)) {
		free_session_m11(sess, FALSE_m11);
		open_session = TRUE_m11;
	}
	if (open_session == TRUE_m11) {
		// get varargs
		va_start(args, slice);
		file_list = va_arg(args, void *);
		list_len = va_arg(args, si4);
		flags = va_arg(args, ui4);
		password = va_arg(args, si1 *);
		va_end(args);
		// open session
		sess = open_session_m11(sess, slice, file_list, list_len, flags, password);
		if (sess == NULL) {
			error_message_m11("%s(): error opening session\n", __FUNCTION__);
			return(NULL);
		}
	}
	
	// process time slice (passed slice is not modified)
	if (slice == NULL) {
		if (all_zeros_m11((ui1 *) &sess->time_slice, (si4) sizeof(TIME_SLICE_m11)) == TRUE_m11)
			initialize_time_slice_m11(&sess->time_slice);  // read whole session
	} else {  // passed slice supersedes structure slice
		sess->time_slice = *slice;  // passed slice is not modified
	}
	slice = &sess->time_slice;
	if (slice->conditioned == FALSE_m11)
		condition_time_slice_m11(slice);
	
	// get segment range
	if (slice->number_of_segments == UNKNOWN_m11) {
		if (get_segment_range_m11((LEVEL_HEADER_m11 *) sess, slice) == 0) {
			if (free_session == TRUE_m11)
				free_session_m11(sess, TRUE_m11);
			return(NULL);
		}
	}
	seg_offset = get_segment_offset_m11((LEVEL_HEADER_m11 *) sess);
	
	// thread out channel reads
	#ifdef MATLAB_m11
	force_behavior_m11(SUPPRESS_ALL_OUTPUT_m11 | RETURN_ON_FAIL_m11);  // no printf output from threads in mex functions
	#endif
	active_ts_chans = active_vid_chans = 0;
	if (sess->number_of_time_series_channels) {
		for (i = 0; i < sess->number_of_time_series_channels; ++i)
			if (sess->time_series_channels[i]->flags & LH_CHANNEL_ACTIVE_m11)
				++active_ts_chans;
		ts_chan_thread_infos = (READ_MED_THREAD_INFO_d11 *) calloc((size_t) active_ts_chans, sizeof(READ_MED_THREAD_INFO_d11));
		for (i = j = 0; i < sess->number_of_time_series_channels; ++i) {
			chan = sess->time_series_channels[i];
			if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
				ts_chan_thread_infos[j].MED_struct = (LEVEL_HEADER_m11 *) chan;
				ts_chan_thread_infos[j].slice = slice;
				ts_chan_thread_infos[j].password = NULL;
				launch_thread_d11(&ts_chan_thread_infos[j].thread_id, read_channel_thread_d11, (void *) (ts_chan_thread_infos + j), HIGH_PRIORITY_d11, "~0", NULL, FALSE_m11, "read_channel_thread");
				++j;
			}
		}
	}
	if (sess->number_of_video_channels) {
		for (i = 0; i < sess->number_of_video_channels; ++i)
			if (sess->video_channels[i]->flags & LH_CHANNEL_ACTIVE_m11)
				++active_vid_chans;
		vid_chan_thread_infos = (READ_MED_THREAD_INFO_d11 *) calloc((size_t) active_vid_chans, sizeof(READ_MED_THREAD_INFO_d11));
		for (i = j = 0; i < sess->number_of_video_channels; ++i) {
			chan = sess->video_channels[i];
			if (chan->flags & LH_CHANNEL_ACTIVE_m11) {
				vid_chan_thread_infos[j].MED_struct = (LEVEL_HEADER_m11 *) chan;
				vid_chan_thread_infos[j].slice = slice;
				vid_chan_thread_infos[j].password = NULL;
				launch_thread_d11(&vid_chan_thread_infos[j].thread_id, read_channel_thread_d11, (void *) (vid_chan_thread_infos + j), HIGH_PRIORITY_d11, "~0", NULL, FALSE_m11, "read_channel_thread");
				++j;
			}
		}
	}

	// wait for threads
	if (active_ts_chans) {
		for (i = 0; i < active_ts_chans; ++i)
			pthread_join_d11(ts_chan_thread_infos[i].thread_id, NULL);
		free((void *) ts_chan_thread_infos);
	}
	if (active_vid_chans) {
		for (i = 0; i < active_vid_chans; ++i)
			pthread_join_d11(vid_chan_thread_infos[i].thread_id, NULL);
		free((void *) vid_chan_thread_infos);
	}
	#ifdef MATLAB_m11
	force_behavior_m11(RESTORE_BEHAVIOR_m11);
	#endif
	
	// update slice
	chan = globals_m11->reference_channel;
	if (!(chan->flags & LH_CHANNEL_ACTIVE_m11)) {  // find first active channel
		ref_chan_sf = chan->metadata_fps->metadata->time_series_section_2.sampling_frequency;
		for (i = 0; i < sess->number_of_time_series_channels; ++i) {
			chan = sess->time_series_channels[i];
			if (chan->flags & LH_CHANNEL_ACTIVE_m11)
				break;
		}
		if (i == sess->number_of_time_series_channels) {
			for (i = 0; i < sess->number_of_video_channels; ++i) {
				chan = sess->video_channels[i];
				if (chan->flags & LH_CHANNEL_ACTIVE_m11)
					break;
			}
		}
		// reference channel not active
		if (chan->metadata_fps->metadata->time_series_section_2.sampling_frequency != ref_chan_sf) {
			slice->start_sample_number = slice->end_sample_number = SAMPLE_NUMBER_NO_ENTRY_m11;
		} else {
			slice->start_sample_number = chan->time_slice.start_sample_number;
			slice->end_sample_number = chan->time_slice.end_sample_number;
		}
	} else {
		slice->start_sample_number = chan->time_slice.start_sample_number;
		slice->end_sample_number = chan->time_slice.end_sample_number;
	}
	slice->start_time = chan->time_slice.start_time;
	slice->end_time = chan->time_slice.end_time;
	
	// read session record data
	if (sess->flags & LH_READ_SESSION_RECORDS_m11)
		if (sess->record_indices_fps != NULL)
			read_record_data_m11((LEVEL_HEADER_m11 *) sess, slice, 0);
		
	// read segmented session record data (ephemeral data updated on open here)
	ssr = sess->segmented_sess_recs;
	if (sess->flags & LH_READ_SEGMENTED_SESS_RECS_m11 && ssr != NULL) {
		for (i = slice->start_segment_number, j = seg_offset; i <= slice->end_segment_number; ++i, ++j) {
			// allocate new segment records
			if (ssr->record_indices_fps[j] == NULL) {
				numerical_fixed_width_string_m11(num_str, FILE_NUMBERING_DIGITS_m11, i);
				sprintf_m11(tmp_str, "%s/%s_s%s.%s", ssr->path, ssr->name, num_str, RECORD_INDICES_FILE_TYPE_STRING_m11);
				if (file_exists_m11(tmp_str) == FILE_EXISTS_m11)
					ssr->record_indices_fps[j] = read_file_m11(ssr->record_indices_fps[j], tmp_str, 0, 0, 0, ssr->flags, NULL, USE_GLOBAL_BEHAVIOR_m11);
				sprintf_m11(tmp_str, "%s/%s_s%s.%s", ssr->path, ssr->name, num_str, RECORD_DATA_FILE_TYPE_STRING_m11);
				if (file_exists_m11(tmp_str) == FILE_EXISTS_m11)
					ssr->record_data_fps[j] = read_file_m11(ssr->record_data_fps[j], tmp_str, 0, 0, 0, ssr->flags, NULL, USE_GLOBAL_BEHAVIOR_m11);
				if (sess->flags & LH_GENERATE_EPHEMERAL_DATA_m11) {
					if (sess->time_series_metadata_fps != NULL)
						merge_universal_headers_m11(sess->time_series_metadata_fps, ssr->record_data_fps[j], NULL);
					if (sess->video_metadata_fps != NULL)
						merge_universal_headers_m11(sess->video_metadata_fps, ssr->record_data_fps[j], NULL);
					// if new segment records opened, new segment was opened too - flag set in open segment so ephemeral universal header will be fixed below
				}
			}
			if (ssr->record_indices_fps[j] != NULL)
				read_record_data_m11((LEVEL_HEADER_m11 *) ssr, slice, i);
		}
	}
	
	// update ephemeral data
	if (sess->flags & LH_GENERATE_EPHEMERAL_DATA_m11) {
		// time series ephemeral data
		for (i = 0; i < sess->number_of_time_series_channels; ++i) {
			chan = sess->time_series_channels[i];
			if (chan->flags & (LH_CHANNEL_ACTIVE_m11 | LH_UPDATE_EPHEMERAL_DATA_m11)) {
				merge_universal_headers_m11(sess->time_series_metadata_fps, chan->metadata_fps, NULL);
				merge_metadata_m11(sess->time_series_metadata_fps, chan->metadata_fps, NULL);
				chan->flags &= ~LH_UPDATE_EPHEMERAL_DATA_m11;  // clear flag
			}
		}
		// video ephemeral data
		for (i = 0; i < sess->number_of_video_channels; ++i) {
			chan = sess->video_channels[i];
			if (chan->flags & (LH_CHANNEL_ACTIVE_m11 | LH_UPDATE_EPHEMERAL_DATA_m11)) {
				merge_universal_headers_m11(sess->video_metadata_fps, chan->metadata_fps, NULL);
				merge_metadata_m11(sess->video_metadata_fps, chan->metadata_fps, NULL);
				chan->flags &= ~LH_UPDATE_EPHEMERAL_DATA_m11;  // clear flag
			}
		}
		// fix session ephemeral universal header (from merge functions)
		if (sess->number_of_time_series_channels) {
			uh = sess->time_series_metadata_fps->universal_header;
			uh->type_code = TIME_SERIES_METADATA_FILE_TYPE_CODE_m11;
			uh->segment_number = UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m11;
			uh->session_UID = globals_m11->session_UID;
			uh->channel_UID = uh->segment_UID = UID_NO_ENTRY_m11;
		}
		if (sess->number_of_video_channels) {
			uh = sess->video_metadata_fps->universal_header;
			uh->type_code = VIDEO_METADATA_FILE_TYPE_CODE_m11;
			uh->segment_number = UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m11;
			uh->session_UID = globals_m11->session_UID;
			uh->channel_UID = uh->segment_UID = UID_NO_ENTRY_m11;
		}
	}
	
	// verbose
	if (globals_m11->verbose == TRUE_m11) {
		if (sess->time_series_metadata_fps != NULL) {
			printf_m11("--------- Session Time Series Universal Header ---------\n");
			show_universal_header_m11(sess->time_series_metadata_fps, NULL);
			printf_m11("------------ Session Time Series Metadata --------------\n");
			show_metadata_m11(sess->time_series_metadata_fps, NULL, 0);
		}
		if (sess->video_metadata_fps != NULL) {
			printf_m11("------------ Session Video Universal Header ------------\n");
			show_universal_header_m11(sess->video_metadata_fps, NULL);
			printf_m11("--------------- Session Video Metadata -----------------\n");
			show_metadata_m11(sess->video_metadata_fps, NULL, 0);
		}
	}

	sess->last_access_time = current_uutc_m11();
	
	return(sess);
}


TERN_m11	recover_passwords_d11(UNIVERSAL_HEADER_m11 *universal_header)
{
	TERN_m11	ret_val;
	si1 		level_3_password[PASSWORD_BYTES_m11 + 1];
	
	
	set_L3_pw_d11(level_3_password);
	ret_val = recover_passwords_m11(level_3_password, universal_header);
	
	return(ret_val);
}


void    rectify_d11(si4 *input_buffer, si4 *output_buffer, si8 len)
{
	si4        *si4_p1, *si4_p2;
	si8        i;
	
	
	// rectify data from input_buffer to output_buffer
	// if input_buffer == output_buffer rectification will be done in place
	
	si4_p1 = input_buffer;
	si4_p2 = output_buffer;
	for (i = len; i--; ++si4_p1)
		*si4_p2++ = ABS_m11(*si4_p1);
	
	return;
}


si8	recv_transmission_d11(TRANSMISSION_INFO_d11 *trans_info)
{
	ui1				*buffer, *ui1_p;
	ui4				saved_ID_code;
	si4				i, sock_fd, encryption_blocks;
	si8				bytes_received, ret_val, buffer_bytes, transmission_bytes;
	TRANSMISSION_HEADER_d11		*header;


	if (globals_d11->sk_matrix == NULL)
		initialize_sk_matrix_d11();

	if (trans_info == NULL) {
		warning_message_m11("%s(): transmission info is NULL\n", __FUNCTION__);
		return((si8) FALSE_m11);
	}
	header = trans_info->header;
	buffer_bytes = trans_info->buffer_bytes;
	buffer = trans_info->buffer;
	if ((sock_fd = trans_info->sock_fd) == -1) {
		if (connect_to_server_d11(trans_info, NULL, NULL, 0) == FALSE_m11) {
			warning_message_m11("%s(): failed to open socket %s\n", __FUNCTION__, trans_info->addr_str);
			return(TH_ERR_SOCK_FAILED_TO_OPEN_d11);
		}
		sock_fd = trans_info->sock_fd;
	}
	saved_ID_code = header->ID_code;  // save - will be overwritten

	// get transmission header
	bytes_received = 0;
	do {
		bytes_received += recv(sock_fd, (void *) (buffer + bytes_received), buffer_bytes - bytes_received, 0);
		if (bytes_received == 0) {
			warning_message_m11("%s(): socket %s closed\n", __FUNCTION__, trans_info->addr_str);
			close_transmission_d11(trans_info);
			return(TH_ERR_SOCK_CLOSED_d11);
		}
		if (bytes_received == -1) {
			warning_message_m11("%s(): socket %s error\n", __FUNCTION__, trans_info->addr_str);
			close_transmission_d11(trans_info);
			return(TH_ERR_SOCK_FAILED_d11);
		}
	} while (bytes_received < (si8) TH_HEADER_BYTES_d11);
	
	// check transmission ID, if passed
	if (saved_ID_code != TH_ID_CODE_NO_ENTRY_d11) {
		if (saved_ID_code != header->ID_code) {
			warning_message_m11("%s(): socket %s transmission ID mismatch\n", __FUNCTION__, trans_info->addr_str);
			close_transmission_d11(trans_info);
			return(TH_ERR_ID_MISMATCH_d11);
		}
	}
	
	// realloc if necessary
	transmission_bytes = header->transmission_bytes;
	if (transmission_bytes > buffer_bytes) {
		trans_info = alloc_trans_info_d11(transmission_bytes, trans_info, NULL, NULL, 0, LS_SOCK_TIMEOUT_SECS_d11);
		buffer = trans_info->buffer;
		header = trans_info->header;
	}

	// receive remainder of transmission (if any)
	while (bytes_received < transmission_bytes) {
#if defined MACOS_m11 || defined LINUX_m11
		ret_val = recv(sock_fd, (void *) (buffer + bytes_received), buffer_bytes - bytes_received, 0);
#endif
#ifdef WINDOWS_m11
		ret_val = recv(sock_fd, (void *) (buffer + bytes_received), buffer_bytes - bytes_received, 0);
#endif		
		if (ret_val == 0) {
			warning_message_m11("%s(): socket %s closed => returning bytes received\n", __FUNCTION__, trans_info->addr_str);
			close_transmission_d11(trans_info);
			return(bytes_received);
		}
		if (ret_val == -1) {
			warning_message_m11("%s(): socket %s closed => returning bytes received\n", __FUNCTION__, trans_info->addr_str);
			close_transmission_d11(trans_info);
			return(bytes_received);
		}
		bytes_received += ret_val;
	}

	// decrypt
	if (header->flags & TH_FLAGS_ENCRYPTED_MASK_d11) {
		ui1_p = buffer + TH_HEADER_BYTES_d11;
		encryption_blocks = (transmission_bytes - 1) / ENCRYPTION_BLOCK_BYTES_m11;  // works b/c TH_HEADER_BYTES_d11 == ENCRYPTION_BLOCK_BYTES_m11
		for (i = 0; i < encryption_blocks; ++i) {
			AES_decrypt_m11(ui1_p, ui1_p, NULL, globals_d11->sk_matrix);
			ui1_p += ENCRYPTION_BLOCK_BYTES_m11;
		}
		header->flags &= ~TH_FLAGS_ENCRYPTED_MASK_d11;  // reset encryption flag
	}

	// close
	if (header->flags & TH_FLAGS_CLOSE_CONNECTION_MASK_d11) {
		close_transmission_d11(trans_info);
		header->flags &= ~TH_FLAGS_CLOSE_CONNECTION_MASK_d11;  // reset close flag
	}

	return(bytes_received);
}


si4     remove_alloc_entity_d11(void *ptr, const si1 *function, si4 line)
{
	si4                     i;
	ALLOC_ENTITY_d11        *ae;
	
	
	if (globals_d11->alloc_tracking == FALSE_m11) {
		warning_message_m11("%s(): Memory allocation tracking is not enabled", __FUNCTION__);
		return(-1);
	}
	
	// search backwards
	ae = globals_d11->alloc_entities + globals_d11->ae_n_entities - 1;
	for (i = 0; i < globals_d11->ae_n_entities; ++i, --ae)
		if (ae->ptr == ptr)
			break;
	if (i == globals_d11->ae_n_entities) {
		warning_message_m11("%s(): object at location %lu was not allocated  [%s(), line %d]", __FUNCTION__, (ui8) ptr, function, line);
		return(-1);
	}
	if (globals_d11->verbose == TRUE_m11)
		show_alloc_entity_d11(ae);
	if (ae->freed == TRUE_m11) {
		warning_message_m11("%s(): object at location %lu is already freed  [%s(), line %d]", __FUNCTION__, (ui8) ptr, function, line);
		return(-1);
	}
	
	ae->freed = TRUE_m11;
	globals_d11->ae_curr_allocated_bytes -= ae->n_bytes;
	--globals_d11->ae_curr_allocated_entities;
	
	return(0);
}


TERN_m11	send_message_d11(TRANSMISSION_INFO_d11 *trans_info, ui1 type, TERN_m11 encrypt, si1 *fmt, ...)
{
	si1				*message_text;
	si8				bytes_sent;
	va_list 			args;
	TRANSMISSION_HEADER_d11		*header;
	MESSAGE_HEADER_d11		*msg;


	switch (type) {
		case TH_ERROR_TYPE_d11:
		case TH_WARNING_TYPE_d11:
		case TH_SUCCESS_TYPE_d11:
		case TH_MESSAGE_TYPE_d11:
			break;
		default:
			warning_message_m11("%s(): unrecognized message type", __FUNCTION__);
			return(FALSE_m11);
	}

	header = trans_info->header;
	msg = (MESSAGE_HEADER_d11 *) (header + 1);
	message_text = (si1 *) (msg + 1);
	va_start(args, fmt);
	vsprintf_m11(message_text, fmt, args);
	va_end(args);
	build_message_d11(msg, message_text);

	header->type = type;
	if (type == TH_ERROR_TYPE_d11)
		header->flags |= TH_FLAGS_CLOSE_CONNECTION_MASK_d11;
	if (encrypt == TRUE_m11)
		header->flags |= TH_FLAGS_ENCRYPTED_MASK_d11;
	header->transmission_bytes = TH_HEADER_BYTES_d11 + MESSAGE_HEADER_BYTES_d11 + msg->message_bytes;
	
	bytes_sent = send_transmission_d11(trans_info);
	header = trans_info->header;
	if (bytes_sent != header->transmission_bytes)
		return(FALSE_m11);

	return(TRUE_m11);
}


si8	send_transmission_d11(TRANSMISSION_INFO_d11 *trans_info)
{
	ui1				*buffer, *ui1_p;
	si4				i, sock_fd, encryption_blocks;
	si8				bytes_sent, ret_val, buffer_bytes, transmission_bytes;
	TRANSMISSION_HEADER_d11		*header;

	
	if (globals_d11->sk_matrix == NULL)
		initialize_sk_matrix_d11();

	if (trans_info == NULL) {
		warning_message_m11("%s(): transmission info is NULL\n", __FUNCTION__);
		return((si8) FALSE_m11);
	}
	header = trans_info->header;
	buffer_bytes = trans_info->buffer_bytes;
	transmission_bytes = header->transmission_bytes;
	buffer = trans_info->buffer;
	if ((sock_fd = trans_info->sock_fd) == -1) {
		if (connect_to_server_d11(trans_info, NULL, NULL, 0) == FALSE_m11) {
			warning_message_m11("%s(): failed to open socket %s\n", __FUNCTION__, trans_info->addr_str);
			return((si8) FALSE_m11);
		}
		sock_fd = trans_info->sock_fd;
	}

	if (transmission_bytes > buffer_bytes) {
		warning_message_m11("%s(): buffer too small for transmission\n", __FUNCTION__, trans_info->addr_str);
		return((si8) FALSE_m11);
	}

	// encrypt
	if (header->flags & TH_FLAGS_ENCRYPTED_MASK_d11) {
		ui1_p = buffer + TH_HEADER_BYTES_d11;
		encryption_blocks = (header->transmission_bytes - 1) / ENCRYPTION_BLOCK_BYTES_m11;  // works b/c TH_HEADER_BYTES_d11 == ENCRYPTION_BLOCK_BYTES_m11
		for (i = 0; i < encryption_blocks; ++i) {
			AES_encrypt_m11(ui1_p, ui1_p, NULL, globals_d11->sk_matrix);
			ui1_p += ENCRYPTION_BLOCK_BYTES_m11;
		}
	}

	// transmit
	bytes_sent = 0;
	while (bytes_sent < transmission_bytes) {
		ret_val = send(sock_fd, (void *) (buffer + bytes_sent), transmission_bytes - bytes_sent, 0);
		if (ret_val == 0) {
			warning_message_m11("%s(): socket %s closed => returning bytes sent", __FUNCTION__, trans_info->addr_str);
			close_transmission_d11(trans_info);
			return(bytes_sent);
		}
		if (ret_val == -1) {
			warning_message_m11("%s(): socket %s error => returning bytes sent", __FUNCTION__, trans_info->addr_str);
			close_transmission_d11(trans_info);
			return(bytes_sent);
		}
		bytes_sent += ret_val;
	}
	
	// reset encryption flag
	header->flags &= ~TH_FLAGS_ENCRYPTED_MASK_d11;

	// close
	if (header->flags & TH_FLAGS_CLOSE_CONNECTION_MASK_d11) {
		close_transmission_d11(trans_info);
		header->flags &= ~TH_FLAGS_CLOSE_CONNECTION_MASK_d11;  // reset close flag
	}

	return(bytes_sent);
}


void    sendgrid_email_d11(si1 *sendgrid_key, si1 *to_email, si1 *cc_email, si1 *to_name, si1 *subject, si1 *content, si1 *from_email, si1 *from_name, si1 *reply_to_email, si1 *reply_to_name)
{
	TERN_m11	include_cc = TRUE_m11;
	si1     	command[2048], escaped_content[2048];
	
	
	if (content != NULL) {
		if (*content)
			re_escape_d11(content, escaped_content);
	} else {
		content = " ";  // sendgrid requires at least one character
	}
	
	if (cc_email == NULL)
		include_cc = FALSE_m11;
	else if (*cc_email == 0)
		include_cc = FALSE_m11;

#if defined MACOS_m11 || defined LINUX_m11
	if (include_cc == TRUE_m11)
		sprintf(command, "curl --request POST --url https://api.sendgrid.com/v3/mail/send --header 'authorization: Bearer %s' --header 'content-type: application/json' --data '{\"personalizations\":[{\"to\": [{\"email\": \"%s\", \"name\": \"%s\"}], \"cc\": [{\"email\": \"%s\"}], \"subject\": \"%s\"}], \"content\": [{\"type\": \"text/plain\", \"value\": \"%s\"}], \"from\": {\"email\": \"%s\", \"name\": \"%s\"}, \"reply_to\": {\"email\": \"%s\", \"name\": \"%s\"}}' > %s 2>&1", sendgrid_key, to_email, to_name, cc_email, subject, escaped_content, from_email, from_name, reply_to_email, reply_to_name, NULL_DEVICE_m11);
	else
		sprintf(command, "curl --request POST --url https://api.sendgrid.com/v3/mail/send --header 'authorization: Bearer %s' --header 'content-type: application/json' --data '{\"personalizations\":[{\"to\": [{\"email\": \"%s\", \"name\": \"%s\"}], \"subject\": \"%s\"}], \"content\": [{\"type\": \"text/plain\", \"value\": \"%s\"}], \"from\": {\"email\": \"%s\", \"name\": \"%s\"}, \"reply_to\": {\"email\": \"%s\", \"name\": \"%s\"}}' > %s 2>&1", sendgrid_key, to_email, to_name, subject, escaped_content, from_email, from_name, reply_to_email, reply_to_name, NULL_DEVICE_m11);
	system(command);
#endif
	
#ifdef WINDOWS_m11
	if (include_cc == TRUE_m11)
		sprintf(command, "curl.exe --request POST --url https://api.sendgrid.com/v3/mail/send --header \"authorization: Bearer %s\" --header \"content-type: application/json\" --data \"{\\\"personalizations\\\":[{\\\"to\\\": [{\\\"email\\\": \\\"%s\\\", \\\"name\\\": \\\"%s\\\"}], \\\"cc\\\": [{\\\"email\\\": \\\"%s\\\"}], \\\"subject\\\": \\\"%s\\\"}], \\\"content\\\": [{\\\"type\\\": \\\"text/plain\\\", \\\"value\\\": \\\"%s\\\"}], \\\"from\\\": {\\\"email\\\": \\\"%s\\\", \\\"name\\\": \\\"%s\\\"}, \\\"reply_to\\\": {\\\"email\\\": \\\"%s\\\", \\\"name\\\": \\\"%s\\\"}}\" > %s 2>&1", sendgrid_key, to_email, to_name, cc_email, subject, escaped_content, from_email, from_name, reply_to_email, reply_to_name, NULL_DEVICE_m11);
	else
		sprintf(command, "curl.exe --request POST --url https://api.sendgrid.com/v3/mail/send --header \"authorization: Bearer %s\" --header \"content-type: application/json\" --data \"{\\\"personalizations\\\":[{\\\"to\\\": [{\\\"email\\\": \\\"%s\\\", \\\"name\\\": \\\"%s\\\"}], \\\"subject\\\": \\\"%s\\\"}], \\\"content\\\": [{\\\"type\\\": \\\"text/plain\\\", \\\"value\\\": \\\"%s\\\"}], \\\"from\\\": {\\\"email\\\": \\\"%s\\\", \\\"name\\\": \\\"%s\\\"}, \\\"reply_to\\\": {\\\"email\\\": \\\"%s\\\", \\\"name\\\": \\\"%s\\\"}}\" > %s 2>&1", sendgrid_key, to_email, to_name, subject, escaped_content, from_email, from_name, reply_to_email, reply_to_name, NULL_DEVICE_m11);
	win_system_m11(command);
#endif

	return;
}


void	set_L3_pw_d11(si1 *level_3_password)
{
	if (globals_d11->sk_matrix == NULL)
		initialize_sk_matrix_d11();

	memcpy(level_3_password, globals_d11->sk_matrix + ENCRYPTION_KEY_BYTES_m11, PASSWORD_BYTES_m11);
	level_3_password[PASSWORD_BYTES_m11] = 0;
	
	return;
}


#ifdef LINUX_m11
TERN_m11    set_thread_affinity_d11(pthread_t_d11 *thread_id_p, pthread_attr_t_d11 *attributes, cpu_set_t_d11 *cpu_set_p, TERN_m11 wait_for_lauch)
{
	const si4	MAX_ATTEMPTS = 100;
	TERN_m11	use_attributes;
	si1		thread_name[THREAD_NAME_BYTES_d11];
	si4		err, attempts;
	pthread_t	thread_id;
	
	
	// if thread_id_p is passed, it is used
	// if thread_id_p is NULL & attributes is passed, it is used
	// attributes allow affinity to be set befor thread is launched
	// thread_id can be used to change affinity whie thread is running
	
	if (thread_id_p == NULL) {
		if (attributes == NULL)
			return(FALSE_m11);
		err = pthread_attr_setaffinity_np((pthread_attr_t *) attributes, sizeof(cpu_set_t), cpu_set_p);  // _np is for "not portable"
		use_attributes = TRUE_m11;
	} else {
		thread_id = *((pthread_t *) thread_id_p);
		err = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), cpu_set_p);  // _np is for "not portable"
		use_attributes = FALSE_m11;
	}
	
	if (wait_for_lauch == TRUE_m11) {
		for (attempts = MAX_ATTEMPTS; err == ESRCH && attempts--;) {  // ESRCH == "thread not found" => threads can take a beat to launch
			nap_d11("10 ms");
			if (use_attributes == TRUE_m11)
				err = pthread_attr_setaffinity_np((pthread_attr_t *) attributes, sizeof(cpu_set_t), cpu_set_p);  // _np is for "not portable"
			else
				err = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), cpu_set_p);  // _np is for "not portable"
		}
	}
	
	if (err) {
		*thread_name = 0;
		if (thread_id_p != NULL)
			pthread_getname_np(thread_id, thread_name, (size_t) THREAD_NAME_BYTES_d11);  // _np is for "not portable"
		if (*thread_name)
			warning_message_m11("%s(): error setting affinity for thread \"%s\" => not set", __FUNCTION__, thread_name);
		else
			warning_message_m11("%s(): error setting thread affinity => not set", __FUNCTION__);
		return(FALSE_m11);
	}
	
	return(TRUE_m11);
}
#endif  // LINUX_m11


#ifdef WINDOWS_m11
TERN_m11    set_thread_affinity_d11(pthread_t_d11 *thread_handle_p, pthread_attr_t_d11 *attributes, cpu_set_t_d11 *cpu_set_p, TERN_m11 wait_for_lauch)
{
	const si4	MAX_ATTEMPTS = 100;
	si1		thread_name[THREAD_NAME_BYTES_d11];
	wchar_t		*w_thread_name;
	si4		err, attempts;
	HANDLE		thread_h;
	HRESULT		hr;
	
	
	// no correlate of attributes in Windows (argument ignored)
	
	thread_h = *((HANDLE *) thread_handle_p);
	err = SetThreadAffinityMask(thread_h, (DWORD_PTR) *cpu_set_p);  // Note Windows uses DWORD_PTR to ensure a ui8 - not used as pointer to ui4
	
	if (wait_for_lauch == TRUE_m11) {
		for (attempts = MAX_ATTEMPTS; err == 0 && attempts--;) {  // zero == unspecified error => can take a bit to launch
			nap_d11("10 ms");
			err = SetThreadAffinityMask(thread_h, (DWORD_PTR) *cpu_set_p);  // Note Windows uses DWORD_PTR to ensure a ui8 - not used as pointer to ui4
		}
	}
    
	if (err == 0) {
		*thread_name = 0;
		hr = GetThreadDescription(thread_h, (PWSTR *) &w_thread_name);
		if (SUCCEEDED(hr)) {
			wchar2char_m11(thread_name, w_thread_name);
			free((void *) w_thread_name);
		}
		if (*thread_name)
			warning_message_m11("%s(): error setting affinity for thread \"%s\" => not set", __FUNCTION__, thread_name);
		else
			warning_message_m11("%s(): error setting thread affinity => not set", __FUNCTION__);
		return(FALSE_m11);
	}

	return(TRUE_m11);
}
#endif  // WINDOWS_m11


#ifdef MACOS_m11
TERN_m11    set_thread_affinity_d11(pthread_t_d11 *thread_id_p, pthread_attr_t_d11 *attributes, cpu_set_t_d11 *cpu_set_p, TERN_m11 wait_for_lauch)
{
	// setting processor affinities can be done in MacOS but takes some work (see http://www.hybridkernel.com/2015/01/18/binding_threads_to_cores_osx.html)
	// this site suggests that it must be one core to one thread, not a range, which could be ok, but probably better to do some more reading
	
	*cpu_set_p = 0xFFFFFFFE;
	
	return(TRUE_m11);
}
#endif


void    show_alloc_entities_d11(ui1 mode)
{
	TERN_m11		show;
	si4                     i;
	ALLOC_ENTITY_d11        *ae;
	
	
	if (globals_d11->alloc_tracking == FALSE_m11) {
		warning_message_m11("%s(): Memory allocation tracking is not enabled", __FUNCTION__);
		return;
	}

	ae = globals_d11->alloc_entities;
	for (i = 0; i < globals_d11->ae_n_entities; ++i, ++ae) {
		show = TRUE_m11;
		switch (mode) {
			case AE_SHOW_ALL_d11:
				break;
			case AE_SHOW_FREED_ONLY_d11:
				if (ae->freed == FALSE_m11)
					show = FALSE_m11;
				break;
			case AE_SHOW_ALLOCATED_ONLY_d11:
				if (ae->freed == TRUE_m11)
					show = FALSE_m11;
				break;
		}
		if (show == TRUE_m11)
			printf_m11("%d) location: %lu, bytes: %lu, freed: %hhd   [%s(), line %d]\n", i, (ui8) ae->ptr, ae->n_bytes, ae->freed, ae->function, ae->line);
	}
	printf_m11("allocated bytes: %ld (max: %ld), allocated entities = %ld\n\n", globals_d11->ae_curr_allocated_bytes, globals_d11->ae_max_allocated_bytes, globals_d11->ae_curr_allocated_entities);
	
	return;
}


si4    show_alloc_entity_d11(void *ptr)
{
	si1			*freed_str = "unknown";
	si4                     i;
	ALLOC_ENTITY_d11        *ae;
	
	
	if (globals_d11->alloc_tracking == FALSE_m11) {
		warning_message_m11("%s(): Memory allocation tracking is not enabled", __FUNCTION__);
		return(-1);
	}
	
	ae = globals_d11->alloc_entities + globals_d11->ae_n_entities - 1;
	for (i = 0; i < globals_d11->ae_n_entities; ++i, --ae)
		if (ae->ptr == ptr)
			break;
	if (i == globals_d11->ae_n_entities) {
		warning_message_m11("%s(): object at location %lu is not allocated", __FUNCTION__, (ui8) ptr);
		return(-1);
	}
	if (ae->freed == TRUE_m11)
		freed_str = "yes";
	else if (ae->freed == FALSE_m11)
		freed_str = "no";

	printf_m11("location: %lu, bytes: %lu, freed: %s   [%s(), line %d]\n", (ui8) ae->ptr, ae->n_bytes, freed_str, ae->function, ae->line);

	return(0);
}


void	show_cpu_info_d11(void)
{
	CPU_INFO_d11	*cpu_info;
	
	
	if (globals_d11->cpu_info.logical_cores == 0)
		get_cpu_info_d11();
	cpu_info = &globals_d11->cpu_info;
	
	printf_m11("logical_cores = %d\n", cpu_info->logical_cores);
	if (cpu_info->physical_cores == 0)
		printf_m11("physical_cores = unknown\n");
	else
		printf_m11("physical_cores = %d\n", cpu_info->physical_cores);
	printf_m11("hyperthreading = ");
	switch (cpu_info->hyperthreading) {
		case FALSE_m11:
			printf_m11("false\n");
			break;
		case TRUE_m11:
			printf_m11("true\n");
			break;
		case UNKNOWN_m11:
			printf_m11("unknown\n");
			break;
		default:
			printf_m11("invalid value (%hhd)\n", cpu_info->hyperthreading);
			break;
	}
	printf_m11("endianness = ");
	switch (cpu_info->endianness) {
		case BIG_ENDIAN_m11:
			printf_m11("big endian\n");
			break;
		case LITTLE_ENDIAN_m11:
			printf_m11("little endian\n");
			break;
		default:
			printf_m11("invalid value (%hhu)\n", cpu_info->endianness);
			break;
	}

	return;
}


void    show_globals_d11(void)
{
	printf_m11("DHN Globals\n");
	printf_m11("-----------\n\n");
	printf_m11("file_creation_umask: %u\n", globals_d11->file_creation_umask);
	show_cpu_info_d11();
	printf_m11("machine_serial: %s\n", globals_d11->machine_serial);
	printf_m11("LS_machine_code: %04x\n", globals_d11->LS_machine_code);
	printf_m11("LS_customer_code: %u\n", globals_d11->LS_customer_code);
	printf_m11("all_structures_aligned: %hhd\n", globals_d11->all_structures_aligned);
	printf_m11("license_file_entry_aligned: %hhd\n", globals_d11->license_file_entry_aligned);
	printf_m11("transmission_header_aligned: %hhd\n", globals_d11->transmission_header_aligned);
	printf_m11("alloc_tracking: %hhd\n", globals_d11->alloc_tracking);
	printf_m11("verbose: %hhd\n\n", globals_d11->verbose);

	return;
}


TERN_m11	show_message_d11(TRANSMISSION_HEADER_d11 *header)
{
	ui1			type;
	si1			*msg;
	MESSAGE_HEADER_d11	*msg_header;
	

	type = header->type;
	
	switch (type) {
		case TH_TYPE_OPERATION_SUCCEEDED_d11:
			message_m11("%s(): operation succeeded", __FUNCTION__);
			return(TRUE_m11);
		case TH_TYPE_OPERATION_FAILED_d11:
			warning_message_m11("%s(): operation failed", __FUNCTION__);
			return(TRUE_m11);
		case TH_TYPE_OPERATION_SUCCEEDED_WITH_MESSAGE_d11:
		case TH_TYPE_MESSAGE_d11:
		case TH_TYPE_OPERATION_FAILED_WITH_WARNING_MESSAGE_d11:
		case TH_TYPE_OPERATION_FAILED_WITH_ERROR_MESSAGE_d11:
			msg_header = (MESSAGE_HEADER_d11 *) (header + 1);
			msg = (si1 *) (msg_header + 1);
			break;
		default:
			return(FALSE_m11);
	}

	switch (type) {
		case TH_TYPE_OPERATION_SUCCEEDED_WITH_MESSAGE_d11:
		case TH_TYPE_MESSAGE_d11:
			message_m11("%s", msg);
			return(TRUE_m11);
		case TH_TYPE_OPERATION_FAILED_WITH_WARNING_MESSAGE_d11:
			warning_message_m11("%s", msg);
			return(TRUE_m11);
		case TH_TYPE_OPERATION_FAILED_WITH_ERROR_MESSAGE_d11:
			error_message_m11("%s", msg);
			return(TRUE_m11);
	}
	
	return(FALSE_m11);  // never gets here - just to shut compiler up
}


#if defined MACOS_m11 || defined LINUX_m11
void    show_network_parameters_d11(NETWORK_PARAMETERS_d11 *np)
{
	si1        hex_str[HEX_STRING_BYTES_m11(MAC_ADDRESS_BYTES_d11)];
	
	
	printf_m11("host_name: %s\n", np->host_name);
	generate_hex_string_m11(np->MAC_address_bytes, MAC_ADDRESS_BYTES_d11, hex_str);
	printf_m11("MAC_address_bytes: %s\n", hex_str);
	printf_m11("MAC_address_string_LC: %s\n", np->MAC_address_string_LC);
	printf_m11("MAC_address_string_UC: %s\n", np->MAC_address_string_UC);
	generate_hex_string_m11(np->WAN_IPv4_address_bytes, IPV4_ADDRESS_BYTES_d11, hex_str);
	printf_m11("WAN_IPv4_address_bytes: %s\n", hex_str);
	printf_m11("WAN_IPv4_address_string: %s\n", np->WAN_IPv4_address_string);
	generate_hex_string_m11(np->LAN_IPv4_address_bytes, IPV4_ADDRESS_BYTES_d11, hex_str);
	printf_m11("LAN_IPv4_address_bytes: %s\n", hex_str);
	printf_m11("LAN_IPv4_address_string: %s\n", np->LAN_IPv4_address_string);
	generate_hex_string_m11(np->LAN_IPv4_subnet_mask_bytes, IPV4_ADDRESS_BYTES_d11, hex_str);
	printf_m11("LAN_IPv4_subnet_mask_bytes: %s\n", hex_str);
	printf_m11("LAN_IPv4_subnet_mask_string: %s\n", np->LAN_IPv4_subnet_mask_string);
	printf_m11("MTU: %d\n", np->MTU);
	printf_m11("link speed: %s\n", np->link_speed);
	if (np->active == TRUE_m11)
		printf_m11("active: true\n");
	else if (np->active == FALSE_m11)
		printf_m11("active: false\n");
	else
		printf_m11("active: unknown (%hhd)\n", np->plugged_in);
#ifdef LINUX_m11
	printf_m11("duplex: %s\n", np->duplex);
	if (np->plugged_in == TRUE_m11)
		printf_m11("plugged_in: true\n");
	else if (np->plugged_in == FALSE_m11)
		printf_m11("plugged_in: false\n");
	else
		printf_m11("plugged_in: unknown (%hhd)\n", np->plugged_in);
#endif

	return;
}
#endif  // MACOS_m11 || LINUX_m11


#ifdef WINDOWS_m11
void    show_network_parameters_d11(NETWORK_PARAMETERS_d11* np)
{	
	return;
}
#endif  // WINDOWS_m11


#ifdef MACOS_m11
void    show_thread_affinity_d11(pthread_t_d11 *thread_id_p)
{
	return;  // thread affinity can be done in MacOS, but it takes some work
}
#endif  // MACOS_m11


#ifdef LINUX_m11
void    show_thread_affinity_d11(pthread_t_d11 *thread_id_p)
{
	si1		thread_name[THREAD_NAME_BYTES_d11];
	si4             i, n_cpus;
	cpu_set_t       cpu_set;
	pthread_t 	thread_id;
    
	
	thread_id = *thread_id_p;
	if (globals_d11->cpu_info.logical_cores == 0)
		get_cpu_info_d11();

	*thread_name = 0;
	pthread_getname_np(thread_id, thread_name, (size_t) THREAD_NAME_BYTES_d11);  // _np is for "not portable"
	if (*thread_name)
		printf_m11("thread \"%s()\": ", thread_name);
	
	pthread_getaffinity_np(thread_id, sizeof(cpu_set_t), &cpu_set);  // _np is for "not portable"
	
	n_cpus = globals_d11->cpu_info.logical_cores;
	for (i = 0; i < n_cpus; ++i) {
		if (CPU_ISSET(i, &cpu_set))
			printf_m11("1 ");
		else
			printf_m11("0 ");
	}
	printf_m11("\n\n");

	return;
}
#endif  // LINUX_m11


#ifdef WINDOWS_m11
void    show_thread_affinity_d11(pthread_t_d11 *thread_handle_p)
{
	si1		thread_name[THREAD_NAME_BYTES_d11];
	wchar_t		*w_thread_name;
	si4             i, n_cpus;
	cpu_set_t_d11	cpu_set, tmp_cpu_set, mask;
	HANDLE 		thread_h;
	HRESULT		hr;
    
	
	thread_h = *thread_handle_p;
	if (globals_d11->cpu_info.logical_cores == 0)
		get_cpu_info_d11();

	SuspendThread(thread_h);  // suspend thread to get current cpu set
	tmp_cpu_set = ~((cpu_set_t_d11) 0);
	cpu_set = (cpu_set_t_d11) SetThreadAffinityMask(thread_h, (DWORD_PTR) tmp_cpu_set);  // use SetThreadAffinityMask() to return existing cpu set
	tmp_cpu_set = (cpu_set_t_d11) SetThreadAffinityMask(thread_h, (DWORD_PTR) cpu_set);  // reset to existing cpu set
	ResumeThread(thread_h);

	if (tmp_cpu_set == 0) {
		warning_message_m11("%s(): error %d from SetThreadAffinityMask()\n", __FUNCTION__, GetLastError());
		return;
	}

	n_cpus = globals_d11->cpu_info.logical_cores;
	*thread_name = 0;
	hr = GetThreadDescription(thread_h, (PWSTR *) &w_thread_name);
	if (SUCCEEDED(hr)) {
		wchar2char_m11(thread_name, w_thread_name);
		free((void *) w_thread_name);
	}
	if (*thread_name)
		printf_m11("thread \"%s()\": ", thread_name);
	else
		printf_m11("thread: ");
	
	for (mask = 1, i = 0; i < n_cpus; ++i, mask <<= 1) {
		if (cpu_set & mask)
			printf_m11("1 ");
		else
			printf_m11("0 ");
	}
	printf_m11("\n\n");

	return;
}
#endif  // WINDOWS
	
	
void	show_transmission_info_d11(TRANSMISSION_INFO_d11 *trans_info)
{
	si1				hex_str[HEX_STRING_BYTES_m11(sizeof(ui4))];
	TRANSMISSION_HEADER_d11		*header;
		

	// transmission info
	printf_m11("-------------- Transmission Info - START ------------\n");
	if (trans_info->buffer == NULL)
		printf_m11("Buffer: NULL\n");
	else
		printf_m11("Buffer: allocated\n");
	
	printf_m11("Buffer Bytes: %ld\n", trans_info->buffer_bytes);
	printf_m11("Socket File Descriptor: %d\n", trans_info->sock_fd);
	if (*trans_info->addr_str == 0)
		printf_m11("Socket Address: not set\n");
	else
		printf_m11("Socket Address: %s\n", trans_info->addr_str);
	
	if (*trans_info->port_str == 0)
		printf_m11("Port: not set\n");
	else
		printf_m11("Port: %s\n", trans_info->port_str);
	printf_m11("--------------- Transmission Info - END -------------\n");

	// header
	if (trans_info->buffer == NULL)
		return;
	printf_m11("-------------- Transmission Header - START ------------\n");
	header = trans_info->header;
	if (header->ID_code == TH_ID_CODE_NO_ENTRY_d11) {
		printf_m11("ID String: no entry\n");
	} else {
		generate_hex_string_m11((ui1 *) header->ID_string, sizeof(ui4), hex_str);
		printf_m11("ID String: %s (%s)\n", header->ID_string, hex_str);
	}
	if (header->type == TH_TYPE_NO_ENTRY_d11)
		printf_m11("Type: %hhu (no entry)\n", header->type);
	else
		printf_m11("Type: %hhu\n", header->type);
	if (header->version == TH_VERSION_NO_ENTRY_d11)
		printf_m11("Version: %hhu (no entry)\n", header->version);
	else
		printf_m11("Version: %hhu\n", header->version);
	if (header->flags == TH_FLAGS_NO_ENTRY_d11)
		printf_m11("Flags: %hhu (no entry)\n", header->flags);
	else
		printf_m11("Flags: %hhu\n", header->flags);
	if (header->transmission_bytes == TH_TRANSMISSION_BYTES_NO_ENTRY_d11)
		printf_m11("Transmission Bytes: %ld (no entry)\n", header->transmission_bytes);
	else
		printf_m11("Transmission Bytes: %ld\n", header->transmission_bytes);
	printf_m11("--------------- Transmission Header - END -------------\n");

	// body
	switch (header->ID_code) {
		case TH_ID_CODE_NO_ENTRY_d11:
			break;
		case LS_TH_ID_CODE_d11:
			break;
		default:
			warning_message_m11("%s(): unrecognized transmission ID code");
			break;
	}
	return;
}


//***********************************************************************//
//**********************  END GENERAL DHN FUNCTIONS  ********************//
//***********************************************************************//


si1     *size_string_d11(si1 *size_str, si8 n_bytes)
{
	static si1              private_size_str[SIZE_STRING_BYTES_m11];
	static const si1        units[6][8] = {"bytes", "kB", "MB", "GB", "TB", "PB"};
	ui8                     i, j, t;
	sf8                     size;
	
	
	// Note: if size_str == NULL, this function is not thread safe
	if (size_str == NULL)
		size_str = private_size_str;
	
	for (i = 0, j = 1, t = n_bytes; t >>= 10; ++i, j <<= 10);
	size = (sf8) n_bytes / (sf8) j;

	sprintf_m11(size_str, "%0.2lf %s", size, units[i]);
	
	return(size_str);
}


void    textbelt_text_d11(si1 *phone_number, si1 *content, si1 *textbelt_key)
{
	si1     command[1024];
	
	
#if defined MACOS_m11 || defined LINUX_m11
	sprintf(command, "curl -X POST https://textbelt.com/text --data-urlencode phone='%s' --data-urlencode message='%s' -d key=%s > %s 2>&1", phone_number, content, textbelt_key, NULL_DEVICE_m11);
	system(command);
#endif
#ifdef WINDOWS_m11
	sprintf(command, "curl.exe -X POST https://textbelt.com/text --data-urlencode phone=\"%s\" --data-urlencode message=\"%s\" -d key=%s > %s 2>&1", phone_number, content, textbelt_key, NULL_DEVICE_m11);
	win_system_m11(command);
#endif

	return;
}


void	trim_addr_str_d11(si1 *addr_str)
{
	size_t	len;
	
	
	if (strncmp(addr_str, "::ffff:", 7) == 0) {
		len = strlen(addr_str);
		memmove(addr_str, addr_str + 7, len - 6);
	}
	
	return;
}


