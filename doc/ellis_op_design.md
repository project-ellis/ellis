# Ellis-Op Design

Ellis Op is an RPC-like system that uses ellis objects to represent requests
and responses, which can then be serialized as needed using any suitable ellis
codec.

A client has the option of interacting with an ellis-op server using only
basic REST primitives and no ellis-specific libraries, or it may use a more
comfortable infrastructure that has additional features and allows the process
to have its own registry of modules that will either communicate with a remote
server (over ellis protocol or otherwise) or handle the request directly.
That is to say, the code making the request just fills out a request, and
gets a response back, and the request might never have to be serialized if
it can be handled locally; a separate policy dictates how the particular
module and procedure are to be implemented--such as local or remote.

## REST: some positive points

* Universally available
  * pretty much any language supports it
  * works in mobile devices and browsers
  * http is usually allowed through firewalls
* Simpler than competition, not proprietary
* Loose coupling between parts of system
  * autonomy between teams is usually a good thing
* Focus on protocols, schemas
  * not required to use a particular implementation
* Data oriented
  * "Show me your tables, and I won't usually need your flowchart--
     it'll be obvious."
  * typically the URL amounts to an operand address
* Stateless--a request stands on its own
  * requests can be done out of order
* Minimalist verb set: HEAD, GET, PUT, DELETE
  * idempotence when side-effects present (e.g. PUT, DELETE)
  * no side effects for GET and HEAD
  * caching semantics enabled by GET, PUT unless "no-cache" attribute

## REST: shortcomings

* GET/PUT: not every procedure has a single operand
* GET/PUT: not every procedure relates to storage
* Not all storage has the same caching semantics
* "Just use REST"
  * that's like saying "oh, I found it on the internet"
  * you still have to do all the legwork, put it all together
  * the opposite of being overly prescriptive
* Schemas? nonstandard, separately enforced and maintained
* More aspirational than actual...
  * old cruft from HTTP repainted and resold
  * REST compliance--a gray area
  * a school of thought with different camps, opinions
  * a lot of REST rumination, for all the 80s pain it buys
* Many different APIs and implementations
* Everything is XML!  Er, I mean JSON!
  * yet, both options are disappointing... pick one?
* Where do request parameters go?
  * as positionally encoded path elements?
  * as URL-encoded parameters?  one big single level key/value list
    * ridiculous URI parsing hell--the bad old days before XML and JSON
    * does the param affect caching?
  * as part of the payload?  reliable but application specific
  * only payload supports hierarchical... and json is bad at binary...
* State overlayed on stateless system, via long URLs and sessions
* HEAD, GET, PUT, POST, and DELETE are a conceptual mess
  * quaint like SQL but less formalized
  * POST as container creation?  spoken with utter lack of conviction!
* Only loosely standardized--e.g. POST contents are an unplanned jungle
* Shortcomings due to HTTP 1
  * header repitition inefficiency
  * requires hacks -- connection pools, pipelineing
    * pipelining is still not uniformly supported
    * pipelining disabled by default in browsers due to IIS etc
    * pipelining completely disabled for POST (for coherence)
    * pipelining interferes with fairness in proxies
  * head of line blocking
  * can't cancel
  * can't prioritize
  * not so good at streaming
    * websockets provides a solution, but DIY protocols and not ubiquitous
    * SSE (server side events) is lame--text only
    * long poll: how to avoid ping-pong latency? multiple queued requests?
    * generally consumes a precious resource (server connection)
* No remote tracing or telemetry provided, once again DIY
* Cache: here are some rough primitives, implement it yourself
* On your own, re 1 to many
* On your own, re NAT

## Priorities: what will ellis-op bring that REST doesn't normally provide?

* Ease of use, framework supplied, batteries included
* No worrying about parsing
  * it's all parsed for you in the request and response objects
  * systematic structure
  * schemas applied before request delivered to server, or response to client
* Use whatever encoding
  * encoding is flexible and auto-negotiated (if have ellis codec)
* Low latency, out of order request completion
* Telemetry and system status always present
* Provide tools for working around NAT firewalls and implementing fan-out
* Application just creates requests and submits them
  * request might be handled locally immediately, or may wait for server
  * routing table maps module/procname to handler/config, endpoints, etc

## Additional abstract goals

* Simplicity
  * easy to understand how it works
  * easy to use
* Performance
  * designed for performance from the outset
  * in such a way as compatible with simplicity
* Flexibility
  * provide configurable toolkit
  * suitably constrained to encourage sanity
* Open alternative to proprietary frameworks
* Interoperation
  * other ellis-op infrastructure: seamless
  * REST and other standards: almost seamless
  * proprietary non-standards: possible with plug-ins
