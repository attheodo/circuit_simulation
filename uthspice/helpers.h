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

// takes the element type as a char and returns the enum'ed type of it
element_type element_type_for_string(char element_as_string){
    
    if(element_as_string == 'V' || element_as_string == 'v'){
        
        return elementTypeVoltageSource;
        
    } else if (element_as_string == 'I' || element_as_string == 'i'){
        
        return elementTypeCurrentSource;
        
    } else if (element_as_string == 'R' || element_as_string == 'r'){
        
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

#pragma mark - Parsing Related

// opens a netlist file and counts the number of lines in it
int numOfNetlistLines(char *filename){
    int ch = 0, i = 0;
	FILE *fd;
	if (!(fd = fopen(filename, "r")))
	{
		fputs("could not open file!\n", stderr);
		exit(-1);
	}
	while ((ch = fgetc(fd)) != EOF)
	{
		if (ch == '\n')
			++i;
	}
    fclose(fd);
    return i+1;
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

// takes the name of a node as a string and assigns a unique int id to id
int generate_uniqueid(char *string){
    
    char *endptr;
    int value = (int)strtol(string, &endptr, 10);
    
    // node is alphanumeric
    if (endptr == string) {

        struct node_data *node_item = NULL;
        
        HASH_FIND_STR(nodes_hashtable,string,node_item);
        
        if(!node_item){
            nodeid_index++;
            return nodeid_index;
        }
    
    } else {
    
        return value;
    }
    
    return -1;
}

// takes the name of a node and returns the (int) id
// of it in the hashtable we have for all nodes
int id_for_node_in_hash(char *nodename){
    
    struct node_data *node_item = NULL;
    
    HASH_FIND_STR(nodes_hashtable, nodename, node_item);
   
    if(node_item){
        return node_item->node_num;
    }
    
    return -1;
}

// returns the position in z vector (know part of the linear equations) in
// which the requested voltage source resides
int z_index_position_for_voltagesource(char *vsource_name){
    
    int index = numof_circuit_nodes;
    struct element *el;
    
    LL_FOREACH(head, el){
        if(strcmp(el->element_name,vsource_name)==0){
            return index;
        }
        index++;
    }
    
    return -1;
}

// Reads an option line from the netlist (the ones starting with .) and
// saves it in a linked list for future reference
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
        printf("\n[-] DC sweep option set \n\tNode: %s\tstart_value: %f\tend_value: %f\tstep: %f\n",option->node_name,option->start_value,option->end_value,option->step);
        
      
        if (pthread_rwlock_wrlock(&options_list_lock) != 0) {
            fprintf(stderr,"[!] Error: options_list_lock: can't acquire write lock\n");
            exit(-1);
        }
        LL_APPEND(netlist_options, option);
        pthread_rwlock_unlock(&options_list_lock);
        
    }
    // PLOT/PRINT DC sweep option parsing
    else if( (strcmp(option_args[0],".PLOT") == 0) || (strcmp(option_args[0],".PRINT") == 0) ){
        
        found_plotting_node = true;
        
        option->option_type = optionTypePlot;
        char *tmp = strndup(option_args[1]+2, strlen(option_args[1])-3);
        option->node_name = tmp;
        
        printf("\n[-] Plotting option set\n\tVoltage results for node: %s\n",option->node_name);
        
        // add it to the options list
        if (pthread_rwlock_wrlock(&options_list_lock) != 0) {
            fprintf(stderr,"[!] Error: options_list_lock: can't acquire write lock\n");
            exit(-1);
        }
        LL_APPEND(netlist_options, option);
        pthread_rwlock_unlock(&options_list_lock);

        
    }
    // .OPTION SPD
    // PLOT/PRINT DC sweep option parsing
    else if(strcmp(option_args[0],".OPTIONS") == 0){
        
        spd = true;
        
        option->option_type = optionTypeOther;
        char *tmp = strdup(option_args[1]);
        option->other_option_field = tmp;
        
        printf("\n[-] Option SPD set. Solving with Cholesky.\n");
        
        // add it to the options list
        if (pthread_rwlock_wrlock(&options_list_lock) != 0) {
            fprintf(stderr,"[!] Error: options_list_lock: can't acquire write lock\n");
            exit(-1);
        }
        LL_APPEND(netlist_options, option);
        pthread_rwlock_unlock(&options_list_lock);
        
        
    }

}

// opens the input netlist file and parses the assigned lines set
// in thread_args into the global linked list (head)
void *parse_input_netlist(void *thread_args) {
    
    FILE *netlist_file = NULL;
    char *line = NULL;
    int current_line = 0;
    size_t len = 0;
    ssize_t line_length;
    element *parsed_element;
    
    struct thread_data *data = thread_args;
    
    int start_line = data->start_line-1;
    int end_line = data->end_line-1;
    
    if((netlist_file = fopen(netlist_filename, "r"))){
    
        // parse the file line by line
        while ((line_length = getline(&line, &len, netlist_file)) != -1) {
        
            if(current_line < start_line){
                current_line++;
                continue;
            }
            
            char **tokens = NULL;
            
            // if line is a comment or empty line, skip it
            if((strncmp(line, "*",1) == 0) || line_length <= 2) {
                current_line++;
                continue;
            }
            
            // remove newline character from the end
            line[strlen(line)-1] = 0;
            
            tokens = tokenize(line);
            
            // create a new list node to hold the element attributes
            if( (parsed_element = (struct element *) malloc( sizeof(struct element))) == NULL) { printf("[!] Encountered error while allocating memory for parsed element\n"); exit(-1); };
            
            parsed_element->element_name = tokens[0];
            
            // encountered unknown element
            if(element_type_for_string(tokens[0][0])  == -1){
                printf("[!] Encountered element of uknown type: %s (line: %d)\n",tokens[0],current_line);
            }
            // encountered option
            else if(element_type_for_string(tokens[0][0]) == elementTypeOption){
                parse_netlist_option(tokens);
            }
            
            // known netlist element
            else {
                parsed_element->element_type = element_type_for_string(tokens[0][0]);
                
                if(atoi(tokens[1]) == 0 || atoi(tokens[2]) == 0) didParseGroundNode = true;
                
                parsed_element->first_terminal = tokens[1];
                parsed_element->second_terminal = tokens[2];
                parsed_element->value = atof(tokens[3]);
                
                parsed_element->drain_terminal = tokens[1];
                parsed_element->gate_terminal = tokens[2];
                parsed_element->source_terminal = tokens[3];
                parsed_element->bulk_terminal = tokens[4];
                parsed_element->gate_length = atof(tokens[6]+2);
                parsed_element->gate_width = atof(tokens[7]+2);
                
                // add it to the list
                if (pthread_rwlock_wrlock(&elements_list_lock) != 0) {
                    fprintf(stderr,"[!] Error: elements_list_lock: can't acquire write lock\n");
                    exit(-1);
                }
                
                LL_APPEND(head,parsed_element);
                
                pthread_rwlock_unlock(&elements_list_lock);

                
            }
            
            if(current_line == end_line){
                break;
            }
            
            current_line++;

        
            
        }
        // close gracefully the file
         fclose(netlist_file);
     
    } else {
         // fopen error
         printf("\n[!] Error: %s\n\n",strerror(errno));
         exit(-1);
     }

    pthread_exit(NULL);
}


#pragma mark - Matrix pretty printing

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

void print__A_matrix(int n,int m){
   printf("\nA = [\n");
    // print column indexes
    printf("\t");
    for(int i=0;i<m;i++) printf("%14d",i);
    printf("\n");
    
    for(int i=0; i < n; i++){
        for(int z=0; z < m; z++){
            // print row indexes
            if(z == 0) printf("\t%2d:",i);
            
            printf("%14f",gsl_matrix_get(A_table,i,z));
            
            if(z == m-1) printf("\n");
        }
    }
    
    printf("]\n");
    
}

void print__Z_vector(int numof_circuit_nodes,int numof_voltage_sources){
    
    printf("\nz = [\n");
    for(int i=0;i< numof_circuit_nodes+numof_voltage_sources;i++){
        printf("\t%d: %.5f\n",i,gsl_vector_get(z, i));
    }
    printf("]");
}

// replaces inductors with voltage sources of zero value (for DC)
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
}
