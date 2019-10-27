#include <iostream>
#include "parser.h"
#include <stdexcept>
#include <cstring>


int main(int argc, const char * argv[]) {
    string prog_name;
	if(argc == 2){
		prog_name = argv[1];
        CSEM rpal_cse(prog_name);
        rpal_cse.proc("");
        cout<<endl;
		return 0;
	}
    else if(argc != 3 || strcmp(argv[1], "-ast") != 0){
        throw runtime_error("usage: ./p2 [-ast] filename");
    }
	else{
		prog_name = argv[2];
		CSEM rpal_cse(prog_name);
		rpal_cse.proc("-ast");
        cout<<endl;
		return 0;
	}
}
