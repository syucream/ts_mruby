/*
// ts_mruby.cpp - ts_mruby internal functions
//
*/

#include "ts_mruby_internal.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <mruby/string.h>

using namespace std;
using namespace atscppapi;

namespace {

// These status reasons are based on proxy/hdrs/HTTP.cc on trafficserver.
// XXX Temporary fix. We should use this function from atscppapi.
string http_status_reason_lookup(unsigned status) {
  switch (status) {
  case 0:
    return "None";
  case 100:
    return "Continue";
  case 101:
    return "Switching Protocols";
  case 102:
    return "Processing";
  case 200:
    return "OK";
  case 201:
    return "Created";
  case 202:
    return "Accepted";
  case 203:
    return "Non - Authoritative Information";
  case 204:
    return "No Content";
  case 205:
    return "Reset Content";
  case 206:
    return "Partial Content";
  case 207:
    return "Multi - Status";
  case 208:
    return "Already Reported";
  case 226:
    return "IM Used";
  case 300:
    return "Multiple Choices";
  case 301:
    return "Moved Permanently";
  case 302:
    return "Found";
  case 303:
    return "See Other";
  case 304:
    return "Not Modified";
  case 305:
    return "Use Proxy";
  case 307:
    return "Temporary Redirect";
  case 308:
    return "Permanent Redirect";
  case 400:
    return "Bad Request";
  case 401:
    return "Unauthorized";
  case 402:
    return "Payment Required";
  case 403:
    return "Forbidden";
  case 404:
    return "Not Found";
  case 405:
    return "Method Not Allowed";
  case 406:
    return "Not Acceptable";
  case 407:
    return "Proxy Authentication Required";
  case 408:
    return "Request Timeout";
  case 409:
    return "Conflict";
  case 410:
    return "Gone";
  case 411:
    return "Length Required";
  case 412:
    return "Precondition Failed";
  case 413:
    return "Request Entity Too Large";
  case 414:
    return "Request - URI Too Long";
  case 415:
    return "Unsupported Media Type";
  case 416:
    return "Requested Range Not Satisfiable";
  case 417:
    return "Expectation Failed";
  case 422:
    return "Unprocessable Entity";
  case 423:
    return "Locked";
  case 424:
    return "Failed Dependency";
  case 426:
    return "Upgrade Required";
  case 428:
    return "Precondition Required";
  case 429:
    return "Too Many Requests";
  case 431:
    return "Request Header Fields Too Large";
  case 500:
    return "Internal Server Error";
  case 501:
    return "Not Implemented";
  case 502:
    return "Bad Gateway";
  case 503:
    return "Service Unavailable";
  case 504:
    return "Gateway Timeout";
  case 505:
    return "HTTP Version Not Supported";
  case 506:
    return "Variant Also Negotiates";
  case 507:
    return "Insufficient Storage";
  case 508:
    return "Loop Detected";
  case 510:
    return "Not Extended";
  case 511:
    return "Network Authentication Required";
  default:
    return "";
  }
}

vector<string>
split(const string& str, char delimiter) {
  vector<string> splitted;

  string item;
  istringstream ss(str);
  while(getline(ss, item, delimiter)) {
    if (!item.empty()) {
      splitted.push_back(item);
    }
  }

  return splitted;
}

} // anonymous namespace

bool
judge_tls(const string& scheme) {
  if (scheme == "https") {
    return true;
  }
  return false;
}

pair<string, uint16_t>
get_authority_pair(const string& authority, bool is_tls) {
  const vector<string>& splitted = split(authority, ':');

  uint16_t port = 80;
  if (splitted.size() == 2) {
    port = static_cast<uint16_t>(stoul(splitted[1]));
  } else {
    port = (is_tls) ? 443 : 80;
  }

  return make_pair(splitted[0], port);
}

void RputsPlugin::setStatusCode(int code) { _statusCode = code; }

void RputsPlugin::appendMessage(const string &msg) { _message += msg; }

void RputsPlugin::appendHeader(const pair<string, string> &entry) {
  _headers.push_back(entry);
}

void RputsPlugin::appendHeaders(const HeaderVec &h) {
  _headers.insert(_headers.end(), h.begin(), h.end());
}

void RputsPlugin::handleInputComplete() {
  string response("HTTP/1.1 " + std::to_string(_statusCode) + " " +
                  http_status_reason_lookup(_statusCode) + "\r\n");

  // make response header
  if (!_message.empty()) {
    response += "Content-Length: " + std::to_string(_message.size()) + "\r\n";
  }

  for_each(_headers.begin(), _headers.end(),
           [&response](pair<string, string> entry) {
             response += entry.first + ": " + entry.second + "\r\n";
           });

  // make response body
  response += "\r\n";
  produce(response);
  response = _message + "\r\n";
  produce(response);
  setOutputComplete();
}

void
RemapOverridePlugin::handleReadRequestHeadersPostRemap(Transaction& transaction) {
  transaction.resume();
}

void HeaderRewritePlugin::addRewriteRule(const std::string &key,
                                         const std::string &value,
                                         Operator op) {
  modifiers_.push_back(make_tuple(key, value, op));
}

void HeaderRewritePlugin::handleSendResponseHeaders(Transaction &transaction) {
  Response &resp = transaction.getClientResponse();

  Headers &resp_headers = resp.getHeaders();
  for_each(modifiers_.begin(), modifiers_.end(),
           [&resp_headers](tuple<string, string, Operator> modifier) {
             switch (get<2>(modifier)) {
             case Operator::ASSIGN: {
               resp_headers[get<0>(modifier)] = get<1>(modifier);
               break;
             }
             case Operator::DELETE: {
               resp_headers.erase(get<0>(modifier));
               break;
             }
             default: { break; }
             }
           });

  transaction.resume();
}

void FilterPlugin::appendBody(const string &data) { _transformedBuffer.append(data); }

void FilterPlugin::appendBlock(const mrb_value block) { _block = block; }

void FilterPlugin::consume(const std::string &data) { _origBuffer.append(data); }

void FilterPlugin::handleInputComplete() {
  if (!mrb_nil_p(_block)) {
    mrb_state *state = mrb_open();
    mrb_value rv = mrb_yield(state, _block, mrb_str_new(state, _origBuffer.c_str(), _origBuffer.length()));
    _transformedBuffer = string(mrb_string_value_cstr(state, &rv));
  }

  produce(_transformedBuffer);
  setOutputComplete();
}
