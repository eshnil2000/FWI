/*
 * =============================================================================
 * Copyright (c) 2016, Barcelona Supercomputing Center (BSC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * =============================================================================
 */

#include "test/fwi_tests.h"

#include "fwi/fwi_kernel.h"
#include "fwi/fwi_propagator.h"




////// SetUp/SetDown /////
const extent_t req = {32, 16, 16};
dim_t   dim;
integer nelems;

v_t v_ref;
s_t s_ref;
coeff_t c_ref;
real* rho_ref;

v_t v_cal;
s_t s_cal;
coeff_t c_cal;
real* rho_cal;

void init_array( real* restrict array, const integer length )
{
    for (integer i = 0; i < length; i++)
        array[i] = (i/(1.0+i)) + (rand() + 1.0) / (1.0 * RAND_MAX);

#if defined(_OPENACC)
    #pragma acc update device(array[:length])
#endif
}

void copy_array( real* restrict dest, real* restrict src, const integer length )
{
    for (integer i = 0; i < length; i++)
        dest[i] = src[i];

#if defined(_OPENACC)
    #pragma acc update device(dest[:length])
#endif
}

TEST_GROUP(propagator);

TEST_SETUP(propagator)
{
    alloc_memory_shot(req, &dim, &c_ref, &s_ref, &v_ref, &rho_ref);
    alloc_memory_shot(req, &dim, &c_cal, &s_cal, &v_cal, &rho_cal);

    nelems = dim.pitch * dim.xsize * dim.ysize;

    /* constants initialization */
    init_array(c_ref.c11, nelems);
    init_array(c_ref.c12, nelems);
    init_array(c_ref.c13, nelems);
    init_array(c_ref.c14, nelems);
    init_array(c_ref.c15, nelems);
    init_array(c_ref.c16, nelems);

    init_array(c_ref.c22, nelems);
    init_array(c_ref.c23, nelems);
    init_array(c_ref.c24, nelems);
    init_array(c_ref.c25, nelems);
    init_array(c_ref.c26, nelems);

    init_array(c_ref.c33, nelems);
    init_array(c_ref.c34, nelems);
    init_array(c_ref.c35, nelems);
    init_array(c_ref.c36, nelems);

    init_array(c_ref.c44, nelems);
    init_array(c_ref.c45, nelems);
    init_array(c_ref.c46, nelems);

    init_array(c_ref.c55, nelems);
    init_array(c_ref.c56, nelems);

    init_array(c_ref.c66, nelems);

    /* velocity [reference] initialization */
    init_array(v_ref.tl.u, nelems);
    init_array(v_ref.tl.v, nelems);
    init_array(v_ref.tl.w, nelems);

    init_array(v_ref.tr.u, nelems);
    init_array(v_ref.tr.v, nelems);
    init_array(v_ref.tr.w, nelems);

    init_array(v_ref.bl.u, nelems);
    init_array(v_ref.bl.v, nelems);
    init_array(v_ref.bl.w, nelems);

    init_array(v_ref.br.u, nelems);
    init_array(v_ref.br.v, nelems);
    init_array(v_ref.br.w, nelems);

    /* stresses [reference] initialization */
    init_array(s_ref.tl.zz, nelems);
    init_array(s_ref.tl.xz, nelems);
    init_array(s_ref.tl.yz, nelems);
    init_array(s_ref.tl.xx, nelems);
    init_array(s_ref.tl.xy, nelems);
    init_array(s_ref.tl.yy, nelems);

    init_array(s_ref.tr.zz, nelems);
    init_array(s_ref.tr.xz, nelems);
    init_array(s_ref.tr.yz, nelems);
    init_array(s_ref.tr.xx, nelems);
    init_array(s_ref.tr.xy, nelems);
    init_array(s_ref.tr.yy, nelems);

    init_array(s_ref.bl.zz, nelems);
    init_array(s_ref.bl.xz, nelems);
    init_array(s_ref.bl.yz, nelems);
    init_array(s_ref.bl.xx, nelems);
    init_array(s_ref.bl.xy, nelems);
    init_array(s_ref.bl.yy, nelems);

    init_array(s_ref.br.zz, nelems);
    init_array(s_ref.br.xz, nelems);
    init_array(s_ref.br.yz, nelems);
    init_array(s_ref.br.xx, nelems);
    init_array(s_ref.br.xy, nelems);
    init_array(s_ref.br.yy, nelems);

    /* init rho */
    init_array(rho_ref, nelems);

    ///////////////////////////////
    // Copy 'ref' values into 'cal'
    // to start from equal solutions

    copy_array(v_cal.tl.u, v_ref.tl.u, nelems);
    copy_array(v_cal.tl.v, v_ref.tl.v, nelems);
    copy_array(v_cal.tl.w, v_ref.tl.w, nelems);

    copy_array(v_cal.tr.u, v_ref.tr.u, nelems);
    copy_array(v_cal.tr.v, v_ref.tr.v, nelems);
    copy_array(v_cal.tr.w, v_ref.tr.w, nelems);

    copy_array(v_cal.bl.u, v_ref.bl.u, nelems);
    copy_array(v_cal.bl.v, v_ref.bl.v, nelems);
    copy_array(v_cal.bl.w, v_ref.bl.w, nelems);

    copy_array(v_cal.br.u, v_ref.br.u, nelems);
    copy_array(v_cal.br.v, v_ref.br.v, nelems);
    copy_array(v_cal.br.w, v_ref.br.w, nelems);

    copy_array(s_cal.tl.zz, s_ref.tl.zz, nelems);
    copy_array(s_cal.tl.xz, s_ref.tl.xz, nelems);
    copy_array(s_cal.tl.yz, s_ref.tl.yz, nelems);
    copy_array(s_cal.tl.xx, s_ref.tl.xx, nelems);
    copy_array(s_cal.tl.xy, s_ref.tl.xy, nelems);
    copy_array(s_cal.tl.yy, s_ref.tl.yy, nelems);

    copy_array(s_cal.tr.zz, s_ref.tr.zz, nelems);
    copy_array(s_cal.tr.xz, s_ref.tr.xz, nelems);
    copy_array(s_cal.tr.yz, s_ref.tr.yz, nelems);
    copy_array(s_cal.tr.xx, s_ref.tr.xx, nelems);
    copy_array(s_cal.tr.xy, s_ref.tr.xy, nelems);
    copy_array(s_cal.tr.yy, s_ref.tr.yy, nelems);

    copy_array(s_cal.bl.zz, s_ref.bl.zz, nelems);
    copy_array(s_cal.bl.xz, s_ref.bl.xz, nelems);
    copy_array(s_cal.bl.yz, s_ref.bl.yz, nelems);
    copy_array(s_cal.bl.xx, s_ref.bl.xx, nelems);
    copy_array(s_cal.bl.xy, s_ref.bl.xy, nelems);
    copy_array(s_cal.bl.yy, s_ref.bl.yy, nelems);

    copy_array(s_cal.br.zz, s_ref.br.zz, nelems);
    copy_array(s_cal.br.xz, s_ref.br.xz, nelems);
    copy_array(s_cal.br.yz, s_ref.br.yz, nelems);
    copy_array(s_cal.br.xx, s_ref.br.xx, nelems);
    copy_array(s_cal.br.xy, s_ref.br.xy, nelems);
    copy_array(s_cal.br.yy, s_ref.br.yy, nelems);
}

TEST_TEAR_DOWN(propagator)
{
    free_memory_shot(&c_ref, &s_ref, &v_ref, &rho_ref);
    free_memory_shot(&c_cal, &s_cal, &v_cal, &rho_cal);
}

