#undef NDEBUG
#include <boost/asio.hpp>
#include <ellis/core/emigration.hpp>
#include <ellis/core/immigration.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/system.hpp>
#include <ellis/core/node.hpp>
#include <ellis/core/err.hpp>
#include <ellis/codec/json.hpp>
#include <ellis_private/using.hpp>
#include <thread>

namespace ellis {
namespace ex {


/**
 * Unique identifier for a request.
 *
 * This is used for tracking a request across systems, though proxies may
 * prepend a prefix, or remap, for their own purposes (the latter is likely
 * to be safer).  The scheduler is responsible for guaranteeing that
 * these identifiers are unique amongst currently outstanding requests.
 *
 * Design note: a fixed size integer, say 64 bits, might be slightly more
 * efficient, but we are assuming that the overhead of encoding as a string
 * will be relatively low in terms of overall bandwidth given that we will
 * generally be choosing reliability and flexibility over the complications
 * associated with minor speed improvements.  Using a string may allow for
 * easier debugging, and gives proxies options besides request ID translation
 * for dealing with collisions from multiple connections.
 */
using req_id = string;

req_id autogenerate_req_id()
{
  // TODO: better random string, maybe check with scheduler for unique.
  return std::to_string((unsigned long)random());
}

// TODO: no separate request class? make_request() function instead...

/**
 * A req is fundamentally an ellis::node, but provides additional
 * guarantees and features.  A req is guaranteed to include the following
 * fields:
 *
 * - id
 * - mod
 * - proc
 * - params
 *
 * It may also optionally include:
 *
 * - is_tracing
 */
class request {
public:
  node m_val;
  req_id m_id;
  string m_debugid;
  request(const string &mod,
      const string &proc,
      const node &params,
      const req_id &idparam = autogenerate_req_id(),
      bool is_tracing = false)
    : m_val(::ellis::type::MAP),
      m_id(idparam)
  {
    auto & a = m_val.as_mutable_map();
    a.insert("id", m_id);
    a.insert("mod", mod);
    a.insert("proc", proc);
    a.insert("params", params);
    a.insert("is_tracing", is_tracing);
    m_debugid = string("REQ:") + mod + "/" + proc + "[" + m_id + "]";
  }
  const node &get_field(const string &key)
  {
    return m_val.as_map()[key];
  }
  string debugid() { return m_debugid; }
  req_id id() { return m_id; }
};

/**
 * A response is fundamentally an ellis::node, but provides additional
 * guarantees and features.  A response is guaranteed to include the following
 * fields:
 *
 * - id
 * - mod
 * - proc
 * - nanos
 * - load
 *
 * It will include exactly one of the following:
 *
 * - error
 * - result
 *
 * It may also optionally include:
 *
 * - trace_info
 */
class response {
public:
  node m_val;
  req_id m_id;
  string m_debugid;
  response(const request &req_model)
     : m_val(::ellis::type::MAP)
  {
    auto & a = m_val.as_mutable_map();
    auto &ra = req_model.m_val.as_map();
    auto idarg = (const char *)ra["id"];
    auto mod = (const char *)ra["mod"];
    auto proc = (const char *)ra["proc"];
    a.insert("id", idarg);
    a.insert("mod", mod);
    a.insert("proc", proc);
    m_id = idarg;
    m_debugid = string("RESP:") + mod + "/" + proc + "[" + m_id + "]";
  }
  const node &get_field(const string &key)
  {
    return m_val.as_map()[key];
  }
  string debugid() { return m_debugid; }
  req_id id() { return m_id; }
};

/**
 * Time representation.
 *
 * Hopefully reduces misunderstandings and conversion accidents.
 */
struct timeval {
  int64_t m_nanos_since_epoch;
  explicit operator int64_t() const {
    return m_nanos_since_epoch;
  }
  timeval & operator=(const timeval &o) {
    m_nanos_since_epoch = o.m_nanos_since_epoch;
    return *this;
  }
  bool operator==(const timeval &o) {
    return m_nanos_since_epoch == o.m_nanos_since_epoch;
  }
  bool operator!=(const timeval &o) {
    return m_nanos_since_epoch != o.m_nanos_since_epoch;
  }
  bool operator>=(const timeval &o) {
    return m_nanos_since_epoch >= o.m_nanos_since_epoch;
  }
  bool operator<=(const timeval &o) {
    return m_nanos_since_epoch <= o.m_nanos_since_epoch;
  }
  bool operator<(const timeval &o) {
    return m_nanos_since_epoch < o.m_nanos_since_epoch;
  }
  bool operator>(const timeval &o) {
    return m_nanos_since_epoch > o.m_nanos_since_epoch;
  }
};

timeval operator+(const timeval &a, const timeval &b)
{
  return { a.m_nanos_since_epoch + b.m_nanos_since_epoch };
}

timeval operator-(const timeval &a, const timeval &b)
{
  return { a.m_nanos_since_epoch - b.m_nanos_since_epoch };
}


timeval now()
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME_COARSE, &ts);
  return { ts.tv_sec * 1000000000L + ts.tv_nsec };
}

