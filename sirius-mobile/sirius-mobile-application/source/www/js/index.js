/*
 * Sirius Mobile Application
 * Developed with Apache Cordova and Apache Thrift
 * More info at http://sirius.clarity-lab.org/
 */

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

var image, audio, encodedAudioData = "", encodedImageData = "", text = "";
var storage = window.localStorage;
var mediaTimer = null;
var media = null;

function onload() {
    document.getElementById("ip").value = storage.getItem("ip");
    document.getElementById("port").value = storage.getItem("port");
    // document.getElementById("asr").value = storage.getItem("asr");
    // document.getElementById("imm").value = storage.getItem("imm");
    // document.getElementById("qa").value = storage.getItem("qa");
}

window.addEventListener("load", onload);

function updateDefaults(key, value) {
    storage.setItem(key, value);
}

function updateText(value) {
    text = value;
}

function updateResponseDiv(value) {
    $('#response').empty();
    $('#response').append("<p>" + value + "</p>");
}

function clear() {
    //variables
    clearAudio();
    clearImage();
    clearText();
    console.log("all media removed");
}

function clearAudio() {
    console.log("audio cleared");
    audio = null;
    encodedAudioData = "";
    $('#audio_file').empty();
}

function clearImage() {
    console.log("image cleared");
    image = null;
    encodedImageData = "";
    $('#image_file').empty();
}

function clearText() {
    text = "";
    $('#question').value = "";
}

// Called when capture operation is finished
function captureImageSuccess(mediaFiles) {
    var i, len;
    for (i = 0, len = mediaFiles.length; i < len; i += 1) {
        //uploadFile(mediaFiles[i]);
        image = mediaFiles[i];
        $('#image_file').empty();
        $('#image_file').append(image.name);
        $('#image_file').append("<button class='btnX' id='clearImage' style='margin-left:5px'>X</button>");
        document.getElementById("clearImage").addEventListener("click",clearImage);
        getFS(image, "audio");
    }
}

function captureAudioSuccess(mediaFiles) {
    var i, len;
    for (i = 0, len = mediaFiles.length; i < len; i += 1) {
        //uploadFile(mediaFiles[i]);
        audio = mediaFiles[i];
        $('#audio_file').empty();
        $('#audio_file').append(audio.name);
        $('#audio_file').append("<button class='btn2' id='playAudio' style='margin-left:3px'>Play</button>");
        $('#audio_file').append("<button class='btnX' id='clearAudio' style='margin-left:5px'>X</button>");
        document.getElementById("playAudio").addEventListener("click",playAudio);
        document.getElementById("clearAudio").addEventListener("click",clearAudio);
        getFS(audio, "audio");
        // $('#audio_file').append("<button class='btn2' id='pauseAudio'>Pause</button>");
        // $('#audio_file').append("<button class='btn2' id='stopAudio'>Stop</button>");
        // <p id='audio_position'></p>
        // $('#audio_file').append("<p id='audio_position'></p>");
        // document.getElementById("pauseAudio").addEventListener("click",pauseAudio);
        // document.getElementById("stopAudio").addEventListener("click",stopAudio);
    }
}

// Called if something bad happens.
function captureError(error) {
    var msg = 'An error occurred during capture: ' + error.code;
    navigator.notification.alert(msg, null, 'Uh oh!');
}

// A button will call this function
function captureAudio() {
    // Launch device audio recording application, allowing user to capture up to 1 audio clips
    navigator.device.capture.captureAudio(captureAudioSuccess, captureError, { limit: 1 });
}
document.getElementById("captureAudio").addEventListener("click",captureAudio);

 // A button will call this function            
function captureImage() {
    // Launch device camera application, allowing user to capture up to 1 images
    navigator.device.capture.captureImage(captureImageSuccess, captureError, { limit: 1 });
}
document.getElementById("captureImage").addEventListener("click",captureImage);

function playAudio() {
    if (audio == null) {
        // Nothing to play
        return;
    } 
    media = new Media(audio.fullPath, onSuccess, onError);
    // Play audio
    media.play();
}

// onSuccess Callback
//
function onSuccess() {
    console.log("playAudio():Audio Success");
}

// onError Callback
//
function onError(error) {
    alert('code: '    + error.code    + '\n' +
          'message: ' + error.message + '\n');
}

