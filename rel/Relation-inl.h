#include "rel/Relation.h"

namespace rel {

// TUPLE STUFF

template<typename T>
Tuple<T>::Tuple() {
  #ifdef TUPLE_SIZE
    this->length = 0;
  #endif
}

template<typename T>
Tuple<T>::Tuple(const size_t len) {
  #ifdef TUPLE_SIZE
    this->length = len;
  #else
    this->data.resize(len);
  #endif
}

template<typename T>
Tuple<T>::Tuple(const size_t len, const T &val) {
  #ifdef TUPLE_SIZE
    this->length = len;
  #else
    this->data.resize(len);
  #endif
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for  \
        default(shared)       \
        private(i)            \
        nowait
    for (i = 0; i < len; ++i) {
      this->data[i] = val;
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < len; ++i) {
      this->data[i] = val;
    }
  }
  #elif defined(TUPLE_SIZE)
    fill(this->data, this->data + len, val);
  #else
    fill(this->data.begin(), this->data.end(), val);
  #endif
}

template<typename T>
Tuple<T>::Tuple(const Tuple<T> &copy) {
  size_t len = copy.size();
  #ifdef TUPLE_SIZE
    this->length = len;
  #else
    this->data.resize(len);
  #endif
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for  \
        default(shared)       \
        private(i)            \
        nowait
    for (i = 0; i < len; ++i) {
      this->data[i] = copy.data[i];
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < len; ++i) {
      this->data[i] = copy.data[i];
    }
  }
  #elif defined(TUPLE_SIZE)
    std::copy(copy.data, copy.data + len, this->data);
  #else
    std::copy(copy.begin(), copy.end(), this->data.begin());
  #endif
}

template<typename T>
Tuple<T>::~Tuple() {
  // do nothing
}

template<typename T>
Tuple<T> &Tuple<T>::operator=(const Tuple<T> &rhs) {
  if (this == &rhs) {
    return *this;
  }
  size_t len = rhs.size();
  #ifdef TUPLE_SIZE
    this->length = len;
  #else
    this->data.resize(len);
  #endif
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for  \
        default(shared)       \
        private(i)            \
        nowait
    for (i = 0; i < len; ++i) {
      this->data[i] = rhs.data[i];
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < len; ++i) {
      this->data[i] = rhs.data[i];
    }
  }
  #elif defined(TUPLE_SIZE)
    copy(rhs.data, rhs.data + len, this->data);
  #else
    copy(rhs.begin(), rhs.end(), this->data.begin());
  #endif
  return *this;
}

template<typename T>
void Tuple<T>::swap(Tuple<T> &t) {
  if (this == &t) {
    return;
  }
  #ifndef TUPLE_SIZE
    this->data.swap(t.data);
  #else
    size_t maxlen = max(this->length, t.length);
    std::swap(this->length, t.length);
    #if REL_CONFIG == REL_OMP
    {
      size_t i;
      #pragma omp paralle for \
          default(shared)     \
          private(i)          \
          nowait
      for (i = 0; i < maxlen; ++i) {
        std::swap(this->data[i], t.data[i]);
      }
    }
    #elif REL_CONFIG == REL_MTA
    {
      size_t i;
      #pragma mta assert parallel
      #pragma mta max concurrency TUPLE_SIZE
      for (i = 0; i < maxlen; ++i) {
        std::swap(this->data[i], t.data[i]);
      }
    }
    #else
      swap_ranges(this->data, this->data + maxlen, t.data);
    #endif
  #endif
}

template<typename T>
void Tuple<T>::absorb(Tuple<T> &t) {
  #ifdef TUPLE_SIZE
    *this = t;
  #else
    this->swap(t);
  #endif
}

template<typename T>
inline
T &Tuple<T>::operator[](const size_t i) {
  return this->data[i];
}

template<typename T>
inline
const T &Tuple<T>::operator[](const size_t i) const {
  return this->data[i];
}

template<typename T>
inline
typename Tuple<T>::iterator Tuple<T>::begin() {
  #ifdef TUPLE_SIZE
    return this->data;
  #else
    return this->data.begin();
  #endif
}

template<typename T>
inline
typename Tuple<T>::const_iterator Tuple<T>::begin() const {
  #ifdef TUPLE_SIZE
    return this->data;
  #else
    return this->data.begin();
  #endif
}

template<typename T>
inline
typename Tuple<T>::iterator Tuple<T>::end() {
  #ifdef TUPLE_SIZE
    return this->data + this->length;
  #else
    return this->data.end();
  #endif
}

template<typename T>
inline
typename Tuple<T>::const_iterator Tuple<T>::end() const {
  #ifdef TUPLE_SIZE
    return this->data + this->length;
  #else
    return this->data.end();
  #endif
}

template<typename T>
inline
size_t Tuple<T>::size() const {
  #ifdef TUPLE_SIZE
    return this->length;
  #else
    return this->data.size();
  #endif
}

template<typename T>
inline
void Tuple<T>::resize(const size_t sz) {
  #ifdef TUPLE_SIZE
    this->length = sz;
  #else
    this->data.resize(sz);
  #endif
}

template<typename T>
bool Tuple<T>::compat(const Tuple<T> &lhs, const Tuple<T> &rhs,
                      const size_t *lidx, const size_t *ridx,
                      const size_t len) {
  size_t i;
  bool compatible = true;
  #if REL_CONFIG == REL_OMP
  {
    #pragma omp parallel for default(shared) private(i) reduction(&&:compatible)
    for (i = 0; i < len; ++i) {
      compatible = compatible && lhs[lidx[i]] == rhs[ridx[i]];
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    #pragma mta loop recurrence
    for (i = 0; i < len; ++i) {
      compatible &= lhs[lidx[i]] == rhs[ridx[i]];
    }
  }
  #else
  {
    for (i = 0; compatible && i < len; ++i) {
      compatible = lhs[lidx[i]] == rhs[ridx[i]];
    }
  }
  #endif
  return compatible;
}


// ORDER STUFF

template<typename T, typename TLT>
Order<T, TLT>::Order()
    : total(true) {
  #ifdef TUPLE_SIZE
    this->length = 0;
  #endif
}

template<typename T, typename TLT>
Order<T, TLT>::Order(TLT less_than)
    : total(true), less_than(less_than) {
  #ifdef TUPLE_SIZE
    this->length = 0;
  #endif
}

template<typename T, typename TLT>
Order<T, TLT>::Order(const size_t *indexes, const size_t len,
                     const bool total)
    : total(total) {
  #ifdef TUPLE_SIZE
    this->length = 0;
  #else
    this->order.resize(len);
  #endif
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for default(shared) private(i) nowait
    for (i = 0; i < len; ++i) {
      this->order[i] = indexes[i];
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < len; ++i) {
      this->order[i] = indexes[i];
    }
  }
  #elif defined(TUPLE_SIZE)
    copy(indexes, indexes + len, this->order);
  #else
    copy(indexes, indexes + len, this->order.begin());
  #endif
}

