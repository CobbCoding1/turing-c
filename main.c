#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
    bool *data;
    size_t size;
    size_t head;    
} Machine;
    
typedef enum {
    LEFT = -1,
    STAY = 0,
    RIGHT = 1,
} Direction;
    
typedef struct {
    bool write;
    Direction dir;
    size_t next;
} State;

typedef struct {
    bool expected;
    State yes;
    State no;
} Instruction;
    
typedef struct {
    Machine *machine;
    Instruction *insts;
    size_t inst_count;
    size_t cur;
} Program;
    
void machine_randomize(Machine *machine) {
    srand(time(NULL));
    for(size_t i = 0; i < machine->size; i++) {
        machine->data[i] = rand() % 2;
    }
}
    
size_t machine_execute(Machine *machine, Instruction *inst, size_t inst_count) {
    if(machine->head >= machine->size) return inst_count; 
    
    if(machine->data[machine->head] == inst->expected) {
        machine->data[machine->head] = inst->yes.write;
        if(machine->head == 0 && inst->yes.dir < 0) return inst_count;
        machine->head += inst->yes.dir;
        
        return inst->yes.next;
    }
    machine->data[machine->head] = inst->no.write;    
    if(machine->head == 0 && inst->no.dir < 0) return inst_count;    
    machine->head += inst->no.dir;    
    
    return inst->no.next;       
}
    
void machine_print(Machine *machine) {
    printf("head: %zu, ", machine->head);
    for(size_t i = 0; i < machine->size; i++) {
        if(i != machine->size-1)
            printf("%d -> ", machine->data[i]);
        else
            printf("%d\n", machine->data[i]);        
    }
}

int main(int argc, char **argv) {
    Program program = {0};
    Machine machine = {0};
    if(argc == 1) {
        machine.size = 3;
        machine.data = malloc(sizeof(bool)*machine.size);                
        machine_randomize(&machine);
    } else {
        machine.size = strlen(argv[1]);
        machine.data = malloc(sizeof(bool)*machine.size);        
        for(size_t i = 0; i < machine.size; i++) {
            machine.data[i] = argv[1][i] - '0';
        }
    }
    
    program.machine = &machine;            
    Instruction insts[] = {
        // expected value, value if true, direction if true, next if true,
        // value if false, direction if false, next if false
        {0, {1, RIGHT, 2}, {0, RIGHT, 0}},
        {NULL},
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
