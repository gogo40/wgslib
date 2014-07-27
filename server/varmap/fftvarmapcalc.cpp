/*
Implementação do plugin FFT-Varmap.

Este plugin computa varmaps usando FFT.

(c) 2014, LPM/UFRGS, Péricles Lopes Machado
*/

#include "fftvarmapcalc.h"

#include <cmath>
#include <array>
#include <fftw3.h>

#include <omp.h>

#define REAL 0
#define IMAG 1

#define _p(i, j, k) [((i) * M + (j)) * K + (k)]
#define _p3(i, j, k) [((i) * (2 * M + 1) + (j)) * (2 * K + 1) + (k)]

int _g_fft_n_threads = 1;

WGSLIB_DECL void fft_set_num_threads(int n_threads)
{
	_g_fft_n_threads = (n_threads > 1)? n_threads : 1;
}

WGSLIB_DECL int fft_get_num_threads()
{
	return _g_fft_n_threads;
}

FFTMutex* _g_fft_mutex = 0;

WGSLIB_DECL void set_fft_mutex(FFTMutex* mutex)
{
	_g_fft_mutex = mutex;
}

WGSLIB_DECL void fft_lock()
{
	if (_g_fft_mutex) {
		_g_fft_mutex->lock();
	}
}

WGSLIB_DECL void fft_unlock()
{
	if (_g_fft_mutex) {
		_g_fft_mutex->unlock();
	}
}


void mul_conj(fftw_complex& res, const fftw_complex& Z1, const fftw_complex& Z2)
{
	double a = Z1[REAL] * Z2[REAL] + Z1[IMAG] * Z2[IMAG];
	double b = -Z1[REAL] * Z2[IMAG] + Z1[IMAG] * Z2[REAL];

	res[REAL] = a;
	res[IMAG] = b;
}

WGSLIB_DECL bool fft_varmap_3d(
	VarOut& Yout, 
	const std::vector<double>& data, 
	const std::vector<int>& has_point, 
	int N, int M, int K)
{

	omp_set_num_threads(fft_get_num_threads());
	fftw_plan_with_nthreads(fft_get_num_threads());

	fftw_complex *z,  *Z, *ZI;
	fftw_complex *z2, *Z2;
	fftw_complex *ni, *NI, *INI;

	fftw_plan p;

	int ZS = (2 * N + 1) * (2 * M + 1) * (2 * K + 1);

	fft_lock();
	z = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	Z = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	ZI = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);

	z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	Z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	
	ni = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	NI = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	INI = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	fft_unlock();

	#pragma omp parallel for
	for (int i = 0; i < ZS; ++i) {
		z[i][REAL] = z[i][IMAG] = 0;
		z2[i][REAL] = z2[i][IMAG] = 0;
		ni[i][REAL] = ni[i][IMAG] = 0;
	}

	#pragma omp parallel for
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < M; ++j) {
			for (int k = 0; k < K; ++k) {
				double v = data _p(i, j, k);
				int hp = has_point _p(i, j, k);

				ni _p3(i, j, k)[REAL] = hp;
				
				if (hp) {
					z _p3(i, j, k)[REAL] = v;
				
					z2 _p3(i, j, k)[REAL] = v * v;
				}
			}
		}
	}

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, z, Z, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, z2, Z2, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, ni, NI, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	#pragma omp parallel for
	for (int h = 0; h < ZS; ++h) {
		fftw_complex Z2_I;
		fftw_complex Z_Z;
		fftw_complex I_Z2;
		fftw_complex I_I;

		mul_conj(Z2_I, Z2[h], NI[h]);
		mul_conj(Z_Z, Z[h], Z[h]);
		mul_conj(I_Z2, NI[h], Z2[h]);
		mul_conj(I_I, NI[h], NI[h]);

		Z[h][REAL] = Z2_I[REAL] - 2 * Z_Z[REAL] + I_Z2[REAL];
		Z[h][IMAG] = Z2_I[IMAG] - 2 * Z_Z[IMAG] + I_Z2[IMAG];

		NI[h][REAL] = I_I[REAL];
		NI[h][IMAG] = I_I[IMAG];
	}

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, Z, ZI, FFTW_BACKWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, NI, INI, FFTW_BACKWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	#pragma omp parallel for
	for (int hx = 0; hx < N; ++hx) {
		for (int hy = 0; hy < M; ++hy) {
			for (int hz = 0; hz < K; ++hz) {
				double Y = ZI _p3(hx, hy, hz)[REAL]  / ZS;
				double N = INI _p3(hx, hy, hz)[REAL] / ZS;

				if (N > 0.01) {
					Yout.varmap _p(hx, hy, hz) = Y / (2 * N);
				} else {
					Yout.varmap _p(hx, hy, hz) = 0;
				}

				Yout.ni _p(hx, hy, hz) = N;
			}
		}
	}

	fft_lock();
	fftw_destroy_plan(p);
	fftw_free(z);
	fftw_free(Z);
	fftw_free(ZI);

	fftw_free(z2);
	fftw_free(Z2);

	fftw_free(ni);
	fftw_free(NI);
	fftw_free(INI);
	fft_unlock();

	return true;
}

