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
            
            printf("\t%s: %s terminal_1: %s terminal_2: %s value: %f\n",name_of_element_for_type(current_element->element_type),current_element->element_name,current_element->first_terminal,current_element->second_terminal,current_element->value);
        }
        // print transistor element
        else {
            printf("\t%s: %s drain_terminal: %s gate_terminal: %s source_terminal: %s bulk_terminal: %s gate_length: %.10f gate_width: %.10f\n",name_of_element_for_type(current_element->element_type),current_element->element_name,current_element->drain_terminal,current_element->gate_terminal,current_element->source_terminal,current_element->bulk_terminal,current_element->gate_length,current_element->gate_width);

        }
        
    }
    
    // last element of list
    if(current_element->element_type != elementTypeBJTTransistor && current_element->element_type != elementTypeMOSTransistor){
        printf("\t%s: %s terminal_1: %s terminal_2: %s value: %f\n",name_of_element_for_type(current_element->element_type),current_element->element_name,current_element->first_terminal,current_element->second_terminal,current_element->value);

    } else {
         printf("\t%s: %s drain_terminal: %s gate_terminal: %s source_terminal: %s bulk_terminal: %s gate_length: %.10f gate_width: %.10f\n",name_of_element_for_type(current_element->element_type),current_element->element_name,current_element->drain_terminal,current_element->gate_terminal,current_element->source_terminal,current_element->bulk_terminal,current_element->gate_length,current_element->gate_width);
    }
   
    
}   

// prints a simple help message
void print_help(const char *name)
{
    printf("\n\tUsage: %s <netlist file>\n",name);
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
        return -1;
    }
    
    // open netlist file
    if((netlist_file = fopen(argv[1], "r"))){
        
        char *line = NULL;
        int line_number = 0;
        size_t len = 0;
        ssize_t line_length;
       
        
        // initialize the data structure that'll hold the parsed elements
        initElementsList();
        
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
            element_node = (struct element *) malloc( sizeof(struct element) ); 
            
            if(list_head == NULL){
                element_node->next_element = NULL;
                list_head->next_element = element_node;
            
            } else {
            
                struct element *current_node;
                current_node = (struct element *) malloc( sizeof(struct element) );
                current_node = list_head;
                
                while(current_node != NULL){
                    
                    if(current_node->next_element == NULL){
                        current_node->next_element=element_node;
                        break;
                    }
                    current_node = current_node->next_element;
                }
            }
           
            element_node->element_name = tokens[0];
            
            if(element_type_for_string(tokens[0][0])  == -1){
            
                
                printf("[!] Encountered element of uknown type: %s (line: %d)\n",tokens[0],line_number);
                exit(-1);

            } else {
                element_node->element_type = element_type_for_string(tokens[0][0]);
            }
            
            if(atoi(tokens[1]) == 0 || atoi(tokens[1]) == 0) didParseGroundNode = true;
            
            element_node->first_terminal = tokens[1];
            element_node->second_terminal = tokens[2];
            element_node->value = atof(tokens[3]);
            
            element_node->drain_terminal = tokens[1];
            element_node->gate_terminal = tokens[2];
            element_node->source_terminal = tokens[3];
            element_node->bulk_terminal = tokens[4];
            element_node->gate_length = atof(tokens[6]+2);
            element_node->gate_width = atof(tokens[7]+2);
            
            
            line_number++;
        
        }

        // close gracefully the file
        fclose(netlist_file);
        
    } else {
        // fopen error
        printf("[!] %s: %s",argv[1],strerror(errno));
    }
    
    if(!didParseGroundNode) printf("[WARNING] Netlist file looked legit but no ground node found!\n");
    
    // print the elements parsed
    print_elements_parsed();
    
    return 0;
}

