// complete compileable example begins
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

int live_matrices;

struct Matrix {
    int n; // rows and columns in this range
    int row_stride; // distance between beginnings of rows (units: elements)
    int8_t ref_count; // number of active references to this object, -1 if it's a stack value
    struct Matrix *shown; // a matrix being viewed, if any
    float *ptr;
} typedef Matrix;

void matrix_dump(const Matrix *M) {
    if (!M) {
        printf("Null\n");
        fflush(stdout);
        return;
    }
    const char *kind = 
        !M->ref_count ? "Freed" :
        !M->shown ? "Matrix" : "View";
    printf("%s %dx%d <->%d r(%d,%d) %p", kind, M->n, M->n, M->row_stride,
        M->ref_count, M->shown ? M->shown->ref_count : 0, M->ptr);
    if (M->shown) printf(" ..%p", M->shown->ptr);
    printf("\n");
    fflush(stdout);
}

Matrix *local(const Matrix *M) {
    assert(M->ref_count == -1);
    return (Matrix*)M;
}

Matrix *retain(Matrix **M) {
    Matrix *const mp = *M;
    if (!mp) return NULL;
    assert(mp->ref_count != -1);
    ++mp->ref_count;
    if (mp->shown) retain(&mp->shown);
    return mp;
}

Matrix *release(Matrix **M) {
    Matrix *mp = *M;
    if (!mp) return NULL;
    assert(mp->ref_count != 0);
    if (mp->ref_count == -1) return mp;
    bool const free_this = mp->ref_count > 0 && !--mp->ref_count;
    if (mp->shown) release(&mp->shown);
    if (free_this) {
        printf("Freeing %s (%d) %dx%d %p\n",
            mp->shown ? "View" : "Matrix", live_matrices, mp->n, mp->n, mp);
        fflush(stdout);
        free(mp);
        *M = mp = NULL;
        --live_matrices;
    }
    return mp;
}

void release_all(Matrix **M, ...) {
    va_list args;
    va_start(args, M); 
    while (M) {
        release(M);
        M = va_arg(args, Matrix **);
    }
    va_end(args);
}

void release_local(Matrix *mp) {
    assert(mp->ref_count == -1);
    printf("Freeing View %dx%d r(%d) %p ..%p\n",
        mp->n, mp->n, mp->shown ? mp->shown->ref_count : -2,
        mp->ptr, mp->shown ? mp->shown->ptr : NULL);
    if (mp->shown) release(&(mp->shown));
}

void release_all_local(Matrix *M, ...) {
    va_list args;
    va_start(args, M); 
    while (M) {
        release_local(M);
        M = va_arg(args, Matrix *);
    }
    va_end(args);
}

Matrix *matrix_create(int const n) {
    Matrix *M = malloc(sizeof(Matrix) + (size_t)n * (size_t)n * sizeof(float));
    printf("Allocating (%d) %dx%d %p\n", ++live_matrices, n, n, M);
    fflush(stdout);
    if (!M) {
        fprintf(stderr, "Out of memory\n");
        fflush(stderr);        
        abort();
    }
    M->n = n;
    M->row_stride = n;
    M->ref_count = 0;
    M->ptr = (void*)(M+1);
    return retain(&M);
}

bool matrix_same_layouts(const Matrix *A, const Matrix *B) {
    return A->n == B->n && A->row_stride == B->row_stride;
}

size_t matrix_num_bytes(const Matrix *A) {
    return A ? sizeof(*A) + sizeof(float) * A->n * A->row_stride : 0;
}

Matrix *matrix_zero(const Matrix *M) {
    int const N = M->n;
    float *p = M->ptr;
    for (int i = 0; i < N; ++i) {
        memset(p, 0, sizeof(float) * N);
        p += M->row_stride;
    }
    return (Matrix*)M;
}

Matrix *matrix_randomize(const Matrix *M) {
    float *m = M->ptr;
    for (int i = 0; i < M->n; ++i) {
        for (int j = 0; j < M->n; ++j) 
            m[j] = -1. + 2.*((float)rand())/RAND_MAX;
        m += M->row_stride;
    }
    return (Matrix*)M;
}

const Matrix *matrix_copy_to(const Matrix *M, Matrix *A) {
    assert(M->n == A->n);
    if (matrix_same_layouts(M, A)) {
        memcpy(M->ptr, A->ptr, matrix_num_bytes(M));
        release(&A);
        return M;
    }
    float *m = M->ptr, *a = A->ptr;
    int const N = M->n;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) m[j] = a[j];
        m += M->row_stride;
        a += A->row_stride;
    }
    release(&A);
    return M;
}

