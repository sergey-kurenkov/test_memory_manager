#include <cstddef>
#include <iostream>
#include <cstring>
#include <sstream>
#include <string>

/*
 *
 */
namespace my_std {

template <typename T> class linked_list;

namespace impl {

template <typename T> class linked_list_itr;

/*
 *
 */
template <typename T>
class linked_list_node {
 private:
    friend class linked_list<T>;
    friend class linked_list_itr<T>;

    linked_list_node() {
    }

    explicit linked_list_node(const T& value) : value(value) {
    }

    T value;
    using node_ptr = linked_list_node<T>*;
    node_ptr next;
    node_ptr prev;
};

/*
 *
 */
template <typename T>
class linked_list_itr {
 public:
    T& operator*() {
        return node->value;
    }

    T* operator->() {
        return &node->value;
    }

    linked_list_itr& operator++() {
        node = node->next;
        return *this;
    }

    linked_list_itr& operator--() {
        node = node->prev;
        return *this;
    }

    linked_list_itr prev() {
        linked_list_itr prev_itr(node->prev);
        return prev_itr;
    }


    bool operator!=(const linked_list_itr& b) const {
        return node != b.node;
    }

 private:
    friend class linked_list<T>;
    using node_ptr = linked_list_node<T>*;
    node_ptr node;
    explicit linked_list_itr(node_ptr node) : node(node) {
    }
};

}  // namespace impl

/*
 *
 */
template <typename T>
class linked_list {
 public:
    using iterator = impl::linked_list_itr<T>;
    using const_iterator = impl::linked_list_itr<T>;

    linked_list() : head(nullptr), tail(new node_t), list_size(0) {
        tail->next = nullptr;
        tail->prev = nullptr;
    }

    ~linked_list() {
        while (tail) {
            node_ptr tmp = tail->prev;
            delete tail;
            tail = tmp;
        }
    }

    iterator begin() {
        return head ? iterator(head) : iterator(tail);
    }

    const_iterator begin() const {
        return head ? iterator(head) : iterator(tail);
    }

    iterator end() {
        return iterator(tail);
    }

    const_iterator end() const {
        return iterator(tail);
    }

    void push_back(const T& v) {
        node_ptr a = new node_t(v);
        auto prev_node = tail->prev ? tail->prev : nullptr;
        a->next = tail;
        tail->prev = a;
        a->prev = prev_node;
        if (prev_node) {
            prev_node->next = a;
        }
        if (!head) {
            head = a;
        }
        ++list_size;
    }

    iterator erase(iterator itr) {
        node_ptr before_node = itr.node->prev;
        node_ptr next_node = itr.node->next;
        if (before_node) {
            before_node->next = next_node;
        } else {
            if (next_node != tail)
                head = next_node;
            else
                head = nullptr;
        }
        next_node->prev = before_node;
        delete itr.node;
        --list_size;
        return iterator(next_node);
    }

    size_t size() const {
        return list_size;
    }

 private:
    using node_t = impl::linked_list_node<T>;
    using node_ptr = node_t*;
    node_ptr head;
    node_ptr tail;
    size_t list_size;

    linked_list(const linked_list&) = delete;
    linked_list& operator=(const linked_list&) = delete;
};

}  // namespace my_std

/*
 *
 */
template <typename T>
class MemoryManager {
 public:
    using unit_t = T;

    MemoryManager(unit_t* mem_block, std::size_t mem_size);
    void print() const;
    MemoryManager& defragment();

 private:
    struct mem_node_t {
        unit_t *start;
        size_t len;
    };
    unit_t *mem_start;  // not owner
    unit_t *mem_end;  // not owner
    size_t mem_size;
    my_std::linked_list<mem_node_t> free_list;

    void build_free_list();

    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;
};

/*
 *
 */
template <typename T>
MemoryManager<T>::MemoryManager(unit_t* mem_block, std::size_t mem_size)
: mem_start(mem_block), mem_end(mem_block+mem_size), mem_size(mem_size) {
    build_free_list();
}

/*
 *
 */
template <typename T>
void
MemoryManager<T>::build_free_list() {
    unit_t* free_ptr = nullptr;
    auto free_ptr_size = 0U;
    for (auto begin = mem_start, end = mem_end;
            begin != end; ++begin) {
        if (!*begin) {
            if (!free_ptr) {
                free_ptr = begin;
                free_ptr_size = 1;
            } else {
                ++free_ptr_size;
            }
        } else {
            if (free_ptr) {
                free_list.push_back({free_ptr, free_ptr_size});
                free_ptr = nullptr;
                free_ptr_size = 0U;
            }
        }
    }
    if (free_ptr) {
        free_list.push_back({free_ptr, free_ptr_size});
    }
}

/*
 *
 */
