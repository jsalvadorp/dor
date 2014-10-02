
#pragma once

#include <string>

enum token_t {
    TUNKNOWN = 0,
    TEOF,
    NL,
    TAB,
    SPACE,
    TINVALID,
    TINDENT,
    TDEDENT,
    TLINE,
    TBACKSLASH,
    
    TOP,
    TID,
    TCHAR,
    TSTRING,
    TFLOAT,
    TDINT,
    TXINT,
    TRINT,
    
    TLPAREN,
    TRPAREN,
    TLBRACKET,
    TRBRACKET,
    TLBRACE,
    TRBRACE,
};

#define IS_INDENT(c) ((c) == SPACE || (c) == TAB)
#define IS_LGROUPER(c) \
	((c) == TLPAREN  || (c) == TLBRACE || (c) == TLBRACKET)
#define IS_RGROUPER(c) \
	((c) == TRPAREN  || (c) == TRBRACE || (c) == TRBRACKET || (c) == TEOF)
#define OPPOSITE_GROUPER(c) \
	((c) == TLPAREN ? TRPAREN \
	:(c) == TLBRACKET ? TRBRACKET \
	:(c) == TLBRACE ? TRBRACE \
	: TUNKNOWN)

extern const char* toknames[];

struct Token {
    token_t token;
    int line;
    int column;
    std::string lexeme;
    
    Token() : token(TINVALID), line(-1), column(-1), lexeme("") {}
    Token(int t, int l, int c, const char *s) : token((token_t)t), line(l), column(c), lexeme(s ? s : "") {
        //std::cout << "made token " << toknames[token] << " = {" << lexeme <<"}"<< std::endl;
    }
};

void initLexer();
Token nextToken();




