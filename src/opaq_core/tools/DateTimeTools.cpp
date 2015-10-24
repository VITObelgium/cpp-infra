/*
 * DateTimeTools.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "DateTimeTools.h"

namespace OPAQ {

DateTimeTools::DateTimeTools() {}

DateTimeTools::~DateTimeTools() {}

DateTime DateTimeTools::ceil (const DateTime & datetime, int field) {
	DateTime out = datetime;
	if (field == FIELD_SECOND) return out;
	if (out.getSec() != 0) out.addSeconds(60 - out.getSec());
	if (field == FIELD_MINUTE) return out;
	if (out.getMin() != 0) out.addMinutes(60 - out.getMin());
	if (field == FIELD_HOUR) return out;
	if (out.getHour() != 0) out.addHours(24 - out.getHour());
	return out;
}

DateTime DateTimeTools::floor(const DateTime & datetime, int field) {
	DateTime out = datetime;
	if (field == FIELD_SECOND) return out;
	out.setSec(0);
	if (field == FIELD_MINUTE) return out;
	out.setMin(0);
	if (field == FIELD_HOUR) return out;
	out.setHour(0);
	return out;
}

DateTime DateTimeTools::parseDateTime(const std::string & str) throw (ParseException) {
	std::vector<std::string> tokens = StringTools::tokenize(str);
	if (tokens.size() != 2)
		throw ParseException("failed to tokenize date and time part");
	DateTime out = parseDate(tokens[0]);
	std::vector<std::string> timeTokens = StringTools::tokenize(tokens[1], ":",
			1, false);
	if (timeTokens.size() != 3)
		throw ParseException("failed to tokenize time part");
	out.setHour(atoi(timeTokens[0].c_str()));
	out.setMin(atoi(timeTokens[1].c_str()));
	out.setSec(atoi(timeTokens[2].c_str()));
	return out;
}

DateTime DateTimeTools::parseDate (const std::string & str) throw (ParseException) {
	std::vector<std::string> tokens = StringTools::tokenize(str, "-", 1, false);
	if (tokens.size() != 3)
		throw ParseException("failed to tokenize date part");
	int year = atoi(tokens[0].c_str());
	int month = atoi(tokens[1].c_str());
	int day = atoi(tokens[2].c_str());
	return DateTime(year, month, day);
}

} /* namespace OPAQ */
