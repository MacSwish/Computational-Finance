// Includes
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <chrono>


// Function declerations

// calculate theta(t)
double theta(const double& mu, const double& X, const double& dt, const int& i);

// Thomas solver
std::vector<double> thomas_solve(const std::vector<double>& a, const std::vector<double>& b_, const std::vector<double>& c, const std::vector<double>& d);

// generic lagrange interpolation
double lagrange_interpolation(const std::vector<double>& y, const std::vector<double>& x, double x0, unsigned int n);

// Crank Nicolson Finite Difference
double crank_nicolson(const double& T, const double& F, const double& R, const double& r, const double& kappa, const double& mu,
	const double& S0, const double& X, const double& C, const double& alpha, const double& beta, const double& sigma, const int& i_max, const int& j_max, double& S_max, 
	const double& rho, const double& tol, const int& iter_max);


// Begin main program
int main()
{
	// declare and initialise parameters
	double T{ 2 };
	double F{ 50 };
	double R{ 1 };
	double r{ 0.0114 };
	double kappa{ 0.125 };
	double mu{ 0.0174 };
	double S0{ 50.5 };
	double X{ 50.5 };
	double C{ 0.285 };
	double alpha{ 0.01 };
	double beta{ 0.869 };
	double sigma{ 0.668 };

	// American option params
	double rho{ 1e8 };  // penalty param
	double tol{ 1e-4 };  // tolerance level
	int iter_max{ 10000 };  // maximum iterations

	// declare and initialise grid parameters
	int i_max{ 100 };
	int j_max{ 100 };

	// maximum S
	double S_max = 5 * F;

	auto start = std::chrono::steady_clock::now();  // get start time

	// get option value
	double option_value = crank_nicolson(T, F, R, r, kappa, mu, S0, X, C, alpha, beta, sigma, i_max, j_max, S_max, rho, tol, iter_max);

	auto finish = std::chrono::steady_clock::now();  // get finish time
	auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>> (finish - start);  // time difference

	// output calculation time
	std::cout << elapsed.count() << std::endl;

	// output result
	std::cout << "V(S = " << S0 << ", t = 0) = " << std::setprecision(10) << option_value << std::endl;



}  // End main program


// Function definitions

// calculate theta(t)
double theta(const double& mu, const double& X, const double& dt, const int& i)
{
	return (1 + mu) * X * exp(mu * i * dt);
}

// Thomas solver
std::vector<double> thomas_solve(const std::vector<double>& a, const std::vector<double>& b_, const std::vector<double>& c, std::vector<double>& d)
{
	// get size of vector
	int n = a.size();

	// local parameteres
	std::vector<double> b(n), temp(n);

	// initial first value of b
	b[0] = b_[0];

	// get other values
	for (int j = 1; j < n; j++)
	{
		b[j] = b_[j] - c[j - 1] * a[j] / b[j - 1];
		d[j] = d[j] - d[j - 1] * a[j] / b[j - 1];
	}

	// calculate solution
	temp[n - 1] = d[n - 1] / b[n - 1];
	for (int j = n - 2; j >= 0; j--) temp[j] = (d[j] - c[j] * temp[j + 1]) / b[j];

	return temp;
}

// generic lagrange interpolation
double lagrange_interpolation(const std::vector<double>& y, const std::vector<double>& x, double x0, unsigned int n)
{
	if (x.size() < n) return lagrange_interpolation(y, x, x0, x.size());
	if (n == 0)throw;

	// local parameters
	int nHalf = n / 2;
	double dx = x[1] - x[0];

	// calculate j star
	int jStar;
	if (n % 2 == 0) jStar = int((x0 - x[0]) / dx) - (nHalf - 1);  // even degree
	else jStar = int((x0 - x[0]) / dx + 0.5) - (nHalf);  // odd degree

	jStar = std::max(0, jStar);
	jStar = std::min(int(x.size() - n), jStar);

	if (n == 1)return y[jStar];

	double temp = 0.;
	for (unsigned int i = jStar; i < jStar + n; i++) {

		double  int_temp;
		int_temp = y[i];

		for (unsigned int j = jStar; j < jStar + n; j++) {

			if (j == i) { continue; }
			int_temp *= (x0 - x[j]) / (x[i] - x[j]);
		}
		temp += int_temp;
	}  // end of interpolate

	return temp;
}

