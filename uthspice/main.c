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
#include <gsl/gsl_linalg.h>
#include "utarray.h"
#include "uthash.h"
#include "utlist.h"
#include "types.h"
#include "helpers.h"
#include "mna.h"
#include "solvers.h"


// prints a simple help message
void print_help(const char *name)
{
    printf("\nUsage: .%s [options]\n",name);
    printf("\n\tAvailable options:\n\n \t-i FILE\t\t The netlist file to analyze\t\t\t[REQUIRED]\n");
    printf("\t-s\t\t\t Solve the MNA system. LU or Cholesky if SPD is set.\t[OPTIONAL]\n");
    printf("\t-v\t\t\t Verbose mode (prints lots of stuff)\t[OPTIONAL]\n");
    exit(-1);
}


#pragma mark - Main func
// no comment
int main(int argc, char * argv[])
{

    FILE *netlist_file = NULL;
    char *netlist_filename = NULL;
    bool didParseGroundNode = false;
    int option = 0;
    
   
    while ((option = getopt (argc, argv, "svi:")) != -1)
    {
        switch (option)
        {
            case 'v':
                verbose = true;
                break;
            case 'i':
                netlist_filename = strdup(optarg);
                break;
            case 's':
                should_solve = true;
                break;
            default:
                print_help(argv[0]);
                break;
        }
    }
    
    if(argc < 2 || netlist_filename == NULL){
        print_help(argv[0]);
    }
    
    
    // open netlist file
    if((netlist_file = fopen(netlist_filename, "r"))){
        
        char *line = NULL;
        int line_number = 0;
        size_t len = 0;
        ssize_t line_length;
        element *parsed_element;
       
        printf("[-] Reading file: %s\n",netlist_filename);
        
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
            
            // encountered unknown element
            if(element_type_for_string(tokens[0][0])  == -1){
                
                printf("[!] Encountered element of uknown type: %s (line: %d)\n",tokens[0],line_number);
                

            }
            
            // encountered option
            else if(element_type_for_string(tokens[0][0]) == elementTypeOption){
                parse_netlist_option(tokens);
            }
            
            // known netlist element
            else {
                parsed_element->element_type = element_type_for_string(tokens[0][0]);
               
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

            }
            
                       
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
    
    if(verbose) printf("\n[-] Circuit has %d nodes without the ground node.\n",numof_circuit_nodes);
    if(verbose) printf("[-] Circuit has %d independant Voltage sources.\n\n",numof_indie_voltage_sources);

    if(should_solve){
        solve();
    }
       
    return 0;
}

