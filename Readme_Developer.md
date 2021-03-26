# Developer Notes

## Building and Build Requirements

This application is built off of Qt 5, and utilizes's Qt's networking and Sql featuresets. To build, your specific system may need the following:

1. Qt 5, `qmake`, and possibly Qt Creator IDE.
   1. Binaries located [here](https://www.qt.io/download-qt-installer?hsCtaTracking=99d9dd4f-5681-48d2-b096-470725510d34%7C074ddad0-fdef-4e53-8aa8-5e8a876d6ab4). You may need to alter which downloader you install.
2. SQLite C driver (for SQLite version 3)
   1. On Fedora, this can be installed with `yum install sqlite-devel`
   2. On Arch systems, this can be installed with `pacman -S sqlite-doc`

## Versioning and Update Checks

This application has the ability to check for updates, and present a notification to the user that an update exists. In order to do this, the application needs to know a few key pieces of data. First, the application needs to know what version it is currently running. Second, it needs to know where to ask for new versions. Currently, the version check is accomplished by asking Github -- where this project is stored -- if there are any releases, and then manually checking those results against its stored version. The [Adding Versioning](#adding-versioning) section below details how these values are populated. Note, however, that for any user that wishes to fork this project, these sections will need to be modified in order to either point to your own service or repository, or disabled altogether.

This code is found by tracing its usage from it's initial request. Currently, this request started in `NetMan::checkForNewRelease`.

### Adding Versioning

This application attempts to add versioning data when building. This is accomplished by leveraging qmake's ability to add in preprocessor macros. This can be seen by looking in the `ashirt.pro` file, specifically looking for the `DEFINES` definition/updates, and tracing those additions back. For this project in particular a few fields are defined:

| Field                 | default Value      | Meaning                                                                                                                                                                                  |
| --------------------- | ------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `VERSION_TAG`         | v0.0.0-development | Contains the release tag, if any. Expects a Semantic Version. Displayed to the user, and used for update checks.                                                                         |
| `COMMIT_HASH`         | Unknown            | Contains the commit hash used when building this release. Displayed to the user                                                                                                          |
| `SOURCE_CONTROL_REPO` | (empty string)     | Contains the path to source control, relative to source control's domain. As this relates to github, this should be of the form `owner/repo`, or, for this project `theparanoids/ashirt` |

Currently, these fields are populated by looking at environment variables that GitHub Actions provides. If forking, these environment variables can be replaced by looking for `$$getenv(GITHUB_` prefixes in the `ashirt.pro` file

## Adding a db migration

Updating the database schema is slightly complicated, and the following rules must be followed:

1. Use the helper script `bin/create-migration.sh`
   * This will create file in the `migrations` folder with the indicated name and a timestamp
   * This should also add this migration to the qrc file. However, if this is not done, you can do this manually by editing the `rs_migrations.qrc` file.
2. Inside the new migration file, add the necessary sql to apply the db change under `-- +migrate Up`
3. Inside the new migration file, add the necessary sql to _undo_ the db change under `-- +migrate Down`
4. Multiple statements are not supported by default with sqlite3/Qt, however, support has been hacked together. To use multiple statements in a section, create a line with a single semicolon delimiting the statements.
   e.g.
   ```sql
   INSERT INTO names (first, last) VALUES ('john', 'doe')
   ;
   INSERT INTO names (first, last) VALUES ('jane', 'doe')
   ```

## Adding a new Evidence Filter

Evidence filters require modification in a few files and functions. Here is a rough checklist:

|                          | File                   | Function               | Notes                                                               |
| ------------------------ | ---------------------- | ---------------------- | ------------------------------------------------------------------- |
| <input type="checkbox"/> | evidencefilter.h       |                        | Need to add `FILTER_KEY_` and `FILTER_KEYS_` values                 |
| <input type="checkbox"/> | evidencefilter.cpp     | standardizeFilterKey   | Needed to map filter key alias to the one true filter key           |
| <input type="checkbox"/> | evidencefilter.cpp     | toString               | Need to represent a filter key/value as a string                    |
| <input type="checkbox"/> | evidencefilter.cpp     | parseFilter            | Need to be able to read filter key/value from a string              |
| <input type="checkbox"/> | databaseconnection.cpp | getEvidenceWithFilters | Need to translate the filter key/value to an appropriate sql clause |

Currently, there is already built-in support for adding filters of type:

* String (use the Operation filter as a guide)
* Boolean/Tri (Tris represent Yes/No/Any here, use Error filter as a guide)
* Date Range (use To/From filters as a guide)

## Formatting

This application adopts a modified [Google code style](https://google.github.io/styleguide/cppguide.html), applied via `clang-format`. Note that while formatting style is adhered to, other parts may not be followed, due to not starting with this style in mind.

Google style changes:
* Line limit to 100 characters
  * This is simply easier to read when dealing with long lines
* Line breaks before `else` and `catch`.
  * This is mostly because I find these easier to read.

To apply code formatting (Linux/Bash), run `find src/ -iname "*.cpp" -o -iname "*.h" | xargs clang-format -i`

## Known Issues

1. Evidence manager columns aren't ideally sized
2. No CLI for non-qt/non-tray OSes
3. Remove dock icon on mac?
   1. Possibly useful: https://github.com/keepassxreboot/keepassxc/commit/45344bb2acea61806d5e7f5f9eadfa779ca536ae#diff-a9e708931297992b08350ff7122fcb91R157

## Recommendations for tasks

1. Imports are not versioned or identifiable.
   1. Identification can be accomplished by adding a uuid to the system.json export, and having a table that keeps track of which imported uuids have been used
   2. Simple, perhaps ineffectual, versioning could be accomplished by using the ID above, and adding a column to the evidence table, indicating which export batch was associated with that evidence. The export `evidence.json` would then need to add this identification to each entry where this value was known.
   3. An alternative for identification could be to hash (e.g. md5) each file and provide that has in `evidence.json`. A similar hash would need to exist on each piece of evidence in the database. The system could then use that hash, in combination with other data, to determine if this evidence has been previously imported.
   4. A third alternative would be to go by the server's evidenceID or slug, if available (this is currently not tracked locally). A separate mechanism would be needed for non-submitted evidence.
