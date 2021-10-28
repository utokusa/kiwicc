#include <stdio.h>
#include <stdlib.h>

unsigned short data[] = {
    0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0003, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0003, 0x0002,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0003, 0x0003, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0000, 0x0010, 0x0001,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x6d00, 0x6961, 0x006e, 0x2e00, 0x7973, 0x746d, 0x6261, 0x2e00,
    0x7473, 0x7472, 0x6261, 0x2e00, 0x6873, 0x7473, 0x7472, 0x6261,
    0x2e00, 0x6574, 0x7478, 0x2e00, 0x6164, 0x6174, 0x2e00, 0x7362,
    0x0073, 0x0000, 0x0000, 0x0000
};



static char *output_path = "out.o";
static FILE *out_file;

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
    elf_header.e_shoff = 248;
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

unsigned char text_section_data[] = {
    0x13, 0x05, 0xa0, 0x02, 0x67, 0x80, 0x00, 0x00
};

void gen_text_section() {
    fwrite(&text_section_data, sizeof(text_section_data), 1, out_file); 
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
    {SH_NAME_TEXT, SH_TYPE_PROGBITS, SH_FLAG_AX, 0, 0x40, 8, 0, 0, 4, 0},
    {SH_NAME_DATA, SH_TYPE_PROGBITS, SH_FLAG_WA, 0, 0x48, 0, 0, 0, 1, 0},
    {SH_NAME_BSS, SH_TYPE_NOBITS, SH_FLAG_WA, 0, 0x48, 0, 0, 0, 1, 0},
    {SH_NAME_SYMTAB, SH_TYPE_SYMTAB, SH_FLAG_NULL, 0, 0x48, 0x78, 5, 4, 8, 0x18},
    {SH_NAME_STRTAB, SH_TYPE_STRTAB, SH_FLAG_NULL, 0, 0xc0, 0x6, 0, 0, 1, 0},
    {SH_NAME_SHSTRTAB, SH_TYPE_STRTAB, SH_FLAG_NULL, 0, 0xc6, 0x002c, 0, 0, 1, 0}
};

void gen_section_header_table() { 
    fwrite(&section_header_entries, sizeof(section_header_entries), 1, out_file);
}

// ----------------------
// Main
// ----------------------
int main() {
    out_file = fopen(output_path, "wb"); 
    if (out_file == NULL) {
        fputs("Failed to open file\n", stderr);
        exit(EXIT_FAILURE);
    }

    gen_elf_header();
    gen_program_header_table();
    gen_text_section();

    fwrite(&data, sizeof(data), 1, out_file); 
    
    gen_section_header_table();

    return 0;
}
