here we use g++ compile rather than gcc;
to generate the execuatable file p1, you can simply:
$make 
or $make p2
and to erase all the obj file and exe p1, you can type:
$make clean

only the required switch -ast is implemented here. I used marco to define debug option,
so if you can to want the debug info, just uncomment the "#define debug" line in parser.cpp.

rpal, tests folder and difftest.pl are not included in my submit folder,
but I have already ran all the test cases by typing:
$perl difftest.pl -1 "./rpal FILE" -2 "./p2 FILE" -t ./tests/
in my local environment, the path of tests folder and rpal need to be changed to match your test environments

all the test cases have passed, the results are shown below:
Testing test1...OK!
Testing simple.div...OK!
Testing ftst...OK!
Testing pairs1...OK!
Testing stem1...OK!
Testing conc.1...OK!
Testing t2...OK!
Testing Innerprod2...OK!
Testing fn2...OK!
Testing tiny...OK!
Testing print2...OK!
Testing infix...OK!
Testing sum...OK!
Testing if2...OK!
Testing defns.2...OK!
Testing add...OK!
Testing vectorsum...OK!
Testing recurs.1...OK!
Testing t3...OK!
Testing fn3...OK!
Testing pf...OK!
Testing if1...OK!
Testing t18...OK!
Testing stem2...OK!
Testing reverse...OK!
Testing t19...OK!
Testing string1...OK!
Testing wsum2...OK!
Testing pairs3...OK!
Testing defns.1...OK!
Testing towers...OK!
Testing t16...OK!
Testing t3.1...OK!
Testing trees...OK!
Testing picture...OK!
Testing clean...OK!
Testing pairs2...OK!
Testing print1...OK!
Testing tuples...OK!
Testing tiny.1...OK!
Testing defns.3...OK!
Testing t1...OK!
Testing wsum1...OK!
Testing Treepicture...OK!
Testing conc1...OK!
Testing conc3...OK!
Testing infix2...OK!
Testing fn1...OK!
Testing t9...OK!
Testing send...OK!
Testing prog...OK!
Testing Innerprod...OK!
Testing envlist...OK!
Testing div...OK!
All tests succeeded.