/**
 * Drew Nagy-Kato
 * COP 3402 - System Software
 * Assignment 3 - Parser/Code Generator
 * 7/16/12
**/

/** TODOs
 *  - fine tune error system
		- Add:
			- Variable doesn't start with letter
				- check at var, const declaration
				- check at statement, at token before becomesym
			- Const overwrite check
**/

/** NOTES
 *  -
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CHARS 11
#define MAX_DIGS 5
#define MAX_TABLE_SIZE 500
#define CODE_SIZE 500

struct symtable {
	int kind;					// const = 1, var = 2, proc = 3
	char name[MAX_CHARS];		// name up to 11 chars
	int val;					// number (ASCII value)
	int level;					// L level
	int addr;                   // M address
};

struct codegen {
	int op;                     // opcode
	int l;                      // lexicographical level
	int m;                      // modifier
};

struct tokenlist {
	int token_num;              // Numerical token
	char symname[13];           // Symbol name
	char ident[13];             // Identifier string
	int number;                 // Number
};


int cx = 0;
int errorNum = 0;
int tokindex = 0;
int symdex = 0;
int tok_max = 0;

struct tokenlist** token;
struct symtable** symbol;
struct codegen** code;

enum SYMLIST {nulsym = 1, identsym, numbersym, plussym, minussym, multsym, slashsym, oddsym, eqlsym, neqsym, lessym, leqsym, gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym, periodsym, becomessym, beginsym, endsym, ifsym, thensym, whilesym, dosym, callsym, constsym, intsym, procsym, outsym, insym, elsesym, writesym, readsym, lcommentsym, rcommentsym};
enum OPR_M {OPR_RTN = 0, OPR_NEG, OPR_ADD, OPR_SUB, OPR_MUL, OPR_DIV, OPR_ODD, OPR_MOD, OPR_EQL, OPR_NEQ, OPR_LSS, OPR_LEQ, OPR_GTR, OPR_GEQ};
enum OPCODE {LIT = 1, OPR, LOD, STO, CAL, INC, JMP, JPC, WRT, RED};

void program();
void block(int L, int M);
void statement(int L, int M);
void condition();
void expression();
void term();
void factor();
int validateIdent(char* ident);
int getLevel(char* ident);
void emit(int op, int l, int m);
void insertSymbol(char* name, int kind, int L, int M, int val);
int checkRelOp();
void printTokens(FILE* fout);
void printSymbols(FILE* fout);
void printCode(FILE* fout);
void seedError();
void getSymname(int i);
void error(FILE* fout);


int main(void){
	FILE* fin;
	FILE* fout;
	//FILE* vmout;
	int i;
	int len;
	
	fin = fopen("T4_parserin.txt", "r");
	fout = fopen("T4_parserout.txt", "w");

    if(fin == NULL){
		printf("Error reading file\n");
		system("PAUSE");
        return 0;
    }
    else{
		// Determine length of file. Worst case, all data on one line
        /*fseek(fin, 0, SEEK_END);
        len = ftell(fin);
        fseek(fin, 0, 0);*/
        char symname[13];
        char strinput[MAX_CHARS];
        int dinput;
        int numinput;
        int num_elements = 0;

        // Initialize each element of the struct
        code = calloc(CODE_SIZE, sizeof(struct codegen*));
        token = calloc(CODE_SIZE, sizeof(struct tokenlist*));
        symbol = calloc(CODE_SIZE, sizeof(struct symtable*));
		for(i = 0; i < CODE_SIZE; i++){
            code[i] = malloc(sizeof(struct codegen));
            symbol[i] = malloc(sizeof(struct symtable));
		}

        fprintf(fout, "A print out of the token file:\n");
        if(ferror(fout)){
            printf("Error writing to file\n");
        	return 0;
        }
        printf("A print out of the token file:\n");

		// Read in token file, reprint
		while(!feof(fin)){
			token[tokindex] = malloc(sizeof(struct tokenlist));
			fscanf(fin,"%d ", &dinput);
			fprintf(fout,"%d ", dinput);
			printf("%d ", dinput);
			token[tokindex]->token_num = dinput;
			if(dinput == identsym){
				fscanf(fin,"%s ", strinput);
				fprintf(fout,"%s ", strinput);
				printf("%s ", strinput);
				if(strlen(strinput) <= MAX_CHARS)
					strcpy(token[tokindex]->ident, strinput);
				else{
					printf("\n\nSTRING: %s \n", strinput);
					errorNum = 26;
					break;
				}
			}else if(dinput == numbersym){
				fscanf(fin,"%d ", &numinput);
				fprintf(fout,"%d ", numinput);
				printf("%d ", numinput);
				if(numinput <= 99999)
					strcpy(token[tokindex]->ident, strinput);
				else{
					printf("\n\nNUMBER: %d \n", numinput);
					errorNum = 25;
					break;
				}
				token[tokindex]->number = numinput;
			}
			tokindex++;
		}
		printf("\n");
		fprintf(fout, "\n");
		
		for(i = 0; i < tokindex; i++){
        	getSymname(i);
		}
		printTokens(fout);
		
		if(errorNum == 0){
			// Run the parser
			program();
		}

		error(fout);
		//printf("%d\n", errorNum);
		if(errorNum == 0){
			// Print symbol table and generated code
			printSymbols(fout);
			printCode(fout);
		}
		else{
			//printSymbols(fout);
			//seedError();
		}
		
		for(i = 0; i < CODE_SIZE; i++){
        	free(token[i]);
        	free(code[i]);
        	free(symbol[i]);
		}

    	free(token);
	    free(symbol);
	    free(code);
    } // eof else no error
    
    fclose(fin);
	fclose(fout);
	//return errorNum;
    return 0;
} // eof main

