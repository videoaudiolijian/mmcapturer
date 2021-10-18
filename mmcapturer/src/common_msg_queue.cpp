#include "common_msg_queue.h"

#include <functional>

MessageItem::MessageItem(int msg_id, int first_param, int second_param, void *data)
	: m_msg_id(msg_id), m_first_param(first_param), m_second_param(second_param), m_pdata(data)
{
}

MessageItem::~MessageItem()
{
}

int MessageItem::get_id() const
{
	return this->m_msg_id;
}

int MessageItem::get_first_param() const
{
	return this->m_first_param;
}

int MessageItem::get_second_param() const
{
	return this->m_second_param;
}

void *MessageItem::get_data() const
{
	return this->m_pdata;
}

/*****************************************************************/
/*****************************************************************/

SimpleMessageQueue::SimpleMessageQueue(int max_count)
	: m_max_queue_length(max_count), m_is_running(false), m_initialized(false)
{
}

SimpleMessageQueue::~SimpleMessageQueue()
{
}

void SimpleMessageQueue::start()
{
	m_is_running = true;
	m_running_thread = std::thread(std::bind(&SimpleMessageQueue::run, this));
	m_initialized = true;
}

void SimpleMessageQueue::stop()
{
	m_is_running = false;
	put(MessageItem(EXIT_MSG_ID, 0, 0, nullptr));

	if (m_initialized)
	{
		m_running_thread.join();
	}
}

void SimpleMessageQueue::run()
{
	while (m_is_running)
	{
		MessageItem msg;
		//get the message item
		get(msg);
		if (msg.get_id() != EXIT_MSG_ID)
		{
			handle_msg(msg);
		}

		std::this_thread::yield();
	}

	clear_msg_queue();
}

int SimpleMessageQueue::get(MessageItem &msg)
{
	std::unique_lock<std::mutex> lock(this->m_mutex);

	this->m_condi.wait(lock, [this] {
		if (!m_msg_queue.empty())
		{
			return true;
		}
		else
		{
			return false;
		}
	});

	//get
	msg = m_msg_queue.front();
	m_msg_queue.pop_front();

	return m_msg_queue.size();
}

bool SimpleMessageQueue::put(const MessageItem &msg)
{
	if (!m_is_running && msg.get_id() != EXIT_MSG_ID)
	{
		destroy_msg(msg);
		return false;
	}

	std::unique_lock<std::mutex> lock(this->m_mutex);

	//add item
	if ((int)m_msg_queue.size() < m_max_queue_length)
	{
		m_msg_queue.push_back(msg);

		if (m_msg_queue.size() == 1)
		{
			this->m_condi.notify_one();
		}

		return true;
	}
	else
	{
		//destroy the msg
		if (msg.get_id() != EXIT_MSG_ID)
		{
			destroy_msg(msg);
		}
		return false;
	}
}

void SimpleMessageQueue::clear_msg_queue()
{
	//clear the remaining data
	std::unique_lock<std::mutex> lock(this->m_mutex);
	while (m_msg_queue.size() > 0)
	{
		MessageItem item = m_msg_queue.front();
		m_msg_queue.pop_front();
		if (item.get_id() != EXIT_MSG_ID)
		{
			destroy_msg(item);
		}
	}
}