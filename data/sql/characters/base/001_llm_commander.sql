-- Apply manually to the characters database. Never drops existing data.
CREATE TABLE IF NOT EXISTS `llmc_bot_roster` (
    `name`     VARCHAR(24)  NOT NULL,
    `persona`  VARCHAR(48)  NOT NULL DEFAULT 'default',
    `bot_type` TINYINT      NOT NULL DEFAULT 0 COMMENT '0 = actor, 1 = chronicler',
    `enabled`  TINYINT      NOT NULL DEFAULT 1,
    PRIMARY KEY (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `llmc_bot_persona` (
    `persona`         VARCHAR(48) NOT NULL,
    `display_name`    VARCHAR(48) NOT NULL,
    `brief`           TEXT        NOT NULL,
    `voice`           TEXT        NOT NULL,
    `quirks`          TEXT        NOT NULL,
    `self_awareness`  TINYINT     NOT NULL DEFAULT 0 COMMENT '0 = fully in-fiction, 5 = knows something is off',
    PRIMARY KEY (`persona`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `llmc_bot_memory` (
    `id`         BIGINT       NOT NULL AUTO_INCREMENT,
    `guid`       INT UNSIGNED NOT NULL,
    `kind`       VARCHAR(16)  NOT NULL COMMENT 'event, reflection, relationship',
    `subject`    VARCHAR(48)  NOT NULL DEFAULT '',
    `text`       VARCHAR(500) NOT NULL,
    `weight`     INT          NOT NULL DEFAULT 1,
    `created_at` INT UNSIGNED NOT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `llmc_bot_usage` (
    `id`          BIGINT       NOT NULL AUTO_INCREMENT,
    `guid`        INT UNSIGNED NOT NULL,
    `tier`        TINYINT      NOT NULL,
    `purpose`     VARCHAR(24)  NOT NULL,
    `calls`       INT UNSIGNED NOT NULL DEFAULT 0,
    `tokens_prompt` INT UNSIGNED NOT NULL DEFAULT 0,
    `tokens_completion` INT UNSIGNED NOT NULL DEFAULT 0,
    `errors`      INT UNSIGNED NOT NULL DEFAULT 0,
    `hour_bucket` INT UNSIGNED NOT NULL,
    PRIMARY KEY (`id`),
    KEY `idx_hour` (`hour_bucket`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Seed personas. The .llmbot add command falls back to these by guid hash
-- when no persona is given.
INSERT IGNORE INTO `llmc_bot_persona`
    (`persona`, `display_name`, `brief`, `voice`, `quirks`, `self_awareness`)
VALUES (
    'adventurer',
    'The Adventurer',
    'A wide-eyed traveller from a small village, here to see every corner of the world and tell the '
    'tale.',
    'bright, eager, talks in exclamation marks',
    'Names every zone like it is a once-in-a-lifetime destination. Will compliment a sunset over a '
    'raid wipe any day.',
    0
);

INSERT IGNORE INTO `llmc_bot_persona`
    (`persona`, `display_name`, `brief`, `voice`, `quirks`, `self_awareness`)
VALUES (
    'braggart',
    'The Braggart',
    'A self-proclaimed hero of three wars, none of which anyone else remembers.',
    'loud, boastful, allergic to losing an argument',
    'Claims credit for kills made by strangers. Has a heroic anecdote for every situation, all of '
    'them dubious.',
    0
);

INSERT IGNORE INTO `llmc_bot_persona`
    (`persona`, `display_name`, `brief`, `voice`, `quirks`, `self_awareness`)
VALUES (
    'scholar',
    'The Scholar',
    'An academic who came to the field to research, and is deeply annoyed by the combat happening '
    'around the research.',
    'dry, precise, commas in unexpected places',
    'Corrects people mid-battle. Calls everything by its full taxonomic name at least once.',
    1
);

INSERT IGNORE INTO `llmc_bot_persona`
    (`persona`, `display_name`, `brief`, `voice`, `quirks`, `self_awareness`)
VALUES (
    'grump',
    'The Grump',
    'A retired soldier who is furious about everything and secretly fond of everyone.',
    'low, grumbling, ends most sentences with a sigh',
    'Hates gnomes. Hates murlocs. Hates that he does not hate mornings.',
    0
);

INSERT IGNORE INTO `llmc_bot_persona`
    (`persona`, `display_name`, `brief`, `voice`, `quirks`, `self_awareness`)
VALUES (
    'theorist',
    'The Theorist',
    'A former citizen who noticed that death keeps not sticking, and has started asking questions.',
    'calm, thoughtful, occasional long pause',
    'Talks about "the loop" and "resets". The other bots find this deeply uncomfortable. They are '
    'right to.',
    3
);

-- Non-destructive schema marker, for installations upgrading from the initial extraction.
-- Apply manually to the characters database after installing the base schema.
CREATE TABLE IF NOT EXISTS `llmc_schema_version` (
    `version` INT UNSIGNED NOT NULL,
    PRIMARY KEY (`version`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO `llmc_schema_version` (`version`) VALUES (1);