template<typename T, typename TLT>
Order<T, TLT>::Order(const size_t *indexes, const size_t len,
                     const bool total, TLT less_than)
    : total(total), less_than(less_than) {
  #ifdef TUPLE_SIZE
    this->length = len;
  #else
    this->order.resize(len);
  #endif
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for default(shared) private(i) nowait
    for (i = 0; i < len; ++i) {
      this->order[i] = indexes[i];
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < len; ++i) {
      this->order[i] = indexes[i];
    }
  }
  #elif defined(TUPLE_SIZE)
    copy(indexes, indexes + len, this->order);
  #else
    copy(indexes, indexes + len, this->order.begin());
  #endif
}

template<typename T, typename TLT>
Order<T, TLT>::Order(const Order<T, TLT> &copy)
    : total(copy.total), less_than(copy.less_than) {
  size_t len = copy.size();
  #ifdef TUPLE_SIZE
    this->length = len;
  #else
    this->order.resize(len);
  #endif
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for default(shared) private(i) nowait
    for (i = 0; i < len; ++i) {
      this->order[i] = copy.order[i];
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < len; ++i) {
      this->order[i] = copy.order[i];
    }
  }
  #elif defined(TUPLE_SIZE)
    std::copy(copy.order, copy.order + len, this->order);
  #else
    std::copy(copy.order.begin(), copy.order.end(), this->order.begin());
  #endif
}

template<typename T, typename TLT>
Order<T, TLT>::~Order() {
  // do nothing
}

template<typename T, typename TLT>
Order<T, TLT> &Order<T, TLT>::operator=(const Order<T, TLT> &rhs) {
  if (this == &rhs) {
    return *this;
  }
  this->total = rhs.total;
  this->less_than = rhs.less_than;
  size_t len = rhs.size();
  #ifdef TUPLE_SIZE
    this->length = len;
  #else
    this->order.resize(len);
  #endif
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for default(shared) private(i) nowait
    for (i = 0; i < len; ++i) {
      this->order[i] = rhs.order[i];
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < len; ++i) {
      this->order[i] = rhs.order[i];
    }
  }
  #elif defined(TUPLE_SIZE)
    copy(rhs.order, rhs.order + len, this->order);
  #else
    copy(rhs.order.begin(), rhs.order.end(), this->order.begin());
  #endif
  return *this;
}

template<typename T, typename TLT>
void Order<T, TLT>::swap(Order<T, TLT> &o) {
  if (this == &o) {
    return;
  }
  std::swap(this->total, o.total);
  std::swap(this->less_than, o.less_than);
  #ifndef TUPLE_SIZE
    this->order.swap(o.order);
  #else
    size_t maxlen = max(this->length, o.length);
    std::swap(this->length, o.length);
    #if REL_CONFIG == REL_OMP
    {
      size_t i;
      #pragma omp parallel for default(shared) private(i) nowait
      for (i = 0; i < maxlen; ++i) {
        std::swap(this->order[i], o.order[i]);
      }
    }
    #elif REL_CONFIG == REL_MTA
    {
      size_t i;
      #pragma mta assert parallel
      #pragma mta max concurrency TUPLE_SIZE
      for (i = 0; i < maxlen; ++i) {
        std::swap(this->order[i], o.order[i]);
      }
    }
    #else
      swap_ranges(this->order, this->order + maxlen, o.order);
    #endif
  #endif
}

template<typename T, typename TLT>
inline
size_t Order<T, TLT>::operator[](const size_t i) const {
  return this->order[i];
}

template<typename T, typename TLT>
inline
size_t Order<T, TLT>::size() const {
  #ifdef TUPLE_SIZE
    return this->length;
  #else
    return this->order.size();
  #endif
}

template<typename T, typename TLT>
inline
bool Order<T, TLT>::is_total() const {
  return this->total;
}

#if REL_CONFIG == REL_MTA
#pragma mta parallel off
#endif
template<typename T, typename TLT>
bool Order<T, TLT>::operator()(const Tuple<T> &lhs, const Tuple<T> &rhs)
    const {
  size_t i;
  for (i = 0; i < this->size(); ++i) {
    size_t at = this->order[i];
    if (this->less_than(lhs[at], rhs[at])) {
      return true;
    } else if (this->less_than(rhs[at], lhs[at])) {
      return false;
    }
  }
  if (this->total) {
    size_t minlen = min(lhs.size(), rhs.size());
    for (i = 0; i < minlen; ++i) {
      if (this->less_than(lhs[i], rhs[i])) {
        return true;
      } else if (this->less_than(rhs[i], lhs[i])) {
        return false;
      }
    }
    return lhs.size() < rhs.size();
  }
  return false;
}
#if REL_CONFIG == REL_MTA
#pragma mta parallel on
#endif



// RELATION STUFF

template<typename V, typename T, typename VLT>
Relation<V, T, VLT>::Relation() {
  #ifdef REL_CONTAINER
    #if REL_CONFIG == REL_MTA
      writexf(&this->push_back_lock, 0);
    #endif
  #else
    this->tuples = NULL;
    this->length = 0;
    this->capacity = 0;
  #endif
  #if REL_CONFIG == REL_OMP
    omp_init_lock(&this->add_lock);
  #endif
}

template<typename V, typename T, typename VLT>
Relation<V, T, VLT>::Relation(VLT var_less_than)
    : attributes(map<V, size_t, VLT>(var_less_than)) {
  #ifdef REL_CONTAINER
    #if REL_CONFIG == REL_MTA
      writexf(&this->push_back_lock, 0);
    #endif
  #else
    this->tuples = NULL;
    this->length = 0;
    this->capacity = 0;
  #endif
  #if REL_CONFIG == REL_OMP
    omp_init_lock(&this->add_lock);
  #endif
}

template<typename V, typename T, typename VLT>
Relation<V, T, VLT>::Relation(const Relation<V, T, VLT> &copy)
    : attributes(copy.attributes) {
  #ifdef REL_CONTAINER
    #if REL_CONFIG == REL_MTA
      writexf(&this->push_back_lock, 0);
    #endif
  #else
    this->tuples = NULL;
    this->length = 0;
    this->capacity = 0;
  #endif
  #if REL_CONFIG == REL_OMP
    omp_init_lock(&this->add_lock);
  #endif
  size_t len = copy.size();
  this->resize(len);
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for default(shared) private(i) nowait
    for (i = 0; i < len; ++i) {
      this->tuples[i] = copy.tuples[i];
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < len; ++i) {
      this->tuples[i] = copy.tuples[i];
    }
  }
  #elif defined(REL_CONTAINER)
    std::copy(copy.tuples.begin(), copy.tuples.end(), this->tuples.begin());
  #else
    std::copy(copy.tuples, copy.tuples + copy.length, this->tuples);
  #endif
}

template<typename V, typename T, typename VLT>
Relation<V, T, VLT>::~Relation() {
  #ifndef REL_CONTAINER
    if (this->tuples != NULL) {
      delete[] this->tuples;
    }
  #endif
  #if REL_CONFIG == REL_OMP
    omp_destroy_lock(&this->add_lock);
  #endif
}

