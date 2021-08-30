#include <iostream>
#include <iterator>
#include <vector>
#include <list>
#include <memory>
#include <type_traits>

template <typename T, typename Allocator = std::allocator<T> >
class List {
private:
    struct Node {
        Node* left = nullptr;
        Node* right = nullptr;
        T value;
        
        Node(const T& value) : value(value) {}
        
        Node(const Node& node) : left(node.left),
            right(node.right), value(node.value) {}
        
        virtual ~Node() {}
    };
    
    template<typename IterType, bool IsReversed = false>
    class IteratorTemplate : public std::
        iterator<std::bidirectional_iterator_tag, IterType> {
    private:
        friend List;

        static const bool isConst = std::is_same<IterType, const IterType>();
        
        using ThisIter = IteratorTemplate<IterType, IsReversed>;
        
        Node* ptr;

    public:
        IteratorTemplate() : ptr(nullptr) {}

        IteratorTemplate(Node* ptr) : ptr(ptr) {}

        IteratorTemplate(const ThisIter& it) {
            ptr = it.ptr;
        }
        
        operator IteratorTemplate<const IterType, IsReversed>() const {
            return IteratorTemplate<const IterType, IsReversed>(ptr);
        }

        ThisIter& operator=(const ThisIter& it) {
            ptr = it.ptr;
            return *this;
        }

        IteratorTemplate<IterType, false> base() const {
            return IteratorTemplate<IterType, false>(ptr->right);
        }
        
        ThisIter& operator++() {
            ptr = (IsReversed ? ptr->left : ptr->right);
            return *this;
        }

        ThisIter& operator--() {
            ptr = (IsReversed ? ptr->right : ptr->left);
            return *this;
        }

        ThisIter operator++(int) {
            ThisIter cp = *this;
            operator++();
            return cp;
        }

        ThisIter operator--(int) {
            ThisIter cp = *this;
            operator--();
            return cp;
        }

        bool operator==(const ThisIter& it) const {
            return ptr == it.ptr;
        }

        bool operator!=(const ThisIter& it) const {
            return !operator==(it);
        }

        IterType& operator*() const {
            return ptr->value;
        }

        IterType* operator->() const {
            return &operator*();
        }

        virtual ~IteratorTemplate() {}
    };

    using ThisList = List<T, Allocator>;

    using NodeAllocator = typename Allocator::template rebind<Node>::other;

    Allocator alloc_;
    
    NodeAllocator nodeAlloc;

    Node* rootNode;

    size_t sz;
    
    void insert_back(Node* v) {
        rootNode->left->right = v;
        v->left = rootNode->left;
        rootNode->left = v;
        v->right = rootNode;
    }
    
    void insert_front(Node* v) {
        rootNode->right->left = v;
        v->right = rootNode->right;
        rootNode->right = v;
        v->left = rootNode;
    }
    
    void insert_inside(Node* v, Node* L, Node* R) {
        L->right = v;
        v->left = L;
        R->left = v;
        v->right = R;
    }
public:
    using iterator = IteratorTemplate<T, false>;

    using const_iterator = IteratorTemplate<const T, false>;

    using reverse_iterator = IteratorTemplate<T, true>;

    using const_reverse_iterator = IteratorTemplate<const T, true>;

    explicit List(const Allocator& allocator = Allocator()) :
            alloc_(allocator), nodeAlloc(allocator),
            rootNode(nodeAlloc.allocate(1)), sz(0) {
        rootNode->left = rootNode;
        rootNode->right = rootNode;
    }

    explicit List(size_t count, const Allocator&
            allocator = Allocator()) : alloc_(allocator),
            nodeAlloc(allocator), rootNode(nodeAlloc.allocate(1)) {
        sz = count;
        Node* prevNode = rootNode;
        Node* v;
        for (size_t i = 0; i < count; ++i) {
            v = nodeAlloc.allocate(1);
            alloc_.construct(v);
            v->left = prevNode;
            prevNode->right = v;
            prevNode = v;
        }
        prevNode->right = rootNode;
        rootNode->left = prevNode;
    }
    
