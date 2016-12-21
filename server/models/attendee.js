var mongoose = require("mongoose");

var attendeeSchema = new mongoose.Schema({
    uid: {
        type: String, unique: false
        , index: true
    },
    time: {type: Date, default: Date.now()}
});

module.exports = mongoose.model("Attendee", attendeeSchema);