/////// TESTS ////////////

TEST(propagator, IDX)
{
    TEST_ASSERT_EQUAL_INT( 0*dim.xsize*dim.pitch + 0*dim.pitch + 0, IDX(0, 0, 0, dim) );
    TEST_ASSERT_EQUAL_INT( 1*dim.xsize*dim.pitch + 0*dim.pitch + 0, IDX(0, 0, 1, dim) );
    TEST_ASSERT_EQUAL_INT( 0*dim.xsize*dim.pitch + 1*dim.pitch + 0, IDX(0, 1, 0, dim) );
    TEST_ASSERT_EQUAL_INT( 1*dim.xsize*dim.pitch + 1*dim.pitch + 0, IDX(0, 1, 1, dim) );
    TEST_ASSERT_EQUAL_INT( 0*dim.xsize*dim.pitch + 0*dim.pitch + 1, IDX(1, 0, 0, dim) );
    TEST_ASSERT_EQUAL_INT( 1*dim.xsize*dim.pitch + 0*dim.pitch + 1, IDX(1, 0, 1, dim) );
    TEST_ASSERT_EQUAL_INT( 0*dim.xsize*dim.pitch + 1*dim.pitch + 1, IDX(1, 1, 0, dim) );
    TEST_ASSERT_EQUAL_INT( 1*dim.xsize*dim.pitch + 1*dim.pitch + 1, IDX(1, 1, 1, dim) );
}

TEST(propagator, stencil_Z)
{
    const integer FORWARD  = 1;
    const integer BACKWARD = 0;

    const real dzi = 1.0;

    for (integer y =    0; y < dim.ysize;        y++)
    for (integer x =    0; x < dim.xsize;        x++)
    for (integer z = HALO; z < dim.zsize - HALO; z++)
    {
        TEST_ASSERT_EQUAL_FLOAT(stencil_Z(FORWARD,  v_ref.tl.u, dzi, z,   x, y, dim),
                                stencil_Z(BACKWARD, v_ref.tl.u, dzi, z+1, x, y, dim));
    }
}

TEST(propagator, stencil_X)
{
    const integer FORWARD  = 1;
    const integer BACKWARD = 0;

    const real dzi = 1.0;

    for (integer y =    0; y < dim.ysize;        y++)
    for (integer x = HALO; x < dim.xsize - HALO; x++)
    for (integer z =    0; z < dim.zsize;        z++)
    {
        TEST_ASSERT_EQUAL_FLOAT(stencil_X(FORWARD,  v_ref.tl.v, dzi, z, x,   y, dim),
                                stencil_X(BACKWARD, v_ref.tl.v, dzi, z, x+1, y, dim));
    }
}

TEST(propagator, stencil_Y)
{
    const integer FORWARD  = 1;
    const integer BACKWARD = 0;

    const real dzi = 1.0;

    for (integer y = HALO; y < dim.ysize - HALO; y++)
    for (integer x =    0; x < dim.xsize;        x++)
    for (integer z =    0; z < dim.zsize;        z++)
    {
        TEST_ASSERT_EQUAL_FLOAT(stencil_Y(FORWARD,  v_ref.tl.w, dzi, z, x, y,   dim),
                                stencil_Y(BACKWARD, v_ref.tl.w, dzi, z, x, y+1, dim));
    }
}

TEST(propagator, rho_BL)
{
    for (integer y = 0; y < dim.ysize;   y++)
    for (integer x = 0; x < dim.xsize;   x++)
    for (integer z = 0; z < dim.zsize-1; z++)
    {
        TEST_ASSERT_EQUAL_FLOAT((2.0f/(v_ref.bl.u[IDX(z  ,x,y,dim)]+v_ref.bl.u[IDX(z+1,x,y,dim)])),
                                rho_BL(v_ref.bl.u, z, x, y, dim));
    }
}

TEST(propagator, rho_TR)
{
    for (integer y = 0; y < dim.ysize;   y++)
    for (integer x = 0; x < dim.xsize-1; x++)
    for (integer z = 0; z < dim.zsize;   z++)
    {
        TEST_ASSERT_EQUAL_FLOAT((2.0f/(v_ref.tr.u[IDX(z,x,y,dim)]+v_ref.tr.u[IDX(z,x+1,y,dim)])),
                                rho_TR(v_ref.tr.u, z, x, y, dim));
    }
}

TEST(propagator, rho_BR)
{
    for (integer y = 0; y < dim.ysize-1; y++)
    for (integer x = 0; x < dim.xsize-1; x++)
    for (integer z = 0; z < dim.zsize-1; z++)
    {
        const real ref = ( 8.0f / ( v_ref.br.u[IDX(z  ,x  ,y  ,dim)] +
                                    v_ref.br.u[IDX(z+1,x  ,y  ,dim)] +
                                    v_ref.br.u[IDX(z  ,x+1,y  ,dim)] +
                                    v_ref.br.u[IDX(z  ,x  ,y+1,dim)] +
                                    v_ref.br.u[IDX(z  ,x+1,y+1,dim)] +
                                    v_ref.br.u[IDX(z+1,x+1,y  ,dim)] +
                                    v_ref.br.u[IDX(z+1,x  ,y+1,dim)] +
                                    v_ref.br.u[IDX(z+1,x+1,y+1,dim)]) );

        const real cal = rho_BR(v_ref.br.u, z, x, y, dim);

        TEST_ASSERT_EQUAL_FLOAT(ref, cal);
    }
}

TEST(propagator, rho_TL)
{
    for (integer y = 0; y < dim.ysize-1; y++)
    for (integer x = 0; x < dim.xsize;   x++)
    for (integer z = 0; z < dim.zsize;   z++)
    {
        TEST_ASSERT_EQUAL_FLOAT((2.0f/(v_ref.tl.u[IDX(z,x,y,dim)]+v_ref.tl.u[IDX(z,x,y+1,dim)])),
                                rho_TL(v_ref.tl.u, z, x, y, dim));
    }
}



TEST(propagator, compute_component_vcell_TL)
{
    const real     dt  = 1.0;
    const real     dzi = 1.0;
    const real     dxi = 1.0;
    const real     dyi = 1.0;
    const integer  nz0 = HALO;
    const integer  nzf = dim.zsize-HALO;
    const integer  nx0 = HALO;
    const integer  nxf = dim.xsize-HALO;
    const integer  ny0 = HALO;
    const integer  nyf = dim.ysize-HALO;
    const offset_t SZ = 0;
    const offset_t SX = 0;
    const offset_t SY = 0;
    const phase_t  phase = TWO;

    const real*    szptr = s_ref.bl.xz;
    const real*    sxptr = s_ref.tr.xx;
    const real*    syptr = s_ref.tl.xy;

    // REFERENCE CALCULATION -DON'T TOUCH-
    for(integer y=ny0; y < nyf; y++)
    {
        for(integer x=nx0; x < nxf; x++)
        {
            for(integer z=nz0; z < nzf; z++)
            {
                const real lrho = rho_TL(rho_ref, z, x, y, dim);

                const real stx  = stencil_X( SX, sxptr, dxi, z, x, y, dim);
                const real sty  = stencil_Y( SY, syptr, dyi, z, x, y, dim);
                const real stz  = stencil_Z( SZ, szptr, dzi, z, x, y, dim);
    
                v_ref.tl.u[IDX(z,x,y,dim)] += (stx  + sty  + stz) * dt * lrho;
            }
        }
    }
    ///////////////////////////////////////
    
    {
        compute_component_vcell_TL( v_cal.tl.u, szptr, sxptr, syptr, rho_ref,
            dt, dzi, dxi, dyi,
            nz0, nzf, nx0, nxf, ny0, nyf,
            SZ, SX, SY, dim, phase);
    }
#if defined(_OPENACC)
    #pragma acc update host(v_cal.tl.u[:nelems]) wait(phase)
#endif
    
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.tl.u, v_cal.tl.u, dim );
}

