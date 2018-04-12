/* Tested using Chrome on MacOS
 * 
 */

$(function(){

        var interval = setInterval(sayHello, 1000);
        // when press a key, will 'reset' things
        
        $(this).keydown(function() {
                console.log('pressed')
                $("#special").html("");
            });

        function sayHello() {
                $("#special").append("hello<br>");
                console.log('hello');
         }

        function toCelsius(fahrenheit) {
                return (5/9) * (fahrenheit-32);
         }
                
};