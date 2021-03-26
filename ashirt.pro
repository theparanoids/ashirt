QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# App version number
SOURCE_CONTROL_REPO_PLAIN = $$getenv(GITHUB_REPOSITORY)

VERSION_TAG_PLAIN = $$getenv(GITHUB_REF)
COMMIT_HASH_PLAIN = $$getenv(GITHUB_SHA)

!contains(VERSION_TAG_PLAIN, .*tags/v.*) {
  message("Ref appears to not be a tag (Value: $$VERSION_TAG_PLAIN). Using non-version instead.")
  VERSION_TAG_PLAIN = v0.0.0-development
}

equals(COMMIT_HASH_PLAIN, "") {
  message("commit hash not specified. Please ensure GITHUB_SHA environment variable is set.")
  COMMIT_HASH_PLAIN = Unknown
}

equals(SOURCE_CONTROL_REPO_PLAIN, "") {
  message("Source control repo not specified. Please ensure GITHUB_REPOSITORY environment variable is set.")
}


VERSION_TAG = \\\"$$VERSION_TAG_PLAIN\\\"
COMMIT_HASH = \\\"$$COMMIT_HASH_PLAIN\\\"
SOURCE_CONTROL_REPO = \\\"$$SOURCE_CONTROL_REPO_PLAIN\\\"

message(Building with version: [$$VERSION_TAG]; Hash: [$$COMMIT_HASH]; Source Control: [$$SOURCE_CONTROL_REPO])

DEFINES += "VERSION_TAG=$$VERSION_TAG" \
           "COMMIT_HASH=$$COMMIT_HASH" \
           "SOURCE_CONTROL_REPO=$$SOURCE_CONTROL_REPO"

INCLUDEPATH += src

SOURCES += \
    src/appservers.cpp \
    src/components/aspectratio_pixmap_label/aspectratiopixmaplabel.cpp \
    src/components/aspectratio_pixmap_label/imageview.cpp \
    src/components/code_editor/codeblockview.cpp \
    src/components/code_editor/codeeditor.cpp \
    src/components/custom_keyseq_edit/singlestrokekeysequenceedit.cpp \
    src/components/error_view/errorview.cpp \
    src/components/evidence_editor/evidenceeditor.cpp \
    src/components/evidencepreview.cpp \
    src/components/flow_layout/flowlayout.cpp \
    src/components/loading/qprogressindicator.cpp \
    src/components/loading_button/loadingbutton.cpp \
    src/components/connection_checker/connectionchecker.cpp \
    src/components/servers_editor/connection_properties.cpp \
    src/components/servers_editor/servers_list.cpp \
    src/components/tagging/tag_cache/tagcache.cpp \
    src/components/tagging/tag_cache/tagcacheitem.cpp \
    src/components/tagging/tageditor.cpp \
    src/components/tagging/tagview.cpp \
    src/components/tagging/tagwidget.cpp \
    src/db/databaseconnection.cpp \
    src/forms/add_operation/createoperation.cpp \
    src/forms/evidence_filter/evidencefilter.cpp \
    src/forms/evidence_filter/evidencefilterform.cpp \
    src/forms/getinfo/getinfo.cpp \
    src/forms/porting/porting_dialog.cpp \
    src/forms/settings/connections_settingstab.cpp \
    src/forms/settings/general_settingstab.cpp \
    src/helpers/clipboard/clipboardhelper.cpp \
    src/migrations/migration.cpp \
    src/models/codeblock.cpp \
    src/helpers/multipartparser.cpp \
    src/hotkeymanager.cpp \
    src/main.cpp \
    src/porting/system_manifest.cpp \
    src/traymanager.cpp \
    src/helpers/screenshot.cpp \
    src/helpers/stopreply.cpp \
    src/forms/credits/credits.cpp \
    src/forms/evidence/evidencemanager.cpp \
    src/forms/settings/settings.cpp

HEADERS += \
    src/appservers.h \
    src/components/aspectratio_pixmap_label/aspectratiopixmaplabel.h \
    src/components/aspectratio_pixmap_label/imageview.h \
    src/components/code_editor/codeblockview.h \
    src/components/code_editor/codeeditor.h \
    src/components/custom_keyseq_edit/singlestrokekeysequenceedit.h \
    src/components/error_view/errorview.h \
    src/components/evidence_editor/deleteevidenceresponse.h \
    src/components/evidence_editor/evidenceeditor.h \
    src/components/evidence_editor/saveevidenceresponse.h \
    src/components/evidencepreview.h \
    src/components/flow_layout/flowlayout.h \
    src/components/loading/qprogressindicator.h \
    src/components/loading_button/loadingbutton.h \
    src/components/connection_checker/connectionchecker.h \
    src/components/servers_editor/connection_properties.h \
    src/components/servers_editor/servers_list.h \
    src/components/tagging/tag_cache/tagcache.h \
    src/components/tagging/tag_cache/tagcacheitem.h \
    src/components/tagging/tageditor.h \
    src/components/tagging/tagginglineediteventfilter.h \
    src/components/tagging/tagview.h \
    src/components/tagging/tagwidget.h \
    src/config/basic_config.h \
    src/config/config.h \
    src/config/config_v1.h \
    src/config/config_v2.h \
    src/config/no_config.h \
    src/config/server_item.h \
    src/config/server_set.h \
    src/config/server_v2.h \
    src/db/databaseconnection.h \
    src/db/query_result.h \
    src/dtos/github_release.h \
    src/dtos/checkConnection.h \
    src/exceptions/databaseerr.h \
    src/exceptions/fileerror.h \
    src/forms/add_operation/createoperation.h \
    src/forms/evidence_filter/evidencefilter.h \
    src/forms/evidence_filter/evidencefilterform.h \
    src/forms/getinfo/getinfo.h \
    src/forms/porting/porting_dialog.h \
    src/forms/settings/connections_settingstab.h \
    src/forms/settings/general_settingstab.h \
    src/helpers/clipboard/clipboardhelper.h \
    src/helpers/constants.h \
    src/helpers/request_builder.h \
    src/helpers/system_helpers.h \
    src/helpers/ui_helpers.h \
    src/migrations/migration.h \
    src/migrations/multi_server_migration.h \
    src/models/codeblock.h \
    src/helpers/file_helpers.h \
    src/helpers/http_status.h \
    src/hotkeymanager.h \
    src/models/evidence.h \
    src/models/server_setting.h \
    src/models/tag.h \
    src/porting/evidence_manifest.h \
    src/porting/system_manifest.h \
    src/porting/system_porting_options.h \
    src/traymanager.h \
    src/appconfig.h \
    src/appsettings.h \
    src/helpers/jsonhelpers.h \
    src/helpers/multipartparser.h \
    src/helpers/netman.h \
    src/helpers/screenshot.h \
    src/helpers/stopreply.h \
    src/dtos/tag.h \
    src/dtos/operation.h \
    src/forms/credits/credits.h \
    src/forms/evidence/evidencemanager.h \
    src/forms/settings/settings.h

include(tools/UGlobalHotkey/uglobalhotkey.pri)

macx {
  ICON = icons/ashirt.icns
  QMAKE_TARGET_BUNDLE_PREFIX = com.theparanoids
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    bin/update_migration_resource.py

RESOURCES += \
    res_migrations.qrc \
    res_icons.qrc
