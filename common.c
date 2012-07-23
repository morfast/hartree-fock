#include "common.h"
#include <math.h>

/*
 *  放置一些常用函数
 */
void matrix_output(const gsl_matrix *m, int n, char *msg)
{
    int i, j;

    printf("%s\n",msg);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
            printf("%15.6lE", gsl_matrix_get(m, i, j));
        printf("\n");
    }
}

void vector_output(const gsl_vector *v, int n, char *msg)
{
    int i;

    printf("%s\n",msg);
    for (i = 0; i < n; i++)
        printf("%15.06lE", gsl_vector_get(v, i));
    printf("\n");
}

#define F_INC_GAMMA_CYCLE    100
#define F_INC_GAMMA_delta  1.0E-12
double F_inc_gamma(int m ,double w)
{
    double result = 0;
    double tmp = 0;
    int i;
    
    if (w < 17) {
        result = tmp = 1.0 / factorial_2(2*m + 1);
        for (i = 1; i < F_INC_GAMMA_CYCLE; i++) {
            tmp *= ((2*w) / (2*m + 2*i + 1));
            if ((tmp - F_INC_GAMMA_delta) < 0)
                break;
            result += tmp;
        }
        return result * factorial_2(2 * m -1) * exp(-w);;
    }else
        result = factorial_2(2*m -1) / pow(2*w, m + 0.5) * sqrt(M_PI_2);
    return result;
}

int factorial(int n)
{
    int i, result = 1;

    if (n <= 1) return 1;

    for (i = 1; i <= n; i++)
        result *= i;
    return result;
}

int factorial_2(int n)
{
    int i, result = 1;

    if (n % 2 == 0) {
        if (n <= 2) return 2;
        for (i = 2; i <= n; i += 2)
            result *= i;
    }else{
        if (n <= 1) return 1;
        for (i = 1; i <= n; i += 2)
            result *= i;
    }
    return result;
}
