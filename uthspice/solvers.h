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
                
                fprintf(output_file,"\t%s=%g\t\t\t\t\tV%s=%f\n",option->node_name,start_value,plotting_node,gsl_vector_get(x, x_index));
                
                gsl_vector_free(x);
            
            }
            
        }
        
    }

}


/* Iterative Solvers */

// norm calculation
double norm(gsl_vector *vector,int num_of_nodes){
    double sum = 0.0;
    
    for(int i=0;i<num_of_nodes;i++){
        sum = sum + pow(gsl_vector_get(vector,i),2);
    }
    
    sum = sqrt(sum);
    return(sum);
}

// preconditioner solver
gsl_vector *precond_solver(gsl_vector *m,gsl_vector *r,int num_of_nodes){
    int i;
    gsl_vector *z = gsl_vector_alloc(numof_circuit_nodes + numof_indie_voltage_sources);
    
    for(i=0;i<num_of_nodes;i++){
        gsl_vector_set(z,i,gsl_vector_get(m,i)*gsl_vector_get(r,i));
    }
    
    return(z);
}

// Matrix Vector Product

gsl_vector *product_MV(gsl_matrix *A_table,gsl_vector *p,int num_of_nodes){
   
    double sum = 0.0;
    gsl_vector *q;
    
    q = gsl_vector_alloc(numof_circuit_nodes + numof_indie_voltage_sources);
    
    for(int i=0;i<num_of_nodes;i++){
        sum=0.0;
        for(int j=0;j<num_of_nodes;j++){
            sum = sum + gsl_matrix_get(A_table,i,j)*gsl_vector_get(p,j);
        }
        
        gsl_vector_set(q,i,sum);
        
    }
    return(q);
}

