#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

#define NUM_STATES 3

typedef struct {
    unsigned char *data;
    size_t count;
    size_t capacity;
} Tape;
    
typedef struct {
    Tape tape;
    size_t head;    
} Machine;
    
typedef enum {
    LEFT = -1,
    STAY = 0,
    RIGHT = 1,
} Direction;
    
typedef struct {
    unsigned char expected;
    unsigned char write;
    Direction dir;
    size_t next;
} State;

typedef struct {
    State value[NUM_STATES];
} Instruction;
    
typedef struct {
    Machine *machine;
    Instruction *insts;
    size_t inst_count;
    size_t cur;
} Program;
    
void tape_randomize(Tape *tape) {
    srand(time(NULL));
    for(size_t i = 0; i < tape->capacity; i++) {
        tape->data[i] = rand() % NUM_STATES;
    }
}
    
size_t machine_execute(Machine *machine, Instruction *inst, size_t inst_count) {
    if(machine->head >= machine->tape.capacity) {
        Tape tape = {0};
        tape.capacity = machine->tape.capacity*2;
        tape.data = calloc(tape.capacity, sizeof(*machine->tape.data));
        memcpy(tape.data, machine->tape.data, sizeof(*machine->tape.data)*machine->tape.capacity);
        free(machine->tape.data);
        machine->tape = tape;
    }
        
    for(size_t i = 0; i < NUM_STATES; i++) {
        if(machine->tape.data[machine->head] == inst->value[i].expected) {    
            machine->tape.data[machine->head] = inst->value[i].write;
            if(machine->head == 0 && inst->value[i].dir < 0) return inst_count;
            machine->head += inst->value[i].dir;
            if(machine->head > machine->tape.count) machine->tape.count = machine->head;        
            
            return inst->value[i].next;
        }
    }
    fprintf(stderr, "value was not in expected range\n");
    exit(1);
}
    
void machine_print(Machine *machine) {
    printf("head: %zu, ", machine->head);
    for(size_t i = 0; i < machine->tape.count; i++) {
        if(i != machine->tape.count-1)
            printf("%d -> ", machine->tape.data[i]);
        else
            printf("%d\n", machine->tape.data[i]);        
    }
}

int main(int argc, char **argv) {
    Program program = {0};
    Machine machine = {0};
    if(argc == 1) {
        machine.tape.capacity = 3;
        machine.tape.count = machine.tape.capacity;        
        machine.tape.data = malloc(sizeof(unsigned char)*machine.tape.capacity);                
        tape_randomize(&machine.tape);
    } else {
        machine.tape.capacity = strlen(argv[1]);
        machine.tape.count = machine.tape.capacity;
        machine.tape.data = malloc(sizeof(unsigned char)*machine.tape.capacity);        
        for(size_t i = 0; i < machine.tape.capacity; i++) {
            machine.tape.data[i] = argv[1][i] - '0';
        }
    }
    
    program.machine = &machine;            
    Instruction insts[] = {
        {{
            {0, 1, RIGHT, 2},
            {1, 0, RIGHT, 0},
            {2, 1, RIGHT, 0},                    
        }},
        {{{0}}},
    };
    program.inst_count = sizeof(insts)/sizeof(*insts);
    program.insts = insts;    
    
    machine_print(program.machine);    
    while(program.cur <= program.inst_count-1) {
        program.cur = machine_execute(program.machine, &program.insts[program.cur], program.inst_count);    
        machine_print(program.machine);        
    }    
    return 0;
}
