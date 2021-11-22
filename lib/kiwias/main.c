#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>

static char *output_path = "out.o";
static FILE *out_file;

#define INIT_VAL 0

unsigned long text_section_offset = 0;
unsigned long text_section_size = 0;
unsigned long symtab_section_offset = 0;


typedef enum {
    ND_INST_ADDI,
    ND_INST_JR,
    ND_INST_ADD,
    ND_INST_SUB,
    ND_INST_LA,
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node *next;
    int imm;
    unsigned rs1;
    unsigned rs2;
    unsigned rd;
    char *symbol;
};

Node *nodes;

// ------------
//  Utility
// ------------
// Report error
// Take the same arguments as printf()
void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}


// ------------
//  ELF header
// ------------

#define SIZEOF_ELF_HEADER 0x40

#define OFFSET_EI_MAG0 0x00
#define OFFSET_EI_MAG1 0x01
#define OFFSET_EI_MAG2 0x02
#define OFFSET_EI_MAG3 0x03
#define OFFSET_EI_CLASS 0x04
#define OFFSET_EI_DATA 0x05
#define OFFSET_EI_VERSION 0x06
#define OFFSET_EI_OSABI 0x07
#define OFFSET_EI_ABIVERSION 0x08
#define OFFSET_EI_PAD 0x09

typedef struct {
    unsigned char e_ident[0x10];
    unsigned short e_type;
    unsigned short e_machine;
    unsigned int e_version;
    unsigned long e_entry;
    unsigned long e_phoff;
    unsigned long e_shoff;
    unsigned int e_flag;
    unsigned short e_ehsize;
    unsigned short e_phentsize;
    unsigned short e_phnum;
    unsigned short e_shentisize;
    unsigned short e_shnum;
    unsigned short e_shstrndx;

} ElfHeader;

void gen_elf_header() {
    ElfHeader elf_header;

    // Magic number
    // 0x7F followed by ELF(45 4c 46) in ASCII
    elf_header.e_ident[OFFSET_EI_MAG0] = 0x7f;
    elf_header.e_ident[OFFSET_EI_MAG1] = 0x45;
    elf_header.e_ident[OFFSET_EI_MAG2] = 0x4c;
    elf_header.e_ident[OFFSET_EI_MAG3] = 0x46;
    
    elf_header.e_ident[OFFSET_EI_CLASS] = 2; // 64-bit
    elf_header.e_ident[OFFSET_EI_DATA] = 1; // little endian
    elf_header.e_ident[OFFSET_EI_VERSION] = 1; // original version of elf
    elf_header.e_ident[OFFSET_EI_OSABI] = 0; // Unix - System V
    elf_header.e_ident[OFFSET_EI_ABIVERSION] = 0;
    elf_header.e_ident[OFFSET_EI_PAD] = 0; // Currently not used in ELF
    elf_header.e_ident[OFFSET_EI_PAD + 1] = 0;
    elf_header.e_ident[OFFSET_EI_PAD + 2] = 0;
    elf_header.e_ident[OFFSET_EI_PAD + 3] = 0;
    elf_header.e_ident[OFFSET_EI_PAD + 4] = 0;
    elf_header.e_ident[OFFSET_EI_PAD + 5] = 0;
    elf_header.e_ident[OFFSET_EI_PAD + 6] = 0;

    elf_header.e_type = 1; // REL (RElocatable file)
    elf_header.e_machine = 0xf3; // RISC-v
    elf_header.e_version = 1; // original version of ELF
    // Entry point address
    elf_header.e_entry = 0x0;
    // Start of program headers
    elf_header.e_phoff = 0; // Current output doesn't have program header
    // Start of section headers
    // It will be overwritten later
    elf_header.e_shoff = 0;
    // Flags
    elf_header.e_flag = 0x4;
    // Size of this header
    elf_header.e_ehsize = 64; // byte
    // Size of program headers
    elf_header.e_phentsize = 0; // byte
    // Number of progrem headers
    elf_header.e_phnum = 0;
    // Size of section headers
    elf_header.e_shentisize = 64; // byte
    // Number of section headers
    elf_header.e_shnum = 7;
    // Section header string rtable index
    elf_header.e_shstrndx = 6;

    fwrite(&elf_header, sizeof(elf_header), 1, out_file);

}