// Crank Nicolson Finite Difference
double crank_nicolson(const double& T, const double& F, const double& R, const double& r, const double& kappa, const double& mu,
	const double& S0, const double& X, const double& C, const double& alpha, const double& beta, const double& sigma, const int& i_max, const int& j_max, double& S_max, 
	const double& rho, const double& tol, const int& iter_max)
{
	// declare and initialise local parameters (dS, dt)
	double dS = S_max / j_max;
	double dt = T / i_max;

	// create storage for stock price and old and new option price
	std::vector<double> S(j_max + 1);
	std::vector<double> v_old(j_max + 1);
	std::vector<double> v_new(j_max + 1);

	// initialise our stock prices
	for (int j{ 0 }; j <= j_max; j++) S[j] = j * dS;

	// initialise final conditions on the option price
	for (int j{ 0 }; j <= j_max; j++) {
		v_old[j] = std::max(F, R * S[j]);
		v_new[j] = std::max(F, R * S[j]);
	}

	// loop over the time levels
	for (int i{ i_max - 1 }; i >= 0; i--) {

		// declare matrix parameter vectors
		std::vector<double> a(j_max + 1);
		std::vector<double> b(j_max + 1);
		std::vector<double> c(j_max + 1);
		std::vector<double> d(j_max + 1);

		// initial values at j = 0
		a[0] = 0;
		b[0] = -(1 / dt) - (kappa * theta(mu, X, dt, i) / dS) - (r / 2);
		c[0] = kappa * theta(mu, X, dt, i) / dS;
		d[0] = (-(1 / dt) + (r / 2)) * v_old[0] - C * exp(-i * dt);

		// loop through middling values of j
		for (int j{ 1 }; j <= j_max - 1; j++) {

			a[j] = -0.25 * pow(sigma, 2) * pow(j, 2 * beta) * pow(dS, 2 * (beta - 1)) + (kappa / (4 * dS)) * (theta(mu, X, dt, i) - j * dS);
			b[j] = (1 / dt) + 0.5 * pow(sigma, 2) * pow(j, 2 * beta) * pow(dS, 2 * (beta - 1)) + (r / 2);
			c[j] = -0.25 * pow(sigma, 2) * pow(j, 2 * beta) * pow(dS, 2 * (beta - 1)) - (kappa / (4 * dS)) * (theta(mu, X, dt, i) - j * dS);
			d[j] = (0.25 * pow(sigma, 2) * pow(j, 2 * beta) * pow(dS, 2 * (beta - 1)) - (kappa / (4 * dS)) * (theta(mu, X, dt, i) - j * dS)) * v_old[j - 1]
				+ ((1 / dt) - 0.5 * pow(sigma, 2) * pow(j, 2 * beta) * pow(dS, 2 * (beta - 1)) - (r / 2)) * v_old[j]
				+ (0.25 * pow(sigma, 2) * pow(j, 2 * beta) * pow(dS, 2 * (beta - 1)) + (kappa / (4 * dS)) * (theta(mu, X, dt, i) - j * dS)) * v_old[j + 1]
				+ C * exp(-alpha * i * dt);
		}

		// initialise values at j = j_max
		a[j_max] = 0;
		b[j_max] = 1;
		c[j_max] = 0;
		d[j_max] = R * S[j_max];
		// d[j_max] = R * (S[j_max] - X) * exp(-(kappa + r) * (T - i * dt)) + (C / (alpha + r)) * exp(-alpha * i * dt) + (X * R - (C / (alpha + r)) * exp(-alpha * T)) * exp(-r * (T - i * dt));

		// penalty method
		int penalty_itr;
		for (penalty_itr = 0; penalty_itr < iter_max; penalty_itr++) {
		
			// create new vectors containing a copy of the FD approximations
			std::vector<double> a_hat(a), b_hat(b), c_hat(c), d_hat(d);

			// apply penalty to finite difference scheme
			for (int j{ 1 }; j < j_max; j++) {
			
				// if current value suggests to apply penalty, adjust matrix equations
				if (v_new[j] < R * S[j]) {
					b_hat[j] = b[j] + rho;
					d_hat[j] = d[j] + rho * (R * S[j]);
				}
			}

			// solve with Thomas method
			std::vector<double> y = thomas_solve(a_hat, b_hat, c_hat, d_hat);
			
			// check for differenc between y and v_new
			double error = 0;
			for (int j{ 0 }; j < j_max; j++) {
				error += pow(v_new[j] - y[j], 2);
			}

			// update v_new
			v_new = y;

			// exit if solution converged
			if (error < pow(tol, 2)) {
				std::cout << "Solved after " << penalty_itr << " iterations" << std::endl;
				break;
			}
		}

		// if no solution found
		if (penalty_itr >= iter_max) {
			std::cout << "Error: No convergence" << std::endl;
			throw;
		}

		// set old to new
		v_old = v_new;
	}

	// use lagrange interpolation to get estimated option value
	double option_value = lagrange_interpolation(v_new, S, S0, 8);

	return option_value;
}