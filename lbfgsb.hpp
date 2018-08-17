/**
 *           c++11-only implementation of the L-BFGS-B algorithm
 *
 * Copyright (c) 2014 Patrick Wieschollek
 *               https://github.com/PatWie/LBFGSB
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef LBFGSB_H_
#define LBFGSB_H_

#include "meta.h"

#include <list>
#include <stdio.h>
#include <iostream>
#include <functional>
#include <vector>
#include <armadillo>

/* coded from scratch !!!
 * based on the paper
 * A LIMITED MEMORY ALGORITHM FOR BOUND CONSTRAINED OPTIMIZATION
 * (Byrd, Lu, Nocedal, Zhu)
 */

class LBFGSB {

	// contains options for optimization process
	Options Options_;

	// oracles for function value and gradient
	FunctionOracleType FunctionObjectiveOracle_;
	GradientOracleType FunctionGradientOracle_;

	arma::mat W, M;
	arma::vec lb, ub;
	double theta;
	int DIM;

	std::list<arma::vec> xHistory;

public:

	arma::vec XOpt;

	LBFGSB(const arma::vec &l, const arma::vec &u) :
			lb(l), ub(u), theta(1.0), DIM(l.n_rows) {
		lb = l;
		ub = u;
		theta = 1.0;
		DIM = l.n_rows;
		W = arma::zeros(DIM, 0);
		M = arma::zeros(0, 0);
	}

	LBFGSB(Options &Options, const arma::vec &l, const arma::vec &u) {
		Options_ = Options;
		lb = l;
		ub = u;
		theta = 1.0;
		DIM = l.n_rows;
		W = arma::zeros(DIM, 0);
		M = arma::zeros(0, 0);

	}

	/// <summary>
	/// find cauchy point in x
	/// </summary>
	/// <parameter name="x">start in x</parameter>
	void GetGeneralizedCauchyPoint(arma::vec &x, arma::vec &g, arma::vec &x_cauchy,
																 arma::vec &c) {
		const int DIM = x.n_rows;
		// PAGE 8
		// Algorithm CP: Computation of the generalized Cauchy point
		// Given x,l,u,g, and B = \theta I-WMW

		// {all t_i} = { (idx,value), ... }
		// TODO: use "std::set" ?
		std::vector<std::pair<int, double> > SetOfT;
		// the feasible set is implicitly given by "SetOfT - {t_i==0}"
		arma::vec d = arma::zeros(DIM);

		// n operations
		for (int j = 0; j < DIM; j++) {
			if (g(j) == 0) {
				SetOfT.push_back(std::make_pair(j, INF));
			} else {
				double tmp = 0;
				if (g(j) < 0) {
					tmp = (x(j) - ub(j)) / g(j);
				} else {
					tmp = (x(j) - lb(j)) / g(j);
				}
				d(j) = -g(j);
				SetOfT.push_back(std::make_pair(j, tmp));
			}

		}Debug(d.transpose());

		// paper: using heapsort
		// sortedindices [1,0,2] means the minimal element is on the 1th entry
		std::vector<int> SortedIndices = sort_indexes(SetOfT);

		x_cauchy = x;
		// Initialize
		// p := 	W^T*p
		arma::vec p = (W.t() * d);						// (2mn operations)
		// c := 	0
		c = arma::zeros(M.n_rows);
		// f' := 	g^T*d = -d^Td
		double f_prime = arma::dot(-d,d);							// (n operations)
		// f'' :=	\theta*d^T*d-d^T*W*M*W^T*d = -\theta*f' - p^T*M*p
		double f_doubleprime = (double) (-1.0 * theta) * f_prime - arma::dot(p,M * p);// (O(m^2) operations)
		// \delta t_min :=	-f'/f''
		double dt_min = -f_prime / f_doubleprime;
		// t_old := 	0
		double t_old = 0;
		// b := 	argmin {t_i , t_i >0}
		int i = 0;
		for (int j = 0; j < DIM; j++) {
			i = j;
			if (SetOfT[SortedIndices[j]].second != 0)
				break;
		}
		int b = SortedIndices[i];
		// see below
		// t        			:= 	min{t_i : i in F}
		double t = SetOfT[b].second;
		// \delta t 			:= 	t - 0
		double dt = t - t_old;

		// examination of subsequent segments
		while ((dt_min >= dt) && (i < DIM)) {
			if (d(b) > 0)
				x_cauchy(b) = ub(b);
			else if (d(b) < 0)
				x_cauchy(b) = lb(b);

			// z_b = x_p^{cp} - x_b
			double zb = x_cauchy(b) - x(b);
			// c   :=  c +\delta t*p
			c += dt * p;
			// cache
			arma::rowvec wbt = W.row(b);

			f_prime += dt * f_doubleprime + g(b) * g(b)
					+ theta * g(b) * zb
					- g(b) * arma::dot(wbt.t(), (M * c));
			f_doubleprime += -1.0 * theta * g(b) * g(b)
					-  2.0 * (g(b) * arma::dot(wbt.t(),(M * p)))
					-  g(b) * g(b) * arma::dot(wbt.t(),M * wbt);
			p += g(b) * wbt.t();
			d(b) = 0;
			dt_min = -f_prime / f_doubleprime;
			t_old = t;
			++i;
			if (i < DIM) {
				b = SortedIndices[i];
				t = SetOfT[b].second;
				dt = t - t_old;
			}

		}

		dt_min = max(dt_min, 0);
		t_old += dt_min;

		Debug(SortedIndices[0]<< " "<< SortedIndices[1]);

#pragma omp parallel for
		for (int ii = i; ii < x_cauchy.n_rows; ii++) {
			x_cauchy(SortedIndices[ii]) = x(SortedIndices[ii])
					+ t_old * d(SortedIndices[ii]);
		}Debug(x_cauchy.transpose());

		c += dt_min * p;
		Debug(c.transpose());

	}

