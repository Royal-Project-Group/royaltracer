/*
 * Source: https://github.com/matheusgomes28/base64pp
MIT License

Copyright (c) 2022 - Matheus Gomes

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
 */

//! @file base64pp.h
//! @brief This file defines the public functions
//! for the base64pp library
#ifndef BASE64PP_H
#define BASE64PP_H

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

//! base64pp API namespace
namespace base64pp
{

    //! @brief This function will encode a blob of data into a base64
    //! string.
    //! @param input - a span pointing to a binary blob to encode.
    //! @return a base64 string containing the encoded data.
    std::string encode(std::span<std::uint8_t const> input);

    //! @brief Overload of the encode function for string_view. This converts
    //! the string input to a span and calls the conventional `encode`.
    //! @param input - a string_view to be encoded into base64.
    //! @return a base64 string containing the encoded string.
    std::string encode_str(std::string_view input);

    //! @brief Decodes a base64 encoded string, returning an optional
    //! blob. If the decoding fails, it returns std::nullopt
    //! @param encoded_str - the base64 encoded string
    //! @return an optional containing a valid blob of data, if
    //! decoding was successful. Otherwise, returns std::nullopt
    //! @note this function accepts unpadded strings, if they are valid
    //! otherwise. It rejects odd-sized unpadded strings.
    std::optional<std::vector<std::uint8_t>> decode(std::string_view encoded_str);
} // namespace base64pp

#endif // BASE64PP_H