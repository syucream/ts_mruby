// ts_mruby internal functions

// Copied from proxy/hdrs/HTTP.cc on trafficserver and modified to return by std::string.
// XXX Temporary fix. We should use this function from atscppapi.
std::string
reason_lookup(unsigned status)
{
#define HTTP_STATUS_ENTRY(value, reason) \
  case value:                            \
    return std::string(#reason)

  switch (status) {
    HTTP_STATUS_ENTRY(0, None);                  // TS_HTTP_STATUS_NONE
    HTTP_STATUS_ENTRY(100, Continue);            // [RFC2616]
    HTTP_STATUS_ENTRY(101, Switching Protocols); // [RFC2616]
    HTTP_STATUS_ENTRY(102, Processing);          // [RFC2518]
    // 103-199 Unassigned
    HTTP_STATUS_ENTRY(200, OK);                              // [RFC2616]
    HTTP_STATUS_ENTRY(201, Created);                         // [RFC2616]
    HTTP_STATUS_ENTRY(202, Accepted);                        // [RFC2616]
    HTTP_STATUS_ENTRY(203, Non - Authoritative Information); // [RFC2616]
    HTTP_STATUS_ENTRY(204, No Content);                      // [RFC2616]
    HTTP_STATUS_ENTRY(205, Reset Content);                   // [RFC2616]
    HTTP_STATUS_ENTRY(206, Partial Content);                 // [RFC2616]
    HTTP_STATUS_ENTRY(207, Multi - Status);                  // [RFC4918]
    HTTP_STATUS_ENTRY(208, Already Reported);                // [RFC5842]
    // 209-225 Unassigned
    HTTP_STATUS_ENTRY(226, IM Used); // [RFC3229]
    // 227-299 Unassigned
    HTTP_STATUS_ENTRY(300, Multiple Choices);  // [RFC2616]
    HTTP_STATUS_ENTRY(301, Moved Permanently); // [RFC2616]
    HTTP_STATUS_ENTRY(302, Found);             // [RFC2616]
    HTTP_STATUS_ENTRY(303, See Other);         // [RFC2616]
    HTTP_STATUS_ENTRY(304, Not Modified);      // [RFC2616]
    HTTP_STATUS_ENTRY(305, Use Proxy);         // [RFC2616]
    // 306 Reserved                                                   // [RFC2616]
    HTTP_STATUS_ENTRY(307, Temporary Redirect); // [RFC2616]
    HTTP_STATUS_ENTRY(308, Permanent Redirect); // [RFC-reschke-http-status-308-07]
    // 309-399 Unassigned
    HTTP_STATUS_ENTRY(400, Bad Request);                     // [RFC2616]
    HTTP_STATUS_ENTRY(401, Unauthorized);                    // [RFC2616]
    HTTP_STATUS_ENTRY(402, Payment Required);                // [RFC2616]
    HTTP_STATUS_ENTRY(403, Forbidden);                       // [RFC2616]
    HTTP_STATUS_ENTRY(404, Not Found);                       // [RFC2616]
    HTTP_STATUS_ENTRY(405, Method Not Allowed);              // [RFC2616]
    HTTP_STATUS_ENTRY(406, Not Acceptable);                  // [RFC2616]
    HTTP_STATUS_ENTRY(407, Proxy Authentication Required);   // [RFC2616]
    HTTP_STATUS_ENTRY(408, Request Timeout);                 // [RFC2616]
    HTTP_STATUS_ENTRY(409, Conflict);                        // [RFC2616]
    HTTP_STATUS_ENTRY(410, Gone);                            // [RFC2616]
    HTTP_STATUS_ENTRY(411, Length Required);                 // [RFC2616]
    HTTP_STATUS_ENTRY(412, Precondition Failed);             // [RFC2616]
    HTTP_STATUS_ENTRY(413, Request Entity Too Large);        // [RFC2616]
    HTTP_STATUS_ENTRY(414, Request - URI Too Long);          // [RFC2616]
    HTTP_STATUS_ENTRY(415, Unsupported Media Type);          // [RFC2616]
    HTTP_STATUS_ENTRY(416, Requested Range Not Satisfiable); // [RFC2616]
    HTTP_STATUS_ENTRY(417, Expectation Failed);              // [RFC2616]
    HTTP_STATUS_ENTRY(422, Unprocessable Entity);            // [RFC4918]
    HTTP_STATUS_ENTRY(423, Locked);                          // [RFC4918]
    HTTP_STATUS_ENTRY(424, Failed Dependency);               // [RFC4918]
    // 425 Reserved                                                   // [RFC2817]
    HTTP_STATUS_ENTRY(426, Upgrade Required); // [RFC2817]
    // 427 Unassigned
    HTTP_STATUS_ENTRY(428, Precondition Required); // [RFC6585]
    HTTP_STATUS_ENTRY(429, Too Many Requests);     // [RFC6585]
    // 430 Unassigned
    HTTP_STATUS_ENTRY(431, Request Header Fields Too Large); // [RFC6585]
    // 432-499 Unassigned
    HTTP_STATUS_ENTRY(500, Internal Server Error);      // [RFC2616]
    HTTP_STATUS_ENTRY(501, Not Implemented);            // [RFC2616]
    HTTP_STATUS_ENTRY(502, Bad Gateway);                // [RFC2616]
    HTTP_STATUS_ENTRY(503, Service Unavailable);        // [RFC2616]
    HTTP_STATUS_ENTRY(504, Gateway Timeout);            // [RFC2616]
    HTTP_STATUS_ENTRY(505, HTTP Version Not Supported); // [RFC2616]
    HTTP_STATUS_ENTRY(506, Variant Also Negotiates);    // [RFC2295]
    HTTP_STATUS_ENTRY(507, Insufficient Storage);       // [RFC4918]
    HTTP_STATUS_ENTRY(508, Loop Detected);              // [RFC5842]
    // 509 Unassigned
    HTTP_STATUS_ENTRY(510, Not Extended);                    // [RFC2774]
    HTTP_STATUS_ENTRY(511, Network Authentication Required); // [RFC6585]
    // 512-599 Unassigned
  }

#undef HTTP_STATUS_ENTRY

  return "";
}

