#ifndef PRIVATE_THREAD_SAFE_QUEUE_HPP
#define PRIVATE_THREAD_SAFE_QUEUE_HPP
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
namespace
{
/// @brief This is a thread-safe queue based on listing 4.5 of C++ Concurrency
///        in Action, 2nd Edition. 
template<typename T>
class ThreadSafeQueue
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    ThreadSafeQueue() = default;
    /// @brief Copy constructor.
    /// @param[in] queue  The queue from which to initialize this class.
    ThreadSafeQueue(const ThreadSafeQueue &queue)
    {
        *this = queue;
    }
    /// @brief Move constructor.
    /// @param[in,out] queue  The queue from which to initialize this class.
    ///                       On exit, queue's behavior is undefined.
    ThreadSafeQueue(ThreadSafeQueue &&queue) noexcept
    {
        *this = std::move(queue);
    }
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment.
    /// @param[in] queue  The queue to copy to this.
    /// @result A deep copy of the input queue.
    ThreadSafeQueue& operator=(const ThreadSafeQueue &queue)
    {
        if (&queue == this){return *this;}
        std::lock_guard<std::mutex> lockGuard(queue.mMutex);
        mDataQueue = queue.mDataQueue;
    }
    /// @brief Move assignment.
    /// @param[in,out] queue  The queue whose memory will be moved to this.
    ///                       On exit, queue's behavior is undefined. 
    /// @result The memory from queue moved to this.
    ThreadSafeQueue& operator=(ThreadSafeQueue &&queue) noexcept
    {
        if (&queue == this){return *this;}
        std::lock_guard<std::mutex> lockGuard(queue.mMutex);
        mDataQueue = std::move(queue.mDataQueue);
    }
    /// @}
 
    /// @brief Adds a value to the back of the queue.
    /// @param[in] value  the value to add to the queue.
    void push(const T &value)
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mDataQueue.push(value);
        mConditionVariable.notify_one(); // Let waiting thread know 
    }
    /// @brief Copies the value from the front of the queue and removes that
    ///        value from the front of the queue.
    /// @param[out] value  A copy of the value at the front of the queue.
    void wait_and_pop(T *value)
    {
        // Waiting thread needs to unlock mutex while waiting and lock again
        // afterward.  unique_lock does this while lock_guard does not.
        std::unique_lock<std::mutex> lockGuard(mMutex);
        mConditionVariable.wait(lockGuard, [this]
                                {
                                    return !mDataQueue.empty();
                                });
        *value = mDataQueue.front();
        mDataQueue.pop();
    }
    /// @brief Copies the value from the front of the queue and removes that
    ///        value from the front of the queue.
    /// @param[out] value  The value from the front of the queue.  Or, if the
    ///                    function times out, then a NULL pointer.
    /// @result True indicates that the value was set whereas false indicate.
    ///         the class timed out.
    /// @note This variant can time out and return NULL.
    [[nodiscard]]
    bool wait_until_and_pop(T *value,
                            const std::chrono::milliseconds &waitFor
                               = static_cast<std::chrono::milliseconds> (10))
    {
        auto now = std::chrono::system_clock::now();
        std::unique_lock<std::mutex> lockGuard(mMutex);
        if (mConditionVariable.wait_until(lockGuard, now + waitFor, [this] 
                                         {
                                             return !mDataQueue.empty();
                                         }))
        {
            *value = mDataQueue.front();
            mDataQueue.pop();
            return true;
        }
        return false;
    }
    /// @brief Pops the front of the queue.  This returns nothing.
    /// @note This is useful when enforcing a maximum queue size.
    void pop()
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mDataQueue.pop();
    }
    /// @result A container with the value from the front of the queue.  The
    ///         value at front of the queue is removed.
    /// @result The value at the front of the queue.
    [[nodiscard]] std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lockGuard(mMutex);
        mConditionVariable.wait(lockGuard, [this]
                                {
                                    return !mDataQueue.empty();
                                });
        std::shared_ptr<T> result(std::make_shared<T> (mDataQueue.front()));
        mDataQueue.pop();
        return result;
    }
    /// @brief Attempts to copy the value of the value at the front of the queue
    ///        and remove it from the queue.
    /// @param[out] value  If not a nullptr, then this is the value at the front
    ///                    at the front of the queue.
    /// @retval True indicates that the queue was not empty and value 
    ///         corresponds to the value that was at the front of the queue.
    /// @retval False indicates that the queue was empty. 
    [[nodiscard]] bool try_pop(T *value)
    {
        *value = nullptr;
        std::lock_guard<std::mutex> lockGuard(mMutex);
        if (mDataQueue.empty()){return false;}
        *value = mDataQueue.front();
        mDataQueue.pop();
        return true;
    }
    /// @brief A container with the value at the front of the queue provided
    ///        that the queue is not empty.  If the queue is not empty then
    ///        then this value is removed from the front. 
    /// @result The value at the front of the queue or a nullptr if the queue
    ///         was empty. 
    [[nodiscard]] std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        std::shared_ptr<T> result;
        if (mDataQueue.empty())
        {
            result = nullptr;
            return result;
        }
        result = std::make_shared<T> (mDataQueue.front());
        mDataQueue.pop();
        return result;
    }
    /// @result True indicates that the queue is empty.
    [[nodiscard]] bool empty() const
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        return mDataQueue.empty();
    }
    /// @result The number of elements in the queue.
    [[nodiscard]] size_t size() const
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        return mDataQueue.size();
    }
    /// @name Constructors
    /// @{

    /// @brief Empties the queue.
    //void clear() noexcept
    //{
    //    std::lock_guard<std::mutex> lockGuard(mMutex);
    //    mDataQueue.clear();
    //}
    /// @brief Destructor.
    ~ThreadSafeQueue() = default;
    /// @}
private:
    mutable std::mutex mMutex;
    std::queue<T> mDataQueue;
    std::condition_variable mConditionVariable;
};
}
#endif
