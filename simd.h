#ifndef CARP_SIMD_H
#define CARP_SIMD_H

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER) || defined(__MINGW32__)
  #include <malloc.h>
#endif

// Carp String Symbol Helper
#ifndef Carp_allocate_string
  extern char *String_from_MINUS_cstr(char*);
  #define Carp_allocate_string String_from_MINUS_cstr
#endif

#if defined(__AVX512F__)
  typedef float simd_float __attribute__((vector_size(64), aligned(64)));
  typedef int   simd_mask  __attribute__((vector_size(64), aligned(64)));
  typedef int   simd_int   __attribute__((vector_size(64), aligned(64)));
  #define SIMD_LANES 16
  #define SIMD_ALIGN 64
#elif defined(__AVX2__) || defined(__AVX__)
  typedef float simd_float __attribute__((vector_size(32), aligned(32)));
  typedef int   simd_mask  __attribute__((vector_size(32), aligned(32)));
  typedef int   simd_int   __attribute__((vector_size(32), aligned(32)));
  #define SIMD_LANES 8
  #define SIMD_ALIGN 32
#else
  typedef float simd_float __attribute__((vector_size(16), aligned(16)));
  typedef int   simd_mask  __attribute__((vector_size(16), aligned(16)));
  typedef int   simd_int   __attribute__((vector_size(16), aligned(16)));
  #define SIMD_LANES 4
  #define SIMD_ALIGN 16
#endif

// Unaligned load / store
static inline simd_float simd_load(const float *ptr) {
    simd_float res;
    memcpy(&res, ptr, sizeof(simd_float));
    return res;
}

static inline void simd_store(float *ptr, simd_float val) {
    memcpy(ptr, &val, sizeof(simd_float));
}

// Aligned load / store
static inline simd_float simd_load_aligned(const float *ptr) {
    const float *aligned_ptr = (const float*)__builtin_assume_aligned(ptr, SIMD_ALIGN);
    return *(const simd_float*)aligned_ptr;
}

static inline void simd_store_aligned(float *ptr, simd_float val) {
    float *aligned_ptr = (float*)__builtin_assume_aligned(ptr, SIMD_ALIGN);
    *(simd_float*)aligned_ptr = val;
}

// Bounded partial load / store
static inline simd_float simd_load_n(const float *ptr, int count) {
    simd_float res = {0};
    int copy_count = count < SIMD_LANES ? count : SIMD_LANES;
    if (copy_count > 0) {
        memcpy(&res, ptr, copy_count * sizeof(float));
    }
    return res;
}

static inline void simd_store_n(float *ptr, simd_float val, int count) {
    int copy_count = count < SIMD_LANES ? count : SIMD_LANES;
    if (copy_count > 0) {
        memcpy(ptr, &val, copy_count * sizeof(float));
    }
}

// Element access
static inline float simd_get(simd_float val, int idx) {
    return val[idx];
}

static inline simd_float simd_set(simd_float val, int idx, float element) {
    simd_float res = val;
    res[idx] = element;
    return res;
}

// Broadcast scalar
static inline simd_float simd_splat(float val) {
    simd_float res;
    for (int i = 0; i < SIMD_LANES; i++) {
        res[i] = val;
    }
    return res;
}

// Basic Arithmetic
static inline simd_float simd_add(simd_float a, simd_float b) { return a + b; }
static inline simd_float simd_sub(simd_float a, simd_float b) { return a - b; }
static inline simd_float simd_mul(simd_float a, simd_float b) { return a * b; }
static inline simd_float simd_div(simd_float a, simd_float b) { return a / b; }
static inline simd_float simd_neg(simd_float a)               { return -a; }
static inline simd_float simd_fma(simd_float a, simd_float b, simd_float c) { return (a * b) + c; }

// Math Functions
static inline simd_float simd_sqrt(simd_float val) {
    simd_float res;
    for (int i = 0; i < SIMD_LANES; i++) res[i] = sqrtf(val[i]);
    return res;
}

static inline simd_float simd_rsqrt(simd_float val) {
    simd_float res;
    for (int i = 0; i < SIMD_LANES; i++) res[i] = 1.0f / sqrtf(val[i]);
    return res;
}

static inline simd_float simd_abs(simd_float val) {
    simd_float res;
    for (int i = 0; i < SIMD_LANES; i++) res[i] = fabsf(val[i]);
    return res;
}

