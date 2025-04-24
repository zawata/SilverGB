#include "arm_instruction.hh"

namespace Arm {
    std::string DisassembleArm(Instruction const &instr) {
        return std::visit(
                // static_assert(InstructionConcept<std::decay_t<decltype(arg)>>); // Optional: check concept
                [](const auto &instr) { return instr.Disassemble(); },
                instr);
    }

    std::string DisassembleThumb(Instruction const &instr) {
        // return std::visit(
        //         // static_assert(InstructionConcept<std::decay_t<decltype(arg)>>); // Optional: check concept
        //         [](const auto &instr) { return instr.DisassembleThumb(); },
        //         instr);
    }

    Instruction DecodeThumb(const u16 word) {
        //                  15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // Mov Shifted Reg   0  0  0 [Op ] [Off5        ] [Rs    ] [Rd    ]
        // Add/Sub           0  0  0  1  1  I  O [Rn/O3 ] [Rs    ] [Rd    ]
        // Mov/Comp/Add/Sub  0  0  1 [Op ] [Rd    ] [Off8                 ]
        // alu               0  1  0  0  0  0 [Op       ] [Rs    ] [Rd    ]
        // HRO/BX            0  1  0  0  0  1 [Op ] H1 H2 [Rs/Hs ] [Rd/Hd ]
        // PC-relative load  0  1  0  0  1 [Rd    ] [Word8                ]
        // LS with reg off   0  1  0  1  L  B  0 [Ro    ] [Rb    ] [Rd    ]
        // LS with sign-ext  0  1  0  1  H  S  1 [Ro    ] [Rb    ] [Rd    ]
        // LS with imm off   0  1  1  B  L [Off5        ] [Rb    ] [Rd    ]
        // LS halfword       1  0  0  0  L [Off5        ] [Rb    ] [Rd    ]
        // SP-relative LS    1  0  0  1  L [Rd    ] [Word8                ]
        // Load addr         1  0  1  0 SP [Rd    ] [Word8                ]
        // Add off to SP     1  0  1  1  0  0  0  0  S [SWord7            ]
        // Push/Pop          1  0  1  1  L  1  0  R [RList                ]
        // multiple LS       1  1  0  0  L [Rb    ] [RList                ]
        // cond branch       1  1  0  1 [Cond     ] [SOff8                ]
        // SWI               1  1  0  1  1  1  1  1 [Value8               ]
        // branch            1  1  1  0  0 [Off11                         ]
        // long branch/link  1  1  1  1  H [Offset                        ]
        switch(Bit::range(Bit::Mask::between_inc<u16>(12, 15), word)) {
        case 0x0:
        case 0x1:
            // MOV shifted reg
            // add/Sub
        case 0x2:
        case 0x3:
            // MOV/comp/add/sub
        case 0x4:
            // alu
            // HRO/BX
            // PC-relative load
        case 0x5:
            // LS with reg off
            // LS with sign-ext
        case 0x6:
        case 0x7:
            // LS with imm off
        case 0x8:
            // LS halfword
        case 0x9:
            // SP-relative LS
        case 0xa:
            // load addr
            break;
        case 0xb:
            // add off
            // push/pop
        case 0xc:
            // multiple LS
            break;
        case 0xd:
            // SWI
            // cond branch
            break;
        case 0xe:
            // branch
            break;
        case 0xf:
            // long branch/link
            break;
        }
    }

    Instruction DecodeArm(const u32 word) {
        switch(Bit::range(Bit::Mask::between_inc<u32>(24, 27), word)) {
        // Class 1 instructions
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3: {
            //       31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
            // dp    [Cond     ] 0  0  I  [OpCode   ] S  [Rn       ] [Rd       ] [Operand 2                        ]
            // psr   [Cond     ] 0  0  I  1  0  P  D  0  [ID       ] [Rm       ] [Operand 2                        ]
            // mul   [Cond     ] 0  0  0  0  0  0  A  S  [Rn       ] [Rd       ] [Rs       ] 1  0  0  1  [Rm       ]
            // mull  [Cond     ] 0  0  0  0  1  U  A  S  [H-Rd     ] [L-Rd     ] [Rn       ] 1  0  0  1  [Rm       ]
            // swap  [Cond     ] 0  0  0  1  0  B  0  0  [Rn       ] [Rd       ] X  X  X  X  1  0  0  1  [Rm       ]
            // bex   [Cond     ] 0  0  0  1  0  0  1  0  1  1  1  1  1  1  1  1  1  1  1  1  0  0  0  1  [Rn       ]
            // hdt   [Cond     ] 0  0  0  P  U  I  W  L  [Rn       ] [Rd       ] [0/Offset ] 1  S  H  1  [Rn/Offset]

            // if 25 is set, it can only be DP or PSR
            if(!Bit::test(word, 25)) {
                // check bex early since it doesn't fit smoothly with everything else
                if(Bit::range(Mask::bex_const, word) == Literal::BEX_CONSTANT) {
                    return Branch::Decode(word);
                }

                // check the 4,7 bits to see if it could be something other than data processing...
                if(Bit::test(word, 4) && Bit::test(word, 7)) {
                    //...Yep

                    // if 5 or 6 are set then this is a HalfwordTransfer
                    if(Bit::test(word, 5) || Bit::test(word, 5)) {
                        return HalfwordDataTransfer::Decode(word);
                    }

                    // if bit 24 is checked, then this is  a swap
                    if(Bit::test(word, 24)) {
                        return Swap::Decode(word);
                    }

                    // if we made it here then it's likely this is a multiply, determine which one
                    if(Bit::test(word, 23)) {
                        return MultiplyLong::Decode(word);
                    } else {
                        return Multiply::Decode(word);
                    }
                }
            }

            using DPI   = DataProcessing;
            bool is_psr = bounded(Bit::range(DPI::BitMask::OpCode, word),
                                  (uint)DPI::OperationCode::TST,
                                  (uint)DPI::OperationCode::CMN)
                       && !Bit::mask(word, DPI::BitMask::isSetFlags);

            if(is_psr) {
                return PSRTransfer::Decode(word);
            }

            return DataProcessing::Decode(word);
        }

        // Class 2 Instructions
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7: return SingleDataTransfer::Decode(word);

        case 0x8:
        case 0x9: return BlockDataTransfer::Decode(word);
        case 0xA:
        case 0xB: return Branch::Decode(word);
        case 0xC:
        case 0xD: return CoprocessorDataTransfer::Decode(word);
        case 0xE: {
            bool is_crt = Bit::range(Mask::cp_is_crt, word);

            if(is_crt) {
                return CoprocessorRegisterTransfer::Decode(word);
            } else {
                return CoprocessorDataOperation::Decode(word);
            }
        }
        case 0xF: return SoftwareInterrupt::Decode(word);
        default:  unreachable();
        }
    }
};
