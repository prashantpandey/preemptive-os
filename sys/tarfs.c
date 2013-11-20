#include <defs.h>
#include <stdio.h>
#include <sys/tarfs.h>
#include <common.h>
#include <stdio.h>

uint64_t is_file_exists(char* filename)
{
/*
struct posix_header_ustar {
        char name[100];
        char mode[8];
        char uid[8];
        char gid[8];
        char size[12];
        char mtime[12];
        char checksum[8];
        char typeflag[1];
        char linkname[100];
        char magic[6];
        char version[2];
        char uname[32];
        char gname[32];
        char devmajor[8];
        char devminor[8];
        char prefix[155];
        char pad[12];
};
*/

	printf("\n Binary tarfs start:  %x", &_binary_tarfs_start);
	struct posix_header_ustar *test_tarfs = (struct posix_header_ustar *)&_binary_tarfs_start;
	int i = 1, temp = 512;
	uint64_t size;
	//printf("\n Name %s \t Mode %s \t uid %s \t gid %s\t",test_tarfs->name, test_tarfs->mode, test_tarfs->uid, test_tarfs->gid);
	while(strlen(test_tarfs->name) != 0)
	{
		test_tarfs = (struct posix_header_ustar *)(&_binary_tarfs_start + temp);
		size = octalToDecimal(stoi(test_tarfs->size));
	//	printf("\nName of file is %s and size in octal is %d", test_tarfs->name, test_tarfs->size);
		if(strlen(test_tarfs->name) == 0)
			return 999;
		if(strcmp(test_tarfs->name, filename) >= 0)
			return temp + 512;
		if(size == 0)
			temp = temp + 512;
		else
			temp +=  (size%512==0) ? size + 512 : size + (512 - size%512) + 512;
		//printf("    %d", temp);
		i += 1;
	}
	//printf("%d", size);
	return 0;
}

void get_file_sections(char* filename)
{
	uint64_t offset = is_file_exists(filename);
	printf("\n %d File Offset", offset);
	//char *file_start= (char *)(&_binary_tarfs_start + offset);
	struct posix_header_ustar *test_tarfs = (struct posix_header_ustar *)(&_binary_tarfs_start + offset - 512);
	printf("\n Magic number %s",test_tarfs->magic);
	printf("\n Name %s \t Mode %s \t uid %s \t gid %s\t",test_tarfs->name, test_tarfs->mode, test_tarfs->uid, test_tarfs->gid);
	/*Elf64_Ehdr *elfhdr = (Elf64_Ehdr *) (&_binary_tarfs_start + offset);
	printf("\nentry %x", elfhdr->e_entry);
	printf("\nphysoffset %x num %d", elfhdr->e_phoff, elfhdr->e_phnum);
	printf("\nssoffset %x num %d", elfhdr->e_shoff, elfhdr->e_shnum);*/
	Elf_hdr *elfhdr1 = ( Elf_hdr *) (&_binary_tarfs_start + offset);
	printf("\n %x", elfhdr1->e_entry);
	printf("\nphysoffset %x num %d", elfhdr1->e_phoff, elfhdr1->e_phnum);
	printf("\nssoffset %x num %d", elfhdr1->e_shoff, elfhdr1->e_shnum);
	
	Elf64_Phdr *elfhdr_p = (Elf64_Phdr *)((uint64_t)elfhdr1 + (uint64_t)elfhdr1->e_phoff);
	printf("\nphysoffset %x ", elfhdr_p->p_paddr);
	Elf64_Shdr *elfhdr_s = (Elf64_Shdr *)((uint64_t)elfhdr1 + (uint64_t)elfhdr1->e_shoff);
//	int i = 0;
	
	//for(i=0; i<16; i++)
	//{
		printf("\noffset %x ", elfhdr_s->sh_offset);
		printf("\naddress %x ", elfhdr_s->sh_addr);
		printf("  type %x ", elfhdr_s->sh_type);
		printf("  size %x ", elfhdr_s->sh_size);
		printf("  name %c ", elfhdr_s->sh_name);
	//	elfhdr_s++;
	//}
	
	//puts((char *)*elfhdr_s->sh_name);
	/*struct Proghdr *ph, *eph;
	if (elfhdr->e_magic != ELF_MAGIC)
		printf("elf header's magic is not correct\n");
	printf("\n %x", elfhdr);
	printf("\n %x", elfhdr->e_phoff);
	printf("\n %x", elfhdr->e_phnum);
	printf("\n %x", elfhdr->e_shnum);
	ph = (struct Proghdr *) ((uint8_t *) elfhdr + elfhdr->e_phoff);
	printf("\n %x", ph->p_filesz);
	printf("\n %x", ph->p_offset);
	printf("\n %x", elfhdr->e_shnum);

	eph = ph + elfhdr->e_phnum;
	printf("\n %x", eph->p_filesz);
	printf("\n %x", ph->p_offset);
	struct Secthdr *sectHdr = (struct Secthdr *)elfhdr + elfhdr->e_shoff;
	printf("\n %x", elfhdr->e_shoff);
	printf("\n %x", sectHdr->sh_offset);*/
	//puts((char *)(sectHdr->sh_name));
	/*lcr3(PADDR(e->env_pgdir));*/

	/*int i;
	for(i=0; i< 1000; i++)
	{
		printf("%c", *file_start++);
	}*/
}

