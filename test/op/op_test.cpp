/*
 * Copyright (c) 2016 Surround.IO Corporation. All Rights Reserved.
 * Copyright (c) 2017 Xevo Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#undef NDEBUG
#include <atomic>
#include <boost/asio.hpp>
#include <ellis/core/array_node.hpp>
#include <ellis/core/emigration.hpp>
#include <ellis/core/immigration.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/system.hpp>
#include <ellis/core/node.hpp>
#include <ellis/core/err.hpp>
#include <ellis/codec/json.hpp>
#include <ellis_private/using.hpp>
#include <thread>

/* Needed for server. */
#include <nghttp2/asio_http2.h>
#include <nghttp2/asio_http2_client.h>
#include <nghttp2/asio_http2_server.h>

namespace ellis {
namespace op {

using std::atomic;


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

/**
 * Make a request, which is an ellis::node with a particular structure.
 *
 * A proper request will always have the following fields:
 *
 * - id
 * - module
 * - procedure
 * - params
 *
 * It may also optionally include:
 *
 * - is_tracing
 */
unique_ptr<node> make_request(
    const string &mod,
    const string &proc,
    const node &params,
    const req_id &id = autogenerate_req_id(),
    bool is_tracing = false)
{
  auto n = make_unique<node>(::ellis::type::MAP);
  auto & a = n->as_mutable_map();
  a.insert("id", id);
  a.insert("module", mod);
  a.insert("procedure", proc);
  a.insert("params", params);
  a.insert("is_tracing", is_tracing);
  return n;
}

/**
 * Make a response, patterned off of a request.
 *
 * A response is an ellis::node with a particular structure...
 * A proper response will always have the following fields:
 *
 * - id
 * - module
 * - procedure
 * - nanos
 *
 * It will eventually include exactly one of the following:
 *
 * - error
 * - result
 *
 * It may also optionally include:
 *
 * - trace_info
 */
unique_ptr<node> make_response(
    const node &req_model)
{
  auto n = make_unique<node>(::ellis::type::MAP);
  auto & a = n->as_mutable_map();
  auto &ra = req_model.as_map();
  auto id = (const char *)ra["id"];
  auto mod = (const char *)ra["module"];
  auto proc = (const char *)ra["procedure"];
  a.insert("id", id);
  a.insert("module", mod);
  a.insert("procedure", proc);
  return n;
}

/**
 * Convenience function to get access a particular field of a request or
 * response and convert it to const char pointer.
 */
const char * get_strfield(const node &n, const char *key)
{
  return (const char *)(n.as_map()[key]);
}

/**
 * Convenience function to get make a debug-friendly label for a request or
 * response, using its method, procedure, and id fields.
 */
string get_debugid(const node &n)
{
  return ELLIS_SSTRING(get_strfield(n, "module")
      << "/" << get_strfield(n, "procedure")
      << "[" << get_strfield(n, "id") << "]");
}

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
#define ELLIS_OP_TRACE(REQ, TELEM, STAGE) \
  do { \
    if ((TELEM)->m_is_tracing) { \
      (TELEM)->m_traces.push_back({ now(), STAGE }); \
    } \
    ELLIS_LOG(DBUG, "%s: stage -> %s", \
        get_debugid(*(REQ)).c_str(), STAGE); \
  } while (0)

/**
 * The owner of the data associated with a procedure call, while in progress.
 *
 * This object is guaranteed to live until all the responses have been
 * consumed by callbacks.
 */
class call {
public:
  unique_ptr<node> m_req;
  unique_ptr<node> m_resp;
  call_telemetry m_telemetry;
public:
  call(unique_ptr<node> req, bool is_tracing = true) :
    m_req(std::move(req))
  {
    m_telemetry.m_is_tracing = is_tracing;
    m_telemetry.m_start_time = now();
    ELLIS_OP_TRACE(m_req, &m_telemetry, "CREATED");
  }
  void complete(unique_ptr<node> resp)
  {
    m_resp = std::move(resp);
    ELLIS_OP_TRACE(m_req, &m_telemetry, "COMPLETED");
    m_telemetry.m_total_nanos = (int64_t)(now() - m_telemetry.m_start_time);
    m_resp->as_mutable_map().insert("nanos", m_telemetry.m_total_nanos);
    if (m_telemetry.m_is_tracing) {
      auto trnode = node(ellis::type::ARRAY);
      auto & trnodea = trnode.as_mutable_array();
      for (auto &trent : m_telemetry.m_traces) {
        auto ent = node(ellis::type::ARRAY);
        ent.as_mutable_array().append(trent.m_stage);
        ent.as_mutable_array().append(trent.m_time.m_nanos_since_epoch);
        trnodea.append(ent);
      }
      m_resp->as_mutable_map().insert("trace_info", trnode);
    }
  }
};

/**
 * Exponential histogram, which records a histogram of the exponents
 * of values between 2**0 and 2**63.
 */
struct exp_histogram {
  int64_t m_counts[64];
};

/**
 * General stats object, for keeping track of count and sum, can do
 * average, min, max, population variance, and histogram.
 */
struct stats {
  int64_t m_count = 0;
  int64_t m_sum = 0;
  int64_t m_sum_square = 0;
  int64_t m_min = 0;
  int64_t m_max = 0;
  exp_histogram m_hist;
  void update(int64_t v)
  {
    m_count++;
    m_sum += v;
    m_sum_square += v*v;
    if (m_min > v) {
      m_min = v;
    }
    if (m_max < v) {
      m_max = v;
    }
  }
};

/**
 * A function to be called by a procedure implementation when it has
 * completed the response.
 */
using procedure_imp_cb = std::function<void(
    unique_ptr<node> resp)>;

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
  mutex m_stat_mutex;
  stats m_stats;  // since system start, for this procedure
  atomic<int64_t> m_num_in_flight {0};
  // TODO: actually update and use the stats

