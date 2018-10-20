/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct disk *disk;
int *frames;

/*
se delcaran aca y se definen mas abajo para que el copilador no joda con variables usadas que 
pudieran no estar inicializadas
*/
void page_fault_handler( struct page_table *pt, int page ); //este sera el handler con el algoritmo custom
void page_fault_handler_rand( struct page_table *pt, int page);
void page_fault_handler_FIFO( struct page_table *pt, int page);


int main( int argc, char *argv[] )
{
	if(argc!=5) {
		/* Add 'random' replacement algorithm if the size of your group is 3 */
		printf("use: virtmem <npages> <nframes> <rand|fifo|custom> <sort|scan|focus>\n");
		return 1;
	}
	/* Aca podemos dejarlo asi o hacer getopt */
	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);
	frames = malloc(sizeof(int) * nframes);
	const char *algorithm = argv[3]; //este es el algoritmo a ocupar
	const char *program = argv[4];


	struct disk *disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}


	struct page_table *pt = page_table_create(npages, nframes, page_fault_handler);
	
	char *virtmem = page_table_get_virtmem(pt);
	char *physmem = page_table_get_physmem(pt);
	

	/*
	Hay que manejar la cracion de la tabla de pagina segun el algoritmo que se vaya a usar,
	en la parte page_table_create el ultimo argumento es el algoritmo que va a usar la pagina para
	manejar las faltas. Quizas usar pt como variable global pueda ayudar, aun que puede que solo baste con
	condicionar la inicializacion en base a la variable algorithm
	*/

	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}


	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[3]);

	}
	printf("==========page_table==========\n");
	page_table_print(pt);
	printf("==========page_table==========\n");

	page_table_delete(pt);
	disk_close(disk);

	free(frames);

	return 0;
	//printf("read: %d, write: %d, exec: %d\n", PROT_READ, PROT_WRITE, PROT_EXEC );
}


int cnt = 0;
//esta entrando bien aca cuando hay faltas de paginas
void page_fault_handler( struct page_table *pt, int page ) {
	int *frame = malloc(sizeof(int));
	int *bits = malloc(sizeof(int));
	printf("page: %d\n", page);
	//printf("%d\n", random_frame);
	page_table_get_entry(pt, page, frame, bits);
	int nframes = page_table_get_nframes(pt);
	int random_frame = lrand48() % nframes;


	//printf("bits: %d\n", bits[0]);
	if (cnt == page && cnt < nframes) {
		page_table_set_entry(pt, page, page, PROT_READ | PROT_WRITE | PROT_EXEC);
		frames[cnt] = page;
		cnt++;
	}
	if (cnt >= nframes && bits[0] == 0) {
		printf("[before set_entry] frame: %d, bits: %d\n", frame[0], bits[0]);

		char *physmem = page_table_get_physmem(pt);


		printf("frames[%d]: %d\n", random_frame, frames[random_frame]);
		disk_write(disk, frames[random_frame], &physmem[random_frame*PAGE_SIZE]); //pagina que ocupa el marco
		disk_read(disk, page, &physmem[random_frame*BLOCK_SIZE]); //pagina que quiere ocupar el marco		
		
		frames[random_frame] = page;
		page_table_set_entry(pt, page, random_frame, PROT_READ | PROT_WRITE | PROT_EXEC);


		page_table_get_entry(pt, page, frame, bits);
		printf("[after set_entry] frame: %d, bits: %d\n", *frame, *bits);

	}

	free(frame);
	free(bits);	
}