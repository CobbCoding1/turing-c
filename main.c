#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>

#define NUM_STATES 256
#define DATA_START_CAPACITY 128
#define TAPE_START_CAPACITY 32

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
	memset(tape->data, B, sizeof(char)*tape->capacity);
}
	
void tape_set(Tape *tape, char *value) {
	for(size_t i = 0; i < tape->count; i++) {
		if(isdigit(value[i])) tape->data[i] = value[i] - '0';
		else tape->data[i] = value[i];
	}
}
	
void machine_free(Machine *machine) {
	free(machine->tape.data);	
}

Tape tape_init(size_t capacity, size_t count) {
	Tape tape = {0};
	tape.count = count;
	tape.capacity = capacity;
	tape.data = malloc(sizeof(unsigned char)*tape.capacity);
	tape_default(&tape);
	return tape;
}
    
size_t machine_execute(Machine *machine, Instruction *inst, size_t inst_count) {
	// expand tape if necessary
    if(machine->head >= machine->tape.capacity) {
        Tape tape = tape_init(machine->tape.capacity*2, machine->tape.count);
        memcpy(tape.data, machine->tape.data, sizeof(*machine->tape.data)*machine->tape.capacity);
        free(machine->tape.data);
        machine->tape = tape;
    }
        
    for(size_t i = 0; i < NUM_STATES; i++) {
        if(machine->tape.data[machine->head] == inst->value[i].expected) {    
			// if current instruction is HALT, then return inst_count which triggers an exit
			if(inst->value[i].dir == HALT) return inst_count;
				
            machine->tape.data[machine->head] = inst->value[i].write;
				
			// check left boundary to ensure there is no indexing issues
            if(machine->head == 0 && inst->value[i].dir < 0) {
                fprintf(stderr, "out of bounds!\n");
                exit(1);
            }
			// move the head the proper direction (-1, 0, or 1)
            machine->head += inst->value[i].dir;
            if(machine->head > machine->tape.count) machine->tape.count = machine->head;        
            
            return inst->value[i].next;
        }
    }
	fprintf(stderr, "ERROR: Not all cases handled in states\n");
	exit(1);
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
		printf("symbol: %zu expected: %c write: %c dir: %d next: %zu\n", 
				value.symbol, value.expected, value.write, value.dir, value.next);
	}
}

void print_usage(char *program, char *error) {
	fprintf(stderr, "ERROR: %s\n", error);
	fprintf(stderr, "USAGE: %s <input_file.turing> <optional starting tape>\n", program);
	exit(1);
}
	
char *read_from_file(char *filename, char *program, size_t *data_s) {
	FILE *file = fopen(filename, "r");
	if(file == NULL) print_usage(program, "Cannot open file");
		
	fseek(file, 0, SEEK_END);
	size_t len = ftell(file);
	fseek(file, 0, SEEK_SET);	
	
	char *data = calloc(len+1, sizeof(char));
	fread(data, sizeof(char), len, file);
	
	fclose(file);
	*data_s = len;
	
	return data;
}
	
size_t get_number(char *data, size_t data_s, size_t *index) {
	char num[256] = {0};
	size_t num_s = 0;
	while(*index < data_s && isdigit(data[*index])) {
		num[num_s++] = data[*index];
		(*index)++;
	}
	return atol(num);
}
	
Direction get_dir(char c, size_t index) {
	switch(c) {
		case 'R': return RIGHT;
		case 'L': return LEFT;
		case 'H': return HALT;
		default:
			fprintf(stderr, "error: expected R, L, or H for the direction but found %c at %zu\n", c, index);
			exit(1);
	}
}
	
Instructions get_insts(char *data, size_t data_s) {
	Instructions insts = {0};
	for(size_t i = 0; i < data_s; i++) {
		Instruction inst = {0};
		size_t cur = 0;
		while(true) {
			State state = {0};
			state.symbol = get_number(data, data_s, &i);
			i++;
			state.expected = data[i++];
			i++;
			state.write = data[i++];
			i++;
			state.dir = get_dir(data[i], i);
			i += 2;
			state.next = get_number(data, data_s, &i);
			i++;			
			inst.value[cur++] = state;
			size_t start = i;
			if(get_number(data, data_s, &i) != state.symbol) {
				i = start-1;	
				break;
			}
		}
		DA_APPEND(&insts, inst);		
	}
	return insts;
}
	

int main(int argc, char **argv) {
	char *prog_name = argv[0];
	if(argc < 2) {
		print_usage(prog_name, "Not enough arguments");
	}
		
	size_t data_s = 0;
	char *data = read_from_file(argv[1], prog_name, &data_s);
	
	Machine machine = {0};
	Program program = {0};
	Instructions insts = get_insts(data, data_s);
	
	if(argc <= 2) {	
		// set the defaults if no tape is passed
	    machine.tape.capacity = TAPE_START_CAPACITY;
	    machine.tape.count = machine.tape.capacity;        
	    machine.tape.data = malloc(sizeof(unsigned char)*machine.tape.capacity);                
		tape_default(&machine.tape);
	} else {
		// set value to user arg if passed
		size_t tape_len = strlen(argv[2]);
		machine.tape.capacity = tape_len*2;
		machine.tape.count = tape_len;
	    machine.tape.data = malloc(sizeof(unsigned char)*machine.tape.capacity);                		
		tape_default(&machine.tape);		
		tape_set(&machine.tape, argv[2]);
	}
    
    program.machine = &machine;            
    program.insts = insts;    
	
	//insts_print(insts);
    
    while(program.cur <= program.insts.count-1) {
        program.cur = machine_execute(program.machine, &program.insts.data[program.cur], program.insts.count);    
    }    
		
	// free the memory
    machine_print(program.machine);			
	machine_free(program.machine);
	free(insts.data);
	free(data);
	
    return 0;
}
