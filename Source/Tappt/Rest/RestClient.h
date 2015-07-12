#ifndef RestClient_h
#define RestClient_h

#include "application.h"
#include "HttpClient.h"
#include "RestResponse.h"
#include "RestRequest.h"

class RestClient {
public:
  RestClient(String baseUrl);

  RestResponse& Execute(RestRequest& request);
private:
  String baseUrl;
  //HttpClient httpClient;

  RestResponse restResponse;

  ~RestClient();
};

#endif
