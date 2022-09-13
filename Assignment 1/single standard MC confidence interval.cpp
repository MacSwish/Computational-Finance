// HEADER
// Student ID: 10134521
// Title: Assignment 1
// Date Created: 18/03/21
// Last Edited:


// math constants
#define _USE_MATH_DEFINES_


// Includes
#include <random>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <math.h>
#include <vector>


// Function declerations

// perform monte carlo
double MonteCarlo(const double& initial_share_price, const double& interest_rate, const double& dividend_rate, const double& volatility,
	const double& expiration, const int& N, const int& put_number, const int& call_number, const int& binary_put_number,
	const int& binary_call_number, const int& zero_strike_call_number, const double& put_strike, const double& call_strike,
	const double& binary_put_strike, const double& binary_call_strike);

// calculate d1
double d1(const double& share_price, const double& strike_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time);

// calculate d2
double d2(const double& share_price, const double& strike_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time);

// payoff for put
double payoff_put(const double& share_price, const double& strike_price);

// analystical put
double analytic_put(const double& share_price, const double& strike_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time);

// payoff for call
double payoff_call(const double& share_price, const double& strike_price);

// analystical call
double analytic_call(const double& share_price, const double& strike_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time);

// payoff for binary put
double payoff_binary_put(const double& share_price, const double& strike_price);

// analytic binary put
double analytic_binary_put(const double& share_price, const double& strike_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time);

// payoff for binary call
double payoff_binary_call(const double& share_price, const double& strike_price);

// analytic binary call
double analytic_binary_call(const double& share_price, const double& strike_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time);

// payoff for zero strike call
double payoff_zero_strike_call(const double& share_price);

// analytic zero strike call
double analytic_zero_strike_call(const double& share_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time);

// calculate portfolio payoff
double portfolio_payoff(const int& put_number, const int& call_number, const int& binary_put_number, const int& binary_call_number,
	const int& zero_strike_call_number, const double& put_strike, const double& call_strike, const double& binary_put_strike,
	const double& binary_call_strike, const double& share_price);

// calculate analytical portfolio value
double portfolio_analytic(const int& put_number, const int& call_number, const int& binary_put_number, const int& binary_call_number,
	const int& zero_strike_call_number, const double& put_strike, const double& call_strike, const double& binary_put_strike,
	const double& binary_call_strike, const double& share_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time);

// normal cummulative distribution
double norm_cumm(const double& x);


// Begin main program
int main()
{
	// constants
	double expiration{ 0.5 };
	double volatility{ 0.25 };
	double interest_rate{ 0.03 };
	double dividend_rate{ 0.01 };
	double X1{ 450 };
	double X2{ 700 };

	// portfolio setup
	int put_number{ 2 };
	int call_number{ 1 };
	int binary_put_number{ -700 };
	int binary_call_number{ 0 };
	int zero_strike_call_number{ -1 };
	double put_strike{ X1 };
	double call_strike{ X2 };
	double binary_put_strike{ X2 };
	double binary_call_strike{ 0 };
	double initial_share_price{ X1 };

	int M{ 100 };  // number of calculations
	std::vector<double> samples;  // store result from each calculation

	int N{ 200000 };  // number of monte carlo simulations to perform

	// loop over the number of calculations
	for (int i{ 0 }; i < M; i++) {

		// store monte carlo result
		samples.push_back(MonteCarlo(initial_share_price, interest_rate, dividend_rate, volatility, expiration, N, put_number, call_number, binary_put_number,
			binary_call_number, zero_strike_call_number, put_strike, call_strike, binary_put_strike, binary_call_strike));
	}

	// calculate the mean
	double sum_mean{ 0 };
	for (int i{ 0 }; i < samples.size(); i++) sum_mean += samples[i];
	double sample_mean = sum_mean / M;

	// calculate the variance
	double sum_var{ 0 };
	for (int i{ 0 }; i < samples.size(); i++) sum_var += pow(samples[i] - sample_mean, 2);
	double sample_variance = sum_var / (M - 1.);

	// calculate population mean and std
	double pop_mean = sample_mean;
	double pop_std = sqrt(sample_variance / M);

	// calculate confidence intervals
	double upper_95 = pop_mean + 2 * pop_std;
	double lower_95 = pop_mean - 2 * pop_std;

	// output results
	std::cout << "95% confidence result is in [" << lower_95 << "," << upper_95 << "] with N = " << N << std::endl;

	// calculate analystical result
	double current_time = 0;
	double current_ana_portfolio_value = portfolio_analytic(put_number, call_number, binary_put_number, binary_call_number, zero_strike_call_number,
		put_strike, call_strike, binary_put_strike, binary_call_strike, initial_share_price, interest_rate, dividend_rate, volatility,
		expiration, current_time);
	std::cout << "Analytic Pi(S,t=0) = " << current_ana_portfolio_value << std::endl;

	return 0;
}  // End main progrma


// Function definitions

