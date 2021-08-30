#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>
#include <set>
#include <stdlib.h>

template<size_t ChunkSize>
class FixedMemoryManager  {
private:
    using ThisManager = FixedMemoryManager<ChunkSize>;

    const size_t BLOCK_SIZE = 8;

    const size_t CLEAR_FREE_SPACE_C = 16;

    const size_t MEMORY_SIZE_IS_BIG_C = 1024;

    std::vector<void*> blocks_;

    std::vector<void*> freeMemory_;

    size_t lastBlockPos_ = BLOCK_SIZE;

    size_t lastBlockDeallocatedPtrCount_ = 0;

    void newChunk() {
        void* ptr = aligned_alloc(ChunkSize * BLOCK_SIZE, ChunkSize * BLOCK_SIZE);
        lastBlockPos_ = 0;
        lastBlockDeallocatedPtrCount_ = 0;
        blocks_.push_back(ptr);
    }

    void* getBlockByPtr(void* ptr) {
        return static_cast<int8_t*>(ptr) - 
            reinterpret_cast<int64_t>(ptr) % (ChunkSize * BLOCK_SIZE);
    } 

public:
    FixedMemoryManager() = default;

    void* allocate() {
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

    void deallocate(void* ptr) {
        if (getBlockByPtr(ptr) == blocks_.back()) {
            ++lastBlockDeallocatedPtrCount_;
        }
        freeMemory_.push_back(ptr);
        if (freeMemory_.size() >= blocks_.size() * BLOCK_SIZE - CLEAR_FREE_SPACE_C
                && freeMemory_.size() > MEMORY_SIZE_IS_BIG_C) {
            clear();
        }
    }

    void clear() {
        std::vector<void*> emptyBlocks;
        std::set<void*> notFreePositions;
        for (size_t i = 0; i < freeMemory_.size(); ++i) {
            emptyBlocks.insert(getBlockByPtr(freeMemory_[i]));
        }
        std::vector<void*> newBlocks;
        for (size_t i = 0; i < blocks_.size(); ++i) {
            if (notEmptyBlocks.count(blocks_[i]) == 0) {
                free(blocks_[i]);
            }
        }
        blocks_.clear();
        freeMemory_.clear();
        lastBlockPos_ = BLOCK_SIZE;
    }

    virtual ~FixedMemoryManager() {
        clear();
    }
};

inline FixedMemoryManager<1> memoryManager1;
inline FixedMemoryManager<2> memoryManager2;
inline FixedMemoryManager<4> memoryManager4;
inline FixedMemoryManager<8> memoryManager8;
inline FixedMemoryManager<16> memoryManager16;
inline FixedMemoryManager<32> memoryManager32;

template<typename T>
class FastAllocator {
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
        if (allocateSize <= 1) {
            m = memoryManager1.allocate();
        } else {
            if (allocateSize <= 2) {
                m = memoryManager2.allocate();
            } else {
                if (allocateSize <= 4) {
                    m = memoryManager4.allocate();
                } else {
                    if (allocateSize <= 8) {
                        m = memoryManager8.allocate();
                    } else {
                        if (allocateSize <= 16) {
                            m = memoryManager16.allocate();
                        } else {
                            if (allocateSize <= 32) {
                                m = memoryManager32.allocate();
                            } else {
                                m = ::operator new(allocateSize);
                            }
                        }
                    }
                }
            }
        }
        return static_cast<T*>(m);
    }

    void deallocate(T* ptr, size_t count) {
        size_t deallocateSize = count * sizeof(T);
        if (deallocateSize <= 1) {
            memoryManager1.deallocate(ptr);
        } else {
            if (deallocateSize <= 2) {
                memoryManager2.deallocate(ptr);
            } else {
                if (deallocateSize <= 4) {
                    memoryManager4.deallocate(ptr);
                } else {
                    if (deallocateSize <= 8) {
                        memoryManager8.deallocate(ptr);
                    } else {
                        if (deallocateSize <= 16) {
                            memoryManager16.deallocate(ptr);
                        } else {
                            if (deallocateSize <= 32) {
                                memoryManager32.deallocate(ptr);
                            } else {
                                ::operator delete(ptr);
                            }
                        }
                    }
                }
            }
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

template <typename T, typename Allocator = std::allocator<T> >
class List {
private:
    struct Node {
        T value;
        Node* left = nullptr;
        Node* right = nullptr;
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
        sz = count;
        Node* prevNode = rootNode;
        Node* v;
        for (size_t i = 0; i < count; ++i) {
            v = alloc.allocate(1);
            std::allocator_traits<NodeAllocator>::construct(alloc, v);
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
        if (changeCopyAllocator) {
            alloc = std::allocator_traits<NodeAllocator>::
                select_on_container_copy_construction(list.alloc);
            tAlloc = std::allocator_traits<Allocator>::
                select_on_container_copy_construction(list.tAlloc);
        } else {
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
        ThisList cp(list, false);
        swap(cp, false);
        cp.alloc = alloc;
        cp.tAlloc = tAlloc; 
        if (std::allocator_traits<NodeAllocator>::
            propagate_on_container_copy_assignment::value) {
            alloc = list.alloc;
            tAlloc = list.tAlloc;
        }
        return *this;
    }

    size_t size() const {
        return sz;
    }

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

    void push_back(const T& x) {
        Node* v = alloc.allocate(1); 
        std::allocator_traits<NodeAllocator>::construct(alloc, v, x);
        insert_back(v);
        ++sz;
    }

    void push_front(const T& x) {
        Node* v = alloc.allocate(1);
        std::allocator_traits<NodeAllocator>::construct(alloc, v, x);
        insert_front(v);
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
        Node* v = alloc.allocate(1);
        std::allocator_traits<NodeAllocator>::construct(alloc, v, x);
        insert_inside(v, it.ptr->left, it.ptr);
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
            Node* u = v;
            v = v->right;
            std::allocator_traits<NodeAllocator>::destroy(alloc, u);
            alloc.deallocate(u, 1);
        }
        alloc.deallocate(rootNode, 1);
    }
};
