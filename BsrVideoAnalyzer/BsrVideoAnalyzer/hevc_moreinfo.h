#pragma once


extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavutil/parseutils.h"
#include "libavutil/mem.h"
#include "libavutil/avutil.h"


}
//put_bits.h
typedef struct PutBitContext {
	uint32_t bit_buf;
	int bit_left;
	uint8_t *buf, *buf_ptr, *buf_end;
	int size_in_bits;
} PutBitContext;

//cabac.h
typedef struct CABACContext {
	int low;
	int range;
	int outstanding_count;
	const uint8_t *bytestream_start;
	const uint8_t *bytestream;
	const uint8_t *bytestream_end;
	PutBitContext pb;
}CABACContext;


//videodsp.h
#define EMULATED_EDGE(depth) \
void ff_emulated_edge_mc_ ## depth(uint8_t *dst, const uint8_t *src, \
                                   ptrdiff_t dst_stride, ptrdiff_t src_stride, \
                                   int block_w, int block_h,\
                                   int src_x, int src_y, int w, int h);

EMULATED_EDGE(8)
EMULATED_EDGE(16)

typedef struct VideoDSPContext {
	/**
	* Copy a rectangular area of samples to a temporary buffer and replicate
	* the border samples.
	*
	* @param dst destination buffer
	* @param dst_stride number of bytes between 2 vertically adjacent samples
	*                   in destination buffer
	* @param src source buffer
	* @param dst_linesize number of bytes between 2 vertically adjacent
	*                     samples in the destination buffer
	* @param src_linesize number of bytes between 2 vertically adjacent
	*                     samples in both the source buffer
	* @param block_w width of block
	* @param block_h height of block
	* @param src_x x coordinate of the top left sample of the block in the
	*                source buffer
	* @param src_y y coordinate of the top left sample of the block in the
	*                source buffer
	* @param w width of the source buffer
	* @param h height of the source buffer
	*/
	void(*emulated_edge_mc)(uint8_t *dst, const uint8_t *src,
		ptrdiff_t dst_linesize,
		ptrdiff_t src_linesize,
		int block_w, int block_h,
		int src_x, int src_y, int w, int h);

	/**
	* Prefetch memory into cache (if supported by hardware).
	*
	* @param buf    pointer to buffer to prefetch memory from
	* @param stride distance between two lines of buf (in bytes)
	* @param h      number of lines to prefetch
	*/
	void(*prefetch)(uint8_t *buf, ptrdiff_t stride, int h);
} VideoDSPContext;


//hevcpred.h
#if 0
struct HEVCPredContext;
#else
typedef struct HEVCPredContext {
	void(*intra_pred[4])(struct HEVCContext *s, int x0, int y0, int c_idx);

	void(*pred_planar[4])(uint8_t *src, const uint8_t *top,
		const uint8_t *left, ptrdiff_t stride);
	void(*pred_dc)(uint8_t *src, const uint8_t *top, const uint8_t *left,
		ptrdiff_t stride, int log2_size, int c_idx);
	void(*pred_angular[4])(uint8_t *src, const uint8_t *top,
		const uint8_t *left, ptrdiff_t stride,
		int c_idx, int mode);
} HEVCPredContext;
#endif
//bswapdsp.h
typedef struct BswapDSPContext {
	void(*bswap_buf)(uint32_t *dst, const uint32_t *src, int w);
	void(*bswap16_buf)(uint16_t *dst, const uint16_t *src, int len);
} BswapDSPContext;

//thread.h
typedef struct ThreadFrame {
	AVFrame *f;
	AVCodecContext *owner;
	// progress->data is an array of 2 ints holding progress for top/bottom
	// fields
	AVBufferRef *progress;
} ThreadFrame;

//get_bits.h
typedef struct GetBitContext {
	const uint8_t *buffer, *buffer_end;
	int index;
	int size_in_bits;
	int size_in_bits_plus8;
} GetBitContext;

//hevcdsp.h
#define MAX_PB_SIZE 64

typedef struct SAOParams {
	int offset_abs[3][4];   ///< sao_offset_abs
	int offset_sign[3][4];  ///< sao_offset_sign

	uint8_t band_position[3];   ///< sao_band_position

	int eo_class[3];        ///< sao_eo_class

	int16_t offset_val[3][5];   ///<SaoOffsetVal

	uint8_t type_idx[3];    ///< sao_type_idx
} SAOParams;

typedef struct HEVCDSPContext {
	void(*put_pcm)(uint8_t *_dst, ptrdiff_t _stride, int width, int height,
	struct GetBitContext *gb, int pcm_bit_depth);

	void(*transform_add[4])(uint8_t *_dst, int16_t *coeffs, ptrdiff_t _stride);

	void(*transform_skip)(int16_t *coeffs, int16_t log2_size);

	void(*transform_rdpcm)(int16_t *coeffs, int16_t log2_size, int mode);

	void(*idct_4x4_luma)(int16_t *coeffs);

	void(*idct[4])(int16_t *coeffs, int col_limit);

	void(*idct_dc[4])(int16_t *coeffs);

	void(*sao_band_filter[5])(uint8_t *_dst, uint8_t *_src, ptrdiff_t _stride_dst, ptrdiff_t _stride_src,
		int16_t *sao_offset_val, int sao_left_class, int width, int height);

	/* implicit stride_src parameter has value of 2 * MAX_PB_SIZE + AV_INPUT_BUFFER_PADDING_SIZE */
	void(*sao_edge_filter[5])(uint8_t *_dst /* align 16 */, uint8_t *_src /* align 32 */, ptrdiff_t stride_dst,
		int16_t *sao_offset_val, int sao_eo_class, int width, int height);

	void(*sao_edge_restore[2])(uint8_t *_dst, uint8_t *_src, ptrdiff_t _stride_dst, ptrdiff_t _stride_src,
	struct SAOParams *sao, int *borders, int _width, int _height, int c_idx,
		uint8_t *vert_edge, uint8_t *horiz_edge, uint8_t *diag_edge);

	void(*put_hevc_qpel[10][2][2])(int16_t *dst, uint8_t *src, ptrdiff_t srcstride,
		int height, intptr_t mx, intptr_t my, int width);
	void(*put_hevc_qpel_uni[10][2][2])(uint8_t *dst, ptrdiff_t dststride, uint8_t *src, ptrdiff_t srcstride,
		int height, intptr_t mx, intptr_t my, int width);
	void(*put_hevc_qpel_uni_w[10][2][2])(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
		int height, int denom, int wx, int ox, intptr_t mx, intptr_t my, int width);

	void(*put_hevc_qpel_bi[10][2][2])(uint8_t *dst, ptrdiff_t dststride, uint8_t *_src, ptrdiff_t _srcstride,
		int16_t *src2,
		int height, intptr_t mx, intptr_t my, int width);
	void(*put_hevc_qpel_bi_w[10][2][2])(uint8_t *dst, ptrdiff_t dststride, uint8_t *_src, ptrdiff_t _srcstride,
		int16_t *src2,
		int height, int denom, int wx0, int wx1,
		int ox0, int ox1, intptr_t mx, intptr_t my, int width);
	void(*put_hevc_epel[10][2][2])(int16_t *dst, uint8_t *src, ptrdiff_t srcstride,
		int height, intptr_t mx, intptr_t my, int width);

	void(*put_hevc_epel_uni[10][2][2])(uint8_t *dst, ptrdiff_t dststride, uint8_t *_src, ptrdiff_t _srcstride,
		int height, intptr_t mx, intptr_t my, int width);
	void(*put_hevc_epel_uni_w[10][2][2])(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
		int height, int denom, int wx, int ox, intptr_t mx, intptr_t my, int width);
	void(*put_hevc_epel_bi[10][2][2])(uint8_t *dst, ptrdiff_t dststride, uint8_t *_src, ptrdiff_t _srcstride,
		int16_t *src2,
		int height, intptr_t mx, intptr_t my, int width);
	void(*put_hevc_epel_bi_w[10][2][2])(uint8_t *dst, ptrdiff_t dststride, uint8_t *_src, ptrdiff_t _srcstride,
		int16_t *src2,
		int height, int denom, int wx0, int ox0, int wx1,
		int ox1, intptr_t mx, intptr_t my, int width);

	void(*hevc_h_loop_filter_luma)(uint8_t *pix, ptrdiff_t stride,
		int beta, int32_t *tc,
		uint8_t *no_p, uint8_t *no_q);
	void(*hevc_v_loop_filter_luma)(uint8_t *pix, ptrdiff_t stride,
		int beta, int32_t *tc,
		uint8_t *no_p, uint8_t *no_q);
	void(*hevc_h_loop_filter_chroma)(uint8_t *pix, ptrdiff_t stride,
		int32_t *tc, uint8_t *no_p, uint8_t *no_q);
	void(*hevc_v_loop_filter_chroma)(uint8_t *pix, ptrdiff_t stride,
		int32_t *tc, uint8_t *no_p, uint8_t *no_q);
	void(*hevc_h_loop_filter_luma_c)(uint8_t *pix, ptrdiff_t stride,
		int beta, int32_t *tc,
		uint8_t *no_p, uint8_t *no_q);
	void(*hevc_v_loop_filter_luma_c)(uint8_t *pix, ptrdiff_t stride,
		int beta, int32_t *tc,
		uint8_t *no_p, uint8_t *no_q);
	void(*hevc_h_loop_filter_chroma_c)(uint8_t *pix, ptrdiff_t stride,
		int32_t *tc, uint8_t *no_p,
		uint8_t *no_q);
	void(*hevc_v_loop_filter_chroma_c)(uint8_t *pix, ptrdiff_t stride,
		int32_t *tc, uint8_t *no_p,
		uint8_t *no_q);
} HEVCDSPContext;


