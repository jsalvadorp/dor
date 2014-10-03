#include "ast.hpp"

namespace ast {

Ptr<AVoid> voidv;

Ptr<AVoid> AVoid::value() {
    if(voidv) return voidv;
    return voidv = newPtr<AVoid>();
} 


/*Ptr<List> &append(Ptr<List> &l, Ptr<Atom> a) {
    while(l) l = l->tail;
    
    l = newPtr<List>(new List(a, nullptr));
    l = l->tail;
    
    return l;
}*/

Ptr<List> *append(Ptr<List> *l, Ptr<Atom> a) {
    while(*l) l = &(*l)->tail;
    
    *l = newPtr<List>(a, nullptr);
    l = &(*l)->tail;
    
    return l;
}

Ptr<List> *appendList(Ptr<List> *l, Ptr<List> a) {
    while(*l) l = &(*l)->tail;
    
    *l = a;
    //l = &(*l)->tail;
    while(*l) l = &(*l)->tail;
    
    return l;
}

Ptr<List> list(std::initializer_list<Ptr<Atom> > l) {
    Ptr<List> new_list, *dest = &new_list;
    
    for(Ptr<Atom> a : l) {
        dest = append(dest, a);
    }
    
    return new_list;
}

Ptr<List> listPlus(std::initializer_list<Ptr<Atom> > l) {
    Ptr<List> new_list, *dest = &new_list;
    auto it = l.begin();
    
    for(; it != l.end() - 1; it++) {
        dest = append(dest, *it);
    }
    
    assert(it != l.end());
    assert((*it)->type() == ListType);
    *dest = std::dynamic_pointer_cast<List, Atom>(*it);
    
    return new_list;
}


}