Matrix *matrix_sum(Matrix *A, Matrix *B) {
    const Matrix *const sum = matrix_create(A->n);
    assert(A->n == B->n);
    float *s = sum->ptr, *a = A->ptr, *b = B->ptr;
    int const N = A->n;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) s[j] = a[j] + b[j];
        s += sum->row_stride;
        a += A->row_stride;
        b += B->row_stride;
    }
    release_all(&A, &B, NULL);
    return (Matrix*)sum;
}

const Matrix *matrix_add_to(const Matrix *sum, Matrix *B) {
    assert(sum->n == B->n);
    float *s = sum->ptr, *b = B->ptr;
    int const N = sum->n;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) s[j] += b[j];
        s += sum->row_stride;
        b += B->row_stride;
    }
    release(&B);
    return sum;
}

Matrix *matrix_diff(Matrix *A, Matrix *B) {
    assert(A->n == B->n);
    int N = A->n;
    const Matrix *diff = matrix_create(N);
    float *s = diff->ptr, *a = A->ptr, *b = B->ptr;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) s[j] = a[j] - b[j];
        s += diff->row_stride;
        a += A->row_stride;
        b += B->row_stride;
    }
    release_all(&A, &B, NULL);
    return (Matrix*)diff;
}

const Matrix *matrix_sub_from(const Matrix *diff, Matrix *B) {
    assert(diff->n == B->n);
    float *d = diff->ptr, *b = B->ptr;
    int const N = diff->n;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) d[j] -= b[j];
        d += diff->row_stride;
        b += B->row_stride;
    }
    release(&B);
    return diff;
}

void print(const Matrix *M) {
    float *m = M->ptr;
    for (int i = 0; i < M->n; ++i) {
        for (int j = 0; j < M->n; ++i) printf("%f\t", m[j]);
        printf("\n");
        m += M->row_stride;
    }
}

//! Returns a sub-block *view* of a given matrix. The block's index is i,j (0-based),
//! out of an nxn square of blocks. Thew view is by-value and is meant to be
//! kept by value. It doesn't allocate.
Matrix matrix_block_view(const Matrix *M, int i, int j, int nbl) {
    const Matrix *shown = M;
    while (shown->shown) shown = shown->shown; // resolve the ultimate target matrix
    Matrix view = { .ref_count = -1, .shown = (Matrix*)shown };
    view.n = M->n / nbl;
    view.row_stride = M->row_stride;
    view.ptr = M->ptr + ((size_t)i * view.n * view.row_stride) + ((size_t)j * view.n);
    printf("New View %dx%d <->%d (%d,%d) %p\n",
        view.n, view.n, view.row_stride, view.ref_count, view.shown->ref_count, view.ptr);
    retain((Matrix**)&shown);
    return view;
}

static void matrix_mac_impl_(const Matrix *C, const Matrix *A, const Matrix *B) {
    float *c = C->ptr, *a = A->ptr, *b = B->ptr;
    const int NA = A->n, NB = B->n;
    for (int i = 0; i < NA; ++i) {
        for (int j = 0; j < NB; ++j) {
            float *bp = b;
            float accum = 0;
            for (int k = 0; k < NA; ++k) {
                accum += a[k] * *bp;
                // row of A * column of B
                bp += B->row_stride;
            }
            c[j] = accum;
            b ++;
        }
        a += A->row_stride;
        c += C->row_stride;
    }
}

void matrix_mul_add_to_impl_(const Matrix *C, const Matrix *A, const Matrix *B) {
    assert(A->n == B->n);
    if (A->n == 1) {
        C->ptr[0] += A->ptr[0] * B->ptr[0];
        return;
    }
    if (A->n == 2) {
        const int A_rs = A->row_stride, B_rs = B->row_stride, C_rs = C->row_stride;
        const float *const Ap = A->ptr, *Bp = B->ptr;
        float *const Cp = C->ptr;
        Cp[0] += Ap[0] * Bp[0]
               + Ap[1] * Bp[0+B_rs];
        Cp[1] += Ap[0] * Bp[1]
               + Ap[1] * Bp[1+B_rs];
        Cp[0+C_rs] += Ap[0+A_rs] * Bp[0]
                    + Ap[1+A_rs] * Bp[0+B_rs];
        Cp[1+C_rs] += Ap[0+A_rs] * Bp[1]
                    + Ap[1+A_rs] * Bp[1+B_rs];
        return;
    }
    // some other special cases could go here
    matrix_mac_impl_(C, A, B);
}

