function set_temp(){
    let command = document.getElementsByName("temp_cmd");
    let actual_command;
    for(let i=0;i<command.length;i++){
        if(command[i].checked === true){
            actual_command = command[i].value;
        }
    }
    let requestLink = '/send_data?control=' + 'temp&command=' + actual_command;
    $.get(requestLink, function(data){
        if(data === '0'){
            console.log("finish setting!");
        }
    })
}

function set_led(){
    let red_value = document.getElementsByName("red_value").value;
    let green_value = document.getElementsByName("green_value").value;
    let blue_value = document.getElementsByName("blue_value").value;
    let command_value = red_value + '|' + green_value + '|' + blue_value
    let requestLink = '/send_data?control=led&command=' + command_value;
    $.get(requestLink, function(data){
        if(data === '0'){
            console.log("finish setting!");
        }
    })
}

function set_data(){
    let lcd_data = document.getElementById("lcd_print").value;
    console.log(lcd_data);
    let requestLink = '/send_data?control=data&command=' + lcd_data;
    $.get(requestLink, function(data){
        if(data === '0'){
            console.log("finish setting!");
        }
    })
}