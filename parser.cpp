#include "parser.h"
#include <algorithm>
#include <cmath>
#include <ctype.h>
#include <stdexcept>
#include <cstring>

using namespace std;

//#define DEBUG


string tostring(int val){
  return to_string(static_cast<long long> (val));
}
// parser constructor
parser::parser(string prog_name){
	rpal_prog.open(prog_name.c_str(), ifstream::in); // open the program file
    scan_char = ' '; // initialize the scan_char
}
parser::~parser(){
	rpal_prog.close();
}

bool parser:: check_reserved(string word){
	int num_rws = 20;
	for(int i = 0; i < num_rws; ++i){
		if(word == token::reserved_words[i]) return true;
	}	
	return false;
} 
// process the rpal program and print the ast
void parser::proc(string opt){
#ifdef DEBUG
	cout<<"in parser's proc( "<<opt<<" )"<<endl;
#endif
    if(opt == "-ast"){
		next_token = scan();
		E();
		print_ast();
        standard_tree();
    }
	else if(opt == ""){
		next_token = scan();
		E();
        standard_tree();
	}
	else{
		throw runtime_error("Error: only support no switch or -ast swicth now in proc()");
	}
}

// scan the input to read the next token
token* parser::scan(){
	char ip_c;
    token* tmp_pt;
    string tmp_str = "";
    ip_c = scan_char; // retrieved from last scan;
    
    while( ip_c == ' '|| ip_c == '\t'|| ip_c == '\n'){ // ignore space, tab, end of line
#ifdef DEBUG
		cout<<"**in scan()'read space loop, ip_c: "<<ip_c<<endl;
#endif
        ip_c = rpal_prog.get();
    }
#ifdef DEBUG
	cout<<"**exit scan()'read space loop, ip_c: "<<ip_c<<endl;
#endif


	while (ip_c == '/'){ // treat comment first
        ip_c = rpal_prog.get();
        if(ip_c == '/'){
            for(; ip_c != '\n'; ip_c = rpal_prog.get()) ;
            
            while( ip_c == ' '|| ip_c == '\t'|| ip_c == '\n'){  // after the commentline
				ip_c = rpal_prog.get();
            }
#ifdef DEBUG
	    cout<<"in scan(), scan comment"<<endl;
#endif
        }
        else if (find(token::op_symb, token::op_symb + 26, ip_c) != token::op_symb + 26){
			tmp_str += '/';
            tmp_str += ip_c;
            for(; find(token::op_symb, token::op_symb + 26, ip_c) != token::op_symb; ip_c = rpal_prog.get())
                tmp_str += ip_c;
            scan_char = ip_c;
            tmp_pt = new token(tmp_str, Operator);
            return tmp_pt;
        }
        else{
            tmp_str += '/';
            scan_char = ip_c;
            tmp_pt = new token(tmp_str, Operator);
#ifdef DEBUG
			cout<<"in scan(), new token: (value) "<<tmp_pt->getvalue()<<" (type) "<<tmp_pt->gettype()<<endl;
#endif
			return tmp_pt;
        }
    }
    
    if(isalpha(ip_c)){ // Identifier, begin with letter
        
        for(; isalnum(ip_c) || ip_c == '_'; ip_c = rpal_prog.get())
			tmp_str += ip_c;
        scan_char = ip_c; // store the unused char
		if(check_reserved(tmp_str))   // check if it's reserved word
			tmp_pt = new token(tmp_str, Reserved_Word);
		else
			tmp_pt = new token(tmp_str, Identifier);
    }
    else if(isdigit(ip_c)){ // integer, begin with digit
#ifdef DEBUG
		cout<<"in scan(), int branch"<<endl;
#endif
		for(; isdigit(ip_c); ip_c = rpal_prog.get()){
			tmp_str += ip_c;
		}
        scan_char = ip_c;
        tmp_pt = new token(tmp_str, Integer);
    }
    else if (find(token::op_symb, token::op_symb + 26, ip_c) != token::op_symb + 26){  // operator
#ifdef DEBUG
		cout<<"in scan(), operator branch"<<endl;
#endif
        for(; find(token::op_symb, token::op_symb + 26, ip_c) != token::op_symb + 26; ip_c = rpal_prog.get()){
            tmp_str += ip_c;
		}
		scan_char = ip_c;
#ifdef DEBUG
		cout<<"in scan(), operator branch end, scan_char: "<<scan_char<<endl; 
#endif
        tmp_pt = new token(tmp_str, Operator);
    }
    else if(ip_c == '\'' ){  // string, begin with '
		tmp_str += ip_c;
		ip_c = rpal_prog.get();
        while(ip_c != '\''){ // end with '
            if(ip_c == '\\'){ // ip_c == '\'
                ip_c = rpal_prog.get();
                if(ip_c == '\''){
                    tmp_str += '\'';
                }
                else{
                    tmp_str += '\\';
                }
            }
            else if (ip_c == EOF){
				throw runtime_error("Error: Need ' to enclose the string in scan()");
            }
            else{
                tmp_str += ip_c;
                ip_c = rpal_prog.get();
            }
        }
		tmp_str += ip_c;
        scan_char = ' ';  // exit loop, ip_c is '
        tmp_pt = new token(tmp_str, String);
    }
    else if(ip_c == '(' || ip_c == ')' || ip_c == ';' || ip_c == ','){ // punction
        tmp_str += ip_c;
	scan_char = rpal_prog.get(); // for next token
        tmp_pt =  new token(tmp_str,Punction);
    }
    else if (ip_c == EOF){
#ifdef DEBUG
		cout<<"in scan(), EOF branch"<<endl;
#endif
        tmp_pt = new token("", END_OF_FILE);
    }
#ifdef DEBUG
    cout<<"in scan(), new token: (value) "<<tmp_pt->getvalue()<<" (type) "<<tmp_pt->gettype()<<endl;
#endif
    return tmp_pt;
}

// verify and update the next_token
void parser:: read(string nt_value){  // not for <ID>, <INT>, <STR>
    if(next_token->getvalue() != nt_value)
		throw runtime_error(string("Error: Expected ") + next_token->getvalue() + string(", not ") + nt_value);
    delete next_token;
    next_token = scan(); // read next_token
}

void parser:: read(type nt_type){
    string tmp_label;
    if(nt_type == Identifier){
        tmp_label = "<ID:";
        tmp_label += (next_token->getvalue() + ">");
    }
    else if (nt_type == Integer){
        tmp_label = "<INT:";
        tmp_label += (next_token->getvalue() + ">");
    }
    else if (nt_type == String){
        tmp_label = "<STR:";
        tmp_label += (next_token->getvalue() + ">");
    }
    else
		throw runtime_error(string("Error: ID, or INT, or STR are expected, not ") + next_token->gettype() + "in read(type)");
    build_tree(tmp_label, 0);
    delete next_token;
#ifdef DEBUG
    cout<<"in read(nt_type), push finished"<<endl;
#endif
    next_token = scan(); // read next_token
}

