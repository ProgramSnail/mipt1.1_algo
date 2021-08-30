#include <iostream>
#include <vector>
#include <list>

template<typename Key, typename Value, typename Hash = std::hash<Key>,
    typename Equal = std::equal_to<Key>, typename Allocator =
    std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
private:
    class Node {
    private:
        Key key;
        Value value;
        size_t hash;
    public:
        Node(Key key) {
            
        }
    };

    template<typename T>
    class IteratorT {
    private:
        using ThisIter = IteratorT<T>;
    public:
        IteratorT() {

        }
        IteratorT(const ThisIter& it) {

        }
        ThisIter& operator++() {

        }
        ThisIter operator++(int) {
            ThisIter cp(*this);
            
        }
        
        bool operator==(const ThisIter& it) {

        }

        bool operator!=(const ThisIter& it) {

        }
    };

    using ThisUMap = UnorderedMap<Key, Value, Hash, Equal, Allocator>;

    using NodeAllocator = typename Allocator::template rebind<Node>::other;

    static const double sizeK;

    static const size_t minTableSize;

    Allocator alloc;

    NodeAllocator nodeAlloc;

    Hash hash;

    size_t sz = 0;

    // size_t maxSz; // ??

    double maxLoadFactor = 0.5; 

    double loadFactor = 0;

    std::vector<typename std::list<Node>::iterator> table;

    std::list<Node> data;
public:
    using NodeType = std::pair<const Key, Value>;

    using Iterator = IteratorT<NodeType>;

    using ConstIterator = IteratorT<const NodeType>;

    explicit UnorderedMap(const Allocator& alloc = Allocator(),
        const Hash& hash = Hash()) : alloc(alloc), nodeAlloc(alloc), hash(hash) {}

    UnorderedMap(const ThisUMap& m) : UnorderedMap(m.alloc, m.hash) {

    }

    UnorderedMap(const ThisUMap&& m) : UnorderedMap(m.alloc, m.hash) {

    }

    virtual ~UnorderedMap() {

    }

    Value& operator[](const Key& key) {

    }

    Value& at(const Key& key) {

    }

    const Value& at(const Key& key) const {

    }

    Iterator begin() {

    }

    ConstIterator begin() const {

    }

    ConstIterator cbegin() const {

    }

    Iterator end() {

    }

    ConstIterator end() const {

    }

    ConstIterator cend() const {

    }

    Iterator insert(const NodeType& node) {

    }

    // Iterator insert(InputIterator begin, InputIterator end) {}

    template<typename ...Args>
    Iterator emplace(Args&&... args) {

    }

    void erase(Iterator& it) {

    }

    void erase(Iterator& b, Iterator& e) {

    }

    Iterator find(const Key& key) {

    }

    ConstIterator find(const Key& key) const {

    }

    size_t size() const {

    }

    void reserve(size_t n) {

    }

    size_t max_size() {

    }

    double load_factor() {

    }

    double max_load_factor() {

    }
};

template<typename Key, typename Value, typename Hash,
    typename Equal, typename Allocator>
const double UnorderedMap<Key, Value, Hash, Equal, Allocator>::sizeK = 2.0;

template<typename Key, typename Value, typename Hash,
    typename Equal, typename Allocator>
const size_t UnorderedMap<Key, Value, Hash, Equal, Allocator>::minTableSize = 100;