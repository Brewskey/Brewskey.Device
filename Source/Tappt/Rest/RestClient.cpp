#include "RestClient.h"

RestClient::RestClient(String baseUrl) {
  this->baseUrl = baseUrl;
}

RestResponse& RestClient::Execute(RestRequest& request) {
  return this->restResponse;
}
