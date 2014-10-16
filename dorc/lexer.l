
%option yylineno

%{

#include <iostream>
#include <stack>
#include <queue>
#include "../dorc/tokens.hpp"
#include "../dorc/ast.hpp"

extern "C" token_t yylex();

#define YY_DECL token_t yylex()

const char* toknames[] = {
    "UNK",
    "TEOF",
    "NL",
    "TAB",
    "SPACE",
    "TINVALID",
    "TINDENT",
    "TDEDENT",
    "TLINE",
    "TBACKSLASH",
    "TOP",
    "TID",
    "TCHAR",
    "TSTRING",
    "TFLOAT",
    "TDINT",
    "TXINT",
    "TRINT",
    
    "TLPAREN",
    "TRPAREN",
    "TLBRACKET",
    "TRBRACKET",
    "TLBRACE",
    "TRBRACE"
};

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


ffrac       \.[0-9][0-9_]*
fexp        [Ee][+-]?[0-9]+
opc         [-!#%&*+./<=>?@\^|~:]
delim       {opc}|[\{\}\[\]\(\)\n\t\v \\,;\$\"\']


%%
\\\n                                    {} /*ignore escaped newline*/ 
\#[^\n]*(\n)                    		{return NL;} /* also eof?  */
\n                                      {return NL;}
\r                                      {}
\t                                      {return TAB;}
" "                                     {return SPACE;}
\"([^\"]|\\.)*\"                        {return TSTRING;}
\$(.|\\.)                              		 	{return TCHAR;}
[a-zA-Z_][a-zA-Z0-9_]*                          {return TID;}
[0-9][0-9_]*({ffrac}|{fexp}|{ffrac}{fexp})      {return TFLOAT;}
[0-9][0-9_]*                                    {return TDINT;}
0x[0-9a-fA-F][0-9a-fA-F_]*                      {return TXINT;}
[0-9]+r[0-9A-Za-z][0-9A-Za-z_]*                 {return TRINT;} 
    /*reemplazar la anterior por muchas reglas, una para cada
    base para asegurar que sea correcto el numero*/
{opc}+                                  {return TOP;}
\`[a-zA-Z_][a-zA-Z0-9_]*                {return TOP;}
\,						                {return TOP;}
<<EOF>>                                 {return TEOF;}
\\                                      {return TBACKSLASH;}
\(                                     	{return TLPAREN;}
\)                                     	{return TRPAREN;}
\[                                     	{return TLBRACKET;}
\]                                     	{return TRBRACKET;}
\{                                     	{return TLBRACE;}
\}                                     	{return TRBRACE;}

%%

bool line_start;
bool using_tabs;
int indent_level;
int column;
bool no_indent, no_line;

token_t indentation_type;

// store the indentation levels. push on indent, pop on dedent.
std::stack<int> indentation; // should be a vector, to contract

// store the grouping symbol levels, each carries a strict minimum 
// indentation for their contents
std::stack<std::pair<token_t, int> > groupers;

std::queue<Token> pending_tokens;


#define FETCH_TOKEN (t_code = yylex(), t = Token(t_code, yylineno, column, yytext));

#define CHECK_GROUPER \
	if(IS_LGROUPER(t.token)) { \
			no_indent = true; \
			no_line = true; \
			groupers.push(std::make_pair( \
				OPPOSITE_GROUPER(t.token), indentation.top())); \
		\
        } else if(IS_RGROUPER(t.token)) { \
			no_indent = false; \
			no_line = false; \
			if(groupers.top().first != t.token) \
				assert(!"Unexpected closing grouping symbol"); \
			\
			while(groupers.top().second < indentation.top()) { \
				indentation.pop(); \
			} \
			assert(indentation.top() == groupers.top().second); \
			groupers.pop(); \
        } else no_line = false;

Token nextToken() {
	token_t t_code;
	Token t;
    if(!pending_tokens.empty()) {
        t = pending_tokens.front();
        pending_tokens.pop();
        
        CHECK_GROUPER;
        
        assert(t.token != SPACE && t.token != NL && t.token != TAB);
		//std::cout << "tok  --  " << t.lexeme << std::endl;
        return t;
    }

    FETCH_TOKEN;
    
    while(!line_start && (t.token == SPACE || t.token == TAB)) {
		FETCH_TOKEN;
	}
    
    if(t.token == NL) {
		line_start = true;
	}
	
    while(line_start) {
        int new_ind = 0;
        bool valid_ind = true;
        token_t curr_indentation_type = indentation_type;
        while(IS_INDENT(t.token)) {
            if(!curr_indentation_type) {
				curr_indentation_type = t.token;
            }
            if(curr_indentation_type != t.token)
                valid_ind = false;
            
            new_ind++;
            FETCH_TOKEN;
        }
        
        line_start = false;
        
        if(t.token == NL) {
			new_ind = 0;
			//indentation_type = TUNKNOWN; // unknown indentation type
			line_start = true;
			FETCH_TOKEN;
        } else if(IS_RGROUPER(t.token)) { //if (t.token == TEOF) {
			if(!valid_ind) 
			    assert(!"Mixing spaces and tabs at the start of a line");
				indentation_type = curr_indentation_type;
				
			assert(new_ind >= groupers.top().second);
			//CHECK_GROUPER;
			
			//return t;
        } else {
			assert(new_ind > groupers.top().second); // meets minimum
			
			if(!valid_ind) 
			    assert(!"Mixing spaces and tabs at the start of a line");
				indentation_type = curr_indentation_type;
			    
            if(no_line) { // push indentation without emitting
				indentation.push(new_ind);
				no_line = false;
				no_indent = false;
            } else if(new_ind == indentation.top()) { // LINE
                pending_tokens.push(t);
                return Token(TLINE, yylineno - 1, column, "");
            } else if(no_indent) { // push indentation without emitting
				indentation.push(new_ind);
				pending_tokens.push(t);
				no_line = false;
				no_indent = false;
                return Token(TLINE, yylineno - 1, column, "");
            } else if(new_ind < indentation.top()) { // DEDENT
                indentation.pop();
                
                while(new_ind < indentation.top()) { // any extra levels of dedentation
					indentation.pop();
					pending_tokens.push(Token(TDEDENT, yylineno, column, ""));
				}
				
                if(new_ind != indentation.top()) {
					assert(!(
						"Decrease of indentation to invalid level"));
                }
                    
                pending_tokens.push(t); // non-tab token
                return Token(TDEDENT, yylineno - 1, column, "");
            } else if(new_ind > indentation.top()) { // INDENT
                indentation.push(new_ind);
                pending_tokens.push(t);
                return Token(TINDENT, yylineno - 1, column, "");
            }
        }
        
    }
    
    //while(t.token == SPACE || t.token == TAB) FETCH_TOKEN;
    assert(t.token != NL && t.token != SPACE && t.token != TAB);
    
    CHECK_GROUPER;
    
    assert(t.token != SPACE && t.token != NL && t.token != TAB);
    //std::cout << "tok  --  " << t.lexeme << std::endl;
    return t;
}

void initLexer() {
	line_start = true;
	no_indent = false;
	no_line = false;
	indentation.push(-1);
	indentation.push(0);
	groupers.push(std::make_pair(TEOF, -1));
}


