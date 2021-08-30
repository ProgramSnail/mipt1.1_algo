#include <iostream>
#include <iterator>
#include <memory>
#include <vector>
#include <map>

/*
Right realisation with ML 

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
*/

// Fast realisation, but without right structure

template<size_t ChunkSize>
class FixedMemoryManager {
private:
    static size_t chunkPosition;
    static std::vector<void*> memory;
    static std::vector<size_t> chunkNum; 
    static std::map<size_t, std::vector<void*>> freeMemory;

    using ThisManager = FixedMemoryManager<ChunkSize>;

    void newChunk() {
        memory.push_back(::operator new(ChunkSize));
        chunkPosition = 0;
    }
public:
    FixedMemoryManager() = default;

    FixedMemoryManager(const ThisManager&) = default;

    ThisManager& operator=(const ThisManager&) = default;

    bool operator==(const ThisManager&) const {
        return true;
    }

    bool operator!=(const ThisManager& manager) const {
        return !operator==(manager);
    }

    void* allocate(size_t count) {
        auto it = freeMemory.find(count);
        if (it != freeMemory.end() && it->second.size() > 0) {
            void* ptr = it->second.back();
            it->second.pop_back();
            return ptr;
        }
        if (chunkPosition + count > ChunkSize) {
            newChunk();
        }
        chunkPosition += count;
        return static_cast<int8_t*>(memory.back()) + chunkPosition - count;
    }

    void deallocate(void* ptr, size_t count) {
        freeMemory[count].push_back(ptr);
    }

    virtual ~FixedMemoryManager() {
        /*for (size_t i = 0; i < memory.size(); ++i) {
            ::operator delete(memory[i]);
        }*/
    }
};

template<size_t ChunkSize>
size_t FixedMemoryManager<ChunkSize>::chunkPosition = ChunkSize;

template<size_t ChunkSize>
std::vector<void*> FixedMemoryManager<ChunkSize>::memory;

template<size_t ChunkSize>
std::vector<size_t> FixedMemoryManager<ChunkSize>::chunkNum;

template<size_t ChunkSize>
std::map<size_t, std::vector<void*>>
    FixedMemoryManager<ChunkSize>::freeMemory;


template<typename T>
class FastAllocator {
private:
    static const size_t CHUNK_SIZE = 32;
    static const size_t USE_MANAGER_C = 32;
    FixedMemoryManager<CHUNK_SIZE> memoryManager;
public:
    template<typename U> 
    struct rebind {
        typedef FastAllocator<U> other;
    };

    typedef T value_type;

    FastAllocator() = default;

    template<typename U>
    FastAllocator(const FastAllocator<U>& alloc) {
        memoryManager = alloc.getMemoryManager();
    }

    const FixedMemoryManager<CHUNK_SIZE>& getMemoryManager() const {
        return memoryManager;
    }

    T* allocate(size_t count) {
        void* m;
        if (count * sizeof(T) <= USE_MANAGER_C) {
            m = memoryManager.allocate(count * sizeof(T));
        } else {
            m = ::operator new(count * sizeof(T));
        }
        return static_cast<T*>(m);
    }

    void deallocate(T* ptr, size_t count) {
        if (count * sizeof(T) <= USE_MANAGER_C) {
            memoryManager.deallocate(ptr, count * sizeof(T));
        } else {
            ::operator delete(ptr);
        }
    }

    template<typename... Args>
    void construct(T* ptr, const Args&... args) { // used by test, therefore i can't remove
        new(ptr) T(args...);
    }

    const FastAllocator& operator=(const FastAllocator& alloc) {
        memoryManager = alloc.memoryManager;
        return *this;
    }

    bool operator==(const FastAllocator<T>& alloc) const {
        return memoryManager == alloc.memoryManager;
    }

    bool operator!=(const FastAllocator<T>& alloc) const {
        return !operator==(alloc);
    }

    virtual ~FastAllocator() {}
};

template <typename T, typename Allocator = std::allocator<T> >
class List {
private:
    struct Node {
        Node* left = nullptr;
        Node* right = nullptr;
        T value;
        Node() : value(T()) {}
        Node(const T& value) : value(value) {}
        Node(const Node& node) : left(node.left),
            right(node.right), value(node.value) {}
        virtual ~Node() {}
    };

