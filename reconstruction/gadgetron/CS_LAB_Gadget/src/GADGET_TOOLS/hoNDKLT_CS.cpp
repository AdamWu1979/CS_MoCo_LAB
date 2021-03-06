#include "hoNDKLT_CS.h"

#define NumElementsUseThreading 64*1024

using namespace Gadgetron;

#ifndef __GADGETRON_VERSION_HIGHER_3_6__
	template<class T> void clear(hoNDArray<T> *x)
	{
		if (x->get_number_of_elements() > 0) {
			memset(x->get_data_ptr(), 0, x->get_number_of_elements()*sizeof(T));
		}
	}

	template<class T> void clear(hoNDArray<T> &x)
	{
		if (x.get_number_of_elements() > 0) {
			memset(x.get_data_ptr(), 0, x.get_number_of_elements()*sizeof(T));
		}
	}
#endif

template <typename T>
void sum_over_dimension(const hoNDArray<T> &x, hoNDArray<T> &y, size_t dim);

template <typename T>
void sum_over_dimension(const hoNDArray<T> &x, hoNDArray<T> &r, size_t dim)
{
	try {
		size_t D = x.get_number_of_dimensions();

		if (dim >= D) {
			r = x;
			return;
		}

		std::vector<size_t> dimX, dimR;
		x.get_dimensions(dimX);
		dimR = dimX;
		dimR[dim] = 1;

		if (!r.dimensions_equal(&dimR)) {
			r.create(dimR);
		}

		if (dim == 0) {
			size_t X = x.get_size(0);
			size_t num = x.get_number_of_elements() / X;

			const T *pX = x.begin();
			T *pR = r.begin();

			long long n;

			#pragma omp parallel for default(none) private(n) shared(X, num, pX, pR)
			for (n = 0; n < (long long)num; n++) {
				T xsum = pX[n*X];

				for (size_t ro = 1; ro<X; ro++) {
					xsum += pX[n*X + ro];
				}

				pR[n] = xsum;
			}
		} else {
			size_t strideX = x.get_size(0);

			for (size_t d = 1; d <= dim; d++) {
				strideX *= x.get_size(d);
			}

			size_t strideR = strideX / x.get_size(dim);
			size_t num = x.get_number_of_elements() / strideX;
			size_t nDim = x.get_size(dim);

			const T* pX = x.begin();
			T* pR = r.begin();

			if (nDim == 1) {
				memcpy(pR, pX, x.get_number_of_bytes());
				return;
			}

			long long n;

			#pragma omp parallel for default(none) private(n) shared(strideX, strideR, num, nDim, pX, pR)
			for (n = 0; n < (long long)num; n++) {
				const T* pX_curr = pX + n*strideX;
				T* pR_curr = pR + n*strideR;

				memcpy(pR_curr, pX_curr, sizeof(T)*strideR);

				size_t p, c;
				for (p = 1; p < nDim; p++) {
					for (c = 0; c < strideR; c++) {
						pR_curr[c] += pX_curr[p*strideR+c];
					}
				}
			}
		}
	} catch (...) {
		GERROR("Errors happened in sum_over_dimension(const hoNDArray<T>& x, hoNDArray<T>& y, size_t dim) ...\n");
	}
}

template <typename T> void scal(size_t N, T a, hoNDArray<T> &x);
template <typename T> void scal(T a, hoNDArray<std::complex<T> > &x);
template <typename T> void scal(T a, hoNDArray<complext<T> > &x);

inline void scal(size_t N, float a, float *x)
{
	long long n;

	#pragma omp parallel for default(none) private(n) shared(N, x, a) if (N>NumElementsUseThreading)
	for (n = 0; n < (long long)N; n++) {
		x[n] *= a;
	}
}

inline void scal(size_t N, double a, double *x)
{
	long long n;

	#pragma omp parallel for default(none) private(n) shared(N, x, a) if (N>NumElementsUseThreading)
	for (n = 0; n < (long long)N; n++) {
		x[n] *= a;
	}
}

