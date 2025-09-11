#pragma once

#include <string_view>
#include <sys/types.h>
#include <variant>

#include "util/bit.hpp"
#include "util/decl.hpp"
#include "util/types/primitives.hpp"
#include "util/util.hpp"

namespace Arm {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    struct reg_t {
        u8 v;

        reg_t() :
            v(0) { }

        explicit reg_t(u8 reg, bool high_bit = false) { v = Bit::set_cond(reg, 4, high_bit); }

        operator u8 () const { return v; }

        std::string to_string() const { return "R" + std::to_string(v); }
    };
#pragma clang diagnostic pop

    inline std::ostream &operator<< (std::ostream &o, reg_t r) { return o << r.to_string(); }

    namespace Registers {
        const static reg_t SP = reg_t {13};
        const static reg_t LR = reg_t {14};
        const static reg_t PC = reg_t {15};
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
            bex.rN        = reg_t {(u8)Bit::range(Mask::bex_reg_N, word)};
            return bex;
        }

        static BranchAndExchange DecodeThumb(u16 word) {
            BranchAndExchange bex {};
            bex.condition = Condition::Always;
            bex.rN        = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(3, 5), word), Bit::test(word, 6)};
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

        std::string DisassembleThumb() const {
            std::stringstream ss;

            ss << to_string(Mnemonic()) << " " << rN;
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

        bool          isThumbLongBranch;
        bool          isThumbLongBranchFirstInstruction;

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

        static Branch DecodeThumb(u16 word) {
            if(Bit::range(Bit::Mask::between_inc<u16>(12, 15), word) == 0xD) {
                // conditional Branch

                Branch b {};
                b.condition = (Condition)Bit::range(Bit::Mask::between_inc<u16>(8, 11), word);
                b.link      = false;

                // The branch offset is left shifted by 1 then sign-extended.
                // The offset is 0-10bits + 1 for the shift, making the sign bit 11
                b.offset    = Bit::sign_extend<u16, s32>(Bit::mask<u16>(0xFF, word) << 1, 11);

                return b;
            } else if(Bit::range(Bit::Mask::between_inc<u16>(12, 15), word) == 0xE) {
                // unconditional branch
                Branch b {};
                b.condition = Condition::Always;
                b.link      = false;

                // The branch offset is left shifted by 1 then sign-extended.
                // The offset is 0-10bits + 1 for the shift, making the sign bit 11
                b.offset    = Bit::sign_extend<u16, s32>(Bit::mask(0xFF_u16, word) << 1, 11);

                return b;
            } else if(Bit::range(Bit::Mask::between_inc<u16>(12, 15), word) == 0xF) {
                // long branch/link
                Branch b {};
                b.condition                         = Condition::Always;
                b.link                              = true;
                b.isThumbLongBranch                 = true;
                b.isThumbLongBranchFirstInstruction = Bit::test(word, 11);

                if(b.isThumbLongBranchFirstInstruction) {
                    b.offset = Bit::range(Bit::Mask::between_inc<u16>(0, 10), word);
                } else {
                    b.offset = Bit::range(Bit::Mask::between_inc<u16>(0, 10), word) << 1;
                }

                // The branch offset is left shifted by 1 then sign-extended.
                // The offset is 0-10bits + 1 for the shift, making the sign bit 11
                b.offset = Bit::sign_extend<u16, s32>(Bit::mask(0xFF_u16, word) << 1, 11);

                return b;
            }

            assert(false);
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

        std::string DisassembleThumb() const {
            std::stringstream ss;

            ss << to_string(Mnemonic()) << " ";
            // From the ISA Doc:
            // The branch offset must take account of the prefetch operation, which causes the PC to be 2 words (8
            // bytes) ahead of the current instruction. account for this visually
            ss << (offset + 4);
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
            dp.rN             = reg_t {(u8)Bit::range(BitMask::Reg_N, word)};
            dp.rD             = reg_t {(u8)Bit::range(BitMask::Reg_D, word)};

            if(dp.is_imm) {
                dp.ImmediateOperand.rotate    = Bit::range(BitMask::ImmediateRotation, word);
                dp.ImmediateOperand.immediate = Bit::range(BitMask::ImmediateValue, word);
            } else {
                dp.ShiftedRegisterOperand.is_reg     = Bit::range(BitMask::RegisterShiftValue, word);
                dp.ShiftedRegisterOperand.shift_type = (ShiftType)Bit::range(BitMask::RegisterShiftType, word);
                if(dp.ShiftedRegisterOperand.is_reg) {
                    dp.ShiftedRegisterOperand.rS = reg_t {(u8)Bit::range(BitMask::Reg_S, word)};
                } else {
                    dp.ShiftedRegisterOperand.shift_amount = Bit::range(BitMask::RegisterShiftImmediate, word);
                }
                dp.ShiftedRegisterOperand.rM = reg_t {(u8)Bit::range(BitMask::Reg_M, word)};
            }

            return dp;
        }

        static DataProcessing DecodeThumb(u16 word) {

            switch(Bit::range(Bit::Mask::between_inc<u16>(11, 15), word)) {
            case 0x0:
            case 0x1:
            case 0x2: {
                // move shifted register
                DataProcessing dp {};
                dp.condition      = Condition::Always;
                dp.operation_code = OperationCode::MOV;
                dp.is_imm         = false;
                dp.set_flags      = false;
                dp.rN             = reg_t {};
                dp.rD             = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(0, 2), word), false};
                dp.ShiftedRegisterOperand.is_reg     = false;
                dp.ShiftedRegisterOperand.shift_type = (ShiftType)Bit::range(Bit::Mask::between_inc<u16>(11, 12), word);
                dp.ShiftedRegisterOperand.shift_amount = Bit::range(Bit::Mask::between_inc<u16>(6, 10), word);
                dp.ShiftedRegisterOperand.rM = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(3, 5), word)};

                return dp;
            }
            case 0x3: {
                // add/subtract
                DataProcessing dp {};
                dp.condition      = Condition::Always;
                dp.operation_code = Bit::test(word, 9) ? OperationCode::SUB : OperationCode::ADD;
                dp.is_imm         = Bit::test(word, 10);
                dp.set_flags      = true;
                dp.rN             = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(3, 5), word), false};
                dp.rD             = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(0, 2), word), false};
                if(dp.is_imm) {
                    dp.ImmediateOperand.immediate = Bit::range(Bit::Mask::between_inc<u16>(6, 8), word);
                    dp.ImmediateOperand.rotate    = 0;
                } else {
                    dp.ShiftedRegisterOperand.is_reg       = true;
                    dp.ShiftedRegisterOperand.shift_amount = 0;
                    dp.ShiftedRegisterOperand.rM = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(6, 8), word)};
                };
                return dp;
            }

            case 0x4:
            case 0x5:
            case 0x6:
            case 0x7: {
                // move/compare/add/subtract immediate
                DataProcessing dp {};
                dp.condition = Condition::Always;
                u16 opBits   = Bit::range(Bit::Mask::between_inc<u16>(11, 12), word);
                if(opBits == 0x0) {
                    dp.operation_code = OperationCode::MOV;
                } else if(opBits == 0x1) {
                    dp.operation_code = OperationCode::CMP;
                } else if(opBits == 0x2) {
                    dp.operation_code = OperationCode::ADD;
                } else if(opBits == 0x3) {
                    dp.operation_code = OperationCode::SUB;
                } else {
                    assert(false);
                }
                dp.is_imm                     = true;
                dp.set_flags                  = true;
                dp.rN                         = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(8, 10), word), false};
                dp.rD                         = dp.rN;
                dp.ImmediateOperand.immediate = Bit::range(Bit::Mask::between_inc<u16>(0, 8), word);
                dp.ImmediateOperand.rotate    = 0;
                return dp;
            }
            case 0x8: {
                if(!Bit::test(word, 10)) {
                    // ALU Operations
                    DataProcessing dp {};
                    dp.condition = Condition::Always;
                    dp.is_imm    = false;
                    dp.set_flags = true;
                    reg_t reg1   = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(0, 2), word), false};
                    reg_t reg2 {(u8)Bit::range(Bit::Mask::between_inc<u16>(3, 5), word), false};

                    switch(Bit::range(Bit::Mask::between_inc<u16>(3, 5), word)) {
                    case 0x0: // AND
                        dp.operation_code                      = OperationCode::AND;
                        dp.rD                                  = reg1;
                        dp.rN                                  = reg1;
                        dp.ShiftedRegisterOperand.is_reg       = false;
                        dp.ShiftedRegisterOperand.rM           = reg2;
                        dp.ShiftedRegisterOperand.shift_amount = 0;
                        break;
                    case 0x1: // EOR
                        dp.operation_code                      = OperationCode::EOR;
                        dp.rD                                  = reg1;
                        dp.rN                                  = reg1;
                        dp.ShiftedRegisterOperand.is_reg       = false;
                        dp.ShiftedRegisterOperand.rM           = reg2;
                        dp.ShiftedRegisterOperand.shift_amount = 0;
                        break;
                    case 0x2: // LSL
                        dp.operation_code                    = OperationCode::MOV;
                        dp.rD                                = reg1;
                        dp.ShiftedRegisterOperand.is_reg     = true;
                        dp.ShiftedRegisterOperand.rM         = reg1;
                        dp.ShiftedRegisterOperand.shift_type = ShiftType::LogicalLeft;
                        dp.ShiftedRegisterOperand.rS         = reg2;
                        break;
                    case 0x3: // LSR
                        dp.operation_code                    = OperationCode::MOV;
                        dp.rD                                = reg1;
                        dp.ShiftedRegisterOperand.is_reg     = true;
                        dp.ShiftedRegisterOperand.rM         = reg1;
                        dp.ShiftedRegisterOperand.shift_type = ShiftType::LogicalRight;
                        dp.ShiftedRegisterOperand.rS         = reg2;
                        break;
                    case 0x4: // ASR
                        dp.operation_code                    = OperationCode::MOV;
                        dp.rD                                = reg1;
                        dp.ShiftedRegisterOperand.is_reg     = true;
                        dp.ShiftedRegisterOperand.rM         = reg1;
                        dp.ShiftedRegisterOperand.shift_type = ShiftType::ArithmeticRight;
                        dp.ShiftedRegisterOperand.rS         = reg2;
                        break;
                    case 0x5: // ADC
                        dp.operation_code                      = OperationCode::ADC;
                        dp.rD                                  = reg1;
                        dp.rN                                  = reg1;
                        dp.ShiftedRegisterOperand.is_reg       = false;
                        dp.ShiftedRegisterOperand.rM           = reg2;
                        dp.ShiftedRegisterOperand.shift_amount = 0;
                        break;
                    case 0x6: // SBC
                        dp.operation_code                      = OperationCode::SBC;
                        dp.rD                                  = reg1;
                        dp.rN                                  = reg1;
                        dp.ShiftedRegisterOperand.is_reg       = false;
                        dp.ShiftedRegisterOperand.rM           = reg2;
                        dp.ShiftedRegisterOperand.shift_amount = 0;
                        break;
                    case 0x7: // ROR
                        dp.operation_code                    = OperationCode::MOV;
                        dp.rD                                = reg1;
                        dp.ShiftedRegisterOperand.is_reg     = true;
                        dp.ShiftedRegisterOperand.rM         = reg1;
                        dp.ShiftedRegisterOperand.shift_type = ShiftType::RotateRight;
                        dp.ShiftedRegisterOperand.rS         = reg2;
                        break;
                    case 0x8: // TST
                        dp.operation_code                      = OperationCode::TST;
                        dp.rD                                  = reg1;
                        dp.ShiftedRegisterOperand.is_reg       = false;
                        dp.ShiftedRegisterOperand.rM           = reg2;
                        dp.ShiftedRegisterOperand.shift_amount = 0;
                        break;
                    case 0x9: // NEG
                        dp.operation_code             = OperationCode::RSB;
                        dp.rD                         = reg1;
                        dp.rN                         = reg2;
                        dp.is_imm                     = true;
                        dp.ImmediateOperand.immediate = 0;
                        break;
                    case 0xA: // CMP
                        dp.operation_code                      = OperationCode::CMP;
                        dp.rD                                  = reg1;
                        dp.ShiftedRegisterOperand.is_reg       = false;
                        dp.ShiftedRegisterOperand.rM           = reg2;
                        dp.ShiftedRegisterOperand.shift_amount = 0;
                        break;
                    case 0xB: // CMN
                        dp.operation_code                      = OperationCode::CMN;
                        dp.rD                                  = reg1;
                        dp.ShiftedRegisterOperand.is_reg       = false;
                        dp.ShiftedRegisterOperand.rM           = reg2;
                        dp.ShiftedRegisterOperand.shift_amount = 0;
                        break;
                    case 0xC: // ORR
                        dp.operation_code                      = OperationCode::ORR;
                        dp.rD                                  = reg1;
                        dp.rN                                  = reg1;
                        dp.ShiftedRegisterOperand.is_reg       = false;
                        dp.ShiftedRegisterOperand.rM           = reg2;
                        dp.ShiftedRegisterOperand.shift_amount = 0;
                        break;
                    case 0xD: // MUL
                        // implemented elsewhere
                        assert(false);
                    case 0xE: // BIC
                        dp.operation_code                      = OperationCode::BIC;
                        dp.rD                                  = reg1;
                        dp.rN                                  = reg1;
                        dp.ShiftedRegisterOperand.is_reg       = false;
                        dp.ShiftedRegisterOperand.rM           = reg2;
                        dp.ShiftedRegisterOperand.shift_amount = 0;
                        break;

                    case 0xF: // MVN
                        dp.operation_code                      = OperationCode::MVN;
                        dp.rD                                  = reg1;
                        dp.ShiftedRegisterOperand.is_reg       = false;
                        dp.ShiftedRegisterOperand.rM           = reg2;
                        dp.ShiftedRegisterOperand.shift_amount = 0;
                        break;
                    }

                    return dp;
                } else {
                    // Hi Register Operations
                    DataProcessing dp {};
                    dp.condition                           = Condition::Always;
                    dp.is_imm                              = false;
                    dp.ShiftedRegisterOperand.is_reg       = false;
                    dp.ShiftedRegisterOperand.shift_amount = 0;
                    dp.rD = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(0, 2), word), Bit::test(word, 7)};
                    dp.ShiftedRegisterOperand.rM
                            = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(3, 5), word), Bit::test(word, 8)};

                    u16 op = Bit::range(Bit::Mask::between_inc<u16>(8, 9), word);
                    if(op == 0) {
                        dp.operation_code = OperationCode::ADD;
                        dp.rN             = dp.ShiftedRegisterOperand.rM;
                        dp.set_flags      = false;
                    } else if(op == 1) {
                        dp.operation_code = OperationCode::CMP;
                        dp.set_flags      = true;
                    } else if(op == 2) {
                        dp.operation_code = OperationCode::MOV;
                        dp.set_flags      = false;
                    }

                    return dp;
                }
            }
            case 0x14:
            case 0x15: {
                // load address
                DataProcessing dp {};
                dp.condition                  = Condition::Always;
                dp.is_imm                     = true;
                dp.set_flags                  = false;
                dp.rD                         = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(8, 10), word), false};
                dp.rN                         = Bit::test(word, 11) ? Registers::SP : Registers::PC;
                dp.ImmediateOperand.immediate = Bit::range(Bit::Mask::between_inc<u16>(0, 7), word) << 2;
                dp.ImmediateOperand.rotate    = 0;
                return dp;
            }
            case 0x16: {
                // add offset to stack pointer
                DataProcessing dp {};
                dp.condition                  = Condition::Always;
                dp.is_imm                     = true;
                dp.set_flags                  = false;
                dp.rD                         = Registers::SP;
                dp.rN                         = Registers::SP;
                dp.ImmediateOperand.immediate = Bit::range(Bit::Mask::between_inc<u16>(0, 7), word) << 2;
                dp.ImmediateOperand.rotate    = 0;
                return dp;
            }
            }

            assert(false);
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

        std::string DisassembleThumb() const {
            std::stringstream ss;

            ss << to_string(Mnemonic()) << " " << rD.to_string() << ", ";
            if(Mnemonic() == Mnemonic::ADD) {
                ss << rN << ", ";
            }

            ss << ShiftedRegisterOperand.rM.to_string();

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
                psr.PSRToRegister.rD = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u32>(12, 15), word)};
            } else if(psr_type == Literal::MSR_CONSTANT) {
                psr.type             = Type::RegisterToPSR;
                psr.PSRToRegister.rD = reg_t {(u8)Bit::mask(Bit::Mask::until<u32>(3), word)};
            } else if(psr_type == Literal::MSRF_CONSTANT) {
                psr.type                  = Type::RegisterToPSRF;
                psr.RegisterToPSRF.is_imm = Bit::test(word, 25);
                if(psr.RegisterToPSRF.is_imm) {
                    psr.RegisterToPSRF.ImmediateOperand.rotation = Bit::range(Bit::Mask::between_inc<u32>(8, 11), word);
                    psr.RegisterToPSRF.ImmediateOperand.value    = Bit::mask(Bit::Mask::until_inc<u32>(7), word);
                } else {
                    psr.RegisterToPSRF.RegisterOperand.rM = reg_t {(u8)Bit::mask(Bit::Mask::until_inc<u32>(3), word)};
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

        std::string DisassembleThumb() const {
            std::stringstream ss;
            // TODO
            ss << to_string(Mnemonic()) << to_string(condition) << " ;TODO";
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
            mul.accumulate = Bit::test(word, 21);
            mul.set_flags  = Bit::test(word, 20);
            mul.rD         = reg_t {(u8)Bit::range(Mask::mul_reg_D, word)};
            mul.rN         = reg_t {(u8)Bit::range(Mask::mul_reg_N, word)};
            mul.rS         = reg_t {(u8)Bit::range(Mask::mul_reg_S, word)};
            mul.rM         = reg_t {(u8)Bit::range(Mask::mul_reg_M, word)};
            return mul;
        }

        static Multiply DecodeThumb(u16 word) {
            Multiply mul {};
            mul.condition  = Condition::Always;
            mul.accumulate = false;
            mul.set_flags  = true;
            mul.rD         = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(0, 2), word), false};
            mul.rM         = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(3, 5), word), false};
            mul.rS         = mul.rD;

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

        std::string DisassembleThumb() const { return this->Disassemble(); };
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
            mul.rDHi       = reg_t {(u8)Bit::range(Mask::mul_reg_D, word)};
            mul.rDLo       = reg_t {(u8)Bit::range(Mask::mul_reg_N, word)};
            mul.rS         = reg_t {(u8)Bit::range(Mask::mul_reg_S, word)};
            mul.rM         = reg_t {(u8)Bit::range(Mask::mul_reg_M, word)};
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

        std::string DisassembleThumb() const { assert(false); };
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
            sdt.rN         = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u32>(16, 19), word)};
            sdt.rD         = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u32>(12, 15), word)};
            if(sdt.is_imm) {
                sdt.ImmediateOperand.offset = Bit::mask(Bit::Mask::until_inc<u32>(11), word);
            } else {
                sdt.ShiftedRegisterOperand.shift_amount = Bit::mask(Bit::Mask::between_inc<u32>(4, 11), word);
                sdt.ShiftedRegisterOperand.rM           = reg_t {(u8)Bit::mask(Bit::Mask::until_inc<u32>(3), word)};
            }
            return sdt;
        }

        static SingleDataTransfer DecodeThumb(u16 word) {
            if(Bit::range(Bit::Mask::between_inc<u16>(11, 15), word) == 0b01001) {
                // PC relative load
                SingleDataTransfer sdt {};
                sdt.condition               = Condition::Always;
                sdt.is_imm                  = true;
                sdt.is_pre_idx              = true;
                sdt.is_inc                  = true;
                sdt.is_byte                 = false;
                sdt.write_back              = false;
                sdt.load                    = true;
                sdt.rN                      = Registers::PC;
                sdt.rD                      = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(8, 10), word), false};
                sdt.ImmediateOperand.offset = Bit::mask(Bit::Mask::until_inc<u16>(7), word) << 2;
                return sdt;
            } else if(Bit::range(Bit::Mask::between_inc<u16>(12, 15), word) == 0b0101 && !Bit::test(word, 9)) {
                // LS with register offset
                SingleDataTransfer sdt {};
                sdt.condition  = Condition::Always;
                sdt.is_imm     = true;
                sdt.is_pre_idx = true;
                sdt.is_inc     = true;
                sdt.is_byte    = Bit::test(word, 10);
                sdt.write_back = false;
                sdt.load       = Bit::test(word, 11);
                sdt.rN         = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(3, 5), word), false};
                sdt.rD         = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(0, 2), word), false};
                sdt.ShiftedRegisterOperand.shift_amount = 0;
                sdt.ShiftedRegisterOperand.rM = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(6, 8), word), false};
                return sdt;
            } else if(Bit::range(Bit::Mask::between_inc<u16>(13, 15), word) == 0b011) {
                // LS with immediate offset
                SingleDataTransfer sdt {};
                sdt.condition               = Condition::Always;
                sdt.is_imm                  = true;
                sdt.is_pre_idx              = true;
                sdt.is_inc                  = true;
                sdt.is_byte                 = Bit::test(word, 12);
                sdt.write_back              = false;
                sdt.load                    = Bit::test(word, 11);
                sdt.rN                      = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(3, 5), word), false};
                sdt.rD                      = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(0, 2), word), false};
                sdt.ImmediateOperand.offset = Bit::mask(Bit::Mask::between_inc<u16>(6, 10), word);
                if(!sdt.is_byte) {
                    sdt.ImmediateOperand.offset <<= 2;
                }
            }

            assert(false);
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

        std::string DisassembleThumb() const { return this->Disassemble(); };
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

        static HalfwordDataTransfer Decode(u32 word) {
            HalfwordDataTransfer hdt {};
            hdt.condition  = (Condition)Bit::range(Mask::condition, word);
            hdt.is_pre_idx = Bit::test(word, 24);
            hdt.is_inc     = Bit::test(word, 23);
            hdt.is_imm     = Bit::test(word, 22);
            hdt.write_back = Bit::test(word, 21);
            hdt.load       = Bit::test(word, 20);
            hdt.rN         = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u32>(19, 16), word)};
            hdt.rD         = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u32>(15, 12), word)};
            assert(Bit::range(Bit::Mask::between_inc<u32>(5, 6), word) != 0);
            hdt.signed_data = Bit::test(word, 6);
            hdt.halfwords   = Bit::test(word, 5);
            if(hdt.is_imm) {
                hdt.ImmediateOperand.offset = (Bit::range(Bit::Mask::between_inc<u32>(11, 8), word) << 4)
                                            | Bit::mask(Bit::Mask::until_inc<u32>(3), word);
            } else {
                hdt.RegisterOperand.rM = reg_t {(u8)Bit::range(Bit::Mask::until_inc<u32>(3), word)};
            }
            return hdt;
        }

        static HalfwordDataTransfer DecodeThumb(u16 word) {
            u8 bits = Bit::range(Bit::Mask::between_inc<u16>(12, 15), word);
            if(bits == 0b0101 && Bit::test(word, 9)) {
                // LS Sign-Extended byte/halfword
                HalfwordDataTransfer hdt {};
                hdt.condition  = Condition::Always;
                hdt.is_pre_idx = true;
                hdt.is_inc     = true;
                hdt.is_imm     = false;
                hdt.write_back = false;
                if(Bit::test(word, 10)) {
                    hdt.load        = Bit::test(word, 11);
                    hdt.signed_data = false;
                    hdt.halfwords   = true;
                } else {
                    hdt.load        = true;
                    hdt.signed_data = true;
                    hdt.halfwords   = Bit::test(word, 11);
                }

                hdt.rD                 = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(0, 2), word)};
                hdt.rN                 = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(3, 5), word)};
                hdt.RegisterOperand.rM = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(6, 8), word)};
                return hdt;

            } else if(bits == 0b1000) {
                // LS halfword
                HalfwordDataTransfer hdt {};
                hdt.condition               = Condition::Always;
                hdt.is_pre_idx              = true;
                hdt.is_inc                  = true;
                hdt.is_imm                  = true;
                hdt.write_back              = false;
                hdt.load                    = Bit::test(word, 11);
                hdt.signed_data             = false;
                hdt.halfwords               = false;
                hdt.rD                      = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(0, 2), word)};
                hdt.rN                      = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u16>(3, 5), word)};
                hdt.ImmediateOperand.offset = Bit::range(Bit::Mask::between_inc<u16>(6, 10), word) << 1;
                return hdt;
            }

            assert(false);
        }

        static HalfwordDataTransfer Assemble(const std::string_view &s) {
            // TODO
        }

        enum Mnemonic Mnemonic() const { return load ? Mnemonic::LDR : Mnemonic::STR; }

        u32           Encode() const {
            u8 highNibble = (is_imm ? (ImmediateOperand.offset >> 5) : 0) & 0xF;
            u8 lowNibble  = (is_imm ? (ImmediateOperand.offset >> 1) : (u8)RegisterOperand.rM) & 0xF;
            return 0_u32 | (u8)condition << 28 | Bit::from_bool(is_pre_idx) << 24 | Bit::from_bool(is_inc) << 23
                 | Bit::from_bool(write_back) << 21 | Bit::from_bool(load) << 20 | (u8)rN << 16 | (u8)rD << 12
                 | highNibble << 8 | Bit::Mask::bit<u32>(7) | Bit::from_bool(signed_data) << 6
                 | Bit::from_bool(halfwords) << 5 | Bit::Mask::bit<u32>(4) | lowNibble;
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

        std::string DisassembleThumb() const { return this->Disassemble(); };
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
            bdt.rN         = reg_t {(u8)Bit::range(Bit::Mask::between_inc<u32>(19, 16), word)};
            bdt.registers  = Bit::mask(Bit::Mask::until_inc<u32>(15), word);
        }

        static BlockDataTransfer DecodeThumb(u16 word) {
            u8 thumb_id = Bit::range<u16>(0xF000, word);
            if(thumb_id == 0b1011) {
                // push/pop
                BlockDataTransfer bdt {};
                bdt.condition  = Condition::Always;
                bool load      = Bit::test(word, 11);
                bool set_pc_lr = Bit::test(word, 8);
                bdt.is_pre_idx = load;
                bdt.is_inc     = load;
                bdt.load_psr   = false;
                bdt.write_back = true;
                bdt.load       = load;
                bdt.rN         = Registers::SP;
                bdt.registers  = Bit::range<u16>(0xF, word);
                if(set_pc_lr) {
                    Bit::set(bdt.registers, load ? 15 : 14);
                }

                return bdt;
            } else if(thumb_id == 0b1100) {
                // multiple LS
                BlockDataTransfer bdt {};
                bdt.condition  = Condition::Always;
                bool load      = Bit::test(word, 11);
                bdt.is_pre_idx = load;
                bdt.is_inc     = load;
                bdt.load_psr   = false;
                bdt.write_back = true;
                bdt.load       = load;
                bdt.rN         = reg_t {(u8)Bit::range<u16>(0x0F00, word)};
                bdt.registers  = Bit::range<u16>(0xFF, word);
            }

            assert(false);
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
            ss << to_string(Mnemonic()) << to_string(condition) << " {";

            // loop through the register bitfield and print set bits
            for(u8 i = 0; i < 16; ++i) {
                if(Bit::test(registers, i)) {
                    reg_t r {i};
                    ss << r.to_string() << ", ";
                }
            }

            ss << "}";

            return ss.str();
        }

        std::string DisassembleThumb() const {
            std::stringstream ss;

            if(rN == Registers::SP) {
                if(load) {
                    ss << "PUSH";
                } else {
                    ss << "POP";
                }
            } else {
                ss << to_string(Mnemonic());
            }
            ss << " ";

            // loop through the register bitfield and print set bits
            for(u8 i = 0; i < 16; ++i) {
                if(Bit::test(registers, i)) {
                    reg_t r {i};
                    ss << r.to_string() << ", ";
                }
            }
            return ss.str();
        };
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
            swp.rN        = reg_t {(u8)Bit::range(word, Bit::Mask::between<u32>(16, 19))};
            swp.rD        = reg_t {(u8)Bit::range(word, Bit::Mask::between<u32>(16, 19))};
            swp.rM        = reg_t {(u8)Bit::range(word, Bit::Mask::between<u32>(16, 19))};
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

        std::string DisassembleThumb() const {
            std::stringstream ss;
            // TODO
            ss << to_string(Mnemonic()) << to_string(condition) << " ;TODO";
            return ss.str();
        };
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

        static SoftwareInterrupt DecodeThumb(u16 word) {
            SoftwareInterrupt swi {};
            swi.condition = Condition::Always;
            swi.value     = Bit::range<u16>(0x0FF, word);
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
            ss << to_string(Mnemonic()) << to_string(condition) << " " << value;

            return ss.str();
        }

        std::string DisassembleThumb() const {
            std::stringstream ss;

            ss << to_string(Mnemonic()) << " " << value;
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

        std::string DisassembleThumb() const {
            std::stringstream ss;
            // TODO
            ss << to_string(Mnemonic()) << to_string(condition) << " ;TODO";
            return ss.str();
        };
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

        std::string DisassembleThumb() const {
            std::stringstream ss;
            // TODO
            ss << to_string(Mnemonic()) << to_string(condition) << " ;TODO";
            return ss.str();
        };
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

        std::string DisassembleThumb() const {
            std::stringstream ss;
            // TODO
            ss << to_string(Mnemonic()) << to_string(condition) << " ;TODO";
            return ss.str();
        };
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

        std::string DisassembleThumb() const {
            std::stringstream ss;
            ss << "Undefined(" << as_hex(word) << ")";
            return ss.str();
        };
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
