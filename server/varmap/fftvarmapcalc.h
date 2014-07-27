/*
Implementação do plugin FFT-Varmap.

Este plugin computa varmaps usando FFT.

(c) 2014, LPM/UFRGS, Péricles Lopes Machado
*/



#ifndef __PLUGINS_LPM_UFRGS_FFTVARMAPCALC_H___ 
#define __PLUGINS_LPM_UFRGS_FFTVARMAPCALC_H___ 


#include <vector>
#include <common.h>

struct WGSLIB_DECL VarOut {

	VarOut()
	: varmap(1), ni(1) {}

	VarOut(int N)
		: varmap(N), ni(N) {}

	VarOut(const std::vector<double>& varmap, const std::vector<int>& ni)
		: varmap(varmap), ni(ni) {}


	void clear() {
		varmap.clear();
		ni.clear();
	}

	void resize(int N) {
		varmap.resize(N);
		ni.resize(N);
	}

	std::vector<double> varmap;
	std::vector<int> ni;
};

class WGSLIB_DECL FFTMutex {
public:
	virtual void lock() = 0;
	virtual void unlock() = 0;
};

WGSLIB_DECL void set_fft_mutex(FFTMutex* mutex);
WGSLIB_DECL void fft_lock();
WGSLIB_DECL void fft_unlock();

WGSLIB_DECL bool fft_varmap_3d(
	VarOut& Yout, 
	const std::vector<double>& data, 
	const std::vector<int>& has_point, 
	int N, int M, int K);

WGSLIB_DECL bool fft_varmap_3d_declus(
	VarOut& Yout,
	const std::vector<double>& data,
	const std::vector<double>& weigth,
	const std::vector<int>& has_point,
	int N, int M, int K);

WGSLIB_DECL bool fft_crossvarmap_3d(
	VarOut& Yout, 
	const std::vector<double>& data1,
	const std::vector<int>& has_point1, 
	const std::vector<double>& data2,
	const std::vector<int>& has_point2,
	int N, int M, int K);

WGSLIB_DECL bool fft_crossvarmap_3d_declus(
	VarOut& Yout,
	const std::vector<double>& weigth1,
	const std::vector<double>& data1,
	const std::vector<int>& has_point1,

	const std::vector<double>& data2,
	const std::vector<int>& has_point2,
	int N, int M, int K);

WGSLIB_DECL void fft_set_num_threads(int n_threads);

#endif