template<typename V, typename T, typename VLT>
Relation<V, T, VLT> &Relation<V, T, VLT>::operator=(
    const Relation<V, T, VLT> &rhs) {
  if (this == &rhs) {
    return *this;
  }
  this->attributes = rhs.attributes;
  size_t len = rhs.size();
  this->resize(len);
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for default(shared) private(i) nowait
    for (i = 0; i < len; ++i) {
      this->tuples[i] = rhs.tuples[i];
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < len; ++i) {
      this->tuples[i] = rhs.tuples[i];
    }
  }
  #elif defined(REL_CONTAINER)
    copy(rhs.tuples.begin(), rhs.tuples.end(), this->tuples.begin());
  #else
    copy(rhs.tuples, rhs.tuples + rhs.length, this->tuples);
  #endif
  return *this;
}

template<typename V, typename T, typename VLT>
void Relation<V, T, VLT>::swap(Relation<V, T, VLT> &r) {
  if (this == &r) {
    return;
  }
  this->attributes.swap(r.attributes);
  #ifdef REL_CONTAINER
    this->tuples.swap(r.tuples);
  #else
    std::swap(this->tuples, r.tuples);
    std::swap(this->length, r.length);
    std::swap(this->capacity, r.capacity);
  #endif
}

template<typename V, typename T, typename VLT>
inline
Tuple<T> &Relation<V, T, VLT>::operator[](const size_t i) {
  return this->tuples[i];
}

template<typename V, typename T, typename VLT>
inline
const Tuple<T> &Relation<V, T, VLT>::operator[](const size_t i) const {
  return this->tuples[i];
}

template<typename V, typename T, typename VLT>
inline
bool Relation<V, T, VLT>::empty() const {
  #ifdef REL_CONTAINER
    return this->tuples.empty();
  #else
    return this->length <= 0;
  #endif
}

template<typename V, typename T, typename VLT>
inline
size_t Relation<V, T, VLT>::size() const {
  #ifdef REL_CONTAINER
    return this->tuples.size();
  #else
    return this->length;
  #endif
}

template<typename V, typename T, typename VLT>
inline
size_t Relation<V, T, VLT>::capacity() const {
  #ifdef REL_CONTAINER
    return this->tuples.max_size();
  #else
    return this->capacity;
  #endif
}

template<typename V, typename T, typename VLT>
void Relation<V, T, VLT>::resize(const size_t sz) {
  #ifdef REL_CONTAINER
    this->tuples.resize(sz);
  #else
    this->reserve(sz);
    this->length = sz;
  #endif
}

// NOT thread-safe.
template<typename V, typename T, typename VLT>
void Relation<V, T, VLT>::reserve(const size_t sz) {
  #ifndef REL_CONTAINER
    if (sz > this->capacity) {
      Tuple<T> *new_tuples = new Tuple<T>[sz];
      if (new_tuples == NULL) {
        cerr << "[ERROR] Unable to allocate array of " << sz << " tuples." << endl;
      }
      #if REL_CONFIG == REL_OMP
      {
        size_t i;
        #pragma omp parallel for default(shared) private(i) nowait
        for (i = 0; i < this->length; ++i) {
          new_tuples[i].absorb(this->tuples[i]);
        }
      }
      #elif REL_CONFIG == REL_MTA
      {
        size_t i;
        #pragma mta assert parallel
        for (i = 0; i < this->length; ++i) {
          new_tuples[i].absorb(this->tuples[i]);
        }
      }
      #else
      {
        size_t i;
        for (i = 0; i < this->length; ++i) {
          new_tuples[i].absorb(this->tuples[i]);
        }
      }
      #endif
      if (this->tuples != NULL) {
        delete[] this->tuples;
      }
      this->tuples = new_tuples;
      this->capacity = sz;
    }
  #endif
}

// Thread-safe.
// NOTE that this is NOT guaranteed to be a dynamically
// resizing container.  You must ensure there is enough
// capacity for the add by calling reserve beforehand.
template<typename V, typename T, typename VLT>
void Relation<V, T, VLT>::add(const Tuple<T> &tuple) {
  #if REL_CONFIG == REL_OMP
  {
    #ifdef REL_CONTAINER
      omp_set_lock(&this->add_lock)
      this->tuples.push_back(tuple);
      omp_unset_lock(&this->add_lock)
    #else
      omp_set_lock(&this->add_lock)
      size_t at = this->length++;
      omp_unset_lock(&this->add_lock)
      this->tuples[at] = tuple;
    #endif
  }
  #elif REL_CONFIG == REL_MTA
  {
    #ifdef REL_CONTAINER
      readfe(&this->push_back_lock);
      this->tuples.push_back(tuple);
      writeef(&this->push_back_lock, 1);
    #else
      this->tuples[int_fetch_add(&this->length, 1)] = tuple;
    #endif
  }
  #elif defined(REL_CONTAINER)
    this->tuples.push_back(tuple);
  #else
    this->tuples[this->length++] = tuple;
  #endif
}

template<typename V, typename T, typename VLT>
inline
void Relation<V, T, VLT>::clear() {
  #ifdef REL_CONTAINER
    this->tuples.clear();
  #else
    this->length = 0;
  #endif
}

template<typename V, typename T, typename VLT>
inline
typename Relation<V, T, VLT>::iterator Relation<V, T, VLT>::begin() {
  #ifdef REL_CONTAINER
    return this->tuples.begin();
  #else
    return this->tuples;
  #endif
}

template<typename V, typename T, typename VLT>
inline
typename Relation<V, T, VLT>::const_iterator Relation<V, T, VLT>::begin() const {
  #ifdef REL_CONTAINER
    return this->tuples.begin();
  #else
    return this->tuples;
  #endif
}

template<typename V, typename T, typename VLT>
inline
typename Relation<V, T, VLT>::iterator Relation<V, T, VLT>::end() {
  #ifdef REL_CONTAINER
    return this->tuples.end();
  #else
    return this->tuples + this->length;
  #endif
}

template<typename V, typename T, typename VLT>
inline
typename Relation<V, T, VLT>::const_iterator Relation<V, T, VLT>::end() const {
  #ifdef REL_CONTAINER
    return this->tuples.end();
  #else
    return this->tuples + this->length;
  #endif
}

template<typename V, typename T, typename VLT>
inline
const map<V, size_t, VLT> &Relation<V, T, VLT>::attrs() const {
  return this->attributes;
}

template<typename V, typename T, typename VLT>
void Relation<V, T, VLT>::meld(const Relation<V, T, VLT> &r) {
  size_t old_size = this->size();
  size_t add_size = r.size();
  this->resize(old_size + add_size);
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for default(shared) private(i) nowait
    for (i = 0; i < add_size; ++i) {
      this->data[old_size+i] = r.data[i];
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < add_size; ++i) {
      this->data[old_size+i] = r.data[i];
    }
  }
  #elif defined(REL_CONTAINER)
    copy(r.data.begin(), r.data.end(), this->data.begin() + old_size);
  #else
    copy(r.data, r.data + add_size, this->data + old_size);
  #endif
}