TEST(propagator, compute_component_vcell_TR)
{
    const real     dt  = 1.0;
    const real     dzi = 1.0;
    const real     dxi = 1.0;
    const real     dyi = 1.0;
    const integer  nz0 = HALO;
    const integer  nzf = dim.zsize-HALO;
    const integer  nx0 = HALO;
    const integer  nxf = dim.xsize-HALO;
    const integer  ny0 = HALO;
    const integer  nyf = dim.ysize-HALO;
    const offset_t SZ = 0;
    const offset_t SX = 0;
    const offset_t SY = 0;
    const phase_t  phase = TWO;

    const real*    szptr = s_ref.br.xz;
    const real*    sxptr = s_ref.tl.xx;
    const real*    syptr = s_ref.tr.xy;

    // REFERENCE CALCULATION -DON'T TOUCH-
    for(integer y=ny0; y < nyf; y++)
    {
        for(integer x=nx0; x < nxf; x++)
        {
            for(integer z=nz0; z < nzf; z++)
            {
                const real lrho = rho_TR(rho_ref, z, x, y, dim);
                
                const real stx  = stencil_X( SX, sxptr, dxi, z, x, y, dim);
                const real sty  = stencil_Y( SY, syptr, dyi, z, x, y, dim);
                const real stz  = stencil_Z( SZ, szptr, dzi, z, x, y, dim);

                v_ref.tr.u[IDX(z,x,y,dim)] += (stx  + sty  + stz) * dt * lrho;
            }
        }
    }
    ///////////////////////////////////////

    {
        compute_component_vcell_TR( v_cal.tr.u, szptr, sxptr, syptr, rho_ref,
            dt, dzi, dxi, dyi,
            nz0, nzf, nx0, nxf, ny0, nyf,
            SZ, SX, SY, dim, phase);
    } 
#if defined(_OPENACC)
    #pragma acc update host(v_cal.tr.u[:nelems]) wait(phase)
#endif

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.tr.u, v_cal.tr.u, dim );
}


TEST(propagator, compute_component_vcell_BR)
{
    const real     dt  = 1.0;
    const real     dzi = 1.0;
    const real     dxi = 1.0;
    const real     dyi = 1.0;
    const integer  nz0 = HALO;
    const integer  nzf = dim.zsize-HALO;
    const integer  nx0 = HALO;
    const integer  nxf = dim.xsize-HALO;
    const integer  ny0 = HALO;
    const integer  nyf = dim.ysize-HALO;
    const offset_t SZ = 0;
    const offset_t SX = 0;
    const offset_t SY = 0;
    const phase_t  phase = TWO;

    const real*    szptr = s_ref.tr.xz;
    const real*    sxptr = s_ref.bl.xx;
    const real*    syptr = s_ref.br.xy;

    // REFERENCE CALCULATION -DON'T TOUCH-
    for(integer y=ny0; y < nyf; y++)
    {
        for(integer x=nx0; x < nxf; x++)
        {
            for(integer z=nz0; z < nzf; z++)
            {
#if defined(VCELL_BR_TEXTURE)
                // CUDA TEXTURE TRILINEAR-INTERPOLATION has a flaw:
                // coefficients 'alpha', 'beta' & 'gamma' are stored
                // in 9-bit fixed point format with 8-bit fractional value
                //
                // therefore precission is VERY POOR compared with traditional
                // IEE754 32bit
                //
                // HERE we emulate 9-bit coefficients:
                float increment = 0.5f;
                float zb = (float)z - 0.5f + increment;
                float xb = (float)x - 0.5f + increment;
                float yb = (float)y - 0.5f + increment;
 
                float a = zb - (float)(int)zb;
                float b = xb - (float)(int)xb;
                float c = yb - (float)(int)yb;
                {
                    float a256 = (int) (a*256.0f+0.5f);
                    float b256 = (int) (b*256.0f+0.5f);
                    float c256 = (int) (c*256.0f+0.5f);
                
                    a = a256/256.0f;
                    b = b256/256.0f;
                    c = c256/256.0f;
                }
                int i = (int) zb;
                int j = (int) xb;
                int k = (int) yb;

                const real lrho = 1.0f/
                   ((1.0f-a)*(1.0f-b)*(1.0f-c)*rho_ref[IDX(i  ,j  ,k  ,dim)] +
                          a *(1.0f-b)*(1.0f-c)*rho_ref[IDX(i+1,j  ,k  ,dim)] +
                    (1.0f-a)*      b *(1.0f-c)*rho_ref[IDX(i  ,j+1,k  ,dim)] +
                          a *      b *(1.0f-c)*rho_ref[IDX(i+1,j+1,k  ,dim)] +
                    (1.0f-a)*(1.0f-b)*      c *rho_ref[IDX(i  ,j  ,k+1,dim)] +
                          a *(1.0f-b)*      c *rho_ref[IDX(i+1,j  ,k+1,dim)] +
                    (1.0f-a)*      b *      c *rho_ref[IDX(i  ,j+1,k+1,dim)] +
                          a *      b *      c *rho_ref[IDX(i+1,j+1,k+1,dim)]);
#else
                const real lrho = rho_BR(rho_ref, z, x, y, dim);
#endif

                const real stx  = stencil_X( SX, sxptr, dxi, z, x, y, dim);
                const real sty  = stencil_Y( SY, syptr, dyi, z, x, y, dim);
                const real stz  = stencil_Z( SZ, szptr, dzi, z, x, y, dim);

                v_ref.br.u[IDX(z,x,y,dim)] += (stx  + sty  + stz) * dt * lrho;
            }
        }
    }
    ///////////////////////////////////////

    {
        compute_component_vcell_BR( v_cal.br.u, szptr, sxptr, syptr, rho_ref,
            dt, dzi, dxi, dyi,
            nz0, nzf, nx0, nxf, ny0, nyf,
            SZ, SX, SY, dim, phase);
    }
#if defined(_OPENACC)
    #pragma acc update host(v_cal.br.u[:nelems]) wait(phase)
#endif

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.br.u, v_cal.br.u, dim );
}

TEST(propagator, compute_component_vcell_BL)
{
    const real     dt  = 1.0;
    const real     dzi = 1.0;
    const real     dxi = 1.0;
    const real     dyi = 1.0;
    const integer  nz0 = HALO;
    const integer  nzf = dim.zsize-HALO;
    const integer  nx0 = HALO;
    const integer  nxf = dim.xsize-HALO;
    const integer  ny0 = HALO;
    const integer  nyf = dim.ysize-HALO;
    const offset_t SZ = 0;
    const offset_t SX = 0;
    const offset_t SY = 0;
    const phase_t  phase = TWO;

    const real*    szptr = s_ref.tl.xz;
    const real*    sxptr = s_ref.br.xx;
    const real*    syptr = s_ref.bl.xy;

    // REFERENCE CALCULATION -DON'T TOUCH-
    for(integer y=ny0; y < nyf; y++)
    {
        for(integer x=nx0; x < nxf; x++)
        {
            for(integer z=nz0; z < nzf; z++)
            {
                const real lrho = rho_BL(rho_ref, z, x, y, dim);

                const real stx  = stencil_X( SX, sxptr, dxi, z, x, y, dim);
                const real sty  = stencil_Y( SY, syptr, dyi, z, x, y, dim);
                const real stz  = stencil_Z( SZ, szptr, dzi, z, x, y, dim);

                v_ref.bl.u[IDX(z,x,y,dim)] += (stx  + sty  + stz) * dt * lrho;
            }
        }
    }
    ///////////////////////////////////////

    {
        compute_component_vcell_BL( v_cal.bl.u, szptr, sxptr, syptr, rho_ref,
            dt, dzi, dxi, dyi,
            nz0, nzf, nx0, nxf, ny0, nyf,
            SZ, SX, SY, dim, phase);
    }
#if defined(_OPENACC)
    #pragma acc update host(v_cal.bl.u[:nelems]) wait(phase)
#endif

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.bl.u, v_cal.bl.u, dim );
}

