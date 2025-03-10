// Copyright (c) 2024 Leandro T. C. Melo <ltcmelo@gmail.com>
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

#include "Type__IMPL__.inc"
#include "Type_Qualified.h"

#include <iostream>
#include <sstream>

using namespace psy;
using namespace C;

QualifiedType::Qualifiers::Qualifiers()
    : BD_(0)
{
}

struct QualifiedType::QualifiedTypeImpl : TypeImpl
{
    QualifiedTypeImpl(const Type* unqualTy)
        : TypeImpl(TypeKind::Qualified)
        , unqualTy_(unqualTy)
    {}

    const Type* unqualTy_;
    Qualifiers qualifiers_;
};

QualifiedType::QualifiedType(const Type* unqualTy)
    : Type(new QualifiedTypeImpl(unqualTy))
{}

const Type* QualifiedType::unqualifiedType() const
{
    return P_CAST->unqualTy_;
}

void QualifiedType::resetUnqualifiedType(const Type* unqualTy) const
{
    P_CAST->unqualTy_ = unqualTy;
}

const QualifiedType::Qualifiers QualifiedType::qualifiers() const
{
    return P_CAST->qualifiers_;
}

bool QualifiedType::Qualifiers::hasConst() const
{
    return F_.const_;
}

bool QualifiedType::Qualifiers::hasVolatile() const
{
    return F_.volatile_;
}

bool QualifiedType::Qualifiers::hasRestrict() const
{
    return F_.restrict_;
}

bool QualifiedType::Qualifiers::hasAtomic() const
{
    return F_.atomic_;
}

bool QualifiedType::Qualifiers::operator==(const QualifiedType::Qualifiers& other) const
{
    return BD_ == other.BD_;
}

bool QualifiedType::Qualifiers::operator!=(const QualifiedType::Qualifiers& other) const
{
    return !(BD_ == other.BD_);
}

void QualifiedType::qualifyWithConst()
{
    P_CAST->qualifiers_.F_.const_ = 1;
}

void QualifiedType::qualifyWithVolatile()
{
    P_CAST->qualifiers_.F_.volatile_ = 1;
}

void QualifiedType::qualifyWithRestrict()
{
    P_CAST->qualifiers_.F_.restrict_ = 1;
}

void QualifiedType::qualifyWithAtomic()
{
    P_CAST->qualifiers_.F_.atomic_ = 1;
}

namespace psy {
namespace C {

PSY_C_API std::ostream& operator<<(std::ostream& os, const QualifiedType* qualTy)
{
    if (!qualTy)
        return os << "<QualifiedType is null>";
    os << "<QualifiedType | ";
    os << "unqualified-type:" << qualTy->unqualifiedType();
    os << ">";
    return os;
}

} // C
} // psy
