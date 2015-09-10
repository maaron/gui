#pragma once

#include <boost/iterator/iterator_facade.hpp>
#include <deque>

namespace ui
{
    template <typename T>
    struct tree_traits
    {
        typedef typename T::value_type value_type;
        typedef typename T::container container;

        value_type& value_of(T& t) { return t.value(); }
        container& children_of(T& t) { return t.children(); }
    };

    template <typename Tree>
    struct depth_first_tree_iterator
        : boost::iterator_facade<
        depth_first_tree_iterator<Tree>,
        typename Tree::value_type,
        boost::forward_traversal_tag>
    {
        typedef typename Tree::container container;
        typedef typename container::iterator child_iterator;
        typedef typename Tree::value_type value_type;

        struct range
        {
            child_iterator begin;
            child_iterator end;

            range(container& c) : begin(c.begin()), end(c.end()) {}

            bool is_end() { return begin == end; }

            void increment() { begin++; }
        };

        Tree* root;
        std::deque<range> path;

        depth_first_tree_iterator(Tree* t) : root(t) {}

        value_type& dereference() const
        {
            if (path.empty()) return root->value;
            else return tree_traits<Tree>::value_of(*path.back().begin);
        }

        void increment()
        {
            if (path.empty())
            {
                path.push_back(range(
                    tree_traits<Tree>::children_of(root->children())));
            }
            else
            {
                path.back().increment();

                if (path.back().is_end())
                {
                    path.pop_back();
                    if (path.empty()) root = nullptr;
                    else path.back().increment();
                }
            }
        }

        bool equal(depth_first_tree_iterator const& other)
        {
            return *this = other;
        }
    };

    template <typename Tree>
    struct depth_first_tree_view
    {
        typedef depth_first_tree_iterator<Tree> iterator;

        Tree& t;

        depth_first_tree_view(Tree& t) : t(t) {}

        iterator begin() { return iterator(&t); }
        iterator end() { return iterator(nullptr); }
    };
}