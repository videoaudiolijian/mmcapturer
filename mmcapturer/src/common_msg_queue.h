#ifndef __H_COMMON_MSG_QUEUE_H__
#define __H_COMMON_MSG_QUEUE_H__

#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <atomic>


//the exit msg id
constexpr int EXIT_MSG_ID = 0xFFFFFFFF;

/**
 * the message item in SimpleMessageQueue
 */
class MessageItem
{
public:
	MessageItem() = default;

	/**
	 * constructor
	 * @param:
	 *     msgID -- message id
	 *     firstParam -- the first param
	 *     secondParam -- the second param
	 *     data -- the attatched data pointer
	 */
	MessageItem(int msgID, int firstParam, int secondParam, void *data);

	virtual ~MessageItem();

	int get_id() const;

	int get_first_param() const;

	int get_second_param() const;

	void *get_data() const;

protected:
	//message id
	int m_msg_id;

	//fist param
	int m_first_param;

	//second param
	int m_second_param;

	//data pointer
	void *m_pdata;
};

/** 
* A Simple Message Queue 
*/
class SimpleMessageQueue
{
public:
	SimpleMessageQueue(const SimpleMessageQueue &) = delete;
	SimpleMessageQueue &operator=(const SimpleMessageQueue &) = delete;
	SimpleMessageQueue(const SimpleMessageQueue &&) = delete;
	SimpleMessageQueue &operator=(const SimpleMessageQueue &&) = delete;

	//constructor
	//@param maxCount -- the maximux capacity of the queue
	SimpleMessageQueue(int maxCount = 2048);
	virtual ~SimpleMessageQueue();

	//add message item msg to the queue.
	//if the queue if full, the queue will ignore the item
	//@param: msg -- message item
	//@return true--success, false--fail
	bool put(const MessageItem &msg);

protected:
	//start the message queue loop
	void start();

	//stop the message queue loop
	void stop();

	//get the first item in the queue.
	//if the queue is empty, this function will blocked.
	//and will return the message item when the queue is not empty.
	//@param: msg -- message item
	//@param: the remaining Message item count in the queue
	int get(MessageItem &msg);

	//message process function
	virtual void handle_msg(const MessageItem &msg) = 0;
	virtual void run();
	virtual void destroy_msg(const MessageItem &msg) = 0;
	virtual void clear_msg_queue();

private:
	
	bool m_initialized;
	//the max queue length
	int m_max_queue_length;

	//is the message queue running?
	std::atomic<bool> m_is_running;

	std::thread m_running_thread;

	//the inner queue
	std::deque<MessageItem> m_msg_queue;

	std::mutex m_mutex;
	std::condition_variable m_condi;
};

#endif