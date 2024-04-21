# Check of modes correctness

## Info

https://blog.janestreet.com/oxidizing-ocaml-locality/

https://blog.janestreet.com/oxidizing-ocaml-ownership/

- **locality:** *global* (default) or *local* (value should not be passed out of context, can be allocated on stach)

- **uniqueness:** *shared* (default), *unique* (used only once, consumed), *exclusive* (used only once in the same time)

- **linearity:** *many* (default), *once* (result value can be used by caller only once), *separated* (result value can be used by caller only once in the same time)

## Done

- *unique* check +
- *exclusive* check
- *local* check
- *once* check
- *separated* check
- type check (optional)

## Examples

- *unique:* let f (unique x) = x * x in f;; -> error  

---

**bad design decisions:**

- shared_ptr instead of unique_ptr
- using namespace std
- use of indicies instead of visitor for std::variant

going to fix later (?)
