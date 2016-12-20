var express = require("express"),
    router = express.Router(),
    slack = require("slack"),
    fs = require('fs'),
    configs = require("../configs.json");
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
    //var logFile = fs.writeFileSync('./log.txt', log, 'utf-8');
    res.writeHead(200, {
        'Content-Type': 'application/force-download',
        'Content-disposition': 'attachment; filename=NuIEEEMeetingLog.txt'
    });
    res.end(log);
    res.redirect("/");
});

router.post("/checkin", function (req, res) {
    var uid = req.body;
    Attendee.create({uid: uid}, function (error, attendee) {
        if (error)
            console.log(error);
    });
    Dictionary.findOne({uid: uid}, function (error, person) {
        if (error) {
            console.log(error);
        } else {
            slack.chat.postMessage({
                token: configs.slack.token,
                channel: "inventions_rfid",
                text: "New Person Checked in!\n" + uid
            }, function (error, data) {
                console.log(error);
            });
        }
    });
});

router.get("*", function (req, res) {
    res.redirect("/");
});

module.exports = router;