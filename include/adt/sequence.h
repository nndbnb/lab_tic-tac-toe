#pragma once

#include <iostream>
#include <stdexcept>
#include <initializer_list>
#include "adt/linked_list.h"
#include "adt/dynamic_array.h"

namespace adt {

template <typename T>
class Sequence {
public:
    virtual ~Sequence() = default;

    virtual T               GetFirst()                               const = 0;
    virtual T               GetLast()                                const = 0;
    virtual T               Get(int idx)                             const = 0;
    virtual Sequence<T>    *GetSubsequence(int l, int r)             const = 0;
    virtual int             GetLength()                              const = 0;

    virtual Sequence<T>    *Append(T item)                           const = 0;
    virtual Sequence<T>    *Prepend(T item)                          const = 0;
    virtual Sequence<T>    *InsertAt(T item, int idx)                const = 0;
    virtual Sequence<T>    *Concat(Sequence<T> *other)               const = 0;
};

template <typename T>
class MutableSequence : public Sequence<T> {
public:
    virtual void AppendInPlace(T item)               = 0;
    virtual void PrependInPlace(T item)              = 0;
    virtual void InsertAtInPlace(T item, int index)  = 0;
};

template <typename T>
class ArraySequence : public MutableSequence<T> {
public:
    ArraySequence() = default;
    ArraySequence(T *items, int n) : arr(items, n) {}
    
    ArraySequence(std::initializer_list<T> init) {
        for (const auto& item : init) {
            AppendInPlace(item);
        }
    }
    
    ArraySequence(const ArraySequence &other) : arr(other.arr) {}
    
    ArraySequence& operator=(const ArraySequence &other) {
        if (this != &other) {
            arr = other.arr;
        }
        return *this;
    }

    T GetFirst() const override {
        if (!arr.GetSize()) throw std::out_of_range("Sequence is empty");
        return arr.Get(0);
    }
    T GetLast()  const override {
        if (!arr.GetSize()) throw std::out_of_range("Sequence is empty");
        return arr.Get(arr.GetSize() - 1);
    }
    T Get(int i) const override { return arr.Get(i); }

    Sequence<T>* GetSubsequence(int l, int r) const override {
        if (l < 0 || r >= arr.GetSize() || l > r) throw std::out_of_range("Invalid indices");
        auto *sub = new ArraySequence<T>();
        for (int i = l; i <= r; ++i) sub->AppendInPlace(arr.Get(i));
        return sub;
    }
    int GetLength() const override { return arr.GetSize(); }

    Sequence<T>* Append(T item) const override {
        auto *copy = new ArraySequence<T>(*this);
        copy->AppendInPlace(item);
        return copy;
    }
    Sequence<T>* Prepend(T item) const override {
        auto *res = new ArraySequence<T>();
        res->AppendInPlace(item);
        for (int i = 0; i < arr.GetSize(); ++i) res->AppendInPlace(arr.Get(i));
        return res;
    }
    Sequence<T>* InsertAt(T item, int idx) const override {
        if (idx < 0 || idx > arr.GetSize()) throw std::out_of_range("Index out of range");
        auto *res = new ArraySequence<T>();
        for (int i = 0; i < idx; ++i) res->AppendInPlace(arr.Get(i));
        res->AppendInPlace(item);
        for (int i = idx; i < arr.GetSize(); ++i) res->AppendInPlace(arr.Get(i));
        return res;
    }
    Sequence<T>* Concat(Sequence<T>* other) const override {
        auto *res = new ArraySequence<T>(*this);
        for (int i = 0; i < other->GetLength(); ++i) res->AppendInPlace(other->Get(i));
        return res;
    }

    void AppendInPlace(T item) override { arr.Append(item); }

    void PrependInPlace(T item) override {
        arr.Resize(arr.GetSize() + 1);
        for (int i = arr.GetSize() - 1; i > 0; --i) arr.Set(i, arr.Get(i - 1));
        arr.Set(0, item);
    }

    void InsertAtInPlace(T item, int idx) override {
        if (idx < 0 || idx > arr.GetSize()) throw std::out_of_range("Index out of range");
        
        int oldSize = arr.GetSize();
        arr.Resize(oldSize + 1);
        
        for (int i = oldSize; i > idx; --i) {
            arr.Set(i, arr.Get(i - 1));
        }
        
        arr.Set(idx, item);
    }

    bool Empty() const { return arr.GetSize() == 0; }
    
    int Size() const { return GetLength(); }
    
    T& operator[](int index) { return arr.GetRef(index); }
    
    const T& operator[](int index) const { return arr.GetRef(index); }
    
    T& Back() { 
        if (Empty()) throw std::out_of_range("Sequence is empty");
        return arr.GetRef(arr.GetSize() - 1); 
    }
    
    const T& Back() const { 
        if (Empty()) throw std::out_of_range("Sequence is empty");
        return arr.GetRef(arr.GetSize() - 1);
    }
    
    void PopBack() {
        if (Empty()) throw std::out_of_range("Sequence is empty");
        arr.Resize(arr.GetSize() - 1);
    }
    
    void Reserve(int capacity) { arr.Reserve(capacity); }
    
