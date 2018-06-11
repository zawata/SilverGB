#include "cpu.hpp"

CPU::CPU(Memory_Map *m) :
mem(m),
PC(0x100) {}

void CPU::cycle() {
    std::cout << std::hex << PC << " ";
    switch(fetch_8()) {
        case 0x00: //  4  NOP
            std::cout <<" NOP";
            break;
        case 0x01: // 12  LD BC, yyxx
            break;
        case 0x02: //  8  LD (BC), A
            break;
        case 0x03: //  8  INC BC
            std::cout <<" INC BC ";
            BC.i_BC++;
            break;
        case 0x04: //  4  INC B
            std::cout <<" INC B ";
            BC.b_BC.B++;
            break;
        case 0x05: //  4  DEC B
            std::cout <<" DEC B ";
            BC.b_BC.B--;
            break;
        case 0x06: //  8  LD B, xx
            std::cout <<" LD B, ";
            load_8(&BC.b_BC.C, fetch_8());
            break;
        case 0x07: //  4  RLCA
            break;
        case 0x08: // ??  LD (yyxx), SP  (?)
            break;
        case 0x09: //  8  ADD HL, BC
            break;
        case 0x0a: //  8  LD A, (BC)
            break;
        case 0x0b: //  8  DEC BC
            std::cout <<" DEC BC ";
            BC.i_BC--;
            break;
        case 0x0c: //  4  INC C
            break;
        case 0x0d: //  4  DEC C
            break;
        case 0x0e: //  8  LD C, xx
            std::cout <<" LD C, ";
            load_8(&BC.b_BC.C, fetch_8());
            break;
        case 0x0f: //  4  RRCA
            break;
        case 0x10: // ??  STOP
            break;
        case 0x11: // 12  LD DE, yyxx
            break;
        case 0x12: //  8  LD (DE), A
            break;
        case 0x13: //  8  INC DE
            break;
        case 0x14: //  4  INC D
            break;
        case 0x15: //  4  DEC D
            break;
        case 0x16: //  8  LD D, xx
            break;
        case 0x17: //  4  RLA
            break;
        case 0x18: // 12  JR xx
            break;
        case 0x19: //  8  ADD HL, DE
            break;
        case 0x1a: //  8  LD A,(DE)
            break;
        case 0x1b: //  8  DEC DE
            break;
        case 0x1c: //  4  INC E
            break;
        case 0x1d: //  4  DEC E
            break;
        case 0x1e: //  8  LD E, xx
            break;
        case 0x1f: //  4  RRA
            break;
        case 0x20: // ??  JR NZ, xx
            break;
        case 0x21: // 12  LD HL, yyxx
            std::cout <<" LD HL, ";
            load_16(&HL.i_HL, fetch_16());
            break;
        case 0x22: //  8  LDI (HL),A
            break;
        case 0x23: //  8  INC HL
            break;
        case 0x24: //  4  INC H
            break;
        case 0x25: //  4  DEC H
            break;
        case 0x26: //  8  LD H, xx
            break;
        case 0x27: //  4  DAA
            break;
        case 0x28: // ??  JR Z, xx
            break;
        case 0x29: //  8  ADD HL, HL
            break;
        case 0x2a: //  8  LDI A, (HL)
            break;
        case 0x2b: //  8  DEC HL
            break;
        case 0x2c: //  4  INC L
            break;
        case 0x2d: //  4  DEC L
            break;
        case 0x2e: //  8  LD L, xx
            break;
        case 0x2f: //  4  CPL
            break;
        case 0x30: // ??  JR NC, xx
            break;
        case 0x31: // 12  LD SP, yyxx
            break;
        case 0x32: //  8  LDD (HL), A
            std::cout <<" LDD (HL), A";
            store(HL.i_HL--, AF.b_AF.A);
            break;
        case 0x33: //  8  INC SP
            std::cout <<" INC SP";
            SP++;
            break;
        case 0x34: // 12  INC (HL)
            fetch_8();
            break;
        case 0x35: // 12  DEC (HL)
            break;
        case 0x36: // 12  LD (HL), xx
            break;
        case 0x37: //  4  SCF
            break;
        case 0x38: // ??  JR C, xx
            break;
        case 0x39: //  8  ADD HL, SP
            break;
        case 0x3a: //  8  LDD A, (HL)
            break;
        case 0x3b: //  8  DEC SP
            break;
        case 0x3c: //  4  INC A
            break;
        case 0x3d: //  4  DEC A
            break;
        case 0x3e: //  8  LD A, xx
            break;
        case 0x3f: //  4  CCF
            break;
        case 0x40: //  4  LD B, B
            break;
        case 0x41: //  4  LD B, C
            break;
        case 0x42: //  4  LD B, D
            break;
        case 0x43: //  4  LD B, E
            break;
        case 0x44: //  4  LD B, H
            break;
        case 0x45: //  4  LD B, L
            break;
        case 0x46: //  8  LD B, (HL)
            break;
        case 0x47: //  4  LD B, A
            break;
        case 0x48: //  4  LD C, B
            break;
        case 0x49: //  4  LD C, C
            break;
        case 0x4a: //  4  LD C, D
            break;
        case 0x4b: //  4  LD C, E
            break;
        case 0x4c: //  4  LD C, H
            break;
        case 0x4d: //  4  LD C, L
            break;
        case 0x4e: //  8  LD C, (HL)
            break;
        case 0x4f: //  4  LD C, A
            break;
        case 0x50: //  4  LD D, B
            break;
        case 0x51: //  4  LD D, C
            break;
        case 0x52: //  4  LD D, D
            break;
        case 0x53: //  4  LD D, E
            break;
        case 0x54: //  4  LD D, H
            break;
        case 0x55: //  4  LD D, L
            break;
        case 0x56: //  8  LD D, (HL)
            break;
        case 0x57: //  4  LD D, A
            break;
        case 0x58: //  4  LD E, B
            break;
        case 0x59: //  4  LD E, C
            break;
        case 0x5a: //  4  LD E, D
            break;
        case 0x5b: //  4  LD E, E
            break;
        case 0x5c: //  4  LD E, H
            break;
        case 0x5d: //  4  LD E, L
            break;
        case 0x5e: //  8  LD E, (HL)
            break;
        case 0x5f: //  4  LD E, A
            break;
        case 0x60: //  4  LD H, B
            break;
        case 0x61: //  4  LD H, C
            break;
        case 0x62: //  4  LD H, D
            break;
        case 0x63: //  4  LD H, E
            break;
        case 0x64: //  4  LD H, H
            break;
        case 0x65: //  4  LD H, L
            break;
        case 0x66: //  8  LD H, (HL)
            break;
        case 0x67: //  4  LD H, A
            break;
        case 0x68: //  4  LD L, B
            break;
        case 0x69: //  4  LD L, C
            break;
        case 0x6a: //  4  LD L, D
            break;
        case 0x6b: //  4  LD L, E
            break;
        case 0x6c: //  4  LD L, H
            break;
        case 0x6d: //  4  LD L, L
            break;
        case 0x6e: //  8  LD L, (HL)
            break;
        case 0x6F: //  4  LD L, A
            break;
        case 0x70: //  8  LD (HL), B
            break;
        case 0x71: //  8  LD (HL), C
            break;
        case 0x72: //  8  LD (HL), D
            break;
        case 0x73: //  8  LD (HL), E
            break;
        case 0x74: //  8  LD (HL), H
            break;
        case 0x75: //  8  LD (HL), L
            break;
        case 0x76: //  *  HALT
            break;
        case 0x77: //  8  LD (HL), A
            break;
        case 0x78: //  4  LD A, B
            break;
        case 0x79: //  4  LD A, C
            break;
        case 0x7a: //  4  LD A, D
            break;
        case 0x7b: //  4  LD A, E
            break;
        case 0x7c: //  4  LD A, H
            break;
        case 0x7d: //  4  LD A, L
            break;
        case 0x7e: //  8  LD A, (HL)
            break;
        case 0x7f: //  4  LD A, A
            break;
        case 0x80: //  4  ADD A, B
            break;
        case 0x81: //  4  ADD A, C
            break;
        case 0x82: //  4  ADD A, D
            break;
        case 0x83: //  4  ADD A, E
            break;
        case 0x84: //  4  ADD A, H
            break;
        case 0x85: //  4  ADD A, L
            break;
        case 0x86: //  8  ADD A, (HL)
            break;
        case 0x87: //  4  ADD A, A
            break;
        case 0x88: //  4  ADC A, B
            break;
        case 0x89: //  4  ADC A, C
            break;
        case 0x8a: //  4  ADC A, D
            break;
        case 0x8b: //  4  ADC A, E
            break;
        case 0x8c: //  4  ADC A, H
            break;
        case 0x8d: //  4  ADC A, L
            break;
        case 0x8e: //  8  ADC A, (HL)
            break;
        case 0x8f: //  4  ADC A, A
            break;
        case 0x90: //  4  SUB B
            break;
        case 0x91: //  4  SUB C
            break;
        case 0x92: //  4  SUB D
            break;
        case 0x93: //  4  SUB E
            break;
        case 0x94: //  4  SUB H
            break;
        case 0x95: //  4  SUB L
            break;
        case 0x96: //  8  SUB (HL)
            break;
        case 0x97: //  4  SUB A
            break;
        case 0x98: //  4  SBC A, B
            break;
        case 0x99: //  4  SBC A, C
            break;
        case 0x9a: //  4  SBC A, D
            break;
        case 0x9b: //  4  SBC A, E
            break;
        case 0x9c: //  4  SBC A, H
            break;
        case 0x9d: //  4  SBC A, L
            break;
        case 0x9e: //  8  SBC A, (HL)
            break;
        case 0x9f: //  4  SBC A, A
            break;
        case 0xa0: //  4  AND B
            break;
        case 0xa1: //  4  AND C
            break;
        case 0xa2: //  4  AND D
            break;
        case 0xa3: //  4  AND E
            break;
        case 0xa4: //  4  AND H
            break;
        case 0xa5: //  4  AND L
            break;
        case 0xa6: //  8  AND (HL)
            break;
        case 0xa7: //  4  AND A
            break;
        case 0xa8: //  4  XOR B
            break;
        case 0xa9: //  4  XOR C
            break;
        case 0xaa: //  4  XOR D
            break;
        case 0xab: //  4  XOR E
            break;
        case 0xac: //  4  XOR H
            break;
        case 0xad: //  4  XOR L
            break;
        case 0xae: //  8  XOR (HL)
            break;
        case 0xaf: //  4  XOR A
            std::cout <<" XOR A";
            ex_or(AF.b_AF.A);
            break;
        case 0xb0: //  4  OR B
            break;
        case 0xb1: //  4  OR C
            break;
        case 0xb2: //  4  OR D
            break;
        case 0xb3: //  4  OR E
            break;
        case 0xb4: //  4  OR H
            break;
        case 0xb5: //  4  OR L
            break;
        case 0xb6: //  8  OR (HL)
            break;
        case 0xb7: //  4  OR A
            break;
        case 0xb8: //  4  CP B
            break;
        case 0xb9: //  4  CP C
            break;
        case 0xba: //  4  CP D
            break;
        case 0xbb: //  4  CP E
            break;
        case 0xbc: //  4  CP H
            break;
        case 0xbd: //  4  CP L
            break;
        case 0xbe: //  8  CP (HL)
            break;
        case 0xbf: //  4  CP A
            break;
        case 0xc0: // ??  RET NC
            break;
        case 0xc1: // 12  POP BC
            break;
        case 0xc2: // ??  JP NZ yyxx
            break;
        case 0xc3: // 16  JP yyxx
            std::cout <<" JP ";
            jump(fetch_16());
            break;
        case 0xc4: // ??  CALL NZ yyxx
            break;
        case 0xc5: // 16  PUSH BC
            break;
        case 0xc6: //  8  ADD A, xx
            break;
        case 0xc7: // 16  RST 00h
            break;
        case 0xc8: // ??  CALL Z yyxx
            break;
        case 0xc9: // 16  RET
            break;
        case 0xca: // ??  JP Z yyxx
            break;
        case 0xcb:
            switch (fetch_8()){
            case 0x40: //  8  BIT 0, B
                break;
            case 0x41: //  8  BIT 0, C
                break;
            case 0x42: //  8  BIT 0, D
                break;
            case 0x43: //  8  BIT 0, E
                break;
            case 0x44: //  8  BIT 0, H
                break;
            case 0x45: //  8  BIT 0, L
                break;
            case 0x46: // 12  BIT 0, (HL)
                break;
            case 0x47: //  8  BIT 0, A
                break;
            case 0x48: //  8  BIT 1, B
                break;
            case 0x49: //  8  BIT 1, C
                break;
            case 0x4a: //  8  BIT 1, D
                break;
            case 0x4b: //  8  BIT 1, E
                break;
            case 0x4c: //  8  BIT 1, H
                break;
            case 0x4d: //  8  BIT 1, L
                break;
            case 0x4e: // 12  BIT 1, (HL)
                break;
            case 0x4f: //  8  BIT 1, A
                break;
            case 0x50: //  8  BIT 2, B
                break;
            case 0x51: //  8  BIT 2, C
                break;
            case 0x52: //  8  BIT 2, D
                break;
            case 0x53: //  8  BIT 2, E
                break;
            case 0x54: //  8  BIT 2, H
                break;
            case 0x55: //  8  BIT 2, L
                break;
            case 0x56: // 12  BIT 2, (HL)
                break;
            case 0x57: //  8  BIT 2, A
                break;
            case 0x58: //  8  BIT 3, B
                break;
            case 0x59: //  8  BIT 3, C
                break;
            case 0x5a: //  8  BIT 3, D
                break;
            case 0x5b: //  8  BIT 3, E
                break;
            case 0x5c: //  8  BIT 3, H
                break;
            case 0x5d: //  8  BIT 3, L
                break;
            case 0x5e: // 12  BIT 3, (HL)
                break;
            case 0x5f: //  8  BIT 3, A
                break;
            case 0x60: //  8  BIT 4, B
                break;
            case 0x61: //  8  BIT 4, C
                break;
            case 0x62: //  8  BIT 4, D
                break;
            case 0x63: //  8  BIT 4, E
                break;
            case 0x64: //  8  BIT 4, H
                break;
            case 0x65: //  8  BIT 4, L
                break;
            case 0x66: // 12  BIT 4, (HL)
                break;
            case 0x67: //  8  BIT 4, A
                break;
            case 0x68: //  8  BIT 5, B
                break;
            case 0x69: //  8  BIT 5, C
                break;
            case 0x6a: //  8  BIT 5, D
                break;
            case 0x6b: //  8  BIT 5, E
                break;
            case 0x6c: //  8  BIT 5, H
                break;
            case 0x6d: //  8  BIT 5, L
                break;
            case 0x6e: // 12  BIT 5, (HL)
                break;
            case 0x6f: //  8  BIT 5, A
                break;
            case 0x70: //  8  BIT 6, B
                break;
            case 0x71: //  8  BIT 6, C
                break;
            case 0x72: //  8  BIT 6, D
                break;
            case 0x73: //  8  BIT 6, E
                break;
            case 0x74: //  8  BIT 6, H
                break;
            case 0x75: //  8  BIT 6, L
                break;
            case 0x76: // 12  BIT 6, (HL)
                break;
            case 0x77: //  8  BIT 6, A
                break;
            case 0x78: //  8  BIT 7, B
                break;
            case 0x79: //  8  BIT 7, C
                break;
            case 0x7a: //  8  BIT 7, D
                break;
            case 0x7b: //  8  BIT 7, E
                break;
            case 0x7c: //  8  BIT 7, H
                break;
            case 0x7d: //  8  BIT 7, L
                break;
            case 0x7e: // 12  BIT 7, (HL)
                break;
            case 0x7f: //  8  BIT 7, A
                break;
            case 0x80: //  8  RES 0, B
                break;
            case 0x81: //  8  RES 0, C
                break;
            case 0x82: //  8  RES 0, D
                break;
            case 0x83: //  8  RES 0, E
                break;
            case 0x84: //  8  RES 0, H
                break;
            case 0x85: //  8  RES 0, L
                break;
            case 0x86: // 16  RES 0, (HL)
                break;
            case 0x87: //  8  RES 0, A
                break;
            case 0x88: //  8  RES 1, B
                break;
            case 0x89: //  8  RES 1, C
                break;
            case 0x8a: //  8  RES 1, D
                break;
            case 0x8b: //  8  RES 1, E
                break;
            case 0x8c: //  8  RES 1, H
                break;
            case 0x8d: //  8  RES 1, L
                break;
            case 0x8e: // 16  RES 1, (HL)
                break;
            case 0x8f: //  8  RES 1, A
                break;
            case 0x90: //  8  RES 2, B
                break;
            case 0x91: //  8  RES 2, C
                break;
            case 0x92: //  8  RES 2, D
                break;
            case 0x93: //  8  RES 2, E
                break;
            case 0x94: //  8  RES 2, H
                break;
            case 0x95: //  8  RES 2, L
                break;
            case 0x96: // 16  RES 2, (HL)
                break;
            case 0x97: //  8  RES 2, A
                break;
            case 0x98: //  8  RES 3, B
                break;
            case 0x99: //  8  RES 3, C
                break;
            case 0x9a: //  8  RES 3, D
                break;
            case 0x9b: //  8  RES 3, E
                break;
            case 0x9c: //  8  RES 3, H
                break;
            case 0x9d: //  8  RES 3, L
                break;
            case 0x9e: // 16  RES 3, (HL)
                break;
            case 0x9f: //  8  RES 3, A
                break;
            case 0xa0: //  8  RES 4, B
                break;
            case 0xa1: //  8  RES 4, C
                break;
            case 0xa2: //  8  RES 4, D
                break;
            case 0xa3: //  8  RES 4, E
                break;
            case 0xa4: //  8  RES 4, H
                break;
            case 0xa5: //  8  RES 4, L
                break;
            case 0xa6: // 16  RES 4, (HL)
                break;
            case 0xa7: //  8  RES 4, A
                break;
            case 0xa8: //  8  RES 5, B
                break;
            case 0xa9: //  8  RES 5, C
                break;
            case 0xaa: //  8  RES 5, D
                break;
            case 0xab: //  8  RES 5, E
                break;
            case 0xac: //  8  RES 5, H
                break;
            case 0xad: //  8  RES 5, L
                break;
            case 0xae: // 16  RES 5, (HL)
                break;
            case 0xaf: //  8  RES 5, A
                break;
            case 0xb0: //  8  RES 6, B
                break;
            case 0xb1: //  8  RES 6, C
                break;
            case 0xb2: //  8  RES 6, D
                break;
            case 0xb3: //  8  RES 6, E
                break;
            case 0xb4: //  8  RES 6, H
                break;
            case 0xb5: //  8  RES 6, L
                break;
            case 0xb6: // 16  RES 6, (HL)
                break;
            case 0xb7: //  8  RES 6, A
                break;
            case 0xb8: //  8  RES 7, B
                break;
            case 0xb9: //  8  RES 7, C
                break;
            case 0xba: //  8  RES 7, D
                break;
            case 0xbb: //  8  RES 7, E
                break;
            case 0xbc: //  8  RES 7, H
                break;
            case 0xbd: //  8  RES 7, L
                break;
            case 0xbe: // 16  RES 7, (HL)
                break;
            case 0xbf: //  8  RES 7, A
                break;
            case 0xc0: //  8  SET 0, B
                break;
            case 0xc1: //  8  SET 0, C
                break;
            case 0xc2: //  8  SET 0, D
                break;
            case 0xc3: //  8  SET 0, E
                break;
            case 0xc4: //  8  SET 0, H
                break;
            case 0xc5: //  8  SET 0, L
                break;
            case 0xc6: // 16  SET 0, (HL)
                break;
            case 0xc7: //  8  SET 0, A
                break;
            case 0xc8: //  8  SET 1, B
                break;
            case 0xc9: //  8  SET 1, C
                break;
            case 0xca: //  8  SET 1, D
                break;
            case 0xcb: //  8  SET 1, E
                break;
            case 0xcc: //  8  SET 1, H
                break;
            case 0xcd: //  8  SET 1, L
                break;
            case 0xce: // 16  SET 1, (HL)
                break;
            case 0xcf: //  8  SET 1, A
                break;
            case 0xd0: //  8  SET 2, B
                break;
            case 0xd1: //  8  SET 2, C
                break;
            case 0xd2: //  8  SET 2, D
                break;
            case 0xd3: //  8  SET 2, E
                break;
            case 0xd4: //  8  SET 2, H
                break;
            case 0xd5: //  8  SET 2, L
                break;
            case 0xd6: // 16  SET 2, (HL)
                break;
            case 0xd7: //  8  SET 2, A
                break;
            case 0xd8: //  8  SET 3, B
                break;
            case 0xd9: //  8  SET 3, C
                break;
            case 0xda: //  8  SET 3, D
                break;
            case 0xdb: //  8  SET 3, E
                break;
            case 0xdc: //  8  SET 3, H
                break;
            case 0xdd: //  8  SET 3, L
                break;
            case 0xde: // 16  SET 3, (HL)
                break;
            case 0xdf: //  8  SET 3, A
                break;
            case 0xe0: //  8  SET 4, B
                break;
            case 0xe1: //  8  SET 4, C
                break;
            case 0xe2: //  8  SET 4, D
                break;
            case 0xe3: //  8  SET 4, E
                break;
            case 0xe4: //  8  SET 4, H
                break;
            case 0xe5: //  8  SET 4, L
                break;
            case 0xe6: // 16  SET 4, (HL)
                break;
            case 0xe7: //  8  SET 4, A
                break;
            case 0xe8: //  8  SET 5, B
                break;
            case 0xe9: //  8  SET 5, C
                break;
            case 0xea: //  8  SET 5, D
                break;
            case 0xeb: //  8  SET 5, E
                break;
            case 0xec: //  8  SET 5, H
                break;
            case 0xed: //  8  SET 5, L
                break;
            case 0xee: // 16  SET 5, (HL)
                break;
            case 0xef: //  8  SET 5, A
                break;
            case 0xf0: //  8  SET 6, B
                break;
            case 0xf1: //  8  SET 6, C
                break;
            case 0xf2: //  8  SET 6, D
                break;
            case 0xf3: //  8  SET 6, E
                break;
            case 0xf4: //  8  SET 6, H
                break;
            case 0xf5: //  8  SET 6, L
                break;
            case 0xf6: // 16  SET 6, (HL)
                break;
            case 0xf7: //  8  SET 6, A
                break;
            case 0xf8: //  8  SET 7, B
                break;
            case 0xf9: //  8  SET 7, C
                break;
            case 0xfa: //  8  SET 7, D
                break;
            case 0xfb: //  8  SET 7, E
                break;
            case 0xfc: //  8  SET 7, H
                break;
            case 0xfd: //  8  SET 7, L
                break;
            case 0xfe: // 16  SET 7, (HL)
                break;
            case 0xff: //  8  SET 7, A
                break;
            }
            break;
        case 0xcc: // ??  CALL Z, yyxx
            break;
        case 0xcd: // 24  CALL yyxx
            break;
        case 0xce: //  8  ADC A, xx
            break;
        case 0xcf: // 16  RST 08h
            break;
        case 0xd0: // ??  CALL NC, yyxx
            break;
        case 0xd1: // 12  POP DE
            break;
        case 0xd2: // ??  JP NC, yyxx
            break;
        case 0xd3: // --  ----
            break;
        case 0xd4: // ??  CALL NC yyxx
            break;
        case 0xd5: // 12  PUSH DE
            break;
        case 0xd6: //  8  SUB xx
            break;
        case 0xd7: // 16  RST 10h
            break;
        case 0xd8: // ??  CALL C yyxx
            break;
        case 0xd9: // 16  RETI
            break;
        case 0xda: // ??  JP C yyxx
            break;
        case 0xdb: // --  ----
            break;
        case 0xdc: // ??  CALL C yyxx
            break;
        case 0xdd: // --  ----
            break;
        case 0xde: //  8  SBC A, xx
            break;
        case 0xdf: // 16  RST 18h
            break;
        case 0xe0: // 12  LD ($FF00 + xx), A
            break;
        case 0xe1: // 12  POP HL
            break;
        case 0xe2: //  8  LD ($FF00 + C), A
            break;
        case 0xe3: // --  ----
            break;
        case 0xe4: // --  ----
            break;
        case 0xe5: // 16  PUSH HL
            break;
        case 0xe6: //  8  AND xx
            break;
        case 0xe7: // 16  RST 20h
            break;
        case 0xe8: //  8  ADD  SP, xx
            break;
        case 0xe9: //  4  JP (HL)
            break;
        case 0xea: // 16  LD (yyxx), A
            break;
        case 0xeb: // --  ----
            break;
        case 0xec: // --  ----
            break;
        case 0xed: // --  ----
            break;
        case 0xee: //  8  XOR xx
            break;
        case 0xef: // 16  RST 28h
            break;
        case 0xf0: // 12  LD A, ($FF00 + xx)
            break;
        case 0xf1: // 12  POP AF
            break;
        case 0xf2: //  8  LD A, (FF00 + C)
            break;
        case 0xf3: //  4  DI
            break;
        case 0xf4: // --  ----
            break;
        case 0xf5: // 16  PUSH AF
            break;
        case 0xf6: //  8  OR xx
            break;
        case 0xf7: // 16  RST 30h
            break;
        case 0xf8: // 12  LD HL, SP + xx
            break;
        case 0xf9: //  8  LD SP, HL
            break;
        case 0xfa: // 16  LD A, (xx)
            break;
        case 0xfb: //  4  EI
            break;
        case 0xfc: // --  ----
            break;
        case 0xfd: // --  ----
            break;
        case 0xfe: //  8  CP xx
            break;
        case 0xff: // 16  RST 38h
            break;
    }
}

u8 CPU::fetch_8() {
    u8 dat = mem->read(PC);
    std::cout << std::hex << (u16)dat << std::dec;
    PC++;
    return dat;
}

u16 CPU::fetch_16() {
    u8 xx = mem->read(PC++);
    u8 yy = mem->read(PC++);
    u16 dat = (u16)yy << 8 | xx;
    std::cout << std::hex << dat << std::dec;
    return dat;
}

void CPU::store(u16 offset, u8 data) {mem->write(offset, data);}

void CPU::load_8(u8 *dest, u8 data) { *dest = data; }
void CPU::load_16(u16 *dest, u16 data) { *dest = data; }

void CPU::ex_or(u8 data) { AF.b_AF.A ^= data; }

void CPU::jump(u16 offset) { PC = offset; }
void CPU::jump_cond(u8 flag, u16 offset) { if(AF.b_AF.i_F & flag) jump(offset); }