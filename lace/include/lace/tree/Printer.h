//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_PRINTER_H_
#define LOVELACE_PRINTER_H_

#include "lace/tree/Type.h"
#include "lace/tree/VisitorBase.h"

namespace lace {

class Printer final : public VisitorBase {
    uint32_t m_indent = 0;
    std::ostream& m_out;

    inline void print_indent() const {
        m_out << std::string(m_indent * 2, ' ');
    }

public:
    Printer(Options& options, std::ostream& out);

    void visit(AST& node) override;

    void visit(LoadDefn& node) override;
    void visit(VariableDefn& node) override;
    void visit(ParameterDefn& node) override;
    void visit(FunctionDefn& node) override;
    void visit(FieldDefn& node) override;
    void visit(VariantDefn& node) override;
    void visit(AliasDefn& node) override;
    void visit(StructDefn& node) override;
    void visit(EnumDefn& node) override;

    void visit(AdapterStmt& node) override;
    void visit(BlockStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(RestartStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(StopStmt& node) override;
    void visit(UntilStmt& node) override;

    void visit(BoolLiteral& node) override;
    void visit(CharLiteral& node) override;
    void visit(IntegerLiteral& node) override;
    void visit(FloatLiteral& node) override;
    void visit(NullLiteral& node) override;
    void visit(StringLiteral& node) override;
    
    void visit(BinaryOp& node) override;
    void visit(UnaryOp& node) override;

    void visit(AccessExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(CastExpr& node) override;
    void visit(ParenExpr& node) override;
    void visit(RefExpr& node) override;
    void visit(SizeofExpr& node) override;
    void visit(SubscriptExpr& node) override;
};

} // namespace lace

#endif // LOVELACE_PRINTER_H_
