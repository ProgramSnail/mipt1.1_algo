#include <iostream>

using T = int;

class SplayTree {
private:
    struct Vertex {
        T assign = 0;
        bool isAssigned = false;
        T add = 0;
        bool isInversed = false;
        bool isDUpdated = true;
        bool isUUpdated = true;
        size_t maxSuffI = 1;
        size_t maxPrefI = 1;
        size_t maxSuffD = 1;
        size_t maxPrefD = 1;

        T value = 0;
        T sum = 0;
        size_t sz = 1;
        Vertex* left = nullptr;
        Vertex* right = nullptr;
        Vertex* parent = nullptr;

        Vertex() = default;

        Vertex(T value, Vertex* parent = nullptr) : value(value), sum(value), parent(parent)  {}

        static size_t getSz(Vertex* v) {
            if (v) {
                return v->sz;
            }
            return 0;
        }

        static void update(Vertex* v) {
            if (v) {
                v->update();
            }
        }

        static void forceDUpdate(Vertex* v) {
            if (v) {
                v->forceDUpdate();
            }
        }

        static void forceUUpdate(Vertex* v) {
            if (v) {
                v->forceUUpdate();
            }
        }

        void print(std::ostream& out) {
            out << (left ? "< " : "") << value << ':' << sz 
                << ':' << (parent ? parent->value : 0) << (right ? " >" : "");
        }

        void dUpdate() {
            if (isDUpdated) {
                return;
            }
            isDUpdated = true;
            if (isInversed) {
                std::swap(left, right);
                std::swap(maxSuffI, maxPrefD);
                std::swap(maxSuffD, maxPrefI);
            }
            value += add;
            sum += add * sz;
            if (isAssigned) {
                value = assign;
                sum = assign * sz;
            }
            if (left) {
                left->isDUpdated = false;
                left->isInversed = (left->isInversed != isInversed);
                if (isAssigned) {
                    left->isAssigned = true;
                    left->assign = assign;
                    left->add = 0;
                }
                left->add += add;
            }
            if (right) {
                right->isDUpdated = false;
                right->isInversed = (right->isInversed != isInversed);
                if (isAssigned) {
                    right->isAssigned = true;
                    right->assign = assign;
                    left->add = 0;
                }
                right->add += add;
            }
            isInversed = false;
            isAssigned = false;
            add = 0;
            assign = 0;
        }

        void uUpdate() {
            if (isUUpdated) {
                return;
            }
            isUUpdated = true;
            sz = 1 + getSz(left) + getSz(right);
            Vertex* leftLast = nullptr;
            Vertex* rightFirst = nullptr;
            if (left) {
                leftLast = left->last();
            }
            if (right) {
                rightFirst = right->first();
            }
            sum = value;
            if (left) {
                sum += left->sum;
                maxPrefI = left->maxPrefI;
                maxPrefD = left->maxPrefD;
                if (right) {
                    if (left->maxPrefI == left->sz && leftLast <= rightFirst) {
                        maxPrefI += right->maxPrefI;
                    }
                    if (left->maxPrefD == left->sz && leftLast >= rightFirst) {
                        maxPrefD += right->maxPrefD;
                    }
                }
            }
            if (right) {
                sum += right->sum;
                maxSuffI = right->maxSuffI;
                maxSuffD = right->maxSuffD;
                if (left) {
                    if (right->maxSuffI == right->sz && left && leftLast <= rightFirst) {
                        maxSuffI += left->maxSuffI;
                    }
                    if (right->maxSuffD == right->sz && left && leftLast >= rightFirst) {
                        maxSuffD += left->maxSuffD;
                    }
                }
            }
            if (parent) {
                parent->isUUpdated = false;
            }
        }

        Vertex* first() {
            update();
            if (left) {
                return left->first();
            }
            return this;
        }

        Vertex* last() {
            update();
            if (right) {
                return right->last();
            }
            return this;
        }

        Vertex* nearest(bool isEarler) {
            Vertex* v = this;
            Vertex* u = nullptr; 
            for (;; v = v->parent) {
                if (!v->parent) {
                    return nullptr;
                }
                u = (isEarler ? v->parent->left : v->parent->right);
                if (u && u != v) {
                    return isEarler ? u->last() : u->first();
                }
            }
        }

