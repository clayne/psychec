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
#include "Binder__MACROS__.inc"

#include "SyntaxTree.h"

#include "binder/Scope.h"
#include "compilation/Compilation.h"
#include "symbols/Symbol_ALL.h"
#include "syntax/SyntaxFacts.h"
#include "syntax/Lexeme_ALL.h"
#include "syntax/SyntaxNodes.h"
#include "syntax/SyntaxUtilities.h"
#include "types/Type_ALL.h"

#include "../common/infra/Assertions.h"

#include <iostream>

using namespace psy;
using namespace C;

Binder::Binder(SemanticModel* semaModel, const SyntaxTree* tree)
    : SyntaxVisitor(tree)
    , semaModel_(semaModel)
    , stashedScope_(nullptr)
    , decltorIsOfTydef_(false)
    , diagReporter_(this)
{}

Binder::~Binder()
{
}

void Binder::bind()
{
    // Sentinels for the outermost scope and symbol.
    scopes_.push(nullptr);
    syms_.push(nullptr);

    visit(tree_->root());

    PSY_ASSERT_W_MSG(scopes_.size() == 1, return, "expected 1 scope");
    PSY_ASSERT_W_MSG(scopes_.top() == nullptr, return, "expected sentinel scope");
    PSY_ASSERT_W_MSG(syms_.size() == 1, return, "expected 1 symbol");
    PSY_ASSERT_W_MSG(syms_.top() == nullptr, return, "expected sentinel symbol");

    scopes_.pop();
    syms_.pop();
}

void Binder::openScope(ScopeKind scopeK)
{
    std::unique_ptr<Scope> scope(new Scope(scopeK));
    scopes_.push(scope.get());

    auto enclosingScope = scopes_.top();
    enclosingScope->enclose(std::move(scope));
}

void Binder::openStashedScope()
{
    PSY_ASSERT(stashedScope_, return);
    scopes_.push(stashedScope_);
}

void Binder::closeScope()
{
    scopes_.pop();
}

void Binder::closeAndStashScope()
{
    stashedScope_ = scopes_.top();
    scopes_.pop();
}

void Binder::pushSym(Symbol* sym)
{
    DEBUG_SYM_STACK(sym);
    syms_.push(sym);
}

Symbol* Binder::popSym()
{
    PSY_ASSERT(!syms_.empty(), return nullptr);
    DEBUG_SYM_STACK(syms_.top());
    auto sym = syms_.top();
    syms_.pop();
    return sym;
}

void Binder::pushTy(Type* ty)
{
    DEBUG_TY_STACK(ty);
    tys_.push(ty);
}

Type* Binder::popTy()
{
    PSY_ASSERT(!tys_.empty(), return nullptr);
    DEBUG_TY_STACK(tys_.top());
    auto ty = tys_.top();
    tys_.pop();
    return ty;
}

const Identifier* Binder::lexemeOrEmptyIdent(const SyntaxToken& tk) const
{
    return tk.lexeme()
            ? tk.lexeme()->asIdentifier()
            : tree_->findIdentifier("", 0);
}

//--------------//
// Declarations //
//--------------//

SyntaxVisitor::Action Binder::visitTranslationUnit(const TranslationUnitSyntax* node)
{
    std::unique_ptr<TranslationUnit> sym(
                new TranslationUnit(tree_,
                                    scopes_.top(),
                                    syms_.top()));
    auto rawSym = semaModel_->keepTranslationUnit(node, std::move(sym));
    pushSym(rawSym);
    openScope(ScopeKind::File);

    for (auto declIt = node->declarations(); declIt; declIt = declIt->next)
        visit(declIt->value);

    closeScope();
    popSym();

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitIncompleteDeclaration(const IncompleteDeclarationSyntax* node)
{
    diagReporter_.UselessDeclaration(node->lastToken());

    for (auto specIt = node->specifiers(); specIt; specIt = specIt->next)
        ;

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitStructOrUnionDeclaration(const StructOrUnionDeclarationSyntax* node)
{
    return visitStructOrUnionDeclaration_AtSpecifier(node);
}

SyntaxVisitor::Action Binder::visitStructOrUnionDeclaration_DONE(const StructOrUnionDeclarationSyntax* node)
{
    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitTypedefDeclaration(const TypedefDeclarationSyntax* node)
{
    return visitTypedefDeclaration_AtSpecifier(node);
}

SyntaxVisitor::Action Binder::visitTypedefDeclaration_DONE(const TypedefDeclarationSyntax* node)
{
    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitEnumDeclaration(const EnumDeclarationSyntax* node)
{
    return visitEnumDeclaration_AtSpecifier(node);
}

SyntaxVisitor::Action Binder::visitEnumDeclaration_DONE(const EnumDeclarationSyntax* node)
{
    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitVariableAndOrFunctionDeclaration(
        const VariableAndOrFunctionDeclarationSyntax* node)
{
    return visitVariableAndOrFunctionDeclaration_AtSpecifiers(node);
}

SyntaxVisitor::Action Binder::visitVariableAndOrFunctionDeclaration_DONE(
        const VariableAndOrFunctionDeclarationSyntax*)
{
    popTy();

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitFieldDeclaration(const FieldDeclarationSyntax* node)
{
    return visitFieldDeclaration_AtSpecifiers(node);
}

SyntaxVisitor::Action Binder::visitFieldDeclaration_DONE(const FieldDeclarationSyntax*)
{
    popTy();

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitEnumeratorDeclaration(const EnumeratorDeclarationSyntax* node)
{
    return visitEnumeratorDeclaration_AtImplicitSpecifier(node);
}

SyntaxVisitor::Action Binder::visitEnumeratorDeclaration_DONE(const EnumeratorDeclarationSyntax* node)
{
    popTy();

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitParameterDeclaration(const ParameterDeclarationSyntax* node)
{
    return visitParameterDeclaration_AtSpecifiers(node);
}

SyntaxVisitor::Action Binder::visitParameterDeclaration_DONE(const ParameterDeclarationSyntax*)
{
    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitStaticAssertDeclaration(const StaticAssertDeclarationSyntax*)
{
    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitFunctionDefinition(const FunctionDefinitionSyntax* node)
{
    return visitFunctionDefinition_AtSpecifiers(node);
}

SyntaxVisitor::Action Binder::visitFunctionDefinition_DONE(const FunctionDefinitionSyntax* node)
{
    return Action::Skip;
}

//------------//
// Statements //
//------------//

SyntaxVisitor::Action Binder::visitCompoundStatement(const CompoundStatementSyntax* node)
{
    openScope(ScopeKind::Block);

    for (auto stmtIt = node->statements(); stmtIt; stmtIt = stmtIt->next)
        visit(stmtIt->value);

    closeScope();

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitDeclarationStatement(const DeclarationStatementSyntax* node)
{
    visit(node->declaration());

    return Action::Skip;
}
