#include "token.h"

const string token::type_name[7] = {"Reserved_Word","Identifier", "Integer", "Operator", "String", "Punction", "EOF"};
const char token::op_symb[26] = {'+', '-', '*', '<', '>', '&', '.', '@', '/', \
':', '=', '~','|','$','!','#','%','^', '_', '[', ']', '{', '}', '"', '`', '?'};
const string token::reserved_words[20] = {"let", "in", "fn", "where", "aug", "or", "not", "gr", "ge", \
				 "ls", "le", "eq", "ne", "true", "false", "nil", "dummy", \
				 "within", "and", "rec"};
