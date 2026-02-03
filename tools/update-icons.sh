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
  echo "Copied icon-development-kit/icons/$1.svg to $2/icons/scalable/$3-symbolic.svg"
  cp "icon-development-kit/icons/$1.svg" "../$2/icons/scalable/$3-symbolic.svg"
}

copy_icon_rtl() {
  echo "Copied icon-development-kit/icons/$1.svg to $2/icons/scalable/$3-symbolic-rtl.svg"
  cp "icon-development-kit/icons/$1.svg" "../$2/icons/scalable/$3-symbolic-rtl.svg"
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

copy_icon     "adaptive"            "src"  "actions/adw-adaptive-preview"
copy_icon     "exit"                "src"  "actions/adw-application-exit"
copy_icon_rtl "exit-rtl"            "src"  "actions/adw-application-exit"
copy_icon     "object-select"       "src"  "actions/adw-entry-apply"
copy_icon     "pencil"              "src"  "actions/adw-entry-edit"
copy_icon     "disclose-arrow-up"   "src"  "actions/adw-expander-arrow"
copy_icon     "external-link"       "src"  "actions/adw-external-link"
copy_icon     "mail-send"           "src"  "actions/adw-mail-send"
copy_icon     "rotate-acw"          "src"  "actions/adw-rotate-acw"
copy_icon     "rotate-cw"           "src"  "actions/adw-rotate-cw"
copy_icon     "camera"              "src"  "actions/adw-screenshot"
copy_icon     "sidebar-left"        "src"  "actions/adw-sidebar"
copy_icon_rtl "sidebar-right"       "src"  "actions/adw-sidebar"
copy_icon     "plus-framed"         "src"  "actions/adw-tab-new"

copy_icon     "person"              "src"  "status/adw-avatar-default"
copy_icon     "tab-counter"         "src"  "status/adw-tab-counter"
copy_icon     "tab-icon-missing"    "src"  "status/adw-tab-icon-missing"
copy_icon     "tab-overflow"        "src"  "status/adw-tab-overflow"
copy_icon     "view-pin"            "src"  "status/adw-tab-unpin"

copy_icon     "bounce"              "demo" "actions/animations"
copy_icon     "camera"              "demo" "actions/camera-photo"
copy_icon     "video-camera"        "demo" "actions/camera-video"
copy_icon     "alarm-clock"         "demo" "actions/clock-alarm"
copy_icon     "stopwatch"           "demo" "actions/clock-stopwatch"
copy_icon     "sand-watch"          "demo" "actions/clock-timer"
copy_icon     "globe"               "demo" "actions/clock-world"
copy_icon     "curved-arrow-right"  "demo" "actions/edit-redo"
copy_icon_rtl "curved-arrow-left"   "demo" "actions/edit-redo"
copy_icon     "curved-arrow-left"   "demo" "actions/edit-undo"
copy_icon_rtl "curved-arrow-right"  "demo" "actions/edit-undo"
copy_icon     "text-center"         "demo" "actions/format-justify-center"
copy_icon     "text-fill"           "demo" "actions/format-justify-fill"
copy_icon     "text-left"           "demo" "actions/format-justify-left"
copy_icon     "text-right"          "demo" "actions/format-justify-right"
copy_icon     "media-skip-backward" "demo" "actions/media-skip-backward"
copy_icon     "media-skip-forward"  "demo" "actions/media-skip-forward"
copy_icon     "blocks"              "demo" "actions/preferences-window-layout"
copy_icon     "loupe"               "demo" "actions/preferences-window-search"
copy_icon     "cleaning-brush"      "demo" "actions/style-classes"
copy_icon     "plus-framed"         "demo" "actions/tab-new"
copy_icon     "sidebar-right"       "demo" "actions/view-sidebar-end"
copy_icon_rtl "sidebar-left"        "demo" "actions/view-sidebar-end"
copy_icon     "sidebar-left"        "demo" "actions/view-sidebar-start"
copy_icon_rtl "sidebar-right"       "demo" "actions/view-sidebar-start"
copy_icon     "info-outline"        "demo" "actions/widget-about"
copy_icon     "widget-banner"       "demo" "actions/widget-banner"
copy_icon     "bottom-sheet"        "demo" "actions/widget-bottom-sheet"
copy_icon     "widget-buttons"      "demo" "actions/widget-buttons"
copy_icon     "carousel"            "demo" "actions/widget-carousel"
copy_icon     "clamp"               "demo" "actions/widget-clamp"
copy_icon     "dialog"              "demo" "actions/widget-dialog"
copy_icon     "list"                "demo" "actions/widget-list"
copy_icon     "multi-layout"        "demo" "actions/widget-multi-layout"
copy_icon     "navigation-view"     "demo" "actions/widget-navigation-view"
copy_icon     "sidebar-left"        "demo" "actions/widget-split-views"
copy_icon_rtl "sidebar-right"       "demo" "actions/widget-split-views"
copy_icon     "tab-oldschool"       "demo" "actions/widget-tab-view"
copy_icon     "bell"                "demo" "actions/widget-toast"
copy_icon     "toggle-group"        "demo" "actions/widget-toggle-group"
copy_icon     "view-switch"         "demo" "actions/widget-view-switcher"
copy_icon     "text-left"           "demo" "actions/widget-wrap-box"
copy_icon_rtl "text-right"          "demo" "actions/widget-wrap-box"
copy_icon     "view-dual"           "demo" "actions/view-dual"
copy_icon     "window-new"          "demo" "actions/window-new"

copy_icon     "speaker-cross"       "demo" "status/tab-audio-muted"
copy_icon_rtl "speaker-cross-rtl"   "demo" "status/tab-audio-muted"
copy_icon     "speaker"             "demo" "status/tab-audio-playing"
copy_icon_rtl "speaker-rtl"         "demo" "status/tab-audio-playing"

copy_icon     "camera"              "doc/tools" "actions/camera-photo"
copy_icon     "video-camera"        "doc/tools" "actions/camera-video"
copy_icon     "curved-arrow-right"  "doc/tools" "actions/edit-redo"
copy_icon_rtl "curved-arrow-left"   "doc/tools" "actions/edit-redo"
copy_icon     "curved-arrow-left"   "doc/tools" "actions/edit-undo"
copy_icon_rtl "curved-arrow-right"  "doc/tools" "actions/edit-undo"
copy_icon     "tab-icon-missing"    "doc/tools" "actions/generic"
copy_icon     "media-skip-backward" "doc/tools" "actions/media-skip-backward"
copy_icon     "media-skip-forward"  "doc/tools" "actions/media-skip-forward"
copy_icon     "star"                "doc/tools" "actions/starred"
copy_icon     "circle-check-cross"  "doc/tools" "actions/success"
copy_icon     "bookmark"            "doc/tools" "actions/user-bookmarks"
