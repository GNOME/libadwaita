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
  echo "Copied icon-development-kit/icons/$1.svg to $2/icons/$3.svg"
  cp "icon-development-kit/icons/$1.svg" "../$2/icons/$3.svg"
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

copy_icon "adaptive"            "src" "adw-adaptive-preview"
copy_icon "exit"                "src" "adw-application-exit"
copy_icon "exit-rtl"            "src" "adw-application-exit-rtl"
copy_icon "person"              "src" "adw-avatar-default"
copy_icon "object-select"       "src" "adw-entry-apply"
copy_icon "pencil"              "src" "adw-entry-edit"
copy_icon "disclose-arrow-up"   "src" "adw-expander-arrow"
copy_icon "external-link"       "src" "adw-external-link"
copy_icon "mail-send"           "src" "adw-mail-send"
copy_icon "rotate-acw"          "src" "adw-rotate-acw"
copy_icon "rotate-cw"           "src" "adw-rotate-cw"
copy_icon "camera"              "src" "adw-screenshot"
copy_icon "sidebar-left"        "src" "adw-sidebar"
copy_icon "sidebar-right"       "src" "adw-sidebar-rtl"
copy_icon "tab-counter"         "src" "adw-tab-counter"
copy_icon "tab-icon-missing"    "src" "adw-tab-icon-missing"
copy_icon "plus-framed"         "src" "adw-tab-new"
copy_icon "tab-overflow"        "src" "adw-tab-overflow"
copy_icon "view-pin"            "src" "adw-tab-unpin"

copy_icon "bounce"              "demo" "animations"
copy_icon "camera"              "demo" "camera-photo"
copy_icon "video-camera"        "demo" "camera-video"
copy_icon "alarm-clock"         "demo" "clock-alarm"
copy_icon "stopwatch"           "demo" "clock-stopwatch"
copy_icon "sand-watch"          "demo" "clock-timer"
copy_icon "globe"               "demo" "clock-world"
copy_icon "curved-arrow-right"  "demo" "edit-redo"
copy_icon "curved-arrow-left"   "demo" "edit-redo-rtl"
copy_icon "curved-arrow-left"   "demo" "edit-undo"
copy_icon "curved-arrow-right"  "demo" "edit-undo-rtl"
copy_icon "text-center"         "demo" "format-justify-center"
copy_icon "text-fill"           "demo" "format-justify-fill"
copy_icon "text-left"           "demo" "format-justify-left"
copy_icon "text-right"          "demo" "format-justify-right"
copy_icon "media-skip-backward" "demo" "media-skip-backward"
copy_icon "media-skip-forward"  "demo" "media-skip-forward"
copy_icon "blocks"              "demo" "preferences-window-layout"
copy_icon "loupe"               "demo" "preferences-window-search"
copy_icon "cleaning-brush"      "demo" "style-classes"
copy_icon "speaker-cross"       "demo" "tab-audio-muted"
copy_icon "speaker-cross-rtl"   "demo" "tab-audio-muted-rtl"
copy_icon "speaker"             "demo" "tab-audio-playing"
copy_icon "speaker-rtl"         "demo" "tab-audio-playing-rtl"
copy_icon "plus-framed"         "demo" "tab-new"
copy_icon "sidebar-right"       "demo" "view-sidebar-end"
copy_icon "sidebar-left"        "demo" "view-sidebar-end-rtl"
copy_icon "sidebar-left"        "demo" "view-sidebar-start"
copy_icon "sidebar-right"       "demo" "view-sidebar-start-rtl"
copy_icon "info-outline"        "demo" "widget-about"
copy_icon "widget-banner"       "demo" "widget-banner"
copy_icon "bottom-sheet"        "demo" "widget-bottom-sheet"
copy_icon "widget-buttons"      "demo" "widget-buttons"
copy_icon "carousel"            "demo" "widget-carousel"
copy_icon "clamp"               "demo" "widget-clamp"
copy_icon "dialog"              "demo" "widget-dialog"
copy_icon "list"                "demo" "widget-list"
copy_icon "multi-layout"        "demo" "widget-multi-layout"
copy_icon "navigation-view"     "demo" "widget-navigation-view"
copy_icon "sidebar-left"        "demo" "widget-split-views"
copy_icon "sidebar-right"       "demo" "widget-split-views-rtl"
copy_icon "tab-oldschool"       "demo" "widget-tab-view"
copy_icon "bell"                "demo" "widget-toast"
copy_icon "toggle-group"        "demo" "widget-toggle-group"
copy_icon "view-switch"         "demo" "widget-view-switcher"
copy_icon "text-left"           "demo" "widget-wrap-box"
copy_icon "text-right"          "demo" "widget-wrap-box-rtl"
copy_icon "view-dual"           "demo" "view-dual"
copy_icon "libadwaita"          "demo" "welcome"
copy_icon "window-new"          "demo" "window-new"

copy_icon "camera"              "doc/tools" "camera-photo"
copy_icon "video-camera"        "doc/tools" "camera-video"
copy_icon "curved-arrow-right"  "doc/tools" "edit-redo"
copy_icon "curved-arrow-left"   "doc/tools" "edit-redo-rtl"
copy_icon "curved-arrow-left"   "doc/tools" "edit-undo"
copy_icon "curved-arrow-right"  "doc/tools" "edit-undo-rtl"
copy_icon "tab-icon-missing"    "doc/tools" "generic"
copy_icon "media-skip-backward" "doc/tools" "media-skip-backward"
copy_icon "media-skip-forward"  "doc/tools" "media-skip-forward"
copy_icon "star"                "doc/tools" "starred"
copy_icon "circle-check-cross"  "doc/tools" "success"
copy_icon "bookmark"            "doc/tools" "user-bookmarks"
