#include <iostream>
#include <memory>

template<typename U>
class WeakPtr;

namespace Details {
    class ControlBlockBase {
    protected:
        virtual ~ControlBlockBase() {}
    public:  
        size_t sharedCounter;

        size_t weakCounter; 

        virtual void destroy() {} 

        ControlBlockBase(size_t sharedCounter = 0, size_t weakCounter = 0) 
            : sharedCounter(sharedCounter), weakCounter(weakCounter) {}
    };

    class ControlBlockWithObjectBase : public ControlBlockBase {
    protected:
        virtual ~ControlBlockWithObjectBase() {}
    public:
        virtual void destroyObj() {}

        virtual void* getObject() {
            return nullptr;
        }

        using ControlBlockBase::ControlBlockBase;
    };

    template<typename U, typename Alloc>
    class ControlBlockWithObject : public ControlBlockWithObjectBase {
        using ThisAlloc = typename std::allocator_traits<Alloc>
            ::template rebind_alloc<ControlBlockWithObject<U, Alloc> >;
    private:
        U obj;

        ThisAlloc alloc;

        Alloc objAlloc;
    public:
        void destroyObj() override {
            std::allocator_traits<Alloc>::destroy(objAlloc, &obj);
        }

        void destroy() override {
            ThisAlloc tmpAlloc = alloc;
            alloc.~ThisAlloc();
            tmpAlloc.deallocate(this, 1);
        }

        void* getObject() override {
            return &obj;
        }

        template<typename... Args>
        ControlBlockWithObject(const Alloc &alloc_, Args&&... args) 
                : obj(std::forward<Args>(args)...), alloc(alloc_), objAlloc(alloc_) {}
    };

    class ControlBlockWithoutObjectBase : public ControlBlockBase {
    protected:
        virtual ~ControlBlockWithoutObjectBase() {}
    public:
        virtual void callDeleter(void*) {}

        using ControlBlockBase::ControlBlockBase; 
    };
    

    template<typename U, typename Deleter, typename Alloc>
    class ControlBlockWithoutObject : public ControlBlockWithoutObjectBase {
    private:
        using ThisAlloc = typename std::allocator_traits<Alloc>
            ::template rebind_alloc<ControlBlockWithoutObject<U, Deleter, Alloc>>;

        Deleter deleter;

        ThisAlloc alloc;
    public:
        void callDeleter(void* ptr) override {
            deleter(static_cast<U*>(ptr));
        }

        void destroy() override {
            ThisAlloc tmpAlloc = alloc;
            deleter.~Deleter();
            alloc.~ThisAlloc();
            tmpAlloc.deallocate(this, 1);
        }

        ControlBlockWithoutObject(const Deleter& deleter, const Alloc& alloc) :
            deleter(deleter), alloc(alloc) {}
    };
}

template<typename T>
class SharedPtr {
private:

    template<typename U>
    friend class SharedPtr;

    template<typename U>
    friend class WeakPtr;

    template<typename U, typename Alloc, typename... Args>
    friend SharedPtr<U> allocateShared(const Alloc& alloc, Args&&... args);

    struct MakeSharedT {};
    
    T* ptr = nullptr;

    Details::ControlBlockBase* controlBlock = nullptr;

    void increaseCounter(int value) {
        if (!controlBlock) {
            return;
        }
        controlBlock->sharedCounter += value;
    }

    SharedPtr(MakeSharedT, Details::ControlBlockWithObjectBase* controlBlock) 
            : controlBlock(controlBlock) {
        increaseCounter(1);
    }

    template<typename U>
    SharedPtr(const WeakPtr<U>& p) : ptr(p.ptr), controlBlock(p.controlBlock) {
        increaseCounter(1);
    }

public:
    SharedPtr() {}
    
    template<typename U, typename Deleter = std::default_delete<U>,
        typename Alloc = std::allocator<U>> 
    SharedPtr(U* ptr, const Deleter& d = Deleter(),
            const Alloc& alloc = Alloc()) : ptr(ptr) {
        if (ptr == nullptr) {
            return;
        }
        using ControlBlockWithoutObjectAlloc = typename std::allocator_traits<Alloc>
            ::template rebind_alloc<Details::ControlBlockWithoutObject<U, Deleter, Alloc> >;
        ControlBlockWithoutObjectAlloc controlBlockWithoutObjectAlloc(alloc);
        Details::ControlBlockWithoutObject<U, Deleter, Alloc>* 
            temporaryControlBlockWithoutObject = controlBlockWithoutObjectAlloc.allocate(1);
        new (temporaryControlBlockWithoutObject) 
            Details::ControlBlockWithoutObject<U, Deleter, Alloc>(d, alloc);
        controlBlock = temporaryControlBlockWithoutObject;
        increaseCounter(1);
    }

    template<typename U>
    SharedPtr(const SharedPtr<U>& p) : ptr(p.ptr), controlBlock(p.controlBlock) {
        increaseCounter(1);
    }

    template<typename U>
    SharedPtr(SharedPtr<U>&& p) : ptr(p.ptr), controlBlock(p.controlBlock) {
        SharedPtr<U>().swap(p);
    }

    SharedPtr(const SharedPtr<T>& p) : ptr(p.ptr), controlBlock(p.controlBlock) {
        increaseCounter(1);
    }

    SharedPtr(SharedPtr<T>&& p) {
        swap(p);
    }

    template<typename U>
    const SharedPtr<T>& operator=(const SharedPtr<U>& p) {
        destroy();
        ptr = p.ptr;
        controlBlock = p.controlBlock;
        increaseCounter(1);
        return *this;
    }

