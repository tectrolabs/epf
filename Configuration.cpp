/**
 *   Copyright (c) 2018 TectroLabs L.L.C.
 *
 *    Permission is hereby granted, free of charge, to any person obtaining
 *    a copy of this software and associated documentation files (the "Software"),
 *    to deal in the Software without restriction, including without limitation
 *    the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *    and/or sell copies of the Software, and to permit persons to whom the Software
 *    is furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 *    OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 *    @file Configuration.cpp
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief retrieves application configuration properties from a file
 */

#include "Configuration.h"

namespace entropyservice {

/**
 * Load properties from a configuration file
 *
 * @param filePathName complete path and name of the configuration file
 * @return true when configuration file loaded successfully
 *
 */
bool Configuration::loadFromFile(char *filePathName) {
	std::ifstream in(filePathName);

	if (!in.is_open()) {
		return false;
	}

	propertyMap.clear();
	parseLines(in);

	in.close();
	return true;
}

/**
 * Parse one line and extract the property name and value. Ignore empty or commented lines
 *
 * @param &lineStream to parse
 *
 */
void Configuration::parseLines(std::istream& lineStream) {

	std::string s, key, value;

	while (std::getline(lineStream, s)) {
		std::string::size_type begin = s.find_first_not_of(" \f\t\v");

		if (begin == std::string::npos)
			continue;

		if (std::string("#;").find(s[begin]) != std::string::npos)
			continue;

		std::string::size_type end = s.find('=', begin);
		key = s.substr(begin, end - begin);

		key.erase(key.find_last_not_of(" \f\t\v\n\r") + 1);

		if (key.empty())
			continue;

		if (end < s.size() - 1) {
			begin = s.find_first_not_of(" \f\n\r\t\v", end + 1);
			end = s.find_last_not_of(" \f\n\r\t\v") + 1;
			value = s.substr(begin, end - begin);
			propertyMap[key] = value;
		}
	}

}

/**
 * Retrieve a property value by property name
 *
 * @param propName name of the property
 * @return Property instance
 *
 */
Property Configuration::getProperty(std::string propName) {
	std::string propValue = propertyMap[propName];
	return Property(propName, propValue);
}

/**
 * Check to see if the property has been declared
 *
 * @param propName name of the property
 * @return true if property declared
 *
 */
bool Configuration::isPropertyDeclared(std::string propName) {
	return propertyMap.count(propName) > 0;
}

} /* namespace entropyservice */
