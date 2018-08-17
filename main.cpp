/**
 *           c++11-only implementation of the L-BFGS-B algorithm
 *
 * Copyright (c) 2014 Patrick Wieschollek
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

#include <iostream>
#include "meta.h"
#include "lbfgsb.hpp"

/* this code will solve the optimization problem:
 *
 *
 * min <x,A*x> + <b,x>
 * s.t.  l <=  x <= r
 *
 */

arma::mat A(2, 2);
arma::vec b(2);
arma::vec l(2), r(2);

void initConstants() {
	A = {{3, 3.1}, {3.1, 10}};
	b = {1, 3};
	l = {-arma::datum::inf, 0};
	r = {arma::datum::inf, arma::datum::inf};
}

double Quadratic_ValueSimple(const arma::vec& x) {
	// return the value of the function
	return arma::dot(x.t(),  A * x) + arma::dot(b, x);
}

void Quadratic_GradientSimple(const arma::vec& X, arma::vec& Y) {
	// return the gradient of the function (vector)
	Y = arma::zeros(2, 1);
	Y = 2 * A * X + b;
}

int main() {
	// init predefined constants
	initConstants();
	// init solver with bounds
	LBFGSB MySolver(l, r);
	// create starting point
	arma::vec XOpt = {0.3, 0.3};
	// solve the problem
	MySolver.Solve(XOpt, Quadratic_ValueSimple, Quadratic_GradientSimple);
	// print the solution
	std::cout << MySolver.XOpt.t();
	std::cout << std::endl;

	return 0;
}
