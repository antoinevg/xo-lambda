# Copyright (c) 2008, Antoine van Gelder <hummingbird@hivemind.net>
# All rights reserved.                                                                                            
#                                                                                                                   
# Permission is hereby granted, free of charge, to any person obtaining a  
# copy of this software and associated documentation files (the 'Software'),                                        
# to deal in the Software without restriction, including without limitation                                          
# the rights to use, copy, modify, merge, publish, distribute, and/or sell                                          
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, provided that the above copyright notice(s) and this
# permission notice appear in all copies of the Software and that both the
# above copyright notice(s) and this permission notice appear in supporting
# documentation.
#
# THE SOFTWARE IS PROVIDED 'AS IS'.  USE ENTIRELY AT YOUR OWN RISK.

import os
import logging
from gettext import gettext as _
import gtk
from sugar.activity import activity
from sugar import env
from sugar.graphics.toolbutton import ToolButton
from sugar.graphics.palette import Palette


import cairo
import ctypes
class PycairoContext(ctypes.Structure):
    _fields_ = [("PyObject_HEAD", ctypes.c_byte * object.__basicsize__),
                ("ctx", ctypes.c_void_p),
                ("base", ctypes.c_void_p)]
import console
default_width = 1200
default_height = 900

class LambdaActivity(activity.Activity):
    """A simple Lisp interpreter for the XO"""

    def __init__(self, handle):
        """Set up the activity."""

        super(LambdaActivity, self).__init__(handle)
        logging.debug('Starting the Lambda Activity')

        self.set_title(_('Lambda Activity'))

        toolbox = activity.ActivityToolbox(self)
        self._edit_toolbar = activity.EditToolbar()
        toolbox.add_toolbar(_('Edit'), self._edit_toolbar)
        self._edit_toolbar.show()
        self._edit_toolbar.undo.props.visible = False
        self._edit_toolbar.redo.props.visible = False
        self._edit_toolbar.separator.props.visible = False
        self._edit_toolbar.copy.connect('clicked', self._copy_cb)
        self._edit_toolbar.paste.connect('clicked', self._paste_cb)
        activity_toolbar = toolbox.get_activity_toolbar()
        activity_toolbar.share.props.visible = False
        activity_toolbar.keep.props.visible = False

        self.set_toolbox(toolbox)
        toolbox.show()
        
        self.lambda_window = gtk.HBox(False, 4)
        self.lambda_window.set_flags(gtk.HAS_FOCUS | gtk.CAN_FOCUS)
        self.lambda_window.set_app_paintable(True)
        self.lambda_window.show()
        self.set_canvas(self.lambda_window)
        self.show_all()

        #self.window.set_default_size(default_width, default_height)
        context = self.lambda_window.window.cairo_create()
        cairo_t = PycairoContext.from_address(id(context)).ctx
        self.console = console.Console(cairo_t, 24.0)
        self.connect('key-press-event', self.__key_press_event)
        self.lambda_window.connect('expose-event',    self.__expose_event)
        self.canvas.grab_focus()


    def __key_press_event(self, widget, event):
        if event.type is not gtk.gdk.KEY_PRESS:
            print "Not a gtk.gdk.KEY_PRESS"
            return

        keyval = event.keyval
        if event.state & gtk.gdk.CONTROL_MASK:         
            if keyval == ord('x'): 
                self.__extended_mode = True
            elif keyval == ord('e'):
                if self.__extended_mode: self.console.key_execute() # inferior-emacs-mode :)
            elif keyval == ord('g'):
                self.__extended_mode = False
            elif keyval == gtk.keysyms.Return or keyval == ord(' '):
                self.console.key_execute()
            self.lambda_window.queue_draw()
            return True
        if keyval == gtk.keysyms.Escape:
            gtk.main_quit()
        elif keyval == gtk.keysyms.Return:
            keyval = ord('\n') 
        elif keyval == gtk.keysyms.Shift_L:
            return False
        elif keyval == gtk.keysyms.Shift_R:
            return False
        elif keyval == gtk.keysyms.BackSpace:
            self.console.key_backspace()
        elif keyval == gtk.keysyms.Delete:
            self.console.key_delete()
        elif keyval == gtk.keysyms.Left:
            self.console.key_left()
        elif keyval == gtk.keysyms.Right:
            self.console.key_right()
        elif keyval == gtk.keysyms.Up:
            self.console.key_up()
        elif keyval == gtk.keysyms.Down:
            self.console.key_down()
        if keyval >= 0 and keyval <= 255:
            self.console.key_press(keyval)
        self.lambda_window.queue_draw()
        return True


    def __expose_event(self, widget, event):
        self.console.width  = widget.allocation.width
        self.console.height = widget.allocation.height
        context = widget.window.cairo_create()
        context.set_source_rgb(1, 1, 1)
        context.paint()
        context.save()
        cairo_t = PycairoContext.from_address(id(context)).ctx
        self.console.expose(cairo_t)
        context.restore()
        return False


    def _copy_cb(self, button):
        pass


    def _paste_cb(self, button):
        pass

