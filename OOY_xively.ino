/**
 * Make a HTTP connection to the Xively server
 */
void sendData() {
  // form the string for the API header parameter:
  String apiString = "X-ApiKey: ";
  apiString += APIKEY;
 
  // form the string for the URL parameter:
  String url = "https://api.xively.com/v2/feeds/";
  url += FeedId;
  url += ".csv";
 
  // Send the HTTP PUT request to xively.com
 
  Process xively;
  Console.print("Sending data... ");
  xively.begin("curl");
  xively.addParameter("-k");
  xively.addParameter("--request");
  xively.addParameter("PUT");
  xively.addParameter("--data");
  xively.addParameter(dataString);
  xively.addParameter("--header");
  xively.addParameter(apiString);
  xively.addParameter(url);
  xively.run();
  Console.println("done!");
 
  // If there's incoming data from the net connection,
  // send it out the Serial:
  while (xively.available()>0) {
    char c = xively.read();
    Console.write(c);
  }
}
