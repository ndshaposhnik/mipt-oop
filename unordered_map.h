#include <iostream>
#include <type_traits>
#include <vector>
using std::cin;
using std::cout;
using std::vector;

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
    Allocator allocatorT;

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
    
    void insertNode(Node* pr, Node* val) {
        val->prev = pr;
        val->next = pr->next;
        pr->next->prev = val;
        pr->next = val;
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
        head = head->next;
        for (size_t i = 0; i < sz; i++) {
            head = head->next;
            allocator.destroy(head->prev);
            allocator.deallocate(head->prev, 1);
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
        using typePtr = std::conditional_t<isConst, const T*, T*>;
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

        typeRef operator*() const {
            return node->val;
        }

        typePtr operator->() const {
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

    List(List&& other) {
        sz = other.sz;
        head = other.head;
        allocator = std::move(other.allocator);
        other.sz = 0;
        other.head = allocator.allocate(1);
        other.head->prev = other.head->next = other.head;
    }

    List& operator = (List&& other) {
        removeList();
        sz = other.sz;
        head = other.head;
        other.sz = 0;
        other.head = allocator.allocate(1);
        other.head->prev = other.head;
        other.head->next = other.head;
        return *this;
    }

    List(const List& other) :
        List(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.get_allocator())) {
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

    template<typename... Args>
    void emplace_back(Args&&... args) {
        Node* newnode = allocator.allocate(1);
        allocatorT.construct(&newnode->val, std::forward<Args>(args)...);
        insertNode(head->prev, newnode);
    }

    void erase(const_iterator it) {
        eraseNode(it.getNode());
    }

    void clear() {
        removeList();
        *this = List();
    }

};


template<typename Key, typename Value, typename Hash = std::hash<Key>,
         typename Equal = std::equal_to<Key>,
         typename Allocator = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
public:
    using NodeType = std::pair<const Key, Value>;
    using Iterator = typename List<NodeType, Allocator>::iterator;
    using ConstIterator = typename List<NodeType, Allocator>::const_iterator;

private:
    using AllocIt = typename std::allocator_traits<Allocator>::template rebind_alloc<Iterator>;
    using AllocList = typename std::allocator_traits<Allocator>::template rebind_alloc<List<Iterator, AllocIt>>;

    Hash hashInstance = Hash();
    Equal equalInstance = Equal();

    size_t getBucketId(const Key& key) {
        return hashInstance(key) % hashTable.size();
    }

    const size_t initialSize_ = 1;

    vector<List<Iterator, AllocIt>, AllocList> hashTable;
    List<NodeType, Allocator> elements;
    float maxLoadFactor_ = 2.0;

    void rehash() {
        for (auto &i : hashTable) {
            i.clear();
        }
        while (hashTable.size() < elements.size() * maxLoadFactor_) {
            hashTable.resize(2 * hashTable.size() + 1);
            if (hashTable.empty()) {
                hashTable.resize(initialSize_);
            }
        }
        for (auto it = elements.begin(); it != elements.end(); ++it) {
            size_t hash = getBucketId(it->first);
            hashTable[hash].push_back(it);
        }
    }

    void checkRehash() {
        if (hashTable.size() < maxLoadFactor_ * elements.size()) {
            rehash();
        }
    }

public:
    UnorderedMap() {
        hashTable.resize(initialSize_);
    }

    UnorderedMap(UnorderedMap&& other) : hashTable(std::move(other.hashTable)), 
            elements(std::move(other.elements)), maxLoadFactor_(other.maxLoadFactor_) {} 

    ~UnorderedMap() = default;

    UnorderedMap& operator = (const UnorderedMap& other) {
        elements = other.elements;
        hashTable = other.hashTable;
        maxLoadFactor_ = other.maxLoadFactor_;
        return *this;
    }

    UnorderedMap& operator = (UnorderedMap&& other) {
        if (this == &other) {
            return *this;
        }
        elements = std::move(other.elements);
        hashTable = std::move(other.hashTable);
        maxLoadFactor_ = other.maxLoadFactor_;
        return *this;
    }

    UnorderedMap(const UnorderedMap& other) : elements(other.elements), maxLoadFactor_(other.maxLoadFactor_) {
        rehash();
    }

    Iterator find(const Key& key) {
        if (hashTable.empty()) {
            return elements.end();
        }
        size_t hash = getBucketId(key);
        for (auto it : hashTable[hash]) {
            if (equalInstance(key, it->first)) {
                return it;
            } 
        }
        return elements.end();
    }

    ConstIterator find(const Key& key) const {
        if (hashTable.empty()) {
            return elements.end();
        }
        size_t hash = getBucketId(key);
        for (auto it : hashTable[hash]) {
            if (equalInstance(key, it->first)) {
                return const_cast<ConstIterator>(it);
            } 
        }
        return elements.cend();
    }

    template<typename... Args>
    std::pair<Iterator, bool> emplace(Args&&... args) {
        elements.emplace_back(std::forward<Args>(args)...);
        auto newEl = elements.end();
        newEl--;
        auto itTable = find(newEl->first);
        if (itTable != elements.end()) {
            elements.pop_back();
            return {itTable, false};
        }
        size_t hash = getBucketId(newEl->first);
        hashTable[hash].push_back(newEl);
        checkRehash();
        return {newEl, true};
    }

    std::pair<Iterator, bool> insert(const NodeType& node) {
        return emplace(node);
    }

    std::pair<Iterator, bool> insert(NodeType&& node) {
        return emplace(std::forward<NodeType>(node));
    }

    template<class P>
    std::pair<Iterator, bool> insert(P&& value) {
        return emplace(std::forward<P>(value));
    }

    template<typename InputIterator>
    void insert(InputIterator fst, InputIterator snd) {
        for (auto it = fst; it != snd; it++) {
            insert(*it);
        }
    }

    Value& at(const Key& key) {
        auto it = find(key);
        if (it != elements.end()) {
            return it->second;
        }
        throw("Key not found.");
    }

    const Value& at(const Key& key) const {
        auto it = find(key);
        if (it != elements.end()) {
            return it->second;
        }
        throw("Key not found.");
    }

    Value& operator[](const Key& key) {
        auto it = find(key);
        if (it != elements.end()) {
            return it->second;
        }
        return emplace(NodeType(key, Value())).first->second;
    }

    void erase(ConstIterator it) {
        size_t hash = getBucketId(it->first);
        for (auto &x : hashTable[hash]) {
            if (equalInstance(it->first, x->first)) {
                x = *hashTable[hash].rend();
                hashTable[hash].pop_back();
                break;
            }
        }
        elements.erase(it);
    }

    void erase(ConstIterator fst, ConstIterator snd) {
        for (auto it = fst; it != snd; it++) {
            erase(it);
        }
    }

    Iterator begin() const {
        return elements.begin();
    }

    Iterator end() const {
        return elements.end();
    }

    ConstIterator cbegin() const {
        return elements.cbegin();
    }

    ConstIterator cend() const {
        return elements.cend();
    }

    size_t size() const {
        return elements.size();
    }

    void reserve(size_t sz) {
        if (hashTable.size() < sz) {
            hashTable.resize(sz);
            rehash();
        }
    }

    size_t max_size() {
        return elements.size() * maxLoadFactor_;
    }

    float load_factor() {
        if (hashTable.empty()) {
            hashTable.resize(initialSize_);
            checkRehash();
        }
        return static_cast<float>(elements.size()) / static_cast<float>(hashTable.size());
    }

    float max_load_factor() {
        return maxLoadFactor_;
    }

    void max_load_factor(float newMaxLoadFactor) {
        maxLoadFactor_ = newMaxLoadFactor;
        checkRehash();
    }
};
