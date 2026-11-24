#include "vm.h"

static TBL tlb[TLB_SIZE];
static int tlb_next; // próxima posição do FIFO

void tlb_init(void){
    for (int i = 0; i < TLB_SIZE; i++){
        tlb[i].valid = false;
    }
    tlb_next = 0;
}

int tlb_search(int page, int *frame){
    for (int i = 0; i < TLB_SIZE; i++)
    {
        if (tlb[i].valid && tlb[i].page == page)
        {
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