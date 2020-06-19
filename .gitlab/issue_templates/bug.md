# What problem did you encounter?

## In what part of libhandy did you experience the problem? Note that multiple boxes may be checked.

 - [ ] build system
 - [ ] documentation
 - [ ] example application
 - [ ] HdyActionRow
 - [ ] HdyCarousel
 - [ ] HdyClamp
 - [ ] HdyComboRow
 - [ ] HdyEnumValueObject
 - [ ] HdyExpanderRow
 - [ ] HdyHeaderBar
 - [ ] HdyKeypad
 - [ ] HdyLeaflet
 - [ ] HdyPreferencesGroup
 - [ ] HdyPreferencesPage
 - [ ] HdyPreferencesRow
 - [ ] HdyPreferencesWindow
 - [ ] HdySearchBar
 - [ ] HdySqueezer
 - [ ] HdySwipeable
 - [ ] HdySwipeGroup
 - [ ] HdyTitleBar
 - [ ] HdyValueObject
 - [ ] HdyViewSwitcher
 - [ ] HdyViewSwitcherBar
 - [ ] somewhere else (please elaborate)

## What is the actual behaviour?

## What is the expected behaviour?

## How to reproduce?

  Please provide steps to reproduce the issue. If it's a graphical issue please
  attach screenshot.

# Which version did you encounter the bug in?

 - [ ] I compiled it myself. If you compiled libhandy from source please provide the
   git revision via e.g. by running ``git log -1 --pretty=oneline`` and pasting
   the output below.

 - [ ] I used the precompiled Debian package (e.g. by running a prebuilt
   image). Please determine which package you have installed and paste the package status (dpkg -s)

```
$ dpkg -l | grep libhandy
ii  gir1.2-handy-1:amd64                1.0.0~203.gbp18952a                     amd64        GObject introspection files for libhandy
ii  libhandy-1-0:amd64                  1.0.0~203.gbp18952a                     amd64        Library with GTK+ widgets for mobile phones
ii  libhandy-1-dev:amd64                1.0.0~203.gbp18952a                     amd64        Development files for libhandy

$ dpkg -s libhandy-1-0
```

# What hardware are you running libhandy on?

 - [ ] amd64 qemu image
 - [ ] Librem5 devkit
 - [ ] other (please elaborate)

# Relevant logfiles

  Please provide relevant logs with ``G_MESSAGES_DEBUG=all <yourappliation>``