/**
 * An element in a call execution trace.
 */
struct trace_entry {
  timeval m_time;
  string m_stage;
};

/**
 * Timing/tracing info for a single call.
 */
struct call_telemetry {
  timeval m_start_time;
  int64_t m_total_nanos = 0;
  bool m_is_tracing = false;
  vector<trace_entry> m_traces;
};

/**
 * Macro for execution tracing.
 *
 * We use a macro here, so that if tracing is not enabled, the cost is
 * minimized.
 *
 * Similar to regular logging, but takes a "stage" parameter, which must be a
 * simple character string constant (not a C++ string), and can be used for
 * building timing charts.
 */
#define ELLISEXTRACE(REQ, TELEM, STAGE) \
  do { \
    if ((TELEM)->m_is_tracing) { \
      (TELEM)->m_traces.push_back({ now(), STAGE }); \
    } \
    ELLIS_LOG(DBUG, "%s: stage -> %s", \
        (REQ)->debugid().c_str(), STAGE); \
  } while (0)

/**
 * The owner of the data associated with a procedure call, while in progress.
 *
 * This object is guaranteed to live until all the responses have been
 * consumed by callbacks.
 */
class call {
public:
  unique_ptr<request> m_req;
  unique_ptr<response> m_resp;
  call_telemetry m_telemetry;
public:
  call(unique_ptr<request> req, bool is_tracing = true) :
    m_req(std::move(req))
  {
    m_telemetry.m_is_tracing = is_tracing;
    m_telemetry.m_start_time = now();
    ELLISEXTRACE(m_req, &m_telemetry, "CREATED");
  }
  void complete(unique_ptr<response> resp)
  {
    m_resp = std::move(resp);
    ELLISEXTRACE(m_req, &m_telemetry, "COMPLETED");
    m_telemetry.m_total_nanos = (int64_t)(now() - m_telemetry.m_start_time);
  }
};

/**
 * Exponential histogram, which records a histogram of the exponents
 * of values between 2**0 and 2**63.
 */
class exp_histogram {
  int64_t m_counts[64];
};

/**
 * General stats object, for keeping track of count and sum, can do
 * average, min, max, population variance, and histogram.
 */
class stats {
  int64_t m_count = 0;
  int64_t m_sum = 0;
  int64_t m_sum_square = 0;
  int64_t m_min = 0;
  int64_t m_max = 0;
  exp_histogram m_hist;
};

/**
 * A function to be called by a procedure implementation when it has
 * completed the response.
 */
using procedure_imp_cb = std::function<void(
    unique_ptr<response> resp)>;

/**
 * A procedure implementation.
 *
 * This is the function that does the work of computing the relevant response
 * for a given request.
 *
 * When complete, the implementation should call the complete() function on
 * the call object.
 */
using procedure_imp = std::function<void(
    call *c,
    const procedure_imp_cb &cb
    )>;

/**
 * Information that varies per procedure.
 */
struct procedure_info {
  string m_name;
  procedure_imp m_imp;
  // TODO: node m_schema;
  stats m_stats;  // since system start, for this procedure
  int64_t m_num_in_flight = 0;
  // TODO: actually update and use the stats

  procedure_info(const string &name, const procedure_imp &imp) :
    m_name(name),
    m_imp(imp)
  {
  }
};

/**
 * A module is a collection of procedures and state that relate to a
 * particular type of functionality.
 */
class module_info {
  string m_name;
  map<string, procedure_info> m_procmap;
public:
  module_info(const string &name) : m_name(name)
  {
  }

  const string &get_name() const
  {
    return m_name;
  }

  /**
   * Add procedure info.
   */
  void add_procedure(const string &procname, const procedure_imp &imp)
  {
    m_procmap.emplace(procname, procedure_info(procname, imp));
  }

  /**
   * Lookup procedure info for the named procedure.
   *
   * Throws std::out_of_range if no such procedure name is known.
   */
  procedure_info &lookup_procedure_info(const string &procname)
  {
    return m_procmap.at(procname);
  }
};

/**
 * Callback function type.  This is the function that an async caller of
 * a procedure will provide, for notification when the procedure is finished
 * and the response ready, etc.
 *
 * Callee does not own the call (or by extension the request, response, or
 * call_telemetry contained inside).  Callee may assume that they will exist
 * until the callback function has finished, but must not assume that those
 * objects will continue to exist afterwards.
 */
using callback = std::function<void(call *)>;

/**
 * Config file example:
 *
 * {
 *   "module": {
 *     "video" : {
 *       "handler" : "video_server.so",
 *       "config" : { "synthesize_keyframes" : false }
 *     },
 *     "docdb" : {
 *       "handler" : "proxy.so",
 *       "config" : { "uris" : [ "http://w.x.y.z:port" ] }
 *     }
 *   }
 * }
 */

