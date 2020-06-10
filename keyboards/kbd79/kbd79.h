#ifndef KBD79_H_
#define KBD79_H_

#include "quantum.h"

#ifdef RGB_MATRIX_ENABLE
//rgb led driver
#include "is31fl3737.h"
#endif

#define LAYOUT( \
    k00, k01, k02, k03, k04, k05, k06, k07, k08, k09, k0A, K0B, K0C, K0D, \
    k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k1A, k1B, k1C, k1D, k3D, \
    k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k2A, k2B, k2C, k2D, \
    k30, k31, k32, k33, k34, k35, k36, k37, k38, k39, k3A, k3B, k3C, \
    k40, k41, k42, k43, k44, k45, k46, k47, k48, k49, k4A, k4B, k4C, k4D, \
    k50, k51, k52,            k53,      k55, k56, k57, k58, k5A \
) \
{ \
    {k00, k01, k02, k03, k04, k05, k06, k07, k08, k09, k0A, K0B, K0C, K0D}, \
    {k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k1A, k1B, k1C, k1D, k3D}, \
    {k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k2A, k2B, k2C,  KC_NO}, \
    {k30, k31, k32, k33, k34, k35, k36, k37, k38, k39, k3A, k3B, k3C}, \
    {k40, k41, k42, k43, k44, k45, k46, k47, k48, k49, k4A, k4B, k4C, k4D}, \
    {k50, k51, k52,            k53,      k55, k56, k57, k58, k5A} \
}


#endif /* KBD79_H_ */
