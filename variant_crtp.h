#include <iostream>
#include <type_traits>

template<typename... Types>
class Variant {
private:
    using ThisVariant = Variant<Types...>;

    template<size_t I, typename T, typename... Args>
    struct IthType {
        using type = typename IthType<I - 1, Args...>::type;
    };

    template<typename T, typename... Args>
    struct IthType<0, T, Args...> {
        using type = T;
    };

    template<typename T, typename... Args>
    union VarUnion {
        T x;

        VarUnion<Args...> u;

        template<typename U>
        U& getValue() {
            if (std::is_same<T, U>::value) {
                return x;
            }
            return u.template getValue<U>();
        }

        template<typename U>
        void setValue(const U&& value) {
            if (std::is_same<T, U>::value) {
                x = std::forward<U>(value);
            }
            u.template setValue<U>(std::forward<U>(value));
            // ?? forward must be writen in other way?
        }

        template<typename U, typename... ConstructArgs>
        void constructValue(ConstructArgs&&... args) {
            if (std::is_same<T, U>::value) {
                new (x) U(std::forward<ConstructArgs>(args)...);
            }
            u.template constructValue<U>(std::forward<ConstructArgs>(args)...);
        }

        template<typename U>
        void destroyValue() {
            if (std::is_same<T, U>::value) {
                x.~U();
            }
            u.template destroyValue<U>();
        }
    };

    // change is_same to make support of inherited classes, etc.

    template<typename T, typename... Args>
    friend bool holds_alternative(const Variant<Args...> &v);

    template<typename T, typename... Args>
    friend T& get(const Variant<Args...> &v);

    template<size_t I, typename... Args>
    friend typename IthType<I, Args...>::type get(const Variant<Args...> &v);

    int classId = -1;

    VarUnion<Types...> varUnion; 

public:
    Variant() {
        varUnion.template constructValue<IthType<0, Types...>>();
    }

    template<typename T>
    Variant(T&& x) {
        varUnion.template setValue<T>(std::forward<T>(x));
    }

    Variant(const ThisVariant& v) {

    }

    Variant(ThisVariant&& v) {

    }

    template<typename T>
    ThisVariant& operator=(T&& x) {

    }

    ThisVariant& operator=(const ThisVariant& v) {

    }

    ThisVariant& operator=(ThisVariant&& v) {

    }

    template<typename T, typename... Args>
    T& emplace(Args&&... args) {
        // destroy previus value
        varUnion.template constructValue<T>(std::forward<Args>(args)...);
    }

    template<size_t I, typename... Args>
    typename IthType<I, Types...>::type& emplace(Args&&... args) {
        // varUnion.template destroyValue<IthType<classId, Types...>>(); // problem - not compile time classId
        varUnion.template constructValue<IthType<I, Types...>>(std::forward<Args>(args)...);
    }

    size_t index() {
        return classId;
    }

    bool valueless_by_exeption() {
        return classId < 0;
    }

    ~Variant() {
        // destroy last value
    }
};

template<typename T, typename... Args>
bool holds_alternative(const Variant<Args...>& v) { // CE if T is not in V

}

template<typename T, typename... Args>
T& get(Variant<Args...>& v) {

}

// add variants for const &, &&

template<size_t I, typename... Args>
typename Variant<Args...>::template IthType<I, Args...>::type& get(Variant<Args...>& v) { // get type T by I

}