* Keep it simple
  * how much time has gone into SOAP and REST frameworks?
  * simpler results in less bugs, savvier users
  * big frameworks require big buy-in commitment

## Non-goals

* http/1.1 support--use an outer proxy like nginx if you want that

## Request and response structure

Requests and responses can be handled locally, without ever being sent to a
remote server, if applicable.  Similarly, depending on the cache semantics
of a particular module, the request may be handled from a local cache.  In
case the request will be sent to a remote server, it is up to the client or
proxy implementation to decide (based on negotiation with the remote server)
which encoding to use when sending the request over the wire.

The current intent is that it will always be safe for the client to use either
msgpack or json encoding, though other formats may be supported, as provided
by the ellis library.

#### HTTP manifestation

Remote requests will be sent via POST, and only in case there is no handler
for the given procedure will an error (404) be returned, in which case the
server will generate a suitable body, including an ellis error object.  In
the case of success, the response will be 200 (but maybe 206 for incremental
notifications?).  Supposing an error was generated by the request handler,
then the details of the error will be included in the response, and the HTTP
response type of 409 will be used.

#### Request

A request is a hierarchical object with the following fields at the top level:

```
  id:
  module:
  procedure:
  params:
  (optional) is_tracing:
  (optional) signature:
```

The `params` field is a map, but the details of that field will depend on the
nature of the particular module and procedure.

If `is_tracing` is present and set to `true`, then the timing and stages of
execution will be traced while executing the request.  This allows periodic
sampling when investigating performance issues.

