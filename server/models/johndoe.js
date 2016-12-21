var mongoose = require("mongoose");

var johndoeSchema = new mongoose.Schema({
    uid: String,
    time: {type: Date, default: Date.now()}
});

module.exports = mongoose.model("JohnDoe", johndoeSchema);