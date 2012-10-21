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

    /*if(DEBUG){
        struct node_data *s;
        printf("\n");
        for(s=nodes; s != NULL; s=(struct node_data*)(s->hh.next)) {
            printf("[nodes_hashtable] key %s has id %d\n", s->node_name,s->node_num);
        }

    }*/
    
    
    // return the number of nodes with out the ground (-1)
    return HASH_COUNT(nodes_hashtable) - 1;
}

// returns the number of voltage sources found in the parsed circuit
int numOfIndependentVoltageSources(element *parsed_elements){
    
    element *elem = NULL;
    int numOfVoltageSources = 0;
    
    // start browsing the list
    LL_FOREACH(parsed_elements, elem){
        if(elem->element_type == elementTypeVoltageSource) numOfVoltageSources++;
    }
    
    // return the number of nodes with out the ground (-1)
    return numOfVoltageSources;

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
                    
                    if(DEBUG) printf("[g_table] 1/%s=%f (%s,%s) added to (%d,%d)\n",elem->element_name,elem->value,elem->first_terminal,elem->second_terminal,i,i);
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
              
                if(DEBUG) printf("[g_table] -1/%s=%f (%s,%s) added to (%d,%d) and (%d,%d)\n",elem->element_name,elem->value,elem->first_terminal,elem->second_terminal,first_nodeid,second_nodeid,second_nodeid,first_nodeid);
                
                g_table[first_nodeid][second_nodeid] += -1/elem->value;
                g_table[second_nodeid][first_nodeid] += -1/elem->value;

            }
        }
    }
}

// calculates the B table
void calculate__b_table(element *parsed_elements, int num_of_nodes, int num_of_voltage_sources){
    
    // initialize table
    for(int i=0; i < num_of_nodes; i++){
        for(int z=0; z < num_of_nodes; z++){
            b_table[i][z] = 0;
        }
    }

}