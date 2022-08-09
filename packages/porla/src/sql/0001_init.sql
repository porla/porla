CREATE TABLE addtorrentparams (
    id INTEGER PRIMARY KEY,
    info_hash_v1 TEXT,
    info_hash_v2 TEXT,
    queue_position INTEGER NOT NULL,
    resume_data_buf TEXT NOT NULL,
    CHECK (info_hash_v1 IS NOT NULL OR info_hash_v2 IS NOT NULL),
    UNIQUE (info_hash_v1),
    UNIQUE (info_hash_v2)
);

CREATE TABLE sessionparams (
    data BLOB NOT NULL,
    timestamp INTEGER NOT NULL DEFAULT (strftime('%s'))
);

CREATE TABLE config (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);