// block "."
void program(){
	int L = 0;  // first L level is zero
	int M = 3; 	// reserve space for the SL, DL, RA
	tok_max = tokindex;
	tokindex = 0;
	emit(INC, L, M);  // Increment - INC 0, M

	//printf("%s \n", token[tokindex]->symname);
		
	block(L, M);
	//printf("prgm post-block: %s \n", token[tokindex]->symname);
	if(errorNum == 0){
		tokindex++;
		//printf("prgm period: %s \n", token[tokindex]->symname);
		if(token[tokindex]->token_num != periodsym){
			errorNum = 9;
			return;
		}
		else
			emit(OPR, 0, OPR_RTN);    // Return - OPR 0, 0
	}
}

// block ::= const-declaration var-declaration procedure-declaration statement
void block(int L, int M){
	char name[MAX_CHARS];
	int kind = 0;
	int value;
	if(errorNum == 0){
		// constdeclaration ::= ["const" ident "=" number {"," ident "=" number} ";"].
		if(token[tokindex]->token_num == constsym){  // add constant
			kind = 1;

			do{
				tokindex++;
				//printf("%s \n", token[tokindex]->symname);
				if(token[tokindex]->token_num != identsym){     // needs INC after first call
					errorNum = 4;
					return;
				}else{
					strcpy(name, token[tokindex]->ident);
				}

				tokindex++;
				//printf("%s \n", token[tokindex]->symname);
				if(token[tokindex]->token_num != eqlsym){
					errorNum = 3;
					return;
				}

				tokindex++;
				//printf("%s \n", token[tokindex]->symname);
				if(token[tokindex]->token_num != numbersym){
					errorNum = 2;
					return;
				}else if(token[tokindex]->number >= 100000){
					errorNum = 25;
					return;
				}else
					value = token[tokindex]->number;

				insertSymbol(name, kind, L, M, value);
				emit(INC, L, 1);    	// Increment 1 space for each constant added
				emit(LIT, L, value);
				emit(STO, L, M);
				M++;
	   			tokindex++;
	   			//printf("%s \n", token[tokindex]->symname);
			} while(token[tokindex]->token_num == commasym);

			if(token[tokindex]->token_num != semicolonsym){
				errorNum = 10;
				return;
			}
			tokindex++;
			//printf("%s \n", token[tokindex]->symname);
		}
		// var-declaration ::= [ "int "ident {"," ident} “;"]
		if(token[tokindex]->token_num == intsym){   // add variable
			kind = 2;
			do{
				tokindex++;
				//printf("%s \n", token[tokindex]->symname);
				if(token[tokindex]->token_num != identsym){
					errorNum = 4;
					return;
				}
				else{
					strcpy(name, token[tokindex]->ident);
				}

				insertSymbol(name, kind, L, M, 0);
				emit(INC, L, 1);    // INC 1 space for each variable added
				M++;
				tokindex++;
				//printf("%s \n", token[tokindex]->symname);
			}while(token[tokindex]->token_num == commasym);

			if(token[tokindex]->token_num != semicolonsym){
				errorNum = 5;
				return;
			}
			tokindex++;
			//printf("%s \n", token[tokindex]->symname);
		}
		/*
		// procedure-declaration ::= { "procedure" ident ";" block ";" }
		while(token[tokindex]->token_num == procsym){  // add procedure
			kind = 3;

			tokindex++;
			//printf("%s \n", token[tokindex]->symname);
			if(token[tokindex]->token_num != identsym){
					errorNum = 4;
					return;
			}
			else{
				strcpy(name, token[tokindex]->ident);
			}
			if(token[tokindex]->token_num != semicolonsym){
				errorNum = 10;
				return;
			}

			insertSymbol(name, kind, L, M);

			M++;
			token[tokindex]->token_num
			block(L++, M);

			if(token[tokindex]->token_num != semicolonsym)
				errorNum = 10;
			L--;
			tokindex++;

			if(cx > CODE_SIZE)
				errorNum = 25;
			else{
				emit(2, 0, 0);   // Return - OPR 0,0
			}
			tokindex++;
			printf("%s \n", token[tokindex]->symname);
		}*/
		//printf("pre-statement: %s \n", token[tokindex]->symname);
		statement(L, M);   // final begin <statement> end
	}
}