template<typename V, typename T, typename VLT>
void Relation<V, T, VLT>::meld(const Relation<V, T, VLT> &r,
                               Relation<V, T, VLT> &results) const {
  Relation<V, T, VLT> temp(results.less_than);
  size_t old_size = this->size();
  size_t add_size = r.size();
  temp.resize(old_size+add_size);
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for default(shared) private(i) nowait
    for (i = 0; i < old_size; ++i) {
      temp[i] = this->data[i];
    }
    #pragma omp parallel for default(shared) private(i) nowait
    for (i = 0; i < add_size; ++i) {
      temp[old_size+i] = r.data[i];
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < old_size; ++i) {
      temp[i] = this->data[i];
    }
    #pragma mta assert parallel
    for (i = 0; i < add_size; ++i) {
      temp[old_size+i] = r.data[i];
    }
  }
  #elif defined(REL_CONTAINER)
  {
    copy(this->data.begin(), this->data.end(), temp.data.begin());
    copy(r.data.begin(), r.data.end(), temp.data.begin() + old_size);
  }
  #else
  {
    copy(this->data, this->data + old_size, temp.data);
    copy(r.data, r.data + add_size, temp.data + old_size);
  }
  #endif
  results.swap(temp);
}

template<typename V, typename T, typename VLT>
size_t Relation<V, T, VLT>::index(
    const V *vars, const T *values, const size_t len1,
    const V *vars1, const V *vars2, const size_t len2,
    const bool sign, size_t *&idx) const {
  if (this->empty()) {
    idx = NULL;
    return 0;
  }
  size_t k, retval;
  size_t *varn, *varn1, *varn2;
  varn = (size_t*)malloc(len1 * sizeof(size_t));
  varn1 = (size_t*)malloc(len2 * sizeof(size_t));
  varn2 = (size_t*)malloc(len2 * sizeof(size_t));
  for (k = 0; k < len1; ++k) {
    typename map<V, size_t, VLT>::const_iterator it = this->attributes.find(vars[k]);
    if (it == this->attributes.end()) {
      cerr << "[ERROR] Index requested for variable that is not an attribute of the relation.\n";
    } else {
      varn[k] = it->second;
    }
  }
  for (k = 0; k < len2; ++k) {
    typename map<V, size_t, VLT>::const_iterator it = this->attributes.find(vars1[k]);
    if (it == this->attributes.end()) {
      cerr << "[ERROR] Index requested for variable that is not an attribute of the relation.\n";
    } else {
      varn1[k] = it->second;
    }
    it = this->attributes.find(vars2[k]);
    if (it == this->attributes.end()) {
      cerr << "[ERROR] Index requested for variable that is not an attribut of the relation.\n";
    } else {
      varn2[k] = it->second;
    }
  }
  #if REL_CONFIG == REL_OMP
  {
    vector<deque<size_t> > nums;
    int tid, nthreads;
    size_t i;
    #pragma omp parallel default(shared) private(tid, nthreads, i)
    {
      tid = omp_get_thread_num();
      nthreads = omp_get_num_threads();
      #pragma omp single
      {
        nums.resize(nthreads);
        #pragma omp flush (nums)
      }
      pair<size_t, size_t> bounds = this->thread_bounds(tid, nthreads);
      size_t begin = bounds.first;
      size_t end = bounds.second;
      for (; begin < end; ++begin) {
        const Tuple<T> &tuple = this->tuples[begin];
        bool match = true;
        #pragma omp for reduction(&&:match)
        for (i = 0; i < len1; ++i) {
          match = match && 
            (( sign && tuple[varn[i]] == values[i]) ||
             (!sign && tuple[varn[i]] != values[i]));
        }
        if (!match) {
          continue;
        }
        #pragma omp for reduction(&&:match)
        for (i = 0; i < len2; ++i) {
          match = match && 
            (( sign && tuple[varn1[i]] == tuple[varn2[i]]) ||
             (!sign && tuple[varn1[i]] != tuple[varn2[i]]));
        }
        if (match) {
          nums[tid].push_back(begin);
        }
      }
    }
    size_t num_nums = 0;
    #pragma omp parallel for default(shared) private(i) reduction(+:num_nums)
    for (i = 0; i < nums.size(); ++i) {
      num_nums = num_nums + nums[i].size();
    }
    if (num_nums <= 0) {
      idx = NULL;
      return 0;
    }
    idx = (size_t*)malloc(num_nums * sizeof(size_t));
    size_t offset = 0;
    for (i = 0; i < nums.size(); ++i) {
      size_t j;
      size_t max = nums[i].size();
      #pragma omp parallel for default(shared) private(j)
      for (j = 0; j < max; ++j) {
        idx[offset+j] = nums[i][j];
      }
      offset += max;
    }
    retval = num_nums;
  }
  #elif REL_CONFIG == REL_MTA
  {
    int sid, nstreams;
    size_t ntuples = this->size();
    idx = (size_t*)malloc(ntuples * sizeof(size_t));
    size_t count = 0;
    #pragma mta for all streams sid of nstreams
    {
      size_t i;
      deque<size_t> nums;
      pair<size_t, size_t> bounds = this->thread_bounds(sid, nstreams);
      size_t begin = bounds.first;
      size_t end = bounds.second;
      #pragma mta loop serial
      for (; begin < end; ++begin) {
        const Tuple &tuple = this->tuples[begin];
        bool match = true;
        #pragma mta loop recurrence
        for (i = 0; i < len1; ++i) {
          match = match && 
            (( sign && tuple[varn[i]] == values[i]) ||
             (!sign && tuple[varn[i]] != values[i]));
        }
        if (!match) {
          continue;
        }
        #pragma mta loop recurrence
        for (i = 0; i < len2; ++i) {
          match = match && 
            (( sign && tuple[varn1[i]] == tuple[varn2[i]]) ||
             (!sign && tuple[varn1[i]] != tuple[varn2[i]]));
        }
        if (match) {
          nums.push_back(begin);
        }
      }
      size_t max = nums.size();
      size_t offset = int_fetch_add(&count, max);
      #pragma mta assert parallel
      for (i = 0; i < max; ++i) {
        idx[offset+i] = nums[i];
      }
    }
    idx = (size_t*)realloc(idx, count * sizeof(size_t));
    retval = count;
  }
  #else
  {
    deque<size_t> nums;
    size_t i, j;
    for (i = 0; i < this->size(); ++i) {
      const Tuple<T> &tuple = this->tuples[i];
      for (j = 0; j < len1; ++j) {
        if (sign && tuple[varn[j]] != values[j]) {
          break;
        } else if (!sign && tuple[varn[j]] == values[j]) {
          break;
        }
      }
      if (j < len1) {
        continue;
      }
      for (j = 0; j < len2 && tuple[varn1[j]] == tuple[varn2[j]]; ++j) {
        if (sign && tuple[varn1[j]] != tuple[varn2[j]]) {
          break;
        } else if (!sign && tuple[varn1[j]] == tuple[varn2[j]]) {
          break;
        }
      }
      if (j >= len2) {
        nums.push_back(i);
      }
    }
    if (nums.size() <= 0) {
      idx = NULL;
    } else {
      idx = (size_t*)malloc(nums.size() * sizeof(size_t));
      copy(nums.begin(), nums.end(), idx);
    }
    retval = nums.size();
  }
  #endif
  free(varn);
  free(varn1);
  free(varn2);
  return retval;
}

template<typename V, typename T, typename VLT>
bool Relation<V, T, VLT>::select(const V *vars, const T *values, const size_t len1,
                                 const V *vars1, const V *vars2, const size_t len2,
                                 const bool sign) {
  Relation<V, T, VLT> temp;
  if (this->select(vars, values, len1, vars1, vars2, len2, sign, temp)) {
    this->swap(temp);
    return true;
  }
  return false;
}

