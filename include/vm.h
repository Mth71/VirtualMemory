#ifndef VM_H
#define VM_H

#include <stdio.h>
#include <stdbool.h>

// tamanhos 
#define PAGE_SIZE 256   // bytes por página (= bytes por frame)
#define NUM_PAGES 256   // entradas da tabela de páginas
#define TLB_SIZE  16    // entradas da TLB

#ifndef NUM_FRAMES
#define NUM_FRAMES 128  // frames da memória física
#endif

//algoritmos de substituição 
#define ALG_FIFO 0
#define ALG_LRU  1

//uma entrada da TLB
typedef struct tlb_entry {
    int page;
    int frame;
    bool valid;
    int last_use;
}TBL;


//memory.c
void mem_init(void);    // inicializa tabela de páginas e frames
int  mem_lookup(int page);  // frame da página, ou -1
int  mem_page_fault (int page, FILE *backing, int alg, int *evicted); // page fault: carrega a página e retorna o frame e grava em *evicted a página removida (ou -1)
void mem_touch(int frame, int now); // marca o frame como usado agora (LRU)
signed char mem_read (int frame, int offset);  // lê o byte do frame no deslocamento

//tlb.c 
void tlb_init(void);                        // todas as entradas inválidas
int  tlb_search (int page, int *frame); // busca com threads: índice, ou -1; grava o frame em *frame
int  tlb_input (int page, int frame, int alg, int now);  // insere; retorna o índice usado
void tlb_touch (int index, int now);    // marca a entrada como usada agora (LRU)
void tlb_invalidate (int page); // invalida a entrada dessa página

#endif