//statement ::= [ ident ":=" expression || "begin" statement { ";" statement } "end" ||
// "if" condition "then" statement ["else" statement] || "while" condition "do" statement ||
// "read" ident || "write" expression || null ]
void statement(int L, int M){
	char name[MAX_CHARS];

	if(errorNum == 0){
		//printf("stmnt NEW_STATEMENT: %s %d\n", token[tokindex]->symname, tokindex);
		if(token[tokindex]->token_num == identsym){ // ident ":=" expression
			//printf("stmnt ident: %s \n", token[tokindex]->symname);
			int sto_addr = validateIdent(token[tokindex]->ident);
			int sto_level = getLevel(token[tokindex]->ident);

			if(sto_addr > 0){

				tokindex++;
		  		//printf("stmnt post-ident: %s \n", token[tokindex]->symname);
				if(token[tokindex]->token_num != becomessym){   // :=
					errorNum = 13;
					return;
				}
				tokindex++;
				//printf("stmnt ident pre-epxression: %s \n", token[tokindex]->symname);
				expression();
				//printf("stmnt ident post-epxression: %s \n", token[tokindex]->symname);

				if(errorNum == 0){
					if(cx > CODE_SIZE)
						errorNum = 25;
					else{
						emit(STO, sto_level, sto_addr);    // Store - STO L, M
					}
				}
			}
			else{
				errorNum = 11;
				return;
			}
		}
		/*
		else if(token[tokindex]->token_num == callsym){ // "call" ident
			current = current->next;
			if(token[tokindex]->token_num != identsym){
				errorNum = 4;
				return;
			}
			tokindex++;

			if(cx > CODE_SIZE)
				errorNum = 25;
			else{
				emit(CAL, L, M);  // Call - CAL L,M
			}
		}*/
		// "begin" statement {";" statement} "end"
		else if(token[tokindex]->token_num == beginsym){
	 		//int cx1;
			tokindex++;
			//printf("stmnt post-begin: %s %d\n", token[tokindex]->symname, tokindex);
			statement(L, M);
			//printf("stmnt bgn post-1st stmnt: %s %d\n", token[tokindex]->symname, tokindex);
			//tokindex++;
			while(token[tokindex]->token_num == semicolonsym){
				tokindex++;
				//printf("stmnt bgn pre-stmnt loop: %s %d\n", token[tokindex]->symname, tokindex);
				statement(L, M);
				//printf("stmnt bgn post-stmnt loop: %s %d\n", token[tokindex]->symname, tokindex);
				//tokindex++;
				if(errorNum != 0)
					break;
			}
			if(errorNum == 0){
				printf("stmnt pre no-end: %s %d, %s \n", token[tokindex]->symname, tokindex, token[tokindex+1]->symname);
				if(token[tokindex]->token_num != endsym){
					errorNum = 27;
					return;
				}
			}
		}
		// "if" condition "then" statement ["else" statement]
		else if(token[tokindex]->token_num == ifsym){
			tokindex++;
			//printf("stmnt IF pre-cndtn: %s %d\n", token[tokindex]->symname, tokindex);
			condition();
			//printf("stmnt IF post-cndtn: %s %d\n", token[tokindex]->symname, tokindex);
			if(errorNum == 0){
				if(token[tokindex]->token_num != thensym){
					errorNum = 16;
					return;
				}

				tokindex++;
				int ctemp = cx;

				if(cx > CODE_SIZE){
					errorNum = 25;
					return;
				}
				else
					emit(JPC, 0, 0);      // JPC 0, 0
				//printf("stmnt THEN pre-stmnt: %s %d\n", token[tokindex]->symname, tokindex);
				statement(L, M);
				//printf("stmnt THEN post-stmnt: %s %d\n", token[tokindex]->symname, tokindex);
				if(errorNum == 0){
					code[ctemp]->m = cx;				// changes JPC 0 0 to JPC 0 cx

					if(token[tokindex]->token_num == elsesym){
						tokindex++;
						//int ctemp = cx;

						if(cx > CODE_SIZE){
							errorNum = 25;
							return;
						}
						else
							emit(JPC, 0, 0);      // JPC 0, 0
						//printf("stmnt ELSE pre-stmnt: %s %d\n", token[tokindex]->symname, tokindex);
						statement(L, M);
						//printf("stmnt ELSE post-stmnt: %s %d\n", token[tokindex]->symname, tokindex);
						code[ctemp]->m = cx;			// changes JPC 0 0 to JPC 0 cx
					}
				}
			}
		}
		// "while" condition "do" statement
		else if(token[tokindex]->token_num == whilesym){
			int cx1 = cx;
			tokindex++;
			//printf("stmnt WHILE pre-cndtn: %s %d\n", token[tokindex]->symname, tokindex);
			condition();
			//printf("stmnt WHILE post-cndtn: %s %d\n", token[tokindex]->symname, tokindex);
			if(errorNum == 0){
				int cx2 = cx;
				if(cx > CODE_SIZE){
					errorNum = 25;
					return;
				}else
					emit(JPC, 0, 0);      // JPC 0, 0
				
				if(token[tokindex]->token_num != dosym){
					errorNum = 18;
					return;
				}
				
				tokindex++;
				//printf("stmnt DO pre-stmnt: %s %d\n", token[tokindex]->symname, tokindex);
				statement(L, M);
				tokindex++;
				//printf("stmnt DO post-stmnt: %s %d\n", token[tokindex]->symname, tokindex);
				if(errorNum == 0){
					if(cx > CODE_SIZE){
						errorNum = 25;
						return;
					}
					else{
						emit(JMP, 0, cx1);    // JMP 0, M
						code[cx2]->m = cx;
					}
				}
			}
		}
		// "read"
		else if(token[tokindex]->token_num == readsym){
			//printf("stmnt READ pre-EMIT: %s %d\n", token[tokindex]->symname, tokindex);
			tokindex++;
			emit(RED, 0, 2);        	// RED 0, 2
			//printf("stmnt READ post-EMIT: %s %d\n", token[tokindex]->symname, tokindex);
		}
		// "write"
		else if(token[tokindex]->token_num == writesym){
			tokindex++;
			//printf("stmnt WRITE pre-EMIT: %s %d\n", token[tokindex]->symname, tokindex);
			//expression();
   			emit(WRT, 0, 1);            // WRT 0, 1
   			//printf("stmnt WRITE post-EMIT: %s %d\n", token[tokindex]->symname, tokindex);
		}
		// "in" ident
		else if(token[tokindex]->token_num == insym){
			tokindex++;

			if(token[tokindex]->token_num != identsym){
				errorNum = 4;
				return;
			}else{
				int sto_addr = validateIdent(token[tokindex]->ident);
				int sto_level = getLevel(token[tokindex]->ident);

				if(errorNum == 0){
					if(cx > CODE_SIZE)
						errorNum = 25;
					else{
						emit(RED, 0, 2);            		// RED 0, 2
						emit(STO, sto_level, sto_addr);		// STO L, M
						tokindex++;
					}
				}
				
				emit(STO, sto_level, sto_addr);     // STO L, M
				
			}
		}
		// "out" ident
		else if(token[tokindex]->token_num == outsym){
			tokindex++;

			if(token[tokindex]->token_num != identsym){
				errorNum = 4;
				return;
			}else{
				int lod_addr = validateIdent(token[tokindex]->ident);
				int lod_level = getLevel(token[tokindex]->ident);

				if(!lod_addr){
					errorNum = 11;
					//printf("Error: %d\n", errorNum);
					return;
				}
				else{
					emit(LOD, lod_level, lod_addr);     // LOD L, M
					emit(WRT, 0, 1);            		// WRT 0, 1
					tokindex++;
				}
			}
		}
	}
}

