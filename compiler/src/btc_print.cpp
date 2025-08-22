#include "bytecode.hpp"

#include <iomanip>
#include <string>

using namespace stm;

void Operand::print(std::ostream& os) const {
    switch (kind) {
    case Kind::Register:
        os << 'v' << std::to_string(reg.id);
        break;

    case Kind::Immediate:
        switch (imm.kind) {
        case Immediate::Kind::Integer:
            os << std::to_string(imm.i);
            break;
        case Immediate::Kind::Float:
            os << std::to_string(imm.f);
            break;
        case Immediate::Kind::String:
            os << std::string(imm.s);
            break;
        }
        break;

    case Kind::Memory:
        os << '[';
        if (mem.base.id == 0)
            os << "stack";
        else
            os << 'v' << std::to_string(mem.base.id);
        
        if (mem.offset != 0)
            os << '+' << std::to_string(mem.offset);

        os << ']';
        break;

    case Kind::Argument:
        os << "arg." << std::to_string(arg.index);
        break;

    case Kind::Return:
        os << "ret." << std::to_string(ret.index);
        break;

    case Kind::Block:
        os << "bb" << std::to_string(block.pBlock->get_number());
        break;

    case Kind::Function:
        os << '@' << function.pFunction->get_name();
        break;
    }
}

void Instruction::print(std::ostream& os) const {
    std::ostringstream oss;
    oss << m_meta.file.filename() << ':' << m_meta.line;

    constexpr i32 TOTAL_WIDTH = 25;
    std::string left = oss.str();
    std::string pos_str = std::to_string(m_position);
    i32 pad = TOTAL_WIDTH - static_cast<i32>(left.size() + pos_str.size());
    if (pad < 0) 
        pad = 0;

    os << left << std::string(pad, ' ') << pos_str << "│";

    if (m_size == Size::None)
        os << ' ';

    os << std::setw(9);

    switch (m_op) {
    case Opcode::Constant:
        os << "const";
        break;
    case Opcode::Move:
        os << "mov";
        break;
    case Opcode::Lea:
        os << "lea";
        break;
    case Opcode::Copy:
        os << "cpy";
        break;
    case Opcode::Jump:
        os << "jmp";
        break;
    case Opcode::BranchTrue:
        os << "brt";
        break;
    case Opcode::BranchFalse:
        os << "brf";
        break;
    case Opcode::SetTrue:
        os << "sett";
        break;
    case Opcode::SetFalse:
        os << "setf";
        break;
    case Opcode::Call:
        os << "call";
        break;
    case Opcode::Return:
        os << "ret";
        break;
    case Opcode::Add:
        os << "add";
        break;
    case Opcode::Sub:
        os << "sub";
        break;
    case Opcode::Mul:
        os << "mul";
        break;
    case Opcode::Div:
        os << "div";
        break;
    case Opcode::Inc:
        os << "inc";
        break;
    case Opcode::Dec:
        os << "dec";
        break;
    case Opcode::Neg:
        os << "neg";
        break;
    case Opcode::And:
        os << "and";
        break;
    case Opcode::Or:
        os << "or";
        break;
    case Opcode::Xor:
        os << "xor";
        break;
    case Opcode::Not:
        os << "not";
        break;
    case Opcode::Shl:
        os << "shl";
        break;
    case Opcode::Sar:
        os << "sar";
        break;
    case Opcode::Shr:
        os << "shr";
        break;
    case Opcode::SExt:
        os << "sext";
        break;
    case Opcode::ZExt:
        os << "zext";
        break;
    case Opcode::FExt:
        os << "fext";
        break;
    case Opcode::Trunc:
        os << "trunc";
        break;
    case Opcode::FTrunc:
        os << "ftrunc";
        break;
    case Opcode::SI2SS:
        os << "si2ss";
        break;
    case Opcode::SI2SD:
        os << "si2sd";
        break;
    case Opcode::UI2SS:
        os << "ui2ss";
        break;
    case Opcode::UI2SD:
        os << "ui2sd";
        break;
    case Opcode::SS2SI:
        os << "ss2si";
        break;
    case Opcode::SD2SI:
        os << "sd2si";
        break;
    case Opcode::SS2UI:
        os << "ss2ui";
        break;
    case Opcode::SD2UI:
        os << "sd2ui";
        break;
    case Opcode::Cmpeq:
        os << "cmpeq";
        break;
    case Opcode::Cmpne:
        os << "cmpne";
        break;
    case Opcode::Cmpoeq:
        os << "cmpoeq";
        break;
    case Opcode::Cmpone:
        os << "cmpone";
        break;
    case Opcode::Cmpuneq:
        os << "cmpuneq";
        break;
    case Opcode::Cmpunne:
        os << "cmpunne";
        break;
    case Opcode::Cmpslt:
        os << "cmpslt";
        break;
    case Opcode::Cmpsle:
        os << "cmpsle";
        break;
    case Opcode::Cmpsgt:
        os << "cmpsgt";
        break;
    case Opcode::Cmpsge:
        os << "cmpsge";
        break;
    case Opcode::Cmpult:
        os << "cmpult";
        break;
    case Opcode::Cmpule:
        os << "cmpule";
        break;
    case Opcode::Cmpugt:
        os << "cmpugt";
        break;
    case Opcode::Cmpuge:
        os << "cmpuge";
        break;
    case Opcode::Cmpolt:
        os << "cmpolt";
        break;
    case Opcode::Cmpole:
        os << "cmpole";
        break;
    case Opcode::Cmpogt:
        os << "cmpogt";
        break;
    case Opcode::Cmpoge:
        os << "cmpoge";
        break;
    case Opcode::Cmpunlt:
        os << "cmpunlt";
        break;
    case Opcode::Cmpunle:
        os << "cmpunle";
        break;
    case Opcode::Cmpungt:
        os << "cmpungt";
        break;
    case Opcode::Cmpunge:
        os << "cmpunge";
        break;
    }

    switch (m_size) {
    case Size::None:
        os << " ";
        break;
    case Size::Byte:
        os << "b ";
        break;
    case Size::Half:
        os << "h ";
        break;
    case Size::Quad:
        os << "q ";
        break;
    case Size::Word:
        os << "w ";
        break;
    case Size::Single:
        os << "ss";
        break;
    case Size::Double:
        os << "sd";
        break;
    }

    os << "   ";

    for (u32 idx = 0, e = num_operands(); idx != e; ++idx) {
        m_operands[idx].print(os);
        if (idx + 1 != e) os << ", ";
    }
}

void BasicBlock::print(std::ostream& os) const {
    os << "bb" << std::to_string(get_number()) << ":\n\n";

    for (const Instruction* curr = pFront; curr; curr = curr->next()) {
        curr->print(os);
        if (curr->next())
            os << "\n";
    }

    os << '\n';
}

void Function::print(std::ostream& os) const {
    os << "Bytecode for '" << name << "'\n";

    if (!stack.empty()) {
        os << "\nstack (" << std::to_string(get_stack_size()) << " bytes)\n";
    }

    for (auto slot : stack) {
        os << "    ~ offset: " << std::to_string(slot->get_offset()) << 
            ", name: " << slot->get_name() << '\n';
    }

    os << "\n";

    for (const BasicBlock* curr = pFront; curr; curr = curr->next()) {
        curr->print(os);
        if (curr->next())
            os << "\n";
    }

    os << "\n";
}

void Frame::print(std::ostream& os) const {
    os << std::right;

    for (auto [ name, function ] : m_functions)
        function->print(os);
}
