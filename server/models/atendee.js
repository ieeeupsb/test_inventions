var mongoose = require("mongoose");

var atendeeSchema = new mongoose.Schema({
    time: { type: Date, default: Date.now },
    uid: String
});

module.exports = mongoose.model("Atendee", atendeeSchema);