#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>

class FixedMemoryManagerBase {
public:
    FixedMemoryManagerBase() = default;

    virtual void* allocate() { return nullptr; }

    virtual void deallocate(void*) {}

    virtual ~FixedMemoryManagerBase() {}
};

template<size_t ChunkSize>
class FixedMemoryManager : public FixedMemoryManagerBase {
private:
    using ThisManager = FixedMemoryManager<ChunkSize>;

    const size_t BLOCK_SIZE = 16;

    std::vector<void*> blocks_;

    std::vector<void*> freeMemory_;

    size_t lastBlockPos_ = BLOCK_SIZE;

    void newChunk() {
        void* ptr = ::operator new(ChunkSize * BLOCK_SIZE);
        lastBlockPos_ = 0;
        blocks_.push_back(ptr);
    }

public:
    FixedMemoryManager() = default;

    void* allocate() override {
        if (freeMemory_.size() > 0) {
            void* ptr = freeMemory_.back();
            freeMemory_.pop_back();
            return ptr;
        }
        if (lastBlockPos_ >= BLOCK_SIZE) {
            newChunk();
        }
        ++lastBlockPos_;
        return static_cast<int8_t*>(blocks_.back()) + (lastBlockPos_ - 1) * ChunkSize;
    }

    void deallocate(void* ptr) override {
        freeMemory_.push_back(ptr);
    }

    void clear() {
        for (size_t i = 0; i < blocks_.size(); ++i) {
            ::operator delete(blocks_[i]);
        }
        blocks_.clear();
        freeMemory_.clear();
        lastBlockPos_ = BLOCK_SIZE;
    }

    virtual ~FixedMemoryManager() {
        clear();
    }
};

static constexpr size_t calculateNextChunkSize(size_t chunkSize) {
    return chunkSize / 2;
}

template<size_t MaxChunkSize, size_t SizeOfT>
class MemoryManagers {
private:
    FixedMemoryManager<MaxChunkSize * SizeOfT> manager_;

    MemoryManagers<calculateNextChunkSize(MaxChunkSize), SizeOfT> otherManagers_;

public:
    template<size_t ChunkSize>
    FixedMemoryManagerBase* getMemoryManager() {
        if constexpr (ChunkSize == MaxChunkSize) {
            return &manager_;
        }
        return otherManagers_.template getMemoryManager<
                calculateNextChunkSize(MaxChunkSize)>();
    }

    void clear() {
        manager_.clear();
        otherManagers_.clear();
    }
};

template<size_t SizeOfT>
class MemoryManagers<0, SizeOfT> {
public:
    template<size_t ChunkSize>
    FixedMemoryManagerBase* getMemoryManager() {
        return nullptr;
    }

    void clear() {}
};

template<size_t MaxChunkSize, size_t MemoryManagerChunkSize, size_t SizeOfT>
struct FindMemoryManager {
    static FixedMemoryManagerBase* func(size_t count,
            MemoryManagers<MemoryManagerChunkSize, SizeOfT>& managers) {
        constexpr size_t nextChunkSize = calculateNextChunkSize(MaxChunkSize);
        if (count > nextChunkSize) {
            return managers.template getMemoryManager<MaxChunkSize>();
        }
        return FindMemoryManager<nextChunkSize, MemoryManagerChunkSize,
            SizeOfT>::func(count, managers);
    }
};

template<size_t MemoryManagerChunkSize, size_t SizeOfT>
struct FindMemoryManager<0, MemoryManagerChunkSize, SizeOfT> {
    static FixedMemoryManagerBase* func(size_t,
            MemoryManagers<MemoryManagerChunkSize, SizeOfT>&) {
        return nullptr;
    }
};

const size_t MAX_MANAGER_CHUNK_SIZE = 32;

const size_t MANAGER_STEP_CONST = 1;

MemoryManagers<MAX_MANAGER_CHUNK_SIZE, MANAGER_STEP_CONST> memoryManagers;

template<typename T>
class FastAllocator {
private:
    static const size_t USE_MANAGER_C = 32;
    
public:
    template<typename U> 
    struct rebind {
        typedef FastAllocator<U> other;
    };

    typedef T value_type;

    FastAllocator() = default;

    template<typename U>
    FastAllocator(const FastAllocator<U>&) {}

    T* allocate(size_t count) {
        void* m;
        size_t allocateSize = count * sizeof(T);
        if (allocateSize <= USE_MANAGER_C) {
            m = FindMemoryManager<USE_MANAGER_C, MAX_MANAGER_CHUNK_SIZE, MANAGER_STEP_CONST>
                ::func(allocateSize, memoryManagers)->allocate();
        } else {
            m = ::operator new(allocateSize);
        }
        return static_cast<T*>(m);
    }

    void deallocate(T* ptr, size_t count) {
        size_t deallocateSize = count * sizeof(T);
        if (deallocateSize <= USE_MANAGER_C) {
            FindMemoryManager<USE_MANAGER_C, MAX_MANAGER_CHUNK_SIZE, MANAGER_STEP_CONST>
                ::func(deallocateSize, memoryManagers)->deallocate(ptr);
        } else {
            ::operator delete(ptr);
        }
    }

    template<typename... Args>
    void construct(T* ptr, const Args&... args) {
        new(ptr) T(args...);
    }

    void destroy(T* ptr) {
        ptr->~T();
    }

    const FastAllocator& operator=(const FastAllocator&) {
        return *this;
    }

    bool operator==(const FastAllocator<T>&) const {
        return true;
    }

    bool operator!=(const FastAllocator<T>& alloc) const {
        return !operator==(alloc);
    }

    virtual ~FastAllocator() {}
};

/*template<typename T>
MemoryManagers<FastAllocator<T>::USE_MANAGER_C, sizeof(T)> FastAllocator<T>::memoryManagers;*/

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