void parser:: build_tree(string node_lable, int num_child){
    ast_node* tmp_head = new ast_node(node_lable);
    ast_node *tmp_cchild = NULL, *tmp_pchild = NULL;
#ifdef DEBUG
    cout<<"******in build_tree()**********"<<endl;
#endif
    while (num_child >= 1){
		if(ast_stk.top() == NULL)
			throw runtime_error("Error: Stack empty in build_tree()");
		tmp_cchild = ast_stk.top();
#ifdef DEBUG
		cout<<"*******pop node:"<<tmp_cchild->getlabel()<<endl;
#endif
        ast_stk.pop();
        tmp_cchild->r_sibling = tmp_pchild;
        tmp_pchild = tmp_cchild;
        --num_child;
    }
    tmp_head->l_child = tmp_cchild;
#ifdef DEBUG
    cout<<"********in build_tree(), push node( value: "<<node_lable<<" ) with "<<num_child<<" childs into stack"<<endl;
#endif
    ast_stk.push(tmp_head);
}

//*************Grammar Rules***********************
// E -> 'let' D 'in' E  => 'let'
//   -> 'fn' Vb+ '.' E  => 'lambda'
//   -> Ew;
void parser::E(){
    string tmp_str = next_token->getvalue();
#ifdef DEBUG
    cout<<"in E(), next_token: (value) "<<tmp_str<<" (type) "<<next_token->gettype()<<endl;
#endif
    if(tmp_str == "let"){
        read("let");
        D();
        read("in");
        E();
        build_tree("let", 2);
    }
    else if(tmp_str == "fn"){
        read("fn");
        Vb();
        int count = 1;
        while(next_token->getvalue() != "."){
            Vb();
            ++count;
        }
        read(".");
        E();
        build_tree("lambda", count + 1);
    }else
        Ew();
}

// Ew -> T 'where' Dr   => 'where'
//    -> T;
void parser::Ew(){
#ifdef DEBUG
	cout<<"in Ew(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    T();
    string tmp_str = next_token->getvalue();
    if(tmp_str == "where"){
        read("where");
        Dr();
#ifdef DEBUG
	cout<<"now build where"<<endl;
#endif	
        build_tree("where", 2);
    }
}

// T -> Ta (',' Ta)+    => tau
//   -> Ta;
void parser::T(){
#ifdef DEBUG
  cout<<"in T(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    Ta();
    int count = 1;
    while(next_token->getvalue()==","){
#ifdef DEBUG
      cout<<"in T()'s loop, next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
      cout<<"count: "<<count<<endl;
#endif
        read(",");
        Ta();
        ++count;

    }
    if(count > 1) build_tree("tau", count);
}

// Ta -> Ta 'aug' Tc    => 'aug'
//    -> Tc;
void parser::Ta(){
#ifdef DEBUG
  cout<<"in Ta(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    Tc();
    while(next_token->getvalue() == "aug"){
        read("aug");
        Tc();
        build_tree("aug", 2);
    }
}

// Tc -> B '->' Tc '|' Tc   => '->'
//    -> B;
void parser::Tc(){
#ifdef DEBUG
  cout<<"in Tc(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    B();
    if(next_token->getvalue() == "->"){
        read("->");
        Tc();
        read("|");
        Tc();
        build_tree("->", 3);
    }
}

// B -> B 'or' Bt   => 'or'
//   -> Bt;
void parser::B(){
#ifdef DEBUG
  cout<<"in B(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    Bt();
    while(next_token->getvalue() == "or"){
        read("or");
        Bt();
        build_tree("or", 2);
    }
}

// Bt -> Bt '&' Bs  => '&'
//    -> Bs;
void parser::Bt(){
#ifdef DEBUG
  cout<<"in Bt(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    Bs();
    while(next_token->getvalue() == "&"){
        read("&");
        Bs();
        build_tree("&", 2);
    }
}

// Bs -> 'not' Bp   => 'not'
//    -> Bp;
void parser::Bs(){
#ifdef DEBUG
  cout<<"in Bs(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    if(next_token->getvalue() == "not"){
        read("not");
        Bp();
        build_tree("not", 1);
    }
    else{
        Bp();
    }
}

void parser::Bp(){
#ifdef DEBUG
  cout<<"in Bp(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    A();
    string tmp_str = next_token->getvalue();
    if(tmp_str == "gr" || tmp_str == ">"){
        read(tmp_str);
        A();
        build_tree("gr", 2);
    }
    else if (tmp_str == "ge" || tmp_str == ">="){
        read(tmp_str);
        A();
        build_tree("ge", 2);
    }
    else if (tmp_str == "ls" || tmp_str == "<"){
        read(tmp_str);
        A();
        build_tree("ls", 2);
    }
    else if (tmp_str == "le" || tmp_str == "<="){
        read(tmp_str);
        A();
        build_tree("le", 2);
    }
    else if (tmp_str == "eq"){
        read(tmp_str);
        A();
        build_tree("eq", 2);
    }
    else if (tmp_str == "ne"){
        read(tmp_str);
        A();
        build_tree("ne", 2);
    }
}

void parser::A(){
#ifdef DEBUG
  cout<<"in A(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    string tmp_str = next_token->getvalue();
    if(tmp_str == "+" && next_token->gettype()== "Operator"){
        read("+");
        At();
    }
    else if (tmp_str == "-" && next_token->gettype()== "Operator"){
        read("-");
        At();
        build_tree("neg", 1);
    }
    else{
        At();
    }
    while(next_token->getvalue() == "+" || next_token->getvalue() == "-")
	{
        string tmp_str1 = next_token->getvalue();
        read(tmp_str1);
        At();
        build_tree(tmp_str1, 2);
    }
}

// At -> At '*' Af   => '*'
//    -> At '/' Af   => '/'
//    -> Af;  left recursion, left associative, left branching
void parser::At(){
#ifdef DEBUG
  cout<<"in At(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    Af();
    while( next_token->getvalue() == "*" || next_token->getvalue() == "/")
	{
		string tmp_str = next_token->getvalue();
        read(tmp_str);
		Af();
        build_tree(tmp_str, 2); // build "*" or "/" ast_node on top of two ast_nodes
    }
}

// Af -> Ap '**' Af   => '**'
//    -> Ap;  common prefix, right recursion, right associative, right branching
void parser::Af(){
#ifdef DEBUG
  cout<<"in Af(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    Ap();
    string tmp_str = next_token->getvalue();
#ifdef DEBUG
    cout<<"in Af(), check whether new ** node, nt_value: "<<tmp_str<<endl;
#endif
    if(tmp_str == "**" && next_token->gettype()== "Operator"){
        read(tmp_str);
        Af();
        build_tree(tmp_str, 2);
    }
}

// Ap -> Ap '@' '<IDENTIFIER>' R    => '@'
//    -> R; left recursion, left associative, left branching
void parser::Ap(){
#ifdef DEBUG
  cout<<"in Ap(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    R();
    while(next_token->getvalue() == "@"){
		string tmp_str = next_token->getvalue();
        read(tmp_str);
        if(! (next_token->gettype() == "Identifier"))
			throw runtime_error(string("Error: Expected type is Identifier, not ") + next_token->gettype());
        read(Identifier);
        R();
        build_tree(tmp_str, 3);
    }
}

