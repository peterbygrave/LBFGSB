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

Matrix A(2, 2);
Vector b(2);
Vector l(2), r(2);

void initConstants() {
	A << 3, 3.1, 3.1, 10;
	b << 1, 3;
	l << -INF, 0;
	r << INF, INF;
}

double Quadratic_ValueSimple(const Vector& x) {
	// return the value of the function
	return x.transpose() * A * x + b.dot(x);
}

void Quadratic_GradientSimple(const Vector& X, Vector& Y) {
	// return the gradient of the function (vector)
	Y = Vector::Zero(2, 1);
	Y = 2 * A * X + b;
}

int main() {
	// init predefined constants
	initConstants();
	// init solver with bounds
	LBFGSB MySolver(l, r);
	// create starting point
	Vector XOpt = Vector::Zero(2, 1);
	XOpt << 0.3, 0.3;
	// solve the problem
	MySolver.Solve(XOpt, Quadratic_ValueSimple, Quadratic_GradientSimple);
	// print the solution
	std::cout << MySolver.XOpt.transpose();
	std::cout << std::endl;

	return 0;
}