template<typename V, typename T, typename VLT>
bool Relation<V, T, VLT>::select(const V *vars, const T *values, const size_t len1,
                                 const V *vars1, const V *vars2, const size_t len2,
                                 const bool sign, Relation<V, T, VLT> &results) const {
  Relation<V, T, VLT> temp;
  temp.attributes = this->attributes;
  if (this->empty()) {
    results.swap(temp);
    return true;
  }
  size_t *idx;
  size_t nselect = this->index(vars, values, len1, vars1, vars2, len2, sign, idx);
  temp.resize(nselect);
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for default(shared) private(i) nowait
    for (i = 0; i < nselect; ++i) {
      temp[i] = this->tuples[idx[i]];
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < nselect; ++i) {
      temp[i] = this->tuples[idx[i]];
    }
  }
  #else
  {
    size_t i;
    for (i = 0; i < nselect; ++i) {
      temp[i] = this->tuples[idx[i]];
    }
  }
  #endif
  free(idx);
  results.swap(temp);
  return true;
}

template<typename V, typename T, typename VLT>
bool Relation<V, T, VLT>::project(const V *vars, const size_t len) {
  bool retval = true;
  map<V, size_t, VLT> projected(this->attributes.key_comp());
  size_t i;
  for (i = 0; i < len; ++i) {
    typename map<V, size_t, VLT>::const_iterator it = this->attributes.find(vars[i]);
    if (it == this->attributes.end()) {
      cerr << "[ERROR] Attempted to project a variable that is not an attribute of the relation.\n";
      retval = false;
    } else {
      projected.insert(*it);
    }
  }
  this->attributes.swap(projected);
  return retval;
}

template<typename V, typename T, typename VLT>
bool Relation<V, T, VLT>::project(const V *vars, const size_t len,
                                  Relation<V, T, VLT> &results) const {
  bool retval = true;
  map<V, size_t, VLT> projected(this->attributes.key_comp());
  size_t *varn = (size_t*)malloc(len * sizeof(size_t));
  if (varn == NULL) {
    cerr << "[ERROR] Unable to allocate " << len * sizeof(size_t) << " bytes for variable index array.\n";
    retval = false;
  }
  size_t i, max;
  for (i = 0; i < len; ++i) {
    typename map<V, size_t, VLT>::const_iterator it = this->attributes.find(vars[i]);
    if (it == this->attributes.end()) {
      cerr << "[ERROR] Attempted to project a variable that is not an attribute of the relation.\n";
      retval = false;
    } else {
      projected.insert(std::pair<V, size_t>(it->first, i));
      varn[i] = it->second;
    }
  }
  Relation<V, T, VLT> temp;
  max = this->size();
  temp.resize(max);
  #if REL_CONFIG == REL_OMP
  {
    #pragma omp parallel for default(shared) private(i) nowait
    for (i = 0; i < max; ++i) {
      temp[i].resize(len);
      size_t j;
      #pragma omp for default(shared) private(j) nowait
      for (j = 0; j < len; ++j) {
        temp[i][j] = this->data[i][varn[j]];
      }
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    #pragma mta assert parallel
    for (i = 0; i < max; ++i) {
      temp[i].resize(len);
      size_t j;
      #pragma mta assert parallel
      for (j = 0; j < len; ++j) {
        temp[i][j] = this->data[i][varn[j]];
      }
    }
  }
  #else
  {
    for (i = 0; i < max; ++i) {
      temp[i].resize(len);
      size_t j;
      for (j = 0; j < len; ++j) {
        temp[i][j] = this->data[i][varn[j]];
      }
    }
  }
  #endif
  free(varn);
  results.swap(temp);
  return retval;
}

template<typename V, typename T, typename VLT>
void Relation<V, T, VLT>::name(const map<V, size_t, VLT> &namings) {
  this->attributes = namings;
}

template<typename V, typename T, typename VLT>
bool Relation<V, T, VLT>::rename(const map<V, V, VLT> &renamings) {
  bool retval = true;
  map<V, size_t, VLT> new_attrs(this->attributes.key_comp());
  typename map<V, V, VLT>::const_iterator it = renamings.begin();
  for (; it != renamings.end(); ++it) {
    new_attrs[it->second] = this->attributes[it->first];
  }
  if (new_attrs.size() != renamings.size()) {
    cerr << "[ERROR] Renamings have different variables mapped to the same variable.\n";
    retval = false;
  }
  this->attributes.swap(new_attrs);
  return retval;
}

template<typename V, typename T, typename VLT>
bool Relation<V, T, VLT>::rename(const map<V, V, VLT> &renamings,
                                 Relation<V, T, VLT> &results) const {
  Relation<V, T, VLT> temp (*this);
  bool retval = temp.rename(renamings);
  results.swap(temp);
  return retval;
}

template<typename V, typename T, typename VLT>
std::pair<size_t, size_t> Relation<V, T, VLT>::thread_bounds(
    const size_t id, const size_t n) const {
  std::pair<size_t, size_t> bounds;
  size_t amount = this->size();
  size_t chunk_size = amount / n;
  size_t remainder = amount % n;
  bounds.first = id * chunk_size;
  bounds.second = bounds.first + chunk_size;
  if (id < remainder) {
    bounds.first += id;
    bounds.second += id + 1;
  } else {
    bounds.first += remainder;
    bounds.second += remainder;
  }
  return bounds;
}

template<typename V, typename T, typename VLT>
template<typename TLT>
Order<T, TLT> Relation<V, T, VLT>::make_order(const Relation<V, T, VLT> &r,
                                              const V *vars, const size_t len,
                                              TLT t_less_than) {
  size_t *indexes = (size_t*)malloc(len * sizeof(size_t));
  if (indexes == NULL) {
    cerr << "[ERROR] Could not allocate " << len * sizeof(size_t) << " bytes to make order.\n";
  }
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for default(shared) private(i) nowait
    for (i = 0; i < len; ++i) {
      typename map<V, size_t, VLT>::const_iterator it = r.attributes.find(vars[i]);
      if (it == r.attributes.end()) {
        cerr << "[ERROR] Requested order to be made for variable that is not an attribute of the relation.\n";
      } else {
        indexes[i] = it->second;
      }
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < len; ++i) {
      typename map<V, size_t, VLT>::const_iterator it = r.attributes.find(vars[i]);
      if (it == r.attributes.end()) {
        cerr << "[ERROR] Requested order to be made for variable that is not an attribute of the relation.\n";
      } else {
        indexes[i] = it->second;
      }
    }
  }
  #else
  {
    size_t i;
    for (i = 0; i < len; ++i) {
      typename map<V, size_t, VLT>::const_iterator it = r.attributes.find(vars[i]);
      if (it == r.attributes.end()) {
        cerr << "[ERROR] Requested order to be made for variable that is not an attribute of the relation.\n";
      } else {
        indexes[i] = it->second;
      }
    }
  }
  #endif
  Order<T, TLT> order(indexes, len, false, t_less_than);
  free(indexes);
  return order;
}

