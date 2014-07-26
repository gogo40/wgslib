/*
Implementação do plugin FFT-Varmap.

Este plugin computa varmaps usando FFT.

(c) 2014, LPM/UFRGS, Péricles Lopes Machado
*/



#ifndef __PLUGINS_LPM_UFRGS_FFTVARMAPCALC_H___ 
#define __PLUGINS_LPM_UFRGS_FFTVARMAPCALC_H___ 


#include <vector>

struct VarOut {

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

class FFTMutex {
public:
	virtual void lock() = 0;
	virtual void unlock() = 0;
};

void set_fft_mutex(FFTMutex* mutex);
void fft_lock();
void fft_unlock();

bool fft_varmap_3d(
	VarOut& Yout, 
	const std::vector<double>& data, 
	const std::vector<int>& has_point, 
	int N, int M, int K);

bool fft_varmap_3d_declus(
	VarOut& Yout,
	const std::vector<double>& data,
	const std::vector<double>& weigth,
	const std::vector<int>& has_point,
	int N, int M, int K);

bool fft_crossvarmap_3d(
	VarOut& Yout, 
	const std::vector<double>& data1,
	const std::vector<int>& has_point1, 
	const std::vector<double>& data2,
	const std::vector<int>& has_point2,
	int N, int M, int K);

bool fft_crossvarmap_3d_declus(
	VarOut& Yout,
	const std::vector<double>& weigth1,
	const std::vector<double>& data1,
	const std::vector<int>& has_point1,

	const std::vector<double>& data2,
	const std::vector<int>& has_point2,
	int N, int M, int K);

void fft_set_num_threads(int n_threads);

#endif
