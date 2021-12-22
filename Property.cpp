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
 *    @file Property.cpp
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief provides a storage for name/value property pairs
 *
 */

#include "Property.h"

namespace entropyservice {

/**
 * Constructor
 *
 * @param propName
 * @param propValue
 *
 */
Property::Property(const std::string &propName, const std::string &propValue) {
	this->propName = propName;
	this->propValue = propValue;
}

/**
 * @return property name
 */
std::string Property::getName() const {
	return propName;
}

/**
 * @return property value as string
 */
std::string Property::getStringValue() const {
	return propValue;
}

/**
 * @return true if property value is provided
 */
bool Property::isProvided() const {
	return propValue.size() > 0;
}

/**
 * @return true if property value is an integer
 */
bool Property::isInteger() {
	return !propValue.empty() && propValue.find_first_not_of("-0123456789") == std::string::npos;
}

/**
 * @return property value as integer
 */
int Property::getIntValue() {
	return atoi(propValue.c_str());
}

/**
 * @return property value as boolean
 */
bool Property::getBoolValue() {
	if (propValue.size() == 0) {
		return false;
	}
	if (propValue.compare("true") == 0) {
		return true;
	} else {
		return false;
	}
}

/**
 * @return true if property value is a boolean
 */
bool Property::isBoolean() {
	if (propValue.compare("true") == 0 || propValue.compare("false") == 0) {
		return true;
	} else {
		return false;
	}
}

} /* namespace entropyservice */