// R -> R Rn    => 'gamma'
//   -> Rn;  // left recursion
void parser::R(){
#ifdef DEBUG
  cout<<"in R(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    Rn();
    while(next_token->gettype() == "Identifier" || next_token->gettype() == "Integer" || next_token->gettype() == "String" || next_token->getvalue() == "true" || next_token->getvalue() == "false" || next_token->getvalue() == "nil" || next_token->getvalue() == "(" || next_token->getvalue() == "dummy"){
        Rn();
        build_tree("gamma", 2);
    }
}
//
void parser::Rn(){
#ifdef DEBUG
  cout<<"in Rn(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    if(next_token->gettype() == "Identifier")
        read(Identifier);
    else if(next_token->gettype() == "Integer")
        read(Integer);
    else if (next_token->gettype() == "String")
        read(String);
    else if (next_token->getvalue() == "true"){
        read("true");
        build_tree("<true>", 0);
    }
    else if (next_token->getvalue() == "false"){
        read("false");
        build_tree("<false>", 0);
    }
    else if (next_token->getvalue() == "nil"){
        read("nil");
        build_tree("<nil>", 0);
    }
    else if (next_token->getvalue() == "("){
#ifdef DEBUG
      cout<<"in Rn(), Rn -> '(' E ')'"<<endl;
#endif
        read("(");
        E();
        read(")");
    }
    else if (next_token->getvalue() == "dummy"){
        read("dummy");
        build_tree("<dummy>", 0);
    }
    else
      throw runtime_error(string("Error: ") + next_token->getvalue() + string(" doesn't match Rn()"));
}

// D -> Da 'within' D   => 'within'
//   -> Da; common prefix, right recursion
void parser::D(){
#ifdef DEBUG
  cout<<"in D(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    Da();
    if(next_token->getvalue() == "within"){
        read("within");
        D();
        build_tree("within", 2);
    }
}

// Da -> Dr ( 'and'  Dr)+   => 'and'
//    -> Dr; common prefix
void parser::Da(){
#ifdef DEBUG
  cout<<"in Da(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    Dr();
    int count = 1;
    while(next_token->getvalue() == "and"){
        read("and");
        Dr();
        ++count;
    }
    if(count > 1) build_tree("and", count);
}

// Dr -> 'rec' Db   => 'rec'
//    -> Db;
void parser::Dr(){
#ifdef DEBUG
  cout<<"in Dr(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    if(next_token->getvalue() == "rec"){
        read("rec");
        Db();
        build_tree("rec", 1);
    }
    else
        Db();
}

// Db -> Vl '=' E   => '='
//    -> '<IDENTIFIER>' Vb+ '=' E   => 'fcn_form'
//    -> '(' D ')';
void parser::Db(){
#ifdef DEBUG
  cout<<"in Db(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    if(next_token->gettype() == "Identifier"){
        read(Identifier);
        if(next_token->getvalue() == "," || next_token->getvalue() == "="){  // rule: Db -> Vl '=' E  => '='
            Vl();
            read("=");
            E();
            build_tree("=", 2);
        }
        else{
            Vb();
            int count = 1;
            while(next_token->getvalue() != "="){
#ifdef DEBUG
	      cout<<"in Db()'s loop, next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
	      cout<<"count: "<<count<<endl;
#endif
                Vb();
                ++count;
            }
            read("=");
            E();
            build_tree("function_form", count + 2);
        }
    }
    else if (next_token->getvalue() == "("){
        read("(");
        D();
        read(")");
    }
    else
      throw runtime_error(string("Error: ") + next_token->getvalue() + string(" doesn't match Db()"));
}

// Vb -> '<IDENTIFIER>'
//    -> '(' Vl ')'
//    -> '(' ')'    => '()'
void parser::Vb(){
#ifdef DEBUG
  cout<<"in Vb(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
  if(next_token->gettype() == "Identifier"){ // Vb -> '<Id>'
#ifdef DEBUG
    cout<<"in Vb() if , next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
        read(Identifier);
  }
    else if (next_token->getvalue() == "("){
        read("(");
        if(next_token->gettype() == "Identifier"){ // Vb -> '(' Vl ')'
            read(Identifier);
            Vl();
            read(")");
        }
        else if (next_token->getvalue() == ")"){
            read(")");
            build_tree("()", 0);
        }
        else
	  throw runtime_error(string("Error: ") + next_token->getvalue() + string(" doesn't match Vb()"));
    }

}

// Vl -> '<Id>' list ','    => ','?
void parser::Vl(){
#ifdef DEBUG
  cout<<"in Vl(), next_token: (value) "<<next_token->getvalue()<<" (type) "<<next_token->gettype()<<endl;
#endif
    //already read one <Id> in Vb or Db
    int count = 1;
    while(next_token->getvalue() == ","){
        read(",");
        if(next_token->gettype() != "Identifier")
			throw runtime_error(string("Error: <Identifier> needed in Vl(), not ") + next_token->gettype() + " " + next_token->getvalue());
        read(Identifier);
        ++count;
    }
    if(count > 1) build_tree(",", count);
}

