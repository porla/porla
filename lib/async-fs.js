const fs = require('fs');
const { promisify } = require('util');

module.exports = {
    existsSync: fs.existsSync,
    mkdir: promisify(fs.mkdir),
    readdir: promisify(fs.readdir),
    readFile: promisify(fs.readFile),
    unlink: promisify(fs.unlink),
    writeFile: promisify(fs.writeFile)
};
