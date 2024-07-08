-- Create table for key types
CREATE TABLE IF NOT EXISTS key_types (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    type_name TEXT NOT NULL UNIQUE
);

-- Insert predefined key types
INSERT OR IGNORE INTO key_types (type_name) VALUES 
    ('CURVE25519'),
    ('ED25519'),
    ('AES'),
    ('RSA'),
    ('API_KEY');

-- Modify the module_keys table
CREATE TABLE IF NOT EXISTS module_keys (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    module_name TEXT NOT NULL,
    key_type_id INTEGER NOT NULL,
    key_name TEXT NOT NULL,
    key_value BLOB NOT NULL,
    created_at DATETIME DEFAULT (datetime('now')),
    expires_at DATETIME,
    is_active INTEGER DEFAULT 1,
    FOREIGN KEY (key_type_id) REFERENCES key_types(id),
    UNIQUE(module_name, key_type_id, key_name)
);

-- Create index on module_name for faster lookups
CREATE INDEX IF NOT EXISTS idx_module_name ON module_keys(module_name);

-- Create index on key_type_id for faster lookups
CREATE INDEX IF NOT EXISTS idx_key_type_id ON module_keys(key_type_id);

-- Create index on is_active for faster filtering of active keys
CREATE INDEX IF NOT EXISTS idx_is_active ON module_keys(is_active);