template <typename T>
void
MemoryManager<T>::print() const {
    std::cout << "Free block length: ";
    auto first_itr = true;
    auto print_comma = [&first_itr] () {
        if (first_itr)
            first_itr = false;
        else
            std::cout << ", ";
    };
    if (free_list.size() > 0) {
        for (auto i = free_list.begin(), end = free_list.end(); i != end; ++i) {
            print_comma();
            std::cout << i->len;
        }
    } else {
        std::cout << 0;
    }
    std::cout << " | ";

    std::cout << "Occupied block contents: ";
    unit_t* occupied_ptr = mem_start;
    auto free_itr = free_list.begin();
    first_itr = true;
    auto print_occupied_substr = [&occupied_ptr, &print_comma](unit_t* end) {
        print_comma();
        while (occupied_ptr < end) {
            std::cout << static_cast<char>(*occupied_ptr++);
        }
    };
    while (occupied_ptr < mem_end) {
        if (free_itr != free_list.end()) {
            if (occupied_ptr < free_itr->start) {
                print_occupied_substr(free_itr->start);
            } else {
                occupied_ptr = free_itr->start + free_itr->len;
                ++free_itr;
            }
        } else {
            print_occupied_substr(mem_end);
        }
    }
    std::cout << std::endl;
}

/*
 *
 */
template <typename T>
MemoryManager<T>&
MemoryManager<T>::defragment() {
    while (free_list.size() > 1) {
        auto last_free_block = --free_list.end();
        auto before_last_free_block = last_free_block.prev();

        std::size_t last_free_block_len = last_free_block->len;
        unit_t* old_occupied_start = before_last_free_block->start +
                before_last_free_block->len;
        std::ptrdiff_t occupied_block_len = last_free_block->start -
                (before_last_free_block->start +  before_last_free_block->len);
        unit_t* new_occupied_start = old_occupied_start+last_free_block_len;

        std::memmove(new_occupied_start, old_occupied_start,
                occupied_block_len * sizeof(unit_t));

        before_last_free_block->len += last_free_block_len;
        std::memset(old_occupied_start, unit_t(0),
                last_free_block_len * sizeof(unit_t));

        free_list.erase(last_free_block);
    }
    if (free_list.size() == 1 && mem_start != free_list.begin()->start) {
        auto free_block = free_list.begin();
        std::size_t free_block_len = free_block->len;
        unit_t* old_occupied_start = mem_start;
        std::ptrdiff_t occupied_block_len =
                free_block->start - old_occupied_start;
        unit_t* new_occupied_start = old_occupied_start+free_block_len;
        std::memmove(new_occupied_start, old_occupied_start,
                occupied_block_len * sizeof(unit_t));
        std::memset(old_occupied_start, unit_t(0),
                free_block_len * sizeof(unit_t));
        free_block->start = old_occupied_start;
    }
    return *this;
}


static void run_unit_tests();

/*
 *
 */
int main() {
    run_unit_tests();

    int array[] = {
         0, 0, 0,                   // free block of 3
         'C', 'O', 'N', 'T', 'I',   // occupied block of 5
         0, 0, 0, 0, 0,             // free block of 5
         'G', 'U', 'O', 'U',        // occupied block of 4
         0, 0,                      // free block of 2
         'S', '!' };                // occupied block of 2

    MemoryManager<int> mm(array, 21);
    mm.print();
    mm.defragment().print();
    return 0;
}

#define TEST_STR(name, function_call, output_c_str) \
{ \
    std::string output(output_c_str); \
    std::streambuf *cout_rdbuf = std::cout.rdbuf(); \
    std::stringstream test_buffer; \
    std::cout.rdbuf(test_buffer.rdbuf()); \
    function_call; \
    std::string test_str = test_buffer.str(); \
    std::cout.rdbuf(cout_rdbuf); \
    if (test_buffer.str() != output) { \
        std::cout << "ERROR, Unit test [" << name << \
            "], expected: \"" << output << \
            "\", got: \"" << test_buffer.str() << "\"" << std::endl; \
    } else { \
        if (!output.empty() && *(--output.end()) == '\n') { \
            output.erase(--output.end()); \
            test_str.erase(--test_str.end()); \
        } \
        std::cout << "OK, Unit test [" << name << \
            "], expected: \"" << output << \
            "\", got: \"" << test_str << "\"" << std::endl; \
    } \
}

#include <type_traits>

/*
 *
 */
