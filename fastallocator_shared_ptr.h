#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>

namespace Details {
    using UnorderedMapType = std::unordered_map<size_t,
        std::pair<std::pair<std::vector<void*>, size_t>,
        std::vector<void*>>>;

    void clearMemory(UnorderedMapType* m) {
        for (auto& it : (*m)) {
            for (size_t i = 0; i < it.second.first.first.size(); ++i) {
                ::operator delete(it.second.first.first[i]);
            }
        }
        m->clear();
        delete m;
    }
}

template<size_t ChunkSize>
class FixedMemoryManager {
private:
    
    std::shared_ptr<Details::UnorderedMapType> globalMemory_;

    using ThisManager = FixedMemoryManager<ChunkSize>;

    size_t getNearest2Pow(size_t x) {
        size_t ans = 1;
        for (; ans < x; ans *= 2) {}
        return ans;
    }

    void newChunk(size_t sz) {
        void* ptr = ::operator new(ChunkSize * sz);
        (*globalMemory_)[sz].first.second = 0;
        (*globalMemory_)[sz].first.first.push_back(ptr);
    }

public:
    FixedMemoryManager() : globalMemory_(
            new Details::UnorderedMapType, Details::clearMemory) {};

    FixedMemoryManager(const ThisManager& memoryManager) 
            : globalMemory_(memoryManager.globalMemory_) {}

    void swap(ThisManager& memoryManager) {
        std::swap(globalMemory_, memoryManager.globalMemory_);
    }

    const ThisManager& operator=(const ThisManager& memoryManager) {
        globalMemory_ = memoryManager.globalMemory_;
        return *this;
    }

    bool operator==(const ThisManager& memoryManager) const {
        return globalMemory_ == memoryManager.globalMemory_;
    }

    bool operator!=(const ThisManager& manager) const {
        return !operator==(manager);
    }

    void* allocate(size_t sz) {
        sz = getNearest2Pow(sz);
        auto& memory = (*globalMemory_);
        auto it = memory.find(sz);
        if (it == memory.end()) {
            newChunk(sz);
            it = memory.find(sz);
        }
        size_t& pos = it->second.first.second;
        std::vector<void*>& v = it->second.second;
        if (v.size() > 0) {
            void* ptr = v.back();
            v.pop_back();
            return ptr;
        }
        if (pos >= ChunkSize) {
            newChunk(sz);
        }
        ++pos;
        return static_cast<int8_t*>(it->second.first.first.back()) + (pos - 1) * sz;
    }

    void deallocate(void* ptr, size_t count) {
        //::operator delete(ptr);
        (*globalMemory_)[count].second.push_back(ptr);
    }

    virtual ~FixedMemoryManager() {
        // clearMemory();
    }
};

/*
template<size_t ChunkSize>
std::unordered_map<size_t, std::pair<std::pair<void*, size_t>,
    std::vector<void*>>> FixedMemoryManager<ChunkSize>::globalMemory_;*/

template<typename T>
class FastAllocator {
private:
    static const size_t CHUNK_SIZE = 1024;
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
    void construct(T* ptr, const Args&... args) {
        new(ptr) T(args...);
    }

    void destroy(T* ptr) {
        ptr->~T();
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

/*
template<typename T>
FixedMemoryManager<FastAllocator<T>::CHUNK_SIZE> FastAllocator<T>::memoryManager;*/

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
            if (alloc != list.alloc) {
                alloc = list.alloc;
            }
            if (tAlloc != list.tAlloc) {
                tAlloc = list.tAlloc;
            }
        }
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
