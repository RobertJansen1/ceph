// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
#ifndef CEPH_LIBRBD_ASYNC_OBJECT_THROTTLE_H
#define CEPH_LIBRBD_ASYNC_OBJECT_THROTTLE_H

#include "include/int_types.h"
#include "include/Context.h"

#include <boost/function.hpp>
#include "include/assert.h"

namespace librbd
{
class ProgressContext;

class AsyncObjectThrottleFinisher {
public:
  virtual ~AsyncObjectThrottleFinisher() {};
  virtual void finish_op(int r) = 0;
};

class C_AsyncObjectThrottle : public Context {
public:
  C_AsyncObjectThrottle(AsyncObjectThrottleFinisher &finisher)
    : m_finisher(finisher)
  {
  }

  virtual void finish(int r)
  {
    m_finisher.finish_op(r);
  }

  virtual int send() = 0;

private:
  AsyncObjectThrottleFinisher &m_finisher;
};

class AsyncObjectThrottle : public AsyncObjectThrottleFinisher {
public:
  typedef boost::function<C_AsyncObjectThrottle*(AsyncObjectThrottle&,
      					   uint64_t)> ContextFactory;

  AsyncObjectThrottle(const ContextFactory& context_factory, Context *ctx,
		      ProgressContext &prog_ctx, uint64_t object_no,
		      uint64_t end_object_no);

  int start_ops(uint64_t max_concurrent);
  virtual void finish_op(int r);

private:
  Mutex m_lock;
  ContextFactory m_context_factory;
  Context *m_ctx;
  ProgressContext &m_prog_ctx;
  uint64_t m_object_no;
  uint64_t m_end_object_no;
  uint64_t m_current_ops;
  int m_ret;

  int start_next_op();
};

} // namespace librbd

#endif // CEPH_LIBRBD_ASYNC_OBJECT_THROTTLE_H