// Vector Min / Max
static inline simd_float simd_min(simd_float a, simd_float b) {
    simd_float res;
    for (int i = 0; i < SIMD_LANES; i++) res[i] = (a[i] < b[i]) ? a[i] : b[i];
    return res;
}

static inline simd_float simd_max(simd_float a, simd_float b) {
    simd_float res;
    for (int i = 0; i < SIMD_LANES; i++) res[i] = (a[i] > b[i]) ? a[i] : b[i];
    return res;
}

static inline simd_float simd_clamp(simd_float v, simd_float min_v, simd_float max_v) {
    return simd_min(simd_max(v, min_v), max_v);
}

// Comparisons & Selection
static inline simd_mask simd_cmp_lt(simd_float a, simd_float b) { return a < b; }
static inline simd_mask simd_cmp_gt(simd_float a, simd_float b) { return a > b; }
static inline simd_mask simd_cmp_eq(simd_float a, simd_float b) { return a == b; }

static inline simd_float simd_select(simd_mask mask, simd_float a, simd_float b) {
    simd_float res;
    for (int i = 0; i < SIMD_LANES; i++) {
        res[i] = mask[i] ? a[i] : b[i];
    }
    return res;
}

// Full Horizontal Sum
static inline float simd_sum(simd_float val) {
    #if SIMD_LANES == 4
        return (val[0] + val[1]) + (val[2] + val[3]);
    #elif SIMD_LANES == 8
        return ((val[0] + val[1]) + (val[2] + val[3])) + ((val[4] + val[5]) + (val[6] + val[7]));
    #elif SIMD_LANES == 16
        float s0 = (val[0] + val[1]) + (val[2] + val[3]);
        float s1 = (val[4] + val[5]) + (val[6] + val[7]);
        float s2 = (val[8] + val[9]) + (val[10] + val[11]);
        float s3 = (val[12] + val[13]) + (val[14] + val[15]);
        return (s0 + s1) + (s2 + s3);
    #else
        float s = 0.0f;
        for (int i = 0; i < SIMD_LANES; i++) s += val[i];
        return s;
    #endif
}

// 4-Lane Reduction Semantics
static inline float simd_sum_4(simd_float val) {
    return (val[0] + val[1]) + (val[2] + val[3]);
}

static inline float simd_min_4(simd_float val) {
    float m0 = val[0] < val[1] ? val[0] : val[1];
    float m1 = val[2] < val[3] ? val[2] : val[3];
    return m0 < m1 ? m0 : m1;
}

static inline float simd_max_4(simd_float val) {
    float m0 = val[0] > val[1] ? val[0] : val[1];
    float m1 = val[2] > val[3] ? val[2] : val[3];
    return m0 > m1 ? m0 : m1;
}

// Full-Width Reductions
static inline float simd_reduce_min(simd_float val) {
    float m = val[0];
    for (int i = 1; i < SIMD_LANES; i++) {
        if (val[i] < m) m = val[i];
    }
    return m;
}

static inline float simd_reduce_max(simd_float val) {
    float m = val[0];
    for (int i = 1; i < SIMD_LANES; i++) {
        if (val[i] > m) m = val[i];
    }
    return m;
}

// Vector dot product across all lanes
static inline float simd_dot(simd_float a, simd_float b) {
    return simd_sum(a * b);
}

// 4D Dot product (specifically for 3D/4D game math on padded vectors)
static inline float simd_dot_4(simd_float a, simd_float b) {
    return simd_sum_4(a * b);
}

// Vector magnitude / length (4-lane)
static inline float simd_len_4(simd_float a) {
    return sqrtf(simd_dot_4(a, a));
}

// Vector normalization (4-lane)
static inline simd_float simd_normalize_4(simd_float a) {
    float d = simd_dot_4(a, a);
    if (d > 1e-8f) {
        float inv_len = 1.0f / sqrtf(d);
        return a * simd_splat(inv_len);
    }
    return simd_splat(0.0f);
}

// Batch Transform: dst[i] = a[i] op b[i]
typedef simd_float (*simd_binop_fn)(simd_float, simd_float);
typedef float (*scalar_binop_fn)(float, float);

