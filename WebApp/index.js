function add(data){
    div = document.getElementById("info")
    div.innerHTML = ""
    data = JSON.parse(data)
    messages = []
    for(let i = 0; i < data.length; i++){
        entry = {}
        entry["id_time"]=data[i]["id_time"]["N"]
        entry["msg"]=data[i]["fill_level"]["M"]["message"]["S"]
        messages.push(entry)
    }
    messages.sort(function(a, b) {
        var keyA = a.id_time, keyB = b.id_time;
        if (keyA < keyB) return -1;
        if (keyA > keyB) return 1;
        return 0;
    });
    for(let i = 0; i < messages.length; i++){
        if (messages[i]["msg"]>330){
            msgvalue = 330
        }
        else if (messages[i]["msg"]<150) {
            msgvalue = 150
        }
        else {
            msgvalue = messages[i]["msg"]
        }
        msgvalue = 100-((msgvalue-150)*100)/180
        msgvalue = msgvalue.toFixed(2)
        dataora = new Date(parseInt(messages[i]["id_time"]))
        if (dataora.getMinutes()<10){
            time=dataora.getHours().toString() + ":0" + dataora.getMinutes().toString()
        }
        else {
            time=dataora.getHours().toString() + ":" + dataora.getMinutes().toString()
        }
        const para = document.createElement("p")
        const node = document.createTextNode(time + " -> " + msgvalue +"%")
        para.appendChild(node)
        div.appendChild(para)
        addData(time, msgvalue)
    }
    counter = document.getElementById("count")
    counter.innerHTML = messages.length.toString()

}

function callAPI(){
    // instantiate a headers object
    var myHeaders = new Headers();
    var requestOptions = {
        method: 'GET',
        headers: myHeaders,
    };
    // make API call with parameters and use promises to get response
    fetch("https://67iwrocdaj.execute-api.us-east-1.amazonaws.com/dev", requestOptions)
    .then(response => response.text())
    .then(result => add(JSON.parse(result).body))
}

var ch;

function createPlot() {
    var xValues = [];
    var yValues = [];

    ch = new Chart("myChart", {
        type: "line",
        data: {
            labels: xValues,
            datasets: [{
                fill: false,
                lineTension: 0,
                backgroundColor: "rgba(0,0,255,1.0)",
                borderColor: "rgba(0,0,255,0.1)",
                data: yValues
            }]
        },
        options: {
            legend: {display: false},
            scales: {
                yAxes: [{ticks: {min: 0, max:100}}],
            }
        }
    });
}

function addData(label, data) {
    ch.data.labels.push(label);
    ch.data.datasets.forEach((dataset) => {
        dataset.data.push(data);
    });
    ch.update();
}

function WebSocketLaunch() {
    if ("WebSocket" in window) {
        var ws = new WebSocket("wss://1c2u3s5olh.execute-api.us-east-1.amazonaws.com/production");
        ws.onmessage = function (evt) { 
            var received_msg = evt.data;
            hh = new Date().getHours()
            mm = new Date().getMinutes()
            if (mm<10){
                time=hh.toString() + ":0" + mm.toString()
            }
            else {
                time=hh.toString() + ":" + mm.toString()
            }
            msg = JSON.parse(received_msg)
            if (msg["message"]>330){
                msgvalue = 330
            }
            else if (msg["message"]<150) {
                msgvalue = 150
            }
            else {
                msgvalue = msg["message"]
            }
            msgvalue = 100-((msgvalue-150)*100)/180
            msgvalue = msgvalue.toFixed(2)
            addData(time, msgvalue)

            div = document.getElementById("info")
            const para = document.createElement("p")
            const node = document.createTextNode(time + " -> " + msgvalue +"%")
            para.appendChild(node)
            div.appendChild(para)
            div.scrollTop = div.scrollHeight;

            counter = document.getElementById("count")
            counter.innerHTML = parseInt(counter.innerHTML)+1;

            document.getElementById("dispensed").innerHTML = "Dispensed";
            setTimeout(function(){
                document.getElementById("dispensed").innerHTML = '';
            }, 2000);

            }
    } else {
        alert("WebSocket NOT supported by your Browser!");
    }
}

function Init(){
    createPlot();
    callAPI();
    WebSocketLaunch();

    hh = new Date().getHours()
    hh = hh-1
    mm = new Date().getMinutes()
    if (mm<10){
        time=hh.toString() + ":0" + mm.toString()
    }
    else {
        time=hh.toString() + ":" + mm.toString()
    }
    counter = document.getElementById("firsttime")
    counter.innerHTML = time
}

function dispense(){
    var myHeaders = new Headers();
    var requestOptions = {
        method: 'POST',
        headers: myHeaders
    };
    fetch("https://67iwrocdaj.execute-api.us-east-1.amazonaws.com/dev", requestOptions)
    .then(response => response.text())
    .then()
}