// ----------------------
//  Program header table
// ----------------------

void gen_program_header_table() {
    // Currently it does nothing
}

// ----------------------
// .text section 
// ----------------------

Node *nodes;

// About the type of instruction, see "2.3 Immediate Encoding Variants" of following.
// https://riscv.org/wp-content/uploads/2017/05/riscv-spec-v2.2.pdf
struct RTypeInst {
    unsigned funct7: 7;
    unsigned rs2: 5;
    unsigned rs1: 5;
    unsigned funct3: 3;
    unsigned rd: 5;
    unsigned opecode: 7;
};
typedef struct RTypeInst RTypeInst;

struct ITypeInst {
    unsigned imm: 12;
    unsigned rs1: 5;
    unsigned funct3: 3;
    unsigned rd: 5;
    unsigned opecode: 7;
};
typedef struct ITypeInst ITypeInst;

struct UTypeInst {
    unsigned imm: 20;
    unsigned rd: 5;
    unsigned opecode: 7;
};
typedef struct UTypeInst UTypeInst;

typedef enum {
    REG_ZERO = 0,
    REG_RA,
    REG_SP,
    REG_GP,
    REG_TP,
    REG_T0,
    REG_T1,
    REG_T2,
    REG_S0_FP,
    REG_S1,
    REG_A0,
    REG_A1,
    REG_A2,
    REG_A3,
    REG_A4,
    REG_A5,
    REG_A6,
    REG_A7,
    REG_S2,
    REG_S3,
    REG_S4,
    REG_S5,
    REG_S6,
    REG_S7,
    REG_S8,
    REG_S9,
    REG_S10,
    REG_S11,
    REG_T3,
    REG_T4,
    REG_T5,
    REG_T6
} Register;

unsigned signed_int_12bit(int x) {
    if (x < 0) {
        x = ~(-x);
        x = (x + 1) & 0b111111111111;
    }
    return x;
}

void output_i_type_inst(ITypeInst *inst) {
    unsigned data = 0;
    unsigned mask;

    mask = 0b11111111111100000000000000000000;
    data = data | (mask & ((inst->imm) << 20));

    mask = 0b00000000000011111000000000000000;
    data = data | (mask & ((inst->rs1) << 15));

    mask = 0b00000000000000000111000000000000;
    data = data | (mask & ((inst->funct3) << 12));

    mask = 0b00000000000000000000111110000000;
    data = data | (mask & ((inst->rd) << 7));

    mask = 0b00000000000000000000000001111111;
    data = data | (mask & inst->opecode);

    fwrite(&data, sizeof(data), 1, out_file);
}

void output_r_type_inst(RTypeInst *inst) {
    unsigned data = 0;
    unsigned mask;

    mask = 0b11111110000000000000000000000000;
    data = data | (mask & ((inst->funct7) << 25));
    
    mask = 0b00000001111100000000000000000000;
    data = data | (mask & ((inst->rs2) << 20));

    mask = 0b00000000000011111000000000000000;
    data = data | (mask & ((inst->rs1) << 15));

    mask = 0b00000000000000000111000000000000;
    data = data | (mask & ((inst->funct3) << 12));

    mask = 0b00000000000000000000111110000000;
    data = data | (mask & ((inst->rd) << 7));

    mask = 0b00000000000000000000000001111111;
    data = data | (mask & inst->opecode);

    fwrite(&data, sizeof(data), 1, out_file);
}

void output_u_type_inst(UTypeInst *inst) {
    unsigned data = 0;
    unsigned mask;

    mask = 0b11111111111111111111000000000000;
    data = data | (mask & ((inst->imm) << 12));

    mask = 0b00000000000000000000111110000000;
    data = data | (mask & ((inst->rd) << 7));

    mask = 0b00000000000000000000000001111111;
    data = data | (mask & inst->opecode);

    fwrite(&data, sizeof(data), 1, out_file);
}

