/*
 * Math.h
 *
 *  Created on: Dec 21, 2015
 *      Author: bino
 */

#ifndef SRC_OPAQ_CORE_MATH_H_
#define SRC_OPAQ_CORE_MATH_H_

#include <vector>

namespace OPAQ {

class Math {
public:
	Math();
	virtual ~Math();

	/**
	 * Guess...
	 */
	constexpr static double Pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286209;

	/**
	 * Computes the average wind direction vector (normalised) for the given wdir's (in degrees). The routine
	 * \param *x_vec  points to the x component to be returned
	 * \param *x_vec  points to the y component to be returned
	 * \param wdir    is the vector of wind directions
	 * \param missing is a missing value which can occurr in the vector of wind directions
	 * \param ok      returns true if succesfull, false if e.g. nothing was found
	 */
	static void winddir( double *x_vec, double *y_vec, const std::vector<double>& wdir, double missing, bool *ok );
};

} /* namespace OPAQ */

#endif /* SRC_OPAQ_CORE_MATH_H_ */
