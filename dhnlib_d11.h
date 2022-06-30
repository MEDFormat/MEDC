
//**********************************************************************************//
//************************  DARK HORSE NEURO C Library 1.0.1  **********************//
//**********************************************************************************//

// Written by Matt Stead
// Copyright Dark Horse Neuro Inc, 2020


#ifndef DHNLIB_IN_d11
#define DHNLIB_IN_d11


//**********************************************************************************//
//********************************  Library Includes  ******************************//
//**********************************************************************************//

#include "targets_m11.h"
#include "medlib_m11.h"

// Platform-specific Includes & Defines
#if defined MACOS_m11 || defined LINUX_m11
	#include <pthread.h>
	#include <sched.h>
	#include <sys/mman.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <sys/param.h>
	#include <sys/mount.h>
	#include <termios.h>
#endif
#ifdef LINUX_m11
	#include <sys/statfs.h>
	#include <sys/sysinfo.h>
#endif
#ifdef MACOS_m11
	#include <sys/sysctl.h>
#endif
#ifdef WINDOWS_m11
#include <stddef.h>
#include <io.h>
#include <winsock.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <process.h>
#include <processthreadsapi.h>
#include <synchapi.h>
#include <sysinfoapi.h>
#pragma comment(lib, "Ws2_32.lib")  // Link with ws2_32.lib
#endif


//**********************************************************************************//
//***********  Memory-related Standard Functions (with alloc tracking)  ************//
//**********************************************************************************//


// Function Prototypes (allocation tracking integrated)
void	*calloc_d11(size_t n_members, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail);
void	**calloc_2D_d11(size_t dim1, size_t dim2, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail);
void	free_d11(void *ptr, const si1 *function, si4 line);
void	free_2D_d11(void **ptr, si8 dim1, const si1 *function, si4 line);
void	*malloc_d11(size_t n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail);
si4	mlock_d11(void *ptr, size_t n_bytes, TERN_m11 zero_data, const si1 *function, si4 line, ui4 behavior_on_fail);
si4	munlock_d11(void *ptr, size_t n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail);
void	*realloc_d11(void *ptr, size_t n_bytes, const si1 *function, si4 line, ui4 behavior_on_fail);
void	**realloc_2D_d11(void **ptr, size_t curr_dim1, size_t new_dim1, size_t curr_dim2, size_t new_dim2, size_t el_size, const si1 *function, si4 line, ui4 behavior_on_fail);



//**********************************************************************************//
//***********************  CMP (COMPRESSION / COMPUTATION)  ************************//
//**********************************************************************************//

// Binterpolate() center mode codes
#define CMP_CENT_MODE_NONE_d11			0  // extrema only
#define CMP_CENT_MODE_MIDPOINT_d11		1  // best performance if extrema needed: (min + max) / 2
#define CMP_CENT_MODE_MEAN_d11			2  // best performance  if extrema not needed
#define CMP_CENT_MODE_MEDIAN_d11		3  // best measure of central tendency
#define CMP_CENT_MODE_FASTEST_d11		4  // CMP_CENT_MODE_MIDPOINT_d11 if extrema requested, CMP_CENT_MODE_MEAN_d11 if not
#define CMP_CENT_MODE_DEFAULT_d11		CMP_CENT_MODE_MEAN_d11

// Normal cumulative distribution function values from -3 to +3 standard deviations in 0.1 sigma steps
#define CMP_NORMAL_CDF_TABLE_ENTRIES_d11        61
#define CMP_NORMAL_CDF_TABLE_d11	      { 0.00134989803163010, 0.00186581330038404, 0.00255513033042794, 0.00346697380304067, \
						0.00466118802371875, 0.00620966532577614, 0.00819753592459614, 0.01072411002167580, \
						0.01390344751349860, 0.01786442056281660, 0.02275013194817920, 0.02871655981600180, \
						0.03593031911292580, 0.04456546275854310, 0.05479929169955800, 0.06680720126885810, \
						0.08075665923377110, 0.09680048458561040, 0.11506967022170800, 0.13566606094638300, \
						0.15865525393145700, 0.18406012534676000, 0.21185539858339700, 0.24196365222307300, \
						0.27425311775007400, 0.30853753872598700, 0.34457825838967600, 0.38208857781104700, \
						0.42074029056089700, 0.46017216272297100, 0.50000000000000000, 0.53982783727702900, \
						0.57925970943910300, 0.61791142218895300, 0.65542174161032400, 0.69146246127401300, \
						0.72574688224992600, 0.75803634777692700, 0.78814460141660300, 0.81593987465324100, \
						0.84134474606854300, 0.86433393905361700, 0.88493032977829200, 0.90319951541439000, \
						0.91924334076622900, 0.93319279873114200, 0.94520070830044200, 0.95543453724145700, \
						0.96406968088707400, 0.97128344018399800, 0.97724986805182100, 0.98213557943718300, \
						0.98609655248650100, 0.98927588997832400, 0.99180246407540400, 0.99379033467422400, \
						0.99533881197628100, 0.99653302619695900, 0.99744486966957200, 0.99813418669961600, \
						0.99865010196837000 }

#define CMP_SUM_NORMAL_CDF_d11                  30.5
#define CMP_SUM_SQ_NORMAL_CDF_d11               24.864467406647070
#define CMP_KS_CORRECTION_d11                   ((sf8) 0.0001526091333688973)

#define VDS_THRESHOLD_MAP_TABLE_ENTRIES_d11	101
#define VDS_THRESHOLD_MAP_TABLE_d11 { \
	{ 0.0, 0.653145437887747, 0.087080239615529 }, \
	{ 0.1, 0.666524244368605, 0.096431833334447 }, \
	{ 0.2, 0.679901382612239, 0.105786886279366 }, \
	{ 0.3, 0.693195278745327, 0.115136328803986 }, \
	{ 0.4, 0.706328010262686, 0.124471091262003 }, \
	{ 0.5, 0.719312446055280, 0.133782104007116 }, \
	{ 0.6, 0.732256199033084, 0.143060297393023 }, \
	{ 0.7, 0.745271324448842, 0.152296601773421 }, \
	{ 0.8, 0.758469877555299, 0.161481947502008 }, \
	{ 0.9, 0.771963913605199, 0.170607264932484 }, \
	{ 1.0, 0.785865487851287, 0.179663484418544 }, \
	{ 1.1, 0.800286655103895, 0.188641536313887 }, \
	{ 1.2, 0.815276097313215, 0.197532350972212 }, \
	{ 1.3, 0.830643391297273, 0.206329712723042 }, \
	{ 1.4, 0.846141732247177, 0.215054845489400 }, \
	{ 1.5, 0.861524315354039, 0.223744284804766 }, \
	{ 1.6, 0.876544335808968, 0.232434732668414 }, \
	{ 1.7, 0.890954988803074, 0.241162891079617 }, \
	{ 1.8, 0.904515693506558, 0.249965462037652 }, \
	{ 1.9, 0.917339694399690, 0.258879147541791 }, \
	{ 2.0, 0.930077675257187, 0.267940649591310 }, \
	{ 2.1, 0.943425290156307, 0.277186670185483 }, \
	{ 2.2, 0.958078193174307, 0.286653911323584 }, \
	{ 2.3, 0.974732038388447, 0.296379075004888 }, \
	{ 2.4, 0.994082479875984, 0.306398209551974 }, \
	{ 2.5, 1.016674233046698, 0.316738820477894 }, \
	{ 2.6, 1.041615782368465, 0.327422417795757 }, \
	{ 2.7, 1.067220754080376, 0.338470387411485 }, \
	{ 2.8, 1.091794360492593, 0.349904115230999 }, \
	{ 2.9, 1.113758598041713, 0.361744987160223 }, \
	{ 3.0, 1.133017949669186, 0.374014389105076 }, \
	{ 3.1, 1.150496180583432, 0.386733706971483 }, \
	{ 3.2, 1.167137104226452, 0.399924326665363 }, \
	{ 3.3, 1.183884534040251, 0.413618810138173 }, \
	{ 3.4, 1.201682114424834, 0.427895185364072 }, \
	{ 3.5, 1.221180179543847, 0.442843040517614 }, \
	{ 3.6, 1.242133433412453, 0.458551963775380 }, \
	{ 3.7, 1.264126230660231, 0.475111543313956 }, \
	{ 3.8, 1.286742925916765, 0.492611367309924 }, \
	{ 3.9, 1.309573914909860, 0.511146276456767 }, \
	{ 4.0, 1.332419792627307, 0.530891512927402 }, \
	{ 4.1, 1.355340749569296, 0.552085150181815 }, \
	{ 4.2, 1.378413034757496, 0.574966933045358 }, \
	{ 4.3, 1.401712891490464, 0.599776606343383 }, \
	{ 4.4, 1.425298489977415, 0.626806804776781 }, \
	{ 4.5, 1.449169878829132, 0.656567276545111 }, \
	{ 4.6, 1.473315445926537, 0.689623472979960 }, \
	{ 4.7, 1.497723579150550, 0.726578911803450 }, \
	{ 4.8, 1.522488258560118, 0.768420064481964 }, \
	{ 4.9, 1.548262083298817, 0.816356801068373 }, \
	{ 5.0, 1.575881693662255, 0.871725955480487 }, \
	{ 5.1, 1.606179052450206, 0.936062669220404 }, \
	{ 5.2, 1.638914040756128, 1.010951239878917 }, \
	{ 5.3, 1.671426199703439, 1.097583980703378 }, \
	{ 5.4, 1.700723162658563, 1.195933599766891 }, \
	{ 5.5, 1.725174105101550, 1.305244713299358 }, \
	{ 5.6, 1.748170849533368, 1.423680806598649 }, \
	{ 5.7, 1.774263115611458, 1.548589345494656 }, \
	{ 5.8, 1.807974531435571, 1.678111066838628 }, \
	{ 5.9, 1.849031541061601, 1.810472754539325 }, \
	{ 6.0, 1.886797330616806, 1.944453521649560 }, \
	{ 6.1, 1.910022474285093, 2.079203902778041 }, \
	{ 6.2, 1.922265774682071, 2.214633042233715 }, \
	{ 6.3, 1.940778181878725, 2.350143024521086 }, \
	{ 6.4, 1.983325396122841, 2.485226121348756 }, \
	{ 6.5, 2.061401451135639, 2.619918574839387 }, \
	{ 6.6, 2.133662260846322, 2.754123018668519 }, \
	{ 6.7, 2.176369286579269, 2.887938280170844 }, \
	{ 6.8, 2.208654721196260, 3.021880758743963 }, \
	{ 6.9, 2.244989253035562, 3.156354273762239 }, \
	{ 7.0, 2.288044683671034, 3.290783349148508 }, \
	{ 7.1, 2.338500365190698, 3.425483693188855 }, \
	{ 7.2, 2.390947777679918, 3.560967005267627 }, \
	{ 7.3, 2.438785285059024, 3.697362380003563 }, \
	{ 7.4, 2.488873238561683, 3.834765208006531 }, \
	{ 7.5, 2.551586167259061, 3.973524655912109 }, \
	{ 7.6, 2.621198805891288, 4.113956660573023 }, \
	{ 7.7, 2.687949002958154, 4.256298709504494 }, \
	{ 7.8, 2.749458372117692, 4.400925898369564 }, \
	{ 7.9, 2.819492290144386, 4.548231341439871 }, \
	{ 8.0, 2.895978757474240, 4.698585849519076 }, \
	{ 8.1, 2.955556103053909, 4.852443976714582 }, \
	{ 8.2, 3.069362319165638, 5.010398618334849 }, \
	{ 8.3, 3.168267272628890, 5.172880252796954 }, \
	{ 8.4, 3.259217694239259, 5.340703709726352 }, \
	{ 8.5, 3.372393284309650, 5.514642980430471 }, \
	{ 8.6, 3.480506090966784, 5.695814408509766 }, \
	{ 8.7, 3.613539318894531, 5.885218071139942 }, \
	{ 8.8, 3.746867732981435, 6.084360685571760 }, \
	{ 8.9, 3.892198202949155, 6.294816359498106 }, \
	{ 9.0, 4.047865691316694, 6.518719318957218 }, \
	{ 9.1, 4.238233461465871, 6.758827410761595 }, \
	{ 9.2, 4.427441777303221, 7.018317189344236 }, \
	{ 9.3, 4.652784008515709, 7.301166878082886 }, \
	{ 9.4, 4.900609519620827, 7.612468057618706 }, \
	{ 9.5, 5.207111696945288, 7.960975856395534 }, \
	{ 9.6, 5.573339028134005, 8.358863805875041 }, \
	{ 9.7, 6.028522124546244, 8.820648191204029 }, \
	{ 9.8, 6.700281313487421, 9.364866314353193 }, \
	{ 9.9, 7.649180239136531, 10.019987941003953 }, \
	{ 10.0, 9.326074985746468, 10.846993908980265 } \
}