/*
 * ## Output of objdump
 * 0:   02a00513                addi    a0,zero,42
 * This is in little endian.
 *
 * ## print(format(0x02a00513, '#034b')) with python
 * 0b00000010101000000000010100010011 in binary
 * It matches with the way that RISC-V manual uses.
 *
 *
 * 000000101010|00000|000|01010|0010011
 * imm          rs1       rd    opecode
 *
 * 00000010|10100000|00000101|00010011 
 * 
 * ## Output of xxd
 * 00000040: 1305 a002
 * It outputs bytes one by one.
 *
 */
void gen_text_section() {
    text_section_offset = ftell(out_file);

    Node *cur = nodes;
    while (cur) {
        switch (cur->kind) {
            case ND_INST_ADDI:
            {
                ITypeInst node = {
                    signed_int_12bit(cur->imm),
                    cur->rs1,
                    0b000,
                    cur->rd,
                    0b0010011
                };
                output_i_type_inst(&node);
            }
            break;
            case ND_INST_ADD:
            {
                RTypeInst node = {
                    0b0000000,
                    cur->rs2,
                    cur->rs1,
                    0b000,
                    cur->rd,
                    0b0110011
                }; 
                output_r_type_inst(&node);
            }
            break;
            case ND_INST_SUB:
            {
                RTypeInst node = {
                    0b0100000,
                    cur->rs2,
                    cur->rs1,
                    0b000,
                    cur->rd,
                    0b0110011
                }; 
                output_r_type_inst(&node);
            }
            break;
            case ND_INST_JR:
            {
                // Currently only accepts `jr ra`.
                // It will be translated to `jalr zero, ra, 0`
                
                // result of riscv64-unknown-linux-gnu-objdump -d -Mno-aliases xxxx.o
                // 4:   00008067                jalr    zero,0(ra)
                // It's 0b00000000000000001000000001100111 in binary
                // 000000000000|00001|000|00000|1100111
                // imm          rs1       rd    opecode
                ITypeInst node = {
                    signed_int_12bit(0),
                    REG_RA,
                    0b000,
                    REG_ZERO,
                    0b1100111
                };
                output_i_type_inst(&node);
            }
            break;
            case ND_INST_LA:
            {
                // It will be transtated to `auipc rd, symbol[31:12]; addi rd, rd, symbol[31:12]`
                UTypeInst node_auipc = {
                    0, // Temporary value. It will be replaced by linker
                    cur->rd,
                    0b0010111
                };
                output_u_type_inst(&node_auipc);

                ITypeInst node_addi = {
                    0, // Temporary value. It will be replaced by linker
                    cur->rd,
                    0b000,
                    cur->rd,
                    0b0010011
                };
                output_i_type_inst(&node_addi);
            }
            break;
        } 
        cur = cur->next;
    }
 
    text_section_size = ftell(out_file) - text_section_offset;
}

// ----------------------
// .data section 
// ----------------------
void gen_data_section() {
    // Currently it does nothing
}

// ----------------------
// .bss section 
// ----------------------
void gen_bss_section() {
    // Currently it does nothing
}

// ----------------------
// .symtab section 
// ----------------------

// Based on Elf64_Sym
typedef struct {
   unsigned st_name;
   unsigned char st_info;
   unsigned char st_other;
   unsigned short st_shndx;
   unsigned long st_value;
   unsigned long st_size;
} Elf64Symbol;

// st_info
typedef enum {
    INFO_NOTYPE_LOCAL = 0,
    INFO_SECTION_LOCAL = 3,
    INFO_NOTYPE_GLOBAL = 0x10,
} StInfoValue;

// st_other is for visibility
typedef enum {
    VIS_DEFAULT = 0,
} StOtherValue;

Elf64Symbol symtab_section_data[] = {
    {0, INFO_NOTYPE_LOCAL, VIS_DEFAULT, 0, 0, 0},
    {0, INFO_SECTION_LOCAL, VIS_DEFAULT, 1, 0, 0}, // .text
    {0, INFO_SECTION_LOCAL, VIS_DEFAULT, 2, 0, 0}, // .data
    {0, INFO_SECTION_LOCAL, VIS_DEFAULT, 3, 0, 0}, // .bss
    {1, INFO_NOTYPE_GLOBAL, VIS_DEFAULT, 1, 0, 0}, // main
};

