//
//  solvers.h
//  uthspice
//
//  Created by Athanasios Theodoridis (461) and Aris Eleftheriadis (460) on 4/10/12.
//  

void LU_solve(int size){
    
    gsl_vector *x;
    
    int perm_sign,solve_status = 0;
    
    x = gsl_vector_alloc (size);
    
    perm_matrix = gsl_permutation_alloc(size);

    gsl_set_error_handler_off();
    
    if(!spd){
        printf("\n\n[-] Solving %dx%d system with LU decomposition\n",size,size);
        gsl_linalg_LU_decomp (A_table, perm_matrix, &perm_sign);
        solve_status = gsl_linalg_LU_solve (A_table, perm_matrix, z, x);
    } else {
        printf("\n\n[-] Solving %dx%d system with Cholesky decomposition\n",size,size);
        gsl_linalg_cholesky_decomp(A_table);
        solve_status = gsl_linalg_cholesky_solve(A_table, z, x);
    }
    
     
    if(solve_status == 1){
        printf("\n[!] The circuit has no solution (matrix is singular)\n");
        gsl_vector_free(x);
        return;
    
    } else {
        printf("\nx = [\n");
        gsl_vector_fprintf (stdout, x, "\t%g");
        printf("]\n");
        gsl_vector_free(x);
    }
    
}

void perform_DC_sweep(){
    
    struct netlist_option *option, *option2;
    char *plotting_node;
    FILE *output_file = NULL;
    
    if(dcoutput_filename == NULL) {
        
        printf("[-] Saving DC sweep output to dcsweep_output.txt");
        dcoutput_filename = "dc_sweep_output.txt";
    
    }
    
    // open the output file
    if ((output_file = fopen(dcoutput_filename, "w+")) == NULL) {
        fprintf(stderr, "Can't open output file %s!\n",dcoutput_filename);
        exit(1);
    }

    
    // find the .dc sweep option in the options linked list we keep for this
    LL_FOREACH(netlist_options, option){
        
        if(option->option_type == optionTypeDCSweep){
            
            // find the voltage node we want to print/plot in our options list
            // this is what's right next the .PLOT/.PRINT comamnd in the netlist
            LL_FOREACH(netlist_options, option2){
                if(option2->option_type == optionTypePlot){
                    plotting_node = option2->node_name;
                }
            }
            
            // find the position in the z vector of the corresponding Voltage Source we want to increment.
            int z_index = z_index_position_for_voltagesource(option->node_name);
           
            if(verbose) printf("\n[-] DC SWEEP: Found %s in z vector at position: %d\n",option->node_name,z_index);
            if(verbose) printf("\tPlotting/Printing voltage for node: %s\n",plotting_node);
            
            for(float start_value = option->start_value; start_value <= option->end_value; start_value += option->step){
                
                gsl_vector *x = gsl_vector_alloc(numof_indie_voltage_sources+numof_circuit_nodes);
                gsl_vector_set(z,z_index,start_value);
                
                int solve_status = 0;
                
                if(!spd){
                    solve_status = gsl_linalg_LU_solve (A_table, perm_matrix, z, x);
                } else {
                    solve_status = gsl_linalg_cholesky_solve(A_table, z, x);
                }
                
                if(solve_status != 0) {
                    printf("[!] FATAL ERROR: Error solving the system\n");
                }
                
                int x_index = id_for_node_in_hash(plotting_node);
                
                fprintf(stdout,"\t%s=%g\t\t\t\t\tV%s=%f\n",option->node_name,start_value,plotting_node,gsl_vector_get(x, x_index));
                
                gsl_vector_free(x);
            
            }
            
        }
        
    }

}

void solve(){
    
    if(verbose){
        struct node_data *s;
        printf("\n");
        for(s=nodes_hashtable; s != NULL; s=(struct node_data*)(s->hh.next)) {
            printf("[nodes_hashtable] key %s has id %d\n", s->node_name,s->node_num);
        }
        
    }
    
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
    
    A_table = gsl_matrix_alloc(numof_circuit_nodes+numof_indie_voltage_sources, numof_circuit_nodes+numof_indie_voltage_sources);
    
    // initialize Z table
    z = gsl_vector_alloc(numof_circuit_nodes + numof_indie_voltage_sources);
    
    calculate__g_table(head,numof_circuit_nodes);
    calculate__b_table(numof_circuit_nodes, numof_indie_voltage_sources);
    calculate__c_table(numof_circuit_nodes, numof_indie_voltage_sources);
    calculate__d_table(numof_indie_voltage_sources);
    calculate__A_matrix(numof_circuit_nodes, numof_indie_voltage_sources);
    create__z_vector(numof_circuit_nodes, numof_indie_voltage_sources,numof_current_sources);
    
    if(verbose)print__g_table(numof_circuit_nodes);
    if(verbose)print__b_table(numof_circuit_nodes,numof_indie_voltage_sources);
    if(verbose)print__c_table(numof_indie_voltage_sources,numof_circuit_nodes);
    if(verbose)print__A_matrix(numof_circuit_nodes+numof_indie_voltage_sources, numof_indie_voltage_sources+numof_circuit_nodes);
    if(verbose)print__Z_vector(numof_circuit_nodes, numof_indie_voltage_sources);
    
    LU_solve(numof_indie_voltage_sources+numof_circuit_nodes);
        
    if(dc_sweep && found_plotting_node){
        perform_DC_sweep();
    }
    
    free(g_table);
    free(b_table);
    free(c_table);
    free(d_table);
    
    gsl_matrix_free(A_table);
    gsl_vector_free(z);
    gsl_permutation_free(perm_matrix);
    
}