// "odd" expression | expression rel-op expression
void condition(){
	int relation = 0;
	if(errorNum ==0){
		//printf("cndtn NEW CONDITION: %s %d\n", token[tokindex]->symname, tokindex);
		if(token[tokindex]->token_num == oddsym){
			int oddop = token[tokindex]->token_num;
			tokindex++;
			//printf("cndtn ODD pre-expr: %s %d\n", token[tokindex]->symname, tokindex);
			expression();
			//printf("cndtn ODD post-expr: %s %d\n", token[tokindex]->symname, tokindex);
			if(errorNum == 0){
				if(cx > CODE_SIZE){
					errorNum = 25;
					return;
				}
				else
					emit(OPR, 0, OPR_ODD);		// ODD - OPR 0, 6
			}
		}
		else {
			//printf("cndtn RELOP pre-1st expr: %s %d\n", token[tokindex]->symname, tokindex);
			expression();
			//printf("cndtn RELOP post-1st expr: %s %d\n", token[tokindex]->symname, tokindex);
			if(errorNum == 0){
				relation = checkRelOp(token[tokindex]->token_num);
				tokindex++;
				//printf("cndtn RELOP pre-2nd expr: %s %d\n", token[tokindex]->symname, tokindex);
				expression();
				//printf("cndtn RELOP post-2nd expr: %s %d\n", token[tokindex]->symname, tokindex);

				if(relation > 0){
					if(cx > CODE_SIZE)
						errorNum = 25;
					else
						emit(OPR, 0, relation);   //Creates code for either EQL, NEQ, LSS, LEQ, GTR, or GEQ
				}else{
					errorNum = 20;
					return;
				}
			}
		}
	}
}

