#include <stdio.h>
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
    bool expected;
    
    bool write_yes;
    Direction dir_yes;    
    size_t next_yes;    
    
    bool write_no;
    Direction dir_no;        
    size_t next_no;
} Instruction;
    
void machine_randomize(Machine *machine) {
    srand(time(NULL));
    for(size_t i = 0; i < machine->size; i++) {
        machine->data[i] = rand() % 2;
    }
}
    
size_t machine_execute(Machine *machine, Instruction *inst) {
    if(machine->data[machine->head] == inst->expected) {
        machine->data[machine->head] = inst->write_yes;
        machine->head += inst->dir_yes;
        return inst->next_yes;
    }
    machine->data[machine->head] = inst->write_no;    
    machine->head += inst->dir_no;    
    return inst->next_no;       
}
    
void machine_print(Machine *machine) {
    printf("head: %zu, ", machine->head);
    for(size_t i = 0; i < machine->size; i++) {
        printf("%d -> ", machine->data[i]);
    }
    printf("\n");
}

int main() {
    Machine machine = {0};
    machine.size = 8;
    machine.data = malloc(sizeof(bool)*machine.size);
    machine_randomize(&machine);
    
    Instruction insts[] = {
        // expected value, value if true, direction if true, next if true,
        // value if false, direction if false, next if false
        {0, 1, RIGHT, 2, 0, RIGHT, 0},
        {NULL},
    };
    size_t inst_count = sizeof(insts)/sizeof(*insts);
    size_t cur = 0;
    machine_print(&machine);    
    while(cur <= inst_count-1) {
        cur = machine_execute(&machine, &insts[cur]);    
        machine_print(&machine);        
    }
    return 0;
}