inline void scal(size_t N, std::complex<float> a, std::complex<float> *x)
{
	long long n;

	#pragma omp parallel for default(none) private(n) shared(N, x, a) if (N>NumElementsUseThreading)
	for (n = 0; n < (long long)N; n++) {
		const std::complex<float> & c = x[n];
		const float re = c.real();
		const float im = c.imag();
		const float ar = a.real();
		const float ai = a.imag();

		reinterpret_cast<float(&)[2]>(x[n])[0] = re*ar-im*ai;
		reinterpret_cast<float(&)[2]>(x[n])[1] = re*ai+im*ar;
	}
}

inline void scal(size_t N, std::complex<double> a, std::complex<double> *x)
{
	long long n;
	#pragma omp parallel for default(none) private(n) shared(N, x, a) if (N>NumElementsUseThreading)
	for (n = 0; n < (long long)N; n++) {
		const std::complex<double> & c = x[n];
		const double re = c.real();
		const double im = c.imag();
		const double ar = a.real();
		const double ai = a.imag();

		reinterpret_cast<double(&)[2]>(x[n])[0] = re*ar-im*ai;
		reinterpret_cast<double(&)[2]>(x[n])[1] = re*ai+im*ar;
	}
}

inline void scal(size_t N, complext<float> a, complext<float> *x)
{
	long long n;

	#pragma omp parallel for default(none) private(n) shared(N, x, a) if (N>NumElementsUseThreading)
	for (n = 0; n < (long long)N; n++) {
		const complext<float> & c = x[n];
		const float re = c.real();
		const float im = c.imag();
		const float ar = a.real();
		const float ai = a.imag();

		reinterpret_cast<float(&)[2]>(x[n])[0] = re*ar-im*ai;
		reinterpret_cast<float(&)[2]>(x[n])[1] = re*ai+im*ar;
	}
}

inline void scal(size_t N, complext<double> a, complext<double> *x)
{
	long long n;

	#pragma omp parallel for default(none) private(n) shared(N, x, a) if (N>NumElementsUseThreading)
	for (n = 0; n < (long long)N; n++) {
		const complext<double> & c = x[n];
		const double re = c.real();
		const double im = c.imag();
		const double ar = a.real();
		const double ai = a.imag();

		reinterpret_cast<double(&)[2]>(x[n])[0] = re*ar-im*ai;
		reinterpret_cast<double(&)[2]>(x[n])[1] = re*ai+im*ar;
	}
}

template <typename T>
void scal(T a, hoNDArray<T> &x)
{
	scal(x.get_number_of_elements(), a, x.begin());
}

template void scal(float a, hoNDArray<float> &x);
template void scal(double a, hoNDArray<double> &x);
template void scal(std::complex<float> a, hoNDArray< std::complex<float> > &x);
template void scal(std::complex<double> a, hoNDArray< std::complex<double> > &x);
template void scal(complext<float> a, hoNDArray<complext<float> > &x);
template void scal(complext<double> a, hoNDArray<complext<double> > &x);

template<typename T>
hoNDKLT_CS<T>::hoNDKLT_CS()
{
}

template<typename T>
hoNDKLT_CS<T>::hoNDKLT_CS(const hoNDArray<T> &data, size_t dim, size_t output_length)
{
	this->prepare(data, dim, output_length);
}

template<typename T>
hoNDKLT_CS<T>::hoNDKLT_CS(const hoNDArray<T> &data, size_t dim, value_type thres)
{
	this->prepare(data, dim, thres);
}

template<typename T>
hoNDKLT_CS<T>::hoNDKLT_CS(const Self &v)
{
	*this = v;
}

template<typename T>
hoNDKLT_CS<T>::~hoNDKLT_CS()
{
}

