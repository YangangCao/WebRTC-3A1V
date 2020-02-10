/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "cng.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#define WEBRTC_SPL_ABS_W16(a) \
    (((int16_t)a >= 0) ? ((int16_t)a) : -((int16_t)a))
#define WEBRTC_SPL_MUL(a, b) \
    ((int32_t) ((int32_t)(a) * (int32_t)(b)))
#define WEBRTC_SPL_MUL_16_16(a, b) \
    ((int32_t) (((int16_t)(a)) * ((int16_t)(b))))
#define WEBRTC_SPL_MUL_16_16_RSFT(a, b, c) \
    (WEBRTC_SPL_MUL_16_16(a, b) >> (c))
#define WEBRTC_SPL_ABS_W32(a) \
    (((int32_t)a >= 0) ? ((int32_t)a) : -((int32_t)a))

// Shifting with negative numbers not allowed
// We cannot do casting here due to signed/unsigned problem
#define WEBRTC_SPL_LSHIFT_W32(x, c)     ((x) << (c))

int32_t WebRtcSpl_DivW32W16(int32_t num, int16_t den) {
    // Guard against division with 0
    if (den != 0) {
        return (int32_t) (num / den);
    } else {
        return (int32_t) 0x7FFFFFFF;
    }
}

void WebRtcSpl_ScaleVector(const int16_t *in_vector, int16_t *out_vector,
                           int16_t gain, size_t in_vector_length,
                           int16_t right_shifts) {
    // Performs vector operation: out_vector = (gain*in_vector)>>right_shifts
    size_t i;
    const int16_t *inptr;
    int16_t *outptr;

    inptr = in_vector;
    outptr = out_vector;

    for (i = 0; i < in_vector_length; i++) {
        *outptr++ = (int16_t) ((*inptr++ * gain) >> right_shifts);
    }
}

static const uint32_t kMaxSeedUsed = 0x80000000;

static const int16_t kRandNTable[] = {
        9178, -7260, 40, 10189, 4894, -3531, -13779, 14764,
        -4008, -8884, -8990, 1008, 7368, 5184, 3251, -5817,
        -9786, 5963, 1770, 8066, -7135, 10772, -2298, 1361,
        6484, 2241, -8633, 792, 199, -3344, 6553, -10079,
        -15040, 95, 11608, -12469, 14161, -4176, 2476, 6403,
        13685, -16005, 6646, 2239, 10916, -3004, -602, -3141,
        2142, 14144, -5829, 5305, 8209, 4713, 2697, -5112,
        16092, -1210, -2891, -6631, -5360, -11878, -6781, -2739,
        -6392, 536, 10923, 10872, 5059, -4748, -7770, 5477,
        38, -1025, -2892, 1638, 6304, 14375, -11028, 1553,
        -1565, 10762, -393, 4040, 5257, 12310, 6554, -4799,
        4899, -6354, 1603, -1048, -2220, 8247, -186, -8944,
        -12004, 2332, 4801, -4933, 6371, 131, 8614, -5927,
        -8287, -22760, 4033, -15162, 3385, 3246, 3153, -5250,
        3766, 784, 6494, -62, 3531, -1582, 15572, 662,
        -3952, -330, -3196, 669, 7236, -2678, -6569, 23319,
        -8645, -741, 14830, -15976, 4903, 315, -11342, 10311,
        1858, -7777, 2145, 5436, 5677, -113, -10033, 826,
        -1353, 17210, 7768, 986, -1471, 8291, -4982, 8207,
        -14911, -6255, -2449, -11881, -7059, -11703, -4338, 8025,
        7538, -2823, -12490, 9470, -1613, -2529, -10092, -7807,
        9480, 6970, -12844, 5123, 3532, 4816, 4803, -8455,
        -5045, 14032, -4378, -1643, 5756, -11041, -2732, -16618,
        -6430, -18375, -3320, 6098, 5131, -4269, -8840, 2482,
        -7048, 1547, -21890, -6505, -7414, -424, -11722, 7955,
        1653, -17299, 1823, 473, -9232, 3337, 1111, 873,
        4018, -8982, 9889, 3531, -11763, -3799, 7373, -4539,
        3231, 7054, -8537, 7616, 6244, 16635, 447, -2915,
        13967, 705, -2669, -1520, -1771, -16188, 5956, 5117,
        6371, -9936, -1448, 2480, 5128, 7550, -8130, 5236,
        8213, -6443, 7707, -1950, -13811, 7218, 7031, -3883,
        67, 5731, -2874, 13480, -3743, 9298, -3280, 3552,
        -4425, -18, -3785, -9988, -5357, 5477, -11794, 2117,
        1416, -9935, 3376, 802, -5079, -8243, 12652, 66,
        3653, -2368, 6781, -21895, -7227, 2487, 7839, -385,
        6646, -7016, -4658, 5531, -1705, 834, 129, 3694,
        -1343, 2238, -22640, -6417, -11139, 11301, -2945, -3494,
        -5626, 185, -3615, -2041, -7972, -3106, -60, -23497,
        -1566, 17064, 3519, 2518, 304, -6805, -10269, 2105,
        1936, -426, -736, -8122, -1467, 4238, -6939, -13309,
        360, 7402, -7970, 12576, 3287, 12194, -6289, -16006,
        9171, 4042, -9193, 9123, -2512, 6388, -4734, -8739,
        1028, -5406, -1696, 5889, -666, -4736, 4971, 3565,
        9362, -6292, 3876, -3652, -19666, 7523, -4061, 391,
        -11773, 7502, -3763, 4929, -9478, 13278, 2805, 4496,
        7814, 16419, 12455, -14773, 2127, -2746, 3763, 4847,
        3698, 6978, 4751, -6957, -3581, -45, 6252, 1513,
        -4797, -7925, 11270, 16188, -2359, -5269, 9376, -10777,
        7262, 20031, -6515, -2208, -5353, 8085, -1341, -1303,
        7333, 5576, 3625, 5763, -7931, 9833, -3371, -10305,
        6534, -13539, -9971, 997, 8464, -4064, -1495, 1857,
        13624, 5458, 9490, -11086, -4524, 12022, -550, -198,
        408, -8455, -7068, 10289, 9712, -3366, 9028, -7621,
        -5243, 2362, 6909, 4672, -4933, -1799, 4709, -4563,
        -62, -566, 1624, -7010, 14730, -17791, -3697, -2344,
        -1741, 7099, -9509, -6855, -1989, 3495, -2289, 2031,
        12784, 891, 14189, -3963, -5683, 421, -12575, 1724,
        -12682, -5970, -8169, 3143, -1824, -5488, -5130, 8536,
        12799, 794, 5738, 3459, -11689, -258, -3738, -3775,
        -8742, 2333, 8312, -9383, 10331, 13119, 8398, 10644,
        -19433, -6446, -16277, -11793, 16284, 9345, 15222, 15834,
        2009, -7349, 130, -14547, 338, -5998, 3337, 21492,
        2406, 7703, -951, 11196, -564, 3406, 2217, 4806,
        2374, -5797, 11839, 8940, -11874, 18213, 2855, 10492
};

