// Copyright (c) 2021/22 Leandro T. C. Melo <ltcmelo@gmail.com>
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

#ifndef PSYCHE_C_TYPEDEF_H__
#define PSYCHE_C_TYPEDEF_H__

#include "Declaration_Type.h"

#include "MIXIN_TypeableSymbol.h"

namespace psy {
namespace C {

class PSY_C_API Typedef final
        : public TypeDeclaration
        , public MIXIN_TypeableSymbol
{
public:
    //!@{
    /**
     * Cast \c this TypeDeclaration as a Typedef.
     */
    virtual Typedef* asTypedef() override { return this; }
    virtual const Typedef* asTypedef() const override { return this; }
    //!@}

    /**
     * Compute a displayable string for \c this Typedef.
     */
    virtual std::string toDisplayString() const override;

    /**
     * The Identifier with which \c this Typedef is declared.
     */
    virtual const Identifier* identifier() const override;

    /**
     * The TypedefType defined by \c this Typedef.
     */
    const TypedefType * definedType() const;

    /**
     * The Type synonymized by \c this Typedef.
     */
    const Type* synonymizedType() const;

PSY_INTERNAL:
    PSY_GRANT_INTERNAL_ACCESS(Binder);
    PSY_GRANT_INTERNAL_ACCESS(TypeResolver);

    Typedef(const Symbol* containingSym,
            const SyntaxTree* tree,
            const Scope* enclosingScope,
            TypedefType* tydefTy,
            const Type* synonymizedTy);

    Typedef(const Symbol* containingSym,
            const SyntaxTree* tree,
            const Scope* enclosingScope,
            TypedefType* tydefTy);

    virtual void setType(const Type* ty) override;
    virtual const Type* retypeableType() const override;

private:
    const Type* synonymizedTy_;
};

PSY_C_API std::string to_string(const Typedef* tydef);

} // C
} // psy

#endif
