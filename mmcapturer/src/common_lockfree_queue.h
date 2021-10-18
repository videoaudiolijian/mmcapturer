#ifndef _H_COMMON_LOCK_FREE_QUEUE_H_
#define _H_COMMON_LOCK_FREE_QUEUE_H_

#include <atomic>
#include <string.h>

template <class T>
class LockFreeQueue
{
public:
    LockFreeQueue(unsigned int count = 1024)
        : m_size(count), m_head(0), m_tail(0)
    {
		m_size = count;
		m_head = m_tail = 0;
        m_buffer = new T[m_size];
    }

    ~LockFreeQueue()
    {
		if (m_buffer)
		{
			delete[] m_buffer;
			m_buffer = nullptr;
		}
    }

    bool is_full() const
    {
        return m_head == (m_tail + 1) % m_size;
    }

    bool is_empty() const
    {
        return m_head == m_tail;
    }

    unsigned int front() const
    {
        return m_head;
    }

    unsigned int last() const
    {
        return m_tail;
    }

    /**
     * @brief push the item to the queue
     * 
     * @param val -- item
     * @return true -- success
     * @return false -- if the queue is full, return false
     */
    bool push(const T &val)
    {
        //lock
        while (m_lock.test_and_set())
            ;

        if (is_full())
        {
			m_lock.clear();
            return false;
        }

        memcpy(m_buffer + m_tail, &val, sizeof(T));

		m_tail = (m_tail + 1) % m_size;

		m_lock.clear();
        return true;
    }

    /**
     * @brief pop the head item
     * 
     * @param val -- output parameter, the head item
     * @return true -- success
     * @return false -- if the queue is empty, return false
     */
    bool pop(T &val)
    {
        //lock
        while (m_lock.test_and_set())
            ;

        if (is_empty())
        {
			m_lock.clear();
            return false;
        }

        memcpy(&val, m_buffer + m_head, sizeof(T));
		m_head = (m_head + 1) % m_size;
		m_lock.clear();

        return true;
    }

protected:
    std::atomic<unsigned int> m_size;
    std::atomic<unsigned int> m_head;
    std::atomic<unsigned int> m_tail;
	std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
    T* m_buffer;
};

#endif