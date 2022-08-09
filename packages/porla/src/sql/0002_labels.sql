CREATE TABLE labels (
    id INTEGER PRIMARY KEY,
    addtorrentparams_id INTEGER NOT NULL REFERENCES addtorrentparams(id),
    name TEXT NOT NULL,
    value TEXT NOT NULL,
    UNIQUE(addtorrentparams_id,name)
);
