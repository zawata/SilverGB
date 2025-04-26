#pragma once

#include <string_view>
#include <sys/types.h>
#include <variant>

#include "util/bit.hpp"
#include "util/decl.hpp"
#include "util/flags.hpp"
#include "util/types/primitives.hpp"
#include "util/util.hpp"

namespace Arm {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    struct reg_t {
        u8 v;

        reg_t() :
            v(0) { }

        [[maybe_unused]] reg_t(u8 v) :
            v(v) { }

        operator u8 () const { return v; }

        std::string to_string() const { return "R" + std::to_string(v); }
    };
#pragma clang diagnostic pop

    inline std::ostream &operator<< (std::ostream &o, reg_t r) { return o << r.to_string(); }

    namespace Registers {
        const static reg_t SP = 13;
        const static reg_t LR = 14;
        const static reg_t PC = 15;
    }

    namespace Mask {
        using namespace Bit::Mask;

        // all
        const static u32 condition   = between_inc<u32>(28, 31);
        // branch and exchange
        const static u32 bex_const   = between_inc<u32>(4, 27);
        const static u32 bex_reg_N   = until_inc<u32>(3);
        // branch
        const static u32 br_is_link  = bit<u32>(24);
        const static u32 br_offset   = until_inc<u32>(23);
        // multiply
        const static u32 mul_accum   = bit<u32>(21);
        const static u32 mul_reg_D   = between_inc<u32>(16, 19);
        const static u32 mul_reg_N   = between_inc<u32>(12, 15);
        const static u32 mul_reg_S   = between_inc<u32>(8, 11);
        const static u32 mul_reg_M   = between_inc<u32>(0, 3);
        // software interrupt
        const static u32 swi_comment = 0x00FFFFFF;
        // Coprocessor
        const static u32 cp_is_crt   = bit<u32>(4);
    } // namespace Mask

    namespace Literal {
        const static u32 MUL_CONSTANT  = 0b1001;
        const static u32 BEX_CONSTANT  = 0b000100101111111111110001;
        const static u32 MRS_CONSTANT  = 0b001111;
        const static u32 MSR_CONSTANT  = 0b101001;
        const static u32 MSRF_CONSTANT = 0b101000;
    } // namespace Literal

    enum struct Mnemonic {
        ADC,  // Add with carry
        ADD,  // Add
        AND,  // AND
        B,    // Branch
        BIC,  // Bit Clear
        BL,   // Branch with Link
        BX,   // Branch and Exchange
        CDP,  // Coprocessor Data Processing
        CMN,  // Compare Negative
        CMP,  // Compare
        EOR,  // Exclusive OR
        LDC,  // Load coprocessor from memory
        LDM,  // Load multiple registers
        LDR,  // Load register From memory
        MCR,  // Move CPU register to coprocessor register
        MLA,  // Multiply and Accumulate
        MLAL, // Multiply and Accumulate Long
        MOV,  // Move register or constant
        MRC,  // Move from coprocessor register to CPU register
        MRS,  // Move PSR status/flags to register
        MSR,  // Move register to PSR status/flags
        MUL,  // Multiply
        MULL, // Multiply Long
        MVN,  // Move negative register
        ORR,  // OR
        RSB,  // Reverse Subtract
        RSC,  // Reverse Subtract with Carry
        SBC,  // Subtract with Carry
        STC,  // Store coprocessor register to memory
        STM,  // Store Multiple
        STR,  // Store register to memory
        SUB,  // Subtract
        SWI,  // Software Interrupt
        SWP,  // Swap register with memory
        TEQ,  // Test bitwise equality
        TST,  // Test bits
        Undefined,
    };

    inline std::string to_string(Mnemonic mnemonic) {
        static const char *mnemonic_strings[]
                = {"ADC", "ADD", "AND", "B",    "BIC", "BL",  "BX",  "CDP", "CMN", "CMP",  "EOR", "LDC", "LDM",
                   "LDR", "MCR", "MLA", "MLAL", "MOV", "MRC", "MRS", "MSR", "MUL", "MLAL", "MVN", "ORR", "RSB",
                   "RSC", "SBC", "STC", "STM",  "STR", "SUB", "SWI", "SWP", "TEQ", "TST",  "---"};
        static_assert(array_length(mnemonic_strings) == (int)Mnemonic::Undefined + 1);
        return mnemonic_strings[(int)mnemonic];
    }

