

var log = document.getElementById("log");
var full = document.getElementById("full");

var lines = [];
var index = 0;

window.onload = function() {
  var text = full.innerHTML.split("\n");
  for (var i = 0; i < text.length; i++) {
    lines[i] = {time: 0, text: text[i]};
  }
  parseTimes(times);

  index = 0;
  showNext();
}

function parseTimes(tx) {
  var output = "";
  var currentLine = 0;

  _.each(_.groupBy(tx, "line"), (function(line) {
    waitTime = _.reduce(line, function(a, b){ return a + (b.time ? b.time : 0); }, 0);
    length = _.reduce(line, function(a, b){ return a + (b.length ? b.length : 0); }, 0);
    lines[currentLine].time = waitTime;
    lines[currentLine].length = length;
    output += line[0].line + " \n";

    currentLine++;
  }));

  document.getElementById("lines").innerHTML = output;
}

function showNext() {
  log.innerHTML += lines[index++].text + "\n";
  if (index < lines.length) {
    if (lines[index].time == 0) {
      showNext();
    } else {
      wait(lines[index].time * 500);
    }
  }
}

function wait(ms) {
  window.setTimeout(showNext, ms);
}