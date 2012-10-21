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
#include "utarray.h"
#include "uthash.h"
#include "utlist.h"
#include "types.h"
#include "helpers.h"
#include "mna.h"


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
    
    replace_L_C();
    // print the elements parsed
    print_elements_parsed();
    
        
    numof_circuit_nodes = numberOfNodes(head);
    numof_indie_voltage_sources = numOfIndependentVoltageSources();
    numof_current_sources = numOfCurrentSources();
    
    if(DEBUG){
        struct node_data *s;
        printf("\n");
        for(s=nodes_hashtable; s != NULL; s=(struct node_data*)(s->hh.next)) {
            printf("[nodes_hashtable] key %s has id %d\n", s->node_name,s->node_num);
        }
        
    }

    if(DEBUG) printf("\n[-] Circuit has %d nodes without the ground node.\n",numof_circuit_nodes);
    if(DEBUG) printf("[-] Circuit has %d independant Voltage sources.\n\n",numof_indie_voltage_sources);
    
    // Initialize G Table
    g_table = (double **)malloc(numof_circuit_nodes * sizeof(double *));
    for(int i=0; i < numof_circuit_nodes; i++){
        g_table[i] = (double *)malloc(numof_circuit_nodes * sizeof(double));
    }
    
    // Initialize B table
    b_table = (int **)malloc(numof_circuit_nodes * sizeof(int *));
    for(int i=0; i < numof_circuit_nodes; i++){
        b_table[i] = (int *)malloc(numof_indie_voltage_sources * sizeof(int));
    }
    
    // Initialize C table
    c_table = (int **)malloc(numof_indie_voltage_sources * sizeof(int *));
    for(int i=0; i < numof_indie_voltage_sources; i++){
        c_table[i] = (int *)malloc(numof_circuit_nodes * sizeof(int));
        
    }
    
    // Initialize D table
    d_table = (int **)malloc(numof_indie_voltage_sources * sizeof(int *));
    for(int i=0; i < numof_indie_voltage_sources; i++){
        d_table[i] = (int *)malloc(numof_indie_voltage_sources * sizeof(int));
        
    }
    
    // Initialize A table
    A_table = (double **)malloc((numof_circuit_nodes+numof_indie_voltage_sources) * sizeof(double *));
    for(int i=0; i < numof_circuit_nodes+numof_indie_voltage_sources; i++){
        A_table[i] = (double *)malloc((numof_circuit_nodes+numof_indie_voltage_sources) * sizeof(double));
    }

    // initialize Z table
    z_table = (double *)malloc((numof_current_sources+numof_indie_voltage_sources)*sizeof(double));
    
    calculate__g_table(head,numof_circuit_nodes);
    calculate__b_table(numof_circuit_nodes, numof_indie_voltage_sources);
    calculate__c_table(numof_circuit_nodes, numof_indie_voltage_sources);
    calculate__d_table(numof_indie_voltage_sources);
    calculate__A_table(numof_circuit_nodes, numof_indie_voltage_sources);
    create__z_table(numof_circuit_nodes, numof_indie_voltage_sources,numof_current_sources);
    
    if(DEBUG)print__g_table(numof_circuit_nodes);
    if(DEBUG)print__b_table(numof_circuit_nodes,numof_indie_voltage_sources);
    if(DEBUG)print__c_table(numof_indie_voltage_sources,numof_circuit_nodes);
    print__A_table(numof_circuit_nodes+numof_indie_voltage_sources, numof_indie_voltage_sources+numof_circuit_nodes);
    print__Z_table(numof_circuit_nodes, numof_indie_voltage_sources);
    
    return 0;
}

