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
                } else {
                    res.render("index", {attendees: people});
                }
            });
        }
    });
});

router.get("/log", function (req, res) {
    var log = "";
    Attendee.find({}, function (error, attendees) {
        if (error) {
            console.log(error);
        } else {
            attendees.forEach(function (attendee) {
                Dictionary.findOne({
                    uid: attendee.uid
                }, function (error, people) {
                    if (error) {
                        console.log(error);
                    } else {
                        log += attendee.time + " : " + people.name + "\n";
                    }
                });
            });
        }
    });
    fs.writeFileSync('./log.txt', log, 'utf-8');
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