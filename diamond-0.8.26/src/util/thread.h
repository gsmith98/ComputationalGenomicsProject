/****
Copyright (c) 2016, University of Tuebingen, Benjamin Buchfink
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****/

#ifndef THREAD_H_
#define THREAD_H_

#include <vector>
#include <exception>
#include <stdexcept>
#include "fast_mutex.h"
#include "tinythread.h"
#include "util.h"

using tthread::thread;
using std::vector;

template<typename _t>
struct Atomic
{
	Atomic(const _t &v):
		v_ (v)
	{ }
	volatile _t operator++(int)
	{
		mtx_.lock();
		_t r = v_++;
		mtx_.unlock();
		return r;
	}
	_t operator--(int)
	{
		mtx_.lock();
		_t r = v_--;
		mtx_.unlock();
		return r;
	}
private:
	volatile _t v_;
	tthread::mutex mtx_;
};

template<typename _context>
struct Thread_p
{
	Thread_p(unsigned thread_id, _context &context):
		thread_id (thread_id),
		context (&context)
	{ }
	unsigned thread_id;
	_context *context;
};

template<typename _context>
void pool_worker(void *p)
{
	((Thread_p<_context>*)p)->context->operator()(((Thread_p<_context>*)p)->thread_id);
	TLS::clear();
}

template<typename _context>
void launch_thread_pool(_context &context, unsigned threads)
{
	vector<tthread::thread*> t;
	vector<Thread_p<_context> > p;
	p.reserve(threads);
	unsigned n = 0;
	for(unsigned i=0;i<threads;++i) {
		p.push_back(Thread_p<_context> (i, context));
		t.push_back(new tthread::thread(pool_worker<_context>, (void*)&p.back()));
		n += t.back()->get_id() == tthread::thread::id () ? 0 : 1;
	}
	for(vector<tthread::thread*>::iterator i=t.begin();i!=t.end();++i) {
		(*i)->join();
		delete *i;
	}
	if(n != threads)
		throw std::runtime_error("Failed to create thread.");
}

template<typename _context>
struct Schedule_context
{
	Schedule_context(_context &context, unsigned count):
		context (context),
		n (0),
		count (count)
	{ }
	void operator()(unsigned thread_id)
	{
		unsigned idx;
		while((idx = n++) < count)
			context(thread_id, idx);
	}
	_context &context;
	Atomic<unsigned> n;
	const unsigned count;
};

template<typename _context>
void launch_scheduled_thread_pool(_context &context, unsigned count, unsigned threads)
{
	Schedule_context<_context> c (context, count);
	launch_thread_pool(c, threads);
}

template<typename _f, typename _t1>
struct Thread_p1
{
	Thread_p1(_f f, _t1 p1) :
		f(f),
		p1(p1)
	{ }
	_f f;
	_t1 p1;
};

template<typename _f, typename _t1>
void thread_worker(void *p)
{
	Thread_p1<_f, _t1> *q = (Thread_p1<_f, _t1>*)p;
	q->f(q->p1);
	delete q;
	TLS::clear();
}

template<typename _f, typename _t1>
thread* launch_thread(_f f, _t1 p1)
{
	return new thread(thread_worker<_f, _t1>, new Thread_p1<_f, _t1>(f, p1));
}

template<typename _f, typename _t1, typename _t2, typename _t3>
struct Thread_p3
{
	Thread_p3(_f f, _t1 p1, _t2 p2, _t3 p3) :
		f(f),
		p1(p1),
		p2(p2),
		p3(p3)
	{ }
	_f f;
	_t1 p1;
	_t2 p2;
	_t3 p3;
};

template<typename _f, typename _t1, typename _t2, typename _t3>
void thread_worker(void *p)
{
	Thread_p3<_f, _t1, _t2, _t3> *q = (Thread_p3<_f, _t1, _t2, _t3>*)p;
	q->f(q->p1, q->p2, q->p3);
	delete q;
	TLS::clear();
}

template<typename _f, typename _t1, typename _t2, typename _t3>
thread* launch_thread(_f f, _t1 p1, _t2 p2, _t3 p3)
{
	return new thread(thread_worker<_f, _t1, _t2, _t3>, new Thread_p3<_f, _t1, _t2, _t3>(f, p1, p2, p3));
}

template<typename _f, typename _t1, typename _t2, typename _t3, typename _t4>
struct Thread_p4
{
	Thread_p4(_f f, _t1 p1, _t2 p2, _t3 p3, _t4 p4):
		f (f),
		p1 (p1),
		p2 (p2),
		p3 (p3),
		p4 (p4)
	{ }
	_f f;
	_t1 p1;
	_t2 p2;
	_t3 p3;
	_t4 p4;
};

template<typename _f, typename _t1, typename _t2, typename _t3, typename _t4>
void thread_worker(void *p)
{
	Thread_p4<_f,_t1,_t2,_t3,_t4> *q = (Thread_p4<_f,_t1,_t2,_t3,_t4>*)p;
	q->f(q->p1, q->p2, q->p3, q->p4);
	delete q;
	TLS::clear();
}

template<typename _f, typename _t1, typename _t2, typename _t3, typename _t4>
thread* launch_thread(_f f, _t1 p1, _t2 p2, _t3 p3, _t4 p4)
{ return new thread (thread_worker<_f,_t1,_t2,_t3,_t4>, new Thread_p4<_f,_t1,_t2,_t3,_t4> (f, p1, p2, p3, p4)); }

template<typename _f, typename _t1, typename _t2, typename _t3, typename _t4, typename _t5>
struct Thread_p5
{
	Thread_p5(_f f, _t1 p1, _t2 p2, _t3 p3, _t4 p4, _t5 p5) :
		f(f),
		p1(p1),
		p2(p2),
		p3(p3),
		p4(p4),
		p5(p5)
	{ }
	_f f;
	_t1 p1;
	_t2 p2;
	_t3 p3;
	_t4 p4;
	_t5 p5;
};

template<typename _f, typename _t1, typename _t2, typename _t3, typename _t4, typename _t5>
void thread_worker(void *p)
{
	Thread_p5<_f, _t1, _t2, _t3, _t4, _t5> *q = (Thread_p5<_f, _t1, _t2, _t3, _t4, _t5>*)p;
	q->f(q->p1, q->p2, q->p3, q->p4, q->p5);
	delete q;
	TLS::clear();
}

template<typename _f, typename _t1, typename _t2, typename _t3, typename _t4, typename _t5>
thread* launch_thread(_f f, _t1 p1, _t2 p2, _t3 p3, _t4 p4, _t5 p5)
{
	return new thread(thread_worker<_f, _t1, _t2, _t3, _t4, _t5>, new Thread_p5<_f, _t1, _t2, _t3, _t4, _t5>(f, p1, p2, p3, p4, p5));
}


struct Thread_pool : public vector<thread*>
{
	void join_all()
	{
		for (iterator i = begin(); i != end(); ++i) {
			(*i)->join();
			delete *i;
		}
	}
};

#endif /* THREAD_H_ */