TEST(propagator, velocity_propagator)
{
    const real     dt  = 1.0;
    const real     dzi = 1.0;
    const real     dxi = 1.0;
    const real     dyi = 1.0;
    const integer  nz0 = HALO;
    const integer  nzf = dim.zsize-HALO;
    const integer  nx0 = HALO;
    const integer  nxf = dim.xsize-HALO;
    const integer  ny0 = HALO;
    const integer  nyf = dim.ysize-HALO;
    const phase_t  phase = TWO;

    // REFERENCE CALCULATION
    {
        compute_component_vcell_TL (v_ref.tl.w, s_ref.bl.zz, s_ref.tr.xz, s_ref.tl.yz, rho_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, back_offset, back_offset, forw_offset, dim, phase);
        compute_component_vcell_TR (v_ref.tr.w, s_ref.br.zz, s_ref.tl.xz, s_ref.tr.yz, rho_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, back_offset, forw_offset, back_offset, dim, phase);
        compute_component_vcell_BL (v_ref.bl.w, s_ref.tl.zz, s_ref.br.xz, s_ref.bl.yz, rho_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, forw_offset, back_offset, back_offset, dim, phase);
        compute_component_vcell_BR (v_ref.br.w, s_ref.tr.zz, s_ref.bl.xz, s_ref.br.yz, rho_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, forw_offset, forw_offset, forw_offset, dim, phase);
        compute_component_vcell_TL (v_ref.tl.u, s_ref.bl.xz, s_ref.tr.xx, s_ref.tl.xy, rho_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, back_offset, back_offset, forw_offset, dim, phase);
        compute_component_vcell_TR (v_ref.tr.u, s_ref.br.xz, s_ref.tl.xx, s_ref.tr.xy, rho_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, back_offset, forw_offset, back_offset, dim, phase);
        compute_component_vcell_BL (v_ref.bl.u, s_ref.tl.xz, s_ref.br.xx, s_ref.bl.xy, rho_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, forw_offset, back_offset, back_offset, dim, phase);
        compute_component_vcell_BR (v_ref.br.u, s_ref.tr.xz, s_ref.bl.xx, s_ref.br.xy, rho_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, forw_offset, forw_offset, forw_offset, dim, phase);
        compute_component_vcell_TL (v_ref.tl.v, s_ref.bl.yz, s_ref.tr.xy, s_ref.tl.yy, rho_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, back_offset, back_offset, forw_offset, dim, phase);
        compute_component_vcell_TR (v_ref.tr.v, s_ref.br.yz, s_ref.tl.xy, s_ref.tr.yy, rho_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, back_offset, forw_offset, back_offset, dim, phase);
        compute_component_vcell_BL (v_ref.bl.v, s_ref.tl.yz, s_ref.br.xy, s_ref.bl.yy, rho_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, forw_offset, back_offset, back_offset, dim, phase);
        compute_component_vcell_BR (v_ref.br.v, s_ref.tr.yz, s_ref.bl.xy, s_ref.br.yy, rho_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, forw_offset, forw_offset, forw_offset, dim, phase);
    }
    ///////////////////////////////////////


    {
        velocity_propagator(v_cal, s_ref, c_ref, rho_ref,
                dt, dzi, dxi, dyi,
                nz0, nzf, nx0, nxf, ny0, nyf,
                dim, phase);
    }

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.bl.u, v_cal.bl.u, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.bl.v, v_cal.bl.v, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.bl.w, v_cal.bl.w, dim );

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.br.u, v_cal.br.u, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.br.v, v_cal.br.v, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.br.w, v_cal.br.w, dim );

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.tr.u, v_cal.tr.u, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.tr.v, v_cal.tr.v, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.tr.w, v_cal.tr.w, dim );

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.tl.u, v_cal.tl.u, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.tl.v, v_cal.tl.v, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( v_ref.tl.w, v_cal.tl.w, dim );
}

TEST(propagator, stress_update)
{
    const real dt = 1.0;
    const real c1 = 1.0;
    const real c2 = 2.0;
    const real c3 = 3.0;
    const real c4 = 4.0;
    const real c5 = 6.0;
    const real c6 = 6.0;

    const real u_x = 5.0;
    const real v_x = 6.0;
    const real w_x = 7.0;

    const real u_y = 8.0;
    const real v_y = 9.0;
    const real w_y = 10.0;

    const real u_z = 11.0;
    const real v_z = 12.0;
    const real w_z = 13.0;

    // REFERENCE CALCULATION -DON'T TOUCH-
    for (integer y = 0; y < dim.ysize-1; y++)
    for (integer x = 0; x < dim.xsize-1; x++)
    for (integer z = 0; z < dim.zsize-1; z++)
    {
        const real accum = (dt * c1 * u_x) +
                           (dt * c2 * v_y) +
                           (dt * c3 * w_z) +
                           (dt * c4 * (w_y + v_z)) +
                           (dt * c5 * (w_x + u_z)) +
                           (dt * c6 * (v_x + u_y));
        s_ref.tr.xz[IDX(z,x,y,dim)] += accum;
    }
    ////////////////////////////////////

    for (integer y = 0; y < dim.ysize-1; y++)
    for (integer x = 0; x < dim.xsize-1; x++)
    for (integer z = 0; z < dim.zsize-1; z++)
    {
        stress_update( s_cal.tr.xz, c1, c2, c3, c4, c5, c6,
                z, x, y, dt, u_x, u_y, u_z, v_x, v_y, v_z, w_x, w_y, w_z,
                dim );
    }

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.xz, s_cal.tr.xz, dim );
}


TEST(propagator, cell_coeff_BR)
{
    for (integer y = 0; y < dim.ysize-1; y++)
    for (integer x = 0; x < dim.xsize-1; x++)
    for (integer z = 0; z < dim.zsize-1; z++)
    {
        const real ref = ( 1.0f / (2.5f *( c_ref.c11[IDX(z  ,x  ,y  ,dim)] +
                                           c_ref.c11[IDX(z  ,x+1,y  ,dim)] +
                                           c_ref.c11[IDX(z+1,x  ,y  ,dim)] +
                                           c_ref.c11[IDX(z+1,x+1,y  ,dim)])) );

        const real cal = cell_coeff_BR( c_ref.c11, z, x, y, dim);

        TEST_ASSERT_EQUAL_FLOAT(ref, cal);
    }
}

TEST(propagator, cell_coeff_TL)
{
    for (integer y = 0; y < dim.ysize-1; y++)
    for (integer x = 0; x < dim.xsize-1; x++)
    for (integer z = 0; z < dim.zsize-1; z++)
    {
        const real ref = ( 1.0f / c_ref.c11[IDX(z,x,y,dim)]);

        const real cal = cell_coeff_TL( c_ref.c11, z, x, y, dim);

        TEST_ASSERT_EQUAL_FLOAT(ref, cal);
    }
}

