#include <deque>

template<typename T, size_t K>
class BTree {
private:
    struct Vertex { // ??
        std::deque<T> v;
        std::deque<Vertex*> e;
        Vertex* parent = nullptr;
        void add(T elem) {

        }
        void remove(size_t id) {

        }
    };
    Vertex* root = nullptr;
    void insert(Vertex* v, T key) {

    }
    bool remove(Vertex* v, T key) {

    }
    std::pair<Vertex*, std::pair<Vertex*, Vertex*>> split(Vertex* v, T key) {

    }
    Vertex* merge(Vertex* a, Vertex* b, Vertex* v) {

    }
    void destroyTree(Vertex* v) {
        if (!v) {
            return;
        }
        for (size_t i = 0; i < v->e.size(); ++i) {
            destroyTree(v->e[i]);
        }
    }
public: 
    BTree() = default;
    BTree(T key) {

    }
    BTree(const BTree& tree) {

    }
    void insert(T key) {
        insert(root, key);
    }
    bool remove(T key) {
        return remove(root, key);
    }
    std::pair<BTree<T, K>, std::pair<BTree<T, K>, BTree<T, K>>> split(T key) {

    }
    void merge(T key, BTree& tree) {

    }

    ~BTree() {

    }
};