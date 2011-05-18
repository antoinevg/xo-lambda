; import cairo functions
(set 'libcairo (xl:dlopen "/Users/antoine/Projects/phlo/install/lib/libcairo.dylib"))
;(set 'libcairo (xl:dlopen "/opt/local/lib/libcairo.dylib"))
;(set 'libcairo (xl:dlopen "libcairo.dylib"))
;(set 'libcairo (xl:dlopen "libcairo.so"))
(set 'cairo:image-surface-create (xl:dlsym libcairo "cairo_image_surface_create"))
(set 'cairo:create               (xl:dlsym libcairo "cairo_create"))
(set 'cairo:translate            (xl:dlsym libcairo "cairo_translate"))
(set 'cairo:rotate               (xl:dlsym libcairo "cairo_rotate"))
(set 'cairo:move-to              (xl:dlsym libcairo "cairo_move_to"))
(set 'cairo:set-source-rgba      (xl:dlsym libcairo "cairo_set_source_rgba"))
(set 'cairo:set-source-rgb       (xl:dlsym libcairo "cairo_set_source_rgb"))
(set 'cairo:set-line-width       (xl:dlsym libcairo "cairo_set_line_width"))
(set 'cairo:line-to              (xl:dlsym libcairo "cairo_line_to"))
(set 'cairo:close-path           (xl:dlsym libcairo "cairo_close_path"))
(set 'cairo:rectangle            (xl:dlsym libcairo "cairo_rectangle"))
(set 'cairo:fill                 (xl:dlsym libcairo "cairo_fill"))
(set 'cairo:stroke               (xl:dlsym libcairo "cairo_stroke"))
(set 'cairo:surface-write-to-png (xl:dlsym libcairo "cairo_surface_write_to_png"))
(set 'cairo:destroy              (xl:dlsym libcairo "cairo_destroy"))
(set 'cairo:surface-destroy      (xl:dlsym libcairo "cairo_surface_destroy"))

; create surface & context
(set 'console:width 492) 
(set 'console:height 344)
(set 'surface (cairo:image-surface-create 0 console:width console:height))
(set 'context (cairo:create surface))

; draw a boring thing
;(cairo:set-source-rgba context 1.0 1.0 1.0 0.5)
;(cairo:set-line-width context 15.0)
;(cairo:move-to context 200.0 100.0)
;(cairo:line-to context 300.0 300.0)
;(cairo:line-to context 100.0 300.0)
;(cairo:close-path context)
;(cairo:stroke context)

; draw a pretty thing
; TODO - console:width
(set 'pi 3.141592)
(set 'pretty '(lambda (cr arc theta) 
  (cond ((lte arc 0.0) 0.0) 
        ('t (progn 
              (print arc)
              (cairo:rotate cr theta)
              (cairo:rectangle cr 50.0 50.0 80.0 30.0)
              (cairo:fill cr)
              (xl:gc)
              (pretty cr (sub arc theta) theta))))))
(cairo:set-source-rgba context 0.0 0.0 1.0 0.5)
(cairo:translate context (div console:width 2.0) (div console:height 2.0))
(pretty context (mul pi 2.0) (div pi 20.0))

; write out to file
(cairo:surface-write-to-png surface "/tmp/triangle-lithp.png")

; clean up
(cairo:destroy context)
(cairo:surface-destroy surface)
(xl:dlclose libcairo)
