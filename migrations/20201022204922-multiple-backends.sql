-- +migrate Up

ALTER TABLE evidence ADD COLUMN server_uuid TEXT
;

UPDATE evidence SET server_uuid = '20a28c7c-ea24-4ee0-bb94-0ee63018d34b'
;


-- +migrate Down
DROP TABLE servers;