    enum struct Condition : u8 {
        EQ,
        Equal = EQ,
        NE,
        NotEqual = NE,
        CS,
        CarrySet = CS,
        CC,
        CarryClear = CC,
        MI,
        Minus = MI,
        PL,
        Plus = PL,
        VS,
        Overflow = VS,
        VC,
        NoOverflow = VC,
        HI,
        UnsignedGreaterThan = HI,
        LS,
        UnsignedLessThanOrEqual = LS,
        GE,
        SignedGreaterThanOrEqual = GE,
        LT,
        SignedLessThan = LT,
        GT,
        SignedGreaterThan = GT,
        LE,
        SignedLessThanOrEqual = LE,
        AL,
        Always = AL,
        NV,
        Never = NV
    };

    inline std::string to_string(Condition condition) {
        static const char *condition_strings[]
                = {"EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE", "", "NV"};
        return condition_strings[(u8)condition];
    }

    enum struct ShiftType : u8 { LogicalLeft, LogicalRight, ArithmeticRight, RotateRight };

    inline std::string to_string(ShiftType shiftType) {
        static const char *shift_strings[] = {"LSL", "LSR", "ASR", "ROR"};
        return shift_strings[(u8)shiftType];
    }

    /**
     * Branch And Exchange
     * BX
     */
    struct BranchAndExchange {
        Condition                condition;
        reg_t                    rN;

        static BranchAndExchange Decode(u32 word) {
            BranchAndExchange bex {};
            bex.condition = (Condition)Bit::range<u32>(Mask::condition, word);
            bex.rN        = (u8)Bit::range(Mask::bex_reg_N, word);
            return bex;
        }

        static BranchAndExchange Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return Mnemonic::BX; }

        u32 Encode() const { return 0_u32 | (u8)condition << 28 | Literal::BEX_CONSTANT << 4 | (rN & Mask::bex_reg_N); }

