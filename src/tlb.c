#include "vm.h"
#include <pthread.h>

static TBL tlb[TLB_SIZE];
static int tlb_next; // próxima posição do FIFO

typedef struct {
    int index;   
    int page;    
    int found;   
} ThreadArgs;

static void *check_page(void *arg){
    ThreadArgs *a = arg;    
    a->found = (tlb[a->index].valid && tlb[a->index].page == a->page);
    return NULL;
}

void tlb_init(void){
    for (int i = 0; i < TLB_SIZE; i++){
        tlb[i].valid = false;
    }
    tlb_next = 0;
}

int tlb_search(int page, int *frame){
    pthread_t threads[TLB_SIZE];
    ThreadArgs args[TLB_SIZE];

    for(int i = 0; i < TLB_SIZE; i++){
        args[i].index = i;
        args[i].page = page;
        args[i].found = 0;

        pthread_create(&threads[i], NULL, check_page, &args[i]);
    }

    for(int i = 0; i < TLB_SIZE; i++){
        pthread_join(threads[i], NULL);
    }

    for(int i = 0; i < TLB_SIZE; i++){
        if(args[i].found){
            *frame = tlb[i].frame;
            return i;
        }
    }

    return -1;
}

int tlb_input(int page, int frame, int alg, int now){
    int idx;
    if (alg == ALG_FIFO){
        idx = tlb_next;
        tlb_next = (tlb_next + 1) % TLB_SIZE;
    }
    else{
        idx = -1;
        for (int i = 0; i < TLB_SIZE; i++){
            if (!(tlb[i].valid)){
                idx = i;
                break;
            }
        }
        if(idx == -1){
            int vitima = 0;
            for(int i = 1; i<TLB_SIZE;i++){ //assume a vitima como 0
                if(tlb[i].last_use < tlb[vitima].last_use){
                    vitima = i;
                }
            }
            idx = vitima;
        }
    }
    tlb[idx].page = page;
    tlb[idx].frame = frame;
    tlb[idx].valid = true;
    tlb[idx].last_use = now;
    return idx;
}


void tlb_touch(int index, int now){
    tlb[index].last_use = now;
}

void tlb_invalidate(int page){
    for (int i = 0; i < TLB_SIZE; i++){
        if (tlb[i].valid && tlb[i].page == page){
            tlb[i].valid = false;
        }
    }
}