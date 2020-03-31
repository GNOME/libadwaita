#!/usr/bin/python3

import gi

gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
gi.require_version('Handy', '1')
from gi.repository import Handy
import sys


window = Gtk.Window(title = "Keypad Example with Python")
vbox = Gtk.Box(orientation = Gtk.Orientation.VERTICAL)
entry = Gtk.Entry()
keypad = Handy.Keypad()

vbox.add(entry)     # widget to show dialed number
vbox.add(keypad)
vbox.set_halign(Gtk.Align.CENTER)
vbox.set_valign(Gtk.Align.CENTER)

vbox.props.margin = 18
vbox.props.spacing = 18
keypad.set_row_spacing(6)
keypad.set_column_spacing(6)

keypad.set_entry(entry)     # attach the entry widget

window.connect("destroy", Gtk.main_quit)
window.add(vbox)
window.show_all()
Gtk.main()