template<typename V, typename T, typename VLT>
template<typename TLT>
void Relation<V, T, VLT>::sort(Relation<V, T, VLT> &r,
                               const Order<T, TLT> &order) {
  if (r.empty()) {
    return;
  }
  typename Relation<V, T, VLT>::iterator begin = r.begin();
  size_t len = r.size();
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    #pragma omp parallel for default(shared) private(i)
    for (i = 0; i < len; i += 1024) {
      size_t upper_bound = i + min(1024, len - i);
      std::sort(begin + i, begin + upper_bound, order);
    }
    for (i = 2048; i < (len << 1); i <<= 1) {
      size_t j;
      #pragma omp parallel for default(shared) private(j)
      for (j = 0; j < len; j += i) {
        size_t mid_bound = j + min((i >> 1), len - j);
        size_t upper_bound = j + min(i, len - j);
        inplace_merge(begin + j, begin + mid_bound, begin + upper_bound, order);
      }
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #if !defined(REL_CONTAINER) && USE_MTGL
      mtgl::merge_sort(begin, len, order);
    #else
      #pragma mta assert parallel
      for (i = 0; i < len; i += 1024) {
        size_t upper_bound = i + min(1024, len - i);
        std::sort(begin + i; begin + upper_bound, order);
      }
      #pragma mta loop serial
      for (i = 2048; i < (len << 1); i <<= 1) {
        size_t j;
        #pragma mta assert parallel
        for (j = 0; j < len; j += i) {
          size_t mid_bound = j + min((i >> 1), len - j);
          size_t upper_bound = j + min(i, len - j);
          inplace_merge(begin + j, begin + mid_bound, begin + upper_bound, order);
        }
      }
    #endif
  }
  #else
  {
    std::sort(begin, begin + len, order);
  }
  #endif
}

template<typename V, typename T, typename VLT>
template<typename TLT>
void Relation<V, T, VLT>::uniq(Relation<V, T, VLT> &r, const Order<T, TLT> &order) {
  if (r.empty()) {
    return;
  }
  size_t len = r.size();
  Relation<V, T, VLT>::sort(r, order);
  #if REL_CONFIG == REL_OMP
  {
    size_t i;
    vector<size_t> begins, ends, offsets;
    size_t nregions = (len >> 10) + ((len & 1023) == 0 ? 0 : 1);
    begins.resize(nregions);
    ends.resize(nregions);
    offsets.resize(nregions);
    #pragma omp parallel for default(shared) private(i)
    for (i = 0; i < len; i += 1024) {
      size_t offset = i;
      size_t j;
      size_t upper = offset + min(1024, len - i);
      for (j = offset + 1; j < upper; ++j) {
        if (order(r[offset], r[j])) {
          r[++offset] = r[j];
        }
      }
      ends[i >> 10] = offset + 1;
    }
    begins[0] = 0;
    #pragma omp parallel for default(shared) private(i)
    for (i = 1; i < nregions; ++i) {
      const Tuple &predecessor = r[ends[i-1]-1];
      for (begins[i] = i << 10;
           begins[i] < ends[i] && !order(predecessor, r[begins[i]]);
           ++begins[i]) {
        // loop does the word
      }
    }
    offsets[0] = 0;
    for (i = 1; i < nregions; ++i) {
      offsets[i] = offsets[i-1] + (ends[i-1] - begins[i-1]);
    }
    #pragma omp parallel for default(shared) private(i)
    for (i = 0; i < nregions; ++i) {
      size_t j;
      #pragma omp for default(shared) private(j) nowait
      for (j = begins[i]; j < ends[i]; ++j) {
        r[offsets[i]+(j-begins[i])] = r[j];
      }
    }
    r.resize(offsets[nregions-1] + (ends[nregions-1] - begins[nregions-1]));
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    size_t *begins, *ends, *offsets;
    size_t nregions = (len >> 10) + ((len & 1023) == 0 ? 0 : 1);
    begins = (size_t*)malloc(nregions * sizeof(size_t));
    ends = (size_t*)malloc(nregions * sizeof(size_t));
    offsets = (size_t*)malloc(nregions * sizeof(size_t));
    #pragma mta assert parallel
    for (i = 0; i < len; i += 1024) {
      size_t offset = i;
      size_t j;
      size_t upper = offset + min(1024, len - i);
      #pragma mta loop serial
      for (j = offset + 1; j < upper; ++j) {
        if (order(r[offset], r[j])) {
          r[++offset] = r[j];
        }
      }
      ends[i >> 10] = offset + 1;
    }
    begins[0] = 0;
    #pragma mta assert parallel
    for (i = 1; i < nregions; ++i) {
      const Tuple &predecessor = r[ends[i-1]-1];
      #pragma mta loop serial
      for (begins[i] = i << 10;
           begins[i] < ends[i] && !order(predecessor, r[begins[i]]);
           ++begins[i]) {
        // loop does the work
      }
    }
    offsets[0] = 0;
    #pragma mta loop recurrence
    for (i = 1; i < nregions; ++i) {
      offsets[i] = offsets[i-1] + (ends[i-1] - begins[i-1]);
    }
    #pragma mta assert parallel
    for (i = 0; i < nregions; ++i) {
      size_t j;
      #pragma mta assert parallel
      for (j = begins[i]; j < ends[i]; ++j) {
        r[offsets[i]+(j-begins[i])] = r[j];
      }
    }
    r.resize(offsets[nregions-1] + (ends[nregions-1] - begins[nregions-1]));
    free(begins);
    free(ends);
    free(offsets);
  }
  #else
  {
    size_t offset = 0;
    size_t i;
    for (i = 1; i < len; ++i) {
      if (order(r[offset], r[i])) {
        r[++offset] = r[i];
      }
    }
    r.resize(++offset);
  }
  #endif
}

