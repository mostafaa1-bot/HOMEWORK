#include "fixed_point.h"
#include <stdio.h>
#include <stdint.h>

/* Print raw / 2^q with up to 6 decimal places, truncated (toward zero). */
void print_fixed(int16_t raw, int16_t q) {
    if (q < 0) {
        /* Degenerate case: just print as integer. */
        printf("%d", (int)raw);
        return;
    }

    int16_t sign = 0;
    int32_t abs_raw = raw;
    if (raw < 0) {
        sign = 1;
        abs_raw = -abs_raw;
    }

    /* Integer part: floor(abs_raw / 2^q) */
    int32_t int_part = abs_raw >> q;

    /* Fractional bits */
    int32_t frac_raw = abs_raw & ((1 << q) - 1);

    /* Scale fractional part to 6 decimal digits, truncating (no rounding). */
    int64_t scaled = (int64_t)frac_raw * 1000000;  /* use 64-bit to avoid overflow */
    int32_t frac_part = (q > 0) ? (int32_t)(scaled >> q) : 0;

    if (sign) {
        printf("-");
    }
    printf("%d.%06d", (int)int_part, (int)frac_part);
}

int16_t add_fixed(int16_t a, int16_t b) {
    return (int16_t)(a + b);
}

int16_t subtract_fixed(int16_t a, int16_t b) {
    return (int16_t)(a - b);
}

int16_t multiply_fixed(int16_t a, int16_t b, int16_t q) {
    int64_t product = (int64_t)a * (int64_t)b;
    /* Bring back to the same Q-format by shifting right q bits. */
    return (int16_t)(product >> q);
}

void eval_poly_ax2_minus_bx_plus_c_fixed(int16_t x,
                                         int16_t a,
                                         int16_t b,
                                         int16_t c,
                                         int16_t q) {
    /* Compute y = a*x^2 - b*x + c in fixed-point. */

    /* x^2 */
    int16_t x_sq  = multiply_fixed(x, x, q);
    /* a * x^2 */
    int16_t term1 = multiply_fixed(a, x_sq, q);
    /* b * x */
    int16_t term2 = multiply_fixed(b, x, q);
    /* a*x^2 - b*x */
    int16_t diff  = subtract_fixed(term1, term2);
    /* y = a*x^2 - b*x + c */
    int16_t y     = add_fixed(diff, c);

    printf("the polynomial output for a=");
    print_fixed(a, q);
    printf(", b=");
    print_fixed(b, q);
    printf(", c=");
    print_fixed(c, q);
    printf(" is ");
    print_fixed(y, q);
    printf("\n");
}
