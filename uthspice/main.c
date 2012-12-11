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
#include <math.h>
#include <float.h>
#include <gsl/gsl_linalg.h>
#include <pthread.h>
#include "utarray.h"
#include "uthash.h"
#include "utlist.h"
#include "types.h"
#include "helpers.h"
#include "mna.h"
#include "solvers.h"


// CXSparse Lib
#include "cs.h"


// prints a simple help message
void print_help(const char *name)
{
    printf("\nUsage: .%s [options]\n",name);
    printf("\n\tAvailable options:\n\n \t-i FILE\t\t The netlist file to analyze\t\t\t[REQUIRED]\n");
    printf("\t-s\t\t\t Solve the MNA system. LU or Cholesky if SPD is set.\t[OPTIONAL]\n");
    printf("\t-v\t\t\t Verbose mode (prints lots of stuff)\t[OPTIONAL]\n");
    printf("\t-o\t\t\t Output file for DC sweeping\t[OPTIONAL]\n");
    exit(-1);
}


#pragma mark - Main func
// no comment
int main(int argc, char * argv[])
{

    int thread_create_status,option = 0;
    
    while ((option = getopt (argc, argv, "svi:o::")) != -1){
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
            case 'o':
                dcoutput_filename = strdup(optarg);
                break;
            default:
                print_help(argv[0]);
                break;
        }
    }
    
    if(argc < 2 || netlist_filename == NULL){
        print_help(argv[0]);
    }
    
    printf("[-] Reading file: %s\n",netlist_filename);
    
    // number of lines in the input netlist
    int lines_in_netlist = numOfNetlistLines(netlist_filename);
    
    // numof processor cores available for exploitation
    int numofcpus = 1;//(int)sysconf(_SC_NPROCESSORS_ONLN);
    
    pthread_t parsing_threads[numofcpus];
    struct thread_data parsing_threads__data_array[numofcpus];
    
    if(verbose) printf("\n[-] Netlist has %d lines.\n",lines_in_netlist);
    if(verbose) printf("\n[-] Spawning %d threads... \n",numofcpus);
    
    int chunk_size = lines_in_netlist / numofcpus;
    
    // initialize the  list locks
    if (pthread_rwlock_init(&elements_list_lock,NULL) != 0) {
        fprintf(stderr,"[!] Lock init failed\n");
        exit(-1);
    }
    if (pthread_rwlock_init(&options_list_lock,NULL) != 0) {
        fprintf(stderr,"[!] Lock init failed\n");
        exit(-1);
    }

    
    // start the (s)pwnage
    for (int i=0; i < lines_in_netlist/chunk_size; i++){

        int start_line = 1+i*chunk_size;
        int end_line = chunk_size + i* chunk_size;
        
        printf("[-] Assigning lines %d:%d to thread(%d)\n",start_line,end_line,i);
        
        // prepare thread arguments
        parsing_threads__data_array[i].thread_id = i;
        parsing_threads__data_array[i].start_line = start_line;
        parsing_threads__data_array[i].end_line = end_line;
        
        thread_create_status = pthread_create(&parsing_threads[i], NULL, parse_input_netlist, (void *) &parsing_threads__data_array[i]);
    
    }
    
    // wait for all the threads to complete
    for(int i = 0; i < lines_in_netlist/chunk_size; i++) {
        
        int rc = pthread_join(parsing_threads[i], NULL);
        if(rc < 0) { fprintf(stderr,"[!] Error encountered while trying to join thread with main thread\n"); }
    
    }
    
    if(!didParseGroundNode) printf("\n[WARNING] Netlist file looked legit but no ground node found!\n\n");
    
    replace_L_C();
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