typedef struct {
	sf8	user_threshold;
	sf8	algorithm_threshold_LFP;
	sf8	algorithm_threshold_no_filt;
} VDS_THRESHOLD_MAP_ENTRY_d11;


void		CMP_binterpolate_sf8_d11(sf8 *in_data, si8 in_len, sf8 *out_data, si8 out_len, ui4 center_mode, TERN_m11 extrema, sf8 *minima, sf8 *maxima);
sf8      	CMP_calculate_mean_residual_ratio_d11(si4 *original_data, si4 *lossy_data, ui4 n_samps);
void    	CMP_detrend_d11(si4 *input_buffer, si4 *output_buffer, si8 len, CMP_PROCESSING_STRUCT_m11 *cps);
void		CMP_encode_d11(FILE_PROCESSING_STRUCT_m11 *fps, si8 start_time, si4 acquisition_channel_number, ui4 number_of_samples);
TERN_m11	CMP_find_amplitude_scale_d11(CMP_PROCESSING_STRUCT_m11 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m11 *cps));
si8    		*CMP_find_crits_d11(sf8 *data, si8 data_len, si8 *n_crits, si8 *crit_xs);
void    	CMP_find_crits_2_d11(sf8 *data, si8 data_len, si8 *n_peaks, si8 *peak_xs, si8 *n_troughs, si8 *trough_xs);
TERN_m11	CMP_find_frequency_scale_d11(CMP_PROCESSING_STRUCT_m11 *cps, void (*compression_f)(CMP_PROCESSING_STRUCT_m11 *cps));
void    	CMP_generate_lossy_data_d11(CMP_PROCESSING_STRUCT_m11 *cps, si4* input_buffer, si4 *output_buffer, ui1 mode);
void    	CMP_lad_reg_2_sf8_d11(sf8 *x_input_buffer, sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lad_reg_2_si4_d11(si4 *x_input_buffer, si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lad_reg_sf8_d11(sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lad_reg_si4_d11(si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lin_reg_2_sf8_d11(sf8 *x_input_buffer, sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lin_reg_2_si4_d11(si4 *x_input_buffer, si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lin_reg_sf8_d11(sf8 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void    	CMP_lin_reg_si4_d11(si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
ui1     	CMP_normality_score_d11(si4 *data, ui4 n_samps);
sf8     	CMP_quantval_d11(sf8 *data, si8 len, sf8 quantile, TERN_m11 preserve_input, sf8 *buff);
sf8		CMP_splope_d11(sf8 *xa, sf8 *ya, sf8 *d2y, sf8 x, si8 lo_pt, si8 hi_pt);
void		CMP_VDS_encode_d11(CMP_PROCESSING_STRUCT_m11 *cps);
void		CMP_VDS_generate_template_d11(CMP_PROCESSING_STRUCT_m11 *cps, si8 data_len);
sf8		CMP_VDS_get_theshold_d11(CMP_PROCESSING_STRUCT_m11 *cps);



//**********************************************************************************//
//************************************  FILTER  ************************************//
//**********************************************************************************//

// ATTRIBUTION
//
// Some of the filter code was adapted from Matlab functions (MathWorks, Inc).
// www.mathworks.com
//
// The c code was written entirely from scratch.
//
// NOTE: This code requires long double (sf16) math.
// It may require an explicit compiler option to implement true long floating point math.
// The option in icc is: "-Qoption,cpp,--extended_float_type"


// Constants
#define FILT_LOWPASS_TYPE_d11                   	1
#define FILT_BANDPASS_TYPE_d11                  	2
#define FILT_HIGHPASS_TYPE_d11                  	3
#define FILT_BANDSTOP_TYPE_d11                  	4
#define FILT_TYPE_DEFAULT_d11                   	FILT_LOWPASS_TYPE_d11
#define FILT_ORDER_DEFAULT_d11                  	5
#define FILT_PAD_SAMPLES_PER_POLE_d11			3  // minimum == 3
#define FILT_MAX_ORDER_d11                      	10
#define FILT_BAD_FILTER_d11                     	-1
#define FILT_BAD_DATA_d11                       	-2
#define FILT_EPS_SF8_d11                        	2.22045e-16
#define FILT_EPS_SF16_d11                       	1.0842e-19
#define FILT_RADIX_d11                          	2
#define FILT_ZERO_d11                           	((sf16) 0.0)
#define FILT_ONE_d11                            	((sf16) 1.0)
#define FILT_LINE_NOISE_HARMONICS_DEFAULT_d11   	4
#define FILT_ANTIALIAS_FREQ_DIVISOR_DEFAULT_d11 	((sf8) 3.5);
#define FILT_UNIT_THRESHOLD_DEFAULT_d11			CMP_PARAMETERS_VDS_UNIT_THRESHOLD_DEFAULT_m11
#define FILT_NFF_BUFFERS_d11				4
#define FILT_VDS_TEMPLATE_MIN_PS_d11			0  // index of CPS filtps
#define FILT_VDS_TEMPLATE_LFP_PS_d11			1  // index of CPS filtps

// Quantfilt Tail Options
#define FILT_TRUNCATE_d11                        1
#define FILT_EXTRAPOLATE_d11                     2
#define FILT_ZEROPAD_d11                         3
#define FILT_DEFAULT_TAIL_OPTION_CODE_d11        FILT_TRUNCATE_d11

// Macros
#define FILT_ABS_d11(x)             		((x) >= FILT_ZERO_d11 ? (x) : (-x))
#define FILT_SIGN_d11(x, y)         		((y) >= FILT_ZERO_d11 ? FILT_ABS_d11(x) : -FILT_ABS_d11(x))
// filtps->n_poles = poles = n_fcs * order;
#define FILT_POLES_d11(order, cutoffs)		(order * cutoffs)
#define FILT_FILT_PAD_SAMPLES_d11(poles)	(poles * FILT_PAD_SAMPLES_PER_POLE_d11 * 2)
#define FILT_OFFSET_ORIG_DATA_d11(filtps)	(filtps->filt_data + (filtps->n_poles * FILT_PAD_SAMPLES_PER_POLE_d11))

// Typedefs & Structs
typedef struct {
	ui4	behavior_on_fail;
	si4	order;
	si4	n_poles;  // n_poles == order * n_cutoffs
	si4	type;
	sf8	sampling_frequency;
	si8	data_length;
	sf8	cutoffs[2];
	sf8	*numerators;  // entries == n_poles + 1
	sf8	*denominators;  // entries == n_poles + 1
	sf8	*initial_conditions;  // entries == n_poles
	sf8	*orig_data;
	sf8	*filt_data;
	sf8	*buffer;
} FILT_PROCESSING_STRUCT_d11;

typedef struct {
	sf16        real;
	sf16        imag;
} FILT_LONG_COMPLEX_d11;

typedef struct FILT_NODE_STRUCT {
	sf8			val;
	si4			idx;
	struct FILT_NODE_STRUCT *prev, *next;
} FILT_NODE_d11;

// Prototypes
void    FILT_balance_d11(sf16 **a, si4 poles);
si4     FILT_butter_d11(FILT_PROCESSING_STRUCT_d11 *filtps);
void    FILT_complex_divl_d11(FILT_LONG_COMPLEX_d11 *a, FILT_LONG_COMPLEX_d11 *b, FILT_LONG_COMPLEX_d11 *quotient);
void    FILT_complex_expl_d11(FILT_LONG_COMPLEX_d11 *exponent, FILT_LONG_COMPLEX_d11 *ans);
void    FILT_complex_multl_d11(FILT_LONG_COMPLEX_d11 *a, FILT_LONG_COMPLEX_d11 *b, FILT_LONG_COMPLEX_d11 *product);
void    FILT_elmhes_d11(sf16 **a, si4 poles);
void	FILT_excise_transients_d11(CMP_PROCESSING_STRUCT_m11 *cps, si8 data_len, si8 *n_extrema);
si4     FILT_filtfilt_d11(FILT_PROCESSING_STRUCT_d11 *filtps);
void	FILT_free_CPS_filtps_d11(CMP_PROCESSING_STRUCT_m11 *cps, TERN_m11 free_orig_data, TERN_m11 free_filt_data, TERN_m11 free_buffer);
void    FILT_free_processing_struct_d11(FILT_PROCESSING_STRUCT_d11 *filtps, TERN_m11 free_orig_data, TERN_m11 free_filt_data, TERN_m11 free_buffer, TERN_m11 free_structure);
FILT_PROCESSING_STRUCT_d11  *FILT_initialize_processing_struct_d11(si4 order, si4 type, sf8 samp_freq, si8 data_len, TERN_m11 alloc_orig_data, TERN_m11 alloc_filt_data, TERN_m11 alloc_buffer, ui4 behavior_on_fail, sf8 cutoff_1, ...);
void    FILT_generate_initial_conditions_d11(FILT_PROCESSING_STRUCT_d11 *filtps);
void    FILT_hqr_d11(sf16 **a, si4 poles, FILT_LONG_COMPLEX_d11 *eigs);
void    FILT_invert_matrix_d11(sf16 **a, sf16 **inv_a, si4 order);
void    FILT_mat_multl_d11(void *a, void *b, void *product, si4 outer_dim1, si4 inner_dim, si4 outer_dim2);
sf8    	*FILT_noise_floor_filter_d11(sf8 *data, sf8 *filt_data, si8 data_len, sf8 rel_thresh, sf8 abs_thresh, CMP_BUFFERS_m11 *nff_buffers);
sf8	*FILT_quantfilt_d11(sf8 *x, sf8 *qx, si8 len, sf8 quantile, si8 span, si1 tail_option_code);
ui1     FILT_remove_line_noise_d11(si4 *data, si8 n_samps, sf8 samp_freq, sf8 line_freq, si4 cycles_in_template, ui1 thresh_score, si1 calc_score_flag, si1 remove_noise_flag, si4 n_harmonics);
si4     FILT_sf8_sort_d11(const void *n1, const void *n2);
si4     FILT_sort_by_idx_d11(const void *n1, const void *n2);
si4     FILT_sort_by_val_d11(const void *n1, const void *n2);
void    FILT_unsymmeig_d11(sf16 **a, si4 poles, FILT_LONG_COMPLEX_d11 *eigs);



//**********************************************************************************//
//*********************************  TRANSMISSION  *********************************//
//**********************************************************************************//


// Transmission Header (TH)
#define PORT_STRLEN_d11			8
#define TH_HEADER_BYTES_d11		((si8) 16)

// Transmission Header Types
// • Type numbers 0-63 reserved for generic transmission types
// • Type numbers 64-255 used for application specific transmission types

// Generic Transmission Header (TH) Types
#define TH_TYPE_NO_ENTRY_d11					((ui1) 0)
#define TH_TYPE_MESSAGE_d11					((ui1) 1)
#define TH_TYPE_OPERATION_SUCCEEDED_d11				((ui1) 2)
#define TH_TYPE_OPERATION_SUCCEEDED_WITH_MESSAGE_d11		((ui1) 3)
#define TH_TYPE_OPERATION_FAILED_d11				((ui1) 4)
#define TH_TYPE_OPERATION_FAILED_WITH_WARNING_MESSAGE_d11	((ui1) 5)
#define TH_TYPE_OPERATION_FAILED_WITH_ERROR_MESSAGE_d11		((ui1) 6)

// Transmission Error Codes
#define TH_ERR_UNSPECIFIED_d11					(si8) FALSE_m11
#define TH_ERR_SOCK_FAILED_d11					(si8) -1
#define TH_ERR_SOCK_FAILED_TO_OPEN_d11				(si8) -2
#define TH_ERR_SOCK_CLOSED_d11					(si8) -3
#define TH_ERR_ID_MISMATCH_d11					(si8) -4

// Header Message Type Aliases (shorter :)
#define TH_ERROR_TYPE_d11	TH_TYPE_OPERATION_FAILED_WITH_ERROR_MESSAGE_d11
#define TH_WARNING_TYPE_d11	TH_TYPE_OPERATION_FAILED_WITH_WARNING_MESSAGE_d11
#define TH_SUCCESS_TYPE_d11	TH_TYPE_OPERATION_SUCCEEDED_WITH_MESSAGE_d11
#define TH_MESSAGE_TYPE_d11	TH_TYPE_MESSAGE_d11

// Transmission Flags
#define TH_FLAGS_ENDIAN_MASK_d11		((ui1) 1)       // Bit 0  (BIG_ENDIAN_m11 == 0, LITTLE_ENDIAN_m11 == 1)
#define TH_FLAGS_ENCRYPTED_MASK_d11		((ui1) 2)       // Bit 1  (body, header is not encrypted)
#define TH_FLAGS_CLOSE_CONNECTION_MASK_d11	((ui1) 4)	// Bit 2  (close socket after send/recv)

// TH Flag Defaults
#define TH_FLAGS_ENDIANNESS_DEFAULT_d11			((ui1) LITTLE_ENDIAN_m11)
#define TH_FLAGS_ENCRYPTED_DEFAULT_d11			((ui1) 0)  // don't encrypt / not encrypted => reset by send/recv transmission
#define TH_FLAGS_CLOSE_CONNECTION_DEFAULT_d11		((ui1) 0)  // don't close socket => reset by send/recv transmission

// TH Defaults
#define TH_VERSION_DEFAULT_d11		((ui1) 1)
#define TH_TYPE_DEFAULT_d11		TH_TYPE_NO_ENTRY_d11
#define TH_FLAGS_DEFAULT		(TH_FLAGS_ENDIANNESS_DEFAULT_d11 | TH_FLAGS_ENCRYPTED_DEFAULT_d11 | TH_FLAGS_CLOSE_CONNECTION_DEFAULT_d11)
#define TH_ID_CODE_NO_ENTRY_d11		0  // ui4
#define TH_ID_CODE_DEFAULT		TH_ID_CODE_NO_ENTRY_d11

// Transmission Message
#define MESSAGE_HEADER_BYTES_d11			16
#define MESSAGE_HEADER_TIME_OFFSET_d11			0	// si8
#define MESSAGE_HEADER_NO_ENTRY_d11			UUTC_NO_ENTRY_m11
#define MESSAGE_HEADER_MESSAGE_BYTES_OFFSET_d11		8	// si8
#define MESSAGE_HEADER_MESSAGE_BYTES_NO_ENTRY_d11	0

// Transmission Header (TH) Format Constants
#define TH_BYTES_d11					16
#define TH_ID_STRING_OFFSET_d11				0	                	// ascii[4]
#define TH_ID_STRING_TERMINAL_ZERO_OFFSET_d11		(TH_ID_STRING_OFFSET_d11 + 4)	// si1
#define TH_ID_CODE_OFFSET_d11				TH_ID_STRING_OFFSET_d11		// ui4
// TH_ID_CODE_NO_ENTRY_d11 defined above
#define TH_TYPE_OFFSET_d11				5	                	// ui1
#define TH_VERSION_OFFSET_d11				6	                	// ui1
#define TH_VERSION_NO_ENTRY_d11				0
#define TH_FLAGS_OFFSET_d11				7	                	// ui1
#define TH_FLAGS_NO_ENTRY_d11				0xFF
#define TH_TRANSMISSION_BYTES_OFFSET_d11		8				// si8
#define TH_TRANSMISSION_BYTES_NO_ENTRY_d11		0

// Miscellaneous
#define TH_SOCK_TIMEOUT_NEVER_d11			0
#define TH_SOCK_TIMEOUT_USE_EXISTING_d11		-1  // alloc_trans_info() will leave existing value


// Typedefs
typedef struct {
	union {
		struct {
			si1     ID_string[TYPE_BYTES_m11];  // transmission ID is typically application specific
			ui1     type;  // transmission type (transmission ID specific)
			ui1     version;  // transmission header version
			ui1     flags;
		};
		struct {
			ui4     ID_code;  // transmission ID is typically application specific
			si1	ID_string_terminal_zero;
		};
	};
	si8	transmission_bytes;  // includes header & body
} TRANSMISSION_HEADER_d11;

typedef struct {
	ui1			*buffer;  // first portion is the transmission header
	si8			buffer_bytes;
	TRANSMISSION_HEADER_d11	*header;
	si4			sock_fd;
	si1			addr_str[INET6_ADDRSTRLEN];
	si1			port_str[PORT_STRLEN_d11];
	si4			timeout_seconds;
} TRANSMISSION_INFO_d11;

typedef struct {
	si8	time;		// uutc
	si8	message_bytes;	// includes text, & pad bytes, NOT header bytes
} MESSAGE_HEADER_d11;		// text follows structure, padded with zeros to 16 byte alignment


// Prototypes
TRANSMISSION_INFO_d11	*alloc_trans_info_d11(si8 buffer_bytes, TRANSMISSION_INFO_d11 *trans_info, si1 *addr_str, si1 *port_str, ui4 ID_code, si4 timeout_seconds);
TERN_m11		check_transmission_header_alignment_d11(ui1 *bytes);
void			close_transmission_d11(TRANSMISSION_INFO_d11 *trans_info);
TERN_m11		connect_to_server_d11(TRANSMISSION_INFO_d11 *trans_info, si1 *addr_str, si1 *port_str, ui4 ID_code);
void			free_transmission_info_d11(TRANSMISSION_INFO_d11 **trans_info_ptr, TERN_m11 free_structure);
si8			recv_transmission_d11(TRANSMISSION_INFO_d11 *trans_info);
TERN_m11		send_message_d11(TRANSMISSION_INFO_d11 *trans_info, ui1 type, TERN_m11 encrypt, si1 *fmt, ...);
si8			send_transmission_d11(TRANSMISSION_INFO_d11 *trans_info);
TERN_m11		show_message_d11(TRANSMISSION_HEADER_d11 *header);
void			show_transmission_info_d11(TRANSMISSION_INFO_d11 *trans_info);


//**********************************************************************************//
//*********************************  GENERAL DHN  **********************************//
//**********************************************************************************//


// Miscellaneous Constants
#define SK_MATRIX_ENTRIES_d11	256
#define SK_MATRIX_d11 { \
	0x44, 0x61, 0x72, 0x6b,    0x20, 0x48, 0x6f, 0x72,    0x73, 0x65, 0x20, 0x4e,    0x65, 0x75, 0x72, 0x6f,  \
	0xd8, 0x21, 0xda, 0x26,    0xf8, 0x69, 0xb5, 0x54,    0x8b, 0x0c, 0x95, 0x1a,    0xee, 0x79, 0xe7, 0x75,  \
	0x6c, 0xb5, 0x47, 0x0e,    0x94, 0xdc, 0xf2, 0x5a,    0x1f, 0xd0, 0x67, 0x40,    0xf1, 0xa9, 0x80, 0x35,  \
	0xbb, 0x78, 0xd1, 0xaf,    0x2f, 0xa4, 0x23, 0xf5,    0x30, 0x74, 0x44, 0xb5,    0xc1, 0xdd, 0xc4, 0x80,  \
														  \
	0x72, 0x64, 0x1c, 0xd7,    0x5d, 0xc0, 0x3f, 0x22,    0x6d, 0xb4, 0x7b, 0x97,    0xac, 0x69, 0xbf, 0x17,  \
	0x9b, 0x6c, 0xec, 0x46,    0xc6, 0xac, 0xd3, 0x64,    0xab, 0x18, 0xa8, 0xf3,    0x07, 0x71, 0x17, 0xe4,  \
	0x18, 0x9c, 0x85, 0x83,    0xde, 0x30, 0x56, 0xe7,    0x75, 0x28, 0xfe, 0x14,    0x72, 0x59, 0xe9, 0xf0,  \
	0x93, 0x82, 0x09, 0xc3,    0x4d, 0xb2, 0x5f, 0x24,    0x38, 0x9a, 0xa1, 0x30,    0x4a, 0xc3, 0x48, 0xc0,  \
														  \
	0x3d, 0xd0, 0xb3, 0x15,    0x70, 0x62, 0xec, 0x31,    0x48, 0xf8, 0x4d, 0x01,    0x02, 0x3b, 0x05, 0xc1,  \
	0xc4, 0xbb, 0xcb, 0x62,    0xb4, 0xd9, 0x27, 0x53,    0xfc, 0x21, 0x6a, 0x52,    0xfe, 0x1a, 0x6f, 0x93,  \
	0x50, 0x13, 0x17, 0xd9,    0xe4, 0xca, 0x30, 0x8a,    0x18, 0xeb, 0x5a, 0xd8,    0xe6, 0xf1, 0x35, 0x4b,  \
	0x48, 0x20, 0x6b, 0x7b,    0x5e, 0x23, 0x71, 0x78,    0x60, 0x68, 0x67, 0x45,    0x5f, 0x30, 0x63, 0x77,  \
														  \
	0x4a, 0x1b, 0xfd, 0xc0,    0x19, 0x19, 0x98, 0xa2,    0xce, 0x9c, 0xeb, 0xb0,    0x2a, 0x69, 0x56, 0x31,  \
	0x36, 0x84, 0xc2, 0x9a,    0x83, 0x66, 0x40, 0x65,    0xbb, 0x0b, 0x50, 0xd0,    0xdf, 0x35, 0x84, 0x29,  \
	0x50, 0x81, 0xe9, 0x69,    0x9b, 0x81, 0x0b, 0x69,    0x1d, 0xf6, 0x19, 0x47,    0x60, 0x6f, 0x79, 0x96,  \
	0xf3, 0x3b, 0x30, 0x76,    0xa2, 0x70, 0xdb, 0x5d,    0x7b, 0x2c, 0x2d, 0x5a,    0x61, 0xb1, 0x84, 0xb1 }

#define TEXTBELT_KEY_d11				"13b559d431ab2dda7b18f7dc8dcc18676e5c6fdcPfcYHcR7r9YZZaBRVi62VGn5W"
#define SENDGRID_KEY_d11				"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"
#define THREAD_NAME_BYTES_d11				64
#define GLOBALS_VERBOSE_DEFAULT_d11			FALSE_m11
#if defined MACOS_m11 || defined LINUX_m11
#define GLOBALS_FILE_CREATION_UMASK_DEFAULT_d11		S_IWOTH  // removes write permission for "other" (defined in <sys/stat.h>)
#endif
#ifdef WINDOWS_m11
#define GLOBALS_FILE_CREATION_UMASK_DEFAULT_d11		0  // full permissions for everyone (Windows does not support "other" category)
#endif
#define DHN_SOFTWARE_PATH_DEFAULT_d11			"DHN"  // actually "~/DHN"

#define GLOBALS_LS_MACHINE_CODE_DEFAULT_d11		0
#define GLOBALS_LS_CUSTOMER_CODE_DEFAULT_d11		0

// Alloc Tracking Constants
#define GLOBALS_ALLOC_TRACKING_DEFAULT_d11		FALSE_m11
#define GLOBALS_INIT_ALLOC_TRACKING_ARRAY_LEN_d11	100
#define GLOBALS_ALLOC_TRACKING_FUNCTION_STRING_LEN_d11	42
#define AE_SHOW_ALL_d11					0
#define AE_SHOW_FREED_ONLY_d11				1
#define AE_SHOW_ALLOCATED_ONLY_d11			2

// Resource File Constants
#define RC_NO_ENTRY_d11    	-1
#define RC_NO_OPTION_d11   	0
#define RC_STRING_TYPE_d11      1
#define RC_FLOAT_TYPE_d11       2
#define RC_INTEGER_TYPE_d11     3
#define RC_TERNARY_TYPE_d11     4

// Thread Management Constants
#define DEFAULT_PRIORITY_d11    0x7FFFFFFF
#define MIN_PRIORITY_d11        0x7FFFFFFE
#define LOW_PRIORITY_d11        0x7FFFFFFD
#define MEDIUM_PRIORITY_d11     0x7FFFFFFC
#define HIGH_PRIORITY_d11       0x7FFFFFFB
#define MAX_PRIORITY_d11        0x7FFFFFFA
#define UNDEFINED_PRIORITY_d11  0x7FFFFFF9

// Network Constants
#define MAC_ADDRESS_BYTES_d11		6
#define MAC_ADDRESS_STR_BYTES_d11	(MAC_ADDRESS_BYTES_d11 * 3)  // 6 hex bytes plus colons & terminal zero
#define IPV4_ADDRESS_BYTES_d11		4
#define IPV4_ADDRESS_STR_BYTES_d11	(IPV4_ADDRESS_BYTES_d11 * 4)  // 4 de bytes plus periods & terminal zero


// Typedefs & Structs
#if defined MACOS_m11 || defined LINUX_m11
	#ifdef MACOS_m11
	typedef	ui4			cpu_set_t_d11;  // max 32 logical cores
	#else
	typedef	cpu_set_t		cpu_set_t_d11;  // unknown logical cores
	#endif
	typedef	pthread_t		pthread_t_d11;
	typedef pthread_attr_t		pthread_attr_t_d11;
	typedef void *			pthread_rval_d11;
	typedef pthread_rval_d11 	(*pthread_fn_d11)(void *);
	typedef	pthread_mutex_t		pthread_mutex_t_d11;
	typedef	pthread_mutexattr_t	pthread_mutexattr_t_d11;
#endif

#ifdef WINDOWS_m11
	typedef	ui8			cpu_set_t_d11;  // max 64 logical cores (defined as DWORD_PTR in Windows, but not used as a pointer; used as ui8)
	typedef	HANDLE			pthread_t_d11;
	typedef void *			pthread_attr_t_d11;
	typedef ui4 			pthread_rval_d11;
	typedef pthread_rval_d11 	(*pthread_fn_d11)(void *);
	typedef	HANDLE			pthread_mutex_t_d11;
	typedef	SECURITY_ATTRIBUTES	pthread_mutexattr_t_d11;
#endif

typedef struct {
	si4		physical_cores;
	si4		logical_cores;
	TERN_m11	hyperthreading;
	sf8		minimum_speed;
	sf8		maximum_speed;
	sf8		current_speed;
	ui1		endianness;
	si1		manufacturer[64];
	si1		model[64];
} CPU_INFO_d11;

typedef struct {
	si1		host_name[256];  // max 253 ascii characters
	ui1             MAC_address_bytes[MAC_ADDRESS_BYTES_d11];
	si1             MAC_address_string_LC[MAC_ADDRESS_STR_BYTES_d11]; // lower case hex with colons
	si1             MAC_address_string_UC[MAC_ADDRESS_STR_BYTES_d11]; // upper case hex with colons
	ui1             LAN_IPv4_address_bytes[IPV4_ADDRESS_BYTES_d11];
	si1             LAN_IPv4_address_string[IPV4_ADDRESS_STR_BYTES_d11];  // dec with periods
	ui1             LAN_IPv4_subnet_mask_bytes[IPV4_ADDRESS_BYTES_d11];
	si1             LAN_IPv4_subnet_mask_string[IPV4_ADDRESS_STR_BYTES_d11];  // dec with periods
	ui1             WAN_IPv4_address_bytes[IPV4_ADDRESS_BYTES_d11];
	si1             WAN_IPv4_address_string[IPV4_ADDRESS_STR_BYTES_d11];  // dec with periods
	si4             MTU;  // maximum transmission unit
	si1             link_speed[16];
	si1             duplex[16];
	TERN_m11        active;  // interface status
	TERN_m11        plugged_in;
} NETWORK_PARAMETERS_d11;

typedef struct {
	void		*ptr;
	ui8		n_bytes;
	si4		line;
	si1		function[GLOBALS_ALLOC_TRACKING_FUNCTION_STRING_LEN_d11];
	ui1		zero_term;
	TERN_m11	freed;
} ALLOC_ENTITY_d11;

typedef struct {
	ui4			file_creation_umask;
	CPU_INFO_d11		cpu_info;
	si1			machine_serial[56];  // maximum serial number length is 50 characters
	ui4			LS_machine_code;
	ui4			LS_customer_code;
	// Alignment
	TERN_m11		all_structures_aligned;
	TERN_m11		license_file_entry_aligned;
	TERN_m11		transmission_header_aligned;
	// CMP
	sf8			*CMP_normal_CDF_table;
	// Alloc Tracking
	TERN_m11		alloc_tracking;
	si8			ae_n_entities, ae_array_len, ae_curr_allocated_entities;
	ui8			ae_max_allocated_bytes, ae_curr_allocated_bytes;
	ALLOC_ENTITY_d11	*alloc_entities;
	// Miscellaneous
	TERN_m11			verbose;
	ui1				*sk_matrix;
	VDS_THRESHOLD_MAP_ENTRY_d11	*CMP_VDS_threshold_map;
} GLOBALS_d11;

typedef struct {
	pthread_t_d11		thread_id;
	si1			MED_dir[FULL_FILE_NAME_BYTES_m11];
	ui4			flags;
	LEVEL_HEADER_m11	*MED_struct;  // CHANNEL_m11 or SEGMENT_m11 pointer
	LEVEL_HEADER_m11	*returned_MED_struct;  // CHANNEL_m11 or SEGMENT_m11 pointer
	TIME_SLICE_m11		*slice;
	si1			*password;
} READ_MED_THREAD_INFO_d11;


// Prototypes
void            add_alloc_entity_d11(void *ptr, ui8 n_bytes, const si1 *function, si4 line);
void		build_message_d11(MESSAGE_HEADER_d11 *msg, si1 *message_text);
void		byte_to_hex_d11(ui1 byte, si1 *hex);
TERN_m11        check_all_alignments_d11(const si1 *function, si4 line);
TERN_m11	check_file_system_d11(si1 *file_system_path, si4 is_cloud, ...);  // varargs: si1 *cloud_directory, si1 *cloud_service_name, si1 *cloud_utilities_directory
TERN_m11	check_internet_connection_d11(void);
ui4             check_spaces_d11(si1 *string);
si4		compare_sf8_d11(const void *a, const void * b);
si4		compare_si4_d11(const void *a, const void * b);
si4     	compare_si8_d11(const void *a, const void * b);
TERN_m11	domain_to_ip_d11(si1 *domain_name, si1 *ip);
si1		*duration_string_d11(si1 *dur_str, si8 i_usecs);
TERN_m11	enter_ascii_password_d11(si1 *password, si1 *prompt, TERN_m11 create_password);
void            fill_empty_password_bytes_d11(si1 *password_bytes);
void    	free_globals_d11(TERN_m11 cleanup_for_exit);
cpu_set_t_d11	*generate_cpu_set_d11(si1 *affinity_str, cpu_set_t_d11 *cpu_set_p);
void		get_cpu_info_d11(void);
si1		*get_DHN_license_path_d11(si1 *path);
void		*get_in_addr_d11(struct sockaddr *sa);
NETWORK_PARAMETERS_d11 *get_lan_ipv4_address_d11(NETWORK_PARAMETERS_d11 *np);
ui4		get_machine_code_d11(void);
NETWORK_PARAMETERS_d11 *get_network_parameters_d11(si1 *interface_name, NETWORK_PARAMETERS_d11 *np);
TERN_m11	get_terminal_entry_d11(si1 *prompt, si1 type, void *buffer, void *default_input, TERN_m11 required, TERN_m11 validate);
NETWORK_PARAMETERS_d11 *get_wan_ipv4_address_d11(NETWORK_PARAMETERS_d11 *np);
TERN_m11	hex_to_int_d11(ui1 *in, ui1 *out, si4 len);
TERN_m11	increase_process_priority_d11(TERN_m11 verbose_flag);
void    	initialize_alloc_tracking_d11(void);
TERN_m11	initialize_dhnlib_d11(TERN_m11 check_structure_alignments, TERN_m11 initialize_all_tables);
void		initialize_globals_d11(void);
void		initialize_sk_matrix_d11(void);
void    	lad_reg_sf8_d11(sf8 *y, si8 n, sf8 *b, sf8 *m);
void    	lad_reg_2_si4_d11(si4 *x_input_buffer, si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
ui4		launch_thread_d11(pthread_t_d11 *thread_id, pthread_fn_d11 thread_f, void *arg, si4 priority, si1 *affinity_str, cpu_set_t_d11 *cpu_set_p, TERN_m11 detached, si1 *thread_name);
void    	lin_reg_2_d11(si4 *x_input_buffer, si4 *y_input_buffer, si8 len, sf8 *m, sf8 *b);
void            nap_d11(si1 *nap_str);
si4		pthread_join_d11(pthread_t_d11 thread_id, void **value_ptr);
si4		pthread_mutex_destroy_d11(pthread_mutex_t_d11 *mutex);
si4		pthread_mutex_init_d11(pthread_mutex_t_d11 *mutex, pthread_mutexattr_t_d11 *attr);
si4		pthread_mutex_lock_d11(pthread_mutex_t_d11 *mutex);
si4		pthread_mutex_unlock_d11(pthread_mutex_t_d11 *mutex);
ui1             random_byte_d11(ui4 *m_w, ui4 *m_z);
si1     	*re_escape_d11(si1 *str, si1 *esc_str);
CHANNEL_m11	*read_channel_d11(CHANNEL_m11 *chan, TIME_SLICE_m11 *slice, ...);  // varargs: si1 *chan_path, ui4 flags, si1 *password
pthread_rval_d11	read_channel_thread_d11(void *ptr);
si4             read_rc_field_d11(si1 *field_name, si1 **buffer, TERN_m11 update_buffer_ptr, si1 *field_value_str, sf8 *float_val, si8 *int_val, TERN_m11 *TERN_val);
pthread_rval_d11	read_segment_thread_d11(void *ptr);
SESSION_m11	*read_session_d11(SESSION_m11 *sess, TIME_SLICE_m11 *slice, ...);  // varargs: void *file_list, si4 list_len, ui4 flags, si1 *password
TERN_m11	recover_passwords_d11(UNIVERSAL_HEADER_m11 *universal_header);
void    	rectify_d11(si4 *input_buffer, si4 *output_buffer, si8 len);
si4             remove_alloc_entity_d11(void *ptr, const si1 *function, si4 line);
void		sendgrid_email_d11(si1 *sendgrid_key, si1 *to_email, si1 *cc_email, si1 *to_name, si1 *subject, si1 *content, si1 *from_email, si1 *from_name, si1 *reply_to_email, si1 *reply_to_name);
void		set_L3_pw_d11(si1 *level_3_password);
TERN_m11	set_thread_affinity_d11(pthread_t_d11 *thread_id_p, pthread_attr_t_d11 *attributes, cpu_set_t_d11 *cpu_set_p, TERN_m11 wait_for_lauch);
void    	show_alloc_entities_d11(ui1 mode);
si4             show_alloc_entity_d11(void *ptr);
void		show_cpu_info_d11(void);
void    	show_globals_d11(void);
void            show_network_parameters_d11(NETWORK_PARAMETERS_d11 *np);
void		show_rec_Sgmt_type_d11(TRANSMISSION_HEADER_d11 *header);
void            show_thread_affinity_d11(pthread_t_d11 *thread_id);
si1		*size_string_d11(si1 *size_str, si8 n_bytes);
void    	textbelt_text_d11(si1 *phone_number, si1 *content, si1 *textbelt_key);
void		trim_addr_str_d11(si1 *addr_str);



//**********************************************************************************//
//*******************************  DATA MATRIX (DM) ********************************//
//**********************************************************************************//

// Flag Definitions:
#define DM_NO_FLAGS_d11				((ui8) 0)
#define DM_TYPE_SI2_d11				((ui8) 1 << 1)
#define DM_TYPE_SI4_d11				((ui8) 1 << 2)
#define DM_TYPE_SF4_d11				((ui8) 1 << 3)
#define DM_TYPE_SF8_d11				((ui8) 1 << 4)
#define DM_TYPE_MASK_d11	              (	DM_TYPE_SI2_d11 | DM_TYPE_SI4_d11 | DM_TYPE_SF4_d11 | DM_TYPE_SF8_d11 )
#define DM_2D_INDEXING_d11			((ui8) 1 << 7)		// include array of pointers so that matrix[x][y] indexing is possible (expensive with large major dinensions)
#define DM_FMT_SAMPLE_MAJOR_d11			((ui8) 1 << 8)
#define DM_FMT_CHANNEL_MAJOR_d11		((ui8) 1 << 9)
#define DM_FMT_MASK_d11	   	              (	DM_FMT_SAMPLE_MAJOR_d11 | DM_FMT_CHANNEL_MAJOR_d11 )
#define DM_EXTMD_SAMP_FREQ_d11			((ui8) 1 << 12)
#define DM_EXTMD_SAMP_COUNT_d11			((ui8) 1 << 13)
#define DM_EXTMD_COUNT_AND_FREQ_d11            	((ui8) 1 << 14)
#define DM_EXTMD_MASK_d11	              (	DM_EXTMD_SAMP_FREQ_d11 | DM_EXTMD_SAMP_COUNT_d11 | DM_EXTMD_COUNT_AND_FREQ_d11 )
#define DM_FILT_LOWPASS_d11			((ui8) 1 << 16)		// low cutoff == vararg 1
#define DM_FILT_HIGHPASS_d11			((ui8) 1 << 17)		// high cutoff == vararg 1
#define DM_FILT_BANDPASS_d11			((ui8) 1 << 18)		// low cutoff == vararg 1, high cutoff == vararg 2
#define DM_FILT_BANDSTOP_d11			((ui8) 1 << 19)		// low cutoff == vararg 1, high cutoff == vararg 2
#define DM_FILT_CUTOFFS_MASK_d11      	      ( DM_FILT_LOWPASS_d11 | DM_FILT_HIGHPASS_d11 | DM_FILT_BANDPASS_d11 | DM_FILT_BANDSTOP_d11 )
#define DM_FILT_ANTIALIAS_d11			((ui8) 1 << 20)		// lowpass with high cutoff computed, no varargs
#define DM_FILT_MASK_d11	      	      ( DM_FILT_CUTOFFS_MASK_d11 | DM_FILT_ANTIALIAS_d11 )
#define DM_INTRP_SPLINE_d11			((ui8) 1 << 24)
#define DM_INTRP_LINEAR_d11			((ui8) 1 << 25)
#define DM_INTRP_UP_SPLINE_DN_LINEAR_d11	((ui8) 1 << 26)		// if sampling frequency ratio >= DM_INTRP_SPLINE_UPSAMPLE_SF_RATIO_d11
#define DM_INTRP_SPLINE_UPSAMPLE_SF_RATIO_d11	CMP_SPLINE_UPSAMPLE_SF_RATIO_m11	// require (out_sf / in_sf) be >= this before spline upsampling, if lower use linear (prevents unnatural spline turns)
#define DM_INTRP_BINTRP_MDPT_d11		((ui8) 1 << 27)		// binterpolate with midpoint center mode
#define DM_INTRP_BINTRP_MEAN_d11		((ui8) 1 << 28)		// binterpolate with mean center mode
#define DM_INTRP_BINTRP_MEDN_d11		((ui8) 1 << 29)		// binterpolate with median center mode
#define DM_INTRP_BINTRP_FAST_d11		((ui8) 1 << 30)		// binterpolate with fast center mode
#define DM_INTRP_BINTRP_MASK_d1		      ( DM_INTRP_BINTRP_MDPT_d11 | DM_INTRP_BINTRP_MEAN_d11 | DM_INTRP_BINTRP_MEDN_d11 | DM_INTRP_BINTRP_FAST_d11 )
#define DM_INTRP_MASK_d11	              (	DM_INTRP_SPLINE_d11 | DM_INTRP_LINEAR_d11 | DM_INTRP_UP_SPLINE_DN_LINEAR_d11 | DM_INTRP_BINTRP_MASK_d1 )
#define DM_TRACE_RANGES_d11			((ui8) 1 << 32)		// return bin minima & maxima (equal in size, type, & format to data matrix)
#define DM_TRACE_EXTREMA_d11			((ui8) 1 << 33)		// return minima & maxima values also (minimum & maximum per channel, same type as data matrix)
#define DM_DETREND_d11				((ui8) 1 << 34)		// detrend traces (and trace range matrices if DM_TRACE_RANGES_d11 is set)
#define DM_DSCNT_CONTIG_d11			((ui8) 1 << 40)		// return samples as contiguous (locations specified in returned arrays) => matrix may be truncated
									// (DM_DSCNT_CONTIG_d11 is assumed if DM_EXTMD_COUNT_AND_FREQ_d11 is set)
#define DM_DSCNT_NAN_d11			((ui8) 1 << 41)		// fill absent samples with NaNs (locations specified in returned arrays)
									// si2: NAN_SI2_m11 (0x8000)
									// si4: NAN_SI4_m11 (0x80000000)
									// sf4: NAN == nanf("")
									// sf8: NAN == nan("")
#define DM_DSCNT_ZERO_d11			((ui8) 1 << 42)		// fill absent samples with zeros (locations specified in returned arrays)
#define DM_DSCNT_MASK_d11	              (	DM_DSCNT_CONTIG_d11 | DM_DSCNT_NAN_d11 | DM_DSCNT_ZERO_d11 )


// Note: if arrays are allocted as 2D arrays, array[0] is beginning of one dimensional array containing (channel_count * sample_count) values of specfified type
typedef struct {
	si8		channel_count;		// defines dimension of allocated matrix: updated based on active channels
	si8		sample_count;		// defines dimension of allocated matrix: if extent mode (EXTMD) == DM_EXTMD_SAMP_FREQ_m11, resultant sample count & times filled in
	sf8		sampling_frequency;	// defines dimension of allocated matrix: if extent mode (EXTMD) == DM_EXTMD_SAMP_COUNT_m11, resultant sampling frequency & times filled in
	sf8		filter_low_fc;		// optionally passed (can be passed in varargs), always returned
	sf8		filter_high_fc;		// optionally passed (can be passed in varargs), always returned
	void		*data;			// alloced / realloced as needed   (cast to type * or type **, if DM_2D_INDEXING_d11 is set)
	void		*range_minima;		// alloced / realloced as needed, present if DM_TRACE_RANGES_d11 bit is set, otherwise NULL   (cast to type * or type **, if DM_2D_INDEXING_d11 is set)
	void		*range_maxima;		// alloced / realloced as needed, present if DM_TRACE_RANGES_d11 bit is set, otherwise NULL   (cast to type * or type **, if DM_2D_INDEXING_d11 is set)
	void		*trace_minima;  	// alloced / realloced as needed, present if DM_TRACE_EXTREMA_d11 bit is set, otherwise NULL   (cast to type *)
	void		*trace_maxima;  	// alloced / realloced as needed, present if DM_TRACE_EXTREMA_d11 bit is set, otherwise NULL   (cast to type *)
	si4		number_of_contigua;
	CONTIGUON_m11	*contigua;		// sample indexes in matrix frame
	// internal processing elements //
	ui8			flags;
	si8			maj_dim;
	si8			min_dim;
	si8			el_size;
	si8			data_bytes;
	si8			n_proc_bufs;
	CMP_BUFFERS_m11		**in_bufs;
	CMP_BUFFERS_m11		**out_bufs;
	CMP_BUFFERS_m11		**spline_bufs;
} DATA_MATRIX_d11;

typedef struct {
	pthread_t_d11	thread_id;
	DATA_MATRIX_d11	*dm;
	CHANNEL_m11	*chan;
	si8		chan_idx;
} DM_GET_MATRIX_THREAD_INFO_d11;


// Prototypes
void			DM_free_matrix_d11(DATA_MATRIX_d11 *matrix, TERN_m11 free_structure);
DATA_MATRIX_d11 	*DM_get_matrix_d11(DATA_MATRIX_d11 *matrix, SESSION_m11 *sess, TIME_SLICE_m11 *slice, ...);
// DM_get_matrix_d11() varargs: sf8 fc1, sf8 fc2, ui8 flags, si8 sample_count, sf8 sampling_frequency
// varargs DM_FILT_LOWPASS_d11 set: fc1 == high_cutoff
// varargs DM_FILT_HIGHPASS_d11 set: fc1 == low_cutoff
// varargs DM_FILT_BANDPASS_d11 set: fc1 == low_cutoff, fc2 == high_cutoff
// varargs DM_FILT_BANDSTOP_d11 set: fc1 == low_cutoff, fc2 == high_cutoff
// varargs slice == NULL: flags, sample_count, sampling_frequency are passed (fc1 & fc2 must be filled in, but if not used, 0.0 should be passed a place holder)
pthread_rval_d11	DM_gm_thread_f_d11(void *ptr);
DATA_MATRIX_d11		*DM_transpose_d11(DATA_MATRIX_d11 *in_matrix, DATA_MATRIX_d11 *out_matrix);  // if in_matrix == out_matrix, done in place; if out_matrix == NULL, allocated and returned
void			DM_transpose_arrays_d11(DATA_MATRIX_d11 *in_matrix, DATA_MATRIX_d11 *out_matrix, void *in_base, void *out_base);  // used by DM_transpose_d11(), assumes array allocation is taken care of, so use independently with care



//**********************************************************************************//
//**********************  LICENSE SERVER (LS; LSc - client)  ***********************//
//**********************************************************************************//

// License Server (LS) Transmission Header (TH) Types
// • Type numbers 0-63 reserved for generic transmission types
// • Type numbers 64-255 used for application specific transmission types
#define LS_TH_TYPE_ISSUE_LICENSE_d11			((ui1) 64)  // header => aux_lic_data => lic_file_entry [+/- unspec_hash (encryp if present)]
#define LS_TH_TYPE_LICENSE_ISSUED_d11			((ui1) 65)  // header => lic_file_entry
#define LS_TH_TYPE_RELEASE_LICENSE_d11			((ui1) 66)  // header => aux_db_data => unspecified_hash (encryp)
#define LS_TH_TYPE_LICENSE_RELEASED_d11			((ui1) 67)  // header
#define LS_TH_TYPE_ADD_CUSTOMER_d11			((ui1) 68)  // header => aux_lic_data => customer_info => machine_info => stand_hash => ...
								    // ... stand_hint => mast_hash => mast_hint (encryp)
#define LS_TH_TYPE_CUSTOMER_ADDED_d11			((ui1) 69)  // header => aux_lic_data
#define LS_TH_TYPE_SEND_PASSWORD_d11			((ui1) 70)  // header => message (for password prompt)
#define LS_TH_TYPE_PASSWORD_SENT_d11			((ui1) 71)  // header => unspec_hash (encryp)
#define LS_TH_TYPE_CHECK_PASSWORD_d11			((ui1) 72)  // header => aux_lic_data => unspecified_hash (encryp)
#define LS_TH_TYPE_PASSWORD_CHECKED_d11			((ui1) 73)  // header => misc_serv_data
#define LS_TH_TYPE_CHANGE_PASSWORDS_d11			((ui1) 74)  // header => aux_lic_data => curr_mast_hash => new_stand_hash => ...
								    // ... new_stand_hint => new_mast_hash => new_mast_hint (encryp)
#define LS_TH_TYPE_PASSWORDS_CHANGED_d11		((ui1) 75)  // header
#define LS_TH_TYPE_SEND_MACHINE_INFO_d11		((ui1) 76)  // header
#define LS_TH_TYPE_MACHINE_INFO_SENT_d11		((ui1) 77)  // header => machine_info => unspec_hash (encryp)
#define LS_TH_TYPE_BLOCK_MACHINE_d11			((ui1) 78)  // header => aux_lic_data => mast_hash (encryp)
#define LS_TH_TYPE_MACHINE_BLOCKED_d11			((ui1) 79)  // header
#define LS_TH_TYPE_UNBLOCK_MACHINE_d11			((ui1) 80)  // header => aux_lic_data => mast_hash (encryp)
#define LS_TH_TYPE_MACHINE_UNBLOCKED_d11		((ui1) 81)  // header
#define LS_TH_TYPE_CHANGE_DEFAULT_TIMEOUT_INC_d11	((ui1) 82)  // header => aux_lic_data => misc_serv_data => mast_hash (encryp)
#define LS_TH_TYPE_DEFAULT_TIMEOUT_INC_CHANGED_d11	((ui1) 83)  // header
#define LS_TH_TYPE_CHANGE_MACHINE_TIMEOUT_INC_d11	((ui1) 84)  // header => aux_lic_data => misc_serv_data => mast_hash (encryp)
#define LS_TH_TYPE_MACHINE_TIMEOUT_INC_CHANGED_d11	((ui1) 85)  // header
#define LS_TH_TYPE_SEND_LIST_d11			((ui1) 86)  // header => aux_lic_data => misc_serv_data => mast_hash (encryp)
#define LS_TH_TYPE_LIST_SENT_d11			((ui1) 87)  // header => misc_serv_data => list entries

// List Info Types
#define LS_TH_TYPE_LIST_ALL_MACHINES_d11		((si4) 1)
#define LS_TH_TYPE_LIST_LICENSED_MACHINES_d11		((si4) 2)
#define LS_TH_TYPE_LIST_BLOCKED_MACHINES_d11		((si4) 3)
#define LS_TH_TYPE_LIST_PRODUCT_LICENSES_d11		((si4) 4)

// License Types
#define LS_NO_LICENSE_TYPE_d11				((ui1) 0)
#define LS_UNKNOWN_LICENSE_TYPE_d11			LS_NO_LICENSE_TYPE_d11
#define LS_USER_LICENSE_TYPE_d11			((ui1) 1)
#define LS_GROUP_LICENSE_TYPE_d11			((ui1) 2)
#define LS_PERPETUAL_LICENSE_TYPE_d11			((ui1) 4)
#define LS_SUBSCRIPTION_LICENSE_TYPE_d11		((ui1) 8)
#define LS_TRIAL_LICENSE_TYPE_d11			((ui1) 16)
#define LS_EMERGENCY_LICENSE_TYPE_d11			((ui1) 32)
// #define LS_MASTER_LICENSE_TYPE_d11			((ui1) 64)  // not used at this time, but if implemented: unlimited users; no usage or upgrade expiration
// #define LS_UNREGISTERED_LICENSE_TYPE_d11		((ui1) 128)  // not used at this time

// Structure Sizes
#define LS_LICENSE_FILE_ENTRY_BYTES_d11			16
#define	LS_LICENSE_FILE_ENTRY_BASE_CHARACTERS_d11	32
#define	LS_LICENSE_FILE_ENTRY_STRLEN_d11		39
#define	LS_PASSWORD_HASH_STRLEN_d11			31
#define	LS_PASSWORD_HASH_BYTES_d11			32
#define	LS_PASSWORD_HINT_BYTES_d11			256
#define LS_MACHINE_INFO_BYTES_d11			336
#define LS_CUSTOMER_INFO_BYTES_d11			1568
#define LS_PRODUCT_LIST_ENTRY_BYTES_d11			64
#define LS_MACHINE_LIST_ENTRY_BYTES_d11			464
#define LS_MISC_SERVER_DATA_BYTES_d11			16
#define LS_AUXILIARY_LICENSE_DATA_BYTES_d11		32

// Miscellaneous License Server Constants
#define LS_UNKNOWN_MACHINE_CODE_d11			((ui4) GLOBALS_LS_MACHINE_CODE_DEFAULT_d11)  // == 0
#define LS_UNKNOWN_USER_CODE_d11			((ui4) GLOBALS_LS_USER_CODE_DEFAULT_d11)  // == 0
#define LS_UNKNOWN_CUSTOMER_CODE_d11			((ui4) GLOBALS_LS_CUSTOMER_CODE_DEFAULT_d11)  // == 0
#define LS_NO_ACCESS_d11				((si4) 0)
#define LS_STANDARD_ACCESS_d11				((si4) 1)
#define LS_MASTER_ACCESS_d11				((si4) 2)
#define LS_MINIMUM_TIMEOUT_INCREMENT_d11		((ui4) 60)  		// == 1 minute (seconds)
#define LS_MAXIMUM_TIMEOUT_INCREMENT_d11		((ui4) 0xFFFFFFFF)      // results in end of usage license expiration
#define LS_DEFAULT_TIMEOUT_INCREMENT_d11		((ui4) 86400)  		// == 1 day (seconds)
#define LS_HOST_NAME_BYTES_d11				252
#define LS_PRODUCT_NAME_BYTES_d11			13
#define LS_USER_NAME_BYTES_d11				20
#define LS_SERIAL_NUMBER_BYTES_d11			52
#define LS_EMAIL_ADDRESS_BYTES_d11			320
#define LS_MACH_LIST_PRODS_IN_USE_BYTES_d11		99
#define LS_SOCK_TIMEOUT_SECS_d11			30  // seconds of intactivity before socket will close (enough to type in password twice)
#define LS_SOCK_TIMEOUT_NEVER_d11			TH_SOCK_TIMEOUT_NEVER_d11  // == 0
#define LS_SOCK_TIMEOUT_USE_EXISTING_d11		TH_SOCK_TIMEOUT_USE_EXISTING_d11  // alloc_trans_info() will leave existing value
#define LS_DHN_INCEPTION_TIME_d11			((ui4) 0x5E0BE100)  // (dec: 1577836800) Wednesday, January 1, 2020 12:00:00 AM
#define LS_END_OF_TIME_d11				((si8) 0xFFFFFFFF + (si8) LS_DHN_INCEPTION_TIME_d11)  // (dec: 5872804095) Friday, February 7, 2156 6:28:15 AM GMT

// License File Entry (LFE) Format Constants
#define LS_LFE_PRODUCT_STRING_OFFSET_d11			0	                		// ascii[4]
#define LS_LFE_PRODUCT_STRING_TERMINAL_ZERO_OFFSET_d11		(LS_LFE_PRODUCT_STRING_OFFSET_d11 + 4)	// si1
#define LS_LFE_PRODUCT_CODE_OFFSET_d11				LS_LFE_PRODUCT_STRING_OFFSET_d11	// ui4
#define LS_LFE_PRODUCT_CODE_NO_ENTRY_d11			0	                		// ui4
#define LS_LFE_PRODUCT_VERSION_MAJOR_OFFSET_d11			5	                		// ui1
#define LS_LFE_PRODUCT_VERSION_MAJOR_NO_ENTRY_d11		0xFF
#define LS_LFE_PRODUCT_VERSION_MINOR_OFFSET_d11			6	                		// ui1
#define LS_LFE_PRODUCT_VERSION_MINOR_NO_ENTRY_d11		0xFF
#define LS_LFE_LICENSE_TYPE_OFFSET_d11				7	                		// ui1
#define LS_LFE_LICENSE_TYPE_NO_ENTRY_d11			LS_NO_TYPE_d11
#define LS_LFE_TIMEOUT_OFFSET_d11				8					// ui4
#define LS_LFE_TIMEOUT_NO_ENTRY_d11				0
#define LS_LFE_MACHINE_CODE_OFFSET_d11				12					// ui4
#define LS_LFE_MACHINE_CODE_NO_ENTRY_d11			0

// Products
#define LS_NUMBER_OF_PRODUCTS_d11		5  // not including test product

#define LS_TEST_PROD_STRING_d11			"test"			// ascii[4]
#define LS_TEST_PROD_CODE_d11			(ui4) 0x74736574	// ui4 (little endian)
// #define LS_TEST_PROD_CODE_d11		(ui4) 0x74657374        // ui4 (big endian)

#define LS_DAT2MED_PROD_STRING_d11		"DATM"			// ascii[4]
#define LS_DAT2MED_PROD_CODE_d11		(ui4) 0x4D544144	// ui4 (little endian)
// #define LS_DAT2MED_PROD_CODE_d11		(ui4) 0x4441544D       	// ui4 (big endian)

#define LS_NLX_MED_ACQ_PROD_STRING_d11		"NMAq"			// ascii[4]
#define LS_NLX_MED_ACQ_PROD_CODE_d11		(ui4) 0x71414D4E	// ui4 (little endian)
// #define LS_NLX_MED_ACQ_PROD_CODE_d11		(ui4) 0x4E4D4171        // ui4 (big endian)

#define LS_READ_MED_PROD_STRING_d11		"RdMd"			// ascii[4]
#define LS_READ_MED_PROD_CODE_d11		(ui4) 0x644D6452	// ui4 (little endian)
// #define LS_READ_MED_PROD_CODE_d11		(ui4) 0x52644D64       	// ui4 (big endian)

#define LS_NRD2MED_PROD_STRING_d11		"NRDM"			// ascii[4]
#define LS_NRD2MED_PROD_CODE_d11		(ui4) 0x4D44524E	// ui4 (little endian)
// #define LS_NRD2MED_PROD_CODE_d11		(ui4) 0x4E52444D       	// ui4 (big endian)

#define LS_CSC2MED_PROD_STRING_d11		"CSCM"			// ascii[4]
#define LS_CSC2MED_PROD_CODE_d11		(ui4) 0x4D435343	// ui4 (little endian)
// #define LS_CSC2MED_PROD_CODE_d11		(ui4) 0x4353434D       	// ui4 (big endian)

// Transmission Header ID Code
#define LS_TH_ID_STRING_d11			"LSrv"			// ascii[4]
#define LS_TH_ID_CODE_d11			(ui4) 0x7672534C	// ui4 (little endian)
// #define LS_TH_ID_CODE_d11			(ui4) 0x4C537276       	// ui4 (big endian)

// #define LS_SERVER_IP_ADDRESS_d11			"10.0.1.201"  // test system
#define LS_SERVER_IP_ADDRESS_d11			"72.174.93.146"  // port 8191 forwarded to "10.1.1.9" on Neuralynx LAN
#define LS_PORT_d11					"8191"	// 5th Mersenne prime (2^13 - 1) (6th if you count 1 as prime :)
#define LS_TRANSMISSION_BUFFER_BYTES_DEFAULT_d11	16384	// 16k => plenty for license server transactions

#ifdef LINUX_m11
	#define LS_MC_FILE_STRUCT_BYTES_d11	56
	#define LS_MC_FILE_NAME_d11		"/usr/local/etc/.dhnmc"
#endif  // LINUX_m11

// Typedefs
typedef struct {
	union {
		struct {
			si1     product_string[TYPE_BYTES_m11];
			ui1     product_version_major;
			ui1     product_version_minor;
			ui1     license_type;
		};
		struct {
			ui4     product_code;  // also in auxiliary license data
			si1	product_string_terminal_zero;
		};
	};
	ui4	timeout;	// updated by server ( == current_UTC - LS_DHN_INCEPTION_TIME_d11 == secs since 00:00 Jan 1, 2020 UTC)
	ui4	machine_code;	// so users can't copy license entries from one machine to another (also in auxiliary license data)
} LS_LICENSE_FILE_ENTRY_d11;

typedef struct {
	ui4	customer_code;
	ui4	product_code;  // also in license file entry
	ui4	machine_code;  // also in license file entry
	si1	user[LS_USER_NAME_BYTES_d11];
} LS_AUXILIARY_LICENSE_DATA_d11;

typedef struct {
	si4	list_type;
	si4	number_of_list_entries;
	ui4	timeout_increment;
	si1	access_level;
	si1	pad_bytes[3];
} LS_MISC_SERVER_DATA_d11;

typedef struct {
	si1	host_name[LS_HOST_NAME_BYTES_d11];  // 252 == established max length
	si1	serial_number[LS_SERIAL_NUMBER_BYTES_d11];  // 52 == established max length
	si1	WAN_IPv4_address[IPV4_ADDRESS_STR_BYTES_d11];	// as string ; LAN_IPv4_address may equal WAN_IPv4_addres
	si1	LAN_IPv4_address[IPV4_ADDRESS_STR_BYTES_d11];
} LS_MACHINE_INFO_d11;

typedef struct {
	ui1	license_type;
	ui1	eligible_product_version_major;
	ui1	eligible_product_version_minor;
	ui1	product_name[LS_PRODUCT_NAME_BYTES_d11];
	ui4	product_code;
	ui4	license_count;
	ui4	licenses_in_use;
	ui4	last_use;
	si1	last_user[LS_USER_NAME_BYTES_d11];
	ui4	usage_expiration;
	ui4	upgrade_expiration;
	ui4	default_timeout_increment;
} LS_PRODUCT_LIST_ENTRY_d11;

typedef struct {
	ui4		machine_code;
	si1		host_name[LS_HOST_NAME_BYTES_d11];  // 252 == established max length
	si1		serial_number[LS_SERIAL_NUMBER_BYTES_d11];  // 52 == established max length
	si1		WAN_IPv4_address[IPV4_ADDRESS_STR_BYTES_d11];
	si1		LAN_IPv4_address[IPV4_ADDRESS_STR_BYTES_d11];
	ui4		last_use;
	si1		last_user[LS_USER_NAME_BYTES_d11];
	TERN_m11	blocked;
	si1		products_in_use[LS_MACH_LIST_PRODS_IN_USE_BYTES_d11];
} LS_MACHINE_LIST_ENTRY_d11;

typedef struct {
	si1	hint[LS_PASSWORD_HINT_BYTES_d11];  // struct so it can be assigned, rather than copied when convenient
} LS_PASSWORD_HINT_d11;

typedef struct {
	si1	hash[LS_PASSWORD_HASH_BYTES_d11];  // struct so it can be assigned, rather than copied when convenient
} LS_PASSWORD_HASH_d11;

typedef struct {
	si1		salutation[8];
	si1		forename[32];
	si1		surname[32];
	si1		degrees[16];
	si1		occupation[64];
	si1		job_title[64];
	si1		gender[16];
	si1		organization[64];
	si1		department[64];
	si1		division[64];
	si1		country[64];
	si1		territory[64];
	si1		locality[64];
	si1		street_address[128];
	si1		postal_code[16];
	si1		standard_timezone_acronym[8];  // TIMEZONE_ACRONYM_BYTES_m11
	si1		email_primary[LS_EMAIL_ADDRESS_BYTES_d11];  // 320 == established max length
	si1		email_secondary[LS_EMAIL_ADDRESS_BYTES_d11];  // 320 == established max length
	si1		phone_primary[64];
	si1		phone_secondary[64];
	si1		research[8];
	si1		clinical[8];
	si1		industry[8];
	si1		customer_type[8];
} LS_CUSTOMER_INFO_d11;

#ifdef LINUX_m11
typedef struct{
	ui4	machine_code;
	si1	serial_number[LS_SERIAL_NUMBER_BYTES_d11];  // 52 == established max length
} LS_MC_FILE_STRUCT_d11;
#endif  // LINUX_m11


// Prototypes
TERN_m11	LSc_add_customer_d11(LS_PASSWORD_HASH_d11 *returned_master_pw_hash);
TERN_m11	LSc_check_license_d11(ui4 product_code, ui1 version_major, ui1 version_minor);
TERN_m11	LSc_check_license_file_entry_alignment_d11(ui1 *bytes);
si1		LSc_check_password_d11(TRANSMISSION_INFO_d11 *trans_info, si1 *password, si1 *prompt, LS_PASSWORD_HASH_d11 *returned_pw_hash);
TERN_m11	LSc_get_machine_info_d11(LS_MACHINE_INFO_d11 *machine_info, si1 *WAN_IPv4_address_string);
TERN_m11	LSc_issue_license_d11(TRANSMISSION_INFO_d11 *trans_info, LS_LICENSE_FILE_ENTRY_d11 *lfe, LS_PASSWORD_HASH_d11 *pw_hash);
TERN_m11	LSc_process_password_d11(si1 *pw, LS_PASSWORD_HASH_d11 *pw_hash);
TERN_m11	LSc_read_license_file_d11(LS_LICENSE_FILE_ENTRY_d11 **license_entries, si4 *number_of_license_entries);
TERN_m11	LSc_send_machine_info_d11(TRANSMISSION_INFO_d11 *trans_info, LS_PASSWORD_HASH_d11 *pw_hash);
TERN_m11	LSc_send_password_d11(TRANSMISSION_INFO_d11 *trans_info, si1 *prompt, LS_PASSWORD_HASH_d11 *pw_hash);
void		LSc_show_license_entries_d11(LS_LICENSE_FILE_ENTRY_d11 *license_entries, si4 number_of_license_entries);
TERN_m11	LSc_write_license_file_d11(LS_LICENSE_FILE_ENTRY_d11 *licence_entries, si4 number_of_license_entries);


//**********************************************************************************//
//*************************  MED SERVER (MS; MSc - client)  ************************//
//**********************************************************************************//

// MED Server (MS) Transmission Header (TH) Types
// • Type numbers 0-63 reserved for generic transmission types
// • Type numbers 64-255 used for application specific transmission types
#define MS_TH_TYPE_DATA_REQUEST_d11		((ui1) 64)  // header => MS_DATA_REQUEST_d11 => [+/- CMP_parameters] => [+/- CMP_directives] => MS_LIST_INFO_d11 => MS_CHANNEL_NAME_LIST_ENTRY_d11(s)
#define MS_TH_TYPE_SESSION_METADATA_d11		((ui1) 65)  // header => MS_SESSION_METADATA_d11
#define MS_TH_TYPE_CHANNEL_METADATA_d11		((ui1) 66)  // header => CHANNEL_METADATA_d11
#define MS_TH_TYPE_CHANNEL_DATA_BLOCK_d11	((ui1) 67)  // header => CHANNEL_DATA_BLOCK_d11
#define MS_TH_TYPE_REPEAT_DATA_REQUEST_d11	((ui1) 68)  // header => MS_REPEAT_DATA_REQUEST_d11
#define MS_TH_TYPE_SEND_LIST_d11		((ui1) 69)  // header => MS_SEARCH_CRITERIA_d11
#define MS_TH_TYPE_LIST_d11			((ui1) 70)  // header => MS_LIST_INFO_d11 => list_entries

// Transmission Header ID Code
#define MS_TH_ID_STRING_d11			"MSrv"			// ascii[4]
#define MS_TH_ID_CODE_d11			(ui4) 0x7672534D	// ui4 (little endian)
// #define MS_TH_ID_CODE_d11			(ui4) 0x4D537276       	// ui4 (big endian)

// Data Format Constants
#define MS_CMP_BLOCKS_d11		1
#define MS_INTEGERS_d11			2
#define MS_SINGLES_d11			3
#define MS_DOUBLES_d11			4

// Data Output Target Constants
#define MS_CLIENT_MEMORY_d11		1
#define MS_SHARED_MEMORY_d11		2
#define MS_LOCAL_FILE_SYSTEM_d11	3

// Miscellaneous Constants
#define MS_LINEAR_INTERPOLATION_d11			1
#define MS_CUBIC_SPLINE_INTERPOLATION_d11		2
#define MS_UPSAMPLE_SPLINE_DOWNSAMPLE_LINEAR_d11	3  	// linear upsample if (true_sf / upsample_sf) >
								// MS_SPLINE_UPSAMPLE_SF_RATIO_THRESH_d11, spline if greater
								// (spline will give spurious curves if steps too small)
#define MS_DEFAULT_INTERPOLATION_d11			MS_UPSAMPLE_SPLINE_DOWNSAMPLE_LINEAR_d11
#define MS_DEFAULT_FILTER_ORDER_d11			3
#define MS_MAX_FILTER_ORDER_d11				8
#define MS_USER_NAME_BYTES_d11				32
#define MS_SPLINE_UPSAMPLE_SF_RATIO_THRESH_d11		CMP_SPLINE_UPSAMPLE_SF_RATIO_m11
#define MS_BLOCK_SAMPLES_DEFAULT_d11			50000
#define MS_SOCK_DEFAULT_TIMEOUT_SECS_d11		60

#define MS_START_TIME_DEFAULT_d11			BEGINNING_OF_TIME_m11
#define MS_END_TIME_DEFAULT_d11				END_OF_TIME_m11
#define MS_START_INDEX_DEFAULT_d11			SAMPLE_NUMBER_NO_ENTRY_m11
#define MS_END_INDEX_DEFAULT_d11			SAMPLE_NUMBER_NO_ENTRY_m11
#define MS_OUTPUT_BLOCK_SAMPLES_DEFAULT_d11		50000
#define MS_CMP_DIRECTIVES_ATTACHED_DEFAULT_d11		FALSE_m11
#define MS_CMP_PARAMETERS_ATTACHED_DEFAULT_d11		FALSE_m11
#define MS_DATA_FORMAT_DEFAULT_d11			MS_CMP_BLOCKS_d11
#define MS_OUTPUT_TARGET_DEFAULT_d11			MS_CLIENT_MEMORY_d11
#define MS_BYPASS_DATABASE_DEFAULT_d11			TRUE_m11  // if path is passed, more efficient
#define MS_KEEP_CONNECTION_DEFAULT_d11			FALSE_m11  // leave socket open & child alive

// Structure Sizes
#define MS_SESSION_METADATA_BYTES_d11			9248  // (% 16 == 0)
#define	MS_CHANNEL_METADATA_BYTES_d11			4720  // (% 16 == 0)
#define	MS_CHANNEL_DATA_BLOCK_BYTES_d11			40  // (% 8 == 0)
#define	MS_DATA_REQUEST_BYTES_d11			1704  // (% 8 == 0)
#define	MS_REPEAT_DATA_REQUEST_BYTES_d11		40  // (% 8 == 0)
#define	MS_LIST_INFO_BYTES_d11				8  // (% 8 == 0)
#define	MS_CHANNEL_NAME_LIST_ENTRY_BYTES_d11		264  // (% 8 == 0)

// List Info Types
#define MS_TH_TYPE_LIST_SESSIONS_d11			((si4) 1)
#define MS_TH_TYPE_LIST_CHANNELS_d11			((si4) 2)

#define MS_SERVER_LOCAL_IP_ADDRESS_d11			"127.0.0.1"  // loopback
#define MS_PORT_d11					"8192"	// 2^13
#define MS_TRANSMISSION_BUFFER_BYTES_DEFAULT_d11	1048576	 // 1 MB

// Typedefs
typedef struct {
	si8	session_UID;
	si1	path[FULL_FILE_NAME_BYTES_m11];
	si8	start_time;
	si8	end_time;
	si1	start_time_string[TIME_STRING_BYTES_m11];
	si1	end_time_string[TIME_STRING_BYTES_m11];
	si8	session_start_time;
	si8	session_end_time;
	si1	session_start_time_string[TIME_STRING_BYTES_m11];
	si1	session_end_time_string[TIME_STRING_BYTES_m11];
	si1	session_name[BASE_FILE_NAME_BYTES_m11];
	si1	anonymized_subject_ID[UNIVERSAL_HEADER_ANONYMIZED_SUBJECT_ID_BYTES_m11];
	si1	session_description[METADATA_SESSION_DESCRIPTION_BYTES_m11];
	si1	equipment_description[METADATA_EQUIPMENT_DESCRIPTION_BYTES_m11];
	si1	reference_description[TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m11];
	sf8	sampling_frequency;
	sf8	low_frequency_filter_setting;
	sf8	high_frequency_filter_setting;
	sf8	notch_filter_frequency_setting;
	sf8	AC_line_frequency;
	sf8	amplitude_units_conversion_factor;
	si1	amplitude_units_description[TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m11];
	sf8	time_base_units_conversion_factor;
	si1	time_base_units_description[TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m11];
	si8	recording_time_offset;
	si1	standard_timezone_string[TIMEZONE_STRING_BYTES_m11];
	si1	standard_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m11];
	si1	daylight_timezone_string[TIMEZONE_STRING_BYTES_m11];
	si1	daylight_timezone_acronym[TIMEZONE_ACRONYM_BYTES_m11];
	DAYLIGHT_TIME_CHANGE_CODE_m11	daylight_time_start_code;
	DAYLIGHT_TIME_CHANGE_CODE_m11	daylight_time_end_code;
	si1	subject_name_1[METADATA_SUBJECT_NAME_BYTES_m11];
	si1	subject_name_2[METADATA_SUBJECT_NAME_BYTES_m11];
	si1	subject_name_3[METADATA_SUBJECT_NAME_BYTES_m11];
	si1	subject_ID[METADATA_SUBJECT_ID_BYTES_m11];
	si1	recording_country[METADATA_RECORDING_LOCATION_BYTES_m11];
	si1	recording_territory[METADATA_RECORDING_LOCATION_BYTES_m11];
	si1	recording_locality[METADATA_RECORDING_LOCATION_BYTES_m11];
	si1	recording_institution[METADATA_RECORDING_LOCATION_BYTES_m11];
	si4	standard_UTC_offset;
	ui4     number_of_discontinuities;  // updated on receipt
	si4	number_of_channels;
	si4	shared_memory_key;  // channel data will be put in shared memory allocated by server (local use only)
	si4	socket_fd;  // for open socket, repeat calls
	ui1	pad_bytes[4];  // for 16 byte alignment (for encryption)
} MS_SESSION_METADATA_d11;

typedef struct {
	si8	channel_UID;
	si1	channel_name[BASE_FILE_NAME_BYTES_m11];
	si1	channel_description[METADATA_CHANNEL_DESCRIPTION_BYTES_m11];
	si1	equipment_description[METADATA_EQUIPMENT_DESCRIPTION_BYTES_m11];
	si1	reference_description[TIME_SERIES_METADATA_REFERENCE_DESCRIPTION_BYTES_m11];
	si8	absolute_start_index;  // relative to session
	si8	absolute_end_index;  // relative to session
	sf8	sampling_frequency;
	sf8	low_frequency_filter_setting;
	sf8	high_frequency_filter_setting;
	sf8	notch_filter_frequency_setting;
	sf8	amplitude_units_conversion_factor;
	si1	amplitude_units_description[TIME_SERIES_METADATA_AMPLITUDE_UNITS_DESCRIPTION_BYTES_m11];
	sf8	time_base_units_conversion_factor;
	si1	time_base_units_description[TIME_SERIES_METADATA_TIME_BASE_UNITS_DESCRIPTION_BYTES_m11];
	si8     number_of_samples;  // updated on receipt
	si8	number_of_blocks;  // updated on receipt
	si8     maximum_block_bytes;  // updated on receipt
	ui4     maximum_block_samples;  // updated on receipt
	ui4     maximum_block_difference_bytes;  // updated on receipt
	si4	acquisition_channel_number;
	ui4     number_of_discontinuities;  // updated on receipt
} MS_CHANNEL_METADATA_d11;

typedef struct {
	si8		channel_UID;
	si8		number_of_samples;
	si8		time;
	si8		index;  // relative to requested data block, not session
	ui4		block_number;  // in case arrive out of order, or dropped
	ui1		data_format;  // block, ints
	TERN_m11	discontinuity;
	TERN_m11	end_of_data;
	ui1		pad_byte;
} MS_CHANNEL_DATA_BLOCK_d11;

typedef struct {
	si1		server_IPv4_address[IPV4_ADDRESS_STR_BYTES_d11] ;
	si8		sess_uid;
	si1		sess_name[BASE_FILE_NAME_BYTES_m11];
	si1		sess_path[FULL_FILE_NAME_BYTES_m11];
	si1		user_name[MS_USER_NAME_BYTES_d11];  // implies auxiliary user & db use
	si1		password[PASSWORD_BYTES_m11];  // encrypted
	si8		start_time;
	si8		end_time;
	si8		start_index;  // relative session start
	si8		end_index;  // relative session start
	si8		idx_ref_chan_uid;  // needed for data requests by index, in sessions with variable sampling frequencies
	si1		idx_ref_chan_name[BASE_FILE_NAME_BYTES_m11];  // needed for data requests by index, in sessions with variable sampling frequencies
	sf8		pass_filt_low_fc;
	sf8		pass_filt_high_fc;
	sf8		stop_filt_low_fc;
	sf8		stop_filt_high_fc;
	ui4		output_block_samples;  // for block format transmission
	si4		timeout_seconds;  // time before socket closes (relevent if repeat data requests expected)
	ui1		filter_order;
	ui1 		CMP_parameters_attached;  // follows DATA_REQUEST
	ui1		CMP_directives_attached;  // follows DATA_REQUEST and CMP_PARAMETERS (if present)
	ui1		interpolation_type;
	ui1		data_format;  // compressed blocks, ints (si4), singles (sf4), or doubles (sf8)
	ui1		output_target;  // client, shared memory (key returned in via TCP MS_SESSION_DATA structure => channel data in shared memory allocated by server)
	TERN_m11	bypass_database;  // use path info / password to read session, even if db exists
	TERN_m11	keep_connection;  // don't close socket, repeat data requests expected
	TERN_m11	buffer_mode;
	ui1		buffer_mode_forward_pages;  // zero if not buffer mode (keep_connection flag required) 
	ui1		buffer_mode_backward_pages;  // zero if not buffer mode (keep_connection flag required)
	ui1		pad_byte;
} MS_DATA_REQUEST_d11;

typedef struct {
	si8		start_time;
	si8		end_time;
	si8		start_index;  // relative session start
	si8		end_index;  // relative session start
	si4		shared_memory_key;  // server may have had to reallocate, if using shared memory
	TERN_m11	keep_connection;  // don't close socket, further data requests expected
	ui1		pad_bytes[3];
} MS_REPEAT_DATA_REQUEST_d11;	// uses info from initial DATA_REQUEST, replaces existing data
				// MS_SESSION_METADATA & MS_CHANNEL_METADATA structures updated on receipt

typedef struct {
	si4	list_entry_type;
	si4	number_of_list_entries;
} MS_LIST_INFO_d11;

typedef struct {
	si8	channel_UID;
	si1	channel_name[BASE_FILE_NAME_BYTES_m11];
} MS_CHANNEL_NAME_LIST_ENTRY_d11;


// Prototypes
void	MSc_initialize_data_request_d11(MS_DATA_REQUEST_d11 *data_request);


#endif /* DHNLIB_IN_d11 */
