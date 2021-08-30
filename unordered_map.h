#include <iostream>
#include <iterator>
#include <vector>
#include <list>

template<typename Key, typename Value, typename Hash = std::hash<Key>,
    typename Equal = std::equal_to<Key>, typename Allocator =
    std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
public:
    using NodeType = std::pair<const Key, Value>;
private:
    struct Node {
        NodeType data; // how to make const and init ?
        size_t hash; // must be const
        Node(Key key, Value value = Value()) :
            data(key, value), hash(Hash(key)) {}
    };    

    using ThisUMap = UnorderedMap<Key, Value, Hash, Equal, Allocator>;

    using NodeAllocator = typename Allocator::template rebind<Node>::other;

    using DataListType = std::list<Node, NodeAllocator>;

    template<typename T>
    class IteratorT {
    private:
        using ThisIter = IteratorT<T>;

        using ListIter = typename std::list<Node, NodeAllocator>::iterator;

        ListIter listIt;
    public:
        friend UnorderedMap;

        IteratorT(const ListIter& listIt) : listIt(listIt) {}

        IteratorT(const ThisIter& it) : listIt(it.listIt) {}

        operator IteratorT<const T>() const {
            return IteratorT<const T>(listIt);
        }

        ThisIter& operator++() {
            ++listIt;
        }

        ThisIter operator++(int) {
            ThisIter cp(*this);
            operator++();
            return cp;
        }
        
        bool operator==(const ThisIter& it) {
            return listIt == it.listIt;
        }

        bool operator!=(const ThisIter& it) {
            return !operator==(it);
        }

        T& operator* () {
            return listIt->data;
        }

        T* operator-> () {
            return &operator*();
        }
    };

    static const double sizeK;

    static const size_t minTableSize;

    Allocator alloc;

    NodeAllocator nodeAlloc;

    size_t sz = 0;

    double maxLoadFactor = 0.5; 

    double loadFactor = 0;

    DataListType data;

    std::vector<typename DataListType::iterator, Allocator> table;

    void rehash(size_t n) {
        std::list<Node, NodeAllocator> oldData = std::move(data);
        table.clear();
        table.resize(n, end());
        for (auto& it : oldData) {
            insert(it->data);
        }
    }

    void updateLoadFactor() {
        loadFactor = static_cast<double>(sz) / data.size();
        if (loadFactor >= maxLoadFactor) {
            rehash(sizeK * data.size());
        } 
    }
public:
    using Iterator = IteratorT<NodeType>;

    using ConstIterator = IteratorT<const NodeType>;

    explicit UnorderedMap(const Allocator& alloc = Allocator()) :
        alloc(alloc), nodeAlloc(alloc), table(minTableSize, data.end()) {}

    UnorderedMap(const ThisUMap& m) : alloc(m.alloc),
        nodeAlloc(m.alloc), data(m.data), table(m.table) {
    }

    UnorderedMap(const ThisUMap&& m) : alloc(m.alloc), nodeAlloc(alloc),
        data(std::move(m.data)), table(std::move(m.table)) {}

    virtual ~UnorderedMap() {}

    Value& operator[](const Key& key) {
        try {
            return at(key);
        } catch(std::out_of_range) {
            return *insert(key, Value());
        }
    }

    Value& at(const Key& key) {
        Iterator it = find(key);
        if (it.listIt == data.end()) {
            throw std::out_of_range("");
        }
        return *it;
    }

    const Value& at(const Key& key) const {
        ConstIterator it = find(key);
        if (it.listIt == data.end()) {
            throw std::out_of_range("");
        }
        return *it;
    }

    Iterator begin() {
        return Iterator(data.begin());
    }

    ConstIterator cbegin() const {
        return ConstIterator(data.begin());
    }

    ConstIterator begin() const {
        return cbegin();
    }

    Iterator end() {
        return Iterator(data.end());
    }

    ConstIterator cend() const {
        return ConstIterator(data.end());
    }

    ConstIterator end() const {
        return cend();
    }

    void swap(const ThisUMap& m) {
        std::swap(sz, m.sz);
        std::swap(maxLoadFactor, m.maxLoadFactor);
        std::swap(loadFactor, m.loadFactor);
        data.swap(m.data);
        table.swap(m.table);
    }

    ThisUMap& operator=(const ThisUMap& m) {
        ThisUMap cp = m;
        swap(cp);
        return *this;
    }

    ThisUMap& operator=(const ThisUMap&& m) {
        ThisUMap cp = std::move(m);
        swap(cp);
        return *this;
    }

    Iterator insert(const NodeType& node) {
        auto& it = table[Hash(node.key) % table];
        if (it == data.end()) {
            data.push_back(node);
            it = prev(data.end());
        } else {
            data.insert(it, Node(node.key, node.value));
            // ?? --it; ??
            // iterator on fist elem ??
        }
        ++sz;
        updateLoadFactor();
        return it;
    }

    // Iterator insert(InputIterator begin, InputIterator end) {}

    template<typename ...Args>
    Iterator emplace(Args&&... args) {
        // ??
        ++sz;
        updateLoadFactor();
    }

    void erase(Iterator& it) {
        data.erase(it.listIt);
        --sz;
        updateLoadFactor();
        // move iter forward ??
    }

    void erase(Iterator& b, Iterator& e) {
        Iterator& c = b;
        for (; b != e;) {
            ++b;
            data.erase(c);
            c = b;
        }
    }

    Iterator find(const Key& key) {
        size_t hash = Hash(key) % table.size();
        Iterator it = Iterator(table[hash]);
        for (; it != end() && it->hash == hash && key != it.key; ++it) {}
        if (it != end() && it->hash != hash) {
            it = end();
        }
        return it;
    }

    ConstIterator find(const Key& key) const {
        size_t hash = Hash(key) % table.size();
        ConstIterator it = ConstIterator(table[hash]);
        for (; it != end() && it->hash == hash && key != it.key; ++it) {}
        if (it != end() && it->hash != hash) {
            it = end();
        }
        return it;
    }

    Allocator get_allocator() {
        return alloc;
    }

    size_t size() const {
        return sz;
    }

    void reserve(size_t n) {
        rehash(n);
    }

    size_t max_size() {
        return data.sz();
        // ??
    }

    double load_factor() {
        return loadFactor;
    }

    double max_load_factor() {
        return maxLoadFactor;
    }
};

template<typename Key, typename Value, typename Hash,
    typename Equal, typename Allocator>
const double UnorderedMap<Key, Value, Hash, Equal, Allocator>::sizeK = 2.0;

template<typename Key, typename Value, typename Hash,
    typename Equal, typename Allocator>
const size_t UnorderedMap<Key, Value, Hash, Equal, Allocator>::minTableSize = 100;