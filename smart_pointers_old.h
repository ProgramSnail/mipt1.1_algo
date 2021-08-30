#include <iostream>
#include <memory>

template<typename U>
class WeakPtr;

template<typename T>
class SharedPtr {
private:
    template<typename U>
    friend class SharedPtr;

    template<typename U>
    friend class WeakPtr;

    template<typename U, typename Alloc, typename... Args>
    friend SharedPtr<U> allocateShared(const Alloc& alloc, Args&&... args);

    class ControlBlockBase {
    protected:
        virtual ~ControlBlockBase() {}
    public:
        std::pair<size_t, size_t> counters; // change to weak and shared counter

        virtual void destroyObj() {}

        virtual void destroy() {}

        virtual T* getObj() {
            return nullptr;
        }

        explicit ControlBlockBase(
                const std::pair<size_t, size_t>& counters = {0, 0}) 
            : counters(counters) {}
    };

    template<typename U, typename Alloc>
    struct ControlBlock : public ControlBlockBase {
        using ThisAlloc = typename std::allocator_traits<Alloc>
            ::template rebind_alloc<ControlBlock<U, Alloc> >;
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

        T* getObj() override {
            return &obj;
        }

        template<typename... Args>
        ControlBlock(const Alloc &alloc_, Args&&... args) :
            obj(std::forward<Args>(args)...), alloc(alloc_), objAlloc(alloc_) {}

        /*template<typename W>
        ControlBlock(const typename SharedPtr<W>::template ControlBlock<U, Alloc>& controlBlock) :
            ControlBlockBase(controlBlock.counters), obj(controlBlock.obj),
            alloc(controlBlock.alloc), objAlloc(controlBlock.objAlloc) {}

        template<typename W>
        ControlBlock(const typename SharedPtr<W>::ControlBlockBase& controlBlock) :
            ControlBlock(reinterpret_cast<const typename
                SharedPtr<W>::template ControlBlock<U, Alloc>&>(controlBlock)) {}*/
    };

    class HelpersBase {
    protected:
        virtual ~HelpersBase() {}
    public:
        std::pair<size_t, size_t> counters;
        
        virtual void callDeleter(T*) {}

        virtual void destroy() {}

        HelpersBase(std::pair<size_t, size_t>
            counters = {0, 0}) : counters(counters) {}     
    };

    template<typename U, typename Deleter, typename Alloc>
    class Helpers : public HelpersBase {
    private:
        using ThisAlloc = typename std::allocator_traits<Alloc>
            ::template rebind_alloc<Helpers<U, Deleter, Alloc>>;

        Deleter deleter;

        ThisAlloc alloc;
    public:
        void callDeleter(T* ptr) override {
            deleter(ptr);
        }

        void destroy() override {
            ThisAlloc tmpAlloc = alloc;
            deleter.~Deleter();
            alloc.~ThisAlloc();
            tmpAlloc.deallocate(this, 1);
        }

        Helpers(const Deleter& deleter, const Alloc& alloc) :
            deleter(deleter), alloc(alloc) {}

        template<typename W>
        Helpers(const typename SharedPtr<W>::template Helpers<U, Deleter, Alloc>& helpers) :
            HelpersBase(helpers.counters), deleter(helpers.deleter), alloc(helpers.alloc) {}

        template<typename W>
        Helpers(const typename SharedPtr<W>::HelpersBase& helpers) :
            Helpers(reinterpret_cast<const typename
                SharedPtr<W>::template Helpers<U, Deleter, Alloc>&>(helpers)) {}
    };

    struct MakeSharedT {};

    HelpersBase* helpers = nullptr;
    
    T* ptr = nullptr;
    
    ControlBlockBase* controlBlock = nullptr;

    void increaseCounter(int value) {
        if (!helpers && !controlBlock) {
            return;
        }
        if (helpers) {
            helpers->counters.first += value;
        } else {
            controlBlock->counters.first += value;
        }   
    }

    SharedPtr(MakeSharedT, ControlBlockBase* controlBlock) : controlBlock(controlBlock) {
        increaseCounter(1);
    }

    template<typename U>
    SharedPtr(const WeakPtr<U>& p) : helpers(p.helpers),
            ptr(p.ptr), controlBlock(p.controlBlock) {
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
        using HelpersAlloc = typename std::allocator_traits<Alloc>
            ::template rebind_alloc<Helpers<U, Deleter, Alloc> >;
        HelpersAlloc helpersAlloc(alloc);
        Helpers<U, Deleter, Alloc>* tmpHelpers = helpersAlloc.allocate(1);
        new (tmpHelpers) Helpers<U, Deleter, Alloc>(d, alloc);
        helpers = tmpHelpers;
        increaseCounter(1);
    }

    template<typename U>
    SharedPtr(const SharedPtr<U>& p) :
            helpers(reinterpret_cast<HelpersBase*>(p.helpers)), ptr(p.ptr), 
            controlBlock(reinterpret_cast<ControlBlockBase*>(p.controlBlock)) {
        increaseCounter(1);
    }

    template<typename U>
    SharedPtr(SharedPtr<U>&& p) :
            helpers(reinterpret_cast<HelpersBase*>(p.helpers)), ptr(p.ptr), 
            controlBlock(reinterpret_cast<ControlBlockBase*>(p.controlBlock)) {
        SharedPtr<U>().swap(p);
    }

    SharedPtr(const SharedPtr<T>& p) : helpers(p.helpers),
        ptr(p.ptr), controlBlock(p.controlBlock) {
        increaseCounter(1);
    }

    SharedPtr(SharedPtr<T>&& p) {
        swap(p);
    }