    template<typename IType, bool IsReversed>
    class IteratorT : public std::
        iterator<std::bidirectional_iterator_tag, IType> {
    private:
        friend List;

        static const bool isConst = std::is_same<IType, const IType>();
        
        using ThisIter = IteratorT<IType, IsReversed>;
        
        Node* ptr;

    public:
        IteratorT() : ptr(nullptr) {}

        IteratorT(Node* ptr) : ptr(ptr) {}

        IteratorT(const ThisIter& it) {
            ptr = it.ptr;
        }
        
        operator IteratorT<const IType, IsReversed>() const {
            return IteratorT<const IType, IsReversed>(ptr);
        }

        ThisIter& operator=(const ThisIter& it) {
            ptr = it.ptr;
            return *this;
        }

        IteratorT<IType, false> base() const {
            return IteratorT<IType, false>(ptr->right);
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

        IType& operator*() const {
            return ptr->value;
        }

        IType* operator->() const {
            return &operator*();
        }

        virtual ~IteratorT() {}
    };

    using ThisList = List<T, Allocator>;

    using NodeAllocator = typename Allocator::template rebind<Node>::other;

    NodeAllocator alloc;

    Allocator tAlloc;

    Node* rootNode;

    size_t sz;

public:
    using iterator = IteratorT<T, false>;

    using const_iterator = IteratorT<const T, false>;

    using reverse_iterator = IteratorT<T, true>;

    using const_reverse_iterator = IteratorT<const T, true>;

    explicit List(const Allocator& allocator = Allocator()) :
        alloc(allocator), tAlloc(allocator),
        rootNode(alloc.allocate(1)), sz(0) {
        rootNode->left = rootNode;
        rootNode->right = rootNode;
    }

    explicit List(size_t count, const Allocator&
        allocator = Allocator()) : alloc(allocator),
        tAlloc(allocator), rootNode(alloc.allocate(1)) {
        std::cerr << 'b';
        sz = count;
        Node* prevNode = rootNode;
        Node* v;
        for (size_t i = 0; i < count; ++i) {
            v = alloc.allocate(1);
            alloc.construct(v);
            v->left = prevNode;
            prevNode->right = v;
            prevNode = v;
        }
        prevNode->right = rootNode;
        rootNode->left = prevNode;
    }
    
    List(size_t count, const T& x,
        const Allocator& allocator = Allocator()) :
        alloc(allocator), tAlloc(allocator), rootNode(alloc.allocate(1)) {
        std::cerr << 'c';
        sz = count;
        Node* prevNode = rootNode;
        Node* v;
        for (size_t i = 0; i < count; ++i) {
            v = alloc.allocate(1);
            std::allocator_traits<NodeAllocator>::construct(alloc, v, x);
            v->left = prevNode;
            prevNode->right = v;
            prevNode = v;
        }
        prevNode->right = rootNode;
        rootNode->left = prevNode;
    }
    
    List(const ThisList& list, bool changeCopyAllocator = true) {
        std::cerr << 'd';
        if (changeCopyAllocator/* && std::allocator_traits<NodeAllocator>::
            propagate_on_container_copy_assignment::value*/) {
            alloc = std::allocator_traits<NodeAllocator>::
                select_on_container_copy_construction(list.alloc);
            tAlloc = std::allocator_traits<Allocator>::
                select_on_container_copy_construction(list.tAlloc);
        } else { // ?
            alloc = list.alloc;
            tAlloc = list.tAlloc;
        }
        
        rootNode = alloc.allocate(1);
        sz = list.sz;
        Node* prevNode = rootNode;
        Node* v;
        for (auto it = list.begin(); it != list.end(); ++it) {
            v = alloc.allocate(1);
            std::allocator_traits<NodeAllocator>::construct(alloc, v, *it);
            v->left = prevNode;
            prevNode->right = v;
            prevNode = v;
        }
        prevNode->right = rootNode;
        rootNode->left = prevNode;
    }

    void swap(ThisList& list, bool swapAllocators = true) {
        if (swapAllocators &&
            std::allocator_traits<NodeAllocator>::
            propagate_on_container_swap::value) {
            std::swap(alloc, list.alloc);
            std::swap(tAlloc, list.tAlloc);
        }
        std::swap(rootNode, list.rootNode);
        std::swap(sz, list.sz);
    }

    ThisList& operator=(const ThisList& list) {
        std::cerr << 'e';
        ThisList cp(list, false);
        swap(cp, false);
        cp.alloc = alloc;
        cp.tAlloc = tAlloc; 
        if (std::allocator_traits<NodeAllocator>::
            propagate_on_container_copy_assignment::value) {
            alloc = list.alloc;
            tAlloc = list.tAlloc;
        } /*else {
            alloc = tAlloc; // ???
        }*/
        return *this;
    }

    size_t size() const {
        return sz;
    }

    void push_back(const T& x) {
        Node* v = alloc.allocate(1); 
        std::allocator_traits<NodeAllocator>::construct(alloc, v, x);
        rootNode->left->right = v;
        v->left = rootNode->left;
        rootNode->left = v;
        v->right = rootNode;
        ++sz;
    }

    void push_front(const T& x) {
        Node* v = alloc.allocate(1);
        std::allocator_traits<NodeAllocator>::construct(alloc, v, x);
        rootNode->right->left = v;
        v->right = rootNode->right;
        rootNode->right = v;
        v->left = rootNode;
        ++sz;
    }

    void pop_back() {
        Node* v = rootNode->left;
        v->left->right = rootNode;
        rootNode->left = v->left;
        std::allocator_traits<NodeAllocator>::destroy(alloc, v);
        alloc.deallocate(v, 1);
        --sz;
    }

    void pop_front() {
        Node* v = rootNode->right;
        v->right->left = rootNode;
        rootNode->right = v->right;
        std::allocator_traits<NodeAllocator>::destroy(alloc, v);
        alloc.deallocate(v, 1);
        --sz;
    }

    void insert(const const_iterator& it, const T& x) {
        Node* u = alloc.allocate(1);
        std::allocator_traits<NodeAllocator>::construct(alloc, u, x);
        Node* v = it.ptr->left;
        u->right = it.ptr;
        u->left = v;
        it.ptr->left = u;
        v->right = u;
        ++sz;
    }
    
    void erase(const const_iterator& it) {
        Node* u = it.ptr->left;
        Node* v = it.ptr->right;
        u->right = v;
        v->left = u;
        std::allocator_traits<NodeAllocator>::destroy(alloc, it.ptr);
        std::allocator_traits<NodeAllocator>::deallocate(alloc, it.ptr, 1);
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
        return tAlloc;
    }

    virtual ~List() {
        for (Node* v = rootNode->right; v != rootNode;) {
            Node* u = v->right;
            std::allocator_traits<NodeAllocator>::destroy(alloc, v);
            alloc.deallocate(v, 1);
            v = u;
        }
        alloc.deallocate(rootNode, 1);
    }
};
