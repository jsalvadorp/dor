#include "ast.hpp"

namespace ast {

AVoid *voidv;

AVoid *AVoid::value() {
    if(voidv) return voidv;
    return voidv = new AVoid();
} 


List **append(List **l, Atom *a) {
    while(*l) l = &(*l)->tail;
    
    *l = new List(a, nullptr);
    l = &(*l)->tail;
    
    return l;
}

}
