#!/usr/bin/env python2.5

import cairo

surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 400, 400)
ctx = cairo.Context(surface)
ctx.set_line_width(15)
ctx.move_to(200, 100)
ctx.line_to(300, 300)
ctx.rel_line_to(-200, 0)
ctx.close_path()
ctx.stroke()
surface.write_to_png("triangle-python.png")

