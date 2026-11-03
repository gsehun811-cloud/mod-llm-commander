-- Non-destructive schema marker, for installations upgrading from the initial extraction.
-- Apply manually to the characters database after installing the base schema.
CREATE TABLE IF NOT EXISTS `llmc_schema_version` (
    `version` INT UNSIGNED NOT NULL,
    PRIMARY KEY (`version`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO `llmc_schema_version` (`version`) VALUES (1);
