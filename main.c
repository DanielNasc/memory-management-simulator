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
    int swap[_mem_size];
    _mem = memory; // defines system (virtual) memory
    _swap = swap; // defines system (virtual) swap
    _raw_end = _mem + (_mem_size - (size_t)(_mem_size * _mem_ratio));

    for (size_t i = 0; i < _mem_size; ++i)
        swap[i] = memory[i] = EOF; // Fill it with "trash memory" (blank for visualization purposes)

    boot();

    return EXIT_SUCCESS;
}


/* Print a dialog by chicko. Escape sequences %
 * %a: pass a function call (must receive no args and return void),
 * %n: non-blocking mode: line will not wait for user input
 * %s: set skip mode: must receive an int flag skip mode
 * dialog will stop until completed. */
void print_chicko(char const *s, ...)
{
    va_list ap; /* Variable argument list */
    va_start(ap, s); /* Point to first arg. */

    printf("🐣 " CLIS_CHICKO);
    bool block = true;
    int arg = 0;
    size_t lines = 1;
    int do_skip = skip & (SKIP_ALL | SKIP_TO_EVENT);

    while (*s != '\0') {
        switch (*s) {
            case '\n': lines++; break;
            case '%':
                switch (*++s) {
                    case 'a': ;
                        void (* action)(void) = va_arg(ap, void (*)(void));
                        fflush(stdout); // In case of skip mode
                        action();
                        ++s;
                        break;
                    case 's': {
                        do_skip |= va_arg(ap, int);
                        ++s;
                    } break;
                    case 'n': { // Non blocking mode 
                        block = false;
                        ++s;
                    } break;
                    default: break;
                }
                default: break;
        }
        if (!do_skip) {
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

#define MAX_PROCS 2
    int threads_n = 1; // #of processes
    ProcessRegister procs[MAX_PROCS];

#define MAX_NAME_LEN 120
    char scope_name[MAX_NAME_LEN] = "main";
    struct goto_scope_args main_scope = {
        .emu=&procs[0], .pc=0, .scope_name=scope_name
    };

    // Hello World process
    Command proc_hello_world[] = {
        { .line="int main() {", .call=NULL, .args=NULL },
        { .line="\tprint(\"Hello\")", .call=NULL, .args=NULL },
        { .line="\tputchar(' ')", .call=NULL, .args=NULL },
        { .line="\tprint(\"World\")", .call=NULL, .args=NULL },
        { .line="}", .call=NULL, .args=NULL },
    };

    // Tutorial Process
    struct set_var_args sva1 = { .emu=&procs[0], .mem=_raw_end, .value=0 };
    Command proc_tuto[] = {
        { .line="int main() {", .call=goto_scope, .args=&main_scope },
        { .line="\t""int i;", .call=inc_stack, .args=&procs[0] },
        { .line="\t""i = 0;", .call=set_var, .args=&sva1} ,
        { .line="}", .call=NULL, .args=NULL },
    };

    // Recursive process
    char rec_header_fmt[] = "int rec(int i = %d)";
    char rec_name[] = "rec";
    //
    int var_a = 0;
    // Segment stream = { .start=&var_a, .end=(&var_a)+1 };
    struct inc_var_args inc_args = { .emu=&procs[0], .mem=&(procs[0].args.values.start) };
    struct goto_scope_args rec_scope = {
        .emu=&procs[0], .pc=0, .scope_name=rec_name
    };
    struct recursive_call_args rec_es_args = {
        .emu=&procs[0], .header.fmt=rec_header_fmt, .rec_lvl=0,
        .scope={ .name=rec_name, .p=scope_name },
    };
    //
    int comp_value = _mem + _mem_size - _raw_end + 6;
    int *comp_ref = &comp_value;
    // TODO -> Correct this
    struct comp_var_args comp_args = {
        .emu=&procs[0], .jump=5, .a=&(procs[0].args.values.start),
        .b=&comp_ref, .comp='<'
    };
    // struct goto_scope_args gsa = {.emu = args->emu, .scope_name = args->scope.p, .pc = 0};
    // goto_scope(&gsa);
    Command proc_rec[] = {
        { .line="int rec(int i = 0) {", .call=recursive_call, .args=&rec_es_args },
        { .line="", .call=comp_var, .args=&comp_args },
        { .line="\t\t""i++;", .call=inc_var, .args=&inc_args },
        { .line="\t\t""rec(i);", .call=NULL, .args=NULL },
        { .line="\t}", .call=call_scope, .args=&rec_scope },
        { .line="}", .call=NULL, .args=NULL },
    };
    sprintf(proc_rec[1].line, "\tif (i < %d) {", comp_value);
    rec_es_args.header.p = proc_rec[0].line;

    // Setup processes list
    procs[0] = (ProcessRegister) {
        .commands = proc_hello_world,
        .last_mod = { NULL, NULL },
        .scope_name = scope_name,
        .size = sizeof(proc_hello_world) / sizeof(Command),
        .stack.lim = _mem + _mem_size,
        .swap.lim = _mem + _mem_size,
    }; // Setup default process
    // procs[0].stack.head = procs[0].stack.tail = _mem + _mem_size;
    procs[0].pc = procs[0].size + 1;

    // Setup stack on proc[0]
    procs[0].stack.head = procs[0].stack.tail = _raw_end;
    procs[0].stack.lim = _mem + _mem_size;

    // Setup swap on proc[0]
    procs[0].swap.head = procs[0].swap.tail = _swap;
    procs[0].swap.lim = _swap + _mem_size;

    // Setup arguments for recursion
    procs[0].args.n = 1;
    procs[0].args.values.start = &var_a;
    procs[0].args.values.end = procs[0].args.values.start + procs[0].args.n;

    /* processing */
    int skip_case = -1; // Skip a case "event" sub-step
    int simult_proc = 0; // which process is running in simultaneous mode
    int stick_count = 0;
    bool threaded_mode = false;

    /* swap */
    bool use_swap = false;

#define MAX_STEP 25
    for (size_t step = 0; step != MAX_STEP; step++) {

        if (threaded_mode) {
            int c = 0;
            for (int c_proc = 0; c_proc < threads_n; c_proc++)
                c |= step_proc(procs + c_proc); // Step through threads
            step -= c; // Don't step if any is running
        }
        else {
            int is_running = step_proc(procs + simult_proc);
            if (simult_proc < (threads_n - 1))
                simult_proc += -is_running; // Simultaneous steps
            step -= is_running;         // Don't step while running
        }

        if (skip_to_event && (step == skip_to_event))
            skip &= ~SKIP_TO_EVENT; // remove skipping

        printf("\033[32m"
               "Mini (virtual) Operational System emulation started!\033[m\n");

        print_memory(_mem, procs, threads_n);

        if (use_swap)
            print_memory(_swap, procs, threads_n);

        for (int i = 0; i < threads_n; i++)
            print_code(procs + i);

        putchar('\n');
        print_mem_hex(_mem, procs, threads_n);

        if (use_swap)
            print_mem_hex(_swap, procs, threads_n);
        putchar('\n');

        switch (step) {
            case 0: {
                print_chicko("Hello World!" CLIS_RESET "\n" CLIS_CHICKO
                            "Eu sou o galinho m(v)x, vou ser o seu guia nessa simulação!"
                            CLIS_RESET "\n\t\t" CLIS_CHICKO "Pressione "
                            CLIS_CK_EMPHASIS("[BARRA DE ESPAÇO]") " para continuar!");
                print_chicko("Nesta simulação do nosso Mini (virtual) Sistema Operacional "
                            "descobriremos como ocorre o gerenciamento de memória em um OS.");
                skip |= SKIP_REFRESH;
            } break;
            case 1: {
                print_chicko("Na barra superior está o espaço utilizado na sua memória no momento.");
                // TODO #1 -> Destacar a barra superior
            } break;
            case 2: {
                print_chicko("No centro você verá o código do processo que iremos executar no nosso sistema.");
                // TODO #2 -> Destacar barra do centro
            } break;
            case 3: {
                print_chicko("Logo abaixo você pode ver o que acontece com a memória conforme o processo é executado.");
                // TODO #3 -> Destacar o inferior
            } break;
            case 4: {
                print_chicko("Pelo propósito dessa simulação, iremos focar apenas na memória do "
                            CLIS_CK_EMPHASIS("espaço do usuário.") CLIS_RESET "\n" CLIS_CHICKO
                            "Então eu escondi pra você a parte referente ao interior "
                            "do sistema " CLIS_CK_PURPLE("(em roxo)")".\n"
                            "😳 " CLIS_CK_STRIKE("Favor, respeitar ok? 👉👈"));
                setup_proc(&procs[0], proc_tuto, sizeof(proc_tuto) / sizeof(Command),
                           _raw_end, _mem + _mem_size );
                           // single process occupies all the stack space
                step++;
            } break;
            case 5: {
                if (skip_case != 5)
                    print_chicko("Um " CLIS_CK_EMPHASIS("processo") " selvagem apareceu!");
                print_chicko("%sPara controlar a execução pressione o botão " CLIS_CK_EMPHASIS("[ESPAÇO]")
                            " e iremos mostrar o que acontece. " CLIS_CK_BOLD("Tente!"), (skip_case == 5 ? SKIP_ALL : 0));
                skip_case = 5;
            } break;
            case 6: {
                print_chicko("Muito bem, você deve ter entendido. " CLIS_CK_ITALICS("Certo?"));
                setup_proc(&procs[0], proc_rec, sizeof(proc_rec) / sizeof(Command), _raw_end, _mem + _mem_size);
                step++;
            } break;
            case 7: {
                print_chicko("%sAgora iremos mostrar o que acontecesse quando temos uma "
                            CLIS_CK_EMPHASIS("recursão") " no programa.", (skip_case == 7 ? SKIP_ALL : 0));
                skip_case = 7;

                stick_count++;
                if (stick_count == 7) {
                    stick_count = 0;
                    step++;
                }
            } break;
            case 8: {
                print_chicko("%sOk! Agora deixa comigo!!!%n", (skip_case == 8 ? SKIP_ALL : 0));
                skip_case = 8;
                stick_count++;

                if (!(skip & SKIP_TO_EVENT))
                    msleep(max(min(1000 / (stick_count), 1000), 10)); // Used min here cause the number can start to grow if greater than 1000, just to be safe

                if (procs[0].last_mod.stack.end >= (_mem + _mem_size - 10))
                    step++;
            } break;
            case 9: {
                // TODO #7 -> Implementar o "travamento" do chicko no método print em %Z (frozen) (continua até pressionar espaço)
                print_chicko(CLIS_CK_EMPHASIS("%sOh no! Que problemão, a nossa memória vai "
                             "acabarrrrrrrrrrrrrrrrrrrrrrrrrrrrrr...%n"), SKIP_ALL);

                if (!(skip & SKIP_TO_EVENT))
                    msleep(max(min(1000 / (stick_count), 1000), 10));

                stick_count++;
                if (procs[0].last_mod.stack.end == (_mem + _mem_size))
                    step++;
            } break;
            case 10: {
                print_chicko(CLIS_CK_EMPHASIS("%sOh no! Que problemão, a nossa memória vai "
                                              "acabarrrrrrrrrrrrrrrrrrrrrrrrrrrrrr... "), SKIP_TO_EVENT);
                clear_partition(_mem, _mem + _mem_size);
                procs[0].pc = sizeof(proc_rec) / sizeof(Command);
            }; break;
            case 11: {
                print_chicko(CLIS_CK_BOLD("Droga, eu travei!") " Isso que acabou de acontecer é "
                            "chamado de " CLIS_CK_EMPHASIS("\"Stack Overflow\"") "... "
                            CLIS_CK_ITALICS("Não o site bobão!"));
                print_chicko("Bem, podemos resolver o problema anterior usando uma estratégia "
                            "chamada " CLIS_CK_UNDER("\"swapping\"") ", eu vou mostrar pra você.");
                use_swap = true;
                skip &= ~SKIP_REFRESH;
            }; break;
            case 12: {
                if (skip_case != 12) {
                    print_chicko(CLIS_CK_BOLD("Ta da!") " Esta nova partição aqui é chamada de... "
                                CLIS_CK_ITALICS("isso ") CLIS_CK_EMPHASIS("SWAP")
                                "," CLIS_RESET "\n" CLIS_CHICKO
                                "é basicamente um arquivo no disco do PC. Vamos ver agora!");
                    // reset and do recursion again
                    procs[0].args.values.start = &var_a;
                    procs[0].args.values.end = procs[0].args.values.start + procs[0].args.n;
                    procs[0].pc = 0;
                    procs[0].stack.tail = procs[0].stack.head;
                    setup_proc(&procs[0], proc_rec, sizeof(proc_rec) / sizeof(Command), _raw_end, _mem + _mem_size);
                    skip |= SKIP_REFRESH;
                    _flags &= ~COMP;
                    skip_case = 12;
                    stick_count = 0;
                }
                else {
                    print_chicko("%s" CLIS_CK_BOLD("Ta da!") " Esta nova partição aqui é chamada de... "
                                CLIS_CK_ITALICS("isso ") CLIS_CK_EMPHASIS("SWAP")
                                "," CLIS_RESET "\n" CLIS_CHICKO
                                "é basicamente um arquivo no disco do PC. Vamos ver agora!%n", SKIP_ALL);
                    stick_count++;

                    if (!(skip & SKIP_TO_EVENT))
                        msleep(max(min(1000 / (stick_count), 1000), 10));

                    if (procs[0].last_mod.swap.end == (_swap + _mem_size))
                        step++;
                }
            }; break;
            case 13: {
                print_chicko(CLIS_CK_BOLD("Ótimo!"));
                print_chicko("Mas eu posso fazer mais do que isso.\n" CLIS_RESET CLIS_CHICKO
                             "Agora vamos ver o que acontecesse quando há vários "
                             "programas sendo executados.");
                // TODO #6 -> Implementar multiprogramação
            }; break;
            case 14: {
                print_chicko("Queremos tirar proveito dos recursos da máquina, então faremos "
                             "esses dois programas serem carregados simultaneamente.");

                print_chicko("Poderemos então executar um após o outro!");
                // TODO #8 -> <mostrar execução single thread>
            }; break;
            case 15: {
                print_chicko("Ou alternar entre os dois, em sequência. Isso é útil "
                             "principalmente quando há múltiplas threads.");
                // TODO #9 -> <mostrar execução multithread>
            } break;
            case 16: {
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
            case 17: {
                print_chicko("Olhe, temos um problema! O segundo programa não pode continuar\n"
                            "por quê não há espaço o suficiente na máquina.");
                print_chicko("Podemos resolver isso, balanceando o uso da memória.");
                // TODO -> <mostrar alocação de tamanhos diferentes>
            }; break;
            case 18: {
                // print_chicko("Ok! Agora vou te mostrar algo legal... Algo PERIGOSO!!! ☠️ "
                //             CLIS_CK_UNDER("RADICAL! 😎 ") CLIS_CK_EMPHASIS("Ponteiros"));
                // TODO -> <carregar demonstração da memória heap> (IGNORAR)
                // print_chicko("Esse espaço no final é chamado de heap (ou monte), é aqui que\n"
                //             "fica a memória adicional. Muitos programas utilizam a heap\n"
                //             "quando querem crescer variavelmente, é um pouco de trabalho\n"
                //             "para mim, mas é um trabalho honesto 🤠.");
                // TODO -> <simular erro de segmentation fault (IGNORAR)
                // print_chicko("Espere, você não deveria acessar esta memória, o outro programa\n"
                //             "vai ter problema. " CLIS_CK_BOLD("Erro!!! " CLIS_CK_EMPHASIS("Erro!!")));
                // print_chicko("Como eu esperava, esse é mais um caso de " CLIS_CK_BOLD("Segmentation fault.")
                //             "\nUm programador responsável nunca permitiria ponteiros soltos assim.\n"
                //             "Eu não permitirei acessar além da fronteira. "
                //             CLIS_CK_EMPHASIS("Segmentation fault!") " Ouviu bem? 😠");
                // TODO -> <carregar código de acesso null> (IGNORAR)
                // print_chicko("Espere, esse endereço não existe! O endereço 00 é especial, "
                //             "nós chamamos ele de" CLIS_CK_BOLD("NULL") ".\nÉ uma exceção especial "
                //             "pra facilitar o trabalho de alguns programadores.\n"
                //             "Nós chamamos isso de " CLIS_CK_EMPHASIS("Null pointer Exception"));

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

                // print_chicko("Bem, eu não estou satisfeito. Ainda há espaço que não estamos usando...\n"
                //             "E que tal se dividirmos o processo em pedaços?");
                // TODO -> <simulação de quebra do processo> (IGNORAR)
            }; break;
            case 24: {
                // print_chicko("Quanto aos endereços (virtuais) internos do programa, não se preocupe!\n"
                //             "Nós podemos usar uma tabela pra registrar a quebra, e deixamos a\n"
                //             "conversão ser feita por mim em uma "
                //             CLIS_CK_EMPHASIS("TLB (Translation look-aside buffer)")
                //             "\ne meu poderoso " CLIS_CK_UNDER("MMU") "! h3h3");
                // TODO -> <apresentação da tabela de endereços> (IGNORAR)
            } break;
            case MAX_STEP: {
                // print_chicko("Pronto! Agora sim!");

                print_chicko("Então, é isso... Até mais!");
            }; break;
            default: break;
        }
        printf("\33[2J\033[0;0f"); // Clear screen and go to start position
    }
    enable_canonical(); // Returns terminal to default state
}
