; open libc
(set 'libc (xl:dlopen "libc.dylib"))
;(set 'libc (xl:dlopen "/lib/libc.so.6"))

; pull some functions out of libc
;(set 'c:malloc (xl:dlsym libc "malloc"))
;(set 'c:free   (xl:dlsym libc "free"))
(set 'c:printf (xl:dlsym libc "printf"))

; test malloc
;(set 'c:gcvt   (xl:dlsym libc "gcvt"))
;(set 'buffer (c:malloc 32))
;(c:gcvt 23.0 6 buffer)
;(c:free buffer)

; need to put some strain on the system
(set '+ 'add)
(set '- 'sub)
(set '* 'mul)
(set '/ 'div)
(set 'factorial '(label factorial (lambda (x) (cond ((eq x 0) 1) ('t (* x (factorial (- x 1))))))))

; test printf
(c:printf "Simple string")
(c:printf "Testing strings: %s" "a string")
;(c:printf "Testing integer numbers: %d" 42)
;(c:printf "Testing real numbers: %f" 42.0)
;(c:printf "Testing two integer numbers: %d %d" 1 2)
;(c:printf "Testing two real numbers: %f %f" 1.0 2.0)
;(c:printf "Testing mixed numbers: %f %d %f" 1.0 2 3.0)
;(c:printf "Testing mixed numbers: %d %f %d" 1 2.0 3)
;(c:printf "Testing two mixed args: %s %d" "one" 2)
;(c:printf "Testing two mixed args: %s %f" "one" 2.0)
;(c:printf "Testing three mixed args: string '%s' integer '%d' string: '%f'" "one" 2 3.0)
;(c:printf "Testing mixed numbers and strings: %d %f %d %f %f %d %d %s" 1 2.0 3 4.0 5.0 6 7 "eight")
;(c:printf "Testing mixed numbers and strings: %d %s %f %s %d" 1 "two" 3.0 "four" 5)

(factorial 40.0)
(factorial 40.0)

;(factorial 48.0)

;(c:printf "Factorial: %f" (factorial 1.0))

; close libc
(xl:dlclose libc)

