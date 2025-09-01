// SafeQueue.h (additions)

template <class T>
class SafeQueue {
public:
	SafeQueue() : closed_(false), push_count_(0) {}

	void close() {
		{
			std::lock_guard<std::mutex> lk(m_);
			closed_ = true;
		}
		cv_.notify_all();
	}

	void push(const T& value) {
		{
			std::lock_guard<std::mutex> lk(m_);
			if (closed_) return;
			q_.push(value);
			++push_count_;                 // <-- bump on every push
		}
		cv_.notify_one();
	}

	void push(T&& value) {
		{
			std::lock_guard<std::mutex> lk(m_);
			if (closed_) return;
			q_.push(std::move(value));
			++push_count_;                 // <-- bump on every push
		}
		cv_.notify_one();
	}

	template<class... Args>
	void emplace(Args&&... args) {
		{
			std::lock_guard<std::mutex> lk(m_);
			if (closed_) return;
			q_.emplace(std::forward<Args>(args)...);
			++push_count_;                 // <-- bump on every push
		}
		cv_.notify_one();
	}

	// Edge-triggered wait: returns only if a NEW item was pushed after the call started,
	// or the timeout expires. Pops exactly one item on success.
	template<class Rep, class Period>
	bool wait_for_new_item_and_pop(
		T& out,
		const std::chrono::duration<Rep, Period>& timeout)
	{
		using clock = std::chrono::steady_clock;
		std::unique_lock<std::mutex> lk(m_);

		const std::size_t start_count = push_count_;
		auto deadline = clock::now() + timeout;

		std::size_t observed = start_count;
		for (;;) {
			// Wait until push_count_ changes (new push) OR closed & empty OR timeout
			bool ok = cv_.wait_until(lk, deadline, [&] {
				return (push_count_ != observed) || (closed_ && q_.empty());
				});

			if (!ok) return false;                 // timeout
			if (closed_ && q_.empty() && push_count_ == observed)
				return false;                      // closed without a new push

			// If an item exists, try to pop it
			if (!q_.empty()) {
				out = std::move(q_.front());
				q_.pop();
				return true;                       // success (after a new push)
			}

			// A new push happened (push_count_ changed) but another consumer grabbed it.
			// Update observed and keep waiting within the remaining timeout.
			observed = push_count_;
			// loop continues until next push or timeout
		}
	}

private:
	mutable std::mutex m_;
	std::condition_variable cv_;
	std::queue<T> q_;
	std::atomic<bool> closed_;
	std::size_t push_count_;        // <-- counts pushes under lock
};