//[ "+"|"-"] term {("+"|"-") term}
void expression(){
	int addop;

	if(errorNum == 0){
		//printf("expr NEW_EXPRESSION: %s %d\n", token[tokindex]->symname, tokindex);
		if(token[tokindex]->token_num == plussym || token[tokindex]->token_num == minussym){
			addop = token[tokindex]->token_num;
			tokindex++;
			term();
			if(errorNum == 0){
				if(addop == minussym){
					if(cx > CODE_SIZE){
						errorNum = 25;
						return;
					}
					else
						emit(OPR, 0 , OPR_NEG); // negate
				}
			}
		}
		//printf("expr pre-term: %s \n", token[tokindex]->symname);
		term();
		//tokindex++;
		//printf("expr pre-(+,-): %s %d\n", token[tokindex]->symname, tokindex);
		while(token[tokindex]->token_num == plussym || token[tokindex]->token_num == minussym){
			addop = token[tokindex]->token_num;
			tokindex++;
			//printf("expr pre-2nd term: %s %d\n", token[tokindex]->symname, tokindex);
			term();
			//printf("expr post-2nd term: %s %d\n", token[tokindex]->symname, tokindex);

			//if(token[tokindex]->token_num == rparentsym)
			//	tokindex++;

			if(errorNum == 0){
				if(addop == plussym){
					if(cx > CODE_SIZE)
						errorNum = 25;
					else{
						emit(OPR, 0, OPR_ADD); // addition
					}
				}else{
					if(cx > CODE_SIZE){
						errorNum = 25;
						return;
					}
					else
						emit(OPR, 0, OPR_SUB); // subtraction
				}
			}else
				break;
		}
	}
}

// factor {("*")|("/") factor}
void term(){
	int mulop;
	
	if(errorNum == 0){
		//printf("term NEW_TERM: %s %d\n", token[tokindex]->symname, tokindex);
		factor();
		tokindex++;
		//printf("term pre-(*,/): %s %d\n", token[tokindex]->symname, tokindex);
		while(token[tokindex]->token_num == multsym || token[tokindex]->token_num == slashsym){
			mulop = token[tokindex]->token_num;
			tokindex++;
			//printf("term pre-2nd fctr: %s %d\n", token[tokindex]->symname, tokindex);
			factor();
			//printf("term post-2nd fctr: %s %d\n", token[tokindex]->symname, tokindex);
			tokindex++;
			if(errorNum == 0){
				if(mulop == multsym){
					if(cx > CODE_SIZE){
						errorNum = 25;
						return;
					}
					else
						emit(OPR, 0, OPR_MUL); // multiplication
				}else{
					if(cx > CODE_SIZE){
						errorNum = 25;
						return;
					}
					else
						emit(OPR, 0, OPR_DIV); // division
				}
			}else
				break;
		}
	}
}

// ident | number | "(" expression ")"
void factor(){
	int value;
	//char* ident;
	if(errorNum == 0){
		//printf("fctr NEW_FACTOR: %s %d\n", token[tokindex]->symname, tokindex);
		if(token[tokindex]->token_num == identsym){      	// ident
			//printf("fctr ident: %s \n", token[tokindex]->symname);
			int lod_addr;
			int lod_level;
			lod_addr = validateIdent(token[tokindex]->ident);
			lod_level = getLevel(token[tokindex]->ident);

			if(!lod_addr){
				errorNum = 11;
				//printf("Error: %d\n", errorNum);
				return;
			}
			else
				emit(LOD, lod_level, lod_addr);

			//tokindex++;
		}
		else if(token[tokindex]->token_num == numbersym){   // number
			//printf("fctr nmbr: %s %d\n", token[tokindex]->symname, tokindex);
			value = token[tokindex]->number;
			if(cx > CODE_SIZE){
				errorNum = 25;
				return;
			}
			else
				emit(LIT, 0, value);     // LIT 0, M
			//tokindex++;
		}
		else if(token[tokindex]->token_num == lparentsym){  // "("
			tokindex++;
			//printf("fctr post-lparen: %s \n", token[tokindex]->symname);
			expression(); // expression
			//tokindex++;
			//printf("fctr pre-rparen: %s \n", token[tokindex]->symname);
			if(errorNum == 0){
				if(token[tokindex]->token_num != rparentsym){    // ")"
					errorNum = 22;
					return;
				}
				//tokindex++;
			}
		}
		else{
			errorNum = 19;
			return;
		}
	}
}

