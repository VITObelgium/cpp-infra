/*
 * DateTimeTools.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef DATETIMETOOLS_H_
#define DATETIMETOOLS_H_

#include "../DateTime.h"
#include "../Exceptions.h"
#include <vector>
#include "StringTools.h"
#include <stdlib.h>

namespace OPAQ {

class DateTimeTools {
public:
	DateTimeTools();
	virtual ~DateTimeTools();

	static const int FIELD_SECOND = 0;
	static const int FIELD_MINUTE = 1;
	static const int FIELD_HOUR = 2;
	static const int FIELD_DAY = 3;

	static DateTime ceil(const DateTime & datetime, int field = FIELD_DAY);

	static DateTime floor(const DateTime & datetime, int field = FIELD_DAY);

	static DateTime parseDateTime(const std::string & str) throw (ParseException);

	static DateTime parseDate (const std::string & str) throw (ParseException);
};

} /* namespace OPAQ */
#endif /* DATETIMETOOLS_H_ */