#define MAX_DPB_SIZE 16 // A.4.1
#define MAX_REFS 16

#define MAX_NB_THREADS 16
#define SHIFT_CTB_WPP 2

/**
* 7.4.2.1
*/
#define MAX_SUB_LAYERS 7
#define MAX_VPS_COUNT 16
#define MAX_SPS_COUNT 32
#define MAX_PPS_COUNT 256
#define MAX_SHORT_TERM_RPS_COUNT 64
#define MAX_CU_SIZE 128

//TODO: check if this is really the maximum
#define MAX_TRANSFORM_DEPTH 5

#define MAX_TB_SIZE 32
#define MAX_LOG2_CTB_SIZE 6
#define MAX_QP 51
#define DEFAULT_INTRA_TC_OFFSET 2

#define HEVC_CONTEXTS 199

#define MRG_MAX_NUM_CANDS     5

#define L0 0
#define L1 1

#define EPEL_EXTRA_BEFORE 1
#define EPEL_EXTRA_AFTER  2
#define EPEL_EXTRA        3
#define QPEL_EXTRA_BEFORE 3
#define QPEL_EXTRA_AFTER  4
#define QPEL_EXTRA        7

#define EDGE_EMU_BUFFER_STRIDE 80

/**
* Value of the luma sample at position (x, y) in the 2D array tab.
*/
#define SAMPLE(tab, x, y) ((tab)[(y) * s->sps->width + (x)])
#define SAMPLE_CTB(tab, x, y) ((tab)[(y) * min_cb_width + (x)])

#define IS_IDR(s) ((s)->nal_unit_type == NAL_IDR_W_RADL || (s)->nal_unit_type == NAL_IDR_N_LP)
#define IS_BLA(s) ((s)->nal_unit_type == NAL_BLA_W_RADL || (s)->nal_unit_type == NAL_BLA_W_LP || \
                   (s)->nal_unit_type == NAL_BLA_N_LP)
#define IS_IRAP(s) ((s)->nal_unit_type >= 16 && (s)->nal_unit_type <= 23)

/**
* Table 7-3: NAL unit type codes
*/
enum NALUnitType {
	NAL_TRAIL_N = 0,
	NAL_TRAIL_R = 1,
	NAL_TSA_N = 2,
	NAL_TSA_R = 3,
	NAL_STSA_N = 4,
	NAL_STSA_R = 5,
	NAL_RADL_N = 6,
	NAL_RADL_R = 7,
	NAL_RASL_N = 8,
	NAL_RASL_R = 9,
	NAL_BLA_W_LP = 16,
	NAL_BLA_W_RADL = 17,
	NAL_BLA_N_LP = 18,
	NAL_IDR_W_RADL = 19,
	NAL_IDR_N_LP = 20,
	NAL_CRA_NUT = 21,
	NAL_VPS = 32,
	NAL_SPS = 33,
	NAL_PPS = 34,
	NAL_AUD = 35,
	NAL_EOS_NUT = 36,
	NAL_EOB_NUT = 37,
	NAL_FD_NUT = 38,
	NAL_SEI_PREFIX = 39,
	NAL_SEI_SUFFIX = 40,
};

enum RPSType {
	ST_CURR_BEF = 0,
	ST_CURR_AFT,
	ST_FOLL,
	LT_CURR,
	LT_FOLL,
	NB_RPS_TYPE,
};

/*
enum SliceType {
B_SLICE = 0,
P_SLICE = 1,
I_SLICE = 2,
};
*/
enum SyntaxElement {
	SAO_MERGE_FLAG = 0,
	SAO_TYPE_IDX,
	SAO_EO_CLASS,
	SAO_BAND_POSITION,
	SAO_OFFSET_ABS,
	SAO_OFFSET_SIGN,
	END_OF_SLICE_FLAG,
	SPLIT_CODING_UNIT_FLAG,
	CU_TRANSQUANT_BYPASS_FLAG,
	SKIP_FLAG,
	CU_QP_DELTA,
	PRED_MODE_FLAG,
	PART_MODE,
	PCM_FLAG,
	PREV_INTRA_LUMA_PRED_FLAG,
	MPM_IDX,
	REM_INTRA_LUMA_PRED_MODE,
	INTRA_CHROMA_PRED_MODE,
	MERGE_FLAG,
	MERGE_IDX,
	INTER_PRED_IDC,
	REF_IDX_L0,
	REF_IDX_L1,
	ABS_MVD_GREATER0_FLAG,
	ABS_MVD_GREATER1_FLAG,
	ABS_MVD_MINUS2,
	MVD_SIGN_FLAG,
	MVP_LX_FLAG,
	NO_RESIDUAL_DATA_FLAG,
	SPLIT_TRANSFORM_FLAG,
	CBF_LUMA,
	CBF_CB_CR,
	TRANSFORM_SKIP_FLAG,
	EXPLICIT_RDPCM_FLAG,
	EXPLICIT_RDPCM_DIR_FLAG,
	LAST_SIGNIFICANT_COEFF_X_PREFIX,
	LAST_SIGNIFICANT_COEFF_Y_PREFIX,
	LAST_SIGNIFICANT_COEFF_X_SUFFIX,
	LAST_SIGNIFICANT_COEFF_Y_SUFFIX,
	SIGNIFICANT_COEFF_GROUP_FLAG,
	SIGNIFICANT_COEFF_FLAG,
	COEFF_ABS_LEVEL_GREATER1_FLAG,
	COEFF_ABS_LEVEL_GREATER2_FLAG,
	COEFF_ABS_LEVEL_REMAINING,
	COEFF_SIGN_FLAG,
	LOG2_RES_SCALE_ABS,
	RES_SCALE_SIGN_FLAG,
	CU_CHROMA_QP_OFFSET_FLAG,
	CU_CHROMA_QP_OFFSET_IDX,
};

enum PartMode {
	PART_2Nx2N = 0,
	PART_2NxN = 1,
	PART_Nx2N = 2,
	PART_NxN = 3,
	PART_2NxnU = 4,
	PART_2NxnD = 5,
	PART_nLx2N = 6,
	PART_nRx2N = 7,
};

enum PredMode {
	MODE_INTER = 0,
	MODE_INTRA,
	MODE_SKIP,
};

enum InterPredIdc {
	PRED_L0 = 0,
	PRED_L1,
	PRED_BI,
};

enum PredFlag {
	PF_INTRA = 0,
	PF_L0,
	PF_L1,
	PF_BI,
};