int validateIdent(char* ident){
	int i;
	int M;
	for(i = 0; i < symdex; i++){
		if(strcmp(ident, symbol[i]->name) == 0){
			/*if(symbol[i]->kind == 1){
				M = symbol[i]->val;
				//printf("symmatch: %s Value:%d\n", symbol[i]->name, symbol[i]->val);
			}
			else{
				M = symbol[i]->addr;
				//printf("symmatch: %s L:%d  M:%d\n", symbol[i]->name, symbol[i]->level, symbol[i]->addr);
			}*/
			M = symbol[i]->addr;
			//printf("M:%d\n", M);
			return M;
		}
	}
	return 0;
}

// Returns the L level of the identifier in question
int getLevel(char* ident){
	int i;
	int L;
	for(i = 0; i < symdex; i++){
		if(strcmp(ident, symbol[i]->name) == 0){
			//printf("symmatch: %s L:%d  M:%d\n", symbol[i]->name, symbol[i]->level, symbol[i]->addr);
			L = symbol[i]->level;
			//printf("L:%d\n", L);
			return L;
		}
	}
	return 0;
}

// Generate assembly code for VM
void emit(int op, int l, int m){
		//printf("cx: %d \n", cx);
		code[cx]->op = op;
		code[cx]->l = l;
		code[cx]->m = m;
		cx++;
}

void insertSymbol(char* name, int kind, int L, int M, int val){

 	switch(kind){
		case 1:		// const (kind, name, value)
			symbol[symdex]->kind = kind;
			strcpy(symbol[symdex]->name, name);
			symbol[symdex]->level = L;
			symbol[symdex]->addr = M;
			symbol[symdex]->val = val;
			break;
		case 2:     // var  (kind, name, L, M)
			symbol[symdex]->kind = kind;
			strcpy(symbol[symdex]->name, name);
			symbol[symdex]->level = L;
			symbol[symdex]->addr = M;
			break;
		case 3:     // proc (kind, name, L, M)
			symbol[symdex]->kind = kind;
			strcpy(symbol[symdex]->name, name);
			symbol[symdex]->level = L;
			symbol[symdex]->addr = M;
			break;
	}
	symdex++;
}

int checkRelOp(int relsym){

	switch(relsym){
		case eqlsym: // EQL
   			return OPR_EQL;
			break;
		case neqsym: // NEQ
   			return OPR_NEQ;
			break;
		case lessym: // LSS
   			return OPR_LSS;
			break;
		case leqsym: // LEQ
   			return OPR_LEQ;
			break;
		case gtrsym: // GTR
   			return OPR_GTR;
			break;
		case geqsym: // GEQ
   			return OPR_GEQ;
			break;
		default: // not a relation symbol
			return 0;
			break;
	}
}

void printTokens(FILE* fout){
	int i;

	printf("\nAnd its symbolic representation:\n");
	fprintf(fout, "\nAnd its symbolic representation:\n");
	for(i = 0; i < tokindex; i++){
		printf("%s %d ", token[i]->symname , i);
		fprintf(fout, "%s ", token[i]->symname);
	}
	printf("\n\n");
	fprintf(fout, "\n");
}

void printSymbols(FILE* fout){
	int i;

	printf("\nSymbol Table:\n"
		"KIND  NAME          L   M\n");
	fprintf(fout, "\nSymbol Table:\n"
		"KIND  NAME          L   M\n");
	for(i = 0; i < symdex; i++){
		switch(symbol[i]->kind){
		case 1:		// const (kind, name, value)
			fprintf(fout, "const %-11s   %-d\n", symbol[i]->name, symbol[i]->val);
			printf("const %-11s   %-d\n", symbol[i]->name, symbol[i]->val);
			break;
		case 2:     // var  (kind, name, L, M)
			fprintf(fout, "var   %-11s   %-2d  %-2d\n", symbol[i]->name, symbol[i]->level, symbol[i]->addr);
			printf("%var   %-11s   %-2d  %-2d\n", symbol[i]->name , symbol[i]->level, symbol[i]->addr);
			break;
		case 3:     // proc (kind name, L, M)
			fprintf(fout, "proc    %-11s   %-2d  %-2d\n", symbol[i]->name, symbol[i]->level, symbol[i]->addr);
			printf("proc  %-11s   %-2d  %-2d\n",  symbol[i]->name, symbol[i]->level, symbol[i]->addr);
			break;
		}
	}
	//printf("\n");
	//fprintf(fout, "\n");
}

