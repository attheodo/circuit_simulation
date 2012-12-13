//
//  mna.h
//
//  Created by Athanasios Theodoridis (461) and Aris Eleftheriadis (460) on 4/10/12.
//
//

// Returns the number of nodes without the ground. It also populated 'unique_circuit_nodes' array
// with all the nodes of the circuit.
int numberOfNodes(element *parsed_elements){

    element *elem;
    int index = 1;
    
    struct node_data *node_item = NULL;

    // start browsing the list
    LL_FOREACH(parsed_elements, elem){
       
        // check if the node is already in the hashtable, if not add it
        HASH_FIND_STR(nodes_hashtable, elem->first_terminal, node_item);
        if (node_item == NULL) {
            node_item = (struct node_data *)malloc(sizeof(struct node_data));
            node_item->node_num = generate_uniqueid(elem->first_terminal);
            strcpy(node_item->node_name, elem->first_terminal);
            HASH_ADD_STR(nodes_hashtable, node_name, node_item);
            index++;
        
        }
        
        node_item = NULL;
        
        // check if the node is already in the hashtable, if not add it
        HASH_FIND_STR(nodes_hashtable, elem->second_terminal, node_item);
        if (node_item == NULL) {
            node_item = (struct node_data *)malloc(sizeof(struct node_data));
            node_item->node_num = generate_uniqueid(elem->second_terminal);
            strcpy(node_item->node_name, elem->second_terminal);
            HASH_ADD_STR(nodes_hashtable, node_name, node_item);
            index++;
        }
        
        node_item = NULL;

    }

    free(node_item);
        
    
    // return the number of nodes with out the ground (-1)
    return HASH_COUNT(nodes_hashtable) - 1;
}

// returns the number of voltage sources found in the parsed circuit
int numOfIndependentVoltageSources(){
    
    element *elem = NULL;
    int numOfVoltageSources = 0;
    
    // start browsing the list
    LL_FOREACH(head, elem){
        if(elem->element_type == elementTypeVoltageSource) numOfVoltageSources++;
    }
    
    // return the number of nodes with out the ground (-1)
    return numOfVoltageSources;

}

// return the number of current sources in the parsed circuit
int numOfCurrentSources(){
    element *elem = NULL;
    int numOfCurrentSources = 0;
    
    // start browsing the list
    LL_FOREACH(head, elem){
        if(elem->element_type == elementTypeCurrentSource) numOfCurrentSources++;
    }
    
    // return the number of nodes with out the ground (-1)
    return numOfCurrentSources;
}

void calculate__g_table(element *parsed_elements, int num_of_nodes){
    
    element *elem = NULL;
    
    // initialize table
    for(int i=0; i < num_of_nodes; i++){
        for(int z=0; z < num_of_nodes; z++){
            g_table[i][z] = 0.0;
        }
    }
   
    // first calculate the diagonal table
    for(int i=1; i <= num_of_nodes;i++){
        
        LL_FOREACH(parsed_elements, elem){
            
            // we only care about resistors
            if(elem->element_type == elementTypeResistance){
                
                // if the element has at least one terminal connected to our now, add it to the table
                if( (id_for_node_in_hash(elem->first_terminal) == i) || (id_for_node_in_hash(elem->second_terminal) == i)){
                    
                    if(verbose) printf("[g_table] 1/%s=%f (%s,%s) added to (%d,%d)\n",elem->element_name,1/elem->value,elem->first_terminal,elem->second_terminal,i,i);
                    g_table[i-1][i-1] += 1/elem->value;
               
                }

            }
            
        }
    }
    
    // for each element that has not a node connected to the ground, add it
    // to (i,z) and (z,i) in the table with minus it's value
    LL_FOREACH(parsed_elements, elem){
        
        // we only care about resistors
        if(elem->element_type == elementTypeResistance){
           
            int first_nodeid = id_for_node_in_hash(elem->first_terminal);
            int second_nodeid = id_for_node_in_hash(elem->second_terminal);
            
            if( (first_nodeid != 0) && (second_nodeid != 0)){
              
                if(verbose) printf("[g_table] -1/%s=%f (%s,%s) added to (%d,%d) and (%d,%d)\n",elem->element_name,1/elem->value,elem->first_terminal,elem->second_terminal,first_nodeid,second_nodeid,second_nodeid,first_nodeid);
                
                g_table[first_nodeid-1][second_nodeid-1] += -(1/elem->value);
                g_table[second_nodeid-1][first_nodeid-1] += -(1/elem->value);

            }
        }
    }
    
}

