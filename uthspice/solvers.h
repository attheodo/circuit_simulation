//
//  solvers.h
//  uthspice
//
//  Created by Athanasios Theodoridis (461) and Aris Eleftheriadis (460) on 4/10/12.
//  

void LU_solve(int size){
    
    gsl_vector *x;
    gsl_permutation *perm_matrix;
    int perm_sign = 0;
    
    x = gsl_vector_alloc (size);
    
    printf("\n\n[-] Solving %dx%d system with LU decomposition\n",size,size);
    
    perm_matrix = gsl_permutation_alloc(size);
    
    gsl_linalg_LU_decomp (A_table, perm_matrix, &perm_sign);
    gsl_linalg_LU_solve (A_table, perm_matrix, z, x);
    
    printf("\nx = [\n");
    gsl_vector_fprintf (stdout, x, "\t%g");
    printf("]\n");

}