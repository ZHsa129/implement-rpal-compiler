#include <queue>
#include <stack>
#include <string>
#include <iostream>
#include <fstream>
#include "token.h"
#include <unordered_map>
#include <unordered_set>
using namespace std;

class ast_node;

class parser{
private:
    stack<ast_node*> ast_stk;
    token* next_token;
    ifstream rpal_prog;
    
    char scan_char; // used to store the last scan char
    // helper function
    void read(string nt_value); // verifies the value of upcoming token and consumes it
    void read(type nt_type); // push a new ast-node into the stack, updates token
    bool check_reserved(string word);
    token* scan(); // scan the input, return token
    void build_tree(string node_lable, int num_child);
    void print_ast_node(ast_node* node, int level); //helper for print_ast
    void rec_standard_tree(ast_node*);
    
public:
    //constructor
    parser(string prog_name); //prog_name: the input program file name
    ~parser();
    // getter for ast_node* head
    ast_node* get_asthead(){
        return ast_stk.top();
    }
    // print ast
    void print_ast();
    // process the rpal and print ast tree
    void proc(string opt);
    // standardize the ast_tree
    void standard_tree();
    // all the procedures defined in RPAL grammar
    void E();
    void Ew();
    void T();
    void Ta();
    void Tc();
    void B();
    void Bt();
    void Bs();
    void Bp();
    void A();
    void At();
    void Af();
    void Ap();
    void R();
    void Rn();
    void D();
    void Da();
    void Dr();
    void Db();
    void Vb();
    void Vl();
};

class ast_node{
private:
    string ast_label;
public:
    ast_node* l_child; // the leftmost child of the node
    ast_node* r_sibling; // the right neighbor of the node (same parent node)
    //construct
    ast_node(){
        l_child = NULL;
        r_sibling = NULL;
    }
    ast_node(string label){
        l_child = NULL;
        r_sibling = NULL;
        ast_label = label;
    }
    void setlabel(string label){
        ast_label = label;
    }
    string getlabel(){
        return ast_label;
    }
    string getid();
};

// CES machine: Control, Stack, Environment, Machine
enum exe_node_type {EXE_INT, EXE_STR, EXE_NIL, EXE_TRUE, EXE_FALSE, EXE_DUMMY, EXE_ID, EXE_PRIM_FUNC, EXE_LAMBDA, EXE_ENV, EXE_TUPLE, EXE_YSTAR, EXE_ITA};
enum ctrl_node_type {CTRL_INT, CTRL_STR, CTRL_NIL, CTRL_TRUE, CTRL_FALSE, CTRL_DUMMY, CTRL_ID, CTRL_LAMBDA, CTRL_ENV, CTRL_TAU, CTRL_YSTAR, CTRL_GAMMA, CTRL_DELTA, CTRL_BETA, CTRL_BINOP, CTRL_UOP};
// CTRL_TAU => EXE_TUPLE
// RPAL built-in func/operator:
// primitive function: Print, Conc, Order, Istuple, Isinteger, Istruthvalue, Isstring, Isfunction, Stem, Stern, ItoS
//
// binary operator: boolean: or, &, gr, ge, ls, le, eq, ne;
//                  arithmetric: +, -, *, /, **
//                                 aug
// unary operator:  boolean: not, neg
class env;
class ctrl_node{
    int childnum; // for tau control node to store number of children
    string label; // the corresponding st_node for this control node
    ctrl_node_type type;
//    env* linked_env;
    
public:
    ast_node* pt_stnode; // the corresponding ast node in st tree
    ctrl_node* next; //next ctrl_node in the same detla
    ctrl_node* next_delta; // next delta for a new lambda func
    ast_node* bind_var; // the bound ast node (variable name) to lambda func
//    void set_env(env* l){
//        linked_env = l;
//    }
    void set_type(ctrl_node_type t){
        type = t;
    }
    ctrl_node_type get_type(){
        return type;
    }
//    void set_label(ast_node* stnode){
//        label = stnode->getlabel();
//    }
    void set_label(string l){
        label = l;
    }
    string get_label(){
        return label;
    }
    void set_stnode(ast_node* stnode){
        pt_stnode = stnode;
        label = stnode->getlabel();
    }
    void set_childnum(int n){
        childnum = n;
    }
    int get_childnum(){
        return childnum;
    }
    ctrl_node(ast_node* stnode){
        pt_stnode = stnode;
        label = stnode->getid();
        next = NULL;
        next_delta = NULL;
        bind_var = NULL;
        childnum = 0;
    }
    ctrl_node(ctrl_node_type t, string l): type(t), label(l){
        pt_stnode = NULL;
        next = NULL;
        next_delta = NULL;
        bind_var = NULL;
        childnum = 0;
    }
    ctrl_node(){
        label = "";
        pt_stnode = NULL;
        next = NULL;
        next_delta = NULL;
        bind_var = NULL;
        childnum = 0;
    }
};
class exe_node{
    exe_node_type type; // node type
    string val;
public:
    // ast_node* pt_stnode;
    ctrl_node* pt_ctrlnode;
    env* linked_env;  // only need by EXE_LAMBDA and EXE_ITA
    // ast_node* bind_var;
    queue<exe_node*> tuple_que; // store the tuple elements for tuple node
    exe_node(){
    }
    exe_node(exe_node_type t, string v): type(t), val(v){
//        pt_stnode = NULL;
    }
    exe_node(ctrl_node* c_node){
        val = c_node->get_label();
//        pt_stnode = c_node->pt_stnode;
        pt_ctrlnode = c_node;
//        bind_var = c_node->bind_var;
    }
    void set_val(string v){
        val = v;
    }
    void set_type(exe_node_type t){
        type = t;
    }
    string get_val(){
        return val;
    }
    exe_node_type get_type(){
        return type;
    }
    ast_node* get_bindvar(){
        if(pt_ctrlnode){
            return pt_ctrlnode->bind_var;
        }
        else
            return NULL;
    }
    ctrl_node* get_delta_head(){
        if(pt_ctrlnode){
            return pt_ctrlnode->next_delta;
        }
        else
            return NULL;
    }
    exe_node(exe_node& l){
        type = l.type;
        val = l.val;
        pt_ctrlnode = l.pt_ctrlnode;
        linked_env = l.linked_env;
        tuple_que = l.tuple_que;
        
    }
};
// environment
class env{
private:
    int env_indx;
public:
    unordered_map<string, exe_node*> bind_var; // bound variable in this environment

    exe_node* lookup(string key);
    env* parent;
    void insert(string key, exe_node* val);
    env(){
        env_indx = 0;
        parent = NULL;
    }
    env(int indx){
        env_indx = indx;
    }
//    ~env();
};


class CSEM{
private:
    stack<ctrl_node *> ctrl_stk; // control stack to store node
    stack<exe_node *> exe_stk;
    stack<env *> env_stk; // store the env var for execution
    int env_indx;
    parser parse_tree;
    ctrl_node* ctrl_head;
public:
    env* cur_env;
    static unordered_set<string> prim_func;
    static unordered_set<string> binary_op_set;
    static unordered_set<string> unary_op_set;
    void init_execute();
    void rec_generate_ctrl_struct(ast_node* st_node, ctrl_node* &cur_ctrl_node);
    void generate_ctrl_struct();
    void execute(); // execute the rpal program
    void rpal_print(exe_node*);
    void load_ctrl_delta(ctrl_node* delta_node);
    CSEM(string program_name):parse_tree(program_name){}
    // ~CSEM(){}
    void proc(string args);
};
