#include "cpu.hpp"

#include "util/util.hpp"

std::string CPU::i8() {
    return itoh(io->read(PC+1), 2);
}
std::string CPU::i16() {
    return itoh((u16)(io->read(PC+1) | (u16)(io->read(PC+2) << 8)), 4);
}

std::string CPU::format(std::string fmt) {
    for(u16 i = 0; i < fmt.size(); i++) {
        if(fmt[i] == '%') {
            fmt.erase(i, 1);
            fmt.insert(i, i8());
        } else if(fmt[i] == '#') {
            fmt.erase(i, 1);
            fmt.insert(i, i16());
        }
    }

    return fmt;
}

std::string CPU::getOpString(u8 op) {
    switch(op) {
        case 0x00:
            return format("NOP");
        case 0x01:
            return format("LD BC, #");
        case 0x02:
            return format("LD (BC), A");
        case 0x03:
            return format("INC BC");
        case 0x04:
            return format("INC B");
        case 0x05:
            return format("DEC B");
        case 0x06:
            return format("LD B, %%");
        case 0x07:
            return format("RLCA");
        case 0x08:
            return format("LD (#), SP");
        case 0x09:
            return format("ADD HL, BC");
        case 0x0a:
            return format("LD A, (BC)");
        case 0x0b:
            return format("DEC BC ");
        case 0x0c:
            return format("INC C");
        case 0x0d:
            return format("DEC C");
        case 0x0e:
            return format("DEC C");
        case 0x0f:
            return format("RRCA");
        case 0x10:
            return format("STOP");
        case 0x11:
            return format("LD DE, #");
        case 0x12:
            return format("LD (DE), A");
        case 0x13:
            return format("INC DE");
        case 0x14:
            return format("INC D");
        case 0x15:
            return format("DEC D");
        case 0x16:
            return format("LD D, %%");
        case 0x17:
            return format("RLA");
        case 0x18:
            return format("JR %%");
        case 0x19:
            return format("ADD HL, DE");
        case 0x1a:
            return format("LD A,(DE)");
        case 0x1b:
            return format("DEC DE");
        case 0x1c:
            return format("INC E");
        case 0x1d:
            return format("DEC E");
        case 0x1e:
            return format("LD E, %%");
        case 0x1f:
            return format("RRA");
        case 0x20:
            return format("JR NZ, %%");
        case 0x21:
            return format("LD HL, #");
        case 0x22:
            return format("LDI (HL),A");
        case 0x23:
            return format("INC HL");
        case 0x24:
            return format("INC H");
        case 0x25:
            return format("DEC H");
        case 0x26:
            return format("LD H, %%");
        case 0x27:
            return format("DAA");
        case 0x28:
            return format("JR Z, %%");
        case 0x29:
            return format("ADD HL, HL");
        case 0x2a:
            return format("LDI A, (HL)");
        case 0x2b:
            return format("DEC HL");
        case 0x2c:
            return format("INC L");
        case 0x2d:
            return format("DEC L");
        case 0x2e:
            return format("LD L, %%");
        case 0x2f:
            return format("CPL");
        case 0x30:
            return format("JR NC, %%");
        case 0x31:
            return format("LD SP, #");
        case 0x32:
            return format("LDD (HL), A");
        case 0x33:
            return format("INC SP");
        case 0x34:
            return format("INC (HL)");
        case 0x35:
            return format("DEC (HL)");
        case 0x36:
            return format("LD (HL), %%");
        case 0x37:
            return format("SCF");
        case 0x38:
            return format("JR C, %%");
        case 0x39:
            return format("ADD HL, SP");
        case 0x3a:
            return format("LDD A, (HL)");
        case 0x3b:
            return format("DEC SP");
        case 0x3c:
            return format("INC A");
        case 0x3d:
            return format("DEC A");
        case 0x3e:
            return format("LD A, %%");
        case 0x3f:
            return format("CCF");
        case 0x40:
            return format("LD B, B");
        case 0x41:
            return format("LD B, C");
        case 0x42:
            return format("LD B, D");
        case 0x43:
            return format("LD B, E");
        case 0x44:
            return format("LD B, H");
        case 0x45:
            return format("LD B, L");
        case 0x46:
            return format("LD B, (HL)");
        case 0x47:
            return format("LD B, A");
        case 0x48:
            return format("LD C, B");
        case 0x49:
            return format("LD C, C");
        case 0x4a:
            return format("LD C, D");
        case 0x4b:
            return format("LD C, E");
        case 0x4c:
            return format("LD C, H");
        case 0x4d:
            return format("LD C, L");
        case 0x4e:
            return format("LD C, (HL)");
        case 0x4f:
            return format("LD C, A");
        case 0x50:
            return format("LD D, B");
        case 0x51:
            return format("LD D, C");
        case 0x52:
            return format("LD D, D");
        case 0x53:
            return format("LD D, E");
        case 0x54:
            return format("LD D, H");
        case 0x55:
            return format("LD D, L");
        case 0x56:
            return format("LD D, (HL)");
        case 0x57:
            return format("LD D, A");
        case 0x58:
            return format("LD E, B");
        case 0x59:
            return format("LD E, C");
        case 0x5a:
            return format("LD E, D");
        case 0x5b:
            return format("LD E, E");
        case 0x5c:
            return format("LD E, H");
        case 0x5d:
            return format("LD E, L");
        case 0x5e:
            return format("LD E, (HL)");
        case 0x5f:
            return format("LD E, A");
        case 0x60:
            return format("LD H, B");
        case 0x61:
            return format("LD H, C");
        case 0x62:
            return format("LD H, D");
        case 0x63:
            return format("LD H, E");
        case 0x64:
            return format("LD H, H");
        case 0x65:
            return format("LD H, L");
        case 0x66:
            return format("LD H, (HL)");
        case 0x67:
            return format("LD H, A");
        case 0x68:
            return format("LD L, B");
        case 0x69:
            return format("LD L, C");
        case 0x6a:
            return format("LD L, D");
        case 0x6b:
            return format("LD L, E");
        case 0x6c:
            return format("LD L, H");
        case 0x6d:
            return format("LD L, L");
        case 0x6e:
            return format("LD L, (HL)");
        case 0x6F:
            return format("LD L, A");
        case 0x70:
            return format("LD (HL), B");
        case 0x71:
            return format("LD (HL), C");
        case 0x72:
            return format("LD (HL), D");
        case 0x73:
            return format("LD (HL), E");
        case 0x74:
            return format("LD (HL), H");
        case 0x75:
            return format("LD (HL), L");
        case 0x76:
            return format("HALT");
        case 0x77:
            return format("LD (HL), A");
        case 0x78:
            return format("LD A, B");
        case 0x79:
            return format("LD A, C");
        case 0x7a:
            return format("LD A, D");
        case 0x7b:
            return format("LD A, E");
        case 0x7c:
            return format("LD A, H");
        case 0x7d:
            return format("LD A, L");
        case 0x7e:
            return format("LD A, (HL)");
        case 0x7f:
            return format("LD A, A");
        case 0x80:
            return format("ADD A, B");
        case 0x81:
            return format("ADD A, C");
        case 0x82:
            return format("ADD A, D");
        case 0x83:
            return format("ADD A, E");
        case 0x84:
            return format("ADD A, H");
        case 0x85:
            return format("ADD A, L");
        case 0x86:
            return format("ADD A, (HL)");
        case 0x87:
            return format("ADD A, A");
        case 0x88:
            return format("ADC A, B");
        case 0x89:
            return format("ADC A, C");
        case 0x8a:
            return format("ADC A, D");
        case 0x8b:
            return format("ADC A, E");
        case 0x8c:
            return format("ADC A, H");
        case 0x8d:
            return format("ADC A, L");
        case 0x8e:
            return format("ADC A, (HL)");
        case 0x8f:
            return format("ADC A, A");
        case 0x90:
            return format("SUB B");
        case 0x91:
            return format("SUB C");
        case 0x92:
            return format("SUB D");
        case 0x93:
            return format("SUB E");
        case 0x94:
            return format("SUB H");
        case 0x95:
            return format("SUB L");
        case 0x96:
            return format("SUB (HL)");
        case 0x97:
            return format("SUB A");
        case 0x98:
            return format("SBC A, B");
        case 0x99:
            return format("SBC A, C");
        case 0x9a:
            return format("SBC A, D");
        case 0x9b:
            return format("SBC A, E");
        case 0x9c:
            return format("SBC A, H");
        case 0x9d:
            return format("SBC A, L");
        case 0x9e:
            return format("SBC A, (HL)");
        case 0x9f:
            return format("SBC A, A");
        case 0xa0:
            return format("AND B");
        case 0xa1:
            return format("AND C");
        case 0xa2:
            return format("AND D");
        case 0xa3:
            return format("AND E");
        case 0xa4:
            return format("AND H");
        case 0xa5:
            return format("AND L");
        case 0xa6:
            return format("AND (HL)");
        case 0xa7:
            return format("AND A");
        case 0xa8:
            return format("XOR B");
        case 0xa9:
            return format("XOR C");
        case 0xaa:
            return format("XOR D");
        case 0xab:
            return format("XOR E");
        case 0xac:
            return format("XOR H");
        case 0xad:
            return format("XOR L");
        case 0xae:
            return format("XOR (HL)");
        case 0xaf:
            return format("XOR A");
        case 0xb0:
            return format("OR B");
        case 0xb1:
            return format("OR C");
        case 0xb2:
            return format("OR D");
        case 0xb3:
            return format("OR E");
        case 0xb4:
            return format("OR H");
        case 0xb5:
            return format("OR L");
        case 0xb6:
            return format("OR (HL)");
        case 0xb7:
            return format("OR A");
        case 0xb8:
            return format("CP B");
        case 0xb9:
            return format("CP C");
        case 0xba:
            return format("CP D");
        case 0xbb:
            return format("CP E");
        case 0xbc:
            return format("CP H");
        case 0xbd:
            return format("CP L");
        case 0xbe:
            return format("CP (HL)");
        case 0xbf:
            return format("CP A");
        case 0xc0:
            return format("RET NC");
        case 0xc1:
            return format("POP BC");
        case 0xc2:
            return format("JP NZ #");
        case 0xc3:
            return format("JP #");
        case 0xc4:
            return format("CALL NZ, #");
        case 0xc5:
            return format("PUSH BC");
        case 0xc6:
            return format("ADD A, %%");
        case 0xc7:
            return format("RST 00h");
        case 0xc8:
            return format("CALL Z, #");
        case 0xc9:
            return format("RET");
        case 0xca:
            return format("JP Z, #");
        case 0xcb:
            return getCBOpString(PC+1);
        case 0xcc:
            return  format("CALL Z, #");
        case 0xcd:
            return  format("CALL #");
        case 0xce:
            return  format("ADC A, %%");
        case 0xcf:
            return  format("RST 08h");
        case 0xd0:
            return  format("CALL NC, #");
        case 0xd1:
            return  format("POP DE");
        case 0xd2:
            return  format("JP NC, #");
        case 0xd4:
            return  format("CALL NC #");
        case 0xd5:
            return  format("PUSH DE");
        case 0xd6:
            return  format("SUB %%");
        case 0xd7:
            return  format("RST 10h");
        case 0xd8:
            return  format("CALL C #");
        case 0xd9:
            return  format("RETI");
        case 0xda:
            return  format("JP C #");
        case 0xdc:
            return  format("CALL C #");
        case 0xde:
            return  format("SBC A, %%");
        case 0xdf:
            return  format("RST 18h");
        case 0xe0:
            return  format("LD ($FF00 + %%), A");
        case 0xe1:
            return  format("POP HL");
        case 0xe2:
            return  format("LD ($FF00 + C), A");
        case 0xe5:
            return  format("PUSH HL");
        case 0xe6:
            return  format("AND %%");
        case 0xe7:
            return  format("RST 20h");
        case 0xe8:
            return  format("ADD  SP, %%");
        case 0xe9:
            return  format("JP (HL)");
        case 0xea:
            return  format("LD (#), A");
        case 0xee:
            return  format("XOR %%");
        case 0xef:
            return  format("RST 28h");
        case 0xf0:
            return  format("LD A, ($FF00 + %%)");
        case 0xf1:
            return  format("POP AF");
        case 0xf2:
            return  format("LD A, (FF00 + C)");
        case 0xf3:
            return  format("DI");
        case 0xf5:
            return  format("PUSH AF");
        case 0xf6:
            return  format("OR %%");
        case 0xf7:
            return  format("RST 30h");
        case 0xf8:
            return  format("LD HL, SP + %%");
        case 0xf9:
            return  format("LD SP, HL");
        case 0xfa:
            return  format("LD A, (%%)");
        case 0xfb:
            return  format("EI");
        case 0xfe:
            return  format("CP %%");
        case 0xff:
            return  format("RST 38h");
        default:
            return "Invalid Op";
    }
}

