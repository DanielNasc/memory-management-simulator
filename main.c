#include "globals.h"
#include "utils.h"
#include "sleep.h"
#include "print.h"
#include "input.h"
#include "mem_brush.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h> // typing
#include <pthread.h> // threads
#include <uchar.h>
#include <stdarg.h> // variable arguments


void boot(); /* Boot (virtual) OS emulation */


/* Mi(V)OSEmu: Mini (Virtual) Operational System Emulation
 * Emulates (virtual) memory processing algorithms.
 */
int main(int argc, char **argv)
{
    utils_init(); /* Setup lib */

    /* Filter args */
    for (argv++; --argc != 0; ++argv)
        if (**argv == '-')
            switch (*(++*argv)) {
                case 'B':
                case 'm': { // set memory size (bytes)
                    if (argc == 1)
                        break; // no arg after flag
                    ++argv;
                    long size = strtol(*argv, argv + 1, 10);
                    if (size <= 0)
                        print_err("Invalid memory size, argument ignored, fallback to default (1KB)", );
                    else
                        _mem_size = size;
                }; break;
                case 's':
                    skip |= SKIP_ALL;
                    break;
                case '=': {
                    skip |= SKIP_TO_EVENT;
                    skip_to_event = atoi(++*argv);
                } break;
                default:
                    print_err("Invalid argument ignored!");
            }


    /* Allocation starts here */
    int memory[_mem_size];
    _mem = memory; // defines system (virtual) memory

    for (size_t i = 0; i < _mem_size; ++i)
        memory[i] = EOF; // Fill it with "trash memory" (blank for visualization purposes)

    // start registers
    _registers.pc = 0;
    _registers.stack_tail = _raw_end = _mem_size - (size_t)(_mem_size * _mem_ratio);
    _registers.heap_end = _mem_size;
    _registers.last_mod.start = _registers.last_mod.end = EOF;

    boot();

    return EXIT_SUCCESS;
}


/* Print a dialog by chicko. Escape sequences %
 * %a: pass a function call (must receive no args and return void),
 * %n: non-blocking mode: line will not wait for user input
 * dialog will stop until completed. */
void print_chicko(char const *s, ...)
{
#define CHECK_CHAR(C)\

    va_list ap; /* Variable argument list */
    va_start(ap, s); /* Point to first arg. */

    printf("🐣 " CLIS_CHICKO);
    bool block = true;
    int arg = 0;
    size_t lines = 1;

    while (*s != '\0') {
        switch (*s) {
            case '\n': lines++; break;
            case '%':
                switch (*++s) {
                    case 'a':
                        void (* action)(void) = va_arg(ap, void (*)(void));
                        fflush(stdout); // In case of skip mode
                        action();
                        ++s;
                        break;
                    case 'n': // Non blocking mode
                        block = false;
                    break;
                    default: break;
                }
                default: break;
        }
        if (!skip) {
            fflush(stdout);
            msleep(15);
        }
        putchar(*(s++));
    }
    printf("\033[m\n");

    if (block && !(skip & SKIP_TO_EVENT)) // block input
        wait_input_event(' ');

    while (lines-- != 0)
        printf("\33[1A\33[2K\r"); // Clear last line
    va_end(ap);
}
#include <string.h>


