//
//  helpers.h
//
//  Created by Athanasios Theodoridis (461) and Aris Eleftheriadis (460) on 4/10/12.
//
//

// takes an element_type (int) and returns
// it's type as a human readable string
char *name_of_element_for_type(element_type i){
    switch(i){
        case elementTypeVoltageSource:
            return "VoltageSource";
        case elementTypeCurrentSource:
            return "CurrentSource";
        case elementTypeResistance:
            return "Resistance";
        case elementTypeCapacitor:
            return "Capacitor";
        case elementTypeInductor:
            return "Inductor";
        case elementTypeDiode:
            return "Diode";
        case elementTypeMOSTransistor:
            return "MOSTransistor";
        case elementTypeBJTTransistor:
            return "BJTTransistor";
        case elementTypeOption:
            return "Option:";
        default:
            return "Something went terribly wrong\n";
    }
    
    return NULL;
}

element_type element_type_for_string(char element_as_string){
    
    if(element_as_string == 'V'){
        
        return elementTypeVoltageSource;
        
    } else if (element_as_string == 'I'){
        
        return elementTypeCurrentSource;
        
    } else if (element_as_string == 'R'){
        
        return elementTypeResistance;
        
    } else if (element_as_string == 'C'){
        
        return elementTypeCapacitor;
        
    } else if (element_as_string == 'L'){
        
        return elementTypeInductor;
        
    } else if (element_as_string == 'D') {
        
        return elementTypeDiode;
        
    } else if (element_as_string == 'M') {
        
        return elementTypeMOSTransistor;
        
    } else if (element_as_string == 'Q') {
        
        return elementTypeBJTTransistor;
        
    } else if(element_as_string == '.') {
        
        return elementTypeOption;
    }
    
    return -1;
}

// takes the parsed line and returns an array of tokens split by " "
char **tokenize(char *line)
{
    char **tokens;
    char *token;
    int i=0;
    
    tokens = (char **)malloc(MAX_TOKENS_IN_LINE*sizeof(char*));
    
    for(int y=0;y<MAX_TOKENS_IN_LINE;y++){
        tokens[y] = (char *)malloc(MAX_TOKEN_LEN*sizeof(char));
    }
    
    token = strtok(line," ");
    
    while (token != NULL) {
        
        // potential buffer overflow here but who cares ;)
        strcpy((char *)tokens[i],token);
        token = strtok(NULL, " ");
        i++;
        
    }
    
    return tokens;
}


//prints the list of the nodes parsed
void print_elements_parsed() {
    
    element *elem;
    
    printf("\n[-] Parsed elements:\n");
    
    printf("\n         Type    Name    Terminal 1    Terminal 2      Value    Drain Terminal    Gate Terminal    Source Terminal    Bulk Terminal    Gate Length    Gate Width\n");
    
    LL_FOREACH(head, elem){
        if(elem->element_type != elementTypeBJTTransistor && elem->element_type != elementTypeMOSTransistor){
            printf("%13s %7s %8s %13s %15.1e          -                  -                 -                  -                -            - \n",name_of_element_for_type(elem->element_type),elem->element_name,elem->first_terminal,elem->second_terminal,elem->value);
            
        } else {
            printf("%s %7s        -             -            - %13s %18s %17s %18s %18.1e %12.1e\n",name_of_element_for_type(elem->element_type),elem->element_name,elem->drain_terminal,elem->gate_terminal,elem->source_terminal,elem->bulk_terminal,elem->gate_length,elem->gate_width);
        }
        
    }
    
}

int generate_uniqueid(char *string){
    
    char *endptr;
    int value = (int)strtol(string, &endptr, 10);
    
    // node is alphanumeric
    if (endptr == string) {
        
        // TODO: this should first go through all the ids from 1 to num_of_nodes and assign a non-used one
        // instead of using this stupid hack that created large integers
        return string[0]+string[1]+string[3];
    
    // return the proper int value form the numeric string
    } else {
    
        return value;
    }
    
    return -1;
}

int id_for_node_in_hash(char *nodename){
    
    struct node_data *node_item = NULL;
    
    HASH_FIND_STR(nodes_hashtable, nodename, node_item);
   
    if(node_item){
        return node_item->node_num;
    }
    
    return -1;
}

