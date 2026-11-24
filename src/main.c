#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vm.h"

#define LINE_MAX_LEN 256
#define MAX_ADDRESS  65535         // o maior endereço válido (16 bits)

// tira do fim da linha TODOS os caracteres de espaço (' ', '\t', '\n', '\r')
static void strip_trailing(char *s){
    size_t tamanho = strlen(s);
    while(tamanho > 0 && isspace((unsigned char)s[tamanho - 1])){
        s[tamanho - 1] = '\0'; // apaga o último caractere
        tamanho--; 
    }
}

// 1 se a linha estiver vazia (só espaços), 0 se tiver conteúdo
static int is_blank(const char *s){
    for(int i = 0 ; s[i] != '\0'; i++){
        if(!(isspace((unsigned char)s[i]))){
            return 0;
        }
    }
    return 1;
}
// converte a linha num endereço entre 0 e MAX_ADDRESS
// retorna 0 e grava em *out; -1 se não for um endereço válido
static int parse_address(const char *s, int *out){
    char *end;
    long v = strtol(s, &end, 10);
    if(end == s || *end != '\0' || v < 0 || v> MAX_ADDRESS){
        return -1;
    }
    *out = (int)v;
    return 0;
}

// confere o arquivo inteiro; retorna quantos endereços tem, ou -1 se houver erro
static int validate_addresses(const char *filename){
    FILE *f = fopen(filename, "r");
    if(f == NULL){
        fprintf(stderr, "Erro: Não foi possível abrir %s\n", filename);
        return -1;
    }
    char line[LINE_MAX_LEN];
    int lineno = 0, count = 0, addr;
    while(fgets(line, sizeof line, f) != NULL){
        lineno++;
        strip_trailing(line);
        if(is_blank(line)) continue;
        if(parse_address(line, &addr) != 0){
            fprintf(stderr, "Erro: arquivo %s na linha %d invalido\n", filename, lineno);            
            fclose(f);
            return -1;
        }
        count++;
    }
    fclose(f);
    if(count == 0){
        fprintf(stderr, "Erro: arquivo sem endereço\n");
        return -1;
    }
    return count;
}

// "fifo" -> ALG_FIFO, "lru" -> ALG_LRU, outro -> -1
static int parse_alg(const char *s){
    if(strcmp(s, "fifo") == 0){
        return ALG_FIFO;
    }
    if(strcmp(s, "lru") == 0){
        return ALG_LRU;
    }
    return -1;
}

int main(int argc, char *argv[]){
    if(argc != 4){
        fprintf(stderr, "Uso: ./vm <arquivo> <fifo|lru> <fifo|lru>\n");      
        return 1;
    }

    int page_alg = parse_alg(argv[2]);
    int tlb_alg  = parse_alg(argv[3]);
    if(page_alg < 0){ 
        fprintf(stderr, "Erro: algoritmo da página %s é inválido\n", argv[2]);
        return 1;
     }
    if(tlb_alg  < 0){
        fprintf(stderr, "Erro: algoritmo da TLB %s é inválido\n", argv[3]);
        return 1;
    }

    int total = validate_addresses(argv[1]);
    if(total < 0) return 1;

    FILE *backing = fopen("BACKING_STORE.bin", "rb");
    if(backing == NULL){
        fprintf(stderr, "Erro: Não foi possivel abrir o arquivo{BACKING_STORE.bin}\n");
        return 1;
    }
    FILE *in  = fopen(argv[1], "r");       // já validado, abre de novo para traduzir
    FILE *out = fopen("correct.txt", "w");
    if(in == NULL || out == NULL){
        fprintf(stderr, "Erro: nao foi possivel abrir os arquivos\n");
        if(in != NULL){
            fclose(in); 
        }
        if(out != NULL){
            fclose(out); 
        }
        fclose(backing); 
        return 1;
    }  

    mem_init();
    tlb_init(); 

    char line[LINE_MAX_LEN];
    int addr, now = 0, faults = 0, hits = 0, total_lidos = 0;
    while(fgets(line, sizeof line, in) != NULL){
        strip_trailing(line);
        if(is_blank(line)) continue;
        parse_address(line, &addr);         // já validado, não falha
        now++;
        total_lidos++;

        int page   = (addr >> 8) & 0xFF;    // os 8 bits de cima
        int offset = addr & 0xFF;   // os 8 bits de baixo

        int frame;
        int idx = tlb_search(page, &frame);
        if(idx >= 0){                             
            hits++;
            tlb_touch(idx, now);
        } else {                                  
            frame = mem_lookup(page);             
            if(frame == -1){
                faults++;
                int removida;
                frame = mem_page_fault(page, backing, page_alg, &removida);
                if(removida != -1){                         
                    tlb_invalidate(removida);
                }
            }
            idx = tlb_input(page, frame, tlb_alg, now);
        }
        mem_touch(frame, now);                
        int fisico = (frame * PAGE_SIZE) + offset;
        int valor  = mem_read(frame, offset);
        fprintf(out, "Virtual address: %d TLB: %d Physical address: %d Value: %d\n",addr, idx, fisico, valor);    
    }

    fprintf(out, "Number of Translated Addresses = %d\n", total_lidos);
    fprintf(out, "Page Faults = %d\n",faults);
    fprintf(out, "Page Fault Rate = %.3f\n", (double) faults / total_lidos);
    fprintf(out, "TLB Hits = %d\n", hits);
    fprintf(out, "TLB Hit Rate = %.3f\n", (double) hits / total_lidos);

    fclose(in); 
    fclose(out); 
    fclose(backing);
    return 0;
}