TEST(propagator, cell_coeff_BL)
{
    for (integer y = 0; y < dim.ysize-1; y++)
    for (integer x = 0; x < dim.xsize-1; x++)
    for (integer z = 0; z < dim.zsize-1; z++)
    {
        const real ref = ( 1.0f / (2.5f *( c_ref.c11[IDX(z  ,x  ,y  ,dim)] +
                                           c_ref.c11[IDX(z  ,x  ,y+1,dim)] +
                                           c_ref.c11[IDX(z+1,x  ,y  ,dim)] +
                                           c_ref.c11[IDX(z+1,x  ,y+1,dim)])) );

        const real cal = cell_coeff_BL( c_ref.c11, z, x, y, dim);

        TEST_ASSERT_EQUAL_FLOAT(ref, cal);
    }
}

TEST(propagator, cell_coeff_TR)
{
    for (integer y = 0; y < dim.ysize-1; y++)
    for (integer x = 0; x < dim.xsize-1; x++)
    for (integer z = 0; z < dim.zsize-1; z++)
    {
        const real ref = ( 1.0f / (2.5f *( c_ref.c11[IDX(z  ,x  ,y  ,dim)] +
                                           c_ref.c11[IDX(z  ,x+1,y  ,dim)] +
                                           c_ref.c11[IDX(z  ,x  ,y+1,dim)] +
                                           c_ref.c11[IDX(z  ,x+1,y+1,dim)])) );

        const real cal = cell_coeff_TR( c_ref.c11, z, x, y, dim);

        TEST_ASSERT_EQUAL_FLOAT(ref, cal);
    }
}

TEST(propagator, cell_coeff_ARTM_BR)
{
    for (integer y = 0; y < dim.ysize-1; y++)
    for (integer x = 0; x < dim.xsize-1; x++)
    for (integer z = 0; z < dim.zsize-1; z++)
    {
        const real ref = ((1.0f / c_ref.c11[IDX(z  ,x  ,y  ,dim)] +
                           1.0f / c_ref.c11[IDX(z  ,x+1,y  ,dim)] +
                           1.0f / c_ref.c11[IDX(z+1,x  ,y  ,dim)] +
                           1.0f / c_ref.c11[IDX(z+1,x+1,y  ,dim)])* 0.25f);

        const real cal = cell_coeff_ARTM_BR( c_ref.c11, z, x, y, dim);

        TEST_ASSERT_EQUAL_FLOAT(ref, cal);
    }
}

TEST(propagator, cell_coeff_ARTM_TL)
{
    for (integer y = 0; y < dim.ysize-1; y++)
    for (integer x = 0; x < dim.xsize-1; x++)
    for (integer z = 0; z < dim.zsize-1; z++)
    {
        const real ref = (1.0f / c_ref.c11[IDX(z,x,y,dim)]);

        const real cal = cell_coeff_ARTM_TL( c_ref.c11, z, x, y, dim);

        TEST_ASSERT_EQUAL_FLOAT(ref, cal);
    }
}

TEST(propagator, cell_coeff_ARTM_BL)
{
    for (integer y = 0; y < dim.ysize-1; y++)
    for (integer x = 0; x < dim.xsize-1; x++)
    for (integer z = 0; z < dim.zsize-1; z++)
    {
        const real ref = ((1.0f / c_ref.c11[IDX(z  ,x  ,y  ,dim)] +
                           1.0f / c_ref.c11[IDX(z  ,x  ,y+1,dim)] +
                           1.0f / c_ref.c11[IDX(z+1,x  ,y  ,dim)] +
                           1.0f / c_ref.c11[IDX(z+1,x  ,y+1,dim)])* 0.25f);

        const real cal = cell_coeff_ARTM_BL( c_ref.c11, z, x, y, dim);

        TEST_ASSERT_EQUAL_FLOAT(ref, cal);
    }
}

TEST(propagator, cell_coeff_ARTM_TR)
{
    for (integer y = 0; y < dim.ysize-1; y++)
    for (integer x = 0; x < dim.xsize-1; x++)
    for (integer z = 0; z < dim.zsize-1; z++)
    {
        const real ref = ((1.0f / c_ref.c11[IDX(z  ,x  ,y  ,dim)] +
                           1.0f / c_ref.c11[IDX(z  ,x+1,y  ,dim)] +
                           1.0f / c_ref.c11[IDX(z  ,x  ,y+1,dim)] +
                           1.0f / c_ref.c11[IDX(z  ,x+1,y+1,dim)])* 0.25f);

        const real cal = cell_coeff_ARTM_TR( c_ref.c11, z, x, y, dim);

        TEST_ASSERT_EQUAL_FLOAT(ref, cal);
    }
}

TEST(propagator, compute_component_scell_TR)
{
    const real     dt  = 1.0;
    const real     dzi = 1.0;
    const real     dxi = 1.0;
    const real     dyi = 1.0;
    const integer  nz0 = HALO;
    const integer  nzf = dim.zsize-HALO;
    const integer  nx0 = HALO;
    const integer  nxf = dim.xsize-HALO;
    const integer  ny0 = HALO;
    const integer  nyf = dim.ysize-HALO;
    const offset_t SZ = 0;
    const offset_t SX = 0;
    const offset_t SY = 0;
    const phase_t  phase = TWO;

    // REFERENCE CALCULATION -DON'T TOUCH-
    for (integer y = ny0; y < nyf; y++)
    for (integer x = nx0; x < nxf; x++)
    for (integer z = nz0; z < nzf; z++ )
    {
        const real c11 = cell_coeff_TR      (c_ref.c11, z, x, y, dim);
        const real c12 = cell_coeff_TR      (c_ref.c12, z, x, y, dim);
        const real c13 = cell_coeff_TR      (c_ref.c13, z, x, y, dim);
        const real c14 = cell_coeff_ARTM_TR (c_ref.c14, z, x, y, dim);
        const real c15 = cell_coeff_ARTM_TR (c_ref.c15, z, x, y, dim);
        const real c16 = cell_coeff_ARTM_TR (c_ref.c16, z, x, y, dim);
        const real c22 = cell_coeff_TR      (c_ref.c22, z, x, y, dim);
        const real c23 = cell_coeff_TR      (c_ref.c23, z, x, y, dim);
        const real c24 = cell_coeff_ARTM_TR (c_ref.c24, z, x, y, dim);
        const real c25 = cell_coeff_ARTM_TR (c_ref.c25, z, x, y, dim);
        const real c26 = cell_coeff_ARTM_TR (c_ref.c26, z, x, y, dim);
        const real c33 = cell_coeff_TR      (c_ref.c33, z, x, y, dim);
        const real c34 = cell_coeff_ARTM_TR (c_ref.c34, z, x, y, dim);
        const real c35 = cell_coeff_ARTM_TR (c_ref.c35, z, x, y, dim);
        const real c36 = cell_coeff_ARTM_TR (c_ref.c36, z, x, y, dim);
        const real c44 = cell_coeff_TR      (c_ref.c44, z, x, y, dim);
        const real c45 = cell_coeff_ARTM_TR (c_ref.c45, z, x, y, dim);
        const real c46 = cell_coeff_ARTM_TR (c_ref.c46, z, x, y, dim);
        const real c55 = cell_coeff_TR      (c_ref.c55, z, x, y, dim);
        const real c56 = cell_coeff_ARTM_TR (c_ref.c56, z, x, y, dim);
        const real c66 = cell_coeff_TR      (c_ref.c66, z, x, y, dim);

        const real u_x = stencil_X (SX, v_ref.tl.u, dxi, z, x, y, dim);
        const real v_x = stencil_X (SX, v_ref.tl.v, dxi, z, x, y, dim);
        const real w_x = stencil_X (SX, v_ref.tl.w, dxi, z, x, y, dim);

        const real u_y = stencil_Y (SY, v_ref.tr.u, dyi, z, x, y, dim);
        const real v_y = stencil_Y (SY, v_ref.tr.v, dyi, z, x, y, dim);
        const real w_y = stencil_Y (SY, v_ref.tr.w, dyi, z, x, y, dim);

        const real u_z = stencil_Z (SZ, v_ref.br.u, dzi, z, x, y, dim);
        const real v_z = stencil_Z (SZ, v_ref.br.v, dzi, z, x, y, dim);
        const real w_z = stencil_Z (SZ, v_ref.br.w, dzi, z, x, y, dim);

        stress_update (s_ref.tr.xx,c11,c12,c13,c14,c15,c16,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.tr.yy,c12,c22,c23,c24,c25,c26,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.tr.zz,c13,c23,c33,c34,c35,c36,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.tr.yz,c14,c24,c34,c44,c45,c46,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.tr.xz,c15,c25,c35,c45,c55,c56,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.tr.xy,c16,c26,c36,c46,c56,c66,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
    }
    ////////////////////////////////////

    {
        compute_component_scell_TR( s_cal, v_ref.br, v_ref.tl, v_ref.tr, c_ref,
            dt, dzi, dxi, dyi,
            nz0, nzf, nx0, nxf, ny0, nyf,
            SZ, SX, SY, dim, phase);
    }
#if defined(_OPENACC)
    #pragma acc update host(s_cal.tr.xx[:nelems]) wait(phase)
    #pragma acc update host(s_cal.tr.yy[:nelems]) 
    #pragma acc update host(s_cal.tr.zz[:nelems]) 
    #pragma acc update host(s_cal.tr.yz[:nelems]) 
    #pragma acc update host(s_cal.tr.xz[:nelems]) 
    #pragma acc update host(s_cal.tr.xy[:nelems]) 
#endif

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.xx, s_cal.tr.xx, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.yy, s_cal.tr.yy, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.zz, s_cal.tr.zz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.yz, s_cal.tr.yz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.xz, s_cal.tr.xz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.xy, s_cal.tr.xy, dim );
}