void parse_netlist_option(char **option_args){
    
    netlist_option *option = (struct netlist_option *) malloc( sizeof(struct netlist_option));

    // DC sweep option parsing
    if(strcmp(option_args[0],".DC") == 0){
        
        dc_sweep = true;
        option->option_type = optionTypeDCSweep;
        option->node_name = option_args[1];
        option->start_value = atof(option_args[2]);
        option->end_value = atof(option_args[3]);
        option->step = atof(option_args[4]);
        printf("[-] DC sweep option set \n\tNode: %s\tstart_value: %f\tend_value: %f\tstep: %f\n",option->node_name,option->start_value,option->end_value,option->step);
        
        // add it to the list holding options
        LL_APPEND(netlist_options, option);
        
    }
    // PLOT/PRINT DC sweep option parsing
    else if( (strcmp(option_args[0],".PLOT") == 0) || (strcmp(option_args[0],".PRINT") == 0) ){
        
        dc_sweep_plot = true;
        
        option->option_type = optionTypePlot;
        char *tmp = strndup(option_args[1]+2, strlen(option_args[1])-3);
        option->node_name = tmp;
        
        printf("\n[-] Plotting option set\n\tVoltage results for node: %s\n",option->node_name);

        // add it to the options list
        LL_APPEND(netlist_options, option);
               
    }
    
    free(option);
}

// prints the g_table in a human readable format
void print__g_table(int size){
    
    printf("\nG = [\n");
    // print column indexes
    for(int i=0;i<size;i++) printf("%15d",i);
    printf("\n");
   
    for(int i=0; i < size; i++){
        for(int z=0; z < size; z++){
            // print row indexes
            if(z == 0) printf("\t%d:",i);
           
            printf("%15.10f",g_table[i][z]);
            if(z == size-1) printf("\n");
        }
    }
    
    printf("]\n");
}

// prints the b_table in a human readable format
void print__b_table(int n,int m){
    
    printf("\nB = [\n");
    // print column indexes
    printf("\t");
    for(int i=0;i<m;i++) printf("\t%2d",i);
    printf("\n");
    
    for(int i=0; i < n; i++){
        for(int z=0; z < m; z++){
            // print row indexes
            if(z == 0) printf("\t%d:",i);
            
            printf("\t%2d",b_table[i][z]);
            
            if(z == m-1) printf("\n");
        }
    }
    
    printf("]\n");
}

// prints the b_table in a human readable format
void print__c_table(int n,int m){
    
    printf("\nC = [\n");
    // print column indexes
    printf("\t");
    for(int i=0;i<m;i++) printf("\t%2d",i);
    printf("\n");
    
    for(int i=0; i < n; i++){
        for(int z=0; z < m; z++){
            // print row indexes
            if(z == 0) printf("\t%d:",i);
            
            printf("\t%2d",c_table[i][z]);
            
            if(z == m-1) printf("\n");
        }
    }
    
    printf("]\n");
}

void print__A_table(int n,int m){
    printf("\nA = [\n");
    // print column indexes
    printf("\t");
    for(int i=0;i<m;i++) printf("%14d",i);
    printf("\n");
    
    for(int i=0; i < n; i++){
        for(int z=0; z < m; z++){
            // print row indexes
            if(z == 0) printf("\t%2d:",i);
            
            printf("%14f",A_table[i][z]);
            
            if(z == m-1) printf("\n");
        }
    }
    
    printf("]\n");

}

void print__Z_table(int numof_circuit_nodes,int numof_voltage_sources){
    
    printf("\nz = [\n");
    for(int i=0;i< numof_circuit_nodes+numof_voltage_sources;i++){
        printf("\t%d: %.5f\n",i,z_table[i]);
    }
    printf("]");
}

void replace_L_C (){
  
    element *temp_element;
    element *new_element;
    
    LL_FOREACH(head,temp_element){
        
       if(temp_element->element_type == elementTypeInductor){
           
           if( (new_element = (struct element *) malloc( sizeof(struct element))) == NULL) exit(-1);
           
           new_element->element_type = element_type_for_string('V');
           new_element->element_name = (char *)malloc(strlen(temp_element->element_name)*sizeof(char));
           sprintf(new_element->element_name,"V%s",temp_element->element_name);
           
           new_element->value = 0.0;
           
           new_element->first_terminal = temp_element->first_terminal;
           new_element->second_terminal = temp_element->second_terminal;
            
           // add it to the list
            LL_APPEND(head,new_element);
           
           
            
        }
      
    }
    
    free(new_element);
    free(temp_element);
}
