namespace task {

namespace util {

template <class T>
T* RefCounter<T>::GetPtr(RefCounter<T>* counter) {
    return (counter == nullptr) ? nullptr : counter->ptr;
};

template <class T>
long RefCounter<T>::UseCount(RefCounter<T>* counter) {
    return (counter == nullptr) ? 0 : counter->use_count;
};

template <class T>
long RefCounter<T>::WeakCount(RefCounter<T>* counter) {
    return (counter == nullptr) ? 0 : counter->weak_count;
};

template <class T>
RefCounter<T>* RefCounter<T>::SharedCounter(T* ptr) {
    auto counter = new RefCounter();
    counter->ptr = ptr;
    counter->use_count = 1;
    counter->weak_count = 0;
    return counter;
};

template <class T>
RefCounter<T>* RefCounter<T>::IncrementShared(RefCounter<T>* counter) {
    if (counter != nullptr) {
        counter->use_count++;
    }
    return counter;
};

template <class T>
RefCounter<T>* RefCounter<T>::DecrementShared(RefCounter<T>* counter) {
    if (counter == nullptr) {
        return nullptr;
    }

    if (--counter->use_count == 0) {
        if (counter->ptr != nullptr) {
            delete counter->ptr;
            counter->ptr = nullptr;
        }
        if (counter->weak_count == 0) {
            delete counter;
        }
        return nullptr;         
    }
    return counter;
};

template <class T>
RefCounter<T>* RefCounter<T>::IncrementWeak(RefCounter<T>* counter) {
    if (counter != nullptr) {
        counter->weak_count++;
    }
    return counter;
};

template <class T>
RefCounter<T>* RefCounter<T>::DecrementWeak(RefCounter<T>* counter) {
    if (counter == nullptr) {
        return nullptr;
    }

    if (--counter->weak_count == 0 && counter->use_count == 0) {
        if (counter->ptr != nullptr) {
            delete counter->ptr;
        }
        delete counter;
        return nullptr;         
    }
    return counter;
};

}

template <class T>
UniquePtr<T>::UniquePtr(UniquePtr<T>&& other) {
    ptr_ = std::move(other.ptr_);
    other.ptr_ = nullptr;
};

template <class T>
UniquePtr<T>& UniquePtr<T>::operator=(UniquePtr<T>&& other) {
    if (this == &other) {
        return *this;
    }

    release();
    std::swap(ptr_, other.ptr_);
    return *this;
};

template <class T>
T* UniquePtr<T>::release() {
    T* tmp = nullptr;
    std::swap(ptr_, tmp);
    return tmp;
};

template <class T>
void UniquePtr<T>::reset(T* ptr) {
    auto tmp = ptr_;
    ptr_ = ptr;
    if (tmp != nullptr) {
        delete tmp;
    }
};

template <class T>
void UniquePtr<T>::reset(std::nullptr_t ptr) {
    if (ptr_ != nullptr) {
        delete ptr_;
    }
    ptr_ = nullptr;
};

template <class T>
void UniquePtr<T>::swap(UniquePtr<T>& other) {
    std::swap(ptr_, other.ptr_);
};

template <class T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr<T>& other) {
    if (this != &other) {
        Counter::DecrementShared(refCounter_);
        refCounter_ = Counter::IncrementShared(other.refCounter_);
    }
    return *this;
};

template <class T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr<T>&& other) {
    if (this != &other) {
        Counter::DecrementShared(refCounter_);
        refCounter_ = nullptr;
        std::swap(refCounter_, other.refCounter_);
    }
    return *this;
};

template <class T>
void SharedPtr<T>::reset(T* ptr) {
    refCounter_ = Counter::DecrementShared(refCounter_);
    if (ptr != nullptr) {
        refCounter_ = Counter::SharedCounter(ptr);
    }
};

template <class T>
void SharedPtr<T>::swap(SharedPtr& other) {
    std::swap(refCounter_, other.refCounter_);
};

template <class T>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr<T>& other) {
    if (this != &other) {
        Counter::DecrementWeak(refCounter_);
        refCounter_ = Counter::IncrementWeak(other.refCounter_);
    }
    return *this;
};

template <class T>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr<T>&& other) {
    if (this != &other) {
        Counter::DecrementWeak(refCounter_);
        refCounter_ = nullptr;
        std::swap(refCounter_, other.refCounter_);
    }
    return *this;
};

template <class T>
WeakPtr<T>& WeakPtr<T>::operator=(SharedPtr<T>& shared) {
    Counter::DecrementWeak(refCounter_);
    refCounter_ = Counter::IncrementWeak(shared.refCounter_);
    return *this;
};

template <class T>
void WeakPtr<T>::reset() {
    Counter::DecrementWeak(refCounter_);
    refCounter_ = nullptr;
};

template <class T>
void WeakPtr<T>::swap(WeakPtr<T>& other) {
    std::swap(refCounter_, other.refCounter_);
};

}