int strassen_entry_count;

Matrix *strassen_mul(Matrix *A, Matrix *B);

const Matrix *strassen_mul_to(const Matrix *C, Matrix *A, Matrix *B) {
    matrix_dump(A);
    matrix_dump(B);
    matrix_dump(C);
    assert(C->n == A->n && C->n == B->n);
    int const N = C->n;
    ++ strassen_entry_count;
    if (N <= 4) { // short-circuit
        matrix_zero(C);
        matrix_mul_add_to_impl_(C, A, B);
    } else {
        Matrix A11 = matrix_block_view(A, 0, 0, 2);
        Matrix A12 = matrix_block_view(A, 0, 1, 2);
        Matrix A21 = matrix_block_view(A, 1, 0, 2);
        Matrix A22 = matrix_block_view(A, 1, 1, 2);
        Matrix B11 = matrix_block_view(B, 0, 0, 2);
        Matrix B12 = matrix_block_view(B, 0, 1, 2);
        Matrix B21 = matrix_block_view(B, 1, 0, 2);
        Matrix B22 = matrix_block_view(B, 1, 1, 2);
        Matrix C11 = matrix_block_view(C, 0, 0, 2);
        Matrix C12 = matrix_block_view(C, 0, 1, 2);
        Matrix C21 = matrix_block_view(C, 1, 0, 2);
        Matrix C22 = matrix_block_view(C, 1, 1, 2);
        const Matrix *M1 = strassen_mul_to(&C22, matrix_sum(&A11, &A22), matrix_sum(&B11, &B22));
        const Matrix *M2 = strassen_mul_to(&C21, matrix_sum(&A21, &A22), &B11);
        const Matrix *M3 = strassen_mul_to(&C12, &A11, matrix_diff(&B12, &B22));
        Matrix *M4 = strassen_mul(&A22, matrix_diff(&B21,& B11));
        Matrix *M5 = strassen_mul(matrix_sum(&A11, &A12), &B22);
        Matrix *M6 = strassen_mul(matrix_diff(&A21, &A11), matrix_sum(&B11, &B12));
        Matrix *M7 = strassen_mul(matrix_diff(&A12, &A22), matrix_sum(&B21, &B22));
        /* C12 == M3 */
        /* C21 == M2 */ 
        /* C22 == M1 */
        // use M1, M2, M3 before they get overwritten
        // C11
        matrix_copy_to(&C11, local(M1));
        matrix_add_to(&C11, retain(&M4));
        matrix_sub_from(&C11, retain(&M5));
        matrix_add_to(&C11, M7);
        // C22 == M1
        matrix_sub_from(&C22, local(M2));
        matrix_add_to(&C22, local(M3));
        matrix_add_to(&C22, M6);
        // C12 == M3
        matrix_add_to(&C12, M5);
        // C21 == M2
        matrix_dump(M4);
        matrix_add_to(&C21, M4);
        release_all_local(&A11, &A12, &A21, &A22, &B11, &B12, &B21, &B22, &C11, &C12, &C21, &C22, NULL);
    }
    release_all(&A, &B, NULL);
    return C;
}

Matrix *strassen_mul(Matrix *A, Matrix *B) {
    Matrix *C = matrix_create(A->n);
    strassen_mul_to(C, A, B);
    return C;
}

typedef struct timeval timeval;

timeval get_time(void) {
    struct timeval result;
    gettimeofday(&result, 0);
    return result;
}

double time_delta(const timeval *start, const timeval *end) {
    double const t1 = start->tv_sec + 1e6*start->tv_usec;
    double const t2 = end->tv_sec + 1e6*end->tv_usec;
    return t2-t1;
}

int main()
{
    size_t const N = 64;
    Matrix *A = matrix_randomize(matrix_create(N));
    Matrix *B = matrix_randomize(matrix_create(N));
    Matrix *C = matrix_create(N);
    
    struct timeval start = get_time();
    strassen_mul_to(C, retain(&A), retain(&B));
    struct timeval end = get_time();

    printf("Number of entries to Strassen multiplication: %d\n\n", strassen_entry_count);
    printf("Total wall time: %f\n", time_delta(&start, &end));

    release_all(&C, &B, &A, NULL);
}
// complete compileable example ends
