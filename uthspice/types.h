//
//  types.h
//
//  Created by Athanasios Theodoridis (461) and Aris Eleftheriadis (460) on 4/10/12.
//
//

// various definitions
#define MAX_TOKENS_IN_LINE 8
#define MAX_TOKEN_LEN      10

// data structures and type definitions
typedef struct element {
    
	char *element_name;
	int element_type;
    char *first_terminal;
	char *second_terminal;
    
    char *drain_terminal;
    char *gate_terminal;
    char *source_terminal;
    char *bulk_terminal;
	
    char *model_name;
    
    double gate_length;
    double gate_width;
	
    char *area;
	
    double value;
    
	struct element *next;
    
} element;

// simple enum for holding the type of circuit elements
// we might parse
typedef enum {
    
    elementTypeVoltageSource = 0,
    elementTypeCurrentSource,
    elementTypeResistance,
    elementTypeCapacitor,
    elementTypeInductor,
    elementTypeDiode,
    elementTypeMOSTransistor,
    elementTypeBJTTransistor
    
} element_type;

// simple BOOL typedef
typedef enum { false, true } bool;