std::string CPU::getCBOpString(u8 op) {
    switch (op){
    case 0x40:
        return   "BIT 0, B";
    case 0x41:
        return   "BIT 0, C";
    case 0x42:
        return   "BIT 0, D";
    case 0x43:
        return   "BIT 0, E";
    case 0x44:
        return   "BIT 0, H";
    case 0x45:
        return   "BIT 0, L";
    case 0x46:
        return   "BIT 0, (HL)";
    case 0x47:
        return   "BIT 0, A";
    case 0x48:
        return   "BIT 1, B";
    case 0x49:
        return   "BIT 1, C";
    case 0x4a:
        return   "BIT 1, D";
    case 0x4b:
        return   "BIT 1, E";
    case 0x4c:
        return   "BIT 1, H";
    case 0x4d:
        return   "BIT 1, L";
    case 0x4e:
        return   "BIT 1, (HL)";
    case 0x4f:
        return   "BIT 1, A";
    case 0x50:
        return   "BIT 2, B";
    case 0x51:
        return   "BIT 2, C";
    case 0x52:
        return   "BIT 2, D";
    case 0x53:
        return   "BIT 2, E";
    case 0x54:
        return   "BIT 2, H";
    case 0x55:
        return   "BIT 2, L";
    case 0x56:
        return   "BIT 2, (HL)";
    case 0x57:
        return   "BIT 2, A";
    case 0x58:
        return   "BIT 3, B";
    case 0x59:
        return   "BIT 3, C";
    case 0x5a:
        return   "BIT 3, D";
    case 0x5b:
        return   "BIT 3, E";
    case 0x5c:
        return   "BIT 3, H";
    case 0x5d:
        return   "BIT 3, L";
    case 0x5e:
        return   "BIT 3, (HL)";
    case 0x5f:
        return   "BIT 3, A";
    case 0x60:
        return   "BIT 4, B";
    case 0x61:
        return   "BIT 4, C";
    case 0x62:
        return   "BIT 4, D";
    case 0x63:
        return   "BIT 4, E";
    case 0x64:
        return   "BIT 4, H";
    case 0x65:
        return   "BIT 4, L";
    case 0x66:
        return   "BIT 4, (HL)";
    case 0x67:
        return   "BIT 4, A";
    case 0x68:
        return   "BIT 5, B";
    case 0x69:
        return   "BIT 5, C";
    case 0x6a:
        return   "BIT 5, D";
    case 0x6b:
        return   "BIT 5, E";
    case 0x6c:
        return   "BIT 5, H";
    case 0x6d:
        return   "BIT 5, L";
    case 0x6e:
        return   "BIT 5, (HL)";
    case 0x6f:
        return   "BIT 5, A";
    case 0x70:
        return   "BIT 6, B";
    case 0x71:
        return   "BIT 6, C";
    case 0x72:
        return   "BIT 6, D";
    case 0x73:
        return   "BIT 6, E";
    case 0x74:
        return   "BIT 6, H";
    case 0x75:
        return   "BIT 6, L";
    case 0x76:
        return   "BIT 6, (HL)";
    case 0x77:
        return   "BIT 6, A";
    case 0x78:
        return   "BIT 7, B";
    case 0x79:
        return   "BIT 7, C";
    case 0x7a:
        return   "BIT 7, D";
    case 0x7b:
        return   "BIT 7, E";
    case 0x7c:
        return   "BIT 7, H";
    case 0x7d:
        return   "BIT 7, L";
    case 0x7e:
        return   "BIT 7, (HL)";
    case 0x7f:
        return   "BIT 7, A";
    case 0x80:
        return   "RES 0, B";
    case 0x81:
        return   "RES 0, C";
    case 0x82:
        return   "RES 0, D";
    case 0x83:
        return   "RES 0, E";
    case 0x84:
        return   "RES 0, H";
    case 0x85:
        return   "RES 0, L";
    case 0x86:
        return   "RES 0, (HL)";
    case 0x87:
        return   "RES 0, A";
    case 0x88:
        return   "RES 1, B";
    case 0x89:
        return   "RES 1, C";
    case 0x8a:
        return   "RES 1, D";
    case 0x8b:
        return   "RES 1, E";
    case 0x8c:
        return   "RES 1, H";
    case 0x8d:
        return   "RES 1, L";
    case 0x8e:
        return   "RES 1, (HL)";
    case 0x8f:
        return   "RES 1, A";
    case 0x90:
        return   "RES 2, B";
    case 0x91:
        return   "RES 2, C";
    case 0x92:
        return   "RES 2, D";
    case 0x93:
        return   "RES 2, E";
    case 0x94:
        return   "RES 2, H";
    case 0x95:
        return   "RES 2, L";
    case 0x96:
        return   "RES 2, (HL)";
    case 0x97:
        return   "RES 2, A";
    case 0x98:
        return   "RES 3, B";
    case 0x99:
        return   "RES 3, C";
    case 0x9a:
        return   "RES 3, D";
    case 0x9b:
        return   "RES 3, E";
    case 0x9c:
        return   "RES 3, H";
    case 0x9d:
        return   "RES 3, L";
    case 0x9e:
        return   "RES 3, (HL)";
    case 0x9f:
        return   "RES 3, A";
    case 0xa0:
        return   "RES 4, B";
    case 0xa1:
        return   "RES 4, C";
    case 0xa2:
        return   "RES 4, D";
    case 0xa3:
        return   "RES 4, E";
    case 0xa4:
        return   "RES 4, H";
    case 0xa5:
        return   "RES 4, L";
    case 0xa6:
        return   "RES 4, (HL)";
    case 0xa7:
        return   "RES 4, A";
    case 0xa8:
        return   "RES 5, B";
    case 0xa9:
        return   "RES 5, C";
    case 0xaa:
        return   "RES 5, D";
    case 0xab:
        return   "RES 5, E";
    case 0xac:
        return   "RES 5, H";
    case 0xad:
        return   "RES 5, L";
    case 0xae:
        return   "RES 5, (HL)";
    case 0xaf:
        return   "RES 5, A";
    case 0xb0:
        return   "RES 6, B";
    case 0xb1:
        return   "RES 6, C";
    case 0xb2:
        return   "RES 6, D";
    case 0xb3:
        return   "RES 6, E";
    case 0xb4:
        return   "RES 6, H";
    case 0xb5:
        return   "RES 6, L";
    case 0xb6:
        return   "RES 6, (HL)";
    case 0xb7:
        return   "RES 6, A";
    case 0xb8:
        return   "RES 7, B";
    case 0xb9:
        return   "RES 7, C";
    case 0xba:
        return   "RES 7, D";
    case 0xbb:
        return   "RES 7, E";
    case 0xbc:
        return   "RES 7, H";
    case 0xbd:
        return   "RES 7, L";
    case 0xbe:
        return   "RES 7, (HL)";
    case 0xbf:
        return   "RES 7, A";
    case 0xc0:
        return   "SET 0, B";
    case 0xc1:
        return   "SET 0, C";
    case 0xc2:
        return   "SET 0, D";
    case 0xc3:
        return   "SET 0, E";
    case 0xc4:
        return   "SET 0, H";
    case 0xc5:
        return   "SET 0, L";
    case 0xc6:
        return   "SET 0, (HL)";
    case 0xc7:
        return   "SET 0, A";
    case 0xc8:
        return   "SET 1, B";
    case 0xc9:
        return   "SET 1, C";
    case 0xca:
        return   "SET 1, D";
    case 0xcb:
        return   "SET 1, E";
    case 0xcc:
        return   "SET 1, H";
    case 0xcd:
        return   "SET 1, L";
    case 0xce:
        return   "SET 1, (HL)";
    case 0xcf:
        return   "SET 1, A";
    case 0xd0:
        return   "SET 2, B";
    case 0xd1:
        return   "SET 2, C";
    case 0xd2:
        return   "SET 2, D";
    case 0xd3:
        return   "SET 2, E";
    case 0xd4:
        return   "SET 2, H";
    case 0xd5:
        return   "SET 2, L";
    case 0xd6:
        return   "SET 2, (HL)";
    case 0xd7:
        return   "SET 2, A";
    case 0xd8:
        return   "SET 3, B";
    case 0xd9:
        return   "SET 3, C";
    case 0xda:
        return   "SET 3, D";
    case 0xdb:
        return   "SET 3, E";
    case 0xdc:
        return   "SET 3, H";
    case 0xdd:
        return   "SET 3, L";
    case 0xde:
        return   "SET 3, (HL)";
    case 0xdf:
        return   "SET 3, A";
    case 0xe0:
        return   "SET 4, B";
    case 0xe1:
        return   "SET 4, C";
    case 0xe2:
        return   "SET 4, D";
    case 0xe3:
        return   "SET 4, E";
    case 0xe4:
        return   "SET 4, H";
    case 0xe5:
        return   "SET 4, L";
    case 0xe6:
        return   "SET 4, (HL)";
    case 0xe7:
        return   "SET 4, A";
    case 0xe8:
        return   "SET 5, B";
    case 0xe9:
        return   "SET 5, C";
    case 0xea:
        return   "SET 5, D";
    case 0xeb:
        return   "SET 5, E";
    case 0xec:
        return   "SET 5, H";
    case 0xed:
        return   "SET 5, L";
    case 0xee:
        return   "SET 5, (HL)";
    case 0xef:
        return   "SET 5, A";
    case 0xf0:
        return   "SET 6, B";
    case 0xf1:
        return   "SET 6, C";
    case 0xf2:
        return   "SET 6, D";
    case 0xf3:
        return   "SET 6, E";
    case 0xf4:
        return   "SET 6, H";
    case 0xf5:
        return   "SET 6, L";
    case 0xf6:
        return   "SET 6, (HL)";
    case 0xf7:
        return   "SET 6, A";
    case 0xf8:
        return   "SET 7, B";
    case 0xf9:
        return   "SET 7, C";
    case 0xfa:
        return   "SET 7, D";
    case 0xfb:
        return   "SET 7, E";
    case 0xfc:
        return   "SET 7, H";
    case 0xfd:
        return   "SET 7, L";
    case 0xfe:
        return   "SET 7, (HL)";
    case 0xff:
        return   "SET 7, A";
    default:
        return "Invalid OpCode";
    }
}