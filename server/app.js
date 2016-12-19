var express = require("express"),
    app = express(),
    bodyParser = require("body-parser"),
    mongoose = require("mongoose");

var indexRoutes = require("./routes/index");

mongoose.connect("mongodb://localhost/rfid");
app.use(bodyParser.urlencoded({extended: true}));
app.set("view engine", "ejs");
app.use(express.static(__dirname + "/public"));

app.use(indexRoutes);

app.listen(3000, function () {
    console.log("Server running on port 3000!");
});