/**
 * This object schedules execution of calls, and arranges for cleanup on
 * completion.
 *
 * It maintains call lifetime by way of a shared_ptr that is bound by a
 * lambda, which is lower latency than having to synchronize manipulation
 * of a shared table, though it doesn't give us a way to ask about what calls
 * are in flight at any given moment.  If such features is later desired, we
 * can add such a table.
 *
 * At startup, there is one call tracker per system, which all threads
 * share.  Operations on this table, other than construction and destruction,
 * must be thread safe.  All outstanding calls tracked by this table are
 * guaranteed to have unique request ids.  If there are remote connections
 * made for a particular module (for forwarding requests to a remote server),
 * we'll share one connection per alias (e.g. server and role/credential).
 *
 * For now: always one copy, fossilized at startup.  In the future, as needed
 * for perf optimization: RCU map from thread local id to entry in vector of
 * scheduler that expands on demand.  Similarly, the internal thread
 * scheduling system may be changed in the future for higher performance.
 */
class scheduler {
  vector<std::thread> m_worker_threads;
  map<string, module_info> m_modules;
  boost::asio::io_service m_ios;
  boost::asio::io_service::work m_work;
  mutex m_sync_mu;
  condition_variable m_sync_cv;

public:
  scheduler() :
    m_ios(),
    m_work(m_ios)
  {
  }

  /* Not copyable. */
  scheduler(const scheduler &) = delete;
  scheduler & operator= (const scheduler &) = delete;

  ~scheduler()
  {
  }

  void add_module(const module_info &mi) {
    m_modules.emplace(mi.get_name(), mi);
  }

  void start()
  {
    // TODO: something less arbitrary here--depends on whether we will try to
    // preferentially isolate separate scheduler per thread and improve on
    // boost_asio's default semantics, external interactions, whether emphasis
    // would be on internal work vs RPC to server, etc.
    int num_threads = std::thread::hardware_concurrency();
    for (int i = 0; i < num_threads; i++) {
      m_worker_threads.emplace_back(
          [this, i]
          ()
          {
            ELLIS_LOG(DBUG, "worker thread %d starting", i);
            m_ios.run();
            ELLIS_LOG(DBUG, "worker thread %d stopping", i);
          });
    }
  }

  void stop()
  {
    m_ios.stop();
    // TODO: or should we do the work reset thing?  either way, what about
    // outstanding calls at stop() time?
    for (auto & thr : m_worker_threads) {
      thr.join();
    }
    m_worker_threads.clear();
  }

  void submit_async(
      unique_ptr<request> req,
      const callback &cb)
  {
    // TODO: replace std::out_of_range with different exception?
    const char *modname = (const char *)(req->get_field("mod"));
    ELLIS_LOG(DBUG, "looking up module %s", modname);
    auto &m = m_modules.at(modname);
    const char *procname = (const char *)(req->get_field("proc"));
    ELLIS_LOG(DBUG, "looking up procedure %s", procname);
    auto &procinfo = m.lookup_procedure_info(procname);
    /* TODO: Validate request. */
    auto c = make_shared<call>(std::move(req));
    procedure_imp_cb finish =
      [this, cb, c]
      (unique_ptr<response> resp)
      {
        c->complete(std::move(resp));
        cb(c.get());
      };
    m_ios.post(
        [this, finish, c, &procinfo]
        ()
        {
          ELLISEXTRACE(c->m_req, &c->m_telemetry, "CALL_RUN_IMP");
          (procinfo.m_imp)(c.get(), finish);
        });
  }

  unique_ptr<response> do_sync(
      unique_ptr<request> req)
  {
    unique_ptr<response> awaited_response;

    auto wake_func =
      [this, &awaited_response]
      (call *c)
      {
        ELLIS_LOG(DBUG, "waking up sync call");
        unique_lock<mutex> lk(m_sync_mu);
        /* Capture response to the awaited_response variable. */
        awaited_response = std::move(c->m_resp);
        lk.unlock();
        m_sync_cv.notify_one();
      };

    /* Submit request asynchronously... */
    submit_async(
        std::move(req),
        wake_func);

    /* Wait for completion signalling. */
    // TODO: improve all these log messages.
    ELLIS_LOG(DBUG, "sync call waiting for result");
    unique_lock<mutex> lk(m_sync_mu);
    m_sync_cv.wait(lk,
        [this, &awaited_response]()
        {
          /* Has the awaited response been furnished? */
          return (bool)awaited_response;
        });

    return awaited_response;
  }
};


}  /* namespace ::ellis::ex */
}  /* namespace ::ellis */

int main() {

  using namespace ::ellis::ex;
  using namespace ::ellis;

  set_system_log_prefilter(log_severity::DBUG);
  module_info hm("hello");
  hm.add_procedure("world",
      []
      (call *c, const procedure_imp_cb &cb)
      {
        /* Count the square of request's x parameter. */
        auto x = c->m_req->m_val.at_path("{params}{x}").as_int64();
        auto resp = make_unique<response>(*(c->m_req));
        resp->m_val.as_mutable_map().insert("result", x*x);
        cb(std::move(resp));
      });
  scheduler sched;
  sched.start();
  sched.add_module(hm);
  node params(type::MAP);
  params.as_mutable_map().insert("x", 8);
  auto resp = sched.do_sync(make_unique<request>(
        "hello",
        "world",
        params
        ));
  sched.stop();
  return 0;
}
