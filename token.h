#include <string>
#include <vector>
using namespace std;

enum type {Reserved_Word, Identifier, Integer, Operator, String, Punction, END_OF_FILE};

class token{
private:
    static const string type_name[7]; // store the type name (string) of the token objects
    string t_value;
    type t_type;
    
public:
    static const char op_symb[26];
    static const string reserved_words[20];
    //constructor
    token(string t_v, type t_t):t_value(t_v), t_type(t_t){}
    string gettype(){
        return type_name[t_type];
    }
    string getvalue(){
        return t_value;
    }
    
};
