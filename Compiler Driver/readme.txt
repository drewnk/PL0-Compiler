All of the information below is described in greater detail in the PDFs within this project, but let this serve as a quick guide.

Running the compiler:
	1) This compiler driver allows for several command line arguments. The following are the supported compiler directives:

	-l : print the list of lexemes/tokens (scanner output) to the screen	-a : print the generated assembly code (parser/codegen output) to the screen	-v : print virtual machine execution trace (virtual machine output) to the screen
			
For example, if you wanted to run the program printing all the supported compiler directives, the execute command would look like:

	./driver input.txt -l -a -v

It does not matter what order the compiler directives are in, but the input file MUST be the first argument in order for the program to run properly.

2) If running the program in a GUI-based interface such as Eclipse or Dev C++, which allows you to enter a string of arguments, follow the same rules for compiler directives as described above. The input file must be a text file, and it MUST be listed first among the program's arguments.

3) The compiler will generate a file containing the lexeme list, generated assembly code, and the stack output to a text file named "output.txt".

Notes (regarding input grammar):
	1) Read/Write v. In/Out:
		a) Read follows the grammar: "'read' ". It simply asks the user for an integer and pushes it on top of the stack.
		b) Write follows the grammar: "'write' ". It simply prints the value located at the top of the stack to the console and decrements SP.
		c) In follows the grammar: "'in' ident". It asks the user for an integer and stores it to the location in the stack indicated by the ident given in the source file.
		d) Out follows the grammar: "'out' ident". The current value of the given ident is copied to the top of the stack, popped off, and printed to the console.
	2) Use of ";" following statements:
		Within a given begin/end statement section, a ; cannot be present following the final statement preceding "end", as per the EBNF grammar.
		Please be sure your input file does not make this common grammatical mistake.
	3) Please note that this program is functionally correct, however due to time constraints there are some error checks beyond the basics which have not been implemented, such as catching any attempt to overwrite a constants during parsing. Any syntactically correct input files should parse as intended.
