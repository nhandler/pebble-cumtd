var messageSendQueue = [];
var sent = 0;
var received = 0;
function sendNextMessage() {
  if (messageSendQueue.length === 0) {
    return;
  }
  var transactionId = Pebble.sendAppMessage( messageSendQueue.shift(), messageSendSuccess,messageSendFail );
  sent++;
} 

function addMessage(dict) {
  console.log("Queing Message: " + JSON.stringify(dict));
  messageSendQueue.push(dict);
  if (sent == received) {
    sendNextMessage();
  }
}
  
function messageReceived(e) {
  console.log("JavaScript Received message: " + e.payload.code);
  getDepartures(e.payload.code);
}

function messageSendSuccess(e) {
  console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
  received++;
  sendNextMessage();
}

function messageSendFail(e) {
  console.log("Unable to deliver message with transactionId=" + e.data.transactionId + " Error is: " + e.error.message);
  received++;
  sendNextMessage();
}

function gotLocation(position) {
  console.log("Latitude: " + position.coords.latitude);
  console.log("Longitude: " + position.coords.longitude);
  getStops(position.coords.latitude, position.coords.longitude);
}

function getLocation() {
  if ("geolocation" in navigator) {
    /* geolocation is available */
    console.log("Geolocation is available");
    navigator.geolocation.getCurrentPosition(gotLocation);
  }
  else {
    /* geolocation is NOT available */
    console.log("Geolocation is NOT available");
    //var transactionId = Pebble.sendAppMessage( { "firstKey": 42, "name": "Nathan" }, messageSendSuccess,messageSendFail );
  }
}

function getStops(lat,lon) {
  console.log("Getting Stops");
  var baseUrl = "https://developer.cumtd.com/api/v2.2/json/GetStopsByLatLon?";
  var key = "d65939a6432b46b38a65b53a998ed15f";
  var count = 5;
  var url = baseUrl + "key=" + key + "&lat=" + lat + "&lon=" + lon + "&count=" + count;
  var req = new XMLHttpRequest();
  req.overrideMimeType("application/json");
  console.log("URL: " + url);
  req.open('GET', url, true);
  req.onload = function(e) {
    console.log("Request onload Function");
    if (req.readyState == 4 && req.status == 200) {
      if(req.status == 200) {
        var response = JSON.parse(req.responseText);
        for (var i=0; i < 5 ; i++) {
            var stop = response.stops[i].stop_name;
            var code = response.stops[i].stop_id;
            if (stop) {
                stop = stop.toString();
                console.log("Stop: " + stop);
            }
            if (code) {
                code = code.toString();
                console.log("Code: " + code);
            }
            if (stop && code) {
                var message = JSON.parse(JSON.stringify({"stop":stop,"code":code}));
                addMessage(message);
            }
            else {
                console.log("NOT Queueing Message");
            }
        }
      }
      else {
        console.log("Error Getting Stops");
      }
    }
    else {
      console.log("Unknown Error Getting Stops: " + req.status);
    }
  };
  req.send(null);
}

function getDepartures(stop) {
  console.log("Getting Stops");
  var baseUrl = "https://developer.cumtd.com/api/v2.2/json/GetDeparturesByStop?";
  var key = "d65939a6432b46b38a65b53a998ed15f";
  var count = 5;
  var url = baseUrl + "key=" + key + "&stop_id=" + stop + "&count=" + count;
  var req = new XMLHttpRequest();
  req.overrideMimeType("application/json");
  console.log("URL: " + url);
  req.open('GET', url, true);
  req.onload = function(e) {
    console.log("Request onload Function");
    if (req.readyState == 4 && req.status == 200) {
      if(req.status == 200) {
        var response = JSON.parse(req.responseText);
        for (var i=0; i < 5 ; i++) {
          if (response.departures[i]) {
            var headsign = response.departures[i].headsign;
            var time = response.departures[i].expected_mins;
            if (headsign) {
              headsign = headsign.toString();
              console.log("Headsign: " + headsign);
            }
            if (time) {
              if (time === 0) {
                time = "Due";
              }
              else {
                time = time.toString() + " Minutes";
              }
              console.log("Time: " + time);
            }
            if (headsign && time) {
              var message = JSON.parse(JSON.stringify({"code":stop,"headsign":headsign,"esttime":time}));
              addMessage(message);
            }
            else {
              console.log("NOT Queueing Message");
            }
          }
          else {
            console.log("responses.departures[" + i + "] not defined");
            var message = JSON.parse(JSON.stringify({"code":stop,"headsign":"Departure N/A","esttime":"N/A"}));
            addMessage(message);
          }
        }
      }
      else {
        console.log("Error Getting Departures");
      }
    }
    else {
      console.log("Unknown Error Getting Departures: " + req.status);
    }
  };
  req.send(null);
}
Pebble.addEventListener("ready",
    function(e) {
        console.log("Pebble Account Token: " + Pebble.getAccountToken());
        Pebble.addEventListener("appmessage", messageReceived);
        getLocation();
    }
);