	/// <summary>
	/// find valid alpha for (8.5)
	/// </summary>
	/// <parameter name="x_cp">cauchy point</parameter>
	/// <parameter name="du">unconstrained solution of subspace minimization</parameter>
	/// <parameter name="FreeVariables">flag (1 if is free variable and 0 if is not free variable)</parameter>
	double FindAlpha(arma::vec &x_cp, arma::vec &du,
			std::vector<int> &FreeVariables) {
		/* this returns
		 * a* = max {a : a <= 1 and  l_i-xc_i <= a*d_i <= u_i-xc_i}
		 */
		double alphastar = 1;
		const unsigned int n = FreeVariables.size();
		for (unsigned int i = 0; i < n; i++) {
			if (du(i) > 0) {
				alphastar = min(alphastar,
						(ub(FreeVariables[i]) - x_cp(FreeVariables[i]))
								/ du(i));
			} else {
				alphastar = min(alphastar,
						(lb(FreeVariables[i]) - x_cp(FreeVariables[i]))
								/ du(i));
			}
		}
		return alphastar;
	}

	/// <summary>
	/// using linesearch to determine step width
	/// </summary>
	/// <parameter name="x">start in x</parameter>
	/// <parameter name="dx">direction</parameter>
	/// <parameter name="f">current value of objective (will be changed)</parameter>
	/// <parameter name="g">current gradient of objective (will be changed)</parameter>
	/// <parameter name="t">step width (will be changed)</parameter>
	void LineSearch(arma::vec &x, arma::vec dx, double &f, arma::vec &g, double &t) {

		const double alpha = 0.2;
		const double beta = 0.8;

		const double f_in = f;
		const arma::vec g_in = g;
		const double Cache = alpha * arma::dot(g_in,dx);

		t = 1.0;
		f = FunctionObjectiveOracle_(x + t * dx);
		while (f > f_in + t * Cache) {
			t *= beta;
			f = FunctionObjectiveOracle_(x + t * dx);
		}
		FunctionGradientOracle_(x + t * dx, g);
		x += t * dx;

	}

	/// <summary>
	/// direct primal approach
	/// </summary>
	/// <parameter name="x">start in x</parameter>
	void SubspaceMinimization(arma::vec &x_cauchy, arma::vec &x, arma::vec &c, arma::vec &g,
														arma::vec &SubspaceMin) {

		// cached value: ThetaInverse=1/theta;
		double theta_inverse = 1 / theta;

		// size of "t"
		std::vector<int> FreeVariablesIndex;
		Debug(x_cauchy.transpose());

		//std::cout << "free vars " << FreeVariables.rows() << std::endl;
		for (int i = 0; i < x_cauchy.n_rows; i++) {
			Debug(x_cauchy(i) << " "<< ub(i) << " "<< lb(i));
			if ((x_cauchy(i) != ub(i)) && (x_cauchy(i) != lb(i))) {
				FreeVariablesIndex.push_back(i);
			}
		}
		const int FreeVarCount = FreeVariablesIndex.size();

		arma::mat WZ = arma::zeros(W.n_cols, FreeVarCount);

		for (int i = 0; i < FreeVarCount; i++)
			WZ.col(i) = W.row(FreeVariablesIndex[i]);

		Debug(WZ);

		// r=(g+theta*(x_cauchy-x)-W*(M*c));
		Debug(g);Debug(x_cauchy);Debug(x);
		arma::vec rr = (g + theta * (x_cauchy - x) - W * (M * c));
		// r=r(FreeVariables);
		arma::vec r = arma::zeros(FreeVarCount, 1);
		for (int i = 0; i < FreeVarCount; i++)
			r.row(i) = rr.row(FreeVariablesIndex[i]);

		Debug(r.transpose());

		// STEP 2: "v = w^T*Z*r" and STEP 3: "v = M*v"
		arma::vec v = M * (WZ * r);
		// STEP 4: N = 1/theta*W^T*Z*(W^T*Z)^T
		arma::mat N = theta_inverse * WZ * WZ.t();
		// N = I - MN
		N = arma::eye(N.n_rows, N.n_rows) - M * N;
		// STEP: 5
		// v = N^{-1}*v
		v = arma::inv(N) * v;
		// STEP: 6
		// HERE IS A MISTAKE IN THE ORIGINAL PAPER!
		arma::vec du = -theta_inverse * r
				- theta_inverse * theta_inverse * WZ.t() * v;
		Debug(du.transpose());
		// STEP: 7
		double alpha_star = FindAlpha(x_cauchy, du, FreeVariablesIndex);

		// STEP: 8
		arma::vec dStar = alpha_star * du;

		SubspaceMin = x_cauchy;
		for (int i = 0; i < FreeVarCount; i++) {
			SubspaceMin(FreeVariablesIndex[i]) = SubspaceMin(
					FreeVariablesIndex[i]) + dStar(i);
		}
	}