If `signature` is present, it represents an alternate signature, above and
beyond the security afforded by the SSL HTTP/2 connection, and is computed
based on a canonical ordering of the unpacked request in memory, sans the
`signature` field, but including the request id field (in this case, the
client should take precautions against repetition of the same request id,
e.g. by using a properly random ID chosen from a sufficiently large set.

The signature may specify a different identity than the one associated with
the SSL connection.

#### Response

A response is a hierarchical object with the following fields at the top level:

```
  id:
  module:
  procedure:
  (exactly one of the following)
    error:
    result:
  nanos:
  (optional) trace_info
```

The `id`, `module`, and `procedure` fields match those of the request.

Either the `error` field or the `result` field is present, but not both.
The `error` field is only present when a request has failed, and contains an
error code, a more detailed message, and possibly other details such as the
file, line number, and function that generated the error.  The `result` field
is a map, but the details of that field will depend on the nature of the
particular module and procedure.

The `trace_info` field is only present if the corresponding request specified
a value of true for `is_tracing`.

#### Streaming

Framing of multiple request or response objects will only be potentially
relevant in the case of a notification stream.  Notification streams will be
implemented in the future, with the objective of allowing a real-time updating
client that does not suffer from the latency caused by a request/response
ping-pong, such as one might find in an MMORPG.  This should be compatible
with a long-living HTTP/2 stream, but we need to do the exploration and get
the API structure in place before we consider the API stabile, so as to
provide a better experience for early adopters of Ellis Op.

The framing layer may be considered as independent of the codec, and specifies
terminator, length style, separator, initiator.  So we can have for example:

  * no length encoding plus newline terminator
  * binary length encoding, and no terminator at all
  * whatever combination we want

#### Built-in "batch" request

Batch requests should use explicit collection of subrequests into an array,
but otherwise the same request and response structure in the outer array
(contained in request.params or response.result), except that the response
can only fail in entirity, meaning no inner request has been considered.
Otherwise, the response will be 200, and the result will have error codes
or results for each contained request.

## REST compatibility

#### Special treatment for GET, PUT, HEAD, and DELETE

Part of REST compatibility is to support GET and PUT.

We want to treat these in some uniform way, so that if GET and PUT make sense
for a given module, it has a neat framework to fit into.

So, for example, we translate in a uniform way:

```
GET /ellis/bulk/videos/acme.mp4?start=x&stop=y HTTP/1.1
...various headers...
Content-Type: foo
Accept: jeep
(body)
bar
```

translates to an RPC request as follows:

```
{
  "module" : "bulk",
  "procedure" : "GET",
  "params" : {
    "operand" : "/videos/acme.mp4",
    "start" : "x",
    "stop" : "y",
    "body" : node_decoded_based_on_foo_content_type(body_of_get)
  },
  "id" : (randomly generated)
}
```

PUT, DELETE, and HEAD are translated as with GET, though presumably HEAD and
DELETE have no "body".

So, if your request has the form of GET, HEAD, PUT, or DELETE, the
appropriate procedure name will be selected.  This procedure name may
be an alias or trivial wrapper for a more general procedure name.

#### Translating responses for GET, PUT, HEAD, and DELETE

The ellis object in the POST response needs to have some corresponding reverse
translation, in case the original request was GET, PUT, etc.

We translate the error code if present, from ellis::ex domain to canonical
HTTP response codes.

We extract the result subfield of ellis response object to the HTTP body of
the GET/PUT/etc.

We need to a way to remember when to do this translation.  The most reliable
method is to do it whenever the procedure name inside the ellis object is GET,
PUT, HEAD, or DELETE.  This will encourage consistent thinking of these as
special translated requests, rather than as the fundamental theme about which
an API must be designed, thus freeing designers to concentrate on what is the
most natural API design rather than trying to see their world only in terms of
REST primitives.

So, for example, translating backwards, an RPC response like this:

```
{
  "module" : "bulk",
  "procedure" : "GET",
  "result" : {
    "body" : node(some_video)
  },
  "nanos" : 25975,
  "id" : value_from_translated_post
}
```

translates back to an REST-compatibility HTTP response like this:

```
HTTP/1.1 200 OK
...various headers...
Content-Type: jeep (recorded from Accept header in original REST request)
(body == node_encoded_based_on_jeep_content_type(resp[result[body]]))
```

Note: this means that the HTTP server layer associates more than just
a request ID and stream ID pairing, but also remembers the desired encoding
from the request's HTTP "Accept" header.

#### REST-compatible behavior

* GET -- safe (can cache, no side effects)
* PUT -- unsafe but idempotent (updates cache, has side effects)
* DELETE -- unsafe but idempotent (knows whose refcount to release)
* HEAD -- safe (but can use to invalidate cache); returns current version,
  taking into acount the request params; does not return the result
* ideally GET, PUT, and DELETE have version numbers and policy in the request

#### POST: /ellis only

With POST, REST compatibility translation is not done, and the request goes
through verbatim.  The procedure name must be included already in the ellis
object in the HTTP body that encodes the ellis request object, since it can
not be obtained from the HTTP verb.  The module name is also taken from the
ellis object.  Thus, the HTTP request must be simply "POST /ellis" with no path
or params, though of course it will have headers such as Content-type and
Accept.

This is simpler than trying to borrow translation rules such as for the
module name.  An ellis object can then be proxied comfortably and reasoned
about consistently without somewhat arbitrary translation decision logic.

#### Rendering to a browser

There will be a builtin module called "browser" which is used for rendering
basic information (ellis landing page, system/module status, API descriptions
and submission forms) to a browser, which will, naturally support some
REST compatible predicates.

This will be used to serve, for example, /ellis/browser/index.html, which we
may link from /index.html, though the latter might be masked if ellis is
wrapped in into a larger system e.g. with nginx.

## Automatic telemetry

We want the telemetry to be built-in, because this is a thing people often
want, and a default implementation will suffice for most people and they won't
have to re-implement their own.  This will include not only rate and average
execution time of different API calls, but also histograms and even traces of
where time was spent.

* call counters, time counters
* histograms
* cache hit/miss rates
* can turn on tracing--causes `ELLIS_OP_TRACE` macro to engage
  * "stage" as a separate parameter
  * detailed tracing?
* difference of remote round trip time vs local, for scheduling/network lag
  * LAG and QUEUE stages?
* may be useful for rate limiting
* can do metrics efficiently--e.g. thread local trackers as needed
* per call metrics etc in subsystem status
* heads-up display of metrics
  * straight from server or from read replica
  * later: also aggregation infrastructure (but keep server direct)

## System information

* system status just has version, connection info, and module list
* see module status for more details on that module
* module status lists version, procedure list
* module provides help, schema, and telemetry for each procedure

## Load and pushback

First and foremost: protection of remote server; should not depend too much
on conscientious client.

HTTP/2 provides its own limiting, at the level of individual requests (the
stream window), and at the level of the connection (fixed max outstanding
stream slots).

Running out of stream slots: a sudden wall, which is better than nothing, but
might wedge, might not degrade smoothly.  Maybe each procedure has way of
computing its "relative" load factor vs remaining stream slots on a
connection, and a good client can comprehend this and back off that type of
request accordingly.  Might be based on not just request count, but also
potentially memory/disk/bandwidth use.  Probably specific to a subsystem, so
may need hooks for subsystem-specific influence even when sending to remote
server?  But that might be too complicated.

## Access control

By default, all requests implemented directly are considered to have been
issued by the "system" user, who is presumed automatically to have access.

Requests received over the network by a server will by default be associated
with the identity implied by their SSL connection.

Individual requests may have their identity overridden by the request
signature.  This technique may be useful when proxying or reverse proxying
requests.

The table used by the local request scheduler assumes only that the system
user has access to each of the procedures in each module, by default, though
additional identities can be granted access.

## One to many distribution

This simplest and initial implementation of this feature will be achieved by
providing a daemon program that keeps open SSL connections to all the
potential recipients.  A "merv" request will then be sent which contains in its
params a list of recipients and an inner request.  The request ID of the inner
request may be translated as needed for the implied proxying.

## Proxy queueing

When proxying commands to a remote server, if that remote server can not be
reached, the proxy will maintain a queue of requests to be sent to the remote
server.  In this case, when the remote server becomes available again, the
requests will be delivered, and responses ferried back to the client.

Proxy queueing can be enabled or disabled independently for different
modules and servers.

## Request exfiltration

It may be that edge systems are behind NAT firewalls where the system
maintenance hosts can not easily make contact with them.

In this case, the same request proxy queueing behavior should occur, based on
some perhaps abstracted representation of the client host/identity to which
requests are addressed.

When it is desired that these requests be allowed a pathway to reach the edge
nodes, it is expected that the edge nodes will then be configured to attempt
to connect (as clients) to the well known maintenance servers in a sort of
"pull" mode, and issue "pull request" requests, with a parallel stream issuing
"push response" requests for the responses to these requests.  These builtin
commands will interact with the server's proxy queue and allow the original
requests to complete even though a connection was not made to the proxy.

Request exfiltration is not the only mechanism available to distribute
requests to edge nodes behind firewall.  Edge nodes may also poll for updates
to a central database, and record their responses in the database, in which
case there is no need for individual addressing and proxy queueing.

The one-to-many features, batch requests, request signing, and request
exfiltration, can be used in conjunction.

## Future project: database as a reference ellis op project

* base on ellis objects, rather than a particular format like json or bson
* focus on deep concepts rather than fancy features first
* hash based versions
* uniform version-based request/response precondition
* delete w/o version: head within domain
* delete w/ version: decrease refcount policy for the given branch?
* reads and writes specify refcount on local cache?
* garbage collection of cache
  * some kind of efficient version refcounting inside the graph?
* write durability policy: is local cache evictable with zero refcount?
* write durability policy: whether to block until durable upstream
* write durability policy: whether to bypass local "cache"

#### Listening for updates

* can monitor an entire subtree of filesystem for changes based on checksum
  * similar hierarchical clustering in a document database?
* use 64-bit int nanos since 1970 internally for any timestamps

#### Reference counting

Idea: There's no create/update, there's just a PUT-style request (i.e.
accessible via REST-compatibility layer) with a given version number and
checksum, and similarly a DELETE-style request with the prior checksum which
invokes garbage collection.  Or maybe the interface allows inclusive/exclusive
selection.  Either way, it doesn't delete just that node in the tree, but
ancestors of that node it, if they are clear of alternate paths, so that only
descendants of the deleted node remain undeleted.  The default write policy
can be to decrease the prior version's refcount automatically whenever a new
version is written.  Write request can have a "hold" option that takes
refcount.

#### Versioning

* Review ETag and If-Match as relates to versioning
* Use cryptographic hash for versioning
  * Supported list is additive over time, starts with SHA256, BLAKE2, SHA3.
  * Approved list for newly created hashes hard-specified for ellisex version.
* TODO: need to decide whether to include prior version in hash
* TODO: Do I support some sort of auto-versioning? does hash give me that?
* TODO: Does version care only about local checksum and not prior version?
  * Con: including prior checksum allows inferring the most recent
  * Con: including prior checksum allows ruling out ABA races
  * Pro: a dependent write can succeed if competing branch is removed.
  * Does "conditional write" equate to only one head version allowed?

#### Garbage collection

* TODO: Does DELETE decrease the refcount on the cache or the upstream or both
  or what?  How does this relate to write durability policy

#### Write durability

* TODO: permission--based on path?
* TODO: conditional writes based on version?
* TODO: what happens if allow local commit and version conflicts downstream?
* TODO: write quorums?
* TODO: Degraded modes?
* TODO: easy way to say whether to wait for intermediate or final durability?
* TODO: local cache as staging area for downstream?

## Future tasks

* index.html and javascript for server introspection
* request submission via index.html: auto-fill, schema, docs
* viewing of stats via index.html
* sampling call profiler--automatic periodic tracing
* CSS, AJAX: live and colorful profile telemetry viewing
* built-in batch request
* ellis-op server icon (plus .ico built-in to server code)
* request cancellation--local and remote via HTTP/2 `RST_STREAM`
* streaming updates
* access control--connection
* access control overrides for individual requests?
* unix domain socket support
* unix domain socket access control: local identity/role

## Testing use-cases

* built-in request/response
* client/server call
* multiple module registration
* telemetry accuracy
* procedure stats accuracy
* subsystem status
* coping with runaway request submission
* REST compatibility to/from translation
* update streaming
