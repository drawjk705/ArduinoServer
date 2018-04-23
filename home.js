var temperature = {};
var readings = []
var maxTemp = Number.MIN_VALUE;
var minTemp = Number.MAX_VALUE;
var isCelsius = true;
var isStandby = false;
var isOffline = false;

$(document).ready(function () {

    var url = window.location.href;
    var path = window.location.pathname;

    console.log("URL is: " + url);

    $('button').click(function() {
        var value = $(this).val();
        console.log(value);
        if (((isStandby && value == 'w') || !isStandby) && !isOffline) {
            sendPost(value);
            if (value == 'f') {
                isCelsius = false;
                isStandby = false;
                isOffline = false;
            } else if (value == 'c'){
                isCelsius = true;
                isStandby = false;
                isOffline = false;
            } else if (value == 'q') {
                isStandby = true;
                isOffline = false;
            }
        }
    });

    function sendPost(value) {
        $.post(url, {value}, function(data, status) {
            console.log('sending ' + value + ' to arduino was a ' + status);
        });
        $.getJSON("error.json", function(json) {    
            $.each(json, function(key, val){
                var stat = json['status']; 
                console.log("STATUS == " + stat);   
                if (stat == 0) {
                    console.log('WE CAN BREAK')
                    return;
                } else {
                    sendPost(value);
                }
            });
        });
    }

    // get the json
    function getData() {
        // console.log('getting data')
        $.getJSON("output.json", function(data) {
            $.each(data, function(time, temp) {
                var stat = data["Status"];
                if (stat == 'F') {
                    isCelsius = false;
                    isStandby = false;
                    isOffline = false;
                    updatePage();
                } else if (stat == 'C'){
                    isCelsius = true;
                    isStandby = false;
                    isOffline = false;
                    updatePage();
                } else if (stat == 'Q') {
                    isStandby = true;
                    isOffline = false;
                    updatePage();
                } else if (stat == 'O') {
                    isOffline = true;
                    isStandby = false;
                    updatePage();
                }
                console.log('Status: ' + stat);
                if (!isNaN(parseFloat(temp))) {
                    temperature[time] = temp;
                }
                // console.log(readings);
            })
        });
    }

    getData();
    setInterval(getData, 5000);

    function updatePage() {

        // console.log('updating page');
        console.log('Standby mode is ' + isStandby);
        
        var a = "Current temperature: ";
        var b = "Max temperature: ";
        var c = "Min temperature: ";
        var d = "Average temperture: ";

        var current = readings[readings.length - 1];
        var curMax = maxTemp;
        var curMin = minTemp;
        var curAvg = averageReadingsForHour();

        // console.log('current = ' + current);
        // console.log('max = ' + curMax);
        // console.log('min = ' + curMin);
        // console.log('avg = ' + curAvg);

        if (!isStandby && !isOffline) {
            if (!isCelsius) {
                current = toFahrenheit(current) + ' F';
                curMax  = toFahrenheit(curMax)  + ' F';
                curMin  = toFahrenheit(curMin)  + ' F';
                curAvg  = toFahrenheit(curAvg)  + ' F';
            } else {
                current += ' C';
                curMax  += ' C';
                curMin  += ' C';
                curAvg  += ' C';
            }
            $('#1').text(a + current);
            $('#2').text(b + curMax);
            $('#3').text(c + curMin);
            $('#4').text(d + curAvg);
        } else if (isStandby) {
            $('#1').text("In standby mode");
            $('#2').text('');
            $('#3').text('');
            $('#4').text('');
        } else if (isOffline) {
            $('#1').text("ERROR: Arduino disconnected");
            $('#2').text('');
            $('#3').text('');
            $('#4').text('');
        }
        
    }
    updatePage();
    setInterval(updatePage(), 1000);


    function toFahrenheit(num) {
        return (num * 9 / 5) + 32;
    }

    function averageReadingsForHour() {
        var total = 0;
        var count = 0;
        readings = [];
        for (time in temperature) {
            if (withinHour(time)) {
                temp = temperature[time];
                temp = parseFloat(temp);
                if (!isNaN(temp)) {
                    if (temp > maxTemp) {
                        maxTemp = temp;
                    }
                    if (temp < minTemp) {
                        minTemp = temp;
                    }
                    readings.push(temp);
                    total += temp;
                    count += 1;
                }
            }
        }
        // console.log('total = ' + total);
        // console.log('count = ' + count);
        return total / count;
    }

    function withinHour(time) {

        // 15:32:56
        // console.log(time);
        // console.log(typeof time);

        var now = new Date();
        // console.log(now.getHours())
        // console.log(now.getMinutes())
        // console.log(now.getSeconds())

        var hour, min, sec;

        var split = time.split(":");

        if (now.getHours() <= split[0]) {
            return true;
        } else if (now.getMinutes() <= split[1]) {
            return true;
        } else {
            return false;
        }
    }

});