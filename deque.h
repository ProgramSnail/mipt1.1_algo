#include <vector>
#include <iostream>

template<typename T>
class Deque {
private:
    static const int BATCH_SIZE = 20;
    int min_batch = 0;
    int max_batch = 0;
    int first_batch = 0;
    int last_batch = 0;
    int begin_pos = 0;
    int end_pos = 0;
    using vec = std::vector<T>;
    std::vector<vec*> begin_array;
    std::vector<vec*> end_array;

    const vec& getBatch(int n) const {
        n -= min_batch;
        size_t m = n;
        return begin_array.size() > m ? 
            *begin_array[begin_array.size() - m - 1] 
            : *end_array[m - begin_array.size()];
    }

    vec& getBatch(int n) {
        return const_cast<vec&>(static_cast<const Deque<T>*>(this)->getBatch(n));
    }

    template<typename IType>
    class iterator_template;
public:
    using const_iterator = iterator_template<const T>;

    using iterator = iterator_template<T>;

    Deque() {
        end_array.push_back(new vec(BATCH_SIZE));
    }

    Deque(size_t n, T x = T()) : max_batch(n / BATCH_SIZE), 
        last_batch(max_batch), end_pos(n % BATCH_SIZE), end_array(max_batch + 1) {
        for (size_t i = 0; i < static_cast<size_t>(max_batch); ++i) {
            end_array[i] = new vec(BATCH_SIZE, x);
        }
        end_array[max_batch] = new vec(BATCH_SIZE);
        for (size_t i = 0; i < static_cast<size_t>(end_pos); ++i) {
            (*end_array[last_batch])[i] = x;
        }
    }

    Deque(const Deque<T>& d) :  min_batch(d.min_batch), max_batch(d.max_batch),
        first_batch(d.first_batch), last_batch(d.last_batch), 
        begin_pos(d.begin_pos), end_pos(d.end_pos), 
        begin_array(d.begin_array.size()), end_array(d.end_array.size()) {
        for (size_t i = 0; i < begin_array.size(); ++i) {
            begin_array[i] = new vec(*d.begin_array[i]);
        }
        for (size_t i = 0; i < end_array.size(); ++i) {
            end_array[i] = new vec(*d.end_array[i]);
        }
    } 

    void swap(Deque<T>& d) {
        std::swap(min_batch, d.max_batch); 
        std::swap(max_batch, d.min_batch); 
        std::swap(first_batch, d.first_batch);
        std::swap(last_batch, d.last_batch);
        std::swap(begin_pos, d.begin_pos);
        std::swap(end_pos, d.end_pos);
        std::swap(begin_array, d.begin_array);
        std::swap(end_array, d.end_array);
    }

    Deque<T>& operator=(const Deque<T>& d) {
        Deque<T> cp = d;
        swap(cp);
        return *this;
    }

    ~Deque() {
        for (size_t i = 0; i < begin_array.size(); ++i) {
            delete begin_array[i];
        }
        for (size_t i = 0; i < end_array.size(); ++i) {
            delete end_array[i];
        }
    }

    size_t size() const {
        return first_batch == last_batch ? (end_pos - begin_pos) : (end_pos 
            + BATCH_SIZE - begin_pos + (last_batch - first_batch - 1) * BATCH_SIZE);
    }

    T& operator[](size_t n) {
        n += begin_pos;
        return getBatch(n / BATCH_SIZE)[n % BATCH_SIZE];
    }
    
    const T& operator[](size_t n) const {
        n += begin_pos;
        return getBatch(n / BATCH_SIZE)[n % BATCH_SIZE];
    }

    T& at(size_t n) {
        if (n >= size()) {
            throw std::out_of_range("Deque.at() : out of range");
        }
        return operator[](n);
    }

    const T& at(size_t n) const {
        if (n >= size()) {
            throw std::out_of_range("Deque.at() const : out of range");
        }
        return operator[](n);
    }

    void push_back(T x) {
        getBatch(last_batch)[end_pos] = x;
        ++end_pos;
        if (end_pos == BATCH_SIZE) {
            end_pos = 0;
            ++last_batch;
            if (last_batch > max_batch) {
                ++max_batch;
                end_array.push_back(new vec(BATCH_SIZE));
            }
        }
    }

