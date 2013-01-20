#ifndef __REL__RELATION_H__
#define __REL__RELATION_H__

#ifdef REL_SEQ
#undef REL_SEQ
#endif

#ifdef REL_OMP
#undef REL_OMP
#endif

#ifdef REL_MTA
#undef REL_MTA
#endif

#ifdef REL_MAX
#undef REL_MAX
#endif

#define REL_SEQ 0
#define REL_OMP 1
#define REL_MTA 2
#define REL_MAX 2

#ifndef REL_CONFIG
#define REL_CONFIG REL_SEQ
#warning "REL_CONFIG was not set.  Defaulting to REL_SEQ."
#elif REL_CONFIG > REL_MAX
#error "Unrecognized value fo REL_CONFIG."
#endif

#ifdef REL_CONTAINER
#if REL_CONFIG != REL_SEQ
#warning "Be sure that REL_CONTAINER is random accessible."
#endif
#endif

#if REL_CONFIG == REL_OMP
#include <omp.h>
#endif

#if REL_CONFIG == REL_MTA
#ifndef USE_MTGL
#warning "You didn't say whether you wanted to use MTGL using -DMTGL=[0|1].  Defaulting to 0, but I definitely recommend using MTGL."
#define USE_MTGL 0
#endif
#if USE_MTGL
#include "mtgl/merge_sort.hpp"
#endif
#endif

#include <vector>

namespace rel {

using namespace std;

template<typename T>
class Tuple {
private:
#ifdef TUPLE_SIZE
  T data[TUPLE_SIZE];
  size_t length;
  //void safety_check(const size_t sz) const;
#else
  vector<T> data;
#endif
public:
#ifdef TUPLE_SIZE
  typedef T* iterator;
  typedef const T* const_iterator;
#else
  typedef typename vector<T>::iterator iterator;
  typedef typename vector<T>::const_iterator const_iterator;
#endif
  Tuple();
  explicit Tuple(const size_t len);
  Tuple(const size_t len, const T &val);
  Tuple(const Tuple<T> &copy);
  virtual ~Tuple();
  Tuple<T> &operator=(const Tuple<T> &rhs);
  void swap(Tuple<T> &t);
  void absorb(Tuple<T> &t);
  T &operator[](const size_t i);
  const T &operator[](const size_t i) const;
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  size_t size() const;
  void resize(const size_t sz);
  static bool compat(const Tuple<T> &lhs, const Tuple<T> &rhs,
                     const size_t *lidx, const size_t *ridx,
                     const size_t len);
};

template<typename T, typename TLT=less<T> >
class Order {
private:
#ifdef TUPLE_SIZE
  size_t order[TUPLE_SIZE];
  size_t length;
  //void safety_check(const size_t sz) const;
#else
  vector<size_t> order;
#endif
  bool total;
  TLT less_than;
public:
  Order(); // total ordering
  Order(TLT less_than); // total ordering
  Order(const size_t *indexes, const size_t len, const bool total);
  Order(const size_t *indexes, const size_t len, const bool total,
        TLT less_than);
  Order(const Order<T, TLT> &copy);
  virtual ~Order();
  Order<T, TLT> &operator=(const Order<T, TLT> &rhs);
  void swap(Order<T, TLT> &o);
  size_t operator[](const size_t i) const;
  size_t size() const;
  bool is_total() const;
  bool operator()(const Tuple<T> &lhs, const Tuple<T> &rhs) const;
};

template<typename V, typename T, typename VLT=less<V> >
class Relation {
private:
#ifdef REL_CONTAINER
  REL_CONTAINER<Tuple<T> > tuples;
#if REL_CONFIG == REL_MTA
  uint64_t push_back_lock;
#endif
#else
  Tuple<T> *tuples;
  size_t length;
  size_t capacity;
#endif
#if REL_CONFIG == REL_OMP
  omp_lock_t add_lock;
#endif
  map<V, size_t, VLT> attributes;
public:
#ifdef REL_CONTAINER
  typedef typename REL_CONTAINER<Tuple<T> >::iterator iterator;
  typedef typename REL_CONTAINER<Tuple<T> >::const_iterator const_iterator;
#else
  typedef Tuple<T>* iterator;
  typedef const Tuple<T>* const_iterator;
#endif
  Relation();
  Relation(VLT var_less_than);
  Relation(const Relation<V, T, VLT> &copy);
  virtual ~Relation();
  Relation<V, T, VLT> &operator=(const Relation<V, T, VLT> &rhs);
  void swap(Relation<V, T, VLT> &r);
  Tuple<T> &operator[](const size_t i);
  const Tuple<T> &operator[](const size_t i) const;
  bool empty() const;
  size_t size() const;
  size_t capacity() const;
  void resize(const size_t sz);
  void reserve(const size_t sz);
  void clear();
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  const map<V, size_t, VLT> &attrs() const;
  void add(const Tuple<T> &tuple);
  void meld(const Relation<V, T, VLT> &r);
  void meld(const Relation<V, T, VLT> &r,
            Relation<V, T, VLT> &results) const;
  size_t index(const V *vars, const T *values, const size_t len1,
               const V *vars1, const V *vars2, const size_t len2,
               const bool sign, size_t *&idx) const;
  bool select(const V *vars, const T *values, const size_t len1,
              const V *vars1, const V *vars2, const size_t len2,
              const bool sign);
  bool select(const V *vars, const T *values, const size_t len1,
              const V *vars1, const V *vars2, const size_t len2,
              const bool sign, Relation<V, T, VLT> &results) const;
  bool project(const V *vars, const size_t len);
  bool project(const V *vars, const size_t len,
               Relation<V, T, VLT> &results) const;
  void name(const map<V, size_t, VLT> &namings);
  bool rename(const map<V, V, VLT> &renamings);
  bool rename(const map<V, V, VLT> &renamings,
              Relation<V, T, VLT> &results) const;
  std::pair<size_t, size_t> thread_bounds(const size_t id, const size_t n) const;

  template<typename TLT>
  static Order<T, TLT> make_order(const Relation<V, T, VLT> &r,
                                  const V *vars, const size_t len,
                                  TLT t_less_than);

  template<typename TLT>
  static void sort(Relation<V, T, VLT> &r, const Order<T, TLT> &order);

  template<typename TLT>
  static void uniq(Relation<V, T, VLT> &r, const Order<T, TLT> &order);

  static void nested_loop_join(const Relation<V, T, VLT> &lhs,
                               const Relation<V, T, VLT> &rhs,
                               Relation<V, T, VLT> &results);
  static void index_join(Relation<V, T, VLT> &lhs,
                         Relation<V, T, VLT> &rhs,
                         Relation<V, T, VLT> &results);
// TODO maybe one day
//  static void sort_merge_join(const Relation<V, T, VLT> &lhs,
//                              const Relation<V, T, VLT> &rhs,
//                              Relation<V, T, VLT> &results);
//
//  static void one_sided_index_join(const Relation<V, T, VLT> &lhs,
//                                   const Relation<V, T, VLT> &rhs,
//                                   Relation<V, T, VLT> &results);
//
//  static void two_sided_index_join(const Relation<V, T, VLT> &lhs,
//                                   const Relation<V, T, VLT> &rhs,
//                                   Relation<V, T, VLT> &results);
};

}

#include "rel/Relation-inl.h"

#endif /* __REL__RELATION_H__ */
