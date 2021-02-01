
var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
connection.onopen = function () {
  connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {
  console.log('Server: ', e.data);
};
connection.onclose = function () {
  console.log('WebSocket connection closed');
};

function colorP1(){
    color = document.getElementById("colorPicker1").value;
    if(color == null){
      console.log('null')
    } else {

      colorS =  color.toString(10)
      console.log(colorS)
      connection.send(colorS.substring(0,1) + "1" + colorS.substring(1));

    }
    

};

function colorP2(){
  color = document.getElementById("colorPicker2").value;
  if(color == null){
    console.log('null')
  } else {

    colorS = color.toString(10)
    console.log(colorS)
    connection.send(colorS.substring(0,1) + "2" + colorS.substring(1));

  }
  

};


function glow(){
  connection.send("glow");

}

function solid(){
  connection.send("solid");
}

function fade(){
  connection.send("fade");

}
function toggle(){
  connection.send("toggle");

}
function rainbow(){
  connection.send("rainbow");

}

function updateSpeed(){
  value = document.getElementById("speed").value
  connection.send('D' + value.toString())
  console.log('D' + value.toString())
}
