// Copyright 2010 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <limits.h>
#include <math.h>

#include "double-conversion.h"

#include "bignum-dtoa.h"
#include "fast-dtoa.h"
#include "ieee.h"
#include "utils.h"
#include <stdexcept>

namespace double_conversion
{

const DoubleToStringConverter& DoubleToStringConverter::EcmaScriptConverter()
{
  int flags = UNIQUE_ZERO | EMIT_POSITIVE_EXPONENT_SIGN;
  static DoubleToStringConverter converter(flags,
                                           "Infinity",
                                           "NaN",
                                           'e',
                                           -6, 21,
                                           6, 0);
  return converter;
}


bool DoubleToStringConverter::HandleSpecialValues(double value, StringBuilder & result_builder) const
{
	Double double_inspect(value);
	if (double_inspect.IsInfinite())
	{
		if (infinity_symbol_ == NULL) return false;
		if (value < 0)
		{
			result_builder.AddCharacter('-');
		}
		result_builder.AddString(infinity_symbol_);
		return true;
	}
	if (double_inspect.IsNan())
	{
		if (nan_symbol_ == NULL) return false;
		result_builder.AddString(nan_symbol_);
		return true;
	}
	return false;
}


void DoubleToStringConverter::CreateExponentialRepresentation(
		const char* decimal_digits,
		int length,
		int exponent,
		StringBuilder* result_builder) const {
	DOUBLE_CONVERSION_ASSERT(length != 0);
	result_builder->AddCharacter(decimal_digits[0]);
	if (length != 1) {
		result_builder->AddCharacter('.');
		result_builder->AddSubstring(&decimal_digits[1], length-1);
	}
	result_builder->AddCharacter(exponent_character_);
	if (exponent < 0) {
		result_builder->AddCharacter('-');
		exponent = -exponent;
	} else {
		if ((flags_ & EMIT_POSITIVE_EXPONENT_SIGN) != 0) {
			result_builder->AddCharacter('+');
		}
	}
	if (exponent == 0) {
		result_builder->AddCharacter('0');
		return;
	}
	DOUBLE_CONVERSION_ASSERT(exponent < 1e4);
	const int kMaxExponentLength = 5;
	char buffer[kMaxExponentLength + 1];
	buffer[kMaxExponentLength] = '\0';
	int first_char_pos = kMaxExponentLength;
	while (exponent > 0)
	{
		buffer[--first_char_pos] = '0' + (exponent % 10);
		exponent /= 10;
	}
	result_builder->AddSubstring(&buffer[first_char_pos], kMaxExponentLength - first_char_pos);
}


void DoubleToStringConverter::CreateDecimalRepresentation(
    const char* decimal_digits,
    int length,
    int decimal_point,
    int digits_after_point,
    StringBuilder* result_builder) const {
  // Create a representation that is padded with zeros if needed.
  if (decimal_point <= 0) {
      // "0.00000decimal_rep".
    result_builder->AddCharacter('0');
    if (digits_after_point > 0) {
      result_builder->AddCharacter('.');
      result_builder->AddPadding('0', -decimal_point);
	  DOUBLE_CONVERSION_ASSERT(length <= digits_after_point - (-decimal_point));
      result_builder->AddSubstring(decimal_digits, length);
      int remaining_digits = digits_after_point - (-decimal_point) - length;
      result_builder->AddPadding('0', remaining_digits);
    }
  } else if (decimal_point >= length) {
    // "decimal_rep0000.00000" or "decimal_rep.0000"
    result_builder->AddSubstring(decimal_digits, length);
    result_builder->AddPadding('0', decimal_point - length);
    if (digits_after_point > 0) {
      result_builder->AddCharacter('.');
      result_builder->AddPadding('0', digits_after_point);
    }
  } else {
    // "decima.l_rep000"
	DOUBLE_CONVERSION_ASSERT(digits_after_point > 0);
    result_builder->AddSubstring(decimal_digits, decimal_point);
    result_builder->AddCharacter('.');
	DOUBLE_CONVERSION_ASSERT(length - decimal_point <= digits_after_point);
    result_builder->AddSubstring(&decimal_digits[decimal_point],
                                 length - decimal_point);
    int remaining_digits = digits_after_point - (length - decimal_point);
    result_builder->AddPadding('0', remaining_digits);
  }
  if (digits_after_point == 0) {
    if ((flags_ & EMIT_TRAILING_DECIMAL_POINT) != 0) {
      result_builder->AddCharacter('.');
    }
    if ((flags_ & EMIT_TRAILING_ZERO_AFTER_POINT) != 0) {
      result_builder->AddCharacter('0');
    }
  }
}


bool DoubleToStringConverter::ToShortestIeeeNumber(double value, StringBuilder & result_builder, DoubleToStringConverter::DtoaMode mode) const
{
	DOUBLE_CONVERSION_ASSERT(mode == SHORTEST || mode == SHORTEST_SINGLE);
	if (Double(value).IsSpecial())
	{
		return HandleSpecialValues(value, result_builder);
	}

	const int kDecimalRepCapacity = kBase10MaximalLength + 1;
	char decimal_rep[kDecimalRepCapacity];

	DoubleToAsciiResult result = DoubleToAscii(value, mode, 0, decimal_rep, kDecimalRepCapacity);

	bool unique_zero = (flags_ & UNIQUE_ZERO) != 0;
	if (result.sign && (value != 0.0 || !unique_zero))
	{
		result_builder.AddCharacter('-');
	}

	int exponent = result.decimal_point - 1;
	if ((decimal_in_shortest_low_ <= exponent) && (exponent < decimal_in_shortest_high_))
	{
		CreateDecimalRepresentation(decimal_rep, result.length, result.decimal_point, Max(0, result.length - result.decimal_point), &result_builder);
	}
	else
	{
		CreateExponentialRepresentation(decimal_rep, result.length, exponent, &result_builder);
	}
	return true;
}


bool DoubleToStringConverter::ToExponential(double value, int requested_digits, StringBuilder & result_builder) const
{
	if (Double(value).IsSpecial())
	{
		return HandleSpecialValues(value, result_builder);
	}

	if (requested_digits < -1) return false;
	if (requested_digits > kMaxExponentialDigits) return false;

	// Add space for digit before the decimal point and the '\0' character.
	const int kDecimalRepCapacity = kMaxExponentialDigits + 2;
	DOUBLE_CONVERSION_ASSERT(kDecimalRepCapacity > kBase10MaximalLength);
	char decimal_rep[kDecimalRepCapacity];

	DoubleToAsciiResult result;
	if (requested_digits == -1)
	{
		result = DoubleToAscii(value, SHORTEST, 0, decimal_rep, kDecimalRepCapacity);
	}
	else
	{
		result = DoubleToAscii(value, PRECISION, requested_digits + 1, decimal_rep, kDecimalRepCapacity);
		DOUBLE_CONVERSION_ASSERT(result.length <= requested_digits + 1);

		for (int i = result.length; i < requested_digits + 1; ++i)
		{
			decimal_rep[i] = '0';
		}
		result.length = requested_digits + 1;
	}

	bool unique_zero = ((flags_ & UNIQUE_ZERO) != 0);
	if (result.sign && (value != 0.0 || !unique_zero))
	{
		result_builder.AddCharacter('-');
	}

	int exponent = result.decimal_point - 1;
	CreateExponentialRepresentation(decimal_rep, result.length, exponent, &result_builder);
	return true;
}


bool DoubleToStringConverter::ToPrecision(double value, int precision, StringBuilder & result_builder) const
{
	if (Double(value).IsSpecial())
	{
	return HandleSpecialValues(value, result_builder);
	}

	if (precision < kMinPrecisionDigits || precision > kMaxPrecisionDigits)
	{
	return false;
	}

	// Find a sufficiently precise decimal representation of n.
	// Add one for the terminating null character.
	const int kDecimalRepCapacity = kMaxPrecisionDigits + 1;
	char decimal_rep[kDecimalRepCapacity];

	DoubleToAsciiResult result = DoubleToAscii(value, PRECISION, precision, decimal_rep, kDecimalRepCapacity);
	DOUBLE_CONVERSION_ASSERT(result.length <= precision);

	bool unique_zero = ((flags_ & UNIQUE_ZERO) != 0);
	if (result.sign && (value != 0.0 || !unique_zero))
	{
		result_builder.AddCharacter('-');
	}

	// The exponent if we print the number as x.xxeyyy. That is with the
	// decimal point after the first digit.
	int exponent = result.decimal_point - 1;

	int extra_zero = ((flags_ & EMIT_TRAILING_ZERO_AFTER_POINT) != 0) ? 1 : 0;
	if ((-result.decimal_point + 1 > max_leading_padding_zeroes_in_precision_mode_) ||
			(result.decimal_point - precision + extra_zero >
			max_trailing_padding_zeroes_in_precision_mode_))
	{
		// Fill buffer to contain 'precision' digits.
		// Usually the buffer is already at the correct length, but 'DoubleToAscii'
		// is allowed to return less characters.
		for (int i = result.length; i < precision; ++i)
		{
			decimal_rep[i] = '0';
		}

		CreateExponentialRepresentation(decimal_rep, precision, exponent, &result_builder);
	}
	else
	{
		CreateDecimalRepresentation(decimal_rep, result.length, result.decimal_point, Max(0, precision - result.decimal_point), &result_builder);
	}
	return true;
}


static BignumDtoaMode DtoaToBignumDtoaMode(DoubleToStringConverter::DtoaMode dtoa_mode)
{
	switch (dtoa_mode)
	{
	case DoubleToStringConverter::SHORTEST:  return BIGNUM_DTOA_SHORTEST;
	case DoubleToStringConverter::SHORTEST_SINGLE: return BIGNUM_DTOA_SHORTEST_SINGLE;
	case DoubleToStringConverter::PRECISION: return BIGNUM_DTOA_PRECISION;
	}
	throw std::logic_error("Missing case in the switch statement above");
}


DoubleToStringConverter::DoubleToAsciiResult DoubleToStringConverter::DoubleToAscii(double v, DtoaMode mode, int requested_digits, char * buffer, int buffer_length)
{
	Vector<char> vector(buffer, buffer_length);
	DOUBLE_CONVERSION_ASSERT(!Double(v).IsSpecial());
	DOUBLE_CONVERSION_ASSERT(mode == SHORTEST || mode == SHORTEST_SINGLE || requested_digits >= 0);

	bool sign;

	if (Double(v).Sign() < 0)
	{
		sign = true;
		v = -v;
	}
	else sign = false;

	if (mode == PRECISION && requested_digits == 0)
	{
		vector[0] = '\0';
		return { sign, 0, 0 };
	}

	if (v == 0)
	{
		vector[0] = '0';
		vector[1] = '\0';
		return { sign, 1, 1 };
	}

	FastDtoaResult fast_result;
	switch (mode)
	{
	case SHORTEST:
		fast_result = FastDtoa(v, FAST_DTOA_SHORTEST, 0, vector);
		break;
	case SHORTEST_SINGLE:
		fast_result = FastDtoa(v, FAST_DTOA_SHORTEST_SINGLE, 0, vector);
		break;
	case PRECISION:
		fast_result = FastDtoa(v, FAST_DTOA_PRECISION, requested_digits, vector);
		break;
	}
	if (fast_result.succeeded) return { sign, fast_result.length, fast_result.decimal_point };

	// If the fast dtoa didn't succeed use the slower bignum version.
	BignumDtoaMode bignum_mode = DtoaToBignumDtoaMode(mode);
	int length = 0;
	int point = 0;
	BignumDtoa(v, bignum_mode, requested_digits, vector, &length, &point);
	vector[length] = '\0';
	return { sign, length, point };
}


// Consumes the given substring from the iterator.
// Returns false, if the substring does not match.
template <class Iterator>
static bool ConsumeSubString(Iterator* current,
                             Iterator end,
                             const char* substring) {
  DOUBLE_CONVERSION_ASSERT(**current == *substring);
  for (substring++; *substring != '\0'; substring++) {
    ++*current;
    if (*current == end || **current != *substring) return false;
  }
  ++*current;
  return true;
}

}  // namespace double_conversion