// print ast_node, helper function for print_ast
void parser:: print_ast_node(ast_node* node, int level){
    if(node == NULL) return;
    for(int i = 1; i <= level; ++i)
        cout<<".";
    cout<<node->getlabel()<<" "<<endl;
    for(ast_node* child_itr = node->l_child; child_itr != NULL; child_itr = child_itr->r_sibling)
        print_ast_node(child_itr, level + 1);
}
// print the built ast, from the top of ast_stk
void parser:: print_ast(){
    print_ast_node(ast_stk.top(), 0);
}
// helper function for standardizing the ast into st
void parser::rec_standard_tree(ast_node* pnode){
    if(pnode->l_child != NULL){
        rec_standard_tree(pnode->l_child);
    }
    if(pnode->r_sibling != NULL){
        rec_standard_tree(pnode->r_sibling);
    }
    if(pnode->getlabel()=="let"){
        ast_node* eq_node = pnode->l_child;
        if(eq_node->getlabel() == "="){
            ast_node* x_node = eq_node->l_child;
            ast_node* e_node = x_node->r_sibling;
            ast_node* p_node = eq_node->r_sibling;
            // switch the pointers
            eq_node->r_sibling = e_node;
            x_node->r_sibling = p_node;
            // change the labels
            pnode->setlabel("gamma");
            pnode->l_child->setlabel("lambda");
        }
        else{
            throw runtime_error("Error: \"let\"'s left child is not \"=\" (rec_standard_tree())");
        }
    }
    else if (pnode->getlabel() == "where"){
        ast_node* eq_node = pnode->l_child->r_sibling;
        if(eq_node->getlabel() == "="){
            ast_node* p_node = pnode->l_child;
            ast_node* x_node = eq_node->l_child;
            ast_node* e_node = x_node->r_sibling;
            // switch the pointers
            pnode->l_child = eq_node;
            x_node->r_sibling = p_node;
            p_node->r_sibling = NULL;
            eq_node->r_sibling = e_node;
            // change to labels
            pnode->setlabel("gamma");
            eq_node->setlabel("lambda");
        }
        else{
            throw runtime_error("Error: \"where\"'s right child is not \"=\" (rec_standard_tree())");
        }
    }
    else if (pnode->getlabel() == "within"){
        ast_node* left_eq_node = pnode->l_child;
        ast_node* right_eq_node = left_eq_node->r_sibling;
        if(left_eq_node->getlabel() == "=" && right_eq_node->getlabel() == "="){
            ast_node* x1_node = left_eq_node->l_child;
            ast_node *e1_node, *e2_node;
            if(x1_node){
                e1_node = x1_node->r_sibling;
            }
            else{
                throw runtime_error("Error: \"within\"'s X1 is NULL (rec_standard_tree())");
            }
            ast_node* x2_node = right_eq_node->l_child;
            if(x2_node){
                e2_node = x2_node->r_sibling;
            }
            else{
                throw runtime_error("Error: \"within\"'s X2 is NULL (rec_standard_tree())");
            }
            pnode->setlabel("=");
            pnode->l_child = x2_node;
            ast_node* gamma_tmp = new ast_node("gamma");
            x2_node->r_sibling = gamma_tmp;
            ast_node* lambda_tmp = new ast_node("lambda");
            gamma_tmp->l_child = lambda_tmp;
            lambda_tmp->r_sibling = e1_node;
            lambda_tmp->l_child = x1_node;
            x1_node->r_sibling = e2_node;
            delete left_eq_node;
            delete right_eq_node;
        }
        else{
            throw runtime_error("Error: \"within\"'s two children are not both \"=\" (rec_standard_tree())");
        }
    }
    else if (pnode->getlabel() == "rec"){
        ast_node* eq_node = pnode->l_child;
        if(eq_node->getlabel() != "="){
            throw runtime_error("the only child of \"rec\" is not \"=\" (rec_standard_tree())");
        }
        ast_node *x_node = eq_node->l_child;
        if(x_node == NULL){
            throw runtime_error("X is null in the subtree of \"=\" of \"rec\" (rec_standard_tree())");
        }
        ast_node *e_node = x_node->r_sibling;
        if(e_node == NULL){
            throw runtime_error("E is null in the subtree of \"=\" of \"rec\" (rec_standard_tree())");
        }
        else if (e_node->r_sibling != NULL){
            throw runtime_error("E's right sbiling isn't null in the subtree of \"=\" of \"rec\" (rec_standard_tree())");
        }
        pnode->setlabel("=");
        delete eq_node;
        pnode->l_child = x_node;
        ast_node* gamma_node = new ast_node("gamma");
        x_node->r_sibling = gamma_node;
        ast_node* ystar_node = new ast_node("<Y*>");
        gamma_node->l_child = ystar_node;
        ast_node* lambda_node = new ast_node("lambda");
        ystar_node->r_sibling = lambda_node;
        // copy X into new X node under lambda
        // child structure same as X, but the neighbor changes to E
        ast_node* x_copy_node = new ast_node(x_node->getlabel());
        lambda_node->l_child = x_copy_node;
        x_copy_node->l_child = x_node->l_child;
        x_copy_node->r_sibling = e_node;
    }
    else if (pnode->getlabel()=="function_form"){   // function form
        pnode->setlabel("=");
        ast_node* p_node = pnode->l_child;
        if(p_node != NULL){
            ast_node* v_node = p_node->r_sibling;
            if(v_node != NULL){
                while (v_node->r_sibling) {
                    ast_node* tmp = new ast_node("lambda");
                    tmp->l_child = v_node;
                    p_node->r_sibling = tmp;
                    p_node = v_node;
                    v_node = v_node->r_sibling;
                } // exit loop when v_node is E
                p_node->r_sibling = v_node; // p_node is the last V, and v_node is E
            }
            else{
                throw runtime_error("Error: \"func_form\" has no Vs (rec_standard_tree())");
            }
        }
        else{
            throw runtime_error("Error: \"func_form\" has no P as left child (rec_standard_tree())");
        }
    }
    else if (pnode->getlabel() == "and"){
        // at least two "=" children
        ast_node* eq1_node = pnode->l_child;
        if(eq1_node->getlabel() == "="){
            ast_node* eq2_node = eq1_node->r_sibling;
            ast_node* x1_node = eq1_node->l_child;
            ast_node *x2_node, *e1_node, *e2_node;
            ast_node *comma_node = new ast_node(",");
            ast_node *tau_node = new ast_node("tau");
            if(x1_node){
                e1_node = x1_node->r_sibling;
                comma_node->l_child = x1_node;
                tau_node->l_child = e1_node;
                if(e1_node == NULL){
                    throw runtime_error("no child \"E\" under \"and\"'s child 1st \"=\" (rec_standard_tree())");
                }
                while(eq2_node){
                    delete eq1_node;
                    x2_node = eq2_node->l_child;
                    if(x2_node){
                        e2_node = x2_node->r_sibling;
                        x1_node->r_sibling = x2_node;
                        e1_node->r_sibling = e2_node;
                        if(e2_node == NULL){
                            throw runtime_error("no child \"E\" under \"and\"'s child \"=\" (rec_standard_tree())");
                        }
                        x1_node = x2_node;
                        e1_node = e2_node;  // iterate to next loop
                        eq1_node = eq2_node;
                        eq2_node = eq2_node->r_sibling;
                        x2_node->r_sibling = NULL;
                    }
                    else{
                        throw runtime_error("no child \"X\" under \"and\"'s child \"=\" (rec_standard_tree())");
                    }
                }
                pnode->setlabel("=");
                pnode->l_child = comma_node;
                comma_node->r_sibling = tau_node;
            }
            else{
                throw runtime_error("no child \"X\" under \"and\"'s child 1st '=' (rec_standard_tree())");
            }
        }
        else{
            throw runtime_error("1st child of \"and\" is not \"=\" (rec_standard_tree())");
        }
        
    }
    else if (pnode->getlabel() == "@"){
        ast_node* e1_node = pnode->l_child;
        if(e1_node == NULL){
            throw runtime_error("1st child E1 of \"at\" is null (rec_standard_tree())");
        }
        ast_node* n_node = e1_node->r_sibling;
        if(n_node == NULL){
            throw runtime_error("2nd child N of \"at\" is null (rec_standard_tree())");
        }
        ast_node* e2_node = n_node->r_sibling;
        if(e2_node == NULL){
            throw runtime_error("3rd child E2 of \"at\" is null (rec_standard_tree())");
        }
        else if (e2_node->r_sibling != NULL) {
            throw runtime_error("3rd child E2 of \"at\"'s right sibling is not null (rec_standard_tree())");
        }
        pnode->setlabel("gamma");
        ast_node *gamma_node = new ast_node("gamma");
        pnode->l_child = gamma_node;
        gamma_node->l_child = n_node;
        n_node->r_sibling = e1_node;
        e1_node->r_sibling = NULL;
        gamma_node->r_sibling = e2_node;
    }
}
void parser::standard_tree(){
    rec_standard_tree(ast_stk.top());
}

unordered_set<string> CSEM::binary_op_set = {"or", "&", "gr", "ge", "ls", "le", "eq", "ne", "+", "-", "*", "/", "**", "aug"};
unordered_set<string> CSEM::unary_op_set = {"not", "neg"};
unordered_set<string> CSEM::prim_func = {"<ID:Print>", "<ID:Conc>", "<ID:Order>", "<ID:Istuple>", "<ID:Isinteger>", "<ID:Istruthvalue>", "<ID:Isstring>", "<ID:Isfunction>", "<ID:Stem>", "<ID:Stern>", "<ID:ItoS>"};

string ast_node::getid(){ // only used for identifier
    if(ast_label[0]=='<'){
        if(ast_label.find(":") == string::npos){ // <id> format:<dummy>, <true>, <false>, <nil>, etc.
            return ast_label.substr(1,ast_label.length() - 2);
        }
        else if(ast_label.substr(0,4) == "<ID:"){
            return ast_label;
        }
        else if (ast_label.substr(0,5) == "<STR:"){  // <STR:'id'>
            return ast_label.substr(6,ast_label.size()-8);
        }
        else{ // <INT:id>
            size_t start = ast_label.find(":"), end = ast_label.find_last_of(">");
            return ast_label.substr(start+1, end-start-1);
        }
    }
    else // other conditions
        return ast_label;
}
//*******************ENVIRONMENT************************
void env::insert(string key, exe_node* val){
    bind_var[key]= val;
}
// look up the name in current environment, func of env
exe_node* env::lookup(string key){
    if(bind_var.find(key) != bind_var.end()){
        return bind_var[key];
    }
    else if(parent != NULL){
        return parent->lookup(key);
    }
    else{ // no match at the outest environment
        return NULL;
    }
}

