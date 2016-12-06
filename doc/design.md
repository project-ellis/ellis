# Design points

## C++ implementation, C wrappers

Modern C++11 allows for comfortable coding idioms that reduce bugs, as
well as shorter code (which also reduces bugs), while still allowing us to
achieve the efficiency of C.  So we think it is best to do a single
implementation in C++, and then provide corresponding C wrappers.

As such, the C implementation should be as barren as possible, and even the
C headers should simply refer to C++ headers for documentation, ideally, in
order to avoid replication of doc text.

Either the C wrappers or the C++ code can be used as the basis for bindings
for other languages.

## Refcounting with copy-on-write

For the sake of convenience, and to make sure that getting good performance
does not depend on C++ tricks, we use refcounting internally.

That way, in the event that the compiler generates a copy of some large
document hierarchy, the cost is minimal.

Copy-on-write means that if we modify a copy, it makes a copy of the needed
section of the copy at that time, and changes that, while leaving the original
intact.  The refcount is used to determine whether there are other copies to
leave intact, or whether this is the only copy and may be safely changed in
place.

The copy-on-write behavior is needed when working with refcounted trees
so that our C++ API matches expected C++ behavior, e.g. relating to const
correctness.

## Refcount glued to container

We have a union of different container types (for map, array, and binary
blobs), which is allocated in the same block (the "payload") as the reference
count.  Due to the desired copy-on-write behavior, neither the container nor
the reference count can live in the node structure, but rather than have a
pointer to the reference count and a pointer to the container, it suffices
to allocate a single block for both of them (cutting fragmentation and
improving locality) and just use a single pointer.

This comes at the cost of either having to deal with bizarre union
construction semantics in C++ (which seems like trouble) or having to
use in-place construction.  We opted for the latter, since it seemed
conceptually easier to do correctly.

## Mutable and constant versions of some functions

In carefully const-correct classes, one often finds two versions of a given
API, as for example with std::string::at().

The compiler is pretty good at making sure that non-const operations never
leak into const variables, though the other direction is safe.

However, extensive use of const and non-const overloading can get to be
confusing to both the compiler and the human.

Therefore, on certain operations that have both const and non-const versions,
we choose different names as an added precaution.

Rather than marking the const versions (as C++ STL has done with their
const iterators), we decided that the const case should be the normal
case, and we add `_mutable` to the function name for mutable access to
the contents of a node (e.g. `as_mutable_array`), so it's safe by default and
you have to be explicit when you are mutating.

In addition to safety/correctness, there is also a performance angle.  There
are some non-const operations that will cause proactive copy-on-write on the
assumption that a write is coming, and this too should not be casually
triggered when working with a const object.

## The `as_whatever` functions as gateways

Rather than doing repetitive checks to make sure a node is of type ARRAY
inside all the array-specific functions, or of type MAP inside all of the
map-specific functions, we decided that it was better to provide gateway
functions `as_array` and `as_map` that provide access to a casted alias type
that defines those functions.

This makes the type checking explicit and unavoidable, but allows the user to
store a reference to the alias, so the type checking code need only be
executed once.  For example,

```
  const array_node &a = n.as_array();
  for (size_t i = 0; i < a.length(); i++) {
    sum += a[i];
  }
```

Similar names are used for primitive data types, e.g. `as_double()` and so
forth.  These all return references, including for primitives, so you can
use the mutable versions to make changes:

```
  node n(type::DOUBLE);
  n.as_mutable_double() = 7.1;
```

## Casting operators
We provide cast operators for all the supported types. `std::string` and `const
char *` casts are explicit, in order to avoid ambiguity when presented with the
following code:

```
string s = (string)n;
```

With implicit string casts, that code will not compile because n can be cast
either directly to `std::string` or first to `const char *` and then to `const
char *`.

The other cast operators are implicit in order to reduce the number of operator
overload functions that need to be written, and to make the calling code easier
to use.

## Overloading for int

We do this so that when a person uses the constant 0, which is just an int
rather than an `int64_t`, the overloading will not cause this to be
interpreted in a completely crazy way e.g. bool or const char \*.
Experimentally, we determined that adding int, unsigned int, and `int64_t`
covers all cases we can find.

## Overloading for `uint64_t`

This would probably make things convenient for the user since `size_t` is
often of `uint64_t`, and we want for people to be able to use and set values
comfortably without manually casting to `int64_t`.

Such convenience comes at a cost, however, since it implies lying when we
offer to take arbitrarily large 64 bit unsigned values into a signed 64 bit
value.

We would have to either add checks for overflow, and throw an exception if a
sufficiently large value is ever assigned, which has performance implications
and can lead to a run-time surprise with unfortunate timing (aka bug), or we
reinterpret the bits as signed, send it over the wire as such, and leave it to
the user to do remember to do a symmetric cast to unsigned on the other side,
rather than accepting the signed value sent over the wire, which is also
accident prone and can lead to a bug.

We see no way to do provide such convenience without either proliferating
the number of internal types or risking being the cause of bugs, which seems
like a bad tradeoff.

Therefore, we do not provide overloading for `uint64_t` despite the loss of
convenience that is implied.
