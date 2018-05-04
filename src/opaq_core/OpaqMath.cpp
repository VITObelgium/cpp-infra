/*
 * Math.cpp
 *
 *  Created on: Dec 21, 2015
 *      Author: bino
 */
#include <cmath>
#include "OpaqMath.h"
#include "Exceptions.h"

namespace opaq {

Math::Math() {

}

Math::~Math() {
}

void Math::winddir( double *x_vec, double *y_vec, const std::vector<double>& wdir, double missing, bool *ok ) {

	if ( ! x_vec || ! y_vec ||  ! ok ) throw NullPointerException( "invalid pointer in Math::winddir" );
	*ok    = false;
	*x_vec = 0;
	*y_vec = 0;

	int n = 0;

	for ( auto w : wdir ) {
		if ( fabs( w - missing ) > 1.e-6 ) {
			*x_vec += cos( 2.*Pi*w/360.);
			*y_vec += sin( 2.*Pi*w/360.);
			n++;
		}
	}

	if ( ! n ) return;
	*ok = true;

	double norm = sqrt( *x_vec * *x_vec + *y_vec * *y_vec );

	*x_vec /= norm;
	*y_vec /= norm;

	return;
}


} /* namespace OPAQ */
