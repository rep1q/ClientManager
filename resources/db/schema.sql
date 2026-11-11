-- 1. Cities
CREATE TABLE IF NOT EXISTS cities(
    id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL UNIQUE
);

-- 2. Clients
CREATE TABLE IF NOT EXISTS clients (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL
        CHECK (length(trim(name))>0),
    phone TEXT UNIQUE,
    email TEXT UNIQUE,
    age INTEGER
        CHECK(age IS NULL OR age BETWEEN 0 AND 99),
    city_id INTEGER,
    status_id INTEGER NOT NULL,

    FOREIGN KEY(city_id)
        REFERENCES cities(id)
        ON UPDATE CASCADE
        ON DELETE SET NULL,
    FOREIGN KEY(status_id)
        REFERENCES client_statuses(id)
        ON UPDATE CASCADE
        ON DELETE RESTRICT
);

-- 3. Client statuses
CREATE TABLE IF NOT EXISTS client_statuses(
    id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL UNIQUE, description TEXT
);

-- 4. Client notes
CREATE TABLE IF NOT EXISTS client_notes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    client_id INTEGER NOT NULL,
    content TEXT NOT NULL
        CHECK(length(trim(content)) > 0),
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (client_id)
        REFERENCES clients(id)
        ON DELETE CASCADE
);

-- 5. Client activities
CREATE TABLE IF NOT EXISTS client_activities(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    client_id INTEGER NOT NULL,
    action_type TEXT NOT NULL,
    description TEXT,
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (client_id)
        REFERENCES clients(id)
        ON DELETE CASCADE
);

-- INDEXES
CREATE INDEX IF NOT EXISTS id_clients_city_id
    ON clients(city_id);

CREATE INDEX IF NOT EXISTS id_clients_status_id
    ON clients(status_id);

CREATE INDEX IF NOT EXISTS id_client_notes_city_id
    ON client_notes(client_id);

CREATE INDEX IF NOT EXISTS id_client_activities_city_id
    ON client_activities(client_id);