    List(size_t count, const T& x,
            const Allocator& allocator = Allocator()) :
            alloc_(allocator), nodeAlloc(allocator),
            rootNode(nodeAlloc.allocate(1)) {
        sz = count;
        Node* prevNode = rootNode;
        Node* v;
        for (size_t i = 0; i < count; ++i) {
            v = nodeAlloc.allocate(1);
            std::allocator_traits<Allocator>::construct(alloc_, &v->value, x);
            v->left = prevNode;
            prevNode->right = v;
            prevNode = v;
        }
        prevNode->right = rootNode;
        rootNode->left = prevNode;
    }
    
    List(const ThisList& list, bool changeCopyAllocator = true) {
        if (changeCopyAllocator) {
            alloc_ = std::allocator_traits<Allocator>::
                select_on_container_copy_construction(list.alloc_);
            nodeAlloc = std::allocator_traits<Allocator>::
                select_on_container_copy_construction(list.nodeAlloc);
        } else {
            alloc_ = list.alloc_;
            nodeAlloc = list.nodeAlloc;
        }
        
        rootNode = nodeAlloc.allocate(1);
        sz = list.sz;
        Node* prevNode = rootNode;
        Node* v;
        for (auto it = list.begin(); it != list.end(); ++it) {
            v = nodeAlloc.allocate(1);
            std::allocator_traits<Allocator>::construct(alloc_, v, *it);
            v->left = prevNode;
            prevNode->right = v;
            prevNode = v;
        }
        prevNode->right = rootNode;
        rootNode->left = prevNode;
    }
    
    List(ThisList&& list) : alloc_(list.alloc_), nodeAlloc(list.nodeAlloc),
            rootNode(list.rootNode), sz(list.sz) {
        list.rootNode = list.nodeAlloc.allocate(1);
        list.rootNode->left = list.rootNode;
        list.rootNode->right = list.rootNode;
        list.sz = 0;
    }

    void swap(ThisList& list, bool swapAllocators = true) {
        if (swapAllocators &&
            std::allocator_traits<Allocator>::
            propagate_on_container_swap::value) {
            std::swap(alloc_, list.alloc_);
            std::swap(nodeAlloc, list.nodeAlloc);
        }
        std::swap(rootNode, list.rootNode);
        std::swap(sz, list.sz);
    }

    ThisList& operator=(const ThisList& list) {
        ThisList cp(list, false);
        swap(cp, false);
        cp.alloc_ = alloc_;
        cp.nodeAlloc = nodeAlloc; 
        if (std::allocator_traits<Allocator>::
            propagate_on_container_copy_assignment::value) {
            alloc_ = list.alloc_;
            nodeAlloc = list.nodeAlloc;
        }
        return *this;
    }
    
    ThisList& operator=(ThisList&& list) {
        ThisList cp = std::move(list);
        swap(cp, false);
        cp.alloc_ = alloc_;
        cp.nodeAlloc = nodeAlloc;
        if (std::allocator_traits<Allocator>::
            propagate_on_container_move_assignment::value) {
            alloc_ = list.alloc_;
            nodeAlloc = list.nodeAlloc;
        }
    }
    
    // possible do this without repeating ??

    size_t size() const {
        return sz;
    }

    void push_back(const T& x) {
        Node* v = nodeAlloc.allocate(1); 
        std::allocator_traits<Allocator>::
            construct(alloc_, &v->value, std::forward<T>(x));
        insert_back(v);
        ++sz;
    }

    void push_front(const T& x) {
        Node* v = nodeAlloc.allocate(1);
        std::allocator_traits<Allocator>::
            construct(alloc_, &v->value, std::forward<T>(x));
        insert_front(v);
        ++sz;
    }
    
    template<typename... Args>
    void emplace_back(Args&&... args) {
        Node* v = nodeAlloc.allocate(1);
        std::allocator_traits<Allocator>::
            construct(alloc_, &v->value, std::forward<Args>(args)...);
        insert_back(v);
        ++sz;
    }
    
    template<typename... Args>
    void emplace_front(Args&&... args) {
        Node* v = nodeAlloc.allocate(1);
        std::allocator_traits<Allocator>::
            construct(alloc_, &v->value, std::forward<Args>(args)...);
        insert_front(v);
        ++sz;
    }

    void pop_back() {
        Node* v = rootNode->left;
        v->left->right = rootNode;
        rootNode->left = v->left;
        std::allocator_traits<Allocator>::destroy(alloc_, &v->value);
        nodeAlloc.deallocate(v, 1);
        --sz;
    }