static uint32_t IncreaseSeed(uint32_t *seed) {
    seed[0] = (seed[0] * ((int32_t) 69069) + 1) & (kMaxSeedUsed - 1);
    return seed[0];
}

int16_t WebRtcSpl_RandN(uint32_t *seed) {
    return kRandNTable[IncreaseSeed(seed) >> 23];
}


#define WEBRTC_SPL_MEMCPY_W16(v1, v2, length) \
  memcpy(v1, v2, (length) * sizeof(int16_t))

void WebRtcSpl_CopyFromEndW16(const int16_t *vector_in,
                              size_t length,
                              size_t samples,
                              int16_t *vector_out) {
    // Copy the last <samples> of the input vector to vector_out
    WEBRTC_SPL_MEMCPY_W16(vector_out, &vector_in[length - samples], samples);
}

size_t WebRtcSpl_FilterAR(const int16_t *a,
                          size_t a_length,
                          const int16_t *x,
                          size_t x_length,
                          int16_t *state,
                          size_t state_length,
                          int16_t *state_low,
                          size_t state_low_length,
                          int16_t *filtered,
                          int16_t *filtered_low,
                          size_t filtered_low_length) {
    int64_t o;
    int32_t oLOW;
    size_t i, j, stop;
    const int16_t *x_ptr = &x[0];
    int16_t *filteredFINAL_ptr = filtered;
    int16_t *filteredFINAL_LOW_ptr = filtered_low;

    for (i = 0; i < x_length; i++) {
        // Calculate filtered[i] and filtered_low[i]
        const int16_t *a_ptr = &a[1];
        // The index can become negative, but the arrays will never be indexed
        // with it when negative. Nevertheless, the index cannot be a size_t
        // because of this.
        int filtered_ix = (int) i - 1;
        int16_t *state_ptr = &state[state_length - 1];
        int16_t *state_low_ptr = &state_low[state_length - 1];

        o = (int32_t) (*x_ptr++) * (1 << 12);
        oLOW = (int32_t) 0;

        stop = (i < a_length) ? i + 1 : a_length;
        for (j = 1; j < stop; j++) {
//                    RTC_DCHECK_GE(filtered_ix, 0);
            o -= *a_ptr * filtered[filtered_ix];
            oLOW -= *a_ptr++ * filtered_low[filtered_ix];
            --filtered_ix;
        }
        for (j = i + 1; j < a_length; j++) {
            o -= *a_ptr * *state_ptr--;
            oLOW -= *a_ptr++ * *state_low_ptr--;
        }

        o += (oLOW >> 12);
        *filteredFINAL_ptr = (int16_t) ((o + (int32_t) 2048) >> 12);
        *filteredFINAL_LOW_ptr++ =
                (int16_t) (o - ((int32_t) (*filteredFINAL_ptr++) * (1 << 12)));
    }

    // Save the filter state
    if (x_length >= state_length) {
        WebRtcSpl_CopyFromEndW16(filtered, x_length, a_length - 1, state);
        WebRtcSpl_CopyFromEndW16(filtered_low, x_length, a_length - 1, state_low);
    } else {
        for (i = 0; i < state_length - x_length; i++) {
            state[i] = state[i + x_length];
            state_low[i] = state_low[i + x_length];
        }
        for (i = 0; i < x_length; i++) {
            state[state_length - x_length + i] = filtered[i];
            state[state_length - x_length + i] = filtered_low[i];
        }
    }

    return x_length;
}


const size_t kCngMaxOutsizeOrder = 640;

// TODO(ossu): Rename the left-over WebRtcCng according to style guide.
void WebRtcCng_K2a16(int16_t *k, int useOrder, int16_t *a);

const int32_t WebRtcCng_kDbov[94] = {
        1081109975, 858756178, 682134279, 541838517, 430397633, 341876992,
        271562548, 215709799, 171344384, 136103682, 108110997, 85875618,
        68213428, 54183852, 43039763, 34187699, 27156255, 21570980,
        17134438, 13610368, 10811100, 8587562, 6821343, 5418385,
        4303976, 3418770, 2715625, 2157098, 1713444, 1361037,
        1081110, 858756, 682134, 541839, 430398, 341877,
        271563, 215710, 171344, 136104, 108111, 85876,
        68213, 54184, 43040, 34188, 27156, 21571,
        17134, 13610, 10811, 8588, 6821, 5418,
        4304, 3419, 2716, 2157, 1713, 1361,
        1081, 859, 682, 542, 430, 342,
        272, 216, 171, 136, 108, 86,
        68, 54, 43, 34, 27, 22,
        17, 14, 11, 9, 7, 5,
        4, 3, 3, 2, 2, 1,
        1, 1, 1, 1
};

const int16_t WebRtcCng_kCorrWindow[WEBRTC_CNG_MAX_LPC_ORDER] = {
        32702, 32636, 32570, 32505, 32439, 32374,
        32309, 32244, 32179, 32114, 32049, 31985
};

ComfortNoiseDecoder::ComfortNoiseDecoder() {
    /* Needed to get the right function pointers in SPLIB. */
    Reset();
}