enum IntraPredMode {
	INTRA_PLANAR = 0,
	INTRA_DC,
	INTRA_ANGULAR_2,
	INTRA_ANGULAR_3,
	INTRA_ANGULAR_4,
	INTRA_ANGULAR_5,
	INTRA_ANGULAR_6,
	INTRA_ANGULAR_7,
	INTRA_ANGULAR_8,
	INTRA_ANGULAR_9,
	INTRA_ANGULAR_10,
	INTRA_ANGULAR_11,
	INTRA_ANGULAR_12,
	INTRA_ANGULAR_13,
	INTRA_ANGULAR_14,
	INTRA_ANGULAR_15,
	INTRA_ANGULAR_16,
	INTRA_ANGULAR_17,
	INTRA_ANGULAR_18,
	INTRA_ANGULAR_19,
	INTRA_ANGULAR_20,
	INTRA_ANGULAR_21,
	INTRA_ANGULAR_22,
	INTRA_ANGULAR_23,
	INTRA_ANGULAR_24,
	INTRA_ANGULAR_25,
	INTRA_ANGULAR_26,
	INTRA_ANGULAR_27,
	INTRA_ANGULAR_28,
	INTRA_ANGULAR_29,
	INTRA_ANGULAR_30,
	INTRA_ANGULAR_31,
	INTRA_ANGULAR_32,
	INTRA_ANGULAR_33,
	INTRA_ANGULAR_34,
};

enum SAOType {
	SAO_NOT_APPLIED = 0,
	SAO_BAND,
	SAO_EDGE,
	SAO_APPLIED
};

enum SAOEOClass {
	SAO_EO_HORIZ = 0,
	SAO_EO_VERT,
	SAO_EO_135D,
	SAO_EO_45D,
};

enum ScanType {
	SCAN_DIAG = 0,
	SCAN_HORIZ,
	SCAN_VERT,
};

typedef struct ShortTermRPS {
	unsigned int num_negative_pics;
	int num_delta_pocs;
	int rps_idx_num_delta_pocs;
	int32_t delta_poc[32];
	uint8_t used[32];
} ShortTermRPS;

typedef struct LongTermRPS {
	int     poc[32];
	uint8_t used[32];
	uint8_t nb_refs;
} LongTermRPS;

typedef struct RefPicList {
	struct HEVCFrame *ref[MAX_REFS];
	int list[MAX_REFS];
	int isLongTerm[MAX_REFS];
	int nb_refs;
} RefPicList;

typedef struct RefPicListTab {
	RefPicList refPicList[2];
} RefPicListTab;

typedef struct HEVCWindow {
	unsigned int left_offset;
	unsigned int right_offset;
	unsigned int top_offset;
	unsigned int bottom_offset;
} HEVCWindow;

typedef struct VUI {
	AVRational sar;

	int overscan_info_present_flag;
	int overscan_appropriate_flag;

	int video_signal_type_present_flag;
	int video_format;
	int video_full_range_flag;
	int colour_description_present_flag;
	uint8_t colour_primaries;
	uint8_t transfer_characteristic;
	uint8_t matrix_coeffs;

	int chroma_loc_info_present_flag;
	int chroma_sample_loc_type_top_field;
	int chroma_sample_loc_type_bottom_field;
	int neutra_chroma_indication_flag;

	int field_seq_flag;
	int frame_field_info_present_flag;

	int default_display_window_flag;
	HEVCWindow def_disp_win;

	int vui_timing_info_present_flag;
	uint32_t vui_num_units_in_tick;
	uint32_t vui_time_scale;
	int vui_poc_proportional_to_timing_flag;
	int vui_num_ticks_poc_diff_one_minus1;
	int vui_hrd_parameters_present_flag;

	int bitstream_restriction_flag;
	int tiles_fixed_structure_flag;
	int motion_vectors_over_pic_boundaries_flag;
	int restricted_ref_pic_lists_flag;
	int min_spatial_segmentation_idc;
	int max_bytes_per_pic_denom;
	int max_bits_per_min_cu_denom;
	int log2_max_mv_length_horizontal;
	int log2_max_mv_length_vertical;
} VUI;

typedef struct PTLCommon {
	uint8_t profile_space;
	uint8_t tier_flag;
	uint8_t profile_idc;
	uint8_t profile_compatibility_flag[32];
	uint8_t level_idc;
	uint8_t progressive_source_flag;
	uint8_t interlaced_source_flag;
	uint8_t non_packed_constraint_flag;
	uint8_t frame_only_constraint_flag;
} PTLCommon;

typedef struct PTL {
	PTLCommon general_ptl;
	PTLCommon sub_layer_ptl[MAX_SUB_LAYERS];

	uint8_t sub_layer_profile_present_flag[MAX_SUB_LAYERS];
	uint8_t sub_layer_level_present_flag[MAX_SUB_LAYERS];
} PTL;

typedef struct HEVCVPS {
	uint8_t vps_temporal_id_nesting_flag;
	int vps_max_layers;
	int vps_max_sub_layers; ///< vps_max_temporal_layers_minus1 + 1

	PTL ptl;
	int vps_sub_layer_ordering_info_present_flag;
	unsigned int vps_max_dec_pic_buffering[MAX_SUB_LAYERS];
	unsigned int vps_num_reorder_pics[MAX_SUB_LAYERS];
	unsigned int vps_max_latency_increase[MAX_SUB_LAYERS];
	int vps_max_layer_id;
	int vps_num_layer_sets; ///< vps_num_layer_sets_minus1 + 1
	uint8_t vps_timing_info_present_flag;
	uint32_t vps_num_units_in_tick;
	uint32_t vps_time_scale;
	uint8_t vps_poc_proportional_to_timing_flag;
	int vps_num_ticks_poc_diff_one; ///< vps_num_ticks_poc_diff_one_minus1 + 1
	int vps_num_hrd_parameters;
} HEVCVPS;

typedef struct ScalingList {
	/* This is a little wasteful, since sizeID 0 only needs 8 coeffs,
	* and size ID 3 only has 2 arrays, not 6. */
	uint8_t sl[4][6][64];
	uint8_t sl_dc[2][6];
} ScalingList;

typedef struct HEVCSPS {
	unsigned vps_id;
	int chroma_format_idc;
	uint8_t separate_colour_plane_flag;

	///< output (i.e. cropped) values
	int output_width, output_height;
	HEVCWindow output_window;

	HEVCWindow pic_conf_win;

	int bit_depth;
	int pixel_shift;
	enum AVPixelFormat pix_fmt;

	unsigned int log2_max_poc_lsb;
	int pcm_enabled_flag;

	int max_sub_layers;
	struct {
		int max_dec_pic_buffering;
		int num_reorder_pics;
		int max_latency_increase;
	} temporal_layer[MAX_SUB_LAYERS];

	VUI vui;
	PTL ptl;

	uint8_t scaling_list_enable_flag;
	ScalingList scaling_list;

	unsigned int nb_st_rps;
	ShortTermRPS st_rps[MAX_SHORT_TERM_RPS_COUNT];

	uint8_t amp_enabled_flag;
	uint8_t sao_enabled;

	uint8_t long_term_ref_pics_present_flag;
	uint16_t lt_ref_pic_poc_lsb_sps[32];
	uint8_t used_by_curr_pic_lt_sps_flag[32];
	uint8_t num_long_term_ref_pics_sps;

	struct {
		uint8_t bit_depth;
		uint8_t bit_depth_chroma;
		unsigned int log2_min_pcm_cb_size;
		unsigned int log2_max_pcm_cb_size;
		uint8_t loop_filter_disable_flag;
	} pcm;
	uint8_t sps_temporal_mvp_enabled_flag;
	uint8_t sps_strong_intra_smoothing_enable_flag;

	unsigned int log2_min_cb_size;
	unsigned int log2_diff_max_min_coding_block_size;
	unsigned int log2_min_tb_size;
	unsigned int log2_max_trafo_size;
	unsigned int log2_ctb_size;
	unsigned int log2_min_pu_size;

	int max_transform_hierarchy_depth_inter;
	int max_transform_hierarchy_depth_intra;

	int transform_skip_rotation_enabled_flag;
	int transform_skip_context_enabled_flag;
	int implicit_rdpcm_enabled_flag;
	int explicit_rdpcm_enabled_flag;
	int intra_smoothing_disabled_flag;
	int persistent_rice_adaptation_enabled_flag;

	///< coded frame dimension in various units
	int width;
	int height;
	int ctb_width;
	int ctb_height;
	int ctb_size;
	int min_cb_width;
	int min_cb_height;
	int min_tb_width;
	int min_tb_height;
	int min_pu_width;
	int min_pu_height;
	int tb_mask;

	int hshift[3];
	int vshift[3];

	int qp_bd_offset;
} HEVCSPS;

