#include "RestRequest.h"

void RestRequest::AddParameter(String key, String value)
{
  if (this->parameters.length() > 0) {
    this->parameters += "&";
  }

  this->parameters += key + "=" + value;
}

void RestRequest::AddUrlSegment(String key, String value)
{
  if (this->urlSegments.length() > 0) {
    this->urlSegments += "&";
  }

  this->urlSegments += key + "=" + value;
}

void RestRequest::AddHeader(String header, String value)
{
  this->headers[this->headerIndex].header = header;
  this->headers[this->headerIndex].value = value;

  this->headerIndex++;
}
