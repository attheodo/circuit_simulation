//
//  sparse.h
//  uthspice
//
//  Created by Athanasios Theodoridis on 12/12/12.
//  Copyright (c) 2012 @attheodo. All rights reserved.
//

void prepare_sparse_matrices() {
    
    element *elem = NULL;
    int matrix_dimension;
    int voltagesource_id = 0;
    
    // allocate A sparse matrix
    matrix_dimension = numof_circuit_nodes + numof_indie_voltage_sources;
    A_table_sparse = cs_spalloc(matrix_dimension,matrix_dimension,nonzeroes,1,1);
    
    // populate the A matrix directly
    LL_FOREACH(head, elem){
        
        // *** Resistor contributions
        if(elem->element_type == elementTypeResistance){
            
            int first_nodeid = id_for_node_in_hash(elem->first_terminal);
            int second_nodeid = id_for_node_in_hash(elem->second_terminal);
            
            // if grounded, it contributes only to the diagonal
            if (first_nodeid == GROUND || second_nodeid == GROUND){

                if(verbose) printf("[A_table] 1/%s=%f (%s,%s) added to (%d,%d)\n",elem->element_name,1/elem->value,elem->first_terminal,elem->second_terminal,first_nodeid != 0 ? first_nodeid : second_nodeid,first_nodeid != 0 ? first_nodeid : second_nodeid);
                
                //g_table[i-1][i-1] += 1/elem->value;
                int index = first_nodeid != 0 ? first_nodeid - 1 : second_nodeid - 1;
                cs_entry(A_table_sparse, index, index, 1/elem->value);
            }
            
            // if not grounded it contributes to 4 different locations
            else if(first_nodeid != GROUND && second_nodeid != GROUND){
                
                // minus the value of conductance to (first_term, second_term) and (second_term, first_term)
                if(verbose) printf("[A_table] -1/%s=%f (%s,%s) added to (%d,%d) and (%d,%d)\n",elem->element_name,1/elem->value,elem->first_terminal,elem->second_terminal,first_nodeid,second_nodeid,second_nodeid,first_nodeid);
                
                cs_entry(A_table_sparse, first_nodeid - 1, second_nodeid - 1, -(1/elem->value));
                cs_entry(A_table_sparse, second_nodeid - 1, first_nodeid - 1, -(1/elem->value));
                
                // the value of conductance on the diagonal
                if(verbose) printf("[A_table] 1/%s=%f (%s,%s) added to (%d,%d)\n",elem->element_name,1/elem->value,elem->first_terminal,elem->second_terminal,first_nodeid, first_nodeid);
                if(verbose) printf("[A_table] 1/%s=%f (%s,%s) added to (%d,%d)\n",elem->element_name,1/elem->value,elem->first_terminal,elem->second_terminal,second_nodeid, second_nodeid);
                
                cs_entry(A_table_sparse, first_nodeid - 1, first_nodeid - 1, 1/elem->value);
                cs_entry(A_table_sparse, second_nodeid - 1, second_nodeid - 1, 1/elem->value);

                
            }
        }
        // *** Voltage source contributions
        if(elem->element_type == elementTypeVoltageSource){
            
            int positive_terminal = id_for_node_in_hash(elem->first_terminal);
            int negative_terminal = id_for_node_in_hash(elem->second_terminal);
            
            // if voltage source is not connected to ground, it will have two entries in the table
            if((positive_terminal != GROUND) && (negative_terminal != GROUND)){
                
                if(verbose) printf("[A_table] Voltage source %s (%d,%d) is not grounded. Entries in A_table[%d][%d]=-1 and A_table[%d][%d]=1\n",elem->element_name,positive_terminal,negative_terminal,negative_terminal,(numof_circuit_nodes) + voltagesource_id,positive_terminal,(numof_circuit_nodes) + voltagesource_id);
                
                cs_entry(A_table_sparse, negative_terminal - 1, (numof_circuit_nodes + 1) + voltagesource_id, -1);
                cs_entry(A_table_sparse, positive_terminal - 1, (numof_circuit_nodes + 1) + voltagesource_id, 1);
                
                if(verbose) printf("[A_table] Voltage source %s (%d,%d) is not grounded. Entries in A_table[%d][%d]=-1 and A_table[%d][%d]=1\n",elem->element_name,positive_terminal,negative_terminal,(numof_circuit_nodes) + voltagesource_id,negative_terminal,(numof_circuit_nodes) + voltagesource_id,positive_terminal);
                
                cs_entry(A_table_sparse, (numof_circuit_nodes + 1) + voltagesource_id, negative_terminal - 1, -1);
                cs_entry(A_table_sparse, (numof_circuit_nodes + 1) + voltagesource_id, positive_terminal - 1, 1);
                
                
                voltagesource_id++;

            }
            // if voltage source is grounded, there's one entry in the table
            else {
               
                if(positive_terminal == GROUND){
                    
                    if(verbose) printf("[A_table] Voltage source %s (%d,%d) is grounded. Entry in A_table[%d][%d]=-1\n",elem->element_name,positive_terminal,negative_terminal,negative_terminal,(numof_circuit_nodes) + voltagesource_id);
                   

                    cs_entry(A_table_sparse, negative_terminal - 1, numof_circuit_nodes + 1 + voltagesource_id, -1);
                    cs_entry(A_table_sparse, numof_circuit_nodes + voltagesource_id, negative_terminal - 1, -1);
                    
                    //voltagesource_id++;
                    
                } else if(negative_terminal == GROUND){
                    
                    if(verbose) printf("[A_table] Voltage source %s (%d,%d) is grounded. Entry in A_table[%d][%d]=1\n",elem->element_name,positive_terminal,negative_terminal,positive_terminal,(numof_circuit_nodes) + voltagesource_id);
                    
                    cs_entry(A_table_sparse, positive_terminal - 1, numof_circuit_nodes + voltagesource_id, 1);
                    cs_entry(A_table_sparse, numof_circuit_nodes + voltagesource_id, positive_terminal - 1, 1);
                    
                    //voltagesource_id++;
                    
                }

            }
        }
        
    }
    
    // sum duplicates
    C_sparse_compressed = cs_compress(A_table_sparse);
    cs_dupl(C_sparse_compressed);
    cs_free(A_table_sparse);
    
    if(verbose) cs_print(C_sparse_compressed, 0);
    
}