typedef struct HEVCPPS {
	unsigned int sps_id; ///< seq_parameter_set_id

	uint8_t sign_data_hiding_flag;

	uint8_t cabac_init_present_flag;

	int num_ref_idx_l0_default_active; ///< num_ref_idx_l0_default_active_minus1 + 1
	int num_ref_idx_l1_default_active; ///< num_ref_idx_l1_default_active_minus1 + 1
	int pic_init_qp_minus26;

	uint8_t constrained_intra_pred_flag;
	uint8_t transform_skip_enabled_flag;

	uint8_t cu_qp_delta_enabled_flag;
	int diff_cu_qp_delta_depth;

	int cb_qp_offset;
	int cr_qp_offset;
	uint8_t pic_slice_level_chroma_qp_offsets_present_flag;
	uint8_t weighted_pred_flag;
	uint8_t weighted_bipred_flag;
	uint8_t output_flag_present_flag;
	uint8_t transquant_bypass_enable_flag;

	uint8_t dependent_slice_segments_enabled_flag;
	uint8_t tiles_enabled_flag;
	uint8_t entropy_coding_sync_enabled_flag;

	int num_tile_columns;   ///< num_tile_columns_minus1 + 1
	int num_tile_rows;      ///< num_tile_rows_minus1 + 1
	uint8_t uniform_spacing_flag;
	uint8_t loop_filter_across_tiles_enabled_flag;

	uint8_t seq_loop_filter_across_slices_enabled_flag;

	uint8_t deblocking_filter_control_present_flag;
	uint8_t deblocking_filter_override_enabled_flag;
	uint8_t disable_dbf;
	int beta_offset;    ///< beta_offset_div2 * 2
	int tc_offset;      ///< tc_offset_div2 * 2

	uint8_t scaling_list_data_present_flag;
	ScalingList scaling_list;

	uint8_t lists_modification_present_flag;
	int log2_parallel_merge_level; ///< log2_parallel_merge_level_minus2 + 2
	int num_extra_slice_header_bits;
	uint8_t slice_header_extension_present_flag;
	uint8_t log2_max_transform_skip_block_size;
	uint8_t cross_component_prediction_enabled_flag;
	uint8_t chroma_qp_offset_list_enabled_flag;
	uint8_t diff_cu_chroma_qp_offset_depth;
	uint8_t chroma_qp_offset_list_len_minus1;
	int8_t  cb_qp_offset_list[5];
	int8_t  cr_qp_offset_list[5];
	uint8_t log2_sao_offset_scale_luma;
	uint8_t log2_sao_offset_scale_chroma;

	// Inferred parameters
	unsigned int *column_width;  ///< ColumnWidth
	unsigned int *row_height;    ///< RowHeight
	unsigned int *col_bd;        ///< ColBd
	unsigned int *row_bd;        ///< RowBd
	int *col_idxX;

	int *ctb_addr_rs_to_ts; ///< CtbAddrRSToTS
	int *ctb_addr_ts_to_rs; ///< CtbAddrTSToRS
	int *tile_id;           ///< TileId
	int *tile_pos_rs;       ///< TilePosRS
	int *min_tb_addr_zs;    ///< MinTbAddrZS
	int *min_tb_addr_zs_tab;///< MinTbAddrZS
} HEVCPPS;

typedef struct HEVCParamSets {
	AVBufferRef *vps_list[MAX_VPS_COUNT];
	AVBufferRef *sps_list[MAX_SPS_COUNT];
	AVBufferRef *pps_list[MAX_PPS_COUNT];

	/* currently active parameter sets */
	const HEVCVPS *vps;
	const HEVCSPS *sps;
	const HEVCPPS *pps;
} HEVCParamSets;

typedef struct SliceHeader {
	unsigned int pps_id;

	///< address (in raster order) of the first block in the current slice segment
	unsigned int   slice_segment_addr;
	///< address (in raster order) of the first block in the current slice
	unsigned int   slice_addr;

	enum SliceType slice_type;

	int pic_order_cnt_lsb;

	uint8_t first_slice_in_pic_flag;
	uint8_t dependent_slice_segment_flag;
	uint8_t pic_output_flag;
	uint8_t colour_plane_id;

	///< RPS coded in the slice header itself is stored here
	int short_term_ref_pic_set_sps_flag;
	int short_term_ref_pic_set_size;
	ShortTermRPS slice_rps;
	const ShortTermRPS *short_term_rps;
	int long_term_ref_pic_set_size;
	LongTermRPS long_term_rps;
	unsigned int list_entry_lx[2][32];

	uint8_t rpl_modification_flag[2];
	uint8_t no_output_of_prior_pics_flag;
	uint8_t slice_temporal_mvp_enabled_flag;

	unsigned int nb_refs[2];

	uint8_t slice_sample_adaptive_offset_flag[3];
	uint8_t mvd_l1_zero_flag;

	uint8_t cabac_init_flag;
	uint8_t disable_deblocking_filter_flag; ///< slice_header_disable_deblocking_filter_flag
	uint8_t slice_loop_filter_across_slices_enabled_flag;
	uint8_t collocated_list;

	unsigned int collocated_ref_idx;

	int slice_qp_delta;
	int slice_cb_qp_offset;
	int slice_cr_qp_offset;

	uint8_t cu_chroma_qp_offset_enabled_flag;

	int beta_offset;    ///< beta_offset_div2 * 2
	int tc_offset;      ///< tc_offset_div2 * 2

	unsigned int max_num_merge_cand; ///< 5 - 5_minus_max_num_merge_cand

	unsigned *entry_point_offset;
	int * offset;
	int * size;
	int num_entry_point_offsets;

	int8_t slice_qp;

	uint8_t luma_log2_weight_denom;
	int16_t chroma_log2_weight_denom;

	int16_t luma_weight_l0[16];
	int16_t chroma_weight_l0[16][2];
	int16_t chroma_weight_l1[16][2];
	int16_t luma_weight_l1[16];

	int16_t luma_offset_l0[16];
	int16_t chroma_offset_l0[16][2];

	int16_t luma_offset_l1[16];
	int16_t chroma_offset_l1[16][2];

	int slice_ctb_addr_rs;
} SliceHeader;

typedef struct CodingUnit {
	int x;
	int y;

	enum PredMode pred_mode;    ///< PredMode
	enum PartMode part_mode;    ///< PartMode

								// Inferred parameters
	uint8_t intra_split_flag;   ///< IntraSplitFlag
	uint8_t max_trafo_depth;    ///< MaxTrafoDepth
	uint8_t cu_transquant_bypass_flag;
} CodingUnit;

typedef struct Mv {
	int16_t x;  ///< horizontal component of motion vector
	int16_t y;  ///< vertical component of motion vector
} Mv;

typedef struct MvField {
	DECLARE_ALIGNED(4, Mv, mv)[2];
	int8_t ref_idx[2];
	int8_t pred_flag;
} MvField;

typedef struct NeighbourAvailable {
	int cand_bottom_left;
	int cand_left;
	int cand_up;
	int cand_up_left;
	int cand_up_right;
	int cand_up_right_sap;
} NeighbourAvailable;

typedef struct PredictionUnit {
	int mpm_idx;
	int rem_intra_luma_pred_mode;
	uint8_t intra_pred_mode[4];
	Mv mvd;
	uint8_t merge_flag;
	uint8_t intra_pred_mode_c[4];
	uint8_t chroma_mode_c[4];
} PredictionUnit;

typedef struct TransformUnit {
	int cu_qp_delta;

	int res_scale_val;

	// Inferred parameters;
	int intra_pred_mode;
	int intra_pred_mode_c;
	int chroma_mode_c;
	uint8_t is_cu_qp_delta_coded;
	uint8_t is_cu_chroma_qp_offset_coded;
	int8_t  cu_qp_offset_cb;
	int8_t  cu_qp_offset_cr;
	uint8_t cross_pf;
} TransformUnit;

typedef struct DBParams {
	int beta_offset;
	int tc_offset;
} DBParams;

#define HEVC_FRAME_FLAG_OUTPUT    (1 << 0)
#define HEVC_FRAME_FLAG_SHORT_REF (1 << 1)
#define HEVC_FRAME_FLAG_LONG_REF  (1 << 2)
#define HEVC_FRAME_FLAG_BUMPING   (1 << 3)


