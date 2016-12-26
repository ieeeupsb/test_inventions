var mongoose = require("mongoose");

var dictionarySchema = new mongoose.Schema({
    uid: String,
    name: String,
    photo: String
});

module.exports = mongoose.model("Dictionary", dictionarySchema);