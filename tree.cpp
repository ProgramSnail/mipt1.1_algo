#include <iostream>
#include <ostream>

template<typename T>
class BinaryTree {
protected:
    struct Vertex {
        T value;
        Vertex* left = nullptr;
        Vertex* right = nullptr;
        Vertex* parent = nullptr;
        size_t sz = 1;
        Vertex(T value, Vertex* parent = nullptr, size_t sz = 1) 
            : value(value), parent(parent), sz(sz) {}
    };
    Vertex* root = nullptr;
    size_t getSz(Vertex* v) const {
        if (!v) {
            return 0;
        }
        return v->sz;
    }
    void update(Vertex* v) {
        if (!v) {
            return;
        }
        v->sz = 1 + getSz(v->left) + getSz(v->right);
    }
    void updateUp(Vertex* v) {
        if (!v) {
            return;
        }
        update(v);
        updateUp(v->parent);
    }
    void printTree(Vertex* v, std::ostream& out) const {
        if (!v) {
            return;
        }
        printTree(v->left, out);
        out << (v->left ? "< " : "") << v->value << ':' << v->sz << ':' << (v->parent ? v->parent->value : 0) << (v->right ? " >" : "") << ' ';
        printTree(v->right, out);
    }
    Vertex* getExtremum(Vertex* v, bool isMinimum) const {
        if (isMinimum ? v->left : v->right) {
            return getExtremum((isMinimum ? v->left : v->right), isMinimum);
        }
        return v;
    }
    Vertex* search(Vertex* v, T key) const {
        if (key == v->value) {
            return v;
        }
        if (key < v->value) {
            if (v->left) {
                return search(v->left, key);
            }
        } else {
            if (v->right) {
                return search(v->right, key);
            }
        }   
        return v;
    }
    Vertex* searchK(Vertex* v, size_t k, bool isFromMinimum) const {
        size_t sz =  getSz(isFromMinimum ? v->left : v->right);
        if (k <= sz) {
            return searchK(isFromMinimum ? v->left : v->right, k, isFromMinimum);
        } 
        if (k > sz + 1) {
            return searchK(isFromMinimum ? v->right : v->left, k - sz - 1, isFromMinimum);
        }
        return v;
    }
    void updateParent(Vertex* v, Vertex* p) {
        if (!v) {
            return;
        }
        v->parent = p;
    }
    void updateChildrenParent(Vertex* v) {
        if (!v) {
            return;
        }
        updateParent(v->left, v);
        updateParent(v->right, v);
    }
    void vertexSwap(Vertex* u, Vertex* v) {
        if (u->parent) {
            (u->parent->left == u ? u->parent->left : u->parent->right) = v;
        } else {
            root = v;
        }
        if (v->parent) {
            (v->parent->left == v ? v->parent->left : v->parent->right) = u;
        } else {
            root = u;
        }
        std::swap(u->left, v->left);
        std::swap(u->right, v->right);
        std::swap(u->parent, v->parent);
        std::swap(u->sz, v->sz);
        updateChildrenParent(u);
        updateChildrenParent(v);
    }
    Vertex* rotate(Vertex* v, bool isLeft) {
        if (!(isLeft ? v->right : v->left)) {
            return v;
        }
        Vertex *u = (isLeft ? v->right : v->left);
        vertexSwap(u, v);
        std::swap(u, v);
        if (isLeft) {
            std::swap(v->right->left, v->right->right);
            std::swap(v->right, v->left);
            std::swap(v->right, v->left->left);
            updateParent(v->right, v);
            updateParent(v->left->left, v->left);
        } else {
            std::swap(v->left->left, v->left->right);
            std::swap(v->right, v->left);
            std::swap(v->left, v->right->right);
            updateParent(v->left, v);
            updateParent(v->right->right, v->right);
        }
        update(v->right);
        update(v->left);
        update(v);
        return v;
    }
    void destroyTree(Vertex* v) {
        if (!v) {
            return;
        }
        destroyTree(v->left);
        destroyTree(v->right);
        delete v;
    }
    void copyTree(Vertex* from, Vertex*& to) {
        if (!from) {
            return;
        }
        to = new Vertex(from->value, nullptr, from->sz);
        copyTree(from->left, to->left);
        if (to->left) {
            to->left->parent = to;
        }
        copyTree(from->right, to->right);
        if (to->right) {
            to->right->parent = to;
        }
    }
    void insert(Vertex*& r, const T& key) { // add update
        if (!r) {
            r = new Vertex(key);
            return;
        }
        Vertex* v = search(r, key);
        Vertex* u;
        if (v->value < key) {
            /*if (v->right) {
                u = getExtremum(v->right, true);
                u = (u->left = new Vertex(key, u));
            } else {*/
            u = (v->right = new Vertex(key, v));
            //}
        } else {
            /*if (v->left) {
                u = getExtremum(v->left, false);
                u = (u->right = new Vertex(key, u));
            } else {*/
            u = (v->left = new Vertex(key, v));
            //}
        }
        balance(u);
    }
    bool erase(Vertex* r, const T& key) { // add update
        if (!r) {
            return false;
        }
        Vertex* v = search(r, key);
        if (v->value != key) {
            return false;       
        }
        if (v->left && v->right) {
            Vertex* u = getExtremum(v->right, true);
            v->value = u->value;
            Vertex* p = u->parent;
            (p->left == u ? p->left : p->right) = nullptr;
            delete u;
            balance(p);
        } else {
            if (v->left || v->right) {
                Vertex* u = (v->left ? v->left : v->right);
                v->value = u->value;
                v->right = u->right;
                if (v->right) {
                    v->right->parent = v;
                }
                v->left = u->left;
                if (v->left) {
                    v->left->parent = v;
                }
                delete u;
                balance(v);
            } else {
                if (v->parent) {
                    Vertex* p = v->parent;
                    (v == p->left ? p->left : p->right) = nullptr;
                    delete v;
                    balance(p);
                } else {
                    root = nullptr;
                    delete v;
                }
            }
        }
        return true;
    }
    virtual void balance(Vertex*& v) {
        updateUp(v);
    }
public:
    BinaryTree() = default;
    BinaryTree(T key) : root(new Vertex(key)) {}
    BinaryTree(const BinaryTree<T>& tree) {
        copyTree(tree.root, root);
    }
    void swap(BinaryTree<T>& tree) {
        std::swap(root, tree.root);
    }
    void print(std::ostream& out) const {
        printTree(root, out);
    }
    BinaryTree<T>& operator=(const BinaryTree<T>& tree) {
        BinaryTree<T> cp = tree;
        swap(cp);
        return *this;
    }
    const T& searchKMin(size_t k) {
        Vertex* v = searchK(root, k, true);
        balance(v);
        return v->value;
    }
    const T& searchKMax(size_t k) {
        Vertex* v = searchK(root, k, false);
        balance(v);
        return v->value;
    }
    const T& getMinimum() {
        Vertex* v = getExtremum(root, true);
        balance(v);
        return v->value;    
    }
    const T& getMaximum() {
        Vertex* v = getExtremum(root, false);
        balance(v);
        return v->value;
    }
    void insert(const T& key) {
        insert(root, key);
    }
    bool erase(const T& key) {
        return erase(root, key);
    }
    virtual ~BinaryTree() {
        destroyTree(root);
    }
};

