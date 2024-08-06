#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

#define NUM_STATES 256
#define DATA_START_CAPACITY 128

#define DA_APPEND(da, item) do {                                                       		\
	    if ((da)->count >= (da)->capacity) {                                               	\
		        (da)->capacity = (da)->capacity == 0 ? DATA_START_CAPACITY : (da)->capacity*2; \
		        void *new = calloc(((da)->capacity+1), sizeof(*(da)->data));                   \
		        assert(new && "outta ram");                                                       \
		        memcpy(new, (da)->data, (da)->count);                                          \
		        free((da)->data);                                                              \
		        (da)->data = new;                                                              \
		    }                                                                                  \
	    (da)->data[(da)->count++] = (item);                                                	\
	} while (0)

#define B '_'

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
	HALT,
} Direction;
    
typedef struct {
	size_t symbol;
    unsigned char expected;
    unsigned char write;
    Direction dir;
    size_t next;
} State;

typedef struct {
    State value[NUM_STATES];
} Instruction;
	
typedef struct {
	Instruction *data;
	size_t count;
	size_t capacity;
} Instructions;
    
typedef struct {
    Machine *machine;
    Instructions insts;
    size_t cur;
} Program;
    
void tape_randomize(Tape *tape) {
    srand(time(NULL));
    for(size_t i = 0; i < tape->capacity; i++) {
        tape->data[i] = rand() % NUM_STATES;
    }
}
	
void tape_default(Tape *tape) {
    srand(time(NULL));
    for(size_t i = 0; i < tape->capacity; i++) {
        tape->data[i] = B;
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
		if(i > 3) break;
        if(machine->tape.data[machine->head] == inst->value[i].expected) {    
			if(inst->value[i].dir == HALT) return inst_count;
            machine->tape.data[machine->head] = inst->value[i].write;
            if(machine->head == 0 && inst->value[i].dir < 0) {
                fprintf(stderr, "out of bounds!\n");
                exit(1);
            }
            machine->head += inst->value[i].dir;
            if(machine->head > machine->tape.count) machine->tape.count = machine->head;        
            
            return inst->value[i].next;
        }
    }
	assert(false && "WHAT THE HECK");
    return inst_count;
}
    
void machine_print(Machine *machine) {
    printf("head: %zu, ", machine->head);
    for(size_t i = 0; i < machine->tape.count; i++) {
		if(machine->tape.data[i] != B) {
            printf("%c", machine->tape.data[i]);
		}
    }
	printf("\n");
}
	
void insts_print(Instructions insts) {
	for(size_t i = 0; i < insts.count; i++) {
		State value = insts.data[i].value[0];
		printf("%zu %c %c %d %zu\n", value.symbol, value.expected, value.write, value.dir, value.next);
	}
}
	
char *read_from_file(char *filename) {
	FILE *file = fopen(filename, "r");
		
	fseek(file, 0, SEEK_END);
	size_t len = ftell(file);
	fseek(file, 0, SEEK_SET);	
	
	char *data = calloc(len+1, sizeof(char));
	fread(data, sizeof(char), len, file);
	
	return data;
}

int main(int argc, char **argv) {
	char *data = read_from_file(argv[1]);
	Instructions insts = {0};
	for(size_t i = 0; data[i] != '\0'; i++) {
		Instruction inst = {0};
		size_t cur = 0;
		while(true) {
			State state = {0};
			state.symbol = data[i++];
			i++;
			state.expected = data[i++];
			i++;
			state.write = data[i++];
			i++;
			switch(data[i++]) {
				case 'R':
					state.dir = RIGHT;
					break;
				case 'L':
					state.dir = LEFT;
					break;
				case 'H':
					state.dir = HALT;
					break;
				default:
					fprintf(stderr, "error: expected R, L, or H for the direction but found %d\n", data[i]);
					exit(1);
			}
			i++;
			state.next = data[i++] - '0';
			inst.value[cur++] = state;
			i++;
			if((size_t)data[i] != state.symbol) {
				i--;	
				break;
			}
		}
		DA_APPEND(&insts, inst);		
	}
    Program program = {0};
    Machine machine = {0};
    if(argc == 1) {
        machine.tape.capacity = 4;
        machine.tape.count = machine.tape.capacity;        
        machine.tape.data = malloc(sizeof(unsigned char)*machine.tape.capacity);                
		tape_default(&machine.tape);
    } else {
        machine.tape.capacity = strlen(argv[1]);
        machine.tape.count = machine.tape.capacity;
        machine.tape.data = malloc(sizeof(unsigned char)*machine.tape.capacity);        
        for(size_t i = 0; i < machine.tape.capacity; i++) {
            machine.tape.data[i] = argv[1][i];
        }
		tape_default(&machine.tape);		
    }
    
    program.machine = &machine;            
    program.insts = insts;    
	
	//insts_print(insts);
    
    while(program.cur <= program.insts.count-1) {
	    machine_print(program.machine);        		
        program.cur = machine_execute(program.machine, &program.insts.data[program.cur], program.insts.count);    
    }    
    return 0;
}
