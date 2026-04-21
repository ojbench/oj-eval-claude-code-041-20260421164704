#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

#include <functional>
#include <cstddef>
#include "utility.hpp"

namespace sjtu {

template<
	class Key,
	class T,
	class Compare = std::less<Key>
> class map {
public:
	/**
	 * the tree node
	 */
	class value_type : public pair<const Key, T> {
    public:
        using pair<const Key, T>::pair;
    };

    struct Node {
        value_type *data;
        Node *left, *right, *parent;
        int height;

        Node(const value_type &v) : left(nullptr), right(nullptr), parent(nullptr), height(1) {
            data = reinterpret_cast<value_type*>(new char[sizeof(value_type)]);
            new (data) value_type(v);
        }
        Node(value_type &&v) : left(nullptr), right(nullptr), parent(nullptr), height(1) {
            data = reinterpret_cast<value_type*>(new char[sizeof(value_type)]);
            new (data) value_type(std::move(v));
        }
        ~Node() {
            if (data) {
                data->~value_type();
                delete[] reinterpret_cast<char*>(data);
            }
        }
    };

    Node *root;
    size_t _size;
    Compare comp;

    int getHeight(Node *t) const { return t ? t->height : 0; }
    void updateHeight(Node *t) {
        if (t) {
            int lh = getHeight(t->left);
            int rh = getHeight(t->right);
            t->height = (lh > rh ? lh : rh) + 1;
        }
    }

    void rotateLeft(Node *&t) {
        Node *r = t->right;
        t->right = r->left;
        if (r->left) r->left->parent = t;
        r->parent = t->parent;
        if (t->parent) {
            if (t->parent->left == t) t->parent->left = r;
            else t->parent->right = r;
        } else {
            root = r;
        }
        r->left = t;
        t->parent = r;
        updateHeight(t);
        updateHeight(r);
        t = r;
    }

    void rotateRight(Node *&t) {
        Node *l = t->left;
        t->left = l->right;
        if (l->right) l->right->parent = t;
        l->parent = t->parent;
        if (t->parent) {
            if (t->parent->left == t) t->parent->left = l;
            else t->parent->right = l;
        } else {
            root = l;
        }
        l->right = t;
        t->parent = l;
        updateHeight(t);
        updateHeight(l);
        t = l;
    }

    void balance(Node *&t) {
        if (!t) return;
        int bf = getHeight(t->left) - getHeight(t->right);
        if (bf > 1) {
            if (getHeight(t->left->left) >= getHeight(t->left->right)) {
                rotateRight(t);
            } else {
                rotateLeft(t->left);
                rotateRight(t);
            }
        } else if (bf < -1) {
            if (getHeight(t->right->right) >= getHeight(t->right->left)) {
                rotateLeft(t);
            } else {
                rotateRight(t->right);
                rotateLeft(t);
            }
        } else {
            updateHeight(t);
        }
    }

    Node* findNode(const Key &key) const {
        Node *curr = root;
        while (curr) {
            if (comp(key, curr->data->first)) curr = curr->left;
            else if (comp(curr->data->first, key)) curr = curr->right;
            else return curr;
        }
        return nullptr;
    }

    void clear(Node *&t) {
        if (!t) return;
        clear(t->left);
        clear(t->right);
        delete t;
        t = nullptr;
    }

    Node* copy(Node *t, Node *p) {
        if (!t) return nullptr;
        Node *newNode = new Node(*(t->data));
        newNode->parent = p;
        newNode->height = t->height;
        newNode->left = copy(t->left, newNode);
        newNode->right = copy(t->right, newNode);
        return newNode;
    }

	/**
	 * see BidirectionalIterator at [http://en.cppreference.com/w/cpp/concept/BidirectionalIterator]
	 */
	class const_iterator;
	class iterator {
	private:
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
        friend class map;
        Node *node;
        const map *m;

	public:
		iterator() : node(nullptr), m(m) {}
		iterator(Node *n, const map *mp) : node(n), m(mp) {}
		iterator(const iterator &other) : node(other.node), m(other.m) {}
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
		/**
		 * TODO ++iter
		 */
		iterator & operator++() {
            if (!node) return *this;
            if (node->right) {
                node = node->right;
                while (node->left) node = node->left;
            } else {
                Node *p = node->parent;
                while (p && node == p->right) {
                    node = p;
                    p = p->parent;
                }
                node = p;
            }
            return *this;
        }
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }
		/**
		 * TODO --iter
		 */
		iterator & operator--() {
            if (!node) { // end()
                node = m->root;
                if (!node) return *this; // map is empty
                while (node->right) node = node->right;
            } else {
                if (node->left) {
                    node = node->left;
                    while (node->right) node = node->right;
                } else {
                    Node *p = node->parent;
                    while (p && node == p->left) {
                        node = p;
                        p = p->parent;
                    }
                    node = p;
                }
            }
            return *this;
        }
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const { return *(node->data); }
		bool operator==(const iterator &rhs) const { return node == rhs.node && m == rhs.m; }
		bool operator==(const const_iterator &rhs) const { return node == rhs.node && m == rhs.m; }
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
		bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }

