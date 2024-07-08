-- Create table for module keys
CREATE TABLE IF NOT EXISTS module_keys (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    module_name TEXT NOT NULL,
    key_type TEXT NOT NULL,
    key_name TEXT NOT NULL,
    key_value BLOB NOT NULL,
    created_at DATETIME DEFAULT (datetime('now')),
    expires_at DATETIME,
    is_active INTEGER DEFAULT 1,
    UNIQUE(module_name, key_type, key_name)
);

-- Create index on module_name for faster lookups
CREATE INDEX IF NOT EXISTS idx_module_name ON module_keys(module_name);

-- Create index on key_type for faster lookups
CREATE INDEX IF NOT EXISTS idx_key_type ON module_keys(key_type);

-- Create index on is_active for faster filtering of active keys
CREATE INDEX IF NOT EXISTS idx_is_active ON module_keys(is_active);