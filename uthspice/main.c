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

// list head, should always be initialized to NULL
element *head = NULL;

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


//prints the list of the nodes parsed
void print_elements_parsed() {
    
    element *elem;
    
    printf("\n[-] Parsed elements:\n");
    
    printf("\n         Type    Name    Terminal 1    Terminal 2      Value    Drain Terminal    Gate Terminal    Source Terminal    Bulk Terminal    Gate Length    Gate Width\n");
    
    LL_FOREACH(head, elem){
        if(elem->element_type != elementTypeBJTTransistor && elem->element_type != elementTypeMOSTransistor){
            printf("%13s %7s %8s %13s %15.1e          -                  -                -                 -                -            - \n",name_of_element_for_type(elem->element_type),elem->element_name,elem->first_terminal,elem->second_terminal,elem->value);
            
        } else {
            printf("%s %7s        -             -            - %13s %18s %17s %18s %16.1e %14.1e\n",name_of_element_for_type(elem->element_type),elem->element_name,elem->drain_terminal,elem->gate_terminal,elem->source_terminal,elem->bulk_terminal,elem->gate_length,elem->gate_width);
        }

    }
    
}   

// prints a simple help message
void print_help(const char *name)
{
    printf("\n\tUsage: %s <netlist file>\n",name);
    exit(-1);
}

// takes the parsed line and returns an array of tokens split by " "
char **tokenize(char *line)
{
    char **tokens;
    char *token;
    int i=0;

    tokens = (char **)malloc(MAX_TOKENS_IN_LINE*sizeof(char*));
    
    for(int y=0;y<MAX_TOKENS_IN_LINE;y++){
        tokens[y] = (char *)malloc(MAX_TOKEN_LEN*sizeof(char));
    }
    
    token = strtok(line," ");
    
    while (token != NULL) {

        // potential buffer overflow here but who cares ;)
        strcpy((char *)tokens[i],token);
        token = strtok(NULL, " ");
        i++;
   
    }
    
    return tokens;
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
    
    if(!didParseGroundNode) printf("[WARNING] Netlist file looked legit but no ground node found!\n");
    
    // print the elements parsed
    print_elements_parsed();
    
    return 0;
}

