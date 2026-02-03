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

copy_icon     "adaptive"            "src"  "actions/adw-adaptive-preview"
copy_icon     "object-select"       "src"  "actions/adw-entry-apply"
copy_icon     "disclose-arrow-up"   "src"  "actions/adw-expander-arrow"
copy_icon     "external-link"       "src"  "actions/adw-external-link"
copy_icon     "mail-send"           "src"  "actions/adw-mail-send"

copy_icon     "tab-counter"         "src"  "status/adw-tab-counter"
copy_icon     "tab-icon-missing"    "src"  "status/adw-tab-icon-missing"
copy_icon     "tab-overflow"        "src"  "status/adw-tab-overflow"
copy_icon     "view-pin"            "src"  "status/adw-tab-unpin"
copy_icon     "person"              "src"  "status/avatar-default"

copy_icon     "bounce"              "demo" "actions/animations"
copy_icon     "edit-delete"         "demo" "actions/avatar-delete"
copy_icon     "document-save"       "demo" "actions/avatar-save"
copy_icon     "alarm-clock"         "demo" "actions/clock-alarm"
copy_icon     "stopwatch"           "demo" "actions/clock-stopwatch"
copy_icon     "sand-watch"          "demo" "actions/clock-timer"
copy_icon     "globe"               "demo" "actions/clock-world"
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

copy_icon     "speaker-cross"       "demo" "status/tab-audio-muted"
copy_icon_rtl "speaker-cross-rtl"   "demo" "status/tab-audio-muted"
copy_icon     "speaker"             "demo" "status/tab-audio-playing"
copy_icon_rtl "speaker-rtl"         "demo" "status/tab-audio-playing"