//*******************CSE Machine************************
void CSEM::proc(string args = ""){  // main function to control the execution of input rpal program
    parse_tree.proc(args);
    execute();
}
// generate control structure for CSE Machine, pre-order traverse
void CSEM::generate_ctrl_struct(){
    rec_generate_ctrl_struct(parse_tree.get_asthead(), ctrl_head);
}
void CSEM::rec_generate_ctrl_struct(ast_node* st_node, ctrl_node* &cur_ctrl_node){
    // for lambda node
    // add lambda_k_x to the current control structure, and generate a new control structure
    if(st_node->getlabel() == "lambda"){
        cur_ctrl_node = new ctrl_node(st_node);
        cur_ctrl_node->set_type(CTRL_LAMBDA);
        // set the value of bind_var: bound variable name to this lambda function.
        if(st_node->l_child->getlabel() != ",") {  // just one variable
            cur_ctrl_node->bind_var = st_node->l_child;
        }
        else{ // multiple variables st_node->l_child is comma ','
            cur_ctrl_node->bind_var = st_node->l_child; // other variable names are linked by rsibling
        }
        rec_generate_ctrl_struct(st_node->l_child->r_sibling, cur_ctrl_node->next_delta);
        if(st_node->r_sibling){
            rec_generate_ctrl_struct(st_node->r_sibling, cur_ctrl_node->next);
        }
    }
    else if (st_node->getlabel() == "gamma"){
        cur_ctrl_node = new ctrl_node(st_node);
        cur_ctrl_node->set_type(CTRL_GAMMA);
        if(st_node->l_child){
            rec_generate_ctrl_struct(st_node->l_child, cur_ctrl_node->next);
        }
        if(st_node->r_sibling){
            ctrl_node* last_node = cur_ctrl_node;  // walk to the last node
            while(last_node->next){
                last_node = last_node->next;
            }
            rec_generate_ctrl_struct(st_node->r_sibling,last_node->next);
        }
    }
    else if( st_node->getlabel() == "tau"){
        cur_ctrl_node = new ctrl_node(st_node);
        cur_ctrl_node->set_type(CTRL_TAU);
        ast_node* tmp_child = st_node->l_child;
        if(tmp_child == NULL){
            throw runtime_error("the child of \"tau\" is NULL (rec_generate_ctrl_struct())");
        }
        rec_generate_ctrl_struct(tmp_child, cur_ctrl_node->next);
        int child_num = 0;
        while(tmp_child){
            ++child_num;
            tmp_child = tmp_child->r_sibling;
        }
        cur_ctrl_node->set_childnum(child_num);
        if(st_node->r_sibling != NULL){
            ctrl_node* last_node = cur_ctrl_node;
            while (last_node->next) {
                last_node = last_node->next;
            }
            rec_generate_ctrl_struct(st_node->r_sibling, last_node->next); // next_ctrl_node is NULL;
        }
    }
    else if (st_node->getlabel() == "->"){  // delta_then, delta_else, beta, B
        ast_node* B = st_node->l_child;
        if(B == NULL){
            throw runtime_error("the 1st child B of \"->\" is NULL (rec_generate_ctrl_struct())");
        }
        ast_node* T = B->r_sibling;
        if(T == NULL){
            throw runtime_error("the 2nd child T of \"->\" is NULL in (rec_generate_ctrl_struct())");
        }
        ast_node* E = T->r_sibling;
        if(E == NULL){
            throw runtime_error("the 3rd child E of \"->\" is NULL in (rec_generate_ctrl_struct())");
        }
        // delta_then control node
        B->r_sibling = NULL;
        T->r_sibling = NULL;
        E->r_sibling = NULL;
        ctrl_node* then_delta = new ctrl_node(CTRL_DELTA, "then");
        cur_ctrl_node = then_delta;
        rec_generate_ctrl_struct(T, then_delta->next_delta);
        // delta_else control node
        ctrl_node* else_delta = new ctrl_node(CTRL_DELTA, "else");
        then_delta->next = else_delta;
        rec_generate_ctrl_struct(E, else_delta->next_delta);
        ctrl_node* beta_node = new ctrl_node(CTRL_BETA, "beta");
        else_delta->next = beta_node;
        rec_generate_ctrl_struct(B, beta_node->next);
        if(st_node->r_sibling != NULL){
            ctrl_node* last_node = beta_node;
            while(last_node->next){
                last_node = last_node->next;
            }
            rec_generate_ctrl_struct(st_node->r_sibling, last_node->next);
        }
    }
    else{ // pre-order walk through
        string tmp_label = st_node->getlabel();
        int len = tmp_label.length();
        cur_ctrl_node = new ctrl_node(st_node);
        if(tmp_label.substr(0,5) == "<INT:"){
            cur_ctrl_node->set_type(CTRL_INT);
        }
        else if (tmp_label == "<Y*>"){
            cur_ctrl_node->set_type(CTRL_YSTAR);
        }
        else if (tmp_label.substr(0,5) == "<STR:"){
            cur_ctrl_node->set_type(CTRL_STR);
        }
        else if (tmp_label.substr(0,4) == "<ID:"){
            cur_ctrl_node->set_type(CTRL_ID);
        }
        else if (tmp_label == "<nil>"){
            cur_ctrl_node->set_type(CTRL_NIL);
        }
        else if (tmp_label == "<true>"){
            cur_ctrl_node->set_type(CTRL_TRUE);
        }
        else if (tmp_label == "<false>"){
            cur_ctrl_node->set_type(CTRL_FALSE);
        }
        else if (tmp_label == "<dummy>"){
            cur_ctrl_node->set_type(CTRL_DUMMY);
        }
        else if(binary_op_set.find(tmp_label) != binary_op_set.end()){
            cur_ctrl_node->set_type(CTRL_BINOP);
        }
        else if(unary_op_set.find(tmp_label) != unary_op_set.end()){
            cur_ctrl_node->set_type(CTRL_UOP);
        }
        // traverse l_child if exits
        if(st_node->l_child){
            rec_generate_ctrl_struct(st_node->l_child, cur_ctrl_node->next);
        }
        // traverse right sibling if exits
        if(st_node->r_sibling){
            ctrl_node* last_node = cur_ctrl_node;
            while(last_node->next){
                last_node = last_node->next; // arrive the last one
            }
            rec_generate_ctrl_struct(st_node->r_sibling, last_node->next);
        }

    }
}

// initialize the CSE Machine's control stack, execution stack, environment
void CSEM::init_execute(){
    cur_env = new env();
    env_indx = 0;
    env_stk.push(cur_env);
    exe_node* start_exe_node = new exe_node(EXE_ENV, "e0");
    exe_stk.push(start_exe_node);
    
    ctrl_node* start_ctrl_node = new ctrl_node(CTRL_ENV, "e0");
    ctrl_stk.push(start_ctrl_node);
    // push delta_node into the control stack
    ctrl_node* itr_ctrl_node = ctrl_head;
    while(itr_ctrl_node){
        ctrl_stk.push(itr_ctrl_node);
        itr_ctrl_node = itr_ctrl_node->next;
    }
}

