# Gerenciador de Memória Virtual (Implementação 6)

Implementação em C de um gerenciador de memória virtual (Silberschatz, *Operating System Concepts*, 10ª ed., projeto P-51): traduz endereços lógicos de 16 bits em físicos usando TLB e tabela de páginas, com 128 frames e substituição FIFO ou LRU tanto para as páginas quanto para a TLB. Disciplina de Infraestrutura de Software, CESAR School.

## Sistema operacional

Desenvolvido e testado no **Windows 11**, compilando e executando no **WSL** (Ubuntu 26.04.1 LTS), com `gcc` e `make` (pacote `build-essential`).

## Arquivos

| Arquivo | Descrição |
|---|---|
| `include/vm.h` | Constantes (tamanho da página, nº de páginas, frames e entradas da TLB), `struct` da entrada da TLB e protótipos das funções. |
| `src/main.c` | Valida os argumentos e o arquivo de endereços, traduz cada endereço (TLB → tabela de páginas → page fault) e gera o `correct.txt` com as estatísticas. |
| `src/memory.c` | Tabela de páginas, memória física (frames), carga de páginas do `BACKING_STORE.bin` e substituição de páginas FIFO e LRU. |
| `src/tlb.c` | TLB de 16 entradas: busca com uma thread por entrada (`pthread`), inserção com substituição FIFO ou LRU e invalidação de entradas. |
| `Makefile` | Compila (`make`), testa (`make test`) e limpa (`make clean`). |

## Entrada e saída

- **Argumentos**: arquivo de endereços, algoritmo de substituição de páginas e algoritmo de substituição da TLB (`fifo` ou `lru`).
- **`addresses.txt`**: um endereço lógico por linha (inteiro de 0 a 65535).
- **`BACKING_STORE.bin`**: armazenamento secundário de 65536 bytes (256 páginas de 256 bytes).
- **`correct.txt`**: gerado pelo programa, com uma linha por endereço (`Virtual address`, índice da `TLB`, `Physical address` e `Value`) e, ao final, o total de endereços traduzidos, page faults, taxa de page faults, TLB hits e taxa de TLB hits.

O `addresses.txt` e o `BACKING_STORE.bin` são fornecidos pelo professor e devem estar na pasta onde o programa é executado.

## Como compilar

```bash
make clean
make
```

Gera o executável `vm`.

## Como executar

```bash
./vm addresses.txt lru fifo
cat correct.txt
```

O primeiro algoritmo é o de substituição de páginas e o segundo, o da TLB. Qualquer combinação de `fifo` e `lru` é aceita.

## Como testar

Com o `correct.txt` do enunciado salvo como `correct_esperado.txt` na mesma pasta:

```bash
make test
```

Executa `./vm addresses.txt lru fifo` e compara a saída com `diff`. Se não imprimir diferenças, a saída está idêntica à esperada.

Casos de erro (o programa mostra uma mensagem, encerra com código 1 e não gera o `correct.txt`):

```bash
./vm                               # sem argumentos
./vm addresses.txt lru             # quantidade de argumentos errada
./vm addresses.txt abc fifo        # algoritmo de páginas inválido
./vm addresses.txt lru xyz         # algoritmo da TLB inválido
./vm naoexiste.txt lru fifo        # arquivo de endereços inexistente
```

Também são tratados: linhas do arquivo de endereços que não são números, endereços negativos ou maiores que 65535 (a mensagem indica a linha), arquivo sem endereços e `BACKING_STORE.bin` ausente.

## Como limpar

```bash
make clean
```

Remove o executável `vm` e o `correct.txt`.