    void Resize(int new_size) { arr.Resize(new_size); }
    
    void Clear() { arr.Resize(0); }
    
    void SortInPlace(const std::function<bool(const T&, const T&)>& comp = 
                     [](const T& a, const T& b) { return a < b; }) {
        arr.SortInPlace(comp);
    }
    
    void RemoveAt(int index) {
        if (index < 0 || index >= GetLength()) 
            throw std::out_of_range("Index out of range");
        for (int i = index; i < GetLength() - 1; ++i) {
            arr.Set(i, arr.Get(i + 1));
        }
        arr.Resize(arr.GetSize() - 1);
    }
    
    void Set(int index, const T& value) {
        if (index < 0 || index >= GetLength())
            throw std::out_of_range("Index out of range");
        arr.Set(index, value);
    }

private:
    DynamicArray<T> arr;
};


template <typename T>
class ListSequence : public MutableSequence<T> {
public:
    ListSequence() = default;
    ListSequence(T *items, int n) { for (int i = 0; i < n; ++i) list.Append(items[i]); }
    ListSequence(const ListSequence &o) { for (int i = 0; i < o.GetLength(); ++i) list.Append(o.Get(i)); }

    T GetFirst() const override { if (!list.GetLength()) throw std::out_of_range("Sequence is empty"); return list.Get(0); }
    T GetLast()  const override { if (!list.GetLength()) throw std::out_of_range("Sequence is empty"); return list.Get(list.GetLength() - 1); }
    T Get(int i) const override { return list.Get(i); }

    Sequence<T>* GetSubsequence(int l, int r) const override {
        if (l < 0 || r >= list.GetLength() || l > r) throw std::out_of_range("Invalid indices");
        auto *sub = new ListSequence<T>();
        for (int i = l; i <= r; ++i) sub->AppendInPlace(list.Get(i));
        return sub;
    }
    int GetLength() const override { return list.GetLength(); }

    Sequence<T>* Append(T item) const override {
        auto *copy = new ListSequence<T>(*this);
        copy->AppendInPlace(item);
        return copy;
    }
    Sequence<T>* Prepend(T item) const override {
        auto *res = new ListSequence<T>();
        res->AppendInPlace(item);
        for (int i = 0; i < list.GetLength(); ++i) res->AppendInPlace(list.Get(i));
        return res;
    }
    Sequence<T>* InsertAt(T item, int idx) const override {
        if (idx < 0 || idx > list.GetLength()) throw std::out_of_range("Index out of range");
        auto *res = new ListSequence<T>();
        for (int i = 0; i < idx; ++i) res->AppendInPlace(list.Get(i));
        res->AppendInPlace(item);
        for (int i = idx; i < list.GetLength(); ++i) res->AppendInPlace(list.Get(i));
        return res;
    }
    Sequence<T>* Concat(Sequence<T>* other) const override {
        auto *res = new ListSequence<T>(*this);
        for (int i = 0; i < other->GetLength(); ++i) res->AppendInPlace(other->Get(i));
        return res;
    }

    void AppendInPlace(T item) override { list.Append(item); }
    void PrependInPlace(T item) override { list.Prepend(item); }
    void InsertAtInPlace(T item, int idx) override { list.InsertAt(item, idx); }

private:
    LinkedList<T> list;
};

template <typename T>
class ImmutableSequence : public Sequence<T> {
public:
    explicit ImmutableSequence(Sequence<T>* src) : seq(src) {}
    
    ~ImmutableSequence() { delete seq; }
    
    ImmutableSequence(const ImmutableSequence &o) {
        if (o.seq->GetLength() > 0) {
            seq = o.seq->GetSubsequence(0, o.seq->GetLength() - 1);
        } else {
            seq = new ArraySequence<T>();
        }
    }
    
    ImmutableSequence& operator=(const ImmutableSequence &o) {
        if (this != &o) {
            delete seq;
            if (o.seq->GetLength() > 0) {
                seq = o.seq->GetSubsequence(0, o.seq->GetLength() - 1);
            } else {
                seq = new ArraySequence<T>();
            }
        }
        return *this;
    }

    T GetFirst() const override { return seq->GetFirst(); }
    T GetLast()  const override { return seq->GetLast(); }
    T Get(int i) const override { return seq->Get(i); }
    int GetLength() const override { return seq->GetLength(); }
    
    Sequence<T>* GetSubsequence(int l, int r) const override { 
        return new ImmutableSequence<T>(seq->GetSubsequence(l, r)); 
    }
    Sequence<T>* Append(T item) const override { 
        return new ImmutableSequence<T>(seq->Append(item)); 
    }
    Sequence<T>* Prepend(T item) const override { 
        return new ImmutableSequence<T>(seq->Prepend(item)); 
    }
    Sequence<T>* InsertAt(T item, int idx) const override { 
        return new ImmutableSequence<T>(seq->InsertAt(item, idx)); 
    }
    Sequence<T>* Concat(Sequence<T>* other) const override { 
        return new ImmutableSequence<T>(seq->Concat(other)); 
    }

private:
    Sequence<T>* seq;
};

} // namespace adt

