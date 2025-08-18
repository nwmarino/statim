#include "bytecode.hpp"
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
        os << 'v' << std::to_string(reg);
        break;
    case Kind::Memory:
        os << "[v" << std::to_string(memory.reg);
        if (memory.offset != 0)
            os << '+' << std::to_string(memory.offset);

        os << ']';
        break;
    case Kind::Stack:
        os << "";
        break;
    case Kind::Integer:
        os << std::to_string(imm);
        break;
    case Kind::Float:
        os << std::to_string(fp);
        break;
    case Kind::String:
        os << std::string(pString);
        break;
    case Kind::Block:
        os << std::to_string(pBlock->get_number());
        break;
    case Kind::Function:
        os << pFunction->get_name();
        break;
    }
}

void Instruction::print(std::ostream& os) const {
    os << meta.file.filename() << ':' << std::to_string(meta.line) << 
        '\t' << std::to_string(position) << "|\t";

    switch (op) {
    case Opcode::Constant:
        os << "constant";
        break;
    case Opcode::String:
        os << "string";
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
    case Opcode::Branch:
        os << "br";
        break;
    case Opcode::BranchIf:
        os << "br_if";
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

    os << ' ';
    for (u32 idx = 0, e = num_operands(); idx != e; ++idx) {
        operands[idx].print(os);
        if (idx + 1 != e)
            os << ", ";
    }
}

void BasicBlock::print(std::ostream& os) const {
    os << "--- Basic Block " << std::to_string(get_number()) << " ---\n\n";

    for (const Instruction* curr = pFront; curr; curr = curr->next()) {
        curr->print(os);
        if (curr->next())
            os << "\n";
    }
}

void Function::print(std::ostream& os) const {
    os << name << " :: (";

    for (u32 idx = 0, e = args.size(); idx != e; ++idx) { 
        print_valuetype(os, args[idx]);
        if (idx + 1 != e)
            os << ", ";
    }

    os << ") -> ";
    print_valuetype(os, ret);
    os << "\n\n";

    for (const BasicBlock* curr = pFront; curr; curr = curr->next()) {
        curr->print(os);
        if (curr->next())
            os << "\n";
    }

    os << "\n";
}

void Frame::print(std::ostream& os) const {
    for (auto [ name, function ] : functions)
        function->print(os);
}