typedef struct HEVCFrame {
	AVFrame *frame;
	ThreadFrame tf;
	MvField *tab_mvf;
	RefPicList *refPicList;
	RefPicListTab **rpl_tab;
	int ctb_count;
	int poc;
	struct HEVCFrame *collocated_ref;

	HEVCWindow window;

	AVBufferRef *tab_mvf_buf;
	AVBufferRef *rpl_tab_buf;
	AVBufferRef *rpl_buf;

	AVBufferRef *hwaccel_priv_buf;
	void *hwaccel_picture_private;

	/**
	* A sequence counter, so that old frames are output first
	* after a POC reset
	*/
	uint16_t sequence;

	/**
	* A combination of HEVC_FRAME_FLAG_*
	*/
	uint8_t flags;
} HEVCFrame;


typedef struct HEVCNAL {
	uint8_t *rbsp_buffer;
	int rbsp_buffer_size;

	int size;
	const uint8_t *data;

	int raw_size;
	const uint8_t *raw_data;

	GetBitContext gb;

	enum NALUnitType type;
	int temporal_id;

	int skipped_bytes;
	int skipped_bytes_pos_size;
	int *skipped_bytes_pos;
} HEVCNAL;

/* an input packet split into unescaped NAL units */
typedef struct HEVCPacket {
	HEVCNAL *nals;
	int nb_nals;
	int nals_allocated;
} HEVCPacket;

typedef struct HEVCLocalContext {
	uint8_t cabac_state[HEVC_CONTEXTS];

	uint8_t stat_coeff[4];

	uint8_t first_qp_group;

	GetBitContext gb;
	CABACContext cc;

	int8_t qp_y;
	int8_t curr_qp_y;

	int qPy_pred;

	TransformUnit tu;

	uint8_t ctb_left_flag;
	uint8_t ctb_up_flag;
	uint8_t ctb_up_right_flag;
	uint8_t ctb_up_left_flag;
	int     end_of_tiles_x;
	int     end_of_tiles_y;
	/* +7 is for subpixel interpolation, *2 for high bit depths */
	DECLARE_ALIGNED(32, uint8_t, edge_emu_buffer)[(MAX_PB_SIZE + 7) * EDGE_EMU_BUFFER_STRIDE * 2];
	/* The extended size between the new edge emu buffer is abused by SAO */
	DECLARE_ALIGNED(32, uint8_t, edge_emu_buffer2)[(MAX_PB_SIZE + 7) * EDGE_EMU_BUFFER_STRIDE * 2];
	DECLARE_ALIGNED(32, int16_t, tmp[MAX_PB_SIZE * MAX_PB_SIZE]);

	int ct_depth;
	CodingUnit cu;
	PredictionUnit pu;
	NeighbourAvailable na;

#define BOUNDARY_LEFT_SLICE     (1 << 0)
#define BOUNDARY_LEFT_TILE      (1 << 1)
#define BOUNDARY_UPPER_SLICE    (1 << 2)
#define BOUNDARY_UPPER_TILE     (1 << 3)
	/* properties of the boundary of the current CTB for the purposes
	* of the deblocking filter */
	int boundary_flags;
} HEVCLocalContext;

typedef struct HEVCContext {
	const AVClass *c;  // needed by private avoptions
	AVCodecContext *avctx;

	struct HEVCContext  *sList[MAX_NB_THREADS];

	HEVCLocalContext    *HEVClcList[MAX_NB_THREADS];
	HEVCLocalContext    *HEVClc;

	uint8_t             threads_type;
	uint8_t             threads_number;

	int                 width;
	int                 height;

	uint8_t *cabac_state;

	/** 1 if the independent slice segment header was successfully parsed */
	uint8_t slice_initialized;

	AVFrame *frame;
	AVFrame *output_frame;
	uint8_t *sao_pixel_buffer_h[3];
	uint8_t *sao_pixel_buffer_v[3];

	HEVCParamSets ps;

	AVBufferPool *tab_mvf_pool;
	AVBufferPool *rpl_tab_pool;

	///< candidate references for the current frame
	RefPicList rps[5];

	SliceHeader sh;
	SAOParams *sao;

	DBParams *deblock;
	enum NALUnitType nal_unit_type;
	int temporal_id;  ///< temporal_id_plus1 - 1
	HEVCFrame *ref;
	HEVCFrame DPB[32];
	int poc;
	int pocTid0;
	int slice_idx; ///< number of the slice being currently decoded
	int eos;       ///< current packet contains an EOS/EOB NAL
	int last_eos;  ///< last packet contains an EOS/EOB NAL
	int max_ra;
	int bs_width;
	int bs_height;

	int is_decoded;
	int no_rasl_output_flag;

	HEVCPredContext hpc;
	HEVCDSPContext hevcdsp;
	VideoDSPContext vdsp;
	BswapDSPContext bdsp;
	int8_t *qp_y_tab;
	uint8_t *horizontal_bs;
	uint8_t *vertical_bs;

	int32_t *tab_slice_address;

	//  CU
	uint8_t *skip_flag;
	uint8_t *tab_ct_depth;
	// PU
	uint8_t *tab_ipm;

	uint8_t *cbf_luma; // cbf_luma of colocated TU
	uint8_t *is_pcm;

	// CTB-level flags affecting loop filter operation
	uint8_t *filter_slice_edges;

	/** used on BE to byteswap the lines for checksumming */
	uint8_t *checksum_buf;
	int      checksum_buf_size;

	/**
	* Sequence counters for decoded and output frames, so that old
	* frames are output first after a POC reset
	*/
	uint16_t seq_decode;
	uint16_t seq_output;

	int enable_parallel_tiles;
	int wpp_err;

	const uint8_t *data;

	HEVCPacket pkt;
	// type of the first VCL NAL of the current frame
	enum NALUnitType first_nal_type;

	// for checking the frame checksums
	struct AVMD5 *md5_ctx;
	uint8_t       md5[3][16];
	uint8_t is_md5;

	uint8_t context_initialized;
	uint8_t is_nalff;       ///< this flag is != 0 if bitstream is encapsulated
							///< as a format defined in 14496-15
	int apply_defdispwin;

	int active_seq_parameter_set_id;

	int nal_length_size;    ///< Number of bytes used for nal length (1, 2 or 4)
	int nuh_layer_id;

	/** frame packing arrangement variables */
	int sei_frame_packing_present;
	int frame_packing_arrangement_type;
	int content_interpretation_type;
	int quincunx_subsampling;

	/** display orientation */
	int sei_display_orientation_present;
	int sei_anticlockwise_rotation;
	int sei_hflip, sei_vflip;

	int picture_struct;

	uint8_t* a53_caption;
	int a53_caption_size;

	/** mastering display */
	int sei_mastering_display_info_present;
	uint16_t display_primaries[3][2];
	uint16_t white_point[2];
	uint32_t max_mastering_luminance;
	uint32_t min_mastering_luminance;

} HEVCContext;

///////////////////////////////////
//For H.264
///////////////////////////////////


//h264dsp.h
typedef void(*h264_weight_func)(uint8_t *block, int stride, int height,
	int log2_denom, int weight, int offset);
typedef void(*h264_biweight_func)(uint8_t *dst, uint8_t *src,
	int stride, int height, int log2_denom,
	int weightd, int weights, int offset);