    void pop_front() {
        Node* v = rootNode->right;
        v->right->left = rootNode;
        rootNode->right = v->right;
        std::allocator_traits<Allocator>::destroy(alloc_, &v->value);
        nodeAlloc.deallocate(v, 1);
        --sz;
    }

    void insert(const const_iterator& it, const T& x) {
        Node* v = nodeAlloc.allocate(1);
        std::allocator_traits<Allocator>::
            construct(alloc_, &v->value, x);
        insert_inside(v, it.ptr->left, it.ptr);
        ++sz;
    }
    
    void insert_back_node(const const_iterator& it) {
        if (it == end()) {
            return;
        }
        Node* v = rootNode->left;
        rootNode->left = v->left;
        v->left->right = rootNode;
        insert_inside(v, it.ptr->left, it.ptr);
    }
    
    void insert_node(const const_iterator&
        to, const const_iterator& from) {
        if (to == from) {
            return;
        }
        from.ptr->left->right = from.ptr->right;
        from.ptr->right->left = from.ptr->left;
        insert_inside(from.ptr, to.ptr->left, to.ptr);
    }
    
    template <typename... Args>
    void emplace(const const_iterator& it, Args&&... args) {
        Node* v = nodeAlloc.allocate(1);
        std::allocator_traits<Allocator>::
            construct(alloc_, &v->value, std::forward<Args>(args)...);
        insert_inside(v, it.ptr->left, it.ptr);
        ++sz;
    }
    
    void erase(const const_iterator& it) {
        it.ptr->left->right = it.ptr->right;
        it.ptr->right->left = it.ptr->left;
        std::allocator_traits<Allocator>::destroy(alloc_, &it.ptr->value);
        nodeAlloc.deallocate(it.ptr, 1);
        --sz;
    }

    iterator begin() {
        return iterator(rootNode->right);
    }

    const_iterator begin() const {
        return const_iterator(rootNode->right);
    }

    const_iterator cbegin() const {
        return const_iterator(rootNode->right);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(rootNode->left);
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(rootNode->left);
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(rootNode->left);
    }
    iterator end() {
        return iterator(rootNode);
    }

    const_iterator end() const {
        return const_iterator(rootNode);
    }

    const_iterator cend() const {
        return const_iterator(rootNode);
    }

    reverse_iterator rend() {
        return reverse_iterator(rootNode);
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(rootNode);
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(rootNode);
    }

    Allocator get_allocator() const {
        return alloc_;
    }

    virtual ~List() {
        for (Node* v = rootNode->right; v != rootNode;) {
            Node* u = v;
            v = v->right;
            std::allocator_traits<Allocator>::destroy(alloc_, &u->value);
            nodeAlloc.deallocate(u, 1);
        }
        nodeAlloc.deallocate(rootNode, 1);
    }
};

template<typename Key, typename Value, typename Hash = std::hash<Key>,
    typename Equal = std::equal_to<Key>, typename Allocator =
    std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
public:
    using NodeType = std::pair<const Key, Value>;
private:
    using ThisUnorderedMap = UnorderedMap<Key, Value, Hash, Equal, Allocator>;

    using NodeAllocator = typename Allocator::template rebind<NodeType>::other;

    using DataListType = List<NodeType, NodeAllocator>;

    using IterAllocator = typename Allocator::template
        rebind<std::pair<typename DataListType::iterator,
        typename DataListType::iterator>>::other;

    template<typename IterType>
    class IteratorTemplate : public
        std::iterator<std::forward_iterator_tag, IterType> {
    private:
        using ThisIter = IteratorTemplate<IterType>;

        using ListIter = typename std::conditional
            <std::is_const<IterType>::value,
            typename DataListType::const_iterator,
            typename DataListType::iterator>::type;

        ListIter listIter;
    public:
        friend UnorderedMap;

        IteratorTemplate(const ListIter& listIter) : listIter(listIter) {}

        IteratorTemplate(const ThisIter& it) : listIter(it.listIter) {}

        operator IteratorTemplate<const IterType>() const {
            return IteratorTemplate<const IterType>(listIter);
        }

        ThisIter& operator++() {
            ++listIter;
            return *this;
        }

        ThisIter operator++(int) {
            ThisIter cp(*this);
            operator++();
            return cp;
        }
        
        ThisIter operator--() {
            --listIter;
        }
        
        ThisIter operator--(int) {
            ThisIter cp(*this);
            operator--();
            return cp;
        }
        
        bool operator==(const ThisIter& it) const {
            return listIter == it.listIter;
        }

        bool operator!=(const ThisIter& it) const {
            return !operator==(it);
        }

        IterType& operator* () const {
            return *listIter;
        }

        IterType* operator-> () const {
            return &operator*();
        }
    };
