#include <iostream>
#include <list>
#include <type_traits>
#include <vector>
using std::cin;
using std::cout;
using std::vector;

template <size_t chunkSize>
class FixedAllocator {
private:
    std::vector<int8_t*> blocks;
    std::vector<int8_t*> vacant;
    static const size_t STATIC_SIZE = 1024;

    size_t cnt = 0;
    
    void newBlock() {
        int8_t* last = new int8_t[STATIC_SIZE];
        blocks.push_back(last);
        cnt = 0;
    }

    FixedAllocator() {
        newBlock();
    }

public:
    static FixedAllocator& getAllocator() {
        static FixedAllocator instance;
        return instance;
    }

    void* allocate() {
        if (cnt + chunkSize > STATIC_SIZE && vacant.empty()) {
            newBlock();
        }
        int8_t* ans;
        if (!vacant.empty()) {
            ans = vacant.back();
            vacant.pop_back();
        } else {
            ans = blocks.back() + cnt;
            cnt += chunkSize;
        }
        return static_cast<void*>(ans); 
    }

    void deallocate(void* ptr) {
        vacant.push_back(static_cast<int8_t*>(ptr));    
    }

    ~FixedAllocator() {
        for (auto b : blocks) {
            ::operator delete(b);   
        }    
    }
};

template<typename T>
class FastAllocator {
public:
    template<typename U>
    struct rebind {
        typedef FastAllocator<U> other;
    };
    using value_type = T;

    FastAllocator<T>() = default;
    FastAllocator<T>(const FastAllocator<T>&) = default;

    template<typename U>
    FastAllocator(const FastAllocator<U>&) {}

    T* allocate(size_t size) {
        if (size == 1) {
            return static_cast<T*>(FixedAllocator<sizeof(T)>::getAllocator().allocate());
        }
        return std::allocator<T>().allocate(size);
    }

    void deallocate(T* ptr, size_t size) {
        if (size == 1) {
            FixedAllocator<sizeof(T)>::getAllocator().deallocate(static_cast<void*>(ptr));
        } else {
            std::allocator<T>().deallocate(ptr, size);
        }
    }

    template<typename... Args>
    void construct(T* ptr, Args&... args) {
        new (ptr) T(args...);
    }

    void destroy(T* ptr) {
        ptr->~T();
    }

};

template<typename T, typename U>
bool operator == (const FastAllocator<T>& a, const FastAllocator<U>& b) {
    return &a == &b;
}

template<typename T, typename U>
bool operator !=(const FastAllocator<T>& a, const FastAllocator<U>& b) {
    return !(a == b);
}

//template <typename T, typename Allocator = std::allocator<T>>
//using List = std::list<T, Allocator>;

template <typename Allocator, typename T>
using RebindAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;


template<typename T, typename Allocator = std::allocator<T>>
class List {
private:
    struct Node {
        T val;
        Node* prev = nullptr;
        Node* next = nullptr;
        
        Node() = default;
        Node(Node* prev = nullptr, Node* next = nullptr) : prev(prev), next(next) {}
        Node(const T& v, Node* prev = nullptr, Node* next = nullptr)
                : val(v), prev(prev), next(next) {}
    };

    RebindAlloc<Allocator, Node> allocator;
    using traits = typename std::allocator_traits<Allocator>;

    size_t sz = 0;

    Node* head = nullptr;

    void eraseNode(Node* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        allocator.destroy(node);
        allocator.deallocate(node, 1);
        sz--;
    }

    void insertNode(Node* pr, const T& val) {
        Node* newnode = allocator.allocate(1);
        allocator.construct(newnode, val, pr, pr->next);
        pr->next->prev = newnode;
        pr->next = newnode;
        sz++;
    }
    
    void createList(size_t n) {
        sz = n;
        head = allocator.allocate(1);
        Node* last = head;
        for (size_t i = 0; i < n; i++) {
            Node* newnode = allocator.allocate(1);
            newnode->prev = last;
            last->next = newnode;
            last = newnode;
        }
        head->prev = last;
        last->next = head;
    }

    void removeList() {
        if (head == nullptr) {
            return;
        }
        for (size_t i = 0; i < sz; i++) {
            Node* xx = head;
            head = head->next;
            allocator.destroy(xx);
            allocator.deallocate(xx, 1);
        }
        allocator.deallocate(head, 1);
        head = nullptr;
        sz = 0;
    }

    void copy(const List<T, Allocator>& other) {
        removeList();
        if (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) {
            allocator = other.get_allocator();
        }

        createList(other.size());
        if (sz == 0) {
            return;
        }
        Node* f = head->next;
        Node* s = other.head->next;
        for (size_t i = 0; i < sz; i++) {
            allocator.construct(f, s->val, f->prev, f->next);
            f = f->next;
            s = s->next;
        }
    }

