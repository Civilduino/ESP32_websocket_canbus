const socket = new WebSocket('ws://myserver.local/ws');

// Connection opened
socket.addEventListener('open', function (event) {
    socket.send('Hello Server!');
});

// Listen for messages
socket.addEventListener('message', function (event) {
    console.log('Message from server ', event.data);

    var data = JSON.parse(event.data)
    document.getElementById("temp").innerHTML = data.temp;

});

function setColor(where, Color) {
    if (where == "backgroundcolour")
        document.getElementById('textarea').style.backgroundColor = Color;
    if (where == "colourtext")
        document.getElementById('textarea').style.color = Color;
};