WGSLIB_DECL bool fft_varmap_3d_declus(
	VarOut& Yout,
	const std::vector<double>& data,
	const std::vector<double>& weigth,
	const std::vector<int>& has_point,
	int N, int M, int K)
{
	omp_set_num_threads(fft_get_num_threads());
	fftw_plan_with_nthreads(fft_get_num_threads());

	fftw_complex *z, *Z, *ZI;
	fftw_complex *z2, *Z2;
	fftw_complex *ni, *NI, *INI;

	fftw_complex *w, *W;

	fftw_complex *z2w, *Z2W;
	fftw_complex *zw, *ZW;
	
	fftw_plan p;


	int ZS = (2 * N + 1) * (2 * M + 1) * (2 * K + 1);

	fft_lock();
	z = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	Z = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	ZI = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);

	z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	Z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	
	ni = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	NI = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	INI = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	
	w = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	W = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	
	z2w = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	Z2W = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	
	zw = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	ZW = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	fft_unlock();


	#pragma omp parallel for
	for (int i = 0; i < ZS; ++i) {
		z[i][REAL] = z[i][IMAG] = 0;
		z2[i][REAL] = z2[i][IMAG] = 0;
		ni[i][REAL] = ni[i][IMAG] = 0;
		w[i][REAL] = w[i][IMAG] = 0;
		z2w[i][REAL] = z2w[i][IMAG] = 0;
		zw[i][REAL] = zw[i][IMAG] = 0;
	}

	#pragma omp parallel for
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < M; ++j) {
			for (int k = 0; k < K; ++k) {
				double v = data _p(i, j, k);
				double w_ = weigth _p(i, j, k);
				int hp = has_point _p(i, j, k);

				ni _p3(i, j, k)[REAL] = hp;
				
				if (hp) {
					z _p3(i, j, k)[REAL] = v;
					
					z2 _p3(i, j, k)[REAL] = v * v;
					
					zw _p3(i, j, k)[REAL] = v * w_;

					z2w _p3(i, j, k)[REAL] = v * v * w_;

					w _p3(i, j, k)[REAL] = w_;

				}
			}
		}
	}


	//LEVA z para o dominio da frequencia
	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, z, Z, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, w, W, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, zw, ZW, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, z2w, Z2W, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, z2, Z2, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, ni, NI, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	//Realiza convolução
	#pragma omp parallel for
	for (int h = 0; h < ZS; ++h) {
		fftw_complex Z2W_I;
		fftw_complex ZW_Z;
		fftw_complex W_Z2;
		fftw_complex Z2_W;
		fftw_complex Z_ZW;
		fftw_complex I_Z2W;
		fftw_complex W_I;
		fftw_complex I_W;

		mul_conj(Z2W_I, Z2W[h], NI[h]);
		mul_conj(ZW_Z, ZW[h], Z[h]);
		mul_conj(W_Z2, W[h], Z2[h]);
		mul_conj(Z2_W, Z2[h], W[h]);
		mul_conj(Z_ZW, Z[h], ZW[h]);
		mul_conj(I_Z2W, NI[h], Z2W[h]);

		mul_conj(I_W, NI[h], W[h]);
		mul_conj(W_I, W[h], NI[h]);

		Z[h][REAL] = Z2W_I[REAL] - 2 * ZW_Z[REAL] + W_Z2[REAL] + 
					 Z2_W[REAL] - 2 * Z_ZW[REAL] + I_Z2W[REAL];

		Z[h][IMAG] = Z2W_I[IMAG] - 2 * ZW_Z[IMAG] + W_Z2[IMAG] +
					 Z2_W[IMAG] - 2 * Z_ZW[IMAG] + I_Z2W[IMAG];

		NI[h][REAL] = 2 * (W_I[REAL] + I_W[REAL]);
		NI[h][IMAG] = 2 * (W_I[IMAG] + I_W[IMAG]);
	}

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, Z, ZI, FFTW_BACKWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, NI, INI, FFTW_BACKWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);
	
	#pragma omp parallel for
	for (int hx = 0; hx < N; ++hx) {
		for (int hy = 0; hy < M; ++hy) {
			for (int hz = 0; hz < K; ++hz) {
				double Y = ZI _p3(hx, hy, hz)[REAL] / ZS;
				double N = INI _p3(hx, hy, hz)[REAL] / ZS;

				if (N > 0.01) {
					Yout.varmap _p(hx, hy, hz) = Y / N;
				}
				else {
					Yout.varmap _p(hx, hy, hz) = 0;
				}

				Yout.ni _p(hx, hy, hz) = N;
			}
		}
	}

	fft_lock();
	fftw_destroy_plan(p);
	fftw_free(z);
	fftw_free(Z);
	fftw_free(ZI);

	fftw_free(z2);
	fftw_free(Z2);

	fftw_free(zw);
	fftw_free(ZW);

	fftw_free(z2w);
	fftw_free(Z2W);

	fftw_free(w);
	fftw_free(W);

	fftw_free(ni);
	fftw_free(NI);
	fftw_free(INI);
	fft_unlock();

	return true;
}

