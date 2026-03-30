#!/bin/sh

if [ -d 'icon-development-kit' ]; then
    echo "Updating icon-development-kit..."
    pushd icon-development-kit
    git pull
    popd
else
    git clone https://gitlab.gnome.org/Teams/Design/icon-development-kit.git
fi

copy_icon() {
  echo "Copied icon-development-kit/icons/$1.svg to $2/$3.svg"
  cp "icon-development-kit/icons/$1.svg" "../$2/$3.svg"
}

# The following icons are in GTK and don't need updates for now
# speaker-mid          -> audio-volume-medium
# caps-lock            -> caps-lock
# one-way              -> dialog-error
# questionmark         -> dialog-question
# dialog-warning       -> dialog-warning
# document-save        -> document-save
# document-open        -> document-open
# clock                -> document-open-recent
# edit-copy            -> edit-copy
# scissors             -> edit-cut
# edit-paste           -> edit-paste
# loupe                -> edit-find
# music-node           -> folder-music
# image                -> folder-pictures
# video-camera         -> folder-videos
# go-down              -> go-down
# go-next              -> go-next
# go-previous          -> go-previous
# go-up                -> go-up
# list-add             -> list-add
# list-remove          -> list-remove
# media-playback-pause -> media-playback-pause
# media-playback-start -> media-playback-start
# object-select        -> object-select
# open-menu            -> open-menu
# pan-down             -> pan-down
# pan-end              -> pan-end
# pan-start            -> pan-start
# pan-up               -> pan-up
# process-working      -> process-working
# user-trash           -> user-trash
# eye-crossed          -> view-conceal
# view-grid            -> view-grid
# view-list            -> view-list
# view-more            -> view-more
# view-refresh         -> view-refresh
# eye                  -> view-reveal
# x                    -> window-close

copy_icon "adaptive"            "src/icons" "adw-adaptive-preview"
copy_icon "exit"                "src/icons" "adw-application-exit"
copy_icon "exit-rtl"            "src/icons" "adw-application-exit-rtl"
copy_icon "person"              "src/icons" "adw-avatar-default"
copy_icon "object-select"       "src/icons" "adw-entry-apply"
copy_icon "pencil"              "src/icons" "adw-entry-edit"
copy_icon "disclose-arrow-up"   "src/icons" "adw-expander-arrow"
copy_icon "external-link"       "src/icons" "adw-external-link"
copy_icon "mail-send"           "src/icons" "adw-mail-send"
copy_icon "rotate-acw"          "src/icons" "adw-rotate-acw"
copy_icon "rotate-cw"           "src/icons" "adw-rotate-cw"
copy_icon "camera"              "src/icons" "adw-screenshot"
copy_icon "sidebar-left"        "src/icons" "adw-sidebar"
copy_icon "sidebar-right"       "src/icons" "adw-sidebar-rtl"
copy_icon "tab-counter"         "src/icons" "adw-tab-counter"
copy_icon "tab-icon-missing"    "src/icons" "adw-tab-icon-missing"
copy_icon "plus-framed"         "src/icons" "adw-tab-new"
copy_icon "tab-overflow"        "src/icons" "adw-tab-overflow"
copy_icon "view-pin"            "src/icons" "adw-tab-unpin"


copy_icon "open-menu"            "src/icons/public" "open-menu"
copy_icon "view-more"            "src/icons/public" "view-more"
copy_icon "view-more-horizontal" "src/icons/public" "content-loading"
copy_icon "go-previous"          "src/icons/public" "go-previous"
copy_icon "go-next"              "src/icons/public" "go-previous-rtl"
copy_icon "go-next"              "src/icons/public" "go-next"
copy_icon "go-previous"          "src/icons/public" "go-next-rtl"
copy_icon "info-outline"         "src/icons/public" "info"
copy_icon "document-open"        "src/icons/public" "document-open"
copy_icon "document-save"        "src/icons/public" "document-save"
copy_icon "document-save-as"     "src/icons/public" "document-save-as"
copy_icon "pencil"               "src/icons/public" "edit"
copy_icon "edit-copy"            "src/icons/public" "edit-copy"
copy_icon "scissors"             "src/icons/public" "edit-cut"
copy_icon "edit-paste"           "src/icons/public" "edit-paste"
copy_icon "curved-arrow-left"    "src/icons/public" "undo"
copy_icon "curved-arrow-right"   "src/icons/public" "undo-rtl"
copy_icon "curved-arrow-right"   "src/icons/public" "redo"
copy_icon "curved-arrow-left"    "src/icons/public" "redo-rtl"
copy_icon "loupe"                "src/icons/public" "search"
copy_icon "x"                    "src/icons/public" "window-close"
copy_icon "plus-framed"          "src/icons/public" "tab-new"
copy_icon "paper-sheet-plus"     "src/icons/public" "document-new"
copy_icon "view-fullscreen"      "src/icons/public" "view-fullcreen"
copy_icon "view-restore"         "src/icons/public" "view-restore"
copy_icon "heart"                "src/icons/public" "favorite"
copy_icon "view-list"            "src/icons/public" "view-list"
copy_icon "view-grid"            "src/icons/public" "view-grid"
copy_icon "text-left"            "src/icons/public" "text-justify-left"
copy_icon "text-right"           "src/icons/public" "text-justify-right"
copy_icon "text-center"          "src/icons/public" "text-justify-center"
copy_icon "text-fill"            "src/icons/public" "text-justify-fill"
copy_icon "text-italic"          "src/icons/public" "text-italic"
copy_icon "text-bold"            "src/icons/public" "text-bold"
copy_icon "text-underline"       "src/icons/public" "text-underline"
copy_icon "text-strikethrough"   "src/icons/public" "text-strikethrough"
copy_icon "cogged-wheel"         "src/icons/public" "system-menu"
copy_icon "speaker-min"          "src/icons/public" "audio-volume-low"
copy_icon "speaker-mid"          "src/icons/public" "audio-volume-medium"
copy_icon "speaker-max"          "src/icons/public" "audio-volume-high"
copy_icon "attachment"           "src/icons/public" "attach"
copy_icon "sidebar-left"         "src/icons/public" "sidebar-left"
copy_icon "sidebar-right"        "src/icons/public" "sidebar-left-rtl"
copy_icon "sidebar-right"        "src/icons/public" "sidebar-right"
copy_icon "sidebar-left"         "src/icons/public" "sidebar-right-rtl"
copy_icon "clock"                "src/icons/public" "clock"
copy_icon "bell"                 "src/icons/public" "bell"
copy_icon "star"                 "src/icons/public" "star"
copy_icon "folder"               "src/icons/public" "folder"
copy_icon "import"               "src/icons/public" "import"
copy_icon "share"                "src/icons/public" "export"

