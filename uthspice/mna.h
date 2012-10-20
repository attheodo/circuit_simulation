//
//  mna.h
//
//  Created by Athanasios Theodoridis (461) and Aris Eleftheriadis (460) on 4/10/12.
//
//

// Returns the number of nodes without the ground. It also populated 'unique_circuit_nodes' array
// with all the nodes of the circuit.
int numberOfNodes(element *parsed_elements,struct node_data *nodes){

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

void calculate_g_table(double **g_table, element *parsed_elements, struct node_data *nodes_hash, int num_of_nodes, int num_of_voltage_sources){
    
    element *elem = NULL;
    
    // initialize table
    for(int i=0; i < num_of_nodes; i++){
        for(int z=0; z < num_of_nodes; z++){
            g_table[i][z] = 0.0;
        }
    }
    
    // first calculate the diagonal table
    for(int i=0; i < num_of_nodes;i++){
        
        LL_FOREACH(parsed_elements, elem){
            
            if(id_for_node_in_hash(elem->first_terminal, nodes_hash) == i){
                printf("KAVLA\n");
            }
            
        }
        
    }
}