void ComfortNoiseDecoder::Reset() {
    dec_seed_ = 7777;  /* For debugging only. */
    dec_target_energy_ = 0;
    dec_used_energy_ = 0;
    for (auto &c : dec_target_reflCoefs_)
        c = 0;
    for (auto &c : dec_used_reflCoefs_)
        c = 0;
    for (auto &c : dec_filtstate_)
        c = 0;
    for (auto &c : dec_filtstateLow_)
        c = 0;
    dec_order_ = 5;
    dec_target_scale_factor_ = 0;
    dec_used_scale_factor_ = 0;
}

void ComfortNoiseDecoder::UpdateSid(ArrayView<const uint8_t> sid) {
    int16_t refCs[WEBRTC_CNG_MAX_LPC_ORDER];
    int32_t targetEnergy;
    size_t length = sid.size();
    /* Throw away reflection coefficients of higher order than we can handle. */
    if (length > (WEBRTC_CNG_MAX_LPC_ORDER + 1))
        length = WEBRTC_CNG_MAX_LPC_ORDER + 1;

    dec_order_ = static_cast<uint16_t>(length - 1);

    uint8_t sid0 = std::min<uint8_t>(sid[0], 93);
    targetEnergy = WebRtcCng_kDbov[sid0];
    /* Take down target energy to 75%. */
    targetEnergy = targetEnergy >> 1;
    targetEnergy += targetEnergy >> 2;

    dec_target_energy_ = targetEnergy;

    /* Reconstruct coeffs with tweak for WebRtc implementation of RFC3389. */
    if (dec_order_ == WEBRTC_CNG_MAX_LPC_ORDER) {
        for (size_t i = 0; i < (dec_order_); i++) {
            refCs[i] = sid[i + 1] << 8; /* Q7 to Q15*/
            dec_target_reflCoefs_[i] = refCs[i];
        }
    } else {
        for (size_t i = 0; i < (dec_order_); i++) {
            refCs[i] = (sid[i + 1] - 127) * (1 << 8); /* Q7 to Q15. */
            dec_target_reflCoefs_[i] = refCs[i];
        }
    }

    for (size_t i = (dec_order_); i < WEBRTC_CNG_MAX_LPC_ORDER; i++) {
        refCs[i] = 0;
        dec_target_reflCoefs_[i] = refCs[i];
    }
}