// biCG implementation
void Bi_CG(gsl_vector *x,gsl_matrix *A_table,gsl_vector *b,double itol,int num_of_nodes,gsl_vector *m){
	
    if(verbose) {
        printf("\n[-] Solving with Bi-Conjugate Gradients iterative method\n");
    }

    // transposed A
	gsl_matrix *A_T;
	int iter = 0;
	gsl_vector *z,*z_1,*p,*p_1,*q,*q_1,*r,*r_1,*Ax_product;
	double b_norm,rho,rho_1,alpha,beta,omega;
	
    // vector allocations
	r   = gsl_vector_alloc(num_of_nodes);
	r_1 = gsl_vector_alloc(num_of_nodes);
	z   = gsl_vector_alloc(num_of_nodes);
	z_1 = gsl_vector_alloc(num_of_nodes);
	p   = gsl_vector_alloc(num_of_nodes);
	p_1 = gsl_vector_alloc(num_of_nodes);
	q   = gsl_vector_alloc(num_of_nodes);
	q_1 = gsl_vector_alloc(num_of_nodes);
	
    Ax_product = gsl_vector_alloc(num_of_nodes);
	
	// Calculate the transpode of A
	A_T = gsl_matrix_alloc(num_of_nodes,num_of_nodes);
	for(int i=0;i<num_of_nodes;i++){
	    for(int j=0;j<num_of_nodes;j++){
            gsl_matrix_set(A_T,j,i,gsl_matrix_get(A_table,i,j));
	    }
	}
	
	Ax_product = product_MV(A_table,x,num_of_nodes);
	
	// r = b-Ax
	for(int i=0;i<num_of_nodes;i++){
	    gsl_vector_set(r,i,gsl_vector_get(b,i) - gsl_vector_get(Ax_product,i));
	    gsl_vector_set(r_1,i,gsl_vector_get(b,i) - gsl_vector_get(Ax_product,i));
	}
    
	
	//check if b_norm is zero
	b_norm = norm(z,num_of_nodes);
	if(b_norm == 0){
	    b_norm = 1;
	}
    
	while( (norm(r,num_of_nodes)/b_norm > itol) && (iter < num_of_nodes) ) {
	    iter=iter+1;
	    //preconditioner solve
	    z = precond_solver(m,r,num_of_nodes);
	    z_1 = precond_solver(m,r_1,num_of_nodes);
	    
	    rho = 0;
	    for(int i=0;i<num_of_nodes;i++){
            rho = rho + gsl_vector_get(r_1,i)*gsl_vector_get(z,i);
	    }
	    if(fabs(rho) < DBL_EPSILON){
            printf("[!] Algorithm failed. (rho < 1e-14)\n");
            exit(2); //algorithm failure
	    }
        
	    if(iter==1){
            for(int i=0;i<num_of_nodes;i++){
                //copy z to p
                gsl_vector_set(p,i,gsl_vector_get(z,i));
                gsl_vector_set(p_1,i,gsl_vector_get(z_1,i));
            }
	    }
	    else{
           
            beta = rho/rho_1;
            
            for(int i=0;i<num_of_nodes;i++){
                gsl_vector_set(p,i,gsl_vector_get(z,i) + beta*gsl_vector_get(p,i));
                gsl_vector_set(p_1,i,gsl_vector_get(z_1,i) + beta*gsl_vector_get(p_1,i));
            }
	    }
	    
	    rho_1 = rho;
	    q = product_MV(A_table,p,num_of_nodes);
	    q_1 = product_MV(A_T,p_1,num_of_nodes);
	    
	    omega = 0;
        
	    for(int i=0;i<num_of_nodes;i++){
            omega = omega + gsl_vector_get(p_1,i)*gsl_vector_get(q,i);
	    }
        
	    if(fabs(omega) < DBL_EPSILON){
            printf("[!] Algorithm failed. (omega < 1e-14)\n");
            exit(2); //algorithm failure
	    }
	    alpha = rho/omega;
	    
	    for(int i=0;i<num_of_nodes;i++){
	        gsl_vector_set(x,i,gsl_vector_get(x,i) + alpha*gsl_vector_get(p,i));
            gsl_vector_set(r,i,gsl_vector_get(r,i) - alpha*gsl_vector_get(q,i));
            gsl_vector_set(r_1,i,gsl_vector_get(r_1,i) - alpha*gsl_vector_get(q_1,i));
	    }
		
	}
	// print the solution vector
    printf("\nx = [\n");
	for(int i=0;i<num_of_nodes;i++){
	    printf("\t%f\n",gsl_vector_get(x,i));
	}
    printf("]\n");
	
	free(r);
	free(p);
	free(q);
	free(z);
	free(Ax_product);
	free(r_1);
	free(p_1);
	free(q_1);
	free(z_1);
	free(A_T);
}

