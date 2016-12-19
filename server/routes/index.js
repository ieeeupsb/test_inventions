var express = require("express"),
    router = express.Router(),
    passport = require("passport"),
    User = require("../models/user");

router.get("/", function (req, res) {
    res.render("index");
});

router.get("*", function (req, res) {
    res.send("<h1>404 Not a valid path</h1>");
});

router.post("/entry", function (req, res) {
    var uidEncripted = req.body;
    var uid = uidEncripted;
});