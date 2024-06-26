// Copyright (c) 2022 Leandro T. C. Melo <ltcmelo@gmail.com>
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

#ifndef PSYCHE_C_DISAMBIGUATOR_H__
#define PSYCHE_C_DISAMBIGUATOR_H__

#include "API.h"
#include "Fwds.h"

#include "parser/ParseOptions.h"

#include "../common/infra/AccessSpecifiers.h"

#include <cstdint>
#include <unordered_set>
#include <vector>

namespace psy {
namespace C {

class PSY_C_INTERNAL_API Reparser
{
PSY_INTERNAL:
    PSY_GRANT_INTERNAL_ACCESS(SyntaxTree);
    PSY_GRANT_INTERNAL_ACCESS(InternalsTestSuite);

    Reparser();

    enum class DisambiguationStrategy : std::uint8_t
    {
        TypeSynonymsVerification,
        SyntaxCorrelation,
        GuidelineImposition
    };

    void addDisambiguationStrategy(DisambiguationStrategy strategy);

    void reparse(SyntaxTree* tree);

    bool eliminatedAllAmbiguities() const;
    bool ambiguityPersists(const SyntaxNode* node) const;

private:
    std::vector<DisambiguationStrategy> strategies_;
    std::unordered_set<const SyntaxNode*> persistentAmbigs_;
};

} // C
} // psy

#endif
