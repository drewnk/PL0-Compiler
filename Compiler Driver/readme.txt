Drew Nagy-Kato
COP 3402 - System Software
Assignment 4 - Compiler Driver
8/27/2012

To use the compiler:
	1) Compile 'driver.c' in the desired environment.
	2) Run the program:
		a) If in Eustis, type "./driver " followed by the desired filename
			followed by any additonal arguments as desired, separated by spaces.
			For example, if you wanted to run the program printing all the
			supported compiler directives, simply type "./driver input.txt -l -a -v".
			It does not matter what order the compiler directives are in, but the
			input file MUST be the first argument in order for the program to run
			properly.
		b) If running the program elsewhere such as Dev C++, which allows you to
			enter a string of arguments, follow the same rules as part (2a). The
			input file must be a text file, and it MUST be listed first among
			the program's arguments
	3) The compiler will generate a file containing the lexeme list, generated
		assembly code, and stack output to a text file named "output.txt".

Notes:
	1) Read/Write v. In/Out:
		a) Read follows the grammar: "'read' ". It simply asks the user for an
			integer and pushes it on top of the stack.
		b) Write follows the grammar: "'write' ". It simply prints the value
			located at the top of the stack to the console and decrements SP.
		c) In follows the grammar: "'in' ident". It asks the user for an integer
			and stores it to the location in the stack indicated by the ident
			given in the source file.
		d) Out follows the grammar: "'out' ident". The current value of the
			given ident is copied to the top of the stack, popped off, and printed
			to the console.
	2) Use of ";" following statements:
		Within a given begin/end statement section, a ; cannot be present
		following the final statement preceeding "end", as per the EBNF grammar.
		Please be sure your input file does not make this common gramatical mistake.
	3) Please note that this program is functionally correct, however due to time
		constraints there are some error checks beyond the basics which have not
		been implemented, such as catching any attempt to overwrite a constants
		during parsing. Any syntactially correct input files should parse as
		intended.
