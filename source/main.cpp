// 15.1 Introduction
// objects and arrays hold data
// pointers refer to data

// main comainallity with container and pointer abstractions: correct and efficient use requires encapsulation of data and
// a set of functions to access and manipulate the data

// pointers are a general and efficient abstraction of machine addresses, but using them correctly to represent ownership
// of resources is very error prone

// std provides resource pts, which encapsulate pointers to encapsulate those langauge types that don't have overhead

// 15.2 Pointers
//Pointer: allows us to refer to an object and access it according to its type

// more than one pointer can refer to an object
// owning pointer: in charge of deleting the pointer it refers to. 
// a non-owning pointer can dangle - pointer to a value that's been deleted or gone out of scope. 

// reading or writing through a dangling ptr is undefined.

// guidelines for avoiding problems with ptrs
// don't retain a ptr to an object that's gone out of scope
// never return a ptr to a local object from a function. 
// never store a ptr of uncertain provenance in a long-lived data structure
// use owning pointers to allocate objects on the free store
// pointers to static objects(like global variables) can't dangle
// leave ptr arithmetic to the implementation of resouce handles
// string_views and spans are kinds of non-owning pointers


//15.2.1 unique_ptr and shared_ptr
//resource: something that must be acquired and then released (explicitly or implicitly)
// examples: memory, locks, sockets, thread handles, file handles

// for a long running program, failing to release a resouce in a timely manner (a leak) can cause performance problems and crashes

// even for short programs, a leak can increase the run time

// std lib components don't leak resources by using constructor/destructor pairs to prevent the resource from outliving the object responsible for it
// this approachs interacts correctly with error handling using exceptions

// this technique is used for std lock classes

mutex m; // used to protect access to shared data

void f()
{
    scoped_lock lck {m}; // acquire the mutex ,
    // .. manipulate shared data ...
}

// thread won't proceed until lck's construct has acquired the mutex
// the destructor releases the mutex

// scoped_lock''s destructor releases the mutex when the thread of control leaves f()

// this is an example of RAII
// RAII is fundamental to handling of resources in C++

// The memory library provides two objects to help manage objects on the free store

// unique_ptr represents unique ownership (it's destructor destroys the object)
// shared_ptr represents shared ownership (that last shared ptr's destructor destroys the object)

// the most basic use of smart ptrs is to prevent memory leaks caused by careless programming

// example
void f(int i, int j) // X* vs. unique_ptr<X>
{
    X* p = new X; // allocate a new X
    unique_ptr<X> sp {new X}; // allocate a new X and givs its pointer to unique_ptr
    // ..

    if (i<99) throw Z{}; // may throw an exception
    if (j<77) return; // may return "early"
    // ... use p and sp ..
    delete p; // destrop *p
}

// the above example doesn't destroy p is i <99 or j<77
// unique_ptr will destroy X no matter how we exit f()

// we can also solve this problem by not using a ptr or new

void f(int i, int j)
{
    X x;
    // ...
}

// don't overuse new and ptrs

// unique_ptr is lightweight and has no overhead in terms of space or time
// unique_ptr can pass free store allocated objects in and out of functions

unique_ptr<X> make_X(int i)
// make an X and immediatly gve it to a unique_ptr
{
    // .. check i, etc ...
    return unique_ptr<X>(new X{i});
}

// unqie_ptr is a handle to an individual object or array in much the same way that vector is a handle to a sequence of objects

// both control the lifetime of other objects (with RAII) and both rely on elimination of copying or on move semantics to make return simple and efficient

// shared_ptr is copied with unique_ptr is moved

// shared_ptrs for an object share ownership. 
// the object is destroyed when its last shared ptr are destroyed

void f(shared_ptr<fstream>);
void g(shared_ptr<fstream>);

void user(const string& name, ios_base::openmode mode)
{
    shared_ptr<fstream> fp {new fstream(name,mode)};
    if (!*fp) // make sure the file was properly opened
        throw No_file{};
    
    f(fp);
    g(fp);
    // ..
}

// the last function to explicitly or implicitly destroy a copy of fp will close the file opened by fp's constructor
// shared_ptr provides a form of garbage collection tha respects the destructor-based resource management of the memory-managed objects
// this makes the lifetime of the shared object hard to predict

// shared_ptr provides a form of garbage collection that respects the destructor-based resource management of the memory-manged objects
// this isn't really expensive, but it makes the lifetime of a shared object tough to predicts
// think if you actually need shared owneship before calling on shared_ptr

