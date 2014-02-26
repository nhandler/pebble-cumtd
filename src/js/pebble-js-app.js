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
  //var coords = position.coords.latitude + "," + position.coords.longitude;
  //var transactionId = Pebble.sendAppMessage( { "firstKey": 42, "name": coords }, messageSendSuccess,messageSendFail );
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
  console.log("URL: " + url);
  req.open('GET', url, true);
  req.onload = function(e) {
    console.log("Request onload Function");
    if (req.readyState == 4 && req.status == 200) {
      if(req.status == 200) {
        var response = JSON.parse(req.responseText);
        var stop1= response.stops[0].stop_name;
        var stop2= response.stops[1].stop_name;
        var stop3= response.stops[2].stop_name;
        var stop4= response.stops[3].stop_name;
        var stop5= response.stops[4].stop_name;
        var code1= response.stops[0].stop_id;
        var code2= response.stops[1].stop_id;
        var code3= response.stops[2].stop_id;
        var code4= response.stops[3].stop_id;
        var code5= response.stops[4].stop_id;
        console.log("Stop1: " + stop1);
        console.log("Stop2: " + stop2);
        console.log("Stop3: " + stop3);
        console.log("Stop4: " + stop4);
        console.log("Stop5: " + stop5);
        console.log("Code1: " + code1);
        console.log("Code2: " + code2);
        console.log("Code3: " + code3);
        console.log("Code4: " + code4);
        console.log("Code5: " + code5);
        addMessage({"stop":stop1,"code":code1});
        addMessage({"stop":stop2,"code":code2});
        addMessage({"stop":stop3,"code":code3});
        addMessage({"stop":stop4,"code":code4});
        addMessage({"stop":stop5,"code":code5});
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
  console.log("URL: " + url);
  req.open('GET', url, true);
  req.onload = function(e) {
    console.log("Request onload Function");
    if (req.readyState == 4 && req.status == 200) {
      if(req.status == 200) {
        var response = JSON.parse(req.responseText);
        var hs1 = response.departures[0].headsign;
        var time1 = response.departures[0].expected_mins.toString() + " Minutes";
        var hs2 = response.departures[1].headsign;
        var time2 = response.departures[1].expected_mins.toString() + " Minutes";
        var hs3 = response.departures[2].headsign;
        var time3 = response.departures[2].expected_mins.toString() + " Minutes";
        var hs4 = response.departures[3].headsign;
        var time4 = response.departures[3].expected_mins.toString() + " Minutes";
        var hs5 = response.departures[4].headsign;
        var time5 = response.departures[4].expected_mins.toString() + " Minutes";
        console.log("HS1: " + hs1);
        console.log("HS2: " + hs2);
        console.log("HS3: " + hs3);
        console.log("HS4: " + hs4);
        console.log("HS5: " + hs5);
        console.log("TIME1: " + time1);
        console.log("TIME2: " + time2);
        console.log("TIME3: " + time3);
        console.log("TIME4: " + time4);
        console.log("TIME5: " + time5);
        addMessage({"code":stop,"headsign":hs1,"esttime":time1});
        addMessage({"code":stop,"headsign":hs2,"esttime":time2});
        addMessage({"code":stop,"headsign":hs3,"esttime":time3});
        addMessage({"code":stop,"headsign":hs4,"esttime":time4});
        addMessage({"code":stop,"headsign":hs5,"esttime":time5});
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
        console.log("Hello world! - Sent from your javascript application.");
        console.log("Pebble Account Token: " + Pebble.getAccountToken());
        Pebble.addEventListener("appmessage", messageReceived);
        getLocation();
    }
);