template<typename T>
hoNDKLT_CS<T>& hoNDKLT_CS<T>::operator=(const Self &v)
{
	if (&v == this) {
		return *this;
	}

	this->V_ = v.V_;
	this->E_ = v.E_;
	this->output_length_ = v.output_length_;
	size_t N = this->V_.get_size(0);
	this->M_.create(N, this->output_length_, V_.begin());

	return *this;
}

template<typename T>
void hoNDKLT_CS<T>::compute_eigen_vector(const hoNDArray<T> &data, bool remove_mean)
{
	try {
		size_t NDim = data.get_number_of_dimensions();
		size_t N = data.get_size(NDim-1);
		size_t M = data.get_number_of_elements() / N;

		hoNDArray<T> data2D;
		data2D.create(M, N, const_cast<T*>(data.begin()));

		V_.create(N, N);
		E_.create(N, 1);
		Gadgetron::clear(V_);
		Gadgetron::clear(E_);
		size_t m, n;
		
		// compute and subtract mean from data
		hoNDArray<T> data2DNoMean;
		arma::Mat<T> Am;
		if (remove_mean) {
			hoNDArray<T> dataMean(1, N);
			Gadgetron::sum_over_dimension(data2D, dataMean, 0);
			Gadgetron::scal((T)(1.0 / M), dataMean);
			data2DNoMean.create(M, N);

			for (n = 0; n < N; n++) {
				for (m = 0; m < M; m++) {
					data2DNoMean(m, n) = data2D(m, n) - dataMean(0, n);
				}
			}
			Am = as_arma_matrix(data2DNoMean);
		} else {
			Am = as_arma_matrix(data2D);
		}

		// call svd
		arma::Mat<T> Vm = as_arma_matrix(V_);
		arma::Mat<T> Um;
		arma::Col<value_type> Sv;
		GADGET_CHECK_THROW(arma::svd_econ(Um, Sv, Vm, Am, 'r'));

		for (n = 0; n < N; n++) {
			value_type v = Sv(n);
			E_(n) = v*v; // the E is eigen value, the square of singular value
		}

		//// compute mean
		//hoNDArray<T> mean(N);
		//// compute data'*data
		//char uplo = 'L';
		//bool isAHA = true;
		//Gadgetron::herk(V, data2D, uplo, isAHA);
		//hoMatrix<T> m;
		//m.createMatrix(N, N, V.begin());
		//m.copyLowerTriToUpper();
		//if (remove_mean)
		//{
		//	// compute and subtract mean for every mode
		//	hoNDArray<T> mean(N, 1);
		//	hoNDArray<T> dataMean(1, N, mean.begin());
		//	Gadgetron::sum_over_dimension(data2D, dataMean, 0);
		//	Gadgetron::scal((T)(1.0 / M), mean);
		//	hoNDArray<T> MMH(N, N);
		//	Gadgetron::clear(MMH);
		//	hoNDArray<T> meanCopy(mean);
		//	Gadgetron::gemm(MMH, meanCopy, false, mean, true);
		//	Gadgetron::subtract(V, MMH, V);
		//}
		//// compute eigen vector and values
		//Gadgetron::heev(V, E);
		//// make the first eigen channel with the largest eigen value
		//hoNDArray<T> V_flipped(V), E_flipped(E);
		//for (size_t r = 0; r < N; r++)
		//{
		//	for (size_t c = 0; c < N; c++)
		//	{
		//		V_flipped(r, c) = V(r, N - 1 - c);
		//	}
		//	E_flipped(r) = E(N - 1 - r);
		//}
		//V = V_flipped;
		//E = E_flipped;
	} catch (...) {
		GERROR("Errors in hoNDKLT_CS<T>::compute_eigen_vector(output_length) ...\n");
	}
}

