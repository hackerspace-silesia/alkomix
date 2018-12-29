#include <Arduino.h>
#include "webHandler.h"

void _resetData(WebData *data) {
    data->isReady = false;
    for (int i=0; i < 4; i++)
        data->percents[i] = 0;
}

String _parseData(ESP8266WebServer &server, WebData *data) {
    bool isReady = false;
    int percentSum = 0;

    for (int i=0; i < 4; i++) {
        String arg = String("pump:"); arg += i;
        int percent = server.arg(arg).toInt();
        if (percent < 0 || percent > 100) {
            _resetData(data);
            String msg = "<div class='alert-danger'>pump:"; msg += i; msg += " has a wrong value</div>";
            return msg;
        }
        data->percents[i] = percent;
        percentSum += percent;
    }

    if (percentSum > 100) { // try to normalize value
        for (int i=0; i < 4; i++) {
            int percent = data->percents[i];
            data->percents[i] = (percent * 100) / percentSum;
        }
    }

    data->isReady = true;
    return "<div class='alert-success'>RUN!</div>";
}

String _addPumpInput(int index, int percent) {
    String msg = "";
    msg += "<div class='form-group row'>";
      msg += "<label for='pump:"; msg += index; msg += "' class='col-sm-2 col-form-label'>PUMP "; msg += index; msg +=": </label>";
      msg += "<div class='col-sm-8'>";
        msg += "<input "; 
          msg += "id='pump-input-"; msg += index; msg +="'";
          msg += "name='pump:"; msg += index; msg +="'";
          msg += "class='form-control-range' type='range' min='0' max='100' step='5'";
          msg += "value='"; msg += percent; msg += "'";
          msg += "oninput='changePumpValue("; msg += index; msg += ")'";
          msg += "/>";
      msg += "</div>";
      msg += "<div id='pump-label-"; msg += index; msg += "' class='col-sm-2'></div>";
      msg += "<script>changePumpValue("; msg += index; msg += ")</script>";
    msg += "</div>";

    return msg;
}

void _sendMainPage(ESP8266WebServer &server, String additionalMsg, WebData *data) {
    String msg = "";
    msg += (
        "<head>"
            "<title>Alkomixer</title>"
            "<meta charset='utf-8' />"
            "<meta name='viewport' content='width=device-width, initial-scale=1, shrink-to-fit=no' />"
            "<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\">"
            "<script>"
              "function changePumpValue(index){"
              "document.querySelector('#pump-label-' + index).innerHTML = document.querySelector('#pump-input-' + index).value + ' %';"
              "}"
            "</script>"
        "</head>"
        "<body><div class='container'>"
            "<h1>Alkomixer</h1>"
    );

    msg += additionalMsg + "<br />";
    msg += "<form method='POST' action='/'>";
    msg += _addPumpInput(0, data->percents[0]);
    msg += _addPumpInput(1, data->percents[1]);
    msg += _addPumpInput(2, data->percents[2]);
    msg += _addPumpInput(3, data->percents[3]);

    msg += (
            "<button type='submit' class='btn btn-primary'>RUN!</button>"
            "</form>"
        "</div></body>"
    );
    server.send(200, "text/html", msg);
}

void handleMainPage(ESP8266WebServer &server, WebData *data) {
    _sendMainPage(server, "", data);
}

void handleNotFound(ESP8266WebServer &server, WebData *data) {
    _sendMainPage(server, "Srsly another pages doesnt exist ;_;", data);
}

void handlePostData(ESP8266WebServer &server, WebData *data) {
    String additionalMsg = _parseData(server, data);
    _sendMainPage(server, additionalMsg, data);
}
