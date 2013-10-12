#include "Mandelbrot.h"
#include <emmintrin.h>

//reference implementation for math clarity
//we calculate z=z^2+c, that is: z_re^2+2*z_re*z_im+z_im*z_im+re+im, with starting z=(0,0)
//z_re'=z_re^2-z_im^2+re
//z_im'=2*z_re*z_im+im
void mandelbrotKernelPure(const float re, const float im, unsigned char *out_color)
{
	float z_re = 0.0f;
	float z_im = 0.0f;
	float z_re2 = 0.0f;
	float z_im2 = 0.0f;
	int iter = 0;
	do
	{
		z_im = 2.0f * z_re * z_im + im;
		z_re = z_re2- z_im2 + re;
		z_re2 = z_re * z_re;
		z_im2 = z_im * z_im;
		iter++;
	}
	while(((z_re2 + z_im2) < 9.0f) && (iter < 32));
	unsigned char col = (iter == 32) ? 255 : (iter * 8);
	*out_color ++= col;
	*out_color ++= col;
	*out_color ++= col;
	*out_color ++= 255;
}

//same thing, but processes 4 pixels simultaneously using SSE2 intrinsics
//probably can be made faster, but I'm no expert at low level code :)
void mandelbrotKernelSSE2(__m128 re, __m128 im, unsigned char *out_color)
{
	const __m128 escape = _mm_set_ps(9.0f, 9.0f, 9.0f, 9.0f);
	__m128i iter_inc = _mm_set_epi32(1, 1, 1, 1);

	__m128 z_re = _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
	__m128 z_im = _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
	__m128 z_re2 = _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
	__m128 z_im2 = _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
	__m128i iter_mask = _mm_set_epi32(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
	__m128i iter = _mm_set_epi32(0, 0, 0, 0);
	int i = 0;
	int iter_mask_v[4];
	for(i=0; i < 32; i++)
	{
		z_im = _mm_mul_ps(z_re, z_im);
		z_im = _mm_add_ps(z_im, z_im);
		z_im = _mm_add_ps(z_im, im);
		z_re = _mm_sub_ps(z_re2, z_im2);
		z_re = _mm_add_ps(z_re, re);
		z_re2 = _mm_mul_ps(z_re, z_re);
		z_im2 = _mm_mul_ps(z_im, z_im);
		__m128 iter_mask_new=_mm_cmplt_ps(_mm_add_ps(z_re2, z_im2), escape);
		iter_mask = _mm_castps_si128(_mm_and_ps(_mm_castsi128_ps(iter_mask), iter_mask_new));
		iter_inc = _mm_castps_si128(_mm_and_ps(_mm_castsi128_ps(iter_inc), _mm_castsi128_ps(iter_mask)));
		iter = _mm_add_epi32(iter, iter_inc);
        //not sure if it really speeds up the code, we are doing conditional based on
        //SSE2 register, probably there's much better way to do it
		_mm_storeu_ps((float*)iter_mask_v, _mm_castsi128_ps(iter_mask));
		if(!(iter_mask_v[0] || iter_mask_v[1] || iter_mask_v[2] || iter_mask_v[3]))
		{
			break;
		}
	}
	int iters[4];
	_mm_storeu_ps((float*)iters, _mm_castsi128_ps(iter));
	for(i=0;i<4;i++)
	{
		unsigned char col = (iters[3 - i] == 32) ? 255 : (iters[3 - i] * 8);
		*out_color ++= col;
		*out_color ++= col;
		*out_color ++= col;
		*out_color ++= 255;
	}
}

void mandelbrotPure(const BufferInfo &output, const MandelbrotParams &params, int firstLineIndex, int lineCount)
{
	int i,j;
    int lastLine = firstLineIndex + lineCount;
	for(i = firstLineIndex; i < lastLine; i++)
	{
		unsigned char *line = output.buffer + 4 * i * output.width;
		float im = params.imStart + ((float)i / output.height) * params.imDelta;
		for(j=0; j < output.width; j++)
		{
			mandelbrotKernelPure(params.reStart + ((float)j / output.width) * params.reDelta, im, line);
			line += 4;
		}
	}
}

void mandelbrotSSE2(const BufferInfo &output, const MandelbrotParams &params, int firstLineIndex, int lineCount)
{
	const __m128 xmm_index_inc = _mm_set_ps(4.0f, 4.0f, 4.0f, 4.0f);
	const __m128 xmm_re_start = _mm_set_ps(params.reStart, params.reStart, params.reStart, params.reStart);
	const __m128 xmm_re_delta = _mm_set_ps(params.reDelta, params.reDelta, params.reDelta, params.reDelta);
	const __m128 xmm_inv_screen_x = _mm_set_ps(1.0f / output.width, 1.0f / output.width, 1.0f / output.width, 1.0f / output.width);

    int i,j;
    int lastLine = firstLineIndex + lineCount;
	for(i = firstLineIndex; i < lastLine; i++)
	{
		unsigned char *line = output.buffer + 4 * i * output.width;
		float im = params.imStart + ((float)i / output.height) * params.imDelta;

		__m128 xmm_im = _mm_set_ps(im, im, im, im);
		__m128 xmm_index = _mm_set_ps(0.0f, 1.0f, 2.0f, 3.0f);
		for(j=0; j < output.width / 4; j++)
		{
			__m128 re = _mm_mul_ps(xmm_index, xmm_inv_screen_x);
			re = _mm_mul_ps(re, xmm_re_delta);
			re = _mm_add_ps(re, xmm_re_start);
			mandelbrotKernelSSE2(re, xmm_im, line);
			xmm_index = _mm_add_ps(xmm_index, xmm_index_inc);
			line += 16;
		}
	}
}

