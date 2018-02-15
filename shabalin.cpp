#include <iostream>
#include <utility>
#include <stdexcept>

template<typename T>
class VectorBase {
public:
    VectorBase<T>(const size_t& size = 0) : capacity_(size) {
        data_ = static_cast<T*>(operator new(size * sizeof(T)));
    }

    void swap(VectorBase& other) {
        std::swap(data_, other.data_);
        std::swap(capacity_, other.capacity_);
    }

    ~VectorBase() {
        operator delete(data_);
    }

    size_t capacity_{0};
    T * data_{nullptr};
};

template<typename T>
class Vector : private VectorBase<T> {
public:
    Vector() {}

    Vector(const Vector& other) {
        reallocate(other.capacity_);
        for (size_t i = 0; i != other.size_; ++i)
            data_[i] = other.data_[i];
        size_ = other.size_;
        capacity_ = other.capacity_;
    }

    Vector(const size_t& size) : size_(size), VectorBase<T>(size) {
        size_t i = 0;
        try {
            for (; i != size; ++i) {
                new(data_ + i) T(0);
            }
        } catch (...) {
            for (int j = 0; j != i; ++j)
                data_[j].~T();
            throw;
        }
    }

    ~Vector() {
        for (int i = 0; i != size_; ++i) {
            data_[i].~T();
        }
    }

    Vector & operator = (const Vector& other) {
        reallocate(other.capacity_);
        for (size_t i = 0; i != other.size_; ++i)
            data_[i] = other.data_[i];
        size_ = other.size_;
        capacity_ = other.capacity_;
        return *this;
    }

    void push_back(const T& value) {
        if (capacity_ == size_) {
            if (size_ == 0)
                capacity_ = 1;
            else
                capacity_ = size_ * 2;
            reallocate(capacity_);
        }
        new(data_ + size_) T(value);
        ++size_;
    }

    void push_back(T&& value) {
        if (capacity_ == size_) {
            if (size_ == 0)
                capacity_ = 1;
            else
                capacity_ = size_ * 2;
            reallocate(capacity_);
        }
        new(data_ + size_) T(std::move(value));
        ++size_;
    }

    void pop_back() {
        data_[size_ - 1].~T();
        --size_;
    }

    void shrink_to_fit() {
        reallocate(size_);
    }

    void clear() {
        for (size_t i = 0; i != size_; ++i) {
            data_[i].~T();
        }
    }

    const T * begin() noexcept {
        return data_;
    }

    const T * end() noexcept {
        return data_ + size_;
    }

    void reserve(const size_t& n) {
        if (capacity_ < n) {
            reallocate(n);
        }
    }

    void resize(const size_t& n) {
        if (n < size_) {
            for (size_t i = n; i != size_; ++i)
                data_[i].~T();
            size_ = n;
            return;
        } else if (n > size_) {
            if (n > capacity_) {
                VectorBase<T> temporary_(n);
                size_t i = 0;
                try {
                    for (; i != size_; ++i) {
                        new(temporary_.data_ + i) T(data_[i]);
                    }
                } catch (...) {
                    for (size_t j = 0; j != i; ++j)
                        temporary_.data_[j].~T();
                    throw;
                }

                VectorBase<T>::swap(temporary_);
                for (size_t i = size_; i != n; ++i)
                    new(data_ + i) T();
                for (size_t i = 0; i != size_; ++i)
                    temporary_.data_[i].~T();
            } else if (n < capacity_) {
                for (size_t i = size_; i != n; ++i) {
                    new(data_ + i) T();
                }
            }
        }
        size_ = n;
    }

    bool empty() const {
        return !size_;
    }

    const size_t capacity() const {
        return capacity_;
    }

    const size_t size() const {
        return size_;
    }

    T* data() noexcept {
        return const_cast<T*>(const_cast<const Vector*>(this)->data());
    }

    const T* data() const noexcept {
        return data_;
    }

    T operator[] (size_t n) const {
        return data_[n];
    }

    T& operator[] (size_t n) {
        return data_[n];
    }

    T at(size_t n) const {
        if (n >= size() || n < 0) {
            throw std::out_of_range("");
        } else {
            return data_[n];
        }
    }

    T& at(size_t n) {
        if (n >= size() || n < 0) {
            throw std::out_of_range("");
        } else {
            return data_[n];
        }
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (capacity() == size()) {
            if (size() == 0)
                capacity() = 1;
            else
                capacity() = size() * 2;
            reallocate(capacity());
        }
        new(data() + size()) T(args...);
        ++size_;
}

    T& front() {
        return data_[0];
    }

    T& back() {
        return data_[size_ - 1];
    }

    T front() const {
        return data_[0];
    }

    T back() const {
        return data_[size_ - 1];
    }

    class Iterator {
    public:
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = ssize_t;

        Iterator(pointer ptr) : ptr_(ptr) {}

        Iterator operator++() {
            Iterator i = *this;
            ptr_++;
            return i;
        }

        Iterator operator++(int self) {
            ptr_++;
            return *this;
        }

        Iterator operator--() {
            Iterator i = *this;
            ptr_--;
            return i;
        }

        Iterator operator--(int self) {
            ptr_--;
            return *this;
        }

        Iterator operator += (const ssize_t& n) {
            ptr_ = ptr_ + n;
            return *this;
        }

        Iterator operator -= (const ssize_t& n) {
            ptr_ = ptr_ - n;
            return *this;
        }

        Iterator operator + (const ssize_t& n) {
            return ptr_ += n;
        }

        Iterator operator - (const ssize_t& n) {
            return ptr_ -= n;
        }

        reference operator*() {
            return *ptr_;
        }

        bool operator == (const Iterator &it) {
            return ptr_ == it.ptr_;
        }

        bool operator != (const Iterator &it) {
            return ptr_ != it.ptr_;
        }

    private:
        pointer ptr_;
    };

private:
    size_t size_{0};

    using VectorBase<T>::data_;
    using VectorBase<T>::capacity_;

    void reallocate(const size_t & size) {
        VectorBase<T> temporary_(size);
        size_t i = 0;
        try {
            for (; i != size_; ++i) {
               new(temporary_.data_ + i) T(data_[i]);
            }
        } catch (...) {
            for (size_t j = 0; j != i; ++j)
                temporary_.data_[j].~T();
            throw;
        }

        VectorBase<T>::swap(temporary_);
        for (size_t i = 0; i != size_; ++i)
            temporary_.data_[i].~T();
    }
};