// perform monte carlo
double MonteCarlo(const double& initial_share_price, const double& interest_rate, const double& dividend_rate, const double& volatility,
	const double& expiration, const int& N, const int& put_number, const int& call_number, const int& binary_put_number,
	const int& binary_call_number, const int& zero_strike_call_number, const double& put_strike, const double& call_strike,
	const double& binary_put_strike, const double& binary_call_strike)
{
	// declare random number generator
	static std::mt19937 rng;

	// declare the normal distrubtion
	std::normal_distribution<double> ND(0., 1.);

	// initialise sum to 0
	double sum = 0;

	// run the simulations
	for (int i{ 0 }; i < N; i++) {

		// draw a random normally distributed number
		double phi = ND(rng);

		// get random value of stock value at maturity 
		double final_share_price = initial_share_price * exp((interest_rate - dividend_rate - 0.5 * pow(volatility, 2)) * expiration + volatility * phi * pow(expiration, 0.5));

		// increment the sum
		sum += portfolio_payoff(put_number, call_number, binary_put_number, binary_call_number, zero_strike_call_number, put_strike,
			call_strike, binary_put_strike, binary_call_strike, final_share_price);
	}

	// output average over all paths
	return exp(-interest_rate * expiration) * sum / N;

}

// calculate d1
double d1(const double& share_price, const double& strike_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time)
{
	return (log(share_price / strike_price) + (interest_rate - divident_rate + pow(volatility, 2) / 2) * (expiration - time)) / (volatility * pow(expiration - time, 0.5));
}

// calculate d2
double d2(const double& share_price, const double& strike_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time)
{
	return d1(share_price, strike_price, interest_rate, divident_rate, volatility, expiration, time) - volatility * pow(expiration - time, 0.5);
}

// payoff for put
double payoff_put(const double& share_price, const double& strike_price)
{
	return std::max(strike_price - share_price, 0.);
}

// analystical put
double analytic_put(const double& share_price, const double& strike_price, const double& interest_rate, const double& dividend_rate,
	const double& volatility, const double& expiration, const double& time)
{
	double d1_val = d1(share_price, strike_price, interest_rate, dividend_rate, volatility, expiration, time);
	double d2_val = d2(share_price, strike_price, interest_rate, dividend_rate, volatility, expiration, time);

	return strike_price * exp(-interest_rate * (expiration - time)) * norm_cumm(-d2_val) - share_price * exp(-dividend_rate * (expiration - time)) * norm_cumm(-d1_val);
}

// payoff for call
double payoff_call(const double& share_price, const double& strike_price)
{
	return std::max(share_price - strike_price, 0.);
}

// analystical call
double analytic_call(const double& share_price, const double& strike_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time)
{
	double d1_val = d1(share_price, strike_price, interest_rate, divident_rate, volatility, expiration, time);
	double d2_val = d2(share_price, strike_price, interest_rate, divident_rate, volatility, expiration, time);

	return share_price * exp(-divident_rate * (expiration - time)) * norm_cumm(d1_val) - strike_price * exp(-interest_rate * (expiration - time)) * norm_cumm(d2_val);
}

// payoff for binary put
double payoff_binary_put(const double& share_price, const double& strike_price)
{
	if (share_price <= strike_price) return 1;
	else return 0;
}

// analytic binary put
double analytic_binary_put(const double& share_price, const double& strike_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time)
{
	double d2_val = d2(share_price, strike_price, interest_rate, divident_rate, volatility, expiration, time);

	return exp(-interest_rate * (expiration - time)) * norm_cumm(-d2_val);
}

// payoff for binary call
double payoff_binary_call(const double& share_price, const double& strike_price)
{
	if (share_price <= strike_price) return 0;
	else return 1;
}

// analytic binary call
double analytic_binary_call(const double& share_price, const double& strike_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time)
{
	double d2_val = d2(share_price, strike_price, interest_rate, divident_rate, volatility, expiration, time);

	return exp(-interest_rate * (expiration - time)) * norm_cumm(d2_val);
}

// payoff for zero strike call
double payoff_zero_strike_call(const double& share_price)
{
	return share_price;
}

// analytic zero strike call
double analytic_zero_strike_call(const double& share_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time)
{
	return share_price * exp(-divident_rate * (expiration - time));
}

// calculate portfolio value
double portfolio_payoff(const int& put_number, const int& call_number, const int& binary_put_number, const int& binary_call_number,
	const int& zero_strike_call_number, const double& put_strike, const double& call_strike, const double& binary_put_strike,
	const double& binary_call_strike, const double& share_price)
{
	return put_number * payoff_put(share_price, put_strike) + call_number * payoff_call(share_price, call_strike) +
		binary_put_number * payoff_binary_put(share_price, binary_put_strike) + binary_call_number * payoff_binary_call(share_price, binary_call_strike) +
		zero_strike_call_number * payoff_zero_strike_call(share_price);
}

// calculate analystical portfolio
double portfolio_analytic(const int& put_number, const int& call_number, const int& binary_put_number, const int& binary_call_number,
	const int& zero_strike_call_number, const double& put_strike, const double& call_strike, const double& binary_put_strike,
	const double& binary_call_strike, const double& share_price, const double& interest_rate, const double& divident_rate,
	const double& volatility, const double& expiration, const double& time)
{
	return put_number * analytic_put(share_price, put_strike, interest_rate, divident_rate, volatility, expiration, time) +
		call_number * analytic_call(share_price, call_strike, interest_rate, divident_rate, volatility, expiration, time) +
		binary_put_number * analytic_binary_put(share_price, binary_put_strike, interest_rate, divident_rate, volatility, expiration, time) +
		binary_call_number * analytic_binary_call(share_price, binary_call_strike, interest_rate, divident_rate, volatility, expiration, time) +
		zero_strike_call_number * analytic_zero_strike_call(share_price, interest_rate, divident_rate, volatility, expiration, time);
}

// normal cummulative distribution
double norm_cumm(const double& x)
{
	return 0.5 * erfc(-x / pow(2, 0.5));
}