copy_icon "bounce"              "demo/icons" "animations"
copy_icon "camera"              "demo/icons" "camera-photo"
copy_icon "video-camera"        "demo/icons" "camera-video"
copy_icon "alarm-clock"         "demo/icons" "clock-alarm"
copy_icon "stopwatch"           "demo/icons" "clock-stopwatch"
copy_icon "sand-watch"          "demo/icons" "clock-timer"
copy_icon "globe"               "demo/icons" "clock-world"
copy_icon "curved-arrow-right"  "demo/icons" "edit-redo"
copy_icon "curved-arrow-left"   "demo/icons" "edit-redo-rtl"
copy_icon "curved-arrow-left"   "demo/icons" "edit-undo"
copy_icon "curved-arrow-right"  "demo/icons" "edit-undo-rtl"
copy_icon "text-center"         "demo/icons" "format-justify-center"
copy_icon "text-fill"           "demo/icons" "format-justify-fill"
copy_icon "text-left"           "demo/icons" "format-justify-left"
copy_icon "text-right"          "demo/icons" "format-justify-right"
copy_icon "media-skip-backward" "demo/icons" "media-skip-backward"
copy_icon "media-skip-forward"  "demo/icons" "media-skip-forward"
copy_icon "blocks"              "demo/icons" "preferences-window-layout"
copy_icon "loupe"               "demo/icons" "preferences-window-search"
copy_icon "cleaning-brush"      "demo/icons" "style-classes"
copy_icon "speaker-cross"       "demo/icons" "tab-audio-muted"
copy_icon "speaker-cross-rtl"   "demo/icons" "tab-audio-muted-rtl"
copy_icon "speaker"             "demo/icons" "tab-audio-playing"
copy_icon "speaker-rtl"         "demo/icons" "tab-audio-playing-rtl"
copy_icon "plus-framed"         "demo/icons" "tab-new"
copy_icon "sidebar-right"       "demo/icons" "view-sidebar-end"
copy_icon "sidebar-left"        "demo/icons" "view-sidebar-end-rtl"
copy_icon "sidebar-left"        "demo/icons" "view-sidebar-start"
copy_icon "sidebar-right"       "demo/icons" "view-sidebar-start-rtl"
copy_icon "info-outline"        "demo/icons" "widget-about"
copy_icon "widget-banner"       "demo/icons" "widget-banner"
copy_icon "bottom-sheet"        "demo/icons" "widget-bottom-sheet"
copy_icon "widget-buttons"      "demo/icons" "widget-buttons"
copy_icon "carousel"            "demo/icons" "widget-carousel"
copy_icon "clamp"               "demo/icons" "widget-clamp"
copy_icon "dialog"              "demo/icons" "widget-dialog"
copy_icon "list"                "demo/icons" "widget-list"
copy_icon "multi-layout"        "demo/icons" "widget-multi-layout"
copy_icon "navigation-view"     "demo/icons" "widget-navigation-view"
copy_icon "sidebar-left"        "demo/icons" "widget-split-views"
copy_icon "sidebar-right"       "demo/icons" "widget-split-views-rtl"
copy_icon "tab-oldschool"       "demo/icons" "widget-tab-view"
copy_icon "bell"                "demo/icons" "widget-toast"
copy_icon "toggle-group"        "demo/icons" "widget-toggle-group"
copy_icon "view-switch"         "demo/icons" "widget-view-switcher"
copy_icon "text-left"           "demo/icons" "widget-wrap-box"
copy_icon "text-right"          "demo/icons" "widget-wrap-box-rtl"
copy_icon "view-dual"           "demo/icons" "view-dual"
copy_icon "libadwaita"          "demo/icons" "welcome"
copy_icon "window-new"          "demo/icons" "window-new"

copy_icon "camera"              "doc/tools/icons" "camera-photo"
copy_icon "video-camera"        "doc/tools/icons" "camera-video"
copy_icon "curved-arrow-right"  "doc/tools/icons" "edit-redo"
copy_icon "curved-arrow-left"   "doc/tools/icons" "edit-redo-rtl"
copy_icon "curved-arrow-left"   "doc/tools/icons" "edit-undo"
copy_icon "curved-arrow-right"  "doc/tools/icons" "edit-undo-rtl"
copy_icon "tab-icon-missing"    "doc/tools/icons" "generic"
copy_icon "media-skip-backward" "doc/tools/icons" "media-skip-backward"
copy_icon "media-skip-forward"  "doc/tools/icons" "media-skip-forward"
copy_icon "star"                "doc/tools/icons" "starred"
copy_icon "circle-check-cross"  "doc/tools/icons" "success"
copy_icon "bookmark"            "doc/tools/icons" "user-bookmarks"
