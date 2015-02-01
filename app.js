"use strict";


var fs = require("fs"),
    cp = require('child_process'),
    es = require("event-stream");

var http = require('http'),
    path = require('path'),
    os = require('os'),
    crypto = require("crypto");

var jade = require("jade"),
    Busboy = require('busboy'),
    files = require('node-static'),
    inspect = require('util').inspect;

// Server
  var file = new files.Server('./app');

  http.createServer(function(req, res) {
    if (req.method === 'POST') {
      return post(req, res);
    }
    switch (req.url) {
      case "/":
        getIndex(req, res);
        break;
      default:
        checkFile(req, res);
    }
  }).listen(3010, function() {
    console.log('Listening for requests');
  });

// Templates
  var tmplIndex = jade.compileFile("views/index.jade");
  var tmplLog = jade.compileFile("views/log.jade");
  var tmplFileSave = jade.compileFile("views/filesave.jade");

// Methods
  function post(req, res) {
    var busboy = new Busboy({ headers: req.headers });
    var id = crypto.randomBytes(7).toString('hex');
    
    var fields = {};
    fields.id = id;

    busboy.on('file', function(fieldname, file, filename, encoding, mimetype) {
      console.log('File [' + fieldname + ']: filename: ' + filename + ', encoding: ' + encoding + ', mimetype: ' + mimetype);
      
      var saveTo = path.join("./tmp", path.basename(id));
      file.pipe(fs.createWriteStream(saveTo));

      file.on('end', function() {
        console.log('File [' + fieldname + '] Finished to: ', saveTo);
        fields["file"] = id;
      });
    });

    busboy.on('field', function(fieldname, val, fieldnameTruncated, valTruncated) {
      console.log('Field [' + fieldname + ']: value: ' + inspect(val));
      if (fieldname != "text")
        fields[fieldname] = inspect(val);
      else
        fields[fieldname] = val;
    });

    busboy.on('finish', function() {
      console.log('Done parsing form!');
      finishPost(fields, req, res);
    });
    req.pipe(busboy);
  }

  function getIndex(req, res) {
    res.writeHead(200, { 'Connection': 'close' });
    var html = tmplIndex();
    res.end(html);
  }

  function finishPost(fields, req, res) {
    if (fields.file) {
      res.writeHead(303, { Connection: 'close', Location: '/' + fields.id });
      res.end();
    } else {
      fs.writeFile("logs/" + fields.id, fields.text, function (err) {
        if (err) throw err;
        res.writeHead(303, { Connection: 'close', Location: '/' + fields.id });
        res.end();
      });
    }
  }

// Files
  function checkFile(req, res) {
    var logFile = path.join("./logs", req.url);
    var tmpFile = path.join("./tmp", req.url);
    fs.exists(logFile, function(exists) {
      if (exists)
        showFile(req, res);
      else
      fs.exists(tmpFile, function(exists) {
        if (exists)
          parseRecord(req, res);
        else
          file.serve(req, res);
      });
    });
  }

  function showFile(req, res) {
    console.log("Showing ", req.url);
    fs.readFile("./logs" + req.url, function (err, html) {
      if (err) console.error(err);
      res.end(tmplLog({
        id: req.url,
        html: html
      }));
    })
  }

  function parseRecord(req, res) {
    console.log("Parsing ", req.url);
    var saveTo = path.join("./tmp", req.url);
    // var aha = spawn("./aha", [], { stdio: ['pipe', 1, 2, 'ipc'] });
    var aha = cp.exec("./aha", {
      encoding: "utf8",
      timeout: 0,
      maxBuffer: 500*1024
    });
    var html;

    var end = fs.createReadStream(saveTo)
      .pipe(es.duplex(aha.stdin, aha.stdout))

    end.on("data", function(chunk) {
      html += chunk;
    })

    end.on("close", function(code) {
      console.log("Written", req.url, html.length);
      res.end(tmplLog({
        id: req.url,
        html: html
      }));
      fs.writeFile(path.join("./logs", req.url), html);
    })

  }