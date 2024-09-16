-- PRAGMA journal_mode = DELETE;

CREATE TABLE quotes(
    instrument varchar(15) PRIMARY KEY NOT NULL,
    bid real,
    ask real,
    timestamp integer
);


PRAGMA journal_mode;
PRAGMA journal_mode = WAL;