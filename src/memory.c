#include <stdio.h>
#include "vm.h"

static int page_table[NUM_PAGES];              // frame de cada página, ou -1
static int frame_page[NUM_FRAMES];             // página de cada frame, ou -1
static signed char memory[NUM_FRAMES][PAGE_SIZE];  // a memória física
static int last_use[NUM_FRAMES];               // para o LRU
static int next_free;                          // próximo frame livre (0 a NUM_FRAMES-1)
static int fifo_next;                          // próxima vítima do FIFO

void mem_init(void){
    for(int i = 0; i< NUM_PAGES; i++){
        page_table[i] = -1;
    }
    for(int i = 0 ; i<NUM_FRAMES; i++){
        frame_page[i] = -1;
        last_use[i] = 0;
    }
    next_free = 0;
    fifo_next = 0;
}

int mem_lookup(int page){
    return page_table[page];
}

int mem_page_fault(int page, FILE *backing, int alg, int *evicted){
    int frame;
    if(next_free < NUM_FRAMES){
        frame = next_free;
        next_free++;
        *evicted = -1;
    } else {
        if(alg == ALG_FIFO){
            frame = fifo_next;
            fifo_next=(fifo_next + 1) % NUM_FRAMES;
        }else{
            int vitima = 0;
            for(int i = 1; i<NUM_FRAMES;i++){   //supoe que a vitima e o 0
                if(last_use[i] < last_use[vitima]){
                    vitima = i;
                }
            }
            frame = vitima;
        }
        *evicted = frame_page[frame];
        page_table[*evicted] = -1;
    }
    fseek(backing, (page * PAGE_SIZE), SEEK_SET);
    fread(memory[frame], 1, PAGE_SIZE, backing);
    page_table[page] = frame; frame_page[frame] = page;
    return frame;
}