WGSLIB_DECL bool fft_crossvarmap_3d(
	VarOut& Yout,
	const std::vector<double>& data1,
	const std::vector<int>& has_point1,
	const std::vector<double>& data2,
	const std::vector<int>& has_point2,
	int N, int M, int K)
{
	omp_set_num_threads(fft_get_num_threads());
	fftw_plan_with_nthreads(fft_get_num_threads());

	fftw_complex *Z1I;
	fftw_complex *I1I;

	fftw_complex *z1z2, *Z1Z2;

	fftw_complex *i2z1, *I2Z1;
	fftw_complex *i1z2, *I1Z2;

	fftw_complex *i1i2, *I1I2;
	
	fftw_plan p;


	int ZS = (2 * N + 1) * (2 * M + 1) * (2 * K + 1);

	fft_lock();
	Z1I = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	I1I = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);

	z1z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	Z1Z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	
	i2z1 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	I2Z1 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	
	i1z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	I1Z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	
	i1i2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	I1I2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	fft_unlock();

	#pragma omp parallel for
	for (int i = 0; i < ZS; ++i) {
		z1z2[i][REAL] = z1z2[i][IMAG] = 0;
		i1z2[i][REAL] = i1z2[i][IMAG] = 0;
		i2z1[i][REAL] = i2z1[i][IMAG] = 0;
		i1i2[i][REAL] = i1i2[i][IMAG] = 0;
	}

	#pragma omp parallel for
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < M; ++j) {
			for (int k = 0; k < K; ++k) {
				double v1 = data1 _p(i, j, k);
				double v2 = data2 _p(i, j, k);
				int hp1 = has_point1 _p(i, j, k);
				int hp2 = has_point2 _p(i, j, k);	
		
				z1z2 _p3(i, j, k)[REAL] =  v1 * v2;
				i1z2 _p3(i, j, k)[REAL] = hp1 * v2;
				i2z1 _p3(i, j, k)[REAL] = hp2 * v1;
				i1i2 _p3(i, j, k)[REAL] = hp1 * hp2;
			}
		}
	}

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, z1z2, Z1Z2, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, i1z2, I1Z2, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, i2z1, I2Z1, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, i1i2, I1I2, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	#pragma omp parallel for
	for (int h = 0; h < ZS; ++h) {
		fftw_complex A;   
		fftw_complex B;  

		fftw_complex C;  
		fftw_complex D;  
		fftw_complex I1_I2;  
		
		mul_conj(A, Z1Z2[h],  I1I2[h]);
		mul_conj(D, I1I2[h],  Z1Z2[h]);

		mul_conj(B, I2Z1[h], I1Z2[h]);
		mul_conj(C, I1Z2[h], I2Z1[h]);

		mul_conj(I1_I2, I1I2[h], I1I2[h]);

		Z1Z2[h][REAL] = A[REAL] - B[REAL] - C[REAL] + D[REAL];
		Z1Z2[h][IMAG] = A[IMAG] - B[IMAG] - C[IMAG] + D[IMAG];

		I1I2[h][REAL] = I1_I2[REAL];
		I1I2[h][IMAG] = I1_I2[IMAG];
	}

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, Z1Z2, Z1I, FFTW_BACKWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, I1I2, I1I, FFTW_BACKWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	#pragma omp parallel for
	for (int hx = 0; hx < N; ++hx) {
		for (int hy = 0; hy < M; ++hy) {
			for (int hz = 0; hz < K; ++hz) {
				double Y = Z1I _p3(hx, hy, hz)[REAL] / ZS;
				double N = I1I _p3(hx, hy, hz)[REAL] / ZS;

				if (N > 0.01) {
					Yout.varmap _p(hx, hy, hz) =  Y / (2 * N);
				} else {
					Yout.varmap _p(hx, hy, hz) = 0;
				}

				Yout.ni _p(hx, hy, hz) = N;

			}
		}
	}

	fft_lock();
	fftw_destroy_plan(p);

	fftw_free(Z1I);
	fftw_free(I1I);
	
	fftw_free(z1z2); 
	fftw_free(Z1Z2);
	
	fftw_free(i2z1); 
	fftw_free(I2Z1); 
	
	fftw_free(i1z2);
	fftw_free(I1Z2);
	
	fftw_free(i1i2); 
	fftw_free(I1I2);
	fft_unlock();

	return true;
}

