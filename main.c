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
#include <limits.h>

struct Stack { 
    int top; 
    unsigned capacity; 
    int* array; 
}; 

struct Stack* createStack(unsigned capacity) { 
    struct Stack* stack = (struct Stack*) malloc(sizeof(struct Stack)); 
    stack->capacity = capacity; 
    stack->top = -1; 
    stack->array = (int*) malloc(stack->capacity * sizeof(int)); 
    return stack; 
} 
  
int isFull(struct Stack* stack) { 
	return stack->top == stack->capacity - 1; 
} 
  
int isEmpty(struct Stack* stack) { 
	return stack->top == -1; 
} 
  
void push(struct Stack* stack, int item) { 
    if (isFull(stack)) 
        return; 
    stack->array[++stack->top] = item; 
} 

int pop(struct Stack* stack) { 
    if (isEmpty(stack)) 
        return INT_MIN; 
    return stack->array[stack->top--]; 
}


struct disk *disk;
int *frames; // el indice es el marco y el contenido la pagina que lo esta usando
int n_writes; // cuantas veces escribe en el disco
int n_reads; // cuantas veces lee del disco
int n_page_faults; // numero de faltas de pagina
struct Stack *stack; // para implementar el algortimo custom
const char *algorithm;
/*
se delcaran aca y se definen mas abajo para que el copilador no joda con variables usadas que 
pudieran no estar inicializadas
*/
void page_fault_handler( struct page_table *pt, int page);
void page_fault_handler_custom( struct page_table *pt, int page );
void page_fault_handler_rand( struct page_table *pt, int page);
void page_fault_handler_FIFO( struct page_table *pt, int page);

/* para manejar el error en caso de que fallara la creacion de la tabla */
void check_page_table(struct page_table *pt) {
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		exit(-1);
	}
}


int main( int argc, char *argv[] )
{
	n_writes = 0;
	n_reads = 0;
	n_page_faults = 0;
	if(argc!=5) {
		/* Add 'random' replacement algorithm if the size of your group is 3 */
		printf("use: virtmem <npages> <nframes> <rand|fifo|custom> <sort|scan|focus>\n");
		return 1;
	}
	
	/* aca podemos dejarlo asi o hacer getopt */
	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);
	frames = malloc(sizeof(int) * nframes);
	algorithm = argv[3]; //este es el algoritmo a ocupar
	const char *program = argv[4];

	stack = createStack(nframes);

	disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}


	struct page_table *pt = page_table_create(npages, nframes, page_fault_handler);
	
	
	char *virtmem = page_table_get_virtmem(pt);
	char *physmem = page_table_get_physmem(pt);


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
}

int last_fault = 0;
int cnt = 0;
void page_fault_handler( struct page_table *pt, int page) {
	n_page_faults++;
	int *frame = malloc(sizeof(int));
	int *bits = malloc(sizeof(int));
	page_table_get_entry(pt, page, frame, bits);
	int n_frames = page_table_get_nframes(pt);

	if (cnt == page && cnt < n_frames) {
		push(stack, *frame);
		page_table_set_entry(pt, page, page, PROT_READ | PROT_WRITE);
		frames[cnt] = page;
		cnt++;
		last_fault++;
	}

	if (cnt >= n_frames && bits[0] == 0) {
		if (!strcmp(algorithm, "rand")) {
			page_fault_handler_rand(pt, page);
			
		} else if (!strcmp(algorithm, "fifo")) {
			page_fault_handler_FIFO(pt, page);
			
		} else if (!strcmp(algorithm, "custom")) {
			page_fault_handler_custom(pt, page);

		} else {
			fprintf(stderr, "unknown algorithm: %s\n", algorithm);

		}
	}
}

void page_fault_handler_rand( struct page_table *pt, int page ) {
	int *frame = malloc(sizeof(int));
	int *bits = malloc(sizeof(int));
	page_table_get_entry(pt, page, frame, bits);
	int nframes = page_table_get_nframes(pt);
	int random_frame = lrand48() % nframes;

	char *physmem = page_table_get_physmem(pt);

	
	disk_write(disk, frames[random_frame], &physmem[random_frame*PAGE_SIZE]); //pagina que ocupa el marco
	n_writes++;
	disk_read(disk, page, &physmem[random_frame*BLOCK_SIZE]); //pagina que quiere ocupar el marco		
	n_reads++;
	frames[random_frame] = page;
	page_table_set_entry(pt, page, random_frame, PROT_READ | PROT_WRITE);
	page_table_get_entry(pt, page, frame, bits);

	free(frame);
	free(bits);	
}

void page_fault_handler_FIFO(struct page_table *pt, int page) {
	int *frame = malloc(sizeof(int));
	int *bits = malloc(sizeof(int));
	printf("page: %d\n", page);

	/* reemplazo de pagina fifo */

	free(frame);
	free(bits);

}

void page_fault_handler_custom(struct page_table *pt, int page) {
	int *frame = malloc(sizeof(int));
	int *bits = malloc(sizeof(int));
	page_table_get_entry(pt, page, frame, bits);

	char *physmem = page_table_get_physmem(pt);
	int last_used_frame = pop(stack);
	printf("last_used_frame: %d\n", last_used_frame);
	if (last_used_frame == last_fault) {
		last_used_frame = pop(stack) + 1;
		printf("last_used_frame next: %d\n", last_used_frame);
	}

	push(stack, *frame);
	disk_write(disk, frames[last_used_frame], &physmem[last_used_frame*PAGE_SIZE]); //pagina que ocupa el marco
	n_writes++;
	disk_read(disk, page, &physmem[last_used_frame*BLOCK_SIZE]); //pagina que quiere ocupar el marco		
	n_reads++;
	frames[last_used_frame] = page;
	page_table_set_entry(pt, page, last_used_frame, PROT_READ | PROT_WRITE);
	page_table_get_entry(pt, page, frame, bits);

	last_fault = last_used_frame;
	free(frame);
	free(bits);	

}