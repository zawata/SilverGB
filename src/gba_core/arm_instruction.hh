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

    reg_t() : v(0) {}
    [[maybe_unused]] reg_t(u8 v) : v(v) {}


    operator u8() const { return v; }

    std::string to_string() const { return "R" + std::to_string(v); }
  };
#pragma clang diagnostic pop

  inline std::ostream &operator<<(std::ostream &o, reg_t r) {
    return o << r.to_string();
  }

  namespace Mask {
    using namespace Bit::Mask;

    // all
    const static u32 condition = between_inc<u32>(28, 31);
    // branch and exchange
    const static u32 bex_const = between_inc<u32>(4, 27);
    const static u32 bex_reg_N = until_inc<u32>(3);
    // branch
    const static u32 br_is_link = bit<u32>(24);
    const static u32 br_offset = until_inc<u32>(23);
    // multiply
    const static u32 mul_accum = bit<u32>(21);
    const static u32 mul_reg_D = between_inc<u32>(16, 19);
    const static u32 mul_reg_N = between_inc<u32>(12, 15);
    const static u32 mul_reg_S = between_inc<u32>(8, 11);
    const static u32 mul_reg_M = between_inc<u32>(0, 3);
    // software interrupt
    const static u32 swi_comment = 0x00FFFFFF;
    // Coprocessor
    const static u32 cp_is_crt = bit<u32>(4);

  }// namespace Mask

  namespace Literal {
    const static u32 MUL_CONSTANT = 0b1001;
    const static u32 BEX_CONSTANT = 0b000100101111111111110001;
    const static u32 MRS_CONSTANT = 0b001111;
    const static u32 MSR_CONSTANT = 0b101001;
    const static u32 MSRF_CONSTANT = 0b101000;
  }// namespace Literal

  enum struct Mnemonic {
    ADC, // Add with carry
    ADD, // Add
    AND, // AND
    B,   // Branch
    BIC, // Bit Clear
    BL,  // Branch with Link
    BX,  // Branch and Exchange
    CDP, // Coprocessor Data Processing
    CMN, // Compare Negative
    CMP, // Compare
    EOR, // Exclusive OR
    LDC, // Load coprocessor from memory
    LDM, // Load multiple registers
    LDR, // Load register From memory
    MCR, // Move CPU register to coprocessor register
    MLA, // Multiply and Accumulate
    MLAL,// Multiply and Accumulate Long
    MOV, // Move register or constant
    MRC, // Move from coprocessor register to CPU register
    MRS, // Move PSR status/flags to register
    MSR, // Move register to PSR status/flags
    MUL, // Multiply
    MULL,// Multiply Long
    MVN, // Move negative register
    ORR, // OR
    RSB, // Reverse Subtract
    RSC, // Reverse Subtract with Carry
    SBC, // Subtract with Carry
    STC, // Store coprocessor register to memory
    STM, // Store Multiple
    STR, // Store register to memory
    SUB, // Subtract
    SWI, // Software Interrupt
    SWP, // Swap register with memory
    TEQ, // Test bitwise equality
    TST, // Test bits
    Undefined,
  };

  inline std::string to_string(Mnemonic mnemonic) {
    static const char *mnemonic_strings[] = {
        "ADC", "ADD", "AND", "B", "BIC", "BL", "BX", "CDP", "CMN", "CMP",
        "EOR", "LDC", "LDM", "LDR", "MCR", "MLA", "MLAL", "MOV", "MRC", "MRS",
        "MSR", "MUL", "MLAL", "MVN", "ORR", "RSB", "RSC", "SBC", "STC", "STM",
        "STR", "SUB", "SWI", "SWP", "TEQ", "TST", "---"};
    static_assert(array_length(mnemonic_strings) == (int) Mnemonic::Undefined + 1);
    return mnemonic_strings[(int) mnemonic];
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
    static const char *condition_strings[] = {
      "EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC",
      "HI", "LS", "GE", "LT", "GT", "LE", "", "NV"
    };
    return condition_strings[(u8) condition];
  }

  enum struct ShiftType : u8 {
    LogicalLeft,
    LogicalRight,
    ArithmeticRight,
    RotateRight
  };

  inline std::string to_string(ShiftType shiftType) {
    static const char *shift_strings[] = {
        "LSL", "LSR", "ASR", "ROR"};
    return shift_strings[(u8) shiftType];
  }

  enum struct InstructionType {
    Undefined,
    Branch,
    BranchAndExchange,
    DataProcessing,
    PSRTransfer,
    Multiply,
    MultiplyLong,
    SingleDataTransfer,
    HalfwordDataTransfer,
    BlockDataTransfer,
    Swap,
    SoftwareInterrupt,
    CoprocessorDataOperation,
    CoprocessorDataTransfer,
    CoprocessorRegisterTransfer,
  };

  struct Instruction;
  class InstructionDataBase {
  public:
    virtual ~InstructionDataBase() = default;

  private:
    friend struct Instruction;
    virtual enum Mnemonic Mnemonic() const = 0;
    virtual u32 Encode() const = 0;
    virtual std::string Disassemble() const = 0;
  };

  /**
   * Branch And Exchange
   * BX
   */
  struct BranchAndExchange : private InstructionDataBase {
    Condition condition;
    reg_t rN;

  private:
    friend struct Instruction;
    explicit BranchAndExchange(u32 word) {
      condition = (Condition) Bit::range<u32>(Mask::condition, word);
      rN = (u8) Bit::range(Mask::bex_reg_N, word);
    }

    explicit BranchAndExchange(const std::string_view &s) {
      // TODO
    }

    enum Mnemonic Mnemonic() const override {
      return Mnemonic::BX;
    }
    static constexpr enum InstructionType Type() { return InstructionType::BranchAndExchange; }

    u32 Encode() const override {
      return 0_u32 | (u8) condition << 28 | Literal::BEX_CONSTANT << 4 | (rN & Mask::bex_reg_N);
    }

    std::string Disassemble() const override {
      std::stringstream ss;

      ss << to_string(Mnemonic()) << to_string(condition) << " " << rN;
      return ss.str();
    };
  };

  /**
   * Branch
   * B, BL
   */
  struct Branch : private InstructionDataBase {
    Condition condition;
    bool link;
    s32 offset;

  private:
    friend struct Instruction;

    explicit Branch(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      link = (bool) Bit::range(Mask::br_is_link, word);

      u32 raw_offset = Bit::mask(Mask::br_offset, word);

      // The branch offset is left shifted by 2 then sign-extended.
      // The offset is 0-23bits + 2 for the shift, making the sign bit 25
      offset = Bit::sign_extend(raw_offset << 2, 25);
    }

    explicit Branch(const std::string_view &s) {
      // TODO
    }

    enum Mnemonic Mnemonic() const override {
      return link ? Mnemonic::BL : Mnemonic::B;
    }

    u32 Encode() const override {
      return 0_u32 | (u8) condition << 28 | 0b101 << 25 | (link ? 1 : 0) << 24 | Bit::sign_compress<u32>(offset & Mask::br_offset, 25) >> 2;
    }

    std::string Disassemble() const override {
      std::stringstream ss;

      ss << to_string(Mnemonic()) << to_string(condition) << " ";
      // From the ISA Doc:
      // The branch offset must take account of the prefetch operation, which causes the PC to be 2 words (8 bytes) ahead of the current instruction.
      // account for this visually
      ss << (offset + 8);
      return ss.str();
    };

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::Branch;
    }
  };

  /**
   * Data Processing
   */
  struct DataProcessing : private InstructionDataBase {
    Condition condition;
    struct BitMask {
      __disallow_construct(BitMask);

      const static u32 isImmediate = Bit::Mask::bit<u32>(25);
      const static u32 isSetFlags = Bit::Mask::bit<u32>(20);
      const static u32 OpCode = 0x03F00000;
      const static u32 Reg_N = 0x000F0000;
      const static u32 Reg_D = 0x0000F000;
      const static u32 RegisterShiftValue = Bit::Mask::bit<u32>(4);
      const static u32 RegisterShiftType = 0x00000060;
      const static u32 Reg_M = 0x0000000F;
      const static u32 RegisterShiftImmediate = 0x00000F80;
      const static u32 Reg_S = 0x00000F00;
      const static u32 ImmediateRotation = 0x00000F00;
      const static u32 ImmediateValue = 0x000000FF;
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

    bool is_imm;
    bool set_flags;
    reg_t rN, rD;
    union {
      struct {
        bool is_reg;
        ShiftType shift_type;
        union {
          reg_t rS;
          u8 shift_amount;
        };
        reg_t rM;
      } ShiftedRegisterOperand {};
      struct {
        u8 rotate;
        u8 immediate;
      } ImmediateOperand;
    };

  private:
    friend struct Instruction;
    // Decode
    explicit DataProcessing(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      operation_code = (OperationCode) Bit::range(BitMask::OpCode, word);
      is_imm = Bit::range(BitMask::isImmediate, word);
      set_flags = Bit::range(BitMask::isSetFlags, word);
      rN = Bit::range(BitMask::Reg_N, word);
      rD = Bit::range(BitMask::Reg_D, word);

      if (is_imm) {
        ImmediateOperand.rotate = Bit::range(BitMask::ImmediateRotation, word);
        ImmediateOperand.immediate = Bit::range(BitMask::ImmediateValue, word);
      } else {
        ShiftedRegisterOperand.is_reg = Bit::range(BitMask::RegisterShiftValue, word);
        ShiftedRegisterOperand.shift_type =
            (ShiftType) Bit::range(BitMask::RegisterShiftType, word);
        if (ShiftedRegisterOperand.is_reg) {
          ShiftedRegisterOperand.rS = Bit::range(BitMask::Reg_S, word);
        } else {
          ShiftedRegisterOperand.shift_amount =
              Bit::range(BitMask::RegisterShiftImmediate, word);
        }
        ShiftedRegisterOperand.rM = Bit::range(BitMask::Reg_M, word);
      }
    }

    // Assemble
    explicit DataProcessing(const std::string_view &s) {
      // TODO
    }

    enum Mnemonic Mnemonic() const override {
      using I = enum Mnemonic;
      static const enum Mnemonic instr_tbl[] = {
          I::AND, I::EOR, I::SUB, I::RSB, I::ADD, I::ADC, I::SBC, I::RSC,
          I::TST, I::TEQ, I::CMP, I::CMN, I::ORR, I::MOV, I::BIC, I::MVN};

      return instr_tbl[(int) operation_code];
    }

    u32 Encode() const override {
      u16 op2 = 0;
      if (is_imm) {
        op2 = ImmediateOperand.rotate << 8 | ImmediateOperand.immediate;
      } else {
        if (ShiftedRegisterOperand.is_reg) {
          op2 = ShiftedRegisterOperand.rS << 8 |
                (u8) ShiftedRegisterOperand.shift_type << 5 | 1 << 4;
        } else {
          op2 = ShiftedRegisterOperand.shift_amount << 7 |
                (u8) ShiftedRegisterOperand.shift_type << 5;
        }
        op2 |= ShiftedRegisterOperand.rM;
      }

      return 0_u32 | (u8) condition << 28 | Bit::from_bool(is_imm) << 25 |
             (u8) operation_code << 21 | Bit::from_bool(set_flags) << 20 |
             (u8) rN << 16 | (u8) rN << 12 | op2;
    }

    std::string Disassemble() const override {
      std::stringstream ss;

      bool opcode_produces_result = operation_code != OperationCode::CMP &&
                                    operation_code != OperationCode::CMN &&
                                    operation_code != OperationCode::TEQ &&
                                    operation_code != OperationCode::TST;
      bool opcode_uses_2op = operation_code != OperationCode::MOV &&
                             operation_code != OperationCode::MVN;

      // these instructions set flags implicitly and thus, the S is omitted
      bool show_S = set_flags && opcode_produces_result;

      ss << to_string(Mnemonic()) << to_string(condition) << (show_S ? "S" : "")
         << " ";

      if (opcode_produces_result) {
        ss << rD.to_string() << ", ";
      }

      if (opcode_uses_2op) {
        ss << rN.to_string() << ", ";
      }

      // TODO: op2

      return ss.str();
    };

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::DataProcessing;
    }
  };

  /**
   * PSR Transfer
   */
  struct PSRTransfer : private InstructionDataBase {
    Condition condition;
    bool use_spsr;
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

  private:
    friend struct Instruction;
    explicit PSRTransfer(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      use_spsr = Bit::test(word, 22);
      u32 psr_type = Bit::range(Bit::Mask::between_inc<u32>(16, 21), word);
      if (psr_type == Literal::MRS_CONSTANT) {
        type = Type::PSRToRegister;
        PSRToRegister.rD = Bit::range(Bit::Mask::between_inc<u32>(12, 15), word);
      } else if (psr_type == Literal::MSR_CONSTANT) {
        type = Type::RegisterToPSR;
        PSRToRegister.rD = Bit::mask(Bit::Mask::until<u32>(3), word);
      } else if (psr_type == Literal::MSRF_CONSTANT) {
        type = Type::RegisterToPSRF;
        RegisterToPSRF.is_imm = Bit::test(word, 25);
        if (RegisterToPSRF.is_imm) {
          RegisterToPSRF.ImmediateOperand.rotation = Bit::range(Bit::Mask::between_inc<u32>(8, 11), word);
          RegisterToPSRF.ImmediateOperand.value = Bit::mask(Bit::Mask::until_inc<u32>(7), word);
        } else {
          RegisterToPSRF.RegisterOperand.rM = Bit::mask(Bit::Mask::until_inc<u32>(3), word);
        }
      }
    }

    explicit PSRTransfer(const std::string_view &s) {
      //TODO
    }

    enum Mnemonic Mnemonic() const override {
      return type == Type::PSRToRegister ? Mnemonic::MRS : Mnemonic::MSR;
    }

    u32 Encode() const override {
      bool imm_bit = type == Type::RegisterToPSRF && RegisterToPSRF.is_imm;
      static u32 type_const_tbl[] = {
          Literal::MRS_CONSTANT, Literal::MSR_CONSTANT,
          Literal::MSRF_CONSTANT};

      u16 lower_half = 0;
      switch (type) {
      case Type::PSRToRegister:
        lower_half = PSRToRegister.rD << 12;
        break;
      case Type::RegisterToPSR:
        lower_half = 0xF000 | RegisterToPSR.rM;
        break;
      case Type::RegisterToPSRF:
        lower_half = 0xF000;
        if (RegisterToPSRF.is_imm) {
          lower_half |= RegisterToPSRF.RegisterOperand.rM;
        } else {
          lower_half |= RegisterToPSRF.ImmediateOperand.rotation << 8 |
                        RegisterToPSRF.ImmediateOperand.value;
        }
        break;
      }

      return 0_u32 | (u8) condition << 28 | Bit::from_bool(imm_bit) << 25 | 1 << 24 | Bit::from_bool(use_spsr) << 22 | type_const_tbl[(int) type] << 16 | lower_half;
    }

    std::string Disassemble() const override {
      std::stringstream ss;

      // ss << to_string(Mnemonic()) << to_string(condition) << " " <<
      // as_hex(offset);
      return ss.str();
    };

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::PSRTransfer;
    }
  };

  /**
   * Multiply and Multiply and Accumulate
   */
  struct Multiply : private InstructionDataBase {
    Condition condition;
    bool accumulate;
    bool set_flags;
    reg_t rD {}, rN {}, rS {}, rM {};

  private:
    friend struct Instruction;
    explicit Multiply(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      accumulate = Bit::range(Mask::mul_accum, word);

      set_flags = Bit::range(Mask::mul_accum, word);
      rD = Bit::range(Mask::mul_reg_D, word);
      rN = Bit::range(Mask::mul_reg_N, word);
      rS = Bit::range(Mask::mul_reg_S, word);
      rM = Bit::range(Mask::mul_reg_M, word);
    }

    explicit Multiply(const std::string_view &s) {
      //TODO
    }

    enum Mnemonic Mnemonic() const override {
      return accumulate ? Mnemonic::MLA : Mnemonic::MUL;
    }

    u32 Encode() const override {
      return 0_u32 | (u8) condition << 28 | Bit::from_bool(accumulate) << 21 | Bit::from_bool(set_flags) << 20 | (u8) rD << 16 | (u8) rN << 12 | (u8) rS << 8 | (u8) Literal::MUL_CONSTANT << 4 | (u8) rD;
    }

    std::string Disassemble() const override {
      std::stringstream ss;

      ss << to_string(Mnemonic()) << to_string(condition) << (set_flags ? "S" : "")
         << " ";
      ss << rD.to_string() << ", " << rM.to_string() << ", " << rS.to_string();
      if (accumulate) {
        ss << ", " << rN.to_string();
      }
      return ss.str();
    };

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::Multiply;
    }
  };

  /**
   * Multiply Long and Multiply Long and Accumulate
   */
  struct MultiplyLong : private InstructionDataBase {
    Condition condition;
    bool _signed;
    bool accumulate;
    bool set_flags;
    reg_t rDHi, rDLo, rS, rM;

  private:
    friend struct Instruction;
    explicit MultiplyLong(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      _signed = Bit::test(word, 22);
      accumulate = Bit::test(word, 21);
      set_flags = Bit::test(word, 20);
      rDHi = Bit::range(Mask::mul_reg_D, word);
      rDLo = Bit::range(Mask::mul_reg_N, word);
      rS = Bit::range(Mask::mul_reg_S, word);
      rM = Bit::range(Mask::mul_reg_M, word);
    }

    explicit MultiplyLong(const std::string_view &s) {
      //TODO
    }

    enum Mnemonic Mnemonic() const override {
      return accumulate ? Mnemonic::MLAL : Mnemonic::MULL;
    }

    u32 Encode() const override {
      return 0_u32 | (u8) condition << 28 | 1 << 23 | Bit::from_bool(_signed) << 22 | Bit::from_bool(accumulate) << 21 | Bit::from_bool(set_flags) << 20 | (u8) rDHi << 16 | (u8) rDLo << 12 | (u8) rS << 8 | (u8) Literal::MUL_CONSTANT << 4 | (u8) rM;
    }

    std::string Disassemble() const override {
      std::stringstream ss;
      ss << (_signed ? "U" : "S");
      ss << to_string(Mnemonic()) << to_string(condition) << (set_flags ? "S" : "")
         << " ";
      ss << rDLo.to_string() << ", " << rDHi.to_string() << ", " << rM.to_string()
         << ", " << rS.to_string();
      return ss.str();
    };

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::MultiplyLong;
    }
  };

  /**
   * Single Data Transfer
   */
  struct SingleDataTransfer : private InstructionDataBase {
    Condition condition;
    bool is_imm;
    bool is_pre_idx;
    bool is_inc;
    bool is_byte;
    bool write_back;
    bool load;
    reg_t rN {}, rD {};
    union {
      struct {
        ShiftType shift_type;
        u8 shift_amount;
        reg_t rM;
      } ShiftedRegisterOperand;
      struct {
        u16 offset;
      } ImmediateOperand {};
    };


  private:
    friend struct Instruction;
    explicit SingleDataTransfer(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      is_imm = Bit::test(word, 25);
      is_pre_idx = Bit::test(word, 24);
      is_inc = Bit::test(word, 23);
      is_byte = Bit::test(word, 22);
      write_back = Bit::test(word, 21);
      load = Bit::test(word, 20);
      rN = Bit::range(Bit::Mask::between_inc<u32>(16, 19), word);
      rD = Bit::range(Bit::Mask::between_inc<u32>(12, 15), word);
      if (is_imm) {
        ImmediateOperand.offset = Bit::mask(Bit::Mask::until_inc<u32>(11), word);
      } else {
        // shift-reg not allowed for this instruction class
        assert(Bit::test(word, 4) == 0);

        ShiftedRegisterOperand.shift_type = (ShiftType) Bit::range(Bit::Mask::between_inc<u32>(5, 6), word);
        ShiftedRegisterOperand.shift_amount = Bit::mask(Bit::Mask::between_inc<u32>(4, 11), word);
        ShiftedRegisterOperand.rM = Bit::mask(Bit::Mask::until_inc<u32>(3), word);
      }
    }

    explicit SingleDataTransfer(const std::string_view &s) {
      // TODO
    }

    enum Mnemonic Mnemonic() const override {
      return load ? Mnemonic::LDR : Mnemonic::STR;
    }

    u32 Encode() const override {
      // TODO
      return 0;
    }

    std::string Disassemble() const override {
      std::stringstream ss;
      ss << to_string(Mnemonic()) << to_string(condition) << (is_byte ? "B" : "") << (!is_pre_idx && write_back ? "T" : "")
         << " ";
      ss << rD.to_string() << ", ";

      ss << "[" << rN.to_string();
      if (!is_pre_idx) {
        ss << ']';
      }

      if (is_imm) {
        if (ImmediateOperand.offset == 0) {
          ss << (u32) ImmediateOperand.offset;
        } else {
          ss << "]";
        }
      } else {
        ss << (is_inc ? "+" : "-") << ShiftedRegisterOperand.rM.to_string() << to_string(ShiftedRegisterOperand.shift_type);
      }

      if (is_pre_idx) {
        ss << ']';
      }

      return ss.str();
    }

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::SingleDataTransfer;
    }
  };

  /**
   * Halfword and Signed Data Transfer
   */
  struct HalfwordDataTransfer : private InstructionDataBase {
    Condition condition;
    bool is_pre_idx;
    bool is_inc;
    bool is_imm;
    bool write_back;
    bool load;
    reg_t rN, rD;
    bool signed_data;
    bool halfwords;
    union {
      struct {
        reg_t rM;
      } RegisterOperand;
      struct {
        u8 offset;
      } ImmediateOperand {};
    };
    reg_t rM;

  private:
    friend struct Instruction;
    explicit HalfwordDataTransfer(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      is_pre_idx = Bit::test(word, 24);
      is_inc = Bit::test(word, 23);
      is_imm = Bit::test(word, 22);
      write_back = Bit::test(word, 21);
      load = Bit::test(word, 20);
      rN = Bit::range(Bit::Mask::between_inc<u32>(19, 16), word);
      rD = Bit::range(Bit::Mask::between_inc<u32>(15, 12), word);
      assert(Bit::range(Bit::Mask::between_inc<u32>(5, 6), word) != 0);
      signed_data = Bit::test(word, 6);
      halfwords = Bit::test(word, 5);
      if (is_imm) {
        ImmediateOperand.offset = (Bit::range(Bit::Mask::between_inc<u32>(11, 8), word) << 4) | Bit::mask(Bit::Mask::until_inc<u32>(3), word);
      } else {
        rM = Bit::range(Bit::Mask::until_inc<u32>(3), word);
      }
    }

    explicit HalfwordDataTransfer(const std::string_view &s) {
      //TODO
    }

    enum Mnemonic Mnemonic() const override {
      return load ? Mnemonic::LDR : Mnemonic::STR;
    }

    u32 Encode() const override {
      return 0_u32 | (u8) condition << 28 | Bit::from_bool(is_pre_idx) << 24 | Bit::from_bool(is_inc) << 23 | Bit::from_bool(write_back) << 21 | Bit::from_bool(load) << 20 | (u8) rN << 16 | (u8) rD << 12 | Bit::Mask::bit<u32>(7) | Bit::from_bool(signed_data) | Bit::from_bool(halfwords) | Bit::Mask::bit<u32>(4) | (u8) rM;
    }

    std::string Disassemble() const override {
      std::stringstream ss;
      ss << to_string(Mnemonic()) << to_string(condition) << (signed_data ? "S" : "") << (halfwords ? "H" : "B")
         << " ";
      ss << rD.to_string() << ", ";

      ss << "[" << rN.to_string();
      if (!is_pre_idx) {
        ss << ']';
      }

      if (is_imm) {
        if (ImmediateOperand.offset == 0) {
          ss << (u32) ImmediateOperand.offset;
        } else {
          ss << "]";
        }
      } else {
        ss << (is_inc ? "+" : "-") << RegisterOperand.rM.to_string();
      }

      if (is_pre_idx) {
        ss << ']';
      }
      return ss.str();
    }

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::HalfwordDataTransfer;
    }
  };

  /**
   * Block Data Transfer
   */
  struct BlockDataTransfer : private InstructionDataBase {
    Condition condition;
    bool is_pre_idx;
    bool is_inc;
    bool load_psr;
    bool write_back;
    bool load;
    reg_t rN;
    u16 registers;

  private:
    friend struct Instruction;
    explicit BlockDataTransfer(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      is_pre_idx = Bit::test(word, 24);
      is_inc = Bit::test(word, 23);
      load_psr = Bit::test(word, 22);
      write_back = Bit::test(word, 21);
      load = Bit::test(word, 20);
      rN = Bit::range(Bit::Mask::between_inc<u32>(19, 16), word);
      registers = Bit::mask(Bit::Mask::until_inc<u32>(15), word);
    }

    explicit BlockDataTransfer(const std::string_view &s) {
      //TODO
    }

    enum Mnemonic Mnemonic() const override {
      return load ? Mnemonic::LDM : Mnemonic::STM;
    }

    u32 Encode() const override {
      return 0_u32 | (u8) condition << 28 | Bit::Mask::bit<u32>(27) | Bit::from_bool(is_pre_idx) << 24 | Bit::from_bool(is_inc) << 23 | Bit::from_bool(load_psr) | Bit::from_bool(write_back) << 21 | Bit::from_bool(load) << 20 | (u8) rN << 16 | registers;
    }

    std::string Disassemble() const override {
      // TODO
      return "";
    }

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::BlockDataTransfer;
    }
  };

  /**
   * Swap
   */
  struct Swap : private InstructionDataBase {
    Condition condition;
    bool byte;
    reg_t rN, rD, rM;

  private:
    friend struct Instruction;
    explicit Swap(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      byte = Bit::test(word, 22);
      rN = Bit::range(word, Bit::Mask::between<u32>(16,19));
      rD = Bit::range(word, Bit::Mask::between<u32>(16,19));
      rM = Bit::range(word, Bit::Mask::between<u32>(16,19));
    }

    explicit Swap(const std::string_view &s) {
      //TODO
    }

    enum Mnemonic Mnemonic() const override {
      return Mnemonic::SWP;
    }

    u32 Encode() const override {
      return 0_u32 | (u8) condition << 28 | 1 << 24 | (byte ? 1 : 0) << 22 | rN << 16 | rD << 12 | 0b1001 << 4 | rM;
    }

    std::string Disassemble() const override {
      std::stringstream ss;

      ss << to_string(Mnemonic()) << to_string(condition) << (byte ? "B" : "")
         << " ";
      ss << rD << ", " << rM << ", [" << rN << "]";

      return ss.str();
    }

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::Swap;
    }
  };

  /**
   * Software Interrupt
   */
  struct SoftwareInterrupt : private InstructionDataBase {
    Condition condition;
    u32 comment;

  private:
    friend struct Instruction;
    explicit SoftwareInterrupt(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      comment = Bit::range(Mask::swi_comment, word);
    }

    explicit SoftwareInterrupt(const std::string_view &s) {
      //TODO
    }

    enum Mnemonic Mnemonic() const override {
      return Mnemonic::SWI;
    }

    u32 Encode() const override {
      return 0_u32 | (u8) condition << 28 | 0xF << 24 | comment & Mask::swi_comment;
    }

    std::string Disassemble() const override {
      std::stringstream ss;

      ss << to_string(Mnemonic()) << to_string(condition) << " " << as_hex(comment);

      return ss.str();
    }

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::SoftwareInterrupt;
    }
  };

  /**
   * Coprocessor Data Operation
   */
  struct CoprocessorDataOperation : private InstructionDataBase {
    Condition condition;

  private:
    friend struct Instruction;
    explicit CoprocessorDataOperation(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      //TODO
    }

    explicit CoprocessorDataOperation(const std::string_view &s) {
      //TODO
    }

    enum Mnemonic Mnemonic() const override {
      return Mnemonic::CDP;
    }

    u32 Encode() const override {
      // TODO
      return 0;
    }

    std::string Disassemble() const override {
      // TODO
      return "";
    }

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::CoprocessorDataOperation;
    }
  };

  /**
   * Coprocessor Data Transfer
   */
  struct CoprocessorDataTransfer : private InstructionDataBase {
    Condition condition;
    bool is_load;

  private:
    friend struct Instruction;
    explicit CoprocessorDataTransfer(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      //TODO
    }

    explicit CoprocessorDataTransfer(const std::string_view &s) {
      //TODO
    }

    enum Mnemonic Mnemonic() const override {
      return is_load ? Mnemonic::LDC : Mnemonic::STC;
    }

    u32 Encode() const override {
      // TODO
      return 0;
    }

    std::string Disassemble() const override {
      // TODO
      return "";
    }

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::CoprocessorDataTransfer;
    }
  };

  /**
   * Coprocessor Register Transfer
   */
  struct CoprocessorRegisterTransfer : private InstructionDataBase {
    Condition condition;
    bool is_load;

  private:
    friend struct Instruction;
    explicit CoprocessorRegisterTransfer(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      //TODO
    }

    explicit CoprocessorRegisterTransfer(const std::string_view &s) {
      //TODO
    }

    enum Mnemonic Mnemonic() const override {
      return is_load ? Mnemonic::MRC : Mnemonic::MCR;
    }

    u32 Encode() const override {
      // TODO
      return 0;
    }

    std::string Disassemble() const override {
      // TODO
      return "";
    }

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::CoprocessorRegisterTransfer;
    }
  };

  /**
   * Undefined
   */
  struct Undefined : private InstructionDataBase {
    Condition condition;
    u32 word;

  private:
    friend struct Instruction;
    explicit Undefined(u32 word) {
      condition = (Condition) Bit::range(Mask::condition, word);
      this->word = word;
    }

    explicit Undefined(const std::string_view &s) {
      //TODO
    }

    enum Mnemonic Mnemonic() const override {
      return Mnemonic::Undefined;
    }

    u32 Encode() const override {
      return word;
    }

    std::string Disassemble() const override {
      std::stringstream ss;

      ss << "Undefined(" << as_hex(word) << ")" << to_string(condition);

      return ss.str();
    }

    static constexpr InstructionType GetInstructionType() {
      return InstructionType::Undefined;
    }
  };

  /**
   * Unified Instruction Class
   */
  struct Instruction {
    static Instruction Decode(u32 word) {
      return Instruction(word);
    }
    InstructionType Type() const { return type; }

    enum Mnemonic Mnemonic() const {
      ptr_preamble();
      return get_variant_data_ptr()->Mnemonic();
    }
    u32 Encode() const {
      ptr_preamble();
      return get_variant_data_ptr()->Encode();
    }
    std::string Disassemble() const {
      ptr_preamble();
      return get_variant_data_ptr()->Disassemble();
    }

    template<typename T>
    const T &InstructionData() {
      return std::get<T>(data);
    }

  protected:
    std::variant<
        // TODO: this is required to make the variant default-constructible but it also makes it able to be empty!!
        // figure out a better way to have complex logic to construct this variant without default construction
        std::monostate,
        Undefined,
        Branch,
        BranchAndExchange,
        DataProcessing,
        PSRTransfer,
        Multiply,
        MultiplyLong,
        SingleDataTransfer,
        HalfwordDataTransfer,
        BlockDataTransfer,
        Swap,
        SoftwareInterrupt,
        CoprocessorDataOperation,
        CoprocessorDataTransfer,
        CoprocessorRegisterTransfer
    > data;
    // sure, /technically/ taking a pointer to variant data "bypasses its type-safety protections" and "defeats its entire purpose", but it's so much cooler this way
    // plus we get easy function calls for Encode(), Disassemble(), and Mnemonic()
    InstructionDataBase *data_ptr=  nullptr;
    InstructionType type;

    template<typename T, typename... A>
    inline constexpr void set_variant_data(A... a) {
      data = T(a...);

      data_ptr = &std::get<T>(data);
      type = T::GetInstructionType();
    }

    [[nodiscard]] inline InstructionDataBase *get_variant_data_ptr() const {
      return reinterpret_cast<InstructionDataBase *>(data_ptr);
    }

    inline void ptr_preamble() const {
      assert(data_ptr != nullptr);
      assert(!std::holds_alternative<std::monostate>(data));
    }

    explicit Instruction(const u32 word) : type(InstructionType::Undefined) {
      switch (Bit::range(Bit::Mask::between_inc<u32>(24, 27), word)) {
      // Class 1 instructions
      case 0x0:
      case 0x1:
      case 0x2:
      case 0x3: {
        //        31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // dp     [Cond     ] 0  0  I  [OpCode   ] S  [Rn       ] [Rd       ] [Operand 2                        ]
        // psr    [Cond     ] 0  0  I  1  0  P  D  0  [ID       ] [Rm       ] [Operand 2                        ]
        // mul    [Cond     ] 0  0  0  0  0  0  A  S  [Rn       ] [Rd       ] [Rs       ] 1  0  0  1  [Rm       ]
        // mull   [Cond     ] 0  0  0  0  1  U  A  S  [H-Rd     ] [L-Rd     ] [Rn       ] 1  0  0  1  [Rm       ]
        // swap   [Cond     ] 0  0  0  1  0  B  0  0  [Rn       ] [Rd       ] X  X  X  X  1  0  0  1  [Rm       ] // I don't think [8:11] are read, but they could be forced to 0
        // bex    [Cond     ] 0  0  0  1  0  0  1  0  1  1  1  1  1  1  1  1  1  1  1  1  0  0  0  1  [Rn       ]
        // hdt    [Cond     ] 0  0  0  P  U  I  W  L  [Rn       ] [Rd       ] [0/Offset ] 1  S  H  1  [Rn/Offset]

        // if 25 is set, it can only be DP
        if (!Bit::test(word, 25)) {
          // check bex early since it doesn't fit smoothly with everything else
          if (Bit::range(Mask::bex_const, word) == Literal::BEX_CONSTANT) {
            set_variant_data<Branch>(word);
            break;
          }

          // check the 4,7 bits to see if it could be something other than data processing...
          if (Bit::test(word, 4) && Bit::test(word, 7)) {
            //...Yep

            //if 4 or 5 are set then this is a HalfwordTransfer
            if (Bit::test(word, 4) || Bit::test(word, 5)) {
              set_variant_data<HalfwordDataTransfer>(word);
              break;
            }

            // if bit 24 is checked, then this is  a swap
            if (Bit::test(word, 24)) {
              set_variant_data<Swap>(word);
              break;
            }

            // if we made it here then it's likely this is a multiply, determine which one
            if (Bit::test(word, 23)) {
              set_variant_data<MultiplyLong>(word);
            } else {
              set_variant_data<Multiply>(word);
            }

            break;
          }
        }

        using DPI = DataProcessing;
        bool is_psr = bounded(
                          Bit::range(DPI::BitMask::OpCode, word),
                          (uint) DPI::OperationCode::TST,
                          (uint) DPI::OperationCode::CMN) &&
                      (bool) Bit::mask(word, DPI::BitMask::isSetFlags);

        if (is_psr) {
          set_variant_data<PSRTransfer>(word);
          break;
        }

        set_variant_data<DataProcessing>(word);
        break;
      }

      // Class 2 Instructions
      case 0x4:
      case 0x5:
        // reg-indicated shift is not available for this instruction class
        if (Bit::range(Bit::Mask::bit<u32>(4), word)) {
          set_variant_data<Undefined>(word);
        } else {
          set_variant_data<SingleDataTransfer>(word);
        }
        break;
      case 0x6:
      case 0x7:
        // reg-indicated shift is not available for this instruction class
        if (Bit::range(Bit::Mask::bit<u32>(4), word)) {
          set_variant_data<Undefined>(word);
        } else {
          set_variant_data<SingleDataTransfer>(word);
        }
        break;
      case 0x8:
      case 0x9:
        set_variant_data<BlockDataTransfer>(word);
        break;
      case 0xA:
      case 0xB:
        set_variant_data<Branch>(word);
        break;
      case 0xC:
      case 0xD:
        set_variant_data<CoprocessorDataTransfer>(word);
        break;
      case 0xE: {
        bool is_crt = Bit::range(Mask::cp_is_crt, word);

        if (is_crt) {
          set_variant_data<CoprocessorRegisterTransfer>(word);
        } else {
          set_variant_data<CoprocessorDataOperation>(word);
        }
        break;
      }
      case 0xF:
        set_variant_data<SoftwareInterrupt>(word);
        break;
      default:
        unreachable();
      }
    }
  };
}// namespace Arm
