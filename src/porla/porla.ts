import * as fs from "fs/promises";
import Session from "./session";

const session = new Session();
session.on("torrents:added", (err, torrent) => {
});

fs.readdir(__dirname + "/sql")
  .then(console.log);