void gen_symtab_section() {
    // .symtab should be aligned to 8 bytes
    long pos = ftell(out_file);
    int padding_size = pos % 8 ? 8 - (pos % 8) : 0;
    fseek(out_file, padding_size, SEEK_CUR);
    
    symtab_section_offset = ftell(out_file);
    fwrite(&symtab_section_data, sizeof(symtab_section_data), 1, out_file); 
}

// ----------------------
// .strtab section 
// ----------------------

// unsigned char strtab_section_data[] = {
//     '\0', 'm', 'a', 'i', 'n', '\0'
// };

char *strtab_section_strings[] = {
    "",
    "main"
};

void gen_strtab_section() {
    // fwrite(&strtab_section_data, sizeof(strtab_section_data), 1, out_file); 
    int n = sizeof(strtab_section_strings) / sizeof(strtab_section_strings[0]);
    for (int i = 0; i < n; i++) {
        fwrite(strtab_section_strings[i], strlen(strtab_section_strings[i]) + 1, 1, out_file);
    }
}

// ----------------------
// .shstrtab section 
// ----------------------

char *shstrtab_section_strings[] = {
    "",
    ".symtab",
    ".strtab",
    ".shstrtab",
    ".text",
    ".data",
    ".bss"
};

void gen_shstrtab_section() {
    int n = sizeof(shstrtab_section_strings) / sizeof(shstrtab_section_strings[0]);
    for (int i = 0; i < n; i++) {
        fwrite(shstrtab_section_strings[i], strlen(shstrtab_section_strings[i]) + 1, 1, out_file);
    }
}

// ------------------------------------
// Padding before section header table 
// ------------------------------------
void add_padding_before_section_header() {
    // Section header table should be aligned to 8 bytes
    long pos = ftell(out_file);
    int padding_size = pos % 8 ? 8 - (pos % 8) : 0;
    fseek(out_file, padding_size, SEEK_CUR);
}

// ---------------------
// Fix ELF Header
// ---------------------

void fix_elf_header() {
    long cur_pos = ftell(out_file);
    // Fix section header's offset
    long offset_to_e_shoff = 0x28;
    fseek(out_file, offset_to_e_shoff, SEEK_SET);
    fwrite(&cur_pos, sizeof(cur_pos), 1, out_file);

    fseek(out_file, cur_pos, SEEK_SET);
}


// ---------------------
// Section header table
// ---------------------

typedef struct {
   unsigned int name;
   unsigned int type;
   unsigned long flags;
   unsigned long addr;
   unsigned long offset;
   unsigned long size;
   unsigned int link;
   unsigned int info;
   unsigned long addralign;
   unsigned long entsize;
} SectionHeaderEntry;

// sh_type values
// Based on SHT in elfcpp.h
enum SectionHeaderTypeValues
{
    SH_TYPE_NULL = 0,
    SH_TYPE_PROGBITS = 1,
    SH_TYPE_SYMTAB = 2,
    SH_TYPE_STRTAB = 3,
    SH_TYPE_NOBITS =  8
};

// https://reverseengineering.stackexchange.com/questions/21460/meaning-of-flags-in-elf-section-header
enum SectionHeaderFlagsValues
{
    SH_FLAG_NULL = 0,
    SH_FLAG_WA = 3,
    SH_FLAG_AX = 6
};

enum SectionHeaderNameValues
{
    SH_NAME_NULL = 0,
    SH_NAME_TEXT = 0x1b,
    SH_NAME_DATA = 0x21,
    SH_NAME_BSS = 0x27,
    SH_NAME_SYMTAB = 0x01,
    SH_NAME_STRTAB = 0x09,
    SH_NAME_SHSTRTAB = 0x11
};

