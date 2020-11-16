#pragma once

#ifndef _EVENTSYSTEM_H_
#define _EVENTSYSTEM_H_
#include <list>
#include <map>

using std::multimap;
using std::list;
using std::pair;

#include "Event.h"
#include "Listener.h"
#include <iostream>
#include "ThreadPool.h"
#include "Core.h"

namespace Doom {

	enum DOOM_API EventType {
		ONUPDATE,
		ONSTART,
		ONWINDOWRESIZE,
		ONMAINTHREADPROCESS,
		ONCOLLSION,
		ONMISS,
		ONMOVE,
		ONROTATE,
		ONSCALE,
		ONTRANSLATE
	};

	class DOOM_API EventSystem {
	private:

		multimap<EventType, Listener*> database;

		pair<multimap<EventType, Listener*>::iterator,
			multimap<EventType, Listener*>::iterator> range;

		multimap<EventType, Listener*>::iterator iter;

		std::queue<Event> currentEvents;
		mutable std::mutex mtx, mtx1;
		std::condition_variable cond;
		bool process_events = true;

		void DispatchEvent(Event* event);

		EventSystem() {}

		EventSystem& operator=(const EventSystem& rhs) { return *this; }

	public:

		void StopProcessEvents(bool value);

		int GetAmountOfEvents() { return currentEvents.size(); }

		bool AlreadyRegistered(EventType eventId, Listener* client);

		EventSystem(EventSystem& rhs) {}

		~EventSystem() { this->Shutdown(); }

		static EventSystem* GetInstance();

		void RegisterClient(EventType event, Listener* client);

		void UnregisterClient(EventType event, Listener* client);

		void UnregisterAll(Listener* client);

		void SendEvent(EventType eventId, Listener* sender, void* data = 0);

		void ProcessEvents();

		void ClearEvents();

		void Shutdown();
	};

	class DOOM_API MainThread : Listener {
	private:
		Task task;
	public:
		
		MainThread() {
			EventSystem::GetInstance()->RegisterClient(EventType::ONMAINTHREADPROCESS, (Listener*)this);
		}

		static void Init() {
			static MainThread mainthread;
		}

		static MainThread* GetInstance() {
			static MainThread mainthread; return &mainthread;
		}

		virtual void OnMainThreadProcess(void* _task) override {
			task = *static_cast<Task*>(_task);
			task();
			delete _task;
		}
	};

}

#endif // _EVENTSYSTEM_H_