	void Solve(arma::vec &x0, const FunctionOracleType& FunctionValue,
			const GradientOracleType& FunctionGradient) {
		FunctionObjectiveOracle_ = FunctionValue;
		FunctionGradientOracle_ = FunctionGradient;

		Assert(x0.n_rows == lb.n_rows, "lower bound size incorrect");
		Assert(x0.n_rows == ub.n_rows, "upper bound size incorrect");


		Assert(arma::all(x0 >= lb),
				"seed is not feasible (violates lower bound)");
		Assert(arma::all(x0 <= ub),
				"seed is not feasible (violates upper bound)");

		const int DIM = x0.n_rows;

		xHistory.push_back(x0);

		arma::mat yHistory = arma::zeros(DIM, 0);
		arma::mat sHistory = arma::zeros(DIM, 0);

		arma::vec x = x0, g;
		int k = 0;

		double f = FunctionObjectiveOracle_(x);
		FunctionGradientOracle_(x, g);

		theta = 1.0;

		W = arma::zeros(DIM, 0);
		M = arma::zeros(0, 0);

		auto noConvergence =
				[&](arma::vec& x, arma::vec& g)->bool {
			    arma::vec clamped = x - g;
			    const auto too_big = arma::find(clamped > lb);
			    clamped(too_big) = lb(too_big);
					const auto too_small = arma::find(clamped < ub);
					clamped(too_small) = ub(too_small);
					return (arma::norm(clamped - x, "inf")>= Options_.tol);
				};

		while (noConvergence(x, g) && (k < Options_.maxIter)) {
			Debug("iteration "<<k)
			double f_old = f;
			arma::vec x_old = x;
			arma::vec g_old = g;

			// STEP 2: compute the cauchy point by algorithm CP
			arma::vec CauchyPoint = arma::zeros(DIM);
			arma::vec c = arma::zeros(DIM);
			GetGeneralizedCauchyPoint(x, g, CauchyPoint, c);
			// STEP 3: compute a search direction d_k by the primal method
			arma::vec SubspaceMin;
			SubspaceMinimization(CauchyPoint, x, c, g, SubspaceMin);

			arma::mat H;
			double Length = 0;

			// STEP 4: perform linesearch and STEP 5: compute gradient
			LineSearch(x, SubspaceMin - x, f, g, Length);

			xHistory.push_back(x);

			// prepare for next iteration
			arma::vec newY = g - g_old;
			arma::vec newS = x - x_old;

			// STEP 6:
			double test = arma::dot(newS, newY);
			test = (test < 0) ? -1.0 * test : test;

			if (test > EPS * arma::norm(newY, 2)) {
				if (k < Options_.m) {
					yHistory.resize(DIM, k + 1);
					sHistory.resize(DIM, k + 1);
				} else {

					yHistory.head_cols(Options_.m - 1) = yHistory.tail_cols(
							Options_.m - 1);
					sHistory.head_cols(Options_.m - 1) = sHistory.tail_cols(
							Options_.m - 1);
				}
				yHistory.tail_cols(1) = newY;
				sHistory.tail_cols(1) = newS;

				// STEP 7:
				theta = arma::dot(newY, newY)
						/ arma::dot(newY, newS);

				W = arma::zeros(yHistory.n_rows,
						yHistory.n_cols + sHistory.n_cols);

				W(0, 0, arma::size(yHistory)) = yHistory;

        W(0, yHistory.n_cols, arma::size(sHistory)) = theta * sHistory;

				arma::mat A = sHistory.t() * yHistory;
				arma::mat L = arma::trimatl(A);
				arma::mat MM(A.n_rows + L.n_rows, A.n_rows + L.n_cols);
				arma::mat D = -arma::diagmat(A);
				MM(0, 0, arma::size(D)) = D;
				MM(0, D.n_cols, arma::size(L.t())) = L.t();
				MM(D.n_rows, 0, arma::size(L)) = L;
				arma::mat bottom_right = (sHistory.t() * sHistory)
											 * theta;
				MM(D.n_rows, D.n_cols, arma::size(bottom_right)) = bottom_right;

				M = arma::inv(MM);
			}

			arma::vec ttt = arma::zeros(1);
			ttt(0) = f_old - f;
			Debug( "--> "<< arma::norm(ttt));
			if (arma::norm(ttt) < Options_.tol) {
				// successive function values too similar
				break;
			}
			k++;

		}

		XOpt = x;
		x0 = x;

	}
};

#endif /* LBFGSB_H_ */