static SectionHeaderEntry section_header_entries[] = {
    {SH_NAME_NULL, SH_TYPE_NULL, SH_FLAG_NULL, 0, 0, 0, 0, 0, 0, 0},
    {SH_NAME_TEXT, SH_TYPE_PROGBITS, SH_FLAG_AX, 0, INIT_VAL, INIT_VAL, 0, 0, 4, 0},
    {SH_NAME_DATA, SH_TYPE_PROGBITS, SH_FLAG_WA, 0, INIT_VAL, 0, 0, 0, 1, 0},
    {SH_NAME_BSS, SH_TYPE_NOBITS, SH_FLAG_WA, 0, INIT_VAL, 0, 0, 0, 1, 0},
    {SH_NAME_SYMTAB, SH_TYPE_SYMTAB, SH_FLAG_NULL, 0, INIT_VAL, 0x78, 5, 4, 8, 0x18},
    {SH_NAME_STRTAB, SH_TYPE_STRTAB, SH_FLAG_NULL, 0, INIT_VAL, 0x6, 0, 0, 1, 0},
    {SH_NAME_SHSTRTAB, SH_TYPE_STRTAB, SH_FLAG_NULL, 0, INIT_VAL, 0x002c, 0, 0, 1, 0}
};

void gen_section_header_table() {
    // Update temporary values
    section_header_entries[1].offset = text_section_offset;
    section_header_entries[1].size = text_section_size;
    unsigned long next_offset = text_section_offset + text_section_size;
    section_header_entries[2].offset = next_offset;
    section_header_entries[3].offset = next_offset;
    section_header_entries[4].offset = symtab_section_offset;
    section_header_entries[5].offset = symtab_section_offset + section_header_entries[5 - 1].size;
    section_header_entries[6].offset = section_header_entries[6 - 1].offset + section_header_entries[6 - 1].size;

    fwrite(&section_header_entries, sizeof(section_header_entries), 1, out_file);
}


// ----------------------
// Parser
// ----------------------

// TODO: remove duplication
// Return the contents of a given file.
static char *read_file(char *path)
{
  FILE *fp;

  
  if (strcmp(path, "-") == 0)
  {
    // By convention, read from stdin if a given filename is "-".
    fp = stdin;
  }
  else
  {
    fp = fopen(path, "r");
    if (!fp)
      return NULL;
  }

  int buflen = 4096;
  int nread = 0;
  char *buf = malloc(buflen);

  // Read the entire file.
  for (;;)
  {
    int end = buflen - 2; // extra 2 bytes for the trailing "\n\0"
    int n = fread(buf + nread, 1, end - nread, fp);
    if (n == 0)
      break;
    nread += n;
    if (nread == end)
    {
      buflen *= 2;
      buf = realloc(buf, buflen);
    }
  }

  if (fp != stdin)
    fclose(fp);

  // Canonicalize the last line by appending "\n"
  // if it does not end with a newline.
  if (nread == 0 || buf[nread - 1] != '\n')
    buf[nread++] = '\n';
  buf[nread] = '\0';
  return buf;
}


static bool startswith(char *tgt, char *ref)
{
  return strncmp(tgt, ref, strlen(ref)) == 0;
}

char *skip(char *p, char *target) {
    char *cur = target;
    while (*cur && *p && *cur == *p) {
        cur++;
        p++;
    }
    if (*cur)
        error("expected %s", target);
    return p;
}

char *skip_space(char *p) {
    if (!isspace(*p))
        error("expected space at %s", p);
    while (isspace(*p))
        p++;
    return p; 
}

char *skip_comma_and_ignore_space(char *p) {
    bool found_comma = false;
    while (*p && (isspace(*p) || *p == ',')) {
        if (*p == ',') {
            found_comma = true;
        }
        p++;
    }
    if (!found_comma)
        error("expected commma at %s", p);
    return p;
}

int skip_integer(char **rest, char *p) {
    char *end = p;
    while (*end && (*end == '-' || (*end >= '0' && *end <= '9')))
        end++;
    // 123
    // 0123
    // p  e
    char *num_str = strndup(p, end - p);
    int value = atoi(num_str);
    *rest = end;
    return value;
}

