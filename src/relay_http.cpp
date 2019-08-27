#include "relay_http.h"

// Include certificate data (see note above)
#include "cert.h"
#include "private_key.h"

// Create an SSL certificate object from the files included above
SSLCert cert = SSLCert(
  example_crt_DER, example_crt_DER_len,
  example_key_DER, example_key_DER_len
);

// Create an SSL-enabled server that uses the certificate
// The contstructor takes some more parameters, but we go for default values here.
HTTPSServer secureServer = HTTPSServer(&cert);

// Previous millis stores the start of the relay transaction
unsigned long previousMillis = 0;

unsigned long getPreviousMillis(){
    return previousMillis;
}

void setupRelayHttpServer() {

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  while (!MDNS.begin("esp32")) {
      Serial.println("Error setting up MDNS responder!");
      delay(1000);
  }
  Serial.println("mDNS responder started");

  // For every resource available on the server, we need to create a ResourceNode
  // The ResourceNode links URL and HTTP method to a handler function
  ResourceNode * node404    = new ResourceNode("", "GET", &handle404);
  ResourceNode * nodeRelay1 = new ResourceNode("/relay1/*", "GET", &handleRelay1);
  ResourceNode * nodeRelay2 = new ResourceNode("/relay2/*", "GET", &handleRelay2);

  // Add the params nodes to the server
  secureServer.registerNode(nodeRelay1);
  secureServer.registerNode(nodeRelay2);

  // Add the 404 not found node to the server.
  // The path is ignored for the default node.
  secureServer.setDefaultNode(node404);

  // Add middleware for authorization
  secureServer.addMiddleware(&middlewareAuth);

  Serial.println("Starting server...");
  secureServer.start();
  if (secureServer.isRunning()) {
    Serial.println("Server ready.");
  }
}

void loopRelayHttpServer() {
  secureServer.loop();
}

void handleRelay1(HTTPRequest * req, HTTPResponse * res) {
  // Get access to the parameters
  ResourceParameters * params = req->getParams();
  // The url pattern is: /relayX/*
  // This will make the content for the first parameter available on index 0
  std::string param = params->getUrlParameter(0);

  // Set a simple content type
  res->setHeader("Content-Type", "application/json");

  // Print the first parameter value
  res->print("{'relay': '1', 'value': '");
  res->printStd(param);
  res->print("'}");

  if (strcmp(param.c_str(), "0") == 0) {
    digitalWrite(RELAY_1, HIGH);
    previousMillis = millis();
    if ((ULONG_MAX - previousMillis) < 1500) {
      previousMillis = 1500;
    } else {
      previousMillis = previousMillis + 1500;
    }

  }
}

void handleRelay2(HTTPRequest * req, HTTPResponse * res) {
  // Get access to the parameters
  ResourceParameters * params = req->getParams();
  // The url pattern is: /relayX/*
  // This will make the content for the first parameter available on index 0
  std::string param = params->getUrlParameter(0);

  // Set a simple content type
  res->setHeader("Content-Type", "application/json");

  // Print the first parameter value
  res->print("{'relay': '2', 'value': '");
  res->printStd(param);
  res->print("'}");

  if (strcmp(param.c_str(), "0") == 0) {
    digitalWrite(RELAY_2, HIGH);
    previousMillis = millis();
    if ((ULONG_MAX - previousMillis) < 1500) {
      previousMillis = 1500;
    } else {
      previousMillis = previousMillis + 1500;
    }
  }

  // Note: If you have objects that are identified by an ID, you may also use
  // ResourceParameters::getUrlParameterInt(int) for convenience
}

void handle404(HTTPRequest * req, HTTPResponse * res) {
  // Discard request body, if we received any
  // We do this, as this is the default node and may also server POST/PUT requests
  req->discardRequestBody();

  // Set the response status
  res->setStatusCode(404);
  res->setStatusText("Not Found");

  // Set content type of the response
  res->setHeader("Content-Type", "text/html");

  // Write a tiny HTTP page
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Not Found</title></head>");
  res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
  res->println("</html>");
}

void middlewareAuth(HTTPRequest * req, HTTPResponse * res, std::function<void()> next) {
  std::string auth = req->getHeader("Authorization");
  if (auth.length() > 7) { //because of Bearer
    auth = auth.substr(7);
  } else { 
      // Display error page
      res->setStatusCode(401);
      res->setStatusText("Unauthorized");
      res->setHeader("Content-Type", "text/html");
      res->println("<!DOCTYPE html>");
      res->println("<html>");
      res->println("<head><title>Unauthorized</title></head>");
      res->println("<body><h1>401. Unauthorized</h1><p>There are no suitable login information or is invalid.</p></body>");
      res->println("</html>");
      Serial.println("No token: Unauthorized");
      return;
  }
  Serial.print("Authorization: ");
  Serial.println(auth.c_str());
  // then we need to call next().
  if(checkToken(auth.c_str(), ROLE_OPEN_DOOR) == -1) {
    res->setStatusCode(403);
    res->setStatusText("Forbidden");
    res->setHeader("Content-Type", "text/html");
    res->println("<!DOCTYPE html>");
    res->println("<html>");
    res->println("<head><title>Forbidden</title></head>");
    res->println("<body><h1>403. Forbidden</h1><p>User has no privileges.</p></body>");
    res->println("</html>");
    Serial.println("Has no role");
    return;
  }
  next();
}

int checkToken(String token, String roleToCheck) {

  int ret = -1;

  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
   
    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;

      String url = SSO_URL;
      url += "?token=";
      url += token;
      
      if (https.begin(*client, url)) {  // HTTPS

        https.setAuthorization(SSO_AUTHORIZATION);

        // start connection and send HTTP header
        int httpCode = https.GET();
  
        // httpCode will be negative on error
        Serial.printf("[HTTPS SSO] GET... code: %d URL: %s\n", httpCode, url.c_str());
        if (httpCode > 0) {
  
          // file found at server
          if (httpCode == HTTP_CODE_OK) {
            String payload = https.getString();
            ret = payload.indexOf(roleToCheck);
          }
        }
  
        https.end();
      } else {
        Serial.printf("[HTTPS SSO] Unable to connect\n");
      }
    // End extra scoping block
    }
  
    delete client;
  } else {
    Serial.println("[HTTPS SSO] Unable to create client");
  }

  return ret;
}