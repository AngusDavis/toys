#include <functional>
#include <iostream>
#include <vector>
#include <exception>

#ifndef TOYS_HASHTABLE
#define TOYS_HASHTABLE

class NotFoundException : public std::exception {
    virtual const char* what() const throw()
    {
        return "Element not found";
    }
};

class InvalidLoadFactorException : public std::exception {
    virtual const char* what() const throw()
    {
        return "OpenAddressedHashtable requires a loadFactor < 1.0";
    }
};


template <typename TKey, typename TValue>
struct KeyValue {
    TKey key;
    TValue value;
    unsigned long hashcode;
};

template <class TKey, class TValue>
class Hashtable {
protected:
    std::hash<TKey> _hashFunction;
    float _requestedLoadFactor;
    int _currentCount;

public:
    Hashtable(float loadFactor, std::hash<TKey> hashFunction) : _requestedLoadFactor(loadFactor), _hashFunction(hashFunction), _currentCount(0) {
    }

    virtual float loadFactor() = 0;
    virtual TValue get(TKey key) = 0;
    virtual bool exists(TKey key) = 0;
    virtual void add(TKey key, TValue value) = 0;
    virtual ~Hashtable() {
    }
};

template <class TKey, class TValue>
class OpenAddressingHashtable : public Hashtable<TKey, TValue> {

    typedef Hashtable<TKey, TValue> Base;
    typedef KeyValue<TKey, TValue> KV;
    std::vector<KV> _elements;

public:
    OpenAddressingHashtable(float loadFactor) : Base(loadFactor, std::hash<TKey>()), _elements(10) {
        if ( loadFactor >= 1.0f) {
            throw InvalidLoadFactorException();
        }
    }
    virtual TValue get(TKey key) {
        unsigned long hashValue = Base::_hashFunction(key);
        KV& kv = getKeyValue(_elements, key, hashValue);

        if ( kv.hashcode == hashValue ) {
            return kv.value;
        }

        throw NotFoundException();
    }
    virtual bool exists(TKey key) {
        unsigned long hashValue = Base::_hashFunction(key);
        KV& kv = getKeyValue(_elements, key, hashValue);

        if ( kv.hashcode == hashValue ) {
            return true;
        }
        throw false;
    }
    virtual void add(TKey key, TValue value) {
        unsigned long hashValue = Base::_hashFunction(key);
        addToElements(_elements, key, value, hashValue);
	Base::_currentCount++;
        resizeIfRequired();
    }
    virtual float loadFactor() {
        return Base::_currentCount / (float)_elements.size();
    }


protected:

    void addToElements(std::vector<KV> &elements, TKey key, TValue value, unsigned long hashValue) {

        KV& kv = getKeyValue(elements, key, hashValue);

        if ( kv.hashcode == 0 ) {
            kv = {key, value, hashValue};
	    
        }
    }

    KV& getKeyValue(std::vector<KV> &elements, TKey key, unsigned long hashValue) {
        int startIdx = hashValue % elements.size();
        int offset = 0;
        for(offset = 0; (startIdx + offset +1 ) % elements.size() != startIdx &&
                elements.at((startIdx + offset) % elements.size()).hashcode != hashValue &&
                elements.at((startIdx + offset) % elements.size()).hashcode != 0; offset++) {
        }
        int finalIdx = (startIdx + offset) % elements.size();
        KV& kv = elements.at(finalIdx);
        return kv;
    }

    void resizeIfRequired() {
        if ( loadFactor() > Base::_requestedLoadFactor ) {
            std::vector<KV> newElements(_elements.size() * 2 );
            for(auto iter = _elements.begin(); iter != _elements.end(); iter++) {
                addToElements(newElements, iter->key, iter->value, iter->hashcode);
            }

            _elements.swap(newElements);
        }
    }
};


template <class TKey, class TValue>
class ClosedAddressingHashtable : Hashtable<TKey, TValue> {
    typedef std::vector<KeyValue<TKey, TValue>> Bucket;
    typedef Hashtable<TKey, TValue> Base;
    typedef KeyValue<TKey, TValue> KV;

    std::vector<Bucket> _elements;

public:
    ClosedAddressingHashtable(float loadFactor) : Base(loadFactor, std::hash<TKey>()), _elements(10) {
    }

    virtual TValue get(TKey key) {
        unsigned long hashValue = Base::_hashFunction(key);
        Bucket& bucket = _elements.at(hashValue % _elements.size());
        for(auto iter = bucket.begin(); iter != bucket.end(); iter++) {
            if ( iter->hashcode == hashValue ) {
                return iter->value;
            }
        }
        throw NotFoundException();
    }
    virtual bool exists(TKey key) {
        unsigned long hashValue = Base::_hashFunction(key);
        Bucket& bucket = _elements.at(hashValue % _elements.size());
        for(auto iter = bucket.begin(); iter != bucket.end(); iter++) {
            if ( iter->hashcode == hashValue ) {
                return true;
            }
        }
        return false;
    }
    virtual void add(TKey key, TValue value) {
        unsigned long hashValue = Base::_hashFunction(key);
        Bucket& bucket = _elements.at(hashValue % _elements.size());
        KeyValue<TKey, TValue> newValue = {key, value, hashValue};
        bucket.push_back(newValue);
        Base::_currentCount++;
        resizeIfRequired();
    }

    virtual float loadFactor() {
        return Base::_currentCount / (float)_elements.size();
    }

protected:
    void resizeIfRequired() {
        if ( loadFactor() > Base::_requestedLoadFactor ) {
            std::vector<Bucket> newBuckets(_elements.size() * 2);
            for(auto bucket = _elements.begin(); bucket != _elements.end(); bucket++) {
                for(auto iter = bucket->begin(); iter != bucket->end(); iter++) {
                    Bucket& newBucket = newBuckets.at(iter->hashcode % newBuckets.size());
                    newBucket.push_back(*iter);
                }
            }

            _elements.swap(newBuckets);
        }
    }
};

#endif