// creating an object on the free store, assinging it to a ptr, and then passing the ptr to a smart ptr is verbose
// it allows for mistakes, like forgetting to pass the ptr to a unique_ptr or giving the ptr to something that isn't on the free store to shared_ptr

// to avoid these problems, the std provides functions for constructing an object and then returning an appropriate smart ptr
// these are make_shared and make_unique functions

struct S {
    int i;
    string s;
    double d;
    // ..
};

auto p1 = make_shared<S>(1,"Ankh Morpork", 4.65); // p1 is a shared_ptr<S>
auto p2 = make_unique<S>(2, "Oz",7.62); // p2 is a unique_ptr<S>

// p2 is a unique_tr<S> pointing to a free-store allocated object of type S with the value {2, "Oz",7.62}.

// using make_shared() is not just more convenient than seperatly making an object using new and then passing
// it to shared_ptr - it is also notable more efficient because it does not need a seperate allocation for the 
// use count that is essential in the implementation of shared_ptr

// with unique_ptr and shared_ptr, we can implement a "no naked new" policy for many programs

// but, favor containers that manager their own resources of unique_ptr and shared_ptr
// they're still conceptually ptrs

// shared_prs don't provide rules to control access to the shared object
// data races and other forms of confusions still exists when using shared_ptrs

// use unique_ptr rather than resource handles when we need ptr semantics

// when we shared an object, we need ptrs or references to refer to the object. shared_ptr is the obvious choice here
// when we refer to a polymorpgic object in classical object-oriented code, we need a pointer or ref because know the exact type of the object referred to or its size. use unique_ptr
// shared polymorphic objects usually require shared_ptrs

// we don't need to use a pointer to return a collection of objects from a function. 
// a container will do that simply and efficiently with copy elison and move semantics

// 15.2.2 span
// range errors cause a lot of problems in C and C++
// containers, algos, and range-for can reduce these problems
// big soure of range errors: passing ptrs (raw or smart) and then relying on convention to know the number of elements pointed to

// best advice for code outside resource handles is to assume that at most one object is pointed to but without support that advice is unmanageable
// string_view can help, but it's read-only and just for characters

// when writing into and reading out of buggers in lower-level software, it's difficult to maintain high performance
// while still avoiding range errors ("buffer overruns")

// a span is a (pointer,length) pair denoting a sequence of elements

// span gives access to contiguous sequences of elements

// these elements can be stored in many ways, including vectors and built in arrays

// like ptrs, span's don't own the characters they point to

// it is thus similar to string_view

// a common interface style
void fpn(int* p, int n)
{
    for (int i = 0; i<n; ++i)
        p[i] = 0;
}

// we can assume that *p points to n integers
// this assumption is just a convetion, so can't use it to write a range-for loop and the compiler can't implement cheap and efficient range checking
// this assumption can also be wrong

void use(int x)
{
    int a[100]; // Ok
    fpn (a,1000); // woops, my finger slipped
    fpn (a+10,100); // range error in fpn;
    fpn(a,x); // suspect, but looks innocent
}

// span can prevent these kinds of errors
void fs(span<int> p)
{
    for (int& x : p)
        x = 0;
}

void use(int x)
{
    int a[100];
    fs(a); // implicitly creates a span<int>{a,100}
    fs(a,1000); // error: span expected
    fs({a+10,100}); // a range error in fs
    fs({a,x}); // obviously suspect
}

// the common case of creating a span from an array is now safe and notationally simple
// explicitly composiing a span makes error-detection easier. 

// common case where a span is passed along from function to function is simpler than (poiner,count) interfaces and doesn't require extra checking

void f1(span<int> p);

void fs(span<int> p)
{
    // ..
    f1(p);
}

// when span is used for subscripting, range checking isn't done and out-of-range access is undefined behaivor
// an implementation to implement range checking when subscripting, but few do
// the original gsl::span from the Core Guidelines support lib does range checking

//15.3 Containers
// std provides contaienrs that don't fit perfectly in the STL framework
// examples: built in arrays, array, string

// 15.3.1 array
// array is a fixed-size sequence of elements where the number of elements is specified at compile time

// an array can thus be allocated with its elements on the stack, in an object, or in static storage
// these elements are allocated in the scope where array is defined

// array is first understood as a built-in array with its size firmly attached, without implicit conversions to ptr types, with few qol functions

// there is no space or time overhead when using an array compared to using a built-in array

