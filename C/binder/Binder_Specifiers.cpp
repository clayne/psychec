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
#include "compilation/SemanticModel.h"
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

template <class TyDeclT>
SyntaxVisitor::Action Binder::visitTypeDeclaration_AtInternalDeclarations_COMMON(
        const TyDeclT* node,
        Action (Binder::*visit_AtEnd)(const TyDeclT*))
{
    for (auto declIt = node->typeSpecifier()->declarations(); declIt; declIt = declIt->next)
        visit(declIt->value);
    return ((this)->*(visit_AtEnd))(node);
}

SyntaxVisitor::Action Binder::visitStructOrUnionDeclaration_AtSpecifier(
        const StructOrUnionDeclarationSyntax* node)
{
    const TagTypeSpecifierSyntax* tySpec = node->typeSpecifier();
    switch (tySpec->kind()) {
        case SyntaxKind::StructTypeSpecifier: {
            auto tagTy = makeTy<TagType>(
                        TagTypeKind::Struct,
                        lexemeOrEmptyIdent(tySpec->tagToken()));
            makeBindAndPushSym<Struct>(node, tagTy);
            break;
        }
        case SyntaxKind::UnionTypeSpecifier:{
            auto tagTy = makeTy<TagType>(
                        TagTypeKind::Union,
                        lexemeOrEmptyIdent(tySpec->tagToken()));
            makeBindAndPushSym<Union>(node, tagTy);
            break;
        }
        default:
            PSY_ASSERT(false, return Action::Quit);
    }
    return visitTypeDeclaration_AtInternalDeclarations_COMMON(
                node,
                &Binder::visitStructOrUnionDeclaration_AtEnd);
}

SyntaxVisitor::Action Binder::visitEnumDeclaration_AtSpecifier(const EnumDeclarationSyntax* node)
{
    auto tagTy = makeTy<TagType>(
                TagTypeKind::Enum,
                lexemeOrEmptyIdent(node->typeSpec_->tagToken()));
    pushTy(tagTy);
    makeBindAndPushSym<Enum>(node, tagTy);

    return visitTypeDeclaration_AtInternalDeclarations_COMMON(
                node,
                &Binder::visitEnumDeclaration_AtEnd);
}

SyntaxVisitor::Action Binder::visitTypedefDeclaration_AtSpecifier(const TypedefDeclarationSyntax* node)
{
    return visitDeclaration_AtSpecifiers_COMMON(
                node,
                &Binder::visitTypedefDeclaration_AtDeclarators);
}

template <class DeclT>
SyntaxVisitor::Action Binder::visitDeclaration_AtSpecifiers_COMMON(
        const DeclT* node,
        Action (Binder::*visit_AtDeclarators)(const DeclT*))
{
    for (auto specIt = node->specifiers(); specIt; specIt = specIt->next)
        visitIfNotTypeQualifier(specIt->value);

    if (tys_.empty()) {
        diagReporter_.TypeSpecifierMissingDefaultsToInt(node->lastToken());
        pushTy(makeTy<BasicType>(BasicTypeKind::Int));
    }

    for (auto specIt = node->specifiers(); specIt; specIt = specIt->next)
        visitIfTypeQualifier(specIt->value);

    return ((this)->*(visit_AtDeclarators))(node);
}

SyntaxVisitor::Action Binder::visitVariableAndOrFunctionDeclaration_AtSpecifiers(
        const VariableAndOrFunctionDeclarationSyntax* node)
{
    return visitDeclaration_AtSpecifiers_COMMON(
                node,
                &Binder::visitVariableAndOrFunctionDeclaration_AtDeclarators);
}

SyntaxVisitor::Action Binder::visitFunctionDefinition_AtSpecifiers(const FunctionDefinitionSyntax* node)
{
    return visitDeclaration_AtSpecifiers_COMMON(
                node,
                &Binder::visitFunctionDefinition_AtDeclarator);
}

SyntaxVisitor::Action Binder::visitFieldDeclaration_AtSpecifiers(const FieldDeclarationSyntax* node)
{
    return visitDeclaration_AtSpecifiers_COMMON(
                node,
                &Binder::visitFieldDeclaration_AtDeclarators);
}

SyntaxVisitor::Action Binder::visitEnumeratorDeclaration_AtImplicitSpecifier(const EnumeratorDeclarationSyntax* node)
{
    pushTy(makeTy<BasicType>(BasicTypeKind::Int));

    return visitEnumeratorDeclaration_AtDeclarator(node);
}

