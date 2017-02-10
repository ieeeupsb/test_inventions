var mongoose = require("mongoose");

var dictionarySchema = new mongoose.Schema({
    uid: String,
    name: String
});

module.exports = mongoose.model("Dictionary", dictionarySchema);