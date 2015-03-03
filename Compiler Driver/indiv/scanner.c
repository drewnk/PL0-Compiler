/*
 * Drew Nagy-Kato
 * COP 3402 - System Software
 * Assignment 2 - Lexical Analyzer (Scanner)
 * 6/27/2012
**/

/** TODOs
 *	-
**/

/** NOTES
 *	-
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_CHARS 11
#define MAX_DIGS 5
#define CODE_SIZE 500


struct lextable {
	char lexeme[13];		// lexeme string
	int number;             // numbersym number
	int token_type;			// numerical designation of lexeme
};


int lexindex = 0;           // token index
int comm_flag = 0;          // flag for scanning comments
struct lextable** lexeme;

enum SYMLIST {nulsym = 1, identsym, numbersym, plussym, minussym, multsym, slashsym, oddsym, eqlsym, neqsym, lessym, leqsym, gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym, periodsym, becomessym, beginsym, endsym, ifsym, thensym, whilesym, dosym, callsym, constsym, intsym, procsym, outsym, insym, elsesym, writesym, readsym, lcommentsym, rcommentsym};

void insertLexeme(char string[]);
void separateTokens(char string[]);
int examineToken(char string[]);
int check_specsym(char* t);
int check_reserved(char* t);
int parseNumber(char string[]);
void printList(FILE* fout);
void printTable(FILE* fout);


int main(void){
    FILE* fin;
    FILE* fout;
    int i;

    fin = fopen("T2_scanin.txt", "r");
    fout = fopen("T2_scanout.txt", "w");

    if(fin == NULL){
        perror("Error opening file\n");
        return 0;
    }
    else {
        char string[MAX_CHARS];
		
		lexeme = calloc(CODE_SIZE, sizeof(struct lextable*));
		
        fprintf(fout, "Source Program:\n");
        if(ferror(fout)){
            printf("Error writing to file\n");
            return 0;
        }
        printf("Source Program:\n");

		while(!feof(fin)){
			fscanf(fin, "%s ", string);
			fprintf(fout, "%s ", string);
			printf("%s ", string);
			separateTokens(string);
		}
		fprintf(fout,"\n");
		printf("\n");
		
		printTable(fout);
		printList(fout);
		
		for(i = 0; i < CODE_SIZE; i++){
        	free(lexeme[i]);
		}
    	free(lexeme);

    }// eof else (!error)
	
	fclose(fin);
	fclose(fout);
	return 0;
}// eof main


// Insert string into the list
void insertLexeme(char string[]){
	int type;
	int number;
	type = examineToken(string);
	//printf("insert string: %s %d\n", string, type);
	lexeme[lexindex] = malloc(sizeof(struct lextable));
	
	if(comm_flag || type == rcommentsym){
		// Lexeme commented out, do nothing
	}else if(type == numbersym){
  		//printf("pre-number insertion\n");
		number = parseNumber(string);
		lexeme[lexindex]->number = number;
		lexeme[lexindex]->token_type = type;
		//printf("inserted: %d %d\n", lexeme[lexindex]->number, lexeme[lexindex]->token_type);
		lexindex++;
	}else if(type > 0){
		//printf("pre-other insertion\n");
		strcpy(lexeme[lexindex]->lexeme, string);
		//printf("string copied\n");
		lexeme[lexindex]->token_type = type;
		//printf("inserted: %s %d\n", lexeme[lexindex]->lexeme, lexeme[lexindex]->token_type);
		lexindex++;
	}else{
		// Unknown Error!!
		printf("Unknown Error!!\n");
	}
}

// Recursive method to separate each token based on punctuation
//(separation by spaces handled by scanning input)
void separateTokens(char string[]){
	char puncts[] = ".,;:=+-/*><()";
	//printf("%s \n", string);
	int s = strlen(string);
	char string1[s+1];
	char string2[s+1];
	char data[s+1];

	strcpy(data, string);
	int j;

	// Find the first character that is a punctuation marker
	j = strcspn(data, puncts);

	
	// If the string is empty, we do nothing
	if(j == 0 && s == 0){
		return;
	}
	// If the string has no punctuation, we just insert it
	else if(j == s){
		//printf("inserting: %s  j:%d s:%d\n", data, j, s);
		insertLexeme(data);
		return;
	}

	int left = s - j;
	// Break the string apart. The identifier before the punctuation stays in the current node
	// Everything in the original string starting with the first punctuation
	if(j == 0 && ispunct(data[0])){      // first char is punctuation
		//check for multi punct symbol
		if((data[0] == '>' && data[1] == '=') || ((data[0] == '<') && (data[1] == '>' || data[1] == '=')) || (data[0] == ':' && data[1] == '=')|| (data[0] == '/' && data[1] == '*') || (data[0] == '*' && data[1] == '/')){
			strncpy(string1, data, j+2);         // Copy the original token to a temp string
			memmove(data+0, data+j+2, left);	// Move the rest of the token to a new string
			strncpy(string2, data, left);
			string1[j+2] = '\0';           		// nullify everything after the token we want in the original

		}
		else{   // single punct symbol
			strncpy(string1, data, j+1);         // Copy the original token to a temp string
			memmove(data+0, data+j+1, left);	// Move the rest of the token to a new string
			strncpy(string2, data, left);
			string1[strlen(string1)-2] = '\0';           		// nullify everything after the token we want in the original
		}
	}else{
		strncpy(string1, data, j);         	// Copy the original token to a temp string
		memmove(data+0, data+j, left);		// Move the rest of the token to a new string
		strncpy(string2, data, left);
		string1[j] = '\0';           		// Nullify everything after the token we want in the original
	}
	//if(strcmp(string1, " ") != 0)
		//printf("%s ", string1);
	//if(strcmp(string2, " ") != 0)
		//printf("%s\n", string2);
	//printf(" s:%d j:%d\n", s, j);
	insertLexeme(string1);
 	separateTokens(string2);
}

// Loop through the lexeme list, categorizing each entry
int examineToken(char string[]){
	int punct_num;
	int reserved;
	int spec;
	char puncts[] = ".,;:=+-/*><()";
	//printf("examining string: %s \n", string);

	spec = check_specsym(string);
	if(spec == lcommentsym){
		comm_flag = 1;
		return lcommentsym;
	}
	else if(spec == rcommentsym){
		comm_flag = 0;
		return rcommentsym;
	}

	if(!comm_flag){
		if(spec > 0){
			return spec;
		}
		else{
			reserved = check_reserved(string);
			
			if(reserved > 0)
				return reserved;
			else if(isalpha(string[0])){
				if(strlen(string) <= MAX_CHARS)
					return identsym;
				else
					return 0;
			}else if(isdigit(string[0])){
				if(strlen(string) <= MAX_DIGS)
					return numbersym;
				else
					return 0;
			}else
				return 0;
		}
	}else
		return 0;
}

// Checks for reserved special symbols. Returns the corresponding internal token value
int check_specsym(char* t){
	enum SYMLIST sym;

	if(t[0] == '+')
        return plussym;     	// plussym
    else if(t[0] == '-')
        return minussym;   		// minussym
    else if(t[0] == '*'){

		if(t[1] == '/')
			return rcommentsym;	// rcommentsym
		else
        	return multsym;		// multsym
	}
    else if(t[0] == '/'){
		if(t[1] == '*')
			return lcommentsym; // lcommentsym
		else
        	return slashsym;	// slashsym
	}else if(t[0] == '=')
        return eqlsym;       	// eqlsym
	else if(t[0] == '<'){
		if(t[1] == '>')
    		return neqsym;  	// neqsym
		if(t[1] == '=')
    		return leqsym;  	// leqsym
		else
    		return lessym;  	// lessym
	}else if(t[0] == '>'){
		if(t[1] =='=')
        	return geqsym;  	// geqsym
	    else
	        return gtrsym;  	// gtrsym
	}else if(t[0] == '(')
        return lparentsym;  	// lparentsym
    else if(t[0] == ')')
        return rparentsym;  	// rparentsym
    else if(t[0] == ',')
        return commasym;    	// commasym
    else if(t[0] == '.')
        return periodsym;		// periodsym
    else if(t[0] == ';')
        return semicolonsym;	// semicolonsym
	else if(t[0] == ':' && t[1] == '=')
		return becomessym;		// becomessym
    else
        return 0;
}

// Checks for reserved words. Returns the corresponding internal token value
int check_reserved(char* t){

	if(strcmp(t , "null") == 0)
        return nulsym;			// nulsym
	else if(strcmp(t , "odd") == 0)
        return oddsym;			// oddsym
	else if(strcmp(t , "begin") == 0)
        return	beginsym;		// beginsym
    else if(strcmp(t , "end") == 0)
        return endsym;    		// endsym
    else if(strcmp(t , "if") == 0)
        return ifsym;     		// ifsym
    else if(strcmp(t , "then") == 0)
        return thensym;     	// thensym
	else if(strcmp(t , "while") == 0)
        return whilesym;    	// whilesym
	else if(strcmp(t , "do") == 0)
        return dosym;       	// dosym
	else if(strcmp(t , "call") == 0)
        return callsym;     	// callsym
    else if(strcmp(t , "const") == 0)
        return constsym;    	// constsym
	else if(strcmp(t , "int") == 0)
        return intsym;      	// intsym
    else if(strcmp(t , "procedure") == 0)
        return procsym;     	// procsym
    else if(strcmp(t , "out") == 0)
        return outsym;			// outsym
    else if(strcmp(t , "in") == 0)
        return insym;  			// insym
    else if(strcmp(t , "else") == 0)
        return elsesym;     	// elsesym
	else if(strcmp(t , "write") == 0)
        return writesym;     	// writesym
	else if(strcmp(t , "read") == 0)
        return readsym;     	// readsym
    else
        return 0;
}

// Takes in a string of digits and converts it to a corresponding int
int parseNumber(char string[]){
	/**
	*   Take in a string, get length. For each position in string, get digit
	*   and add multiplied by correct base 10 to total
	*/
	int i;
	int digit;
	double number = 0;
	int len;
	len = strlen(string);
	
	for(i = len-1; i >= 0; i--){
		digit = string[i] - '0';
		//printf("string[i]: %d \n", string[i]);
		//printf("digit: %d \n", digit);
		number += digit*pow(10, (len-1)-i);
	}
	//printf("number: %lf", number);
	return (int)number;
}

