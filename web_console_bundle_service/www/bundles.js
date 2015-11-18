function add_action_handler(tag, command_text, id, state) {
        $(tag).click(function() {
                $.getJSON("/bundle?command=" + command_text + "&id=" + id, function(data) {
                        $("#state" + id).html = state;
                        show_bundles(data.Bundles); 
                });
                console.log(command_text + " Button clicked");   
        });
}

function show_bundles(bi) {
        $("#bundle_table").empty();
        var header = "<tr><th>Identification</th><th>State</th><th>Name</th><th>Location</th><th>Action</th></tr>";
        $("#bundle_table").append(header);
        for (var i = 0; i < bi.length; i++) {
                var button_text;
                var command_text;
                if(bi[i].state === "Active") {
                        button_text = "STOP";
                        command_text = "stop";
                } else {
                        button_text = "START";
                        command_text = "start";
                }
                var id = bi[i].id;
                var row = "<tr id=\"bundleid" + id + "\">";
                row += "<td>" + id + "</td>";
                row += "<td id=\"state" +  i + "\">" + bi[i].state + "</td>";
                row += "<td>" + bi[i].name + "</td>";
                row += "<td>" + bi[i].loc + "</td>";
                row += "<td><button id=\"action" + id + "\">" + button_text +"</button></td></tr>";
                $("#bundle_table").append(row);
                console.log("Active row add`ed");
                add_action_handler("#action" + i, command_text, id, bi[id].state);
        }
}

//
// Main
//
//var xmlhttp;

//xmlhttp = new XMLHttpRequest();
//xmlhttp.onreadystatechange = function() {
//        if(xmlhttp.readyState == 4 && xmlhttp.status == 200) {
//                var bundle_info = JSON.parse(xmlhttp.responseText);
//                show_bundles(bundle_info.Bundles);
//        }
//}                
        
function send_it() {
//        xmlhttp.open("GET","/bundle" , true);  
//        xmlhttp.send();
        $.getJSON("/bundle", function(data) {
                show_bundles(data.Bundles); 
        });
        setTimeout(send_it, 1000);
}      
$(document).ready(function() {send_it();});

