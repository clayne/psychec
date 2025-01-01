// Copyright (c) 2021 Leandro T. C. Melo <ltcmelo@gmail.com>
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

#include "Symbol__IMPL__.inc"
#include "Symbol_Declaration.h"

using namespace psy;
using namespace C;

struct Declaration::DeclarationImpl : SymbolImpl
{
    DeclarationImpl(const Symbol* containingSym,
                    DeclarationKind declK,
                    const SyntaxTree* tree,
                    const Scope* enclosingScope,
                    NameSpace ns)
        : SymbolImpl(SymbolKind::Declaration, containingSym)
        , tree_(tree)
        , enclosingScope_(enclosingScope)
    {
        BF_.declK_ = static_cast<std::uint32_t>(declK);
        BF_.ns_ = static_cast<std::uint32_t>(ns);
    }

    const SyntaxTree* tree_;
    const Scope* enclosingScope_;
};