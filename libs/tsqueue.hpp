#pragma once

template<typename T>
class tsqueue {
public:
    tsqueue() = default;
    tsqueue(const tsqueue<T>&) = delete;
    virtual ~tsqueue() { clear(); };
    
    const T& front() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_deque.front();
    }
    
    const T& back() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_deque.back();
    }
    
    T pop_front() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !m_deque.empty(); });
        auto t = std::move(m_deque.front());
        m_deque.pop_front();
        return t;
    }
    
    T pop_back() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !m_deque.empty(); });
        auto t = std::move(m_deque.back());
        m_deque.pop_back();
        return t;
    }
    
    void emplace_front(const T& item) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_deque.emplace_front(std::move(item));
        m_cv.notify_one();
    }
    
    void emplace_back(const T& item) {
        std::scoped_lock lock(m_mutex);
        m_deque.emplace_back(std::move(item));
        m_cv.notify_one();
    }
    
    bool empty() {
        std::scoped_lock lock(m_mutex);
        return m_deque.empty();
    }
    
    size_t count()      {
        std::scoped_lock lock(m_mutex);
        return m_deque.size();
    }
    
    void clear() {
        std::scoped_lock lock(m_mutex);
        m_deque.clear();
    }
    
protected:
    std::mutex m_mutex;
    std::deque<T> m_deque;
    std::condition_variable m_cv;
};
