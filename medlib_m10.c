
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

// The library is written with tab width = indent width = 8 spaces and a monospaced font.
// Set your editor preferences to these for intended alignment.

// All functions, constants, macros, and data types defined in the library are tagged
// with the suffix "_m10" (for "MED 1.0"). This is to facilitate using multiple versions
// of the library in concert in the future; for example to write a MED 1.0 to MED 2.0 converter.



#include "medlib_m10.h"
#include "medrec_m10.h"

// Globals
GLOBALS_m10     *globals_m10 = NULL;



//***********************************************************************//
//************************  MED LIBRARY FUNCTIONS  **********************//
//***********************************************************************//


si8     absolute_index_to_time_m10(si1 *seg_dir, si8 index, si8 absolute_start_sample_number, sf8 sampling_frequency, ui1 mode)
{
        si1                                     path[FULL_FILE_NAME_BYTES_m10], seg_name[SEGMENT_BASE_FILE_NAME_BYTES_m10];
        si8                                     uutc;
        FILE_PROCESSING_STRUCT_m10              *fps;
        TIME_SERIES_METADATA_SECTION_2_m10      *tmd2;
        
        
        extract_path_parts_m10(seg_dir, NULL, seg_name, NULL);
        
        // get absolute start sample number and sampling frequency from metadata
        if (absolute_start_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10 || sampling_frequency == FREQUENCY_NO_ENTRY_m10) {
                sprintf(path, "%s/%s.%s", seg_dir, seg_name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
                fps = read_file_m10(NULL, path, 1, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
                tmd2 = fps->metadata.time_series_section_2;
                absolute_start_sample_number = tmd2->absolute_start_sample_number;
                sampling_frequency = tmd2->sampling_frequency;
                free_file_processing_struct_m10(fps, FALSE_m10);
        }
        
        // read in time series indices
        sprintf(path, "%s/%s.%s", seg_dir, seg_name, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10);
        fps = read_file_m10(NULL, path, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
        
        uutc = uutc_for_sample_number_m10(absolute_start_sample_number, UUTC_NO_ENTRY_m10, index, sampling_frequency, fps, mode);
        
        free_file_processing_struct_m10(fps, FALSE_m10);
        
        return(uutc);
        
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
	
	
	for (i = 0; i < 4; i++) {
		for (j = 0;j < 4; j++) {
			state[j][i] ^= round_key[round * AES_NB_m10 * 4 + i * AES_NB_m10 + j];
		}
	}

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
	
	
	if (expanded_key != NULL) {
		AES_inv_cipher_m10(in, out, state, expanded_key);
	} else if (password != NULL) {
		// password becomes the key (16 bytes, zero-padded if shorter, truncated if longer)
		strncpy_m10((si1 *) key, password, 16);
		
		//The Key-Expansion routine must be called before the decryption routine.
		AES_key_expansion_m10(round_key, key);
		
		// The next function call decrypts the CipherText with the Key using AES algorithm.
		AES_inv_cipher_m10(in, out, state, round_key);
	} else {
		fprintf(stderr, "Error: No password or expanded key => exiting [function \"%s\", line %d]\n", __FUNCTION__, __LINE__);
		exit(-1);
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
	
	
	if (expanded_key != NULL) {
		AES_cipher_m10(in, out, state, expanded_key);
	} else if (password != NULL) {
		// password becomes the key (16 bytes, zero-padded if shorter, truncated if longer)
		strncpy_m10((si1 *) key, password, 16);
		
		// The KeyExpansion routine must be called before encryption.
		AES_key_expansion_m10(round_key, key);
		
		// The next function call encrypts the PlainText with the Key using AES algorithm.
		AES_cipher_m10(in, out, state, round_key);
	} else {
		fprintf(stderr, "Error: No password or expanded key => exiting [function \"%s\", line %d]\n", __FUNCTION__, __LINE__);
		exit(-1);
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
		AES_initialize_rcon_table_m10();
	
	// The first round key is the key itself.
	for (i = 0; i < AES_NK_m10; i++) {
		expanded_key[i * 4] = key[i * 4];
		expanded_key[i * 4 + 1] = key[i * 4 + 1];
		expanded_key[i * 4 + 2] = key[i * 4 + 2];
		expanded_key[i * 4 + 3] = key[i * 4 + 3];
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
			temp[0] = AES_get_sbox_value_m10(temp[0]);
			temp[1] = AES_get_sbox_value_m10(temp[1]);
			temp[2] = AES_get_sbox_value_m10(temp[2]);
			temp[3] = AES_get_sbox_value_m10(temp[3]);
			
			temp[0] = temp[0] ^ globals_m10->AES_rcon_table[i / AES_NK_m10];
		} else if (AES_NK_m10 > 6 && i % AES_NK_m10 == 4) {
			// This takes a four-byte input word and applies the S-box
			// to each of the four bytes to produce an output word.
			temp[0] = AES_get_sbox_value_m10(temp[0]);
			temp[1] = AES_get_sbox_value_m10(temp[1]);
			temp[2] = AES_get_sbox_value_m10(temp[2]);
			temp[3] = AES_get_sbox_value_m10(temp[3]);
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
		AES_initialize_rsbox_table_m10();
	
	return(globals_m10->AES_rsbox_table[num]);
}


inline si4	AES_get_sbox_value_m10(si4 num)
{
	if (globals_m10->AES_sbox_table == NULL)
		AES_initialize_sbox_table_m10();
	
	return(globals_m10->AES_sbox_table[num]);
}


void	AES_initialize_rcon_table_m10(void)
{
	si4	*rcon_table;
	
	
	rcon_table = (si4 *) e_calloc_m10((size_t) AES_RCON_ENTRIES_m10, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	{
		si4 temp[AES_RCON_ENTRIES_m10] = AES_RCON_m10;
		memcpy(rcon_table, temp, AES_RCON_ENTRIES_m10 * sizeof(si4));
	}
	
	globals_m10->AES_rcon_table = rcon_table;
	
	return;
}


void	AES_initialize_rsbox_table_m10(void)
{
	si4	*rsbox_table;
	
	
	rsbox_table = (si4 *) e_calloc_m10((size_t) AES_RSBOX_ENTRIES_m10, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	{
		si4 temp[AES_RSBOX_ENTRIES_m10] = AES_RSBOX_m10;
		memcpy(rsbox_table, temp, AES_RSBOX_ENTRIES_m10 * sizeof(si4));
	}
	
	globals_m10->AES_rsbox_table = rsbox_table;
	
	return;
}


void	AES_initialize_sbox_table_m10(void)
{
	si4	*sbox_table;
	
	
	sbox_table = (si4 *) e_calloc_m10((size_t) AES_SBOX_ENTRIES_m10, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	{
		si4 temp[AES_SBOX_ENTRIES_m10] = AES_SBOX_m10;
		memcpy(sbox_table, temp, AES_SBOX_ENTRIES_m10 * sizeof(si4));
	}
	
	globals_m10->AES_sbox_table = sbox_table;
        
	return;
}


// AES_inv_cipher is the main decryption function
void	AES_inv_cipher_m10(ui1 *in, ui1 *out, ui1 state[][4], ui1 *round_key)
{
	si4	i, j, round = 0;
	
	
	// Copy the input encrypted text to state array.
	for (i = 0; i < 4; i++) {
		for (j = 0;j < 4; j++) {
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
			state[i][j] = AES_get_sbox_invert_m10(state[i][j]);
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
			state[i][j] = AES_get_sbox_value_m10(state[i][j]);
		}
	}
        
	return;
}


//***********************************************************************//
//************************  End AES-128 FUNCTIONS  **********************//
//***********************************************************************//


si1	all_zeros_m10(ui1 *bytes, si4 field_length)
{
	while (field_length--)
		if (*bytes++)
			return(FALSE_m10);
	
	return(TRUE_m10);
}


CHANNEL_m10     *allocate_channel_m10(CHANNEL_m10 *chan, FILE_PROCESSING_STRUCT_m10 *proto_fps, si1 *enclosing_path, si1 *chan_name, ui4 type_code, si4 n_segs, TERN_m10 chan_recs, TERN_m10 seg_recs)
{
        si1                             *type_str;
        si8                             i;
        UNIVERSAL_HEADER_m10            *uh;
        FILE_PROCESSING_STRUCT_m10      *gen_fps;
        
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
                chan = (CHANNEL_m10 *) e_calloc_m10((size_t) 1, sizeof(CHANNEL_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
        } else {
                chan->record_data_fps = chan->record_indices_fps = NULL;
        }
        
        if (n_segs) {
                chan->segments = (SEGMENT_m10 **) e_calloc_2D_m10((size_t) n_segs, 1, sizeof(SEGMENT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                for (i = 0; i < n_segs; ++i)
                        allocate_segment_m10(chan->segments[i], gen_fps, chan->path, chan->name, type_code, i + 1, seg_recs);
        }

        return(chan);
}


FILE_PROCESSING_STRUCT_m10	*allocate_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps, si1 *full_file_name, ui4 type_code, si8 raw_data_bytes, FILE_PROCESSING_STRUCT_m10 *proto_fps, si8 bytes_to_copy)
{
        UNIVERSAL_HEADER_m10	*uh;
        ui1			*data_ptr;
	
	
	// allocate FPS
	if (fps == NULL) {
                fps = (FILE_PROCESSING_STRUCT_m10 *) e_calloc_m10((size_t) 1, sizeof(FILE_PROCESSING_STRUCT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	} else {
		if (fps->raw_data != NULL)
			e_free_m10((void *) fps->raw_data, __FUNCTION__, __LINE__);
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
	fps->raw_data = (ui1 *) e_calloc_m10((size_t) raw_data_bytes, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	uh = fps->universal_header = (UNIVERSAL_HEADER_m10 *) fps->raw_data;
        fps->fd = FPS_FD_NO_ENTRY_m10;
        fps->file_length = 0; // nothing read or written yet
	
        // if a prototype FILE_PROCESSING_STRUCT_m10 was passed - copy its directives, password data, universal_header, and raw data
        if (proto_fps != NULL) {
		fps->directives = proto_fps->directives;
                bytes_to_copy += UNIVERSAL_HEADER_BYTES_m10;  // all files start with universal header
                if ((bytes_to_copy > proto_fps->raw_data_bytes) || (bytes_to_copy > fps->raw_data_bytes))
                        error_message_m10("%s(): copy request size exceeds avaiable data => no copying done", __FUNCTION__);
                else
                	memcpy(fps->raw_data, proto_fps->raw_data, bytes_to_copy);
                uh->type_code = type_code;
                uh->header_CRC = uh->body_CRC = CRC_START_VALUE_m10;
                uh->number_of_entries = 0;
                uh->maximum_entry_size = 0;
        } else {
                (void) initialize_file_processing_directives_m10(&fps->directives);  // set directives to defaults
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
			fps->metadata.section_1 = (METADATA_SECTION_1_m10 *) (fps->metadata.metadata = data_ptr);
                        fps->metadata.time_series_section_2 = (TIME_SERIES_METADATA_SECTION_2_m10 *) (fps->raw_data + METADATA_SECTION_2_OFFSET_m10);
			fps->metadata.video_section_2 = (VIDEO_METADATA_SECTION_2_m10 *) fps->metadata.time_series_section_2;
			fps->metadata.section_3 = (METADATA_SECTION_3_m10 *) (fps->raw_data + METADATA_SECTION_3_OFFSET_m10);
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
                        error_message_m10("%s(): unrecognized type code (code = 0x%4x)", type_code, __FUNCTION__);
                        return(NULL);
        }

        return(fps);
}


METADATA_m10    *allocate_metadata_m10(METADATA_m10 *metadata, ui1 *data_ptr)
{
        // Note that memory is allocated to metadata->metadata, and caller must free that before freeing structure
        if (metadata == NULL)
                metadata = (METADATA_m10 *) e_calloc_m10((size_t) 1, sizeof(METADATA_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        if (data_ptr == NULL)
                data_ptr = (ui1 *) e_calloc_m10((size_t) METADATA_BYTES_m10, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
        metadata->section_1 = (METADATA_SECTION_1_m10 *) data_ptr;
        metadata->time_series_section_2 = (TIME_SERIES_METADATA_SECTION_2_m10 *) (data_ptr + METADATA_SECTION_1_BYTES_m10);
        metadata->video_section_2 = (VIDEO_METADATA_SECTION_2_m10 *) metadata->time_series_section_2;
        metadata->section_3 = (METADATA_SECTION_3_m10 *) (data_ptr + METADATA_SECTION_1_BYTES_m10 + METADATA_SECTION_2_BYTES_m10);
        
        return(metadata);
}


SEGMENT_m10     *allocate_segment_m10(SEGMENT_m10 *seg, FILE_PROCESSING_STRUCT_m10 *proto_fps, si1 *enclosing_path, si1 *chan_name, ui4 type_code, si4 seg_num, TERN_m10 seg_recs)
{
        si1                     num_str[FILE_NUMBERING_DIGITS_m10 + 1];
        UNIVERSAL_HEADER_m10    *uh;
        
        // enclosing_path is the path to the enclosing directory
        // chan_name is the base name, with no extension
        // if time series channels are requested, the CMP_PROCESSING_STRUCT_m10 structures must be allocated seperately.
        // if time series segments are requested, enough memory for one time series index is allocated.
        // if records are requested, enough memory for 1 record of size LARGEST_RECORD_BYTES_m10 is allocated (use reallocate_file_processing_struct_m10() to change this)
        // if records are requested, enough memory for 1 record index is allocated (reallocate_file_processing_struct_m10() to change this)

        if (seg == NULL)
                seg = (SEGMENT_m10 *) e_calloc_m10((size_t) 1, sizeof(SEGMENT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
        } else {
                seg->record_data_fps = seg->record_indices_fps = NULL;
        }
        seg->segmented_session_record_data_fps = seg->segmented_session_record_indices_fps = NULL;  // these have to set in session after segment allocated
  
        return(seg);
}


SESSION_m10     *allocate_session_m10(FILE_PROCESSING_STRUCT_m10 *proto_fps, si1 *enclosing_path, si1 *sess_name, si4 n_ts_chans, si4 n_vid_chans, si4 n_segs, si1 **ts_chan_names, si1 **vid_chan_names, TERN_m10 sess_recs, TERN_m10 segmented_sess_recs, TERN_m10 chan_recs, TERN_m10 seg_recs)
{
        si1                             free_names, number_str[FILE_NUMBERING_DIGITS_m10 + 1];
        si8                             i, j;
        SESSION_m10                     *sess;
        SEGMENT_m10                     *seg;
        UNIVERSAL_HEADER_m10            *uh;
        FILE_PROCESSING_STRUCT_m10      *gen_fps;
        
        // enclosing_path is the path to the enclosing directory
        // sess_name is the base name, with no extension
        // if records are requested, enough memory for 1 record of data size LARGEST_RECORD_BYTES_m10 is allocated (reallocate_file_processing_struct_m10() to change this)
        // if records are requested, enough memory for 1 record index is allocated (reallocate_file_processing_struct_m10() to change this)
                   
        sess = (SESSION_m10 *) e_calloc_m10((size_t) 1, sizeof(SESSION_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
        } else if (n_vid_chans) {
                sess->video_metadata_fps = gen_fps;
                uh->type_code = VIDEO_METADATA_FILE_TYPE_CODE_m10;
        }

        if (sess_recs == TRUE_m10) {
                sess->record_data_fps = allocate_file_processing_struct_m10(NULL, NULL, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, gen_fps, 0);
                snprintf_m10(sess->record_data_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", sess->path, sess->name, RECORD_DATA_FILE_TYPE_STRING_m10);
                sess->record_indices_fps = allocate_file_processing_struct_m10(NULL, NULL, RECORD_INDICES_FILE_TYPE_CODE_m10, RECORD_INDEX_BYTES_m10, gen_fps, 0);
                snprintf_m10(sess->record_indices_fps->full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", sess->path, sess->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
        } else {
                sess->record_data_fps = sess->record_indices_fps = NULL;
        }
        
        if (n_ts_chans) {
                free_names = FALSE_m10;
                if (ts_chan_names == NULL) {
                        ts_chan_names = generate_numbered_names_m10(NULL, "tch_", n_ts_chans);
                        free_names = TRUE_m10;
                }
                sess->time_series_channels = (CHANNEL_m10 **) e_calloc_2D_m10((size_t) n_ts_chans, 1, sizeof(CHANNEL_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                for (i = 0; i < n_ts_chans; ++i) {
                        sess->time_series_metadata_fps->metadata.time_series_section_2->acquisition_channel_number = i + 1;
                        allocate_channel_m10(sess->time_series_channels[i], sess->time_series_metadata_fps, sess->path, ts_chan_names[i], TIME_SERIES_CHANNEL_TYPE_m10, n_segs, chan_recs, seg_recs);
                }
                if (free_names == TRUE_m10)
                        e_free_2D_m10((void **) ts_chan_names, n_ts_chans, __FUNCTION__, __LINE__);
        }
	
        if (n_vid_chans) {
                free_names = FALSE_m10;
                if (vid_chan_names == NULL) {
                        vid_chan_names = generate_numbered_names_m10(NULL, "vch_", n_vid_chans);
                        free_names = TRUE_m10;
                }
                sess->video_channels = (CHANNEL_m10 **) e_calloc_2D_m10((size_t) n_vid_chans, 1, sizeof(CHANNEL_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                for (i = 0; i < n_vid_chans; ++i) {
                        sess->video_metadata_fps->metadata.video_section_2->acquisition_channel_number = i + 1;
                        allocate_channel_m10(sess->video_channels[i], sess->video_metadata_fps, sess->path, vid_chan_names[i], VIDEO_CHANNEL_TYPE_m10, n_segs, chan_recs, seg_recs);
                }
                if (free_names == TRUE_m10)
                        e_free_2D_m10((void **) vid_chan_names, n_vid_chans, __FUNCTION__, __LINE__);
        }
        
        if (segmented_sess_recs == TRUE_m10) {
                sess->segmented_record_data_fps = (FILE_PROCESSING_STRUCT_m10 **) e_calloc_2D_m10(n_segs, 1, sizeof(FILE_PROCESSING_STRUCT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                sess->segmented_record_indices_fps = (FILE_PROCESSING_STRUCT_m10 **) e_calloc_2D_m10(n_segs, 1, sizeof(FILE_PROCESSING_STRUCT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                for (i = 0; i < n_segs; ++i) {
                        if (n_ts_chans)
                                gen_fps = sess->time_series_channels[0]->segments[i]->metadata_fps;
                        else if (n_vid_chans)
                                gen_fps = sess->video_channels[0]->segments[i]->metadata_fps;
                        numerical_fixed_width_string_m10(number_str, FILE_NUMBERING_DIGITS_m10, i + 1); // segments numbered from 1
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
        } else {
                sess->segmented_record_data_fps = sess->segmented_record_indices_fps = NULL;
        }

        return(sess);
}


inline void	apply_recording_time_offset_m10(si8 *time)
{
	if (*time != UUTC_NO_ENTRY_m10)
		*time -=  globals_m10->recording_time_offset;
	
	return;
}


void	calculate_metadata_CRC_m10(FILE_PROCESSING_STRUCT_m10 *fps)
{
        // no real need for this one-liner function - just exists for symmetry
        fps->universal_header->body_CRC = CRC_calculate_m10(fps->metadata.metadata, METADATA_BYTES_m10);

        return;
}


void    calculate_record_data_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_HEADER_m10 *record_header, si8 number_of_items)
{
        ui4     temp_CRC, full_record_CRC;
        si8     i;
        
	
        for (i = 0; i < number_of_items; ++i) {
                // record CRC
                record_header->record_CRC = CRC_calculate_m10((ui1 *) record_header + RECORD_HEADER_RECORD_CRC_START_OFFSET_m10, record_header->total_record_bytes - RECORD_HEADER_RECORD_CRC_START_OFFSET_m10);
                
                // update universal_header->body_CRC
                temp_CRC = CRC_calculate_m10((ui1 *) record_header, RECORD_HEADER_RECORD_CRC_START_OFFSET_m10);
                full_record_CRC = CRC_combine_m10(temp_CRC, record_header->record_CRC, record_header->total_record_bytes - RECORD_HEADER_RECORD_CRC_START_OFFSET_m10);
                fps->universal_header->body_CRC = CRC_combine_m10(fps->universal_header->body_CRC, full_record_CRC, record_header->total_record_bytes);
                
                record_header = (RECORD_HEADER_m10 *) ((ui1 *) record_header + record_header->total_record_bytes);
        }

	return;
}


void    calculate_record_indices_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, RECORD_INDEX_m10 *record_index, si8 number_of_items)
{
        si8     i;
        
	
        for (i = 0; i < number_of_items; ++i, ++record_index)
                fps->universal_header->body_CRC = CRC_update_m10((ui1 *) record_index, RECORD_INDEX_BYTES_m10, fps->universal_header->body_CRC);

        return;
}


void    calculate_time_series_data_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, CMP_BLOCK_FIXED_HEADER_m10 *block_header, si8 number_of_items)
{
        ui4     temp_CRC, full_block_CRC;
        si8     i;
        
	
        for (i = 0; i < number_of_items; ++i) {
                // block CRC
                block_header->block_CRC = CRC_calculate_m10((ui1 *) block_header + CMP_BLOCK_CRC_START_OFFSET_m10, block_header->total_block_bytes - CMP_BLOCK_CRC_START_OFFSET_m10);
                
                // update universal_header->file_CRC
                temp_CRC = CRC_calculate_m10((ui1 *) block_header, CMP_BLOCK_CRC_START_OFFSET_m10);
                full_block_CRC = CRC_combine_m10(temp_CRC, block_header->block_CRC, block_header->total_block_bytes - CMP_BLOCK_CRC_START_OFFSET_m10);
                fps->universal_header->body_CRC = CRC_combine_m10(fps->universal_header->body_CRC, full_block_CRC, block_header->total_block_bytes);
                
                block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *) block_header + block_header->total_block_bytes);
        }

	return;
}


void    calculate_time_series_indices_CRCs_m10(FILE_PROCESSING_STRUCT_m10 *fps, TIME_SERIES_INDEX_m10 *time_series_index, si8 number_of_items)
{
        si8     i;
        
	
        for (i = 0; i < number_of_items; ++i, ++time_series_index)
                fps->universal_header->body_CRC = CRC_update_m10((ui1 *) time_series_index, TIME_SERIES_INDEX_BYTES_m10, fps->universal_header->body_CRC);

        return;
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


//***********************************************************************//
//********************  ALIGNMENT CHECKING FUNCTIONS  *******************//
//***********************************************************************//


TERN_m10        check_all_alignments_m10(const si1 *function, si4 line)
{
	TERN_m10        return_value;
	ui1		*bytes;
	
        
	// see if already checked
	if (globals_m10->all_structures_aligned != UNKNOWN_m10)
		return(globals_m10->all_structures_aligned);
        
	return_value = TRUE_m10;
	bytes = (ui1 *) e_malloc_m10(METADATA_FILE_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);  // METADATA is largest file structure
	
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
	
	e_free_m10(bytes, __FUNCTION__, __LINE__);

	if (return_value == TRUE_m10) {
		globals_m10->all_structures_aligned = TRUE_m10;
		if (globals_m10->verbose == TRUE_m10)
			message_m10("All MED Library structures are aligned");
	} else {
		error_message_m10("%s(): unaligned MED structures\n\tcalled from function %s(), line %d", __FUNCTION__, function, line);
	}
	
	return(return_value);
}


TERN_m10        check_CMP_block_header_alignment_m10(ui1 *bytes)
{
        CMP_BLOCK_FIXED_HEADER_m10    *cbh;
        TERN_m10                      free_flag = FALSE_m10;
        
        
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
                bytes = (ui1 *) e_malloc_m10(CMP_BLOCK_FIXED_HEADER_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                free_flag = TRUE_m10;
        }
        cbh = (CMP_BLOCK_FIXED_HEADER_m10 *) bytes;
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
                e_free_m10(bytes, __FUNCTION__, __LINE__);
        
        if (globals_m10->verbose == TRUE_m10)
		message_m10("CMP_BLOCK_FIXED_HEADER_m10 structure is aligned", __FUNCTION__);
        
        return(TRUE_m10);
        
        // not aligned
CMP_BLOCK_HEADER_NOT_ALIGNED_m10:
        
        if (free_flag == TRUE_m10)
                e_free_m10(bytes, __FUNCTION__, __LINE__);
        
        if (globals_m10->verbose == TRUE_m10)
                error_message_m10("%s(): CMP_BLOCK_FIXED_HEADER_m10 structure is NOT aligned", __FUNCTION__);
        
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
		bytes = (ui1 *) e_malloc_m10(CMP_RECORD_HEADER_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	crh = (CMP_RECORD_HEADER_m10 *) bytes;
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
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("CMP_RECORD_HEADER_m10 structure is aligned");
	
	return(TRUE_m10);
	
	// not aligned
CMP_RECORD_HEADER_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): CMP_RECORD_HEADER_m10 structure is NOT aligned", __FUNCTION__);
	
	return(FALSE_m10);
}


TERN_m10        check_metadata_alignment_m10(ui1 *bytes)
{
	TERN_m10	return_value, free_flag = FALSE_m10;
	
	
	// see if already checked
	if (globals_m10->all_metadata_structures_aligned != UNKNOWN_m10)
		return(globals_m10->all_metadata_structures_aligned);
	
	return_value = TRUE_m10;
	if (bytes == NULL) {
		bytes = (ui1 *) e_malloc_m10(METADATA_FILE_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_flag = TRUE_m10;
	}
	
	// check substructures
	if ((check_metadata_section_1_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_time_series_metadata_section_2_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_video_metadata_section_2_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	if ((check_metadata_section_3_alignment_m10(bytes)) == FALSE_m10)
		return_value = FALSE_m10;
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
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
		bytes = (ui1 *) e_malloc_m10(METADATA_FILE_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("METADATA_SECTION_1_m10 structure is aligned");
	
        return(TRUE_m10);
	
	// not aligned
METADATA_SECTION_1_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): METADATA_SECTION_1_m10 structure is NOT aligned", __FUNCTION__);
	
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
		bytes = (ui1 *) e_malloc_m10(METADATA_FILE_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
        if (md3->recording_city != (si1 *) (bytes + METADATA_RECORDING_CITY_OFFSET_m10))
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
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("METADATA_SECTION_3_m10 structure is aligned");
	
        return(TRUE_m10);
	
	// not aligned
METADATA_SECTION_3_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
        
	if (globals_m10->verbose == TRUE_m10)
	        error_message_m10("%s(): METADATA_SECTION_3_m10 structure is NOT aligned", __FUNCTION__);
        
	return(FALSE_m10);
}


TERN_m10	check_password_m10(si1 *password)
{
        si4	pw_len;
        
        
        // check pointer: return -1 for NULL
        if (password == NULL) {
		warning_message_m10("%s(): password is NULL", __FUNCTION__);
                return(FALSE_m10);
        }
        
        // check password length:  return +1 for length error
        pw_len = UTF8_strlen_m10(password);
        if (pw_len == 0) {
		warning_message_m10("%s(): password has no characters", __FUNCTION__);
                return(FALSE_m10);
        }
        if (pw_len > MAX_PASSWORD_CHARACTERS_m10) {
		warning_message_m10("%s(): password too long (1 to  %d characters)", __FUNCTION__, MAX_PASSWORD_CHARACTERS_m10);
                return(FALSE_m10);
        }
        
	if (globals_m10->verbose == TRUE_m10)
		message_m10("Password is of valid form", __FUNCTION__);
        
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
		bytes = (ui1 *) e_malloc_m10(RECORD_HEADER_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
	if (&rh->version_major != (ui1 *) (bytes + RECORD_HEADER_VERSION_MAJOR_OFFSET_m10))
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	if (&rh->version_minor != (ui1 *) (bytes + RECORD_HEADER_VERSION_MINOR_OFFSET_m10))
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	if (&rh->encryption_level != (si1 *) (bytes + RECORD_HEADER_ENCRYPTION_LEVEL_OFFSET_m10))
		goto RECORD_HEADER_NOT_ALIGNED_m10;
	
	// aligned
        globals_m10->record_header_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("RECORD_HEADER_m10 structure is aligned");
	
        return(TRUE_m10);
	
	// not aligned
RECORD_HEADER_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
        if (globals_m10->verbose == TRUE_m10)
	        error_message_m10("%s(): RECORD_HEADER_m10 structure is NOT aligned", __FUNCTION__);
        
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
		bytes = (ui1 *) e_malloc_m10(RECORD_INDEX_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
	if (&ri->version_major != (ui1 *) (bytes + RECORD_INDEX_VERSION_MAJOR_OFFSET_m10))
		goto RECORD_INDICES_NOT_ALIGNED_m10;
	if (&ri->version_minor != (ui1 *) (bytes + RECORD_INDEX_VERSION_MINOR_OFFSET_m10))
		goto RECORD_INDICES_NOT_ALIGNED_m10;
	if (&ri->encryption_level != (si1 *) (bytes + RECORD_INDEX_ENCRYPTION_LEVEL_OFFSET_m10))
		goto RECORD_INDICES_NOT_ALIGNED_m10;
	
	// aligned
        globals_m10->record_indices_aligned = TRUE_m10;
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		printf("RECORD_INDEX_m10 structure is aligned");
        
        return(TRUE_m10);
	
	// not aligned
RECORD_INDICES_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
        if (globals_m10->verbose == TRUE_m10)
                error_message_m10("%s(): RECORD_INDEX_m10 structure is NOT aligned", __FUNCTION__);
        
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
		bytes = (ui1 *) e_malloc_m10(TIME_SERIES_INDEX_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("TIME_SERIES_INDEX_m10 structure is aligned");
	
	return(TRUE_m10);
	
	// not aligned
TIME_SERIES_INDICES_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
        if (globals_m10->verbose == TRUE_m10)
                error_message_m10("%s(): TIME_SERIES_INDEX_m10 structure is NOT aligned", __FUNCTION__);
	
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
		bytes = (ui1 *) e_malloc_m10(METADATA_FILE_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("TIME_SERIES_METADATA_SECTION_2_m10 structure is aligned");
	
	return(TRUE_m10);
	
	// not aligned
TIME_SERIES_METADATA_SECTION_2_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): TIME_SERIES_METADATA_SECTION_2_m10 structure is NOT aligned", __FUNCTION__);
	
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
		bytes = (ui1 *) e_malloc_m10(UNIVERSAL_HEADER_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
        if (uh->channel_name != (si1 *) (bytes + UNIVERSAL_HEADER_CHANNEL_NAME_OFFSET_m10))
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
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("UNIVERSAL_HEADER_m10 structure is aligned");
	
        return(TRUE_m10);
	
	// not aligned
UNIVERSAL_HEADER_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): UNIVERSAL_HEADER_m10 structure is NOT aligned", __FUNCTION__);
        
        return(FALSE_m10);
}


TERN_m10	check_video_indices_alignment_m10(ui1 *bytes)
{
	VIDEO_INDEX_m10 *vi;
	TERN_m10	free_flag = FALSE_m10;
	
	
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
		bytes = (ui1 *) e_malloc_m10(VIDEO_INDEX_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("VIDEO_INDEX_m10 structure is aligned");
	
	return(TRUE_m10);
	
	// not aligned
VIDEO_INDICES_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
        
	if (globals_m10->verbose == TRUE_m10)
                error_message_m10("%s(): VIDEO_INDEX_m10 structure is NOT aligned", __FUNCTION__);
	
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
		bytes = (ui1 *) e_malloc_m10(METADATA_FILE_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		message_m10("VIDEO_METADATA_SECTION_2_m10 structure is aligned");
	
	return(TRUE_m10);
	
	// not aligned
VIDEO_METADATA_SECTION_2_NOT_ALIGNED_m10:
	
	if (free_flag == TRUE_m10)
		e_free_m10(bytes, __FUNCTION__, __LINE__);
	
	if (globals_m10->verbose == TRUE_m10)
		error_message_m10("%s(): VIDEO_METADATA_SECTION_2_m10 structure is NOT aligned", __FUNCTION__);
	
	return(FALSE_m10);
}


//***********************************************************************//
//******************  END ALIGNMENT CHECKING FUNCTIONS  *****************//
//***********************************************************************//


//***********************************************************************//
//****************************  CMP FUNCTIONS  **************************//
//***********************************************************************//


CMP_PROCESSING_STRUCT_m10    *CMP_allocate_processing_struct_m10(CMP_PROCESSING_STRUCT_m10 *cps, ui4 mode, si8 data_samples, si8 compressed_data_bytes, si8 difference_bytes, ui4 block_samples, CMP_DIRECTIVES_m10 *directives, CMP_PARAMETERS_m10 *parameters)
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
                cps = (CMP_PROCESSING_STRUCT_m10 *) e_calloc_m10((size_t) 1, sizeof(CMP_PROCESSING_STRUCT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        cps->mutex = FALSE_m10;
	if (cps->password_data == NULL)
		cps->password_data = &globals_m10->password_data;

        // set up directives
        if (directives != NULL) {
                cps->directives = *directives;
        } else {
                // set defaults
		if (mode == CMP_COMPRESSION_MODE_NO_ENTRY_m10) {
			error_message_m10("%s(): No compression mode specified", __FUNCTION__);
			exit(1);
		}
                cps->directives.mode = mode;
		cps->directives.free_password_data = CMP_DIRECTIVES_FREE_PASSWORD_DATA_DEFAULT_m10;
                cps->directives.algorithm = CMP_DIRECTIVES_ALGORITHM_DEFAULT_m10;
		cps->directives.encryption_level = CMP_DIRECTIVES_ENCRYPTION_LEVEL_DEFAULT_m10;
                cps->directives.fall_through_to_MBE = CMP_DIRECTIVES_FALL_THROUGH_TO_MBE_DEFAULT_m10;
                cps->directives.reset_discontinuity = CMP_DIRECTIVES_RESET_DISCONTINUITY_DEFAULT_m10;
		cps->directives.include_noise_scores = CMP_DIRECTIVES_INCLUDE_NOISE_SCORES_DEFAULT_m10;
		cps->directives.no_zero_counts = CMP_DIRECTIVES_NO_ZERO_COUNTS_DEFAULT_m10;
		cps->directives.set_derivative_level = CMP_DIRECTIVES_SET_DERIVATIVE_LEVEL_DEFAULT_m10;
		cps->directives.find_derivative_level = CMP_DIRECTIVES_FIND_DERIVATIVE_LEVEL_DEFAULT_m10;
		cps->directives.detrend_data = CMP_DIRECTIVES_DETREND_DATA_DEFAULT_m10;
		cps->directives.require_normality = CMP_DIRECTIVES_REQUIRE_NORMALITY_DEFAULT_m10;
		cps->directives.use_compression_ratio = CMP_DIRECTIVES_USE_COMPRESSION_RATIO_DEFAULT_m10;
		cps->directives.use_mean_residual_ratio = CMP_DIRECTIVES_USE_MEAN_RESIDUAL_RATIO_DEFAULT_m10;
		cps->directives.use_relative_ratio = CMP_DIRECTIVES_USE_RELATIVE_RATIO_DEFAULT_m10;
                cps->directives.set_amplitude_scale = CMP_DIRECTIVES_SET_AMPLITUDE_SCALE_DEFAULT_m10;
		cps->directives.find_amplitude_scale = CMP_DIRECTIVES_FIND_AMPLITUDE_SCALE_DEFAULT_m10;
		cps->directives.set_frequency_scale = CMP_DIRECTIVES_SET_FREQUENCY_SCALE_DEFAULT_m10;
		cps->directives.find_frequency_scale = CMP_DIRECTIVES_FIND_FREQUENCY_SCALE_DEFAULT_m10;
        }

        // set up parameters
        if (parameters != NULL) {
                cps->parameters = *parameters;
        } else {
                // set defaults
		cps->parameters.number_of_block_parameters = 0;
		cps->parameters.block_parameters = NULL;
                cps->parameters.minimum_sample_value = CMP_PARAMETERS_MINIMUM_SAMPLE_VALUE_DEFAULT_m10;
                cps->parameters.maximum_sample_value = CMP_PARAMETERS_MAXIMUM_SAMPLE_VALUE_DEFAULT_m10;
		cps->parameters.discontinuity = CMP_PARAMETERS_DISCONTINUITY_DEFAULT_m10;
		cps->parameters.no_zero_counts_flag = CMP_PARAMETERS_NO_ZERO_COUNTS_FLAG_DEFAULT_m10;
		cps->parameters.derivative_level = CMP_PARAMETERS_DERIVATIVE_LEVEL_DEFAULT_m10;
		cps->parameters.goal_ratio = CMP_PARAMETERS_GOAL_RATIO_DEFAULT_m10;
		cps->parameters.goal_tolerance = CMP_PARAMETERS_GOAL_TOLERANCE_DEFAULT_m10;
		cps->parameters.maximum_goal_attempts = CMP_PARAMETERS_MAXIMUM_GOAL_ATTEMPTS_DEFAULT_m10;
		cps->parameters.minimum_normality = CMP_PARAMETERS_MINIMUM_NORMALITY_DEFAULT_m10;
		cps->parameters.amplitude_scale = CMP_PARAMETERS_AMPLITUDE_SCALE_DEFAULT_m10;
		cps->parameters.frequency_scale = CMP_PARAMETERS_FREQUENCY_SCALE_DEFAULT_m10;
		cps->parameters.user_number_of_records = CMP_USER_NUMBER_OF_RECORDS_DEFAULT_m10;
		cps->parameters.user_record_region_bytes = CMP_USER_RECORD_REGION_BYTES_DEFAULT_m10;
		cps->parameters.user_parameter_flags = CMP_USER_PARAMETER_FLAGS_DEFAULT_m10;
		cps->parameters.protected_region_bytes = CMP_PROTECTED_REGION_BYTES_DEFAULT_m10;
		cps->parameters.user_discretionary_region_bytes = CMP_USER_DISCRETIONARY_REGION_BYTES_DEFAULT_m10;
        }
	        
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
                        cps->count = (ui4 **) e_calloc_2D_m10(num_cats, CMP_RED_MAX_STATS_BINS_m10, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                        cps->sorted_count = (CMP_STATISTICS_BIN_m10 **) e_calloc_2D_m10(num_cats, CMP_RED_MAX_STATS_BINS_m10, sizeof(CMP_STATISTICS_BIN_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                        cps->symbol_map = (ui1 **) e_calloc_2D_m10(num_cats, CMP_RED_MAX_STATS_BINS_m10, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                }
                cps->cumulative_count = (ui8 **) e_calloc_2D_m10(num_cats, CMP_RED_MAX_STATS_BINS_m10 + 1, sizeof(ui8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                cps->minimum_range = (ui8 **) e_calloc_2D_m10(num_cats, CMP_RED_MAX_STATS_BINS_m10, sizeof(ui8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
                cps->input_buffer = cps->original_ptr = cps->original_data = (si4 *) e_calloc_m10((size_t) data_samples, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);

        // compressed_data - caller specified array size
        if (need_compressed_data == TRUE_m10 && cps->compressed_data == NULL)
                        cps->block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) (cps->compressed_data = (ui1 *) e_calloc_m10((size_t) compressed_data_bytes, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10));
        
        // difference_buffer - caller specified or maximum bytes required for specified block size
        if (difference_bytes == 0)
                difference_bytes = CMP_MAX_DIFFERENCE_BYTES_m10(block_samples);
        if (need_difference_buffer == TRUE_m10 && cps->difference_buffer == NULL)
                cps->difference_buffer = (si1 *) e_calloc_m10((size_t) difference_bytes, sizeof(ui1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
        // decompressed_data - caller specified array size
        if (need_decompressed_data == TRUE_m10 && cps->decompressed_data == NULL)
                cps->decompressed_data = cps->decompressed_ptr = (si4 *) e_calloc_m10((size_t) data_samples, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
        // detrended_buffer - maximum bytes required for caller specified block size
        if (need_detrended_buffer == TRUE_m10 && cps->detrended_buffer == NULL)
                cps->detrended_buffer = (si4 *) e_calloc_m10((size_t) block_samples, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
	// derivative_buffer - maximum bytes required for caller specified block size
	if (need_derivative_buffer == TRUE_m10 && cps->derivative_buffer == NULL) {
		derivative_bytes = CMP_MAX_DIFFERENCE_BYTES_m10(block_samples);
		cps->derivative_buffer = (si1 *) e_calloc_m10((size_t) derivative_bytes, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	}

        // scaled_amplitude_buffer - maximum bytes required for caller specified block size
        if (need_scaled_amplitude_buffer == TRUE_m10 && cps->scaled_amplitude_buffer == NULL)
                cps->scaled_amplitude_buffer = (si4 *) e_calloc_m10((size_t) block_samples, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
        // scaled_frequency_buffer - maximum bytes required for caller specified block size
        if (need_scaled_frequency_buffer == TRUE_m10 && cps->scaled_frequency_buffer == NULL)
                cps->scaled_frequency_buffer = (si4 *) e_calloc_m10((size_t) block_samples, sizeof(si4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);

	return(cps);
}


inline sf8      CMP_calculate_mean_residual_ratio_m10(si4 *original_data, si4 *lossy_data, ui4 n_samps)
{
        sf8        sum, mrr, diff, r;
        si8        i;
        
        
        sum = (sf8) 0.0;
        for (i = n_samps; i--;) {
                if (*original_data) {
                        diff = (sf8) (*original_data - *lossy_data++);
                        r = diff / (sf8) *original_data++;
                        sum += ABS_m10(r);
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


void    CMP_calculate_statistics_m10(void *stats_ptr, si4 *input_buffer, si8 len, NODE_m10 *nodes)
{
        NODE_m10        	*np, head, tail;
	TERN_m10		free_nodes;
	REC_Stat_v10_m10	*stats;
        si4             	*x;
        sf16            	sum_x, n, dm, t, sdm2, sdm3, sdm4, m1, m2, m3, m4;
        sf8             	true_median;
        si8             	i, n_nodes, mid_idx, max_cnt, running_cnt;
        ui1             	median_found;
        
        
	// cast
	stats = (REC_Stat_v10_m10 *) stats_ptr;
	
        // allocate
	if (nodes == NULL) {
        	nodes = (NODE_m10 *) e_calloc_m10((size_t) len, sizeof(NODE_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		free_nodes = TRUE_m10;
	} else {
		free_nodes = FALSE_m10;
	}

        // sort
        x = input_buffer;
        n_nodes = ts_sort_m10(x, len, nodes, &head, &tail, FALSE_m10);
        
        // min, max, mean, median, & mode
        sum_x = (sf16) 0.0;
        running_cnt = max_cnt = 0;
        mid_idx = len >> 1;
        median_found = 0;
        for (i = n_nodes, np = head.next; i--; np = np->next) {
                sum_x += (sf16) np->val * (sf16) np->count;
                if (np->count > max_cnt) {
                        max_cnt = np->count;
                        stats->mode = np->val;
                }
                if (median_found == 0) {
                        running_cnt += np->count;
                        if (running_cnt >= mid_idx) {
                                if (running_cnt == mid_idx) {
                                        true_median = (sf8) np->val + (sf8) np->next->val;
                                        stats->median = CMP_round_m10(true_median);
                                } else {
                                        stats->median = np->val;
                                }
                                median_found = 1;
                        }
                }
        }
        n = (sf16) len;
        stats->minimum = head.next->val;
        stats->maximum = head.prev->val;
        m1 = sum_x / n;
        stats->mean = CMP_round_m10((sf8) m1);
        
        // variance
        sdm2 = sdm3 = sdm4 = (sf16) 0.0;
        for (i = n_nodes, np = head.next; i--; np = np->next) {
                dm = (sf16) np->val - m1;
                sdm2 += (t = dm * dm * (sf16) np->count);
                sdm3 += (t *= dm);
                sdm4 += (t *= dm);
        }
        stats->variance = (sf4) (m2 = sdm2 / n);
        m3 = sdm3 / n;
        m4 = sdm4 / n;

        // skewness
        t = m3 / sqrtl(m2 * m2 * m2);
        if (isnan(t))
                t = (sf16) 0.0;  // possible NaN here: set to zero
        else if (len > 2) // correct bias
            t *= sqrtl((n - (sf16) 1) / n) * (n / (n - (sf16) 2));
        stats->skewness = (sf4) t;

        // kurtosis
        t = m4 / (m2 * m2);
        if (len > 3) { // correct bias
            t = ((n + (sf16) 1) * t) - ((sf16) 3 * (n - (sf16) 1));
            t *= (n - (sf16) 1) / ((n - (sf16) 2) * (n - (sf16) 3));
            t += 3;
        }
        stats->kurtosis = (sf4) t;

        // clean up
	if (free_nodes == TRUE_m10)
        	e_free_m10(nodes, __FUNCTION__, __LINE__);

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
                error_message_m10("%s(): \"compressed_data\" is not allocated in the CMP_PROCESSING_STRUCT", __FUNCTION__);
		ret_val = FALSE_m10;
        }
        
        // check difference_buffer
        if (need_difference_buffer == TRUE_m10 && cps->difference_buffer == NULL) {
                error_message_m10("%s(): \"difference_buffer\" is not allocated in the CMP_PROCESSING_STRUCT", __FUNCTION__);
		ret_val = FALSE_m10;
        }
        
        // check original_data
        if (need_original_data == TRUE_m10 && cps->original_data == NULL) {
                error_message_m10("%s(): \"original_data\" is not allocated in the CMP_PROCESSING_STRUCT", __FUNCTION__);
		ret_val = FALSE_m10;
        }
        if (need_original_data == FALSE_m10 && cps->original_data != NULL) {
                warning_message_m10("%s(): \"original_data\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing", __FUNCTION__);
                e_free_m10(cps->original_data, __FUNCTION__, __LINE__);
                cps->original_ptr = cps->original_data = NULL;
		ret_val = FALSE_m10;
        }
        
        // check decompressed_data
        if (need_decompressed_data == TRUE_m10 && cps->decompressed_data == NULL) {
                error_message_m10("%s(): \"decompressed_data\" is not allocated in the CMP_PROCESSING_STRUCT", __FUNCTION__);
		ret_val = FALSE_m10;
        }
        if (need_decompressed_data == FALSE_m10 && cps->decompressed_data != NULL) {
                warning_message_m10("%s(): \"decompressed_data\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing", __FUNCTION__);
                e_free_m10(cps->decompressed_data, __FUNCTION__, __LINE__);
                cps->decompressed_ptr = cps->decompressed_data = NULL;
		ret_val = FALSE_m10;
        }
        
        // check detrended_buffer
        if (need_detrended_buffer == TRUE_m10 && cps->detrended_buffer == NULL) {
                error_message_m10("%s(): \"detrended_buffer\" is not allocated in the CMP_PROCESSING_STRUCT", __FUNCTION__);
		ret_val = FALSE_m10;
        }
        if (need_detrended_buffer == FALSE_m10 && cps->detrended_buffer != NULL) {
                warning_message_m10("%s(): \"detrended_buffer\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing", __FUNCTION__);
                e_free_m10(cps->detrended_buffer, __FUNCTION__, __LINE__);
                cps->detrended_buffer = NULL;
		ret_val = FALSE_m10;
        }
        
        // check derivative_buffer
        if (need_derivative_buffer == TRUE_m10 && cps->derivative_buffer == NULL) {
                error_message_m10("%s(): \"derivative_buffer\" is not allocated in the CMP_PROCESSING_STRUCT", __FUNCTION__);
		ret_val = FALSE_m10;
        }
        if (need_derivative_buffer == FALSE_m10 && cps->derivative_buffer != NULL) {
                warning_message_m10("%s(): \"derivative_buffer\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing", __FUNCTION__);
                e_free_m10(cps->derivative_buffer, __FUNCTION__, __LINE__);
                cps->derivative_buffer = NULL;
		ret_val = FALSE_m10;
        }
        
        // check scaled_amplitude_buffer
        if (need_scaled_amplitude_buffer == TRUE_m10 && cps->scaled_amplitude_buffer == NULL) {
                error_message_m10("%s(): \"scaled_amplitude_buffer\" is not allocated in the CMP_PROCESSING_STRUCT", __FUNCTION__);
		ret_val = FALSE_m10;
        }
        if (need_scaled_amplitude_buffer == FALSE_m10 && cps->scaled_amplitude_buffer != NULL) {
                warning_message_m10("%s(): \"scaled_amplitude_buffer\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing", __FUNCTION__);
                e_free_m10(cps->scaled_amplitude_buffer, __FUNCTION__, __LINE__);
                cps->scaled_amplitude_buffer = NULL;
		ret_val = FALSE_m10;
        }
        
        // check scaled_frequency_buffer
        if (need_scaled_frequency_buffer == TRUE_m10 && cps->scaled_frequency_buffer == NULL) {
                error_message_m10("%s(): \"scaled_frequency_buffer\" is not allocated in the CMP_PROCESSING_STRUCT", __FUNCTION__);
		ret_val = FALSE_m10;
        }
        if (need_scaled_frequency_buffer == FALSE_m10 && cps->scaled_frequency_buffer != NULL) {
                warning_message_m10("%s(): \"scaled_frequency_buffer\" is needlessly allocated in the CMP_PROCESSING_STRUCT => freeing", __FUNCTION__);
                e_free_m10(cps->scaled_frequency_buffer, __FUNCTION__, __LINE__);
                cps->scaled_frequency_buffer = NULL;
		ret_val = FALSE_m10;
        }
        
        return(ret_val);
}


inline void CMP_cps_mutex_off_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
        cps->mutex = FALSE_m10;
        
        return;
}


inline void CMP_cps_mutex_on_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
        while (cps->mutex == TRUE_m10);
        cps->mutex = TRUE_m10;
        
        return;
}


void    CMP_decode_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
        CMP_BLOCK_FIXED_HEADER_m10      *block_header;
	ui4				offset;
	si4				*si4_p;
	sf4				*sf4_p;
	sf8				amplitude_scale, frequency_scale, intercept, gradient;

        
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
                        (void) error_message_m10("%s(): unrecognized compression algorithm (%u)", __FUNCTION__, cps->directives.algorithm);
                        return;
        }

        // unscale frequency-scaled decompressed_data if scaled (in place)
	if (block_header->parameter_flags & CMP_PF_FREQUENCY_SCALE_MASK_m10) {
		sf4_p = (sf4 *) cps->parameters.block_parameters;
		offset = cps->parameters.block_parameters[cps->parameters.block_parameter_map[CMP_PF_FREQUENCY_SCALE_IDX_m10]];
		frequency_scale = (sf8) *(sf4_p + offset);
                CMP_unscale_frequency_m10(cps->decompressed_ptr, cps->decompressed_ptr, (si8) block_header->number_of_samples, frequency_scale);
	}
																				      
        // unscale amplitude-scaled decompressed_data if scaled (in place)
	if (block_header->parameter_flags & CMP_PF_AMPLITUDE_SCALE_MASK_m10) {
		sf4_p = (sf4 *) cps->parameters.block_parameters;
		offset = cps->parameters.block_parameters[cps->parameters.block_parameter_map[CMP_PF_AMPLITUDE_SCALE_IDX_m10]];
		amplitude_scale = (sf8) *(sf4_p + offset);
                CMP_unscale_amplitude_m10(cps->decompressed_ptr, cps->decompressed_ptr, (si8) block_header->number_of_samples, amplitude_scale);
	}
        
        // add trend to decompressed_data if detrended (in place)
	if (CMP_IS_DETRENDED_m10(block_header)) {
		sf4_p = (sf4 *) cps->parameters.block_parameters;
		offset = cps->parameters.block_parameters[cps->parameters.block_parameter_map[CMP_PF_GRADIENT_IDX_m10]];
		gradient = (sf8) *(sf4_p + offset);
		si4_p = (si4 *) cps->parameters.block_parameters;
		offset = cps->parameters.block_parameters[cps->parameters.block_parameter_map[CMP_PF_INTERCEPT_IDX_m10]];
		intercept = (sf8) *(si4_p + offset);
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
        if  (block_header->block_flags & CMP_BF_LEVEL_1_ENCRYPTION_MASK_m10) {
               if  (block_header->block_flags & CMP_BF_LEVEL_2_ENCRYPTION_MASK_m10) {
                       error_message_m10("%s(): Cannot decrypt data: flags indicate both level 1 & level 2 encryption", __FUNCTION__);
                       return(FALSE_m10);
               }
               if (pwd->access_level >= LEVEL_1_ENCRYPTION_m10) {
                       key = pwd->level_1_encryption_key;
               } else {
                       (void) error_message_m10("%s(): Cannot decrypt data: insufficient access\n", __FUNCTION__);
                       return(FALSE_m10);
               }
        } else {  // level 2 bit is set
                if (pwd->access_level == LEVEL_2_ENCRYPTION_m10) {
                        key = pwd->level_2_encryption_key;
                } else {
                        (void) error_message_m10("%s(): Cannot decrypt data: insufficient access\n", __FUNCTION__);
                        return(-1);
                }
        }
        
        // calculated encryption blocks
        encryptable_blocks = (block_header->total_block_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10) / ENCRYPTION_BLOCK_BYTES_m10;
        if (block_header->block_flags | CMP_BF_MBE_ENCODING_MASK_m10) {
                encryption_blocks = encryptable_blocks;
        } else {
                encryption_bytes = block_header->total_header_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10 + ENCRYPTION_BLOCK_BYTES_m10;
                encryption_blocks = ((encryption_bytes - 1) / ENCRYPTION_BLOCK_BYTES_m10) + 1;
                if (encryptable_blocks < encryption_blocks)
                        encryption_blocks = encryptable_blocks;
        }
        
        // decrypt
        ui1_p = (ui1 *) block_header + CMP_BLOCK_ENCRYPTION_START_OFFSET_m10;
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
		sf4_m = (sf4) m;
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
        void                            (*compression_f)(CMP_PROCESSING_STRUCT_m10 *cps);
        CMP_BLOCK_FIXED_HEADER_m10      *block_header;

        
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
	} else if (block_header->block_flags & CMP_BF_RED_ENCODING_MASK_m10) {
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
                        error_message_m10("%s(): unrecognized compression algorithm (%u)", __FUNCTION__, cps->directives.algorithm);
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
				CMP_scale_amplitude_m10(cps->input_buffer, cps->scaled_amplitude_buffer, block_header->number_of_samples, (sf8) cps->parameters.amplitude_scale, cps);
			if (cps->directives.find_amplitude_scale == TRUE_m10)
				data_is_compressed = CMP_find_amplitude_scale_m10(cps, compression_f);
			cps->input_buffer = cps->scaled_amplitude_buffer;
		}
		if (cps->directives.set_frequency_scale == TRUE_m10 || cps->directives.find_frequency_scale == TRUE_m10) {
			if (cps->directives.set_frequency_scale == TRUE_m10)
				CMP_scale_frequency_m10(cps->input_buffer, cps->scaled_frequency_buffer, block_header->number_of_samples, (sf8) cps->parameters.frequency_scale, cps);
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
			warning_message_m10("%s(): Level 1 & 2 bits set in block => cannot encrypt", __FUNCTION__);
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
                } else {
                        key = pwd->level_2_encryption_key;
                        encryption_mask = CMP_BF_LEVEL_2_ENCRYPTION_MASK_m10;
                }
        } else {
                error_message_m10("%s(): Cannot encrypt data => insufficient access", __FUNCTION__);
                return(FALSE_m10);
        }
        
        // encrypt
        encryptable_blocks = (block_header->total_block_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10) / ENCRYPTION_BLOCK_BYTES_m10;
        if (block_header->block_flags & CMP_BF_MBE_ENCODING_MASK_m10) {
                encryption_blocks = encryptable_blocks;
        } else {
                encryption_bytes = block_header->total_header_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10 + ENCRYPTION_BLOCK_BYTES_m10;
                encryption_blocks = ((encryption_bytes - 1) / ENCRYPTION_BLOCK_BYTES_m10) + 1;
                if (encryptable_blocks < encryption_blocks)
                        encryption_blocks = encryptable_blocks;
        }
        ui1_p = (ui1 *) block_header + CMP_BLOCK_ENCRYPTION_START_OFFSET_m10;
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
        si4                     	*input_buffer;
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
                cps->parameters.amplitude_scale = (sf4) 1.0;
                (*compression_f)(cps);
                original_size = (sf8) block_header->number_of_samples * (sf8) sizeof(si4);
                cps->parameters.actual_ratio = (sf8) block_header->total_block_bytes / original_size;
                if (cps->parameters.actual_ratio > goal_high_bound) {
                        // loop until acceptable scale factor found
                        for (i = cps->parameters.maximum_goal_attempts; i--;) {
                                new_scale_factor = cps->parameters.amplitude_scale * (sf4) (cps->parameters.actual_ratio / goal_compression_ratio);
                                if ((ABS_m10(new_scale_factor - cps->parameters.amplitude_scale) <= (sf4) 0.000001) || (new_scale_factor <= (sf4) 1.0))
                                        break;
                                cps->parameters.amplitude_scale = new_scale_factor;
                                (*compression_f)(cps);  // compress
                                cps->parameters.actual_ratio = (sf8) block_header->total_block_bytes / original_size;
                                if ((cps->parameters.actual_ratio <= goal_high_bound) && (cps->parameters.actual_ratio >= goal_low_bound))
                                        break;
                        }
                }
                CMP_generate_lossy_data_m10(cps, input_buffer, cps->decompressed_ptr, CMP_AMPLITUDE_SCALE_MODE_m10);
                cps->input_buffer = cps->decompressed_ptr;
        } else if (cps->directives.use_mean_residual_ratio == TRUE_m10) {
                // get residual ratio at sf 2 & 5 (roughly linear relationship: reasonable sample points)
                cps->parameters.amplitude_scale = (sf4) 2.0;
                CMP_generate_lossy_data_m10(cps, input_buffer, cps->decompressed_ptr, CMP_AMPLITUDE_SCALE_MODE_m10);
                mrr2 = CMP_calculate_mean_residual_ratio_m10(cps->original_ptr, cps->decompressed_ptr, block_header->number_of_samples);
                if (mrr2 == (sf8) 0.0) {  // all zeros in block
                        cps->parameters.amplitude_scale = (sf4) 1.0;
                        cps->parameters.actual_ratio = (sf8) 0.0;
                        (*compression_f)(cps);
                        goto CMP_MRR_DONE;
                }
                cps->parameters.amplitude_scale = (sf4) 5.0;
                CMP_generate_lossy_data_m10(cps, input_buffer, cps->decompressed_ptr, CMP_AMPLITUDE_SCALE_MODE_m10);
                mrr5 = CMP_calculate_mean_residual_ratio_m10(cps->original_ptr, cps->decompressed_ptr, block_header->number_of_samples);
                sf_per_mrr = (sf8) 3.0 / (mrr5 - mrr2);
                // estimate starting points
                goal_mrr = cps->parameters.goal_ratio;
                goal_tol = cps->parameters.goal_tolerance;
                goal_low_bound = goal_mrr - goal_tol;
                goal_high_bound = goal_mrr + goal_tol;
                cps->parameters.amplitude_scale = (sf4) (((goal_mrr - mrr2) * sf_per_mrr) + (sf8) 2.0);
                high_sf = ((goal_high_bound - mrr2) * sf_per_mrr) + (sf8) 2.0;
                high_sf *= (sf8) 2.0;  // empirically reasonable
                low_sf = (sf8) 1.0;
                for (i = cps->parameters.maximum_goal_attempts; i--;) {
                        CMP_generate_lossy_data_m10(cps, input_buffer, cps->decompressed_ptr, CMP_AMPLITUDE_SCALE_MODE_m10);
                        mrr = CMP_calculate_mean_residual_ratio_m10(cps->original_ptr, cps->decompressed_ptr, block_header->number_of_samples);
                        if (mrr < goal_low_bound)
                                low_sf = (sf8) cps->parameters.amplitude_scale;
                        else if (mrr > goal_high_bound)
                                high_sf = (sf8) cps->parameters.amplitude_scale;
                        else
                                break;
                        new_scale_factor = (sf4) ((low_sf + high_sf) / (sf8) 2.0);
                        if (new_scale_factor <= (sf4) 1.0)
                                break;
                        if ((high_sf - low_sf) < (sf8) 0.005) {
                                cps->parameters.amplitude_scale = new_scale_factor;
                                break;
                        }
                        cps->parameters.amplitude_scale = new_scale_factor;
                }
                cps->parameters.actual_ratio = mrr;
                cps->input_buffer = cps->decompressed_ptr;
                (*compression_f)(cps);  // compress
        } else {
                error_message_m10("%s(): either use_compression_ratio or use_mean_residual_ratio directive must be set (mode == %d)", __FUNCTION__, cps->directives.mode);
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
        if (minimum != NULL && maximum != NULL) {
                *minimum = min;
                *maximum = max;
        }
        
        return;
}


TERN_m10	CMP_find_frequency_scale_m10(CMP_PROCESSING_STRUCT_m10 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m10 *cps))
{
	TERN_m10	data_is_compressed;
	
	data_is_compressed = FALSE_m10;
	
	// code not written yet
	
        return(data_is_compressed);
}


void    CMP_free_processing_struct_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
        if (cps == NULL) {
                warning_message_m10("%s(): trying to free a NULL CMP_PROCESSING_STRUCT_m10 => returning with no action", __FUNCTION__);
                return;
        }
        if (cps->original_data != NULL)
                e_free_m10((void *) cps->original_data, __FUNCTION__, __LINE__);
        
        if (cps->decompressed_data != NULL)
                e_free_m10((void *) cps->decompressed_data, __FUNCTION__, __LINE__);
        
        if (cps->compressed_data != NULL)
                e_free_m10((void *) cps->compressed_data, __FUNCTION__, __LINE__);
        
        if (cps->difference_buffer != NULL)
                e_free_m10((void *) cps->difference_buffer, __FUNCTION__, __LINE__);
        
        if (cps->detrended_buffer != NULL)
                e_free_m10((void *) cps->detrended_buffer, __FUNCTION__, __LINE__);
        
        if (cps->scaled_amplitude_buffer != NULL)
                e_free_m10((void *) cps->scaled_amplitude_buffer, __FUNCTION__, __LINE__);

        if (cps->scaled_frequency_buffer != NULL)
                e_free_m10((void *) cps->scaled_frequency_buffer, __FUNCTION__, __LINE__);
        
        if (cps->count != NULL)
                e_free_m10((void *) cps->count, __FUNCTION__, __LINE__);

        if (cps->cumulative_count != NULL)
                e_free_m10((void *) cps->cumulative_count, __FUNCTION__, __LINE__);

        if (cps->sorted_count != NULL)
                e_free_m10((void *) cps->sorted_count, __FUNCTION__, __LINE__);

        if (cps->minimum_range != NULL)
                e_free_m10((void *) cps->minimum_range, __FUNCTION__, __LINE__);

        if (cps->symbol_map != NULL)
                e_free_m10((void *) cps->symbol_map, __FUNCTION__, __LINE__);
	
	if (cps->directives.free_password_data == TRUE_m10)
		if (cps->password_data != &globals_m10->password_data)
			e_free_m10((void *) cps->password_data, __FUNCTION__, __LINE__);
        
	e_free_m10((void *) cps, __FUNCTION__, __LINE__);
        
        return;
}


void    CMP_generate_lossy_data_m10(CMP_PROCESSING_STRUCT_m10 *cps, si4 *input_buffer, si4 *output_buffer, ui1 mode)
{
        CMP_BLOCK_FIXED_HEADER_m10        *block_header;
        
        
        // generates lossy data from input_buffer to output_buffer
        // if input_buffer == output_buffer lossy data will be made in place
        block_header = cps->block_header;
              
	if (mode == CMP_AMPLITUDE_SCALE_MODE_m10) {
		// amplitude scale from input_buffer to output_buffer (lossy)
		CMP_scale_amplitude_m10(input_buffer, output_buffer, block_header->number_of_samples, (sf8) cps->parameters.amplitude_scale, cps);
		// unscale in place
		CMP_unscale_amplitude_m10(output_buffer, output_buffer, block_header->number_of_samples, (sf8) cps->parameters.amplitude_scale);
	} else if (mode == CMP_FREQUENCY_SCALE_MODE_m10) {
		// frequency scale from input_buffer to output_buffer (lossy)
		CMP_scale_frequency_m10(input_buffer, output_buffer, block_header->number_of_samples, (sf8) cps->parameters.frequency_scale, cps);
		// unscale in place
		CMP_unscale_frequency_m10(output_buffer, output_buffer, block_header->number_of_samples, (sf8) cps->parameters.frequency_scale);
	} else {
		error_message_m10("%s(): unrecognized lossy compression mode => no data generated", __FUNCTION__);
	}
	
        return;
}


void    CMP_get_variable_region_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
        ui1                             *var_reg_ptr;
        CMP_BLOCK_FIXED_HEADER_m10      *block_header;
        
        
        block_header = cps->block_header;
	var_reg_ptr = (ui1 *) block_header + CMP_BLOCK_FIXED_HEADER_BYTES_m10;  // pointer to beginning of variable region
	
	// records region
	cps->records = var_reg_ptr;
	var_reg_ptr += block_header->record_region_bytes;
	
	// parameter region
	cps->parameters.block_parameters = (ui4 *) var_reg_ptr;
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

	cps->parameters.number_of_block_parameters = (si4) n_params;
	bh->parameter_region_bytes = (ui2) (n_params * 4);

	return;
}


void	CMP_initialize_normal_CDF_table_m10(void)
{
        sf8        *cdf_table;
        
        
        cdf_table = (sf8 *) e_calloc_m10((size_t) CMP_NORMAL_CDF_TABLE_ENTRIES_m10, sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
        {
                sf8 temp[CMP_NORMAL_CDF_TABLE_ENTRIES_m10] = CMP_NORMAL_CDF_TABLE_m10;
                memcpy(cdf_table, temp, CMP_NORMAL_CDF_TABLE_ENTRIES_m10 * sizeof(sf8));
        }
        
	globals_m10->CMP_normal_CDF_table = cdf_table;
        
        return;
}


void    CMP_lad_reg_m10(si4 *input_buffer, si8 len, sf8 *m, sf8 *b)
{
        sf8             *y, t, *yp, *buff, *bp, min_y, max_y, min_m, max_m, thresh, m_sum;
        sf8             d, ma, ba, m_eps, b_eps, lad_eps, test_m, lad, upper_m, lower_m;
        si8             i;
        const sf8       safe_eps = DBL_EPSILON * (sf8) 1000.0;

        // linear least absolute deviation_regression (1 array input)
        
        // allocate
        buff = (sf8 *) e_calloc_m10((size_t) len, sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        y = (sf8 *) e_calloc_m10((size_t) len, sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
        // copy & cast
        for (yp = y, i = len; i--;)
                *yp++ = (sf8) *input_buffer++;
        
        // setup
        yp = y;
        min_y = max_y = *yp;
        for (i = len; --i;) {
                if (*++yp > max_y)
                        max_y = *yp;
                else if (*yp < min_y)
                        min_y = *yp;
        }
        upper_m = max_m  = (max_y - min_y) / (sf8) (len - 1);
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
        e_free_m10(buff, __FUNCTION__, __LINE__);
        e_free_m10(y, __FUNCTION__, __LINE__);
        
        return;

}


void    CMP_lin_reg_m10(si4 *input_buffer, si8 len, sf8 *m, sf8 *b)
{
        sf8                     sx, sy, sxx, sxy, n, mx, my, c, val;
        si8                     i;
        
        // linear least_squares regression (1 array input)
        
        n = (sf8) len;
        sx = (n * (n + (sf8) 1.0)) / (sf8) 2.0;
        sxx = (n * (n + (sf8) 1.0) * ((n * (sf8) 2.0) + (sf8) 1.0)) / (sf8) 6.0;
        
        c = sy = sxy = (sf8) 0.0;
        for (i = len; i--;) {
                val = (sf8) *input_buffer++;
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
        si4                             *si4_p, bits_per_samp;
        ui4                             n_samps;
        si8                             i, lmin;
        ui8                             out_val, *in_word, mask, temp_mask, high_bits, in_word_val;
        ui1                             in_bit, *model, *comp_p;
        CMP_BLOCK_FIXED_HEADER_m10      *block_header;
        

        // read header (generic fields)
        block_header = cps->block_header;
        comp_p = (ui1 *) block_header + block_header->total_header_bytes;
	model = comp_p - CMP_MBE_MODEL_FIXED_BYTES_m10;
        lmin = (si8) *((si4 *) (model + CMP_MBE_MODEL_MINIMUM_SAMPLE_VALUE_OFFSET_m10));
        bits_per_samp = (si4) *(model + CMP_MBE_MODEL_BITS_PER_SAMPLE_OFFSET_m10);
        n_samps = block_header->number_of_samples;
	
        // MBE decode
        in_word = (ui8 *) block_header + (block_header->total_header_bytes >> 3);
        in_bit = (block_header->total_header_bytes & 7) << 3;
        mask = (ui8) 0xFFFFFFFFFFFFFFFF >> (64 - bits_per_samp);
        si4_p = cps->decompressed_ptr;
        in_word_val = *in_word >> in_bit;
        for (i = n_samps; i--;) {
                out_val = in_word_val & mask;
                in_word_val >>= bits_per_samp;
                if ((in_bit += bits_per_samp) > 63) {
                        in_word_val = *++in_word;
                        if (in_bit &= 63) {
                                temp_mask = (ui8) 0xFFFFFFFFFFFFFFFF >> (64 - in_bit);
                                high_bits = in_word_val & temp_mask;
                                out_val |= (high_bits << (bits_per_samp - in_bit));
                                in_word_val >>= in_bit;
                        }
                }
                *si4_p++ = (si4) ((si8) out_val + lmin);
        }

        return;
}


void    CMP_MBE_encode_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
        si4                             *si4_p, bits_per_samp;
        ui4                             cmp_data_bytes, variable_region_bytes;
        si8                             i, cmp_data_bits, lmin;
        ui8                             out_val, *out_word;
        ui1                             out_bit, *output, *model;
        CMP_BLOCK_FIXED_HEADER_m10      *block_header;
        
        // compress (compression algorithms are responsible for filling in: total_block_bytes, total_header_bytes, & model_region_bytes in the header, and the model details)
        
        block_header = cps->block_header;
        
        // find full value bits per sample
        lmin = (si8) cps->parameters.minimum_sample_value;
        for (bits_per_samp = 0, i = (si8) cps->parameters.maximum_sample_value - lmin; i; i >>= 1)
                ++bits_per_samp;
	
        // calculate header bytes
	variable_region_bytes = cps->parameters.variable_region_bytes;
	block_header->model_region_bytes = CMP_MBE_MODEL_FIXED_BYTES_m10;
        block_header->total_header_bytes = CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes + block_header->model_region_bytes;
	model = (ui1 *) block_header + CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes;
        output = (ui1 *) block_header + block_header->total_header_bytes;
        
        // calculate total encoding bytes
        cmp_data_bits = (si8) block_header->number_of_samples * (si8) bits_per_samp;
        cmp_data_bytes = cmp_data_bits >> 3;
        if (cmp_data_bits & 7)
                ++cmp_data_bytes;
        
        // MBE encode
        memset(output, 0, cmp_data_bytes);
        out_word = (ui8 *) block_header + (block_header->total_header_bytes >> 3);
        out_bit = (block_header->total_header_bytes & 7) << 3;
        si4_p = cps->input_buffer;
	out_val = 0;
        for (i = block_header->number_of_samples; i--;) {
                out_val = (ui8) ((si8) *si4_p++ - lmin);
                *out_word |= (out_val << out_bit);
                if ((out_bit += bits_per_samp) > 63) {
                        out_bit &= 63;
                        *++out_word = (out_val >> (bits_per_samp - out_bit));
                }
        }

        // fill in header
        block_header->total_block_bytes = pad_m10((ui1 *) block_header, (si8) (cmp_data_bytes + block_header->total_header_bytes), 8);
        *((si4 *) (model + CMP_MBE_MODEL_MINIMUM_SAMPLE_VALUE_OFFSET_m10)) = cps->parameters.minimum_sample_value;
        *((ui1 *) (model + CMP_MBE_MODEL_BITS_PER_SAMPLE_OFFSET_m10)) = (ui1) bits_per_samp;
		
        return;
}


ui1     CMP_normality_score_m10(si4 *data, ui4 n_samps)
{
	sf8     sx, sx2, sy, sy2, sxy, mx, mx2, sd, val, z, r, n, *norm_cdf;
	sf8     num, den1, den2, cdf[CMP_NORMAL_CDF_TABLE_ENTRIES_m10];
	si8     i, count[CMP_NORMAL_CDF_TABLE_ENTRIES_m10] = {0};
	si4     *si4_p, bin;
	ui1     ks;
	
	
	// Returns the correlation of the distribution in the data to that expected from a normal distribution.
	// Essentially a Kolmogorov-Smirnov test normalized to range [-1 to 0) = 0 & [0 to 1] = [0 to 254]. 255 is reserved for no entry.
	
	// calculate mean & standard deviation
	n = (sf8) n_samps;
	si4_p = data;
	sx = sx2 = (sf8) 0.0;
	for (i = n_samps; i--;) {
		val = (sf8) *si4_p++;
		sx += val;
		sx2 += val * val;
	}
	mx = sx / n;
	mx2 = sx2 / n;
	sd = sqrt(mx2 - (mx * mx));
	
	// bin the samples
	si4_p = data;
	for (i = n_samps; i--;) {
		val = (sf8) *si4_p++;
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
		cdf[i] = (sf8) count[i] + cdf[i - 1];
	
	// calculate correlation between data CDF and normal CDF
	sx = sx2 = sxy = (sf8) 0.0;
	sy = (sf8) CMP_SUM_NORMAL_CDF_m10;
	sy2 = (sf8) CMP_SUM_SQ_NORMAL_CDF_m10;
	norm_cdf = globals_m10->CMP_normal_CDF_table;
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
        ui1                             *comp_p, *ui1_p, *low_bound_high_byte_p, *high_bound_high_byte_p;
        ui1                             *goal_bound_high_byte_p, prev_cat;
        ui1                             *symbol_map[CMP_PRED_CATS_m10], *symbols, *model;
        si1                             *si1_p1, *si1_p2, *diff_p;
        ui2                             *bin_counts, n_derivs, *stats_entries, *count[CMP_PRED_CATS_m10];
        ui4                             n_diffs, total_stats_entries;
        si4                             *si4_p, current_val, init_val;
        ui8                             **minimum_range, **cumulative_count;
        ui8                             low_bound, high_bound, prev_high_bound, goal_bound, range;
        si8                             i, j;
        CMP_BLOCK_FIXED_HEADER_m10      *block_header;

        
        // CMP decompress from block_header to decompressed_ptr
        block_header = cps->block_header;
	model = (ui1 *) block_header + block_header->total_header_bytes - block_header->model_region_bytes;
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
                        cumulative_count[i][j + 1] = cumulative_count[i][j] + (ui8) count[i][j];
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
                                prev_cat = CMP_PRED_CAT_m10(*diff_p); ++diff_p;
                                j = 0;
                        } else {
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
                } else {
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
                } else {
                        current_val += (si4) *si1_p1++;
                }
                *si4_p++ = current_val;
        }

        return;
}


void    CMP_PRED_encode_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
        ui1                             *low_bound_high_byte_p, *high_bound_high_byte_p, *ui1_p, prev_cat;
        ui1                             *diff_p, *comp_p, *symbols, **symbol_map, *model;
        ui2                             *bin_counts, *stats_entries;
        ui4                             variable_region_bytes, **count, total_diffs, PRED_total_bytes, MBE_total_bytes, total_stats_entries;
        ui4                             cat_total_counts[CMP_PRED_CATS_m10], goal_total_counts, bin, MBE_cmp_data_bytes;
        si4                             *input_buffer, *curr_si4_val_p, *next_si4_val_p, diff, min, max, MBE_bits_per_samp;
        ui8                             **cumulative_count, **minimum_range;
        ui8                             range, high_bound, low_bound;
        si8                             i, j, k, l, extra_counts, scaled_total_counts, MBE_cmp_data_bits;
        CMP_STATISTICS_BIN_m10          **sorted_count, temp_sorted_count;
        CMP_BLOCK_FIXED_HEADER_m10      *block_header;


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
                block_header->model_region_bytes = (ui2) CMP_PRED_MODEL_FIXED_BYTES_m10;
                memset(model, 0, CMP_PRED_MODEL_FIXED_BYTES_m10);
		if (block_header->number_of_samples == 1)
			*((si4 *) (model + CMP_PRED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10)) = input_buffer[0];
		else
			warning_message_m10("%s(): No samples in block => returning without encoding", __FUNCTION__);
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
                } else {
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
        
	// only one unique value (typically all zeros)
        if (total_stats_entries == 1 && stats_entries[CMP_PRED_NIL_m10] == 1) {
		block_header->total_header_bytes = (ui4) (CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes + CMP_PRED_MODEL_FIXED_BYTES_m10);
		block_header->total_block_bytes = pad_m10((ui1 *) block_header, block_header->total_header_bytes, 8);
		block_header->model_region_bytes = (ui2) CMP_PRED_MODEL_FIXED_BYTES_m10;
		memset(model, 0, CMP_PRED_MODEL_FIXED_BYTES_m10);
		*((si4 *) (model + CMP_PRED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10)) = input_buffer[0];
		return;
        }

        // build sorted_count: bubble sort
        for (i = 0; i < CMP_PRED_CATS_m10; ++i) {
                j = stats_entries[i]; do {
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
                        scaled_total_counts += (si8) sorted_count[i][j].count;
                }
                extra_counts = ((si8) goal_total_counts - (si8) scaled_total_counts);
                if (extra_counts > 0) {
                        do {
                                for (j = 0; (j < stats_entries[i]) && extra_counts; ++j) {
                                        ++sorted_count[i][j].count;
                                        --extra_counts;
                                }
                        } while (extra_counts);
                } else if (extra_counts < 0) {
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
                        *bin_counts++ = (ui2) sorted_count[i][j].count;
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
                } else {
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
                for (MBE_bits_per_samp = 0, i = (si8) max - (si8) min; i; i >>= 1)
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
                        buff = (sf8 *) e_malloc_m10((size_t) len * sizeof(sf8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                        if (buff == NULL) {
                                error_message_m10("%s(): Not enough memory", __FUNCTION__);
                                exit(1);
                        }
                        free_buff = TRUE_m10;
                }
                memcpy(buff, x, len * sizeof(sf8));
        } else {
                buff = x;
        }
        
        if (quantile == (sf8) 1.0) {
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
        
        if (free_buff == TRUE_m10)
                e_free_m10(buff, __FUNCTION__, __LINE__);
        
        return(q);
}


void    CMP_RED_decode_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
        ui1                     *comp_p, *ui1_p, *low_bound_high_byte_p, *high_bound_high_byte_p;
        ui1                     *goal_bound_high_byte_p, prev_cat;
        ui1                     *symbol_map[CMP_PRED_CATS_m10], *symbols, *model;
        si1                     *si1_p1, *si1_p2, *diff_p;
        ui2                     *bin_counts, n_derivs, *stats_entries, *count[CMP_PRED_CATS_m10];
        ui4                     n_diffs, total_stats_entries;
        si4                     *si4_p, current_val, init_val;
        ui8                     *minimum_range[CMP_PRED_CATS_m10], *cumulative_count[CMP_PRED_CATS_m10], *ccs, *mrs;
        ui8                     low_bound, high_bound, prev_high_bound, goal_bound, range;
        si8                     i, j;
        CMP_BLOCK_FIXED_HEADER_m10    *block_header;

        
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
                cumulative_count[i] = ccs; ccs +=  stats_entries[i];
                minimum_range[i] = mrs; mrs +=  stats_entries[i];
                for (cumulative_count[i][0] = j = 0; j < stats_entries[i]; ++j) {
                        cumulative_count[i][j + 1] = cumulative_count[i][j] + (ui8) count[i][j];
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
                        } else {
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
                } else {
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
                } else
                        current_val += (si4) *si1_p1++;
                *si4_p++ = current_val;
        }
        
        return;
}


void    CMP_RED_encode_m10(CMP_PROCESSING_STRUCT_m10 *cps)
{
        ui1                             *low_bound_high_byte_p, *high_bound_high_byte_p, *ui1_val_p, prev_cat;
        ui1                             *diff_p, *comp_p, *symbols, **symbol_map, *model;
        ui2                             *bin_counts, *stats_entries, *num_stats;
        ui4                             variable_region_bytes, **count, total_diffs, PRED_total_bytes, MBE_total_bytes, total_stats_entries;
        ui4                             cat_total_counts[CMP_PRED_CATS_m10], goal_total_counts, bin, MBE_cmp_data_bytes;
        si4                             *input_buffer, *curr_si4_val_p, *next_si4_val_p, diff, min, max, MBE_bits_per_samp;
        ui8                             **cumulative_count, **minimum_range;
        ui8                             range, high_bound, low_bound;
        si8                             i, j, k, l, extra_counts, scaled_total_counts, MBE_cmp_data_bits;
        CMP_STATISTICS_BIN_m10          **sorted_count, temp_sorted_count;
        CMP_BLOCK_FIXED_HEADER_m10      *block_header;

        
        // CMP compress from input_buffer to to block_header -> compressed data array
        count = cps->count;
        cumulative_count = cps->cumulative_count;
        minimum_range = cps->minimum_range;
        sorted_count = cps->sorted_count;
        symbol_map = cps->symbol_map;
        block_header = cps->block_header;
        input_buffer = cps->input_buffer;
	variable_region_bytes = cps->parameters.variable_region_bytes;
        model = (ui1 *) block_header + CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes;

        // if no samples: fill in an empty block header & return;
        if (block_header->number_of_samples <= 0) {
                block_header->total_header_bytes = CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes;
		block_header->model_region_bytes = (ui2) 0;
		block_header->total_block_bytes = pad_m10((ui1 *) block_header, block_header->total_header_bytes, 8);
                error_message_m10("%s(): No samples in block => returning without encoding", __FUNCTION__);
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
                        ui1_val_p = (ui1 *) curr_si4_val_p;
                        ++count[prev_cat][*diff_p++ = CMP_RED_KEYSAMPLE_FLAG_m10];
                        prev_cat = CMP_PRED_NEG_m10;
                        j = 4; do {
                                ++count[prev_cat][*diff_p = *ui1_val_p++];
                                prev_cat = CMP_PRED_CAT_m10(*diff_p); ++diff_p;  // do not increment within call to CAT
                        } while (--j);
                } else {
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
                block_header->model_region_bytes = (ui2) CMP_PRED_MODEL_FIXED_BYTES_m10;
                memset(model, 0, CMP_PRED_MODEL_FIXED_BYTES_m10);
                *((ui2 *) (model + CMP_PRED_MODEL_NUMBER_OF_NIL_STATISTICS_BINS_OFFSET_m10)) = 1;
                return;
        }
        
        // build sorted_count: bubble sort
        for (i = 0; i < CMP_PRED_CATS_m10; ++i) {
                j = stats_entries[i]; do {
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
                        scaled_total_counts += (si8) sorted_count[i][j].count;
                }
                extra_counts = ((si8) goal_total_counts - (si8) scaled_total_counts);
                if (extra_counts > 0) {
                        do {
                                for (j = 0; (j < stats_entries[i]) && extra_counts; ++j) {
                                        ++sorted_count[i][j].count;
                                        --extra_counts;
                                }
                        } while (extra_counts);
                } else if (extra_counts < 0) {
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
                } else {
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
        
        if (val >= 0.0) {
                if ((val += 0.5) >= (sf8) POSITIVE_INFINITY_m10)
                        return(POSITIVE_INFINITY_m10);
        } else {
                if ((val -= 0.5) <= (sf8) NEGATIVE_INFINITY_m10)
                        return(NEGATIVE_INFINITY_m10);
        }
        
        return((si4) val);
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
		scale_factor = (sf8) sf4_scale;
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
		sf4_scale = (sf4) scale_factor;
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
        ui1                     	*var_reg_ptr;
        CMP_BLOCK_FIXED_HEADER_m10    	*block_header;

        
        block_header = cps->block_header;

	// reset variable region parameters
	block_header->number_of_records = cps->parameters.user_number_of_records;
	block_header->record_region_bytes = cps->parameters.user_record_region_bytes;
	block_header->parameter_flags = cps->parameters.user_parameter_flags;
	block_header->protected_region_bytes = cps->parameters.protected_region_bytes;
	block_header->discretionary_region_bytes = cps->parameters.user_discretionary_region_bytes;
	
	// records region
	var_reg_ptr = (ui1 *) block_header + CMP_BLOCK_FIXED_HEADER_BYTES_m10; // pointer to start of variable region
	cps->records = var_reg_ptr;
	var_reg_ptr += block_header->record_region_bytes;

	// parameter region
	cps->parameters.block_parameters = (ui4 *) var_reg_ptr;
	
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
        
        
        printf("--------------- CMP Fixed Block Header - START ---------------\n");
        printf("Block Start UID: 0x%lx\n", bh->block_start_UID);
        if (bh->block_CRC == CRC_NO_ENTRY_m10)
                printf("Block CRC: no entry\n");
        else {
                generate_hex_string_m10((ui1 *) &bh->block_CRC, CRC_BYTES_m10, hex_str);
                printf("Block CRC: %s\n", hex_str);
        }
        printf("Block Flag Bits: ");
        for (i = 0, mask = 1; i < 32; ++i, mask <<= 1) {
                if (bh->block_flags & mask)
                        printf("%d ", i);
        }
	printf(" (value: 0x%08x)\n", bh->block_flags);
        if (bh->start_time == UUTC_NO_ENTRY_m10)
                printf("Start Time: no entry\n");
        else {
                time_string_m10(bh->start_time, time_str, TRUE_m10, FALSE_m10, FALSE_m10);
                printf("Start Time: %ld (µUTC), %s\n", bh->start_time, time_str);
        }
        printf("Acquisition Channel Number: %u\n", bh->acquisition_channel_number);
        printf("Total Block Bytes: %u\n", bh->total_block_bytes);
	printf("Number of Samples: %u\n", bh->number_of_samples);
	printf("Number of Records: %hu\n", bh->number_of_records);
	printf("Parameter Flag Bits: ");
	for (i = 0, mask = 1; i < 32; ++i, mask <<= 1) {
		if (bh->parameter_flags & mask)
			printf("%d ", i);
	}
	printf(" (value: 0x%08x)\n", bh->parameter_flags);
	printf("Parameter Region Bytes: %hu\n", bh->parameter_region_bytes);
        printf("Protected Region Bytes: %hu\n", bh->protected_region_bytes);
        printf("Discretionary Region Bytes: %hu\n", bh->discretionary_region_bytes);
        printf("Model Region Bytes: %hu\n", bh->model_region_bytes);
	printf("Total Header Bytes: %u\n", bh->total_header_bytes);
        printf("---------------- CMP Fixed Block Header - END ----------------\n\n");
        
        return;
}


void    CMP_show_block_model_m10(CMP_BLOCK_FIXED_HEADER_m10 *block_header)
{
        ui1     *model, bits_per_sample, derivative_level, no_zero_counts_flag;
        si1     *symbols;
        ui2     number_of_statistics_bins, number_of_NIL_bins, number_of_POS_bins, number_of_NEG_bins, *counts;
        ui4     difference_bytes, variable_region_bytes;
        si4     minimum_value, initial_sample_value, i;
        

        variable_region_bytes = CMP_VARIABLE_REGION_BYTES_v1_m10(block_header);
        model = (ui1 *) block_header + CMP_BLOCK_FIXED_HEADER_BYTES_m10 + variable_region_bytes;

        printf("------------------- CMP Block Model - START ------------------\n");
        switch (block_header->block_flags & CMP_BF_ALGORITHM_MASK_m10) {
                case CMP_BF_RED_ENCODING_MASK_m10:
                        printf("Model: Range Encoded Differences (RED)\n");
                        initial_sample_value = *((si4 *) (model + CMP_RED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10));
                        printf("Initial Sample Value: %d\n", initial_sample_value);
                        difference_bytes = *((ui4 *) (model + CMP_RED_MODEL_DIFFERENCE_BYTES_OFFSET_m10));
                        printf("Difference Bytes: %u\n", difference_bytes);
			derivative_level = *((ui1 *) (model + CMP_RED_MODEL_DERIVATIVE_LEVEL_OFFSET_m10));
                        printf("Derivative Level: %hhu\n", derivative_level);
			no_zero_counts_flag = *((ui1 *) (model + CMP_RED_MODEL_NO_ZERO_COUNTS_FLAG_OFFSET_m10));
			printf("No Zero Counts Flag: %hhu\n", no_zero_counts_flag);
                        number_of_statistics_bins = *((ui2 *) (model + CMP_RED_MODEL_NUMBER_OF_STATISTICS_BINS_OFFSET_m10));
                        // end fixed RED model fields
                        counts = (ui2 *) (model + CMP_RED_MODEL_NUMBER_OF_STATISTICS_BINS_OFFSET_m10);
                        symbols = (si1 *) (counts + number_of_statistics_bins);
                        printf("\nNumber of Statistics Bins: %u\n", number_of_statistics_bins);
                        for (i = 0; i < number_of_statistics_bins; ++i)
                                printf("\tsymbol: %hhd\tcount: %hu\n", *symbols++, *counts++);
                        printf("\n");
                        break;
                        
                case CMP_BF_PRED_ENCODING_MASK_m10:
                        printf("Model: Predictive Range Encoded Differences (PRED)\n");
                        initial_sample_value = *((si4 *) (model + CMP_PRED_MODEL_INITIAL_SAMPLE_VALUE_OFFSET_m10));
                        printf("Initial Sample Value: %d\n", initial_sample_value);
                        difference_bytes = *((ui4 *) (model + CMP_PRED_MODEL_DIFFERENCE_BYTES_OFFSET_m10));
                        printf("Difference Bytes: %u\n", difference_bytes);
			derivative_level = *((ui1 *) (model + CMP_PRED_MODEL_DERIVATIVE_LEVEL_OFFSET_m10));
                        printf("Derivative Level: %hhu\n", derivative_level);
			no_zero_counts_flag = *((ui1 *) (model + CMP_PRED_MODEL_NO_ZERO_COUNTS_FLAG_OFFSET_m10));
			printf("No Zero Counts Flag: %hhu\n", no_zero_counts_flag);
                        number_of_NIL_bins = *((ui2 *) (model + CMP_PRED_MODEL_NUMBER_OF_NIL_STATISTICS_BINS_OFFSET_m10));
                        number_of_POS_bins = *((ui2 *) (model + CMP_PRED_MODEL_NUMBER_OF_POS_STATISTICS_BINS_OFFSET_m10));
                        number_of_NEG_bins = *((ui2 *) (model + CMP_PRED_MODEL_NUMBER_OF_NEG_STATISTICS_BINS_OFFSET_m10));
                        // end fixed PRED model fields
                        counts = (ui2 *) (model + CMP_PRED_MODEL_BIN_COUNTS_OFFSET_m10);
                        symbols = (si1 *) (counts + (number_of_NIL_bins + number_of_POS_bins + number_of_NEG_bins));
                        printf("\nNumber of NIL Statistics Bins: %u\n", number_of_NIL_bins);
                        for (i = 0; i < number_of_NIL_bins; ++i)
                                printf("bin %02d:    symbol: %hhd\tcount: %hu\n", i, *symbols++, *counts++);
                        printf("\nNumber of POS Statistics Bins: %u\n", number_of_POS_bins);
                        for (i = 0; i < number_of_POS_bins; ++i)
                                printf("bin %02d:    symbol: %hhd\tcount: %hu\n", i, *symbols++, *counts++);
                        printf("\nNumber of NEG Statistics Bins: %u\n", number_of_NEG_bins);
                        for (i = 0; i < number_of_NEG_bins; ++i)
                                printf("bin %02d:    symbol: %hhd\tcount: %hu\n", i, *symbols++, *counts++);
                        printf("\n");
                        break;
                        
                case CMP_BF_MBE_ENCODING_MASK_m10:
                        printf("Model: Minimal Bit Encoding (MBE)\n");
                        minimum_value = *((si4 *) (model + CMP_MBE_MODEL_MINIMUM_SAMPLE_VALUE_OFFSET_m10));
                        printf("Minimum Sample Value: %d\n", minimum_value);
                        bits_per_sample = *((ui1 *) (model + CMP_MBE_MODEL_BITS_PER_SAMPLE_OFFSET_m10));
                        printf("Bits per Sample: %hhu\n", bits_per_sample);
                        break;
                        
                default:
                        error_message_m10("%s(): Unrecognized Model (%u)", __FUNCTION__, block_header->block_flags & CMP_BF_ALGORITHM_MASK_m10);
                        break;
        }
        printf("-------------------- CMP Block Model - END -------------------\n\n");

        return;
}


void    CMP_unscale_amplitude_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf8 scale_factor)
{
        si4     *si4_p1, *si4_p2;
        si8     i;
        
        
        // unscale from input_buffer to output_buffer
        // if input_buffer == output_buffer unscaling will be done in place
        
        si4_p1 = input_buffer;
        si4_p2 = output_buffer;
        for (i = len; i--;)
                *si4_p2++ = CMP_round_m10((sf8) *si4_p1++ * scale_factor);
        
        return;
}


void    CMP_unscale_frequency_m10(si4 *input_buffer, si4 *output_buffer, si8 len, sf4 scale_factor)
{
	// not written yet
        return;
}


inline CMP_BLOCK_FIXED_HEADER_m10     *CMP_update_CPS_pointers_m10(CMP_PROCESSING_STRUCT_m10 *cps, ui1 flags)
{
        CMP_BLOCK_FIXED_HEADER_m10        *block_header;
        
        
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
//**************************  END CMP FUNCTIONS  ************************//
//***********************************************************************//


void	condition_time_slice_m10(TIME_SLICE_m10 *slice)
{
	si8	test_time;
	
	
	if (slice == NULL) {
		warning_message_m10("%s(): passed time slice is NULL");
		return;
	}

	if (globals_m10->RTO_known == FALSE_m10) {
		warning_message_m10("%s(): recording time offset is not known => assuming no offset", __FUNCTION__);
		globals_m10->recording_time_offset = 0;  // this is the default value
	}
	
	if (slice->start_time <= 0) {
		if (slice->start_time == UUTC_NO_ENTRY_m10) {
			if (slice->start_index == SAMPLE_NUMBER_NO_ENTRY_m10)
				slice->start_time = BEGINNING_OF_TIME_m10;
		} else {  // relative time
			slice->start_time = globals_m10->session_start_time - slice->start_time;
		}
	} else {  // ? unoffset time
		test_time = slice->start_time - globals_m10->recording_time_offset;
		if (test_time > 0)  // start time is not offset
			slice->start_time = test_time;
	}
	
	if (slice->end_time <= 0) {
		if (slice->end_time == UUTC_NO_ENTRY_m10) {
			if (slice->end_index == SAMPLE_NUMBER_NO_ENTRY_m10)
				slice->end_time = END_OF_TIME_m10;
		} else {  // relative time
			slice->end_time = globals_m10->session_start_time - slice->end_time;
		}
	} else {  // ? unoffset time
		test_time = slice->end_time - globals_m10->recording_time_offset;
		if (test_time > 0 && slice->end_time != END_OF_TIME_m10)  // end time is not offset
			slice->end_time = test_time;
	}
	
	slice->session_start_time = globals_m10->session_start_time;
	slice->conditioned = TRUE_m10;

	return;
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


void	CRC_initialize_table_m10(void)
{
	ui4	**crc_table, c, n, k;

        
        crc_table = (ui4 **) e_calloc_2D_m10((size_t) CRC_TABLES_m10, (size_t) CRC_TABLE_ENTRIES_m10, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);

        // generate a crc for every 8-bit value
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
        
	globals_m10->CRC_table = crc_table;
        
	return;
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
        ui4                     **crc_table;
        register ui4            c;
        register const ui4      *ui4_buf;
        
        
        if (globals_m10->CRC_table == NULL)
                CRC_initialize_table_m10();
	crc_table = globals_m10->CRC_table;

        c = ~current_crc;
        
        // bring block_ptr to 4 byte alignment
        while (block_bytes && ((ui8) block_ptr & (ui8) 3)) {
                c = crc_table[0][(c ^ *block_ptr++) & 0xFF] ^ (c >> 8);
                block_bytes--;
        }

        ui4_buf = (const ui4 *) block_ptr;
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


inline TERN_m10 CRC_validate_m10(const ui1 *block_ptr, si8 block_bytes, ui4 crc_to_validate)
{
	ui4	crc;	
	
	crc = CRC_calculate_m10(block_ptr, block_bytes);
	
	if (crc == crc_to_validate)
		return(TRUE_m10);
	
	return(FALSE_m10);
}


//***********************************************************************//
//*************************  END CRC FUNCTIONS  *************************//
//***********************************************************************//


inline si8      current_uutc_m10(void)
{
        struct timeval  tv;
        si8             uutc;
        
        gettimeofday(&tv, NULL);
        uutc = (si8) tv.tv_sec * (si8) 1000000 + (si8) tv.tv_usec;
 
        return(uutc);
}


inline si4      days_in_month_m10(si4 month, si4 year)
// Note month is [0 - 11], January == 0, as in struct tm.tm_mon
// Note struct tm.tm_year is (year - 1900), this function expects the full value
{
        static const si4        standard_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
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
	ui1		        *ui1_p, *decryption_key;
	si4		        i, decryption_blocks;
        PASSWORD_DATA_m10	*pwd;
        METADATA_SECTION_3_m10  *section_3;
        

        if (fps == NULL) {
                error_message_m10("%s(): FILE_PROCESSING_STRUCT is NULL", __FUNCTION__);
                return(FALSE_m10);
        }
        
	pwd = fps->password_data;
	// section 2 decryption
        if (fps->metadata.section_1->section_2_encryption_level > NO_ENCRYPTION_m10) {  // natively encrypted and currently encrypted
                if (pwd->access_level >= fps->metadata.section_1->section_2_encryption_level) {
                        if (fps->metadata.section_1->section_2_encryption_level == LEVEL_1_ENCRYPTION_m10)
                                decryption_key = pwd->level_1_encryption_key;
                        else
                                decryption_key = pwd->level_2_encryption_key;
                        decryption_blocks = METADATA_SECTION_2_BYTES_m10 / ENCRYPTION_BLOCK_BYTES_m10;
                        ui1_p = fps->raw_data + METADATA_SECTION_2_OFFSET_m10;
                        for (i = 0; i < decryption_blocks; ++i) {
                                AES_decrypt_m10(ui1_p, ui1_p, NULL, decryption_key);
                                ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
                        }
                        fps->metadata.section_1->section_2_encryption_level = -fps->metadata.section_1->section_2_encryption_level;  // mark as currently decrypted
                } else {
                        error_message_m10("%s(): Section 2 of the Metadata is encrypted at level %hhd => cannot decrypt", __FUNCTION__, fps->metadata.section_1->section_2_encryption_level);
			show_password_data_m10(pwd);
			return(FALSE_m10);  // can't do anything without section 2, so fail
                }
        }
	
	// section 3 decryption
        if (fps->metadata.section_1->section_3_encryption_level > NO_ENCRYPTION_m10) {  // natively encrypted and currently encrypted
                if (pwd->access_level >= fps->metadata.section_1->section_3_encryption_level) {
                        if (fps->metadata.section_1->section_3_encryption_level == LEVEL_1_ENCRYPTION_m10)
                                decryption_key = pwd->level_1_encryption_key;
                        else
                                decryption_key = pwd->level_2_encryption_key;
                        decryption_blocks = METADATA_SECTION_3_BYTES_m10 / ENCRYPTION_BLOCK_BYTES_m10;
                        ui1_p = fps->raw_data + METADATA_SECTION_3_OFFSET_m10;
                        for (i = 0; i < decryption_blocks; ++i) {
                                AES_decrypt_m10(ui1_p, ui1_p, NULL, decryption_key);
                                ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
                        }
                        fps->metadata.section_1->section_3_encryption_level = -fps->metadata.section_1->section_3_encryption_level;  // mark as currently decrypted
		} else {
			if (globals_m10->verbose == TRUE_m10) {
				warning_message_m10("%s(): Metadata section 3 encrypted at level %hhd => cannot decrypt", __FUNCTION__, fps->metadata.section_1->section_3_encryption_level);
				show_password_data_m10(pwd);
			}
			return(TRUE_m10);  // can function without section 3, so return TRUE_m10
                }
        }

        // set global time data
	if (globals_m10->RTO_known == FALSE_m10) {
		section_3 = fps->metadata.section_3;
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
                ui1_p = (ui1 *) record_header;
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
                        } else {
				++failed_decryption_count;
				if (globals_m10->verbose == TRUE_m10)
                                	warning_message_m10("%s(): Cannot decrypt record => skipping", __FUNCTION__);
                        }
                }
                record_header = (RECORD_HEADER_m10 *) ((ui1 *) record_header + record_header->total_record_bytes);
        }
	
	if (failed_decryption_count == number_of_items)  // failure == all records unreadable
		return(FALSE_m10);
	
        return(TRUE_m10);
}


TERN_m10     decrypt_time_series_data_m10(CMP_PROCESSING_STRUCT_m10 *cps, si8 number_of_items)
{
        ui1                             *ui1_p, *key;
        si4                             encryption_blocks, encryptable_blocks;
        si8                             i, encryption_bytes;
        CMP_BLOCK_FIXED_HEADER_m10      *bh;
	PASSWORD_DATA_m10		*pwd;
        
        
        if (number_of_items == 0)
                return(TRUE_m10);
        
        
        // get decryption key - assume all blocks encrypted at same level
	pwd = cps->password_data;
        bh = cps->block_header;
        for (i = 0; i < number_of_items; ++i) {
                // check if already decrypted
                if ((bh->block_flags & CMP_BF_ENCRYPTION_MASK_m10) == 0) {
                        bh = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *) bh + bh->total_block_bytes);
                        continue;
                }
                if  (bh->block_flags & CMP_BF_LEVEL_1_ENCRYPTION_MASK_m10) {
                       if  (bh->block_flags & CMP_BF_LEVEL_2_ENCRYPTION_MASK_m10) {
                               error_message_m10("%s(): Cannot decrypt data: flags indicate both level 1 & level 2 encryption", __FUNCTION__);
                               return(FALSE_m10);
                       }
                       if (pwd->access_level >= LEVEL_1_ENCRYPTION_m10) {
                               key = pwd->level_1_encryption_key;
                               break;
                       } else {
                               error_message_m10("%s(): Cannot decrypt data: insufficient access", __FUNCTION__);
                               return(FALSE_m10);
                       }
                } else {  // level 2 bit is set
                        if (pwd->access_level == LEVEL_2_ENCRYPTION_m10) {
                                key = pwd->level_2_encryption_key;
                                break;
                        } else {
                                error_message_m10("%s(): Cannot decrypt data: insufficient access", __FUNCTION__);
                                return(FALSE_m10);
                        }
                }
                bh = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *) bh + bh->total_block_bytes);
        }
        
        // no blocks encrypted
        if (i == number_of_items)
                return(TRUE_m10);

        // decrypt
        for (i = 0; i < number_of_items; ++i) {
                
                // block if already decrypted
                if ((bh->block_flags & CMP_BF_ENCRYPTION_MASK_m10) == 0) {
                        bh = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *) bh + bh->total_block_bytes);
                        continue;
                }
                
                // calculated encyrption blocks
                encryptable_blocks = (bh->total_block_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10) / ENCRYPTION_BLOCK_BYTES_m10;
                if (bh->block_flags | CMP_BF_MBE_ENCODING_MASK_m10) {
                        encryption_blocks = encryptable_blocks;
                } else {
                        encryption_bytes = bh->total_header_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10 + ENCRYPTION_BLOCK_BYTES_m10;
                        encryption_blocks = ((encryption_bytes - 1) / ENCRYPTION_BLOCK_BYTES_m10) + 1;
                        if (encryptable_blocks < encryption_blocks)
                                encryption_blocks = encryptable_blocks;
                }
                
                // decrypt
                ui1_p = (ui1 *) bh + CMP_BLOCK_ENCRYPTION_START_OFFSET_m10;
                for (i = 0; i < encryption_blocks; ++i) {
                        AES_decrypt_m10(ui1_p, ui1_p, NULL, key);
                        ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
                }
                
                // set block flags to decrypted
                bh->block_flags &= ~CMP_BF_ENCRYPTION_MASK_m10;
                
                // set pointer to next block
                bh = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *) bh + bh->total_block_bytes);
        }

        return(TRUE_m10);
}


si4     DST_offset_m10(si8 uutc)
{
        si8                             utc, local_utc, change_utc;
        si4                             i, month, DST_start_month, DST_end_month;
        si4                             first_weekday_of_month, target_day_of_month, last_day_of_month;
        struct tm                       time_info, change_time_info = {0};
        DAYLIGHT_TIME_CHANGE_CODE_m10   *first_DTCC, *last_DTCC, *change_DTCC;
        
        
        // returns seconds to add to standard time (as UUTC) to adjust for DST on that date, in the globally specified timezone
        
	if (globals_m10->daylight_time_start_code.value == DTCC_VALUE_NO_ENTRY_m10) {
		warning_message_m10("%s(): daylight change data not available", __FUNCTION__);
		return(0);
	}
		
        if (globals_m10->observe_DST == FALSE_m10)
                return(0);
        
        utc = uutc / (si8) 1000000;
        
        // get broken out time info
        if (globals_m10->daylight_time_start_code.reference_time == DTCC_LOCAL_REFERENCE_TIME) {
                local_utc = utc + (si8) globals_m10->standard_UTC_offset;
                gmtime_r(&local_utc, &time_info);
        } else {
                gmtime_r(&utc, &time_info);
        }

        month = time_info.tm_mon;
        DST_start_month = globals_m10->daylight_time_start_code.month;
        DST_end_month = globals_m10->daylight_time_end_code.month;
        if (DST_start_month < DST_end_month) {
                first_DTCC = &globals_m10->daylight_time_start_code;
                last_DTCC = &globals_m10->daylight_time_end_code;
        } else {
                first_DTCC = &globals_m10->daylight_time_end_code;
                last_DTCC = &globals_m10->daylight_time_start_code;
        }
        

        // take care of dates not in change months
        if (month != DST_start_month && month != DST_end_month) {
                if (month > first_DTCC->month && month < last_DTCC->month) {
                        if (first_DTCC->month == DST_start_month)
                                return((si4) first_DTCC->shift_minutes * (si4) 60);
                        else
                                return(0);
                } else if (month < first_DTCC->month) {
                       if (first_DTCC->month == DST_start_month)
                               return(0);
                       else
                               return((si4) first_DTCC->shift_minutes * (si4) 60);
                } else {  // month > last_DTCC->month
                        if (last_DTCC->month == DST_end_month)
                                return(0);
                        else
                                return((si4) first_DTCC->shift_minutes * (si4) 60);
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
                } else {
                        for (i = 1; i < change_DTCC->relative_weekday_of_month; ++i)
                                target_day_of_month += 7;
                }
                change_time_info.tm_mday = target_day_of_month;
        } else {
                change_time_info.tm_mday = change_DTCC->day_of_month;
        }
        
        change_utc = timegm(&change_time_info);
        if (globals_m10->daylight_time_start_code.reference_time == DTCC_LOCAL_REFERENCE_TIME)
                change_utc -= globals_m10->standard_UTC_offset;
        
        if (change_DTCC->month == DST_start_month) {
                if (utc >= change_utc)
                        return((si4) change_DTCC->shift_minutes * (si4) 60);
                else
                        return(0);
        } else {  // change_DTCC->month == DST_end_month
                if (utc < change_utc)
                        return((si4) change_DTCC->shift_minutes * (si4) -60);
                else
                        return(0);
        }
                
        return(0);
}


//***********************************************************************//
//****************  ERROR CHECKING STANDARD FUNCTIONS  ******************//
//***********************************************************************//

// Note: error handling in these functions does not use error_message_m10() so they can remain thread-safe


void	*e_calloc_m10(size_t n_members, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	void	*ptr;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if ((ptr = calloc(n_members, el_size)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void) fprintf(stderr, "%c\n\t%s() failed to allocate the requested array (%ld members of size %ld)\n", 7, __FUNCTION__, n_members, el_size);
			(void) fprintf(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void) fprintf(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> returning NULL\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit(1);
	}
	
	return(ptr);
}


void    **e_calloc_2D_m10(size_t dim1, size_t dim2, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail)
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
	ptr = (ui1 **) e_malloc_m10(total_bytes, function, line, behavior_on_fail);
	ptr[0] = (ui1 *) (ptr + dim1);
	memset((void *) ptr[0], 0, content_bytes);  // remove this line to make e_malloc_2D_m10()
	
	for (i = 1; i < dim1; ++i)
		ptr[i] = ptr[i - 1] + dim2_bytes;

	return((void **) ptr);
}


FILE	*e_fopen_m10(si1 *path, si1 *mode, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	FILE	 *fp;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if ((fp = fopen(path, mode)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void) UTF8_fprintf_m10(stderr, "%c\n\t%s() failed to open file \"%s\"\n", 7, __FUNCTION__, path);
			(void) fprintf(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void) fprintf(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> returning NULL\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit(1);
	}

	return(fp);
}


size_t	e_fread_m10(void *ptr, size_t size, size_t n_members, FILE *stream, si1 *path, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	size_t	nr;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if ((nr = fread(ptr, size, n_members, stream)) != n_members) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void) UTF8_fprintf_m10(stderr, "%c\n\t%s() failed to read file \"%s\"\n", 7, __FUNCTION__, path);
			(void) fprintf(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void) fprintf(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> returning number of items read\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(nr);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit(1);
	}
		
	return(nr);
}


void    e_free_m10(void *ptr, const si1 *function, si4 line)
{
	if (ptr == NULL) {
		warning_message_m10("%s(): Attempting to free unallocated object [called from function %s(), line %d]", __FUNCTION__, function, line);
		return;
	}
	
	free(ptr);
	
	return;
}


void    e_free_2D_m10(void **ptr, si8 dim1, const si1 *function, si4 line)
{
	si8     i;
  
	
	if (ptr == NULL)
		return;
	
	// assume allocated en bloc
	if (dim1 == 0) {
		warning_message_m10("%s(): assuming allocated en bloc", __FUNCTION__);
		e_free_m10(ptr, function, line);
		return;
	}
	
	// allocated en bloc
	if ((ui8) ptr[0] == ((ui8) ptr + ((ui8) dim1 * (ui8) sizeof(void *)))) {
		e_free_m10(ptr, function, line);
		return;
	}

	// separately allocated
	for (i = 0; i < dim1; ++i) {
		if (ptr[i] == NULL)
			continue;
		e_free_m10(ptr[i], function, line);
	}
	e_free_m10(ptr, function, line);
		
	return;
}


si4	e_fseek_m10(FILE *stream, size_t offset, si4 whence, si1 *path, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if ((fseek(stream, offset, whence)) == -1) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void) fprintf(stderr, "%c\n\t%s() failed to move the file pointer to requested location (offset %ld, whence %d)\n", 7, __FUNCTION__, offset, whence);
			(void) UTF8_fprintf_m10(stderr, "%\tin file \"%s\"\n", path);
			(void) fprintf(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void) fprintf(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> returning -1\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(-1);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit(1);
	}
	
	return(0);
}


long	e_ftell_m10(FILE *stream, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	long	pos;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if ((pos = ftell(stream)) == -1) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void) fprintf(stderr, "%c\n\t%s() failed obtain the current location\n", 7, __FUNCTION__);
			(void) fprintf(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void) fprintf(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> returning -1\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(-1);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit(1);
	}
	
	return(pos);
}


size_t	e_fwrite_m10(void *ptr, size_t size, size_t n_members, FILE *stream, si1 *path, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	size_t	nw;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if ((nw = fwrite(ptr, size, n_members, stream)) != n_members) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void) UTF8_fprintf_m10(stderr, "%c\n\t%s() failed to write file \"%s\"\n", 7, __FUNCTION__, path);
			(void) fprintf(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void) fprintf(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> returning number of items written\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(nw);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit(1);
	}
	
	return(nw);
}


void	*e_malloc_m10(size_t n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	void	 *ptr;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if ((ptr = malloc(n_bytes)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void) fprintf(stderr, "%c\n\t%s() failed to allocate the requested array (%ld bytes)\n", 7, __FUNCTION__, n_bytes);
			(void) fprintf(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void) fprintf(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> returning NULL\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(NULL);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit(1);
	}

	return(ptr);
}


void	*e_realloc_m10(void *orig_ptr, size_t n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	void    *ptr;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if ((ptr = realloc(orig_ptr, n_bytes)) == NULL) {
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void) fprintf(stderr, "%c\n\t%s() failed to reallocate the requested array (%ld bytes)\n", 7, __FUNCTION__, n_bytes);
			(void) fprintf(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			if (function != NULL)
				(void) fprintf(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> returning unreallocated pointer\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10)
			return(orig_ptr);
		else if (behavior_on_fail & EXIT_ON_FAIL_m10)
			exit(1);
	}
		
	return(ptr);
}


void    **e_realloc_2D_m10(void **curr_ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	si8             i;
	void            **new_ptr;
	size_t          least_dim1, least_dim2;
	

	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	// Returns pointer to a reallocated 2 dimensional array of new_dim1 by new_dim2 elements of size el_size (new unused elements are zeroed)
	// ptr[0] points to a one dimensional array of size (dim1 * dim2)
	// The whole block can be freed with free(ptr)
	if (curr_ptr == NULL) {
		error_message_m10("%s(): attempting to re-allocate NULL pointer (called from function %s(), line %d)\n", __FUNCTION__, function, line);
		return(NULL);
	}

	if (new_dim1 < curr_dim1)
		warning_message_m10("%s(): re-allocating first dimension to smaller size (called from function %s(), line %d)\n", __FUNCTION__, function, line);
	if (new_dim2 < curr_dim2)
		warning_message_m10("%s(): re-allocating second dimension to smaller size (called from function %s(), line %d)\n", __FUNCTION__, function, line);
	
	new_ptr = e_calloc_2D_m10(new_dim1, new_dim2, el_size, function, line, behavior_on_fail);
	
	least_dim1 = (curr_dim1 <= new_dim1) ? curr_dim1 : new_dim1;
	least_dim2 = (curr_dim2 <= new_dim2) ? curr_dim2 : new_dim2;
	for (i = 0; i < least_dim1; ++i)
		memcpy((void *) new_ptr[i], curr_ptr[i], (size_t) (least_dim2 * el_size));
	
	e_free_2D_m10(curr_ptr, curr_dim1, function, line);

	return((void **) new_ptr);
}


si4     e_system_m10(si1 *command, TERN_m10 null_std_streams, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	si1             *temp_command;
	si4             ret_val, len;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	if (null_std_streams == TRUE_m10) {
		len = strlen(command);
		temp_command = e_malloc_m10(len + 18, function, line, behavior_on_fail);
		sprintf(temp_command, "%s > /dev/null 2>&1", command);
	} else {
		temp_command = command;
	}

	ret_val = system(temp_command);
	
	if (ret_val) {
		if (behavior_on_fail & RETRY_ONCE_m10) {
			usleep((useconds_t) 1000);  // wait 1 ms
			if ((ret_val = system(temp_command)) == 0) {
				if (null_std_streams == TRUE_m10)
					e_free_m10(temp_command, __FUNCTION__, __LINE__);
				return(0);
			}
		}
		if (!(behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
			(void) fprintf(stderr, "%c\n%s() failed\n", 7, __FUNCTION__);
			(void) fprintf(stderr, "\tcommand: \"%s\"\n", temp_command);
			(void) fprintf(stderr, "\tsystem error number %d (%s)\n", errno, strerror(errno));
			(void) fprintf(stderr, "\tshell return value %d\n", ret_val);
			if (function != NULL)
				(void) fprintf(stderr, "\tcalled from function \"%s\", line %d\n", function, line);
			if (behavior_on_fail & RETURN_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> returning -1\n\n");
			else if (behavior_on_fail & EXIT_ON_FAIL_m10)
				(void) fprintf(stderr, "\t=> exiting program\n\n");
			fflush(stderr);
		}
		if (behavior_on_fail & RETURN_ON_FAIL_m10) {
			if (null_std_streams == TRUE_m10)
				e_free_m10(temp_command, __FUNCTION__, __LINE__);
			return(-1);
		} else if (behavior_on_fail & EXIT_ON_FAIL_m10) {
			exit(1);
		}
	}
	
	if (null_std_streams == TRUE_m10)
		e_free_m10(temp_command, __FUNCTION__, __LINE__);

	return(0);
}


//***********************************************************************//
//**************  END ERROR CHECKING STANDARD FUNCTIONS  ****************//
//***********************************************************************//


TERN_m10	encrypt_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps)
{
	ui1		        *ui1_p, *encryption_key;
	si4		        i, encryption_blocks;
        PASSWORD_DATA_m10	*pwd;
        
        
	// section 2 encrypt
	pwd = fps->password_data;
	if (fps->metadata.section_1->section_2_encryption_level < NO_ENCRYPTION_m10) {  // natively encrypted and currently decrypted
		if (pwd->access_level >= -fps->metadata.section_1->section_2_encryption_level) {
			fps->metadata.section_1->section_2_encryption_level = -fps->metadata.section_1->section_2_encryption_level;  // mark as currently encrypted
			if (fps->metadata.section_1->section_2_encryption_level == LEVEL_1_ENCRYPTION_m10)
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
	if (fps->metadata.section_1->section_3_encryption_level < NO_ENCRYPTION_m10) {  // natively encrypted and currently decrypted
		if (pwd->access_level >= -fps->metadata.section_1->section_3_encryption_level) {
			fps->metadata.section_1->section_3_encryption_level = -fps->metadata.section_1->section_3_encryption_level;  // mark as currently encrypted
			if (fps->metadata.section_1->section_3_encryption_level == LEVEL_1_ENCRYPTION_m10)
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
        ui1		        *ui1_p, *encryption_key;
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
        ui1                             *ui1_p, *key;
        ui4                             encryption_mask;
        si4                             encryption_blocks, encryptable_blocks;
        si8                             i, encryption_bytes;
        PASSWORD_DATA_m10               *pwd;
	CMP_BLOCK_FIXED_HEADER_m10 	*block_header;
        
        
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
                        } else {
                                key = pwd->level_2_encryption_key;
                                encryption_mask = CMP_BF_LEVEL_2_ENCRYPTION_MASK_m10;
                        }
                } else {
                        error_message_m10("%s(): Cannot encrypt data => returning without encrypting\n", __FUNCTION__);
                        cps->directives.encryption_level = NO_ENCRYPTION_m10;
                        return(FALSE_m10);
                }
        } else {
                return(TRUE_m10);
        }

        for (i = 0; i < number_of_items; ++i) {
                encryptable_blocks = (block_header->total_block_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10) / ENCRYPTION_BLOCK_BYTES_m10;
                if (block_header->block_flags | CMP_BF_MBE_ENCODING_MASK_m10) {
                        encryption_blocks = encryptable_blocks;
                } else {
                        encryption_bytes = block_header->total_header_bytes - CMP_BLOCK_ENCRYPTION_START_OFFSET_m10 + ENCRYPTION_BLOCK_BYTES_m10;
                        encryption_blocks = ((encryption_bytes - 1) / ENCRYPTION_BLOCK_BYTES_m10) + 1;
                        if (encryptable_blocks < encryption_blocks)
                                encryption_blocks = encryptable_blocks;
                }
                ui1_p = (ui1 *) block_header + CMP_BLOCK_ENCRYPTION_START_OFFSET_m10;
                for (i = 0; i < encryption_blocks; ++i) {
                        AES_encrypt_m10(ui1_p, ui1_p, NULL, key);
                        ui1_p += ENCRYPTION_BLOCK_BYTES_m10;
                }
                block_header->block_flags |= encryption_mask;
                block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *) block_header + block_header->total_block_bytes);
        }
        
        return(TRUE_m10);
}


void    error_message_m10(si1 *fmt, ...)
{
	va_list args;


	if (!(globals_m10->behavior_on_fail & SUPPRESS_ERROR_OUTPUT_m10)) {
		va_start(args, fmt);
		UTF8_vfprintf_m10(stderr, fmt, args);
		va_end(args);
		if (globals_m10->behavior_on_fail & EXIT_ON_FAIL_m10)
			fprintf(stderr, " => exiting program\n\n");
		else
			fprintf(stderr, " => returning\n\n");
		fflush(stderr);
	}

	if (globals_m10->behavior_on_fail & EXIT_ON_FAIL_m10)
		exit(1);
	
	return;
}


void    escape_spaces_m10(si1 *string, si8 buffer_len)
{
	si1     *c1, *c2, *tmp_str;
	si8     spaces, len;

	
	// count
	for (spaces = 0, c1 = string; *c1++;)
		if (*c1 == 0x20)
			if (*(c1 - 1) != 0x5c)
				++spaces;
	len = (c1 - string) + spaces;
	if (buffer_len != 0) {  // if zero, proceed at caller's peril
		if (buffer_len < len) {
			error_message_m10("%s(): string buffer too small", __FUNCTION__);
			return;
		}
	}
	
	tmp_str = (si1 *) e_malloc_m10(len, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
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
	
	e_free_m10(tmp_str, __FUNCTION__, __LINE__);

	return;
}


void	extract_path_parts_m10(si1 *full_file_name, si1 *path, si1 *name, si1 *extension)
{
	si1	*c, *cc, cwd[FULL_FILE_NAME_BYTES_m10], temp_full_file_name[FULL_FILE_NAME_BYTES_m10];
	
        
	// check that path starts from root
        if (*full_file_name == '/') {
		strncpy_m10(temp_full_file_name, full_file_name, FULL_FILE_NAME_BYTES_m10);  // do non-destructively
	} else {
                warning_message_m10("%s(): path \"%s\" does not start from root => prepending current working directory", __FUNCTION__, full_file_name);
                getcwd(cwd, FULL_FILE_NAME_BYTES_m10);
		c = full_file_name;
                if (*c == '.') {
                        ++c;
                        if (*c == '/')
                                snprintf_m10(temp_full_file_name, FULL_FILE_NAME_BYTES_m10, "%s%s", cwd, c);
                        else if (*c == 0)
                                snprintf_m10(temp_full_file_name, FULL_FILE_NAME_BYTES_m10, "%s", cwd);
                        else
                                snprintf_m10(temp_full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s", cwd, c - 1);
                } else {
                        snprintf_m10(temp_full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s", cwd, c);
                }
	}

	// move pointer to end of string
	c = temp_full_file_name + strlen(temp_full_file_name) - 1;
        
	// remove terminal "/" if present
	if (*c == '/')
		*c-- = 0;
	
	// step back to first extension
	cc = c;
	while (*--c != '.') {
		if (*c == '/') {
			c = cc;
			break;
		}
	}
	
	// copy extension if allocated
	if (extension != NULL) {
                if (*c == '.')
			strncpy_m10(extension, c + 1, TYPE_BYTES_m10);
                else
			*extension = 0;
	}
	if (*c == '.')
		*c-- = 0;
        
	// step back to next directory break
	while (*--c != '/');
	
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
        si1     *s;     // terminal (most unique) bytes of UTF-8 password
	si4     i, j;
	ui4     ch;
	
        
        s = password;
        i = j = 0;
        do {
                ch = UTF8_nextchar_m10(s, &i);
                password_bytes[j++] = (ui1) (ch & 0xFF);
        } while (ch);
        for (; j < PASSWORD_BYTES_m10; ++j)
                password_bytes[j] = 0;
        
        return;
}


ui4     file_exists_m10(si1 *path)  // can be used for directories also
{
	si1             temp_path[FULL_FILE_NAME_BYTES_m10], *c, cwd[FULL_FILE_NAME_BYTES_m10];
	si4             err;
	struct stat     s;
	
	
	if (path == NULL)
		return(DOES_NOT_EXIST_m10);
	
	if (*path == 0)
		return(DOES_NOT_EXIST_m10);

	if (*path != '/') {
		c = path;
		getcwd(cwd, FULL_FILE_NAME_BYTES_m10);
		if (*c == '.') {
			++c;
			if (*c == '/')
				snprintf_m10(temp_path, FULL_FILE_NAME_BYTES_m10, "%s%s", cwd, c);
			else if (*c == 0)
				snprintf_m10(temp_path, FULL_FILE_NAME_BYTES_m10, "%s", cwd);
			else
				snprintf_m10(temp_path, FULL_FILE_NAME_BYTES_m10, "%s/%s", cwd, c - 1);
		} else {
			snprintf_m10(temp_path, FULL_FILE_NAME_BYTES_m10, "%s/%s", cwd, c);
		}
		path = temp_path;
	}
	
	errno = 0;
	err = stat(path, &s);
	if (err == -1) {
		if (errno == ENOENT)
			return(DOES_NOT_EXIST_m10);
	} else if (S_ISDIR(s.st_mode))
		return(DIR_EXISTS_m10);

	return(FILE_EXISTS_m10);
}


si8	*find_discontinuities_m10(TIME_SERIES_INDEX_m10 *tsi, si8 *num_disconts, si8 number_of_indices, TERN_m10 remove_offsets, TERN_m10 return_sample_numbers)
{
	si8	i, j, *disconts;
	
	
	for (i = *num_disconts = 0; i < number_of_indices; ++i)
		if (tsi[i].file_offset < 0)
			++(*num_disconts);

	disconts = (si8 *) e_malloc_m10(*num_disconts * sizeof(si8), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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


//***********************************************************************//
//************  FILE PROCESSING STRUCT STANDARD FUNCTIONS  **************//
//***********************************************************************//


void	fps_close_m10(FILE_PROCESSING_STRUCT_m10 *fps) {
	
	fclose(fps->fp);
	fps->fp = NULL;
	fps->fd = -1;
	
	return;
}


si4	fps_lock_m10(FILE_PROCESSING_STRUCT_m10 *fps, si4 lock_type, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	struct flock	fl;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	fl.l_type = lock_type;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_pid = getpid();
	if (fcntl(fps->fd, F_SETLKW, &fl) == -1) {
		error_message_m10("%s(): fcntl() failed to lock file\n\tsystem error: %s (# %d)\n\tcalled from function %s(), line %d", __FUNCTION__, strerror(errno), errno, function, line);
		return(-1);
	}
	
	return(0);
}


inline void fps_mutex_off_m10(FILE_PROCESSING_STRUCT_m10 *fps)
{
        fps->mutex = FALSE_m10;
        
        return;
}


inline void fps_mutex_on_m10(FILE_PROCESSING_STRUCT_m10 *fps)
{
        while (fps->mutex == TRUE_m10);
        fps->mutex = TRUE_m10;
        
        return;
}


si4	fps_open_m10(FILE_PROCESSING_STRUCT_m10 *fps, const si1 *function, si4 line, ui4 behavior_on_fail)
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
	switch(fps->directives.open_mode) {
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
			error_message_m10("%s(): invalid open mode (%u)\n\tcalled from function %s(), line %d", __FUNCTION__, fps->directives.open_mode, function, line);
			return(-1);
	}
	
	fps->fp = e_fopen_m10(fps->full_file_name, mode, function, line, RETURN_ON_FAIL_m10 | SUPPRESS_ERROR_OUTPUT_m10);
	if (fps->fp == NULL && errno == ENOENT && create_file == TRUE_m10) {
		// A component of the required directory tree does not exist - build it & try again
		extract_path_parts_m10(fps->full_file_name, path, name, extension);
		sprintf(command, "mkdir -p \"%s\"", path);
                e_system_m10(command, TRUE_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		fps->fp = e_fopen_m10(fps->full_file_name, mode, function, line, behavior_on_fail);
	}
	if (fps->fp == NULL) {
		error_message_m10("%s(): failed to open file \"%s\"\n\tcalled from function %s(), line %d", __FUNCTION__, fps->full_file_name, function, line);
		return(-1);
	}
	
	// get file descriptor
	fps->fd = fileno(fps->fp);
	
	// lock
	if (fps->directives.lock_mode != FPS_NO_LOCK_MODE_m10) {
		lock_type = FPS_NO_LOCK_TYPE_m10;
		if (fps->directives.open_mode == FPS_R_OPEN_MODE_m10) {
			if (fps->directives.lock_mode & FPS_READ_LOCK_ON_READ_OPEN_m10)
				lock_type = F_RDLCK;
			else if (fps->directives.lock_mode & FPS_WRITE_LOCK_ON_READ_OPEN_m10)
				lock_type = F_WRLCK;
		} else if (fps->directives.lock_mode & (FPS_WRITE_LOCK_ON_WRITE_OPEN_m10 | FPS_WRITE_LOCK_ON_READ_WRITE_OPEN_m10)) {
			lock_type = F_WRLCK;
		} else {
			error_message_m10("%s(): incompatible lock (%u) and open (%u) modes\n\tcalled from function %s(), line %d", __FUNCTION__, fps->directives.lock_mode, fps->directives.open_mode, function, line);
			return(-1);
		}
		fps_lock_m10(fps, lock_type, function, line, behavior_on_fail);
	}
	
	// get file length
	fstat(fps->fd, &sb);
	fps->file_length = sb.st_size;
		
	return(0);
}


si4     fps_read_m10(FILE_PROCESSING_STRUCT_m10 *fps, si8 in_bytes, void *ptr, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	// lock
	if (fps->directives.lock_mode & FPS_READ_LOCK_ON_READ_m10)
		fps_lock_m10(fps, F_RDLCK, function, line, behavior_on_fail);
	
	// read
	e_fread_m10(ptr, sizeof(ui1), (size_t) in_bytes, fps->fp, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);

	// unlock
	if (fps->directives.lock_mode & FPS_READ_LOCK_ON_READ_m10)
		fps_unlock_m10(fps, function, line, behavior_on_fail);
	
	return(0);
}


si4	fps_unlock_m10(FILE_PROCESSING_STRUCT_m10 *fps, const si1 *function, si4 line, ui4 behavior_on_fail)
{
	struct flock	fl;
	
	
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
	
	fl.l_type = F_UNLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_pid = getpid();
	if (fcntl(fps->fd, F_SETLKW, &fl) == -1) {
		error_message_m10("%s(): fcntl() failed to unlock file\n\tsystem error: %s (# %d)\n\tcalled from function %s(), line %d", __FUNCTION__, strerror(errno), errno, function, line);
		return(-1);
	}
	
	return(0);
}


si4	fps_write_m10(FILE_PROCESSING_STRUCT_m10 *fps, si8 out_bytes, void *ptr, const si1 *function, si4 line, ui4 behavior_on_fail)
{
        UNIVERSAL_HEADER_m10    *uh;
        struct stat             sb;

        
	if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
		behavior_on_fail = globals_m10->behavior_on_fail;
        
	// lock
	if (fps->directives.lock_mode & FPS_WRITE_LOCK_ON_WRITE_m10)
		fps_lock_m10(fps, F_WRLCK, function, line, behavior_on_fail);
	
	// write
	if (out_bytes == FPS_FULL_FILE_m10)
		out_bytes = fps->raw_data_bytes;
	e_fwrite_m10(ptr, sizeof(ui1), (size_t) out_bytes, fps->fp, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);
        if (fps->directives.flush_after_write == TRUE_m10)
                fflush(fps->fp);  // fflush updates stat structure
        fstat(fps->fd, &sb);
        fps->file_length = sb.st_size;
        
	// unlock
	if (fps->directives.lock_mode & FPS_WRITE_LOCK_ON_WRITE_m10)
		fps_unlock_m10(fps, function, line, behavior_on_fail);
        
        // update universal header, if requested
        if (fps->directives.update_universal_header == TRUE_m10) {
                uh = fps->universal_header;
                
                // update universal_header->file_CRC
                if (uh->body_CRC == CRC_NO_ENTRY_m10)
                        uh->body_CRC = CRC_calculate_m10((ui1 *) uh + UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10, fps->file_length - UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10);
                uh->header_CRC = CRC_calculate_m10((ui1 *) uh + UNIVERSAL_HEADER_HEADER_CRC_START_OFFSET_m10, UNIVERSAL_HEADER_BYTES_m10 - UNIVERSAL_HEADER_HEADER_CRC_START_OFFSET_m10);

                // lock
                if (fps->directives.lock_mode & FPS_WRITE_LOCK_ON_WRITE_m10)
                        fps_lock_m10(fps, F_WRLCK, function, line, behavior_on_fail);
                
                // write
                e_fseek_m10(fps->fp, 0, SEEK_SET, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);
                e_fwrite_m10(fps->raw_data, sizeof(ui1), (size_t) UNIVERSAL_HEADER_BYTES_m10, fps->fp, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);
                if (fps->directives.flush_after_write == TRUE_m10)
                        fflush(fps->fp);    // fflush updates stat structure
                e_fseek_m10(fps->fp, 0, SEEK_END, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);

                // unlock
                if (fps->directives.lock_mode & FPS_WRITE_LOCK_ON_WRITE_m10)
                        fps_unlock_m10(fps, function, line, behavior_on_fail);
        }
        	
	return(0);
}


//***********************************************************************//
//**********  END FILE PROCESSING STRUCT STANDARD FUNCTIONS  ************//
//***********************************************************************//


void	free_channel_m10(CHANNEL_m10 *channel, TERN_m10 channel_allocated_en_bloc)
{
        si4	        i;
        TERN_m10        segments_allocated_en_bloc;
 
        
        if (channel == NULL) {
                warning_message_m10("%s(): trying to free a NULL CHANNEL_m10 structure => returning with no action", __FUNCTION__);
                return;
        }
        
        if (channel->segments != NULL) {
                // allocated with e_calloc_2D
                if ((ui8) channel->segments[0] == ((ui8) channel->segments + (channel->number_of_segments * (ui8) sizeof(void *))))
                        segments_allocated_en_bloc = TRUE_m10;
                else
                        segments_allocated_en_bloc = FALSE_m10;

                for (i = 0; i < channel->number_of_segments; ++i)
                        free_segment_m10(channel->segments[i], segments_allocated_en_bloc);

                if (segments_allocated_en_bloc == TRUE_m10)
                        e_free_m10(channel->segments, __FUNCTION__, __LINE__);
        }
	
        if (channel->metadata_fps != NULL)
                free_file_processing_struct_m10(channel->metadata_fps, FALSE_m10);
	if (channel->record_data_fps != NULL)
		free_file_processing_struct_m10(channel->record_data_fps, FALSE_m10);
	if (channel->record_indices_fps != NULL)
		free_file_processing_struct_m10(channel->record_indices_fps, FALSE_m10);

        if (channel_allocated_en_bloc == FALSE_m10)
                e_free_m10(channel, __FUNCTION__, __LINE__);

        return;
}


void	free_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps, TERN_m10 allocated_en_bloc)
{
        if (fps == NULL) {
                warning_message_m10("%s(): trying to free a NULL FILE_PROCESSING_STRUCT_m10 => returning with no action", __FUNCTION__);
                return;
        }
        
        if (fps->raw_data != NULL && fps->raw_data_bytes > 0)
                e_free_m10(fps->raw_data, __FUNCTION__, __LINE__);
	fps->raw_data = NULL;
        
        if (fps->cps != NULL && fps->directives.free_CMP_processing_struct == TRUE_m10)
                CMP_free_processing_struct_m10(fps->cps);
	fps->cps = NULL;
	
	if (fps->directives.free_password_data == TRUE_m10)
		if (fps->password_data != &globals_m10->password_data)
			e_free_m10(fps->password_data, __FUNCTION__, __LINE__);

	if (fps->directives.close_file == TRUE_m10)
		fps_close_m10(fps);  // if already closed, this fails silently
        
        if (allocated_en_bloc == FALSE_m10)
                e_free_m10(fps, __FUNCTION__, __LINE__);
        
        return;
}


void    free_globals_m10(void)
{
        if (globals_m10 == NULL) {
                warning_message_m10("%s(): trying to free a NULL GLOBALS_m10 structure => returning with no action", __FUNCTION__);
                return;
        }
	if (globals_m10->timezone_table != NULL) {
                e_free_m10(globals_m10->timezone_table, __FUNCTION__, __LINE__);
		globals_m10->timezone_table = NULL;
	}
	if (globals_m10->CMP_normal_CDF_table != NULL) {
                e_free_m10(globals_m10->CMP_normal_CDF_table, __FUNCTION__, __LINE__);
		globals_m10->CMP_normal_CDF_table = NULL;
	}
	if (globals_m10->CRC_table != NULL) {
                e_free_m10(globals_m10->CRC_table, __FUNCTION__, __LINE__);
		globals_m10->CRC_table = NULL;
	}
	if (globals_m10->AES_sbox_table != NULL) {
                e_free_m10(globals_m10->AES_sbox_table, __FUNCTION__, __LINE__);
		globals_m10->AES_sbox_table = NULL;
	}
	if (globals_m10->AES_rsbox_table != NULL) {
                e_free_m10(globals_m10->AES_rsbox_table, __FUNCTION__, __LINE__);
		globals_m10->AES_rsbox_table = NULL;
	}
	if (globals_m10->AES_rcon_table != NULL) {
                e_free_m10(globals_m10->AES_rcon_table, __FUNCTION__, __LINE__);
		globals_m10->AES_rcon_table = NULL;
	}
	if (globals_m10->SHA_h0_table != NULL) {
                e_free_m10(globals_m10->SHA_h0_table, __FUNCTION__, __LINE__);
		globals_m10->SHA_h0_table = NULL;
	}
	if (globals_m10->SHA_k_table != NULL) {
                e_free_m10(globals_m10->SHA_k_table, __FUNCTION__, __LINE__);
		globals_m10->SHA_k_table = NULL;
	}
	if (globals_m10->UTF8_offsets_table != NULL) {
                e_free_m10(globals_m10->UTF8_offsets_table, __FUNCTION__, __LINE__);
		globals_m10->UTF8_offsets_table = NULL;
	}
	if (globals_m10->UTF8_trailing_bytes_table != NULL) {
                e_free_m10(globals_m10->UTF8_trailing_bytes_table, __FUNCTION__, __LINE__);
		globals_m10->UTF8_trailing_bytes_table = NULL;
	}
	
        free(globals_m10);
	globals_m10 = NULL;
        
        return;
}


void    free_metadata_m10(METADATA_m10 *metadata)
{
	e_free_m10((void *) metadata->metadata, __FUNCTION__, __LINE__);
	e_free_m10((void *) metadata, __FUNCTION__, __LINE__);
	
	return;
}


void	free_segment_m10(SEGMENT_m10 *segment, TERN_m10 segment_allocated_en_bloc)
{
        if (segment == NULL) {
                warning_message_m10("$s(): trying to free a NULL SEGMENT_m10 structure => returning with no action", __FUNCTION__);
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
                e_free_m10(segment, __FUNCTION__, __LINE__);

        return;
}


void	free_session_m10(SESSION_m10 *session)
{
        si4	        i;
        TERN_m10        allocated_en_bloc;
        
        
        if (session == NULL) {
                warning_message_m10("%s(): trying to free a NULL SESSION_m10 structure => returning with no action", __FUNCTION__);
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
                if ((ui8) session->time_series_channels[0] == ((ui8) session->time_series_channels + (session->number_of_time_series_channels * (ui8) sizeof(void *))))
                        allocated_en_bloc = TRUE_m10;
                else
                        allocated_en_bloc = FALSE_m10;
                        
                for (i = 0; i < session->number_of_time_series_channels; ++i)
                        free_channel_m10(session->time_series_channels[i], allocated_en_bloc);
                        
                if (allocated_en_bloc == TRUE_m10)
                        e_free_m10(session->time_series_channels, __FUNCTION__, __LINE__);
        }
        
        if (session->number_of_video_channels) {
                if ((ui8) session->video_channels[0] == ((ui8) session->video_channels + (session->number_of_video_channels * (ui8) sizeof(void *))))
                        allocated_en_bloc = TRUE_m10;
                else
                        allocated_en_bloc = FALSE_m10;
                        
                for (i = 0; i < session->number_of_video_channels; ++i)
                        free_channel_m10(session->video_channels[i], allocated_en_bloc);
                        
                if (allocated_en_bloc == TRUE_m10)
                        e_free_m10(session->video_channels, __FUNCTION__, __LINE__);
        }
        
        if (session->segmented_record_data_fps != NULL) {
                if ((ui8) session->segmented_record_data_fps[0] == ((ui8) session->segmented_record_data_fps + (session->number_of_segments * (ui8) sizeof(void *))))
                        allocated_en_bloc = TRUE_m10;
                else
                        allocated_en_bloc = FALSE_m10;

                for (i = 0; i < session->number_of_segments; ++i)
                        free_file_processing_struct_m10(session->segmented_record_data_fps[i], allocated_en_bloc);
                
                if (allocated_en_bloc == TRUE_m10)
                        e_free_m10(session->segmented_record_data_fps, __FUNCTION__, __LINE__);
        }
                
        if (session->segmented_record_indices_fps != NULL) {
                if ((ui8) session->segmented_record_indices_fps[0] == ((ui8) session->segmented_record_indices_fps + (session->number_of_segments * (ui8) sizeof(void *))))
                        allocated_en_bloc = TRUE_m10;
                else
                        allocated_en_bloc = FALSE_m10;

                for (i = 0; i < session->number_of_segments; ++i)
                        free_file_processing_struct_m10(session->segmented_record_indices_fps[i], allocated_en_bloc);
                
                if (allocated_en_bloc == TRUE_m10)
                        e_free_m10(session->segmented_record_indices_fps, __FUNCTION__, __LINE__);
        }

        e_free_m10(session, __FUNCTION__, __LINE__);

        return;
}


si1	**generate_file_list_m10(si1 **file_list, si4 n_in_files, si4 *n_out_files, si1 *enclosing_directory, si1 *name, si1 *extension, ui1 path_parts, TERN_m10 free_input_file_list)
{
	si4	        i, ret_val;
        si1             cwd[FULL_FILE_NAME_BYTES_m10], tmp_enclosing_directory[FULL_FILE_NAME_BYTES_m10];
        si1             tmp_name[SEGMENT_BASE_FILE_NAME_BYTES_m10], tmp_extension[16];
        si1             *command, **tmp_ptr_ptr, temp_str[FULL_FILE_NAME_BYTES_m10];
	FILE	        *fp;

        // can be used to get a directory list also
        // file_list, enclosing_directory, name, & extension can contain regexp
        // file_list should not be statically allocated, as it will be reallocated
        

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

        // make enclosing_directory from root
        if (*enclosing_directory != '/') {
                if (*enclosing_directory) {
                        getcwd(cwd, FULL_FILE_NAME_BYTES_m10);
                        sprintf(temp_str, "%s/%s", cwd, enclosing_directory);
                        strcpy(enclosing_directory, temp_str);
                } else {
                        getcwd(enclosing_directory, FULL_FILE_NAME_BYTES_m10);
                }
        }
        
        if (file_list == NULL) {  // create file list from enclosing_directory, name, & extension
                file_list = (si1 **) e_calloc_2D_m10((size_t) 1, FULL_FILE_NAME_BYTES_m10, sizeof(si1 *), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                strcpy(file_list[0], enclosing_directory);
                if (*name)
                        sprintf(file_list[0], "%s/%s", enclosing_directory, name);
                else
                        sprintf(file_list[0], "%s/*", file_list[0]);
                if (*extension)
                        sprintf(file_list[0], "%s.%s", file_list[0], extension);
                n_in_files = 1;
        } else {  // copy passed file list
                tmp_ptr_ptr = (si1 **) e_calloc_2D_m10((size_t) n_in_files, FULL_FILE_NAME_BYTES_m10, sizeof(si1 *), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                for (i = 0; i < n_in_files; ++i)
                        strcpy(tmp_ptr_ptr[i], file_list[i]);
                if (free_input_file_list == TRUE_m10)
                        e_free_2D_m10((void **) file_list, n_in_files, __FUNCTION__, __LINE__);
                file_list = tmp_ptr_ptr;
        }
        
        // make file list from root
        for (i = 0; i < n_in_files; ++i) {
                if (*file_list[i] != '/') {
                        sprintf(temp_str, "%s/%s", enclosing_directory, file_list[i]);
                        strcpy(file_list[i], temp_str);
                }
        }
        
        // expand regex and remove non-existent files
        command = (si1 *) e_calloc_m10((n_in_files * FULL_FILE_NAME_BYTES_m10) + 32, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        strcpy(command, "ls -1d");
        for (i = 0; i < n_in_files; ++i) {
                escape_spaces_m10(file_list[i], FULL_FILE_NAME_BYTES_m10);
                sprintf(command, "%s %s", command, file_list[i]);
        }
        sprintf(command, "%s > /tmp/junk 2> /dev/null", command);
        e_free_2D_m10((void **) file_list, n_in_files, __FUNCTION__, __LINE__);
        
        // count expanded file list
        *n_out_files = 0;
        if (file_exists_m10("/tmp/junk") == FILE_EXISTS_m10)
                e_system_m10("rm -f /tmp/junk", FALSE_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        ret_val = e_system_m10(command, FALSE_m10, __FUNCTION__, __LINE__, RETURN_ON_FAIL_m10 | SUPPRESS_ERROR_OUTPUT_m10);
        e_free_m10(command, __FUNCTION__, __LINE__);
        if (ret_val) {
                e_system_m10("rm -f /tmp/junk", FALSE_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                return(NULL);
        }
        fp = e_fopen_m10("/tmp/junk", "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
        while (fscanf(fp, "%[^\n]", temp_str) != EOF) {
                fgetc(fp);
                ++(*n_out_files);
        }

        if (*n_out_files == 0) {
                fclose(fp);
                e_system_m10("rm -f /tmp/junk", TRUE_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                return(NULL);
        }
        
        // allocate
        file_list = (si1 **) e_calloc_2D_m10((size_t) *n_out_files, FULL_FILE_NAME_BYTES_m10, sizeof(si1 *), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);

        // build file list
        rewind(fp);
        for (i = 0; i < *n_out_files; ++i) {
                fscanf(fp, "%[^\n]", file_list[i]);
                fgetc(fp);
        }
        
        // clean up
        fclose(fp);
        e_system_m10("rm -f /tmp/junk", TRUE_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
        // return just requested path parts
        if (path_parts != PP_FULL_PATH_m10) {
                for (i = 0; i < *n_out_files; ++i) {
                        extract_path_parts_m10(file_list[i], enclosing_directory, name, extension);
                        switch (path_parts) {
                                case (PP_PATH_m10 | PP_NAME_m10):
                                        sprintf(file_list[i], "%s/%s", enclosing_directory, name);
                                        break;
                                case (PP_NAME_m10 | PP_EXTENSION_m10):
                                        sprintf(file_list[i], "%s.%s", name, extension);
                                        break;
                                case PP_NAME_m10:
                                        strcpy(file_list[i], name);
                                        break;
                                default:
                                        error_message_m10("%s(): unrecognized path component combination (path_parts == %hhu)", __FUNCTION__, path_parts);
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
		string = (si1 *) e_calloc_m10((size_t) ((num_bytes + 1) * 3), sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	s = string;
	*s++ = '0';
	*s++ = 'x';
        
	for (i = 0; i < num_bytes; ++i) {
		sprintf(s, " %2x", bytes[i]);
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
                        error_message_m10("%s(): passed file \"%s\" is not a MED file => returning", __FUNCTION__, path);
                        return(NO_TYPE_CODE_m10);
                }
        } else if (fe == DIR_EXISTS_m10) {
                strcpy(temp_path, unescaped_path);
        } else {
                error_message_m10("%s(): passed path \"%s\" does not exist => returning", __FUNCTION__, path);
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
                        error_message_m10("%s(): passed path \"%s\" is not a MED directory", __FUNCTION__, temp_path);
                        return(NO_TYPE_CODE_m10);
        }
        
        if (MED_dir != NULL)
                snprintf_m10(MED_dir, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", MED_dir, MED_name, extension);

        return(code);
}


si1     **generate_numbered_names_m10(si1 **names, si1 *prefix, si4 number_of_names)
{
        si8     i;
        si1     number_str[FILE_NUMBERING_DIGITS_m10 + 1];
        
        
        if (names == NULL)
                names = (si1 **) e_calloc_2D_m10(number_of_names, SEGMENT_BASE_FILE_NAME_BYTES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
        for (i = 0; i < number_of_names; ++i) {
                numerical_fixed_width_string_m10(number_str, FILE_NUMBERING_DIGITS_m10, i + 1);
                snprintf_m10(names[i], SEGMENT_BASE_FILE_NAME_BYTES_m10, "%s%s", prefix, number_str);
        }
        
        return(names);
}


si8	generate_recording_time_offset_m10(si8 recording_start_time_uutc)
{
	si4		dst_offset;
        si8             epoch_utc, recording_start_time_utc, offset_utc_time;
	struct tm	local_time_info, offset_time_info;
	
	
	// receives UNOFFSET recording start time (or CURRENT_TIME_m10); returns OFFSET recording start time
	
	if (recording_start_time_uutc == CURRENT_TIME_m10) // use current system time
		recording_start_time_uutc = current_uutc_m10();

	recording_start_time_utc = recording_start_time_uutc / (si8) 1000000;

        // get epoch & local time
        epoch_utc = 0;
        gmtime_r(&epoch_utc, &offset_time_info);
        localtime_r(&recording_start_time_utc, &local_time_info);
                
        // set offset time info
        offset_time_info.tm_sec = local_time_info.tm_sec;
        offset_time_info.tm_min = local_time_info.tm_min;
        offset_time_info.tm_hour = local_time_info.tm_hour;
        
        // get offset UTC time
        offset_utc_time = timegm(&offset_time_info);
	dst_offset = DST_offset_m10(recording_start_time_uutc);
        if (dst_offset)  // adjust to standard time if DST in effect
                offset_utc_time -= dst_offset;
        
        // set global offset
        globals_m10->recording_time_offset = (recording_start_time_utc - offset_utc_time) * (si8) 1000000;

        if (globals_m10->verbose == TRUE_m10)
		message_m10("Recording Time Offset = %ld", globals_m10->recording_time_offset);
	
	globals_m10->RTO_known = TRUE_m10;
		
	return(recording_start_time_uutc - globals_m10->recording_time_offset);
}


si1	*generate_segment_name_m10(FILE_PROCESSING_STRUCT_m10 *fps, si1 *segment_name)
{
	si1	segment_number_str[FILE_NUMBERING_DIGITS_m10 + 1];
	
	
	if (segment_name == NULL)  // if NULL is passed, this will be allocated, but the calling function has the responsibility to free it.
		segment_name = (si1 *) e_malloc_m10((size_t) SEGMENT_BASE_FILE_NAME_BYTES_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	numerical_fixed_width_string_m10(segment_number_str, FILE_NUMBERING_DIGITS_m10, fps->universal_header->segment_number);
	
	snprintf_m10(segment_name, SEGMENT_BASE_FILE_NAME_BYTES_m10, "%s_s%s", fps->universal_header->channel_name, segment_number_str);

	return(segment_name);
}


ui8	generate_UID_m10(ui8 *uid)
{
	si4	        i;
        ui1             *ui1_p;
        static ui8      local_UID;
	
	
	// Note if NULL is passed for uid, this function is not thread-safe
	if (uid == NULL)
                uid = (ui8 *) &local_UID;
        ui1_p = (ui1 *) uid;

RESERVED_UID_VALUE_m10:
        for (i = 0; i < UID_BYTES_m10; ++i)
                ui1_p[i] = (ui1) (random() % (ui8) 0x100);
	
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
        FILE_PROCESSING_STRUCT_m10              *ri_fps, *rd_fps;
        TIME_SERIES_METADATA_SECTION_2_m10      *tmd2;
        RECORD_INDEX_m10                        *ri;
        RECORD_HEADER_m10                       *rh;
        UNIVERSAL_HEADER_m10                    *uh;
        REC_Sgmt_v10_m10                        *Sgmt;
        SEGMENT_m10                             *seg;
        

        if (mode != FIND_START_m10 && mode != FIND_END_m10) {
                error_message_m10("%s(): must pass FIND_START_m10 or FIND_END_m10 for mode", __FUNCTION__);
                return(FALSE_m10);
        }
        if (*target_uutc == UUTC_NO_ENTRY_m10) {
                search_mode = INDEX_SEARCH_m10;
                if (*target_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10) {
                        if (mode == FIND_START_m10)
                                target_samp = 0;
                        else  // mode == FIND_END_m10
                                target_samp = END_OF_TIME_m10;
                } else {
                        target_samp = *target_sample_number;
                }
        } else {  // *target_uutc != UUTC_NO_ENTRY_m10
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
                sprintf(tmp_str, "%s/%s.%s", channel->path, channel->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
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
                sprintf(tmp_str, "%s/%s.%s", channel->path, channel->name, RECORD_DATA_FILE_TYPE_STRING_m10);
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
                } else {  // search_mode == TIME_SEARCH_m10
                        prev_ridx_idx = 0;
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
                                                        } else {  // mode == FIND_END_m10
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
                                        error_message_m10("%s(): target sample exceeds channel sample range", __FUNCTION__);
                                        if (free_record_indices == TRUE_m10)
                                                free_file_processing_struct_m10(ri_fps, FALSE_m10);
                                        if (free_record_data == TRUE_m10)
                                                free_file_processing_struct_m10(rd_fps, FALSE_m10);
                                        return(FALSE_m10);
                                }
                                target_samp = Sgmt->absolute_end_sample_number;
                        } else { // search_mode == TIME_SEARCH_m10
                                if (mode == FIND_START_m10) {
                                        error_message_m10("%s(): target uutc exceeds channel times", __FUNCTION__);
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
                        error_message_m10("%s(): cannot search video channels without Sgmt records at this time", __FUNCTION__);
                        return(FALSE_m10);
                }
                for (i = 0; i < channel->number_of_segments; ++i) {
                        seg = channel->segments[i];
                        if (search_mode == INDEX_SEARCH_m10) {
                                tmd2 = seg->metadata_fps->metadata.time_series_section_2;
                                if (target_samp < (tmd2->absolute_start_sample_number + tmd2->number_of_samples))
                                        break;
                        } else {  // search_mode == TIME_SEARCH_m10
                                uh = seg->metadata_fps->universal_header;
                                if (target_time <= uh->file_end_time) {
                                        // time fell between segments
                                        if (target_time < uh->file_start_time) {
                                                if (mode == FIND_START_m10) {
                                                        target_time = uh->file_start_time;
                                                } else {  // mode == FIND_END_m10
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
                                        error_message_m10("%s(): target sample exceeds channel sample range", __FUNCTION__);
                                        if (free_record_indices == TRUE_m10)
                                                free_file_processing_struct_m10(ri_fps, FALSE_m10);
                                        if (free_record_data == TRUE_m10)
                                                free_file_processing_struct_m10(rd_fps, FALSE_m10);
                                        return(FALSE_m10);
                                }
                                target_samp = tmd2->absolute_start_sample_number + tmd2->number_of_samples - 1;
                        } else { // search_mode == TIME_SEARCH_m10
                                if (mode == FIND_START_m10) {
                                        error_message_m10("%s(): target uutc exceeds channel times", __FUNCTION__);
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


si4     get_segment_range_m10(si1 **channel_list, si4 n_channels, TIME_SLICE_m10 *slice)
{
        TERN_m10                        search_succeeded;
        si1                             chan_name[BASE_FILE_NAME_BYTES_m10], tmp_str[FULL_FILE_NAME_BYTES_m10];
        si4                             ref_chan_idx;
        si8                             i;
        
        
        // copy inputs to local variables (substitute NO_ENTRY for NULL)
	if (slice == NULL) {
                error_message_m10("%s(): NULL slice pointer", __FUNCTION__);
		return((si4) FALSE_m10);
	}
        
    	// find index channel
	ref_chan_idx = 0;
        *chan_name = 0;
        if (slice->index_reference_channel_name != NULL) {
                if (*slice->index_reference_channel_name != 0) {
                        force_behavior_m10(globals_m10->behavior_on_fail | SUPPRESS_WARNING_OUTPUT_m10);
                        extract_path_parts_m10(slice->index_reference_channel_name, NULL, chan_name, NULL);
                        force_behavior_m10(RESTORE_BEHAVIOR_m10);
                        for (i = 0; i < n_channels; ++i) {
                                extract_path_parts_m10(channel_list[i], NULL, tmp_str, NULL);
                                if (strcmp(tmp_str, chan_name) == 0)
                                        break;
                        }
			if (i != n_channels)
				ref_chan_idx = i;
			else
                                warning_message_m10("%s(): Cannot find reference channel in file list => using first channel", __FUNCTION__);
                }
        }
	slice->index_reference_channel_index = ref_chan_idx;
	slice->index_reference_channel_name = channel_list[ref_chan_idx];
	
        // search Sgmt records
        search_succeeded = search_Sgmt_records_m10(channel_list[ref_chan_idx], slice);

        // search segment metadata
	if (search_succeeded == FALSE_m10)
                search_succeeded = search_segment_metadata_m10(channel_list[ref_chan_idx], slice);
	
        if (search_succeeded == FALSE_m10)
                return((si4) FALSE_m10);

        return((slice->end_segment_number - slice->start_segment_number) + 1);
}


void    get_segment_target_values_m10(SEGMENT_m10 *segment, si8 *target_uutc, si8 *target_sample_number, ui1 mode)
{
        ui1                                     search_mode;
        si8                                     target_samp, target_time;
        TIME_SERIES_METADATA_SECTION_2_m10      *tmd2;
        UNIVERSAL_HEADER_m10                    *uh;
        
        
        tmd2 = segment->metadata_fps->metadata.time_series_section_2;
        if (*target_uutc == UUTC_NO_ENTRY_m10) {
                search_mode = INDEX_SEARCH_m10;
                if (*target_sample_number == SAMPLE_NUMBER_NO_ENTRY_m10) {
                        uh = segment->metadata_fps->universal_header;
                        if (mode == FIND_START_m10) {
                                *target_sample_number = tmd2->absolute_start_sample_number;
                                *target_uutc = uh->file_start_time;
                                return;
                        } else {  // mode == FIND_END_m10
                                *target_sample_number = tmd2->absolute_start_sample_number + tmd2->number_of_samples - 1;
                                *target_uutc = uh->file_end_time;
                                return;
                        }
                } else {
                        target_samp = *target_sample_number;
                }
        } else {  // *target_uutc != UUTC_NO_ENTRY_m10
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
        CHANNEL_m10                             *chan;
        SEGMENT_m10                             *seg;
        TIME_SERIES_METADATA_SECTION_2_m10      *tmd2;
        VIDEO_METADATA_SECTION_2_m10            *vmd2;
        FILE_PROCESSING_STRUCT_m10              *ri_fps, *rd_fps;
        RECORD_INDEX_m10                        *ri;
        RECORD_HEADER_m10                       *rh;
        UNIVERSAL_HEADER_m10                    *uh;
        REC_Sgmt_v10_m10                        *Sgmt;

        
        same_frequency = TRUE_m10;
        vmd2 = NULL;
        tmd2 = session->time_series_metadata_fps->metadata.time_series_section_2;
        if (tmd2 == NULL) {
                vmd2 = session->time_series_metadata_fps->metadata.video_section_2;
                if (tmd2 == NULL) {
                        error_message_m10("%s(): no section 2 metadata in session", __FUNCTION__);
                        return(FALSE_m10);
                } else if (vmd2->frame_rate == VIDEO_METADATA_FRAME_RATE_NO_ENTRY_m10)
                        same_frequency = FALSE_m10;
        } else {
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
                        } else {  // vmd2 != NULL
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
                        warning_message_m10("%s(): No reference channel passed => using first channel", __FUNCTION__);
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
                sprintf(tmp_str, "%s/%s.%s", session->path, session->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
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
                sprintf(tmp_str, "%s/%s.%s", session->path, session->name, RECORD_DATA_FILE_TYPE_STRING_m10);
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
                        warning_message_m10("%s(): No reference channel passed => using first channel", __FUNCTION__);
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
                        } else {
                                target_samp = *target_sample_number;
                        }
                } else {  // *target_uutc != UUTC_NO_ENTRY_m10
                        search_mode = TIME_SEARCH_m10;
                        target_time = *target_uutc;
                }
        } else {  // same_frequency == FALSE_m10
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
                } else {
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
        } else {  // search_mode == TIME_SEARCH_m10
                prev_ridx_idx = 0;
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
                                                } else {  // mode == FIND_END_m10
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
                                error_message_m10("%s(): target sample exceeds channel sample range", __FUNCTION__);
                                if (free_record_indices == TRUE_m10)
                                        free_file_processing_struct_m10(ri_fps, FALSE_m10);
                                if (free_record_data == TRUE_m10)
                                        free_file_processing_struct_m10(rd_fps, FALSE_m10);
                                return(FALSE_m10);
                        }
                        target_samp = Sgmt->absolute_end_sample_number;
                } else { // search_mode == TIME_SEARCH_m10
                        if (mode == FIND_START_m10) {
                                error_message_m10("%s(): target uutc exceeds channel times", __FUNCTION__);
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


FILE_PROCESSING_DIRECTIVES_m10  *initialize_file_processing_directives_m10(FILE_PROCESSING_DIRECTIVES_m10 *directives)
{
        if (directives == NULL)
                directives = (FILE_PROCESSING_DIRECTIVES_m10 *) e_calloc_m10((size_t) 1, sizeof(FILE_PROCESSING_DIRECTIVES_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
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

void	initialize_globals_m10(void)
{
        if (globals_m10 == NULL) {
		globals_m10 = (GLOBALS_m10 *) calloc((size_t) 1, sizeof(GLOBALS_m10));
                if (globals_m10 == NULL) {
                        error_message_m10("%s(): calloc error", __FUNCTION__);
                        return;
                }
        }
	
        // password structure
	memset((void *) &globals_m10->password_data, 0, sizeof(PASSWORD_DATA_m10));
	// time constants
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
	globals_m10->RTO_known = GLOBALS_RTO_KNOWN_DEFAULT_m10;
	if (globals_m10->timezone_table != NULL)
		e_free_m10((void *) globals_m10->timezone_table, __FUNCTION__, __LINE__);
        globals_m10->timezone_table = NULL;
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
	if (globals_m10->CMP_normal_CDF_table != NULL)
		e_free_m10((void *) globals_m10->CMP_normal_CDF_table, __FUNCTION__, __LINE__);
        globals_m10->CMP_normal_CDF_table = NULL;
	// CRC
	if (globals_m10->CRC_table != NULL)
		e_free_m10((void *) globals_m10->CRC_table, __FUNCTION__, __LINE__);
	globals_m10->CRC_table = NULL;
	globals_m10->CRC_mode = GLOBALS_CRC_MODE_DEFAULT_m10;
	// AES
	if (globals_m10->AES_sbox_table != NULL)
		e_free_m10((void *) globals_m10->AES_sbox_table, __FUNCTION__, __LINE__);
	globals_m10->AES_sbox_table = NULL;
	if (globals_m10->AES_rsbox_table != NULL)
		e_free_m10((void *) globals_m10->AES_rsbox_table, __FUNCTION__, __LINE__);
	globals_m10->AES_rsbox_table = NULL;
	if (globals_m10->AES_rcon_table != NULL)
		e_free_m10((void *) globals_m10->AES_rcon_table, __FUNCTION__, __LINE__);
	globals_m10->AES_rcon_table = NULL;
	// SHA
	if (globals_m10->SHA_h0_table != NULL)
		e_free_m10((void *) globals_m10->SHA_h0_table, __FUNCTION__, __LINE__);
	globals_m10->SHA_h0_table = NULL;
	if (globals_m10->SHA_k_table != NULL)
		e_free_m10((void *) globals_m10->SHA_k_table, __FUNCTION__, __LINE__);
	globals_m10->SHA_k_table = NULL;
	 // UTF-8
	if (globals_m10->UTF8_offsets_table != NULL)
		e_free_m10((void *) globals_m10->UTF8_offsets_table, __FUNCTION__, __LINE__);
	globals_m10->UTF8_offsets_table = NULL;
	if (globals_m10->UTF8_trailing_bytes_table != NULL)
		e_free_m10((void *) globals_m10->UTF8_trailing_bytes_table, __FUNCTION__, __LINE__);
	globals_m10->UTF8_trailing_bytes_table = NULL;
	// miscellaneous
	globals_m10->verbose = GLOBALS_VERBOSE_DEFAULT_m10;
        globals_m10->behavior_on_fail = GLOBALS_BEHAVIOR_ON_FAIL_DEFAULT_m10;

	return;
}


//***********************************************************************//
//**************************  END MED GLOBALS  **************************//
//***********************************************************************//


TERN_m10	initialize_medlib_m10(void)
{
	TERN_m10	return_value;
	
	
	// set up globals
        if (globals_m10 == NULL)
                initialize_globals_m10();

	// check cpu endianness
	if ((get_cpu_endianness_m10()) != LITTLE_ENDIAN_m10) {
		printf("%hhu\n", get_cpu_endianness_m10());
		error_message_m10("%s(): Library only coded for little-endian machines currently", __FUNCTION__);
		exit(1);
	}
        
	// check structure alignments
	return_value = check_all_alignments_m10(__FUNCTION__, __LINE__);
	
	// seed random number generator
	srandom((ui4) time(NULL));
	
	// make CMP table global
	CMP_initialize_normal_CDF_table_m10();
        
	// make CRC table global
	CRC_initialize_table_m10();
	
	// make UTF8 tables global
	UTF8_initialize_offsets_table_m10();
	UTF8_initialize_trailing_bytes_table_m10();
	
	// make AES tables global
	AES_initialize_sbox_table_m10();
	AES_initialize_rsbox_table_m10();
	AES_initialize_rcon_table_m10();
	
	// make SHA tables global
	SHA_initialize_h0_table_m10();
	SHA_initialize_k_table_m10();

        // make timezone table global
        initialize_timezone_table_m10();

	return(return_value);
}


void	initialize_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps, TERN_m10 initialize_for_update)
{
	METADATA_SECTION_1_m10		        *md1;
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2;
        VIDEO_METADATA_SECTION_2_m10	        *vmd2;
	METADATA_SECTION_3_m10		        *md3;
        UNIVERSAL_HEADER_m10                    *uh;
	
        
	// shortcuts
	md1 = fps->metadata.section_1;
	tmd2 = fps->metadata.time_series_section_2;  // can use any channel type here
	vmd2 = fps->metadata.video_section_2;
	md3 = fps->metadata.section_3;
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
			tmd2 = fps->metadata.time_series_section_2;
                        tmd2->sampling_frequency = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10;
                        tmd2->low_frequency_filter_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10;
                        tmd2->high_frequency_filter_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10;
                        tmd2->notch_filter_frequency_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10;
                        tmd2->AC_line_frequency = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10;
                        tmd2->amplitude_units_conversion_factor = TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10;
                        tmd2->time_base_units_conversion_factor = TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10;
                        tmd2->absolute_start_sample_number = TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_NO_ENTRY_m10;
                        if (initialize_for_update == TRUE_m10){
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
                        } else {
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
                        } else {
                                vmd2->number_of_clips = VIDEO_METADATA_NUMBER_OF_CLIPS_NO_ENTRY_m10;
                                vmd2->maximum_clip_bytes = VIDEO_METADATA_MAXIMUM_CLIP_BYTES_NO_ENTRY_m10;
                                vmd2->number_of_video_files = VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_NO_ENTRY_m10;
                        }
			break;
		default:
			error_message_m10("%s(): Unrecognized METADATA SECTION 2 type in file \"%s\"", __FUNCTION__, fps->full_file_name);
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
        memset(md3->recording_city, 0, METADATA_RECORDING_LOCATION_BYTES_m10);
        memset(md3->recording_institution, 0, METADATA_RECORDING_LOCATION_BYTES_m10);
        memset(md3->geotag_format, 0, METADATA_GEOTAG_FORMAT_BYTES_m10);
        memset(md3->geotag_data, 0, METADATA_GEOTAG_DATA_BYTES_m10);
        md3->standard_UTC_offset = globals_m10->standard_UTC_offset;

	return;
}


TIME_SLICE_m10  *initialize_time_slice_m10(TIME_SLICE_m10 *slice)
{
        if (slice == NULL)  // caller responsible for freeing
                slice = (TIME_SLICE_m10 *) e_malloc_m10(sizeof(TIME_SLICE_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	slice->conditioned = FALSE_m10;
        slice->start_time = slice->end_time = UUTC_NO_ENTRY_m10;
        slice->start_index = slice->end_index = SAMPLE_NUMBER_NO_ENTRY_m10;
	slice->local_start_index = slice->local_end_index = SAMPLE_NUMBER_NO_ENTRY_m10;
	slice->number_of_samples = NUMBER_OF_SAMPLES_NO_ENTRY_m10;
        slice->start_segment_number = slice->end_segment_number = SEGMENT_NUMBER_NO_ENTRY_m10;
        slice->session_start_time = slice->session_end_time = UUTC_NO_ENTRY_m10;
	slice->index_reference_channel_name = NULL;
	slice->index_reference_channel_index = 0;  // defaults to first channel
        
        return(slice);
}


void	initialize_timezone_table_m10(void)
{
        TIMEZONE_INFO_m10   *timezone_table;
        
        
        timezone_table = (TIMEZONE_INFO_m10 *) e_calloc_m10((size_t) TIMEZONE_TABLE_ENTRIES_m10, sizeof(TIMEZONE_INFO_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
        {
                TIMEZONE_INFO_m10 temp[TIMEZONE_TABLE_ENTRIES_m10] = TIMEZONE_TABLE_m10;
                memcpy(timezone_table, temp, TIMEZONE_TABLE_ENTRIES_m10 * sizeof(TIMEZONE_INFO_m10));
        }
        
	globals_m10->timezone_table = timezone_table;

        return;
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


si1     *MED_type_string_from_code_m10(ui4 code)
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
	
        return(NULL);
}


ui4     MED_type_code_from_string_m10(si1 *string)
{
        ui4     code;
        si4     len;
        
        
        if (string == NULL) {
                warning_message_m10("%s(): string is NULL", __FUNCTION__);
                return(NO_FILE_TYPE_CODE_m10);
        }
        
        len = strlen(string) - 4;
        if (len < 0)
               return(NO_FILE_TYPE_CODE_m10);
        string += len;
        memcpy(&code, string, sizeof(ui4));
        
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
        
	warning_message_m10("%s(): \"%s\" is not a recognized MED file type", __FUNCTION__, string);
	
        return(NO_FILE_TYPE_CODE_m10);
}


TERN_m10        merge_metadata_m10(FILE_PROCESSING_STRUCT_m10 *md_fps_1, FILE_PROCESSING_STRUCT_m10 *md_fps_2, FILE_PROCESSING_STRUCT_m10 *merged_md_fps)
{
        // if merged_md_fps == NULL, comparison results will be placed in md_fps_1->metadata
        // returns TRUE_m10 if md_fps_1->metadata == md_fps_2->metadata, FALSE_m10 otherwise
        
        ui4                                     type_code;
        METADATA_SECTION_1_m10                  *md1_1, *md1_2, *md1_m;
        TIME_SERIES_METADATA_SECTION_2_m10      *tmd2_1, *tmd2_2, *tmd2_m;
        VIDEO_METADATA_SECTION_2_m10            *vmd2_1, *vmd2_2, *vmd2_m;
        METADATA_SECTION_3_m10                  *md3_1, *md3_2, *md3_m;
        TERN_m10                                equal;
    

        // decrypt if needed
        md1_1 = md_fps_1->metadata.section_1;
        if (md1_1->section_2_encryption_level > NO_ENCRYPTION_m10 || md1_1->section_3_encryption_level > NO_ENCRYPTION_m10)
                decrypt_metadata_m10(md_fps_1);
        md1_2 = md_fps_2->metadata.section_1;
        if (md1_2->section_2_encryption_level > NO_ENCRYPTION_m10 || md1_2->section_3_encryption_level > NO_ENCRYPTION_m10)
                decrypt_metadata_m10(md_fps_2);

        // setup
        if (merged_md_fps == NULL)
                merged_md_fps = md_fps_1;
        else
                memcpy(merged_md_fps->metadata.metadata, md_fps_1->metadata.metadata, METADATA_BYTES_m10);
        md1_m = merged_md_fps->metadata.section_1;

        type_code = md_fps_1->universal_header->type_code;
        if (type_code != md_fps_2->universal_header->type_code) {
                error_message_m10("%s(): mismatched type codes", __FUNCTION__);
                return(UNKNOWN_m10);
        }
        
        switch (type_code) {
                case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
                case VIDEO_METADATA_FILE_TYPE_CODE_m10:
                        break;
                default:
                        error_message_m10("%s(): unrecognized type code 0x%x", __FUNCTION__, type_code);
                        return(UNKNOWN_m10);
        }
        equal = TRUE_m10;

        // section 1
	if (memcmp(md1_1->level_1_password_hint, md1_2->level_1_password_hint, PASSWORD_HINT_BYTES_m10)) {
		memset(md1_m->level_1_password_hint, 0, PASSWORD_HINT_BYTES_m10); equal = FALSE_m10; }
	if (memcmp(md1_1->level_2_password_hint, md1_2->level_2_password_hint, PASSWORD_HINT_BYTES_m10)) {
		memset(md1_m->level_2_password_hint, 0, PASSWORD_HINT_BYTES_m10); equal = FALSE_m10; }
        if (md1_1->section_2_encryption_level != md1_2->section_2_encryption_level) {
                md1_m->section_2_encryption_level = ENCRYPTION_LEVEL_NO_ENTRY_m10; equal = FALSE_m10; }
        if (md1_1->section_3_encryption_level != md1_2->section_3_encryption_level) {
                md1_m->section_3_encryption_level = ENCRYPTION_LEVEL_NO_ENTRY_m10; equal = FALSE_m10; }
        if (memcmp(md1_1->protected_region, md1_2->protected_region, METADATA_SECTION_1_PROTECTED_REGION_BYTES_m10)) {
                memset(md1_m->protected_region, 0, METADATA_SECTION_1_PROTECTED_REGION_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md1_1->discretionary_region, md1_2->discretionary_region, METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m10)) {
                memset(md1_m->discretionary_region, 0, METADATA_SECTION_1_DISCRETIONARY_REGION_BYTES_m10); equal = FALSE_m10; }

        // section 2: times series channel
        if (type_code == TIME_SERIES_METADATA_FILE_TYPE_CODE_m10) {
                tmd2_1 = md_fps_1->metadata.time_series_section_2; tmd2_2 = md_fps_2->metadata.time_series_section_2; tmd2_m = merged_md_fps->metadata.time_series_section_2;
                if (memcmp(tmd2_1->session_description, tmd2_2->session_description, METADATA_SESSION_DESCRIPTION_BYTES_m10)) {
                        memset(tmd2_m->session_description, 0, METADATA_SESSION_DESCRIPTION_BYTES_m10); equal = FALSE_m10; }
                if (memcmp(tmd2_1->channel_description, tmd2_2->channel_description, METADATA_CHANNEL_DESCRIPTION_BYTES_m10)) {
                        memset(tmd2_m->channel_description, 0, METADATA_CHANNEL_DESCRIPTION_BYTES_m10); equal = FALSE_m10; }
                if (memcmp(tmd2_1->segment_description, tmd2_2->segment_description, METADATA_SEGMENT_DESCRIPTION_BYTES_m10)) {
                        memset(tmd2_m->segment_description, 0, METADATA_SEGMENT_DESCRIPTION_BYTES_m10); equal = FALSE_m10; }
                if (memcmp(tmd2_1->equipment_description, tmd2_2->equipment_description, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m10)) {
                        memset(tmd2_m->equipment_description, 0, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m10); equal = FALSE_m10; }
                if (tmd2_1->acquisition_channel_number != tmd2_2->acquisition_channel_number) {
                        tmd2_m->acquisition_channel_number = METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m10; equal = FALSE_m10; }
                if (memcmp(tmd2_1->reference_description, tmd2_2->reference_description, TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m10)) {
                        memset(tmd2_m->reference_description, 0, TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m10); equal = FALSE_m10; }
                if (tmd2_1->sampling_frequency != tmd2_2->sampling_frequency) {
                        if (tmd2_1->sampling_frequency == FREQUENCY_NO_ENTRY_m10 || tmd2_2->sampling_frequency == FREQUENCY_NO_ENTRY_m10)
                        	tmd2_m->sampling_frequency = FREQUENCY_NO_ENTRY_m10; // no entry supercedes variable frequency
                        else
                                tmd2_m->sampling_frequency = FREQUENCY_VARIABLE_m10;
                        equal = FALSE_m10;
                }
                if (tmd2_1->low_frequency_filter_setting != tmd2_2->low_frequency_filter_setting) {
                        tmd2_m->low_frequency_filter_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10; equal = FALSE_m10; }
                if (tmd2_1->high_frequency_filter_setting != tmd2_2->high_frequency_filter_setting) {
                        tmd2_m->high_frequency_filter_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10; equal = FALSE_m10; }
                if (tmd2_1->notch_filter_frequency_setting != tmd2_2->notch_filter_frequency_setting) {
                        tmd2_m->notch_filter_frequency_setting = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10; equal = FALSE_m10; }
                if (tmd2_1->AC_line_frequency != tmd2_2->AC_line_frequency) {
                        tmd2_m->AC_line_frequency = TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10; equal = FALSE_m10; }
                if (tmd2_1->amplitude_units_conversion_factor != tmd2_2->amplitude_units_conversion_factor) {
                        tmd2_m->amplitude_units_conversion_factor = TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10; equal = FALSE_m10; }
                if (memcmp(tmd2_1->amplitude_units_description, tmd2_2->amplitude_units_description, TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m10)) {
                        memset(tmd2_m->amplitude_units_description, 0, TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m10); equal = FALSE_m10; }
                if (tmd2_1->time_base_units_conversion_factor != tmd2_2->time_base_units_conversion_factor) {
                        tmd2_m->time_base_units_conversion_factor = TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10; equal = FALSE_m10; }
                if (memcmp(tmd2_1->time_base_units_description, tmd2_2->time_base_units_description, TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m10)) {
                        memset(tmd2_m->time_base_units_description, 0, TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m10); equal = FALSE_m10; }
                if (tmd2_1->absolute_start_sample_number > tmd2_2->absolute_start_sample_number) {
                        tmd2_m->absolute_start_sample_number = tmd2_2->absolute_start_sample_number; equal = FALSE_m10; }
                if (tmd2_1->number_of_samples < tmd2_2->number_of_samples) {
                        tmd2_m->number_of_samples = tmd2_2->number_of_samples; equal = FALSE_m10; }
                if (tmd2_1->number_of_blocks < tmd2_2->number_of_blocks) {
                        tmd2_m->number_of_blocks = tmd2_2->number_of_blocks; equal = FALSE_m10; }
                if (tmd2_1->maximum_block_bytes < tmd2_2->maximum_block_bytes) {
                        tmd2_m->maximum_block_bytes = tmd2_2->maximum_block_bytes; equal = FALSE_m10; }
                if (tmd2_1->maximum_block_samples < tmd2_2->maximum_block_samples) {
                        tmd2_m->maximum_block_samples = tmd2_2->maximum_block_samples; equal = FALSE_m10; }
                if (tmd2_1->maximum_block_difference_bytes < tmd2_2->maximum_block_difference_bytes) {
                        tmd2_m->maximum_block_difference_bytes = tmd2_2->maximum_block_difference_bytes; equal = FALSE_m10; }
                if (tmd2_1->maximum_block_duration < tmd2_2->maximum_block_duration) {
                        tmd2_m->maximum_block_duration = tmd2_2->maximum_block_duration; equal = FALSE_m10; }
                if (tmd2_1->number_of_discontinuities < tmd2_2->number_of_discontinuities) {
                        tmd2_m->number_of_discontinuities = tmd2_2->number_of_discontinuities; equal = FALSE_m10; }
                if (tmd2_1->maximum_contiguous_blocks < tmd2_2->maximum_contiguous_blocks) {
                        tmd2_m->maximum_contiguous_blocks = tmd2_2->maximum_contiguous_blocks; equal = FALSE_m10; }
                if (tmd2_1->maximum_contiguous_block_bytes < tmd2_2->maximum_contiguous_block_bytes) {
                        tmd2_m->maximum_contiguous_block_bytes = tmd2_2->maximum_contiguous_block_bytes; equal = FALSE_m10; }
                if (tmd2_1->maximum_contiguous_samples < tmd2_2->maximum_contiguous_samples) {
                        tmd2_m->maximum_contiguous_samples = tmd2_2->maximum_contiguous_samples; equal = FALSE_m10; }
                if (memcmp(tmd2_1->protected_region, tmd2_2->protected_region, TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m10)) {
                        memset(tmd2_m->protected_region, 0, TIME_SERIES_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m10); equal = FALSE_m10; }
                if (memcmp(tmd2_1->discretionary_region, tmd2_2->discretionary_region, TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m10)) {
                        memset(tmd2_m->discretionary_region, 0, TIME_SERIES_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m10); equal = FALSE_m10; }
                // section 2: times series channel
        } else if (type_code == VIDEO_METADATA_FILE_TYPE_CODE_m10) {
                vmd2_1 = md_fps_1->metadata.video_section_2; vmd2_2 = md_fps_2->metadata.video_section_2; vmd2_m = merged_md_fps->metadata.video_section_2;
                if (memcmp(vmd2_1->session_description, vmd2_2->session_description, METADATA_SESSION_DESCRIPTION_BYTES_m10)) {
                        memset(vmd2_m->session_description, 0, METADATA_SESSION_DESCRIPTION_BYTES_m10); equal = FALSE_m10; }
                if (memcmp(vmd2_1->channel_description, vmd2_2->channel_description, METADATA_CHANNEL_DESCRIPTION_BYTES_m10)) {
                        memset(vmd2_m->channel_description, 0, METADATA_CHANNEL_DESCRIPTION_BYTES_m10); equal = FALSE_m10; }
                if (memcmp(vmd2_1->equipment_description, vmd2_2->equipment_description, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m10)) {
                        memset(vmd2_m->equipment_description, 0, METADATA_EQUIPMENT_DESCRIPTION_BYTES_m10); equal = FALSE_m10; }
                if (vmd2_1->acquisition_channel_number != vmd2_2->acquisition_channel_number) {
                        vmd2_m->acquisition_channel_number = METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m10; equal = FALSE_m10; }
                if (vmd2_1->horizontal_resolution != vmd2_2->horizontal_resolution) {
                        vmd2_m->horizontal_resolution = VIDEO_METADATA_HORIZONTAL_RESOLUTION_NO_ENTRY_m10; equal = FALSE_m10; }
                if (vmd2_1->vertical_resolution != vmd2_2->vertical_resolution) {
                        vmd2_m->vertical_resolution = VIDEO_METADATA_VERTICAL_RESOLUTION_NO_ENTRY_m10; equal = FALSE_m10; }
                if (vmd2_1->frame_rate != vmd2_2->frame_rate) {
                        vmd2_m->frame_rate = VIDEO_METADATA_FRAME_RATE_NO_ENTRY_m10; equal = FALSE_m10; }
                if (vmd2_1->number_of_clips < vmd2_2->number_of_clips) {
                        vmd2_m->number_of_clips = vmd2_2->number_of_clips; equal = FALSE_m10; }
                if (vmd2_1->maximum_clip_bytes < vmd2_2->maximum_clip_bytes) {
                        vmd2_m->maximum_clip_bytes = vmd2_2->maximum_clip_bytes; equal = FALSE_m10; }
                if (memcmp(vmd2_1->video_format, vmd2_2->video_format, VIDEO_METADATA_VIDEO_FORMAT_BYTES_m10)) {
                        memset(vmd2_1->video_format, 0, VIDEO_METADATA_VIDEO_FORMAT_BYTES_m10); equal = FALSE_m10; }
                if (vmd2_1->number_of_video_files < vmd2_2->number_of_video_files) {
                        vmd2_m->number_of_video_files = vmd2_2->number_of_video_files; equal = FALSE_m10; }
                if (memcmp(vmd2_1->protected_region, vmd2_2->protected_region, VIDEO_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m10)) {
                        memset(vmd2_m->protected_region, 0, VIDEO_METADATA_SECTION_2_PROTECTED_REGION_BYTES_m10); equal = FALSE_m10; }
                if (memcmp(vmd2_1->discretionary_region, vmd2_2->discretionary_region, VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m10)) {
                        memset(vmd2_m->discretionary_region, 0, VIDEO_METADATA_SECTION_2_DISCRETIONARY_REGION_BYTES_m10); equal = FALSE_m10; }
        }
        
        // section 3
        md3_1 = md_fps_1->metadata.section_3; md3_2 = md_fps_2->metadata.section_3; md3_m = merged_md_fps->metadata.section_3;
        if (md3_1->recording_time_offset != md3_2->recording_time_offset) {
                md3_m->recording_time_offset = UUTC_NO_ENTRY_m10; equal = FALSE_m10; }
        if (md3_1->daylight_time_start_code.value != md3_2->daylight_time_start_code.value) {
                md3_m->daylight_time_start_code.value = DTCC_VALUE_NO_ENTRY_m10; equal = FALSE_m10; }
        if (md3_1->daylight_time_end_code.value != md3_2->daylight_time_end_code.value) {
                md3_m->daylight_time_end_code.value = DTCC_VALUE_NO_ENTRY_m10; equal = FALSE_m10; }
        if (memcmp(md3_1->standard_timezone_acronym, md3_2->standard_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m10)) {
                memset(md3_m->standard_timezone_acronym, 0, TIMEZONE_ACRONYM_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->standard_timezone_string, md3_2->standard_timezone_string, TIMEZONE_STRING_BYTES_m10)) {
                memset(md3_m->standard_timezone_string, 0, TIMEZONE_STRING_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->daylight_timezone_acronym, md3_2->daylight_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m10)) {
                memset(md3_m->daylight_timezone_acronym, 0, TIMEZONE_ACRONYM_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->daylight_timezone_string, md3_2->daylight_timezone_string, TIMEZONE_STRING_BYTES_m10))  {
                memset(md3_m->daylight_timezone_string, 0, TIMEZONE_STRING_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->subject_name_1, md3_2->subject_name_1, METADATA_SUBJECT_NAME_BYTES_m10)) {
                memset(md3_m->subject_name_1, 0, METADATA_SUBJECT_NAME_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->subject_name_2, md3_2->subject_name_2, METADATA_SUBJECT_NAME_BYTES_m10)) {
                memset(md3_m->subject_name_2, 0, METADATA_SUBJECT_NAME_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->subject_name_3, md3_2->subject_name_3, METADATA_SUBJECT_NAME_BYTES_m10)) {
                memset(md3_m->subject_name_3, 0, METADATA_SUBJECT_NAME_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->subject_ID, md3_2->subject_ID, METADATA_SUBJECT_ID_BYTES_m10)){
                memset(md3_m->subject_ID, 0, METADATA_SUBJECT_ID_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->recording_country, md3_2->recording_country, METADATA_RECORDING_LOCATION_BYTES_m10)) {
                memset(md3_m->recording_country, 0, METADATA_SUBJECT_ID_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->recording_territory, md3_2->recording_territory, METADATA_RECORDING_LOCATION_BYTES_m10)) {
                memset(md3_m->recording_territory, 0, METADATA_SUBJECT_ID_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->recording_city, md3_2->recording_city, METADATA_RECORDING_LOCATION_BYTES_m10)) {
                memset(md3_m->recording_city, 0, METADATA_RECORDING_LOCATION_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->recording_institution, md3_2->recording_institution, METADATA_RECORDING_LOCATION_BYTES_m10)) {
                memset(md3_m->recording_institution, 0, METADATA_RECORDING_LOCATION_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->geotag_format, md3_2->geotag_format, METADATA_GEOTAG_FORMAT_BYTES_m10)) {
                memset(md3_m->geotag_format, 0, METADATA_GEOTAG_FORMAT_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->geotag_data, md3_2->geotag_data, METADATA_GEOTAG_DATA_BYTES_m10)) {
                memset(md3_m->geotag_data, 0, METADATA_GEOTAG_DATA_BYTES_m10); equal = FALSE_m10; }
        if (md3_1->standard_UTC_offset != md3_2->standard_UTC_offset) {
                md3_m->standard_UTC_offset = STANDARD_UTC_OFFSET_NO_ENTRY_m10; equal = FALSE_m10; }
        if (memcmp(md3_1->protected_region, md3_2->protected_region, METADATA_SECTION_3_PROTECTED_REGION_BYTES_m10)) {
                memset(md3_m->protected_region, 0, METADATA_SECTION_3_PROTECTED_REGION_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(md3_1->discretionary_region, md3_2->discretionary_region, METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m10)) {
                memset(md3_m->discretionary_region, 0, METADATA_SECTION_3_DISCRETIONARY_REGION_BYTES_m10); equal = FALSE_m10; }

        if (globals_m10->verbose == TRUE_m10) {
                switch (type_code) {
                        case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
				printf("------------ Merged Time Series Metadata --------------\n");
                                break;
                        case VIDEO_METADATA_FILE_TYPE_CODE_m10:
				printf("--------------- Merged Video Metadata -----------------\n");
                                break;
                        break;
                }
                show_metadata_m10(NULL, &merged_md_fps->metadata);
        }

        return(equal);
}


TERN_m10        merge_universal_headers_m10(FILE_PROCESSING_STRUCT_m10 *fps_1, FILE_PROCESSING_STRUCT_m10 *fps_2, FILE_PROCESSING_STRUCT_m10 *merged_fps)
{
        // if merged_fps == NULL, comparison results will be placed in fps_1->universal_header
        // returns TRUE_m10 if fps_1->universal_header == fps_2->universal_header, FALSE_m10 otherwise
        
        UNIVERSAL_HEADER_m10    *uh_1, *uh_2, *merged_uh;
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
                memset(merged_uh->type_string, 0, TYPE_BYTES_m10); equal = FALSE_m10; }
        if (uh_1->MED_version_major != uh_2->MED_version_major) {
                merged_uh->MED_version_major = UNIVERSAL_HEADER_MED_VERSION_MAJOR_NO_ENTRY_m10; equal = FALSE_m10; }
        if (uh_1->MED_version_minor != uh_2->MED_version_minor) {
                merged_uh->MED_version_minor = UNIVERSAL_HEADER_MED_VERSION_MINOR_NO_ENTRY_m10; equal = FALSE_m10; }
        if (uh_1->byte_order_code != uh_2->byte_order_code) {
                merged_uh->byte_order_code = UNIVERSAL_HEADER_BYTE_ORDER_CODE_NO_ENTRY_m10; equal = FALSE_m10; }
	if (uh_1->session_start_time == UUTC_NO_ENTRY_m10) {
		if (uh_2->session_start_time == UUTC_NO_ENTRY_m10)
			merged_uh->session_start_time = UUTC_NO_ENTRY_m10;
		else
			merged_uh->session_start_time = uh_2->session_start_time;
	} else if (uh_2->session_start_time == UUTC_NO_ENTRY_m10) {
		merged_uh->session_start_time = uh_1->session_start_time;
	} else {
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
        } else if (uh_2->file_start_time == UUTC_NO_ENTRY_m10) {
                merged_uh->file_start_time = uh_1->file_start_time;
        } else {
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
        } else if (uh_2->file_end_time == UUTC_NO_ENTRY_m10) {
                merged_uh->file_end_time = uh_1->file_start_time;
        } else {
                if (uh_1->file_end_time < uh_2->file_end_time) {
                        merged_uh->file_end_time = uh_2->file_end_time;
                        equal = FALSE_m10;
                }
        }
        if (uh_1->number_of_entries < uh_2->number_of_entries) {
                merged_uh->number_of_entries = uh_2->number_of_entries; equal = FALSE_m10; }
        if (uh_1->maximum_entry_size < uh_2->maximum_entry_size) {
                merged_uh->maximum_entry_size = uh_2->maximum_entry_size; equal = FALSE_m10; }
        if (uh_1->segment_number != uh_2->segment_number) {
                merged_uh->segment_number = UNIVERSAL_HEADER_SEGMENT_NUMBER_NO_ENTRY_m10; equal = FALSE_m10; }
        if (memcmp(uh_1->session_name, uh_2->session_name, BASE_FILE_NAME_BYTES_m10))
                memset(merged_uh->session_name, 0, BASE_FILE_NAME_BYTES_m10);
        if (memcmp(uh_1->channel_name, uh_2->channel_name, BASE_FILE_NAME_BYTES_m10))
                memset(merged_uh->channel_name, 0, BASE_FILE_NAME_BYTES_m10);
        if (memcmp(uh_1->anonymized_subject_ID, uh_2->anonymized_subject_ID, BASE_FILE_NAME_BYTES_m10))
                memset(merged_uh->anonymized_subject_ID, 0, UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_BYTES_m10);
        if (uh_1->session_UID != uh_2->session_UID) {
                merged_uh->session_UID = UID_NO_ENTRY_m10; equal = FALSE_m10; }
        if (uh_1->channel_UID != uh_2->channel_UID) {
                merged_uh->channel_UID = UID_NO_ENTRY_m10; equal = FALSE_m10; }
        if (uh_1->segment_UID != uh_2->segment_UID) {
                merged_uh->segment_UID = UID_NO_ENTRY_m10; equal = FALSE_m10; }
        if (uh_1->file_UID != uh_2->file_UID) {
                merged_uh->file_UID = UID_NO_ENTRY_m10; equal = FALSE_m10; }
        if (uh_1->provenance_UID != uh_2->provenance_UID) {
                merged_uh->provenance_UID = UID_NO_ENTRY_m10; equal = FALSE_m10; }
        if (memcmp(uh_1->level_1_password_validation_field, uh_2->level_1_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10)) {
                memset(merged_uh->level_1_password_validation_field, 0, PASSWORD_VALIDATION_FIELD_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(uh_1->level_2_password_validation_field, uh_2->level_2_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10)) {
                memset(merged_uh->level_2_password_validation_field, 0, PASSWORD_VALIDATION_FIELD_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(uh_1->level_3_password_validation_field, uh_2->level_3_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10)) {
                memset(merged_uh->level_3_password_validation_field, 0, PASSWORD_VALIDATION_FIELD_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(uh_1->protected_region, uh_2->protected_region, UNIVERSAL_HEADER_PROTECTED_REGION_BYTES_m10)) {
                memset(merged_uh->protected_region, 0, UNIVERSAL_HEADER_PROTECTED_REGION_BYTES_m10); equal = FALSE_m10; }
        if (memcmp(uh_1->discretionary_region, uh_2->discretionary_region, UNIVERSAL_HEADER_DISCRETIONARY_REGION_BYTES_m10)) {
                memset(merged_uh->discretionary_region, 0, UNIVERSAL_HEADER_DISCRETIONARY_REGION_BYTES_m10); equal = FALSE_m10; }

        return(equal);
}


void    message_m10(si1 *fmt, ...)
{
	va_list args;

	
	if (!(globals_m10->behavior_on_fail & SUPPRESS_MESSAGE_OUTPUT_m10)) {
		va_start(args, fmt);
		UTF8_vprintf_m10(fmt, args);
		va_end(args);
		printf("\n");
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
		string = (si1 *) e_calloc_m10((size_t) (string_bytes + 1), sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);

	native_numerical_length = 0;
	temp = number;
	while(temp) {
		temp /= 10;
		++native_numerical_length;
	}
	if (number <= 0)
		++native_numerical_length;
	
	c = string;
	temp = string_bytes - native_numerical_length;
	while (temp--)
		*c++ = '0';
        
	(void) sprintf(c, "%d", number);
	
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


TERN_m10	process_password_data_m10(si1 *unspecified_password, si1 *L1_password, si1 *L2_password, si1 *L3_password, si1 *L1_hint, si1 *L2_hint, FILE_PROCESSING_STRUCT_m10 *fps)
{
	PASSWORD_DATA_m10       *pwd;
	ui1			sha[SHA_OUTPUT_SIZE_m10];
	si1			L1_password_bytes[PASSWORD_BYTES_m10], L2_password_bytes[PASSWORD_BYTES_m10], L3_password_bytes[PASSWORD_BYTES_m10];
	si1			unspecified_password_bytes[PASSWORD_BYTES_m10], putative_L1_password_bytes[PASSWORD_BYTES_m10];
	si4			i;
	METADATA_SECTION_1_m10	*md1;
	UNIVERSAL_HEADER_m10	*uh;
	
        
	// can't process passwords without a universal header
	if (fps == NULL) {
		warning_message_m10("%s(): FILE_PROCESSING_STRUCT_m10 is NULL", __FUNCTION__);
		return(FALSE_m10);
	}
	
	// returns FALSE_m10 to indicate no encryption/decryption access
	// regardless of what happens, global password structure is set to processed
	
	pwd = fps->password_data;
	memset((void *) pwd, 0, sizeof(PASSWORD_DATA_m10));
	pwd->processed = TRUE_m10;

	// copy password hints to metadata & password data
	uh = fps->universal_header;
	if (uh->type_code == TIME_SERIES_METADATA_FILE_TYPE_CODE_m10 || uh->type_code == VIDEO_METADATA_FILE_TYPE_CODE_m10) {
		md1 = fps->metadata.section_1;
		if (L1_hint != NULL)
			if (*L1_hint)
				strncpy_m10(md1->level_1_password_hint, L1_hint, PASSWORD_HINT_BYTES_m10);
		if (*md1->level_1_password_hint)
			memcpy(pwd->level_1_password_hint, md1->level_1_password_hint, PASSWORD_HINT_BYTES_m10);
		if (L2_hint != NULL)
			if (*L2_hint)
				strncpy_m10(md1->level_2_password_hint, L2_hint, PASSWORD_HINT_BYTES_m10);
		if (*md1->level_2_password_hint)
			memcpy(pwd->level_2_password_hint, md1->level_2_password_hint, PASSWORD_HINT_BYTES_m10);
	} else {  // copy passed password hints to password data
		if (L1_hint != NULL)
			if (*L1_hint)
				strncpy_m10(pwd->level_1_password_hint, L1_hint, PASSWORD_HINT_BYTES_m10);
		if (L2_hint != NULL)
			if (*L2_hint)
				strncpy_m10(pwd->level_2_password_hint, L2_hint, PASSWORD_HINT_BYTES_m10);
	}

	if (unspecified_password == NULL && L1_password == NULL && L2_password == NULL)
		return(FALSE_m10);
	
        // user passed single password for reading: validate against validation fields and generate encryption keys
	if (unspecified_password != NULL) {
		if (check_password_m10(unspecified_password) == TRUE_m10) {
			
			// get terminal bytes
			extract_terminal_password_bytes_m10(unspecified_password, unspecified_password_bytes);
			
			// check for level 1 access
			SHA_sha_m10((ui1 *) unspecified_password_bytes, PASSWORD_BYTES_m10, sha);  // generate SHA-256 hash of password bytes
			
			for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i)  // compare with stored level 1 hash
				if (sha[i] != uh->level_1_password_validation_field[i])
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
				putative_L1_password_bytes[i] = sha[i] ^ uh->level_2_password_validation_field[i];
			
			SHA_sha_m10((ui1 *) putative_L1_password_bytes, PASSWORD_BYTES_m10, sha); // generate SHA-256 hash of putative level 1 password
			
			for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i)  // compare with stored level 1 hash
				if (sha[i] != uh->level_1_password_validation_field[i])
					break;
			if (i == PASSWORD_VALIDATION_FIELD_BYTES_m10) {  // Level 2 password valid
				pwd->access_level = LEVEL_2_ACCESS_m10;
				AES_key_expansion_m10(pwd->level_1_encryption_key, putative_L1_password_bytes);  // generate key
				AES_key_expansion_m10(pwd->level_2_encryption_key, unspecified_password_bytes);  // generate key
				if (globals_m10->verbose == TRUE_m10)
					message_m10("Unspecified password is valid for Level 1 and Level 2 access");
				return(TRUE_m10);
			}
			
			// invalid as level 2 password
			warning_message_m10("%s(): password is not valid for Level 1 or Level 2 access", __FUNCTION__);
		}
		// check_password_m10() == FALSE_m10 or invalid
		show_password_data_m10(pwd);
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
        SHA_sha_m10((ui1 *) L1_password_bytes, PASSWORD_BYTES_m10, sha);
        memcpy(uh->level_1_password_validation_field, sha, PASSWORD_VALIDATION_FIELD_BYTES_m10);
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
			SHA_sha_m10((ui1 *) L2_password_bytes, PASSWORD_BYTES_m10, sha);
			memcpy(uh->level_2_password_validation_field, sha, PASSWORD_VALIDATION_FIELD_BYTES_m10);
			
			// exclusive or with level 1 password bytes
			for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i)
				uh->level_2_password_validation_field[i] ^= L1_password_bytes[i];
			if (globals_m10->verbose == TRUE_m10)
				message_m10("Level 2 password validation field generated");
			
			// generate encryption key
			AES_key_expansion_m10(pwd->level_2_encryption_key, L2_password_bytes);
			if (globals_m10->verbose == TRUE_m10)
				message_m10("Level 2 encryption key generated");
		} else {
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
			SHA_sha_m10((ui1 *) L3_password_bytes, PASSWORD_BYTES_m10, sha);
			memcpy(uh->level_3_password_validation_field, sha, PASSWORD_VALIDATION_FIELD_BYTES_m10);
			if (pwd->access_level == LEVEL_1_ACCESS_m10) {  // only level 1 password passed
				for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i) // exclusive or with level 1 password bytes
					uh->level_3_password_validation_field[i] ^= L1_password_bytes[i];
			} else {  // level 1 & level 2 passwords passed
				for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i) // exclusive or with level 2 password bytes
					uh->level_3_password_validation_field[i] ^= L2_password_bytes[i];
			}
			if (globals_m10->verbose == TRUE_m10)
				message_m10("Level 3 password validation field generated");
		} else {
			// check_password_m10() == FALSE_m10
			return(FALSE_m10);
		}
	}
        
	return(TRUE_m10);
}


CHANNEL_m10     *read_channel_m10(CHANNEL_m10 *chan, si1 *chan_dir, TIME_SLICE_m10 *slice, si1 *password, TERN_m10 read_time_series_data, TERN_m10 read_record_data)
{
        si1                     full_file_name[FULL_FILE_NAME_BYTES_m10], num_str[FILE_NUMBERING_DIGITS_m10 + 1];
        ui4                     code;
        si4                     i, n_segs;
        si8                     items_read;
        SEGMENT_m10             *seg;
    

	// allocate channel
	if (chan == NULL)
		chan = (CHANNEL_m10 *) e_calloc_m10((size_t) 1, sizeof(CHANNEL_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);

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
                error_message_m10("%s(): No channel directory passed", __FUNCTION__);
                return(NULL);
        }

        switch (code) {
                case TIME_SERIES_CHANNEL_TYPE_m10:
                case VIDEO_CHANNEL_TYPE_m10:
                        break;
                default:
                        error_message_m10("%s(): input file is not MED channel directory", __FUNCTION__);
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
        if (slice->start_segment_number == SEGMENT_NUMBER_NO_ENTRY_m10 || slice->end_segment_number == SEGMENT_NUMBER_NO_ENTRY_m10)
                n_segs = get_segment_range_m10((si1 **) &chan->path, 1, slice);
        else
                n_segs = (slice->end_segment_number - slice->start_segment_number) + 1;
        chan->number_of_segments = n_segs;
        
        // allocate segments
        chan->segments = (SEGMENT_m10 **) e_calloc_2D_m10(n_segs, 1, sizeof(SEGMENT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
        // generate segment names
        for (i = 0; i < n_segs; ++i) {
                seg = chan->segments[i];
                numerical_fixed_width_string_m10(num_str, FILE_NUMBERING_DIGITS_m10, i + slice->start_segment_number);
                switch (code) {
                        case TIME_SERIES_CHANNEL_TYPE_m10:
                                sprintf(full_file_name, "%s/%s_s%s.%s", chan->path, chan->name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m10);
                                break;
                        case VIDEO_CHANNEL_TYPE_m10:
                                sprintf(full_file_name, "%s/%s_s%s.%s", chan->path, chan->name, num_str, VIDEO_SEGMENT_DIRECTORY_TYPE_STRING_m10);
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
	slice->start_index = chan->segments[0]->time_slice.start_index;
	slice->end_time = chan->segments[n_segs - 1]->time_slice.end_time;
	slice->end_index = chan->segments[n_segs - 1]->time_slice.end_index;
	slice->number_of_samples = (slice->end_index - slice->start_index) + 1;
	slice->local_start_index = slice->local_end_index = SAMPLE_NUMBER_NO_ENTRY_m10;

        // create channel-level metadata FPS
        seg = chan->segments[0];
        if (code == TIME_SERIES_CHANNEL_TYPE_m10) {
                sprintf(full_file_name, "%s/%s.%s", chan->path, chan->name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
                chan->metadata_fps = allocate_file_processing_struct_m10(NULL, full_file_name, TIME_SERIES_METADATA_FILE_TYPE_CODE_m10, METADATA_BYTES_m10, seg->metadata_fps, METADATA_BYTES_m10);
        } else {  // VIDEO_CHANNEL_TYPE_m10
                sprintf(full_file_name, "%s/%s.%s", chan->path, chan->name, VIDEO_METADATA_FILE_TYPE_STRING_m10);
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
        sprintf(full_file_name, "%s/%s.%s", chan->path, chan->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
        if (file_exists_m10(full_file_name) == FILE_EXISTS_m10) {
                chan->record_indices_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
                // read record data files (if present)
                sprintf(full_file_name, "%s/%s.%s", chan->path, chan->name, RECORD_DATA_FILE_TYPE_STRING_m10);
                if (file_exists_m10(full_file_name) == FILE_EXISTS_m10) {
                        if (read_record_data == TRUE_m10) {
                                chan->record_data_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
                        } else {
                                chan->record_data_fps = allocate_file_processing_struct_m10(NULL, full_file_name, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, NULL, 0);
                                chan->record_data_fps->directives.close_file = FALSE_m10;
                                read_file_m10(chan->record_data_fps, full_file_name, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
                        }
                        merge_universal_headers_m10(chan->metadata_fps, chan->record_data_fps, NULL);
                } else {
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
		printf("-------------- Channel Universal Header ----------------\n");
                show_universal_header_m10(chan->metadata_fps, NULL);
		printf("------------------ Channel Metadata --------------------\n");
                show_metadata_m10(chan->metadata_fps, NULL);
        }
        
	return(chan);
}


FILE_PROCESSING_STRUCT_m10      *read_file_m10(FILE_PROCESSING_STRUCT_m10 *fps, si1 *full_file_name, si8 number_of_items, ui1 **data_ptr_ptr, si8 *items_read, si1 *password, ui4 behavior_on_fail)
{
        ui1                             *data_ptr, *raw_data_end;
        TERN_m10                        full_file_flag, readable, allocated_flag, external_array;
        si8                             i, in_bytes, tmp_in_bytes, bytes_left_in_file, used_bytes, required_bytes;
        UNIVERSAL_HEADER_m10            *uh;
        RECORD_HEADER_m10               *record_header;
        CMP_BLOCK_FIXED_HEADER_m10      *block_header;


        if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
                behavior_on_fail = globals_m10->behavior_on_fail;
	
	if (items_read != NULL)
		*items_read = 0;

	allocated_flag = FALSE_m10;
	if (fps == NULL) {
		if (full_file_name == NULL) {
			warning_message_m10("%s(): FILE_PROCESSING_STRUCT_m10 and full_file_name are both NULL", __FUNCTION__);
			return(NULL);
		}
                fps = allocate_file_processing_struct_m10(NULL, full_file_name, NO_FILE_TYPE_CODE_m10, 0, NULL, 0);
		allocated_flag = TRUE_m10;
	}

        uh = fps->universal_header;
        if (fps->fp == NULL) {
		// read universal header
                if (!(fps->directives.open_mode & FPS_GENERIC_READ_OPEN_MODE_m10))
                        fps->directives.open_mode = FPS_R_OPEN_MODE_m10;
                fps_open_m10(fps, __FUNCTION__, __LINE__, behavior_on_fail);
		fps_read_m10(fps, UNIVERSAL_HEADER_BYTES_m10, (void *) uh, __FUNCTION__, __LINE__, behavior_on_fail);
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
        } else {
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
	} else {
		data_ptr = fps->raw_data + UNIVERSAL_HEADER_BYTES_m10;
	}
	
	// reallocate raw_data memory space if necessary
	if (external_array == FALSE_m10) {
		used_bytes = (si8) data_ptr - (si8) fps->raw_data;
		required_bytes = used_bytes + in_bytes;
		if (fps->raw_data_bytes < required_bytes) {
			reallocate_file_processing_struct_m10(fps, required_bytes);
			data_ptr = fps->raw_data + used_bytes;
			if (data_ptr_ptr != NULL)
				*data_ptr_ptr = data_ptr;
		}
	}

        // read data
        fps_read_m10(fps, in_bytes, data_ptr, __FUNCTION__, __LINE__, behavior_on_fail);
        switch (uh->type_code) {
                case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
                        // this function is fine, but reading using time series indices is more efficient for reading CMP_blocks
                        block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) data_ptr;
                        for (tmp_in_bytes = i = 0; i < number_of_items; ++i) {
                                tmp_in_bytes += block_header->total_block_bytes;
                                if (tmp_in_bytes > bytes_left_in_file) {
                                        tmp_in_bytes -= block_header->total_block_bytes;
                                        break;
                                }
                                block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *) block_header + block_header->total_block_bytes);
                        }
                        number_of_items = i;
                        e_fseek_m10(fps->fp, tmp_in_bytes - in_bytes, SEEK_CUR, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);
                        in_bytes = tmp_in_bytes;
                        break;
                case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
                        number_of_items = in_bytes / TIME_SERIES_INDEX_BYTES_m10;
                        break;
                case RECORD_DATA_FILE_TYPE_CODE_m10:
                        // this function is fine, but reading using record indices is more efficient for reading records
                        record_header = (RECORD_HEADER_m10 *) data_ptr;
                        for (tmp_in_bytes = i = 0; i < number_of_items; ++i) {
                                tmp_in_bytes += record_header->total_record_bytes;
                                if (tmp_in_bytes > bytes_left_in_file) {
                                        tmp_in_bytes -= record_header->total_record_bytes;
                                        break;
                                }
                                record_header = (RECORD_HEADER_m10 *) ((ui1 *) record_header + record_header->total_record_bytes);
                        }
                        number_of_items = i;
                        e_fseek_m10(fps->fp, tmp_in_bytes - in_bytes, SEEK_CUR, fps->full_file_name, __FUNCTION__, __LINE__, behavior_on_fail);
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
                                validate_time_series_data_CRCs_m10((CMP_BLOCK_FIXED_HEADER_m10 *) data_ptr, number_of_items);
                                break;
                        case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
                                if (full_file_flag == TRUE_m10)
                                        CRC_validate_m10(fps->raw_data + UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10, fps->file_length - UNIVERSAL_HEADER_BODY_CRC_START_OFFSET_m10, uh->body_CRC);
                                break;
                        case RECORD_DATA_FILE_TYPE_CODE_m10:
                                validate_record_data_CRCs_m10((RECORD_HEADER_m10 *) data_ptr, number_of_items);
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
			fps->cps->block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) data_ptr;
			readable = decrypt_time_series_data_m10(fps->cps, number_of_items);
                        break;
                case RECORD_DATA_FILE_TYPE_CODE_m10:
			readable = decrypt_records_m10(fps, (RECORD_HEADER_m10 *) data_ptr, number_of_items);
                        break;
                case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
                case VIDEO_METADATA_FILE_TYPE_CODE_m10:
			readable = decrypt_metadata_m10(fps);
                        break;
		default:
			readable = TRUE_m10;  // file types without encryption
			break;
        }
	if (readable == FALSE_m10) {
		error_message_m10("%s(): Cannot read file \"%s\"", __FUNCTION__, fps->full_file_name);
		if (allocated_flag == TRUE_m10)
			free_file_processing_struct_m10(fps, FALSE_m10);
		return(NULL);
	}

	// show
	if (globals_m10->verbose == TRUE_m10)
		show_file_processing_struct_m10(fps);
                
	return(fps);
}


SEGMENT_m10     *read_segment_m10(SEGMENT_m10 *seg, si1 *seg_dir, TIME_SLICE_m10 *slice, si1 *password, TERN_m10 read_time_series_data, TERN_m10 read_record_data)
{
        ui1                                     search_mode;
	si1                                     full_file_name[FULL_FILE_NAME_BYTES_m10];
        ui4                                     code;
        si4                                     seg_abs_start_sample_number, seg_abs_end_sample_number;
        si8                                     items_read, local_start_idx, local_end_idx;
        sf8                                     sampling_frequency;
        TIME_SERIES_METADATA_SECTION_2_m10      *tmd2;
	
        
	// allocate segment
	if (seg == NULL)
		seg = (SEGMENT_m10 *) e_calloc_m10((size_t) 1, sizeof(SEGMENT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
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
        } else if (slice->start_index != SAMPLE_NUMBER_NO_ENTRY_m10 && slice->end_index != SAMPLE_NUMBER_NO_ENTRY_m10) {
                search_mode = INDEX_SEARCH_m10;
        } else {
                error_message_m10("%s(): no valid limit pair", __FUNCTION__);
                return(0);
        }

	// get segment path & name
        if (seg_dir == NULL) {
                if (seg->path[0] != 0) {
                        seg_dir = seg->path;
                } else {
                        error_message_m10("%s(): no segment directory passed", __FUNCTION__);
                        return(NULL);
                }
        }
        code = generate_MED_path_components_m10(seg_dir, seg->path, seg->name);

	// read segment metadata
	switch (code) {
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			snprintf_m10(full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
			break;
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			snprintf_m10(full_file_name, FULL_FILE_NAME_BYTES_m10, "%s/%s.%s", seg->path, seg->name, VIDEO_METADATA_FILE_TYPE_STRING_m10);
			break;
		default:
			error_message_m10("%s(): unrecognized type code in file \"%s\"", __FUNCTION__, seg->path);
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
                        error_message_m10("%s(): unrecognized type code in file \"%s\"", __FUNCTION__, full_file_name);
                        return(NULL);
        }

        // get local indices
        if (code == TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10) {

                tmd2 = seg->metadata_fps->metadata.time_series_section_2;
                sampling_frequency = tmd2->sampling_frequency;
		seg_abs_start_sample_number = tmd2->absolute_start_sample_number;
		seg_abs_end_sample_number = (seg_abs_start_sample_number + tmd2->number_of_samples) - 1;

                // get local indices
                if (search_mode == INDEX_SEARCH_m10) {  // convert absolute indices to local indices
                        if (slice->start_index == BEGINNING_OF_INDICES_m10)
                                slice->start_index = seg_abs_start_sample_number;
                        if (slice->end_index == END_OF_INDICES_m10)
                                slice->end_index = seg_abs_end_sample_number;
                        local_start_idx = slice->start_index;
                        local_end_idx = slice->end_index;
                        if (slice->start_index > seg_abs_end_sample_number) {
                                local_start_idx -= seg_abs_start_sample_number;
                                local_end_idx -= seg_abs_start_sample_number;
                        }
                        if (local_end_idx >= tmd2->number_of_samples)
                                local_end_idx = tmd2->number_of_samples - 1;
                } else {  // search_mode == TIME_SEARCH_m10, convert input times to local indices
                        local_start_idx = sample_number_for_uutc_m10(SAMPLE_NUMBER_NO_ENTRY_m10, UUTC_NO_ENTRY_m10, slice->start_time, sampling_frequency, seg->time_series_indices_fps, FIND_CURRENT_m10);
                        local_end_idx = sample_number_for_uutc_m10(SAMPLE_NUMBER_NO_ENTRY_m10, UUTC_NO_ENTRY_m10, slice->end_time, sampling_frequency, seg->time_series_indices_fps, FIND_CURRENT_m10);
                }
		
		slice->start_time = uutc_for_sample_number_m10(SAMPLE_NUMBER_NO_ENTRY_m10, UUTC_NO_ENTRY_m10, local_start_idx, tmd2->sampling_frequency, seg->time_series_indices_fps, FIND_START_m10);
		slice->end_time = uutc_for_sample_number_m10(SAMPLE_NUMBER_NO_ENTRY_m10, UUTC_NO_ENTRY_m10, local_end_idx, tmd2->sampling_frequency, seg->time_series_indices_fps, FIND_END_m10);
		
		// update slice
		slice->start_index = seg_abs_start_sample_number + local_start_idx;
		slice->end_index = seg_abs_start_sample_number + local_end_idx;
		slice->local_start_index = local_start_idx;
		slice->local_end_index = local_end_idx;
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
                                (void) read_time_series_data_m10(seg, local_start_idx, local_end_idx, TRUE_m10);
                                fps_close_m10(seg->time_series_data_fps);
                        }
			break;
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			// video segment data are native video files
			break;
		default:
			error_message_m10("%s(): unrecognized type code in file \"%s\"", __FUNCTION__, seg->path);
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
                        } else {
                                seg->record_data_fps = allocate_file_processing_struct_m10(NULL, full_file_name, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, NULL, 0);
                                seg->record_data_fps->directives.close_file = FALSE_m10;
                                read_file_m10(seg->record_data_fps, full_file_name, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
                        }
                } else {
                        warning_message_m10("%s(): Segment record data file does not exist (\"%s\"), but indices file does", __FUNCTION__, full_file_name);
                }
        }

	return(seg);
}


SESSION_m10     *read_session_m10(si1 *sess_dir, si1 **chan_list, si4 n_chans, TIME_SLICE_m10 *slice, si1 *password, TERN_m10 read_time_series_data, TERN_m10 read_record_data)
{
        si1                             full_file_name[FULL_FILE_NAME_BYTES_m10], sess_path[FULL_FILE_NAME_BYTES_m10];
        si1                             tmp_str[FULL_FILE_NAME_BYTES_m10];
        si1                             num_str[FILE_NUMBERING_DIGITS_m10 + 1], **seg_rec_file_names;
        si1                             **ts_chan_list, **vid_chan_list;
        ui4                             type_code;
        si4                             n_ts_chans, n_vid_chans, n_segs;
	si4                             i, j, k, num_seg_rec_files;
        si8                             items_read;
	TIME_SLICE_m10			*chan_slice;
        SESSION_m10                     *sess;
	CHANNEL_m10                     *chan;
        FILE_PROCESSING_STRUCT_m10      *gen_fps;
	
        
	// allocate session
	sess = (SESSION_m10 *) e_calloc_m10((size_t) 1, sizeof(SESSION_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);

        // read whole session
        if (slice == NULL)
                slice = initialize_time_slice_m10(&sess->time_slice);
	else
		sess->time_slice = *slice;  // passed slice is not modified
	slice = &sess->time_slice;
	
        // expand channel list
        chan_list = generate_file_list_m10(chan_list, n_chans, &n_chans, sess_dir, NULL, "[tv]icd", PP_FULL_PATH_m10, FALSE_m10);
        if (n_chans == 0) {
                error_message_m10("%s(): no matching MED channels", __FUNCTION__);
                return(NULL);
        }

        // check that all files are MED channels in the same MED session directory
        // (really should check that they have the same session UIDs, & not require they be in the same directory)
        extract_path_parts_m10(chan_list[0], sess_path, NULL, NULL);
        type_code = MED_type_code_from_string_m10(sess_path);
        if (type_code != SESSION_DIRECTORY_TYPE_CODE_m10) {
                error_message_m10("%s(): files must in a MED session directory", __FUNCTION__);
                return(NULL);
        }
	
        n_ts_chans = n_vid_chans = 0;
        for (i = 0; i < n_chans; ++i) {
                extract_path_parts_m10(chan_list[i], tmp_str, NULL, NULL);
                if (strcmp(sess_path, tmp_str) != 0) {
                        error_message_m10("%s(): channel files must all be in the same directory", __FUNCTION__);
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
                                error_message_m10("%s(): channel files must all be MED channel directories", __FUNCTION__);
                                return(NULL);
                }
        }

        // divide channel lists
        if (n_ts_chans)
                ts_chan_list = (si1 **) e_calloc_2D_m10(n_ts_chans, FULL_FILE_NAME_BYTES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        if (n_vid_chans)
                vid_chan_list = (si1 **) e_calloc_2D_m10(n_vid_chans, FULL_FILE_NAME_BYTES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
        e_free_2D_m10((void **) chan_list, n_chans, __FUNCTION__, __LINE__);
	
	// password
	generate_MED_path_components_m10(sess_path, sess->path, sess->name);
	if (globals_m10->password_data.processed == 0)
		if (set_time_and_password_data_m10(password, sess->path, NULL, NULL) == FALSE_m10)
			return(NULL);

	// process slice (for unoffset & relative times)
	if (slice->conditioned == FALSE_m10)
		condition_time_slice_m10(slice);
	
        // get segment range
        if (n_ts_chans) {
                n_segs = get_segment_range_m10(ts_chan_list, n_ts_chans, slice);
        } else {  // n_vid_chans != 0
                if (slice->start_time != UUTC_NO_ENTRY_m10 && slice->end_time != UUTC_NO_ENTRY_m10) {
                        n_segs = get_segment_range_m10(vid_chan_list, n_vid_chans, slice);
                } else {
                        error_message_m10("%s(): cannot search video channels by indices (frames) for segments at this time", __FUNCTION__);
                        return(NULL);
                }
        }

        sess->number_of_time_series_channels = n_ts_chans;
        sess->number_of_video_channels = n_vid_chans;
        sess->number_of_segments = n_segs;
        
        // read time series channels
        if (n_ts_chans) {
                sess->time_series_channels = (CHANNEL_m10 **) e_calloc_2D_m10(n_ts_chans, 1, sizeof(CHANNEL_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
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
			if (chan_slice->start_index != slice->start_index || chan_slice->end_index != slice->end_index) {
				slice->start_index = slice->end_index = SAMPLE_NUMBER_NO_ENTRY_m10;
				slice->number_of_samples = NUMBER_OF_SAMPLES_NO_ENTRY_m10;
				break;
			}
		}

		// create session level metadata FPS
                sprintf(full_file_name, "%s/%s.%s", sess->path, sess->name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
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
                e_free_2D_m10((void **) ts_chan_list, n_ts_chans, __FUNCTION__, __LINE__);
        }

        // read video channels
        if (n_vid_chans) {
                sess->video_channels = (CHANNEL_m10 **) e_calloc_2D_m10(n_vid_chans, 1, sizeof(CHANNEL_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                sess->video_metadata_fps->fd = FPS_FD_EPHEMERAL_m10;
                for (i = 0; i < n_vid_chans; ++i) {
                        chan = sess->video_channels[i];
                        strncpy_m10(chan->path, vid_chan_list[i], FULL_FILE_NAME_BYTES_m10);
                        if (read_channel_m10(chan, chan->path, slice, NULL, read_time_series_data, read_record_data) == NULL)
				return(NULL);
                }

                // create session level metadata FPS
                sprintf(full_file_name, "%s/%s.%s", sess->path, sess->name, VIDEO_METADATA_FILE_TYPE_STRING_m10);
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
                e_free_2D_m10((void **) vid_chan_list, n_vid_chans, __FUNCTION__, __LINE__);
        }

        // if session has no channels, it can still have records, but it's probably a bad session
        if (n_ts_chans == 0 && n_vid_chans == 0)
                warning_message_m10("%s(): session contains no channels", __FUNCTION__);

        // if session has no segments, it can still have records, but it's probably a bad session
        if (n_segs == 0)
                warning_message_m10("%s(): session contains no segments", __FUNCTION__);

        // read session record indices (if present)
        sprintf(full_file_name, "%s/%s.%s", sess->path, sess->name, RECORD_INDICES_FILE_TYPE_STRING_m10);
        if (file_exists_m10(full_file_name) == FILE_EXISTS_m10) {
                sess->record_indices_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);

                // read record data files (if present)
                sprintf(full_file_name, "%s/%s.%s", sess->path, sess->name, RECORD_DATA_FILE_TYPE_STRING_m10);
                if (file_exists_m10(full_file_name) == FILE_EXISTS_m10) {
                        if (read_record_data == TRUE_m10) {
                                sess->record_data_fps = read_file_m10(NULL, full_file_name, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
                        } else {
                                sess->record_data_fps = allocate_file_processing_struct_m10(NULL, full_file_name, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, NULL, 0);
                                sess->record_data_fps->directives.close_file = FALSE_m10;
                                read_file_m10(sess->record_data_fps, NULL, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
                        }
                        if (n_ts_chans) {
                                merge_universal_headers_m10(sess->time_series_metadata_fps, sess->record_data_fps, NULL);
                        }
                        if (n_vid_chans)
                                merge_universal_headers_m10(sess->video_metadata_fps, sess->record_data_fps, NULL);
                } else {
                        warning_message_m10("%s(): Session record data file does not exist (\"%s\"), but indices file does", __FUNCTION__, full_file_name);
                }
        }

        // check if segmented session records present
        sprintf(full_file_name, "%s/%s.%s", sess->path, sess->name, RECORD_DIRECTORY_TYPE_STRING_m10);
        num_seg_rec_files = 0;
        seg_rec_file_names = generate_file_list_m10(NULL, num_seg_rec_files, &num_seg_rec_files, full_file_name, NULL, RECORD_INDICES_FILE_TYPE_STRING_m10, PP_FULL_PATH_m10, FALSE_m10);
        e_free_2D_m10((void **) seg_rec_file_names, num_seg_rec_files, __FUNCTION__, __LINE__);  // Not using the names, because not every segment must have records,
												 // just seeing if there are ANY segmented session records.
                                                                                                 // Use indices of segments instead of looping over names.
        // read segmented session records
        if (num_seg_rec_files) {
                sess->segmented_record_indices_fps = (FILE_PROCESSING_STRUCT_m10 **) e_calloc_2D_m10((size_t) sess->number_of_segments, 1, sizeof(FILE_PROCESSING_STRUCT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                sess->segmented_record_data_fps = (FILE_PROCESSING_STRUCT_m10 **) e_calloc_2D_m10((size_t) sess->number_of_segments, 1, sizeof(FILE_PROCESSING_STRUCT_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                for (i = 0; i < sess->number_of_segments; ++i) {
                        
                        // read segmented record indices file (if present)
                        gen_fps = sess->segmented_record_indices_fps[i];
                        numerical_fixed_width_string_m10(num_str, FILE_NUMBERING_DIGITS_m10, i + slice->start_segment_number);
                        sprintf(gen_fps->full_file_name, "%s/%s.%s/%s_s%s.%s", sess->path, sess->name, RECORD_DIRECTORY_TYPE_STRING_m10, sess->name, num_str, RECORD_INDICES_FILE_TYPE_STRING_m10);
                        if (file_exists_m10(gen_fps->full_file_name) == FILE_EXISTS_m10) {
                                allocate_file_processing_struct_m10(gen_fps, NULL, RECORD_INDICES_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, NULL, 0);
                                read_file_m10(gen_fps, NULL, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
                                // read segmented record data file (if present)
                                gen_fps = sess->segmented_record_data_fps[i];
                                sprintf(gen_fps->full_file_name, "%s/%s.%s/%s_s%s.%s", sess->path, sess->name, RECORD_DIRECTORY_TYPE_STRING_m10, sess->name, num_str, RECORD_DATA_FILE_TYPE_STRING_m10);
                                if (file_exists_m10(gen_fps->full_file_name) == FILE_EXISTS_m10) {
                                        allocate_file_processing_struct_m10(gen_fps, NULL, RECORD_DATA_FILE_TYPE_CODE_m10, LARGEST_RECORD_BYTES_m10, NULL, 0);
                                        if (read_record_data == TRUE_m10) {
                                                read_file_m10(gen_fps, NULL, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
                                        } else {
                                                gen_fps->directives.close_file = FALSE_m10;
                                                read_file_m10(gen_fps, NULL, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
                                        }
                                        if (sess->number_of_time_series_channels) {
                                                merge_universal_headers_m10(sess->time_series_metadata_fps, gen_fps, NULL);
                                        }
                                        if (sess->number_of_video_channels)
                                                merge_universal_headers_m10(sess->video_metadata_fps, gen_fps, NULL);
                                } else {
                                        warning_message_m10("%s(): Session segmented record data file does not exist (\"%s\"), but indices file does", __FUNCTION__, full_file_name);
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
			printf("--------- Session Time Series Universal Header ---------\n");
                        show_universal_header_m10(sess->time_series_metadata_fps, NULL);
			printf("------------ Session Time Series Metadata --------------\n");
			show_metadata_m10(sess->time_series_metadata_fps, NULL);
		}
		if (sess->number_of_video_channels > 0) {
			printf("------------ Session Video Universal Header ------------\n");
                        show_universal_header_m10(sess->video_metadata_fps, NULL);
			printf("--------------- Session Video Metadata -----------------\n");
                        show_metadata_m10(sess->video_metadata_fps, NULL);
		}
	}

	return(sess);
}


si8     read_time_series_data_m10(SEGMENT_m10 *seg, si8 local_start_idx, si8 local_end_idx, TERN_m10 alloc_cps)
{
        si8                                     i, n_ts_inds, n_samps, samp_num;
        si8                                     offset_pts, start_block, end_block, compressed_data_bytes;
        FILE_PROCESSING_STRUCT_m10              *tsd_fps, *tsi_fps;
        TIME_SERIES_INDEX_m10                   *tsi;
        TIME_SERIES_METADATA_SECTION_2_m10      *tmd2;
        CMP_PROCESSING_STRUCT_m10               *cps;
        CMP_BLOCK_FIXED_HEADER_m10              *bh;

	
        // local_start_idx and local_end_idx are segment relative
        
        if (seg == NULL) {
                error_message_m10("%s(): SEGMENT_m10 structure is NULL", __FUNCTION__);
                return(-1);
        }

        tsd_fps = seg->time_series_data_fps;
        tsi_fps = seg->time_series_indices_fps;
        
        if (tsd_fps == NULL) {
                error_message_m10("%s(): time series data FILE_PROCESSING_STRUCT_m10 is NULL", __FUNCTION__);
                return(-1);
        }
        if (tsi_fps == NULL) {
                error_message_m10("%s(): time series indices FILE_PROCESSING_STRUCT_m10 is NULL", __FUNCTION__);
                return(-1);
        }
        if (tsd_fps->cps == NULL && alloc_cps == FALSE_m10) {
                error_message_m10("%s(): CMP_PROCESSING_STRUCT_m10 is NULL", __FUNCTION__);
                return(-1);
        }

        n_ts_inds = tsi_fps->universal_header->number_of_entries;
        tsi = tsi_fps->time_series_indices;
        tmd2 = seg->metadata_fps->metadata.time_series_section_2;

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
        } else {
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
        } else if (tsd_fps->cps == NULL) {
		error_message_m10("%s(): no CMP_PROCESSING_STRUCT allocated", __FUNCTION__);
		return(-1);
        }
        cps = tsd_fps->cps;

        // read in compressed data
        e_fseek_m10(tsd_fps->fp, REMOVE_DISCONTINUITY_m10(tsi[start_block].file_offset), SEEK_SET, tsd_fps->full_file_name, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        fps_read_m10(tsd_fps, compressed_data_bytes, cps->compressed_data, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);

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


si4	reallocate_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps, si8 new_raw_data_bytes)
{
	void	*data_ptr;
	
        
	// reallocate
	fps->raw_data = (ui1 *) e_realloc_m10((void *) fps->raw_data, (size_t) new_raw_data_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);

	// zero additional memory (realloc() copies existing memory if necessary, but does not zero additional memory allocated)
	if (new_raw_data_bytes > fps->raw_data_bytes)
		memset(fps->raw_data + fps->raw_data_bytes, 0, new_raw_data_bytes - fps->raw_data_bytes);
        fps->raw_data_bytes = new_raw_data_bytes;

	// reset universal header pointer
	fps->universal_header = (UNIVERSAL_HEADER_m10 *) fps->raw_data; // all files start with universal header

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
			fps->metadata.section_1 = (METADATA_SECTION_1_m10 *) (fps->metadata.metadata = data_ptr);
			fps->metadata.time_series_section_2 = (TIME_SERIES_METADATA_SECTION_2_m10 *) (fps->raw_data + METADATA_SECTION_2_OFFSET_m10);
			fps->metadata.video_section_2 = (VIDEO_METADATA_SECTION_2_m10 *) fps->metadata.time_series_section_2;
			fps->metadata.section_3 = (METADATA_SECTION_3_m10 *) (fps->raw_data + METADATA_SECTION_3_OFFSET_m10);
			break;
		case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
                        // CMP_PROCESSING_STRUCT allocation done seperately with CMP_allocate_processing_struct()
			break;
                case RECORD_DATA_FILE_TYPE_CODE_m10:
                        fps->records = (ui1 *) data_ptr;
                        break;
                case RECORD_INDICES_FILE_TYPE_CODE_m10:
                        fps->record_indices = (RECORD_INDEX_m10 *) data_ptr;
                        break;
                default:
                        error_message_m10("%s(): unrecognized file type code (code == 0x%4x)", __FUNCTION__, fps->universal_header->type_code);
                        return(-1);
        }

	return(0);
}


TERN_m10    recover_passwords_m10(si1 *L3_password, UNIVERSAL_HEADER_m10 *universal_header)
{
        ui1     sha[SHA_OUTPUT_SIZE_m10], L3_sha[SHA_OUTPUT_SIZE_m10];
        si1     L3_password_bytes[PASSWORD_BYTES_m10], hex_str[HEX_STRING_BYTES_m10(PASSWORD_BYTES_m10)];
        si1     putative_L1_password_bytes[PASSWORD_BYTES_m10], putative_L2_password_bytes[PASSWORD_BYTES_m10];
        si4     i;

        
        if (check_password_m10(L3_password) == FALSE_m10)
                return(FALSE_m10);
                
        // get terminal bytes
        extract_terminal_password_bytes_m10(L3_password, L3_password_bytes);
        
        // get level 3 password hash
        SHA_sha_m10((ui1 *) L3_password_bytes, PASSWORD_BYTES_m10, L3_sha);  // generate SHA-256 hash of level 3 password bytes
        
        // check for level 1 access
        for (i = 0; i < PASSWORD_BYTES_m10; ++i)  // xor with level 3 password validation field
                putative_L1_password_bytes[i] = L3_sha[i] ^ universal_header->level_3_password_validation_field[i];
        
        SHA_sha_m10((ui1 *) putative_L1_password_bytes, PASSWORD_BYTES_m10, sha); // generate SHA-256 hash of putative level 1 password
        
        for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i)  // compare with stored level 1 hash
                if (sha[i] != universal_header->level_1_password_validation_field[i])
                        break;
        if (i == PASSWORD_VALIDATION_FIELD_BYTES_m10) {  // Level 1 password recovered
                generate_hex_string_m10((ui1 *) putative_L1_password_bytes, PASSWORD_BYTES_m10, hex_str);
		message_m10("Level 1 password (bytes): '%s' (%s)", putative_L1_password_bytes, hex_str);
		return(TRUE_m10);
        }
        
        // invalid for level 1 (alone) => check if level 2 password
        memcpy(putative_L2_password_bytes, putative_L1_password_bytes, PASSWORD_BYTES_m10);
        for (i = 0; i < PASSWORD_BYTES_m10; ++i)  // xor with level 2 password validation field
                putative_L1_password_bytes[i] = sha[i] ^ universal_header->level_2_password_validation_field[i];
        
        SHA_sha_m10((ui1 *) putative_L1_password_bytes, PASSWORD_BYTES_m10, sha); // generate SHA-256 hash of putative level 1 password
        
        for (i = 0; i < PASSWORD_VALIDATION_FIELD_BYTES_m10; ++i)  // compare with stored level 1 hash
                if (sha[i] != universal_header->level_1_password_validation_field[i])
                        break;

        if (i == PASSWORD_VALIDATION_FIELD_BYTES_m10) {  // Level 2 password valid
                generate_hex_string_m10((ui1 *) putative_L1_password_bytes, PASSWORD_BYTES_m10, hex_str);
		message_m10("Level 1 password (bytes): '%s' (%s)", putative_L1_password_bytes, hex_str);
                generate_hex_string_m10((ui1 *) putative_L2_password_bytes, PASSWORD_BYTES_m10, hex_str);
		message_m10("Level 2 password (bytes): '%s' (%s)", putative_L2_password_bytes, hex_str);
        } else {
                warning_message_m10("%s(): the passed password is not valid for Level 3 access", __FUNCTION__, __LINE__);
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
        TIME_SERIES_METADATA_SECTION_2_m10      *tmd2;
        VIDEO_METADATA_SECTION_2_m10            *vmd2;
        
        
        // section 2 fields
        switch (fps->universal_header->type_code) {
                case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
                        tmd2 = fps->metadata.time_series_section_2;
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
                        vmd2 = fps->metadata.video_section_2;
                        vmd2->number_of_clips = 0;
                        vmd2->maximum_clip_bytes = 0;
                        vmd2->number_of_video_files = 0;
                        break;
                default:
                        error_message_m10("%s(): Unrecognized metadata type in file \"%s\"", __FUNCTION__, fps->full_file_name);
                        break;
        }
                
        return;
}


si8     sample_number_for_uutc_m10(si8 ref_sample_number, si8 ref_uutc, si8 target_uutc, sf8 sampling_frequency, FILE_PROCESSING_STRUCT_m10 *time_series_indices_fps, ui1 mode)
{
        si8                     samp_num, n_inds, i, absolute_numbering_offset, tmp_si8;
        sf8                     tmp_sf8;
        TIME_SERIES_INDEX_m10   *tsi;

        // sample_number_for_uutc_m10(ref_sample_number, ref_uutc, target_uutc, sampling_frequency, NULL, 0, mode)
        // returns sample number extrapolated from ref_sample_number
        
        // sample_number_for_uutc_m10(SAMPLE_NUMBER_NO_ENTRY_m10, UUTC_NO_ENTRY_m10, target_uutc, sampling_frequency, tsi, number_of_indices, mode)
        // returns sample number extrapolated from closest time series index in local (segment-relative) sample numbering
        
        // sample_number_for_uutc_m10(ref_sample_number, UUTC_NO_ENTRY_m10, target_uutc, sampling_frequency, tsi, number_of_indices, mode)
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
			sampling_frequency = (sf8) (tsi[i].start_sample_number - ref_sample_number);
			sampling_frequency /= ((sf8) (tsi[i].start_time - ref_uutc) / (sf8) 1e6);
		}
        }

        tmp_sf8 = ((sf8) (target_uutc - ref_uutc) / (sf8) 1e6) * sampling_frequency;
        switch (mode) {
                case FIND_CLOSEST_m10:
                        tmp_si8 = (si8) (tmp_sf8 + (sf8) 0.5);
                        break;
                case FIND_NEXT_m10:
                        tmp_si8 = (si8) tmp_sf8 + 1;
                        break;
                case FIND_CURRENT_m10:
                default:
                        tmp_si8 = (si8) tmp_sf8;
                        break;
        }
        samp_num = ref_sample_number + tmp_si8 + absolute_numbering_offset;
        
        return(samp_num);
}
        

TERN_m10        search_segment_metadata_m10(si1 *MED_dir, TIME_SLICE_m10 *slice)
{
        ui1                                     search_mode;
        si1                                     **seg_list, seg_name[BASE_FILE_NAME_BYTES_m10], tmp_str[FULL_FILE_NAME_BYTES_m10];
        si4                                     n_segs, start_seg_idx, end_seg_idx;
        si8                                     i, items_read, start_seg_start_idx, end_seg_start_idx, absolute_end_sample_number;
        sf8                                     sampling_frequency;
        FILE_PROCESSING_STRUCT_m10              *md_fps, *tsi_fps;
        UNIVERSAL_HEADER_m10                    *uh;
        TIME_SERIES_METADATA_SECTION_2_m10      *tmd2;

        
        if (slice == NULL)
                error_message_m10("%s(): NULL slice pointer", __FUNCTION__);
        else
                slice->start_segment_number = slice->end_segment_number = SEGMENT_NUMBER_NO_ENTRY_m10;

        // check for valid limit pair (time takes priority)
        if (slice->start_time != UUTC_NO_ENTRY_m10 && slice->end_time != UUTC_NO_ENTRY_m10) {
                search_mode = TIME_SEARCH_m10;
        } else if (slice->start_index != SAMPLE_NUMBER_NO_ENTRY_m10 && slice->end_index != SAMPLE_NUMBER_NO_ENTRY_m10) {
                search_mode = INDEX_SEARCH_m10;
        } else {
                error_message_m10("%s(): no valid limit pair", __FUNCTION__);
                return(FALSE_m10);
        }

        // get segment list
        seg_list = generate_file_list_m10(NULL, 0, &n_segs, MED_dir, "*", "tisd", PP_FULL_PATH_m10, FALSE_m10);
        if (n_segs == 0) {
                error_message_m10("%s(): Cannot find segment metadata", __FUNCTION__);
                return(FALSE_m10);
        }

        // find start segment
        slice->start_segment_number = 0;
        for (i = 0; i < n_segs; ++i) {
                extract_path_parts_m10(seg_list[i], NULL, seg_name, NULL);
                sprintf(tmp_str, "%s/%s.%s", seg_list[i], seg_name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
                md_fps = NULL;
                if (file_exists_m10(tmp_str) == FILE_EXISTS_m10)
                        md_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
                if (md_fps == NULL) {
                        e_free_2D_m10((void **) seg_list, n_segs, __FUNCTION__, __LINE__);
                        error_message_m10("%s(): Cannot find segment metadata", __FUNCTION__);
                        return(FALSE_m10);
                }
                tmd2 = md_fps->metadata.time_series_section_2;
                uh = md_fps->universal_header;

                if (search_mode == TIME_SEARCH_m10) {
                        if (slice->start_time <= uh->file_end_time) {
                                slice->start_segment_number = uh->segment_number;
                                if (slice->start_time < uh->file_start_time)
                                        slice->start_time = uh->file_start_time;
                                break;
                        }
                } else {  // search_mode == INDEX_SEARCH_m10
                        absolute_end_sample_number = tmd2->absolute_start_sample_number + tmd2->number_of_samples - 1;
                        if (slice->start_index <= absolute_end_sample_number) {
                                slice->start_segment_number = uh->segment_number;
                                if (slice->start_index < tmd2->absolute_start_sample_number)
                                        slice->start_index = tmd2->absolute_start_sample_number;
                                break;
                        }
                }
                free_file_processing_struct_m10(md_fps, FALSE_m10);
        }
        if (i == n_segs) {
                e_free_2D_m10((void **) seg_list, n_segs, __FUNCTION__, __LINE__);
                error_message_m10("%s(): Start index exceeds session indices", __FUNCTION__);
                return(FALSE_m10);
        }
        start_seg_idx = i;
        start_seg_start_idx = tmd2->absolute_start_sample_number;
        free_file_processing_struct_m10(md_fps, FALSE_m10);

        // find end segment
        slice->end_segment_number = slice->start_segment_number;
        for (; i < n_segs; ++i) {
                extract_path_parts_m10(seg_list[i], NULL, seg_name, NULL);
                sprintf(tmp_str, "%s/%s.%s", seg_list[i], seg_name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
                md_fps = NULL;
                if (file_exists_m10(tmp_str) == FILE_EXISTS_m10)
                        md_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
                if (md_fps == NULL) {
                        e_free_2D_m10((void **) seg_list, n_segs, __FUNCTION__, __LINE__);
                        error_message_m10("%s(): Cannot find segment metadata", __FUNCTION__);
                        return(FALSE_m10);
                }

                tmd2 = md_fps->metadata.time_series_section_2;
		end_seg_start_idx = tmd2->absolute_start_sample_number;
		sampling_frequency = tmd2->sampling_frequency;
                uh = md_fps->universal_header;
                if (search_mode == TIME_SEARCH_m10) {
                        if (slice->end_time <= uh->file_end_time) {
                                slice->end_segment_number = uh->segment_number;
                                break;
                        }
                } else {  // search_mode == INDEX_SEARCH_m10
                        absolute_end_sample_number = tmd2->absolute_start_sample_number + tmd2->number_of_samples - 1;
                        if (slice->end_index <= absolute_end_sample_number) {
                                slice->end_segment_number = uh->segment_number;
                                break;
                        }
                }
                free_file_processing_struct_m10(md_fps, FALSE_m10);
        }
        if (i == n_segs) {
                slice->end_segment_number = uh->segment_number;
                slice->end_index = absolute_end_sample_number;
                slice->end_time = uh->file_end_time;
                end_seg_idx = i - 1;
		md_fps = NULL;
        } else {
                end_seg_idx = i;
		free_file_processing_struct_m10(md_fps, FALSE_m10);
        }
        
        // ********************************************** //
        // ***********  fill in other limits  *********** //
        // ********************************************** //
        
        // fill in slice session start & end times
        extract_path_parts_m10(seg_list[0], NULL, seg_name, NULL);
        sprintf(tmp_str, "%s/%s.%s", seg_list[0], seg_name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
        md_fps = read_file_m10(NULL, tmp_str, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
        slice->session_start_time = md_fps->universal_header->session_start_time;
        if (n_segs > 1) {
                free_file_processing_struct_m10(md_fps, FALSE_m10);
                extract_path_parts_m10(seg_list[n_segs - 1], NULL, seg_name, NULL);
                sprintf(tmp_str, "%s/%s.%s", seg_list[n_segs - 1], seg_name, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
                md_fps = read_file_m10(NULL, tmp_str, FPS_UNIVERSAL_HEADER_ONLY_m10, NULL, &items_read, NULL, USE_GLOBAL_BEHAVIOR_m10);
        }
        slice->session_end_time = md_fps->universal_header->file_end_time;
	free_file_processing_struct_m10(md_fps, FALSE_m10);
	
        // get start segment limits
        extract_path_parts_m10(seg_list[start_seg_idx], NULL, seg_name, NULL);
        sprintf(tmp_str, "%s/%s.%s", seg_list[start_seg_idx], seg_name, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10);
        tsi_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
	if (search_mode == TIME_SEARCH_m10)
                slice->start_index = sample_number_for_uutc_m10(start_seg_start_idx, UUTC_NO_ENTRY_m10, slice->start_time, sampling_frequency, tsi_fps, FIND_CURRENT_m10);
	else  // search_mode == INDEX_SEARCH_m10
                slice->start_time = uutc_for_sample_number_m10(start_seg_start_idx, UUTC_NO_ENTRY_m10, slice->start_index, sampling_frequency, tsi_fps, FIND_START_m10);
	
        // get end segment limits
        if (start_seg_idx != end_seg_idx) {
                free_file_processing_struct_m10(tsi_fps, FALSE_m10);
                extract_path_parts_m10(seg_list[end_seg_idx], NULL, seg_name, NULL);
                sprintf(tmp_str, "%s/%s.%s", seg_list[end_seg_idx], seg_name, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10);
                tsi_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
        }
        if (search_mode == TIME_SEARCH_m10)
                slice->end_index = sample_number_for_uutc_m10(end_seg_start_idx, UUTC_NO_ENTRY_m10, slice->end_time, sampling_frequency, tsi_fps, FIND_CURRENT_m10);
        else  // search_mode == INDEX_SEARCH_m10
                slice->end_time = uutc_for_sample_number_m10(end_seg_start_idx, UUTC_NO_ENTRY_m10, slice->end_index, sampling_frequency, tsi_fps, FIND_END_m10);
        free_file_processing_struct_m10(tsi_fps, FALSE_m10);
        e_free_2D_m10((void **) seg_list, n_segs, __FUNCTION__, __LINE__);

	slice->number_of_samples = (slice->end_index - slice->start_index) + 1;

        return(TRUE_m10);
}


TERN_m10        search_Sgmt_records_m10(si1 *MED_dir, TIME_SLICE_m10 *slice)
{
        ui1                             search_mode;
        si1                             path[FULL_FILE_NAME_BYTES_m10], name[BASE_FILE_NAME_BYTES_m10], tmp_str[FULL_FILE_NAME_BYTES_m10];
        si1                             *passed_channel, **chan_list, num_str[FILE_NUMBERING_DIGITS_m10 + 1];
        si4                             n_chans;
        ui4                             type_code;
        si8                             i, n_recs, items_read, start_seg_start_idx, end_seg_start_idx;
        sf8                             sampling_frequency;
        FILE_PROCESSING_STRUCT_m10      *ri_fps, *rd_fps, *md_fps, *tsi_fps;
        UNIVERSAL_HEADER_m10            *uh;
        RECORD_INDEX_m10                *ri;
        REC_Sgmt_v10_m10                *Sgmt;
        

        if (slice == NULL)
                error_message_m10("%s(): NULL slice pointer", __FUNCTION__);
        else
                slice->start_segment_number = slice->end_segment_number = SEGMENT_NUMBER_NO_ENTRY_m10;

	// check for valid limit pair (time takes priority)
        if (slice->start_time != UUTC_NO_ENTRY_m10 && slice->end_time != UUTC_NO_ENTRY_m10) {
                search_mode = TIME_SEARCH_m10;
        } else if (slice->start_index != SAMPLE_NUMBER_NO_ENTRY_m10 && slice->end_index != SAMPLE_NUMBER_NO_ENTRY_m10) {
                search_mode = INDEX_SEARCH_m10;
        } else {
                error_message_m10("%s(): no valid limit pair", __FUNCTION__);
                return(FALSE_m10);
        }
	
        ri_fps = rd_fps = NULL;
        passed_channel = NULL;

        // open record indices file
        SEARCH_SEG_TRY_SESSION_m10:
        type_code = generate_MED_path_components_m10(MED_dir, path, name);
        if (type_code != TIME_SERIES_CHANNEL_TYPE_m10 && type_code != SESSION_DIRECTORY_TYPE_CODE_m10)
                return(FALSE_m10);
        sprintf(tmp_str, "%s/%s.%s", path, name, RECORD_INDICES_FILE_TYPE_STRING_m10);
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
        sprintf(tmp_str, "%s/%s.%s", path, name, RECORD_DATA_FILE_TYPE_STRING_m10);
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
        } else { // search by index
                for (; i < n_recs; ++i) {
                        if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10) {
                                Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[i].file_offset + RECORD_HEADER_BYTES_m10);
                                if (slice->start_index <= Sgmt->absolute_end_sample_number) {
                                        slice->start_segment_number = Sgmt->segment_number;
                                        if (slice->start_index < Sgmt->absolute_start_sample_number)
                                                slice->start_index = Sgmt->absolute_start_sample_number;
                                        break;
                                }
                        }
                }
        }
        if (i == n_recs) {
                free_file_processing_struct_m10(ri_fps, FALSE_m10);
                free_file_processing_struct_m10(rd_fps, FALSE_m10);
                error_message_m10("%s(): Start time/index exceeds session limits", __FUNCTION__);
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
        } else {  // search by index
                for (; i < n_recs; ++i) {
                        if (ri[i].type_code == REC_Sgmt_TYPE_CODE_m10) {
                                Sgmt = (REC_Sgmt_v10_m10 *) (rd_fps->raw_data + ri[i].file_offset + RECORD_HEADER_BYTES_m10);
                                if (Sgmt->absolute_end_sample_number >= slice->end_index) {
                                        slice->end_segment_number = Sgmt->segment_number;
                                        break;
                                }
                        }
                }
        }
        if (i == n_recs) {
                slice->end_segment_number = Sgmt->segment_number;
                slice->end_time = Sgmt->end_time;
                slice->end_index = Sgmt->absolute_end_sample_number;
        }
        end_seg_start_idx = Sgmt->absolute_start_sample_number;
        

        // ********************************************** //
        // ***********  fill in other limits  *********** //
        // ********************************************** //

        if (type_code == SESSION_DIRECTORY_TYPE_CODE_m10) {
                chan_list = NULL;
                if (passed_channel == NULL) {
                        chan_list = generate_file_list_m10(NULL, 0, &n_chans, path, "*", "ticd", PP_FULL_PATH_m10, FALSE_m10);
                        if (n_chans == 0) {
                                warning_message_m10("%s(): Cannot fill in all limits");
                                return(TRUE_m10);
                        }
                        passed_channel = chan_list[0];
                }
                generate_MED_path_components_m10(passed_channel, path, name);
                if (chan_list != NULL)
                        e_free_2D_m10((void **) chan_list, n_chans, __FUNCTION__, __LINE__);
        }
        
        numerical_fixed_width_string_m10(num_str, FILE_NUMBERING_DIGITS_m10, slice->start_segment_number);
        if (sampling_frequency == FREQUENCY_VARIABLE_m10 || sampling_frequency == FREQUENCY_NO_ENTRY_m10) {
                sprintf(tmp_str, "%s/%s_s%s.%s/%s_s%s.%s", path, name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m10, name, num_str, TIME_SERIES_METADATA_FILE_TYPE_STRING_m10);
                md_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
                sampling_frequency = md_fps->metadata.time_series_section_2->sampling_frequency;
                free_file_processing_struct_m10(md_fps, FALSE_m10);
        }
        
        // get start segment limits
        sprintf(tmp_str, "%s/%s_s%s.%s/%s_s%s.%s", path, name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m10, name, num_str, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10);
        tsi_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
	if (search_mode == TIME_SEARCH_m10)
                slice->start_index = sample_number_for_uutc_m10(start_seg_start_idx, UUTC_NO_ENTRY_m10, slice->start_time, sampling_frequency, tsi_fps, FIND_CURRENT_m10);
	else  // search_mode == INDEX_SEARCH_m10
                slice->start_time = uutc_for_sample_number_m10(start_seg_start_idx, UUTC_NO_ENTRY_m10, slice->start_index, sampling_frequency, tsi_fps, FIND_START_m10);

        // get end segment limits
        if (slice->end_segment_number != slice->start_segment_number) {
                free_file_processing_struct_m10(tsi_fps, FALSE_m10);
                numerical_fixed_width_string_m10(num_str, FILE_NUMBERING_DIGITS_m10, slice->end_segment_number);
                sprintf(tmp_str, "%s/%s_s%s.%s/%s_s%s.%s", path, name, num_str, TIME_SERIES_SEGMENT_DIRECTORY_TYPE_STRING_m10, name, num_str, TIME_SERIES_INDICES_FILE_TYPE_STRING_m10);
                tsi_fps = read_file_m10(NULL, tmp_str, FPS_FULL_FILE_m10, NULL, NULL, NULL, USE_GLOBAL_BEHAVIOR_m10);
        }
        if (search_mode == TIME_SEARCH_m10)
                slice->end_index = sample_number_for_uutc_m10(end_seg_start_idx, UUTC_NO_ENTRY_m10, slice->end_time, sampling_frequency, tsi_fps, FIND_CURRENT_m10);
        else  // search_mode == INDEX_SEARCH_m10
                slice->end_time = uutc_for_sample_number_m10(end_seg_start_idx, UUTC_NO_ENTRY_m10, slice->end_index, sampling_frequency, tsi_fps, FIND_END_m10);
        free_file_processing_struct_m10(tsi_fps, FALSE_m10);
	
	slice->number_of_samples = (slice->end_index - slice->start_index) + 1;

        return(TRUE_m10);
}


void    set_global_time_constants_m10(TIMEZONE_INFO_m10 *timezone_info, si8 session_start_time)
{
        si4                     n_potential_timezones, potential_timezone_entries[TIMEZONE_TABLE_ENTRIES_m10];
        si4                     i, j, entry_num, response_num, items;
        TIMEZONE_INFO_m10       *tz_table;
        
        
        n_potential_timezones = TIMEZONE_TABLE_ENTRIES_m10;
        tz_table = globals_m10->timezone_table;
        for (i = 0; i < n_potential_timezones; ++i)
                potential_timezone_entries[i] = i;

        // match country
        j = 0;
        if (*timezone_info->country) {
                for (i = 0; i < n_potential_timezones; ++i)
                        if ((strcmp(timezone_info->country, tz_table[potential_timezone_entries[i]].country)) == 0)
                                potential_timezone_entries[j++] = potential_timezone_entries[i];
        }
        if (j) {
                if (j == 1)
                        goto TIME_ZONE_MATCH_m10;
                n_potential_timezones = j;
        }

        // match country_acronym_2_letter
        j = 0;
        if (*timezone_info->country_acronym_2_letter) {
                for (i = 0; i < n_potential_timezones; ++i)
                        if ((strcmp(timezone_info->country_acronym_2_letter, tz_table[potential_timezone_entries[i]].country_acronym_2_letter)) == 0)
                                potential_timezone_entries[j++] = potential_timezone_entries[i];
        }
        if (j) {
                if (j == 1)
                        goto TIME_ZONE_MATCH_m10;
                n_potential_timezones = j;
        }

        // match country_acronym_3_letter
        j = 0;
        if (*timezone_info->country_acronym_3_letter) {
                for (i = 0; i < n_potential_timezones; ++i)
                        if ((strcmp(timezone_info->country_acronym_3_letter, tz_table[potential_timezone_entries[i]].country_acronym_3_letter)) == 0)
                                potential_timezone_entries[j++] = potential_timezone_entries[i];
        }
        if (j) {
                if (j == 1)
                        goto TIME_ZONE_MATCH_m10;
                n_potential_timezones = j;
        }

        // match territory
        j = 0;
        if (*timezone_info->territory) {
                for (i = 0; i < n_potential_timezones; ++i)
                        if ((strcmp(timezone_info->territory, tz_table[potential_timezone_entries[i]].territory)) == 0)
                                potential_timezone_entries[j++] = potential_timezone_entries[i];
        }
        if (j) {
                if (j == 1)
                        goto TIME_ZONE_MATCH_m10;
                n_potential_timezones = j;
        }

        // match territory_acronym
        j = 0;
        if (*timezone_info->territory_acronym) {
                for (i = 0; i < n_potential_timezones; ++i)
                        if ((strcmp(timezone_info->territory_acronym, tz_table[potential_timezone_entries[i]].territory_acronym)) == 0)
                                potential_timezone_entries[j++] = potential_timezone_entries[i];
        }
        if (j) {
                if (j == 1)
                        goto TIME_ZONE_MATCH_m10;
                n_potential_timezones = j;
        }

        // match standard_timezone
        j = 0;
        if (*timezone_info->standard_timezone) {
                for (i = 0; i < n_potential_timezones; ++i)
                        if ((strcmp(timezone_info->standard_timezone, tz_table[potential_timezone_entries[i]].standard_timezone)) == 0)
                                potential_timezone_entries[j++] = potential_timezone_entries[i];
        }
        if (j) {
                if (j == 1)
                        goto TIME_ZONE_MATCH_m10;
                n_potential_timezones = j;
        }

        j = 0;
        // match standard_timezone_acronym
        if (*timezone_info->standard_timezone_acronym) {
                for (i = 0; i < n_potential_timezones; ++i)
                        if ((strcmp(timezone_info->standard_timezone_acronym, tz_table[potential_timezone_entries[i]].standard_timezone_acronym)) == 0)
                                potential_timezone_entries[j++] = potential_timezone_entries[i];
        }
        if (j) {
                if (j == 1)
                        goto TIME_ZONE_MATCH_m10;
                n_potential_timezones = j;
        }

        // match standard_UTC_offset
        j = 0;
        if (timezone_info->standard_UTC_offset != STANDARD_UTC_OFFSET_NO_ENTRY_m10) {
                for (i = 0; i < n_potential_timezones; ++i)
                        if (timezone_info->standard_UTC_offset == tz_table[potential_timezone_entries[i]].standard_UTC_offset)
                                potential_timezone_entries[j++] = potential_timezone_entries[i];
        }
        if (j) {
                if (j == 1)
                        goto TIME_ZONE_MATCH_m10;
                n_potential_timezones = j;
        }

        // match daylight_timezone
        j = 0;
        if (*timezone_info->daylight_timezone) {
                for (i = 0; i < n_potential_timezones; ++i)
                        if ((strcmp(timezone_info->daylight_timezone, tz_table[potential_timezone_entries[i]].daylight_timezone)) == 0)
                                potential_timezone_entries[j++] = potential_timezone_entries[i];
        }
        if (j) {
                if (j == 1)
                        goto TIME_ZONE_MATCH_m10;
                n_potential_timezones = j;
        }

        fprintf(stderr, "Multiple potential timezone entries:\n\n");
        for (i = 0; i < n_potential_timezones; ++i) {
                fprintf(stderr, "%d)\n", i + 1);
                show_timezone_info_m10(&tz_table[potential_timezone_entries[i]]);
                fprintf(stderr, "\n");
        }
        fprintf(stderr, "Select one (by number): ");
        items = scanf("%d", &response_num);
        if (items != 1 || response_num < 1 || response_num > n_potential_timezones) {
                fprintf(stderr, "Invalid Choice => exiting\n");
                exit(1);
        }
        --response_num;
        potential_timezone_entries[0] = potential_timezone_entries[response_num];

TIME_ZONE_MATCH_m10:
        
        entry_num = potential_timezone_entries[0];
        memcpy((void *) timezone_info, (void *) (tz_table + entry_num), sizeof(TIMEZONE_INFO_m10));
        globals_m10->standard_UTC_offset = tz_table[entry_num].standard_UTC_offset;
        strncpy_m10(globals_m10->standard_timezone_acronym, timezone_info->standard_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m10);
        strncpy_m10(globals_m10->standard_timezone_string, timezone_info->standard_timezone, TIMEZONE_STRING_BYTES_m10);
        if (timezone_info->observe_DST) {
                globals_m10->observe_DST = TRUE_m10;
                strncpy_m10(globals_m10->daylight_timezone_acronym, timezone_info->daylight_timezone_acronym, TIMEZONE_ACRONYM_BYTES_m10);
                strncpy_m10(globals_m10->daylight_timezone_string, timezone_info->daylight_timezone, TIMEZONE_STRING_BYTES_m10);
                globals_m10->daylight_time_start_code = (DAYLIGHT_TIME_CHANGE_CODE_m10) timezone_info->daylight_time_start_code;
                globals_m10->daylight_time_end_code = (DAYLIGHT_TIME_CHANGE_CODE_m10) timezone_info->daylight_time_end_code;
        } else {
                globals_m10->observe_DST = FALSE_m10;
        }
	
	if (session_start_time)  // pass CURRENT_TIME_m10 for session starting now; pass zero if just need to get timezone_info for a locale
		generate_recording_time_offset_m10(session_start_time);
	
        return;
}


TERN_m10	set_time_and_password_data_m10(si1 *unspecified_password, si1 *MED_directory, si1 *section_2_encryption_level, si1 *section_3_encryption_level)
{
	si1                             command[2048], MED_dir_copy[FULL_FILE_NAME_BYTES_m10], metadata_file[FULL_FILE_NAME_BYTES_m10];
	ui4                             code;
	si4                             ret_val;
	si8                             items_read;
	FILE                            *fp;
	FILE_PROCESSING_STRUCT_m10      *metadata_fps;
	METADATA_SECTION_1_m10		*md1;

	
	// copy directory name to escape spaces for shell
	if (*MED_directory != '/') {
		getcwd(MED_dir_copy, FULL_FILE_NAME_BYTES_m10);
		sprintf(MED_dir_copy, "%s/%s", MED_dir_copy, MED_directory);
	} else {
		strcpy(MED_dir_copy, MED_directory);
	}
	escape_spaces_m10(MED_dir_copy, FULL_FILE_NAME_BYTES_m10);
	
	// find a MED metadata file (using *met)
	code = MED_type_code_from_string_m10(MED_directory);
	switch (code) {
		case SESSION_DIRECTORY_TYPE_CODE_m10:
			sprintf(command, "ls -1d %s/*/*/*met | head -n1 > /tmp/junk 2> /dev/null", MED_dir_copy);
			break;
		case TIME_SERIES_CHANNEL_DIRECTORY_TYPE_CODE_m10:
		case VIDEO_CHANNEL_DIRECTORY_TYPE_CODE_m10:
			sprintf(command, "ls -1d %s/*/*met | head -n1 > /tmp/junk 2> /dev/null", MED_dir_copy);
			break;
		case TIME_SERIES_SEGMENT_DIRECTORY_TYPE_CODE_m10:
		case VIDEO_SEGMENT_DIRECTORY_TYPE_CODE_m10:
			sprintf(command, "ls -1d %s/*met | head -n1 > /tmp/junk 2> /dev/null", MED_dir_copy);
			break;
		default:
			error_message_m10("%s(): \"%s\" is not a MED directory", __FUNCTION__, MED_directory);
			return(FALSE_m10);
	}
	if (file_exists_m10("/tmp/junk") == FILE_EXISTS_m10)
		e_system_m10("rm -f /tmp/junk", FALSE_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	ret_val = e_system_m10(command, FALSE_m10, __FUNCTION__, __LINE__,  USE_GLOBAL_BEHAVIOR_m10);
	if (ret_val)
		return(FALSE_m10);
	fp = e_fopen_m10("/tmp/junk", "r", __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	items_read = (si8) fscanf(fp, "%[^\n]", metadata_file);
	fclose(fp);
	e_system_m10("rm -f /tmp/junk", TRUE_m10, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	if (items_read == 0) {
		error_message_m10("%s(): \"%s\" does not contain any metadata files", __FUNCTION__, MED_directory);
		return(FALSE_m10);
	}

	// read in metadata file (will process password, decrypt metadata and set global time constants)
	globals_m10->password_data.processed = 0;  // not ternary FALSE_m10 (so when structure is zeroed and it is marked as not processed)
	metadata_fps = read_file_m10(NULL, metadata_file, 1, NULL, &items_read, unspecified_password, RETURN_ON_FAIL_m10);
	if (metadata_fps == NULL)
		return(FALSE_m10);
	globals_m10->session_start_time = metadata_fps->universal_header->session_start_time;

	// return metadata encryption level info
	md1 = metadata_fps->metadata.section_1;
	if (section_2_encryption_level != NULL)
		*section_2_encryption_level = md1->section_2_encryption_level;
	if (section_3_encryption_level != NULL)
		*section_3_encryption_level = md1->section_3_encryption_level;

	// clean up
	free_file_processing_struct_m10(metadata_fps, FALSE_m10);

	return(TRUE_m10);
}


//***********************************************************************//
//**************************  SHA-256 FUNCTIONS  ************************//
//***********************************************************************//

// ATTRIBUTION
//
// FIPS 180-2 SHA-224/256/384/512 implementation
// Last update: 02/02/2007
// Issue date:  04/30/2005
//
// Copyright (C) 2005, 2007 Olivier Gay <olivier.gay@a3.epfl.ch>
// All rights reserved.
//
// "Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE."
//
// ONLY SHA-256 FUNCTIONS ARE INCLUDED IN THE MED LIBRARY
//
// Minor modifications for compatibility with the MED Library.


void    SHA_sha_m10(const ui1 *message, ui4 len, ui1 *digest)
{
	SHA_CTX_m10         ctx;
	
	
	SHA_init_m10(&ctx);
	SHA_update_m10(&ctx, message, len);
	SHA_final_m10(&ctx, digest);
	
	return;
}


void    SHA_final_m10(SHA_CTX_m10 *ctx, ui1 *digest)
{
	ui4	block_nb;
	ui4	pm_len;
	ui4	len_b;
	
	
	block_nb = (1 + ((SHA_BLOCK_SIZE_m10 - 9) < (ctx->len % SHA_BLOCK_SIZE_m10)));
	
	len_b = (ctx->tot_len + ctx->len) << 3;
	pm_len = block_nb << 6;
	
	memset(ctx->block + ctx->len, 0, pm_len - ctx->len);
	ctx->block[ctx->len] = 0x80;
	SHA_UNPACK32_m10(len_b, ctx->block + pm_len - 4);
	
	SHA_transf_m10(ctx, ctx->block, block_nb);
	
	SHA_UNPACK32_m10(ctx->h[0], &digest[ 0]);
	SHA_UNPACK32_m10(ctx->h[1], &digest[ 4]);
	SHA_UNPACK32_m10(ctx->h[2], &digest[ 8]);
	SHA_UNPACK32_m10(ctx->h[3], &digest[12]);
	SHA_UNPACK32_m10(ctx->h[4], &digest[16]);
	SHA_UNPACK32_m10(ctx->h[5], &digest[20]);
	SHA_UNPACK32_m10(ctx->h[6], &digest[24]);
	SHA_UNPACK32_m10(ctx->h[7], &digest[28]);
	
	return;
}


void    SHA_init_m10(SHA_CTX_m10 *ctx)
{
	if (globals_m10->SHA_h0_table == NULL)
		(void) SHA_initialize_h0_table_m10();
	
	ctx->h[0] = globals_m10->SHA_h0_table[0]; ctx->h[1] = globals_m10->SHA_h0_table[1];
	ctx->h[2] = globals_m10->SHA_h0_table[2]; ctx->h[3] = globals_m10->SHA_h0_table[3];
	ctx->h[4] = globals_m10->SHA_h0_table[4]; ctx->h[5] = globals_m10->SHA_h0_table[5];
	ctx->h[6] = globals_m10->SHA_h0_table[6]; ctx->h[7] = globals_m10->SHA_h0_table[7];
	
	ctx->len = 0;
	ctx->tot_len = 0;
	
	return;
}


void	SHA_initialize_h0_table_m10(void)
{
	ui4	*sha_h0_table;
	
	
	sha_h0_table = (ui4 *) e_calloc_m10((size_t) SHA_H0_ENTRIES_m10, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	{
		ui4 temp[SHA_H0_ENTRIES_m10] = SHA_H0_m10;
		memcpy(sha_h0_table, temp, SHA_H0_ENTRIES_m10 * sizeof(ui4));
	}
	
	globals_m10->SHA_h0_table = sha_h0_table;
	
	return;
}


void	SHA_initialize_k_table_m10(void)
{
	ui4	*sha_k_table;
	
	
	sha_k_table = (ui4 *) e_calloc_m10((size_t) SHA_K_ENTRIES_m10, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	{
		ui4 temp[SHA_K_ENTRIES_m10] = SHA_K_m10;
		memcpy(sha_k_table, temp, SHA_K_ENTRIES_m10 * sizeof(ui4));
	}
	
	globals_m10->SHA_k_table = sha_k_table;
	
	return;
}


void    SHA_transf_m10(SHA_CTX_m10 *ctx, const ui1 *message, ui4 block_nb)
{
	ui4		w[64];
	ui4		wv[8];
	ui4		t1, t2;
	const ui1	*sub_block;
	si4		i;
        
	
	if (globals_m10->SHA_k_table == NULL)
		SHA_initialize_k_table_m10();
	
	for (i = 0; i < (si4) block_nb; i++) {
		sub_block = message + (i << 6);
		
		SHA_PACK32_m10( &sub_block[ 0], &w[ 0]); SHA_PACK32_m10( &sub_block[ 4], &w[ 1]);
		SHA_PACK32_m10( &sub_block[ 8], &w[ 2]); SHA_PACK32_m10( &sub_block[12], &w[ 3]);
		SHA_PACK32_m10( &sub_block[16], &w[ 4]); SHA_PACK32_m10( &sub_block[20], &w[ 5]);
		SHA_PACK32_m10( &sub_block[24], &w[ 6]); SHA_PACK32_m10( &sub_block[28], &w[ 7]);
		SHA_PACK32_m10( &sub_block[32], &w[ 8]); SHA_PACK32_m10( &sub_block[36], &w[ 9]);
		SHA_PACK32_m10( &sub_block[40], &w[10]); SHA_PACK32_m10( &sub_block[44], &w[11]);
		SHA_PACK32_m10( &sub_block[48], &w[12]); SHA_PACK32_m10( &sub_block[52], &w[13]);
		SHA_PACK32_m10( &sub_block[56], &w[14]); SHA_PACK32_m10( &sub_block[60], &w[15]);
		
		SHA_SCR_m10( 16); SHA_SCR_m10( 17); SHA_SCR_m10( 18); SHA_SCR_m10( 19);
		SHA_SCR_m10( 20); SHA_SCR_m10( 21); SHA_SCR_m10( 22); SHA_SCR_m10( 23);
		SHA_SCR_m10( 24); SHA_SCR_m10( 25); SHA_SCR_m10( 26); SHA_SCR_m10( 27);
		SHA_SCR_m10( 28); SHA_SCR_m10( 29); SHA_SCR_m10( 30); SHA_SCR_m10( 31);
		SHA_SCR_m10( 32); SHA_SCR_m10( 33); SHA_SCR_m10( 34); SHA_SCR_m10( 35);
		SHA_SCR_m10( 36); SHA_SCR_m10( 37); SHA_SCR_m10( 38); SHA_SCR_m10( 39);
		SHA_SCR_m10( 40); SHA_SCR_m10( 41); SHA_SCR_m10( 42); SHA_SCR_m10( 43);
		SHA_SCR_m10( 44); SHA_SCR_m10( 45); SHA_SCR_m10( 46); SHA_SCR_m10( 47);
		SHA_SCR_m10( 48); SHA_SCR_m10( 49); SHA_SCR_m10( 50); SHA_SCR_m10( 51);
		SHA_SCR_m10( 52); SHA_SCR_m10( 53); SHA_SCR_m10( 54); SHA_SCR_m10( 55);
		SHA_SCR_m10( 56); SHA_SCR_m10( 57); SHA_SCR_m10( 58); SHA_SCR_m10( 59);
		SHA_SCR_m10( 60); SHA_SCR_m10( 61); SHA_SCR_m10( 62); SHA_SCR_m10( 63);
		
		wv[0] = ctx->h[0]; wv[1] = ctx->h[1];
		wv[2] = ctx->h[2]; wv[3] = ctx->h[3];
		wv[4] = ctx->h[4]; wv[5] = ctx->h[5];
		wv[6] = ctx->h[6]; wv[7] = ctx->h[7];
		
		SHA_EXP_m10( 0,1,2,3,4,5,6,7, 0); SHA_EXP_m10( 7,0,1,2,3,4,5,6, 1);
		SHA_EXP_m10( 6,7,0,1,2,3,4,5, 2); SHA_EXP_m10( 5,6,7,0,1,2,3,4, 3);
		SHA_EXP_m10( 4,5,6,7,0,1,2,3, 4); SHA_EXP_m10( 3,4,5,6,7,0,1,2, 5);
		SHA_EXP_m10( 2,3,4,5,6,7,0,1, 6); SHA_EXP_m10( 1,2,3,4,5,6,7,0, 7);
		SHA_EXP_m10( 0,1,2,3,4,5,6,7, 8); SHA_EXP_m10( 7,0,1,2,3,4,5,6, 9);
		SHA_EXP_m10( 6,7,0,1,2,3,4,5,10); SHA_EXP_m10( 5,6,7,0,1,2,3,4,11);
		SHA_EXP_m10( 4,5,6,7,0,1,2,3,12); SHA_EXP_m10( 3,4,5,6,7,0,1,2,13);
		SHA_EXP_m10( 2,3,4,5,6,7,0,1,14); SHA_EXP_m10( 1,2,3,4,5,6,7,0,15);
		SHA_EXP_m10( 0,1,2,3,4,5,6,7,16); SHA_EXP_m10( 7,0,1,2,3,4,5,6,17);
		SHA_EXP_m10( 6,7,0,1,2,3,4,5,18); SHA_EXP_m10( 5,6,7,0,1,2,3,4,19);
		SHA_EXP_m10( 4,5,6,7,0,1,2,3,20); SHA_EXP_m10( 3,4,5,6,7,0,1,2,21);
		SHA_EXP_m10( 2,3,4,5,6,7,0,1,22); SHA_EXP_m10( 1,2,3,4,5,6,7,0,23);
		SHA_EXP_m10( 0,1,2,3,4,5,6,7,24); SHA_EXP_m10( 7,0,1,2,3,4,5,6,25);
		SHA_EXP_m10( 6,7,0,1,2,3,4,5,26); SHA_EXP_m10( 5,6,7,0,1,2,3,4,27);
		SHA_EXP_m10( 4,5,6,7,0,1,2,3,28); SHA_EXP_m10( 3,4,5,6,7,0,1,2,29);
		SHA_EXP_m10( 2,3,4,5,6,7,0,1,30); SHA_EXP_m10( 1,2,3,4,5,6,7,0,31);
		SHA_EXP_m10( 0,1,2,3,4,5,6,7,32); SHA_EXP_m10( 7,0,1,2,3,4,5,6,33);
		SHA_EXP_m10( 6,7,0,1,2,3,4,5,34); SHA_EXP_m10( 5,6,7,0,1,2,3,4,35);
		SHA_EXP_m10( 4,5,6,7,0,1,2,3,36); SHA_EXP_m10( 3,4,5,6,7,0,1,2,37);
		SHA_EXP_m10( 2,3,4,5,6,7,0,1,38); SHA_EXP_m10( 1,2,3,4,5,6,7,0,39);
		SHA_EXP_m10( 0,1,2,3,4,5,6,7,40); SHA_EXP_m10( 7,0,1,2,3,4,5,6,41);
		SHA_EXP_m10( 6,7,0,1,2,3,4,5,42); SHA_EXP_m10( 5,6,7,0,1,2,3,4,43);
		SHA_EXP_m10( 4,5,6,7,0,1,2,3,44); SHA_EXP_m10( 3,4,5,6,7,0,1,2,45);
		SHA_EXP_m10( 2,3,4,5,6,7,0,1,46); SHA_EXP_m10( 1,2,3,4,5,6,7,0,47);
		SHA_EXP_m10( 0,1,2,3,4,5,6,7,48); SHA_EXP_m10( 7,0,1,2,3,4,5,6,49);
		SHA_EXP_m10( 6,7,0,1,2,3,4,5,50); SHA_EXP_m10( 5,6,7,0,1,2,3,4,51);
		SHA_EXP_m10( 4,5,6,7,0,1,2,3,52); SHA_EXP_m10( 3,4,5,6,7,0,1,2,53);
		SHA_EXP_m10( 2,3,4,5,6,7,0,1,54); SHA_EXP_m10( 1,2,3,4,5,6,7,0,55);
		SHA_EXP_m10( 0,1,2,3,4,5,6,7,56); SHA_EXP_m10( 7,0,1,2,3,4,5,6,57);
		SHA_EXP_m10( 6,7,0,1,2,3,4,5,58); SHA_EXP_m10( 5,6,7,0,1,2,3,4,59);
		SHA_EXP_m10( 4,5,6,7,0,1,2,3,60); SHA_EXP_m10( 3,4,5,6,7,0,1,2,61);
		SHA_EXP_m10( 2,3,4,5,6,7,0,1,62); SHA_EXP_m10( 1,2,3,4,5,6,7,0,63);
		
		ctx->h[0] += wv[0]; ctx->h[1] += wv[1];
		ctx->h[2] += wv[2]; ctx->h[3] += wv[3];
		ctx->h[4] += wv[4]; ctx->h[5] += wv[5];
		ctx->h[6] += wv[6]; ctx->h[7] += wv[7];
	}
	
	return;
}


void    SHA_update_m10(SHA_CTX_m10 *ctx, const ui1 *message, unsigned int len)
{
	ui4		block_nb;
	ui4		new_len, rem_len, tmp_len;
	const ui1	*shifted_message;
	
	
	tmp_len = SHA_BLOCK_SIZE_m10 - ctx->len;
	rem_len = len < tmp_len ? len : tmp_len;
	
	memcpy(&ctx->block[ctx->len], message, rem_len);
	
	if (ctx->len + len < SHA_BLOCK_SIZE_m10) {
		ctx->len += len;
		return;
	}
	
	new_len = len - rem_len;
	block_nb = new_len / SHA_BLOCK_SIZE_m10;
	
	shifted_message = message + rem_len;
	
	SHA_transf_m10(ctx, ctx->block, 1);
	SHA_transf_m10(ctx, shifted_message, block_nb);
	
	rem_len = new_len % SHA_BLOCK_SIZE_m10;
	
	memcpy(ctx->block, &shifted_message[block_nb << 6], rem_len);
	
	ctx->len = rem_len;
	ctx->tot_len += (block_nb + 1) << 6;
        
	return;
}


//***********************************************************************//
//************************  END SHA-256 FUNCTIONS  **********************//
//***********************************************************************//


void    show_daylight_change_code_m10(DAYLIGHT_TIME_CHANGE_CODE_m10 *code, si1 *prefix)
{
        if (prefix == NULL)
                prefix = "";
        printf("%sCode Type (DST end / DST Not Observed / DST start) ==  (-1 / 0 / +1): %hhd\n", prefix, code->code_type);
        printf("%sDay of Week (No Entry / [Sunday : Saturday]) ==  (0 / [1 : 7]): %hhd\n", prefix, code->day_of_week);
        printf("%sRelative Weekday of Month (No Entry / [First : Fifth] / Last) ==  (0 / [1 : 5] / 6): %hhd\n", prefix, code->relative_weekday_of_month);
        printf("%sDay of Month (No Entry / [1 : 31]) ==  (0 / [1 : 31]): %hhd\n", prefix, code->day_of_month);
        printf("%sMonth (No Entry / [January : December]) ==  (-1 / [0 : 11]): %hhd\n", prefix, code->month);
        printf("%sHours of Day [-128 : +127] hours relative to 00:00 (midnight): %hhd\n", prefix, code->hours_of_day);
        printf("%sReference Time (Local / UTC) ==  (0 / 1): %hhd\n", prefix, code->reference_time);
        printf("%sShift Minutes [-120 : +120] minutes: %hhd\n", prefix, code->shift_minutes);
        printf("%sValue: 0x%lx\n\n", prefix, code->value);
               
        return;
}


void	show_file_processing_struct_m10(FILE_PROCESSING_STRUCT_m10 *fps)
{
        si1	hex_str[HEX_STRING_BYTES_m10(TYPE_STRLEN_m10)], *s;
	si4	i;
        
        
	printf("----------- File Processing Structure - START ----------\n");
	UTF8_printf_m10("Full File Name: %s\n", fps->full_file_name);
	if (fps->fd >= 3)
		printf("File Descriptor: %d (open)\n", fps->fd);
        else if  (fps->fd == -1)
                printf("File Descriptor: %d (closed)\n", fps->fd);
        else if  (fps->fd == FPS_FD_NO_ENTRY_m10)
                printf("File Descriptor: %d (not yet opened)\n", fps->fd);
        else if  (fps->fd == FPS_FD_EPHEMERAL_m10)
                printf("File Descriptor: %d (ephemeral)\n", fps->fd);
        else    // stdin == 0, stdout == 1, stderr == 2
                printf("File Descriptor: %d (standard stream: invalid)\n", fps->fd);
	printf("File Length: ");
	if (fps->file_length == FPS_FILE_LENGTH_UNKNOWN_m10)
		printf("unknown\n");
	else
		printf("%ld\n", fps->file_length);
	s = (si1 *) &fps->universal_header->type_code;
	generate_hex_string_m10((ui1 *) s, TYPE_STRLEN_m10, hex_str);
        printf("File Type Code: %s    (", hex_str);
	for (i = 0; i < 4; ++i)
		printf(" %c ", *s++);
	printf(")\n");
	printf("Raw Data Bytes: %ld\n", fps->raw_data_bytes);
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
	printf("------------ File Processing Structure - END -----------\n\n");
        
	return;
}


void    show_globals_m10(void)
{
        si1     hex_str[HEX_STRING_BYTES_m10(8)];
        
	
        printf("MED Globals\n");
        printf("-----------\n\n");
        printf("Time Constants\n");
        printf("--------------\n");
        printf("recording_time_offset: %ld\n", globals_m10->recording_time_offset);
        printf("standard_UTC_offset: %d\n", globals_m10->standard_UTC_offset);
        printf("standard_timezone_acronym: %s\n", globals_m10->standard_timezone_acronym);
        printf("standard_timezone_string: %s\n", globals_m10->standard_timezone_string);
        printf("observe_DST: %hhd\n", globals_m10->observe_DST);
        printf("daylight_timezone_acronym: %s\n", globals_m10->daylight_timezone_acronym);
        printf("daylight_timezone_string: %s\n", globals_m10->daylight_timezone_string);
        generate_hex_string_m10((ui1 *) &globals_m10->daylight_time_start_code.value, 8, hex_str);
        printf("daylight_time_start_code: %s\n", hex_str);
        generate_hex_string_m10((ui1 *) &globals_m10->daylight_time_end_code.value, 8, hex_str);
        printf("daylight_time_end_code: %s\n", hex_str);
        printf("Alignment Fields\n");
        printf("----------------\n");
        printf("universal_header_aligned: %hhd\n", globals_m10->universal_header_aligned);
        printf("metadata_section_1_aligned: %hhd\n", globals_m10->metadata_section_1_aligned);
        printf("time_series_metadata_section_2_aligned: %hhd\n", globals_m10->time_series_metadata_section_2_aligned);
        printf("video_metadata_section_2_aligned: %hhd\n", globals_m10->video_metadata_section_2_aligned);
        printf("metadata_section_3_aligned: %hhd\n", globals_m10->metadata_section_3_aligned);
        printf("all_metadata_structures_aligned: %hhd\n", globals_m10->all_metadata_structures_aligned);
        printf("time_series_indices_aligned: %hhd\n", globals_m10->time_series_indices_aligned);
        printf("video_indices_aligned: %hhd\n", globals_m10->video_indices_aligned);
        printf("CMP_block_header_aligned: %hhd\n", globals_m10->CMP_block_header_aligned);
        printf("record_header_aligned: %hhd\n", globals_m10->record_header_aligned);
        printf("record_indices_aligned: %hhd\n", globals_m10->record_indices_aligned);
        printf("all_record_structures_aligned: %hhd\n", globals_m10->all_record_structures_aligned);
        printf("all_structures_aligned: %hhd\n\n", globals_m10->all_structures_aligned);
        printf("Miscellaneous\n");
        printf("-------------\n");
        printf("CRC_mode: %u\n", globals_m10->CRC_mode);
        printf("verbose: %hhd\n", globals_m10->verbose);
        printf("behavior_on_fail: %u\n", globals_m10->behavior_on_fail);

        return;
}


void	show_metadata_m10(FILE_PROCESSING_STRUCT_m10 *fps, METADATA_m10 *md)
{
        si1                                     hex_str[HEX_STRING_BYTES_m10(8)];
	METADATA_SECTION_1_m10		        *md1;
	TIME_SERIES_METADATA_SECTION_2_m10	*tmd2, *gmd2;
	VIDEO_METADATA_SECTION_2_m10	        *vmd2;
	METADATA_SECTION_3_m10		        *md3;
	

	// assign
        if (fps != NULL) {
                md1 = fps->metadata.section_1;
                tmd2 = fps->metadata.time_series_section_2;
                vmd2 = fps->metadata.video_section_2;
                md3 = fps->metadata.section_3;
        } else if (md != NULL) {
                md1 = md->section_1;
                tmd2 = md->time_series_section_2;
                vmd2 = md->video_section_2;
                md3 = md->section_3;
        } else {
                error_message_m10("%s(): invalid input", __FUNCTION__);
                return;
        }
	
	// decrypt if needed
        if (md1->section_2_encryption_level > NO_ENCRYPTION_m10 || md1->section_3_encryption_level > NO_ENCRYPTION_m10)
		if (fps != NULL)
                        decrypt_metadata_m10(fps);

	// show
	printf("------------------- Metadata - START -------------------\n");
	printf("------------------ Section 1 - START -------------------\n");
	if (*md1->level_1_password_hint)
		UTF8_printf_m10("Level 1 Password Hint: %s\n", md1->level_1_password_hint);
	if (*md1->level_2_password_hint)
		UTF8_printf_m10("Level 2 Password Hint: %s\n", md1->level_2_password_hint);
	printf("Section 2 Encryption Level: %d ", md1->section_2_encryption_level);
	if (md1->section_2_encryption_level == NO_ENCRYPTION_m10)
		printf("(none)\n");
	else if (md1->section_2_encryption_level == LEVEL_1_ENCRYPTION_m10)
		printf("(level 1, currently encrypted)\n");
	else if (md1->section_2_encryption_level == LEVEL_2_ENCRYPTION_m10)
		printf("(level 2, currently encrypted)\n");
	else if (md1->section_2_encryption_level == -LEVEL_1_ENCRYPTION_m10)
		printf("(level 1, currently decrypted)\n");
	else if (md1->section_2_encryption_level == -LEVEL_2_ENCRYPTION_m10)
		printf("(level 2, currently decrypted)\n");
	else
                printf("(unrecognized code)\n");
	printf("Section 3 Encryption Level: %d ", md1->section_3_encryption_level);
	if (md1->section_3_encryption_level == NO_ENCRYPTION_m10)
		printf("(none)\n");
	else if (md1->section_3_encryption_level == LEVEL_1_ENCRYPTION_m10)
		printf("(level 1, currently encrypted)\n");
	else if (md1->section_3_encryption_level == LEVEL_2_ENCRYPTION_m10)
		printf("(level 2, currently encrypted)\n");
	else if (md1->section_3_encryption_level == -LEVEL_1_ENCRYPTION_m10)
		printf("(level 1, currently decrypted)\n");
	else if (md1->section_3_encryption_level == -LEVEL_2_ENCRYPTION_m10)
		printf("(level 2, currently decrypted)\n");
	else
		printf("(unrecognized code)\n");
	printf("------------------- Section 1 - END --------------------\n");
	printf("------------------ Section 2 - START -------------------\n");
	if (md1->section_2_encryption_level <= NO_ENCRYPTION_m10) {
                
                // type-independent fields
                if (tmd2 != NULL)
                        gmd2 = tmd2;
                else if (vmd2 != NULL)
                        gmd2 = (TIME_SERIES_METADATA_SECTION_2_m10 *) vmd2;
                else {
                        error_message_m10("%s(): unrecognized metadata type input", __FUNCTION__);
                        return;
                }
                if (*gmd2->session_description)
                        UTF8_printf_m10("Session Description: %s\n", gmd2->session_description);
                else
                        printf("Session Description: no entry\n");
                if (*gmd2->channel_description)
                        UTF8_printf_m10("Channel Description: %s\n", gmd2->channel_description);
                else
                        printf("Channel Description: no entry\n");
                if (*gmd2->segment_description)
                        UTF8_printf_m10("Segment Description: %s\n", gmd2->segment_description);
                else
                        printf("Segment Description: no entry\n");
                if (*gmd2->equipment_description)
                        UTF8_printf_m10("Equipment Description: %s\n", gmd2->equipment_description);
                else
                        printf("Equipment Description: no entry\n");
                if (gmd2->acquisition_channel_number == METADATA_ACQUISITION_CHANNEL_NUMBER_NO_ENTRY_m10)
                        printf("Acquisition Channel Number: no entry\n");
                else
                        printf("Acquisition Channel Number: %d\n", gmd2->acquisition_channel_number);

                // type-specific fields
		if (tmd2 != NULL) {
			if (*tmd2->reference_description)
				UTF8_printf_m10("Reference Description: %s\n", tmd2->reference_description);
			else
				printf("Reference Description: no entry\n");
			if (tmd2->sampling_frequency == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10)
				printf("Sampling Frequency: no entry\n");
                        else if (tmd2->sampling_frequency == TIME_SERIES_METADATA_FREQUENCY_VARIABLE_m10)
                                printf("Sampling Frequency: variable\n");
			else
				printf("Sampling Frequency: %lf\n", tmd2->sampling_frequency);
                        if (tmd2->low_frequency_filter_setting == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10)
                                printf("Low Frequency Filter Setting: no entry\n");
                        else
				printf("Low Frequency Filter Setting (Hz): %lf\n", tmd2->low_frequency_filter_setting);
                        if (tmd2->high_frequency_filter_setting == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10)
                                printf("High Frequency Filter Setting: no entry\n");
                        else
				printf("High Frequency Filter Setting (Hz): %lf\n", tmd2->high_frequency_filter_setting);
                        if (tmd2->notch_filter_frequency_setting == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10)
                                printf("Notch Filter Frequency Setting: no entry\n");
                        else
				printf("Notch Filter Frequency Setting (Hz): %lf\n", tmd2->notch_filter_frequency_setting);
			if (tmd2->AC_line_frequency == TIME_SERIES_METADATA_FREQUENCY_NO_ENTRY_m10)
				printf("AC Line Frequency: no entry\n");
			else
				printf("AC Line Frequency (Hz): %lf\n", tmd2->AC_line_frequency);
			if (tmd2->amplitude_units_conversion_factor == TIME_SERIES_METADATA_AMPLITUDE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10)
                                printf("Amplitiude Units Conversion Factor: no entry\n");
                        else
				printf("Amplitude Units Conversion Factor: %lf\n", tmd2->amplitude_units_conversion_factor);
                        if (*tmd2->amplitude_units_description)
				UTF8_printf_m10("Amplitude Units Description: %s\n", tmd2->amplitude_units_description);
                        else
                                printf("Amplitude Units Description: no entry\n");
                        if (tmd2->time_base_units_conversion_factor == TIME_SERIES_METADATA_TIME_BASE_UNITS_CONVERSION_FACTOR_NO_ENTRY_m10)
                                printf("Time Base Units Conversion Factor: no entry\n");
                        else
                                printf("Time Base Units Conversion Factor: %lf\n", tmd2->time_base_units_conversion_factor);
                        if (*tmd2->time_base_units_description)
                                UTF8_printf_m10("Time Base Units Description: %s\n", tmd2->time_base_units_description);
                        else
                                printf("Time Base Units Description: no entry\n");
                        if (tmd2->absolute_start_sample_number == TIME_SERIES_METADATA_ABSOLUTE_START_SAMPLE_NUMBER_NO_ENTRY_m10)
                                printf("Absolute Start Sample Number: no entry\n");
                        else
                                printf("Absolute Start Sample Number: %ld\n", tmd2->absolute_start_sample_number);
                        if (tmd2->number_of_samples == TIME_SERIES_METADATA_NUMBER_OF_SAMPLES_NO_ENTRY_m10)
				printf("Number of Samples: no entry\n");
			else
				printf("Number of Samples: %ld\n", tmd2->number_of_samples);
			if (tmd2->number_of_blocks == TIME_SERIES_METADATA_NUMBER_OF_BLOCKS_NO_ENTRY_m10)
				printf("Number of Blocks: no entry\n");
			else
				printf("Number of Blocks: %ld\n", tmd2->number_of_blocks);
			if (tmd2->maximum_block_bytes == TIME_SERIES_METADATA_MAXIMUM_BLOCK_BYTES_NO_ENTRY_m10)
				printf("Maximum Block Bytes: no entry\n");
			else
				printf("Maximum Block Bytes: %ld\n", tmd2->maximum_block_bytes);
			if (tmd2->maximum_block_samples == TIME_SERIES_METADATA_MAXIMUM_BLOCK_SAMPLES_NO_ENTRY_m10)
				printf("Maximum Block Samples: no entry\n");
			else
				printf("Maximum Block Samples: %u\n", tmd2->maximum_block_samples);
                        if (tmd2->maximum_block_difference_bytes == TIME_SERIES_METADATA_MAXIMUM_BLOCK_DIFFERENCE_BYTES_NO_ENTRY_m10)
                                printf("Maximum Block Difference Bytes: no entry\n");
                        else
				printf("Maximum Block Difference Bytes: %u\n", tmd2->maximum_block_difference_bytes);
                        if (tmd2->maximum_block_duration == TIME_SERIES_METADATA_MAXIMUM_BLOCK_DURATION_NO_ENTRY_m10)
                                printf("Maximum Block Duration: no entry\n");
                        else
				UTF8_printf_m10("Maximum Block Duration: %lf %s\n", tmd2->maximum_block_duration, tmd2->time_base_units_description);
			if (tmd2->number_of_discontinuities == TIME_SERIES_METADATA_NUMBER_OF_DISCONTINUITIES_NO_ENTRY_m10)
				printf("Number of Discontinuities: no entry\n");
			else
				printf("Number of Discontinuities: %ld\n", tmd2->number_of_discontinuities);
			if (tmd2->maximum_contiguous_blocks == TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCKS_NO_ENTRY_m10)
				printf("Maximum Contiguous Blocks: no entry\n");
			else
				printf("Maximum Contiguous Blocks: %ld\n", tmd2->maximum_contiguous_blocks);
			if (tmd2->maximum_contiguous_block_bytes == TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_BLOCK_BYTES_NO_ENTRY_m10)
				printf("Maximum Contiguous Block Bytes: no entry\n");
			else
				printf("Maximum Contiguous Block Bytes: %ld\n", tmd2->maximum_contiguous_block_bytes);
			if (tmd2->maximum_contiguous_samples == TIME_SERIES_METADATA_MAXIMUM_CONTIGUOUS_SAMPLES_NO_ENTRY_m10)
                                printf("Maximum Contiguous Samples: no entry\n");
                        else
				printf("Maximum Contiguous Samples: %ld\n", tmd2->maximum_contiguous_samples);
		}
		else if (vmd2 != NULL) {
			if (vmd2->horizontal_resolution == VIDEO_METADATA_HORIZONTAL_RESOLUTION_NO_ENTRY_m10)
				printf("Horizontal Resolution: no entry\n");
			else
				printf("Horizontal Resolution: %ld\n", vmd2->horizontal_resolution);
			if (vmd2->vertical_resolution == VIDEO_METADATA_VERTICAL_RESOLUTION_NO_ENTRY_m10)
				printf("Vertical Resolution: no entry\n");
			else
				printf("Vertical Resolution: %ld\n", vmd2->vertical_resolution);
                        if (vmd2->frame_rate == VIDEO_METADATA_FRAME_RATE_NO_ENTRY_m10)
				printf("Frame Rate: no entry\n");
			else
				printf("Frame Rate: %lf (frames per second)\n", vmd2->frame_rate);
			if (vmd2->number_of_clips == VIDEO_METADATA_NUMBER_OF_CLIPS_NO_ENTRY_m10)
				printf("Number of Clips: no entry\n");
			else
				printf("Number of Clips: %ld (= number of video indices)\n", vmd2->number_of_clips);
			if (vmd2->maximum_clip_bytes == VIDEO_METADATA_MAXIMUM_CLIP_BYTES_NO_ENTRY_m10)
				printf("Maximum Clip Bytes: no entry\n");
			else
				printf("Maximum Clip Bytes: %ld\n", vmd2->maximum_clip_bytes);
			if (*vmd2->video_format)
				UTF8_printf_m10("Video Format: %s\n", vmd2->video_format);
			else
				printf("Video Format: no entry\n");
                        if (vmd2->number_of_video_files == VIDEO_METADATA_NUMBER_OF_VIDEO_FILES_NO_ENTRY_m10)
                                printf("Number of Video Files: no entry\n");
                        else
                                printf("Number of Video Files: %d\n", vmd2->number_of_video_files);
		}
		else {
			printf("(unrecognized metadata section 2 type)\n");
		}
	} else {
		printf("No access to section 2\n");
	}
	printf("------------------- Section 2 - END --------------------\n");
	printf("------------------ Section 3 - START -------------------\n");
	if (md1->section_3_encryption_level <= NO_ENCRYPTION_m10) {
		if (md3->recording_time_offset == UUTC_NO_ENTRY_m10)
			printf("Recording Time Offset: no entry\n");
		else
			printf("Recording Time Offset: %ld\n", md3->recording_time_offset);
                if (md3->daylight_time_start_code.value == DTCC_VALUE_NO_ENTRY_m10) {
                        printf("Daylight Time Start Code: no entry\n");
                } else {
                        generate_hex_string_m10((ui1 *) &md3->daylight_time_start_code.value, 8, hex_str);
                        printf("Daylight Time Start Code: %s\n", hex_str);
                }
                if (md3->daylight_time_end_code.value == DTCC_VALUE_NO_ENTRY_m10) {
                        printf("Daylight Time End Code: no entry\n");
                } else {
                        generate_hex_string_m10((ui1 *) &md3->daylight_time_end_code.value, 8, hex_str);
                        printf("Daylight Time End Code: %s\n", hex_str);
                }
                if (*md3->standard_timezone_acronym)
                        printf("Standard Timezone Acronym: %s\n", md3->standard_timezone_acronym);
                else
                        printf("Standard Timezone Acronym: no entry\n");
                if (*md3->standard_timezone_string)
                        printf("Standard Timezone String: %s\n", md3->standard_timezone_string);
                else
                        printf("Standard Timezone String: no entry\n");
                if (*md3->daylight_timezone_acronym)
                        printf("Daylight Timezone Acronym: %s\n", md3->daylight_timezone_acronym);
                else
                        printf("Daylight Timezone Acronym: no entry\n");
                if (*md3->daylight_timezone_string)
                        printf("Daylight Timezone String: %s\n", md3->daylight_timezone_string);
                else
                        printf("Daylight Timezone String: no entry\n");
                if (*md3->subject_name_1)
                        UTF8_printf_m10("Subject Name 1: %s\n", md3->subject_name_1);
                else
                        printf("Subject Name 1: no entry\n");
		if (*md3->subject_name_2)
			UTF8_printf_m10("Subject Name 2: %s\n", md3->subject_name_2);
		else
			printf("Subject Name 2: no entry\n");
		if (*md3->subject_name_3)
			UTF8_printf_m10("Subject Name 3: %s\n", md3->subject_name_3);
		else
			printf("Subject Name 3: no entry\n");
		if (*md3->subject_ID)
			UTF8_printf_m10("Subject ID: %s\n", md3->subject_ID);
		else
			printf("Subject ID: no entry\n");
		if (*md3->recording_country)
			UTF8_printf_m10("Recording Country: %s\n", md3->recording_country);
		else
			printf("Recording Country: no entry\n");
                if (*md3->recording_territory)
                        UTF8_printf_m10("Recording Territory: %s\n", md3->recording_territory);
                else
                        printf("Recording Territory: no entry\n");
                if (*md3->recording_city)
                        UTF8_printf_m10("Recording City: %s\n", md3->recording_city);
                else
                        printf("Recording City: no entry\n");
                if (*md3->recording_institution)
                        UTF8_printf_m10("Recording Institution: %s\n", md3->recording_institution);
                else
                        printf("Recording Institution: no entry\n");
                if (*md3->geotag_format)
                        UTF8_printf_m10("GeoTag Format: %s\n", md3->geotag_format);
                else
                        printf("GeoTag Format: no entry\n");
                if (*md3->geotag_data)
                        UTF8_printf_m10("GeoTag Data: %s\n", md3->geotag_data);
                else
                        printf("GeoTag Data: no entry\n");
                if (md3->standard_UTC_offset == STANDARD_UTC_OFFSET_NO_ENTRY_m10)
                        printf("Standard UTC Offset: no entry\n");
                else
                        printf("Standard UTC Offset: %d\n", md3->standard_UTC_offset);
	} else {
		printf("No access to section 3\n");
	}
	printf("------------------- Section 3 - END --------------------\n");
	printf("-------------------- Metadata - END --------------------\n\n");
	
	return;
}


void	show_password_data_m10(PASSWORD_DATA_m10 *pwd)
{
        si1     		hex_str[HEX_STRING_BYTES_m10(ENCRYPTION_KEY_BYTES_m10)];
	
		
	// use message_m10() because show_password_data_m10() is used in normal (no programming) functions => so allow output to be suppressed easily
	if (pwd == NULL) {
		message_m10("\n-------------- Global Password Data - START --------------");
		pwd = &globals_m10->password_data;
	} else {
		message_m10("\n------------------ Password Data - START -----------------");
	}
	if (pwd->access_level >= LEVEL_1_ACCESS_m10) {
                generate_hex_string_m10(pwd->level_1_encryption_key, ENCRYPTION_KEY_BYTES_m10, hex_str);
		message_m10("Level 1 Encryption Key: %s", hex_str);
	}
	if (pwd->access_level == LEVEL_2_ACCESS_m10) {
                generate_hex_string_m10(pwd->level_2_encryption_key, ENCRYPTION_KEY_BYTES_m10, hex_str);
		message_m10("Level 2 Encryption Key: %s", hex_str);
	}
	if (*pwd->level_1_password_hint)
		message_m10("Level 1 Password Hint: %s", pwd->level_1_password_hint);
	if (*pwd->level_2_password_hint)
		message_m10("Level 2 Password Hint: %s", pwd->level_2_password_hint);
	message_m10("Access Level: %hhu", pwd->access_level);
	message_m10("Processed: %hhu", pwd->access_level);
	message_m10("------------------- Password Data - END ------------------\n");
	
	return;
}


void	show_records_m10(FILE_PROCESSING_STRUCT_m10 *fps, ui4 type_code)
{
	si8		        number_of_records;
        ui4		        i, r_cnt;
        ui1		        *ui1_p, *end_p;
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
	} else {
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
	printf("Conditioned: ");
	if (slice->conditioned == TRUE_m10)
		printf("true\n");
	else if (slice->conditioned == FALSE_m10)
		printf("false\n");
	else if (slice->conditioned == UNKNOWN_m10)
		printf("unknown\n");
	else
		printf("invalid value (%hhd)\n", slice->conditioned);

	printf("Start Time: ");
	if (slice->start_time == UUTC_NO_ENTRY_m10)
		printf("no entry\n");
	else if (slice->start_time == BEGINNING_OF_TIME_m10)
		printf("beginning of time\n");
	else if (slice->start_time == END_OF_TIME_m10)
		printf("end of time\n");
	else
		printf("%ld\n", slice->start_time);

	printf("End Time: ");
	if (slice->end_time == UUTC_NO_ENTRY_m10)
		printf("no entry\n");
	else if (slice->end_time == BEGINNING_OF_TIME_m10)
		printf("beginning of time\n");
	else if (slice->end_time == END_OF_TIME_m10)
		printf("end of time\n");
	else
		printf("%ld\n", slice->end_time);

	printf("Start Index: ");
	if (slice->start_index == SAMPLE_NUMBER_NO_ENTRY_m10)
		printf("no entry\n");
	else if (slice->start_index == END_OF_INDICES_m10)
		printf("end of indices\n");
	else
		printf("%ld\n", slice->start_index);

	printf("End Index: ");
	if (slice->end_index == SAMPLE_NUMBER_NO_ENTRY_m10)
		printf("no entry\n");
	else if (slice->end_index == END_OF_INDICES_m10)
		printf("end of indices\n");
	else
		printf("%ld\n", slice->end_index);

	printf("Local Start Index: ");
	if (slice->local_start_index == SAMPLE_NUMBER_NO_ENTRY_m10)
		printf("no entry\n");
	else if (slice->local_start_index == END_OF_INDICES_m10)
		printf("end of segment\n");
	else
		printf("%ld\n", slice->start_index);

	printf("Local End Index: ");
	if (slice->local_end_index == SAMPLE_NUMBER_NO_ENTRY_m10)
		printf("no entry\n");
	else if (slice->local_end_index == END_OF_INDICES_m10)
		printf("end of segment\n");
	else
		printf("%ld\n", slice->end_index);

	if (slice->number_of_samples == NUMBER_OF_SAMPLES_NO_ENTRY_m10)
		printf("Number of Samples: no entry\n");
	else
		printf("Number of Samples: %ld\n", slice->number_of_samples);

	printf("Start Segment Number: ");
	if (slice->start_segment_number == SEGMENT_NUMBER_NO_ENTRY_m10)
		printf("no entry\n");
	else
		printf("%d\n", slice->start_segment_number);

	printf("End Segment Number: ");
	if (slice->end_segment_number == SEGMENT_NUMBER_NO_ENTRY_m10)
		printf("no entry\n");
	else
		printf("%d\n", slice->end_segment_number);

	printf("Session Start Time: ");
	if (slice->session_start_time == UUTC_NO_ENTRY_m10)
		printf("no entry\n");
	else
		printf("%ld\n", slice->session_start_time);

	printf("Session End Time: ");
	if (slice->session_end_time == UUTC_NO_ENTRY_m10)
		printf("no entry\n");
	else
		printf("%ld\n", slice->session_end_time);

	printf("Index Reference Channel Name: ");
	if (slice->index_reference_channel_name == NULL)
		printf("no entry\n");
	else if (*slice->index_reference_channel_name == 0)
		printf("no entry\n");
	else
		printf("%s\n", slice->index_reference_channel_name);

	printf("Index Reference Channel Index: %d\n", slice->index_reference_channel_index);

	printf("\n");

	return;
}


void    show_timezone_info_m10(TIMEZONE_INFO_m10 *timezone_entry)
{
        printf("Country: %s\n", timezone_entry->country);
        printf("Country Acronym (2 letter): %s\n", timezone_entry->country_acronym_2_letter);
        printf("Country Acronym (3 letter): %s\n", timezone_entry->country_acronym_3_letter);
        if (*timezone_entry->territory)
                printf("Territory: %s\n", timezone_entry->territory);
        if (*timezone_entry->territory_acronym)
                printf("Territory Acronym: %s\n", timezone_entry->territory_acronym);
        printf("Standard Timezone: %s\n", timezone_entry->standard_timezone);
        printf("Standard Timezone Acronym: %s\n", timezone_entry->standard_timezone_acronym);
        printf("Standard UTC Offset (secs): %d\n", timezone_entry->standard_UTC_offset);
        printf("Observe DST (0/1 == no/yes): %d\n", timezone_entry->observe_DST);
        if (timezone_entry->observe_DST) {
                printf("Daylight Timezone: %s\n", timezone_entry->daylight_timezone);
                printf("Daylight Timezone Acronym: %s\n", timezone_entry->daylight_timezone_acronym);
                printf("Daylight Time Start Description: %s\n", timezone_entry->daylight_time_start_description);
                printf("Daylight Time Start Code: 0x%lx\n", timezone_entry->daylight_time_start_code);
                printf("Daylight Time End Description: %s\n", timezone_entry->daylight_time_end_description);
                printf("Daylight Time End Code: 0x%lx\n", timezone_entry->daylight_time_end_code);
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
        } else {
                if (uh == NULL) {
                        error_message_m10("%s(): invalid input", __FUNCTION__);
                        return;
                }
                ephemeral_flag = UNKNOWN_m10;
        }
	
	printf("---------------- Universal Header - START ----------------\n");
	if (uh->header_CRC == CRC_NO_ENTRY_m10)
		printf("Header CRC: no entry\n");
	else {
                generate_hex_string_m10((ui1 *) &uh->header_CRC, CRC_BYTES_m10, hex_str);
                printf("Header CRC: %s\n", hex_str);
        }
        if (uh->body_CRC == CRC_NO_ENTRY_m10)
                printf("Body CRC: no entry\n");
        else {
                generate_hex_string_m10((ui1 *) &uh->body_CRC, CRC_BYTES_m10, hex_str);
                printf("Body CRC: %s\n", hex_str);
        }
	if (uh->file_end_time == UUTC_NO_ENTRY_m10)
		printf("File End Time: no entry\n");
	else {
		time_string_m10(uh->file_end_time, time_str, TRUE_m10, FALSE_m10, FALSE_m10);
		printf("File End Time: %ld (oUTC), %s\n", uh->file_end_time, time_str);
	}
	if (uh->number_of_entries == UNIVERSAL_HEADER_NUMBER_OF_ENTRIES_NO_ENTRY_m10)
		printf("Number of Entries: no entry\n");
	else {
		printf("Number of Entries: %ld  ", uh->number_of_entries);
		switch (uh->type_code) {
			case RECORD_DATA_FILE_TYPE_CODE_m10:
				printf("(number of records in the file)\n");
				break;
			case RECORD_INDICES_FILE_TYPE_CODE_m10:
				printf("(number of record indices in the file)\n");
				break;
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
			case VIDEO_METADATA_FILE_TYPE_CODE_m10:
				if (ephemeral_flag == TRUE_m10)
					printf("(maximum number of records in records files at this level and below)\n");
				else if (ephemeral_flag == FALSE_m10)
					printf("(one metadata entry per metadata file)\n");
				else // UNKNOWN
				      printf("(one metadata entry, or maximum number of records in a records file at this level and below)\n");
				break;
			case VIDEO_INDICES_FILE_TYPE_CODE_m10:
				printf("(number of video indices in the file)\n");
				break;
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
				printf("(number of CMP blocks in the file)\n");
				break;
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
				printf("(number of time series indices in the file)\n");
				break;
			default:
				printf("\n");
				break;
		}
	}
	if (uh->maximum_entry_size == UNIVERSAL_HEADER_MAXIMUM_ENTRY_SIZE_NO_ENTRY_m10)
		printf("Maximum Entry Size: no entry\n");
	else {
		printf("Maximum Entry Size: %u  ", uh->maximum_entry_size);
		switch (uh->type_code) {
			case RECORD_DATA_FILE_TYPE_CODE_m10:
				printf("(number of bytes in the largest record in the file)\n");
				break;
			case RECORD_INDICES_FILE_TYPE_CODE_m10:
				printf("(number of bytes in a record index)\n");
				break;
			case TIME_SERIES_METADATA_FILE_TYPE_CODE_m10:
			case VIDEO_METADATA_FILE_TYPE_CODE_m10:
				if (ephemeral_flag == TRUE_m10)
					printf("(maximum number of bytes in a record at this level and below)\n");
				else if (ephemeral_flag == FALSE_m10)
					printf("(number of bytes in a metadata structure)\n");
				else // UNKNOWN
				      printf("(metadata bytes, or maximum number of bytes in a record at this level and below)\n");
				break;
			case VIDEO_INDICES_FILE_TYPE_CODE_m10:
				printf("(number of bytes in a video index)\n");
				break;
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
				printf("(number of bytes in the largest CMP block in the file)\n");
				break;
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
				printf("(number of bytes in a time series index)\n");
				break;
			default:
				printf("\n");
				break;
				
		}
	}
	if (uh->segment_number == UNIVERSAL_HEADER_SEGMENT_NUMBER_NO_ENTRY_m10)
		printf("Segment Number: no entry\n");
	else if (uh->segment_number == UNIVERSAL_HEADER_CHANNEL_LEVEL_CODE_m10)
		printf("Segment Number: channel level\n");
	else if (uh->segment_number == UNIVERSAL_HEADER_SESSION_LEVEL_CODE_m10)
		printf("Segment Number: session level\n");
	else
		printf("Segment Number: %d\n", uh->segment_number);
	if (*uh->type_string)
		printf("File Type String: %s\n", uh->type_string);
	else
		printf("File Type String: no entry\n");
	if (uh->MED_version_major == UNIVERSAL_HEADER_MED_VERSION_MAJOR_NO_ENTRY_m10 || uh->MED_version_minor == UNIVERSAL_HEADER_MED_VERSION_MINOR_NO_ENTRY_m10) {
		if (uh->MED_version_major == UNIVERSAL_HEADER_MED_VERSION_MAJOR_NO_ENTRY_m10)
			printf("MED Version Major: no entry\n");
		else
			printf("MED Version Major: %u\n", uh->MED_version_major);
		if (uh->MED_version_minor == UNIVERSAL_HEADER_MED_VERSION_MINOR_NO_ENTRY_m10)
			printf("MED Version Minor: no entry\n");
		else
			printf("MED Version Minor: %u\n", uh->MED_version_minor);
	} else
		printf("MED Version: %u.%u\n", uh->MED_version_major, uh->MED_version_minor);
	if (uh->byte_order_code == UNIVERSAL_HEADER_BYTE_ORDER_CODE_NO_ENTRY_m10)
		printf("Byte Order Code: no entry ");
	else {
		printf("Byte Order Code: %u ", uh->byte_order_code);
		if (uh->byte_order_code == LITTLE_ENDIAN_m10)
			printf("(little endian)\n");
		else if (uh->byte_order_code == BIG_ENDIAN_m10)
			printf("(big endian)\n");
		else
			printf("(unrecognized code)\n");
	}
	if (uh->session_start_time == UUTC_NO_ENTRY_m10)
		printf("Session Start Time: no entry\n");
	else {
		time_string_m10(uh->session_start_time, time_str, TRUE_m10, FALSE_m10, FALSE_m10);
		printf("Session Start Time: %ld (oUTC), %s\n", uh->session_start_time, time_str);
	}
	if (uh->file_start_time == UUTC_NO_ENTRY_m10)
		printf("File Start Time: no entry\n");
	else {
		time_string_m10(uh->file_start_time, time_str, TRUE_m10, FALSE_m10, FALSE_m10);
		printf("File Start Time: %ld (oUTC), %s\n", uh->file_start_time, time_str);
	}
        if (*uh->session_name)
                UTF8_printf_m10("Session Name: %s\n", uh->session_name);
        else
                printf("Session Name: no entry\n");
	if (*uh->channel_name)
		UTF8_printf_m10("Channel Name: %s\n", uh->channel_name);
	else
		printf("Channel Name: no entry\n");
	if (*uh->anonymized_subject_ID)
		UTF8_printf_m10("Anonymized Subject ID: %s\n", uh->anonymized_subject_ID);
	else
		printf("Anonymized Subject ID: no entry\n");
	if (uh->session_UID == UID_NO_ENTRY_m10)
		printf("Session UID: no entry\n");
	else {
		generate_hex_string_m10((ui1 *) &uh->session_UID, UID_BYTES_m10, hex_str);
		printf("Session UID: %s\n", hex_str);
	}
        if (uh->channel_UID == UID_NO_ENTRY_m10)
                printf("Channel UID: no entry\n");
        else {
                generate_hex_string_m10((ui1 *) &uh->channel_UID, UID_BYTES_m10, hex_str);
                printf("Channel UID: %s\n", hex_str);
        }
        if (uh->segment_UID == UID_NO_ENTRY_m10)
                printf("Segment UID: no entry\n");
        else {
                generate_hex_string_m10((ui1 *) &uh->segment_UID, UID_BYTES_m10, hex_str);
                printf("Segment UID: %s\n", hex_str);
        }
        if (uh->file_UID == UID_NO_ENTRY_m10)
		printf("File UID: no entry\n");
	else {
		generate_hex_string_m10((ui1 *) &uh->file_UID, UID_BYTES_m10, hex_str);
		printf("File UID: %s\n", hex_str);
	}
	if (uh->provenance_UID == UID_NO_ENTRY_m10)
		printf("Provenance UID: no entry\n");
	else {
		generate_hex_string_m10((ui1 *) &uh->provenance_UID, UID_BYTES_m10, hex_str);
		printf("Provenance UID: %s  ", hex_str);
		if (uh->provenance_UID == uh->file_UID)
                        printf("(original data)\n");
		else
                        printf("(derived data)\n");
	}
	if (all_zeros_m10(uh->level_1_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10) == TRUE_m10)
		printf("Level 1 Password Validation_Field: no entry\n");
	else {
                generate_hex_string_m10(uh->level_1_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10, hex_str);
		printf("Level 1 Password Validation_Field: %s\n", hex_str);
	}
	if (all_zeros_m10(uh->level_2_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10) == TRUE_m10)
		printf("Level 2 Password Validation_Field: no entry\n");
	else {
                generate_hex_string_m10(uh->level_2_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10, hex_str);
		printf("Level 2 Password Validation_Field: %s\n", hex_str);
	}
        if (all_zeros_m10(uh->level_3_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10) == TRUE_m10)
                printf("Level 3 Password Validation_Field: no entry\n");
        else {
                generate_hex_string_m10(uh->level_3_password_validation_field, PASSWORD_VALIDATION_FIELD_BYTES_m10, hex_str);
                printf("Level 3 Password Validation_Field: %s\n", hex_str);
        }
	printf("---------------- Universal Header - END ----------------\n\n");
        
	return;
}


void    snprintf_m10(si1 *target_string, si4 target_field_bytes, si1 *format, ...)
{
        va_list        args;
        si1        *c;
        si4        bytes_to_zero;
    
	
        if (target_field_bytes < 1) {
                *target_string = 0;
                return;
        }

        va_start(args, format);
        vsnprintf(target_string, target_field_bytes, format, args);
        va_end(args);
        
        c = target_string;
        while (*c++);
        
        bytes_to_zero = target_field_bytes - (c - target_string);
        if (bytes_to_zero > 0) {
                while (bytes_to_zero--)
                        *c++ = 0;
        } else {
                target_string[target_field_bytes - 1] = 0;
        }
        
        return;
}


si4     sprintf_m10(si1 *target, si1 *format, ...)
{
        va_list        args;
        si1        *c;
        
        
        va_start(args, format);
        vsprintf(target, format, args);
        va_end(args);
        
        c = target;
        while (*c++);
        
	return((si4) (c - target));
}


si4     strcat_m10(si1 *target_string, si1 *source_string)
{
        si1         *target_start;

        
        if (target_string == NULL || source_string == NULL)
                return(-1);

        target_start = target_string;
        while ((*target_string++));
        --target_string;
        while ((*target_string++ = *source_string++));
        
        return((si4) (target_string - target_start));
}


si4     strcpy_m10(si1 *target_string, si1 *source_string)
{
        si1         *source_start;
        
	
	// returns length including terminal zero
	
        if (target_string == NULL || source_string == NULL)
                return(-1);

        source_start = source_string;
        while ((*target_string++ = *source_string++));
        
        return((si4) (source_string - source_start));
}


void    strncat_m10(si1 *target_string, si1 *source_string, si4 target_field_bytes)
{
        if (target_string == NULL)
                return;
        if (target_field_bytes < 1) {
                *target_string = 0;
                return;
        }
        if (source_string == NULL) {
                --target_field_bytes;
        } else {
                while (--target_field_bytes)
                        if (*target_string++ == '\0')
                                break;
        }
        
        --target_string;
        ++target_field_bytes;
        
        while (--target_field_bytes)
                if ((*target_string++ = *source_string++) == '\0')
                        break;
        
        if (target_field_bytes)
                while (--target_field_bytes)
                        *target_string++ = '\0';
        
        *target_string = '\0';
        
        return;
}


void    strncpy_m10(si1 *target_string, si1 *source_string, si4 target_field_bytes)
{
        if (target_string == NULL)
                return;

        if (target_field_bytes < 1) {
                *target_string = 0;
                return;
        }
        
        if (source_string == NULL) {
                --target_field_bytes;
        } else {
                while (--target_field_bytes) {
                        if ((*target_string++ = *source_string++) == 0)
                                break;
                }
        }
        
        if (target_field_bytes)
                while (--target_field_bytes)
                        *target_string++ = '\0';
        
        *target_string = '\0';
        
        return;
}


si1     *time_string_m10(si8 uutc, si1 *time_str, TERN_m10 fixed_width, TERN_m10 relative_days, si4 colored_text, ...)  // time_str buffer sould be of length TIME_STRING_BYTES_m10
{
        si1             *standard_timezone_acronym, *standard_timezone_string, *date_color, *time_color, *color_reset, *meridian;
        static si1      private_time_str[TIME_STRING_BYTES_m10];
        static si1      mos[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        static si1      months[12][12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
        static si1      wdays[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        static si1      mday_num_sufs[32][3] = {"", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", \
                        "th", "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "st"};
        static si1      weekdays[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	TERN_m10	offset;
        si4             microseconds, DST_offset, day_num;
        si8             local_time, test_time;
        sf8             UTC_offset_hours;
        va_list         arg_p;
        struct tm       ti;
        
        
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
			break;
	}
	
	offset = TRUE_m10;
	if (globals_m10->RTO_known == TRUE_m10) {
		test_time = uutc - globals_m10->recording_time_offset;
		if (test_time < 0)  // time is offset
			uutc += globals_m10->recording_time_offset;
		offset = FALSE_m10;
		DST_offset = DST_offset_m10(uutc);
	} else {
		DST_offset = 0;
	}
	
	standard_timezone_acronym = globals_m10->standard_timezone_acronym;
	standard_timezone_string = globals_m10->standard_timezone_string;
	local_time = (si4) (uutc / (si8) 1000000) + DST_offset + globals_m10->standard_UTC_offset;
        microseconds = (si4) (uutc % (si8) 1000000);
        gmtime_r(&local_time, &ti);
        ti.tm_year += 1900;

        if (colored_text == TRUE_m10) {
                va_start(arg_p, colored_text);
                date_color = va_arg(arg_p, si1 *);
                time_color = va_arg(arg_p, si1 *);
                va_end(arg_p);
                color_reset = TC_RESET_m10;
        } else {
                date_color = time_color = color_reset = "";
        }
	if (relative_days == TRUE_m10) {
		uutc -= globals_m10->recording_time_offset;
		day_num = (si4) (uutc / TWENTY_FOURS_HOURS_m10) + 1;
	}

        if (fixed_width == TRUE_m10) {
                UTC_offset_hours = (sf8) (DST_offset + globals_m10->standard_UTC_offset) / (sf8) 3600.0;
		if (relative_days == TRUE_m10)
			sprintf(time_str, "%sDay %04d  %s%02d:%02d:%02d.%06d", date_color, day_num, time_color, ti.tm_hour, ti.tm_min, ti.tm_sec, microseconds);
		else
                	sprintf(time_str, "%s%s %02d %s %d  %s%02d:%02d:%02d.%06d", date_color, wdays[ti.tm_wday], ti.tm_mday, mos[ti.tm_mon], ti.tm_year, time_color, ti.tm_hour, ti.tm_min, ti.tm_sec, microseconds);
                if (DST_offset) {
                        if (UTC_offset_hours >= 0.0)
                                sprintf(time_str, "%s %s (UTC +%0.2lf)%s", time_str, globals_m10->daylight_timezone_acronym, UTC_offset_hours, color_reset);
                        else
                                sprintf(time_str, "%s %s (UTC %0.2lf)%s", time_str, globals_m10->daylight_timezone_acronym, UTC_offset_hours, color_reset);
                } else {
                        if (offset == TRUE_m10)  // no UTC offset displayed
                                sprintf(time_str, "%s %s%s", time_str, standard_timezone_acronym, color_reset);
                        else if (UTC_offset_hours >= 0.0)
                                sprintf(time_str, "%s %s (UTC +%0.2lf)%s", time_str, standard_timezone_acronym, UTC_offset_hours, color_reset);
                        else
                                sprintf(time_str, "%s %s (UTC %0.2lf)%s", time_str, standard_timezone_acronym, UTC_offset_hours, color_reset);
                }
        } else {
                ti.tm_sec += ((microseconds + 5e5) / 1e6);  // round to nearest second
                if (ti.tm_hour < 12) {
                        meridian = "AM";
                        if (ti.tm_hour == 0)
                                ti.tm_hour = 12;
                } else {
                        meridian = "PM";
                        if (ti.tm_hour > 12)
                                ti.tm_hour -= 12;
                }
		if (relative_days == TRUE_m10)
			sprintf(time_str, "%sDay %04d  %s%d:%02d:%02d %s,", date_color, day_num, time_color, ti.tm_hour, ti.tm_min, ti.tm_sec, meridian);
		else
                	sprintf(time_str, "%s%s, %s %d%s, %d  %s%d:%02d:%02d %s,", date_color, weekdays[ti.tm_wday], months[ti.tm_mon], ti.tm_mday, mday_num_sufs[ti.tm_mday], ti.tm_year, time_color, ti.tm_hour, ti.tm_min, ti.tm_sec, meridian);
                if (DST_offset)
                        sprintf(time_str, "%s %s%s", time_str, globals_m10->daylight_timezone_string, color_reset);
                else
                        sprintf(time_str, "%s %s%s", time_str, standard_timezone_string, color_reset);
        }

        return(time_str);
}


si8     ts_sort_m10(si4 *x, si8 len, NODE_m10 *nodes, NODE_m10 *head, NODE_m10 *tail, si4 return_sorted_ts, ...)
{
        TERN_m10        free_nodes;
        NODE_m10        *last_node, *next_node, *prev_node, *np;
        si8             i, j, n_nodes;
        sf8             new_val;
        si4             *sorted_x;
        va_list         arg_p;
                

        // setup
        if (nodes == NULL) {
                nodes = (NODE_m10 *) e_calloc_m10((size_t) len, sizeof(NODE_m10), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
                free_nodes = TRUE_m10;
        } else {
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
                } else if (new_val > last_node->val) {
                        for (next_node = last_node->next; new_val > next_node->val; next_node = next_node->next);
                        if (new_val == next_node->val) {
                                ++next_node->count;
                                last_node = next_node;
                                continue;
                        }
                        prev_node = next_node->prev;
                } else {  // new_val < last_node->val
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
                va_start(arg_p, return_sorted_ts);
                sorted_x = va_arg(arg_p, si4 *);
                va_end(arg_p);
                if (sorted_x == NULL)  {
                        warning_message_m10("%s(): passed sorted array pointer is NULL", __FUNCTION__);
                } else {
                        for (i = n_nodes, np = head->next; i--; np = np->next)
                                for (j = np->count; j--;)
                                        *sorted_x++ = np->val;
                }
        }
        
        if (free_nodes == TRUE_m10)
                e_free_m10(nodes, __FUNCTION__, __LINE__);
        
        return(n_nodes);
}


void    unescape_spaces_m10(si1 *string)
{
        si1     *c1, *c2;

        
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


// byte offset => charnum
si4 UTF8_charnum_m10(si1 *s, si4 offset)
{
	si4	charnum = 0, offs = 0;
	
	
	while (offs < offset && s[offs]) {
		(void)(UTF8_isutf_m10(s[++offs]) || UTF8_isutf_m10(s[++offs]) || UTF8_isutf_m10(s[++offs]) || ++offs);
		charnum++;
	}
        
	return(charnum);
}


inline void UTF8_dec_m10(si1 *s, si4 *i)
{
	(void) (UTF8_isutf_m10(s[--(*i)]) || UTF8_isutf_m10(s[--(*i)]) || UTF8_isutf_m10(s[--(*i)]) || --(*i));
        
        return;
}


si4 UTF8_escape_m10(si1 *buf, si4 sz, si1 *src, si4 escape_quotes)
{
	si4	c = 0, i = 0, amt;
	
	
	while (src[i] && c < sz) {
		if (escape_quotes && src[i] == '"') {
			amt = snprintf(buf, sz - c, "\\\"");
			i++;
		}
		else {
			amt = UTF8_escape_wchar_m10(buf, sz - c, UTF8_nextchar_m10(src, &i));
		}
		c += amt;
		buf += amt;
	}
	if (c < sz)
		*buf = '\0';
	
	return(c);
}


si4 UTF8_escape_wchar_m10(si1 *buf, si4 sz, ui4 ch)
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


si4 UTF8_fprintf_m10(FILE *stream, si1 *fmt, ...)
{
	si4	cnt;
	va_list args;
	
	
	va_start(args, fmt);
	cnt = UTF8_vfprintf_m10(stream, fmt, args);
	va_end(args);
        
	return(cnt);
}


inline si4 UTF8_hex_digit_m10(si1 c)
{
	return((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}


inline void UTF8_inc_m10(si1 *s, si4 *i)
{
	(void) (UTF8_isutf_m10(s[++(*i)]) || UTF8_isutf_m10(s[++(*i)]) || UTF8_isutf_m10(s[++(*i)]) || ++(*i));
}


void	UTF8_initialize_offsets_table_m10(void)
{
	ui4	*utf8_offsets_table;
	
	
	utf8_offsets_table = (ui4 *) e_calloc_m10((size_t) UTF8_OFFSETS_TABLE_ENTRIES_m10, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	{
		ui4 temp[UTF8_OFFSETS_TABLE_ENTRIES_m10] = UTF8_OFFSETS_TABLE_m10;
		memcpy((void *) utf8_offsets_table, (void *) temp, (size_t) UTF8_OFFSETS_TABLE_ENTRIES_m10 * sizeof(ui4));
	}
	
	globals_m10->UTF8_offsets_table = utf8_offsets_table;
	
	return;
}


void	UTF8_initialize_trailing_bytes_table_m10(void)
{
	si1	*UTF8_trailing_bytes_table;
	
	
	UTF8_trailing_bytes_table = (si1 *) e_calloc_m10((size_t) UTF8_TRAILING_BYTES_TABLE_ENTRIES_m10, sizeof(si1), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	
	{
		si1 temp[UTF8_TRAILING_BYTES_TABLE_ENTRIES_m10] = UTF8_TRAILING_BYTES_TABLE_m10;
		memcpy((void *) UTF8_trailing_bytes_table, (void *) temp, (size_t) UTF8_TRAILING_BYTES_TABLE_ENTRIES_m10);
	}
	
	globals_m10->UTF8_trailing_bytes_table = UTF8_trailing_bytes_table;
	
	return;
}


si4     UTF8_is_locale_utf8_m10(si1 *locale)
{
	// this code based on libutf8
	const si1	*cp = locale;
	
        
	for (; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ','; cp++) {
		if (*cp == '.') {
			const si1 *encoding = ++cp;
			for (; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ','; cp++)
				;
			if ((cp - encoding == 5 && !strncmp(encoding, "UTF-8", 5)) || (cp - encoding == 4 && !strncmp(encoding, "utf8", 4)))
				return(1); // it's UTF-8
			break;
		}
	}
	
	return(0);
}


si1     *UTF8_memchr_m10(si1 *s, ui4 ch, size_t sz, si4 *charn)
{
	si4	i = 0, lasti = 0;
	ui4	c;
	si4	csz;
	
        
	if (globals_m10->UTF8_offsets_table == NULL)
		UTF8_initialize_offsets_table_m10();
	
	*charn = 0;
	while (i < sz) {
		c = csz = 0;
		do {
			c <<= 6;
			c += (ui1) s[i++];
			csz++;
		} while (i < sz && !UTF8_isutf_m10(s[i]));
		c -= globals_m10->UTF8_offsets_table[csz - 1];
		
		if (c == ch) {
			return(&s[lasti]);
		}
		lasti = i;
		(*charn)++;
	}
	
	return(NULL);
}


// reads the next utf-8 sequence out of a string, updating an index
ui4     UTF8_nextchar_m10(si1 *s, si4 *i)
{
	ui4	ch = 0;
	si4	sz = 0;
	
        
	if (globals_m10->UTF8_offsets_table == NULL)
		UTF8_initialize_offsets_table_m10();
	
	do {
		ch <<= 6;
		ch += (ui1) s[(*i)++];
		sz++;
	} while (s[*i] && !UTF8_isutf_m10(s[*i]));
	
	ch -= globals_m10->UTF8_offsets_table[sz - 1];
        
	return(ch);
}


inline si4 UTF8_octal_digit_m10(si1 c)
{
	return(c >= '0' && c <= '7');
}


// charnum => byte offset
si4     UTF8_offset_m10(si1 *str, si4 charnum)
{
	si4	offs = 0;
	
        
	while (charnum > 0 && str[offs]) {
		(void)(UTF8_isutf_m10(str[++offs]) || UTF8_isutf_m10(str[++offs]) || UTF8_isutf_m10(str[++offs]) || ++offs);
		charnum--;
	}
	
	return(offs);
}


si4     UTF8_printf_m10(si1 *fmt, ...)
{
	si4	cnt;
	va_list args;
	
        
	va_start(args, fmt);
	cnt = UTF8_vprintf_m10(fmt, args);
	va_end(args);
	
	return(cnt);
}


// assumes that src points to the character after a backslash
// returns number of input characters processed
si4     UTF8_read_escape_sequence_m10(si1 *str, ui4 *dest)
{
	ui4	ch;
	si1	digs[9] = "\0\0\0\0\0\0\0\0";
	si4	dno = 0, i = 1;
	
        
	ch = (ui4) str[0];    // take literal character
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
		UTF8_initialize_trailing_bytes_table_m10();
	
	return(globals_m10->UTF8_trailing_bytes_table[(si4) (ui1) s[0]] + 1);
}


si1     *UTF8_strchr_m10(si1 *s, ui4 ch, si4 *charn)
{
	si4	i = 0, lasti = 0;
	ui4	c;
	
        
	*charn = 0;
	while (s[i]) {
		c = UTF8_nextchar_m10(s, &i);
		if (c == ch) {
			return(&s[lasti]);
		}
		lasti = i;
		(*charn)++;
	}
	
	return(NULL);
}


// number of characters
si4     UTF8_strlen_m10(si1 *s)
{
	si4	count = 0;
	si4	i = 0;
	
        
	while (UTF8_nextchar_m10(s, &i) != 0)
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
si4     UTF8_toucs_m10(ui4 *dest, si4 sz, si1 *src, si4 srcsz)
{
	ui4	ch;
	si1	*src_end = src + srcsz;
	si4	nb;
	si4	i = 0;
	
        
	if (globals_m10->UTF8_offsets_table == NULL)
		UTF8_initialize_offsets_table_m10();
	
	if (globals_m10->UTF8_trailing_bytes_table == NULL)
		UTF8_initialize_trailing_bytes_table_m10();
	
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
// only NUL-terminates if all the characters fit, and there's space for
// the NUL as well.
// the destination string will never be bigger than the source string
si4     UTF8_toutf8_m10(si1 *dest, si4 sz, ui4 *src, si4 srcsz)
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
			ch = (u_int32_t)*src;
			amt = 1;
		}
		src += amt;
		amt = UTF8_wc_toutf8_m10(temp, ch);
		if (amt > sz - c)
			break;
		memcpy(&buf[c], temp, amt);
		c += amt;
	}
	if (c < sz)
		buf[c] = '\0';
	
	return(c);
}


si4     UTF8_vfprintf_m10(FILE *stream, si1 *fmt, va_list ap)
{
	si4	cnt, sz = UTF8_BUFFER_SIZE;
	si1	*buf;
	ui4	*wcs;
	
	
	buf = (si1 *) e_malloc_m10(sz, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	        
	cnt = vsnprintf(buf, sz, fmt, ap);
	if (cnt >= sz) {
		buf = (si1 *) e_realloc_m10(buf, cnt + 1, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		cnt = vsnprintf(buf, cnt + 1, fmt, ap);
	}
	wcs = (ui4 *) e_calloc_m10(cnt + 1, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	cnt = UTF8_toucs_m10(wcs, cnt + 1, buf, cnt);
	fprintf(stream, "%ls", (wchar_t *) wcs);
	
	e_free_m10(buf, __FUNCTION__, __LINE__);
	e_free_m10(wcs, __FUNCTION__, __LINE__);
	
	return(cnt);
}


si4     UTF8_vprintf_m10(si1 *fmt, va_list ap)
{
	si4	cnt, sz = UTF8_BUFFER_SIZE;
	si1	*buf;
	ui4	*wcs;
	
        
	buf = (si1 *) e_malloc_m10(sz, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
        
	cnt = vsnprintf(buf, sz, fmt, ap);
	if (cnt >= sz) {
		buf = (si1 *) e_realloc_m10(buf, cnt + 1, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
		cnt = vsnprintf(buf, cnt + 1, fmt, ap);
	}
	wcs = (ui4 *) e_calloc_m10(cnt + 1, sizeof(ui4), __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
	cnt = UTF8_toucs_m10(wcs, cnt + 1, buf, cnt);
	printf("%ls", (wchar_t *) wcs);
	
	e_free_m10(buf, __FUNCTION__, __LINE__);
	e_free_m10(wcs, __FUNCTION__, __LINE__);
	
	return(cnt);
}


si4     UTF8_wc_toutf8_m10(si1 *dest, ui4 ch)
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
//**************************  END UTF-8 FUNCTIONS  **********************//
//***********************************************************************//


si8     uutc_for_sample_number_m10(si8 ref_sample_number, si8 ref_uutc, si8 target_sample_number, sf8 sampling_frequency, FILE_PROCESSING_STRUCT_m10 *time_series_indices_fps, ui1 mode)
{
        TERN_m10                absolute_numbering_flag = TRUE_m10;
        si8                     uutc, i, tmp_si8, n_inds;
        sf8                     tmp_sf8;
        TIME_SERIES_INDEX_m10   *tsi;
        
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
			sampling_frequency = (sf8) (tsi[i].start_sample_number - ref_sample_number);
			sampling_frequency /= ((sf8) (tsi[i].start_time - ref_uutc) / (sf8) 1e6);
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
                        tmp_si8 = (si8) (((tmp_sf8 + (sf8) 5e5) / sampling_frequency) + (sf8) 0.5);
                        break;
                case FIND_START_m10:
                default:
                        tmp_sf8 = tmp_sf8 / sampling_frequency;
                        tmp_si8 = (si8) tmp_sf8;
                        if (tmp_sf8 != (sf8) tmp_si8)
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

                valid = CRC_validate_m10((ui1 *) record_header + RECORD_HEADER_RECORD_CRC_START_OFFSET_m10, record_header->total_record_bytes - RECORD_HEADER_RECORD_CRC_START_OFFSET_m10, record_header->record_CRC);
                if (valid == FALSE_m10)
                        return(valid);
                
                record_header = (RECORD_HEADER_m10 *) ((ui1 *) record_header + record_header->total_record_bytes);
        }
        
        return(valid);
}


TERN_m10        validate_time_series_data_CRCs_m10(CMP_BLOCK_FIXED_HEADER_m10 *block_header, si8 number_of_items)
{
        TERN_m10        valid;
        si8             i;

        
        valid = TRUE_m10;
        for (i = 0; i < number_of_items; ++i) {

                valid = CRC_validate_m10((ui1 *) block_header + CMP_BLOCK_CRC_START_OFFSET_m10, block_header->total_block_bytes - CMP_BLOCK_CRC_START_OFFSET_m10, block_header->block_CRC);
                if (valid == FALSE_m10)
                        return(valid);
                
                block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *) block_header + block_header->total_block_bytes);
        }
        
        return(valid);
}


void    warning_message_m10(si1 *fmt, ...)
{
        va_list args;

        
        if (!(globals_m10->behavior_on_fail & SUPPRESS_WARNING_OUTPUT_m10)) {
                va_start(args, fmt);
                UTF8_vfprintf_m10(stderr, fmt, args);
                va_end(args);
                fprintf(stderr, "\n");
                fflush(stderr);
        }
        
        return;
}


si8     write_file_m10(FILE_PROCESSING_STRUCT_m10 *fps, ui8 number_of_items, void *data_ptr, ui4 behavior_on_fail)
{
        ui1                             *decrypted_data;
        ui4                             entry_size;
        si8                             i, out_bytes;
        RECORD_HEADER_m10               *record_header;
        CMP_BLOCK_FIXED_HEADER_m10      *block_header, *saved_block_header;
        UNIVERSAL_HEADER_m10            *uh;
	
        
	// mutex on
	fps_mutex_on_m10(fps);
	
        if (behavior_on_fail == USE_GLOBAL_BEHAVIOR_m10)
                behavior_on_fail = globals_m10->behavior_on_fail;
        
	// clobber file if exists and is closed, create if non-existent
	if (fps->fp == NULL) {
		if (!(fps->directives.open_mode & FPS_GENERIC_WRITE_OPEN_MODE_m10))
			fps->directives.open_mode = FPS_W_OPEN_MODE_m10;
		fps_open_m10(fps, __FUNCTION__, __LINE__, behavior_on_fail);
	}
        
        // set pointer, if necessary
        if (data_ptr == NULL)
                data_ptr = (void *) (fps->raw_data + UNIVERSAL_HEADER_BYTES_m10);
        
        out_bytes = 0;
        uh = fps->universal_header;
        if (number_of_items == FPS_UNIVERSAL_HEADER_ONLY_m10) {
                e_fseek_m10(fps->fp, 0, SEEK_SET, fps->full_file_name, __FUNCTION__, __LINE__, EXIT_ON_FAIL_m10);
                out_bytes = UNIVERSAL_HEADER_BYTES_m10;
                data_ptr = (void *) fps->raw_data;
                number_of_items = 0;
        } else if (number_of_items == FPS_FULL_FILE_m10) {
                e_fseek_m10(fps->fp, 0, SEEK_SET, fps->full_file_name, __FUNCTION__, __LINE__, EXIT_ON_FAIL_m10);
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
                                block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) data_ptr;
                                for (i = 0; i < number_of_items; ++i) {
                                        entry_size = block_header->total_block_bytes;
                                        if (uh->maximum_entry_size < entry_size)
                                                uh->maximum_entry_size = entry_size;
                                        out_bytes += entry_size;
                                        block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) ((ui1 *) block_header + block_header->total_block_bytes);
                                }
                                break;
                        case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
                                out_bytes += number_of_items * TIME_SERIES_INDEX_BYTES_m10;
                                break;
                        case RECORD_DATA_FILE_TYPE_CODE_m10:
                                record_header = (RECORD_HEADER_m10 *) data_ptr;
                                for (i = 0; i < number_of_items; ++i) {
                                        entry_size = record_header->total_record_bytes;
                                        if (uh->maximum_entry_size < entry_size)
                                                uh->maximum_entry_size = entry_size;
                                        out_bytes += entry_size;
                                        record_header = (RECORD_HEADER_m10 *) ((ui1 *) record_header + record_header->total_record_bytes);
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
					decrypted_data = (ui1 *) e_malloc_m10(out_bytes, __FUNCTION__, __LINE__, USE_GLOBAL_BEHAVIOR_m10);
					memcpy(decrypted_data, data_ptr, out_bytes);
					break;
			}
                }
                
                // encrypt
                switch (uh->type_code) {
                        case RECORD_DATA_FILE_TYPE_CODE_m10:
                                encrypt_records_m10(fps, (RECORD_HEADER_m10 *) data_ptr, number_of_items);
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
                                        calculate_time_series_data_CRCs_m10(fps, (CMP_BLOCK_FIXED_HEADER_m10 *) data_ptr, number_of_items);
                                        break;
                                case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
                                        calculate_time_series_indices_CRCs_m10(fps, (TIME_SERIES_INDEX_m10 *) data_ptr, number_of_items);
                                        break;
                                case RECORD_DATA_FILE_TYPE_CODE_m10:
                                        calculate_record_data_CRCs_m10(fps, (RECORD_HEADER_m10 *) data_ptr, number_of_items);
                                        break;
                                case RECORD_INDICES_FILE_TYPE_CODE_m10:
                                        calculate_record_indices_CRCs_m10(fps, (RECORD_INDEX_m10 *) data_ptr, number_of_items);
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
        fps_write_m10(fps, out_bytes, data_ptr, __FUNCTION__, __LINE__, behavior_on_fail);
        
	// leave decrypted directive
	if (fps->directives.leave_decrypted == TRUE_m10) {
		switch (uh->type_code) {
			case TIME_SERIES_DATA_FILE_TYPE_CODE_m10:
				// in case data_ptr is not cps block_header
				saved_block_header = fps->cps->block_header;
				fps->cps->block_header = (CMP_BLOCK_FIXED_HEADER_m10 *) data_ptr;
				decrypt_time_series_data_m10(fps->cps, number_of_items);
				fps->cps->block_header = saved_block_header;
				break;
			case TIME_SERIES_INDICES_FILE_TYPE_CODE_m10:
			case VIDEO_INDICES_FILE_TYPE_CODE_m10:
			case RECORD_INDICES_FILE_TYPE_CODE_m10:
			case RECORD_DATA_FILE_TYPE_CODE_m10:
				memcpy(data_ptr, decrypted_data, out_bytes);
				e_free_m10(decrypted_data, __FUNCTION__, __LINE__);
				break;
		}
	}

	// close
	if (fps->directives.close_file == TRUE_m10)
		fps_close_m10(fps);
	
	// show
	if (globals_m10->verbose == TRUE_m10)
		show_file_processing_struct_m10(fps);
	
	// mutex off
	fps_mutex_off_m10(fps);

	return(out_bytes);
}