template<typename T>
void hoNDKLT_CS<T>::prepare(const hoNDArray<T> &data, size_t dim, size_t output_length, bool remove_mean)
{
	try {
		size_t NDim = data.get_number_of_dimensions();
		GADGET_CHECK_THROW(dim<NDim);

		size_t N = data.get_size(dim);
		if (output_length > 0 && output_length <= N) {
			output_length_ = output_length;
		} else {
			output_length_ = N;
		}

		std::vector<size_t> dimD;
		data.get_dimensions(dimD);

		size_t K = 1;
		for (size_t n = dim + 1; n < NDim; n++) {
			K *= dimD[n];
		}

		if (dim == NDim - 1) {
			this->compute_eigen_vector(data, remove_mean);
		} else if (K == 1) {
			std::vector<size_t> dimShrinked(dim + 1);
			memcpy(&dimShrinked[0], &dimD[0], sizeof(size_t)*(dim + 1));
			hoNDArray<T> dataS;
			dataS.create(dimShrinked, const_cast<T*>(data.begin()));
			this->compute_eigen_vector(dataS, remove_mean);
		} else {
			std::vector<size_t> dimOrder(NDim), dimPermuted(dimD);
			size_t l;

			for (l = 0; l<NDim; l++) {
				dimOrder[l] = l;
				dimPermuted[l] = dimD[l];
			}

			dimOrder[dim] = NDim - 1;
			dimOrder[NDim - 1] = dim;
			dimPermuted[dim] = dimD[NDim - 1];
			dimPermuted[NDim - 1] = dimD[dim];

			hoNDArray<T> dataP;
			dataP.create(dimPermuted);
			Gadgetron::permute(data, dataP, dimOrder);
			this->compute_eigen_vector(dataP, remove_mean);
		}

		GADGET_CHECK_THROW(V_.get_size(0)==N);
		GADGET_CHECK_THROW(V_.get_size(1) == N);
		GADGET_CHECK_THROW(E_.get_size(0) == N);

		M_.create(N, output_length_, V_.begin());
	} catch (...) {
		GERROR("Errors in hoNDKLT_CS<T>::prepare(output_length) ...\n");
	}
}

template<typename T>
void hoNDKLT_CS<T>::compute_num_kept(value_type thres)
{
	size_t N = E_.get_size(0);

	if (thres <= 0) {
		output_length_ = N;
	} else {
		size_t n;
		for (n = 1; n < N; n++) {
			if (std::abs(E_(n)) < thres*std::abs(E_(0))) {
				break;
			}
		}

		output_length_ = n;
	}
}

template<typename T>
void hoNDKLT_CS<T>::prepare(const hoNDArray<T> &data, size_t dim, value_type thres, bool remove_mean)
{
	try {
		this->prepare(data, dim, (size_t)0, remove_mean);
		this->compute_num_kept(thres);
		M_.create(E_.get_size(0), output_length_, V_.begin());
	} catch (...) {
		GERROR("Errors in hoNDKLT_CS<T>::prepare(thres) ...\n");
	}
}

template<typename T>
void hoNDKLT_CS<T>::exclude_untransformed(const hoNDArray<T> &data, size_t dim, std::vector<size_t> &untransformed, hoNDArray<T> &dataCropped)
{
	try {
		size_t NDim = data.get_number_of_dimensions();

		GADGET_CHECK_THROW(dim < NDim);

		size_t N = data.get_size(dim);
		size_t unN = untransformed.size();

		GADGET_CHECK_THROW(unN < N);

		for (size_t d = 0; d < unN; d++) {
			GADGET_CHECK_THROW(untransformed[d] < N);
		}

		// crop the data to exclude untransformed slots
		std::vector<size_t> dims;
		data.get_dimensions(dims);
		std::vector<size_t> dimCropped(dims);
		dimCropped[dim] = N - unN;
		dataCropped.create(dimCropped);
		size_t numCopySize = 1;

		for (size_t d = 0; d < dim; d++) {
			numCopySize *= dims[d];
		}

		size_t num = data.get_number_of_elements() / (numCopySize*N);
		for (size_t n = 0; n < num; n++) {
			size_t ind = 0;

			for (size_t d = 0; d < N; d++) {
				bool isUntransformed = false;

				for (size_t un = 0; un < unN; un++) {
					if (untransformed[un] == d) {
						isUntransformed = true;
						break;
					}
				}

				if (isUntransformed) {
					continue;
				} else {
					memcpy(dataCropped.begin() + n*numCopySize*(N - unN) + ind*numCopySize, data.begin() + n*numCopySize*N + d*numCopySize, sizeof(T)*numCopySize);
					ind++;
				}
			}
		}
	} catch (...) {
		GERROR("Errors in hoNDKLT_CS<T>::exclude_untransformed(...) ...\n");
	}
}

