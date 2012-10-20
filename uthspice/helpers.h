//
//  helpers.h
//
//  Created by Athanasios Theodoridis (461) and Aris Eleftheriadis (460) on 4/10/12.
//
//

#include "types.h"
#include "utlist.h"


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
        
    }
    
    return -1;
}


//prints the list of the nodes parsed
void print_elements_parsed(element *head) {
    
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

