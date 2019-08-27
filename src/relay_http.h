// Includes for the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <ESPmDNS.h>

//Includes Https client
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#define RELAY_1 25
#define RELAY_2 26

#define SSO_URL "https://sso2.gpsauriga.com:443/oauth2AuthorizationServer/oauth/check_token"
#define SSO_AUTHORIZATION "cHJ1ZWJhcy5ncHNhdXJpZ2EuY29tOnNlY3JldA=="
//#define ROLE_OPEN_DOOR "ROLE_OPEN_AGUILAS_DOOR"
#define ROLE_OPEN_DOOR "ROLE_TRUSTED_CLIENT"


// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

//To include in main setup function
void setupRelayHttpServer();

void loopRelayHttpServer();

// Declare some handler functions for the various URLs on the server
// The signature is always the same for those functions. They get two parameters,
// which are pointers to the request data (read request body, headers, ...) and
// to the response data (write response, set status code, ...)
void handle404(HTTPRequest * req, HTTPResponse * res);
void handleRelay1(HTTPRequest * req, HTTPResponse * res);
void handleRelay2(HTTPRequest * req, HTTPResponse * res);

// Declare a middleware function.
// Parameters:
// req: Request data, can be used to access URL, HTTP Method, Headers, ...
// res: Response data, can be used to access HTTP Status, Headers, ...
// next: This function is used to pass control down the chain. If you have done your work
//       with the request object, you may decide if you want to process the request.
//       If you do so, you call the next() function, and the next middleware function (if
//       there is any) or the actual requestHandler will be called.
//       If you want to skip the request, you do not call next, and set for example status
//       code 403 on the response to show that the user is not allowed to access a specific
//       resource.
//       The Authentication examples provides more details on this.
void middlewareAuth(HTTPRequest * req, HTTPResponse * res, std::function<void()> next);

// Check the token with the SSO server, retrieves the aithorization data and 
// check that inclues the provided role
int checkToken(String auth, String roleToCheck);

unsigned long getPreviousMillis();