template<typename T>
void hoNDKLT_CS<T>::copy_and_reset_transform(size_t N, std::vector<size_t> &untransformed)
{
	try {
		// adjust the eigen vector matrix
		hoNDArray<T> V;
		V.create(N, N);
		Gadgetron::clear(V);

		hoNDArray<T> E;
		E.create(N, 1);
		Gadgetron::clear(V);

		size_t unN = untransformed.size();

		// set the columns for the untransformed slots
		for (size_t d = 0; d < unN; d++) {
			V(untransformed[d], d) = 1;
			E(d) = E_(0);
		}

		// set the colunmns for the transformed slots
		for (size_t d = unN; d < N; d++) {
			size_t ind = 0;

			for (size_t n = 0; n < N; n++) {
				bool isUntransformed = false;

				for (size_t un = 0; un < unN; un++) {
					if (untransformed[un] == n) {
						isUntransformed = true;
						break;
					}
				}

				if (!isUntransformed) {
					V(n, d) = V_(ind, d - unN);
					ind++;
				}
			}

			E(d) = E_(d - unN);
		}

		V_ = V;
		E_ = E;
	} catch (...) {
		GERROR("Errors in hoNDKLT_CS<T>::copy_and_reset_transform(...) ...\n");
	}
}

template<typename T>
void hoNDKLT_CS<T>::prepare(const hoNDArray<T> &data, size_t dim, std::vector<size_t> &untransformed, size_t output_length, bool remove_mean)
{
	try {
		size_t NDim = data.get_number_of_dimensions();

		GADGET_CHECK_THROW(dim<NDim);

		size_t N = data.get_size(dim);
		size_t unN = untransformed.size();

		GADGET_CHECK_THROW(unN<N);

		if (output_length > 0) {
			GADGET_CHECK_THROW(output_length >= unN);
		}

		for (size_t d = 0; d < unN; d++) {
			GADGET_CHECK_THROW(untransformed[d] < N);
		}

		if (unN > 0) {
			// crop the data to exclude untransformed slots
			hoNDArray<T> dataCropped;
			this->exclude_untransformed(data, dim, untransformed, dataCropped);

			// compute the transform
			if (output_length > 0) {
				this->prepare(dataCropped, dim, output_length - unN, remove_mean);
			} else {
				this->prepare(dataCropped, dim, (size_t)0, remove_mean);
			}

			// adjust the eigen vector matrix
			this->copy_and_reset_transform(N, untransformed);
			output_length_ += unN;
			M_.create(N, output_length_, V_.begin());
		} else {
			this->prepare(data, dim, output_length, remove_mean);
		}
	} catch (...) {
		GERROR("Errors in hoNDKLT_CS<T>::prepare(untransformed, thres) ...\n");
	}
}

template<typename T>
void hoNDKLT_CS<T>::prepare(const hoNDArray<T> &data, size_t dim, std::vector<size_t> &untransformed, value_type thres, bool remove_mean)
{
	try {
		size_t unN = untransformed.size();
		if (unN > 0) {
			this->prepare(data, dim, untransformed, (size_t)(0), remove_mean);
			this->compute_num_kept(thres);
			M_.create(data.get_size(dim), output_length_, V_.begin());
		} else {
			this->prepare(data, dim, thres, remove_mean);
		}
	} catch (...) {
		GERROR("Errors in hoNDKLT_CS<T>::prepare(untransformed, thres) ...\n");
	}
}

