/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

uint64_t __udivdi3(uint64_t n, uint64_t d) {
    uint64_t q = 0, r = 0;
    
    if (d == 0) return 0;

    for (int i = 63; i >= 0; i--) {
        r <<= 1;
        r |= (n >> i) & 1;
        if (r >= d) {
            r -= d;
            q |= ((uint64_t)1 << i);
        }
    }
    return q;
}

int abs(int j) {
    return j < 0 ? -j : j;
}

double pow(double base, int exp) {
    if(base == 0.0) return 0.0;
    if(exp == 0) return 1.0;

    if(exp < 0) {
        base = 1.0 / base;
        if (exp == -2147483648) { 
            return base * pow(base, 2147483647);
        }
        exp = -exp;
    }

    double result = 1.0;
    while(exp > 0) {
        if (exp & 1) {
            result *= base;
        }
        base *= base;
        exp >>= 1;
    }

    return result;
}

int isfinite(double x) {
    union dshape {
        double value;
        uint64_t bits;
    } u;
    u.value = x;
    uint64_t exp_bits = u.bits & 0x7FF0000000000000ULL;
    return exp_bits != 0x7FF0000000000000ULL;
}

double ldexp(double x, int exp) {
    union dshape {
        double value;
        uint64_t bits;
    } u;
    u.value = x;

    if (x == 0.0 || !isfinite(x)) {
        return x;
    }

    int current_exp = (u.bits >> 52) & 0x7FF;

    if (current_exp == 0) {
        x *= 0x1p54; 
        u.value = x;
        current_exp = ((u.bits >> 52) & 0x7FF) - 54;
    }

    int new_exp = current_exp + exp;

    if (new_exp >= 0x7FF) {
        // errno = ERANGE;
        return x < 0 ? -HUGE_VAL : HUGE_VAL;
    }

    if (new_exp <= 0) {
        if (new_exp < -54) {
            // errno = ERANGE;
            return x < 0 ? -0.0 : 0.0;
        }
        new_exp += 54;
        u.bits = (u.bits & ~0x7FF0000000000000ULL) | ((uint64_t)new_exp << 52);
        u.value *= 0x1p-54;
        return u.value;
    }

    u.bits = (u.bits & ~0x7FF0000000000000ULL) | ((uint64_t)new_exp << 52);
    
    return u.value;
}