public:
    using Iterator = IteratorTemplate<NodeType>;

    using ConstIterator = IteratorTemplate<const NodeType>;
private:
    static const double expansionConst_;

    static const size_t minHashTableSize_;
    
    static const size_t perExpansionSizeAdditionConst_;

    Allocator alloc_;
    
    Hash hashFunc_;
    
    Equal equalFunc_;

    double maxLoadFactor_ = 0.5;

    double loadFactor_ = 0;

    DataListType data_;

    size_t getHashTableId(size_t id) {
        return id % hashTable.size();
    }

    std::vector<std::pair<typename DataListType::iterator,
        typename DataListType::iterator>, IterAllocator> hashTable;
    
    std::pair<Iterator, bool> emplaceBase() {
        auto iter = prev(data_.end());
        size_t hash = getHashTableId(hashFunc_(iter->first));
        Iterator findedIter = find(iter->first, hash); 
        if (findedIter != end()) {
            data_.pop_back();
            return {findedIter, false};
        }
        auto& it = hashTable[hash];
        if (it.first == data_.end()) {
            it.first = iter;
            it.second = it.first;
        } else {
            data_.insert_back_node(it.first);
            --it.first;
        }
        return {Iterator(it.first), true};
    }
    
    std::pair<Iterator, bool> emplaceNode(const
        typename DataListType::iterator& it) {
        data_.insert_node(data_.end(), it);
        return emplaceBase();
    }

    void updateLoadFactor() {
        loadFactor_ = static_cast<double>(data_.size()) / hashTable.size();
        if (loadFactor_ >= maxLoadFactor_) {
            rehash(expansionConst_ * hashTable.size() + perExpansionSizeAdditionConst_);
        } 
    }
    
    void rehash(size_t n) {
        hashTable.clear();
        hashTable.resize(n, {data_.end(), data_.end()});
        if (data_.size() > 0) {
            auto oldDataEnd = prev(data_.end());
            bool endCycle = false;
            typename DataListType::iterator iter;
            for (auto it = data_.begin(); !endCycle && it != data_.end();) {
                if (it == oldDataEnd) {
                    endCycle = true;
                }
                iter = it;
                ++it;
                emplaceNode(iter);
            }
        }
    }

    template<bool ThrowExeption>
    Value& getByKey(const Key& key) {
        Iterator it = find(key);
        if (it.listIter == data_.end()) {
            if constexpr (ThrowExeption) {
                throw std::out_of_range("");
            } else {
                return insert(key, Value()).first->second;
            }
        }
        return it->second;
    }

    Iterator find(const Key& key, size_t hash) {
        bool isFinded = true;
        Iterator it = Iterator(hashTable[hash].first);
        for (; it != end() && !equalFunc_(key, it->first); ++it) {
            if (it == hashTable[hash].second) {
                isFinded = false;
                break;
            }
        }
        if (!isFinded) {
            it = end();
        }
        return it;
    }
