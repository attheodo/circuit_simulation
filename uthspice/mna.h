//
//  mna.h
//
//  Created by Athanasios Theodoridis (461) and Aris Eleftheriadis (460) on 4/10/12.
//
//

int numberOfNodes(element *parsed_elements){

    element *elem;
    UT_array *unique_nodes;
    char **index;
    int numOfNodes = 0;
    
    // create a new array
    utarray_new(unique_nodes,&ut_str_icd);

    // start browsing the list
    LL_FOREACH(parsed_elements, elem){
       
        bool found_positive_node = false;
        bool found_negative_node = false;
        
        // set the flags if we found duplicated of the nodes so we don't add them in the array again
        while ( (index = (char**)utarray_next(unique_nodes,index))) {
           
            if(strcmp(*index, elem->first_terminal) == 0){
                found_positive_node = true;
            }
            
            if(strcmp(*index, elem->second_terminal) == 0){
                found_negative_node = true;
            }
        }
    // if we didn't find the nodes, add them
    if(!found_positive_node) utarray_push_back(unique_nodes, &elem->first_terminal);
    if(!found_negative_node) utarray_push_back(unique_nodes, &elem->second_terminal);
    
    }

    // go throught the array and count the number of nodes in it
    while ( (index = (char**)utarray_next(unique_nodes,index))) {
        numOfNodes++;
    }

    // return the number of nodes with out the ground (-1)
    return numOfNodes - 1;
}