void run_unit_tests() {
    {
        int array[] = {};
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, no elements", mm.print(),
                "Free block length: 0 | Occupied block contents: \n");
    }
    {
        int array[] = { 'A' };
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, one element", mm.print(),
                "Free block length: 0 | Occupied block contents: A\n");
    }
    {
        int array[] = { 0 };
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, one free element", mm.print(),
                "Free block length: 1 | Occupied block contents: \n");
    }
    {
        int array[] = { 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A'};
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, ten elements", mm.print(),
                "Free block length: 0 | Occupied block contents: AAAAAAAAAA\n");
    }
    {
        int array[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, ten free element", mm.print(),
                "Free block length: 10 | Occupied block contents: \n");
    }
    {
        int array[] = { 0 };
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, no elements", mm.print(),
                "Free block length: 1 | Occupied block contents: \n");
    }
    {
        unsigned char array[] = {};
        MemoryManager<unsigned char> mm(array,
                std::extent<decltype(array)>::value);
        TEST_STR("unsigned char, no elements", mm.print(),
                "Free block length: 0 | Occupied block contents: \n");
    }
    {
        unsigned char array[] = { 'Z', 'W' };
        MemoryManager<unsigned char> mm(array,
                std::extent<decltype(array)>::value);
        TEST_STR("unsigned char, two elements", mm.print(),
                "Free block length: 0 | Occupied block contents: ZW\n");
    }
    {
        unsigned char array[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        MemoryManager<unsigned char> mm(array,
                std::extent<decltype(array)>::value);
        TEST_STR("unsigned char, ten free element", mm.print(),
                "Free block length: 10 | Occupied block contents: \n");
    }
    {
        int array[] = { 'A', 'A', 0, 0 };
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, two elements, two free", mm.print(),
                "Free block length: 2 | Occupied block contents: AA\n");
    }
    {
        char array[] = { 0, 0 , 'A', 'A'};
        MemoryManager<char> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("char, two free, two elements", mm.print(),
                "Free block length: 2 | Occupied block contents: AA\n");
    }
    {
        int array[] = { 'A', 'A', 0, 0, 'B', 'B' };
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, two elements, two free, two elements", mm.print(),
                "Free block length: 2 | Occupied block contents: AA, BB\n");
    }
    {
        char array[] = { 0, 0, 0 , 'A', 'A', 0, 0};
        MemoryManager<char> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("char, three free, two elements, two free", mm.print(),
                "Free block length: 3, 2 | Occupied block contents: AA\n");
    }
    {
        int array[] = { 'A', 'A', 0, 0, 'B', 'B', 'B', 0, 0, 0, 'C', 'C', \
                'C', 'C'};
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, complex test 1", mm.print(),
                "Free block length: 2, 3 | Occupied block contents: "
                "AA, BBB, CCCC\n");
    }

    {
        long long array[] = { 0 };
        MemoryManager<long long> mm(array,
                std::extent<decltype(array)>::value);
        TEST_STR("long long, one free element", mm.print(),
                "Free block length: 1 | Occupied block contents: \n");
    }
    {
        long long array[] = { 0, 'A', 0, 'B' };
        MemoryManager<long long> mm(array,
                std::extent<decltype(array)>::value);
        TEST_STR("long long, complex test 2", mm.print(),
                "Free block length: 1, 1 | Occupied block contents: A, B\n");
    }
    {
        int array[] = {};
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, defragment, no elements", mm.defragment().print(),
                "Free block length: 0 | Occupied block contents: \n");
    }
    {
        int array[] = { 'A' };
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, defragment, one element", mm.defragment().print(),
                "Free block length: 0 | Occupied block contents: A\n");
    }
    {
        short array[] = { 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A'};
        MemoryManager<short> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, defragment, ten elements", mm.defragment().print(),
                "Free block length: 0 | Occupied block contents: AAAAAAAAAA\n");
    }
    {
        short array[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        MemoryManager<short> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, defragment, ten free element", mm.defragment().print(),
                "Free block length: 10 | Occupied block contents: \n");
    }
    {
        int array[] = { 'A', 'A', 0, 0 };
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, defragment, two elements, two free",
                mm.defragment().print(),
                "Free block length: 2 | Occupied block contents: AA\n");
    }
    {
        int array[] = { 'A', 'A', 0, 0, 'B', 'B' };
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, defragment, four elements, two free",
                mm.defragment().print(),
                "Free block length: 2 | Occupied block contents: AABB\n");
    }
    {
        char array[] = { 0, 0 , 'A', 'A'};
        MemoryManager<char> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("char, defragment, two free, two elements",
                mm.defragment().print(),
                "Free block length: 2 | Occupied block contents: AA\n");
    }
    {
        char array[] = { 0, 0, 0 , 'A', 'A', 0, 0};
        MemoryManager<char> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("char, defragment, five free, two elements, two free",
                mm.defragment().print(),
                "Free block length: 5 | Occupied block contents: AA\n");
    }
    {
        int array[] =
            { 'A', 'A', 0, 0, 'B', 'B', 'B', 0, 0, 0, 'C', 'C', 'C', 'C'};
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, defragment, complex test 3",
                mm.defragment().print(),
                "Free block length: 5 | Occupied block contents: AABBBCCCC\n");
    }
    {
        int array[] = { 0, 0, 0, 0, 'A', 'A', 0, 0, 'B', 'B', 'B',
                0, 0, 0, 'C', 'C', 'C', 'C'};
        MemoryManager<int> mm(array, std::extent<decltype(array)>::value);
        TEST_STR("int, defragment, complex test 4",
                mm.defragment().print(),
                "Free block length: 9 | Occupied block contents: AABBBCCCC\n");
    }
}
