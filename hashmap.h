#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <list>
#include <stdexcept>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
public:
    typedef std::_List_iterator<std::pair<const KeyType, ValueType> > iterator;
    typedef std::_List_const_iterator<std::pair<const KeyType, ValueType> > const_iterator;

private:
    const size_t base_number_of_buckets = 10;
    size_t number_of_buckets = base_number_of_buckets;
    const size_t fullness_coef = 2;
    const size_t resize_coef = 2;
    size_t sz = 0;

    std::list<std::pair<const KeyType, ValueType> > data;
    std::vector<std::vector<std::pair<KeyType, iterator> > > buckets;
    Hash hasher;

    size_t get_bucket(const KeyType& key) const {
        return hasher(key) % number_of_buckets;
    }

    std::pair<size_t, size_t> find_in_buckets(const KeyType& key) const {
        size_t bucket = get_bucket(key);
        for (size_t i = 0; i < buckets[bucket].size(); i++) {
            if (buckets[bucket][i].first == key) {
                return std::make_pair(bucket, i);
            }
        }
        return std::make_pair(bucket, buckets[bucket].size());
    }

    void expand() {
        for (auto elem : data) {
            size_t bucket = get_bucket(elem.first);
            buckets[bucket].clear();
        }
        number_of_buckets *= resize_coef;
        buckets.resize(number_of_buckets);
        for (iterator iter = data.begin(); iter != data.end(); iter++) {
            KeyType key = (*iter).first;
            size_t bucket = get_bucket(key);
            buckets[bucket].emplace_back(key, iter);
        }
    }

    void shrink() {
        for (auto elem : data) {
            size_t bucket = get_bucket(elem.first);
            buckets[bucket].clear();
        }
        number_of_buckets /= resize_coef;
        buckets.resize(number_of_buckets);
        buckets.shrink_to_fit();
        for (iterator iter = data.begin(); iter != data.end(); iter++) {
            KeyType key = (*iter).first;
            size_t bucket = get_bucket(key);
            buckets[bucket].emplace_back(key, iter);
        }
    }

public:

    HashMap(Hash hasher = Hash()): buckets(number_of_buckets), hasher(hasher) {}

    template<typename IteratorType>
    HashMap(IteratorType begin_iter, IteratorType end_iter, Hash hasher = Hash()): buckets(number_of_buckets), hasher(hasher) {
        for (IteratorType iter = begin_iter; iter != end_iter; iter++) {
            insert(*iter);
        }
    }

    HashMap(const std::initializer_list<std::pair<KeyType, ValueType> >& elements, Hash hasher = Hash()): buckets(number_of_buckets), hasher(hasher) {
        for (auto elem : elements) {
            insert(elem);
        }
    }

    HashMap& operator=(const HashMap& other) {
    	if (this == &other)
            return (*this);
        clear();
        for (auto elem : other) {
            insert(elem);
        }
        return (*this);
    }

    size_t size() const {
        return sz;
    }

    bool empty() const {
        return data.empty();
    }

    Hash hash_function() const {
        return hasher;
    }

    std::pair<iterator, bool> insert(const std::pair<KeyType, ValueType>& elem) {
        iterator iter = find(elem.first);
        if (iter != end()) {
            return std::make_pair(iter, false);
        }
        size_t bucket = get_bucket(elem.first);
        data.push_back(elem);
        buckets[bucket].emplace_back(elem.first, --data.end());
        sz++;
        if (size() * fullness_coef > number_of_buckets) {
            expand();
            return std::make_pair(find(elem.first), true);
        }
        return std::make_pair(--data.end(), true);
    }

    size_t erase(const KeyType& key) {
        std::pair<size_t, size_t> res = find_in_buckets(key);
        size_t bucket = res.first;
        size_t i = res.second;
        if (i == buckets[bucket].size()) {
            return 0;
        }
        iterator pos_in_data = buckets[bucket][i].second;
        data.erase(pos_in_data);
        buckets[bucket][i] = buckets[bucket].back();
        buckets[bucket].pop_back();
        sz--;
        if (number_of_buckets > base_number_of_buckets && size() * fullness_coef <= number_of_buckets / resize_coef) {
            shrink();
        }
        return 1;
    }

    iterator begin() {
        return data.begin();
    }

    const_iterator begin() const {
        return data.begin();
    }

    iterator end() {
        return data.end();
    }

    const_iterator end() const {
        return data.end();
    }

    iterator find(const KeyType& key) {
        std::pair<size_t, size_t> res = find_in_buckets(key);
        size_t bucket = res.first;
        size_t i = res.second;
        if (i == buckets[bucket].size()) {
            return end();
        }
        iterator pos_in_data = buckets[bucket][i].second;
        return pos_in_data;
    }

    const_iterator find(const KeyType& key) const {
        std::pair<size_t, size_t> res = find_in_buckets(key);
        size_t bucket = res.first;
        size_t i = res.second;
        if (i == buckets[bucket].size()) {
            return end();
        }
        const_iterator pos_in_data = buckets[bucket][i].second;
        return pos_in_data;
    }

    ValueType& operator[](const KeyType& key) {
        iterator iter = find(key);
        if (iter == end()) {
            iterator new_iter = (*this).insert(std::make_pair(key, ValueType())).first;
            return (*new_iter).second;
        }
        return (*iter).second;
    }

    const ValueType& at(const KeyType& key) const {
        const_iterator iter = find(key);
        if (iter == end()) {
            throw std::out_of_range("No such key");
        }
        return (*iter).second;
    }

    void clear() {
        for (auto elem : data) {
            size_t bucket = get_bucket(elem.first);
            buckets[bucket].clear();
        }
        data.clear();
        sz = 0;
        number_of_buckets = base_number_of_buckets;
        buckets.resize(number_of_buckets);
    }
};
