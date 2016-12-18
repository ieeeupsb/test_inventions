var express = require("express");
var mysql = require("mysql");
var app = express();

var connection = mysql.createConnection({
    host     : 'localhost',
    user     : 'cards',
    password : 'secret',
    database : 'my_db'
});

app.post("/entry", function (req, res) {
    var uidEncripted = req.body;
    var uid = uidEncripted;
    connection.connect();

    /* 
    Assuming table with the entries is named log
    and as two collums (timestamp as DATETIME and uid as ...)
    */
    connection.query('INSERT INTO log(timestamp, uid) VALUES(CURRENT_TIMESTAMP, ' + uid + ')', function (err, rows, fields) {
        if (err) throw err;
    });

    connection.end();
});

app.listen(3000, function () {
    console.log("Server running on port 3000!");
});