template<typename T>
std::ostream& operator<<(std::ostream& out, const BinaryTree<T>& tree) {
    tree.print(out);
    return out;
}

template<typename T>
class SplayTree : public BinaryTree<T> {
protected:
    using typename BinaryTree<T>::Vertex;
    using BinaryTree<T>::root;
    using BinaryTree<T>::rotate;
    void splay(Vertex*& v) {
        if (!v->parent) {
            return;
        }
        Vertex* u = v->parent->parent;
        if (!u) {
            u = v->parent;
            rotate(u, u->right == v);
            return;
        }
        if (u->left == v->parent) { 
            if (v->parent->left == v) {
                u = rotate(u, false);
                u = rotate(u, false);
            } else {
                rotate(v->parent, true);
                u = rotate(u, false);
            }
        } else {
            if (v->parent->left == v) {
                rotate(v->parent, false);
                u = rotate(u, true);
            } else {
                u = rotate(u, true);
                u = rotate(u, true);
            }
        }
        v = u;
        splay(v);
    }
    void balance(Vertex*& v) override { 
        BinaryTree<T>::updateUp(v);
        splay(v);
    }
public:
    SplayTree() = default;
    SplayTree(T key) : BinaryTree<T>(key) {}
    SplayTree(const SplayTree<T>& tree) : BinaryTree<T>(tree) {}
    SplayTree(const BinaryTree<T>& tree) : BinaryTree<T>(tree) {}
};

int main() {
    size_t n;
    int action;
    int param;
    SplayTree<int> tree;
    std::cin >> n;
    for (size_t i = 0; i < n; ++i) {
        std::cin >> action >> param;
        switch(action) {
            case 0:
                std::cout << tree.searchKMax(param) << '\n';
                break;
            case 1:
                tree.insert(param);
                break;
            case -1:
                tree.erase(param);
                break;
            default:
                break;
        }
        // std::cout << tree << '\n';
    }
    return 0;
}

/*
11
+1 5
+1 3
+1 7
0 1
0 2
0 3
-1 5
+1 10
0 1
0 2
0 3
//
7
5
3
10
7
3
*/