/* boot the (virtual) mini operational system emulation */
void boot()
{
    pthread_t input_event;
    disable_canonical();
    pthread_create(&input_event, NULL, track_keyboard_input, NULL);

    if (!skip) {
        pthread_t thread_ids[2];
        size_t timeout = 3 * SEC;
        pthread_create(thread_ids, NULL, wait, &timeout);
        pthread_create(thread_ids + 1, NULL, dots, NULL);
        // pthread_join(thread_ids[0], NULL);
        pthread_join(thread_ids[1], NULL);

        printf("\33[2K\r"); // Erase last line
    }

    // Hello World process
    struct command proc_hello_world[] = {
        { .line="print(\"Hello\")", .call=NULL, .args=NULL },
        { .line="putchar(' ')", .call=NULL, .args=NULL },
        { .line="print(\"World\")", .call=NULL, .args=NULL },
    };
    char hw_scope_name[] = "main";
    struct enter_scope_args scope = { .pointer=0, .scope_name=hw_scope_name };
    int stream[] = { (char)0, EOF };
    struct set_var_args sva = {.pointer=&_registers.stack_tail, .data_stream=stream};

    // Tutorial Process
    struct command proc_tuto[] = {
        { .line="int main() {", .call=enter_scope, .args=&scope },
        { .line="\t""int i;", .call=inc_sp, .args=NULL },
        { .line="\t""i = 0;", .call=set_var, .args=&sva} ,
        { .line="}", .call=none, .args=NULL },
    };

    // FIXME
    // Recursive Process
    char rec_scope_name[] = "rec";
    struct {
        size_t lim; // limit of recursion
        int arg_value; // value of function argument
        struct inc_var_args inc_args; // args of increment call
        struct recursive_args args; // args of recursive call
    } rec = { .args={ .pointer=0, .scope_name=scope.scope_name },
              .lim=_mem_size - _raw_end + 1 };
    rec.inc_args.pointer = rec.args.arg_value = &rec.arg_value;

    char rec_header_fmt[] = "int rec(int i = %d) {";
    struct command proc_recursion[] = {
        { .line="int rec(int i = 0) {", .call=recursive_call, .args=&scope },
        { .line="", .call=NULL, .args=NULL }, // branch
        { .line="\t\ti++", .call=inc_var, .args=&(rec.inc_args) },
        { .line="\t\trec(i)", .call=recursive_call, .args=&(rec.args) },
        { .line="}", .call=NULL, .args=NULL },
    };
    sprintf(proc_recursion[1].line, "\tif (i < %lu)", rec.lim); // set lim into branch
    rec.args.header = proc_recursion[0].line;
    rec.args.header_fmt = rec_header_fmt;

    // Setup default process
    _emu_reg.commands = proc_hello_world;
    _emu_reg.size = sizeof(proc_hello_world) / sizeof(struct command);
    _emu_reg.step = _emu_reg.size + 1;

#define MAX_STEP 25
    int skip_case = -1;
    for (size_t step = 0; step != MAX_STEP; step++) {
        step -= step_proc();

        if (skip_to_event && (step == skip_to_event))
            skip &= ~SKIP_TO_EVENT; // remove skipping

        printf("\033[32m"
               "Mini (virtual) Operational System emulation started!\033[m\n");

        print_memory();
        print_code();
        putchar('\n');
        print_mem_hex();
        putchar('\n');

        switch (step) {
            case 0: {
                print_chicko("Hello World!" CLIS_RESET "\n" CLIS_CHICKO
                            "Eu sou o galinho m(v)x, vou ser o seu guia nessa simulação!"
                            CLIS_RESET "\n\t\t" CLIS_CHICKO "Pressione "
                            CLIS_CK_EMPHASIS("[BARRA DE ESPAÇO]") " para continuar!");
                print_chicko("Nesta simulação do nosso Mini (virtual) Sistema Operacional "
                            "descobriremos como ocorre o gerenciamento de memória em um OS.");
            } break;
            case 1: {
                print_chicko("Na barra superior está o espaço utilizado na sua memória no momento.");
                // TODO -> Destacar a barra superior
            } break;
            case 2: {
                print_chicko("No centro você verá o código do processo que iremos executar no nosso sistema.");
                // TODO -> Destacar barra do centro
            } break;
            case 3: {
                print_chicko("Logo abaixo você pode ver o que acontece com a memória conforme o processo é executado.");
                // TODO -> Destacar o inferior
            } break;
            case 4: {
                print_chicko("Pelo propósito dessa simulação, iremos focar apenas na memória do espaço do usuário."
                            CLIS_RESET "\n" CLIS_CHICKO "Então eu escondi pra você a parte referente ao interior "
                            "do sistema. 😳 " CLIS_CK_STRIKE("Favor, respeitar ok? 👉👈"));
                setup_proc(proc_tuto, sizeof(proc_tuto) / sizeof(struct command));
                step++;
            } break;
            case 5: {
                if (skip_case != 5)
                    print_chicko("Um " CLIS_CK_EMPHASIS("processo") " selvagem apareceu!");
                print_chicko("Para controlar a execução pressione o botão " CLIS_CK_EMPHASIS("[ESPAÇO]")
                            " e iremos mostrar o que acontece. " CLIS_CK_BOLD("Tente!"));
                skip_case = 5;
            } break;
            case 6: {
                print_chicko("Muito bem, você deve ter entendido. " CLIS_CK_ITALICS("Certo?"));
                // setup_proc(proc_recursion, sizeof(proc_recursion) / sizeof(struct command));
                step++;
            } break;
            case 7: {
                // FIXME
                // if (_registers.stack_tail < _registers.heap_end - 1) {
                //     print_chicko("Agora iremos mostrar o que acontecesse quando temos uma "
                //                 CLIS_CK_EMPHASIS("recursão") " no programa.%n");
                //     msleep(200);
                // }
                // else {
                //     print_chicko("Oh no! Que problemão, a nossa memória vai acabarrrrrrrrrrrrrrrrrrrrrrrrrrrrrr... ");
                //     // TODO -> Implementar o "travamento" do chicko no método print em %Z (frozen) (continua até pressionar espaço)
                //     step++;
                // }
            } break;
            case 8: {
                // print_chicko(CLIS_CK_BOLD("Droga, eu travei!") " Isso que acabou de acontecer é "
                //             "chamado de \"Stack Overflow\"... " CLIS_CK_ITALICS("Não o site bobão!"));
                // print_chicko("Bem, podemos resolver o problema anterior usando uma estratégia "
                //             "chamada \"swapping\", eu vou mostrar pra você.");
                printf("Agora vamos conhecer uma técnica chamada " CLIS_CK_EMPHASIS("\"swapping\"") "!");
                // TODO -> <mostrar swap>
            }; break;
            case 9: {
                print_chicko("Esta nova partição aqui é chamada de... " CLIS_CK_ITALICS("isso ")
                             CLIS_CK_EMPHASIS("SWAP") ",\né basicamente um arquivo no disco do PC. "
                             "Vamos ver agora!");
                // TODO -> <loop continua e swap é preenchido, e para antes de travar>
            }
            case 10: {
                print_chicko(CLIS_CK_BOLD("Ótimo!"));
                print_chicko("Mas eu posso fazer mais do que isso. Agora vamos ver o que "
                            "acontecesse quando há vários programas sendo executados.");
                // TODO -> <carrega dois códigos main na tela>
            }; break;
            case 11: {
                print_chicko("Queremos tirar proveito dos recursos da máquina, então faremos "
                            "esses dois programas serem carregados simultaneamente.");

                print_chicko("Poderemos então executar um após o outro!");
                // TODO -> <mostrar execução single thread>
            }; break;
            case 12: {
                print_chicko("Ou alternar entre os dois, em sequência. Isso é útil "
                            "principalmente quando há múltiplas threads.");
                // TODO -> <mostrar execução multithread>
            }; break;
            case 13: {
                print_chicko("É isso, eu tenho dois corações! h3h3");

                print_chicko("Você deve ter notado os endereços que os programas estão usando colidem.\n"
                            "Na verdade, nós não estamos usando os endereços da memória física.\n"
                            "Isso facilita muitos processos pra nós.");
                print_chicko("Esse endereço (virtual) pode ser atribuído em tempo de compilação,\n"
                            "tempo de carga, ou em tempo de execução, chamamos ele de memória lógica.");
                print_chicko("Eu tenho um circuito especial pra lidar com isso no meu cérebro.\n"
                            CLIS_CK_EMPHASIS("O MMU!") " Ele faz a tradução dos endereços e também "
                            "a proteção deles entre processos.\nNote que o endereçamento físico"
                            "é realocado sempre que manipulamos a\nlocalização dos processos"
                            "na memória, para um swap por exemplo.");
                // TODO -> <Stack Overflow em um dos processos>
            }; break;
            case 14: {
                print_chicko("Olhe, temos um problema! O segundo programa não pode continuar\n"
                            "por quê não há espaço o suficiente na máquina.");
                print_chicko("Podemos resolver isso, balanceando o uso da memória.");
                // TODO -> <mostrar alocação de tamanhos diferentes>
            }; break;
            case 15: {
                print_chicko("Ok! Agora vou te mostrar algo legal... Algo PERIGOSO!!! ☠️ "
                            CLIS_CK_UNDER("RADICAL! 😎 ") CLIS_CK_EMPHASIS("Ponteiros"));
                // TODO -> <carregar demonstração da memória heap>
            } break;
            case 16: {
                print_chicko("Esse espaço no final é chamado de heap (ou monte), é aqui que\n"
                            "fica a memória adicional. Muitos programas utilizam a heap\n"
                            "quando querem crescer variavelmente, é um pouco de trabalho\n"
                            "para mim, mas é um trabalho honesto 🤠.");
                // TODO -> <simular erro de segmentation fault
            }; break;
            case 17: {
                print_chicko("Espere, você não deveria acessar esta memória, o outro programa\n"
                            "vai ter problema. " CLIS_CK_BOLD("Erro!!! " CLIS_CK_EMPHASIS("Erro!!")));
                print_chicko("Como eu esperava, esse é mais um caso de " CLIS_CK_BOLD("Segmentation fault.")
                            "\nUm programador responsável nunca permitiria ponteiros soltos assim.\n"
                            "Eu não permitirei acessar além da fronteira. "
                            CLIS_CK_EMPHASIS("Segmentation fault!") " Ouviu bem? 😠");
                // TODO -> <carregar código de acesso null>
            }; break;
            case 18: {
                print_chicko("Espere, esse endereço não existe! O endereço 00 é especial, "
                            "nós chamamos ele de" CLIS_CK_BOLD("NULL") ".\nÉ uma exceção especial "
                            "pra facilitar o trabalho de alguns programadores.\n"
                            "Nós chamamos isso de " CLIS_CK_EMPHASIS("Null pointer Exception"));

                print_chicko("Bem, você já entendeu como processos funcionam. Vamos ignorar\n"
                            "esta parte por enquanto, e focar em entender o que acontece\n"
                            "quando há " CLIS_CK_BOLD("muitos processos") " na memória!");
                // TODO -> <simulação com vários processos (sem código), alguns encerrando>
            }; break;
            case 19: {
                print_chicko("Precisamos encaixar este novo processo, mas não há espaço o\n"
                            "suficiente no momento, nós vamos então esperar pelos outros\n"
                            "encerrarem para inserí-lo. Ou, eu posso mover alguns deles de\n"
                            "menor prioridade para a swap, hihi 🤓");
                // TODO -> <simulação com vários processos, mais deles encerrando>
            }; break;
            case 20: {
                print_chicko("Agora que temos espaço o suficiente podemos colocá-lo...\n"
                            "Mas onde vamos por ele?");
                // TODO -> <simular first fit (alocando no começo)>
            }; break;
            case 21: {
                print_chicko("Faz sentido, no primeiro espaço. Mas, Bem, a gente pode se\n"
                            "preparar melhor para este problema do espaço no futuro.\n"
                            "Que tal inserirmos em outro local?! 🧐");
                // TODO -> <simulação best fit>
            }; break;
            case 22: {
                print_chicko("Eu decidi colocar ele no melhor lugar, onde o processo vai deixar\n"
                            "lacunas de espaço menores. Mas nós podemos fazer um pouco diferente!");
                // TODO -> <simulação worst fit>
            }; break;
            case 23: {
                print_chicko("Dessa vez eu decidi colocar ele no pior lugar? Por quê, você me pergunta?");
                print_chicko("Olha, os próximos processos vão ter tooodo esse espaço livre\n"
                            "pra caber ali, isso é bom, não é mesmo?");

                print_chicko("Bem, eu não estou satisfeito. Ainda há espaço que não estamos usando...\n"
                            "E que tal se dividirmos o processo em pedaços?");
                // TODO -> <simulação de quebra do processo>
            }; break;
            case 24: {
                print_chicko("Quanto aos endereços (virtuais) internos do programa, não se preocupe!\n"
                            "Nós podemos usar uma tabela pra registrar a quebra, e deixamos a\n"
                            "conversão ser feita por mim em uma "
                            CLIS_CK_EMPHASIS("TLB (Translation look-aside buffer)")
                            "\ne meu poderoso " CLIS_CK_UNDER("MMU") "! h3h3");
                // TODO -> <apresentação da tabela de endereços>
            } break;
            case MAX_STEP: {
                print_chicko("Pronto! Agora sim!");

                print_chicko("Então, é isso... Até mais!");
            }; break;
            default: break;
        }
        printf("\33[2J\033[0;0f"); // Clear screen and go to start position
    }
    enable_canonical(); // Returns terminal to default state
}
