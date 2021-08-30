#include <list>
#include <vector>

template<typename T>
class FibonachiHeap { // on minimum
private:
    struct Vertex {
        T key;
        bool isMarked = false;
        size_t parentChild = 0;
        std::vector<Vertex*> children;
        Vertex* parent = nullptr;
        void add(Vertex* v) {
            v->parent = this;
            v->parentChild = children.size();
            children.push_back(v);
        }
        void remove(size_t n) {
            if (n + 1 != children.size()) {
                std::swap(children[n], children.back());
                children[n]->parentChild = n;
            }
            children.back()->clearParent();
            children.pop_back();
        }
        void clearParent() {
            parent = nullptr;
            parentChild = 0;
        }
        Vertex(T key, bool isMarked = false) : key(key), isMarked(isMarked) {}
    };

    std::list<Vertex*> roots;
    typename std::list<Vertex*>::iterator heapMinimum = roots.end();

    void destroyTree(Vertex*& v) {
        for (size_t i = 0; i < v->children.size(); ++i) {
            if (v->children[i]) {
                destroyTree(v->children[i]);
            }
        }
        if (v->parent) {
            v->parent.remove(v->parentChild);
        }
        delete v;
    }
    void copyTree(Vertex* from, Vertex*& to) {
        if (from) {
            to = new Vertex(from->key, from->isMarked);
            for (size_t i = 0; i < to->children.size(); ++i) {
                copyTree(from->children[i], to->children[i]);
                if (to->children[i]) {
                    to->children[i]->parent = this;
                    to->children[i]->parentChild = i;
                }
            }

        }
    }
    void consolidation() {
        std::vector<typename std::list<Vertex*>::iterator> count;
        for(auto it = roots.begin(); it != roots.end();) {
            auto v = it;
            ++it;
            size_t k = (*it)->children.size();
            if (count.size() <= k) {
                count.resize(k + 1, roots.end());
            }
            for (;; ++k) {
                if (count[k] == roots.end()) {
                    count[k] = v;
                    break;
                }
                if (count[k]->key < v->key) {
                    std::swap(count[k], v);
                }
                (*v)->add(count[k]);
                roots.erase(count[k]);
                count[k] = roots.end();
            }
        }
    }
    void cascadeCut(Vertex*& v) {
        Vertex* u = v->parent;
        if (!u) {
            return;
        }
        u->remove(v->parentChild);
        roots.push_back(v);
        v->isMarked = false;
        if (u->isMarked) {
            cascadeCut(u);
        } else {
            if (u->parent) {
                u->isMarked = true;
            }
        }
    }
    void findMinimum() {
        consolidation();
        for (auto it = roots.begin(); it != roots.end(); ++it) {
            if (heapMinimum == roots.end() || (*it)->key < (*heapMinimum)->key) {
                heapMinimum = it;
            }
        }
    }
    void decreaseKey(Vertex*& v, T value) {
        v->key -= value;
        if (v->parent && v->parent->key >= v->key) {
            cascadeCut(v);
        }       
    }
public:
    FibonachiHeap() = default;
    FibonachiHeap(const FibonachiHeap<T>& heap) {
        for (auto it = heap.roots.begin(); it != heap.roots.end(); ++it) {
            roots.push_back(nullptr);
            copyTree(*it, roots.back());
            if (heapMinimum == roots.end() || roots.back()->key < (*heapMinimum)->key) {
                heapMinimum = prev(roots.end());
            }
        }
    }
    void swap(FibonachiHeap<T>& heap) {
        std::swap(roots, heap.roots);
    }
    FibonachiHeap<T>& operator=(const FibonachiHeap<T>& heap) {
        FibonachiHeap<T> cp(heap);
        swap(cp);
        return *this;
    }
    void insert(T key) {
        roots.add(new Vertex(key));
    }
    T getMin() const {
        T key = (*heapMinimum)->key;
        return key;
    }
    T extractMin() {
        T key = (*heapMinimum)->key;
        for (size_t i = 0; i < (*heapMinimum)->children.size(); ++i) {
            (*heapMinimum)->children[i]->clearParent();
            roots.add((*heapMinimum)->children[i]);
        }
        roots.erase(heapMinimum);
        findMinimum();
        return key;
    }
    virtual ~FibonachiHeap() {
        for (auto it = roots.begin(); it != roots.end(); ++it) {
            destroyTree(*it);
        }
    }
};