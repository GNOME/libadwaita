#!/usr/bin/python3

import gi

gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
gi.require_version('Handy', '0.0')
from gi.repository import Handy
import sys


def print_number(dialer, number):
    print("Dial {}".format(number))


def quit(dialer, number=None):
    Gtk.main_quit()


window = Gtk.Window(title="Dialer Example with Python")
dialer = Handy.Dialer()

dialer.connect("submitted", print_number)
dialer.connect("submitted", quit)
window.connect("destroy", quit)

window.add(dialer)
window.show_all()
Gtk.main()