// CG algorithm
void CG(gsl_vector *x,gsl_matrix *A_table,gsl_vector *z,double itol,int num_of_nodes,gsl_vector *m){
	
    if(verbose) {
        printf("\n[-] Solving with Conjugate Gradients iterative method\n");
    }
    
	gsl_vector *r,*p,*q,*z_1,*Ax_product;
	double b_norm,rho,rho_1,alpha,beta,ptq;
	int iter=0;
	
	r   = gsl_vector_alloc(num_of_nodes);
	p   = gsl_vector_alloc(num_of_nodes);
	q   = gsl_vector_alloc(num_of_nodes);
	z_1 = gsl_vector_alloc(num_of_nodes);
	Ax_product = gsl_vector_alloc(num_of_nodes);
    
    Ax_product = product_MV(A_table,x,num_of_nodes);
	
	// r=b-Ax
	for(int i=0;i<num_of_nodes;i++){
	    gsl_vector_set(r,i,gsl_vector_get(z,i) - gsl_vector_get(Ax_product,i));
	}
	
	//check if b_norm is zero
	b_norm = norm(z,num_of_nodes);
	if(b_norm == 0){
	    b_norm = 1;
	}
	
	while( (norm(r,num_of_nodes)/b_norm > itol) && (iter < num_of_nodes) ) {
	    
	    iter=iter+1;
        
	    //preconditioner solve
	    z_1 = precond_solver(m,r,num_of_nodes);
	    
	    rho = 0;
	    for(int i=0;i<num_of_nodes;i++){
            rho = rho + gsl_vector_get(r,i)*gsl_vector_get(z_1,i);
	    }
	    if(iter==1){
            for(int i=0;i<num_of_nodes;i++){
                //copy z to p
                gsl_vector_set(p,i,gsl_vector_get(z_1,i));
            }
	    }
	    else{
            beta = rho/rho_1;
            for(int i=0;i<num_of_nodes;i++){
                gsl_vector_set(p,i,gsl_vector_get(z_1,i) + beta*gsl_vector_get(p,i));
            }
            
	    }
	    rho_1 = rho;
		
		
	    q = product_MV(A_table,p,num_of_nodes);
	    ptq=0;
	    for(int i=0;i<num_of_nodes;i++){
            ptq = ptq + gsl_vector_get(p,i)*gsl_vector_get(q,i);
	    }
	    alpha = rho/ptq;
	    for(int i=0;i<num_of_nodes;i++){
            gsl_vector_set(x,i,gsl_vector_get(x,i) + alpha*gsl_vector_get(p,i));
            gsl_vector_set(r,i,gsl_vector_get(r,i) - alpha*gsl_vector_get(q,i));
	    }
        
	}
	
    // print the solution vector
    printf("x = [\n");
	for(int i=0;i<num_of_nodes;i++){
		printf("\t%f\n",gsl_vector_get(x,i));
	}
    printf("]\n");
    
	free(r);
	free(p);
	free(q);
	free(z_1);
	free(Ax_product);

}

void solve_MNA(){
    
    prepare_MNA_matrices();
    
    // non-iterative methods
    if(!bi_conjgrad && !conjgrad){
        LU_solve(numof_indie_voltage_sources+numof_circuit_nodes);
    }
    
    // conjugate gradients
    else {
        
        gsl_vector *init_guess,*m;
        
        m = gsl_vector_alloc(numof_circuit_nodes + numof_indie_voltage_sources);
        init_guess = gsl_vector_alloc(numof_circuit_nodes + numof_indie_voltage_sources);
        
        for(int i=0;i < numof_circuit_nodes + numof_indie_voltage_sources;i++){
            gsl_vector_set(init_guess,i,0.0);
        }
        
        for(int i=0;i < numof_circuit_nodes + numof_indie_voltage_sources;i++){
            
            if(gsl_matrix_get(A_table,i,i)!= 0){
                gsl_vector_set(m,i,1/gsl_matrix_get(A_table,i,i));
            } else {
                gsl_vector_set(m,i,1.0);
            }
        }
        
        if(!bi_conjgrad && conjgrad){
            CG(init_guess,A_table,z,itol,numof_indie_voltage_sources+numof_circuit_nodes,m);
        }
        // bi-conjugate gradients
        else if(bi_conjgrad){
            Bi_CG(init_guess,A_table,z,itol,numof_indie_voltage_sources+numof_circuit_nodes,m);
        }
        
        gsl_vector_free(init_guess);
        gsl_vector_free(m);
    }
    
    if(dc_sweep && found_plotting_node){
        perform_DC_sweep();
    }
    
    gsl_matrix_free(A_table);
    gsl_vector_free(z);
    gsl_permutation_free(perm_matrix);

}

void solve_sparse() {
    prepare_sparse_matrices();
}

void solve(){
    
    // prints the id of the node_name in the hashtable
    if(verbose){
        struct node_data *s;
        printf("\n");
        for(s=nodes_hashtable; s != NULL; s=(struct node_data*)(s->hh.next)) {
            printf("[nodes_hashtable] key %s has id %d\n", s->node_name,s->node_num);
        }
    }
    
    // Simple matrices for MNA 
    if(!sparse){
        solve_MNA();
    }
    // use sparse matrix techniques
    else {
        solve_sparse();
    }
}


