
$(document).ready(function () {

    var url = window.location.href;
    var path = window.location.pathname;
    var temperature = {};
    var readings = []
    var maxTemp = Number.MIN_VALUE;
    var minTemp = Number.MAX_VALUE;
    var total = 0;
    var stat = null;

    console.log("URL is: " + url);

    $('button').click(function() {
        var value = $(this).val();
        if (value == 'f') {
            stat = 'F';
        } else if (value == 'c'){
            stat = 'C';
        } else if (value == 'q') {
            stat = 'Q'
        }
        console.log(value);
        $.post(url, {value}, function(data, status) {
        });
    });

    // get the json
    function getData() {
        console.log('getting data')
        $.getJSON("output.json", function(data) {
            $.each(data, function(time, temp) {
                stat = data["Status"];
                console.log('Status: ' + stat);
                if (!isNaN(parseFloat(temp))) {
                    temperature[time] = temp;
                }
                // console.log(readings);
            })
        });
        readingsForHour();
        updatePage();
    }

    getData();
    setInterval(getData, 5000);

    function updatePage() {

        console.log('updating page');
        
        var a = "Current temperature: ";
        var b = "Max temperature: ";
        var c = "Min temperature: ";
        var d = "Average temperture: ";

        var current = readings[readings.length - 1];
        var curMax = maxTemp;
        var curMin = minTemp;
        var curAvg = total / readings.length;

        console.log('current = ' + current);
        console.log('max = ' + curMax);
        console.log('min = ' + curMin);
        console.log('avg = ' + curAvg);

        if (stat == 'F') {
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

        if (stat != 'Q' && status != 'O') {
            $('#1').text(a + current);
            $('#2').text(b + curMax);
            $('#3').text(c + curMin);
            $('#4').text(d + curAvg);
        } else if (stat == 'S') {
            $('#1').text("In standby mode");
            $('#2').text('');
            $('#3').text('');
            $('#4').text('');
        } else if (stat == 'O') {
            $('#1').text("ERROR: Arduino disconnected");
            $('#2').text('');
            $('#3').text('');
            $('#4').text('');
        }
        
    }

    updatePage();
    setInterval(updatePage(), 5000);


    function toFahrenheit(num) {
        return (num * 9 / 5) + 32;
    }

    function readingsForHour() {
        total = 0;
        readings = [];
        for (time in temperature) {
            if (withinHour(time)) {
                temp = temperature[time];
                if (temp > maxTemp) {
                    maxTemp = temp;
                }
                if (temp < minTemp) {
                    minTemp = temp;
                }
                readings.push(temp);
                total += temp;
            }
        }
    }

    function withinHour(time) {

        // 15:32:56
        console.log(time);
        console.log(typeof time);

        var now = new Date();
        console.log(now.getHours())
        console.log(now.getMinutes())
        console.log(now.getSeconds())

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