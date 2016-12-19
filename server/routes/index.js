var express = require("express"),
    router = express.Router(),
    Attendee = require("../models/atendee"),
    Dictionary = require("../models/dictionary");

router.get("/", function (req, res) {
    Attendee.find({}, function (error, attendees) {
        if (error) {
            console.log(error);
        } else {
            Dictionary.find({
                uid: attendees.map(function (attendee) {
                    return attendee.uid;
                })
            }, function (error, people) {
                if (error) {
                    console.log(error);
                }
                res.render("index", {attendees: people});
            });
        }
    });
});

router.post("/checkin", function (req, res) {
    var uid = req.body;
    Attendee.create({uid: uid}, function (error, attendee) {
        if (error)
            console.log(error);
    });
});

router.get("*", function (req, res) {
    res.redirect("/");
});

module.exports = router;