        std::string Disassemble() const {
            std::stringstream ss;

            ss << to_string(Mnemonic()) << to_string(condition) << " " << rN;
            return ss.str();
        };
    };

    /**
     * Branch
     * B, BL
     */
    struct Branch {
        Condition     condition;
        bool          link;
        s32           offset;

        static Branch Decode(u32 word) {
            Branch b {};
            b.condition    = (Condition)Bit::range(Mask::condition, word);
            b.link         = (bool)Bit::range(Mask::br_is_link, word);

            u32 raw_offset = Bit::mask(Mask::br_offset, word);

            // The branch offset is left shifted by 2 then sign-extended.
            // The offset is 0-23bits + 2 for the shift, making the sign bit 25
            b.offset       = Bit::sign_extend(raw_offset << 2, 25);

            return b;
        }

        static Branch Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return link ? Mnemonic::BL : Mnemonic::B; }

        u32           Encode() const {
            return 0_u32 | (u8)condition << 28 | 0b101 << 25 | (link ? 1 : 0) << 24
                 | Bit::sign_compress<u32>(offset & Mask::br_offset, 25) >> 2;
        }

        std::string Disassemble() const {
            std::stringstream ss;

            ss << to_string(Mnemonic()) << to_string(condition) << " ";
            // From the ISA Doc:
            // The branch offset must take account of the prefetch operation, which causes the PC to be 2 words (8
            // bytes) ahead of the current instruction. account for this visually
            ss << (offset + 8);
            return ss.str();
        };
    };

    /**
     * Data Processing
     */
    struct DataProcessing {
        Condition condition;

        struct BitMask {
            __disallow_construct(BitMask);

            const static u32 isImmediate            = Bit::Mask::bit<u32>(25);
            const static u32 isSetFlags             = Bit::Mask::bit<u32>(20);
            const static u32 OpCode                 = Bit::Mask::between_inc<u32>(21, 24);
            const static u32 Reg_N                  = 0x000F0000;
            const static u32 Reg_D                  = 0x0000F000;
            const static u32 RegisterShiftValue     = Bit::Mask::bit<u32>(4);
            const static u32 RegisterShiftType      = 0x00000060;
            const static u32 Reg_M                  = 0x0000000F;
            const static u32 RegisterShiftImmediate = 0x00000F80;
            const static u32 Reg_S                  = 0x00000F00;
            const static u32 ImmediateRotation      = 0x00000F00;
            const static u32 ImmediateValue         = 0x000000FF;
        };

        enum struct OperationCode {
            AND,
            EOR,
            SUB,
            RSB,
            ADD,
            ADC,
            SBC,
            RSC,
            TST,
            TEQ,
            CMP,
            CMN,
            ORR,
            MOV,
            BIC,
            MVN,
        } operation_code;

        bool  is_imm;
        bool  set_flags;
        reg_t rN, rD;

        union {
            struct {
                bool      is_reg;
                ShiftType shift_type;

                union {
                    reg_t rS;
                    u8    shift_amount;
                };

                reg_t rM;
            } ShiftedRegisterOperand {};

            struct {
                u8 rotate;
                u8 immediate;
            } ImmediateOperand;
        };

        static DataProcessing Decode(u32 word) {
            DataProcessing dp {};
            dp.condition      = (Condition)Bit::range(Mask::condition, word);
            dp.operation_code = (OperationCode)Bit::range(BitMask::OpCode, word);
            dp.is_imm         = Bit::range(BitMask::isImmediate, word);
            dp.set_flags      = Bit::range(BitMask::isSetFlags, word);
            dp.rN             = Bit::range(BitMask::Reg_N, word);
            dp.rD             = Bit::range(BitMask::Reg_D, word);

            if(dp.is_imm) {
                dp.ImmediateOperand.rotate    = Bit::range(BitMask::ImmediateRotation, word);
                dp.ImmediateOperand.immediate = Bit::range(BitMask::ImmediateValue, word);
            } else {
                dp.ShiftedRegisterOperand.is_reg     = Bit::range(BitMask::RegisterShiftValue, word);
                dp.ShiftedRegisterOperand.shift_type = (ShiftType)Bit::range(BitMask::RegisterShiftType, word);
                if(dp.ShiftedRegisterOperand.is_reg) {
                    dp.ShiftedRegisterOperand.rS = Bit::range(BitMask::Reg_S, word);
                } else {
                    dp.ShiftedRegisterOperand.shift_amount = Bit::range(BitMask::RegisterShiftImmediate, word);
                }
                dp.ShiftedRegisterOperand.rM = Bit::range(BitMask::Reg_M, word);
            }

            return dp;
        }

        // Assemble
        static DataProcessing Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const {
            using I = enum Mnemonic;
            static const enum Mnemonic instr_tbl[]
                    = {I::AND,
                       I::EOR,
                       I::SUB,
                       I::RSB,
                       I::ADD,
                       I::ADC,
                       I::SBC,
                       I::RSC,
                       I::TST,
                       I::TEQ,
                       I::CMP,
                       I::CMN,
                       I::ORR,
                       I::MOV,
                       I::BIC,
                       I::MVN};

            return instr_tbl[(int)operation_code];
        }

        u32 Encode() const {
            u16 op2 = 0;
            if(is_imm) {
                op2 = ImmediateOperand.rotate << 8 | ImmediateOperand.immediate;
            } else {
                if(ShiftedRegisterOperand.is_reg) {
                    op2 = ShiftedRegisterOperand.rS << 8 | (u8)ShiftedRegisterOperand.shift_type << 5 | 1 << 4;
                } else {
                    op2 = ShiftedRegisterOperand.shift_amount << 7 | (u8)ShiftedRegisterOperand.shift_type << 5;
                }
                op2 |= ShiftedRegisterOperand.rM;
            }

            return 0_u32 | (u8)condition << 28 | Bit::from_bool(is_imm) << 25 | (u8)operation_code << 21
                 | Bit::from_bool(set_flags) << 20 | (u8)rN << 16 | (u8)rN << 12 | op2;
        }

        std::string Disassemble() const {
            std::stringstream ss;

            bool opcode_produces_result = operation_code != OperationCode::CMP && operation_code != OperationCode::CMN
                                       && operation_code != OperationCode::TEQ && operation_code != OperationCode::TST;
            bool opcode_uses_2op = operation_code != OperationCode::MOV && operation_code != OperationCode::MVN;

            // these instructions set flags implicitly and thus, the S is omitted
            bool show_S          = set_flags && opcode_produces_result;

            ss << to_string(Mnemonic()) << to_string(condition) << (show_S ? "S" : "") << " ";

            if(opcode_produces_result) {
                ss << rD.to_string() << ", ";
            }

            if(opcode_uses_2op) {
                ss << rN.to_string() << ", ";
            }

            if(is_imm) {
                // TODO: I think this is wrong
                u16 op2 = ImmediateOperand.rotate << 8 | ImmediateOperand.immediate;
                ss << "#" << as_hex(op2);
            } else {
                ss << ShiftedRegisterOperand.rM.to_string() << ", " << to_string(ShiftedRegisterOperand.shift_type)
                   << " ";
                if(ShiftedRegisterOperand.is_reg) {
                    ss << ShiftedRegisterOperand.rS;
                } else {
                    ss << "#" << as_hex(ShiftedRegisterOperand.shift_amount);
                }
            }

            return ss.str();
        };
    };

    /**
     * PSR Transfer
     */
    struct PSRTransfer {
        Condition condition;
        bool      use_spsr;

        enum struct Type {
            PSRToRegister,
            RegisterToPSR,
            RegisterToPSRF,
        } type;

        union {
            struct {
                reg_t rD;
            } PSRToRegister;

            struct {
                reg_t rM;
            } RegisterToPSR;

            struct {
                bool is_imm;

                union {
                    struct {
                        reg_t rM;
                    } RegisterOperand;

                    struct {
                        u8 rotation;
                        u8 value;
                    } ImmediateOperand;
                };
            } RegisterToPSRF;
        };

        static PSRTransfer Decode(u32 word) {
            PSRTransfer psr {};
            psr.condition = (Condition)Bit::range(Mask::condition, word);
            psr.use_spsr  = Bit::test(word, 22);
            u32 psr_type  = Bit::range(Bit::Mask::between_inc<u32>(16, 21), word);
            if(psr_type == Literal::MRS_CONSTANT) {
                psr.type             = Type::PSRToRegister;
                psr.PSRToRegister.rD = Bit::range(Bit::Mask::between_inc<u32>(12, 15), word);
            } else if(psr_type == Literal::MSR_CONSTANT) {
                psr.type             = Type::RegisterToPSR;
                psr.PSRToRegister.rD = Bit::mask(Bit::Mask::until<u32>(3), word);
            } else if(psr_type == Literal::MSRF_CONSTANT) {
                psr.type                  = Type::RegisterToPSRF;
                psr.RegisterToPSRF.is_imm = Bit::test(word, 25);
                if(psr.RegisterToPSRF.is_imm) {
                    psr.RegisterToPSRF.ImmediateOperand.rotation = Bit::range(Bit::Mask::between_inc<u32>(8, 11), word);
                    psr.RegisterToPSRF.ImmediateOperand.value    = Bit::mask(Bit::Mask::until_inc<u32>(7), word);
                } else {
                    psr.RegisterToPSRF.RegisterOperand.rM = Bit::mask(Bit::Mask::until_inc<u32>(3), word);
                }
            }

            return psr;
        }

        static PSRTransfer Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return type == Type::PSRToRegister ? Mnemonic::MRS : Mnemonic::MSR; }

        u32           Encode() const {
            bool       imm_bit = type == Type::RegisterToPSRF && RegisterToPSRF.is_imm;
            static u32 type_const_tbl[] = {Literal::MRS_CONSTANT, Literal::MSR_CONSTANT, Literal::MSRF_CONSTANT};

            u16        lower_half = 0;
            switch(type) {
            case Type::PSRToRegister: lower_half = PSRToRegister.rD << 12; break;
            case Type::RegisterToPSR: lower_half = 0xF000 | RegisterToPSR.rM; break;
            case Type::RegisterToPSRF:
                lower_half = 0xF000;
                if(RegisterToPSRF.is_imm) {
                    lower_half |= RegisterToPSRF.RegisterOperand.rM;
                } else {
                    lower_half |= RegisterToPSRF.ImmediateOperand.rotation << 8 | RegisterToPSRF.ImmediateOperand.value;
                }
                break;
            }

            return 0_u32 | (u8)condition << 28 | Bit::from_bool(imm_bit) << 25 | 1 << 24
                 | Bit::from_bool(use_spsr) << 22 | type_const_tbl[(int)type] << 16 | lower_half;
        }

        std::string Disassemble() const {
            std::stringstream ss;
            ss << to_string(Mnemonic()) << to_string(condition) << " ";

            switch(type) {
            case Type::PSRToRegister:
                ss << PSRToRegister.rD.to_string() << ", " << (use_spsr ? "SPSR_all" : "CPSR_all");
                break;
            case Type::RegisterToPSR:
                ss << (use_spsr ? "SPSR_all" : "CPSR_all") << ", " << RegisterToPSR.rM.to_string();
                break;
            case Type::RegisterToPSRF:
                ss << (use_spsr ? "SPSR_flg" : "CPSR_flg") << ", ";
                if(RegisterToPSRF.is_imm) {
                    u16 op2 = RegisterToPSRF.ImmediateOperand.rotation << 8 | RegisterToPSRF.ImmediateOperand.value;
                    ss << "#" << as_hex(op2);
                } else {
                    ss << RegisterToPSRF.RegisterOperand.rM.to_string();
                }
                break;
            }

            return ss.str();
        };
    };

    /**
     * Multiply and Multiply and Accumulate
     */
    struct Multiply {
        Condition       condition;
        bool            accumulate;
        bool            set_flags;
        reg_t           rD {}, rN {}, rS {}, rM {};

        static Multiply Decode(u32 word) {
            Multiply mul {};
            mul.condition  = (Condition)Bit::range(Mask::condition, word);
            mul.accumulate = Bit::range(Mask::mul_accum, word);

            mul.set_flags  = Bit::range(Mask::mul_accum, word);
            mul.rD         = Bit::range(Mask::mul_reg_D, word);
            mul.rN         = Bit::range(Mask::mul_reg_N, word);
            mul.rS         = Bit::range(Mask::mul_reg_S, word);
            mul.rM         = Bit::range(Mask::mul_reg_M, word);
            return mul;
        }

        static Multiply Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return accumulate ? Mnemonic::MLA : Mnemonic::MUL; }

        u32           Encode() const {
            return 0_u32 | (u8)condition << 28 | Bit::from_bool(accumulate) << 21 | Bit::from_bool(set_flags) << 20
                 | (u8)rD << 16 | (u8)rN << 12 | (u8)rS << 8 | (u8)Literal::MUL_CONSTANT << 4 | (u8)rD;
        }

        std::string Disassemble() const {
            std::stringstream ss;

            ss << to_string(Mnemonic()) << to_string(condition) << (set_flags ? "S" : "") << " ";
            ss << rD.to_string() << ", " << rM.to_string() << ", " << rS.to_string();
            if(accumulate) {
                ss << ", " << rN.to_string();
            }
            return ss.str();
        };
    };

    /**
     * Multiply Long and Multiply Long and Accumulate
     */
    struct MultiplyLong {
        Condition           condition;
        bool                _signed;
        bool                accumulate;
        bool                set_flags;
        reg_t               rDHi, rDLo, rS, rM;

        static MultiplyLong Decode(u32 word) {
            MultiplyLong mul {};
            mul.condition  = (Condition)Bit::range(Mask::condition, word);
            mul._signed    = Bit::test(word, 22);
            mul.accumulate = Bit::test(word, 21);
            mul.set_flags  = Bit::test(word, 20);
            mul.rDHi       = Bit::range(Mask::mul_reg_D, word);
            mul.rDLo       = Bit::range(Mask::mul_reg_N, word);
            mul.rS         = Bit::range(Mask::mul_reg_S, word);
            mul.rM         = Bit::range(Mask::mul_reg_M, word);
            return mul;
        }

        static MultiplyLong Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return accumulate ? Mnemonic::MLAL : Mnemonic::MULL; }

        u32           Encode() const {
            return 0_u32 | (u8)condition << 28 | 1 << 23 | Bit::from_bool(_signed) << 22
                 | Bit::from_bool(accumulate) << 21 | Bit::from_bool(set_flags) << 20 | (u8)rDHi << 16 | (u8)rDLo << 12
                 | (u8)rS << 8 | (u8)Literal::MUL_CONSTANT << 4 | (u8)rM;
        }

        std::string Disassemble() const {
            std::stringstream ss;
            ss << (_signed ? "U" : "S");
            ss << to_string(Mnemonic()) << to_string(condition) << (set_flags ? "S" : "") << " ";
            ss << rDLo.to_string() << ", " << rDHi.to_string() << ", " << rM.to_string() << ", " << rS.to_string();
            return ss.str();
        };
    };

    /**
     * Single Data Transfer
     */
    struct SingleDataTransfer {
        Condition condition;
        bool      is_imm;
        bool      is_pre_idx;
        bool      is_inc;
        bool      is_byte;
        bool      write_back;
        bool      load;
        reg_t     rN {}, rD {};

        union {
            struct {
                ShiftType shift_type;
                u8        shift_amount;
                reg_t     rM;
            } ShiftedRegisterOperand;

            struct {
                u16 offset;
            } ImmediateOperand {};
        };

        static SingleDataTransfer Decode(u32 word) {
            SingleDataTransfer sdt {};
            sdt.condition  = (Condition)Bit::range(Mask::condition, word);
            sdt.is_imm     = !Bit::test(word, 25);
            sdt.is_pre_idx = Bit::test(word, 24);
            sdt.is_inc     = Bit::test(word, 23);
            sdt.is_byte    = Bit::test(word, 22);
            sdt.write_back = Bit::test(word, 21);
            sdt.load       = Bit::test(word, 20);
            sdt.rN         = Bit::range(Bit::Mask::between_inc<u32>(16, 19), word);
            sdt.rD         = Bit::range(Bit::Mask::between_inc<u32>(12, 15), word);
            if(sdt.is_imm) {
                sdt.ImmediateOperand.offset = Bit::mask(Bit::Mask::until_inc<u32>(11), word);
            } else {
                sdt.ShiftedRegisterOperand.shift_amount = Bit::mask(Bit::Mask::between_inc<u32>(4, 11), word);
                sdt.ShiftedRegisterOperand.rM           = Bit::mask(Bit::Mask::until_inc<u32>(3), word);
            }
            return sdt;
        }

        static SingleDataTransfer Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return load ? Mnemonic::LDR : Mnemonic::STR; }

        u32           Encode() const {
            // TODO
            return 0;
        }

        std::string Disassemble() const {
            std::stringstream ss;
            ss << to_string(Mnemonic()) << to_string(condition) << (is_byte ? "B" : "")
               << (!is_pre_idx && write_back ? "T" : "") << " ";
            ss << rD.to_string() << ", ";

            ss << "[" << rN.to_string();
            if(!is_pre_idx) {
                ss << ']';
            } else {
                ss << ", ";
            }

            if(is_imm && ImmediateOperand.offset != 0) {
                ss << "#" << as_hex(ImmediateOperand.offset);
            } else {
                ss << (is_inc ? "" : "-") << ShiftedRegisterOperand.rM.to_string();
                if(ShiftedRegisterOperand.shift_amount > 0) {
                    ss << ", #" << as_hex(ShiftedRegisterOperand.shift_amount);
                }
            }

            if(is_pre_idx) {
                ss << ']';
            }

            return ss.str();
        }
    };

    /**
     * Halfword and Signed Data Transfer
     */
    struct HalfwordDataTransfer {

        Condition condition;
        bool      is_pre_idx;
        bool      is_inc;
        bool      is_imm;
        bool      write_back;
        bool      load;
        reg_t     rN, rD;
        bool      signed_data;
        bool      halfwords;

        union {
            struct {
                reg_t rM;
            } RegisterOperand;

            struct {
                u8 offset;
            } ImmediateOperand {};
        };

        reg_t                       rM;

        static HalfwordDataTransfer Decode(u32 word) {
            HalfwordDataTransfer hdt {};
            hdt.condition  = (Condition)Bit::range(Mask::condition, word);
            hdt.is_pre_idx = Bit::test(word, 24);
            hdt.is_inc     = Bit::test(word, 23);
            hdt.is_imm     = Bit::test(word, 22);
            hdt.write_back = Bit::test(word, 21);
            hdt.load       = Bit::test(word, 20);
            hdt.rN         = Bit::range(Bit::Mask::between_inc<u32>(19, 16), word);
            hdt.rD         = Bit::range(Bit::Mask::between_inc<u32>(15, 12), word);
            assert(Bit::range(Bit::Mask::between_inc<u32>(5, 6), word) != 0);
            hdt.signed_data = Bit::test(word, 6);
            hdt.halfwords   = Bit::test(word, 5);
            if(hdt.is_imm) {
                hdt.ImmediateOperand.offset = (Bit::range(Bit::Mask::between_inc<u32>(11, 8), word) << 4)
                                            | Bit::mask(Bit::Mask::until_inc<u32>(3), word);
            } else {
                hdt.rM = Bit::range(Bit::Mask::until_inc<u32>(3), word);
            }
            return hdt;
        }

        static HalfwordDataTransfer Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return load ? Mnemonic::LDR : Mnemonic::STR; }

        u32           Encode() const {
            return 0_u32 | (u8)condition << 28 | Bit::from_bool(is_pre_idx) << 24 | Bit::from_bool(is_inc) << 23
                 | Bit::from_bool(write_back) << 21 | Bit::from_bool(load) << 20 | (u8)rN << 16 | (u8)rD << 12
                 | Bit::Mask::bit<u32>(7) | Bit::from_bool(signed_data) | Bit::from_bool(halfwords)
                 | Bit::Mask::bit<u32>(4) | (u8)rM;
        }

        std::string Disassemble() const {
            std::stringstream ss;
            ss << to_string(Mnemonic()) << to_string(condition) << (signed_data ? "S" : "") << (halfwords ? "H" : "B")
               << " ";
            ss << rD.to_string() << ", ";

            ss << "[" << rN.to_string();
            if(!is_pre_idx) {
                ss << ']';
            }

            if(is_imm) {
                if(ImmediateOperand.offset == 0) {
                    ss << (u32)ImmediateOperand.offset;
                } else {
                    ss << "]";
                }
            } else {
                ss << (is_inc ? "+" : "-") << RegisterOperand.rM.to_string();
            }

            if(is_pre_idx) {
                ss << ']';
            }
            return ss.str();
        }
    };

    /**
     * Block Data Transfer
     */
    struct BlockDataTransfer {
        Condition                condition;
        bool                     is_pre_idx;
        bool                     is_inc;
        bool                     load_psr;
        bool                     write_back;
        bool                     load;
        reg_t                    rN;
        u16                      registers;

        static BlockDataTransfer Decode(u32 word) {
            BlockDataTransfer bdt {};
            bdt.condition  = (Condition)Bit::range(Mask::condition, word);
            bdt.is_pre_idx = Bit::test(word, 24);
            bdt.is_inc     = Bit::test(word, 23);
            bdt.load_psr   = Bit::test(word, 22);
            bdt.write_back = Bit::test(word, 21);
            bdt.load       = Bit::test(word, 20);
            bdt.rN         = Bit::range(Bit::Mask::between_inc<u32>(19, 16), word);
            bdt.registers  = Bit::mask(Bit::Mask::until_inc<u32>(15), word);
        }

        static BlockDataTransfer Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return load ? Mnemonic::LDM : Mnemonic::STM; }

        u32           Encode() const {
            return 0_u32 | (u8)condition << 28 | Bit::Mask::bit<u32>(27) | Bit::from_bool(is_pre_idx) << 24
                 | Bit::from_bool(is_inc) << 23 | Bit::from_bool(load_psr) | Bit::from_bool(write_back) << 21
                 | Bit::from_bool(load) << 20 | (u8)rN << 16 | registers;
        }

        std::string Disassemble() const {
            std::stringstream ss;
            // TODO
            ss << to_string(Mnemonic()) << to_string(condition) << " ;TODO";

            return ss.str();
        }
    };

    /**
     * Swap
     */
    struct Swap {
        Condition   condition;
        bool        byte;
        reg_t       rN, rD, rM;

        static Swap Decode(u32 word) {
            Swap swp {};
            swp.condition = (Condition)Bit::range(Mask::condition, word);
            swp.byte      = Bit::test(word, 22);
            swp.rN        = Bit::range(word, Bit::Mask::between<u32>(16, 19));
            swp.rD        = Bit::range(word, Bit::Mask::between<u32>(16, 19));
            swp.rM        = Bit::range(word, Bit::Mask::between<u32>(16, 19));
            return swp;
        }

        static Swap Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return Mnemonic::SWP; }

        u32           Encode() const {
            return 0_u32 | (u8)condition << 28 | 1 << 24 | (byte ? 1 : 0) << 22 | rN << 16 | rD << 12 | 0b1001 << 4
                 | rM;
        }

        std::string Disassemble() const {
            std::stringstream ss;

            ss << to_string(Mnemonic()) << to_string(condition) << (byte ? "B" : "") << " ";
            ss << rD << ", " << rM << ", [" << rN << "]";

            return ss.str();
        }
    };

    /**
     * Software Interrupt
     */
    struct SoftwareInterrupt {
        Condition                condition;
        u32                      value;

        static SoftwareInterrupt Decode(u32 word) {
            SoftwareInterrupt swi {};
            swi.condition = (Condition)Bit::range(Mask::condition, word);
            swi.value     = Bit::range(Mask::swi_comment, word);
            return swi;
        }

        static SoftwareInterrupt Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return Mnemonic::SWI; }

        u32           Encode() const { return 0_u32 | (u8)condition << 28 | 0xF << 24 | value & Mask::swi_comment; }

        std::string   Disassemble() const {
            std::stringstream ss;
            // TODO
            ss << to_string(Mnemonic()) << to_string(condition) << " ;TODO";

            return ss.str();
        }
    };

    /**
     * Coprocessor Data Operation
     */
    struct CoprocessorDataOperation {
        Condition                       condition;

        static CoprocessorDataOperation Decode(u32 word) {
            CoprocessorDataOperation cdp {};
            cdp.condition = (Condition)Bit::range(Mask::condition, word);
            // TODO
            return cdp;
        }

        static CoprocessorDataOperation Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return Mnemonic::CDP; }

        u32           Encode() const {
            // TODO
            return 0;
        }

        std::string Disassemble() const {
            std::stringstream ss;
            // TODO
            ss << to_string(Mnemonic()) << to_string(condition) << " ;TODO";

            return ss.str();
        }
    };

    /**
     * Coprocessor Data Transfer
     */
    struct CoprocessorDataTransfer {
        Condition                      condition;
        bool                           is_load;

        static CoprocessorDataTransfer Decode(u32 word) {
            CoprocessorDataTransfer cdt {};
            cdt.condition = (Condition)Bit::range(Mask::condition, word);
            // TODO
            return cdt;
        }

        static CoprocessorDataTransfer Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return is_load ? Mnemonic::LDC : Mnemonic::STC; }

        u32           Encode() const {
            // TODO
            return 0;
        }

        std::string Disassemble() const {
            std::stringstream ss;
            // TODO
            ss << to_string(Mnemonic()) << to_string(condition) << " ;TODO";

            return ss.str();
        }
    };

    /**
     * Coprocessor Register Transfer
     */
    struct CoprocessorRegisterTransfer {
        Condition                          condition;
        bool                               is_load;

        static CoprocessorRegisterTransfer Decode(u32 word) {
            CoprocessorRegisterTransfer crt {};
            crt.condition = (Condition)Bit::range(Mask::condition, word);
            // TODO
            return crt;
        }

        static CoprocessorRegisterTransfer Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return is_load ? Mnemonic::MRC : Mnemonic::MCR; }

        u32           Encode() const {
            // TODO
            return 0;
        }

        std::string Disassemble() const {
            std::stringstream ss;
            // TODO
            ss << to_string(Mnemonic()) << to_string(condition) << " ;TODO";

            return ss.str();
        }
    };

    /**
     * Undefined
     */
    struct Undefined {
        Condition        condition;
        u32              word;

        static Undefined Decode(u32 word) {
            Undefined undef {};
            undef.condition = (Condition)Bit::range(Mask::condition, word);
            undef.word      = word;
            return undef;
        }

        static Undefined Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return Mnemonic::Undefined; }

        u32           Encode() const { return word; }

        std::string   Disassemble() const {
            std::stringstream ss;

            ss << "Undefined(" << as_hex(word) << ")" << to_string(condition);

            return ss.str();
        }
    };

    /**
     * Unified Instruction Class
     */
    using Instruction = std::variant<
            Undefined, Branch, BranchAndExchange, DataProcessing, PSRTransfer, Multiply, MultiplyLong,
            SingleDataTransfer, HalfwordDataTransfer, BlockDataTransfer, Swap, SoftwareInterrupt,
            CoprocessorDataOperation, CoprocessorDataTransfer, CoprocessorRegisterTransfer>;

    std::string DisassembleArm(Instruction const &instr);
    std::string DisassembleThumb(Instruction const &instr);

    Instruction AssembleArm(const std::string_view &s);
    Instruction AssembleThumb(const std::string_view &s);

    u32         EncodeArm(Instruction const &instr);
    u16         EncodeThumb(Instruction const &instr);

    Instruction DecodeArm(u32 word);
    Instruction DecodeThumb(u32 word);

};
