//
//  types.h
//
//  Created by Athanasios Theodoridis (461) and Aris Eleftheriadis (460) on 4/10/12.
//
//

// various definitions
#define MAX_TOKENS_IN_LINE 15
#define MAX_TOKEN_LEN      20

#define GROUND 0

// data structures and type definitions
typedef struct element {
    
	char *element_name;
	int element_type;
    char *first_terminal;
	char *second_terminal;
    
    char *drain_terminal;
    char *gate_terminal;
    char *source_terminal;
    char *bulk_terminal;
	
    char *model_name;
    
    double gate_length;
    double gate_width;
	
    char *area;
	
    double value;
    
    // transient spec
    int transient_spec_type;
    double *transient_values;
    
	struct element *next;
    
} element;

typedef struct netlist_option {
    
    int option_type;
    
    char *node_name;
    char *other_option_field;
    
    float start_value;
    float end_value;
    float step;
    
    struct netlist_option *next;

} netlist_option;

// node data in the hashtable
struct node_data {
    
    char node_name[20];  // name of the node in case it's alphanumeric
    int node_num;
    UT_hash_handle hh;   // makes this structure hashable
    
};

// parameteres passed to parsing threads
struct thread_data {

    int  thread_id;
    int  start_line;
    int  end_line;
    

};

// simple enum for holding the type of circuit elements
// we might parse
typedef enum {
    
    elementTypeVoltageSource = 0,
    elementTypeCurrentSource,
    elementTypeResistance,
    elementTypeCapacitor,
    elementTypeInductor,
    elementTypeDiode,
    elementTypeMOSTransistor,
    elementTypeBJTTransistor,
    elementTypeOption
    
} element_type;

// enum for transient specs type
typedef enum {

    transientTypeExp = 0,
    transientTypeSin,
    transientTypePulse,
    transientTypePwl

} transient_spec_type;

// enum indexes for EXP transient values
typedef enum {
    exp_i1 = 0,
    exp_i2,
    exp_td1,
    exp_tc1,
    exp_td2,
    exp_tc2

} transient_exp_values;

// enum indexes for SIN transient values
typedef enum {
    sin_i1 = 0,
    sin_ia,
    sin_fr,
    sin_td,
    sin_df,
    sin_ph
} transient_sin_values;

// enum indexes for PULSE transient values
typedef enum {
    pulse_i1 = 0,
    pulse_i2,
    pulse_td,
    pulse_tr,
    pulse_tf,
    pulse_pw,
    pulse_per
} transient_pulse_values;

typedef enum {

    optionTypeDCSweep = 0,
    optionTypePlot,
    optionTypeOther

} option_types;

char *netlist_filename;
char *dcoutput_filename;

// rw lock for the list holdign th elements
pthread_rwlock_t elements_list_lock;
pthread_rwlock_t options_list_lock;

// simple BOOL typedef
typedef enum { false, true } bool;

bool didParseGroundNode = false;

// whether to print debugging statements
bool db_info = true;

bool verbose = false;
bool should_solve = false;

int nodeid_index = 0;

bool spd         = false;
bool sparse      = false;
bool iter        = false;
bool conjgrad    = false;
bool bi_conjgrad = false;
float itol       = 1e-3;

// DC Sweep options
bool dc_sweep = false;
bool found_plotting_node = false;


// circuit properties
struct node_data *nodes_hashtable;

int numof_circuit_nodes         = 0;
int numof_indie_voltage_sources = 0;
int numof_current_sources       = 0;

// MNA Matrices
double **g_table = NULL;
int    **b_table = NULL;
int    **c_table = NULL;
int    **d_table = NULL;

gsl_matrix *A_table = NULL;
gsl_permutation *perm_matrix = NULL;
gsl_vector *z  = NULL;

// Sparse Matrices
cs  *A_table_sparse, *C_sparse_compressed;
css *S;
csn *N;
int nonzeroes = 0;

// list that holds the parsed elements, should always be initialized to NULL
element *head = NULL;
netlist_option *netlist_options = NULL;