    template<typename U>
    const SharedPtr<T>& operator=(SharedPtr<U>&& p) {
        reset();
        ptr = p.ptr;
        p.ptr = nullptr;
        std::swap(controlBlock, p.controlBlock);
        return *this;
    } 

    const SharedPtr<T>& operator=(const SharedPtr<T>& p) {
        destroy();
        ptr = p.ptr;
        controlBlock = p.controlBlock;
        increaseCounter(1);
        return *this;
    }

    const SharedPtr<T>& operator=(SharedPtr<T>&& p) {
        SharedPtr<T>().swap(*this);
        swap(p);
        return *this;
    }

    size_t use_count() const {
        if (!controlBlock) {
            return 0;
        }
        return controlBlock->sharedCounter;
    }

    void swap(SharedPtr<T>& p) {
        std::swap(ptr, p.ptr);
        std::swap(controlBlock, p.controlBlock);
    }

    void reset() {
        SharedPtr<T>().swap(*this);
    }

    template<typename U>
    void reset(U* ptr) {
        if (ptr == nullptr) {
            reset();
            return;
        }
        SharedPtr<T>(ptr).swap(*this);
    }

    T* get() const {
        if (!controlBlock) {
            return nullptr;
        }
        if (ptr) {
            return ptr;
        } else {
            return static_cast<T*>(static_cast<Details
                ::ControlBlockWithObjectBase*>(controlBlock)->getObject());
        };
    }

    T& operator*() const {
        return *get();
    }

    T* operator->() const {
        return get();
    }

    void destroy() {
        if (!controlBlock) {
            return;
        }
        increaseCounter(-1);
        if (controlBlock->sharedCounter == 0) {
            if (ptr) {
                static_cast<Details::ControlBlockWithoutObjectBase*>
                    (controlBlock)->callDeleter(ptr);
            } else {
                static_cast<Details::ControlBlockWithObjectBase*>
                    (controlBlock)->destroyObj();
            }
            if (controlBlock->weakCounter == 0) {
                controlBlock->destroy();
            }
        }
    }

    ~SharedPtr() {
        destroy();
    }
};

template<typename T>
class WeakPtr {
private:
    template<typename U>
    friend class SharedPtr;

    template<typename U>
    friend class WeakPtr;

    T* ptr = nullptr;

    typename Details::ControlBlockBase* controlBlock = nullptr;

    void increaseCounter(int value) {
        if (!controlBlock) {
            return;
        }
        controlBlock->weakCounter += value;
    }
public:
    WeakPtr() {}

    template<typename U> 
    WeakPtr(const SharedPtr<U>& p) : ptr(p.ptr), controlBlock(p.controlBlock) {
        increaseCounter(1);
    }

    template<typename U>
    WeakPtr(const WeakPtr<U>& p) : ptr(p.ptr), controlBlock(p.controlBlock) {
        increaseCounter(1);
    }

    template<typename U>
    WeakPtr(WeakPtr<U>&& p) : ptr(p.ptr), controlBlock(p.controlBlock) {
        WeakPtr<U>().swap(p);
    }

    WeakPtr(const WeakPtr<T>& p) : ptr(p.ptr), controlBlock(p.controlBlock) {
        increaseCounter(1);
    }

    WeakPtr(WeakPtr<T>&& p) {
        swap(p);
    }

    template<typename U>
    const WeakPtr<T>& operator=(const WeakPtr<U>& p) {
        destroy();
        ptr = p.ptr;
        controlBlock = p.controlBlock;
        increaseCounter(1);
        return *this;
    }

    template<typename U>
    const WeakPtr<T>& operator=(WeakPtr<U>&& p) {
        WeakPtr<T>().swap(*this);
        ptr = p.ptr;
        p.ptr = nullptr;
        std::swap(controlBlock, p.controlBlock);
        p.reset();
        return *this;
    }

    const WeakPtr<T>& operator=(const WeakPtr<T>& p) {
        destroy();
        ptr = p.ptr;
        controlBlock = p.controlBlock;
        increaseCounter(1);
        return *this;
    }

    const WeakPtr<T>& operator=(WeakPtr<T>&& p) {
        WeakPtr<T>().swap(*this);
        swap(p);
        return *this;
    }

    void swap(WeakPtr<T>& p) {
        std::swap(ptr, p.ptr);
        std::swap(controlBlock, p.controlBlock);
    }

    bool expired() const {
        if (!controlBlock) {
            return true;
        }
        return controlBlock->sharedCounter == 0;
    }

    SharedPtr<T> lock() const {
        return SharedPtr<T>(*this);
    }

    size_t use_count() const {
        if (!controlBlock) {
            return 0;
        }
        return controlBlock->sharedCounter;
    }

    void destroy() {
        if (!controlBlock) {
            return;
        }
        increaseCounter(-1);
        if (controlBlock->sharedCounter == 0 && controlBlock->weakCounter == 0) {
            controlBlock->destroy();
        }
    }

    virtual ~WeakPtr() {
        destroy();
    }
};

template<typename T, typename Alloc = std::allocator<T>, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc = Alloc(),  Args&&... args) {
    using ThisAlloc = typename std::allocator_traits<Alloc>::
        template rebind_alloc<Details::ControlBlockWithObject<T, Alloc> >;
    ThisAlloc blockAlloc(alloc);
    auto ptr = blockAlloc.allocate(1);
    std::allocator_traits<ThisAlloc>::construct(blockAlloc,
        ptr, alloc, std::forward<Args>(args)...);
    return SharedPtr<T>(typename SharedPtr<T>::MakeSharedT(), ptr);
}

template<typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    return allocateShared<T>(std::allocator<T>(), std::forward<Args>(args)...);
}
