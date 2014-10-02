#include <unordered_map>
#include <algorithm>
#include <cstdlib>
#include <stack>
#include "ast.hpp"
#include "tokens.hpp"
#include "parser.hpp"

using namespace ast;

namespace parser {
    
int escapeChar(char c) {
	switch(c) {
	case 'n': return '\n'; // no breaks
	case 'r': return '\r';
	case 't': return '\t';
	case 'a': return '\a';
	case 'b': return '\b';
	case 'f': return '\f';
	case 'v': return '\v';
	case 's': return ' ';
	// default (cases \, ", ' or others), c is already the escaped char
	}
	
	return c;			
}

std::string parseString(std::string &lexeme) {
    std::string new_str;
    new_str.reserve(lexeme.size());
    
    for(auto it = lexeme.begin() + 1; it != lexeme.end() - 1; it++) {
        new_str.push_back(*it == '\\' ? escapeChar(*++it) : *it);
    }
    
    return std::move(new_str);
} 

i64 parseDecInt(std::string &lexeme) {
    lexeme.erase(std::remove(lexeme.begin(), lexeme.end(), '_'), lexeme.end());
    return std::stol(lexeme, nullptr, 10);
}

i64 parseHexInt(std::string &lexeme) {
    lexeme.erase(std::remove(lexeme.begin(), lexeme.end(), '_'), lexeme.end());
    return std::stol(lexeme, NULL, 16);
}

i64 parseRadixInt(std::string &lexeme) {
    lexeme.erase(std::remove(lexeme.begin(), lexeme.end(), '_'), lexeme.end());
    std::size_t pos;
    long radix = stol(lexeme, &pos, 10);
    return strtol(lexeme.c_str() + pos + 1 /* skip r */, NULL, radix);
}

double parseFloat(std::string &lexeme) {
    lexeme.erase(std::remove(lexeme.begin(), lexeme.end(), '_'), lexeme.end());
    return std::stod(lexeme.c_str(), nullptr);
}

Token t;

#define MATCH(tok) \
    if(t.token != (tok)) \
        assert(!"Expected another token"); \
    else t = nextToken();

//Atom *parseGroup(bool force_list = false);



// operators are symbols
// Sym("") --> GLUE
// Sym(-1) --> (none)





// ... LINE
// ... INDENT ... (LINE ...)* DEDENT

// associativity:
// Left:         a * b * c   ------>  (* (* a b) c)
// Left          a + b - c   ------>  (- (+ a b) c)    (same prec)
// Right:        a -> b -> c ------>  (-> a (-> b c))
// Right:        a -> b -> c ------>  (-> a (-> b c))
// Neutral:      a # b # c   ------>  (# a b c)

// Neutral associativity is for variadic operators
// neutral operators of the same precedence are not associative
// between each other, only with themselves

enum assoc_t {Left = 1, Right, Neutral, None};
typedef int order_t;

std::unordered_map<Sym, std::pair<order_t, assoc_t> > infix_ops;
/*
.				l (or v?)
	* / %			l
	+ -				l
	< > <= >= == !=	l
	|| &&			l
	,				v
	<< >>			
	->				r (or v? should be v for simplicity...)
	=>				r
	|				v
	= <-			r
	:=				r
	::				n whole line types
    * */

void initInfixOps() {
    infix_ops = {
        {".",       {10, Left}},
        {"@",       {10, Left}},
        
        {"",        {20, Neutral}}, // GLUE
        
        {"**",      {30, Right}},
        
        {"*",       {40, Left}},
        {"/",       {40, Left}},
        {"%",       {40, Left}},
        
        {"+",       {50, Left}},
        {"-",       {50, Left}},
        
        {"<",       {60, Left}},
        {">",       {60, Left}},
        {"<=",      {60, Left}},
        {">=",      {60, Left}},
        
        {"==",      {70, Left}},
        {"!=",      {70, Left}},
        
        {"&&",      {80, Left}},
        
        {"||",      {90, Left}},
        
        {"<:",      {100, Neutral}},
        {":>",      {100, Neutral}},
        
        {"|",       {110, Neutral}},
        
        {"->",      {120, Right}},
        
        {"=>",      {130, Right}},
        
        {":",       {140, Left}},
        
        {"<-",      {150, Right}},
        {"=",       {150, Right}},
        {":=",      {150, Right}},
        
        {",",       {160, Neutral}},
    };
}

Sym parseInfixOp();

Atom *parseExpr() {
    Atom *a = nullptr;
    
    if(t.token == TID) {
        a = atom(Sym(t.lexeme), t.line, t.column);
    } else if(t.token == TCHAR) {
        a = atom(t.lexeme[0] == '\\' ? escapeChar(t.lexeme[1]) : t.lexeme[0]);
    } else if(t.token == TSTRING) {
        a = atom(parseString(t.lexeme), t.line, t.column);
    } else if(t.token == TDINT) {
        a = atom(parseDecInt(t.lexeme), t.line, t.column);
    } else if(t.token == TXINT) {
        a = atom(parseHexInt(t.lexeme), t.line, t.column);
    } else if(t.token == TRINT) {
        a = atom(parseRadixInt(t.lexeme), t.line, t.column);
    } else if(t.token == TFLOAT) {
        a = atom(parseFloat(t.lexeme), t.line, t.column);
    }
    
    if(a) {
        t = nextToken();
        return a;
    }
    
    assert(t.token != TOP);
    
    if(t.token == TLPAREN) {
        t = nextToken();
        Atom *a;
        
        if(t.token == TOP) { // parenthesized op-expr (+)  (*)
            a = atom(parseInfixOp());
        } else a = parseGroup();
        MATCH(TRPAREN);
        
        if(a == nullptr) a = AVoid::value();
        
        return a;
    }
    
    assert(false);
}

#define GLUE Sym("")

Sym parseInfixOp() {
    if(t.token == TOP) {
        Sym s = t.lexeme[0] == '`' ? Sym(t.lexeme.c_str() + 1) : Sym(t.lexeme);
        t = nextToken();
        //assert(infix_ops.find(s) != infix_ops.end());
        return s;
    } else return GLUE; // GLUE
}

#define ENDS_LINE(t) (IS_RGROUPER(t) || (t) == TLINE || (t) == TDEDENT)

#define prec(op) (-infix_ops[op].first)
#define assoc(op) (infix_ops[op].second)

void helper(
    std::stack<Sym> &operators,
    std::stack<Atom *> &operands,
    std::stack<Sym> &operand_heads,
    std::stack<List *> &operand_dests);

Atom *parseLine() {
    //std::cout << "LINE start " 
    //    << toknames[t.token]<< "="<<t.lexeme<< std::endl;
    std::stack<Atom *> operands;
    std::stack<Sym> operators;
    std::stack<Sym> operand_heads;
    std::stack<List *> operand_dests;
    
    //a = parseExpr();
    operands.push(parseExpr());
    operand_dests.push(nullptr);
    operand_heads.push(Sym(-1));
    
    Sym op = GLUE;
    
    while(!ENDS_LINE(t.token) && t.token != TINDENT) {
        op = parseInfixOp();
        if(infix_ops.find(op) == infix_ops.end())
            assert(!"Undefined infix operator! Precedence and associativity not declared.");
        
        if(!ENDS_LINE(t.token) && t.token != TINDENT) {
            while(!operators.empty() && prec(operators.top()) >= prec(op)) {
                helper(operators, operands, operand_heads, operand_dests);
            }
            
            operators.push(op);
            
            operands.push(parseExpr());
            operand_dests.push(nullptr);
            operand_heads.push(Sym(-1));
            op = GLUE;
        }
    }
    
    if(t.token == TINDENT) {
        t = nextToken();
        
        do {
            while(!operators.empty() && prec(operators.top()) >= prec(op)) {
                helper(operators, operands, operand_heads, operand_dests);
            }
            
            operators.push(op);
            
            operands.push(parseLine());
            operand_dests.push(nullptr);
            operand_heads.push(Sym(-1));
            op = GLUE;
        } while(t.token != TDEDENT && !IS_RGROUPER(t.token));
        if(t.token == TDEDENT) t = nextToken();
    } else {
        if(op != GLUE)
        assert(!"Hanging operator at the end of a line");
    }
    
    if(t.token == TLINE) t = nextToken();
    
    while(!operators.empty()) {
        helper(operators, operands, operand_heads, operand_dests);
    }
    
    Atom *a = operands.top();
    operands.pop();
    
    
    // checking infix definitions:
    if(a->type() == ListType) {
        List *exp = (List *)a;
        if(exp->head->type() == SymType 
            && exp->head->get_Sym() == Sym("infixr")) {
            // found an infixr declaration. must be of the form
            // "infixr op prec" where op is still not defined as an op
            
            assert(exp->tail);
            exp = exp->tail;
            
            assert(exp->head->type() == SymType);
            Sym new_op = exp->head->get_Sym();
            assert(infix_ops.find(new_op) == infix_ops.end());
            
            assert(exp->tail);
            exp = exp->tail;
            
            assert(exp->head->type() == I64Type);
            i64 prec = exp->head->get_i64();
            
            assert(!exp->tail);
            
            infix_ops[new_op] = {prec, Right};
        } else if(exp->head->type() == SymType 
            && exp->head->get_Sym() == Sym("infixl")) {
            // found an infixl declaration. must be of the form
            // "infixl op prec" where op is still not defined as an op
            
            assert(exp->tail);
            exp = exp->tail;
            
            assert(exp->head->type() == SymType);
            Sym new_op = exp->head->get_Sym();
            assert(infix_ops.find(new_op) == infix_ops.end());
            
            assert(exp->tail);
            exp = exp->tail;
            
            assert(exp->head->type() == I64Type);
            i64 prec = exp->head->get_i64();
            
            assert(!exp->tail);
            
            infix_ops[new_op] = {prec, Left};
        }
    }
    
    
    return a;
}




void helper(
    std::stack<Sym> &operators,
    std::stack<Atom *> &operands,
    std::stack<Sym> &operand_heads,
    std::stack<List *> &operand_dests) {
    
    Atom *operand2 = operands.top(); 
    operands.pop();
    operand_heads.pop();
    operand_dests.pop();
    
    Atom *operand1 = operands.top();
    Sym head = operand_heads.top();
    List *dest = operand_dests.top();
    operands.pop();
    operand_heads.pop();
    operand_dests.pop();
    
   
    
    // handle None associtivity
    
    // merge operation:  (+ a b) + c --->  (+ (+ a b) c)
    if(head.valid() 
        && head == operators.top() && assoc(operators.top()) != Left) {
        
							
		
        
        // variadic or neutral associative: append op2 after dest.
        if(assoc(operators.top()) == Neutral) {
            dest = dest->tail = new List(operand2, nullptr);
            operands.push(operand1); // now with op2 included!
            operand_dests.push(dest);
            operand_heads.push(operators.top());
        }
        
        // right associ
        else if(assoc(operators.top()) == Right) {
            Ptr<Atom> dest_head = dest->head;
            List *new_dest;
            //dest->head = nullptr;
            /*dest->head = new List(
                operators.top(),
                new List(
                    dest_head,
                    (new_dest = new List(
                        operand2, 
                        nullptr))));
            dest = new_dest;*/
            
            dest->head = new List(
                operators.top(),
                new List(
                    dest->head,
                    new_dest = new List(
                        operand2, 
                        nullptr)));
            
            operands.push(operand1);
            operand_dests.push(new_dest);
            operand_heads.push(operators.top());
        }
    }
    
    // left associative or first application of operator
    else { 
        List *tmp, *exp = new List(
            operand1,
            tmp = new List(
                operand2,
                nullptr));
        
        if(operators.top() != GLUE)
            exp = new List(operators.top(), exp); // add operator as head
        
        dest = assoc(operators.top()) == Left ? nullptr : tmp;
        operands.push(exp);
        operand_dests.push(dest);
        operand_heads.push(operators.top());
    }
    
    operators.pop();
}

Atom *parseGroup(bool force_list) {
    //std::cout << "groupppp" << std::endl;
    if(IS_RGROUPER(t.token)) return nullptr;
    
    Atom *ret = nullptr;
    List **dest = force_list ? (List **)&ret : nullptr;
    
    while(!IS_RGROUPER(t.token)) {
        Atom *a = parseLine();
        
        //std::cout << "ALINE" << std::endl;
        //a->dump(0);
        
        //assert(*dest == nullptr);
        
        if(dest) {
            dest = append(dest, a);
            
            assert(ret != ((List *) ret)->head);
        } else if(!ret) {
            ret = a;
        } else {
            ret = new List(ret, nullptr);
            dest = &((List *)ret)->tail;
            dest = append(dest, a);
        }
    }
    
    return ret;
}

void initParser() {
    Sym("");
    //std::cout << "glue is " << Sym("").code << endl;
    initInfixOps();
    
    t = nextToken();
    if(t.token == TLINE) t = nextToken();
}

}


int main() {
    initLexer();
	parser::initParser();
    
    /*
    do {
		tok = nextToken();
		assert(tok.token != SPACE && tok.token != NL && tok.token != TAB);
        std::cout << tok.line << " :- " << toknames[tok.token] << " :: "  << tok.lexeme << std::endl;
    } while(tok.token != TEOF);/**/
	
	ast::List *program = (List *)parser::parseGroup(true);
    
    program->dump(0);
    std::cout << endl;
}