TEST(propagator, compute_component_scell_TL)
{
    const real     dt  = 1.0;
    const real     dzi = 1.0;
    const real     dxi = 1.0;
    const real     dyi = 1.0;
    const integer  nz0 = HALO;
    const integer  nzf = dim.zsize-HALO;
    const integer  nx0 = HALO;
    const integer  nxf = dim.xsize-HALO;
    const integer  ny0 = HALO;
    const integer  nyf = dim.ysize-HALO;
    const offset_t SZ = 0;
    const offset_t SX = 0;
    const offset_t SY = 0;
    const phase_t  phase = TWO;

    // REFERENCE CALCULATION -DON'T TOUCH-
    for (integer y = ny0; y < nyf; y++)
    for (integer x = nx0; x < nxf; x++)
    for (integer z = nz0; z < nzf; z++ )
    {
        const real c11 = cell_coeff_TL      (c_ref.c11, z, x, y, dim);
        const real c12 = cell_coeff_TL      (c_ref.c12, z, x, y, dim);
        const real c13 = cell_coeff_TL      (c_ref.c13, z, x, y, dim);
        const real c14 = cell_coeff_ARTM_TL (c_ref.c14, z, x, y, dim);
        const real c15 = cell_coeff_ARTM_TL (c_ref.c15, z, x, y, dim);
        const real c16 = cell_coeff_ARTM_TL (c_ref.c16, z, x, y, dim);
        const real c22 = cell_coeff_TL      (c_ref.c22, z, x, y, dim);
        const real c23 = cell_coeff_TL      (c_ref.c23, z, x, y, dim);
        const real c24 = cell_coeff_ARTM_TL (c_ref.c24, z, x, y, dim);
        const real c25 = cell_coeff_ARTM_TL (c_ref.c25, z, x, y, dim);
        const real c26 = cell_coeff_ARTM_TL (c_ref.c26, z, x, y, dim);
        const real c33 = cell_coeff_TL      (c_ref.c33, z, x, y, dim);
        const real c34 = cell_coeff_ARTM_TL (c_ref.c34, z, x, y, dim);
        const real c35 = cell_coeff_ARTM_TL (c_ref.c35, z, x, y, dim);
        const real c36 = cell_coeff_ARTM_TL (c_ref.c36, z, x, y, dim);
        const real c44 = cell_coeff_TL      (c_ref.c44, z, x, y, dim);
        const real c45 = cell_coeff_ARTM_TL (c_ref.c45, z, x, y, dim);
        const real c46 = cell_coeff_ARTM_TL (c_ref.c46, z, x, y, dim);
        const real c55 = cell_coeff_TL      (c_ref.c55, z, x, y, dim);
        const real c56 = cell_coeff_ARTM_TL (c_ref.c56, z, x, y, dim);
        const real c66 = cell_coeff_TL      (c_ref.c66, z, x, y, dim);

        const real u_x = stencil_X (SX, v_ref.tr.u, dxi, z, x, y, dim);
        const real v_x = stencil_X (SX, v_ref.tr.v, dxi, z, x, y, dim);
        const real w_x = stencil_X (SX, v_ref.tr.w, dxi, z, x, y, dim);

        const real u_y = stencil_Y (SY, v_ref.tl.u, dyi, z, x, y, dim);
        const real v_y = stencil_Y (SY, v_ref.tl.v, dyi, z, x, y, dim);
        const real w_y = stencil_Y (SY, v_ref.tl.w, dyi, z, x, y, dim);

        const real u_z = stencil_Z (SZ, v_ref.bl.u, dzi, z, x, y, dim);
        const real v_z = stencil_Z (SZ, v_ref.bl.v, dzi, z, x, y, dim);
        const real w_z = stencil_Z (SZ, v_ref.bl.w, dzi, z, x, y, dim);

        stress_update (s_ref.tl.xx,c11,c12,c13,c14,c15,c16,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.tl.yy,c12,c22,c23,c24,c25,c26,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.tl.zz,c13,c23,c33,c34,c35,c36,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.tl.yz,c14,c24,c34,c44,c45,c46,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.tl.xz,c15,c25,c35,c45,c55,c56,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.tl.xy,c16,c26,c36,c46,c56,c66,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
    }
    ////////////////////////////////////

    {
        compute_component_scell_TL( s_cal, v_ref.bl, v_ref.tr, v_ref.tl, c_ref,
            dt, dzi, dxi, dyi,
            nz0, nzf, nx0, nxf, ny0, nyf,
            SZ, SX, SY, dim, phase);
    }
#if defined(_OPENACC)
    #pragma acc update host(s_cal.tl.xx[:nelems]) wait(phase)
    #pragma acc update host(s_cal.tl.yy[:nelems]) 
    #pragma acc update host(s_cal.tl.zz[:nelems]) 
    #pragma acc update host(s_cal.tl.yz[:nelems]) 
    #pragma acc update host(s_cal.tl.xz[:nelems]) 
    #pragma acc update host(s_cal.tl.xy[:nelems]) 
#endif

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tl.xx, s_cal.tl.xx, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tl.yy, s_cal.tl.yy, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tl.zz, s_cal.tl.zz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tl.yz, s_cal.tl.yz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tl.xz, s_cal.tl.xz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tl.xy, s_cal.tl.xy, dim );
}