    template<bool isConst, bool isRev>
    class allIterators {
    private:
        Node* node;

        using type = std::conditional_t<isConst, const T, T>;
        using typePtr = std::conditional_t<isConst, const Node*, Node*>;
        using typeRef = std::conditional_t<isConst, const T&, T&>;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = std::ptrdiff_t;

        allIterators(Node* ptr) : node(ptr) {}

        allIterators(const allIterators& other) {
            node = other.node;
        }
        
        template<bool isRev1>
        allIterators(const allIterators<isConst, isRev1>& other) {
            node = other.getNode();
        }

        bool operator == (const allIterators<isConst, isRev>& s) const {
            return node == s.node;
        }

        bool operator != (const allIterators<isConst, isRev>& s) const {
            return !(*this == s);
        }

        allIterators<isConst, isRev> operator=(const allIterators<isConst, isRev>& other) {
            node = other.node;
            return *this;
        }

        Node* getNode() const {
            return node;
        }

        allIterators<isConst, isRev> operator++() {
            if (!isRev) {
                node = node->next;
            } else {
                node = node->prev;
            }
            return *this;
        }

        allIterators<isConst, isRev> operator++(int) {
            auto res = *this;
            ++(*this);
            return res;
        }

        allIterators<isConst, isRev> operator--() {
            if (!isRev) {
                node = node->prev;
            } else {
                node = node->next;
            }
            return *this;
        }

        allIterators<isConst, isRev> operator--(int) {
            auto res = *this;
            --(*this);
            return res;
        }

        typeRef operator*() {
            return node->val;
        }

        typePtr operator->() {
            return &(node->val);
        }

        allIterators<isConst, false> base() {
            return allIterators<isConst, false>(node->next);
        }
        
        template<bool isRev1>
        operator allIterators<true, isRev1>() {
            return allIterators<true, isRev1>(node);
        }

    };

public:
    using iterator = allIterators<false, false>;
    using const_iterator = allIterators<true, false>;
    using reverse_iterator = allIterators<false, true>;
    using const_reverse_iterator = allIterators<false, false>;

    size_t size() const {
        return sz;
    }
    
    explicit List(const Allocator& alloc = Allocator()) : allocator(alloc) {
        sz = 0;
        head = allocator.allocate(1);
        head->prev= head->next = head;
    }

    List(size_t count, const T& value, const Allocator& alloc = Allocator()) : List(alloc) {
        createList(count);
        Node* last = head->next;
        for (size_t i = 0; i < sz; i++) {
            this->allocator.construct(last, value, last->prev, last->next);
            last = last->next;
        }
    }

    List(size_t count, const Allocator& alloc = Allocator()) : List(alloc) {
        createList(count);
        Node* last = head->next;
        for (size_t i = 0; i < count; i++) {
            allocator.construct(last, last->prev, last->next);
            last = last->next;
        }
    }

/*    List(const List<T, Allocator>& other) : List(other.get_allocator()) {
        for (auto it = other.begin(); it != other.end(); ++it) {
            push_back(*it);
        }
    }
*/
    List(const List& other) :
        List(traits::select_on_container_copy_construction(other.get_allocator())) {
        for (auto it = other.begin(); it != other.end(); ++it) {
            push_back(*it);
        }
    }

     List& operator = (const List& other) {
        removeList();
        copy(other);
        return *this;
    }

    ~List() {
        removeList();
    }

    Allocator get_allocator() const {
        return allocator;
    }

    iterator begin() const {
        return head->next;
    }
    const_iterator cbegin() const {
        return head->next;
    }
    reverse_iterator rbegin() const {
        return head->prev;
    }
    const_reverse_iterator crbegin() const {
        return head->prev;
    }

    iterator end() const {
        return head;
    }
    const_iterator cend() const {
        return head->prev;
    }
    reverse_iterator rend() const {
        return head;
    }
    const_reverse_iterator crend() const {
        return head->next;
    }

    void push_back(const T& val) {
        insertNode(head->prev, val);
    }

    void push_front(const T& val) {
        insertNode(head, val);
    }

    void pop_back() {
        eraseNode(head->prev);
    }

    void pop_front() {
        eraseNode(head->next);
    }

    void insert(const_iterator it, const T& v) {
        insertNode(it.getNode()->prev, v);
    }

    void erase(const_iterator it) {
        eraseNode(it.getNode());
    }

    void print() {
        Node* now = head->next;
        for (size_t i = 0; i < sz; i++) {
            std::cout << now->val << ' ';
            now = now->next;
        }
        cout << std::endl;
    }
};

