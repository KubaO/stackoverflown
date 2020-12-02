// https://github.com/KubaO/stackoverflown/blob/master/questions/morse-encoding-65095267/main.c
// complete compileable example begins
#include <assert.h>  // assert
#include <math.h>    // log2
#include <stdbool.h> // bool
#include <stddef.h>  // offsetof
#include <stdint.h>  // int_##t
#include <stdio.h>   // printf
#include <string.h>  // strchr

#define IROOT2_1 .7071067812 // 2^-(2^-1)
#define IROOT2_2 .8408964153 // 2^-(2^-2)
#define IROOT2_3 .9170040432 // 2^-(2^-3)
#define IROOT2_4 .9576032807 // 2^-(2^-4)
#define IROOT2_5 .9785720621 // 2^-(2^-5)
#define IROOT2_6 .9892280132 // 2^-(2^-6)
#define IROOT2_7 .9945994235 // 2^-(2^-7)
#define IROOT2_8 .9972960561 // 2^-(2^-8)
#define IROOT2_9 .9986471129 // 2^-(2^-9)
#define IROOT2_A .9993233275 // 2^-(2^-10)
#define IROOT2_B .9996616065 // 2^-(2^-11)
#define IROOT2_C .9998307889 // 2^-(2^-12)

#define BIT_SCAN_REV(n) \
(n>>15?15:n>>14?14:n>>13?13:n>>12?12:n>>11?11:n>>10?10:n>>9?9:\
n>>8?8:n>>7?7:n>>6?6:n>>5?5:n>>4?4:n>>3?3:n>>2?2:n>>1?1:0)

#define FRAC_LOG2_1(m,n) (1./4096.)*\
                         ((m<=n*IROOT2_1?2048:0)+FRAC_LOG2_2(m,n*(m<=n*IROOT2_1?IROOT2_1:1)))
#define FRAC_LOG2_2(m,n) ((m<=n*IROOT2_2?1024:0)+FRAC_LOG2_3(m,n*(m<=n*IROOT2_2?IROOT2_2:1)))
#define FRAC_LOG2_3(m,n)  ((m<=n*IROOT2_3?512:0)+FRAC_LOG2_4(m,n*(m<=n*IROOT2_3?IROOT2_3:1)))
#define FRAC_LOG2_4(m,n)  ((m<=n*IROOT2_4?256:0)+FRAC_LOG2_5(m,n*(m<=n*IROOT2_4?IROOT2_4:1)))
#define FRAC_LOG2_5(m,n)  ((m<=n*IROOT2_5?128:0)+FRAC_LOG2_6(m,n*(m<=n*IROOT2_5?IROOT2_5:1)))
#define FRAC_LOG2_6(m,n)   ((m<=n*IROOT2_6?64:0)+FRAC_LOG2_7(m,n*(m<=n*IROOT2_6?IROOT2_6:1)))
#define FRAC_LOG2_7(m,n)   ((m<=n*IROOT2_7?32:0)+FRAC_LOG2_8(m,n*(m<=n*IROOT2_7?IROOT2_7:1)))
#define FRAC_LOG2_8(m,n)   ((m<=n*IROOT2_8?16:0)+FRAC_LOG2_9(m,n*(m<=n*IROOT2_8?IROOT2_8:1)))
#define FRAC_LOG2_9(m,n)    ((m<=n*IROOT2_9?8:0)+FRAC_LOG2_A(m,n*(m<=n*IROOT2_9?IROOT2_9:1)))
#define FRAC_LOG2_A(m,n)    ((m<=n*IROOT2_A?4:0)+FRAC_LOG2_B(m,n*(m<=n*IROOT2_A?IROOT2_A:1)))
#define FRAC_LOG2_B(m,n)    ((m<=n*IROOT2_B?2:0)+FRAC_LOG2_C(m,n*(m<=n*IROOT2_B?IROOT2_B:1)))
#define FRAC_LOG2_C(m,n)     (m<=n*IROOT2_C?1:0)

#define FRAC_LOG2(n) (BIT_SCAN_REV(n) + FRAC_LOG2_1(1<<BIT_SCAN_REV(n), n))

// FIELD(type, name, minimum, maximum, number_of_values)
#define WITH_MY_BEACON_FIELDS \
    FIELD(float, Battery_1_Current, -1.0f, 0.0f, 64) \
    FIELD(float, Battery_Voltage, 3.0f, 6.0f, 64) \
    FIELD(float, Battery_1_Temperature, 0.0f, 20.0f, 32) \
    FIELD(float, Battery_2_Temperature, 0.0f, 20.0f, 32) \
    FIELD(float, Battery_2_Current, -1.0f, 0.0f, 64) \
    FIELD(float, C_1_Temperature, 5.0f, 30.0f, 64) \
    FIELD(float, C_2_Temperature, 5.0f, 30.0f, 64) \
    FIELD(float, C_Value, 0.0f, 10.0f, 32) \
    FIELD(char, Beacon_transmitted_by, 0, 3, 4) \
    FIELD(char, V_DLP, 0, 1, 2) \
    FIELD(char, V_CPD, 0, 1, 2) \
    FIELD(char, V_ADS, 0, 1, 2) \


