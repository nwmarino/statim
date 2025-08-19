#include "bytecode.hpp"
#include <iomanip>
#include <string>

using namespace stm;

static void print_valuetype(std::ostream& os, ValueType type) {
    switch (type) {
    case ValueType::None:
        return;
    case ValueType::Int8:
        os << "i8";
        return;
    case ValueType::Int16:
        os << "i16";
        return;
    case ValueType::Int32:
        os << "i32";
        return;
    case ValueType::Int64:
        os << "i64";
        return;
    case ValueType::Float32:
        os << "f32";
        return;
    case ValueType::Float64:
        os << "f64";
        return;
    case ValueType::Pointer:
        os << "ptr";
        return;
    }
}

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

    case Kind::MemoryRef:
        os << '[';
        if (mem.base.id == 0)
            os << "stack";
        else
            os << 'v' << std::to_string(mem.base.id);
        
        if (mem.offset != 0)
            os << '+' << std::to_string(mem.offset);

        os << ']';
        break;

    case Kind::StackRef:
        os << "stack+" + std::to_string(stack.offset);
        break;

    case Kind::BlockRef:
        os << "bb" << std::to_string(block.pBlock->get_number());
        break;

    case Kind::FunctionRef:
        os << '@' << function.pFunction->get_name();
        break;
    }
}

void Instruction::print(std::ostream& os) const {
    std::ostringstream oss;
    oss << meta.file.filename() << ':' << meta.line;

    constexpr i32 total_width = 25;
    std::string left = oss.str();
    std::string pos_str = std::to_string(pos);
    i32 pad = total_width - static_cast<i32>(left.size() + pos_str.size());
    if (pad < 0) pad = 0;

    os << left << std::string(pad, ' ') << pos_str << "|" << std::setw(8);

    switch (op) {
    case Opcode::Constant:
        os << "const";
        break;
    case Opcode::Float:
        os << "float";
        break;
    case Opcode::String:
        os << "str";
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
    case Opcode::Load_Arg:
        os << "load_arg";
        break;
    case Opcode::Store_Arg:
        os << "store_arg";
        break;
    case Opcode::Jump:
        os << "jmp";
        break;
    case Opcode::Branch:
        os << "br";
        break;
    case Opcode::Call:
        os << "call";
        break;
    case Opcode::Return:
        os << "ret";
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
    case Opcode::Shl:
        os << "shl";
        break;
    case Opcode::Sar:
        os << "sar";
        break;
    case Opcode::Shr:
        os << "shr";
        break;
    }

    os << "   ";

    if (op == Opcode::Constant || op == Opcode::Float || op == Opcode::String) {
        operands[1].print(os);
        os << " = ";
        operands[0].print(os);
    } else for (u32 idx = 0, e = num_operands(); idx != e; ++idx) {
        operands[idx].print(os);
        if (idx + 1 != e)
            os << ", ";
    }

    if (desc.size.has_value())
        os << " ~" << *desc.size;
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
    os << "Bytecode for '" << name << " (";

    for (u32 idx = 0, e = args.size(); idx != e; ++idx) { 
        print_valuetype(os, args[idx]);
        if (idx + 1 != e)
            os << ", ";
    }

    os << ") -> ";
    print_valuetype(os, ret);
    os << "'\n\n";

    for (auto [ name, slot ] : stack) {
        os << "    ... offset: " << std::to_string(slot->get_offset()) << 
            ", name: " << name << '\n';
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

    for (auto [ name, function ] : functions)
        function->print(os);
}