void printCode(FILE* fout){
	FILE* vmout;
	vmout = fopen("T4_vminput.txt", "w");
	int i;
	
	// Seeds a '0' in front of generated code into VM input file
	// Notifies compiler driver of successful parsing
	//fprintf(vmout,"%d\n", errorNum);

	printf("\nGenerated Assembly Code:\n");
	fprintf(fout, "\nGenerated Assembly Code:\n");
	for(i = 0; i < cx; i++){
		fprintf(fout,"%d %d %d\n", code[i]->op, code[i]->l, code[i]->m);
		fprintf(vmout,"%d %d %d\n", code[i]->op, code[i]->l, code[i]->m);
		printf("%d %d %d\n", code[i]->op, code[i]->l, code[i]->m);

	}
	//printf("\n");
	//fprintf(fout, "\n");
	
	fclose(vmout);
}

// If there is an error, inserts error number into VM input file
// This notifies the compiler driver and thus the user, of a parser error
void seedError(){
	FILE* vmout;
	vmout = fopen("T2_vminput.txt", "w");
	
	fprintf(vmout,"%d\n", errorNum);
	fclose(vmout);
}

void getSymname(int i){
	
	switch(token[i]->token_num){
		case 1:	// nulsym
			strcpy(token[i]->symname, "nulsym");
			break;
		case 2: // identsym
			strcpy(token[i]->symname, "identsym");
			break;
		case 3: // numbersym
			strcpy(token[i]->symname, "numbersym");
			break;
		case 4: // plussym
			strcpy(token[i]->symname, "plussym");
			break;
		case 5: // minussym
			strcpy(token[i]->symname, "minussym");
			break;
		case 6: // multsym
			strcpy(token[i]->symname, "multsym");
			break;
		case 7: // slashsym
			strcpy(token[i]->symname, "slashsym");
			break;
		case 8: // oddsym
			strcpy(token[i]->symname, "oddsym");
			break;
		case 9: // eqlsym
			strcpy(token[i]->symname, "eqlsym");
			break;
		case 10: // neqsym
			strcpy(token[i]->symname, "neqsym");
			break;
		case 11: // lessym
			strcpy(token[i]->symname, "lessym");
			break;
		case 12: // leqsym
			strcpy(token[i]->symname, "leqsym");
			break;
		case 13: // gtrsym
			strcpy(token[i]->symname, "gtrsym");
			break;
		case 14: // geqsym
			strcpy(token[i]->symname, "geqsym");
			break;
		case 15: // lparentsym
			strcpy(token[i]->symname, "lparentsym");
			break;
		case 16: // rparentsym
			strcpy(token[i]->symname, "rparentsym");
			break;
		case 17: // commasym
			strcpy(token[i]->symname, "commasym");
			break;
		case 18: // semicolonsym
			strcpy(token[i]->symname, "semicolonsym");
			break;
		case 19: // periodsym
			strcpy(token[i]->symname, "periodsym");
			break;
		case 20: // becomessym
			strcpy(token[i]->symname, "becomessym");
			break;
		case 21: // beginsym
			strcpy(token[i]->symname, "beginsym");
			break;
		case 22: // endsym
			strcpy(token[i]->symname, "endsym");
			break;
		case 23: // ifsym
			strcpy(token[i]->symname, "ifsym");
			break;
		case 24: // thensym
			strcpy(token[i]->symname, "thensym");
			break;
		case 25: // whilesym
			strcpy(token[i]->symname, "whilesym");
			break;
		case 26: // dosym
			strcpy(token[i]->symname, "dosym");
			break;
		case 27: // callsym
			strcpy(token[i]->symname, "callsym");
			break;
		case 28: // contsym
			strcpy(token[i]->symname, "constsym");
			break;
		case 29: // intsym
			strcpy(token[i]->symname, "intsym");
			break;
		case 30: // procsym
			strcpy(token[i]->symname, "procsym");
			break;
		case 31: // outsym
			strcpy(token[i]->symname, "outsym");
			break;
		case 32: // insym
			strcpy(token[i]->symname, "insym");
			break;
		case 33: // elsesym
			strcpy(token[i]->symname, "elsesym");
			break;
		case 34: // writesym
			strcpy(token[i]->symname, "writesym");
			break;
		case 35: // readsym
			strcpy(token[i]->symname, "readsym");
			break;
		case 36: // lcommentsym
			strcpy(token[i]->symname, "lcommentsym");
			break;
		case 37: // rcommentsym
			strcpy(token[i]->symname, "rcommentsym");
			break;
		default:
			strcpy(token[i]->symname, "UNKNOWN");
			break;
	}
}