SyntaxVisitor::Action Binder::visitParameterDeclaration_AtSpecifiers(const ParameterDeclarationSyntax* node)
{
    return visitDeclaration_AtSpecifiers_COMMON(
                node,
                &Binder::visitParameterDeclaration_AtDeclarator);
}

SyntaxVisitor::Action Binder::visitIfNotTypeQualifier(const SpecifierSyntax* spec)
{
    if (spec->asTypeQualifier())
        return Action::Skip;

    visit(spec);

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitIfTypeQualifier(const SpecifierSyntax* spec)
{
    if (!spec->asTypeQualifier())
        return Action::Skip;

    visit(spec);

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitBasicTypeSpecifier(const BasicTypeSpecifierSyntax* node)
{
    auto tySpecTk = node->specifierToken();

    if (tys_.empty()) {
        BasicTypeKind basicTyK;
        switch (tySpecTk.kind()) {
            case SyntaxKind::Keyword_char:
                basicTyK = BasicTypeKind::Char;
                break;
            case SyntaxKind::Keyword_short:
                basicTyK = BasicTypeKind::Short;
                break;
            case SyntaxKind::Keyword_int:
                basicTyK = BasicTypeKind::Int;
                break;
            case SyntaxKind::Keyword_long:
                basicTyK = BasicTypeKind::Long;
                break;
            case SyntaxKind::Keyword_float:
                basicTyK = BasicTypeKind::Float;
                break;
            case SyntaxKind::Keyword_double:
                basicTyK = BasicTypeKind::Double;
                break;
            case SyntaxKind::Keyword__Bool:
                basicTyK = BasicTypeKind::Bool;
                break;
            case SyntaxKind::Keyword__Complex:
                basicTyK = BasicTypeKind::DoubleComplex;
                break;
            case SyntaxKind::Keyword_signed:
                basicTyK = BasicTypeKind::Int_S;
                break;
            case SyntaxKind::Keyword_unsigned:
                basicTyK = BasicTypeKind::Int_U;
                break;
            default:
                PSY_ESCAPE_VIA_RETURN(Action::Quit);
        }
        pushTy(makeTy<BasicType>(basicTyK));
        return Action::Skip;
    }

    TY_AT_TOP(ty);
    if (ty->kind() != TypeKind::Basic) {
        //diagReporter_.
        return Action::Skip;
    }

    BasicType* builtinTy = ty->asBasicType();
    auto curBasicTyK = builtinTy->kind();
    BasicTypeKind extraBasicTyK;
    switch (curBasicTyK) {
        case BasicTypeKind::Char:
            switch (tySpecTk.kind()) {
                case SyntaxKind::Keyword_signed:
                    extraBasicTyK = BasicTypeKind::Char_S;
                    break;
                case SyntaxKind::Keyword_unsigned:
                    extraBasicTyK = BasicTypeKind::Char_U;
                    break;
                default:
                    // report
                    return Action::Skip;
            }
            break;

        case BasicTypeKind::Char_S:
        case BasicTypeKind::Char_U:
            // report
            return Action::Skip;

        case BasicTypeKind::Short:
            switch (tySpecTk.kind()) {
                case SyntaxKind::Keyword_signed:
                    extraBasicTyK = BasicTypeKind::Short_S;
                    break;
                case SyntaxKind::Keyword_unsigned:
                    extraBasicTyK = BasicTypeKind::Short_U;
                    break;
                default:
                    // report
                    return Action::Skip;
            }
            break;

        case BasicTypeKind::Short_S:
        case BasicTypeKind::Short_U:
            // report
            return Action::Skip;

        case BasicTypeKind::Int:
            switch (tySpecTk.kind()) {
                case SyntaxKind::Keyword_long:
                    extraBasicTyK = BasicTypeKind::Long;
                    break;
                case SyntaxKind::Keyword_signed:
                    extraBasicTyK = BasicTypeKind::Int_S;
                    break;
                case SyntaxKind::Keyword_unsigned:
                    extraBasicTyK = BasicTypeKind::Int_U;
                    break;
                default:
                    diagReporter_.TwoOrMoreDataTypesInDeclarationSpecifiers(tySpecTk);
                    return Action::Skip;
            }
            break;

        case BasicTypeKind::Int_S:
        case BasicTypeKind::Int_U:
            // report
            return Action::Skip;

        case BasicTypeKind::Long:
            switch (tySpecTk.kind()) {
                case SyntaxKind::Keyword_int:
                    return Action::Skip;
                case SyntaxKind::Keyword_signed:
                    extraBasicTyK = BasicTypeKind::Long_S;
                    break;
                case SyntaxKind::Keyword_unsigned:
                    extraBasicTyK = BasicTypeKind::Long_U;
                    break;
                default:
                    diagReporter_.TwoOrMoreDataTypesInDeclarationSpecifiers(tySpecTk);
                    return Action::Skip;
            }
            break;

        case BasicTypeKind::Long_S:
        case BasicTypeKind::Long_U:
            // report
            return Action::Skip;

        default:
            PSY_ESCAPE_VIA_RETURN(Action::Skip);
    }

    if (extraBasicTyK != curBasicTyK)
        builtinTy->resetBasicTypeKind(extraBasicTyK);

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitVoidTypeSpecifier(const VoidTypeSpecifierSyntax* node)
{
    pushTy(makeTy<VoidType>());

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitTagTypeSpecifier(const TagTypeSpecifierSyntax* node)
{
    if (!node->declarations()) {
        TagTypeKind tagTyK;
        switch (node->kind()) {
            case SyntaxKind::StructTypeSpecifier:
                tagTyK = TagTypeKind::Struct;
                break;

            case SyntaxKind::UnionTypeSpecifier:
                tagTyK = TagTypeKind::Union;
                break;

            case SyntaxKind::EnumTypeSpecifier:
                tagTyK = TagTypeKind::Enum;
                break;

            default:
                PSY_ASSERT(false, return Action::Quit);
        }
        pushTy(makeTy<TagType>(tagTyK, lexemeOrEmptyIdent(node->tagToken())));
    }

    for (auto attrIt = node->attributes(); attrIt; attrIt = attrIt->next)
        visit(attrIt->value);

    for (auto declIt = node->declarations(); declIt; declIt = declIt->next) {
        TyContT tys;
        std::swap(tys_, tys);
        visit(declIt->value);
        std::swap(tys_, tys);
    }

    for (auto attrIt = node->attributes_PostCloseBrace(); attrIt; attrIt = attrIt->next)
        visit(attrIt->value);

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitTagDeclarationAsSpecifier(
        const TagDeclarationAsSpecifierSyntax* node)
{
    visit(node->tagDeclaration());

    const TagTypeSpecifierSyntax* tySpec = node->tagDeclaration()->typeSpecifier();
    switch (tySpec->kind()) {
        case SyntaxKind::StructTypeSpecifier: {
            auto tagTy = makeTy<TagType>(
                        TagTypeKind::Struct,
                        lexemeOrEmptyIdent(tySpec->tagToken()));
            pushTy(tagTy);
            break;
        }

        case SyntaxKind::UnionTypeSpecifier:{
            auto tagTy = makeTy<TagType>(
                        TagTypeKind::Union,
                        lexemeOrEmptyIdent(tySpec->tagToken()));
            pushTy(tagTy);
            break;
        }

        default:
            PSY_ASSERT(false, return Action::Quit);
    }

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitTypedefName(const TypedefNameSyntax* node)
{
    if (tys_.empty())
        pushTy(makeTy<TypedefType>(lexemeOrEmptyIdent(node->identifierToken())));

    return Action::Skip;
}

SyntaxVisitor::Action Binder::visitTypeQualifier(const TypeQualifierSyntax* node)
{
    QualifiedType* qualTy = nullptr;
    TY_AT_TOP(ty);
    switch (ty->kind()) {
        case TypeKind::Qualified:
            qualTy = ty->asQualifiedType();
            break;

        default:
            qualTy = makeTy<QualifiedType>(ty);
            pushTy(qualTy);
            break;
    }
    PSY_ASSERT(qualTy, return Action::Quit);

    const auto tyQualTk = node->qualifierKeyword();
    switch (tyQualTk.kind()) {
        case SyntaxKind::Keyword_const:
            qualTy->qualifyWithConst();
            break;

        case SyntaxKind::Keyword_volatile:
            qualTy->qualifyWithVolatile();
            break;

        case SyntaxKind::Keyword_restrict:
            if (qualTy->unqualifiedType()->kind() == TypeKind::Pointer)
                qualTy->qualifyWithRestrict();
            else
                diagReporter_.InvalidUseOfRestrict(tyQualTk);
            break;

        case SyntaxKind::Keyword__Atomic:
            qualTy->qualifyWithAtomic();
            break;

        default:
            PSY_ESCAPE_VIA_BREAK;
    }

    return Action::Skip;
}