template<typename V, typename T, typename VLT>
void Relation<V, T, VLT>::nested_loop_join(const Relation<V, T, VLT> &lhs,
                                           const Relation<V, T, VLT> &rhs,
                                           Relation<V, T, VLT> &results) {
  const Relation<V, T, VLT> *larger, *smaller;
  if (lhs.size() >= rhs.size()) {
    larger = &lhs;
    smaller = &rhs;
  } else {
    larger = &rhs;
    smaller = &lhs;
  }
  vector<std::pair<size_t, size_t> > eqs;
  vector<size_t> vars;
  map<V, size_t, VLT> attrs(lhs.attributes.key_comp());
  Relation<V, T, VLT> temp;
  typename map<V, size_t, VLT>::const_iterator it = larger->attributes.begin();
  for (; it != larger->attributes.end(); ++it) {
    attrs.insert(std::pair<V, size_t>(it->first, attrs.size()));
    vars.push_back(it->second);
  }
  size_t var_boundary = vars.size();
  for (it = smaller->attributes.begin(); it != smaller->attributes.end(); ++it) {
    typename map<V, size_t, VLT>::const_iterator it2 = attrs.find(it->first);
    if (it2 == attrs.end()) {
      attrs.insert(std::pair<V, size_t>(it->first, attrs.size()));
      vars.push_back(it->second);
    } else {
      eqs.push_back(std::pair<size_t, size_t>(vars[it2->second], it->second));
    }
  }
  size_t tuple_size = attrs.size();
  size_t *lidx, *ridx;
  size_t len = eqs.size();
  lidx = (size_t*)malloc(len * sizeof(size_t));
  ridx = (size_t*)malloc(len * sizeof(size_t));
  size_t i, j, k;
  for (i = 0; i < len; ++i) {
    lidx[i] = eqs[i].first;
    ridx[i] = eqs[i].second;
  }

  temp.attributes.swap(attrs);

  #if REL_CONFIG == REL_OMP
  {
    size_t lmax = larger->size();
    size_t rmax = smaller->size();

    vector<deque<std::pair<size_t, size_t> > > joins;
    int tid, nthreads;
    #pragma omp parallel default(shared) private(tid, nthreads, i, j)
    {
      tid = omp_get_thread_num();
      nthreads = omp_get_num_threads();
      #pragma omp single
      {
        joins.resize(nthreads);
        #pragma omp flush (joins)
      }
      pair<size_t, size_t> bounds = larger->thread_bounds(tid, nthreads);
      size_t begin = bounds.first;
      size_t end = bounds.second;
      for (; begin < end; ++begin) {
        for (j = 0; j < rmax; ++j) {
          if (Tuple<T>::compat((*larger)[begin], (*smaller)[j], lidx, ridx, len)) {
            joins[tid].push_back(std::pair<size_t, size_t>(begin, j));
          }
        }
      }
    }
    size_t nresults = 0;
    #pragma omp parallel for default(shared) private(i) reduction(+:nresults)
    for (i = 0; i < joins.size(); ++i) {
      nresults = nresults + joins[i].size();
    }
    temp.resize(nresults);
    size_t offset = 0;
    for (i = 0; i < joins.size(); ++i) {
      #pragma omp parallel for default(shared) private(j,k)
      for (j = 0; j < joins[i].size(); ++j) {
        const Tuple &ltuple = (*larger)[joins[i][j].first];
        const Tuple &rtuple = (*smaller)[joins[i][j].second];
        Tuple tuple(tuple_size);
        for (k = 0; k < tuple_size; ++k) {
          tuple[k] = k < var_boundary ? ltuple[vars[k]] : rtuple[vars[k]];
        }
        temp[offset + j].absorb(tuple);
      }
      offset += joins[i].size();
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t lmax = larger->size();
    size_t rmax = smaller->size();
    vector<vector<std::pair<size_t, size_t> > > joins;
    int sid, nstreams, resize_lock;
    purge(&resize_lock);
    size_t numcopies = lmax / rmax;
    size_t left_zeros = MTA_BIT_LEFT_ZEROS(numcopies);
    numcopies = (1 << (sizeof(size_t) - left_zeros));
    numcopies = min(numcopies, 512);
    Relation<V, T, VLT> *copies = new Relation<V, T, VLT>[numcopies];
    #pragma mta assert parallel
    for (i = 0; i < numcopies; ++i) {
      copies[i] = (*smaller);
    }
    #pragma mta for all streams sid of nstreams
    {
      if (sid == 0) {
        joins.resize(nstreams);
        writeef(&resize_lock, 0);
      }
      pair<size_t, size_t> bounds = larger->thread_bounds(sid, nstreams);
      size_t begin = bounds.first;
      size_t end = bounds.second;
      if (sid != 0) {
        readff(&resize_lock);
      }
      #pragma mta loop serial
      for (; begin < end; ++begin) {
        size_t j;
        #pragma mta loop serial
        for (j = 0; j < rmax; ++j) {
          if (Tuple<T>::compat((*larger)[begin], copies[MTA_CLOCK(0) & (numcopies-1)][j], lidx, ridx, len)) {
            joins[sid].push_back(std::pair<size_t, size_t>(begin, j));
          }
        }
      }
    }
    size_t *offsets = (size_t*)malloc(joins.size() * sizeof(size_t));
    offsets[0] = 0;
    #pragma mta assert parallel
    for (i = 1; i < joins.size(); ++i) {
      offsets[i] = joins[i-1].size();
    }
    #pragma mta loop recurrence
    for (i = 2; i < joins.size(); ++i) {
      offsets[i] += offsets[i-1];
    }
    size_t nresults = offsets[joins.size()-1] + joins[joins.size()-1];
    temp.resize(nresults);
    std::pair<size_t, size_t> *joins_array = new std::pair<size_t, size_t>[nresults];
    #pragma mta assert parallel
    for (i = 0; i < joins.size(); ++i) {
      #pragma mta assert parallel
      for (j = 0; j < joins[i].size(); ++i) {
        joins_array[offsets[i]+j] = joins[i][j];
      }
    }
    #pragma mta assert parallel
    for (i = 0; i < nresults; ++i) {
      // Hoping that making local copies here will help avoid read hot-spotting.
      Tuple ltuple = (*larger)[joins_array[i].first];
      Tuple rtuple = copies[MTA_CLOCK(0) & (numcopies-1)][joins_array[i].second];
      Tuple &new_tuple = temp[i];
      new_tuple.resize(tuple_size);
      #pragma mta assert parallel
      for (k = 0; k < tuple_size; ++k) {
        new_tuple[k] = k < var_boundary ? ltuple[vars[k]] : rtuple[vars[k]];
      }
    }
    delete[] copies;
    free(offsets);
    free(joins_array);
  }
  #else
  {
    temp.reserve(larger->size() << 1);
    for (i = 0; i < larger->size(); ++i) {
      for (j = 0; j < smaller->size(); ++j) {
        if (Tuple<T>::compat((*larger)[i], (*smaller)[j], lidx, ridx, len)) {
          if (temp.size() >= temp.capacity()) {
            temp.reserve(temp.size() + larger->size());
          }
          Tuple<T> tuple(tuple_size);
          for (k = 0; k < tuple_size; ++k) {
            tuple[k] = k < var_boundary ? (*larger)[i][vars[k]] : (*smaller)[j][vars[k]];
          }
          temp.add(tuple);
        }
      }
    }
  }
  #endif
  free(lidx);
  free(ridx);
  results.swap(temp);
}

template<typename V, typename T, typename VLT>
void Relation<V, T, VLT>::index_join(Relation<V, T, VLT> &lhs,
                                     Relation<V, T, VLT> &rhs,
                                     Relation<V, T, VLT> &results) {
  const Relation<V, T, VLT> *larger;
  Relation<V, T, VLT> *smaller;
  if (lhs.size() >= rhs.size()) {
    larger = &lhs;
    smaller = &rhs;
  } else {
    larger = &rhs;
    smaller = &lhs;
  }
  vector<std::pair<size_t, size_t> > eqs;
  vector<size_t> vars;
  map<V, size_t, VLT> attrs(lhs.attributes.key_comp());
  Relation<V, T, VLT> temp;
  typename map<V, size_t, VLT>::const_iterator it = larger->attributes.begin();
  for (; it != larger->attributes.end(); ++it) {
    attrs.insert(std::pair<V, size_t>(it->first, attrs.size()));
    vars.push_back(it->second);
  }
  size_t var_boundary = vars.size();
  for (it = smaller->attributes.begin(); it != smaller->attributes.end(); ++it) {
    typename map<V, size_t, VLT>::const_iterator it2 = attrs.find(it->first);
    if (it2 == attrs.end()) {
      attrs.insert(std::pair<V, size_t>(it->first, attrs.size()));
      vars.push_back(it->second);
    } else {
      eqs.push_back(std::pair<size_t, size_t>(vars[it2->second], it->second));
    }
  }
  size_t tuple_size = attrs.size();
  size_t *lidx, *ridx;
  size_t len = eqs.size();
  lidx = (size_t*)malloc(len * sizeof(size_t));
  ridx = (size_t*)malloc(len * sizeof(size_t));
  size_t i, j, k;
  size_t maxpos = 0;
  for (i = 0; i < len; ++i) {
    lidx[i] = eqs[i].first;
    ridx[i] = eqs[i].second;
    maxpos = max(maxpos, ridx[i]);
  }

  temp.attributes.swap(attrs);

  Order<T> rorder(ridx, len, false);
  Relation<V, T, VLT>::sort(*smaller, rorder);

  #if REL_CONFIG == REL_OMP
  {
    vector<deque<std::pair<size_t, size_t> > > joins;
    int tid, nthreads;
    #pragma omp parallel default(shared) private(tid, nthreads, i, j)
    {
      tid = omp_get_thread_num();
      nthreads = omp_get_num_threads();
      #pragma omp single
      {
        joins.resize(nthreads);
        #pragma omp flush (joins)
      }
      pair<size_t, size_t> bounds = larger->thread_bounds(tid, nthreads);
      size_t begin;
      size_t end = bounds.second;
      for (begin = bounds.first; begin < end; ++begin) {
        const Tuple<T> &tuple = (*larger)[i];
        Tuple<T> key(maxpos+1);
        for (j = 0; j < len; ++j) {
          key[ridx[j]] = tuple[lidx[j]];
        }
        std::pair<typename Relation<V, T, VLT>::const_iterator,
                  typename Relation<V, T, VLT>::const_iterator> rng =
            equal_range(smaller->begin(), smaller->end(), key, rorder);
        size_t maxj = rng.second - smaller->begin();
        for (j = rng.first - smaller->begin(); j < maxj; ++j) {
          joins[tid].push_back(std::pair<size_t, size_t>(begin, j));
        }
      }
    }
    size_t nresults = 0;
    #pragma omp parallel for default(shared) private(i) reduction(+:nresults)
    for (i = 0; i < joins.size(); ++i) {
      nresults = nresults + joins[i].size();
    }
    temp.resize(nresults);
    size_t offset = 0;
    for (i = 0; i < joins.size(); ++i) {
      #pragma omp parallel for default(shared) private(j,k)
      for (j = 0; j < joins[i].size(); ++j) {
        const Tuple &ltuple = (*larger)[joins[i][j].first];
        const Tuple &rtuple = (*smaller)[joins[i][j].second];
        Tuple tuple(tuple_size);
        for (k = 0; k < tuple_size; ++k) {
          tuple[k] = k < var_boundary ? ltuple[vars[k]] : rtuple[vars[k]];
        }
        temp[offset + j].absorb(tuple);
      }
      offset += joins[i].size();
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    vector<vector<std::pair<size_t, size_t> > > joins;
    int sid, nstreams, resize_lock;
    purge(&resize_lock);
    size_t numcopies = larger->size() / smaller->size();
    size_t left_zeros = MTA_BIT_LEFT_ZEROS(numcopies);
    numcopies = (1 << (sizeof(size_t) - left_zeros));
    numcopies = min(numcopies, 512);
    Relation<V, T, VLT> *copies = new Relation<V, T, VLT>[numcopies];
    #pragma mta assert parallel
    for (i = 0; i < numcopies; ++i) {
      copies[i] = (*smaller);
    }
    #pragma mta for all streams sid of nstreams
    {
      if (sid == 0) {
        joins.resize(nstreams);
        writeef(&resize_lock, 0);
      }
      pair<size_t, size_t> bounds = larger->thread_bounds(sid, nstream);
      size_t begin = bounds.first;
      size_t end = bound.second;
      if (sid != 0) {
        readff(&resize_lock);
      }
      #pragma mta loop serial
      for (; begin < end; ++begin) {
        size_t j;
        const Tuple<T> &tuple = (*larger)[i];
        Tuple<T> key(maxpos+1);
        for (j = 0; j < len; ++j) {
          key[ridx[j]] = tuple[lidx[j]];
        }
        int pick = MTA_CLOCK(0) & (numcopies-1);
        std::pair<typename Relation<V, T, VLT>::const_iterator,
                  typename Relation<V, T, VLT>::const_iterator> rng =
            equal_range(copies[pick]->begin(), copies[pick]->end(), key, rorder);
        size_t maxj = rng.second - copies[pick]->begin();
        for (j = rng.first - copies[pick]->begin(); j < maxj; ++j) {
          joins[sid].push_back(std::pair<size_t, size_t>(begin, j));
        }
      }
    }
    size_t *offsets = (size_t*)malloc(joins.size() * sizeof(size_t));
    offsets[0] = 0;
    #pragma mta assert parallel
    for (i = 1; i < joins.size(); ++i) {
      offsets[i] = joins[i-1].size();
    }
    #pragma mta loop recurrence
    for (i = 2; i < joins.size(); ++i) {
      offsets[i] += offsets[i-1];
    }
    size_t nresults = offsets[joins.size()-1] + joins[joins.size()-1];
    temp.resize(nresults);
    std::pair<size_t, size_t> *joins_array = new std::pair<size_t, size_t>[nresults];
    #pragma mta assert parallel
    for (i = 0; i < joins.size(); ++i) {
      #pragma mta assert parallel
      for (j = 0; j < joins[i].size(); ++i) {
        joins_array[offsets[i]+j] = joins[i][j];
      }
    }
    #pragma mta assert parallel
    for (i = 0; i < nresults; ++i) {
      Tuple ltuple = (*larger)[joins_array[i].first];
      Tuple rtuple = copies[MTA_CLOCK(0) & (numcopies-1)][joins_array[i].second];
      Tuple &new_tuple = temp[i];
      new_tuple.resize(tuple_size);
      #pragma mta assert parallel
      for (k = 0; k < tuple_size; ++k) {
        new_tuple[k] = k < var_boundary ? ltuple[vars[k]] : rtuple[vars[k]];
      }
    }
    delete[] copies;
    free(offsets);
    free(joins_array);
  }
  #else
  {
    for (i = 0; i < larger->size(); ++i) {
      const Tuple<T> &tuple = (*larger)[i];
      Tuple<T> key(maxpos+1);
      for (j = 0; j < len; ++j) {
        key[ridx[j]] = tuple[lidx[j]];
      }
      std::pair<typename Relation<V, T, VLT>::const_iterator,
                typename Relation<V, T, VLT>::const_iterator> rng =
          equal_range(smaller->begin(), smaller->end(), key, rorder);
      size_t maxj = rng.second - smaller->begin();
      for (j = rng.first - smaller->begin(); j < maxj; ++j) {
        if (temp.size() >= temp.capacity()) {
          temp.reserve(temp.size() + larger->size());
        }
        Tuple<T> jointuple(tuple_size);
        Tuple<T> &tuple2 = (*smaller)[j];
        for (k = 0; k < tuple_size; ++k) {
          jointuple[k] = k < var_boundary ? tuple[vars[k]] : tuple2[vars[k]];
        }
        temp.add(jointuple);
      }
    }
  }
  #endif

  free(lidx);
  free(ridx);
  results.swap(temp);
}

}