WGSLIB_DECL bool fft_crossvarmap_3d_declus(
	VarOut& Yout,

	const std::vector<double>& weigth1,
	const std::vector<double>& data1,
	const std::vector<int>& has_point1,

	const std::vector<double>& data2,
	const std::vector<int>& has_point2,
	int N, int M, int K)
{
	omp_set_num_threads(fft_get_num_threads());
	fftw_plan_with_nthreads(fft_get_num_threads());

	fftw_complex *i1i2,  *I1I2;
	fftw_complex *z1i2,  *Z1I2;
	fftw_complex *i1z2,  *I1Z2;
	fftw_complex *z1z2,  *Z1Z2;
	fftw_complex *w,     *W;
	fftw_complex *wz1,   *WZ1;
	fftw_complex *wz2,   *WZ2;
	fftw_complex *wz1z2, *WZ1Z2;
	
	fftw_plan p;

	int ZS = (2 * N + 1) * (2 * M + 1) * (2 * K + 1);

	fft_lock();
	i1i2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);  
	I1I2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	
	z1i2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS); 
	Z1I2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	
	i1z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS); 
	I1Z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);

	z1z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS); 
	Z1Z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);

	w = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS); 
	W = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);

	wz1 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	WZ1 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);

	wz2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	WZ2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	wz1z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS); 
	WZ1Z2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* ZS);
	fft_unlock();

	#pragma omp parallel for
	for (int i = 0; i < ZS; ++i) {
		i1i2[i][REAL] = i1i2[i][IMAG] = 0;
		I1I2[i][REAL] = I1I2[i][IMAG] = 0;

		z1i2[i][REAL] = z1i2[i][IMAG] = 0;
		Z1I2[i][REAL] = Z1I2[i][IMAG] = 0;

		i1z2[i][REAL] = i1z2[i][IMAG] = 0;
		I1Z2[i][REAL] = I1Z2[i][IMAG] = 0;

		z1z2[i][REAL] = z1z2[i][IMAG] = 0;
		Z1Z2[i][REAL] = Z1Z2[i][IMAG] = 0;

		w[i][REAL] = w[i][IMAG] = 0;
		W[i][REAL] = W[i][IMAG] = 0;

		wz1[i][REAL] = wz1[i][IMAG] = 0;
		WZ1[i][REAL] = WZ1[i][IMAG] = 0;

		wz2[i][REAL] = wz2[i][IMAG] = 0;
		WZ2[i][REAL] = WZ2[i][IMAG] = 0;

		wz1z2[i][REAL] = wz1z2[i][IMAG] = 0;
		WZ1Z2[i][REAL] = WZ1Z2[i][IMAG] = 0;
	}

	#pragma omp parallel for
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < M; ++j) {
			for (int k = 0; k < K; ++k) {
				double v1 = data1 _p(i, j, k);
				double w1_ = weigth1 _p(i, j, k);
				int    i1 = has_point1 _p(i, j, k);
				
				double v2 = data2 _p(i, j, k);
				int    i2 = has_point2 _p(i, j, k);

				i1i2  _p3(i, j, k)[REAL] = i1 * i2;
				z1i2  _p3(i, j, k)[REAL] = v1 * i2;
				i1z2  _p3(i, j, k)[REAL] = i1 * v2;
				z1z2  _p3(i, j, k)[REAL] = v1 * v2;

				w     _p3(i, j, k)[REAL] = w1_;
				wz1   _p3(i, j, k)[REAL] = w1_ * v1;
				wz2   _p3(i, j, k)[REAL] = w1_ * v2;
				wz1z2 _p3(i, j, k)[REAL] = w1_ * v1 * v2;
			}
		}
	}

	///////////////////////////////////////////////////////////////////

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, i1i2, I1I2, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();
	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, z1i2, Z1I2, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();
	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, i1z2, I1Z2, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();
	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, z1z2, Z1Z2, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();
	fftw_execute(p);
	
	////////////////////////////////////////////////////////////////////////////////////

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, w, W, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();
	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, wz1, WZ1, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();
	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, wz2, WZ2, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();
	fftw_execute(p);

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, wz1z2, WZ1Z2, FFTW_FORWARD, FFTW_ESTIMATE);
	fft_unlock();
	fftw_execute(p);

	#pragma omp parallel for
	for (int h = 0; h < ZS; ++h) {
		fftw_complex W_I1I2;
		fftw_complex I1I2_W;

		fftw_complex WZ1Z2_I1I2;
		fftw_complex I1I2_WZ1Z2;

		fftw_complex WZ2_Z1I2;
		fftw_complex Z1I2_WZ2;


		fftw_complex WZ1_I1Z2;
		fftw_complex I1Z2_WZ1;

		fftw_complex W_Z1Z2;
		fftw_complex Z1Z2_W;


		mul_conj(W_I1I2, W[h], I1I2[h]);
		mul_conj(I1I2_W, I1I2[h], W[h]);


		mul_conj(WZ1Z2_I1I2, WZ1Z2[h], I1I2[h]);
		mul_conj(I1I2_WZ1Z2, I1I2[h], WZ1Z2[h]);

		mul_conj(WZ2_Z1I2, WZ2[h], Z1I2[h]);
		mul_conj(Z1I2_WZ2, Z1I2[h], WZ2[h]);


		mul_conj(WZ1_I1Z2, WZ1[h], I1Z2[h]);
		mul_conj(I1Z2_WZ1, I1Z2[h], WZ1[h]);

		mul_conj(W_Z1Z2, W[h], Z1Z2[h]);
		mul_conj(Z1Z2_W, Z1Z2[h], W[h]);

		Z1Z2[h][REAL] =
			WZ1Z2_I1I2[REAL] -
			WZ2_Z1I2[REAL] -
			WZ1_I1Z2[REAL] +
			W_Z1Z2[REAL] +

			Z1Z2_W[REAL] -
			I1Z2_WZ1[REAL] -
			Z1I2_WZ2[REAL] +
			I1I2_WZ1Z2[REAL];

		Z1Z2[h][IMAG] =
			WZ1Z2_I1I2[IMAG] -
			WZ2_Z1I2[IMAG] -
			WZ1_I1Z2[IMAG] +
			W_Z1Z2[IMAG] +

			Z1Z2_W[IMAG] -
			I1Z2_WZ1[IMAG] -
			Z1I2_WZ2[IMAG] +
			I1I2_WZ1Z2[IMAG];

		W[h][REAL] = 2 * (W_I1I2[REAL] + I1I2_W[REAL]);
		W[h][IMAG] = 2 * (W_I1I2[IMAG] + I1I2_W[IMAG]);
	}

	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, Z1Z2, z1z2, FFTW_BACKWARD, FFTW_ESTIMATE); 
	fft_unlock();

	fftw_execute(p);
	
	fft_lock();
	p = fftw_plan_dft_3d(2 * N + 1, 2 * M + 1, 2 * K + 1, W, w, FFTW_BACKWARD, FFTW_ESTIMATE);
	fft_unlock();

	fftw_execute(p);

	#pragma omp parallel for
	for (int hx = 0; hx < N; ++hx) {
		for (int hy = 0; hy < M; ++hy) {
			for (int hz = 0; hz < K; ++hz) {
				double Y = z1z2 _p3(hx, hy, hz)[REAL] / ZS;
				double N = w _p3(hx, hy, hz)[REAL] / ZS;

				if (N > 0.01) {
					Yout.varmap _p(hx, hy, hz) = Y / (N);
				}
				else {
					Yout.varmap _p(hx, hy, hz) = 0;
				}

				Yout.ni _p(hx, hy, hz) = N;
			}
		}
	}

	fft_lock();
	fftw_destroy_plan(p);

	fftw_free(i1i2);
	fftw_free(I1I2);

	fftw_free(z1i2);
	fftw_free(Z1I2);

	fftw_free(i1z2);
	fftw_free(I1Z2);

	fftw_free(z1z2);
	fftw_free(Z1Z2);

	fftw_free(w);
	fftw_free(W);

	fftw_free(wz1);
	fftw_free(WZ1);

	fftw_free(wz2);
	fftw_free(WZ2);
	
	fftw_free(wz1z2);
	fftw_free(WZ1Z2);
	fft_unlock();

	return true;
}