/**
* Context for storing H.264 DSP functions
*/
typedef struct H264DSPContext {
	/* weighted MC */
	h264_weight_func weight_h264_pixels_tab[4];
	h264_biweight_func biweight_h264_pixels_tab[4];

	/* loop filter */
	void(*h264_v_loop_filter_luma)(uint8_t *pix /*align 16*/, int stride,
		int alpha, int beta, int8_t *tc0);
	void(*h264_h_loop_filter_luma)(uint8_t *pix /*align 4 */, int stride,
		int alpha, int beta, int8_t *tc0);
	void(*h264_h_loop_filter_luma_mbaff)(uint8_t *pix /*align 16*/, int stride,
		int alpha, int beta, int8_t *tc0);
	/* v/h_loop_filter_luma_intra: align 16 */
	void(*h264_v_loop_filter_luma_intra)(uint8_t *pix, int stride,
		int alpha, int beta);
	void(*h264_h_loop_filter_luma_intra)(uint8_t *pix, int stride,
		int alpha, int beta);
	void(*h264_h_loop_filter_luma_mbaff_intra)(uint8_t *pix /*align 16*/,
		int stride, int alpha, int beta);
	void(*h264_v_loop_filter_chroma)(uint8_t *pix /*align 8*/, int stride,
		int alpha, int beta, int8_t *tc0);
	void(*h264_h_loop_filter_chroma)(uint8_t *pix /*align 4*/, int stride,
		int alpha, int beta, int8_t *tc0);
	void(*h264_h_loop_filter_chroma_mbaff)(uint8_t *pix /*align 8*/,
		int stride, int alpha, int beta,
		int8_t *tc0);
	void(*h264_v_loop_filter_chroma_intra)(uint8_t *pix /*align 8*/,
		int stride, int alpha, int beta);
	void(*h264_h_loop_filter_chroma_intra)(uint8_t *pix /*align 8*/,
		int stride, int alpha, int beta);
	void(*h264_h_loop_filter_chroma_mbaff_intra)(uint8_t *pix /*align 8*/,
		int stride, int alpha, int beta);
	// h264_loop_filter_strength: simd only. the C version is inlined in h264.c
	void(*h264_loop_filter_strength)(int16_t bS[2][4][4], uint8_t nnz[40],
		int8_t ref[2][40], int16_t mv[2][40][2],
		int bidir, int edges, int step,
		int mask_mv0, int mask_mv1, int field);

	/* IDCT */
	void(*h264_idct_add)(uint8_t *dst /*align 4*/,
		int16_t *block /*align 16*/, int stride);
	void(*h264_idct8_add)(uint8_t *dst /*align 8*/,
		int16_t *block /*align 16*/, int stride);
	void(*h264_idct_dc_add)(uint8_t *dst /*align 4*/,
		int16_t *block /*align 16*/, int stride);
	void(*h264_idct8_dc_add)(uint8_t *dst /*align 8*/,
		int16_t *block /*align 16*/, int stride);

	void(*h264_idct_add16)(uint8_t *dst /*align 16*/, const int *blockoffset,
		int16_t *block /*align 16*/, int stride,
		const uint8_t nnzc[15 * 8]);
	void(*h264_idct8_add4)(uint8_t *dst /*align 16*/, const int *blockoffset,
		int16_t *block /*align 16*/, int stride,
		const uint8_t nnzc[15 * 8]);
	void(*h264_idct_add8)(uint8_t **dst /*align 16*/, const int *blockoffset,
		int16_t *block /*align 16*/, int stride,
		const uint8_t nnzc[15 * 8]);
	void(*h264_idct_add16intra)(uint8_t *dst /*align 16*/, const int *blockoffset,
		int16_t *block /*align 16*/,
		int stride, const uint8_t nnzc[15 * 8]);
	void(*h264_luma_dc_dequant_idct)(int16_t *output,
		int16_t *input /*align 16*/, int qmul);
	void(*h264_chroma_dc_dequant_idct)(int16_t *block, int qmul);

	/* bypass-transform */
	void(*h264_add_pixels8_clear)(uint8_t *dst, int16_t *block, int stride);
	void(*h264_add_pixels4_clear)(uint8_t *dst, int16_t *block, int stride);

	/**
	* Search buf from the start for up to size bytes. Return the index
	* of a zero byte, or >= size if not found. Ideally, use lookahead
	* to filter out any zero bytes that are known to not be followed by
	* one or more further zero bytes and a one byte. Better still, filter
	* out any bytes that form the trailing_zero_8bits syntax element too.
	*/
	int(*startcode_find_candidate)(const uint8_t *buf, int size);
} H264DSPContext;

//h264chroma.h
typedef void(*h264_chroma_mc_func)(uint8_t *dst/*align 8*/, uint8_t *src/*align 1*/, int srcStride, int h, int x, int y);

typedef struct H264ChromaContext {
	h264_chroma_mc_func put_h264_chroma_pixels_tab[4];
	h264_chroma_mc_func avg_h264_chroma_pixels_tab[4];
} H264ChromaContext;

//qpeldsp.h
typedef void(*qpel_mc_func)(uint8_t *dst /* align width (8 or 16) */,
	const uint8_t *src /* align 1 */,
	ptrdiff_t stride);

//h264qpel.h
typedef struct H264QpelContext {
	qpel_mc_func put_h264_qpel_pixels_tab[4][16];
	qpel_mc_func avg_h264_qpel_pixels_tab[4][16];
} H264QpelContext;

//h264pred.h
typedef struct H264PredContext {
	void(*pred4x4[9 + 3 + 3])(uint8_t *src, const uint8_t *topright,
		ptrdiff_t stride);
	void(*pred8x8l[9 + 3])(uint8_t *src, int topleft, int topright,
		ptrdiff_t stride);
	void(*pred8x8[4 + 3 + 4])(uint8_t *src, ptrdiff_t stride);
	void(*pred16x16[4 + 3 + 2])(uint8_t *src, ptrdiff_t stride);

	void(*pred4x4_add[2])(uint8_t *pix /*align  4*/,
		int16_t *block /*align 16*/, ptrdiff_t stride);
	void(*pred8x8l_add[2])(uint8_t *pix /*align  8*/,
		int16_t *block /*align 16*/, ptrdiff_t stride);
	void(*pred8x8l_filter_add[2])(uint8_t *pix /*align  8*/,
		int16_t *block /*align 16*/, int topleft, int topright, ptrdiff_t stride);
	void(*pred8x8_add[3])(uint8_t *pix /*align  8*/,
		const int *block_offset,
		int16_t *block /*align 16*/, ptrdiff_t stride);
	void(*pred16x16_add[3])(uint8_t *pix /*align 16*/,
		const int *block_offset,
		int16_t *block /*align 16*/, ptrdiff_t stride);
} H264PredContext;


#define H264_MAX_PICTURE_COUNT 36
#define H264_MAX_THREADS       32

//#define MAX_SPS_COUNT          32
//#define MAX_PPS_COUNT         256

#define MAX_MMCO_COUNT         66

#define MAX_DELAYED_PIC_COUNT  16

#define MAX_MBPAIR_SIZE (256*1024) // a tighter bound could be calculated if someone cares about a few bytes

/* Compiling in interlaced support reduces the speed
* of progressive decoding by about 2%. */
#define ALLOW_INTERLACE

#define FMO 0

/**
* The maximum number of slices supported by the decoder.
* must be a power of 2
*/
#define MAX_SLICES 32

#ifdef ALLOW_INTERLACE
#define MB_MBAFF(h)    (h)->mb_mbaff
#define MB_FIELD(sl)  (sl)->mb_field_decoding_flag
#define FRAME_MBAFF(h) (h)->mb_aff_frame
#define FIELD_PICTURE(h) ((h)->picture_structure != PICT_FRAME)
#define LEFT_MBS 2
#define LTOP     0
#define LBOT     1
#define LEFT(i)  (i)
#else
#define MB_MBAFF(h)      0
#define MB_FIELD(sl)     0
#define FRAME_MBAFF(h)   0
#define FIELD_PICTURE(h) 0
#undef  IS_INTERLACED
#define IS_INTERLACED(mb_type) 0
#define LEFT_MBS 1
#define LTOP     0
#define LBOT     0
#define LEFT(i)  0
#endif
#define FIELD_OR_MBAFF_PICTURE(h) (FRAME_MBAFF(h) || FIELD_PICTURE(h))

#ifndef CABAC
#define CABAC(h) (h)->pps.cabac
#endif

#define CHROMA(h)    ((h)->sps.chroma_format_idc)
#define CHROMA422(h) ((h)->sps.chroma_format_idc == 2)
#define CHROMA444(h) ((h)->sps.chroma_format_idc == 3)

#define EXTENDED_SAR       255

#define MB_TYPE_REF0       MB_TYPE_ACPRED // dirty but it fits in 16 bit
#define MB_TYPE_8x8DCT     0x01000000
#define IS_REF0(a)         ((a) & MB_TYPE_REF0)
#define IS_8x8DCT(a)       ((a) & MB_TYPE_8x8DCT)

#define QP_MAX_NUM (51 + 6*6)           // The maximum supported qp

