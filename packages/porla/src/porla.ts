import * as fs from "fs/promises";
import Session from "./session";
import {root as webRoot} from "@porla/webapp";

console.log(webRoot());

const session = new Session();
session.on("torrents:added", (err, torrent) => {
});

fs.readdir(__dirname + "/sql").then(console.log);