template<typename T>
void hoNDKLT_CS<T>::transform(const hoNDArray<T> &in, hoNDArray<T> &out, size_t dim) const
{
	try {
		size_t NDim = in.get_number_of_dimensions();

		GADGET_CHECK_THROW(dim<NDim);
		GADGET_CHECK_THROW(in.get_size(dim)==M_.get_size(0));

		std::vector<size_t> dim_in;
		in.get_dimensions(dim_in);
		std::vector<size_t> dim_out;
		dim_out = dim_in;
		dim_out[dim] = M_.get_size(1);
		out.create(dim_out);
		size_t N = M_.get_size(0);
		size_t num = in.get_number_of_elements() / N;

		size_t K = 1;
		for (size_t n = dim + 1; n < NDim; n++) {
			K *= dim_in[n];
		}

		if ((dim == NDim - 1) || (K == 1)) {
			hoNDArray<T> in2D;
			in2D.create(num, N, const_cast<T*>(in.begin()));
			hoNDArray<T> out2D;
			out2D.create(num, M_.get_size(1), out.begin());
			Gadgetron::gemm(out2D, in2D, false, M_, false);
		} else {
			std::vector<size_t> dimOrder(NDim), dimPermuted(dim_in);

			for (size_t l = 0; l<NDim; l++) {
				dimOrder[l] = l;
				dimPermuted[l] = dim_in[l];
			}

			dimOrder[dim] = NDim-1;
			dimOrder[NDim - 1] = dim;
			dimPermuted[dim] = dim_in[NDim - 1];
			dimPermuted[NDim - 1] = dim_in[dim];

			hoNDArray<T> inP;
			inP.create(dimPermuted);
			Gadgetron::permute(in, inP, dimOrder);

			hoNDArray<T> inP2D;
			inP2D.create(num, N, inP.begin());
			dimPermuted[NDim - 1] = M_.get_size(1);

			hoNDArray<T> outP;
			outP.create(dimPermuted);
			hoNDArray<T> outP2D;
			outP2D.create(num, M_.get_size(1), outP.begin());

			Gadgetron::gemm(outP2D, inP2D, false, M_, false);
			Gadgetron::permute(outP, out, dimOrder);
		}
	} catch (...) {
		GERROR("Errors in hoNDKLT_CS<T>::transform(hoNDArray<T>& in, hoNDArray<T>& out, size_t dim) ...\n");
	}
}

template<typename T>
void hoNDKLT_CS<T>::ftransform(const hoNDArray<T> &in, hoNDArray<T> &out, size_t dim) const
{
	try {
		size_t NDim = in.get_number_of_dimensions();

		GADGET_CHECK_THROW(dim<NDim);
		GADGET_CHECK_THROW(in.get_size(dim)==M_.get_size(0));

		std::vector<size_t> dim_in;
		in.get_dimensions(dim_in);
		std::vector<size_t> dim_out;
		dim_out = dim_in;
		dim_out[dim] = M_.get_size(1);
		out.create(dim_out);
		size_t N = M_.get_size(0);
		size_t num = in.get_number_of_elements() / N;

		size_t K = 1;
		for (size_t n = dim + 1; n < NDim; n++) {
			K *= dim_in[n];
		}

		if ((dim == NDim - 1) || (K == 1)) {
			hoNDArray<T> in2D;
			in2D.create(num, N, const_cast<T*>(in.begin()));
			hoNDArray<T> out2D;
			out2D.create(num, M_.get_size(1), out.begin());
			Gadgetron::gemm(out2D, in2D, false, M_, false);
		} else {
			std::vector<size_t> dimOrder(NDim), dimPermuted(dim_in);

			for (size_t l = 0; l<NDim; l++) {
				dimOrder[l] = l;
				dimPermuted[l] = dim_in[l];
			}

			dimOrder[dim] = NDim-1;
			dimOrder[NDim - 1] = dim;
			dimPermuted[dim] = dim_in[NDim - 1];
			dimPermuted[NDim - 1] = dim_in[dim];

			hoNDArray<T> inP;
			inP.create(dimPermuted);
			Gadgetron::permute(in, inP, dimOrder);

			hoNDArray<T> inP2D;
			inP2D.create(num, N, inP.begin());
			dimPermuted[NDim - 1] = M_.get_size(1);

			hoNDArray<T> outP;
			outP.create(dimPermuted);

			hoNDArray<T> outP2D;
			outP2D.create(num, M_.get_size(1), outP.begin());

			Gadgetron::gemm(outP2D, inP2D, false, M_, false);
			Gadgetron::permute(outP, out, dimOrder);
		}
	} catch (...) {
		GERROR("Errors in hoNDKLT_CS<T>::transform(hoNDArray<T>& in, hoNDArray<T>& out, size_t dim) ...\n");
	}
}