public:
    explicit UnorderedMap(const Allocator& alloc_ = Allocator()) :
        alloc_(alloc_), hashTable(minHashTableSize_, {data_.end(), data_.end()}) {}

    UnorderedMap(const ThisUnorderedMap& m) : alloc_(m.alloc_),
        data_(m.data_), hashTable(m.hashTable) {}

    UnorderedMap(ThisUnorderedMap&& m) : alloc_(m.alloc_),
        data_(std::move(m.data_)), hashTable(std::move(m.hashTable)) {}

    virtual ~UnorderedMap() {}

    Value& operator[](const Key& key) {
        return getByKey<false>(key);
    }

    Value& at(const Key& key) {
        return getByKey<true>(key);
    }

    const Value& at(const Key& key) const {
        ConstIterator it = find(key);
        if (it.listIter == data_.end()) {
            throw std::out_of_range("");
        }
        return it->second;
    }

    Iterator begin() {
        return Iterator(data_.begin());
    }

    ConstIterator cbegin() const {
        return ConstIterator(data_.cbegin());
    }

    ConstIterator begin() const {
        return cbegin();
    }

    Iterator end() {
        return Iterator(data_.end());
    }

    ConstIterator cend() const {
        return ConstIterator(data_.cend());
    }

    ConstIterator end() const {
        return cend();
    }

    void swap(ThisUnorderedMap& m) {
        std::swap(maxLoadFactor_, m.maxLoadFactor_);
        std::swap(loadFactor_, m.loadFactor_);
        data_.swap(m.data_);
        hashTable.swap(m.hashTable);
    }

    ThisUnorderedMap& operator=(const ThisUnorderedMap& m) {
        ThisUnorderedMap cp = m;
        swap(cp);
        return *this;
    }

    ThisUnorderedMap& operator=(ThisUnorderedMap&& m) {
        ThisUnorderedMap cp = std::move(m);
        swap(cp);
        return *this;
    }
    
    template<typename... Args>
    std::pair<Iterator, bool> emplace(Args&&... args) {
        data_.emplace_back(std::forward<Args>(args)...);
        updateLoadFactor();
        return emplaceBase();
    }

    std::pair<Iterator, bool> insert(const Key& key, const Value& value) {
        return emplace(key, value);
    }

    std::pair<Iterator, bool> insert(const NodeType& node) {
        return emplace(node);
    }
    
    template<typename P>
    std::pair<Iterator, bool> insert(P&& node) {
        return emplace(std::forward<P>(node));
    }

    template<typename InputIterator>
    void insert(const InputIterator& b, const InputIterator& e) {
        for (auto it = b; it != e; ++it) {
            insert(*it);
        }
    }

    void erase(const ConstIterator& it) {
        auto& p = hashTable[getHashTableId(hashFunc_(it->first))];
        typename DataListType::const_iterator iter;
        if (p.first == p.second) {
            iter = p.first;
            if (iter == it.listIter) {
                p.first = data_.end();
                p.second = data_.end();
            }
        } else {
            iter = p.first;
            if (iter == it.listIter) {
                ++p.first;
            } 
            iter = p.second;
            if (iter == it.listIter) {
                --p.second;
            }
        }
        data_.erase(it.listIter);
        updateLoadFactor();
    }

    void erase(const ConstIterator& b, const ConstIterator& e) {
        ConstIterator it = b;
        ConstIterator eraseIt = it;
        for (; it != e;) {
            ++it;
            erase(eraseIt);
            eraseIt = it;
        }
    }

    Iterator find(const Key& key) {
        return find(key, getHashTableId(hashFunc_(key)));
    }

    ConstIterator find(const Key& key) const {
        bool isFinded = true;
        size_t hash = hashFunc_(key) % hashTable.size();
        ConstIterator it = ConstIterator(hashTable[hash].first);
        for (; it != end() && !equalFunc_(key, it->first); ++it) {
            if (it == hashTable[hash].second) {
                isFinded = false;
                break;
            }
        }
        if (!isFinded) {
            it = end();
        }
        return it;
    }
    
    // possible do this without repeating ??

    Allocator get_allocator() {
        return alloc_;
    }

    size_t size() const {
        return data_.size();
    }

    void reserve(size_t n) {
        rehash(n);
    }

    size_t max_size() {
        return hashTable.size();
        // ??
    }

    double load_factor() {
        return loadFactor_;
    }

    double max_load_factor() {
        return maxLoadFactor_;
    }
};

template<typename Key, typename Value, typename Hash,
    typename Equal, typename Allocator>
const double UnorderedMap<Key, Value, Hash,
    Equal, Allocator>::expansionConst_ = 1.6;

template<typename Key, typename Value, typename Hash,
    typename Equal, typename Allocator>
const size_t UnorderedMap<Key, Value, Hash, Equal,
    Allocator>::minHashTableSize_ = 1000;

template<typename Key, typename Value, typename Hash,
    typename Equal, typename Allocator>
const size_t UnorderedMap<Key, Value, Hash, Equal,
    Allocator>::perExpansionSizeAdditionConst_ = 7;