static bool is_alpha(char c) { 
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_alnum(char c) {
  return is_alpha(c) || isdigit(c);
}

char *skip_symbol(char **rest, char *p) {
    // TODO: I didn't investigate the accurate spec of symbol
    char *end = p;
    while (*end && is_alnum(*end))
        end++;
    char *symbol_str = strndup(p, end - p);
    *rest = end;
    return symbol_str;
}

unsigned skip_register(char **rest, char *p) {
   if (startswith(p, "zero")) {
       *rest = skip(p, "zero");
       return REG_ZERO;
   }
   else if (startswith(p, "ra")) {
       *rest = skip(p, "ra");
       return REG_RA;
   }
   else if (startswith(p, "sp")) {
       *rest = skip(p, "sp");
       return REG_SP;
   }
   else if (startswith(p, "gp")) {
       *rest = skip(p, "gp");
       return REG_GP;
   }
   else if (startswith(p, "tp")) {
       *rest = skip(p, "tp");
       return REG_TP;
   }
   else if (startswith(p, "t0")) {
       *rest = skip(p, "t0");
       return REG_T0;
   }
   else if (startswith(p, "t1")) {
       *rest = skip(p, "t1");
       return REG_T1;
   }
   else if (startswith(p, "t2")) {
       *rest = skip(p, "t2");
       return REG_T2;
   }
   else if (startswith(p, "s0")) {
       *rest = skip(p, "s0");
       return REG_S0_FP;
   }
   else if (startswith(p, "fp")) {
       *rest = skip(p, "fp");
       return REG_S0_FP;
   }
   else if (startswith(p, "s1")) {
       *rest = skip(p, "s1");
       return REG_S1;
   }
   else if (startswith(p, "a0")) {
       *rest = skip(p, "a0");
       return REG_A0;
   }
   else if (startswith(p, "a1")) {
       *rest = skip(p, "a1");
       return REG_A1;
   }
   else if (startswith(p, "a2")) {
       *rest = skip(p, "a2");
       return REG_A2;
   }
   else if (startswith(p, "a3")) {
       *rest = skip(p, "a3");
       return REG_A3;
   }
   else if (startswith(p, "a4")) {
       *rest = skip(p, "a4");
       return REG_A4;
   }
   else if (startswith(p, "a5")) {
       *rest = skip(p, "a5");
       return REG_A5;
   }
   else if (startswith(p, "a6")) {
       *rest = skip(p, "a6");
       return REG_A6;
   }
   else if (startswith(p, "a7")) {
       *rest = skip(p, "a7");
       return REG_A7;
   }
   else if (startswith(p, "s2")) {
       *rest = skip(p, "s2");
       return REG_S2;
   }
   else if (startswith(p, "s3")) {
       *rest = skip(p, "s3");
       return REG_S3;
   }
   else if (startswith(p, "s4")) {
       *rest = skip(p, "s4");
       return REG_S4;
   }
   else if (startswith(p, "s5")) {
       *rest = skip(p, "s5");
       return REG_S5;
   }
   else if (startswith(p, "s6")) {
       *rest = skip(p, "s6");
       return REG_S6;
   }
   else if (startswith(p, "s7")) {
       *rest = skip(p, "s7");
       return REG_S7;
   }
   else if (startswith(p, "S8")) {
       *rest = skip(p, "s8");
       return REG_S8;
   }
   else if (startswith(p, "s9")) {
       *rest = skip(p, "s9");
       return REG_S9;
   }
   else if (startswith(p, "s10")) {
       *rest = skip(p, "s10");
       return REG_S10;
   }
   else if (startswith(p, "s11")) {
       *rest = skip(p, "s11");
       return REG_S11;
   }
   else if (startswith(p, "t3")) {
       *rest = skip(p, "t3");
       return REG_T3;
   }
   else if (startswith(p, "t4")) {
       *rest = skip(p, "t4");
       return REG_T4;
   }
   else if (startswith(p, "t5")) {
       *rest = skip(p, "t5");
       return REG_T5;
   }
   else if (startswith(p, "t6")) {
       *rest = skip(p, "t6");
       return REG_T6;
   }
   else if (startswith(p, "x")) {
       p = skip(p, "x");
       unsigned num = (unsigned)(skip_integer(rest, p));
       if (num > 31) {
           error("unknown register name x%d at %s", num, p);
           exit(1);
       }
       return num;
   }
   else {
       error("unknown register name at %s", p);
       exit(1); // Just to avoid warning
   }
}

Node *parse_statement(char *p) {
    Node *node = calloc(1, sizeof(Node));
    // Accept only limited instructions for now.
    if (startswith(p, "addi ")) {
        p = skip(p, "addi");
        p = skip_space(p);
        node->rd = skip_register(&p, p);
        p = skip_comma_and_ignore_space(p);
        node->rs1 = skip_register(&p, p);
        p = skip_comma_and_ignore_space(p);
        int imm_value = skip_integer(&p, p);
        // TODO: allow tailing spaces
        p = skip(p, "\n");
        node->kind = ND_INST_ADDI;
        node->imm = imm_value;
    }
    else if (startswith(p, "add ")) {
        p = skip(p, "add");
        p = skip_space(p);
        node->rd = skip_register(&p, p);
        p = skip_comma_and_ignore_space(p);
        node->rs1 = skip_register(&p, p);
        p = skip_comma_and_ignore_space(p);
        node->rs2 = skip_register(&p, p);
        p = skip(p, "\n");
        node->kind = ND_INST_ADD;
    }
    else if (startswith(p, "sub ")) {
        p = skip(p, "sub");
        p = skip_space(p);
        node->rd = skip_register(&p, p);
        p = skip_comma_and_ignore_space(p);
        node->rs1 = skip_register(&p, p);
        p = skip_comma_and_ignore_space(p);
        node->rs2 = skip_register(&p, p);
        p = skip(p, "\n");
        node->kind = ND_INST_SUB;
    }
    else if (startswith(p, "la ")) {
        // ex: la t1, global_symbol
        p = skip(p, "la");
        p = skip_space(p);
        node->rd = skip_register(&p, p);
        p = skip_comma_and_ignore_space(p);
        node->symbol = skip_symbol(&p, p);
        p = skip(p, "\n");
        node->kind = ND_INST_LA;
    }
    else if (startswith(p, "jr ra")) {
        p = skip(p, "jr ra");
        node->kind = ND_INST_JR;
    }
    return node;
}

bool is_instruction(char *p) {
    return startswith(p, "addi ") ||
        startswith(p, "jr ") ||
        startswith(p, "add ") ||
        startswith(p, "sub ") ||
        startswith(p, "la ");
}

Node *parse_asm(char *path) {
    char *p = read_file(path);
    if (!p)
        error("Empty input or file not found");

    Node head;
    Node *cur = &head;
    while (*p) {
        // Scan a line
        while (*p && *p != '\n') {
            if (*p == ' ') {
                p++;
                continue;
            }
            if (is_instruction(p)) {
                cur->next = parse_statement(p);
                cur = cur->next;
                break;
            }
            p++;
        }

        if (!*p)
            break;
        p++;
    }

    return head.next;
}

// ----------------------
// Main
// ----------------------

static char *input_path;

static void usage(int status) {

    fprintf(stderr, "kiwias [ -o <path> ] <file>\n");
    exit(status);
}

static void parse_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help"))
            usage(0);
        
        if (!strcmp(argv[i], "-o")) {
            if (!argv[++i])
                usage(1);
            output_path = argv[i];
            continue;
        }

        input_path = argv[i];
        if (*input_path != '-' && *input_path != '/')
        {
            // TOOD: Convert relative path to absolute path.
        } 
    }
    if (!input_path)
        error("No input");
}

int main(int argc, char **argv) {
    parse_args(argc, argv);
    nodes = parse_asm(input_path);
    out_file = fopen(output_path, "wb"); 
    if (out_file == NULL) {
        fputs("Failed to open file\n", stderr);
        exit(EXIT_FAILURE);
    }

    gen_elf_header();
    gen_program_header_table();
    
    gen_text_section();

    gen_data_section();
    gen_bss_section();
    gen_symtab_section();
    gen_strtab_section();
    gen_shstrtab_section();
    add_padding_before_section_header();
    fix_elf_header();
    gen_section_header_table();

    return 0;
}