// excute the program, pop and push operations on control stack and execution stack
void CSEM::execute(){
    generate_ctrl_struct();
    init_execute();
    ctrl_node* cur_ctrl_node;
    while (!ctrl_stk.empty()) {
        exe_node* cur_exe_node;
        cur_ctrl_node = ctrl_stk.top();
        ctrl_node_type cur_ctrl_node_type = cur_ctrl_node->get_type();
        string cur_ctrl_node_label = cur_ctrl_node->get_label();
        ctrl_stk.pop();
        // rule 1: stack a name
        if(cur_ctrl_node_type == CTRL_INT){
            cur_exe_node = new exe_node(cur_ctrl_node);
            cur_exe_node->set_type(EXE_INT);
            exe_stk.push(cur_exe_node);
        }
        else if (cur_ctrl_node_type == CTRL_STR){
            cur_exe_node = new exe_node(cur_ctrl_node);
            cur_exe_node->set_type(EXE_STR);
            exe_stk.push(cur_exe_node);
        }
        else if (cur_ctrl_node_type == CTRL_NIL){
            cur_exe_node = new exe_node(cur_ctrl_node);
            cur_exe_node->set_type(EXE_NIL);
            exe_stk.push(cur_exe_node);
        }
        else if (cur_ctrl_node_type == CTRL_DUMMY){
            cur_exe_node = new exe_node(cur_ctrl_node);
            cur_exe_node->set_type(EXE_DUMMY);
            exe_stk.push(cur_exe_node);
        }
        else if (cur_ctrl_node_type == CTRL_TRUE){
            cur_exe_node = new exe_node(cur_ctrl_node);
            cur_exe_node->set_type(EXE_TRUE);
            exe_stk.push(cur_exe_node);
        }
        else if (cur_ctrl_node_type == CTRL_FALSE){
            cur_exe_node = new exe_node(cur_ctrl_node);
            cur_exe_node->set_type(EXE_FALSE);
            exe_stk.push(cur_exe_node);
        }
        else if (cur_ctrl_node_type == CTRL_YSTAR){
            cur_exe_node = new exe_node(cur_ctrl_node);
            cur_exe_node->set_type(EXE_YSTAR);
            exe_stk.push(cur_exe_node);
        }
        else if (cur_ctrl_node_type == CTRL_ID){ // need to look up the env's table
            cur_exe_node = cur_env->lookup(cur_ctrl_node_label);
            if(cur_exe_node){ //find bound exe_node
                exe_stk.push(cur_exe_node);
            }
            // primitive env
            else{
                if(prim_func.find(cur_ctrl_node_label) != prim_func.end()){
                    cur_exe_node = new exe_node(EXE_PRIM_FUNC, cur_ctrl_node_label);
                    exe_stk.push(cur_exe_node);
                }
                else{
                    throw runtime_error("undefined id in CSEM::execute()");
                }
            }

        }
        // rule 2: stack lambda
        else if (cur_ctrl_node_type == CTRL_LAMBDA){
            cur_exe_node = new exe_node(cur_ctrl_node);
            cur_exe_node->linked_env = cur_env;  // cur_env stores the current environment of CSE
            cur_exe_node->set_type(EXE_LAMBDA);
            exe_stk.push(cur_exe_node);
        }
        // rule 3/4/10/11/12/13: apply rator/lambda
        else if (cur_ctrl_node_type == CTRL_GAMMA){
            exe_node* top_exe_node = exe_stk.top();
            exe_stk.pop();
            exe_node_type top_exe_node_type = top_exe_node->get_type();
            string top_exe_node_val = top_exe_node->get_val();
            if(top_exe_node_type == EXE_PRIM_FUNC){ // rule 3
                exe_node *rand = exe_stk.top();
                exe_stk.pop();
                if(rand == NULL){
                    throw runtime_error("operand is NULL for " + top_exe_node_val + " func in CSEM::execute()");
                }
                exe_node_type rand_type = rand->get_type();
                if(top_exe_node_val == "<ID:Print>"){
                    rpal_print(rand);
                    exe_node* result = new exe_node(EXE_DUMMY, "dummy");
                    exe_stk.push(result);
                }
                else if (top_exe_node_val.length() >= 9 && top_exe_node_val.substr(0,9) == "<ID:Conc>"){
                    if(top_exe_node_val.length()==9){ // apply to first operand
                        if(rand->get_type() == EXE_STR){
                            exe_node* result = new exe_node(EXE_PRIM_FUNC, "<ID:Conc>>" + rand->get_val());
                            exe_stk.push(result);
                        }
                        else{
                            throw runtime_error("1st operand is not string for Conc in CSEM::execute()");
                        }
                    }
                    else{ // apply to second operand
                        if(rand->get_type() == EXE_STR){
                            exe_node* result = new exe_node(EXE_STR, top_exe_node_val.substr(10, top_exe_node_val.length()-10) + rand->get_val());
                            exe_stk.push(result);
                        }
                        else{
                            throw runtime_error("1st operand is not string for Conc in CSEM::execute()");
                        }
                    }
                }
                else if (top_exe_node_val == "<ID:Order>" ){
                    exe_node* result = new exe_node(EXE_INT, "0");
                    if(rand_type == EXE_NIL)
                        ;
                    else if (rand_type == EXE_TUPLE){
                        int num = rand->tuple_que.size();
                        result->set_val(tostring(num));
                    }
                    exe_stk.push(result);
                }
                else if (top_exe_node_val == "<ID:Istuple>"){
                    exe_node* result;
                    if(rand_type == EXE_TUPLE || rand_type == EXE_NIL){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                    exe_stk.push(result);
                }
                else if (top_exe_node_val == "<ID:Isinteger>"){
                    exe_node* result;
                    if(rand_type == EXE_INT){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                    exe_stk.push(result);
                }
                else if (top_exe_node_val == "<ID:Istruthvalue>"){
                    exe_node* result;
                    if(rand_type == EXE_TRUE || rand_type == EXE_FALSE){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                    exe_stk.push(result);
                }
                else if (top_exe_node_val == "<ID:Isstring>"){
                    exe_node* result;
                    if(rand_type == EXE_STR){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                    exe_stk.push(result);
                }
                else if (top_exe_node_val == "<ID:Isfunction>"){
                    exe_node* result;
                    if(rand_type == EXE_LAMBDA){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                    exe_stk.push(result);
                }
                else if (top_exe_node_val == "<ID:Stem>"){
                    if(rand_type == EXE_STR){
                        exe_node* result = new exe_node(EXE_STR, rand->get_val().substr(0,1));
                        exe_stk.push(result);
                    }
                    else
                        throw runtime_error("operand is not string for " + top_exe_node_val + " func in CSEM::execute()");
                }
                else if (top_exe_node_val == "<ID:Stern>"){
                    if(rand_type == EXE_STR){
                        int str_len =rand->get_val().length() - 1;
                        exe_node* result = new exe_node(EXE_STR, rand->get_val().substr(1,str_len));
                        exe_stk.push(result);
                    }
                    else
                        throw runtime_error("operand is not string for " + top_exe_node_val + " func in CSEM::execute()");
                }
                else if (top_exe_node_val == "<ID:ItoS>"){
                    if(rand_type == EXE_INT){
                        exe_node* result = new exe_node(EXE_STR, rand->get_val());
                        exe_stk.push(result);
                    }
                    else
                        throw runtime_error("operand is not string for " + top_exe_node_val + " func in CSEM::execute()");
                }
            }
            else if(top_exe_node_type == EXE_TUPLE){ // rule 10
                exe_node* tuple_indx = exe_stk.top();
                exe_stk.pop();
                if(tuple_indx->get_type() == EXE_INT){
                    int indx = stoi(tuple_indx->get_val());
                    exe_node* result;
                    queue<exe_node*> tmp_save;
                    int tuple_size = top_exe_node->tuple_que.size();
                    for(int i = 1; i <= tuple_size; ++i){
                        result = top_exe_node->tuple_que.front();
                        tmp_save.push(result);
                        top_exe_node->tuple_que.pop();
                        if(i == indx) exe_stk.push(result);
                    }
                    while(!tmp_save.empty()){ // push back the node to maintain the original tuple of the var
                        top_exe_node->tuple_que.push(tmp_save.front());
                        tmp_save.pop();
                    }
                }
                else
                    throw runtime_error("operand2 is "+ tuple_indx->get_val() + "(not integer) for " + top_exe_node_val + " func applying to tupple in CSEM::execute()");
            }
            else if(top_exe_node_type == EXE_LAMBDA){ // rule 4 & 11
                // create a new environment
                ++env_indx;
                env* sub_env = new env(env_indx);
                env_stk.push(sub_env); // push the latest env into the env stack.
                sub_env->parent = top_exe_node->linked_env;
                cur_env = sub_env;
                exe_node* bind_val = exe_stk.top();
                exe_stk.pop();
                ast_node* bind_itr = top_exe_node->get_bindvar();
                if(bind_itr->getid() == ","){ //mutiple variables
                    bind_itr = bind_itr->l_child;
                    if(bind_val->get_type() == EXE_TUPLE){
                        exe_node* itr = bind_val->tuple_que.front();
                        while(!bind_val->tuple_que.empty() || bind_itr){
                            if(bind_val->tuple_que.empty()){
                                throw runtime_error("bound values less than bound variables in CSEM::execute()");
                            }
                            if(bind_itr == NULL){
                                throw runtime_error("bound values more than bound variables in CSEM::execute()");
                            }
                            sub_env->insert(bind_itr->getid(), itr);
                            bind_val->tuple_que.pop();
                            itr = bind_val->tuple_que.front();
                            bind_itr = bind_itr->r_sibling;
                        }
                    }
                    else{
                        throw runtime_error("lambda bind error");
                    }
                }
                else{
                    sub_env->insert(bind_itr->getid(), bind_val);
                }
                string env_str = "e" + tostring(env_indx);
                exe_node* env_exe_node = new exe_node(EXE_ENV, env_str);
                exe_stk.push(env_exe_node);
                
                ctrl_node* env_ctrl_node = new ctrl_node(CTRL_ENV, env_str);
                ctrl_stk.push(env_ctrl_node);
                
                // push the new delta into control stack
                ctrl_node* new_delta_itr = top_exe_node->get_delta_head();
                if(new_delta_itr ==  NULL){
                    throw runtime_error("NO bound control delta node for lambda in CSEM::execute()");
                }
                while(new_delta_itr){
                    ctrl_stk.push(new_delta_itr);
                    new_delta_itr = new_delta_itr->next;
                }
            }
            // rule 12 applying Y
            else if (top_exe_node->get_type() == EXE_YSTAR){
                // change to eta
                exe_node* tmp = exe_stk.top();
                if(tmp->get_type() != EXE_LAMBDA){
                    throw runtime_error("NO EXE_LAMBDA after EXE_YSTAR in CSEM::execute()");
                }
                tmp->set_type(EXE_ITA);
                tmp->set_val("ita");
            }
            // rule 13 applying f.p.
            else if (top_exe_node->get_type() == EXE_ITA){
                // since gamma in control stack has been pop out
                // we need to insert gamma twice into the control stack
                exe_stk.push(top_exe_node);
                exe_node* rec_lambda = new exe_node(top_exe_node->pt_ctrlnode);
                rec_lambda->set_type(EXE_LAMBDA);
                rec_lambda->linked_env = top_exe_node->linked_env;
                exe_stk.push(rec_lambda);
                ctrl_stk.push(new ctrl_node(CTRL_GAMMA, "gamma"));
                ctrl_stk.push(new ctrl_node(CTRL_GAMMA, "gamma"));
            }
        }
        // rule 5 exit environment
        else if (cur_ctrl_node_type == CTRL_ENV){
            cur_exe_node = exe_stk.top();
            exe_stk.pop();
            exe_node* next_exe_node = exe_stk.top();
            if(next_exe_node->get_type() == EXE_ENV){
                exe_stk.pop();
                exe_stk.push(cur_exe_node);
            }
            else{
                throw runtime_error("No ENV match the ENV in CONTROL id in CSEM::execute()");
            }
            if(!env_stk.empty()){
                env_stk.pop();
                cur_env = env_stk.empty()?NULL:env_stk.top();
            }
        }
        // rule 6 binary operator
        else if (cur_ctrl_node_type == CTRL_BINOP){
            // top two operands
            exe_node* top_exe_rand1 = exe_stk.top();
            exe_node_type rand1_type = top_exe_rand1->get_type();
            string rand1_val = top_exe_rand1->get_val();
            exe_stk.pop();
            exe_node* top_exe_rand2 = exe_stk.top();
            exe_node_type rand2_type = top_exe_rand2->get_type();
            string rand2_val = top_exe_rand2->get_val();
            exe_stk.pop();
            if(cur_ctrl_node_label == "&" || cur_ctrl_node_label == "or"){
                if((rand1_type != EXE_TRUE && rand1_type != EXE_FALSE) \
                   || (rand2_type != EXE_TRUE && rand2_type != EXE_FALSE)){
                    throw runtime_error("two operands' types are not true/false, don't match binary operator " + cur_ctrl_node_label + " 's requirement in CSEM::execute()");
                }
                exe_node* result;
                if(cur_ctrl_node_label == "&"){
                    if(rand1_type == EXE_TRUE && rand2_type == EXE_TRUE){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                }
                else{ // or
                    if(rand1_type == EXE_TRUE || rand2_type == EXE_TRUE){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                }
                exe_stk.push(result);
            }
            else if (cur_ctrl_node_label == "aug"){	   
                if((rand1_type == EXE_TUPLE || rand1_type == EXE_NIL)		 &&
                    (rand2_type == EXE_TUPLE ||  rand2_type == EXE_NIL || \
                    rand2_type == EXE_INT || rand2_type == EXE_STR || \
                    rand2_type == EXE_TRUE || rand2_type == EXE_FALSE || \
                    rand2_type == EXE_DUMMY)){
                       exe_node* result = top_exe_rand1;
                       result->tuple_que.push(top_exe_rand2);
                       result->set_type(EXE_TUPLE);
                       result->set_val("tuple");
                       exe_stk.push(result);
                }
                else
                    throw runtime_error("two operands' types doesn't match aug binary operator's requirement in CSEM::execute()");
            }
            else if(cur_ctrl_node_label == "eq"){
                exe_node* result;
                if(rand1_type == rand2_type){
                    if(rand1_val == rand2_val){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                    exe_stk.push(result);
                }
                else{
                    throw runtime_error("two operands' types must be same for eq");
                }
            }
            else if(cur_ctrl_node_label == "ne"){
                exe_node* result = new exe_node();
                if(rand1_type == rand2_type){
                    if(rand1_val != rand2_val){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                    exe_stk.push(result);
                }
                else{
                    throw runtime_error("two operands' types must be same for ne");
                }
            }
            // gr, ge, ls, le, eq, ne, +, -, *, /, **
            else{
                if(rand1_type != EXE_INT || rand2_type != EXE_INT){
                    throw runtime_error("two operands' types are not both integer, don't match &/or binary operator " + cur_ctrl_node_label + " 's requirement in CSEM::execute()");
            }
                int rand1_int = stoi(rand1_val);
                int rand2_int = stoi(rand2_val);
                exe_node* result;
                if(cur_ctrl_node_label == "gr"){
                    if(rand1_int > rand2_int){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                }
                else if(cur_ctrl_node_label == "ge"){
                    if(rand1_int >= rand2_int){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                }
                else if(cur_ctrl_node_label == "ls"){
                    if(rand1_int < rand2_int){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                }
                else if(cur_ctrl_node_label == "le"){
                    if(rand1_int <= rand2_int){
                        result = new exe_node(EXE_TRUE, "true");
                    }
                    else{
                        result = new exe_node(EXE_FALSE, "false");
                    }
                }
                else if(cur_ctrl_node_label == "+"){
                    result = new exe_node(EXE_INT,tostring(rand1_int + rand2_int));
                }
                else if(cur_ctrl_node_label == "-"){
                    result = new exe_node(EXE_INT,tostring(rand1_int - rand2_int));
                }
                else if(cur_ctrl_node_label == "*"){
                    result = new exe_node(EXE_INT,tostring(rand1_int * rand2_int));
                }
                else if(cur_ctrl_node_label == "/"){
                    result = new exe_node(EXE_INT,tostring(rand1_int / rand2_int));
                }
                else if(cur_ctrl_node_label == "**"){
                    result = new exe_node(EXE_INT,tostring(pow(rand1_int, rand2_int)));
                }
                exe_stk.push(result);
            }
        }
        // rule 7 unary operator
        else if (cur_ctrl_node_type ==  CTRL_UOP){
            exe_node* rand = exe_stk.top();
            exe_stk.pop();
            exe_node_type rand_type = rand->get_type();
            string rand_val = rand->get_val();
            exe_node* result;
            if(cur_ctrl_node_label == "not"){
                if(rand_type == EXE_TRUE){
                    result = new exe_node(EXE_FALSE, "false");
                }
                else if(rand_type == EXE_FALSE){
                    result = new exe_node(EXE_TRUE, "true");
                }
                else{
                    throw runtime_error("no true/false after not in CSEM::execute()");
                }
            }
            else if (cur_ctrl_node_label == "neg"){
                if(rand_type != EXE_INT){
                    throw runtime_error("no integer after neg in CSEM::execute()");
                }
                int rand_int = stoi(rand_val);
                result = new exe_node(EXE_INT, tostring((0-rand_int)));
            }
            exe_stk.push(result);
        }
        // rule 8: conditional
        else if (cur_ctrl_node_type == CTRL_BETA){
            exe_node *top_exe_node = exe_stk.top();
            exe_stk.pop();
            ctrl_node* else_delta = ctrl_stk.top();
            ctrl_stk.pop();
            ctrl_node* then_delta = ctrl_stk.top();
            ctrl_stk.pop();
            if(else_delta->get_type() != CTRL_DELTA && else_delta->get_label() != "else"){
                throw runtime_error("NO CTRL_DELTA else after CTRL_BETA in CSEM::execute()");
            }
            if(then_delta->get_type() != CTRL_DELTA && then_delta->get_label() != "then"){
                throw runtime_error("NO CTRL_DELTA then after CTRL_BETA in CSEM::execute()");
            }
            if(top_exe_node->get_type() == EXE_TRUE){
                load_ctrl_delta(then_delta);
                
            }
            else if (top_exe_node->get_type() == EXE_FALSE){
                load_ctrl_delta(else_delta);
            }
            else{
                throw runtime_error("NO true/false value on the top of EXE_STACK then for CTRL_BETA in CSEM::execute()");
            }
        }
        // rule 9: tuple formation
        else if (cur_ctrl_node->get_type() == CTRL_TAU){ // tuple
            cur_exe_node = new exe_node(cur_ctrl_node);
            cur_exe_node->set_type(EXE_TUPLE);
            int tau_child = cur_ctrl_node->get_childnum();
            for(int i = 1; i <= tau_child; ++i){
                exe_node* tmp_child = exe_stk.top();
                cur_exe_node->tuple_que.push(tmp_child);
                exe_stk.pop();
            }
            exe_stk.push(cur_exe_node);
        }
    }
}

void CSEM::rpal_print(exe_node* operand){
    exe_node_type tmp = operand->get_type();
    if(tmp == EXE_INT){
        cout<<operand->get_val();
    }
    else if ( tmp == EXE_STR){
      string output_str = operand->get_val(); 
      size_t str_len = output_str.length();
      for(size_t i = 0; i < str_len; ++i){
	if(output_str[i] == '\\'){
	  switch(output_str[++i]){
	  case 'n':
	    cout<<"\n";
	    break;
	  case 't':
	    cout<<"\t";
	  }
	}
	else
	  cout<<output_str[i];
      }
    }
    else if (tmp == EXE_ENV){
        cout<<"<ENVIRONMENT>"<<endl;
    }
    else if (tmp == EXE_TUPLE){
        cout<<"(";
        exe_node* tuple_itr;
        while(operand->tuple_que.size() > 1){
            tuple_itr = operand->tuple_que.front();
            rpal_print(tuple_itr);
            operand->tuple_que.pop();
            cout<<", ";
        }
        tuple_itr = operand->tuple_que.front();
        rpal_print(tuple_itr);
        operand->tuple_que.pop();
        cout<<")";
    }
    else if (tmp == EXE_LAMBDA){
      string bind_varname = operand->get_bindvar()->getid();
      size_t str_len = bind_varname.length();
      cout<<"[lambda closure: "<<bind_varname.substr(4,str_len-5)<<": 2]";
    }
    else if (tmp == EXE_TRUE || tmp == EXE_FALSE || tmp == EXE_NIL || tmp == EXE_DUMMY){
        cout<<operand->get_val();
    }
    else if (tmp == EXE_YSTAR){
        cout<<"<YSTAR>"<<endl;
    }
    else if (tmp == EXE_ITA){
        cout<<"<ITA>"<<endl;
	string bind_varname = operand->get_bindvar()->getid();
        cout<<"[lambda closure: "<<bind_varname.substr(4,bind_varname.size()-4)<<": 2]"<<endl;
    }
    else{
        throw runtime_error("no type match in rpal primitive print func");
    }
}

void CSEM::load_ctrl_delta(ctrl_node* delta_node){
    if(delta_node->get_type() != CTRL_DELTA){
        throw runtime_error("Input control node's type is not CTRL_DELTA in load_ctrl_delta");
    }
    ctrl_node *head = delta_node->next_delta;
    while(head){
        ctrl_stk.push(head);
        head = head->next;
    }
}

// CSEM



