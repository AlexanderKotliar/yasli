#include "StdAfx.h"
#include "utils/StringUtil.h"

static char* escapeTable[256] = {
    "\\0" /* 0x00: */,
    "\\x01" /* 0x01: */,
    "\\x02" /* 0x02: */,
    "\\x03" /* 0x03: */,
    "\\x04" /* 0x04: */,
    "\\x05" /* 0x05: */,
    "\\x06" /* 0x06: */,
    "\\x07" /* 0x07: */,
    "\\x08" /* 0x08: */,
    "\\t"   /* 0x09: \t */,
    "\\n"   /* 0x0A: \n */,
    "\\x0B" /* 0x0B: */,
    "\\x0C" /* 0x0C: */,
    "\\x0D" /* 0x0D: */,
    "\\x0E" /* 0x0E: */,
    "\\x0F" /* 0x0F: */,
    
    
    "\\x10" /* 0x10: */,
    "\\x11" /* 0x11: */,
    "\\x12" /* 0x12: */,
    "\\x13" /* 0x13: */,
    "\\x14" /* 0x14: */,
    "\\x15" /* 0x15: */,
    "\\x16" /* 0x16: */,
    "\\x17" /* 0x17: */,
    "\\x18" /* 0x18: */,
    "\\x19" /* 0x19: */,
    "\\x1A" /* 0x1A: */,
    "\\x1B" /* 0x1B: */,
    "\\x1C" /* 0x1C: */,
    "\\x1D" /* 0x1D: */,
    "\\x1E" /* 0x1E: */,
    "\\x1F" /* 0x1F: */,
    
    
    " " /* 0x20:   */,
    "!" /* 0x21: ! */,
    "\\\"" /* 0x22: " */,
    "#" /* 0x23: # */,
    "$" /* 0x24: $ */,
    "%" /* 0x25: % */,
    "&" /* 0x26: & */,
    "'" /* 0x27: ' */,
    "(" /* 0x28: ( */,
    ")" /* 0x29: ) */,
    "*" /* 0x2A: * */,
    "+" /* 0x2B: + */,
    "," /* 0x2C: , */,
    "-" /* 0x2D: - */,
    "." /* 0x2E: . */,
    "/" /* 0x2F: / */,
    
    
    "0" /* 0x30: 0 */,
    "1" /* 0x31: 1 */,
    "2" /* 0x32: 2 */,
    "3" /* 0x33: 3 */,
    "4" /* 0x34: 4 */,
    "5" /* 0x35: 5 */,
    "6" /* 0x36: 6 */,
    "7" /* 0x37: 7 */,
    "8" /* 0x38: 8 */,
    "9" /* 0x39: 9 */,
    ":" /* 0x3A: : */,
    ";" /* 0x3B: ; */,
    "<" /* 0x3C: < */,
    "=" /* 0x3D: = */,
    ">" /* 0x3E: > */,
    "?" /* 0x3F: ? */,
    
    
    "@" /* 0x40: @ */,
    "A" /* 0x41: A */,
    "B" /* 0x42: B */,
    "C" /* 0x43: C */,
    "D" /* 0x44: D */,
    "E" /* 0x45: E */,
    "F" /* 0x46: F */,
    "G" /* 0x47: G */,
    "H" /* 0x48: H */,
    "I" /* 0x49: I */,
    "J" /* 0x4A: J */,
    "K" /* 0x4B: K */,
    "L" /* 0x4C: L */,
    "M" /* 0x4D: M */,
    "N" /* 0x4E: N */,
    "O" /* 0x4F: O */,
    
    
    "P" /* 0x50: P */,
    "Q" /* 0x51: Q */,
    "R" /* 0x52: R */,
    "S" /* 0x53: S */,
    "T" /* 0x54: T */,
    "U" /* 0x55: U */,
    "V" /* 0x56: V */,
    "W" /* 0x57: W */,
    "X" /* 0x58: X */,
    "Y" /* 0x59: Y */,
    "Z" /* 0x5A: Z */,
    "[" /* 0x5B: [ */,
    "\\\\" /* 0x5C: \ */,
    "]" /* 0x5D: ] */,
    "^" /* 0x5E: ^ */,
    "_" /* 0x5F: _ */,
    
    
    "`" /* 0x60: ` */,
    "a" /* 0x61: a */,
    "b" /* 0x62: b */,
    "c" /* 0x63: c */,
    "d" /* 0x64: d */,
    "e" /* 0x65: e */,
    "f" /* 0x66: f */,
    "g" /* 0x67: g */,
    "h" /* 0x68: h */,
    "i" /* 0x69: i */,
    "j" /* 0x6A: j */,
    "k" /* 0x6B: k */,
    "l" /* 0x6C: l */,
    "m" /* 0x6D: m */,
    "n" /* 0x6E: n */,
    "o" /* 0x6F: o */,
    
    
    "p" /* 0x70: p */,
    "q" /* 0x71: q */,
    "r" /* 0x72: r */,
    "s" /* 0x73: s */,
    "t" /* 0x74: t */,
    "u" /* 0x75: u */,
    "v" /* 0x76: v */,
    "w" /* 0x77: w */,
    "x" /* 0x78: x */,
    "y" /* 0x79: y */,
    "z" /* 0x7A: z */,
    "{" /* 0x7B: { */,
    "|" /* 0x7C: | */,
    "}" /* 0x7D: } */,
    "~" /* 0x7E: ~ */,
    "\x7F" /* 0x7F: */, // for utf-8
    
    
    "\x80" /* 0x80: */,
    "\x81" /* 0x81: */,
    "\x82" /* 0x82: */,
    "\x83" /* 0x83: */,
    "\x84" /* 0x84: */,
    "\x85" /* 0x85: */,
    "\x86" /* 0x86: */,
    "\x87" /* 0x87: */,
    "\x88" /* 0x88: */,
    "\x89" /* 0x89: */,
    "\x8A" /* 0x8A: */,
    "\x8B" /* 0x8B: */,
    "\x8C" /* 0x8C: */,
    "\x8D" /* 0x8D: */,
    "\x8E" /* 0x8E: */,
    "\x8F" /* 0x8F: */,
    
    
    "\x90" /* 0x90: */,
    "\x91" /* 0x91: */,
    "\x92" /* 0x92: */,
    "\x93" /* 0x93: */,
    "\x94" /* 0x94: */,
    "\x95" /* 0x95: */,
    "\x96" /* 0x96: */,
    "\x97" /* 0x97: */,
    "\x98" /* 0x98: */,
    "\x99" /* 0x99: */,
    "\x9A" /* 0x9A: */,
    "\x9B" /* 0x9B: */,
    "\x9C" /* 0x9C: */,
    "\x9D" /* 0x9D: */,
    "\x9E" /* 0x9E: */,
    "\x9F" /* 0x9F: */,
    
    
    "\xA0" /* 0xA0: */,
    "�" /* 0xA1: � */,
    "�" /* 0xA2: � */,
    "�" /* 0xA3: � */,
    "�" /* 0xA4: � */,
    "�" /* 0xA5: � */,
    "�" /* 0xA6: � */,
    "�" /* 0xA7: � */,
    "�" /* 0xA8: � */,
    "�" /* 0xA9: � */,
    "�" /* 0xAA: � */,
    "�" /* 0xAB: � */,
    "�" /* 0xAC: � */,
    "�" /* 0xAD: � */,
    "�" /* 0xAE: � */,
    "�" /* 0xAF: � */,
    
    
    "�" /* 0xB0: � */,
    "�" /* 0xB1: � */,
    "�" /* 0xB2: � */,
    "�" /* 0xB3: � */,
    "�" /* 0xB4: � */,
    "�" /* 0xB5: � */,
    "�" /* 0xB6: � */,
    "�" /* 0xB7: � */,
    "�" /* 0xB8: � */,
    "�" /* 0xB9: � */,
    "�" /* 0xBA: � */,
    "�" /* 0xBB: � */,
    "�" /* 0xBC: � */,
    "�" /* 0xBD: � */,
    "�" /* 0xBE: � */,
    "�" /* 0xBF: � */,
    
    
    "�" /* 0xC0: � */,
    "�" /* 0xC1: � */,
    "�" /* 0xC2: � */,
    "�" /* 0xC3: � */,
    "�" /* 0xC4: � */,
    "�" /* 0xC5: � */,
    "�" /* 0xC6: � */,
    "�" /* 0xC7: � */,
    "�" /* 0xC8: � */,
    "�" /* 0xC9: � */,
    "�" /* 0xCA: � */,
    "�" /* 0xCB: � */,
    "�" /* 0xCC: � */,
    "�" /* 0xCD: � */,
    "�" /* 0xCE: � */,
    "�" /* 0xCF: � */,
    
    
    "�" /* 0xD0: � */,
    "�" /* 0xD1: � */,
    "�" /* 0xD2: � */,
    "�" /* 0xD3: � */,
    "�" /* 0xD4: � */,
    "�" /* 0xD5: � */,
    "�" /* 0xD6: � */,
    "�" /* 0xD7: � */,
    "�" /* 0xD8: � */,
    "�" /* 0xD9: � */,
    "�" /* 0xDA: � */,
    "�" /* 0xDB: � */,
    "�" /* 0xDC: � */,
    "�" /* 0xDD: � */,
    "�" /* 0xDE: � */,
    "�" /* 0xDF: � */,
    
    
    "�" /* 0xE0: � */,
    "�" /* 0xE1: � */,
    "�" /* 0xE2: � */,
    "�" /* 0xE3: � */,
    "�" /* 0xE4: � */,
    "�" /* 0xE5: � */,
    "�" /* 0xE6: � */,
    "�" /* 0xE7: � */,
    "�" /* 0xE8: � */,
    "�" /* 0xE9: � */,
    "�" /* 0xEA: � */,
    "�" /* 0xEB: � */,
    "�" /* 0xEC: � */,
    "�" /* 0xED: � */,
    "�" /* 0xEE: � */,
    "�" /* 0xEF: � */,
    
    
    "�" /* 0xF0: � */,
    "�" /* 0xF1: � */,
    "�" /* 0xF2: � */,
    "�" /* 0xF3: � */,
    "�" /* 0xF4: � */,
    "�" /* 0xF5: � */,
    "�" /* 0xF6: � */,
    "�" /* 0xF7: � */,
    "�" /* 0xF8: � */,
    "�" /* 0xF9: � */,
    "�" /* 0xFA: � */,
    "�" /* 0xFB: � */,
    "�" /* 0xFC: � */,
    "�" /* 0xFD: � */,
    "�" /* 0xFE: � */,
    "�" /* 0xFF: � */
};

