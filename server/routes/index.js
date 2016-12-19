var express = require("express"),
    router = express.Router(),
    Attendee = require("../models/atendee"),
    Dictionary = require("../models/dictionary");

router.get("/", function (req, res) {
    Attendee.find({}, function (error, attendees) {
        if (error) {
            console.log(error);
        } else {
            Dictionary.find({uid: attendees.map(attendee => attendee.uid)}, function (error, people) {
                if (error){
                    console.log(error);
                }
                res.render("index", {attendees: people.name});
            });
        }
    });
});

router.post("/entry", function (req, res) {
    var uid = req.body;
    Dictionary.findOne({uid: uid}, function (error, entry) {
        if (error) {
            console.log(error);
        } else {
            if (!entry.uid) {
                console.log("New user!");
                console.log(Date.now);
                console.log(uid);
            }
            Attendee.create({uid: uid}, function (error, attendee) {
                if (error)
                    console.log(error);
            });
        }
    });
});

router.get("*", function (req, res) {
    res.redirect("/");
});

module.exports = router;