        Vertex* prev() {
            return nearest(true);
        }

        Vertex* next() {
            return nearest(false);
        }

        void setDChanged() {
            isDUpdated = false;
        }

        void setUChanged() {
            isUUpdated = false;
        }

        void update() {
            uUpdate();
            dUpdate();
        }
        
        void forceDUpdate() {
            setDChanged();
            dUpdate();    
        }

        void forceUUpdate() {
            setUChanged();
            uUpdate();
        }

        void forceUUpdateUp() {
            forceUUpdate();
            if (parent) {
                parent->forceUUpdateUp();
            }
        }

        Vertex* searchFirst(const T& x, bool isIncreasing) {
            update();
            Vertex* L = (isIncreasing ? left : right);
            Vertex* R = (isIncreasing ? right : left);
            if (x <= value && L) {
                Vertex* v = L->searchFirst(x, isIncreasing);
                if (v->value != x && value == x) {
                    return this;
                }
                return v;
            }
            if (x > value && R) {
                return R->searchFirst(x, isIncreasing);
            }
            return this;
        }

        Vertex* searchLast(const T& x, bool isIncreasing) {
            update();
            Vertex* L = (isIncreasing ? left : right);
            Vertex* R = (isIncreasing ? right : left);
            if (x < value && L) {
                return L->searchLast(x, isIncreasing);
            }
            if (x >= value && R) {
                Vertex* v = R->searchLast(x, isIncreasing);
                if (v->value != x && value == x) {
                    return this;
                }
                return v;
            }
            return this;
        }

        Vertex* searchK(size_t k) {
            update();
            size_t leftSz = Vertex::getSz(left);
            if (left && k <= leftSz) {
                return left->searchK(k);
            }
            if (right && k > leftSz + 1) {
                return right->searchK(k - left->sz - 1);
            }
            return this;
        }
    };

    Vertex* root = nullptr;

