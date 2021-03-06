////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
// 
// HttpRequest.cpp
// Parsed proxy HttpRequest.
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include "HttpRequest.h"

/**
 * \brief Gets the http status and delivers human readable message
 * \param status HttpRequestStatus received
 * \return char string of a human-readable status message
 */
const char *getHttpRequestStatusStr(const HttpRequestStatus status) {
  switch (status) {
  case HttpRequestStatus::Success: 
    return "Success";
  case HttpRequestStatus::NotGetRequest: 
    return "Request is not of type 'GET'";
  case HttpRequestStatus::NotHttp: 
    return "Request is not http://";
  case HttpRequestStatus::UriParseError: 
    return "Request URL is invalid.";
  case HttpRequestStatus::HeaderError:
    return "Error while parsing headers of request.";
  default: 
    return "Unknown request status code";
  }
}

std::ostream& operator<<(std::ostream& strm, const Header& header) {
  strm << header.name << ": " << header.value;
  return strm;
}

HttpRequest::HttpRequest(HttpRequestStatus status) {
  parseStatus = status;
}

HttpRequest::HttpRequest(std::string &&type_, std::string &&protocol_,
                         HttpUri &&uri_) 
  : type(type_), protocol(protocol_), url(uri_) {
  parseStatus = HttpRequestStatus::Success;

  // Adding the two required headers here and omitting any of these
  // when or if encountered later
  headers.emplace_back(Header{ std::string("Host"), url.getHost() });
  headers.emplace_back(Header{ std::string("Connection"), std::string("close") });
}

HttpRequest::~HttpRequest() = default;

bool HttpRequest::isValid() const {
  return parseStatus == HttpRequestStatus::Success;
}

const char * HttpRequest::getErrorStr() const {
  return getHttpRequestStatusStr(parseStatus);
}

bool HttpRequest::appendHeader(std::string&& line) {
  // First check if line contains ": " making it a valid header
  size_t sepIdx = line.find(':');
  if (std::string::npos == sepIdx) {
	  parseStatus = HttpRequestStatus::HeaderError;
	  return false;
  }

  std::string name = line.substr(0, sepIdx);

  // Make compatible if there is no single space char after ':'
  if (sepIdx + 2 < line.size() && line[sepIdx + 1] == ' '){
    sepIdx = sepIdx + 2;
  } else {
    sepIdx = sepIdx + 1;
  }

  // These are not considered here since created in the ctor
  if (name != "Host" &&
      name != "Connection" &&
      name != "Proxy-Connection") {
    headers.emplace_back(Header{ move(name), line.substr(sepIdx + 1) });
  }

  return true;
}

std::string HttpRequest::getRequestStr() {
  std::stringstream outReq;
  outReq << type << " " << url.getPath() << " " << "HTTP/1.0" << "\r\n";
  for (const Header& h : headers) {
    outReq << h << "\r\n";
  }

  // marks end of the request section
  outReq << "\r\n";

  return outReq.str();
}

//HttpRequest HttpRequest::createFrom(const int clientFd) {
//  std::stringstream strm;
//
//  ssize_t lineSize = readLineIntoStrm(clientFd, strm);
//  if (lineSize <= 0) {
//    LOG_ERROR("line read or socket close");
//    return HttpRequest{HttpRequestStatus::HeaderError};
//  }
//
//  std::string type;
//  std::string absUrl;
//  std::string protocol;
//  strm >> type;
//  strm >> absUrl;
//  strm >> protocol;
//  HttpUri parsedUrl{ move(absUrl) };
//
//  if (type != "GET") {
//    LOG_ERROR("Request is not GET");
//    return HttpRequest{ HttpRequestStatus::NotGetRequest };
//  }
//
//  if (!parsedUrl.isValid()) {
//    LOG_ERROR("Url is not good.");
//    return HttpRequest{ HttpRequestStatus::UriParseError };
//  }
//
//  if (parsedUrl.getScheme() != "http") {
//    LOG_ERROR("Scheme is not http");
//    return HttpRequest{ HttpRequestStatus::NotHttp };
//  }
//
//  HttpRequest result(move(type), move(protocol), std::move(parsedUrl));
//
//  bool lastLineNonblank = true;
//  while (lastLineNonblank) {
//    strm.str(std::string());  // clear the buffer
//    lineSize = readLineIntoStrm(clientFd, strm);
//    if (lineSize < 0) {
//      LOG_ERROR("header read from socket");
//      result.parseStatus = HttpRequestStatus::HeaderError;
//      return result;
//    }
//
//    std::string line = strm.str();
//    lastLineNonblank = !(line.empty() || (line.size() == 1 && line[0] == '\r'));
//
//    if (lastLineNonblank) {
//      if (line.back() == '\r') {
//        line.erase(line.end() - 1);
//      }
//      result.appendHeader(move(line));
//    }
//  }
//
//  return result;
//}

const char * HttpRequest::getHost() const {
  return url.getHost().c_str();
}

int HttpRequest::getPort() const {
  return url.getPort();
}