// calculates the B table
void calculate__b_table(int num_of_nodes, int num_of_voltage_sources){
    
    element *elem = NULL;
    
    int voltagesource_id = 0;
   
    // initialize table with zeros
    for(int i=0; i < num_of_nodes; i++){
        for(int z=0; z < num_of_voltage_sources; z++){
            b_table[i][z] = 0;
        }
    }
    
    // go through the list
    LL_FOREACH(head, elem){
        
        // we only care about voltage sources
        if(elem->element_type == elementTypeVoltageSource){
            
            int positive_terminal = id_for_node_in_hash(elem->first_terminal);
            int negative_terminal = id_for_node_in_hash(elem->second_terminal);
            
            // if voltage source is not connected to ground, it will have two entries in the table
            if((positive_terminal != GROUND) && (negative_terminal != GROUND)){
                
                if(verbose) printf("[A_table] Voltage source %s (%d,%d) is not grounded. Entries in g_table[%d][%d]=-1 and g_table[%d][%d]=1\n",elem->element_name,positive_terminal,negative_terminal,negative_terminal,voltagesource_id,positive_terminal,voltagesource_id);
                b_table[negative_terminal-1][voltagesource_id] = -1;
                b_table[positive_terminal-1][voltagesource_id] = 1;
                voltagesource_id++;
            
            } else {
                
                // voltage source has one grounded node. There's one entry in the b table
                if(positive_terminal == GROUND){
                   
                    if(verbose) printf("[b_table] Voltage source %s (%d,%d) is grounded. Entry in b_table[%d][%d]=-1\n",elem->element_name,positive_terminal,negative_terminal,negative_terminal,voltagesource_id);

                    b_table[negative_terminal-1][voltagesource_id] = -1;
                    voltagesource_id++;
                
                } else if(negative_terminal == GROUND){
                
                    if(verbose) printf("[b_table] Voltage source %s (%d,%d) is grounded. Entry in b_table[%d][%d]=1\n",elem->element_name,positive_terminal,negative_terminal,positive_terminal,voltagesource_id);

                    b_table[positive_terminal-1][voltagesource_id] = 1;
                    voltagesource_id++;
                
                }
                
            }
            
        }
        
    }
}

void calculate__c_table(int numof_circuit_nodes, int numof_indie_voltage_sources){
    
    for(int i=0; i < numof_circuit_nodes; i++){
        for(int z=0; z < numof_indie_voltage_sources; z++){
            c_table[z][i] = b_table[i][z];
        }
    }
}

void calculate__d_table(int numof_indie_voltage_sources){
    for(int i=0;i < numof_indie_voltage_sources;i++){
        for(int z=0; z < numof_indie_voltage_sources; z++){
            d_table[i][z] = 0;
        }
    }
}

void calculate__A_matrix(int numof_circuit_nodes, int numof_indie_voltage_sources){
    
    // merge g table
    for(int i=0; i < numof_circuit_nodes;i++){
        for(int z=0; z < numof_circuit_nodes;z++){
            gsl_matrix_set(A_table,i,z,g_table[i][z]);
        }
    }
    
    // merge b table
    for(int b_row_index=0; b_row_index < numof_circuit_nodes;b_row_index++){
        for(int b_column_index=0,a_column_index=numof_circuit_nodes; b_column_index < numof_indie_voltage_sources; b_column_index++,a_column_index++){
            gsl_matrix_set(A_table,b_row_index,a_column_index,(double)b_table[b_row_index][b_column_index]);
        }
    }
    
    // merge c table
    for(int c_row_index=0,a_row_index=numof_circuit_nodes;c_row_index<numof_indie_voltage_sources;c_row_index++,a_row_index++){
        for(int c_column_index=0; c_column_index < numof_circuit_nodes; c_column_index++){
            gsl_matrix_set(A_table,a_row_index,c_column_index,(double)c_table[c_row_index][c_column_index]);
        }
    }
    
    // merge d table
    for(int d_row_index=0,a_row_index=numof_circuit_nodes;d_row_index<numof_indie_voltage_sources;d_row_index++,a_row_index++){
        for(int d_column_index=0,a_column_index=numof_circuit_nodes;d_column_index<numof_indie_voltage_sources;d_column_index++,a_column_index++){
            gsl_matrix_set(A_table,a_row_index,a_column_index,(double)d_table[d_row_index][d_column_index]);
        }
    }
    
}