		/**
		 * for the support of it->first.
		 * See <http://en.cppreference.com/w/cpp/language/operator_member_access#Built-in_member_access_operators>
		 */
		value_type * operator->() const noexcept { return node->data; }
	};
	class const_iterator {
		// it should has similar member method as iterator.
		//  and it should be able to construct from an iterator.
	private:
		// data members.
        friend class map;
        const Node *node;
        const map *m;
	public:
		const_iterator() : node(nullptr), m(nullptr) {}
		const_iterator(const Node *n, const map *mp) : node(n), m(mp) {}
		const_iterator(const const_iterator &other) : node(other.node), m(other.m) {}
		const_iterator(const iterator &other) : node(other.node), m(other.m) {}

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        const_iterator & operator++() {
            if (!node) return *this;
            if (node->right) {
                node = node->right;
                while (node->left) node = node->left;
            } else {
                const Node *p = node->parent;
                while (p && node == p->right) {
                    node = p;
                    p = p->parent;
                }
                node = p;
            }
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator tmp = *this;
            --(*this);
            return tmp;
        }
        const_iterator & operator--() {
            if (!node) {
                node = m->root;
                if (!node) return *this;
                while (node->right) node = node->right;
            } else {
                if (node->left) {
                    node = node->left;
                    while (node->right) node = node->right;
                } else {
                    const Node *p = node->parent;
                    while (p && node == p->left) {
                        node = p;
                        p = p->parent;
                    }
                    node = p;
                }
            }
            return *this;
        }
        const value_type & operator*() const { return *(node->data); }
        bool operator==(const iterator &rhs) const { return node == rhs.node && m == rhs.m; }
        bool operator==(const const_iterator &rhs) const { return node == rhs.node && m == rhs.m; }
        bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
        bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
        const value_type * operator->() const noexcept { return node->data; }
	};

	/**
	 * TODO two constructors
	 */
	map() : root(nullptr), _size(0) {}
	map(const map &other) : _size(other._size) {
        root = copy(other.root, nullptr);
    }
	/**
	 * TODO assignment operator
	 */
	map & operator=(const map &other) {
        if (this == &other) return *this;
        clear(root);
        _size = other._size;
        root = copy(other.root, nullptr);
        return *this;
    }
	/**
	 * TODO Destructors
	 */
	~map() { clear(root); }
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, throw index_out_of_bound
	 */
	T & at(const Key &key) {
        Node *n = findNode(key);
        return n->data->second;
    }
	const T & at(const Key &key) const {
        Node *n = findNode(key);
        return n->data->second;
    }
	/**
	 * TODO
	 * access specified element [with default construction]
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 * performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
        Node *n = findNode(key);
        if (n) return n->data->second;
        return insert(value_type(key, T())).first->second;
    }
	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const { return at(key); }
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
        Node *curr = root;
        if (!curr) return iterator(nullptr, this);
        while (curr->left) curr = curr->left;
        return iterator(curr, this);
    }
	const_iterator cbegin() const {
        Node *curr = root;
        if (!curr) return const_iterator(nullptr, this);
        while (curr->left) curr = curr->left;
        return const_iterator(curr, this);
    }
	/**
	 * return a iterator to the end
	 * in fact, it's past-the-end.
	 */
	iterator end() { return iterator(nullptr, this); }
	const_iterator cend() const { return const_iterator(nullptr, this); }
	/**
	 * checks whether the container is empty
	 */
	bool empty() const { return _size == 0; }
	/**
	 * returns the number of elements
	 */
	size_t size() const { return _size; }
	/**
	 * clears the contents
	 */
	void clear() {
        clear(root);
        _size = 0;
    }
	/**
	 * insert an element.
	 * return a pair, the first of the pair is the iterator to the new element (or the element that prevented the insertion),
	 * the second is true if value was inserted, false otherwise.
	 */
	pair<iterator, bool> insert(const value_type &value) {
        Node *n = findNode(value.first);
        if (n) return pair<iterator, bool>(iterator(n, this), false);

        root = insert(root, nullptr, value);
        _size++;
        return pair<iterator, bool>(iterator(findNode(value.first), this), true);
    }

    Node* insert(Node *t, Node *p, const value_type &v) {
        if (!t) {
            Node *newNode = new Node(v);
            newNode->parent = p;
            return newNode;
        }
        if (comp(v.first, t->data->first)) {
            t->left = insert(t->left, t, v);
        } else if (comp(t->data->first, v.first)) {
            t->right = insert(t->right, t, v);
        }
        balance(t);
        return t;
    }

	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (should be the end() element or an iterator of other container)
	 */
	void erase(iterator pos) {
        if (pos.m != this || !pos.node) return;
        root = erase(root, pos.node->data->first);
        _size--;
    }

    Node* erase(Node *t, const Key &key) {
        if (!t) return nullptr;
        if (comp(key, t->data->first)) {
            t->left = erase(t->left, key);
        } else if (comp(t->data->first, key)) {
            t->right = erase(t->right, key);
        } else {
            if (!t->left || !t->right) {
                Node *tmp = t->left ? t->left : t->right;
                if (tmp) tmp->parent = t->parent;
                delete t;
                return tmp;
            } else {
                Node *tmp = t->right;
                while (tmp->left) tmp = tmp->left;
                // Swap data
                value_type *old_data = t->data;
                t->data = reinterpret_cast<value_type*>(new char[sizeof(value_type)]);
                new (t->data) value_type(*(tmp->data));

                t->right = erase(t->right, tmp->data->first);

                old_data->~value_type();
                delete[] reinterpret_cast<char*>(old_data);
            }
        }
        balance(t);
        return t;
    }

	/**
	 * Returns the number of elements with key that compares equivalent to the specified argument,
	 * which is either 1 or 0 since this container does not allow duplicates.
	 * The default method of check the equivalence is !(comp(k1, k2) || comp(k2, k1)).
	 */
	size_t count(const Key &key) const {
        return findNode(key) ? 1 : 0;
    }
	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key. If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {
        Node *n = findNode(key);
        return n ? iterator(n, this) : end();
    }
	const_iterator find(const Key &key) const {
        Node *n = findNode(key);
        return n ? const_iterator(n, this) : cend();
    }
};

}

#endif