  procedure_info(const string &name, const procedure_imp &imp) :
    m_name(name),
    m_imp(imp)
  {
  }

  procedure_info(const procedure_info &) = delete;
  procedure_info & operator=(const procedure_info &) = delete;

  void update_stats(int64_t v)
  {
    unique_lock<mutex> lk(m_stat_mutex);
    m_stats.update(v);
  }
};

/**
 * A module is a collection of procedures and state that relate to a
 * particular type of functionality.
 */
class module_info {
  string m_name;
  map<string, unique_ptr<procedure_info>> m_procmap;
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
    m_procmap.emplace(procname, make_unique<procedure_info>(procname, imp));
  }

  /**
   * Lookup procedure info for the named procedure.
   *
   * Throws std::out_of_range if no such procedure name is known.
   */
  procedure_info &lookup_procedure_info(const string &procname)
  {
    return *(m_procmap.at(procname));
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
 * for perf optimization: RCU map from thread local index to entry in vector of
 * scheduler that expands on demand.  Similarly, the internal thread
 * scheduling system may be changed in the future for higher performance.
 */
class scheduler {
  vector<std::thread> m_worker_threads;
  map<string, unique_ptr<module_info>> m_modules;
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

  void add_module(unique_ptr<module_info> mi) {
    m_modules.emplace(mi->get_name(), std::move(mi));
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
      unique_ptr<node> req,
      const callback &cb)
  {
    // TODO: replace std::out_of_range with different exception?
    const char *modname = get_strfield(*req, "module");
    ELLIS_LOG(DBUG, "looking up module %s", modname);
    auto &m = m_modules.at(modname);
    const char *procname = get_strfield(*req, "procedure");
    ELLIS_LOG(DBUG, "looking up procedure %s", procname);
    auto &procinfo = m->lookup_procedure_info(procname);
    procinfo.m_num_in_flight++;
    /* TODO: Validate request. */
    auto c = make_shared<call>(std::move(req));
    procedure_imp_cb finish =
      [this, cb, c, &procinfo]
      (unique_ptr<node> resp)
      {
        c->complete(std::move(resp));
        cb(c.get());
        procinfo.update_stats(c->m_telemetry.m_total_nanos);
        procinfo.m_num_in_flight--;
      };
    m_ios.post(
        [this, finish, c, &procinfo]
        ()
        {
          ELLIS_OP_TRACE(c->m_req, &c->m_telemetry, "CALL_RUN_IMP");
          (procinfo.m_imp)(c.get(), finish);
        });
  }

  unique_ptr<node> do_sync(
      unique_ptr<node> req)
  {
    unique_ptr<node> awaited_response;

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

namespace h2a = nghttp2::asio_http2;
namespace h2ac = nghttp2::asio_http2::client;
namespace h2as = nghttp2::asio_http2::server;

class h2_stream_handler {

  scheduler &m_sched;
  unique_ptr<decoder> m_deco;
  unique_ptr<encoder> m_enco;
  const h2as::request &m_h2req;
  const h2as::response &m_h2res;
  string m_path;
  node_progress m_st;
  unique_ptr<node> m_response_node;
  bool m_allow_chop = true;

public:
  h2_stream_handler(
      scheduler &sched,
      const h2as::request &h2req,
      const h2as::response &h2res) :
    m_sched(sched),
    m_deco(new json_decoder()),
    m_enco(new json_encoder()),
    m_h2req(h2req),
    m_h2res(h2res),
    m_st(stream_state::CONTINUE)
  {
    m_deco->reset();

    // TODO: don't hardcode decoder to json.
    // TODO: borrow from a pool of decoders rather than always creating.
    m_path = h2a::percent_decode(m_h2req.uri().path);
    // TODO: verify path, REST-compatibility logic, etc
  }

  void setup()
  {
    ELLIS_LOG(DBUG, "Setting up new h2 request handler.");
    /* First thing we do is make sure this stream state object will get
     * deleted when the stream is closed. */
    m_h2res.on_close(
        [this](uint32_t reason)
        {
          ELLIS_LOGSTREAM(DBUG, "Stream closed! reason: " << reason);
          delete this;
        });

    /* Set callback for data. */
    m_h2req.on_data(
        [this](const uint8_t *buf, size_t len)
        {
          ELLIS_LOG(DBUG, "Got data %zu bytes input payload:", len);
          if (len > 0) {
            if (m_st.state() == stream_state::CONTINUE) {
              /* Accumulate the data. */
              m_st = m_deco->consume_buffer((const byte *)buf, &len);
              if (m_st.state() == stream_state::SUCCESS) {
                m_allow_chop = false;
              }
            }
          }
          else {
            if (m_allow_chop) {
              m_st = m_deco->chop();
            }
            switch (m_st.state()) {
            case stream_state::ERROR:
              parse_fail(m_st.extract_error());
              return;

            case stream_state::CONTINUE:
              parse_fail(MAKE_UNIQUE_ELLIS_ERR(PARSE_FAIL,
                    "parse interrupted"));
              return;

            case stream_state::SUCCESS:
              parse_success(m_st.extract_value());
              return;
            }
          }
        });
  }
  void fill_header(
      unsigned int status_code,
      const std::initializer_list<pair<string,string>> &kvpairs)
  {
    ELLIS_LOG(DBUG, "Setting %u response status code", status_code);
    /* Set up h2 response headers. */
    auto hdrs = h2a::header_map();
    for (const auto &kv : kvpairs) {
      hdrs.emplace(kv.first, h2a::header_value{kv.second, false});
    }
    m_h2res.write_head(status_code, std::move(hdrs));
  }
  void parse_fail(unique_ptr<err> e)
  {
    string errtxt = e->summary();
    ELLIS_LOG(DBUG, "Sending parse failure response, error was %s",
        errtxt.c_str());
    fill_header(400, {
        {"Content-Type", "text/plain"},
        {"Content-Length", std::to_string(errtxt.size())}
        });
    m_h2res.end(errtxt);
  }
  void parse_success(unique_ptr<node> req)
  {
    ELLIS_LOG(DBUG, "Parse was successful; executing request.");
    m_sched.submit_async(std::move(req),
        [this]
        (call *c)
        {
          m_response_node = std::move(c->m_resp);
          got_response();
        });
  }
  void got_response()
  {
    ELLIS_LOG(DBUG, "Request finished executing; initiating repsonse.");
    // TODO: don't hardcode encoder type.
    fill_header(200, { {"Content-Type", "application/json" }});
    m_enco.reset(new json_encoder());
    m_enco->reset(m_response_node.get());
    h2a::generator_cb cb =
      [this]
      (uint8_t *buf, std::size_t len, uint32_t *data_flags)
      {
        ELLIS_LOG(DBUG, "Got output buffer, size %zu bytes", len);
        auto orig_len = len;
        auto st = m_enco->fill_buffer((byte *)buf, &len);
        switch (st.state()) {
        case stream_state::SUCCESS:
          ELLIS_LOG(DBUG, "Encoding successful, %zu bytes leftover", len);
          // TODO: don't hardcode content-type
          //fill_header(200, { "Content-Type", "application/json" });
          *data_flags = NGHTTP2_DATA_FLAG_EOF;
          return orig_len - len;
        case stream_state::ERROR:
          ELLIS_LOGSTREAM(DBUG, "Encoding error, "
              << st.extract_error()->summary());
          // TODO: how to handle error here?  I think we already have to have
          // set the headers before we call end() on the h2 response, in which
          // case how do we guard against partial encoding?
          //fill_header(500);
          *data_flags = NGHTTP2_DATA_FLAG_EOF;
          return (size_t)NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
        case stream_state::CONTINUE:
          ELLIS_LOG(DBUG, "Encoding used buffer completely, needs more...");
          *data_flags = 0;
          return orig_len - len;
        }
        ELLIS_ASSERT_UNREACHABLE();
      };
    m_h2res.end(cb);
  }
};

class h2_server {
  scheduler &m_sched;
  h2as::http2 m_srv;
  string m_ipaddr;
  string m_port;
  bool m_encrypted;
  string m_ssl_key_file;
  string m_ssl_cert_chain_file;
  boost::asio::ssl::context m_tls;
public:
  h2_server(
      scheduler &sched,
      const char *ipaddr,
      const char *port) :
    m_sched(sched),
    m_srv(),
    m_ipaddr(ipaddr),
    m_port(port),
    m_encrypted(false),
    m_ssl_key_file(),
    m_ssl_cert_chain_file(),
    m_tls(boost::asio::ssl::context::sslv23)
  {
  }
  h2_server(
      scheduler &sched,
      const char *ipaddr,
      const char *port,
      const char *ssl_key_file,
      const char *ssl_cert_chain_file) :
    m_sched(sched),
    m_srv(),
    m_ipaddr(ipaddr),
    m_port(port),
    m_encrypted(true),
    m_ssl_key_file(ssl_key_file),
    m_ssl_cert_chain_file(ssl_cert_chain_file),
    m_tls(boost::asio::ssl::context::sslv23)
  {
  }
  void start()
  {
    m_srv.num_threads(2);
    if (! m_srv.handle("/",
          [this]
          (const h2as::request &h2req, const h2as::response &h2res)
          {
            ELLIS_LOG(DBUG, "Creating new h2 stream handler.");
            auto rh = new h2_stream_handler(m_sched, h2req, h2res);
            rh->setup();
          })) {
      ELLIS_LOG(DBUG, "Oh no, handler registration failure.");
    }
    boost::system::error_code ec;
    if (m_encrypted) {
      ELLIS_LOG(DBUG, "Starting SSL server");
      m_tls.use_private_key_file(
          m_ssl_key_file.c_str(),
          boost::asio::ssl::context::pem);
      m_tls.use_certificate_chain_file(
          m_ssl_cert_chain_file.c_str());
      h2as::configure_tls_context_easy(ec, m_tls);
      if (m_srv.listen_and_serve(ec, m_tls, m_ipaddr, m_port, true)) {
        ELLIS_LOGSTREAM(DBUG, "error: " << ec.message());
      }
    }
    else {
      ELLIS_LOG(DBUG, "Starting non-SSL server");
      if (m_srv.listen_and_serve(ec, m_ipaddr, m_port, true)) {
        ELLIS_LOGSTREAM(DBUG, "error: " << ec.message());
      }
    }
  }
  void stop()
  {
    ELLIS_LOG(DBUG, "Stopping h2 server...");
    m_srv.stop();
  }
  void join()
  {
    ELLIS_LOG(DBUG, "Waiting for server shutdown...");
    m_srv.join();
  }
};

/**
 * An h2 client, for forwarding ellis op requests to a particular h2 server.
 */
class h2_client {
  enum class connect_state {
    NOT_CONNECTED,
    CONNECTING,
    CONNECTED,
    DISCONNECTING
  };
  boost::asio::io_service m_ios;
  vector<std::thread> m_worker_threads;
  unique_ptr<h2ac::session> m_sess;
  string m_ipaddr;
  string m_port;
  //bool m_encrypted;
  //string m_ssl_key_file;
  //string m_ssl_cert_chain_file;
  //boost::asio::ssl::context m_tls;
  mutex m_mutex;
  connect_state m_connect_state = connect_state::NOT_CONNECTED;
  deque<unique_ptr<node>> m_req_q;

public:
  h2_client(
      const char *ipaddr,
      const char *port) :
    m_ios(),
    m_worker_threads(),
    m_ipaddr(ipaddr),
    m_port(port)
    //m_encrypted(false),
    //m_ssl_key_file(),
    //m_ssl_cert_chain_file(),
    //m_tls(boost::asio::ssl::context::sslv23)
  {
    // TODO: don't automatically connect on construction; add logic to connect
    // on demand and disconnect after timeout.
    connect();
  }
  ~h2_client()
  {
    // TODO: assert disconnected?
  }
  void connect()
  {
    /* Update state. */
    {
      unique_lock<mutex> lk(m_mutex);
      m_connect_state = connect_state::CONNECTING;
    }
    ELLIS_LOG(DBUG, "Connecting to host %s port %s",
        m_ipaddr.c_str(), m_port.c_str());
    // TODO: session constructor can throw an exception.  What do we do when
    // a request can not be handled due to connection failure?  I guess we
    // reset connect retry timer--keep trying, don't fail the requests...
    // Possibly have a max tries parameter, defaults to forever...
    m_sess = make_unique<h2ac::session>(m_ios, m_ipaddr, m_port);
    m_sess->on_connect(
        [this]
        (boost::asio::ip::tcp::resolver::iterator endpoint_it)
        {
          std::cout << (*endpoint_it).endpoint();
          ELLIS_LOGSTREAM(DBUG, "connected to " << (*endpoint_it).endpoint());
          unique_lock<mutex> lk(m_mutex);
          m_connect_state = connect_state::CONNECTING;
          for (auto & c : m_req_q) {
            ELLIS_LOG(DBUG, "unqueued request: %s",
                get_debugid(*c).c_str());
            // TODO
            assert(0);
            /* submit */
          }
          m_req_q.clear();
        });
    int num_threads = 1;
    for (int i = 0; i < num_threads; i++) {
      m_worker_threads.emplace_back(
          [this, i]
          ()
          {
            ELLIS_LOG(DBUG, "h2_client worker thread %d starting", i);
            m_ios.run();
            ELLIS_LOG(DBUG, "h2_client worker thread %d stopping", i);
          });
    }
  }
  void disconnect()
  {
    ELLIS_LOG(DBUG, "Disconnecting h2_client...");
    // TODO: do I need to suspend/wait outstanding activity before shutdown?
    m_sess->shutdown();
    // TODO: verify that ios run will now exit on its own
    for (auto & thr : m_worker_threads) {
      thr.join();
    }
    m_worker_threads.clear();
  }
};

}  /* namespace ::ellis::op */
}  /* namespace ::ellis */

int main() {

  using namespace ::ellis::op;
  using namespace ::ellis;

  bool client_server_test = false;

  set_system_log_prefilter(log_severity::DBUG);
  auto hm = make_unique<module_info>("hello");
  hm->add_procedure("world",
      []
      (call *c, const procedure_imp_cb &cb)
      {
        /* Count the square of request's x parameter. */
        int64_t x = c->m_req->at("{params}{x}");
        auto resp = make_response(*(c->m_req));
        resp->install("{result}{x2}", x*x);
        cb(std::move(resp));
      });
  scheduler sched;
  sched.add_module(std::move(hm));
  sched.start();
  auto resp = sched.do_sync(make_request("hello", "world",
        {make_pair("x", 8)}));
  ELLIS_ASSERT_EQ(resp->at("{result}{x2}"), 64);

  if (client_server_test) {
    h2_server srv(sched, "127.0.0.1", "3350");
    srv.start();

    // block here
    srv.join();
  }

  sched.stop();
  return 0;
}