struct {
    #define FIELD(type, name, min, max, N) type name;
    WITH_MY_BEACON_FIELDS
    #undef FIELD
} typedef My_Beacon;

enum { float_T, char_T } typedef Type_Id;
const char *Type_Id_Names[] = { "float", "char" };

union {
    float float_;
    char char_;
} typedef Any_Type;

struct {
    Type_Id type;
    uint16_t offset;
    uint16_t num_values;
    Any_Type min;
    Any_Type range;
    const char *name;
} typedef Field_Descriptor;

const Field_Descriptor My_Beacon_Fields[] = {
    #define FIELD(type, name, min, max, N) \
        { type##_T, offsetof(My_Beacon, name), N, \
          {.type##_ = min}, {.type##_ = (max - min)}, #name },
    WITH_MY_BEACON_FIELDS
    #undef FIELD
};

enum { Num_Fields = sizeof(My_Beacon_Fields)/sizeof(My_Beacon_Fields[0]) };

void Dump_Beacon(const My_Beacon *input) {
    const Field_Descriptor *f = My_Beacon_Fields;
    int i;
    for (i = 0; i < Num_Fields; ++f, ++i) {
        const void *src = ((const char*)input) + f->offset;
        printf("@%2d %s %s = ", f->offset, Type_Id_Names[f->type], f->name);
        switch (f->type) {
        case float_T:
            printf("%g\n", (double)*(const float*)src);
            break;
        case char_T:
            printf("%d\n", (int)*(const char*)src);
            break;
        }
    }
}

const char Encoding_Alphabet[] = {
    "abcdefghijklmnopqrstuvwxyz0123456789"
};

enum {
    Num_Alphabet_Chars = sizeof(Encoding_Alphabet)/sizeof(char) - 1,
    Packed_Length = 1 + (int)((4.0
    #define FIELD(type, name, min, max, N) + FRAC_LOG2(N)
    WITH_MY_BEACON_FIELDS
    #undef FIELD
    )/8.0),
    Encoded_Length = (int)(Packed_Length * 8.0 / FRAC_LOG2(Num_Alphabet_Chars))
};

#undef WITH_MY_BEACON_FIELDS

struct {
    uint8_t c[Encoded_Length + 1]; // null-terminated
} typedef Encoded_Beacon;

struct {
    uint8_t c[Packed_Length];
} typedef Packed_Beacon;

//! Long-multiply the packed bit array by a given multiplier, and add a term to it
void P_MAC(Packed_Beacon *p, uint32_t const mul, uint32_t const term) {
    assert(mul <= 0xFFFFFU);
    uint32_t accum = term;
    int i;
    for (i = 0; i < Packed_Length; ++i) {
        uint32_t const accum_prev = accum;
        accum += p->c[i] * mul;
        assert(accum >= accum_prev);
        p->c[i] = (uint8_t)accum;
        accum >>= 8;
    }
}

//! Performs long division and returns the remainder
uint32_t P_DIV(Packed_Beacon *p, uint32_t const divisor)
{
    assert(divisor <= 0xFFFFFF);
    assert(divisor > 0);
    int const divisor_shift = 8;
    uint32_t const divisor_shifted = divisor << divisor_shift;
    uint32_t remainder = 0;
    int i;
    for (i = Packed_Length-1; i >= 0; --i) {
        uint8_t out = 0;
        int j;
        remainder += p->c[i];
        for (j = 0; j < 8; ++j) {
            out <<= 1;
            remainder <<= 1;
            if (remainder >= divisor_shifted) {
                remainder -= divisor_shifted;
                out |= 1;
            }
            assert(remainder < divisor_shifted);
        }
        p->c[i] = out;
    }
    remainder >>= divisor_shift;
    assert(remainder < divisor);
    return remainder;
}

void Pack_char(const void *field, const Field_Descriptor *f, Packed_Beacon *p) {
    int32_t v_char = *(const char*)field;
    printf(" (%d) ", (int)v_char);
    v_char -= f->min.char_;
    if (v_char < 0) v_char = 0;
    v_char *= (f->num_values - 1) + f->range.char_/2;
    v_char /= f->range.char_;
    if (v_char >= f->num_values) v_char = f->num_values - 1;
    printf("char %d\n", (int)v_char);
    P_MAC(p, f->num_values, v_char);
}

void Unpack_char(void *field, const Field_Descriptor *f, Packed_Beacon *p) {
    int32_t v_char = P_DIV(p, f->num_values);
    printf("char %d ", (int)v_char);            
    v_char *= f->range.char_;
    v_char /= f->num_values - 1;
    v_char += f->min.char_;
    int32_t const max = f->min.char_ + f->range.char_;
    if (v_char < f->min.char_) v_char = f->min.char_;
    else if (v_char >= max) v_char = max;
    printf("(%d)\n", (int)v_char);
    *(char*)field = (char)v_char;
}

void Pack_float(const void *field, const Field_Descriptor *f, Packed_Beacon *p) {
    double v_float = *(const float*)field;
    printf(" (%g) ", v_float);
    v_float -= f->min.float_;
    v_float *= (f->num_values - 1) + f->range.float_/2.0;
    v_float /= f->range.float_;
    printf("float %d\n", (int)v_float);
    P_MAC(p, f->num_values, v_float);
}

void Unpack_float(const void *field, const Field_Descriptor *f, Packed_Beacon *p) {
    double v_float = P_DIV(p, f->num_values);
    printf("float %d ", (int)v_float);
    v_float *= f->range.float_;
    v_float /= (f->num_values - 1) + f->range.float_/2.0;
    v_float += f->min.float_;
    double const max = f->min.float_ + f->range.float_;
    if (v_float < f->min.float_) v_float = f->min.float_;
    else if (v_float >= max) v_float = max;
    printf("(%g)\n", v_float);
    *(float*)field = (float)v_float;
}

Packed_Beacon pack(const My_Beacon *input) {
    Packed_Beacon p = {};
    const Field_Descriptor *fd = My_Beacon_Fields;
    int i;
    for (i = 0; i < Num_Fields; ++i, ++fd) {
        const void *field = ((const char*)input) + fd->offset;
        printf("@%d ", fd->offset);
        switch (fd->type) {
        case char_T:
            Pack_char(field, fd, &p);
            break;
        case float_T:
            Pack_float(field, fd, &p);
            break;
        }
    }
    return p;
}

My_Beacon unpack(const Packed_Beacon *input)
{   
    Packed_Beacon p = *input;
    My_Beacon output;
    const Field_Descriptor *fd = &My_Beacon_Fields[Num_Fields-1];
    int i;
    for (i = Num_Fields-1; i >= 0; --i, --fd) {
        void *field = ((char*)&output) + fd->offset;
        printf("@%d ", fd->offset);
        switch (fd->type) {
        case char_T:
            Unpack_char(field, fd, &p);
            break;
        case float_T:
            Unpack_float(field, fd, &p);
            break;
        }
    }
    return output;
}

Encoded_Beacon encode(const Packed_Beacon *input)
{
    Packed_Beacon p = *input;
    Encoded_Beacon e = {};
    int i;
    for (i = 0; i < Encoded_Length; ++i) {
        uint32_t rem = P_DIV(&p, Num_Alphabet_Chars);
        e.c[i] = Encoding_Alphabet[rem];
    }
    for (i = Encoded_Length-1; i > 0; --i) {
        if (e.c[i] == Encoding_Alphabet[0])
            e.c[i] = '\0'; // remove trailing "zeroes"
        else
            break;
    }
    return e;
}

Packed_Beacon decode(const Encoded_Beacon *e)
{
    Packed_Beacon p = {};
    int i;
    for (i = Encoded_Length-1; i >= 0; --i) {
        uint8_t rem = strchr(Encoding_Alphabet, e->c[i]) - Encoding_Alphabet;
        P_MAC(&p, Num_Alphabet_Chars, rem);
    }
    return p;
}

const My_Beacon example_beacon = {
    .Battery_1_Current = -0.75,
    .Battery_Voltage = 4.12,
    .Battery_1_Temperature = 12.32,
    .Battery_2_Temperature = 8.55,
    .Battery_2_Current = -0.22,
    .C_1_Temperature = 19.14,
    .C_2_Temperature = 16.45,
    .C_Value = 5.5,
    .Beacon_transmitted_by = 1,
    .V_DLP = false,
    .V_CPD = true,
    .V_ADS = false
};

#define DUMP(var) printf("%s = %d\n", #var, var)

int main() {
    int i;
    DUMP(Packed_Length);
    DUMP(Encoded_Length);

    Packed_Beacon p1 = pack(&example_beacon);
    Encoded_Beacon e = encode(&p1);
    printf("encoded: \"%s\"\n", e.c);
    Packed_Beacon p2 = decode(&e);
    My_Beacon unpacked = unpack(&p2);
    assert(memcmp(&p1, &p2, sizeof(p1)) == 0);

    printf("\n");
    printf("** Transmit **\n");
    Dump_Beacon(&example_beacon);
    printf("** On Air\n");
    printf("\"%s\"\n", e.c);
    printf("** Receive **\n");
    Dump_Beacon(&unpacked);
}
// complete compileable example ends
