#pragma once
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <future>
#include "Core.h"

namespace Doom {

	using Task = std::function<void()>;
	class DOOM_API ThreadPool {
	private:

		static ThreadPool* s_Instance;
		std::vector<std::thread> m_Threads;
		std::map <std::thread::id,bool> m_IsThreadBusy;
		std::mutex m_Mutex;
		std::thread::id m_MainId = std::this_thread::get_id();
		std::condition_variable m_CondVar;
		std::queue <Task> m_Tasks;
		int m_NumThreads = 0;
		bool m_IsStoped = false;
		static bool m_IsInitialized;

		void InfiniteLoopFunction();

		ThreadPool& operator=(const ThreadPool& rhs) { return *this; }
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(int n);
		~ThreadPool();

	public:

		inline uint32_t GetAmountOfThreads() { return m_Threads.size(); }
		void Shutdown();
		void Enqueue(Task task);
		static ThreadPool& GetInstance() { return *s_Instance; }
		static void Init();

		friend class Editor;
	};
}