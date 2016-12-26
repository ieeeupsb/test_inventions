var express = require("express"),
    app = express(),
    bodyParser = require("body-parser"),
    mongoose = require("mongoose");

var port = 4444;

var indexRoutes = require("./routes/index");

app.use(bodyParser.urlencoded({extended: true}));

mongoose.connect("mongodb://localhost/rfid");
app.set("view engine", "ejs");
app.use(express.static(__dirname + "/public"));

app.use(indexRoutes);

app.listen(port, function () {
    console.log("Server running on port " + port + "!");
});