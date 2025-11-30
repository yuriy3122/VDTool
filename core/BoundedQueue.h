#pragma once
#include <mutex>
#include <condition_variable>
#include <deque>
#include <optional>

// Simple bounded MPMC queue.
template <typename T>
class BoundedQueue
{
public:
    explicit BoundedQueue(size_t cap)
        : cap_(cap)
    {}

    bool push(const T& v)
    {
        std::unique_lock<std::mutex> lk(m_);
        
        cv_not_full_.wait(lk, [&]{ return closed_ || q_.size() < cap_; });
        
        if (closed_) return false;
        
        q_.push_back(v);
        
        lk.unlock();
        
        cv_not_empty_.notify_one();
        
        return true;
    }

    bool push(T&& v)
    {
        std::unique_lock<std::mutex> lk(m_);
        
        cv_not_full_.wait(lk, [&]{ return closed_ || q_.size() < cap_; });
        
        if (closed_) return false;
        
        q_.push_back(std::move(v));
        
        lk.unlock();
        
        cv_not_empty_.notify_one();
        
        return true;
    }

    std::optional<T> pop()
    {
        std::unique_lock<std::mutex> lk(m_);
        
        cv_not_empty_.wait(lk, [&]{ return closed_ || !q_.empty(); });
        
        if (q_.empty())
            return std::nullopt; // closed & empty
        
        T v = std::move(q_.front());
        
        q_.pop_front();
        
        lk.unlock();
        
        cv_not_full_.notify_one();
        
        return v;
    }

    void close()
    {
        std::lock_guard<std::mutex> lk(m_);
        
        closed_ = true;
        
        cv_not_empty_.notify_all();
        
        cv_not_full_.notify_all();
    }

private:
    std::mutex m_;
    std::condition_variable cv_not_empty_;
    std::condition_variable cv_not_full_;
    std::deque<T> q_;
    size_t cap_;
    bool closed_ = false;
};

