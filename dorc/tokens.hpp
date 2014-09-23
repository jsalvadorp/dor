
#pragma once

#include <string>

enum token_t {
    TEOF = 0,
    NL,
    TAB,
    SPACE,
    TOP,
    TID,
    TCHAR,
    TSTRING,
    TFLOAT,
    TDINT,
    TXINT,
    TRINT,
    
    TINVALID
};

const char* toknames[] = {
    "",
    "NL",
    "TAB",
    "SPACE",
    "TOP",
    "TID",
    "TCHAR",
    "TSTRING",
    "TFLOAT",
    "TDINT",
    "TXINT",
    "TRINT"
};

struct Token {
    token_t token;
    int line;
    int column;
    std::string lexeme;
    
    Token() : token(TINVALID), line(-1), column(-1), lexeme("") {}
    Token(int t, int l, int c, const char *s) : token((token_t)t), line(l), column(c), lexeme(s ? s : "") {}
};

Token next_token();


