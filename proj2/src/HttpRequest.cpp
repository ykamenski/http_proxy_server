#include "HttpRequest.h"
#include <string>
#include <iostream>


std::ostream& operator<<(std::ostream& strm, const Header& header)
{
  strm << header.name << ": " << header.value;
  return strm;
}

HttpRequest::HttpRequest(std::string& firstLine)
{
  parseFirstLine(firstLine);
  parseUri();
}

HttpRequest::HttpRequest(const char* buf, size_t n)
{
  const std::string line{ buf, n };
  parseFirstLine(line);
  parseUri();
}

HttpRequest::HttpRequest()
{
  parseStatus = UriParseErrType::Malformed;
}

HttpRequest::~HttpRequest()
{
}

bool HttpRequest::isValid() const
{
  return parseStatus == UriParseErrType::Success;
}

std::string HttpRequest::getErrorText()
{
  switch (parseStatus)
  {
  case UriParseErrType::Success: return "Success";
  case UriParseErrType::NotHttp: return "NotHttp";
  case UriParseErrType::Malformed: return "Malformed";
  default: return "???";
  }
}

void HttpRequest::adjustHeaderIfNeeded(Header& header)
{
  if (header.name == "Connection") {
    header.value = "close";
  }
}

bool HttpRequest::headerIsFiltered(const Header& header)
{
  return header.name == "Proxy-Connection";
}


bool HttpRequest::appendHeader(std::string&& line)
{
  // TODO: rewrite the header lines as needed
  // forex: connection should be closed,
  // forex: host should be the site name, etc..
  // remove the Proxy-Something-Keep-Alive

  std::cout << "Trace - raw line:" << line << std::endl;
  Header header;
  std::stringstream strm{ line };
  std::getline(strm, header.name, ':');

  if (strm.peek() == ' ') {
    strm.ignore();
  }
  std::getline(strm, header.value, '\r');


  if (!headerIsFiltered(header)) {
    adjustHeaderIfNeeded(header);
    std::cout << "Trace - adding: (" << header << std::endl;
    
    // check to see if it's one of the headers we are tracking for later
    if (header.name == "Connection") hasHeaderConnection = true;
    if (header.name == "Host") hasHeaderHost = true;

    headers.emplace_back(header);
  } else {
    std::cout << "Trace - header was filtered: " << header << std::endl;
  }

  return true;
}


std::string HttpRequest::getPageRequest()
{
  // check to make sure the host and connection headers exist
  if (!hasHeaderConnection) {
    hasHeaderConnection = true;
    std::cout << "Trace - adding connection close header.." << std::endl;
    headers.emplace_back(Header{ "Connection", "close" });
  }
  if (!hasHeaderHost) {
    hasHeaderHost = true;
    std::cout << "Trace - adding host header.." << std::endl;
    headers.emplace_back(Header{ "Host", uri.authority });
  }

  std::stringstream outReq;
  outReq << requestLine.type << " " << uri.path << " " << "HTTP/1.0" << "\r\n";
  for (const Header& h : headers) {
    outReq << h << "\r\n";
  }

  // marks end of the request section
  outReq << "\r\n";

  return outReq.str();
}

void HttpRequest::parseFirstLine(const std::string& line)
{
  std::stringstream strm{line};
  strm >> requestLine.type;
  strm >> requestLine.absUrl;
  strm >> requestLine.protocol;
}

void HttpRequest::parseUri()
{
  std::stringstream strm{ requestLine.absUrl };

  std::getline(strm, uri.scheme, ':');
  if (uri.scheme != "http") {
    parseStatus = UriParseErrType::NotHttp;
    return;
  }

  for (int i = 0; i < 2; ++i) {
    char c;
    strm >> c;
    // do not continue if '//' is not next
    if (c != '/') {
      parseStatus = UriParseErrType::Malformed;
      return;
    }
  }

  std::getline(strm, uri.authority, '/');
  if (!uri.authority.size()) {
    parseStatus = UriParseErrType::Malformed;
    return;
  }

  std::getline(strm, uri.path, '\0');
  uri.path.insert(0, "/");

  parseStatus = UriParseErrType::Success;
}

