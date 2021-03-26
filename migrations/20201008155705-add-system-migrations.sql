-- +migrate Up

CREATE TABLE system_migrations (
  migration_name TEXT NOT NULL PRIMARY KEY
)
;

-- +migrate Down
