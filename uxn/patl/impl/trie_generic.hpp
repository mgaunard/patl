/*-
 | This source code is part of PATL (Practical Algorithm Template Library)
 | Released under the BSD License (http://www.opensource.org/licenses/bsd-license.php)
 | Copyright (c) 2005, 2007..2009, Roman S. Klyujkov (uxnuxn AT gmail.com)
-*/
#ifndef PATL_IMPL_TRIE_GENERIC_HPP
#define PATL_IMPL_TRIE_GENERIC_HPP

#include <algorithm>
#include <functional>
#include "node.hpp"
#include "core_algorithm.hpp"
#include "assoc_generic.hpp"
#include "core_prefix.hpp"

namespace uxn
{
namespace patl
{
namespace impl
{

template <typename T>
class node_gen
    : public node_generic<node_gen<T> >
{
    /// to avoid warning C4512
    node_gen &operator=(const node_gen&);

public:
    explicit node_gen(const typename T::value_type &value)
        : value_(value)
    {
    }

    /// return value stored in node
    typename T::value_type &get_value()
    {
        return value_;
    }
    const typename T::value_type &get_value() const
    {
        return value_;
    }

    const typename T::key_type &get_key() const
    {
        return T::ref_key(value_);
    }

private:
    typename T::value_type value_;
};

template <typename T, typename Container>
class algorithm_gen
    : public core_algorithm_generic<T, algorithm_gen<T, Container>, Container>
{
    using this_t = algorithm_gen<T, Container>;
    using super = core_algorithm_generic<T, this_t, Container>;

public:
    using cont_type = Container;
    using node_type = typename T::node_type;
    using key_type = typename T::key_type;
    using const_key_reference = typename T::const_key_reference;
    using value_type = typename T::value_type;

    explicit algorithm_gen(const cont_type *cont = 0)
        : super(cont)
    {
    }

    algorithm_gen(const cont_type *cont, const node_type *q, word_t qid)
        : super(cont, q, qid)
    {
    }

    algorithm_gen(const cont_type *cont, word_t qq)
        : super(cont, qq)
    {
    }

    algorithm_gen construct(const node_type *q, word_t qid) const
    {
        return algorithm_gen(0, q, qid);
    }

    /// return value stored in algorithm
    value_type &get_value()
    {
        return this->get_p()->get_value();
    }
    const value_type &get_value() const
    {
        return this->get_p()->get_value();
    }

    value_type &get_value(node_type *p)
    {
        return p->get_value();
    }
    const value_type &get_value(const node_type *p) const
    {
        return p->get_value();
    }

    const_key_reference get_key() const
    {
        return this->get_p()->get_key();
    }

    const_key_reference get_key(const node_type *p) const
    {
        return p->get_key();
    }
};

template <typename T, template <typename> class Node>
struct algorithm_gen_traits
    : public T
{
    using node_type = Node<T>;

    using key_type = typename T::key_type;
    using const_key_reference = const key_type&;
};

template <
    typename T,
    template <typename> class Node,
    template <typename, typename> class Algorithm,
    template <typename, typename> class Prefix,
    typename Container>
struct trie_generic_traits
    : public algorithm_gen_traits<T, Node>
{
    using algorithm = Algorithm<algorithm_gen_traits<T, Node>, Container>;
    using prefix = Prefix<Container, Node<T> >;
};

#define SELF static_cast<this_t*>(this)
#define CSELF static_cast<const this_t*>(this)

template <typename T, word_t N = 0>
class trie_generic
    : public trie_generic<T, 0>
{
    using this_t = trie_generic<T, N>;
    using traits = trie_generic_traits<T, node_gen, algorithm_gen, core_prefix_generic, this_t>;
    using super = assoc_generic<trie_generic<T, N>, traits, N>;

protected:
    using node_type = typename traits::node_type;
    using algorithm = typename traits::algorithm;

public:
    using key_type = typename super::key_type;
    using value_type = typename super::value_type;
    using bit_compare = typename super::bit_compare;
    using allocator_type = typename super::allocator_type;
    using size_type = typename super::size_type;
    using const_iterator = typename super::const_iterator;
    using iterator = typename super::iterator;
    using vertex = typename super::vertex;
    using preorder_iterator = typename super::preorder_iterator;
    using postorder_iterator = typename super::postorder_iterator;

    using const_key_reference = typename algorithm::const_key_reference;

    static const word_t N_ = N;

    trie_generic(const bit_compare &bit_comp, const allocator_type &alloc)
        : super(bit_comp, alloc)
    {
    }

    trie_generic(const this_t &b)
        : super(b)
    {
    }

    trie_generic(
        const value_type *first,
        const value_type *last,
        const bit_compare &bit_comp,
        const allocator_type &alloc)
        : super(first, last, bit_comp, alloc)
    {
    }

private:
    using iter_bool_pair = std::pair<iterator, bool>;

public:
    iter_bool_pair insert(const value_type &val)
    {
        // if trie is empty then add root
        if (!this->root_)
            return iter_bool_pair(iterator(vertex(add_root(val))), true);
        algorithm pal(CSELF, this->root_, 0);
        // find a number of first mismatching bit
        const word_t l = pal.mismatch(T::ref_key(val));
        // if this number end at infinity
        if (~word_t(0) == l)
            // then this key already in trie
            return iter_bool_pair(iterator(vertex(pal)), false);
        // ����������� �� ���������� ������� �� N
        algorithm pal2(pal);
        word_t diff = 0;
        while (pal2.get_p()->get_skip() == pal2.get_q()->get_skip())
        {
            pal2.ascend();
            ++diff;
        }
        word_t *sh = get_shortcut(pal2.compact());
        if (sh)
        {
            // �������� �������
        }
        else if (diff >= N / 2)
        {
            // ������� �������
            create_shortcut(pal2.compact());
            // �������� �� ��������� levelorder_iterator'�� � ���������������� �������
            vertex vtx(pal2);
            const word_t next = max0(vtx.skip()) + N;
            /*levelorder_iterator
                lit(vtx.levelorder_begin(next)),
                lit_end(vtx.levelorder_end(next));*/
                //for (; lit != lit_end; ++lit)
        }
        // add new node for value with unique key
        return iter_bool_pair(iterator(vertex(add(val, pal, l))), true);
    }

    /// just for backward compatibility with std assoc containers
    iterator insert(iterator, const value_type &val)
    {
        return insert(val).first;
    }

    /// template of insert range
    template <typename Iter>
    void insert(Iter first, Iter last)
    {
        for (; first != last; ++first)
            insert(*first);
    }
};

template <typename T>
class trie_generic<T, 0>
    : public assoc_generic<
        trie_generic<T, T::N_>,
        trie_generic_traits<T, node_gen, algorithm_gen, core_prefix_generic, trie_generic<T, T::N_> >,
        T::N_>
{
    using this_t = trie_generic<T, T::N_>;
    using traits = trie_generic_traits<T, node_gen, algorithm_gen, core_prefix_generic, this_t>;
    using super = assoc_generic<trie_generic<T, T::N_>, traits, T::N_>;

protected:
    using node_type = typename traits::node_type;
    using algorithm = typename traits::algorithm;

public:
    using key_type = typename super::key_type;
    using value_type = typename super::value_type;
    using bit_compare = typename super::bit_compare;
    using allocator_type = typename super::allocator_type;
    using size_type = typename super::size_type;
    using const_iterator = typename super::const_iterator;
    using iterator = typename super::iterator;
    using const_vertex = typename super::const_vertex;
    using vertex = typename super::vertex;
    using const_preorder_iterator = typename super::const_preorder_iterator;
    using const_postorder_iterator = typename super::const_postorder_iterator;
    using preorder_iterator = typename super::preorder_iterator;
    using postorder_iterator = typename super::postorder_iterator;

    using const_key_reference = typename algorithm::const_key_reference;

    static const word_t N_ = T::N_;

    trie_generic(const bit_compare &bit_comp, const allocator_type &alloc)
        : super(bit_comp)
        , alloc_(alloc)
        , size_(0)
    {
    }

    trie_generic(const this_t &b)
        : super(b.bit_comp_)
        , alloc_(b.alloc_)
        , size_(0)
    {
        insert(b.begin(), b.end());
    }

    trie_generic(
        const value_type *first,
        const value_type *last,
        const bit_compare &bit_comp,
        const allocator_type &alloc)
        : super(bit_comp)
        , alloc_(alloc)
        , size_(0)
    {
        insert(first, last);
    }

    ~trie_generic()
    {
        del_tree(this->root_);
    }

    this_t &operator=(const this_t &b)
    {
        del_tree(this->root_);
        this->bit_comp_ = b.bit_comp_;
        alloc_ = b.alloc_;
        this->root_ = 0;
        size_ = 0;
        insert(b.begin(), b.end());
        return *this;
    }

    void swap(this_t &b)
    {
        std::swap(this->bit_comp_, b.bit_comp_);
        std::swap(alloc_, b.alloc_);
        std::swap(this->root_, b.root_);
        std::swap(size_, b.size_);
    }

    void clear()
    {
        del_tree(this->root_);
        this->root_ = 0;
    }

    size_type size() const
    {
        return size_;
    }

    size_type max_size() const
    {
        return alloc_.max_size();
    }

    allocator_type get_allocator() const
    {
        return alloc_;
    }

private:
    using iter_bool_pair = std::pair<iterator, bool>;

public:
    iter_bool_pair insert(const value_type &val)
    {
        // if trie is empty then add root
        if (!this->root_)
            return iter_bool_pair(iterator(vertex(add_root(val))), true);
        algorithm pal(CSELF, this->root_, 0);
        // find a number of first mismatching bit
        const word_t l = pal.mismatch(T::ref_key(val));
        // if this number end at infinity
        if (~word_t(0) == l)
            // then this key already in trie
            return iter_bool_pair(iterator(vertex(pal)), false);
        // add new node for value with unique key
        return iter_bool_pair(iterator(vertex(add(val, pal, l))), true);
    }

    /// hinted insertion method
    iterator insert(iterator hint, const value_type &val)
    {
        // if trie is empty then add root
        if (!this->root_)
            return iterator(vertex(add_root(val)));
        algorithm &pal = hint;
        const key_type &key = T::ref_key(val);
        const word_t skip = this->bit_comp_.bit_mismatch(key, pal.get_key());
        if (~word_t(0) == skip)
            return hint;
        pal.ascend_less(skip);
        // find a number of first mismatching bit
        {
            const word_t len = this->bit_comp_.bit_length(key);
            if (len == ~word_t(0))
                pal.run(key);
            else
                pal.run(key, len);
        }
        const word_t l = this->bit_comp_.bit_mismatch(key, pal.get_key(), skip);
        // if this number end at infinity
        if (~word_t(0) == l)
            // then this key already in trie
            return hint;
        else
        {
            pal.ascend(l);
            // add new node for value with unique key
            return iterator(vertex(add(val, pal, l)));
        }
    }

    /// template of insert range
    template <typename Iter>
    void insert(Iter first, Iter last)
    {
        for (; first != last; ++first)
            insert(*first);
    }

    /// merge two identical tries (instances of bit_compare must be equal)
    template <typename Handler>
    void merge(const_iterator first, const_iterator last, Handler handler)
    {
        // if trie is empty it is necessary to insert first element
        if (!this->root_ && first != last)
        {
            insert(*first);
            ++first;
        }
        algorithm pal_cur(this, this->root_, 0);
        const_vertex vtx(first);
        word_t skip = 0;
        while (vtx != last)
        {
            pal_cur.ascend_less(skip);
            const word_t l = pal_cur.mismatch(vtx.get_key());
            if (~word_t(0) == l)
                // identical keys found - handler must be applied
                handler(iterator(vertex(pal_cur)), const_iterator(vtx));
            else
                add(vtx.get_value(), pal_cur, l);
            // move to the next
            if (vtx.get_qid())
            {
                const node_type *cur = vtx.get_q();
                for (; cur->get_parent_id(); cur = cur->get_parent()) ;
                vtx = vertex(this, cur->get_parent(), 1);
            }
            else
                vtx.toggle();
            skip = vtx.get_q()->get_skip();
            vtx.template descend<0>();
        }
    }

    void merge(const_iterator first, const_iterator last)
    {
        merge(first, last, [](auto it, auto cit){ });
    }

    /// serialize trie
    template <typename OutIter, typename ShowValue>
    void show(OutIter oit, ShowValue show_val) const
    {
        if (!this->root_)
            return;
        const_vertex vtx(this->root());
        show_val(oit, vtx.parent_key());
        word_t skip_ins = 0;
        for (const_preorder_iterator pit = vtx.preorder_begin()
            ; pit != vtx.preorder_end()
            ; ++pit)
        {
            if (pit->get_qtag())
                ++skip_ins;
            else
            {
                if (skip_ins)
                {
                    *oit++ = highest_bit | skip_ins;
                    skip_ins = 0;
                }
                *oit++ = pit->next_skip();
                show_val(oit, pit->get_key());
            }
        }
    }

    template <typename OutIter>
    void show(OutIter oit) const
    {
        show(oit, [](OutIter &oit, const auto &val){ *oit++ = reinterpret_cast<word_t>(val); });
    }

    /// deserialize trie
    template <typename InIter, typename ReadValue>
    void read(InIter iit, InIter iit_end, ReadValue read_val)
    {
        clear();
        if (iit == iit_end)
            return;
        preorder_iterator pit(vertex(add_root(read_val(iit))));
        while (iit != iit_end)
        {
            const word_t skip = *iit++;
            if (skip & highest_bit)
                std::advance(pit, bits_but_highest(skip));
            else
            {
                static_cast<algorithm&>(*pit) = add(read_val(iit), *pit, skip);
                if (pit->get_qid())
                    pit->toggle();
            }
        }
    }

    template <typename InIter>
    void read(InIter iit, InIter iit_end)
    {
        read(iit, iit_end, [](InIter & iit){ return reinterpret_cast<value_type>(*iit++); });
    }

    void erase(iterator del_it)
    {
        erase_node(del_it);
    }

    /// erase all values with specified prefix
    size_type erase(const key_type &key, word_t prefixLen = ~word_t(0))
    {
        if (this->root_)
        {
            algorithm pal(this, this->root_, 0);
            if (pal.mismatch(key, prefixLen) >= prefixLen)
            {
                const size_type pastSize = size();
                // if erase entire tree
                if (pal.get_q() == this->root_)
                {
                    clear();
                    return pastSize;
                }
                erase_subtree(pal);
                return pastSize - size();
            }
        }
        return 0;
    }

private:
    void del_tree(node_type *node)
    {
        if (node)
        {
            vertex vtx(this->root());
            for (postorder_iterator pit = vtx.postorder_begin()
                ; pit != vtx.postorder_end()
                ; ++pit)
            {
                if (!pit->get_qtag())
                    del_node(pit->get_p());
            }
            del_node(node);
        }
    }

    void del_node(node_type *node)
    {
        alloc_.destroy(node);
        alloc_.deallocate(node, 1);
        --size_;
    }

    void erase_subtree(algorithm pal)
    {
        // if erase subtree
        if (!pal.get_qtag())
        {
            // deallocate subtree
            del_tree(pal.get_p());
            // need to find the node with key whose prefix
            // we erase among nodes from pal.get_q() up to root_
            pal.get_q()->set_xlinktag(pal.get_qid(),
                pal.get_subtree_node(), 1);
        }
        erase_node(pal);
    }

    void erase_node(algorithm pal)
    {
        node_type *p = pal.erase();
        // special case: del root
        if (p == this->root_)
            this->root_ = pal.get_q() == p ? 0 : pal.get_q();
        // sic transit gloria mundi
        del_node(p);
    }

protected:
    /// create root in empty trie
    algorithm add_root(const value_type &val)
    {
        this->root_ = alloc_.allocate(1);
        alloc_.construct(this->root_, node_type(val));
        this->root_->init_root();
        size_ = 1;
        return algorithm(CSELF, this->root_, 0);
    }

    /// add new node with unique key
    algorithm add(const value_type &val, algorithm &pal, word_t prefixLen)
    {
        // brave new node
        node_type *r = alloc_.allocate(1);
        alloc_.construct(r, node_type(val));
        // add new node into trie
        const word_t b = this->bit_comp_.get_bit(T::ref_key(val), prefixLen);
        pal.add(r, b, prefixLen);
        ++size_;
        return algorithm(CSELF, r, b);
    }

    typename allocator_type::template rebind<node_type>::other alloc_;
    size_type size_;
};

#undef SELF
#undef CSELF

template <typename T, word_t N>
inline bool operator==(const trie_generic<T, N> &a, const trie_generic<T, N> &b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

template <typename T, word_t N>
inline bool operator!=(const trie_generic<T, N> &a, const trie_generic<T, N> &b)
{
    return !(a == b);
}

} // namespace impl
} // namespace patl
} // namespace uxn

#endif
