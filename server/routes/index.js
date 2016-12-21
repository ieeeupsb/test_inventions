var express = require("express"),
    router = express.Router(),
    fs = require('fs'),
    moment = require("moment"),
    configs = require("../configs.json"),
    Attendee = require("../models/attendee"),
    Dictionary = require("../models/dictionary"),
    JohnDoe = require("../models/johndoe");

router.get("/", function (req, res) {
    var today = moment().startOf('day');
    var tomorrow = moment(today).add(1, 'days');
    var npeople = 0;
    Attendee.find({
        time: {
            $gte: today.toDate(),
            $lt: tomorrow.toDate()
        }
    }, function (error, attendees) {
        if (error) {
            console.log(error);
        } else {
            JohnDoe.find({}, function (error, johndoe) {
                if (error) {
                    console.log(error);
                } else {
                    npeople = johndoe.length;
                }
            });
            Dictionary.find({
                    uid: {
                        $in: attendees.map(function (attendee) {
                            return attendee.uid
                        })
                    }
                },
                function (error, people) {
                    if (error) {
                        console.log(error);
                    } else {
                        res.render("index", {attendees: people, peopleMissing: npeople});
                    }
                });
        }
    });
});

router.get("/log", function (req, res) {
    // TODO: List with filter functionality to get all the checkins
    res.redirect("/");
});

router.post("/checkin", function (req, res) {
    var ok = false;
    var uid = req.body.uid;
    Attendee.create({uid: uid}, function (error, attendee) {
        if (error) {
            console.log(error);
        } else {
            Dictionary.findOne({uid: uid}, function (error, person) {
                if (error) {
                    console.log(error);
                } else if (person) {
                    ok = true;
                } else {
                    JohnDoe.findOne({uid: uid}, function (error, johndoe) {
                        if (error) {
                            console.log(error);
                        } else {
                            if (!johndoe) {
                                JohnDoe.create({uid: uid}, function (error, johndoe) {
                                    if (error) {
                                        console.log(error);
                                    }
                                });
                            }
                        }
                    });
                }
                res.send(ok ? "0" : "1");
            });
        }
    });
});

router.get("/johndoes", function (req, res) {
    JohnDoe.find({}, function (error, johndoes) {
        if (error) {
            console.log(error);
            res.render("johndoes", {error: error});
        } else {
            res.render("johndoes", {johndoes: johndoes, error: error});
        }
    })
});

router.post("/johndoes", function (req, res) {
    var uid = req.body.uid;
    Dictionary.create({name: req.body.name, uid: req.body.uid}, function (error, person) {
        if (error) {
            console.log(error);
        }
    });
    JohnDoe.remove({uid: uid}, function (error) {
        if (error) {
            console.log(error);
        }
    });
    res.redirect("/johndoes");
});

router.get("*", function (req, res) {
    res.redirect("/");
});

module.exports = router;