    template<typename U>
    const SharedPtr<T>& operator=(const SharedPtr<U>& p) {
        destroy();
        helpers = reinterpret_cast<HelpersBase*>(p.helpers);
        ptr = p.ptr;
        controlBlock = reinterpret_cast<
            ControlBlockBase*>(p.controlBlock);
        increaseCounter(1);
        return *this;
    }

    template<typename U>
    const SharedPtr<T>& operator=(SharedPtr<U>&& p) {
        SharedPtr<T>().swap(*this);
        SharedPtr<T>(p).swap(*this);
        SharedPtr<U>().swap(p);
        return *this;
    } 

    const SharedPtr<T>& operator=(const SharedPtr<T>& p) {
        destroy();
        helpers = p.helpers;
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
        if (!helpers && !controlBlock) {
            return 0;
        }
        if (helpers) {
            return helpers->counters.first;
        } else {
            return controlBlock->counters.first;
        }
    }

    void swap(SharedPtr<T>& p) {
        std::swap(helpers, p.helpers);
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
        if (!helpers && !controlBlock) {
            return nullptr;
        }
        if (helpers) {
            return ptr;
        } else {
            return controlBlock->getObj();
        };
    }

    T& operator*() const {
        return *get();
    }

    T* operator->() const {
        return get();
    }

    void destroy() {
        if (!helpers && !controlBlock) {
            return;
        }
        increaseCounter(-1);
        if (helpers) {
            if (helpers->counters.first == 0) {
                helpers->callDeleter(ptr);
                if (helpers->counters.second == 0) {
                    helpers->destroy();
                }
            }
        } else {
            if (controlBlock->counters.first == 0) {
                controlBlock->destroyObj();
                if (controlBlock->counters.second == 0) {
                    controlBlock->destroy();
                }
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

    typename SharedPtr<T>::HelpersBase* helpers = nullptr;

    T* ptr = nullptr;

    typename SharedPtr<T>::ControlBlockBase* controlBlock = nullptr;

    void increaseCounter(int value) {
        if (!helpers && !controlBlock) {
            return;
        }
        if (helpers) { 
            helpers->counters.second += value;
        } else {
            controlBlock->counters.second += value;
        }
    }
public:
    WeakPtr() {}

    template<typename U> 
    WeakPtr(const SharedPtr<U>& p) : helpers(reinterpret_cast<typename
            SharedPtr<T>::HelpersBase*>(p.helpers)), ptr(p.ptr), controlBlock(
            reinterpret_cast<typename SharedPtr<T>::ControlBlockBase*>(p.controlBlock)) {
        increaseCounter(1);
    }

    template<typename U>
    WeakPtr(const WeakPtr<U>& p) : helpers(reinterpret_cast<typename
            SharedPtr<T>::HelpersBase*>(p.helpers)), ptr(p.ptr), controlBlock(
            reinterpret_cast<typename SharedPtr<T>::ControlBlockBase*>(p.controlBlock)) {
        increaseCounter(1);
    }

    template<typename U>
    WeakPtr(WeakPtr<U>&& p) : helpers(reinterpret_cast<typename
            SharedPtr<T>::HelpersBase*>(p.helpers)), ptr(p.ptr), controlBlock(
            reinterpret_cast<typename SharedPtr<T>::ControlBlockBase*>(p.controlBlock)) {
        WeakPtr<U>().swap(p);
    }

    WeakPtr(const WeakPtr<T>& p) : helpers(p.helpers),
            ptr(p.ptr), controlBlock(p.controlBlock) {
        increaseCounter(1);
    }

    WeakPtr(WeakPtr<T>&& p) {
        swap(p);
    }

    template<typename U>
    const WeakPtr<T>& operator=(const WeakPtr<U>& p) {
        destroy();
        helpers = reinterpret_cast<typename
            SharedPtr<T>::HelpersBase*>(p.helpers);
        ptr = p.ptr;
        controlBlock = reinterpret_cast<typename
            SharedPtr<T>::ControlBlockBase*>(p.controlBlock);
        increaseCounter(1);
        return *this;
    }

    template<typename U>
    const WeakPtr<T>& operator=(WeakPtr<U>&& p) {
        WeakPtr<T>().swap(*this);
        WeakPtr<T>(p).swap(*this);
        WeakPtr<U>().swap(p);
        return *this;
    }

    const WeakPtr<T>& operator=(const WeakPtr<T>& p) {
        destroy();
        helpers = p.helpers;
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

    void swap(WeakPtr& p) {
        std::swap(helpers, p.helpers);
        std::swap(ptr, p.ptr);
        std::swap(controlBlock, p.controlBlock);
    }

    bool expired() const {
        if (!helpers && !controlBlock) {
            return true;
        }
        if (helpers) {
            return helpers->counters.first == 0;
        } else {
            return controlBlock->counters.first == 0;
        }
    }

    SharedPtr<T> lock() const {
        return SharedPtr<T>(*this);
    }

    size_t use_count() const {
        if (!helpers && !controlBlock) {
            return 0;
        }
        if (helpers) {
            return helpers->counters.first;
        } else {
            return controlBlock->counters.first;
        }
    }

    void destroy() {
        if (!helpers && !controlBlock) {
            return;
        }
        increaseCounter(-1);
        if (helpers) {
            if (helpers->counters.first == 0 && helpers->counters.second == 0) {
                helpers->destroy(); 
            }
        } else {
            if (controlBlock->counters.first == 0 && controlBlock->counters.second == 0) {
                controlBlock->destroy();
            }
        }
    }

    virtual ~WeakPtr() {
        destroy();
    }
};

template<typename T, typename Alloc = std::allocator<T>, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc = Alloc(),  Args&&... args) {
    using ThisAlloc = typename std::allocator_traits<Alloc>::
        template rebind_alloc<typename SharedPtr<T>::template ControlBlock<T, Alloc> >;
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