// calculates the z table holding the know quantities of the parsed circuit
void create__z_vector(int numof_circuit_nodes, int numof_indie_voltage_sources, int numof_current_sources){
    
    element *elem = NULL;
    
    for(int i=0;i<numof_circuit_nodes+numof_indie_voltage_sources;i++){
        gsl_vector_set(z, i, 0);
    }
    
    // insert current sources into z table
    LL_FOREACH(head, elem){
        
        if(elem->element_type == elementTypeCurrentSource){
            
            int positive_node = id_for_node_in_hash(elem->first_terminal);
            int negative_node = id_for_node_in_hash(elem->second_terminal);
            
            if((positive_node != GROUND) && (negative_node != GROUND)){
                //z_table[positive_node-1] += -elem->value;
                //z_table[negative_node-1] += elem->value;
                gsl_vector_set(z, positive_node-1, gsl_vector_get(z, positive_node-1) + (elem->value*(-1)));
                gsl_vector_set(z, negative_node-1, gsl_vector_get(z, negative_node-1) + elem->value);
            }
            else if((positive_node != GROUND) && (negative_node == GROUND)){
                //z_table[positive_node-1] += -elem->value;
                gsl_vector_set(z, positive_node-1, gsl_vector_get(z, positive_node-1) + (elem->value*(-1)));
            }
            else if((positive_node == GROUND) && (negative_node != GROUND)){
                //z_table[negative_node-1] += elem->value;
                gsl_vector_set(z, negative_node-1, gsl_vector_get(z, negative_node-1) + elem->value);
            } 
        }
        
    }
    int index=0;
    
    // insert voltage sources into z table
    LL_FOREACH(head, elem){
        
        if(elem->element_type == elementTypeVoltageSource){
            //z_table[numof_circuit_nodes+index] += elem->value;
            gsl_vector_set(z,numof_circuit_nodes+index,gsl_vector_get(z,numof_circuit_nodes+index) + elem->value);
            index++;
        }
        
        
    }
    
}

void prepare_MNA_matrices(){
    // Initialize G Table
    g_table = (double **)malloc(numof_circuit_nodes * sizeof(double *));
    for(int i=0; i < numof_circuit_nodes; i++){
        g_table[i] = (double *)malloc(numof_circuit_nodes * sizeof(double));
    }
    
    calculate__g_table(head,numof_circuit_nodes);
    
    // Initialize B table
    b_table = (int **)malloc(numof_circuit_nodes * sizeof(int *));
    for(int i=0; i < numof_circuit_nodes; i++){
        b_table[i] = (int *)malloc(numof_indie_voltage_sources * sizeof(int));
    }
    
    calculate__b_table(numof_circuit_nodes, numof_indie_voltage_sources);
    
    // Initialize C table
    c_table = (int **)malloc(numof_indie_voltage_sources * sizeof(int *));
    for(int i=0; i < numof_indie_voltage_sources; i++){
        c_table[i] = (int *)malloc(numof_circuit_nodes * sizeof(int));
        
    }
    
    calculate__c_table(numof_circuit_nodes, numof_indie_voltage_sources);
    
    // Initialize D table
    d_table = (int **)malloc(numof_indie_voltage_sources * sizeof(int *));
    for(int i=0; i < numof_indie_voltage_sources; i++){
        d_table[i] = (int *)malloc(numof_indie_voltage_sources * sizeof(int));
        
    }
    calculate__d_table(numof_indie_voltage_sources);
    
    A_table = gsl_matrix_alloc(numof_circuit_nodes+numof_indie_voltage_sources, numof_circuit_nodes+numof_indie_voltage_sources);
    
    // initialize Z table
    z = gsl_vector_alloc(numof_circuit_nodes + numof_indie_voltage_sources);
    
    calculate__A_matrix(numof_circuit_nodes, numof_indie_voltage_sources);
    create__z_vector(numof_circuit_nodes, numof_indie_voltage_sources,numof_current_sources);
    
    if(verbose)print__g_table(numof_circuit_nodes);
    if(verbose)print__b_table(numof_circuit_nodes,numof_indie_voltage_sources);
    if(verbose)print__c_table(numof_indie_voltage_sources,numof_circuit_nodes);
    if(verbose)print__A_matrix(numof_circuit_nodes+numof_indie_voltage_sources, numof_indie_voltage_sources+numof_circuit_nodes);
    if(verbose)print__Z_vector(numof_circuit_nodes, numof_indie_voltage_sources);
    
    free(g_table);
    free(b_table);
    free(c_table);
    free(d_table);

}