// array doesn't follow the "handle to elements" model of Stl containers

// array directly contains its elements

// it's just a safer version of a built-in array

// this implies that an array can and must be initialized by an initializer list

std::array<int,3> a1 = {1,2,3};
// number of elements in the initializer must be equal to or less than the number of elements specified for the array

// the element count is required and we need to state the type of the elements

void f(int n)
{
    std::array<int> a0 = {1,2,3}; // error size not specified
    std::array<string,n> a1 = {"John's", "Queens' "}; // error: size not a constant expression
    std::array<string,0> a2; // error: size must be positive
    std::array<2> a3 = {"John's", "Queens' "}; // error: element type not stated
}

// when you need the element count to be variable, use vector. std::array has a fixed element count

// std::array can be passed to a C-style function that expects a pointer

void f(int * p, int sz); // C-style inferface

void g()
{
    std::array<int, 10> a;

    f(a, a.size()); // error: no conversion
    f(a.data(), a.size()); // C-style use

    auto p = find(a, 777); //C++/STL-style use (a range is passed)
}

// why use array instead of vector
// array is less flexible than vector, and thus simpler
// sometimes, there is an advantage to accessing elements on the stack, instead of allocating them on the free store, accessing them indirectly through a vector, and then deallocating them

// disadvantage: stack is limited, especially on embedded systems, and stack overflow is nasty

// some application areas, like safety-critical real-time constrol, ban free-store allocation
// use of delete and can lead to fragmentation or memory exhaustion

// why use std:: array instead of built in array?
// std::array knows its size and can be copied with =
std::array<int,3> a1 = {1,2,3};

auto a2 = a1; // copy

a2[1] = 5;
a1 = a2; // assign

// std::arrays prevents nasty and unexpected conversions to ptrs

// example involving a class hiearchy
void h()
{
    Circle a1[10];
    std::array<Circle,10> s2;
    // ..
    Shape* p1 = a1; // Ok: disaster waiting to happen
    Shape* p2 = a2; // error:no conversion of array<Circle,10> to Shape*(Good!)
    p1[3].draw(); // disaster
}

// disaster comment assumes that sizeof(Shape)<sizeof(Circle), so subscripting Circle[] through a Shape* gives a wrong offset
// all standard containers provide this advantage over built in arrays

// 15.3.2 bitset

// aspects of the system, like state of the input stream, are often represented as a set of flags indicating binary conditions
// such as good/bad, true/false and on/off

// C++ supports the notion of small sets of flags efficiently through bitwise operations on integers

// bitset<N> generalizes this notion by providing operations on a sequence of N bits, where N is known at compile time. 
// for sets of bits that don't fit into a long long int (64 bits), bitset is much more convienent that using integers directly

// for smaller sets, bitset is usually optimized

// sets or enumerations let you name the bits

// a bitset can be initialized with an int or string

bitset<9> bs1 {"110001111"};
bitset<9> bs2 {0b1'1000'1111}; // binary literal using digit separators 

// bitwise operators left and right shift can be applied

bitset<9> bs3 = ~bs1; // complement: bs3=="001110000"
bitset<9> bs4 = bs1&bs3; // all zeros
bitset<9> bs5 = bs1<<2; // shift left: bs5 = "000111100"

// shift operatos shift in zeros

// operations to_ullong and to_string() provide the inverse operations to the constructors
// for example, we can write out the binary representation of an int

void binary(int i)
{
    bitset<8*sizeof(int)> b = i; // assume 8-bit byte 
    cout << b.to_string() << '\n'; // write out the bits of i
}

void binary2(int i)
{
    bitset<8>sizeof(int)> b = i; // assume 8-bit byte
    cout << b << '\n'; // write out the bits of i
}

// bitset offers functions for using and manipulating sets of bits

// 15.3.3 pair

// it's common for a function to return two values
// simplest and often best way is to define a struct for the purpose
// for example, we can return a value and a success indicator

struct My_res {
    Entry* ptr;
    Error_code err;
}

My_res complex_search(vector<Entry>& v, const string& s)
{
    Entry* found = nullptr;
    Error_code err = Error_code::found;
    // .. search for s in v ..
    return {found,err};
}

void user(const string& s)
{
    My_res r = complex_search(entry_table,s); // search entry_table
    if (r.err != Error_code::good)
    {
        // ... handle error ...
    }
    // use ptr
}

// encoding pair was an end indicator or nullptr might be more elegant, but that can only express one kind of failure

