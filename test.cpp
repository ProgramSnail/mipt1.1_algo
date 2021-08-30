#include <iostream>
#include <vector>
#include <string>

#include "smart_pointers.h"

int main() {
    SharedPtr<int> a(new int());
    std::vector<SharedPtr<std::string> > x(1000, new std::string(" "));
    *x[100] = "sxsaxsax";
    for (size_t i = 1; i < x.size(); ++i) {
        x[x.size() - i] = x[x.size() - i - 1];
    }
    auto c = a;
    auto b = a;
    *c = 3;
    a.reset(new int());
    *a = 2;
    c = a;
    a.reset();
    *c = 5;
    *x[5] = " ";
}
