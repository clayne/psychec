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

#include "Binder.h"

#include "SyntaxTree.h"

using namespace psy;
using namespace C;

/* Declarations */
const std::string Binder::DiagnosticsReporter::ID_of_UselessDeclaration = "Binder-000";

/* Declarators */
const std::string Binder::DiagnosticsReporter::ID_FunctionReturningFunction = "Binder-200-6.7.6.3-1-A";
const std::string Binder::DiagnosticsReporter::ID_FunctionReturningArray = "Binder-200-6.7.6.3-1-B";

/* Type specifiers */
const std::string Binder::DiagnosticsReporter::ID_TypeSpecifierMissingDefaultsToInt = "Binder-100-6.7.2-2-A";
const std::string Binder::DiagnosticsReporter::ID_InvalidType = "Binder-100-6.7.2-2-B";

/* Type qualifiers */
const std::string Binder::DiagnosticsReporter::ID_InvalidUseOfRestrict = "Binder-300-6.7.3-2";

void Binder::DiagnosticsReporter::diagnose(DiagnosticDescriptor&& desc, SyntaxToken tk)
{
    binder_->tree_->newDiagnostic(desc, tk);
};

void Binder::DiagnosticsReporter::UselessDeclaration(SyntaxToken tk)
{
    diagnose(DiagnosticDescriptor(
                 ID_of_UselessDeclaration,
                 "[[useless declaration]]",
                 "declaration does not declare anything",
                 DiagnosticSeverity::Error,
                 DiagnosticCategory::Binding),
             tk);
}

void Binder::DiagnosticsReporter::FunctionReturningFunction(SyntaxToken decltorTk)
{
    auto s = "`"
            + decltorTk.valueText()
            + "' declared as function returning a function";

    diagnose(DiagnosticDescriptor(
                 ID_FunctionReturningFunction,
                 "[[function returning function]]",
                 s,
                 DiagnosticSeverity::Error,
                 DiagnosticCategory::Binding),
             decltorTk);
}

void Binder::DiagnosticsReporter::FunctionReturningArray(SyntaxToken decltorTk)
{
    auto s = "`"
            + decltorTk.valueText()
            + "' declared as function returning an array";

    diagnose(DiagnosticDescriptor(
                 ID_FunctionReturningArray,
                 "[[function returning array]]",
                 s,
                 DiagnosticSeverity::Error,
                 DiagnosticCategory::Binding),
             decltorTk);
}

void Binder::DiagnosticsReporter::TypeSpecifierMissingDefaultsToInt(SyntaxToken declTk)
{
    diagnose(DiagnosticDescriptor(
                 ID_TypeSpecifierMissingDefaultsToInt,
                 "[[type specifier missing]]",
                 "type specifier missing, defaults to `int'",
                 DiagnosticSeverity::Error,
                 DiagnosticCategory::Binding),
             declTk);
}

void Binder::DiagnosticsReporter::InvalidType(SyntaxToken tySpecTk)
{
    diagnose(DiagnosticDescriptor(
                               ID_InvalidType,
                               "[[invaid type]]",
                               "invalid type",
                               DiagnosticSeverity::Error,
                               DiagnosticCategory::Binding),
                           tySpecTk);
}

void Binder::DiagnosticsReporter::InvalidUseOfRestrict(SyntaxToken tyQualTk)
{
    diagnose(DiagnosticDescriptor(
                 ID_InvalidUseOfRestrict,
                 "[[invalid use of restrict]]",
                 "invalid use of `restrict'",
                 DiagnosticSeverity::Error,
                 DiagnosticCategory::Binding),
             tyQualTk);
}
