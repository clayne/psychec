// Copyright (c) 2021/22/23/24 Leandro T. C. Melo <ltcmelo@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef PSYCHE_C_TYPE_KIND_H__
#define PSYCHE_C_TYPE_KIND_H__

#include "API.h"
#include "Fwds.h"

#include "../common/infra/Assertions.h"

#include <cstdint>
#include <string>

/**
 * \brief The TypeKind enum.
 *
 * \note
 * This API is inspired by that of \c Microsoft.CodeAnalysis.TypeKind
 * from Roslyn, the .NET Compiler Platform.
 */
enum class PSY_C_API TypeKind : std::uint8_t
{
    Array,
    Basic,
    Function,
    Pointer,
    Typedef,
    Tag,
    Void,
    Qualified
};

inline PSY_C_API std::string to_string(TypeKind tyK)
{
    switch (tyK) {
        case TypeKind::Array:
            return "Array";
        case TypeKind::Basic:
            return "Basic";
        case TypeKind::Function:
            return "Function";
        case TypeKind::Pointer:
            return "Pointer";
        case TypeKind::Typedef:
            return "Typedef";
        case TypeKind::Tag:
            return "Tag";
        case TypeKind::Void:
            return "Void";
        case TypeKind::Qualified:
            return "Qualified";
    }
    PSY_ASSERT_1(false);
    return "<invalid TypeKind>";
}

#endif
