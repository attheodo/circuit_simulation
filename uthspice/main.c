//
//  uthspice.c
//
//  Created by Athanasios Theodoridis (461) and Aris Eleftheriadis (460) on 4/10/12.
//
//


#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "uthash.h"
#include "utlist.h"
#include "helpers.h"




// list head that holds the parsed elements, should always be initialized to NULL
element *head = NULL;


// various definitions
#define MAX_TOKENS_IN_LINE 8
#define MAX_TOKEN_LEN      10

// data structures and type definitions
struct element {
    
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
    
    struct element *next_element;
    
};

struct element *list_head ,*element_node;

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

typedef enum { false, true } bool;


// takes an element_type (int) and returns
// it's type as a human readable string
char *name_of_element_for_type(element_type i){
    switch(i){
        case elementTypeVoltageSource:
            return "VoltageSource";
        case elementTypeCurrentSource:
            return "CurrentSource";
        case elementTypeResistance:
            return "Resistance";
        case elementTypeCapacitor:
            return "Capacitor";
        case elementTypeInductor:
            return "Inductor";
        case elementTypeDiode:
            return "Diode";
        case elementTypeMOSTransistor:
            return "MOSTransistor";
        case elementTypeBJTTransistor:
            return "BJTTransistor";
        default:
            return "Something went terribly wrong\n";
    }
    
    return NULL;
}

element_type element_type_for_string(char element_as_string){
   
    if(element_as_string == 'V'){
       
        return elementTypeVoltageSource;
    
    } else if (element_as_string == 'I'){
        
        return elementTypeCurrentSource;
    
    } else if (element_as_string == 'R'){
        
        return elementTypeResistance;
    
    } else if (element_as_string == 'C'){
        
        return elementTypeCapacitor;
    
    } else if (element_as_string == 'L'){
        
        return elementTypeInductor;
        
    } else if (element_as_string == 'D') {
        
        return elementTypeDiode;
        
    } else if (element_as_string == 'M') {

        return elementTypeMOSTransistor;
    
    } else if (element_as_string == 'Q') {
        
        return elementTypeBJTTransistor;
        
    }

    return -1;
}

#pragma mark - List related

//initializes a list
void initElementsList(){
    
	list_head = (struct element*) malloc( sizeof(struct element) );
	list_head->next_element = NULL;
	
}

//prints the list of the nodes parsed
void print_elements_parsed() {
    
    struct element *current_element;
    
    printf("[-] Parsed the following circuit elements:\n\n");
    
    if(list_head->next_element == NULL){
        
        printf("[!] No elements parsed. Netlist file corrupt?\n");
        return;
    
    }
    
    for(current_element = list_head->next_element; current_element->next_element != NULL; current_element = current_element->next_element){
        
        // printing non-transistor element
        if(current_element->element_type != elementTypeBJTTransistor && current_element->element_type != elementTypeMOSTransistor){
            
            printf("\t%s: %s terminal_1: %s terminal_2: %s value: %.15f\n",name_of_element_for_type(current_element->element_type),current_element->element_name,current_element->first_terminal,current_element->second_terminal,current_element->value);
        }
        // print transistor element
        else {
            printf("\t%s: %s drain_terminal: %s gate_terminal: %s source_terminal: %s bulk_terminal: %s gate_length: %.15f gate_width: %.15f\n",name_of_element_for_type(current_element->element_type),current_element->element_name,current_element->drain_terminal,current_element->gate_terminal,current_element->source_terminal,current_element->bulk_terminal,current_element->gate_length,current_element->gate_width);

        }
        
    }
    
    // last element of list
    if(current_element->element_type != elementTypeBJTTransistor && current_element->element_type != elementTypeMOSTransistor){
        printf("\t%s: %s terminal_1: %s terminal_2: %s value: %.15f\n",name_of_element_for_type(current_element->element_type),current_element->element_name,current_element->first_terminal,current_element->second_terminal,current_element->value);

    } else {
         printf("\t%s: %s drain_terminal: %s gate_terminal: %s source_terminal: %s bulk_terminal: %s gate_length: %.15f gate_width: %.15f\n",name_of_element_for_type(current_element->element_type),current_element->element_name,current_element->drain_terminal,current_element->gate_terminal,current_element->source_terminal,current_element->bulk_terminal,current_element->gate_length,current_element->gate_width);
    }
   
    
}   
>>>>>>> 442851c9adee3975b9ee5c743d132611a81f2b5f

// prints a simple help message
void print_help(const char *name)
{
    printf("\n\tUsage: %s <netlist file>\n",name);
    exit(-1);
}


#pragma mark - Main func

// no comment
int main(int argc, const char * argv[])
{

    FILE *netlist_file = NULL;
    bool didParseGroundNode = false;

    
    // check that we have a filename
    // maybe update it to be able to receive UNIX pipe in the future
    if(argc < 2){
        print_help(argv[0]);
    }
    
    // open netlist file
    if((netlist_file = fopen(argv[1], "r"))){
        
        char *line = NULL;
        int line_number = 0;
        size_t len = 0;
        ssize_t line_length;
        element *parsed_element;
       
        printf("[-] Reading file: %s\n",argv[1]);
        
        // parse the file line by line
        while ((line_length = getline(&line, &len, netlist_file)) != -1) {
            
            char **tokens = NULL;
            
            // if line is a comment or empty line, skip it
            if((strncmp(line, "*",1) == 0) || line_length <= 2) {
               line_number++;
                continue;
            }
            
            // remove newline character from the end
            line[strlen(line)-1] = 0;
            
            tokens = tokenize(line);
            
            // create a new list node to hold the element attributes
            if( (parsed_element = (struct element *) malloc( sizeof(struct element))) == NULL) exit(-1);
            
            parsed_element->element_name = tokens[0];
            
            if(element_type_for_string(tokens[0][0])  == -1){
                
                printf("[!] Encountered element of uknown type: %s (line: %d)\n",tokens[0],line_number);
                exit(-1);

            } else {
                parsed_element->element_type = element_type_for_string(tokens[0][0]);
            }
            
            if(atoi(tokens[1]) == 0 || atoi(tokens[1]) == 0) didParseGroundNode = true;
            
            parsed_element->first_terminal = tokens[1];
            parsed_element->second_terminal = tokens[2];
            parsed_element->value = atof(tokens[3]);
            
            parsed_element->drain_terminal = tokens[1];
            parsed_element->gate_terminal = tokens[2];
            parsed_element->source_terminal = tokens[3];
            parsed_element->bulk_terminal = tokens[4];
            parsed_element->gate_length = atof(tokens[6]+2);
            parsed_element->gate_width = atof(tokens[7]+2);
            
            // add it to the list
            LL_APPEND(head,parsed_element);
            
            line_number++;
        
        }

        // close gracefully the file
        fclose(netlist_file);
        
    } else {
        // fopen error
        printf("\n[!] %s: %s\n\n",argv[1],strerror(errno));
        exit(-1);
    }
    
    if(!didParseGroundNode) printf("\n[WARNING] Netlist file looked legit but no ground node found!\n\n");
    
    // print the elements parsed
    print_elements_parsed(head);
    
    return 0;
}