    void printTree(Vertex* v, std::ostream& out) const {
        if (!v) {
            return;
        }
        printTree(v->left, out);
        v->print(out);
        out << ' ';
        printTree(v->right, out);
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
        updateChildrenParent(u);
        updateChildrenParent(v);
        Vertex::forceUUpdate(u);
        Vertex::forceUUpdate(v);
    }
    Vertex* rotate(Vertex* v, bool isLeft) {
        if (!v || !(isLeft ? v->right : v->left)) {
            return v;
        }
        Vertex *u = (isLeft ? v->right : v->left);
        Vertex::update(v);
        Vertex::update(v->left);
        Vertex::update(v->right);
        Vertex::update(u->left);
        Vertex::update(u->right);
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
        Vertex::forceUUpdate(v->right);
        Vertex::forceUUpdate(v->left);
        Vertex::forceUUpdate(v);
        return v;
    }
    void splay(Vertex* v) {
        if (!v || !v->parent) {
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
    void destroyTree(Vertex* v) {
        if (!v) {
            return;
        }
        destroyTree(v->left);
        destroyTree(v->right);
        delete v;
    }
    std::pair<SplayTree, SplayTree> splitK(size_t k) {
        SplayTree treeLeft;
        SplayTree treeRight;
        if (k == 0) {
            swap(treeRight);
            return {treeLeft, treeRight};
        }
        if (root) {
            splay(root->searchK(k));
            treeRight.root = root->right;
            root->right = nullptr;
            root->forceUUpdate();
            swap(treeLeft);
        }
        return {treeLeft, treeRight};
    }
    std::pair<SplayTree, std::pair<SplayTree, SplayTree> > split(size_t L, size_t R) {
        std::pair<SplayTree, SplayTree> trees = splitK(L); // [] for 0-numeration, or (L - 1) for 1-numeration
        std::pair<SplayTree, SplayTree> rightTrees = trees.second.splitK(R - L + 1);
        return {trees.first, rightTrees};
    }
    SplayTree& merge(SplayTree& tree) {
        if (root) {
            splay(root->last());
            root->right = tree.root;
            root->forceUUpdate();
        } else {
            root = tree.root;
        }
        tree.root = nullptr;
        return *this;
    }
    SplayTree& merge(std::pair<SplayTree, std::pair<SplayTree, SplayTree> >& trees) {
        return merge(trees.first).merge(trees.second.first).merge(trees.second.second);
    }
public:
    SplayTree() = default;
    SplayTree(const T& key) : root(new Vertex(key)) {}
    void swap(SplayTree& tree) {
        std::swap(root, tree.root);
    }
    void print(std::ostream& out) const {
        printTree(root, out);
    }
    void insertK(size_t k, const T& x) {
        std::pair<SplayTree, SplayTree> trees = splitK(k);
        SplayTree newTree = SplayTree(x);
        merge(trees.first).merge(newTree).merge(trees.second);
    }
    void eraseK(size_t k) {
        auto trees = split(k, k + 1); // for 0-numeration
        merge(trees.first).merge(trees.second.second);
    }
    T getSum() {
        if (root) {
            return root->sum;
        }
        return 0;
    }
    T getSum(size_t L, size_t R) {
        auto trees = split(L, R);
        T sum = trees.first.getSum();
        merge(trees);
        return sum;
    }
    void assign(size_t L, size_t R, const T& x) {
        auto trees = split(L, R);
        Vertex* v = trees.second.first.root;
        if (v) {
            v->update();
            v->assign = x;
            v->isAssigned = true;
            v->forceDUpdate();
        }
        merge(trees);
    }
    void add(size_t L, size_t R, const T& x) {
        auto trees = split(L, R);
        Vertex* v = trees.second.first.root;
        if (v) {
            v->update();
            v->add += x;
            v->forceDUpdate();
        }
        merge(trees);
    }
    void nextPermutation(size_t L, size_t R, bool isNext = true) { // next or prev
        auto trees = split(L, R);
        Vertex* v = trees.second.first.root;
        if (v) {
            v->update();
            size_t k = (isNext ? v->maxSuffD : v->maxSuffI);
            if (k == v->sz) {
                v->isInversed = true;
            } else {
                Vertex* u = v->searchK(v->sz - k);
                auto subTrees = trees.second.first.splitK(v->sz - k);
                Vertex* w;
                if (isNext) {
                    w = subTrees.second.root->searchLast(u->value, false); 
                    if (w->value <= u->value) {
                        w = w->prev();
                    }
                } else {
                    w = subTrees.first.root->searchFirst(u->value, true);
                    if (w->value >= u->value) {
                        w = w->prev();
                    }
                }
                std::swap(u->value, w->value);
                u->forceUUpdateUp();
                w->forceUUpdateUp();
                subTrees.second.root->isInversed = true;
                subTrees.second.root->forceDUpdate();
                trees.second.first.merge(subTrees.first).merge(subTrees.second);
            }
            v->forceDUpdate();
        }
        merge(trees);
    }
    void prevPermutation(size_t L, size_t R) {
        nextPermutation(L, R, false);
    }

    size_t size() const {
        return Vertex::getSz(root);
    }

    virtual ~SplayTree() {
        destroyTree(root);
    }
};

int main() {
    size_t n;
    size_t q;
    SplayTree tree;
    std::cin >> n;
    T x;
    for (size_t i = 0; i < n; ++i) {
        std::cin >> x;
        tree.insertK(x, tree.size());
        tree.print(std::cout);
    }
    std::cin >> q;
    size_t command;
    size_t L, R;
    size_t pos;
    for (size_t i = 0; i < n; ++i) {
        std::cin >> command;
        switch(command) {
            case 1:
                std::cin >> L >> R;
                std::cout << tree.getSum(L, R); 
                break;
            case 2:
                std::cin >> x >> pos;
                tree.insertK(pos, x);
                break;
            case 3:
                std::cin >> pos;
                tree.eraseK(pos);
                break;
            case 4:
                std::cin >> x >> L >> R;
                tree.assign(L, R, x);
                break;
            case 5:
                std::cin >> x >> L >> R;
                tree.add(L, R, x);
                break;
            case 6:
                std::cin >> L >> R;
                tree.nextPermutation(L, R);
                break;
            case 7:
                std::cin >> L >> R;
                tree.nextPermutation(L, R);
                break;
            default:
                break;
        }
    }
    return 0;
}
