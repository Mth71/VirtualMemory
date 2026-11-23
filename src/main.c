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
        fprintf(stderr, "Erro: Não foi possível abrir {%s}\n", filename);
        return -1;
    }
    char line[LINE_MAX_LEN];
    int lineno = 0, count = 0, addr;
    while(fgets(line, sizeof line, f) != NULL){
        lineno++;
        strip_trailing(line);
        if(is_blank(line)) continue;
        if(parse_address(line, &addr) != 0){
            fprintf(stderr, "Erro: arquivo {%s} na linha {%d} invalido\n", filename, lineno);            
            fclose(f);
            return -1;
        }
        count++;
    }
    fclose(f);
    if(count == 0){
        fprintf(stderr, "Erro: arquivos sem endereço\n");
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
        fprintf(stderr, "Erro: algoritmo da página {%s} é inválido\n", argv[2]);
        return 1;
     }
    if(tlb_alg  < 0){
        fprintf(stderr, "Erro: algoritmo da TLB {%s} é inválido\n", argv[3]);
        return 1;
    }

    int total = validate_addresses(argv[1]);
    if(total < 0) return 1;

    // (a tradução vem aqui depois)
    return 0;
}