TEST(propagator, compute_component_scell_BR)
{
    const real     dt  = 1.0;
    const real     dzi = 1.0;
    const real     dxi = 1.0;
    const real     dyi = 1.0;
    const integer  nz0 = HALO;
    const integer  nzf = dim.zsize-HALO;
    const integer  nx0 = HALO;
    const integer  nxf = dim.xsize-HALO;
    const integer  ny0 = HALO;
    const integer  nyf = dim.ysize-HALO;
    const offset_t SZ = 0;
    const offset_t SX = 0;
    const offset_t SY = 0;
    const phase_t  phase = TWO;

    // REFERENCE CALCULATION -DON'T TOUCH-
    for (integer y = ny0; y < nyf; y++)
    for (integer x = nx0; x < nxf; x++)
    for (integer z = nz0; z < nzf; z++ )
    {
        const real c11 = cell_coeff_BR      (c_ref.c11, z, x, y, dim);
        const real c12 = cell_coeff_BR      (c_ref.c12, z, x, y, dim);
        const real c13 = cell_coeff_BR      (c_ref.c13, z, x, y, dim);
        const real c22 = cell_coeff_BR      (c_ref.c22, z, x, y, dim);
        const real c23 = cell_coeff_BR      (c_ref.c23, z, x, y, dim);
        const real c33 = cell_coeff_BR      (c_ref.c33, z, x, y, dim);
        const real c44 = cell_coeff_BR      (c_ref.c44, z, x, y, dim);
        const real c55 = cell_coeff_BR      (c_ref.c55, z, x, y, dim);
        const real c66 = cell_coeff_BR      (c_ref.c66, z, x, y, dim);

        const real c14 = cell_coeff_ARTM_BR (c_ref.c14, z, x, y, dim);
        const real c15 = cell_coeff_ARTM_BR (c_ref.c15, z, x, y, dim);
        const real c16 = cell_coeff_ARTM_BR (c_ref.c16, z, x, y, dim);
        const real c24 = cell_coeff_ARTM_BR (c_ref.c24, z, x, y, dim);
        const real c25 = cell_coeff_ARTM_BR (c_ref.c25, z, x, y, dim);
        const real c26 = cell_coeff_ARTM_BR (c_ref.c26, z, x, y, dim);
        const real c34 = cell_coeff_ARTM_BR (c_ref.c34, z, x, y, dim);
        const real c35 = cell_coeff_ARTM_BR (c_ref.c35, z, x, y, dim);
        const real c36 = cell_coeff_ARTM_BR (c_ref.c36, z, x, y, dim);
        const real c45 = cell_coeff_ARTM_BR (c_ref.c45, z, x, y, dim);
        const real c46 = cell_coeff_ARTM_BR (c_ref.c46, z, x, y, dim);
        const real c56 = cell_coeff_ARTM_BR (c_ref.c56, z, x, y, dim);

        const real u_x = stencil_X (SX, v_ref.bl.u, dxi, z, x, y, dim);
        const real v_x = stencil_X (SX, v_ref.bl.v, dxi, z, x, y, dim);
        const real w_x = stencil_X (SX, v_ref.bl.w, dxi, z, x, y, dim);

        const real u_y = stencil_Y (SY, v_ref.br.u, dyi, z, x, y, dim);
        const real v_y = stencil_Y (SY, v_ref.br.v, dyi, z, x, y, dim);
        const real w_y = stencil_Y (SY, v_ref.br.w, dyi, z, x, y, dim);

        const real u_z = stencil_Z (SZ, v_ref.tr.u, dzi, z, x, y, dim);
        const real v_z = stencil_Z (SZ, v_ref.tr.v, dzi, z, x, y, dim);
        const real w_z = stencil_Z (SZ, v_ref.tr.w, dzi, z, x, y, dim);

        stress_update (s_ref.br.xx,c11,c12,c13,c14,c15,c16,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.br.yy,c12,c22,c23,c24,c25,c26,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.br.zz,c13,c23,c33,c34,c35,c36,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.br.yz,c14,c24,c34,c44,c45,c46,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.br.xz,c15,c25,c35,c45,c55,c56,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.br.xy,c16,c26,c36,c46,c56,c66,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
    }
    ////////////////////////////////////

    {
        compute_component_scell_BR( s_cal, v_ref.tr, v_ref.bl, v_ref.br, c_ref,
            dt, dzi, dxi, dyi,
            nz0, nzf, nx0, nxf, ny0, nyf,
            SZ, SX, SY, dim, phase);
    }
#if defined(_OPENACC)
    #pragma acc update host(s_cal.br.xx[:nelems]) wait(phase)
    #pragma acc update host(s_cal.br.yy[:nelems]) 
    #pragma acc update host(s_cal.br.zz[:nelems]) 
    #pragma acc update host(s_cal.br.yz[:nelems]) 
    #pragma acc update host(s_cal.br.xz[:nelems]) 
    #pragma acc update host(s_cal.br.xy[:nelems]) 
#endif


    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.xx, s_cal.br.xx, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.yy, s_cal.br.yy, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.zz, s_cal.br.zz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.yz, s_cal.br.yz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.xz, s_cal.br.xz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.xy, s_cal.br.xy, dim );
}