// function getPhoto() {
//     // Retrieve image file location from specified source
//     navigator.camera.getPicture(captureImageSuccess, captureError, {
//         quality: 30,
//         targetWidth: 600,
//         targetHeight: 600,
//         destinationType: destinationType.FILE_URI,
//         sourceType: pictureSource.PHOTOLIBRARY
//     });
// }
// document.getElementById("getImage").addEventListener("click",getPhoto;

function getFS(file, type){
    if(file){
        if(type == "audio") {
            console.log("audio file lookup");
            window.requestFileSystem(LocalFileSystem.TEMPORARY, 0, gotFSAudio, fail);
        } else {
            console.log("image file lookup");
            window.requestFileSystem(LocalFileSystem.TEMPORARY, 0, gotFSImage, fail);
        }
    } else {
        console.log("No file recorded!");
    }
}

function gotFSAudio(fileSystem) {
    fileSystem.root.getFile(audio.name, null, gotAudioEntry, fail);
}

function gotFSImage(fileSystem) {
    fileSystem.root.getFile(image.name, null, gotImageEntry, fail);
}

function gotAudioEntry(fileEntry) {
    fileEntry.file(readDataUrlAudio, fail);
}

function gotImageEntry(fileEntry) {
    fileEntry.file(readDataUrlImage, fail);
}

function readDataUrlAudio(file) {
    var reader = new FileReader();
    reader.onloadend = function(evt) {
        console.log("Read as data URL");
        console.log(evt.target.result);
        encodedAudioData = String(evt.target.result);
        encodedAudioData = encodedAudioData.replace(encodedAudioData.substr(0, encodedAudioData.search(",") + 1), "");
    };
    reader.readAsDataURL(file);
}

function readDataUrlImage(file) {
    var reader = new FileReader();
    reader.onloadend = function(evt) {
        console.log("Read as data URL");
        encodedImageData = String(evt.target.result);
        encodedImageData = encodedImageData.replace(encodedImageData.substr(0, encodedImageData.search(",") + 1), "");
        console.log(encodedImageData);
    };
    reader.readAsDataURL(file);
}

function fail(evt) {
    console.log(evt.target.error.code);
}

function sendFile(){
    console.log("Sending file");
    
    try {
        var addr = getAddress(getItem('port'), 'fts');
        var transport = new Thrift.TXHRTransport(addr);
        var protocol  = new Thrift.TJSONProtocol(transport);
        var client = new FileTransferSvcClient(protocol);


        var qType = new QueryType();
        qType.ASR = !!audio;
        qType.IMM = !!image;
        qType.QA = true;

        // var audioFile = new File();
        // audioFile.file = encodedData;
        // audioFile.b64format = true;

        // var txtFile = new File();
        // var immFile = new File();

        var qData = new QueryData();
        qData.audioFile = encodedAudioData;
        qData.imgFile = encodedImageData;
        qData.textFile = text;

        // var qData = new QueryData();
        // qData.audioFile = audioFile;
        // qData.textFile = txtFile;
        // qData.imgFile = immFile;

        // console.log("sending to client");
        client.send_file(qData, qType, window.device.uuid);
    } catch(err) {
        console.log(err);
        //could not connect to server
        if(err.name == "NETWORK_ERR") {
            navigator.notification.alert('There was a problem connecting to the server', null, 'Connection Error');
            updateResponseDiv("Error");
            return;
        } else if(err.name == "TIMEOUT_ERR") {
            navigator.notification.alert('There was a problem connecting to the server', null, 'Connection Error');
            updateResponseDiv("Error");
            return;
        }
        //otherwise ignore the error
    }    

     getResponse();
}
// document.getElementById("thriftMessage").addEventListener("click",getFS);

// function ping(){
//     console.log("Pinging server");
//     var addr = getAddress(getItem('port'), 'fts');
//     console.log(addr);
//     var transport = new Thrift.TXHRTransport(addr);
//     var protocol  = new Thrift.TJSONProtocol(transport);
//     //var client    = new CommandCenterClient(protocol);
//     var client = new FileTransferSvcClient(protocol);
    
//     var msg = client.ping();
//     $('#response').empty();
//     $('#response').append("<p>" + msg + "</p>");
//     console.log(msg);
//     //client.ping( function() { console.log("pinged server"); } );
//     console.log("Client Created");
// }
// document.getElementById("sendToServer").addEventListener("click",ping);

var timeoutFunc;

