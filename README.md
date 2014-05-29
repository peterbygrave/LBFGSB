L-BFGS-B (C++11 only, Eigen, OpenMp)
========

This c++11-only implementation of the 

*Limited-memory Broyden–Fletcher–Goldfarb–Shanno algorithm with bounds*

is based on the paper

**A LIMITED MEMORY ALGORITHM FOR BOUND CONSTRAINED OPTIMIZATION**

(Byrd, Richard H. and Lu, Peihuang and Nocedal, Jorge and Zhu, Ciyou)

This implementation is **not** a converted FORTRAN-implementation, **neither** and interface for the FORTRAN-Routines. It was build from the ground up in C++11 and only needs the [Eigenlibrary](http://eigen.tuxfamily.org/) for the matrix operations.

###Compiling under linux


To compile this with g++ make sure that you set the compiler flags:

```g++ -O3 -Wall -std=c++11 -fopenmp```

### Usage
You only have to declare two functions for the function value and the gradient:
```
double functionValue(const Vector& x);
void functionGradient(const Vector& x, Vector& grad);
```
To solve the problem
you can use
```
MySolver.Solve(x,functionValue,functionGradient);
```
See the file *main.cpp* for a full working example with bound. Unused bounds can be defined as ```INF```.

###License
```
Copyright (c) 2014 Patrick Wieschollek <patrick@wieschollek.info>
All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