TEST(propagator, compute_component_scell_BL)
{
    const real     dt  = 1.0;
    const real     dzi = 1.0;
    const real     dxi = 1.0;
    const real     dyi = 1.0;
    const integer  nz0 = HALO;
    const integer  nzf = dim.zsize-HALO;
    const integer  nx0 = HALO;
    const integer  nxf = dim.xsize-HALO;
    const integer  ny0 = HALO;
    const integer  nyf = dim.ysize-HALO;
    const offset_t SZ = 0;
    const offset_t SX = 0;
    const offset_t SY = 0;
    const phase_t  phase = TWO;

    // REFERENCE CALCULATION -DON'T TOUCH-
    for (integer y = ny0; y < nyf; y++)
    for (integer x = nx0; x < nxf; x++)
    for (integer z = nz0; z < nzf; z++ )
    {
        const real c11 = cell_coeff_BL      (c_ref.c11, z, x, y, dim);
        const real c12 = cell_coeff_BL      (c_ref.c12, z, x, y, dim);
        const real c13 = cell_coeff_BL      (c_ref.c13, z, x, y, dim);
        const real c14 = cell_coeff_ARTM_BL (c_ref.c14, z, x, y, dim);
        const real c15 = cell_coeff_ARTM_BL (c_ref.c15, z, x, y, dim);
        const real c16 = cell_coeff_ARTM_BL (c_ref.c16, z, x, y, dim);
        const real c22 = cell_coeff_BL      (c_ref.c22, z, x, y, dim);
        const real c23 = cell_coeff_BL      (c_ref.c23, z, x, y, dim);
        const real c24 = cell_coeff_ARTM_BL (c_ref.c24, z, x, y, dim);
        const real c25 = cell_coeff_ARTM_BL (c_ref.c25, z, x, y, dim);
        const real c26 = cell_coeff_ARTM_BL (c_ref.c26, z, x, y, dim);
        const real c33 = cell_coeff_BL      (c_ref.c33, z, x, y, dim);
        const real c34 = cell_coeff_ARTM_BL (c_ref.c34, z, x, y, dim);
        const real c35 = cell_coeff_ARTM_BL (c_ref.c35, z, x, y, dim);
        const real c36 = cell_coeff_ARTM_BL (c_ref.c36, z, x, y, dim);
        const real c44 = cell_coeff_BL      (c_ref.c44, z, x, y, dim);
        const real c45 = cell_coeff_ARTM_BL (c_ref.c45, z, x, y, dim);
        const real c46 = cell_coeff_ARTM_BL (c_ref.c46, z, x, y, dim);
        const real c55 = cell_coeff_BL      (c_ref.c55, z, x, y, dim);
        const real c56 = cell_coeff_ARTM_BL (c_ref.c56, z, x, y, dim);
        const real c66 = cell_coeff_BL      (c_ref.c66, z, x, y, dim);

        const real u_x = stencil_X (SX, v_ref.br.u, dxi, z, x, y, dim);
        const real v_x = stencil_X (SX, v_ref.br.v, dxi, z, x, y, dim);
        const real w_x = stencil_X (SX, v_ref.br.w, dxi, z, x, y, dim);

        const real u_y = stencil_Y (SY, v_ref.bl.u, dyi, z, x, y, dim);
        const real v_y = stencil_Y (SY, v_ref.bl.v, dyi, z, x, y, dim);
        const real w_y = stencil_Y (SY, v_ref.bl.w, dyi, z, x, y, dim);

        const real u_z = stencil_Z (SZ, v_ref.tl.u, dzi, z, x, y, dim);
        const real v_z = stencil_Z (SZ, v_ref.tl.v, dzi, z, x, y, dim);
        const real w_z = stencil_Z (SZ, v_ref.tl.w, dzi, z, x, y, dim);

        stress_update (s_ref.br.xx,c11,c12,c13,c14,c15,c16,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.br.yy,c12,c22,c23,c24,c25,c26,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.br.zz,c13,c23,c33,c34,c35,c36,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.br.yz,c14,c24,c34,c44,c45,c46,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.br.xz,c15,c25,c35,c45,c55,c56,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
        stress_update (s_ref.br.xy,c16,c26,c36,c46,c56,c66,z,x,y,dt,u_x,u_y,u_z,v_x,v_y,v_z,w_x,w_y,w_z,dim );
    }
    ////////////////////////////////////

    {
        compute_component_scell_BL( s_cal, v_ref.tl, v_ref.br, v_ref.bl, c_ref,
            dt, dzi, dxi, dyi,
            nz0, nzf, nx0, nxf, ny0, nyf,
            SZ, SX, SY, dim, phase);
    }
#if defined(_OPENACC)
    #pragma acc update host(s_cal.br.xx[:nelems]) wait(phase)
    #pragma acc update host(s_cal.br.yy[:nelems]) 
    #pragma acc update host(s_cal.br.zz[:nelems]) 
    #pragma acc update host(s_cal.br.yz[:nelems]) 
    #pragma acc update host(s_cal.br.xz[:nelems]) 
    #pragma acc update host(s_cal.br.xy[:nelems]) 
#endif

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.xx, s_cal.br.xx, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.yy, s_cal.br.yy, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.zz, s_cal.br.zz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.yz, s_cal.br.yz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.xz, s_cal.br.xz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.xy, s_cal.br.xy, dim );
}

TEST(propagator, stress_propagator)
{
    const real     dt  = 1.0;
    const real     dzi = 1.0;
    const real     dxi = 1.0;
    const real     dyi = 1.0;
    const integer  nz0 = HALO;
    const integer  nzf = dim.zsize-HALO;
    const integer  nx0 = HALO;
    const integer  nxf = dim.xsize-HALO;
    const integer  ny0 = HALO;
    const integer  nyf = dim.ysize-HALO;
    const phase_t  phase = TWO;

    // REFERENCE CALCULATION
    {
        compute_component_scell_BR ( s_ref, v_ref.tr, v_ref.bl, v_ref.br, c_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, forw_offset, back_offset, back_offset, dim, phase);
        compute_component_scell_BL ( s_ref, v_ref.tl, v_ref.br, v_ref.bl, c_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, forw_offset, back_offset, forw_offset, dim, phase);
        compute_component_scell_TR ( s_ref, v_ref.br, v_ref.tl, v_ref.tr, c_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, back_offset, forw_offset, forw_offset, dim, phase);
        compute_component_scell_TL ( s_ref, v_ref.bl, v_ref.tr, v_ref.tl, c_ref, dt, dzi, dxi, dyi, nz0, nzf, nx0, nxf, ny0, nyf, back_offset, back_offset, back_offset, dim, phase);
    }
    ///////////////////////////////////////


    {
        stress_propagator(s_cal, v_ref, c_ref, rho_ref,
                dt, dzi, dxi, dyi,
                nz0, nzf, nx0, nxf, ny0, nyf,
                dim, phase);
    }

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.bl.xx, s_cal.bl.xx, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.bl.yy, s_cal.bl.yy, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.bl.zz, s_cal.bl.zz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.bl.yz, s_cal.bl.yz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.bl.xz, s_cal.bl.xz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.bl.xy, s_cal.bl.xy, dim );

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.xx, s_cal.br.xx, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.yy, s_cal.br.yy, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.zz, s_cal.br.zz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.yz, s_cal.br.yz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.xz, s_cal.br.xz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.br.xy, s_cal.br.xy, dim );

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tl.xx, s_cal.tl.xx, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tl.yy, s_cal.tl.yy, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tl.zz, s_cal.tl.zz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tl.yz, s_cal.tl.yz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tl.xz, s_cal.tl.xz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tl.xy, s_cal.tl.xy, dim );

    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.xx, s_cal.tr.xx, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.yy, s_cal.tr.yy, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.zz, s_cal.tr.zz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.yz, s_cal.tr.yz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.xz, s_cal.tr.xz, dim );
    CUSTOM_ASSERT_EQUAL_FLOAT_3D_ARRAY( s_ref.tr.xy, s_cal.tr.xy, dim );
}

////// TESTS RUNNER //////
TEST_GROUP_RUNNER(propagator)
{
    RUN_TEST_CASE(propagator, IDX);

    RUN_TEST_CASE(propagator, stencil_Z);
    RUN_TEST_CASE(propagator, stencil_X);
    RUN_TEST_CASE(propagator, stencil_Y);

    /* velocity related tests */
    RUN_TEST_CASE(propagator, rho_BL);
    RUN_TEST_CASE(propagator, rho_TR);
    RUN_TEST_CASE(propagator, rho_BR);
    RUN_TEST_CASE(propagator, rho_TL);

    RUN_TEST_CASE(propagator, compute_component_vcell_TL);
    RUN_TEST_CASE(propagator, compute_component_vcell_TR);
    RUN_TEST_CASE(propagator, compute_component_vcell_BR);
    RUN_TEST_CASE(propagator, compute_component_vcell_BL);

    RUN_TEST_CASE(propagator, velocity_propagator);

    /* stresses related tests */
    RUN_TEST_CASE(propagator, stress_update);

    RUN_TEST_CASE(propagator, cell_coeff_BR);
    RUN_TEST_CASE(propagator, cell_coeff_TL);
    RUN_TEST_CASE(propagator, cell_coeff_BL);
    RUN_TEST_CASE(propagator, cell_coeff_TR);

    RUN_TEST_CASE(propagator, cell_coeff_ARTM_BR);
    RUN_TEST_CASE(propagator, cell_coeff_ARTM_TL);
    RUN_TEST_CASE(propagator, cell_coeff_ARTM_BL);
    RUN_TEST_CASE(propagator, cell_coeff_ARTM_TR);

    RUN_TEST_CASE(propagator, compute_component_scell_TR);
    RUN_TEST_CASE(propagator, compute_component_scell_TL);
    RUN_TEST_CASE(propagator, compute_component_scell_BR);
    RUN_TEST_CASE(propagator, compute_component_scell_BL);

    RUN_TEST_CASE(propagator, stress_propagator);
}