/* NAL unit types */
enum {
	NAL_H264_SLICE = 1,
	NAL_H264_DPA = 2,
	NAL_H264_DPB = 3,
	NAL_H264_DPC = 4,
	NAL_H264_IDR_SLICE = 5,
	NAL_H264_SEI = 6,
	NAL_H264_SPS = 7,
	NAL_H264_PPS = 8,
	NAL_H264_AUD = 9,
	NAL_H264_END_SEQUENCE = 10,
	NAL_H264_END_STREAM = 11,
	NAL_H264_FILLER_DATA = 12,
	NAL_H264_SPS_EXT = 13,
	NAL_H264_AUXILIARY_SLICE = 19,
	NAL_H264_FF_IGNORE = 0xff0f001,
};


/**
* Sequence parameter set
*/
typedef struct SPS {
	unsigned int sps_id;
	int profile_idc;
	int level_idc;
	int chroma_format_idc;
	int transform_bypass;              ///< qpprime_y_zero_transform_bypass_flag
	int log2_max_frame_num;            ///< log2_max_frame_num_minus4 + 4
	int poc_type;                      ///< pic_order_cnt_type
	int log2_max_poc_lsb;              ///< log2_max_pic_order_cnt_lsb_minus4
	int delta_pic_order_always_zero_flag;
	int offset_for_non_ref_pic;
	int offset_for_top_to_bottom_field;
	int poc_cycle_length;              ///< num_ref_frames_in_pic_order_cnt_cycle
	int ref_frame_count;               ///< num_ref_frames
	int gaps_in_frame_num_allowed_flag;
	int mb_width;                      ///< pic_width_in_mbs_minus1 + 1
	int mb_height;                     ///< pic_height_in_map_units_minus1 + 1
	int frame_mbs_only_flag;
	int mb_aff;                        ///< mb_adaptive_frame_field_flag
	int direct_8x8_inference_flag;
	int crop;                          ///< frame_cropping_flag

									   /* those 4 are already in luma samples */
	unsigned int crop_left;            ///< frame_cropping_rect_left_offset
	unsigned int crop_right;           ///< frame_cropping_rect_right_offset
	unsigned int crop_top;             ///< frame_cropping_rect_top_offset
	unsigned int crop_bottom;          ///< frame_cropping_rect_bottom_offset
	int vui_parameters_present_flag;
	AVRational sar;
	int video_signal_type_present_flag;
	int full_range;
	int colour_description_present_flag;
	enum AVColorPrimaries color_primaries;
	enum AVColorTransferCharacteristic color_trc;
	enum AVColorSpace colorspace;
	int timing_info_present_flag;
	uint32_t num_units_in_tick;
	uint32_t time_scale;
	int fixed_frame_rate_flag;
	short offset_for_ref_frame[256]; // FIXME dyn aloc?
	int bitstream_restriction_flag;
	int num_reorder_frames;
	int scaling_matrix_present;
	uint8_t scaling_matrix4[6][16];
	uint8_t scaling_matrix8[6][64];
	int nal_hrd_parameters_present_flag;
	int vcl_hrd_parameters_present_flag;
	int pic_struct_present_flag;
	int time_offset_length;
	int cpb_cnt;                          ///< See H.264 E.1.2
	int initial_cpb_removal_delay_length; ///< initial_cpb_removal_delay_length_minus1 + 1
	int cpb_removal_delay_length;         ///< cpb_removal_delay_length_minus1 + 1
	int dpb_output_delay_length;          ///< dpb_output_delay_length_minus1 + 1
	int bit_depth_luma;                   ///< bit_depth_luma_minus8 + 8
	int bit_depth_chroma;                 ///< bit_depth_chroma_minus8 + 8
	int residual_color_transform_flag;    ///< residual_colour_transform_flag
	int constraint_set_flags;             ///< constraint_set[0-3]_flag
	int new0;                              ///< flag to keep track if the decoder context needs re-init due to changed SPS
	uint8_t data[4096];
	size_t data_size;
} SPS;

/**
* Picture parameter set
*/
typedef struct PPS {
	unsigned int sps_id;
	int cabac;                  ///< entropy_coding_mode_flag
	int pic_order_present;      ///< pic_order_present_flag
	int slice_group_count;      ///< num_slice_groups_minus1 + 1
	int mb_slice_group_map_type;
	unsigned int ref_count[2];  ///< num_ref_idx_l0/1_active_minus1 + 1
	int weighted_pred;          ///< weighted_pred_flag
	int weighted_bipred_idc;
	int init_qp;                ///< pic_init_qp_minus26 + 26
	int init_qs;                ///< pic_init_qs_minus26 + 26
	int chroma_qp_index_offset[2];
	int deblocking_filter_parameters_present; ///< deblocking_filter_parameters_present_flag
	int constrained_intra_pred;     ///< constrained_intra_pred_flag
	int redundant_pic_cnt_present;  ///< redundant_pic_cnt_present_flag
	int transform_8x8_mode;         ///< transform_8x8_mode_flag
	uint8_t scaling_matrix4[6][16];
	uint8_t scaling_matrix8[6][64];
	uint8_t chroma_qp_table[2][QP_MAX_NUM + 1];  ///< pre-scaled (with chroma_qp_index_offset) version of qp_table
	int chroma_qp_diff;
	uint8_t data[4096];
	size_t data_size;
} PPS;

typedef struct H264Picture {
	AVFrame *f;
	ThreadFrame tf;

	AVBufferRef *qscale_table_buf;
	int8_t *qscale_table;

	AVBufferRef *motion_val_buf[2];
	int16_t(*motion_val[2])[2];

	AVBufferRef *mb_type_buf;
	uint32_t *mb_type;

	AVBufferRef *hwaccel_priv_buf;
	void *hwaccel_picture_private; ///< hardware accelerator private data

	AVBufferRef *ref_index_buf[2];
	int8_t *ref_index[2];

	int field_poc[2];       ///< top/bottom POC
	int poc;                ///< frame POC
	int frame_num;          ///< frame_num (raw frame_num from slice header)
	int mmco_reset;         /**< MMCO_RESET set this 1. Reordering code must
							not mix pictures before and after MMCO_RESET. */
	int pic_id;             /**< pic_num (short -> no wrap version of pic_num,
							pic_num & max_pic_num; long -> long_pic_num) */
	int long_ref;           ///< 1->long term reference 0->short term reference
	int ref_poc[2][2][32];  ///< POCs of the frames/fields used as reference (FIXME need per slice)
	int ref_count[2][2];    ///< number of entries in ref_poc         (FIXME need per slice)
	int mbaff;              ///< 1 -> MBAFF frame 0-> not MBAFF
	int field_picture;      ///< whether or not picture was encoded in separate fields

	int reference;
	int recovered;          ///< picture at IDR or recovery point + recovery count
	int invalid_gap;
	int sei_recovery_frame_cnt;

	int crop;
	int crop_left;
	int crop_top;
} H264Picture;

typedef struct H264Ref {
	uint8_t *data[3];
	int linesize[3];

	int reference;
	int poc;
	int pic_id;

	H264Picture *parent;
} H264Ref;


