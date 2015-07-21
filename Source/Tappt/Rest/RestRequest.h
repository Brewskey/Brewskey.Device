#ifndef RestRequest_h
#define RestRequest_h

#include "application.h"

typedef struct
{
  String header;
  String value;
} RestHeader;

class RestRequest {
public:
  void AddParameter(String key, String value);
  void AddUrlSegment(String key, String value);
  void AddHeader(String header, String value);
private:
  String parameters;
  String urlSegments;
  RestHeader headers[10];
  int headerIndex;
};

#endif