function getResponse() {
    var msg = "Waiting for response...";
    // var msg = "Waiting for response...    <button class='btnX' id='cancelRequest' style='margin-left:5px'>X</button>";
    updateResponseDiv(msg);
    // document.getElementById("cancelRequest").addEventListener("click",cancelRequest);

    var response = "processing";
    var addr = getAddress(getItem('port'), 'fts');
    var transport = new Thrift.TXHRTransport(addr);
    var protocol  = new Thrift.TJSONProtocol(transport);
    var client = new FileTransferSvcClient(protocol);
    try{
        response = client.get_response(window.device.uuid); 
        console.log(response);
    } catch(err) {
        console.log(err);
        if(err.name == "NETWORK_ERR") {
            navigator.notification.alert('There was a problem connecting to the server', null, 'Connection Error');
            updateResponseDiv("Error");
            return;
        } else if(err.name == "TIMEOUT_ERR") {
            navigator.notification.alert('There was a problem connecting to the server', null, 'Connection Error');
            updateResponseDiv("Error");
            return;
        }
    }

    //poll for response once a second
    if(response == "processing") {
        timeoutFunc = setTimeout(getResponse, 1000);
    } else {
        processResponse(response);
    }
}

function cancelRequest() {
    clearTimeout(timeoutFunc);
    updateResponseDiv("Request Canceled");
}
// document.getElementById("getResponse").addEventListener("click",getResponse);

function askServer() {
    if(audio || image || text) {
        updateResponseDiv("Sending...");
        sendFile();
    } else {
        console.log("Nothing to send!");
        navigator.notification.alert('Nothing to send!', null, 'Oops!');
    }
}
document.getElementById("askServer").addEventListener("click",askServer);

// function sendToServer() {
    
//     if(image){
//         // uploadFile(image);
//         $('#response').empty();
//         $('#response').append("<p>Sending...</p>");
//         type = "image/jpeg";
//         headers = {
//             Connection: "Close",
//             'Content-Type': String(type)
//         };
//         uploadFile(image, getAddress(getItem('imm')), type, headers);
//     } else {
//         if(audio){
//             $('#response').empty();
//             $('#response').append("<p>Sending...</p>");
//             type = "audio/wav";
//             headers = {
//                 Connection: "Close",
//                 'Content-Type': String(type + '; rate=16000')
//                 // 'Content-Type': String(type)
//             };
//             // if(String(window.device.platform) == "iOS"){
//             //     window.encodeAudio(audio.fullPath, success, fail);
//             // }
//             uploadFile(audio, getAddress(getItem('asr')), type, headers);
//         } else {
//             //send text to server
//             q = document.getElementById("question").value
//             if(q) {
//                 $('#response').empty();
//                 $('#response').append("<p>Sending...</p>");
//                 queryServer(q);
//             } else {
//                 $('#response').empty();
//                 $('#response').append("<p>Nothing to send</p>");
//             }
//         }
//     }
// }
// document.getElementById("sendToServer").addEventListener("click",sendToServer);

function getItem(key) {
    return document.getElementById(key).value;
}

function getAddress(port, destination) {
    return 'http://' + getItem('ip') + ':' + port + '/' + destination;
}

// function queryServer(query) {
//     q = getAddress(getItem('qa')) + '?query=' + query;
//     $.get(q).done(function( data ) {
//         processResponse(data);
//     });
// }

// // Upload files to server
// function uploadFile(mediaFile, addr, type, headers) {
//     var ft = new FileTransfer(),
//         path = mediaFile.fullPath,
//         name = mediaFile.name;

//     var options = new FileUploadOptions();
//     options.fileKey = 'file';
//     // options.fileName = name.replace(name.substr(0, name.lastIndexOf('/')+1), '/tmp/');
//     options.fileName = name;
//     options.mimeType = type;
//     options.chunkedMode = false;
//     options.headers = headers;

//     ft.upload(
//         path,
//         encodeURI(addr),
//         function(result) {
//             console.log('Upload success: ' + result.responseCode);
//             console.log("Response: " + result.response);
//             console.log(result.bytesSent + ' bytes sent');
//             processResponse(result.response);
//         },
//         function(error) {
//             $('#response').empty();
//             $('#response').append("<p>Error uploading file</p>");
//             console.log('Error uploading file ' + path + ': ' + error.code);
//         },
//         options);
// }

function processResponse(data) {
    if(data) {
        updateResponseDiv(data);
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
        updateResponseDiv("Response is empty");
    }
}

