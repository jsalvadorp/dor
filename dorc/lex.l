
%option yylineno

%{

#include <iostream>
#include "tokens.hpp"
#include "ast.hpp"

extern "C" int yylex();

/*
int isdelim(int c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r'
		|| c == '(' || c == ')' || c == '\\'
		|| c == '[' || c == ']'
		|| c == '{' || c == '}' || c == '.' || c == EOF
		|| c == ',' || c == '\'' || c == '\"' || c == '$';
} */

%}

    /* http://www.kylheku.com/cgit/txr/tree/parser.l  [-!#$%&*+./<=>?@\^|~] */


ffrac       \.[0-9]+
fexp        [Ee][+-]?[0-9]+
opc			[-!#%&*+./<=>?@\^|~]
delim		{opc}|[\{\}\[\]\(\)\n\t\v \\,;\$\"\']


%%

\#[^\n]*(\n)                    
\n                                      {return NL;}
\r                                      {}
\t                                      {return TAB;}
" "                                     {return SPACE;}
\$(.|\\.)                               {return TCHAR;}
\"([^\"]|\\.)*\"                        {return TSTRING;}
[a-zA-Z_][a-zA-Z0-9_]*                  		{return TID;}
[0-9_]+({ffrac}|{fexp}|{ffrac}{fexp})	   		{return TFLOAT;}
[0-9_]+                                 		{return TDINT;}
0x[0-9a-fA-F]+                         	 		{return TXINT;}
[0-9]+r[0-9A-Za-z_]+                    		{return TRINT;} 
    /*reemplazar lo anterior por muchas reglas, una para cada
    base para asegurar que sea correcto el numero*/
{opc}+                    				{return TOP;}
<<EOF>>									{return TEOF;}

%%



Token next_token() {
	int t = yylex();
    return Token(t, yylineno, 0, yytext);
}

int main() {
	Token tok;
    while(tok = next_token(), tok.token) {
        std::cout << tok.line << " : " << toknames[tok.token] << " :: "  << tok.lexeme << std::endl;
    }

}

/*
int main() {
    int tok;
    
    while(tok = yylex()) {
        std::cout << yylineno << " : " << toknames[tok] << " :: "  << yytext << std::endl;
    }

}/**/