static char hexValueTable[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,

    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
    
void escapeString(MemoryWriter& dest, const char* begin, const char* end)
{
    while(begin != end){
        const char* str = escapeTable[(unsigned char)(*begin)];
        dest.write(str);
        ++begin;
    }
}

void unescapeString(std::string& dest, const char* begin, const char* end)
{
    dest.resize(end - begin);
    char* ptr = &dest[0];
    while(begin != end){
        if(*begin != '\\'){
            *ptr = *begin;
            ++ptr;
        }
        else{
            ++begin;
            if(begin == end)
                break;

            switch(*begin){
            case '0':  *ptr = '\0'; ++ptr; break;
            case 't':  *ptr = '\t'; ++ptr; break;
            case 'n':  *ptr = '\n'; ++ptr; break;
            case '\\': *ptr = '\\'; ++ptr; break;
            case '\"': *ptr = '\"'; ++ptr; break;
            case '\'': *ptr = '\''; ++ptr; break;
            case 'x':
                if(begin + 2 < end){
                    *ptr = hexValueTable[int(begin[1])] * 16 + hexValueTable[int(begin[2])];
                    ++ptr;
                    begin += 2;
                    break;
                }
            default:
                *ptr = *begin;
                ++ptr;
                break;
            }
        }
        ++begin;
    }
    dest.resize(ptr - &dest[0]);
    //*ptr = '\0';
}