/**
* H264Context
*/
typedef struct H264Context {
	AVClass *av_class;
	AVCodecContext *avctx;
	VideoDSPContext vdsp;
	H264DSPContext h264dsp;
	H264ChromaContext h264chroma;
	H264QpelContext h264qpel;
	GetBitContext gb;

	H264Picture DPB[H264_MAX_PICTURE_COUNT];
	H264Picture *cur_pic_ptr;
	H264Picture cur_pic;
	H264Picture last_pic_for_ec;
#if 0
	H264SliceContext *slice_ctx;
#else
	void *slice_ctx;
#endif
	int            nb_slice_ctx;

	int pixel_shift;    ///< 0 for 8-bit H264, 1 for high-bit-depth H264

						/* coded dimensions -- 16 * mb w/h */
	int width, height;
	int chroma_x_shift, chroma_y_shift;

	/**
	* Backup frame properties: needed, because they can be different
	* between returned frame and last decoded frame.
	**/
	int backup_width;
	int backup_height;
	enum AVPixelFormat backup_pix_fmt;

	int droppable;
	int coded_picture_number;
	int low_delay;

	int context_initialized;
	int flags;
	int workaround_bugs;

	int8_t(*intra4x4_pred_mode);
	H264PredContext hpc;

	uint8_t(*non_zero_count)[48];

#define LIST_NOT_USED -1 // FIXME rename?
#define PART_NOT_AVAILABLE -2

	/**
	* block_offset[ 0..23] for frame macroblocks
	* block_offset[24..47] for field macroblocks
	*/
	int block_offset[2 * (16 * 3)];

	uint32_t *mb2b_xy;  // FIXME are these 4 a good idea?
	uint32_t *mb2br_xy;
	int b_stride;       // FIXME use s->b4_stride


	unsigned current_sps_id; ///< id of the current SPS
	SPS sps; ///< current sps
	PPS pps; ///< current pps

	int au_pps_id; ///< pps_id of current access unit

	uint32_t dequant4_buffer[6][QP_MAX_NUM + 1][16]; // FIXME should these be moved down?
	uint32_t dequant8_buffer[6][QP_MAX_NUM + 1][64];
	uint32_t(*dequant4_coeff[6])[16];
	uint32_t(*dequant8_coeff[6])[64];

	uint16_t *slice_table;      ///< slice_table_base + 2*mb_stride + 1

								// interlacing specific flags
	int mb_aff_frame;
	int picture_structure;
	int first_field;

	uint8_t *list_counts;               ///< Array of list_count per MB specifying the slice type

										/* 0x100 -> non null luma_dc, 0x80/0x40 -> non null chroma_dc (cb/cr), 0x?0 -> chroma_cbp(0, 1, 2), 0x0? luma_cbp */
	uint16_t *cbp_table;

	/* chroma_pred_mode for i4x4 or i16x16, else 0 */
	uint8_t *chroma_pred_mode_table;
	uint8_t(*mvd_table[2])[2];
	uint8_t *direct_table;

	uint8_t zigzag_scan[16];
	uint8_t zigzag_scan8x8[64];
	uint8_t zigzag_scan8x8_cavlc[64];
	uint8_t field_scan[16];
	uint8_t field_scan8x8[64];
	uint8_t field_scan8x8_cavlc[64];
	uint8_t zigzag_scan_q0[16];
	uint8_t zigzag_scan8x8_q0[64];
	uint8_t zigzag_scan8x8_cavlc_q0[64];
	uint8_t field_scan_q0[16];
	uint8_t field_scan8x8_q0[64];
	uint8_t field_scan8x8_cavlc_q0[64];

	int x264_build;

	int mb_y;
	int mb_height, mb_width;
	int mb_stride;
	int mb_num;

	// =============================================================
	// Things below are not used in the MB or more inner code

	int nal_ref_idc;
	int nal_unit_type;

	/**
	* Used to parse AVC variant of h264
	*/
	int is_avc;           ///< this flag is != 0 if codec is avc1
	int nal_length_size;  ///< Number of bytes used for nal length (1, 2 or 4)

	int bit_depth_luma;         ///< luma bit depth from sps to detect changes
	int chroma_format_idc;      ///< chroma format from sps to detect changes

	SPS *sps_buffers[MAX_SPS_COUNT];
	PPS *pps_buffers[MAX_PPS_COUNT];

	int dequant_coeff_pps;      ///< reinit tables when pps changes

	uint16_t *slice_table_base;

	// POC stuff
	int poc_lsb;
	int poc_msb;
	int delta_poc_bottom;
	int delta_poc[2];
	int frame_num;
	int prev_poc_msb;           ///< poc_msb of the last reference pic for POC type 0
	int prev_poc_lsb;           ///< poc_lsb of the last reference pic for POC type 0
	int frame_num_offset;       ///< for POC type 2
	int prev_frame_num_offset;  ///< for POC type 2
	int prev_frame_num;         ///< frame_num of the last pic for POC type 1/2

								/**
								* frame_num for frames or 2 * frame_num + 1 for field pics.
								*/
	int curr_pic_num;

	/**
	* max_frame_num or 2 * max_frame_num for field pics.
	*/
	int max_pic_num;

	H264Ref default_ref[2];
	H264Picture *short_ref[32];
	H264Picture *long_ref[32];
	H264Picture *delayed_pic[MAX_DELAYED_PIC_COUNT + 2]; // FIXME size?
	int last_pocs[MAX_DELAYED_PIC_COUNT];
	H264Picture *next_output_pic;
	int next_outputed_poc;
#if 0
	/**
	* memory management control operations buffer.
	*/
	MMCO mmco[MAX_MMCO_COUNT];
	int mmco_index;
	int mmco_reset;

	int long_ref_count;     ///< number of actual long term references
	int short_ref_count;    ///< number of actual short term references

							/**
							* @name Members for slice based multithreading
							* @{
							*/
							/**
							* current slice number, used to initialize slice_num of each thread/context
							*/
	int current_slice;

	/**
	* Max number of threads / contexts.
	* This is equal to AVCodecContext.thread_count unless
	* multithreaded decoding is impossible, in which case it is
	* reduced to 1.
	*/
	int max_contexts;

	int slice_context_count;

	/**
	*  1 if the single thread fallback warning has already been
	*  displayed, 0 otherwise.
	*/
	int single_decode_warning;

	enum AVPictureType pict_type;

	/** @} */

	/**
	* pic_struct in picture timing SEI message
	*/
	SEI_PicStructType sei_pic_struct;

	/**
	* Complement sei_pic_struct
	* SEI_PIC_STRUCT_TOP_BOTTOM and SEI_PIC_STRUCT_BOTTOM_TOP indicate interlaced frames.
	* However, soft telecined frames may have these values.
	* This is used in an attempt to flag soft telecine progressive.
	*/
	int prev_interlaced_frame;

	/**
	* frame_packing_arrangment SEI message
	*/
	int sei_frame_packing_present;
	int frame_packing_arrangement_type;
	int content_interpretation_type;
	int quincunx_subsampling;

	/**
	* display orientation SEI message
	*/
	int sei_display_orientation_present;
	int sei_anticlockwise_rotation;
	int sei_hflip, sei_vflip;

	/**
	* User data registered by Rec. ITU-T T.35 SEI
	*/
	int sei_reguserdata_afd_present;
	uint8_t active_format_description;
	int a53_caption_size;
	uint8_t *a53_caption;

	/**
	* Bit set of clock types for fields/frames in picture timing SEI message.
	* For each found ct_type, appropriate bit is set (e.g., bit 1 for
	* interlaced).
	*/
	int sei_ct_type;

	/**
	* dpb_output_delay in picture timing SEI message, see H.264 C.2.2
	*/
	int sei_dpb_output_delay;

	/**
	* cpb_removal_delay in picture timing SEI message, see H.264 C.1.2
	*/
	int sei_cpb_removal_delay;

	/**
	* recovery_frame_cnt from SEI message
	*
	* Set to -1 if no recovery point SEI message found or to number of frames
	* before playback synchronizes. Frames having recovery point are key
	* frames.
	*/
	int sei_recovery_frame_cnt;

	/**
	* Are the SEI recovery points looking valid.
	*/
	int valid_recovery_point;

	FPA sei_fpa;

	/**
	* recovery_frame is the frame_num at which the next frame should
	* be fully constructed.
	*
	* Set to -1 when not expecting a recovery point.
	*/
	int recovery_frame;

	/**
	* We have seen an IDR, so all the following frames in coded order are correctly
	* decodable.
	*/
#define FRAME_RECOVERED_IDR  (1 << 0)
	/**
	* Sufficient number of frames have been decoded since a SEI recovery point,
	* so all the following frames in presentation order are correct.
	*/
#define FRAME_RECOVERED_SEI  (1 << 1)

	int frame_recovered;    ///< Initial frame has been completely recovered

	int has_recovery_point;

	int missing_fields;

	/* for frame threading, this is set to 1
	* after finish_setup() has been called, so we cannot modify
	* some context properties (which are supposed to stay constant between
	* slices) anymore */
	int setup_finished;

	// Timestamp stuff
	int sei_buffering_period_present;   ///< Buffering period SEI flag
	int initial_cpb_removal_delay[32];  ///< Initial timestamps for CPBs

	int cur_chroma_format_idc;
	int cur_bit_depth_luma;
	int16_t slice_row[MAX_SLICES]; ///< to detect when MAX_SLICES is too low

	uint8_t parse_history[6];
	int parse_history_count;
	int parse_last_mb;

	int enable_er;

	AVBufferPool *qscale_table_pool;
	AVBufferPool *mb_type_pool;
	AVBufferPool *motion_val_pool;
	AVBufferPool *ref_index_pool;

	/* Motion Estimation */
	qpel_mc_func(*qpel_put)[16];
	qpel_mc_func(*qpel_avg)[16];

	/*Green Metadata */
	GreenMetaData sei_green_metadata;
#endif
} H264Context;

