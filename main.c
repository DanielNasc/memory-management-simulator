#include "utils.h"
#include "sleep.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>


#define LINE_WIDTH (_1KB / 8)
#define DEF_MEM_SIZE LINE_WIDTH // default memory size

static int *_mem; // memory starting pointer
static size_t _mem_size = DEF_MEM_SIZE; // memory size (bytes)
const float _mem_ratio = .7; // the ratio of memory allocated against RAW data

static size_t _raw_end; // End of RAW data pool

static struct {
    size_t pc; // Program Counter
    size_t stack_tail; // Stack Pointer
    size_t heap_end; // Heap Limit

    // for the purposes of this emulation, to ease visualization
    struct {
        size_t start;
        size_t end;
    } last_mod; // last modified data space
    char *scope_name;
} _registers;

static bool skip = false; /* skip simulation animations */
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
                        print_err("Invalid memory size, argument ignored, fallback to default (1KB)");
                    else
                        _mem_size = size;
                }; break;
                case 's':
                    skip = true;
                    break;
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


static bool waiting;

// Wait in a separated thread (ms)
void *wait(void *vargp) 
{
    waiting = true;
    msleep((*(size_t *)vargp));
    waiting = false;
    return NULL;
}


/* Boot loading message */
void *dots(void *_vargp)
{
    const long SPEED = 100; // ms
    printf("\033[33m""Starting mini (virtual) Operational System emulation...");
    fflush(stdout);

    while (waiting) {
        msleep(SPEED);
        printf("\b\b\b   \b\b\b"); // clear dots
        fflush(stdout);

        for (int i = 3; i > 0; --i) {
            msleep(SPEED);
            putchar('.');
            fflush(stdout);
        }
    }
    printf("\033[m");
}


/* print memory usage simulation */
void print_memory()
{
    const long SPEED = 5;
    putchar('[');
    printf("\033[35m"); // RAW data section color

    for (size_t i = 0; i != _mem_size; ++i) {
        if (!skip) {
            fflush(stdout);
            msleep(SPEED);
        }

        // Place the mem ratio separation
        if (i == _raw_end)
            printf("\033[m");

        // draw last modified space as green
        if (_registers.last_mod.start == i)
            printf("\033[32m");
        if (_registers.last_mod.end == i)
            printf("\033[m");

        // print mem
        if (_mem[i] == EOF)
            printf("-");
        else
            printf("*");

        if ((i + 1) % LINE_WIDTH == 0)
            printf("\n ");
    }

    if (_mem_size % LINE_WIDTH == 0)
        putchar('\b');
    printf("]\n");
}



/* print memory simulation's data (hex codes) */
void print_mem_hex()
{
    const long SPEED = 3;
    const size_t WIDTH = LINE_WIDTH / 4;
    putchar('[');
    printf("\033[35m"); // RAW data section color

    for (size_t i = 0; i != _mem_size; ++i) {
        if (!skip) {
            fflush(stdout);
            msleep(SPEED);
        }

        // Place the mem ratio separation
        if (i == _raw_end)
            printf("\033[m");

        // draw last modified space as green
        if (_registers.last_mod.start == i)
            printf("\033[32m");
        if (_registers.last_mod.end == i)
            printf("\033[m");

        // print mem
        if (_mem[i] == EOF)
            printf(" -- ");
        else {
            char hex[3];
            ultobyte(_mem[i], hex);
            printf(" %s ", hex);
        }

        if ((i + 1) % WIDTH == 0)
            printf("\n ");
    }

    if (_mem_size % WIDTH == 0)
        putchar('\b');
    printf("]\n");
}


struct enter_scope_args {
    size_t pointer;
    char *scope_name;
};
/* Emulate entering a scope of a (virtual) function */
void enter_scope(void *vargp)
{
    struct enter_scope_args args = *(struct enter_scope_args *)vargp;

    _registers.scope_name = args.scope_name;
    _registers.pc = args.pointer;
}


void none(void *vargp) {}


/* Increments Program Counter */
void inc_pc(void *vargp)
{
    _registers.pc++;
}