static inline void simd_batch_binop(
    float *__restrict__ dst,
    const float *__restrict__ a,
    const float *__restrict__ b,
    int count,
    simd_binop_fn v_fn,
    scalar_binop_fn s_fn
) {
    int i = 0;
    int limit = count - (count % SIMD_LANES);

    for (; i < limit; i += SIMD_LANES) {
        simd_float va = simd_load(&a[i]);
        simd_float vb = simd_load(&b[i]);
        simd_float vr = v_fn(va, vb);
        simd_store(&dst[i], vr);
    }
    for (; i < count; i++) {
        dst[i] = s_fn(a[i], b[i]);
    }
}

// Fixed 4-lane initializer (compound literal syntax)
static inline simd_float simd_init_4(float x0, float x1, float x2, float x3) {
#if SIMD_LANES == 4
    return (simd_float){x0, x1, x2, x3};
#elif SIMD_LANES == 8
    return (simd_float){x0, x1, x2, x3, 0.0f, 0.0f, 0.0f, 0.0f};
#elif SIMD_LANES == 16
    return (simd_float){x0, x1, x2, x3, 0.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
#else
    simd_float res = {0};
    res[0] = x0; res[1] = x1; res[2] = x2; res[3] = x3;
    return res;
#endif
}

// Safe Aligned Allocation
static inline float* simd_alloc_aligned(int count, int align) {
    size_t size = count * sizeof(float);
    size_t actual_align = (align >= (int)sizeof(void*)) ? (size_t)align : sizeof(void*);
#if defined(_MSC_VER) || defined(__MINGW32__)
    return (float*)_aligned_malloc(size, actual_align);
#else
    void *ptr = NULL;
    if (posix_memalign(&ptr, actual_align, size) == 0) {
        return (float*)ptr;
    }
    return NULL;
#endif
}

static inline void simd_free_aligned(float *ptr) {
#if defined(_MSC_VER) || defined(__MINGW32__)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

// --- Integer SIMD Twin (SimdInt) ---
static inline simd_int simd_int_load(const int *ptr) {
    simd_int res;
    memcpy(&res, ptr, sizeof(simd_int));
    return res;
}

static inline void simd_int_store(int *ptr, simd_int val) {
    memcpy(ptr, &val, sizeof(simd_int));
}

static inline simd_int simd_int_splat(int val) {
    simd_int res;
    for (int i = 0; i < SIMD_LANES; i++) res[i] = val;
    return res;
}

static inline int simd_int_get(simd_int val, int idx) {
    return val[idx];
}

static inline simd_int simd_int_set(simd_int val, int idx, int element) {
    simd_int res = val;
    res[idx] = element;
    return res;
}

// Bitwise operations
static inline simd_int simd_and(simd_int a, simd_int b) { return a & b; }
static inline simd_int simd_or(simd_int a, simd_int b)  { return a | b; }
static inline simd_int simd_xor(simd_int a, simd_int b) { return a ^ b; }
static inline simd_int simd_not(simd_int a)             { return ~a; }

// Conversions
static inline simd_int   simd_float_to_int(simd_float a) { return __builtin_convertvector(a, simd_int); }
static inline simd_float simd_int_to_float(simd_int a)   { return __builtin_convertvector(a, simd_float); }

// Gather and Scatter
static inline simd_float simd_gather(const float *base, simd_int indices) {
    simd_float res;
    for (int i = 0; i < SIMD_LANES; i++) {
        res[i] = base[indices[i]];
    }
    return res;
}

static inline void simd_scatter(float *base, simd_int indices, simd_float val) {
    for (int i = 0; i < SIMD_LANES; i++) {
        base[indices[i]] = val[i];
    }
}

// 4x4 Matrix Transform
typedef struct {
    simd_float cols[4];
} simd_mat4;

static inline simd_mat4 simd_mat4_make(simd_float col0, simd_float col1, simd_float col2, simd_float col3) {
    simd_mat4 res;
    res.cols[0] = col0;
    res.cols[1] = col1;
    res.cols[2] = col2;
    res.cols[3] = col3;
    return res;
}

static inline simd_float simd_mat4_mul_vec4(const simd_mat4 *m, simd_float v) {
    simd_float x = simd_splat(simd_get(v, 0));
    simd_float y = simd_splat(simd_get(v, 1));
    simd_float z = simd_splat(simd_get(v, 2));
    simd_float w = simd_splat(simd_get(v, 3));

    simd_float res = simd_mul(m->cols[0], x);
    res = simd_fma(m->cols[1], y, res);
    res = simd_fma(m->cols[2], z, res);
    res = simd_fma(m->cols[3], w, res);
    return res;
}

#endif