template<typename T>
void hoNDKLT_CS<T>::btransform(const hoNDArray<T> &in, hoNDArray<T> &out, size_t dim) const
{
	try {
		size_t NDim = in.get_number_of_dimensions();

		GADGET_CHECK_THROW(dim<NDim);
		GADGET_CHECK_THROW(in.get_size(dim)==M_.get_size(0));

		std::vector<size_t> dim_in;
		in.get_dimensions(dim_in);
		std::vector<size_t> dim_out;
		dim_out = dim_in;
		dim_out[dim] = M_.get_size(1);
		out.create(dim_out);
		size_t N = M_.get_size(0);
		size_t num = in.get_number_of_elements() / N;

		size_t K = 1;
		for (size_t n = dim + 1; n < NDim; n++) {
			K *= dim_in[n];
		}

		if ((dim == NDim - 1) || (K == 1)) {
			hoNDArray<T> in2D;
			in2D.create(num, N, const_cast<T*>(in.begin()));

			hoNDArray<T> out2D;
			out2D.create(num, M_.get_size(1), out.begin());
			Gadgetron::gemm(out2D, in2D, false, M_, true);
		} else {
			std::vector<size_t> dimOrder(NDim), dimPermuted(dim_in);

			for (size_t l = 0; l<NDim; l++) {
				dimOrder[l] = l;
				dimPermuted[l] = dim_in[l];
			}

			dimOrder[dim] = NDim-1;
			dimOrder[NDim - 1] = dim;
			dimPermuted[dim] = dim_in[NDim - 1];
			dimPermuted[NDim - 1] = dim_in[dim];

			hoNDArray<T> inP;
			inP.create(dimPermuted);
			Gadgetron::permute(in, inP, dimOrder);

			hoNDArray<T> inP2D;
			inP2D.create(num, N, inP.begin());
			dimPermuted[NDim - 1] = M_.get_size(1);
			hoNDArray<T> outP;
			outP.create(dimPermuted);
			hoNDArray<T> outP2D;
			outP2D.create(num, M_.get_size(1), outP.begin());

			Gadgetron::gemm(outP2D, inP2D, false, M_, true);
			Gadgetron::permute(outP, out, dimOrder);
		}
	} catch (...) {
		GERROR("Errors in hoNDKLT_CS<T>::transform(hoNDArray<T>& in, hoNDArray<T>& out, size_t dim) ...\n");
	}
}