bool ComfortNoiseDecoder::Generate(ArrayView<int16_t> out_data,
                                   bool new_period) {
    int16_t excitation[kCngMaxOutsizeOrder];
    int16_t low[kCngMaxOutsizeOrder];
    int16_t lpPoly[WEBRTC_CNG_MAX_LPC_ORDER + 1];
    int16_t ReflBetaStd = 26214;  /* 0.8 in q15. */
    int16_t ReflBetaCompStd = 6553;  /* 0.2 in q15. */
    int16_t ReflBetaNewP = 19661;  /* 0.6 in q15. */
    int16_t ReflBetaCompNewP = 13107;  /* 0.4 in q15. */
    int16_t Beta, BetaC;  /* These are in Q15. */
    int32_t targetEnergy;
    int16_t En;
    int16_t temp16;
    const size_t num_samples = out_data.size();

    if (num_samples > kCngMaxOutsizeOrder) {
        return false;
    }

    if (new_period) {
        dec_used_scale_factor_ = dec_target_scale_factor_;
        Beta = ReflBetaNewP;
        BetaC = ReflBetaCompNewP;
    } else {
        Beta = ReflBetaStd;
        BetaC = ReflBetaCompStd;
    }

    /* Calculate new scale factor in Q13 */
    dec_used_scale_factor_ = (int16_t) (
            WEBRTC_SPL_MUL_16_16_RSFT(dec_used_scale_factor_, Beta >> 2, 13) +
            WEBRTC_SPL_MUL_16_16_RSFT(dec_target_scale_factor_, BetaC >> 2, 13));

    dec_used_energy_ = dec_used_energy_ >> 1;
    dec_used_energy_ += dec_target_energy_ >> 1;

    /* Do the same for the reflection coeffs, albeit in Q15. */
    for (size_t i = 0; i < WEBRTC_CNG_MAX_LPC_ORDER; i++) {
        dec_used_reflCoefs_[i] = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(
                dec_used_reflCoefs_[i], Beta, 15);
        dec_used_reflCoefs_[i] += (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(
                dec_target_reflCoefs_[i], BetaC, 15);
    }

    /* Compute the polynomial coefficients. */
    WebRtcCng_K2a16(dec_used_reflCoefs_, WEBRTC_CNG_MAX_LPC_ORDER, lpPoly);


    targetEnergy = dec_used_energy_;

    /* Calculate scaling factor based on filter energy. */
    En = 8192;  /* 1.0 in Q13. */
    for (size_t i = 0; i < (WEBRTC_CNG_MAX_LPC_ORDER); i++) {
        /* Floating point value for reference.
           E *= 1.0 - (dec_used_reflCoefs_[i] / 32768.0) *
           (dec_used_reflCoefs_[i] / 32768.0);
         */

        /* Same in fixed point. */
        /* K(i).^2 in Q15. */
        temp16 = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(
                dec_used_reflCoefs_[i], dec_used_reflCoefs_[i], 15);
        /* 1 - K(i).^2 in Q15. */
        temp16 = 0x7fff - temp16;
        En = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(En, temp16, 15);
    }

    /* float scaling= sqrt(E * dec_target_energy_ / (1 << 24)); */

    /* Calculate sqrt(En * target_energy / excitation energy) */
    targetEnergy = sqrtf(dec_used_energy_);

    En = (int16_t) sqrtf(En) << 6;
    En = (En * 3) >> 1;  /* 1.5 estimates sqrt(2). */
    dec_used_scale_factor_ = (int16_t) ((En * targetEnergy) >> 12);

    /* Generate excitation. */
    /* Excitation energy per sample is 2.^24 - Q13 N(0,1). */
    for (size_t i = 0; i < num_samples; i++) {
        excitation[i] = WebRtcSpl_RandN(&dec_seed_) >> 1;
    }

    /* Scale to correct energy. */
    WebRtcSpl_ScaleVector(excitation, excitation, dec_used_scale_factor_,
                          num_samples, 13);

    /* |lpPoly| - Coefficients in Q12.
     * |excitation| - Speech samples.
     * |nst->dec_filtstate| - State preservation.
     * |out_data| - Filtered speech samples. */
    WebRtcSpl_FilterAR(lpPoly, WEBRTC_CNG_MAX_LPC_ORDER + 1, excitation,
                       num_samples, dec_filtstate_, WEBRTC_CNG_MAX_LPC_ORDER,
                       dec_filtstateLow_, WEBRTC_CNG_MAX_LPC_ORDER,
                       out_data.data(), low, num_samples);

    return true;
}

ComfortNoiseEncoder::ComfortNoiseEncoder(int fs, int interval, int quality)
        : enc_nrOfCoefs_(quality),
          enc_sampfreq_(fs),
          enc_interval_(interval),
          enc_msSinceSid_(0),
          enc_Energy_(0),
          enc_reflCoefs_{0},
          enc_corrVector_{0},
          enc_seed_(7777)  /* For debugging only. */ {
    RTC_CHECK_GT(quality, 0);
    RTC_CHECK_LE(quality, WEBRTC_CNG_MAX_LPC_ORDER);
    /* Needed to get the right function pointers in SPLIB. */

}

void ComfortNoiseEncoder::Reset(int fs, int interval, int quality) {
    RTC_CHECK_GT(quality, 0);
    RTC_CHECK_LE(quality, WEBRTC_CNG_MAX_LPC_ORDER);
    enc_nrOfCoefs_ = quality;
    enc_sampfreq_ = fs;
    enc_interval_ = interval;
    enc_msSinceSid_ = 0;
    enc_Energy_ = 0;
    for (auto &c : enc_reflCoefs_)
        c = 0;
    for (auto &c : enc_corrVector_)
        c = 0;
    enc_seed_ = 7777;  /* For debugging only. */
}

const int8_t kWebRtcSpl_CountLeadingZeros32_Table[64] = {
        32, 8, 17, -1, -1, 14, -1, -1, -1, 20, -1, -1, -1, 28, -1, 18,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 26, 25, 24,
        4, 11, 23, 31, 3, 7, 10, 16, 22, 30, -1, -1, 2, 6, 13, 9,
        -1, 15, -1, 21, -1, 29, 19, -1, -1, -1, -1, -1, 1, 27, 5, 12,
};


// Don't call this directly except in tests!
static __inline int WebRtcSpl_CountLeadingZeros32_NotBuiltin(uint32_t n) {
    // Normalize n by rounding up to the nearest number that is a sequence of 0
    // bits followed by a sequence of 1 bits. This number has the same number of
    // leading zeros as the original n. There are exactly 33 such values.
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;

    // Multiply the modified n with a constant selected (by exhaustive search)
    // such that each of the 33 possible values of n give a product whose 6 most
    // significant bits are unique. Then look up the answer in the table.
    return kWebRtcSpl_CountLeadingZeros32_Table[(n * 0x8c0b2891) >> 26];
}

// Don't call this directly except in tests!
static __inline int WebRtcSpl_CountLeadingZeros64_NotBuiltin(uint64_t n) {
    const int leading_zeros = n >> 32 == 0 ? 32 : 0;
    return leading_zeros + WebRtcSpl_CountLeadingZeros32_NotBuiltin(
            (uint32_t) (n >> (32 - leading_zeros)));
}

// Returns the number of leading zero bits in the argument.
static __inline int WebRtcSpl_CountLeadingZeros32(uint32_t n) {
    return WebRtcSpl_CountLeadingZeros32_NotBuiltin(n);
}

static __inline int16_t WebRtcSpl_GetSizeInBits(uint32_t n) {
    return 32 - WebRtcSpl_CountLeadingZeros32(n);
}

// Return the number of steps a can be left-shifted without overflow,
// or 0 if a == 0.
static __inline int16_t WebRtcSpl_NormW32(int32_t a) {
    return a == 0 ? 0 : WebRtcSpl_CountLeadingZeros32(a < 0 ? ~a : a) - 1;
}

int16_t WebRtcSpl_GetScalingSquare(int16_t *in_vector,
                                   size_t in_vector_length,
                                   size_t times) {
    int16_t nbits = WebRtcSpl_GetSizeInBits((uint32_t) times);
    size_t i;
    int16_t smax = -1;
    int16_t sabs;
    int16_t *sptr = in_vector;
    int16_t t;
    size_t looptimes = in_vector_length;

    for (i = looptimes; i > 0; i--) {
        sabs = (*sptr > 0 ? *sptr++ : -*sptr++);
        smax = (sabs > smax ? sabs : smax);
    }
    t = WebRtcSpl_NormW32(WEBRTC_SPL_MUL(smax, smax));

    if (smax == 0) {
        return 0; // Since norm(0) returns 0
    } else {
        return (t > nbits) ? 0 : nbits - t;
    }
}

int32_t WebRtcSpl_Energy(int16_t *vector,
                         size_t vector_length,
                         int *scale_factor) {
    int32_t en = 0;
    size_t i;
    int scaling =
            WebRtcSpl_GetScalingSquare(vector, vector_length, vector_length);
    size_t looptimes = vector_length;
    int16_t *vectorptr = vector;

    for (i = 0; i < looptimes; i++) {
        en += (*vectorptr * *vectorptr) >> scaling;
        vectorptr++;
    }
    *scale_factor = scaling;

    return en;
}

// Hanning table with 256 entries
static const int16_t kHanningTable[] = {
        1, 2, 6, 10, 15, 22, 30, 39,
        50, 62, 75, 89, 104, 121, 138, 157,
        178, 199, 222, 246, 271, 297, 324, 353,
        383, 413, 446, 479, 513, 549, 586, 624,
        663, 703, 744, 787, 830, 875, 920, 967,
        1015, 1064, 1114, 1165, 1218, 1271, 1325, 1381,
        1437, 1494, 1553, 1612, 1673, 1734, 1796, 1859,
        1924, 1989, 2055, 2122, 2190, 2259, 2329, 2399,
        2471, 2543, 2617, 2691, 2765, 2841, 2918, 2995,
        3073, 3152, 3232, 3312, 3393, 3475, 3558, 3641,
        3725, 3809, 3895, 3980, 4067, 4154, 4242, 4330,
        4419, 4509, 4599, 4689, 4781, 4872, 4964, 5057,
        5150, 5244, 5338, 5432, 5527, 5622, 5718, 5814,
        5910, 6007, 6104, 6202, 6299, 6397, 6495, 6594,
        6693, 6791, 6891, 6990, 7090, 7189, 7289, 7389,
        7489, 7589, 7690, 7790, 7890, 7991, 8091, 8192,
        8293, 8393, 8494, 8594, 8694, 8795, 8895, 8995,
        9095, 9195, 9294, 9394, 9493, 9593, 9691, 9790,
        9889, 9987, 10085, 10182, 10280, 10377, 10474, 10570,
        10666, 10762, 10857, 10952, 11046, 11140, 11234, 11327,
        11420, 11512, 11603, 11695, 11785, 11875, 11965, 12054,
        12142, 12230, 12317, 12404, 12489, 12575, 12659, 12743,
        12826, 12909, 12991, 13072, 13152, 13232, 13311, 13389,
        13466, 13543, 13619, 13693, 13767, 13841, 13913, 13985,
        14055, 14125, 14194, 14262, 14329, 14395, 14460, 14525,
        14588, 14650, 14711, 14772, 14831, 14890, 14947, 15003,
        15059, 15113, 15166, 15219, 15270, 15320, 15369, 15417,
        15464, 15509, 15554, 15597, 15640, 15681, 15721, 15760,
        15798, 15835, 15871, 15905, 15938, 15971, 16001, 16031,
        16060, 16087, 16113, 16138, 16162, 16185, 16206, 16227,
        16246, 16263, 16280, 16295, 16309, 16322, 16334, 16345,
        16354, 16362, 16369, 16374, 16378, 16382, 16383, 16384
};

void WebRtcSpl_GetHanningWindow(int16_t *v, size_t size) {
    size_t jj;
    int16_t *vptr1;

    int32_t index;
    int32_t factor = ((int32_t) 0x40000000);

    factor = WebRtcSpl_DivW32W16(factor, (int16_t) size);
    if (size < 513)
        index = (int32_t) -0x200000;
    else
        index = (int32_t) -0x100000;
    vptr1 = v;

    for (jj = 0; jj < size; jj++) {
        index += factor;
        (*vptr1++) = kHanningTable[index >> 22];
    }

}

int16_t WebRtcSpl_MaxAbsValueW16C(const int16_t *vector, size_t length) {
    size_t i = 0;
    int absolute = 0, maximum = 0;

    RTC_DCHECK_GT(length, 0);

    for (i = 0; i < length; i++) {
        absolute = abs((int) vector[i]);

        if (absolute > maximum) {
            maximum = absolute;
        }
    }

    // Guard the case for abs(-32768).
    if (maximum > WEBRTC_SPL_WORD16_MAX) {
        maximum = WEBRTC_SPL_WORD16_MAX;
    }

    return (int16_t) maximum;
}

void WebRtcSpl_ElementwiseVectorMult(int16_t *out, const int16_t *in,
                                     const int16_t *win, size_t vector_length,
                                     int16_t right_shifts) {
    size_t i;
    int16_t *outptr = out;
    const int16_t *inptr = in;
    const int16_t *winptr = win;
    for (i = 0; i < vector_length; i++) {
        *outptr++ = (int16_t) ((*inptr++ * *winptr++) >> right_shifts);
    }
}

size_t WebRtcSpl_AutoCorrelation(const int16_t *in_vector,
                                 size_t in_vector_length,
                                 size_t order,
                                 int32_t *result,
                                 int *scale) {
    int32_t sum = 0;
    size_t i = 0, j = 0;
    int16_t smax = 0;
    int scaling = 0;

    RTC_DCHECK_LE(order, in_vector_length);

    // Find the maximum absolute value of the samples.
    smax = WebRtcSpl_MaxAbsValueW16C(in_vector, in_vector_length);

    // In order to avoid overflow when computing the sum we should scale the
    // samples so that (in_vector_length * smax * smax) will not overflow.
    if (smax == 0) {
        scaling = 0;
    } else {
        // Number of bits in the sum loop.
        int nbits = WebRtcSpl_GetSizeInBits((uint32_t) in_vector_length);
        // Number of bits to normalize smax.
        int t = WebRtcSpl_NormW32(WEBRTC_SPL_MUL(smax, smax));

        if (t > nbits) {
            scaling = 0;
        } else {
            scaling = nbits - t;
        }
    }

    // Perform the actual correlation calculation.
    for (i = 0; i < order + 1; i++) {
        sum = 0;
        /* Unroll the loop to improve performance. */
        for (j = 0; i + j + 3 < in_vector_length; j += 4) {
            sum += (in_vector[j + 0] * in_vector[i + j + 0]) >> scaling;
            sum += (in_vector[j + 1] * in_vector[i + j + 1]) >> scaling;
            sum += (in_vector[j + 2] * in_vector[i + j + 2]) >> scaling;
            sum += (in_vector[j + 3] * in_vector[i + j + 3]) >> scaling;
        }
        for (; j < in_vector_length - i; j++) {
            sum += (in_vector[j] * in_vector[i + j]) >> scaling;
        }
        *result++ = sum;
    }

    *scale = scaling;
    return order + 1;
}

int WebRtcSpl_DivW32HiLow(int32_t num, int16_t den_hi, int16_t den_low) {
    int16_t approx, tmp_hi, tmp_low, num_hi, num_low;
    int32_t tmpW32;

    approx = (int16_t) WebRtcSpl_DivW32W16((int32_t) 0x1FFFFFFF, den_hi);
    // result in Q14 (Note: 3FFFFFFF = 0.5 in Q30)

    // tmpW32 = 1/den = approx * (2.0 - den * approx) (in Q30)
    tmpW32 = (den_hi * approx << 1) + ((den_low * approx >> 15) << 1);
    // tmpW32 = den * approx

    tmpW32 = (int32_t) 0x7fffffffL - tmpW32; // result in Q30 (tmpW32 = 2.0-(den*approx))
    // UBSan: 2147483647 - -2 cannot be represented in type 'int'

    // Store tmpW32 in hi and low format
    tmp_hi = (int16_t) (tmpW32 >> 16);
    tmp_low = (int16_t) ((tmpW32 - ((int32_t) tmp_hi << 16)) >> 1);

    // tmpW32 = 1/den in Q29
    tmpW32 = (tmp_hi * approx + (tmp_low * approx >> 15)) << 1;

    // 1/den in hi and low format
    tmp_hi = (int16_t) (tmpW32 >> 16);
    tmp_low = (int16_t) ((tmpW32 - ((int32_t) tmp_hi << 16)) >> 1);

    // Store num in hi and low format
    num_hi = (int16_t) (num >> 16);
    num_low = (int16_t) ((num - ((int32_t) num_hi << 16)) >> 1);

    // num * (1/den) by 32 bit multiplication (result in Q28)

    tmpW32 = num_hi * tmp_hi + (num_hi * tmp_low >> 15) +
             (num_low * tmp_hi >> 15);

    // Put result in Q31 (convert from Q28)
    tmpW32 = WEBRTC_SPL_LSHIFT_W32(tmpW32, 3);

    return tmpW32;
}

#define SPL_LEVINSON_MAXORDER 20

int WebRtcSpl_LevinsonDurbin(const int32_t *R, int16_t *A, int16_t *K, size_t order) 
{
    size_t i, j;
    // Auto-correlation coefficients in high precision
    int16_t R_hi[SPL_LEVINSON_MAXORDER + 1], R_low[SPL_LEVINSON_MAXORDER + 1];
    // LPC coefficients in high precision
    int16_t A_hi[SPL_LEVINSON_MAXORDER + 1], A_low[SPL_LEVINSON_MAXORDER + 1];
    // LPC coefficients for next iteration
    int16_t A_upd_hi[SPL_LEVINSON_MAXORDER + 1], A_upd_low[SPL_LEVINSON_MAXORDER + 1];
    // Reflection coefficient in high precision
    int16_t K_hi, K_low;
    // Prediction gain Alpha in high precision and with scale factor
    int16_t Alpha_hi, Alpha_low, Alpha_exp;
    int16_t tmp_hi, tmp_low;
    int32_t temp1W32, temp2W32, temp3W32;
    int16_t norm;

    // Normalize the autocorrelation R[0]...R[order+1]

    norm = WebRtcSpl_NormW32(R[0]);

    for (i = 0; i <= order; ++i) 
    {
        temp1W32 = R[i] * (1 << norm);
        // UBSan: 12 * 268435456 cannot be represented in type 'int'

        // Put R in hi and low format
        R_hi[i] = (int16_t) (temp1W32 >> 16);
        R_low[i] = (int16_t) ((temp1W32 - ((int32_t) R_hi[i] * 65536)) >> 1);
    }

    // K = A[1] = -R[1] / R[0]

    temp2W32 = R[1] * (1 << norm); // R[1] in Q31
    temp3W32 = WEBRTC_SPL_ABS_W32(temp2W32); // abs R[1]
    temp1W32 = WebRtcSpl_DivW32HiLow(temp3W32, R_hi[0], R_low[0]); // abs(R[1])/R[0] in Q31
    // Put back the sign on R[1]
    if (temp2W32 > 0) 
    {
        temp1W32 = -temp1W32;
    }

    // Put K in hi and low format
    K_hi = (int16_t) (temp1W32 >> 16);
    K_low = (int16_t) ((temp1W32 - ((int32_t) K_hi * 65536)) >> 1);

    // Store first reflection coefficient
    K[0] = K_hi;

    temp1W32 >>= 4;  // A[1] in Q27.

    // Put A[1] in hi and low format
    A_hi[1] = (int16_t) (temp1W32 >> 16);
    A_low[1] = (int16_t) ((temp1W32 - ((int32_t) A_hi[1] * 65536)) >> 1);

    // Alpha = R[0] * (1-K^2)

    temp1W32 = ((K_hi * K_low >> 14) + K_hi * K_hi) * 2;  // = k^2 in Q31

    temp1W32 = WEBRTC_SPL_ABS_W32(temp1W32); // Guard against <0
    temp1W32 = (int32_t) 0x7fffffffL - temp1W32; // temp1W32 = (1 - K[0]*K[0]) in Q31

    // Store temp1W32 = 1 - K[0]*K[0] on hi and low format
    tmp_hi = (int16_t) (temp1W32 >> 16);
    tmp_low = (int16_t) ((temp1W32 - ((int32_t) tmp_hi << 16)) >> 1);

    // Calculate Alpha in Q31
    temp1W32 = (R_hi[0] * tmp_hi + (R_hi[0] * tmp_low >> 15) +
                (R_low[0] * tmp_hi >> 15)) << 1;

    // Normalize Alpha and put it in hi and low format

    Alpha_exp = WebRtcSpl_NormW32(temp1W32);
    temp1W32 = WEBRTC_SPL_LSHIFT_W32(temp1W32, Alpha_exp);
    Alpha_hi = (int16_t) (temp1W32 >> 16);
    Alpha_low = (int16_t) ((temp1W32 - ((int32_t) Alpha_hi << 16)) >> 1);

    // Perform the iterative calculations in the Levinson-Durbin algorithm

    for (i = 2; i <= order; i++) 
    {
        /*                    ----
         temp1W32 =  R[i] + > R[j]*A[i-j]
         /
         ----
         j=1..i-1
         */

        temp1W32 = 0;

        for (j = 1; j < i; j++) {
            // temp1W32 is in Q31
            temp1W32 += (R_hi[j] * A_hi[i - j] * 2) +
                        (((R_hi[j] * A_low[i - j] >> 15) +
                          (R_low[j] * A_hi[i - j] >> 15)) * 2);
        }

        temp1W32 = temp1W32 * 16;
        temp1W32 += ((int32_t) R_hi[i] * 65536)
                    + WEBRTC_SPL_LSHIFT_W32((int32_t) R_low[i], 1);

        // K = -temp1W32 / Alpha
        temp2W32 = WEBRTC_SPL_ABS_W32(temp1W32); // abs(temp1W32)
        temp3W32 = WebRtcSpl_DivW32HiLow(temp2W32, Alpha_hi, Alpha_low); // abs(temp1W32)/Alpha

        // Put the sign of temp1W32 back again
        if (temp1W32 > 0) {
            temp3W32 = -temp3W32;
        }

        // Use the Alpha shifts from earlier to de-normalize
        norm = WebRtcSpl_NormW32(temp3W32);
        if ((Alpha_exp <= norm) || (temp3W32 == 0)) {
            temp3W32 = temp3W32 * (1 << Alpha_exp);
        } else {
            if (temp3W32 > 0) {
                temp3W32 = (int32_t) 0x7fffffffL;
            } else {
                temp3W32 = (int32_t) 0x80000000L;
            }
        }

        // Put K on hi and low format
        K_hi = (int16_t) (temp3W32 >> 16);
        K_low = (int16_t) ((temp3W32 - ((int32_t) K_hi * 65536)) >> 1);

        // Store Reflection coefficient in Q15
        K[i - 1] = K_hi;

        // Test for unstable filter.
        // If unstable return 0 and let the user decide what to do in that case

        if ((int32_t) WEBRTC_SPL_ABS_W16(K_hi) > (int32_t) 32750) {
            return 0; // Unstable filter
        }

        /*
         Compute updated LPC coefficient: Anew[i]
         Anew[j]= A[j] + K*A[i-j]   for j=1..i-1
         Anew[i]= K
         */

        for (j = 1; j < i; j++) {
            // temp1W32 = A[j] in Q27
            temp1W32 = (int32_t) A_hi[j] * 65536
                       + WEBRTC_SPL_LSHIFT_W32((int32_t) A_low[j], 1);

            // temp1W32 += K*A[i-j] in Q27
            temp1W32 += (K_hi * A_hi[i - j] + (K_hi * A_low[i - j] >> 15) +
                         (K_low * A_hi[i - j] >> 15)) * 2;

            // Put Anew in hi and low format
            A_upd_hi[j] = (int16_t) (temp1W32 >> 16);
            A_upd_low[j] = (int16_t) (
                    (temp1W32 - ((int32_t) A_upd_hi[j] * 65536)) >> 1);
        }

        // temp3W32 = K in Q27 (Convert from Q31 to Q27)
        temp3W32 >>= 4;

        // Store Anew in hi and low format
        A_upd_hi[i] = (int16_t) (temp3W32 >> 16);
        A_upd_low[i] = (int16_t) (
                (temp3W32 - ((int32_t) A_upd_hi[i] * 65536)) >> 1);

        // Alpha = Alpha * (1-K^2)

        temp1W32 = ((K_hi * K_low >> 14) + K_hi * K_hi) * 2;  // K*K in Q31

        temp1W32 = WEBRTC_SPL_ABS_W32(temp1W32); // Guard against <0
        temp1W32 = (int32_t) 0x7fffffffL - temp1W32; // 1 - K*K  in Q31

        // Convert 1- K^2 in hi and low format
        tmp_hi = (int16_t) (temp1W32 >> 16);
        tmp_low = (int16_t) ((temp1W32 - ((int32_t) tmp_hi << 16)) >> 1);

        // Calculate Alpha = Alpha * (1-K^2) in Q31
        temp1W32 = (Alpha_hi * tmp_hi + (Alpha_hi * tmp_low >> 15) +
                    (Alpha_low * tmp_hi >> 15)) << 1;

        // Normalize Alpha and store it on hi and low format

        norm = WebRtcSpl_NormW32(temp1W32);
        temp1W32 = WEBRTC_SPL_LSHIFT_W32(temp1W32, norm);

        Alpha_hi = (int16_t) (temp1W32 >> 16);
        Alpha_low = (int16_t) ((temp1W32 - ((int32_t) Alpha_hi << 16)) >> 1);

        // Update the total normalization of Alpha
        Alpha_exp = Alpha_exp + norm;

        // Update A[]

        for (j = 1; j <= i; j++) {
            A_hi[j] = A_upd_hi[j];
            A_low[j] = A_upd_low[j];
        }
    }

    /*
     Set A[0] to 1.0 and store the A[i] i=1...order in Q12
     (Convert from Q27 and use rounding)
     */

    A[0] = 4096;

    for (i = 1; i <= order; i++) 
    {
        // temp1W32 in Q27
        temp1W32 = (int32_t) A_hi[i] * 65536
                   + WEBRTC_SPL_LSHIFT_W32((int32_t) A_low[i], 1);
        // Round and store upper word
        A[i] = (int16_t) (((temp1W32 * 2) + 32768) >> 16);
    }
    return 1; // Stable filters
}

size_t ComfortNoiseEncoder::Encode(ArrayView<const int16_t> speech,
                                   bool force_sid,
                                   Buffer *output) 
{
    int16_t arCoefs[WEBRTC_CNG_MAX_LPC_ORDER + 1];
    int32_t corrVector[WEBRTC_CNG_MAX_LPC_ORDER + 1];
    int16_t refCs[WEBRTC_CNG_MAX_LPC_ORDER + 1];
    int16_t hanningW[kCngMaxOutsizeOrder];
    int16_t ReflBeta = 19661;     /* 0.6 in q15. */
    int16_t ReflBetaComp = 13107; /* 0.4 in q15. */
    int32_t outEnergy;
    int outShifts;
    size_t i;
    int stab;
    int acorrScale;
    size_t index;
    size_t ind, factor;
    int32_t *bptr;
    int32_t blo, bhi;
    int16_t negate;
    const int16_t *aptr;
    int16_t speechBuf[kCngMaxOutsizeOrder];

    const size_t num_samples = speech.size();
    RTC_CHECK_LE(num_samples, kCngMaxOutsizeOrder);

    for (i = 0; i < num_samples; i++) {
        speechBuf[i] = speech[i];
    }

    factor = num_samples;

    /* Calculate energy and a coefficients. */
    outEnergy = WebRtcSpl_Energy(speechBuf, num_samples, &outShifts);
    while (outShifts > 0) {
        /* We can only do 5 shifts without destroying accuracy in
         * division factor. */
        if (outShifts > 5) {
            outEnergy <<= (outShifts - 5);
            outShifts = 5;
        } else {
            factor /= 2;
            outShifts--;
        }
    }
    outEnergy = WebRtcSpl_DivW32W16(outEnergy, (int16_t) factor);

    if (outEnergy > 1) {
        /* Create Hanning Window. */
        WebRtcSpl_GetHanningWindow(hanningW, num_samples / 2);
        for (i = 0; i < (num_samples / 2); i++)
            hanningW[num_samples - i - 1] = hanningW[i];

        WebRtcSpl_ElementwiseVectorMult(speechBuf, hanningW, speechBuf, num_samples,
                                        14);

        WebRtcSpl_AutoCorrelation(speechBuf, num_samples, enc_nrOfCoefs_,
                                  corrVector, &acorrScale);

        if (*corrVector == 0)
            *corrVector = WEBRTC_SPL_WORD16_MAX;

        /* Adds the bandwidth expansion. */
        aptr = WebRtcCng_kCorrWindow;
        bptr = corrVector;

        /* (zzz) lpc16_1 = 17+1+820+2+2 = 842 (ordo2=700). */
        for (ind = 0; ind < enc_nrOfCoefs_; ind++) {
            /* The below code multiplies the 16 b corrWindow values (Q15) with
             * the 32 b corrvector (Q0) and shifts the result down 15 steps. */
            negate = *bptr < 0;
            if (negate)
                *bptr = -*bptr;

            blo = (int32_t) *aptr * (*bptr & 0xffff);
            bhi = ((blo >> 16) & 0xffff)
                  + ((int32_t) (*aptr++) * ((*bptr >> 16) & 0xffff));
            blo = (blo & 0xffff) | ((bhi & 0xffff) << 16);

            *bptr = (((bhi >> 16) & 0x7fff) << 17) | ((uint32_t) blo >> 15);
            if (negate)
                *bptr = -*bptr;
            bptr++;
        }
        /* End of bandwidth expansion. */

        stab = WebRtcSpl_LevinsonDurbin(corrVector, arCoefs, refCs,
                                        enc_nrOfCoefs_);

        if (!stab) {
            /* Disregard from this frame */
            return 0;
        }

    } else {
        for (i = 0; i < enc_nrOfCoefs_; i++)
            refCs[i] = 0;
    }

    if (force_sid) {
        /* Read instantaneous values instead of averaged. */
        for (i = 0; i < enc_nrOfCoefs_; i++)
            enc_reflCoefs_[i] = refCs[i];
        enc_Energy_ = outEnergy;
    } else {
        /* Average history with new values. */
        for (i = 0; i < enc_nrOfCoefs_; i++) {
            enc_reflCoefs_[i] = (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(
                    enc_reflCoefs_[i], ReflBeta, 15);
            enc_reflCoefs_[i] +=
                    (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(refCs[i], ReflBetaComp, 15);
        }
        enc_Energy_ =
                (outEnergy >> 2) + (enc_Energy_ >> 1) + (enc_Energy_ >> 2);
    }

    if (enc_Energy_ < 1) {
        enc_Energy_ = 1;
    }

    if ((enc_msSinceSid_ > (enc_interval_ - 1)) || force_sid) {
        /* Search for best dbov value. */
        index = 0;
        for (i = 1; i < 93; i++) {
            /* Always round downwards. */
            if ((enc_Energy_ - WebRtcCng_kDbov[i]) > 0) {
                index = i;
                break;
            }
        }
        if ((i == 93) && (index == 0))
            index = 94;

        const size_t output_coefs = enc_nrOfCoefs_ + 1;
        output->AppendData(output_coefs, [&](ArrayView<uint8_t> output) {
            output[0] = (uint8_t) index;

            /* Quantize coefficients with tweak for WebRtc implementation of
             * RFC3389. */
            if (enc_nrOfCoefs_ == WEBRTC_CNG_MAX_LPC_ORDER) {
                for (i = 0; i < enc_nrOfCoefs_; i++) {
                    /* Q15 to Q7 with rounding. */
                    output[i + 1] = ((enc_reflCoefs_[i] + 128) >> 8);
                }
            } else {
                for (i = 0; i < enc_nrOfCoefs_; i++) {
                    /* Q15 to Q7 with rounding. */
                    output[i + 1] = (127 + ((enc_reflCoefs_[i] + 128) >> 8));
                }
            }

            return output_coefs;
        });

        enc_msSinceSid_ =
                static_cast<int16_t>((1000 * num_samples) / enc_sampfreq_);
        return output_coefs;
    } else {
        enc_msSinceSid_ +=
                static_cast<int16_t>((1000 * num_samples) / enc_sampfreq_);
        return 0;
    }
}

#define WEBRTC_SPL_MAX_LPC_ORDER    14

/* Values in |k| are Q15, and |a| Q12. */
void WebRtcCng_K2a16(int16_t *k, int useOrder, int16_t *a) {
    int16_t any[WEBRTC_SPL_MAX_LPC_ORDER + 1];
    int16_t *aptr;
    int16_t *aptr2;
    int16_t *anyptr;
    const int16_t *kptr;
    int m, i;

    kptr = k;
    *a = 4096; /* i.e., (Word16_MAX >> 3) + 1 */
    *any = *a;
    a[1] = (*k + 4) >> 3;
    for (m = 1; m < useOrder; m++) {
        kptr++;
        aptr = a;
        aptr++;
        aptr2 = &a[m];
        anyptr = any;
        anyptr++;

        any[m + 1] = (*kptr + 4) >> 3;
        for (i = 0; i < m; i++) {
            *anyptr++ =
                    (*aptr++) +
                    (int16_t) ((((int32_t) (*aptr2--) * (int32_t) *kptr) + 16384) >> 15);
        }

        aptr = a;
        anyptr = any;
        for (i = 0; i < (m + 2); i++) {
            *aptr++ = *anyptr++;
        }
    }
}