    void pop_back() {
        if (end_pos == 0) {
            end_pos = BATCH_SIZE;
            --last_batch;
        }
        --end_pos;
    }

    void push_front(T x) {
        if (begin_pos == 0) {
            begin_pos = BATCH_SIZE;
            --first_batch;
            if (first_batch < min_batch) {
                --min_batch;
                begin_array.push_back(new vec(BATCH_SIZE));
            }
        }
        begin_pos--;
        getBatch(first_batch)[begin_pos] = x;
    }

    void pop_front() {
        ++begin_pos;
        if (begin_pos == BATCH_SIZE) {
            begin_pos = 0;
            ++first_batch;
        }
    }

    iterator begin() {
        return iterator(this, first_batch, begin_pos);
    }

    const_iterator cbegin() const {
        return const_iterator(this, first_batch, begin_pos);
    }

    const_iterator begin() const {
        return cbegin();
    }

    iterator end() {
        return iterator(this, last_batch, end_pos);
    }

    const_iterator cend() const {
        return const_iterator(this, last_batch, end_pos);
    }

    const_iterator end() const {
        return cend();
    }

    void insert(iterator it, T x) {
        T y;
        for (; it != end(); ++it) {
            y = x;
            x = *it;
            *it = y;
        }
        push_back(x);
    }

    void erase(iterator it) {
        iterator it_next = it;
        it_next++;
        T x;
        for (; it_next != end(); ++it, ++it_next) {
            x = *it_next;
            *it_next = *it;
            *it = x;
        }
        pop_back();
    }
private:
    template<typename IType>
    class iterator_template {
    protected:
        static const bool IsConstant = std::is_same<IType, const T>();
        const Deque<T>* deque;
        int batch;
        int pos;
    public: 
        iterator_template(const Deque<T>* deque, int batch, int pos) : 
            deque(deque), batch(batch), pos(pos) {}

        operator iterator_template<const IType>() {
            return iterator_template<const IType>(deque, batch, pos);
        } 

        iterator_template<IType>& operator+=(int i) {
            pos += i;
            int x = pos / BATCH_SIZE;
            batch += x;
            pos -= x * BATCH_SIZE;
            if (pos < 0) {
                batch--;
                pos += BATCH_SIZE;
            }
            return *this;
        }

        iterator_template<IType>& operator-=(int i) {
            return operator+=(-i);
        }

        iterator_template<IType>& operator++() {
            operator+=(1);
            return *this;
        }

        iterator_template<IType> operator++(int) {
            iterator_template<IType> cp = *this;
            operator+=(1);
            return cp;
        }

        iterator_template<IType>& operator--() {
            operator-=(1);
            return *this;
        }

        iterator_template<IType> operator--(int) {
            iterator_template<IType> cp = *this;
            operator-=(1);
            return cp;
        }

        iterator_template<IType> operator+(int n) {
            iterator_template<IType> cp = *this;
            cp += n;
            return cp;
        }

        iterator_template<IType> operator-(int n) {
            iterator_template<IType> cp = *this;
            cp -= n;
            return cp;
        }

        int operator-(const iterator_template<IType>& it) const {
            int batch_dist = (batch - it.batch);
            return (batch_dist == 0 ? (pos - it.pos) : ((batch_dist - 1) 
                * BATCH_SIZE + (batch_dist < 0 ? (BATCH_SIZE 
                - pos + it.pos) :(BATCH_SIZE - it.pos + pos))));
        }

        bool operator==(const iterator_template<IType>& it) const {
            return batch == it.batch && pos == it.pos;
        }

        bool operator!=(const iterator_template<IType>& it) const {
            return !operator==(it);
        }

        bool operator<(const iterator_template<IType>& it) const {
            return batch < it.batch || (batch == it.batch && pos < it.pos); 
        }

        bool operator>(const iterator_template<IType>& it) const {
            return (it < *this);
        }

        bool operator>=(const iterator_template<IType>& it) const {
            return !operator<(it);
        }

        bool operator<=(const iterator_template<IType>& it) const {
            return !operator>(it);
        }

        IType& operator*() {
            return const_cast<Deque<T>*>(deque)->getBatch(batch)[pos];
        }

        IType* operator->() {
            return &operator*();
        }        
    };
};