template<typename T>
void hoNDKLT_CS<T>::KL_filter(const hoNDArray<T> &in, hoNDArray<T> &out, size_t dim, size_t mode_kept) const
{
	try {
		size_t NDim = in.get_number_of_dimensions();

		GADGET_CHECK_THROW(dim<NDim);
		GADGET_CHECK_THROW(in.get_size(dim) == V_.get_size(0));

		std::vector<size_t> dim_in;
		in.get_dimensions(dim_in);
		out.create(dim_in);
		size_t N = V_.get_size(0);
		size_t num = in.get_number_of_elements() / N;

		hoMatrix<T> E(N, N);
		memcpy(E.begin(), V_.begin(), sizeof(T)*N*N);

		for (size_t c = mode_kept; c < N; c++) {
			for (size_t r = 0; r < N; r++) {
				E(r, c) = T(0);
			}
		}

		hoMatrix<T> ET;
		ET.createMatrix(N, N);
		memcpy(ET.begin(), V_.begin(), sizeof(T)*N*N);

		Gadgetron::conjugatetrans(E, ET);

		hoMatrix<T> EET(N, N);
		Gadgetron::clear(EET);
		Gadgetron::gemm(EET, E, false, ET, false);

		size_t K = 1;
		for (size_t n = dim + 1; n < NDim; n++) {
			K *= dim_in[n];
		}

		if ((dim == NDim - 1) || (K==1)) {
			hoNDArray<T> in2D;
			in2D.create(num, N, const_cast<T*>(in.begin()));
			hoNDArray<T> out2D;
			out2D.create(num, N, out.begin());
			Gadgetron::gemm(out2D, in2D, false, EET, false);
		} else {
			std::vector<size_t> dimOrder(NDim), dimPermuted(dim_in);

			for (size_t l = 0; l<NDim; l++) {
				dimOrder[l] = l;
				dimPermuted[l] = dim_in[l];
			}

			dimOrder[dim] = NDim - 1;
			dimOrder[NDim - 1] = dim;
			dimPermuted[dim] = dim_in[NDim - 1];
			dimPermuted[NDim - 1] = dim_in[dim];

			hoNDArray<T> inP;
			inP.create(dimPermuted);
			Gadgetron::permute(in, inP, dimOrder);
			hoNDArray<T> inP2D;
			inP2D.create(num, N, inP.begin());
			hoNDArray<T> outP;
			outP.create(dimPermuted);
			hoNDArray<T> outP2D;
			outP2D.create(num, N, outP.begin());
			Gadgetron::gemm(outP2D, inP2D, false, EET, false);
			Gadgetron::permute(outP, out, dimOrder);
		}
	} catch (...) {
		GERROR("Errors in hoNDKLT_CS<T>::KL_filter(const hoNDArray<T>& in, hoNDArray<T>& out, size_t dim, size_t mode_kept) ...\n");
	}
}

template<typename T>
size_t hoNDKLT_CS<T>::transform_length() const
{
	return M_.get_size(0);
}

template<typename T>
size_t hoNDKLT_CS<T>::output_length() const
{
	GADGET_CHECK_THROW(M_.get_size(1) == output_length_);
	return output_length_;
}

template<typename T>
void hoNDKLT_CS<T>::output_length(size_t length)
{
	if (M_.get_size(0) == V_.get_size(0)) {
		GADGET_CHECK_THROW(V_.get_size(0) == V_.get_size(1));

		size_t N = M_.get_size(0);

		if (length > 0 && length <= N) {
			output_length_ = length;
		} else {
			output_length_ = N;
		}

		M_.create(N, output_length_, V_.begin());
	} else {
		size_t N = M_.get_size(0);
		size_t M = M_.get_size(1);
		output_length_ = (length <= M) ? length : M;

		if (output_length_<M) {
			hoNDArray<T> Mc(M_);
			M_.create(N, length);

			memcpy(M_.begin(), Mc.begin(), sizeof(T)*N*length);
		}
	}
}

template<typename T>
void hoNDKLT_CS<T>::KL_transformation(hoNDArray<T> &M) const
{
	M = M_;
}

template<typename T>
void hoNDKLT_CS<T>::eigen_vector(hoNDArray<T> &V) const
{
	V = V_;
}

template<typename T>
void hoNDKLT_CS<T>::eigen_value(hoNDArray<T> &E) const
{
	E = E_;
}