// often, we want to return two seperate values
// defining a named struct for each pair of values often works well and is readable if the names of the pair of values structs and their members are well chosen
// std:: pair provides support for pair of value use cases

std::pair<Entry*, Error_code> complex_search(std::vector<Entry>& v, const std::string& s)
{
    Entry* found = nullptr;
    Error_code err = Error_code::found;
    // ... search for s in v ...
    return {found, err};
}

void user(const string& s)
{
    auto r = complex_search(entry_table,s); // search entry_table
    if (r.second != Error_code::good) {
        // .. handle error...
    }

    // use r.first
}

// members of pair are named first and second
// that makes sense from an implementers point of view, but in application code we may want to use our own names
// structured binding can be used to deal with that

void user(const string& s)
{
    auto [ptr,success] complex_search(entry_table,s); // search entry_table
    if (success != Error_code::good)
    {
        // ... handle error
    }
    // ... use ptr
}

// pair is used for pair of value cases in std library
//example
template<typename Forward_iterator, typename T, typename Compare>
    pair<Forward_iterator,Forward_iterator>
    equal_range(Forward_iterator first, Forward_iterator last, const T& val, Compare cmp);

// given a sorted sequence [first:last), equal_range() will return the pair representing the subsequence that matches the predicate cmp
// we can use that to search in a sorted sequence of Records

auto less = [](const Record& r1, const Record& r2) {return r1.name<r2.name;}; // compare names

void f(const vector<Record>& v) // assume that v is sorted on its "name" field
{
    auto [first,last] = equal_range(v.begin().v.end(),Record{"Reg"},less);

    for (auto p = first; p!=last; ++p) // print all equal records
        cout << *p; // assume that << is defined for Record
}

// a pair provides operators, such as =,==, and <m if its elements do. Type deducion makes it easy to 
// create a pair without explicitly mentioning its type

void f(vector<string>& v)
{
    pair p1{v.begin(),2}; // one way
    auto p2 = make_pair(v.begin(),2); // another way
    // ..
}

// both p1 and p2 are o type pair<vector<string>::iterator,int>
// when the code doesn't have to be generic, a simple struct with named members often leads to more maintainable code

// 15.3.4 Tuple
// std lib containers are homogeneous
// sometimes we want to treat a sequence of elements of different types as a single object: a heterogenous container
// tuple is a generalization of pair with zero or more elements

tuple t0{}; // empty
tuple<string,int,double> t1 {"Shark,123,3.14"}; // the type is explicitly specified
auto t2 = make_tuple(string{"Herring"},10,1.23); // the type is deduced to tuple<string,int,double>
tuple t3 {"Cod"s,20,9.99}; // the type is deduced to tuple<string,int,double>

// the elements (members) of a tuple are independent. 
// there is no invarient maintained amoung them
// if we can an invarient, we have to encapsulate the tuple in a class that enforces it

// for a single, specific use, a simple struct is often ideal. 
// there are many uses many generic uses where the flexibility of typle saves us from having to define a lot of structs
// but it doesn't have mnemonic names for the members
// members of tuple are accessed through get function template

string fish = get<0>(t1); // get the first element: "Shark"
int count = get<1>(t1); // get the second element: 123
double price = get<2>(t1); // get the third element: 3.14

// elements of a tuple are numbered and the index arg to get() must be constant
// function get() is a template function taking the index as a template value argument

// accessing members of a tuple by their index is general, ugle, and error-prone
// if the types are unique, you get them by type

auto fish = get<string>(t1); // get the string: "Shark"
auto count = get<int>(t1); // get the int: 123
auto price = get<double>(t1); // get the double: 3.14

// get can also be used to write into the tuple

get<string>(t1) = "Tuna"; // write to the string
get<int>(t1) = 7; // write to the int
get<double>(t1) = 312; // write to the double

// most uses of tuple are hidden in uses of higher-level constructs

// accessing members of t1 with structured binding
auto [fish, count, price] = t1;
cout << fish << '' << count << '' << price << '\n'; // read
fish = "Sea Bass"; // write

// such binding and its underlying use of type is often used for a function call

auto[fish, count, price] = todays_catch();
cout << fish << ' ' << count << ' ' << price << '\n';

// real strength of tuple is when you have to store or pass an unknown number of elements
// or an unknown type as an object

// iterating over the elements of tuple is messy
// it requires recursion and compile-time evaluation of the function body

