#include <iostream>
#include <conio.h>

#include <boost/numeric/ublas/assignment.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>

/*
 *	boost ublas assignment sample code
 *	http://svn.boost.org/svn/boost/trunk/libs/numeric/ublas/doc/samples/assignment_examples.cpp
 *
 */

using namespace boost::numeric;

namespace bo = boost::numeric::ublas;
using FMatrix = bo::matrix < float > ;
using FVector = bo::vector < float > ;
using FRow = bo::matrix_row < FMatrix > ;

int main(int argc, char** argv)
{
	FMatrix mat(3, 3);
	
	mat <<=  1.0f, 4.0f, -3.0f,
			-2.0f, 8.0f,  5.0f,
			 3.0f, 4.0f,  7.0f;
	std::cout << mat << std::endl;
	
	auto ret = bo::lu_factorize(mat);
	std::cout << mat << std::endl;
	std::cout << "Sigular: " << ret << std::endl;

	printf("press any key to continue...");
	_getch();
	
	return 0;
}