void error(FILE* fout){

	fprintf(fout, "\nErrors Found:\n");
	printf("\nErrors Found:\n");

	switch(errorNum){
		case 0:
			fputs("No errors, program is syntactically correct\n", fout);
			puts("No errors, program is syntactically correct\n");
			break;
		case 1:
			fputs("Error number 1: used = instead of :=\n", fout);
			puts("Error number 1: used = instead of :=\n");
			break;
		case 2:
			fputs("Error number 2: = must be followed by a number\n", fout);
			puts("Error number 2: = must be followed by a number\n");
			break;
		case 3:
			fputs("Error number 3: identifier expected", fout);
			puts("Error number 3: identifier expected");
			break;
		case 4:
			fputs("Error number 4: 'const', 'int', 'procedure' must be followed by an identifier\n", fout);
			puts("Error number 4: 'const', 'int', 'procedure' must be followed by an identifier\n");
			break;
		case 5:
   			fputs("Error number 5, semicolon or comma missing\n", fout);
			puts("Error number 5, semicolon or comma missing\n");
			break;
		case 6:
			fputs("Error number 6, incorrect symbol after procedure declaration\n", fout);
			puts("Error number 6, incorrect symbol after procedure declaration\n");
			break;
		case 7:
			fputs("Error number 7, statement expected\n", fout);
			puts("Error number 7, statement expected\n");
			break;
		case 8:
			fputs("Error number 8, incorrect symbol after statement part in block\n", fout);
			puts("Error number 8, incorrect symbol after statement part in block\n");
			break;
		case 9:
			fputs("Error number 9, period expected\n", fout);
			puts("Error number 9, period expected\n");
			break;
		case 10:
			fputs("Error number 10,  semicolon between statements missing\n", fout);
			puts("Error number 10,  semicolon between statements missing\n");
			break;
		case 11:
			fputs("Error number 11, undeclared identifier\n", fout);
			puts("Error number 11, undeclared identifier\n");
			break;
		case 12:
			fputs("Error number 12, assignment to constant or procedure is not allowed\n", fout);
			puts("Error number 12, assignment to constant or procedure is not allowed\n");
			break;
		case 13:
			fputs("Error number 13, assignment operator expected\n", fout);
			puts("Error number 13, assignment operator expected\n");
			break;
		case 14:
			fputs("Error number 14, 'call' must be followed by an identifier\n", fout);
			puts("Error number 14, 'call' must be followed by an identifier\n");
			break;
		case 15:
			fputs("Error number 15, call of a constant or variable is meaningless\n", fout);
			puts("Error number 15, call of a constant or variable is meaningless\n");
			break;
		case 16:
			fputs("Error number 16, 'then' or 'else' expected\n", fout);
			puts("Error number 16, 'then' or 'else' expected\n");
			break;
		case 17:
			fputs("Error number 17, semicolon or right bracket expected\n", fout);
			puts("Error number 17, semicolon or right braket expected\n");
			break;
		case 18:
			fputs("Error number 18, 'do' expected\n", fout);
			puts("Error number 18, 'do' expected\n");
			break;
		case 19:
			fputs("Error number 19, incorrect symbol following statement\n", fout);
			puts("Error number 19, incorrect symbol following statement\n");
			break;
		case 20:
			fputs("Error number 20, relational operator expected\n", fout);
			puts("Error number 20, relational operator expected\n");
			break;
		case 21:
			fputs("Error number 21, expression must not contain a procedure identifier\n", fout);
			puts("Error number 21, expression must not contain a procedure identifier\n");
			break;
		case 22:
			fputs("Error number 22, right parenthesis missing\n", fout);
			puts("Error number 22, right parenthesis missing\n");
			break;
		case 23:
			fputs("Error number 23, the preceding factor cannot begin with this symbol\n", fout);
			puts("Error number 23, the preceding factor cannot begin with this symbol\n");
			break;
		case 24:
			fputs("Error number 24, an expression cannot begin with ths symbol\n", fout);
			puts("Error number 24, an expression cannot begin with ths symbol\n");
			break;
		case 25:
			fputs("Error number 25, this number is too large\n", fout);
			puts("Error number 25, this number is too large\n");
			break;
		case 26:
			fputs("Error number 26, Identifier name is too long\n", fout);
			puts("Error number 26, Identifier name is too long\n");
			break;
		case 27:
			fputs("Error number 27, 'end' expected\n", fout);
			puts("Error number 27, 'end' expected\n");
			break;
		default:
			fputs("Unknown error occured\n", fout);
			puts("Unknown error occured\n");
			break;
	}
}
