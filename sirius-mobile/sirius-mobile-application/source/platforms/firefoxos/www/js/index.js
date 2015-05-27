/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
var ip = '141.212.106.114',
    asr_port = '8080', 
    imm_port = '8081', 
    qa_port = '8082';

var app = {
    // Application Constructor
    initialize: function() {
        this.bindEvents();
    },
    // Bind Event Listeners
    //
    // Bind any events that are required on startup. Common events are:
    // 'load', 'deviceready', 'offline', and 'online'.
    bindEvents: function() {
        document.addEventListener('deviceready', this.onDeviceReady, false);
    },
    // deviceready Event Handler
    //
    // The scope of 'this' is the event. In order to call the 'receivedEvent'
    // function, we must explicitly call 'app.receivedEvent(...);'
    onDeviceReady: function() {
        app.receivedEvent('deviceready');
    },
    // Update DOM on a Received Event
    receivedEvent: function(id) {
        var parentElement = document.getElementById(id);
        var listeningElement = parentElement.querySelector('.listening');
        var receivedElement = parentElement.querySelector('.received');

        listeningElement.setAttribute('style', 'display:none;');
        receivedElement.setAttribute('style', 'display:block;');

        console.log('Received Event: ' + id);
    }
};

app.initialize();

var image, audio;

function loadXMLDoc(filename)
{
    if (window.XMLHttpRequest) {
      xhttp=new XMLHttpRequest();
    } else {
      xhttp=new ActiveXObject("Microsoft.XMLHTTP");
    }
    xhttp.open("GET",filename,false);
    xhttp.send();
    return xhttp.responseXML;
}

function onload() {
    // var xmlDoc = loadXMLDoc("config.xml");
    // ip = xmlDoc.getElementsByTagName("ip")[0].childNodes[0].nodeValue;
    // asr_port = xmlDoc.getElementsByTagName("asr")[0].childNodes[0].nodeValue;
    // imm_port = xmlDoc.getElementsByTagName("imm")[0].childNodes[0].nodeValue;
    // qa_port = xmlDoc.getElementsByTagName("qa")[0].childNodes[0].nodeValue;
    $('#ip_info').append("<p>IP: " + ip + "<br>ASR: " + asr_port + "<br>IMM: " + imm_port + "<br>QA: " + qa_port + "</p>");
}

window.addEventListener("load", onload);

// Called when capture operation is finished
function captureImageSuccess(mediaFiles) {
    var i, len;
    for (i = 0, len = mediaFiles.length; i < len; i += 1) {
        //uploadFile(mediaFiles[i]);
        image = mediaFiles[i];
        $('#image_file').empty();
        $('#image_file').append("Image File Ready");
    }
}

function captureAudioSuccess(mediaFiles) {
    var i, len;
    for (i = 0, len = mediaFiles.length; i < len; i += 1) {
        //uploadFile(mediaFiles[i]);
        audio = mediaFiles[i];
        $('#question').value = "";
        $('#audio_file').empty();
        $('#audio_file').append("Audio File Ready");
    }
}

// Called if something bad happens.
function captureError(error) {
    var msg = 'An error occurred during capture: ' + error.code;
    navigator.notification.alert(msg, null, 'Uh oh!');
}

// A button will call this function
function captureAudio() {
    // Launch device audio recording application,
    // allowing user to capture up to 2 audio clips
    navigator.device.capture.captureAudio(captureAudioSuccess, captureError, { });
}
document.getElementById("captureAudio").addEventListener("click",captureAudio);

 // A button will call this function            
function captureImage() {
    // Launch device camera application,
    // allowing user to capture up to 2 images
    navigator.device.capture.captureImage(captureImageSuccess, captureError, {});
}
document.getElementById("captureImage").addEventListener("click",captureImage);

function sendToServer() {
    
    if(image){
        // uploadFile(image);
    } else {
        if(audio){
            $('#response').empty();
            $('#response').append("<p>Sending...</p>");
            uploadFile(audio, getAddress(asr_port));
        } else {
            //send text to server
            q = document.getElementById("question").value
            if(q) {
                $('#response').empty();
                $('#response').append("<p>Sending...</p>");
                queryServer(q);
            } else {
                $('#response').empty();
                $('#response').append("<p>Nothing to send</p>");
            }
        }
    }
}
document.getElementById("sendToServer").addEventListener("click",sendToServer);

function getAddress(port) {
    return 'http://' + ip + ':' + port;
}

function queryServer(query) {
    q = getAddress(qa_port) + '?query=' + query;
    $.get(q).done(function( data ) {
        processResponse(data);
    });
}

// Upload files to server
function uploadFile(mediaFile, addr) {
    var ft = new FileTransfer(),
        path = mediaFile.fullPath,
        name = mediaFile.name;

    ft.upload(path,
        encodeURI(addr),
        function(result) {
            console.log('Upload success: ' + result.responseCode);
            console.log("Response: " + result.response);
            console.log(result.bytesSent + ' bytes sent');
            processResponse(result.response);
        },
        function(error) {
            console.log('Error uploading file ' + path + ': ' + error.code);
        },
        { fileName: name });
}

function processResponse(data) {
    $('#response').empty();
    if(data) {
        $('#response').append("<p>" + data + "</p>");
        TTS.speak({
            text: String(data),
            locale: 'en-GB',
            rate: 0.75
        }, function () {
            //do nothing
        }, function (reason) {
            navigator.notification.alert(reason, null, 'Uh oh!');
        });
    } else {
        $('#response').append("<p>Response is empty</p>");
    }
}