// Prints the list of Lexemes
void printList(FILE* fout){
	int i;
	FILE* fparin;
	fparin = fopen("T2_parserin.txt", "w");

	fprintf(fout, "\nLexeme List:\n");
	printf("\nLexeme List:\n");

	for(i = 0; i < lexindex; i++){
		if(lexeme[i]->token_type == numbersym){
			printf("%d %d ", lexeme[i]->token_type, lexeme[i]->number);
			fprintf(fout, "%d %d ", lexeme[i]->token_type, lexeme[i]->number);
			fprintf(fparin, "%d %d ", lexeme[i]->token_type, lexeme[i]->number);
		}else if(lexeme[i]->token_type == identsym){
			printf("%d %s ", lexeme[i]->token_type, lexeme[i]->lexeme);
			fprintf(fout, "%d %s ", lexeme[i]->token_type, lexeme[i]->lexeme);
			fprintf(fparin, "%d %s ", lexeme[i]->token_type, lexeme[i]->lexeme);
		}else{
			printf("%d ", lexeme[i]->token_type);
			fprintf(fout, "%d ", lexeme[i]->token_type);
			fprintf(fparin, "%d ", lexeme[i]->token_type);
		}
	}
	
	fclose(fparin);
}

// Prints the Lexeme Table
void printTable(FILE* fout){
	int i;
	
	fprintf(fout, "\n\nLexeme Table:\n");
	fprintf(fout, "lexeme       token type\n");
	printf("\n\nLexeme Table:\n");
	printf("lexeme       token type\n");

	for(i = 0; i < lexindex; i++){
		if(lexeme[i]->token_type == numbersym){
			printf("%-11d %-d\n", lexeme[i]->number, lexeme[i]->token_type);
			fprintf(fout, "%-11d %-d\n", lexeme[i]->number, lexeme[i]->token_type);
		}else{
			printf("%-11s %-d\n", lexeme[i]->lexeme, lexeme[i]->token_type);
			fprintf(fout, "%-11s %-d\n", lexeme[i]->lexeme, lexeme[i]->token_type);
		}
	}
	
	//fprintf(fout, "\n");
	//printf("\n");
}