/* Increments Stack Pointer */
void inc_sp(void *vargp)
{
    _registers.stack_tail++;
}


struct set_var_args {
    size_t *pointer;
    int *data_stream;
};
/* Set a variable on stack memory */
void set_var(void *vargp)
{
    struct set_var_args args = *(struct set_var_args *)vargp;
    while (*(args.data_stream) != EOF)
        _mem[*args.pointer] = (*(args.data_stream++));
}


/* Emulates a process step-by-step */
void print_code()
{
    struct enter_scope_args esa = {.pointer=0, .scope_name="main"};
    int stream[] = { (char)0, EOF };
    struct set_var_args sva = {.pointer=&(_registers.stack_tail), .data_stream=stream};

    struct command {
        char line[LINE_WIDTH];
        void (* call)(void *);
        void *args;
    } commands[] = {
        {.line="int main()", .call=enter_scope, .args=&esa},
        {.line="{", .call=none, .args=NULL},
        {.line="\t""int i;", .call=inc_sp, .args=NULL},
        {.line="\t""i = 0;", .call=set_var, .args=&sva},
        {.line="}", .call=none, .args=NULL},
    };

    printf("PC: %lu, Stack Tail: %lu\n", _registers.pc, _registers.stack_tail);
    putchar('\n');
    for (int i = 0; i < sizeof(commands) / sizeof(struct command); i++) {
        if (!skip)
            msleep(100);
        printf("%s\n", commands[i].line);
        commands[i].call(commands[i].args);
    }
}

#include <uchar.h>
//#include <stdarg.h>
void print_chicko(char *s, ...)
{
    //va_list ap; // Não implementei pq não foi necessário.
    //va_start(ap, count);
    printf("🐣 \033[32;43m");
    int i = 0;
    if (skip)
        printf("%s", s);
    else while (*s != '\0') {
        fflush(stdout);
        msleep(15);
        putchar(*(s++));
        if (*s != ' ')
            i++;
        // va_arg(ap, <type>);
    }
    printf("\033[m\n");
    //va_end(ap);
}


static struct key_input_event {
    int scancode;
    bool key_pressed;
} *key_input_events = NULL;
static size_t key_input_n = 0;
static bool _input_event_guard;

/* Tracks keyboard input events. */
void *track_keyboard_input(void *_vargp)
{
    for (;;) {
        int code = getchar();
        if (!_input_event_guard && key_input_events != NULL) {
            _input_event_guard = true;
            struct key_input_event *kie = key_input_events;

            for (int i = key_input_n; i != 0; i--)
                if (kie->scancode != code)
                    kie++;
                else
                    kie->key_pressed = true;

            _input_event_guard = false;
        }
    }
    return NULL;
}


void wait_input_event(int scancode)
{
    struct key_input_event kie = { .scancode=scancode, .key_pressed=false};

    // Set input event
    while (_input_event_guard);
    _input_event_guard = true;
    key_input_n = 1;
    key_input_events = &kie;
    _input_event_guard = false;

    while (!kie.key_pressed); // Wait

    // Unset event
    while (_input_event_guard);
    _input_event_guard = true;
    key_input_n = 0;
    key_input_events = NULL;
    _input_event_guard = false;
}


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

        printf("\33[2K\r"); // erase last line
    }


    printf("\033[32m""Mini (virtual) Operational System emulation started!\033[m\n");

    print_memory();
    print_code();
    putchar('\n');
    print_mem_hex();

    putchar('\n');

    print_chicko("Olá, eu sou o galinho m(v)x, vou ser o seu guia nessa simulação!\n\33[30;30m"
                "\t\t\t\033[32;43m" "Pressione \033[41;97m[BARRA DE ESPAÇO]\033[32;43m para continuar!");
    wait_input_event(' ');

    printf("\33[1A");
    printf("\33[2K\r");
    printf("\33[1A");
    printf("\33[2K\r");
    print_chicko("Nesta simulação do nosso Mini (virtual) Sistema Operacional descobriremos como ocorre o gerenciamento de memória em um OS.");
    enable_canonical(); // Returns terminal to default state
}