template<size_t N = 0, typename... Ts>
constexpr void print(tuple<Ts...> tup)
{
    if constexpr (N<sizeof...(Ts)) { // not yet at the end?
    cout << get<N>(tup) << ' '; // print the Nth element
    print<N+1>(tup); // print the next element
    }
};

// sizeof(Ts) gives the number of elements of Ts
// examples of using print
print(t0); // no output
print(t2); // Herring 10 1.23
print(tuple{ "Norah", 17, "Gavin", 14, "Anya", 9, "Courtney", 9, "Ada", 0 });

// tuples provide whatever operators its elements provide
// tuple and pair can convert to one another if tuple has two members

//15.4 Alternatives
// standard offers union, variant<T...>, optional<T>, and any to express alternatives

// these types don't have a unified interface

// 15.4.1 variant
// often safer and more convienent than explicitly using a union
// possibly simplest example is returning either a value or an error code

variant<string, Error_code> compose_message(istream& s)
{
    string mess;
    // ...read from s and compose message ...
    if (no_problems)
        return mess; // return a string
    else
        return Error_code{some_problem}; // return an Error_code
}

// when you assign or initialize a variant with a value, it remembers the type o that value
// we can then inquire about what type the variant holds and extract the value

auto m = compose_message(cin);

if (holds_alternative<string>(m)) {
    cout << get<string>(m);
}
else {
    auto err = get<Error_code>(m);
    // .. handle error
}

// this style has some appeal if you don't like exceptions
// there are other, more interesting uses.

// a simple compiler may need to distinguish between different kinds of nodes with different representations

using Node = variant<Expression, Statement, Declaration, Type>;

void check(Node* p)
{
    if (holds_alternative<Expression>(*p)) {
        Expression& e = get<Expression>(*p);
        // ..
    }
    else if (holds_alternative<Statement>(*p)) 
    {
        Statement* s = get<Statement>(*p);
        // ..
    }
    // .. Declaration and Type ..
}


// this pattern of checking alternatives to decide on the appropriate is so common and relativly insufficient and it deserves direct support

void check(Node* p)
{
    visit(overloaded {
    [](Expression& e) { /* ... */ },
    [](Statement& s) { /* ... */ },
    // ... Declaration and Type ...
    }, *p);
}

// this is equivalent to a virtual function call, but potentially faster
// it's not a big different in performance in most use cases

// overloaded class is necessary and strangely enough, not standard
// overloaded builds a an overload set from a set of arguements
// usually from lambdas

template<class... Ts>
struct overloaded : Ts... { // variadic template
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>; // deduction guide

// visit then applies () to the overloaded object, which selects the best lambda to call based on the overload rules

// deduction guide: a mechanism for resolve ambiguites, particualy for constructors of class templates in foundation libraries
// bad_variant_access error is thrown if we try to access a variant holding a different type from the expected one.

// 15.4.2 optional

//optional<A> can be seen as a special kind of variant (like a variant<A,nothing>)
// or, as a generalaztion that A* can point to an object or nullptr

// an optional can be useful for functions that may or may not return an object

optional<string> compose_message(istream& s)
{
    string mess;
    // .. read from s and compose message ...

    if (no_problems)
        return mess;
    return {}; // the empty optional
}

if (auto m = compose_message(cin))
    cout << *m; // note the dereference
else {
    // .. handle error
}

// this is helpful if you don't like exceptions
// optionals are treated like references to an object, instead of like the object itself
// optional equivalent  to nullptr is the empty object, {}

int sum(optional<int> a, optional<int> b)
{
    int res = 0;
    if (a) res +=*a;
    if (b) res +=*b;
    return res;
}

int x = sum(17,19); // 36
int y = sum(17,{}); // 17
int z = sum({},{}) // 0

// accessing an optional that's doesn't hold a value is undefined behaivor, an error is not thrown
// optional is not guarantteed type safe

int sum2(optional<int> a, optional<int> b)
{
    return *a+*b; // asking for trouble
}

// 15.4.3 any
// an any can hold an arbitrary type and know which type (if any) it holds
// it's like an unconstraine version of variant

any compose_message(istream& s)
{
    string mess;
    // ... read from s and compose message ...
    if(no_problems)
        return mess; // return a string
    else
        return error_number; // return an int
}

// when you assign or initialize an any with a value, it remembers the type of that value.
// we can later extract the value by asserting the value's type

auto m = compose_message(cin);
string& s = any_cast<string>(m);
cout << s;

// if we try to access an any holding a different type than expected, bad_any_access is thrown