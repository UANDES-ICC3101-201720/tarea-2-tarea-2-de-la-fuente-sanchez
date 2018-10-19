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

int npages;
int nframes;
struct disk *disk;

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
	const char *algorithm = argv[3]; //este es el algoritmo a ocupar
	const char *program = argv[4];


	struct disk *disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}


	struct page_table *pt = page_table_create(npages, nframes, page_fault_handler);
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

	char *virtmem = page_table_get_virtmem(pt);

	//char *physmem = page_table_get_physmem(pt);

	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[3]);

	}

	page_table_delete(pt);
	disk_close(disk);

	return 0;
}


//esta entrando bien aca cuando hay faltas de paginas
void page_fault_handler( struct page_table *pt, int page ) {
	/* 
	ignorame
	if (npages == nframes) {
		printf("hola\n");
	}
	if (npages > nframes)
	{
		printf("hola2\n");
	} 
	ignorame
	*/
	printf("entra a page_fault_handler:\n");
	page_table_set_entry(pt, page, 0, PROT_READ); //ese 0 en realidad debiera ser el marco que esa pagina quiere ocupar
	char *physmem = page_table_get_physmem(pt);
	disk_read(disk, page, &physmem[0*PAGE_SIZE]); //aca hay un error, ese 0 es el mismo del page_table_set_entry
	printf("error aca\n");
	exit(1);
	
}