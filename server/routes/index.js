var express = require("express"),
    router = express.Router(),
    fs = require('fs'),
    moment = require("moment"),
    /*configs = require("../configs.json"),*/
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
                Dictionary.find({
                        uid: {
                            $in: attendees.map(function (attendee) {
                                return attendee.uid;
                            })
                        }
                    },
                    function (error, people) {
                        if (error) {
                            console.log(error);
                        } else {
                            res.render("index", {path: "home", attendees: people, peopleMissing: npeople});
                        }
                    });
            });

        }
    });
});

router.get("/log", function (req, res) {
    res.send("<h1>Feature comming soon</h1>");
    // TODO: List with filter functionality to get all the checkins
});

router.post("/checkin", function (req, res) {
    console.log(req.body);
    if(!req.body.uid){
        console.log("Empty uid!");
        res.send("1");
        res.end();
    }
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
                    res.send("0");
                    res.end();
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
                                    res.send("1");
                                    res.end();
                                });
                            } else {
                                res.send("1");
                                res.end();
                            }
                        }
                    });
                }
            });
        }
    });
});

router.get("/johndoes", function (req, res) {
    JohnDoe.find({}, function (error, johndoes) {
        if (!johndoes.length) res.redirect("/");
        res.render("johndoes", {path: "", johndoes: johndoes});
    });
});

router.post("/johndoes", function (req, res) {
    Dictionary.create({name: req.body.name, uid: req.body.uid, photo: req.body.photo}, function (error) {
        JohnDoe.remove({uid: req.body.uid}, function (error) {
            res.redirect("/johndoes");
        });
    });
});

router.get("/about", function (req, res) {
    JohnDoe.find({}, function (error, johndoes) {
        res.render("about", {path: "about", error: error, peopleMissing: johndoes.length});
    });
});

router.get("*", function (req, res) {
    res.redirect("/");
});

module.exports = router;